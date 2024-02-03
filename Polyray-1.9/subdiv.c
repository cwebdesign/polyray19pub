/* subdiv.c

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

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
#include "bound.h"
#include "scan.h"
#include "vector.h"
#include "eval.h"
#include "symtab.h"
#include "subdiv.h"


#define VERT(i, a) ((verts[vbuffer[i]])[a])

/* Storage for polygon indices */
static int *vbuffer = NULL;
static int *poly_end = NULL;
static int axis1, axis2;

/* Find the left most vertex in the polygon that has vertices m ... n. */
static int
leftmost_vertex(int m, int n, fVec *verts)
{
   int l, i;
   double x;

   /* Assume the first vertex is the farthest to the left */
   l = m;
   x = VERT(m, axis1);

   /* Now see if any of the others are farther to the left */
   for (i=m+1;i<=n;i++)
      if (VERT(i, axis1) < x) {
         l = i;
         x = VERT(i, axis1);
         }
   return l;
}

/* Given the leftmost vertex in a polygon, this routine finds another vertex
   can be used to safely split the polygon. */
static int
split_vertex(int l, int la, int lb, int m, int n, fVec *verts)
{
   int t, k, lpu, lpl;
   double yu, yl;

   yu = MAX(VERT(l, axis2), MAX(VERT(la, axis2), VERT(lb, axis2)));
   yl = MIN(VERT(l, axis2), MIN(VERT(la, axis2), VERT(lb, axis2)));
   if (VERT(lb, axis2) > VERT(la, axis2)) {
      lpu = lb;
      lpl = la;
      }
   else {
      lpu = la;
      lpl = lb;
      }
   t = (VERT(lb, axis1) > VERT(la, axis1) ? lb : la);
/* Should this be: k=m;k<=n;k++? (was k<n) */
   for (k=m;k<=n;k++)
      if (k != la && k != l && k != lb)
         if (VERT(k, axis2) <= yu &&
             VERT(k, axis2) >= yl &&
             VERT(k, axis1) < VERT(t, axis1) &&

             ((VERT(k, axis2) - VERT(l, axis2)) *
              (VERT(lpu, axis1) - VERT(l, axis1))) <=
             ((VERT(lpu, axis2) - VERT(l, axis2)) *
              (VERT(k, axis1) - VERT(l, axis1))) &&

             ((VERT(k, axis2) - VERT(l, axis2)) *
              (VERT(lpl, axis1) - VERT(l, axis1))) >=
             ((VERT(lpl, axis2) - VERT(l, axis2)) *
              (VERT(k, axis1) - VERT(l, axis1))))
            t = k;
   return t;
}

/* Test polygon vertices to see if they are linear */
static int
linear_vertices(int m, int n, fVec *verts)
{
   /* Not doing anything right now */
   return 0;
}

/* Shift vertex indices around to make two polygons out of one. */
static void
perform_split(int m, int m1, int n, int n1)
{
   int i, j, k;
   int *vb1, *vb2;

   k = n + 3 - m1;
   /* Move the new polygon up over the place the current one sits */
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1];
        j<=n1;j++,vb1++,vb2++)
      *vb1 = *vb2;

   /* Move top part of remaining polygon */
   for (j=n,vb1=&vbuffer[n+2],vb2=&vbuffer[n];
        j>=n1;j--,vb1--,vb2--)
      *vb1 = *vb2;

   /* Move bottom part of remaining polygon */
   k = n1 - m1 + 1;
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1];
        j>=m;j--,vb1--,vb2--)
      *vb1 = *vb2;

   /* Copy the new polygon so that it sits before the remaining polygon */
   i = n + 3 - m1;
   k = m - m1;
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1+i];
        j<=n1;j++,vb1++,vb2++)
      *vb1 = *vb2;
}

/* Copy an indirectly referenced triangle into the output triangle buffer */
static void
add_new_triangle(int m, int *out_cnt, int **out_verts)
{
   int i;

   if (out_verts != NULL) {
/* printf("New: %d, %d, %d\n", vbuffer[m], vbuffer[m+1], vbuffer[m+2]); */
      i = *out_cnt;
      out_verts[i][0] = vbuffer[m  ];
      out_verts[i][1] = vbuffer[m+1];
      out_verts[i][2] = vbuffer[m+2];
      *out_cnt += 1;
      }
}

