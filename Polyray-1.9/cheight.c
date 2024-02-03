/* cheight.c

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
#include "io.h"
#include "memory.h"
#include "intersec.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "eval.h"
#include "builder.h"
#include "image.h"
#include "roots.h"
#include "height.h"

#define DBG_PRINT 0

/*
   Important: A left handed coordinate system is used for the cylinder where:
      x-axis is through the 0 meridian at the equator
      z-axis is 90 degrees counterclockwise as seen from the north pole
      y-axis goes through the north pole.
*/

void CylHeightRender(Viewpoint *, BinTree *, Object *);
int CylHeightIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int CylHeightInside(Object *obj, Vec P);
void CylHeightDelete(Object *object);

ObjectProcs CylHeightProcs = {
   CylHeightRender,
   NULL,
   GenericInitialize,
   CylHeightIntersect,
   CylHeightInside,
   GenericCopy,
   HeightDelete,
   };

Object *
MakeCylHeight(Object *object, char *filename, int smoothed,
              Flt scale, Flt offset)
{
   HeightData *hf;
   Vec axis;

   object->o_type = T_CYL_HEIGHT_FIELD;
   object->o_procs = &CylHeightProcs;

   /* Attempt to allocate memory for this primitive */
   if ((hf = (HeightData *)polyray_malloc(sizeof(HeightData))) == NULL)
      error("Failed to allocate height data\n");

   /* Read in the height field information and build the data structures */
   read_height_data(filename, offset, scale, &hf->data,
                    &hf->xsize, &hf->zsize, &hf->low, &hf->high);
#if DBG_PRINT
printf("Low: %g, High: %g\n", hf->low, hf->high);
#endif

   /* Precompute the sin and cos values needed for this object */
   create_angle_tables(T_CYL_HEIGHT_FIELD, hf->xsize, hf->zsize,
                       &hf->phi_sin, &hf->phi_cos, &hf->theta_norms);

   /* Check the size */
   if (hf->xsize < 4 || hf->zsize < 2)
      error("Grid size: (%d,%d) is too small, must be at least (4,2)\n",
            hf->xsize, hf->zsize);

   /* Some preprocessing to speed up the ray-hf intersection process */
   hf->norm  = NULL;

   hf->high *= 1.01; /* Make the bounding box 1% larger than needed */

   if (Rendering_Method == WIRE_FRAME ||
       Rendering_Method == HIDDEN_LINE ||
       Rendering_Method == RAW_TRIANGLES ||
       Rendering_Method == CSG_TRIANGLES)
      /* No need for normals if we are only interested vertices of the
         height field.  Note that UV_TRIANGLES requires the smoothed
         vertex normals so it is smoothed. */
      hf->type = 0;
   else if (smoothed) {
      hf->type = 1;
      smooth_height_field(hf, T_CYL_HEIGHT_FIELD);
      }
   else
      hf->type = 0;

   hf->cache_length = 0;
   hf->last_cached  = 0;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   /* Transform the cylinder so that 0<=z<=1 */
   hf->trans = Get_Transformation();

   /* Find the axis and axis length */
   MakeVector(1, 1.0/(Flt)(hf->zsize - 1), 1.0, axis);
   Get_Scaling_Transformation(hf->trans, axis);

   /* Compute bounding information */
   hf->boundbox[0][0] = -hf->high;
   hf->boundbox[0][1] =  0.0;
   hf->boundbox[0][2] = -hf->high;
   hf->boundbox[1][0] =  hf->high;
   hf->boundbox[1][1] =  1.0;
   hf->boundbox[1][2] =  hf->high;
   VecCopy(hf->boundbox[0], object->o_bnd.lower_left);
   VecSub(hf->boundbox[1], hf->boundbox[0], object->o_bnd.lengths);

   return object;
}

