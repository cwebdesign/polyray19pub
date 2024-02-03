/*
  wavefrnt.c

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
#include "symtab.h"
#include "raw.h"
#include "subdiv.h"
#include "bound.h"
#include "wavefrnt.h"

/* Structure to hold a smoothing group */
typedef struct SmoothGroup_struct SmoothGroup;
struct SmoothGroup_struct {
   unsigned group_num;
   Faces *faces;
   SmoothGroup *next;
   };

#define BETWEEN_OBJECTS  0
#define READING_VERTICES 1
#define READING_FACES    2

#define MAX_VERTICES_PER_FACE 16

static long vertex_count, vertex_texture_count, vertex_normal_count;
static long face_count, object_count;
static Texture *current_texture;

static char rbuf[MAXTRILINE];
static int rbuf_offset = 0;
static int rbuf_length = 0;

static int
skip_white_space(FILE *filep)
{
   for (;
        (rbuf[rbuf_offset] == ' ' ||
         rbuf[rbuf_offset] == '\t' ||
         rbuf[rbuf_offset] == '\\') &&
        rbuf_offset < rbuf_length;
        rbuf_offset++)
      if (rbuf[rbuf_offset] == '\\') {
         /* Continuation character, get the next line */
         if (fgets(rbuf, MAXTRILINE, filep) != NULL) {
            rbuf_offset = 0;
            rbuf_length = strlen(rbuf);
            }
         else {
            rbuf[0] = '\0';
            rbuf_offset = 0;
            rbuf_length = 0;
            return 0;
            }
         }
      else {
         /* White space, just ignore it */
         }
   return 1;
}

static void
skip_till_white_space()
{
   for (;
        rbuf[rbuf_offset] != ' ' &&
        rbuf[rbuf_offset] != '\t' &&
        rbuf_offset < rbuf_length;
        rbuf_offset++)
      ;
}

static int
end_of_line()
{
   if (rbuf_offset == rbuf_length ||
       rbuf[rbuf_offset] == '\n' ||
       rbuf[rbuf_offset] == '\0')
      return 1;
   else
      return 0;
}

static int
read_vertex(FILE *filep, long *v, long *vt, long *vn)
{
   float v0, vt0, vn0;

   skip_white_space(filep);
   if (sscanf(&rbuf[rbuf_offset], "%g/%g/%g", &v0, &vt0, &vn0) == 3) {
      *v  = (long)v0;
      *vt = (long)vt0;
      *vn = (long)vn0;
      }
   else if (sscanf(&rbuf[rbuf_offset], "%g//%g", &v0, &vn0) == 2) {
      *v  = (long)v0;
      *vt = (long)0L;
      *vn = (long)vn0;
      }
   else if (sscanf(&rbuf[rbuf_offset], "%g/%g", &v0, &vt0) == 2) {
      *v  = (long)v0;
      *vt = (long)vt0;
      *vn = (long)0L;
      }
   else if (sscanf(&rbuf[rbuf_offset], "%g", &v0) == 1) {
      *v  = (long)v0;
      *vt = 0L;
      *vn = 0L;
      }
   else {
      error("Bad vertex data\n");
      }
   skip_till_white_space();
   return 1;
}

static Faces *
read_face(FILE *filep)
{
   Faces *face;
   int i, vcount, vtp_flag, vnp_flag;
   long v[MAX_VERTICES_PER_FACE], *vp;
   long vt[MAX_VERTICES_PER_FACE], *vtp;
   long vn[MAX_VERTICES_PER_FACE], *vnp;

   vp = &v[0];
   vtp = &vt[0];
   vnp = &vn[0];
   vtp_flag = 0;
   vnp_flag = 0;
   for (vcount=0;
        vcount<MAX_VERTICES_PER_FACE && !end_of_line();
        vcount++,vp++,vtp++,vnp++) {
      read_vertex(filep, vp, vtp, vnp);
      if (*vtp != 0)
         vtp_flag = 1;
      if (*vnp != 0)
         vnp_flag = 1;
      skip_white_space(filep);
      }
   if (vcount == MAX_VERTICES_PER_FACE)
      warning("Too many vertices in a face");
   else if (vcount < 3) {
      warning("Too few vertices in a face");
      return NULL;
      }

   face = polyray_malloc(sizeof(Faces));
   face->verts = polyray_malloc(vcount * sizeof(long));
   if (vtp_flag)
      face->tverts = polyray_malloc(vcount * sizeof(long));
   else
      face->tverts = NULL;
   if (vnp_flag)
      face->nverts = polyray_malloc(vcount * sizeof(long));
   else
      face->nverts = NULL;
   face->vcount = vcount;
   for (i=0;i<vcount;i++) {
      if (v[i] > 0)
         face->verts[i] = v[i] - 1;
      else
         face->verts[i] = vertex_count - vt[i];
      if (vtp_flag) {
         if (vt[i] > 0)
            face->tverts[i] = vt[i] - 1;
         else
            face->tverts[i] = vertex_texture_count - vt[i];
         }
      if (vnp_flag) {
         if (vn[i] > 0)
            face->nverts[i] = vn[i] - 1;
         else
            face->nverts[i] = vertex_normal_count - vn[i];
         }
      }

   face->next = NULL;
   return face;
}