/* Turn a (possibly cocave) polygon into a set of triangles.  The
   vertex list out_verts must already be sufficiently large to
   accomodate (cnt - 2) triangles.  Each triangle is returned as
   a set of three vertex indices. */
void
Split_Polygon(int cnt, fVec *verts, int x_axis, int y_axis,
              int *out_cnt, int **out_verts)
{
   int i, m, m1, n, n1;
   int l, la, lb, ls;

   /* Make sure there is storage for intermediate vertices */
   vbuffer = (int *)malloc(3 * cnt * sizeof(int));
   if (cnt < 2) error("Split_Polygon:cnt must be at least 2");//cm, avoid buffer overrun

   poly_end = (int *)malloc(cnt * sizeof(int));
   if (vbuffer == NULL || poly_end == NULL)
      error("Failed to allocate glyph polygon buffer");

   /* Set the coordinates that we will perform the splitting on. */
   axis1 = x_axis;
   axis2 = y_axis;

   /* No triangles to start with */
   *out_cnt = 0;

   /* Initialize the polygon splitter */
   poly_end[0] = -1;
   poly_end[1] = cnt-1;

   /* Start with a strict identity of vertices in verts and vertices in
      the polygon buffer */
   for (i=0;i<cnt;i++)
      vbuffer[i] = i;

   /* Split and push polygons until they turn into triangles */
   for (i=1;i>0;) {
      m = poly_end[i-1] + 1;
      n = poly_end[i];

      if (n - m == 2) {
         if (!linear_vertices(m, n, verts))
            add_new_triangle(m, out_cnt, out_verts);
         i = i - 1;
         }
      else {
         l = leftmost_vertex(m, n, verts);
         la = (l == n ? m : l + 1);
         lb = (l == m ? n : l - 1);
         ls = split_vertex(l, la, lb, m, n, verts);
         if (ls == la || ls == lb) {
            m1 = (la < lb ? la : lb);
            n1 = (la < lb ? lb : la);
            }
         else {
            m1 = (l < ls ? l : ls);
            n1 = (l < ls ? ls : l);
            }
         perform_split(m, m1, n, n1);
         poly_end[i++] = m + n1 - m1;
         poly_end[i] = n + 2;
         }
      }

   /* Free up the temporary stacks used for polygon decomposition */
   free(vbuffer);
   vbuffer = NULL;
   free(poly_end);
   poly_end = NULL;
}