Object *
MakeCylHeightFn(Object *object, int xsize, int zsize, NODE_PTR fn,
                int smoothed, Flt scale, Flt offset)
{
   HeightData *hf;
   int i, j;
   Flt len, u, v, du, dv, val, fval;
   Vec tvec, axis;
   struct subst_struct subst;
   Transform trans;
   NODE_PTR tnode;

   object->o_type = T_CYL_HEIGHT_FIELD;
   object->o_procs = &CylHeightProcs;

   /* Check the size */
   if (xsize < 4 || zsize < 3)
      error("Spherical size: (%d,%d) is too small, must be at least (4,3)\n",
            xsize, zsize);

   /* Attempt to allocate memory for this primitive */
   if ((hf = (HeightData *)polyray_malloc(sizeof(HeightData))) == NULL)
      error("Failed to allocate height data\n");
   hf->xsize = xsize;
   hf->zsize = zsize;
   hf->low  =  PLY_HUGE;
   hf->high = -PLY_HUGE;

   /* Precompute the sin and cos values needed for this object */
   create_angle_tables(T_CYL_HEIGHT_FIELD, hf->xsize, hf->zsize,
                       &hf->phi_sin, &hf->phi_cos, &hf->theta_norms);

   /* Step through each element of the grid and create the height field */
   du = 1.0 / (Flt)(xsize - 1);
   dv = 1.0 / (Flt)(zsize - 1);
   MakeVector(0, 0, 0, subst.PT);
   MakeVector(0, 0, 0, subst.UT);
   hf->data = (float **)polyray_malloc(zsize * sizeof(float *));
   if (hf->data == NULL)
      error("Failed to allocate hf->data\n");
   for (i=0,v=0.0; i<zsize; i++,v+=dv) {
       hf->data[i] = (float *)polyray_malloc(xsize * sizeof(float));
       if (hf->data[i] == NULL)
          error("Failed to allocate hf->data[%d]\n", i);
       for (j=0,u=0.0; j<xsize; j++,u+=du) {
          MakeVector(u, v, 0, subst.U);
          subst.P[0] = cos(u * TWO_PI);
          subst.P[1] = v;
          subst.P[2] = sin(u * TWO_PI);
          VecCopy(subst.P, subst.W)
          if (eval_node(&subst, fn, &fval, tvec, &tnode) == 1)
             val = scale * fval + offset;
          else
             val = offset;
          hf->data[i][j] = val;
          if (val > hf->high) hf->high = val;
          if (val < hf->low) hf->low = val;
          }
      }

   /* If we want to smooth out the height field, then go
      and start calculating normals */
   if (smoothed) {
      hf->type = 1;
      smooth_height_field(hf, T_CYL_HEIGHT_FIELD);
      }
   else
      hf->type = 0;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   hf->cache_length = 0;
   hf->last_cached  = 0;
   hf->high *= 1.01; /* Make the bounding sphere 1% larger than needed */

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   /* Transform the cylinder so that 0<=z<=1 */
   hf->trans = Get_Transformation();

   /* Find the axis and axis length */
   MakeVector(1, 1.0/(Flt)(hf->zsize - 1), 1.0, axis);
   Get_Scaling_Transformation(hf->trans, axis);

   /* Compute bounding information */
   hf->boundbox[0][0] = -hf->high;
   hf->boundbox[0][1] =  0.0;
   hf->boundbox[0][2] = -hf->high;
   hf->boundbox[1][0] =  hf->high;
   hf->boundbox[1][1] =  1.0;
   hf->boundbox[1][2] =  hf->high;
   VecCopy(hf->boundbox[0], object->o_bnd.lower_left);
   VecSub(hf->boundbox[1], hf->boundbox[0], object->o_bnd.lengths);

   deallocate_node(fn);
   return object;
}

int
CylHeightInside(Object *obj, Vec Pos)
{
   return 0;
}

static int
determine_maximal_dists(Vec P, Vec D, Flt high_radius, Flt zlen,
                        Flt mindist, Flt maxdist, Flt *t0, Flt *t1)
{
   Flt a, b, c, disc, t;
   Flt tmin, tmax;

   /* First check against the bounding planes at y=0 and y=zsteps-1 */
   if (fabs(D[1]) < EPSILON) {
      tmin = mindist;
      tmax = maxdist;
      }
   else {
      tmin = -P[1] / D[1];
      tmax = tmin + zlen / D[1];
      if (tmin > tmax) {
         t = tmin;
         tmin = tmax;
         tmax = t;
         }
      if (tmin > maxdist || tmax < mindist)
         return 0;
      if (tmin < mindist)
         tmin = mindist;
      if (tmax > maxdist)
         tmax = maxdist;
      }

   a = D[0] * D[0] + D[2] * D[2];
   b = -(P[0] * D[0] + P[2] * D[2]);
   c = P[0] * P[0] + P[2] * P[2] - high_radius * high_radius;
   if (c > 0.0 && b < 0.0)
      /* Ray starts outside the cylinder, and continues away from it. */
      return 0;

   *t0 = tmin;
   *t1 = tmax;

   disc = b * b - a * c;
   if (disc < EPSILON)
      return 0;

   disc = sqrt(disc);
   tmin = (b - disc) / a;
   tmax = (b + disc) / a;
   if (tmin > tmax) {
      t = tmin;
      tmin = tmax;
      tmax = t;
      }

   if (tmin > *t1)
      return 0;
   else if (tmin > *t0)
      *t0 = tmin;
   if (tmax < *t0)
      return 0;
   else if (tmax < *t1)
      *t1 = tmax;

   return 2;
}

