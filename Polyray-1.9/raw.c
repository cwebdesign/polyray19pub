/*
  raw.c

  Copyright (C) 1994-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#include "defs.h"
#include "memory.h"
#include "io.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "csg.h"
#include "wavefrnt.h"
#include "raw.h"

static void RawRender(Viewpoint *, BinTree *, Object *);
static int RawIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
static int RawInside(Object *, Vec);
static void RawDelete(Object *);

ObjectProcs RawProcs = {
   RawRender,
   NULL,
   GenericInitialize,
   RawIntersect,
   RawInside,
   GenericCopy,
   RawDelete,
   };

VecVerts *
new_vecvert(float x, float y, float z, float w)
{
   VecVerts *vert;
   vert = polyray_malloc(sizeof(VecVerts));
   vert->V[0] = x;
   vert->V[1] = y;
   vert->V[2] = z;
   vert->V[3] = w;
   vert->next = NULL;
   return vert;
}

Faces *
new_face(int vcount, float *v)
{
   int i;
   Faces *face;

   face = polyray_malloc(sizeof(Faces));
   face->verts = polyray_malloc(vcount * sizeof(long));
   face->vcount = vcount;
   for (i=0;i<vcount;i++)
      face->verts[i] = (long)v[i] - 1;
   face->next = NULL;
   return face;
}

trivstack *
new_tristack(trivstack *tristack)
{
   trivstack *newstack;

   newstack = polyray_malloc(sizeof(struct trivertstack_struct));
   newstack->texture = NULL;
   newstack->verts   = NULL;
   newstack->fstack  = NULL;
   newstack->tcount  = 0;
   newstack->nflag   = 0;
   newstack->uvflag  = 0;
   newstack->next    = tristack;
   tristack = newstack;
   return tristack;
}

static int
Vertexcmp(fVec V1, fVec V2)
{
   int i, j;

   j = 0;
   for (i=0;i<3 && j==0;i++)
      if (V1[i] < V2[i])
         j = -1;
      else if (V1[i] > V2[i])
         j = 1;
   return j;
}

static void
average_vertices(long cnt, fVec *V, fVec *N, float smooth_angle)
{
   long i, j, k, smooth_count;
   fVec Nt, N0, LastV;
   int share_flag;
   unsigned char *nflag;

   /* Flag array to keep track of which normals have been modified */
   nflag = polyray_malloc(cnt * sizeof(unsigned char));
   for (i=0;i<cnt;i++)
      nflag[i] = 0;

   /* March forward through the vertices.  As we find a group that
      that are shared by multiple faces, we average the normals */
   for (i=0,j=0,share_flag=0;i<cnt;)
      if (!share_flag) {
         /* New vertex value, save it for comparison with the
            ones that follow */
         VecCopy(V[i], LastV);
         share_flag = 1;
         j = i;
         }
      else if (V[i][0] != LastV[0] || V[i][1] != LastV[1] ||
               V[i][2] != LastV[2] || i == cnt-1) {
         if (i == cnt-1)
            i++;

         /* No longer matching, average and reset */
         share_flag = 0;
         MakeVector(0.0f, 0.0f, 0.0f, Nt);

         /* Look for all normals within the smoothing angle.  This
            process is repeated until all collections of normals
            are found that are within the smoothing angle. */
         do {
            /* Look for a set of normals within the group of
            vertices from j to i that are close enough to average */
            smooth_count = 0;
            for (k=j;k<i;k++)
               if (nflag[k] == 0 && smooth_count == 0) {
                  VecCopy(N[k], N0)
                  VecCopy(N[k], Nt)
                  smooth_count = 1;
                  nflag[k] = 1;
                  }
               else if (nflag[k] == 0 &&
                        VecDot(N[k], N0) > smooth_angle) {
                  VecAdd(N[k], Nt, Nt)
                  smooth_count++;
                  nflag[k] = 1;
                  }

            fVecNormalize(Nt);

            /* Modify the normals that contributed to the
               averaged one. */
            for (k=j;k<i;k++)
               if (nflag[k] == 1) {
                  VecCopy(Nt, N[k])
                  nflag[k] = 2;
                  }
            } while (smooth_count > 0);
         }
      else
         i++;

   polyray_free(nflag);
}