static void
make_triangles(Object *obj, long vertex_count, long normal_count,
               Faces *fstack, VecVerts *vstack, VecVerts *nstack)
{
   RawData *raw = obj->o_data;
   fVec *V, *N;
   long tcnt, triangle_count;
   VecVerts *vtemp;
   Faces *ftemp1, *ftemp2;
   TriangleObject *tobj;
   bbox_info box;

   triangle_count = 0;

   /* Now we need to allocate space for the vertices and process
      the face stacks into a set of triangles */
   obj->o_vertices = (ObjectVertices *)polyray_malloc(sizeof(ObjectVertices));
   obj->o_vertices->n = vertex_count;
   V = (fVec *)polyray_malloc(vertex_count * sizeof(fVec));
   obj->o_vertices->V = V;
   if (normal_count > 0) {
      N = (fVec *)polyray_malloc(normal_count * sizeof(fVec));
      obj->o_vertices->N = N;
      }
   else {
      N = NULL;
      obj->o_vertices->N = NULL;
      }
   obj->o_vertices->U = NULL;


   /* Copy the vertices into the V array */
   for (tcnt=vertex_count-1;vstack!=NULL&&tcnt>=0;tcnt--) {
      /* Copy this vertex into the array */
      VecCopy(vstack->V, V[tcnt])
      /* Free up the space used for this vertex */
      vtemp = vstack;
      vstack = vstack->next;
      polyray_free(vtemp);
      }
   if (tcnt != -1 || vstack != NULL)
      error("Didn't properly process .obj vertices");

   /* Copy the normals into the N array */
   for (tcnt=normal_count-1;nstack!=NULL&&tcnt>=0;tcnt--) {
      /* Copy this vertex into the array */
      VecCopy(nstack->V, N[tcnt])
      /* Free up the space used for this vertex */
      vtemp = nstack;
      nstack = nstack->next;
      polyray_free(vtemp);
      }
   if (tcnt != -1 || vstack != NULL)
      error("Didn't properly process .obj vertices");

   /* Create triangles in the form we want them */
   for (ftemp1=fstack,tcnt=0;ftemp1!=NULL;tcnt++) {
      /* We need to turn the face into a set of triangles and
         stuff each one onto the stack */
      fVec V0, V1, Norm, *verts;
      Flt d;
      int i, j, out_n, npoints, **out_verts;

      /* Allocate temporary space to hold the polygon and
         subsequent triangles */
      npoints = ftemp1->vcount;
      verts = (fVec *)polyray_malloc(npoints * sizeof(fVec));
      out_verts = (int **)polyray_malloc((npoints - 2) * sizeof(int *));
      for (j=0;j<npoints-2;j++)
         out_verts[j] = (int *)polyray_malloc(3 * sizeof(int));
      /* Stuff the vertices of the polygon into the array
         verts for subsequent chopping. */
      for (j=0;j<npoints;j++)
         VecCopy(V[ftemp1->verts[j]], verts[j])

      /* Figure out what axes to use when splitting the polygon */
      VecSub(verts[1], verts[0], V0);
      VecSub(verts[2], verts[0], V1);
      VecCross(V1, V0, Norm);
      d = sqrt(VecDot(Norm, Norm));
      if (d < EPSILON)
         /* Degenerate triangle, ignore the error */
         d = 1.0;
      else
         d = 1.0 / d;
      VecScale(d, Norm);
      if (fabs(Norm[0]) >= fabs(Norm[1]) &&
          fabs(Norm[0]) >= fabs(Norm[2])) {
         i = 1;
         j = 2;
         }
      else if (fabs(Norm[1]) >= fabs(Norm[0]) &&
               fabs(Norm[1]) >= fabs(Norm[2])) {
         i = 0;
         j = 2;
         }
      else {
         i = 0;
         j = 1;
         }
      Split_Polygon(npoints, verts, i, j, &out_n, out_verts);

      triangle_count += out_n;
      /* Add all the triangles to the list */
      for (j=0;j<out_n;j++) {
         tobj = (TriangleObject *)polyray_malloc(sizeof(TriangleObject));
         tobj->o_type = T_POLYGON;
         tobj->o_texture = ftemp1->texture;;
         tobj->o_parent = obj;
         tobj->o_vert[0] = ftemp1->verts[out_verts[j][0]];
         tobj->o_vert[1] = ftemp1->verts[out_verts[j][1]];
         tobj->o_vert[2] = ftemp1->verts[out_verts[j][2]];
         if (ftemp1->nverts) {
            tobj->o_nvert[0] = ftemp1->nverts[out_verts[j][0]];
            tobj->o_nvert[1] = ftemp1->nverts[out_verts[j][1]];
            tobj->o_nvert[2] = ftemp1->nverts[out_verts[j][2]];
            }
         else {
            tobj->o_nvert[0] = -1;
            tobj->o_nvert[1] = -1;
            tobj->o_nvert[2] = -1;
            }

         if (calc_triangle_bounds(tobj, &box)) {
            VecCopy(box.lower_left, tobj->o_bnd.lower_left)
            VecCopy(box.lengths, tobj->o_bnd.lengths)
            raw->objs.members.list = push_object(raw->objs.members.list,
                                                 (Object *)tobj);
            raw->objs.members.count++;
            }
         else
            /* Degenerate triangle, remove it. */
            polyray_free(tobj);
         }

      /* Clean up temporary memory */
      for (j=0;j<npoints-2;j++)
         polyray_free(out_verts[j]);
      polyray_free(out_verts);
      polyray_free(verts);

      /* Dispose of the ones we just looked at */
      ftemp2 = ftemp1;
      ftemp1 = ftemp1->next;
      polyray_free(ftemp2->verts);
      if (ftemp2->tverts) polyray_free(ftemp2->tverts);
      if (ftemp2->nverts) polyray_free(ftemp2->nverts);
      polyray_free(ftemp2);
      }
}