static int
Determine_Hits(Object *obj, Ray *ray, Flt t, Vec P, Vec D, Isect *hit,
               Flt mindist, Flt maxdist, int x, int z)
{
   Vec N, W, U, V;
   Flt dist;
   triangle triangs[2];
   int i, cnt, Flag = 0;
   HeightData *hf = (HeightData *)obj->o_data;

   cnt = intersect_square(x, z, hf, T_CYL_HEIGHT_FIELD,
                          D, P, triangs);

   for (i=0;i<cnt;i++) {
      dist = triangs[i].dist / t;
      if (dist > mindist && dist < maxdist) {
         VecAddScaled(ray->P, dist, ray->D, W);
         VecAddScaled(P, triangs[i].dist, D, V);
         if ((Global_Shade_Flag & UV_CHECK) &&
             (obj->o_sflag & UV_CHECK)) {
            U[0] = atan2(V[2], V[0]) / TWO_PI;
            if (U[0] < 0)
               U[0] = 1.0 + U[0];
            U[1] = V[1] / (Flt)(hf->zsize-1);
            }
         else
            VecCopy(W, U);
         if (hf->type == 0)
            VecCopy(triangs[i].N, N)
         else
            VecCopy(triangs[i].Ni, N)
         if (Insert_Hit(obj, W, N, dist, U, hit))
            Flag = 1;
         }
      }

   return Flag;
}

/* Are the current values of u and v valid? */
static int
inside_u_bounds(int u, int u0, int u1, int theta_dir_flag)
{
   if (theta_dir_flag)
      /* Moving in positive u */
      if (u0 < u1)
         return (u0 <= u && u <= u1);
      else
         return (u0 <= u || u <= u1);
   else
      /* Moving in negative u */
      if (u0 < u1)
         return (u0 >= u || u >= u1);
      else
         return (u0 >= u && u >= u1);
}