/* Sort all the vertices so that any duplicates will be
   sitting next to each other.  Note that a fuzz value
   would be nice in the vertex comparison. */
static void
sort_raw_triangles(long cnt, fVec *V, fVec *N, long *vindices)
{
   long i, j, l, ir, rrv;
   fVec rra, rrn;

   /* Sort based on the vertex values.  Swap the normals
      and triangle indices as we go. */
   l = (cnt >> 1) + 1;
   ir = cnt;
   for (;;) {
      if (l > 1) {
         --l;
         VecCopy(V[l-1], rra)
         VecCopy(N[l-1], rrn)
         rrv = vindices[l-1];
         }
      else {
         VecCopy(V[ir-1], rra)
         VecCopy(N[ir-1], rrn)
         rrv = vindices[ir-1];

         VecCopy(V[0], V[ir-1])
         VecCopy(N[0], N[ir-1])
         vindices[ir-1] = vindices[0];

         if (--ir == 1) {
            VecCopy(rra, V[0])
            VecCopy(rrn, N[0])
            vindices[0] = rrv;
            break;
            }
         }
      i = l;
      j = l << 1;
      while (j <= ir) {
         if (j < ir && Vertexcmp(V[j-1], V[j]) < 0)
            j++;
         if (Vertexcmp(rra, V[j-1]) < 0) {
            VecCopy(V[j-1], V[i-1])
            VecCopy(N[j-1], N[i-1])
            vindices[i-1] = vindices[j-1];
            j += (i = j);
            }
         else
            j = ir + 1;
         }

      VecCopy(rra, V[i-1])
      VecCopy(rrn, N[i-1])
      vindices[i-1] = rrv;
      }

   /* All done sorting */
}

/* Average vertex normals for the entire mesh.  Note that N0 must be
   allocated, be the same size as V0, and contain valid values */
static void
smooth_raw_triangles(long cnt, fVec *V0, fVec *N0, float smooth_angle)
{
   long *vindices, *vtmp;
   long i, j;
   fVec *V, *N;

   /* Copy the vertices into temporary arrays */
   V = polyray_malloc(cnt * sizeof(fVec));
   N = polyray_malloc(cnt * sizeof(fVec));

   /* Reference vertices to triangles.  It's easy since the
      triangle vertices are stored one after another in
      the array. */
   vindices = polyray_malloc(cnt * sizeof(long));
   for (i=0,vtmp=vindices;i<cnt;i++,vtmp++) {
      VecCopy(V0[i], V[i]);
      VecCopy(N0[i], N[i]);
      *vtmp = i;
      }

   /* Rearrange vertices to make them easier to manipulate */
   sort_raw_triangles(cnt, V, N, vindices);

   /* Average the normals around a shared vertex */
   average_vertices(cnt, V, N, smooth_angle);

   /* Modify the normal values in the original raw object */
   for (i=0;i<cnt;i++) {
      j = vindices[i];
      VecCopy(N[i], N0[j])
      }

   /* Free up the temporary arrays */
   polyray_free(V);
   polyray_free(N);
   polyray_free(vindices);
}