int
Process_Obj_File(Object *obj, FILE *filep)
{
   char ctype[MAXTRILINE], tbuf1[MAXTRILINE], tbuf2[MAXTRILINE];
   float v0, v1, v2, v3;
   long lcnt;
   int icnt, ntype, wflag, state;
   int i, j;
   void *texptr;
   long foffset;
   VecVerts *vstack, *nstack, *vtemp;
   Faces *fstack, *ftemp1;

   fseek(filep, 0, SEEK_SET);

   wflag = 0;
   state = BETWEEN_OBJECTS;
   vstack = NULL;
   nstack = NULL;
   fstack = NULL;
   vertex_count = 0;
   face_count = 0;
   object_count = 0;
   /* Read the entire file, processing triangles as we go. */
   for (lcnt=0;;lcnt++) {
      foffset = ftell(filep);
      if (fgets(rbuf, MAXTRILINE, filep) == NULL)
         break;
      /* First read in the command for this line */
      icnt = sscanf(rbuf, "%s", ctype);
      rbuf_offset = strlen(ctype);
      rbuf_length = strlen(rbuf);

      /* Looking for a statement like: "v x y z w" */
      if (!strcmp(ctype, "v")) {
         /* Read a vertex */
         icnt = sscanf(rbuf, "%s %g %g %g %g", tbuf1, &v0, &v1, &v2, &v3);
         if (icnt == 4 || icnt == 5) {
            /* Valid vertex */
            vtemp = new_vecvert(v0, v1, v2, (icnt == 4 ? 0.0 : v3));
            vtemp->next = vstack;
            vstack = vtemp;
            vertex_count++;
            }
         else
            warning("Bad vertex");
         continue;
         }

      /* Looking for a statement like: "vn x y z" */
      if (!strcmp(ctype, "vn")) {
         /* Read a vertex */
         icnt = sscanf(rbuf, "%s %g %g %g", tbuf1, &v0, &v1, &v2);
         if (icnt == 4) {
            /* Valid vertex */
            vtemp = new_vecvert(v0, v1, v2, 0.0 );
            vtemp->next = nstack;
            nstack = vtemp;
            vertex_normal_count++;
            }
         else
            warning("Bad normal");
         continue;
         }

      /* Looking for a statement like: "vt u v w" */
      if (!strcmp(ctype, "vt")) {
         /* Read a vertex */
         icnt = sscanf(rbuf, "%s %g %g %g", tbuf1, &v0, &v1, &v2);
         /* For now we are ignoring texture coordinates */
         continue;
         }

      /* Look for: "usemtl texture_name" */
      if (!strcmp(ctype, "usemtl")) {
         icnt = sscanf(rbuf, "%s %s", tbuf1, tbuf2);
         if (icnt == 2) {
            /* Got a texture name */
            for (i=0,j=strlen(tbuf2);i<j;i++)
               if (tbuf2[i] == '.')
                  tbuf2[i] = '_';
            Lookup_Definition(tbuf2, &ntype, &texptr);
            if (ntype != T_TEXTURE) {
               warning("Texture '%s' undefined\n", tbuf2);
               current_texture = NULL;
               }
            else
               current_texture = texptr;
            }
         else
            warning("Bad texture (usemtl) name");
         continue;
         }

      if (!strcmp(ctype, "f")) {
         /* Read a face */
         ftemp1 = read_face(filep);
         if (ftemp1 != NULL) {
            ftemp1->texture = current_texture;
            ftemp1->next = fstack;
            fstack = ftemp1;
            face_count++;
            }
         else
            warning("Bad face");
         continue;
         }
      }

   /* Turn the contents of the face stack into triangle objects.  This
      routine removes the memory associated with tristack. */
   make_triangles(obj, vertex_count, vertex_normal_count,
                  fstack, vstack, nstack);

   return face_count;
}