/*
   This is the heart of the ray-cylindrical height field routine.  It
   determines the path of the ray over the surface of a cylinder.
*/
int
CylHeightIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                   Flt mindist, Flt maxdist, Isect *hit)
{
   HeightData *hf;
   int u_steps, v_steps;
   Flt high_radius;
   Vec *theta_norms;

   int theta_dir_flag, theta_flip_flag, y_dir_flag;
   int u, v, ut, ustep, vstep, u0, v0, u1, v1;
   Flt raylen, t, t0, t1, tmid, next_theta_t, next_y_t;
   Flt omega0, omega1, dir_val;
   Vec p, p0, p1, q, q0, q1;
   Flt theta, theta1, dtheta;
   Flt y, dy, dtdy;
   Vec P, D;
   Flt mind, maxd;

   hf = (HeightData *)obj->o_data;

   /* Transform the ray into height field space */
   InvTxVector1(P, ray->P, hf->trans);
   InvTxDirection(D, ray->D, hf->trans);
   raylen = VecNormalize(D);

   /* Find where we hit the bounding box around the height field */
   mind = raylen * mindist;
   maxd = raylen * maxdist;

   /* Set up local variables */
   u_steps     = hf->xsize;
   v_steps     = hf->zsize;
   high_radius = hf->high;
   theta_norms = hf->theta_norms;

   /* Determine the entry and exit points of the ray with the maximal
      sphere of the height field */
   if (!determine_maximal_dists(P, D, high_radius, (Flt)(v_steps-1),
                                mind, maxd, &t0, &t1))
      return 0;

   VecAddScaled(P, t0, D, p0)
   VecAddScaled(P, t1, D, p1)

   /* Find the angles to the start and end points of the ray */
   cartesian_to_cylindrical(p0, q0);
   cartesian_to_cylindrical(p1, q1);

   /* Check to see if this ray goes to the left or right of the
      origin.  We do this by noting that the right handed cross
      product of the vector from p0 to p1 with the vector from
      p0 to the origin will be positive if the ray points to the
      right of the origin (as viewed from above). */
   dir_val = D[2] * p0[0] - D[0] * p0[2];
   if (dir_val > EPSILON) {
      theta_dir_flag = 1;
      theta_flip_flag = 0;
      }
   else if (dir_val < -EPSILON) {
      theta_dir_flag = 0;
      theta_flip_flag = 0;
      }
   else {
      /* ray goes really close to the axis, can't rely on
         the calculation for direction below. */
      u0 = floor((Flt)(u_steps - 1) * q0[0] / TWO_PI);
      if (u0 < 0) u0 += u_steps - 1;
      u1 = floor((Flt)(u_steps - 1) * q1[0] / TWO_PI);
      if (u1 < 0) u1 += u_steps - 1;
      v0 = floor(q0[1]);
      v1 = floor(q1[1]);
      if (v0 > v1)
         for (v=v0;v>=v1;v--) {
            if (Determine_Hits(obj, ray, raylen, P, D, hit, mindist, maxdist, u0, v))
               return 1;
            if (Determine_Hits(obj, ray, raylen, P, D, hit, mindist, maxdist, u1, v))
               return 1;
         }
      else
         for (v=v0;v<=v1;v++) {
            if (Determine_Hits(obj, ray, raylen, P, D, hit, mindist, maxdist, u0, v))
               return 1;
            if (Determine_Hits(obj, ray, raylen, P, D, hit, mindist, maxdist, u1, v))
               return 1;
            }
      return 0;
      }

   /* Figure out the start/end conditions for movement in theta and y. */
   init_u_variables(q0[0], q1[0], &dtheta, &theta, &u0, &u1,
                    &ustep, u_steps, theta_dir_flag);
   v0 = floor(q0[1]); if (v0 > v_steps-1) v0 = v_steps - 1;
   v1 = floor(q1[1]); if (v1 > v_steps-1) v1 = v_steps - 1;
   y  = q0[1];
   if (v1 > v0) { /* q1[1] > q0[1]) { */
      vstep = 1;
      y_dir_flag = 1;
      dy = 1.0;
      }
   else {
      vstep = -1;
      y_dir_flag = 0;
      dy = -1.0;
      }
   dtdy = dy / D[1];

   /* Get the distance to the first points where the ray crosses a
      longitude and latitude line. */
   next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                              theta_norms, t0, mindist, maxdist);

   if (v0 == v1)
      next_y_t = MAX_SPH_DIST;
   else if (y_dir_flag)
      next_y_t = t0 + dtdy * ((float)(v0 + 1) - y);
   else
      next_y_t = t0 + dtdy * (y - (float)v0);

#if DBG_PRINT
printf("next t: [%g, %g]\n", next_theta_t, next_y_t);
#endif

   /* Show the pattern of traversal over the cylinder as the ray crosses
      each longitude and y line. */
   for (t=0.0,u=u0,v=v0;;) {
      /* Check for an intersection with the rectangle at this
         longitude/latitude combination. */
      ut = u + (theta_dir_flag ? -1 : 0); if (ut < 0) ut = u_steps - 2;

      if (Determine_Hits(obj, ray, raylen, P, D, hit, mindist, maxdist, ut, v))
         return 1;

      if (u == u1 && v == v1)
         break;

      if (next_theta_t == MAX_SPH_DIST &&
          next_y_t == MAX_SPH_DIST) {
         /* This only seems to be happening on shadow rays.  Wonder why. */
         /* warning("Bad uv: (%d,%d)\n", u, v); */
         return 0;
         }

      if (next_theta_t <= next_y_t) {
         /* Evaluate at this longtitude crossing */
         t = next_theta_t;

         /* Step in theta */
         u     += ustep;
         theta += dtheta;
         /* Check for rollover of u */
         if (u == -1)
            u = u_steps - 2;
         else if (u == u_steps-1)
            u = 0;
         if (u0 == u1)
            next_theta_t = MAX_SPH_DIST;
         else
            next_theta_t = get_theta_t(u, P, D, theta_dir_flag,
                                       theta_norms, t, mindist, maxdist);
         }
      else {
         t = next_y_t;
         v += vstep;
         y += dy;

         if (v == v1)
            next_y_t = MAX_SPH_DIST;
         else
            next_y_t += dtdy;
         }
      }

   return 0;
}