static int
Process_Raw_File(FILE *filep, char *rawfile, trivstack **tristackptr, float smooth)
{
   char rbuf[MAXTRILINE], tbuf1[MAXTRILINE], tbuf2[MAXTRILINE];
   fVec V0, V1, V2;
   fVec N0, N1, N2;
   fVec U0, U1, U2;
   Vec B[3];
   Flt d;
   triverts *tempt;
   trivstack *tristack;
   long lcnt;
   int vcnt, ntype, wflag;
   void *texptr;

   /* Allocate the first entry in the collection stack */
   tristack = polyray_malloc(sizeof(struct trivertstack_struct));
   tristack->texture = NULL;
   tristack->verts   = NULL;
   tristack->tcount  = 0;
   tristack->nflag   = (smooth > 0.0 ? 1 : 0);
   tristack->uvflag  = 0;
   tristack->next    = NULL;

   wflag = 0;
   /* Read the entire file, processing triangles as we go. */
   for (lcnt=0;;lcnt++) {
      if (fgets(rbuf, MAXTRILINE, filep) == NULL)
         break;
      vcnt = sscanf(rbuf,
     "%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g",
                    &V0[0], &V0[1], &V0[2],
                    &V1[0], &V1[1], &V1[2],
                    &V2[0], &V2[1], &V2[2],
                    &N0[0], &N0[1], &N0[2],
                    &N1[0], &N1[1], &N1[2],
                    &N2[0], &N2[1], &N2[2],
                    &U0[0], &U0[1],
                    &U1[0], &U1[1],
                    &U2[0], &U2[1]);

      if (vcnt == 9) {
         /* Try reading a texture name from the end... */
         vcnt = sscanf(rbuf,
                       "%g %g %g %g %g %g %g %g %g %s",
                    &V0[0], &V0[1], &V0[2],
                    &V1[0], &V1[1], &V1[2],
                    &V2[0], &V2[1], &V2[2],
                    &tbuf1[0]);
         }

      if (vcnt <= 0) {
         if (tbuf1[0] == '#')
            /* Comment in the data, ignore (may be an obj file) */
            continue;

         vcnt = sscanf(rbuf, "%s %s", tbuf1, tbuf2);
         if (vcnt == 2 &&
             (!strcmp(tbuf1, "g") || !strcmp(tbuf1, "v") ||
              !strcmp(tbuf1, "f"))) {
            /* This is an obj format file */
            if (tristack->tcount != 0 ||
                tristack->next != NULL) {
               fclose(filep);
               error("Unreadable raw file");
               }
            else {
               polyray_free(tristack);
               return 0;
               }
            }
         if (vcnt == 1) {
            /* We have a texture name for the triangles that follow */
            if (tristack->tcount != 0)
               /* We already had triangles in the stack, we need to allocate
                  a new stack for the triangles that follow */
               tristack = new_tristack(tristack);
            /* Look up the texture name */
            Lookup_Definition(tbuf1, &ntype, &texptr);
            if (ntype != T_TEXTURE) {
               warning("Texture '%s' undefined\n", tbuf1);
               tristack->texture = NULL;
               }
            else
               tristack->texture = texptr;
            }

         /* No triangle information here, skip to next line */
         continue;
         }
      else if (vcnt != 9 && vcnt != 10 && vcnt != 18 && vcnt != 24) {
         /* Look to see if it is a texture name... */
         warning("Bad raw triangle at line %ld of %s.  (%d entries)\n",
                 lcnt+1, rawfile, vcnt);
         continue;
         }
      else
         tristack->tcount++;

      /* Add this triangle to the list */
      tempt = (triverts *)polyray_malloc(sizeof(struct triverts_struct));
      VecCopy(V0, tempt->V[0])
      VecCopy(V1, tempt->V[1])
      VecCopy(V2, tempt->V[2])

      if (vcnt == 10) {
         /* Look up the texture name */
         Lookup_Definition(tbuf1, &ntype, &texptr);
         if (ntype != T_TEXTURE) {
            if (!wflag) {
               warning("Texture '%s' undefined\n", tbuf1);
               wflag = 1;
               }
            tempt->texture = NULL;
            }
         else
            tempt->texture = texptr;
         }
      else
         tempt->texture = NULL;

      if (vcnt >= 18) {
         /* Vertices plus normals */
         tristack->nflag = 1;
         VecCopy(N0, tempt->N[0])
         VecCopy(N1, tempt->N[1])
         VecCopy(N2, tempt->N[2])
         }
      else {
         VecSub(V1, V0, B[0]);
         VecSub(V2, V0, B[1]);
         VecCross(B[1], B[0], B[2]);
         d = sqrt(VecDot(B[2], B[2]));
         if (d < EPSILON)
            /* Degenerate triangle, ignore the error */
            d = 1.0;
         else
            d = 1.0 / d;
         VecScale(d, B[2]);
         VecCopy(B[2], tempt->N[0])
         VecCopy(B[2], tempt->N[1])
         VecCopy(B[2], tempt->N[2])
         }

      if (vcnt == 24) {
         /* Vertices, normals, and u/v coordinates */
         tristack->uvflag   = 1;
         VecCopy(U0, tempt->U[0])
         VecCopy(U1, tempt->U[1])
         VecCopy(U2, tempt->U[2])
         }
      else {
         VecCopy(V0, tempt->U[0])
         VecCopy(V1, tempt->U[1])
         VecCopy(V2, tempt->U[2])
         }

      tempt->next = tristack->verts;
      tristack->verts = tempt;
      }

   /* Just in case we have more than one textured object, we
      need to update the triangle stack */
   *tristackptr = tristack;
   if (tristack->tcount == 0) {
      *tristackptr = tristack->next;
      polyray_free(tristack);
      return 0;
      }
   else
      return tristack->tcount;
}