static void
displace_vertex(NODE_PTR displacement, Vertex *vert)
{
   Vec vtemp;
   Flt ftemp;
   NODE_PTR ntemp;
   int t;
   struct subst_struct subst;

   /* Build a substitution structure to evaluate the special texture with */
   VecCopy(vert->P, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   VecCopy(vert->U, subst.U);
   VecCopy(vert->W, subst.W);
   VecCopy(vert->N, subst.N);
   MakeVector(0, 0, 1, subst.I);

   if ((t = eval_node(&subst, displacement, &ftemp, vtemp, &ntemp)) == 2)
      VecAdd(vert->W, vtemp, vert->W)
   else if (t == 1)
      VecAddScaled(vert->W, ftemp, subst.N, vert->W)
   else
      error("Bad displacement function\n");
}

static void
average_normals(Poly *p)
{
   int i;
   Vec v0, v1, v2;

   if (p->n < 3) return;
   VecSub(p->vertices[1].W, p->vertices[0].W, v0);
   VecSub(p->vertices[2].W, p->vertices[0].W, v1);
   VecCross(v1, v0, v2);
   VecNormalize(v2);
   for (i=0;i<p->n;i++) {
      p->vertices[i].N[0] += 0.5 * v2[0];
      p->vertices[i].N[1] += 0.5 * v2[1];
      p->vertices[i].N[2] += 0.5 * v2[2];
      }
   fVecNormalize(p->vertices[i].N);
}

static int
add_mesh_normal(fVec *V, int usteps, int vsteps,
                  int i0, int j0,
                  int i1, int j1,
                  int i2, int j2,
                  Vec N)
{
   Vec v0, v1, v2, t0, t1, Nt;

   if (i0 < 0 || j0 < 0 ||
       i1 < 0 || j1 < 0 ||
       i2 < 0 || j2 < 0 ||
       i0 > usteps || j0 > vsteps ||
       i1 > usteps || j1 > vsteps ||
       i2 > usteps || j2 > vsteps) {
      return 0;
      }
   else {
      VecCopy(V[i0 * (vsteps + 1) + j0], v0);
      VecCopy(V[i1 * (vsteps + 1) + j1], v1);
      VecCopy(V[i2 * (vsteps + 1) + j2], v2);
      VecSub(v2, v0, t0);
      VecSub(v1, v0, t1);
      VecCross(t0, t1, Nt);
      VecNormalize(Nt);
      VecAdd(N, Nt, N);
      return 1;
      }
}

/* Given a height field that only contains an elevation grid, this
   routine will walk through the data and produce averaged normals
   for all points on the grid. */
static void
smooth_mesh(fVec *V, fVec *N, int usteps, int vsteps)
{
   int i, j, k;
   Flt t;
   Vec N1;

   /* For now we will do it the hard way - by generating the normals
      individually for each elevation point */
   for (i=0,k=0;i<=usteps;i++) {
      for (j=0;j<=vsteps;j++,k++) {
         VecCopy(N[k], N1);
         k = 1;
         k += add_mesh_normal(V, usteps, vsteps, j, i, j-1, i, j-1, i-1, N1);
         k += add_mesh_normal(V, usteps, vsteps, j, i, j-1, i-1, j, i-1, N1);
         k += add_mesh_normal(V, usteps, vsteps, j, i, j, i-1, j+1, i, N1);
         k += add_mesh_normal(V, usteps, vsteps, j, i, j+1, i, j+1, i+1, N1);
         k += add_mesh_normal(V, usteps, vsteps, j, i, j+1, i+1, j, i+1, N1);
         k += add_mesh_normal(V, usteps, vsteps, j, i, j, i+1, j-1, i, N1);
         t = 1.0 / (Flt)k;
         VecScale(t, N1);
         VecCopy(N1, N[k]);
         }
      }
}

static int
recompute_uv_steps(Object *obj, int width, int height,
                   int *u_steps, int *v_steps)
{
   int size;

   if (!(obj->o_sflag & ADAPTIVE_UV))
      return 0;

   size = MAX(width, height);
   switch (obj->o_type) {
      case T_SPHERE:
         if (size < 8) {
            *u_steps = 4;
            *v_steps = 3;
            }
         else if (size < 16) {
            *u_steps = 6;
            *v_steps = 4;
            }
         else if (size < 32) {
            *u_steps = 8;
            *v_steps =  6;
            }
         else if (size < 64) {
            *u_steps = 12;
            *v_steps =  8;
            }
         else if (size < 128) {
            *u_steps = 16;
            *v_steps = 8;
            }
         else if (size < 256) {
            *u_steps = 32;
            *v_steps = 16;
            }
         else {
            *u_steps = 48;
            *v_steps = 24;
            }
         return 1;
         break;
      case T_DISC:
      case T_CONE:
      case T_CYLINDER:
         if (size < 8) {
            *u_steps = 4;
            *v_steps = 1;
            }
         else if (size < 16) {
            *u_steps = 6;
            *v_steps = 1;
            }
         else if (size < 32) {
            *u_steps = 8;
            *v_steps =  1;
            }
         else if (size < 64) {
            *u_steps = 12;
            *v_steps =  1;
            }
         else if (size < 128) {
            *u_steps = 16;
            *v_steps = 2;
            }
         else if (size < 256) {
            *u_steps = 32;
            *v_steps = 4;
            }
         else {
            *u_steps = 48;
            *v_steps = 4;
            }
         return 1;
         break;
      default:
         return 0;
         }
}

/* Subdivide a u-v surface to an exact value */
void
Uniform_Subdivide(Viewpoint *eye, BinTree *Root, Object *obj)
{
   Object *tobj;
   TriangleObject *obj1, *obj2;
   Vertex Vert;
   Vertex *vertex_row1, *vertex_row2, *temp_row;
   fVec *V, *N, *U;
   int i, j, displace_flag, internal_abort;
   long n, k;
   int u_steps, v_steps, width, height;
   Flt u, v, u_low, u_high, v_low, v_high;
   Flt delta_u, delta_v;
   bbox_info box;
   Poly P;
   Poly *Polygon = &P;

   if ((Check_Abort_Flag == 1) && _kbhit())
      longjmp(abort_environ, 1);

   u_steps = obj->o_uv_steps[0];
   v_steps = obj->o_uv_steps[1];

   if (obj->o_uv_bounds[0] == -PLY_HUGE &&
       obj->o_uv_bounds[1] ==  PLY_HUGE &&
       obj->o_uv_bounds[2] == -PLY_HUGE &&
       obj->o_uv_bounds[3] ==  PLY_HUGE) {
      u_low  = 0.0;
      u_high = 1.0;
      v_low  = 0.0;
      v_high = 1.0;
      }
   else {
      u_low  = obj->o_uv_bounds[0];
      u_high = obj->o_uv_bounds[1];
      v_low  = obj->o_uv_bounds[2];
      v_high = obj->o_uv_bounds[3];
      }
   delta_u = (u_high - u_low) / (Flt)u_steps;
   delta_v = (v_high - v_low) / (Flt)v_steps;

   displace_flag = 0;

   if (Rendering_Method == MESH_CONVERSION) {
      /* Store the computed information in a more compact form and generate
         the triangle objects associated with the parent object */
      obj->o_vertices = (ObjectVertices *)
                        polyray_malloc(sizeof(ObjectVertices));
      n = (long)(u_steps + 1) * (long)(v_steps + 1);
      V = (fVec *)polyray_malloc(n * sizeof(fVec));
      U = (fVec *)polyray_malloc(n * sizeof(fVec));
      if (V == NULL || U == NULL)
         error("Out of polygon memory");
      if (obj->o_sflag & SMOOTH_FLAG) {
         N = (fVec *)polyray_malloc(n * sizeof(fVec));
         if (N == NULL)
            error("Out of polygon memory");
         }
      else
         N = NULL;

      /* Store the vertex information */
      for (i=0,k=0,u=u_low;i<=u_steps;i++,u+=delta_u) {
         for (j=0,v=v_low;j<=v_steps;j++,k++,v+=delta_v) {
            obj->o_procs->evaluate(obj, u, v, &Vert);
            for (tobj=obj;tobj!=NULL;tobj=tobj->o_parent) {
               if (tobj->o_displace) {
                  /* A displacement was found, apply displacement mapping */
                  displace_vertex(tobj->o_displace, &Vert);
                  displace_flag = 1;
                  }
               }
            VecCopy(Vert.W, V[k]);
            if (N != NULL) VecCopy(Vert.N, N[k]);
            if (U != NULL) VecCopy(Vert.U, U[k]);
            }
         }

      /* Put the vertex information into the parent object */
      obj->o_vertices->n = n;
      obj->o_vertices->V = V;
      obj->o_vertices->N = N;
      obj->o_vertices->U = U;

      /* Perform normal averaging on a displaced surface */
      if (displace_flag && (N != NULL))
         smooth_mesh(V, N, u_steps, v_steps);

      /* Generate the individual triangle objects */
      for (i=0,k=0,u=u_low;i<u_steps;i++,u+=delta_u) {
         for (j=0,v=v_low;j<v_steps;j++,k++,v+=delta_v) {
            obj1 = (TriangleObject *)polyray_malloc(sizeof(TriangleObject));
            obj2 = (TriangleObject *)polyray_malloc(sizeof(TriangleObject));
            if (obj1 == NULL || obj2 == NULL)
               error("Insufficient polygon memory");

            /* Initialize the first triangle object */
            obj1->o_type = T_POLYGON;
            obj1->o_parent = obj;
            obj1->o_vert[0] = i * (v_steps + 1) + j;
            obj1->o_vert[1] = i * (v_steps + 1) + (j + 1);
            obj1->o_vert[2] = (i + 1) * (v_steps + 1) + (j + 1);
            obj1->o_texture = obj->o_texture;
            if (calc_triangle_bounds(obj1, &box)) {
               VecCopy(box.lower_left, obj1->o_bnd.lower_left)
               VecCopy(box.lengths, obj1->o_bnd.lengths)
               Root->members.list = push_object(Root->members.list,
                                                (Object *)obj1);
               Root->members.count++;
               }
            else
               polyray_free(obj1);

            /* Initialize second triangle object */
            obj2->o_type = T_POLYGON;
            obj2->o_parent = obj;
            obj2->o_vert[0] = i * (v_steps + 1) + j;
            obj2->o_vert[1] = (i + 1) * (v_steps + 1) + (j + 1);
            obj2->o_vert[2] = (i + 1) * (v_steps + 1) + j;
            obj2->o_texture = obj->o_texture;
            if (calc_triangle_bounds(obj2, &box)) {
               VecCopy(box.lower_left, obj2->o_bnd.lower_left)
               VecCopy(box.lengths, obj2->o_bnd.lengths)
               Root->members.list = push_object(Root->members.list,
                                                (Object *)obj2);
               Root->members.count++;
               }
            else
               polyray_free(obj2);
            }
         }
      }
   else {
      jmp_buf temp_environ;

      memcpy(temp_environ, abort_environ, sizeof(jmp_buf));

      /* Since we may abort rendering somewhere within this
         next loop, we want to be able to catch the abort and
         undo the memory allocation */
      if (setjmp(abort_environ) == 0) {
         /* See if the object is visible on the screen */
         BboxScreenSize(eye, &obj->o_bnd, &width, &height);
         if (width <= 0 || height <= 0)
            return;

         /* See if we are using an adaptive step size */
         if (recompute_uv_steps(obj, width, height, &u_steps, &v_steps)) {
            delta_u = (u_high - u_low) / (Flt)u_steps;
            delta_v = (v_high - v_low) / (Flt)v_steps;
            }

         vertex_row1 = (Vertex *)polyray_malloc((v_steps+1) * sizeof(Vertex));
         vertex_row2 = (Vertex *)polyray_malloc((v_steps+1) * sizeof(Vertex));
         if (vertex_row1 == NULL || vertex_row2 == NULL)
            error("Failed to allocate mesh memory");

         /* Evaluate the surface over the entire mesh */
         for (i=0,u=u_low;i<=u_steps;i++,u+=delta_u) {
            for (j=0,v=v_low;j<=v_steps;j++,v+=delta_v) {
               obj->o_procs->evaluate(obj, u, v, &vertex_row2[j]);
               for (tobj=obj;tobj!=NULL;tobj=tobj->o_parent) {
                  if (tobj->o_displace != NULL) {
                     /* A displacement was found, apply displacement mapping */
                     displace_vertex(tobj->o_displace, &vertex_row2[j]);
                     displace_flag = 1;
                     }
                  }
               }
            if (i > 0) {
               /* Now have at least two rows of vertices, we can go render
                  the current strip */
               for (j=0,v=v_low;j<v_steps;j++,v+=delta_v) {
                  Polygon->n = 4;
                  memcpy(&Polygon->vertices[0], &vertex_row1[j  ], sizeof(Vertex));
                  memcpy(&Polygon->vertices[1], &vertex_row1[j+1], sizeof(Vertex));
                  memcpy(&Polygon->vertices[2], &vertex_row2[j+1], sizeof(Vertex));
                  memcpy(&Polygon->vertices[3], &vertex_row2[j  ], sizeof(Vertex));
                  if (displace_flag)
                     average_normals(Polygon);
                  scan_convert(eye, Root, obj, NULL, Polygon);
                  }
               }
            temp_row    = vertex_row2;
            vertex_row2 = vertex_row1;
            vertex_row1 = temp_row;
            }
         internal_abort = 0;
         }
      else {
         internal_abort = 1;
         }
      memcpy(abort_environ, temp_environ, sizeof(jmp_buf));
      polyray_free(vertex_row1);
      polyray_free(vertex_row2);
      if (internal_abort)
         longjmp(abort_environ, 1);
      }
}