void
CylHeightRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, k, u, v;
   Flt u0, du, v0, dv;
   Vec P[3], U[3], N[3], t0, t1;
   HeightData *hf = (HeightData *)obj->o_data;
   Poly Polygon;
   Object *tobj;

   u = hf->xsize;
   v = hf->zsize;
   du = 1.0 / (Flt)(u-1);
   dv = 1.0 / (Flt)(v-1);
   for (i=0,v0=0.0;i<v-1;i++,v0+=dv) {
      for (j=0,u0=0.0;j<u-1;j++,u0+=du) {
         indexed_cyl_to_cart(hf, j,   i,   P[0]);
         indexed_cyl_to_cart(hf, j+1, i,   P[1]);
         indexed_cyl_to_cart(hf, j,   i+1, P[2]);

         MakeVector(u0,    v0,    0.0, Polygon.vertices[0].U);
         MakeVector(u0+du, v0,    0.0, Polygon.vertices[1].U);
         MakeVector(u0,    v0+dv, 0.0, Polygon.vertices[2].U);

         Polygon.n = 3;
         if (hf->type == 0) {
            VecSub(P[1], P[0], t0);
            VecSub(P[2], P[0], t1);
            VecCross(t1, t0, N[0]);
            VecCopy(N[0], N[1]);
            VecCopy(N[0], N[2]);
            }
         else {
            VecCopy(hf->norm[i][j],     N[0]);
            VecCopy(hf->norm[i][j+1],   N[1]);
            VecCopy(hf->norm[i+1][j],   N[2]);
            }

         for (k=0;k<3;k++) {
            TxVector(P[k], P[k], hf->trans);
            VecCopy(P[k], Polygon.vertices[k].P);
            TxNormal(N[k], N[k], hf->trans);
            }

         tobj = obj;
         if (tobj->o_trans)
            for (k=0;k<3;k++) {
               TxVector(P[k], P[k], tobj->o_trans);
               TxNormal(N[k], N[k], tobj->o_trans);
               }

         for (k=0;k<3;k++)
            VecNormalize(N[k]);

         for (k=0;k<3;k++) {
            VecCopy(P[k], Polygon.vertices[k].W);
            VecCopy(N[k], Polygon.vertices[k].N);
            }

         scan_convert(eye, Root, obj, NULL, &Polygon);

         indexed_cyl_to_cart(hf, j+1,   i, P[0]);
         indexed_cyl_to_cart(hf, j+1, i+1, P[1]);
         indexed_cyl_to_cart(hf, j,   i+1, P[2]);

         MakeVector(u0+du, v0,    0.0, Polygon.vertices[0].U);
         MakeVector(u0+du, v0+dv, 0.0, Polygon.vertices[1].U);
         MakeVector(u0,    v0+dv, 0.0, Polygon.vertices[2].U);

         Polygon.n = 3;

         if (hf->type == 0) {
            VecSub(P[1], P[0], t0);
            VecSub(P[2], P[0], t1);
            VecCross(t1, t0, N[0]);
            VecCopy(N[0], N[1]);
            VecCopy(N[0], N[2]);
            }
         else {
            VecCopy(hf->norm[i][j+1],   N[0]);
            VecCopy(hf->norm[i+1][j+1], N[1]);
            VecCopy(hf->norm[i+1][j],   N[2]);
            }

         for (k=0;k<3;k++) {
            TxVector(P[k], P[k], hf->trans);
            VecCopy(P[k], Polygon.vertices[k].P);
            TxNormal(N[k], N[k], hf->trans);
            }

         tobj = obj;
         if (tobj->o_trans)
            for (k=0;k<3;k++) {
               TxVector(P[k], P[k], tobj->o_trans);
               TxNormal(N[k], N[k], tobj->o_trans);
               }

         for (k=0;k<3;k++)
            VecNormalize(N[k]);

         for (k=0;k<3;k++) {
            VecCopy(P[k], Polygon.vertices[k].W);
            VecCopy(N[k], Polygon.vertices[k].N);
            }
         scan_convert(eye, Root, obj, NULL, &Polygon);
         }
      }
}