static void
make_triangle_objects(Object *obj, long vcnt, trivstack *tristack,
                      fVec *V, fVec *N, fVec *U)
{
   RawData *raw = (RawData *)obj->o_data;
   TriangleObject *tobj;
   trivstack *laststack;
   triverts *tri, *last_tri;
   long i, cnt;
   bbox_info box;

   obj->o_vertices->n = vcnt;
   obj->o_vertices->V = V;
   obj->o_vertices->N = N;
   obj->o_vertices->U = U;

   cnt = 0;
   while (tristack != NULL) {
      for (i=0,tri=tristack->verts;
           i<tristack->tcount&&tri!=NULL;
           i++) {
         tobj = (TriangleObject *)polyray_malloc(sizeof(TriangleObject));
         tobj->o_type = T_POLYGON;
         if (tri->texture != NULL)
            tobj->o_texture = tri->texture;
         else if (tristack->texture != NULL)
            tobj->o_texture = tristack->texture;
         else
            tobj->o_texture = NULL;
         tobj->o_parent = obj;
         tobj->o_vert[0] = cnt;
         tobj->o_vert[1] = cnt + 1;
         tobj->o_vert[2] = cnt + 2;
         VecCopy(tri->V[0], V[cnt])
         VecCopy(tri->V[1], V[cnt+1])
         VecCopy(tri->V[2], V[cnt+2])
         if (N) {
            tobj->o_nvert[0] = cnt;
            tobj->o_nvert[1] = cnt + 1;
            tobj->o_nvert[2] = cnt + 2;
            VecCopy(tri->N[0], N[cnt])
            VecCopy(tri->N[1], N[cnt+1])
            VecCopy(tri->N[2], N[cnt+2])
            }
         else {
            tobj->o_nvert[0] = -1;
            tobj->o_nvert[1] = -1;
            tobj->o_nvert[2] = -1;
            }
         if (U) {
            VecCopy(tri->U[0], U[cnt])
            VecCopy(tri->U[1], U[cnt+1])
            VecCopy(tri->U[2], U[cnt+2])
            }
         if (calc_triangle_bounds(tobj, &box)) {
            VecCopy(box.lower_left, tobj->o_bnd.lower_left);
            VecCopy(box.lengths, tobj->o_bnd.lengths);
            raw->objs.members.list = push_object(raw->objs.members.list,
                                                  (Object *)tobj);
            raw->objs.members.count++;
            cnt += 3;
            }
         else {
            /* Degenerate triangle, remove it. */
            polyray_free(tobj);
         }

         last_tri = tri;
         tri = tri->next;
         polyray_free(last_tri);
         }
       if (i < tristack->tcount || tri != NULL)
          error("Didn't process all faces");

      laststack = tristack;
      tristack = tristack->next;
      polyray_free(laststack);
      }
}

/* Turn the triangles we read into something that the rest of the
   system can deal with.  Note that the contents of tristack will
   be deallocated as a result of this routine. */
static void
process_raw_triangles(Object *obj, trivstack *tristack)
{
   RawData *raw = (RawData *)obj->o_data;
   fVec *V, *N, *U;
   int nflag, uvflag;
   long vcnt;
   trivstack *laststack;

   /* Allocate the triangle storage */
   obj->o_vertices = (ObjectVertices *)
                     polyray_malloc(sizeof(ObjectVertices));

   /* Count how many vertices we need (sum the counts from each
      entry in the stack), check to see if any entry in the
      stack has either normals or uv-coordinates.  */
   nflag = 0;
   uvflag = 0;
   for (vcnt=0,laststack=tristack;
        laststack!=NULL;
        laststack=laststack->next) {
      if (laststack->nflag)
         nflag = 1;
      if (laststack->uvflag)
         uvflag = 1;
      vcnt += 3 * laststack->tcount;
      }

   V = (fVec *)polyray_malloc(vcnt * sizeof(fVec));
   if (nflag)
      N = (fVec *)polyray_malloc(vcnt * sizeof(fVec));
   else
      N = NULL;
   if (uvflag)
      U = (fVec *)polyray_malloc(vcnt * sizeof(fVec));
   else
      U = NULL;

   make_triangle_objects(obj, vcnt, tristack, V, N, U);

   if (N != NULL)
      smooth_raw_triangles(vcnt, V, N, raw->smooth);
}

static trivstack *
Parse_Raw_File(Object *obj, char *rawfile, float smooth)
{
   FILE *filep;
   trivstack *tristack;

   /* A raw file must be ASCII text with line delimiters */
   if ((filep = PathFileOpen(POLYRAY_PATH_STRING, rawfile, "rt")) == NULL)
      error("Unable to open raw triangle file: '%s'", rawfile);

   if (Process_Raw_File(filep, rawfile, &tristack, smooth)) {
      process_raw_triangles(obj, tristack);
      }
   else if (!Process_Obj_File(obj, filep))
      error("Unable to process raw data file");

   /* Close the object file */
   fclose(filep);

   /* Return the raw object */
   return tristack;
}

#define MAX_OCCLUSIONS 1000

static int
RawInside(Object *obj, Vec P)
{
   RawData *raw = (RawData *)obj->o_data;
   Ray ray;
   Isect hit;
   int crossings, old_test;

   /* Don't want to do CSG check during this particular test,
      therefore we use a global to signal the intersection
      routine to skip that part. */
   old_test = Shadow_Test;
   Shadow_Test = 3;

   /* Use Jordans rule for inside/outside testing */
   InvTxVector1(ray.P, P, obj->o_trans)
   MakeVector(0.12345, 0.98765, 0.57392, ray.D);
   VecNormalize(ray.D);

   crossings = 0;
   while (crossings < MAX_OCCLUSIONS) {
      if (Intersect(NULL, &raw->objs, &ray, SMALL, PLY_HUGE, &hit)) {
         /* Move up a little closer to the target */
         VecCopy(hit.W, ray.P)
         crossings++;
         }
      else
         break;
      }

   Shadow_Test = old_test;

   return crossings & 1;
}

int
RawIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
             Flt mindist, Flt maxdist, Isect *hit)
{
   RawData *raw = (RawData *)obj->o_data;

   if (Intersect(Eye, &raw->objs, ray, mindist, maxdist, hit)) {
      /* Compensate for the fact that a raw object used in
         a define statement will have the o_parent component
         set to the original copy of the object rather than
         the instantiated copy. */
      TxVector(hit->W, hit->W, obj->o_trans);
      TxNormal(hit->N, hit->N, obj->o_trans);

      if (Shadow_Test != 3 && !Inside_CSG_Node(obj->o_csg_tree, hit->W)) {
         hit->flag = 0;
         return 0;
         }

      /* Check u,v boundaries */
      if ((Global_Shade_Flag & UV_CHECK) &&
          (obj->o_sflag & UV_CHECK))
         if (hit->U[0] < obj->o_uv_bounds[0] ||
             hit->U[0] > obj->o_uv_bounds[1] ||
             hit->U[1] < obj->o_uv_bounds[2] ||
             hit->U[1] > obj->o_uv_bounds[3])
            return 0;

      hit->obj = obj;
      return 1;
      }
   else
      return 0;
}

Object *
MakeRaw(Object *obj, char *rawfile, Flt smooth)
{
   RawData *raw;
   int i, OldOptim;
   Flt ftemp;
   Vec mins, maxs;
   ostackptr objs;

   obj->o_type = T_RAW_TRIANGLES;
   obj->o_procs = &RawProcs;

   raw = polyray_malloc(sizeof(RawData));
   obj->o_data = (void *)raw;

   /* Create a BinTree to hold the raw triangles */
   Initialize_BinTree(&raw->objs);

   /* Initialize the smoothing value */
   if (smooth < 0) {
      warning("Smooth angle must be 0 degrees or larger, found %g\n", smooth);
      smooth = 0.0;
      }
   else if (smooth > 180.0) {
      warning("Smooth angle must be 180 degrees or smaller, found %g\n",
              smooth);
      smooth = 180.0;
      }
   raw->smooth = cos(M_PI * smooth / 180.0);

   /* Read the triangles in from the file... */
   Parse_Raw_File(obj, rawfile, (float)smooth);

   /* Add the triangles... */
   if ((Optimizer > 0) &&
       ((Rendering_Method == RAY_TRACING) ||
            ((Rendering_Method == GOURAD_SHADE ||
              Rendering_Method == SCAN_CONVERSION) &&
             (Global_Shade_Flag &
              (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK))))) {
      OldOptim = Optimizer;
      Optimizer = 1;
      BuildBoundingSlabs(&raw->objs);
      Optimizer = OldOptim;
      /* Copy the bounding box of the object hierarchy into
             the bounding box for the object itself. */
      VecCopy(raw->objs.slab_root->o_bnd.lower_left,
              obj->o_bnd.lower_left)
      VecCopy(raw->objs.slab_root->o_bnd.lengths,
              obj->o_bnd.lengths)
      }
   else {
      /* Still need the bounding box, look over all objects and find
         the min/max */
      MakeVector(-PLY_HUGE, -PLY_HUGE, -PLY_HUGE, maxs)
      MakeVector( PLY_HUGE,  PLY_HUGE,  PLY_HUGE, mins)
      for (objs=raw->objs.members.list;objs!=NULL;objs=objs->next) {
             for (i=0;i<3;i++) {
                ftemp = objs->element->o_bnd.lower_left[i];
                if (ftemp < mins[i]) mins[i] = ftemp;
                ftemp += objs->element->o_bnd.lengths[i];
                if (ftemp > maxs[i]) maxs[i] = ftemp;
                }
             }
      VecCopy(mins, obj->o_bnd.lower_left)
      VecSub(maxs, mins, obj->o_bnd.lengths)
      }

   return obj;
}

/* Deallocate all of the dynamic storage associated with
   a height field object */
static void
RawDelete(Object *object)
{
   RawData *raw = (RawData *)object->o_data;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0) return;

   /* Free the raw data */
   Delete_BinTree(&raw->objs);
   polyray_free(raw);
}

/* Recursively descend into the tree of raw triangles and spit them out */
static void
RawRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   ostackptr objs;
   RawData *raw = obj->o_data;

   if (Rendering_Method == MESH_CONVERSION) {
      /* Modify the underlying vertex information */
#if 0
      for (i=0;i<obj->o_vertices->n;i++) {
         VecCopy(obj->o_vertices->V[i], vert.W);
         if (obj->o_vertices->N != NULL)
            VecCopy(obj->o_vertices->N[i], vert.N)
         else
            VecCopy(obj->o_vertices->V[i], vert.N)
         if (obj->o_vertices->N != NULL)
            VecCopy(obj->o_vertices->U[i], vert.N)
         else
            VecCopy(obj->o_vertices->V[i], vert.U)
         for (tobj=obj;tobj!=NULL;tobj=tobj->o_parent)
            if (tobj->o_displace)
               displace_vertex(tobj->o_displace, &vert);
         VecCopy(Vert.W, obj->o_vertices->V[i]);
         if (obj->o_vertices->N != NULL)
            VecCopy(Vert.N, obj->o_vertices->N[i])
         if (obj->o_vertices->U != NULL)
            VecCopy(Vert.U, obj->o_vertices->U[i])
         }
#endif
      }
   else
      for (objs=raw->objs.members.list;objs!=NULL;objs=objs->next)
         render_prim(eye, Root, obj, objs->element);
}

