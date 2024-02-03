/*
  sheight.c

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
   Important: A left handed coordinate system is used for the sphere where:
      x-axis is through the 0 meridian at the equator
      z-axis is 90 degrees counterclockwise as seen from the north pole
      y-axis goes through the north pole.
*/

void SphHeightRender(Viewpoint *, BinTree *, Object *);
int SphHeightIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int SphHeightInside(Object *obj, Vec P);
void SphHeightDelete(Object *object);

ObjectProcs SphHeightProcs = {
   SphHeightRender,
   NULL,
   GenericInitialize,
   SphHeightIntersect,
   SphHeightInside,
   GenericCopy,
   HeightDelete,
   };

Object *
MakeSphHeight(Object *object, char *filename, int smoothed,
              Flt scale, Flt offset)
{
   HeightData *hf;

   object->o_type = T_SPH_HEIGHT_FIELD;
   object->o_procs = &SphHeightProcs;

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
   create_angle_tables(T_SPH_HEIGHT_FIELD, hf->xsize, hf->zsize,
                       &hf->phi_sin, &hf->phi_cos, &hf->theta_norms);

   /* Check the size */
   if (hf->xsize < 4 || hf->zsize < 3)
      error("Grid size: (%d,%d) is too small, must be larger than 2\n",
            hf->xsize, hf->zsize);

   /* Some preprocessing to speed up the ray-hf intersection process */
   hf->norm  = NULL;

   /* Compensate for the possibility that the ray may intersect the
      bounding sphere right on a phi or theta boundary (start point
      of the ray right on a transition point of either phi or theta).
      This plus numerical errors can cause the intersection routine
      to fail where it shouldn't.  */
   hf->high *= 1.01; /* Make the bounding sphere 1% larger than needed */

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
      smooth_height_field(hf, T_SPH_HEIGHT_FIELD);
      }
   else
      hf->type = 0;

   hf->cache_length = 0;
   hf->last_cached  = 0;

   hf->boundbox[0][0] = -hf->high;
   hf->boundbox[0][1] = -hf->high;
   hf->boundbox[0][2] = -hf->high;
   hf->boundbox[1][0] =  hf->high;
   hf->boundbox[1][1] =  hf->high;
   hf->boundbox[1][2] =  hf->high;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   /* Transform the box so that 0<=x<=1 and 0<=z<=1 */
   hf->trans = NULL;

   VecCopy(hf->boundbox[0], object->o_bnd.lower_left);
   VecSub(hf->boundbox[1], hf->boundbox[0], object->o_bnd.lengths);
#if DBG_PRINT
printf("Bounds: <%g,%g,%g> - <%g,%g,%g>\n",
       object->o_bnd.lower_left[0],
       object->o_bnd.lower_left[1],
       object->o_bnd.lower_left[2],
       object->o_bnd.lengths[0],
       object->o_bnd.lengths[1],
       object->o_bnd.lengths[2]);
#endif
   return object;
}

Object *
MakeSphHeightFn(Object *object, int xsize, int zsize,
                NODE_PTR fn, int smoothed, Flt scale, Flt offset)
{
   HeightData *hf;
   int i, j;
   Flt u, v, du, dv, val, fval;
   Vec tvec;
   struct subst_struct subst;
   NODE_PTR tnode;

   object->o_type = T_SPH_HEIGHT_FIELD;
   object->o_procs = &SphHeightProcs;

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
   create_angle_tables(T_SPH_HEIGHT_FIELD, hf->xsize, hf->zsize,
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
          subst.P[1] =  hf->phi_sin[i];
          subst.P[2] = -hf->theta_norms[j][0] * hf->phi_cos[i];
          subst.P[0] =  hf->theta_norms[j][2] * hf->phi_cos[i];
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
      smooth_height_field(hf, T_SPH_HEIGHT_FIELD);
      }
   else
      hf->type = 0;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   hf->cache_length = 0;
   hf->last_cached  = 0;
   hf->high *= 1.01; /* Make the bounding sphere 1% larger than needed */

   hf->boundbox[0][0] = -hf->high;
   hf->boundbox[0][1] = -hf->high;
   hf->boundbox[0][2] = -hf->high;
   hf->boundbox[1][0] =  hf->high;
   hf->boundbox[1][1] =  hf->high;
   hf->boundbox[1][2] =  hf->high;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   /* Transform the box so that 0<=x<=1 and 0<=z<=1 */
   hf->trans = NULL;

   VecCopy(hf->boundbox[0], object->o_bnd.lower_left);
   VecSub(hf->boundbox[1], hf->boundbox[0], object->o_bnd.lengths);

   deallocate_node(fn);
   return object;
}

int
SphHeightInside(Object *obj, Vec Pos)
{
   return 0;
}

static int
determine_maximal_dists(Vec P, Vec D, Flt high_radius,
                        Flt mindist, Flt maxdist, Flt *t0, Flt *t1)
{
   Flt l, disc;

   l = -VecDot(P, D);
   disc = l * l - VecDot(P, P) + high_radius * high_radius;
   if (disc < 0.0)
      return 0;
   disc = sqrt(disc);
   *t0 = l - disc;
   if (*t0 > maxdist)
      return 0;
   else if (*t0 < mindist)
      *t0 = mindist;
   *t1 = l + disc;
   if (*t1 < mindist)
      return 0;
   else if (*t1 > maxdist)
      *t1 = maxdist;
   return 2;
}

static void
init_v_variables(double phis, double phie,
                 double *dphi, double *phi0,
                 int *v0, int *v1, int *v,
                 int v_steps, int dir_flag)
{
   phis += M_PI / 2.0;
   phie += M_PI / 2.0;

   *dphi = M_PI / (double)(v_steps - 1);
   if (dir_flag) {
      *v0 = ceil(phis / *dphi);
      *v1 = ceil(phie / *dphi);
      *v  = 1;
      *phi0 = *dphi * (float)*v0;
      /* *phif = *phi0 - phis; */
      }
   else {
      *v0 = floor(phis / *dphi);
      *v1 = floor(phie / *dphi);
      *v  = -1;
      *phi0 = *dphi * (float)*v0;
      /* *phif = *phi0 - phis; */
      *dphi = -*dphi;
      }
}

/* Find the point along a ray that comes closest to the z-axis.  The
   return value is the square of the distance at that point.

   Unfortunatly, the closest point to the z-axis isn't necessarly the
   point where phi changes sign.  Instead of using that calculation,
   we use the following:

   Z[t] = (z0 + t z1)
   R[t_] := Sqrt[(x0 + t x1)^2 + (y0 + t y1)^2]
   Phi[t] := ArcSin[Z[t] / R[t]]

   D[Phi[t], t] =
   (z1/((x0 + t*x1)^2 + (y0 + t*y1)^2)^(1/2) -
    ((2*x1*(x0 + t*x1) + 2*y1*(y0 + t*y1))*(z0 + t*z1))/
     (2*((x0 + t*x1)^2 + (y0 + t*y1)^2)^(3/2)))/
   (1 - (z0 + t*z1)^2/((x0 + t*x1)^2 + (y0 + t*y1)^2))^(1/2)

Solving for D[Phi[t], t] == 0:

   t = -((-(x0*x1*z0) - y0*y1*z0 + x0^2*z1 + y0^2*z1)/
         (-(x1^2*z0) - y1^2*z0 + x0*x1*z1 + y0*y1*z1))
*/
static void
phi_cusp(Vec P, Vec D, Vec c, double *t)
{
   double t0, len;

   len = D[0] * D[0] + D[2] * D[2];
   if (len < EPSILON) {
      /* The line is parallel to the y-axis.  Return the point
         where it intersects the x-z plane */
      if (fabs(D[1]) > EPSILON) {
         *t = -P[1] / D[1];
         VecAddScaled(P, *t, D, c)
         }
      else {
         *t = 0.0;
         MakeVector(0.0, 0.0, 0.0, c)
         }
      }
   else {
#if 0
      /* Project the line into the x-y plane and find the closest
         point to the origin.  We do this by noting that the distance
         from a ray to the origin can be expressed as:

            d = sqrt((x0 + t x)^2 + (y0 + t y)^2)

         This can just as easily be put in terms of d^2.  In either case,
         we simply want to minimize the function.  By minimizing d^2, we
         get:

            d^2 = (x^2 + y^2) t^2 + 2 (x0 x + y0 y) t + x0^2 + y0^2

         The minimum is at the point where the first derivative goes
         to zero:

            min = 2 (x^2 + y^2) t + 2 (x0 x + y0 y) = 0

         or,

            t = -(x0 x + y0 y) / (x^2 + y^2)

         As a check, note that the direction from the origin to the point
         of closest approach Q is at a right angle to the line between the
         point P and Q.  From the pythagorean theorem:


            |P|^2 = |Q|^2 + |Q-P|^2

         Which can be expanded as:

            x0^2 + y0^2 = d^2 + (x^2 + y^2) t^2

         Using this and the previous formula, we can eliminate d^2 and get
         an expression for t:

            x0^2 + y0^2 - (x^2 + y^2) t^2 =
               (x^2 + y^2) t^2 + 2 (x0 x + y0 y) t + x0^2 + y0^2

         Combining terms gives:

            2 (x^2 + y^2) t^2 - 2 (x0 x + y0 y) t = 0

         Which (assuming t != 0) has a solution at

            t = -(x0 x + y0 y) / (x^2 + y^2)
       */
      t0 = -(P[0] * D[0] + P[2] * D[2]) / len;
#else
      t0 = -P[1]*(D[0]*D[0]+D[2]*D[2]) + D[1]*(P[0]*D[0]+P[2]*D[2]);
      if (fabs(t0) > EPSILON)
         t0 = (P[1]*(P[0]*D[0]+P[2]*D[2])-D[1]*(P[0]*P[0]+P[2]*P[2])) / t0;
      else
         t0 = 0.0;
/*
      t0 = -((-(P[0]*D[0]*P[2]) - P[1]*D[1]*P[2] + P[0]*P[0]*D[2] + P[1]*P[1]*D[2])/
             (-(D[0]*D[0]*P[2]) - D[1]*D[1]*P[2] + P[0]*D[0]*D[2] + P[1]*D[1]*D[2]));
*/
#endif
      /* From the distance along the line in the x-z plane, determine
         the amount of change along the z-axis to the intersection point */
      c[0] = P[0] + t0 * D[0];
      c[2] = P[2] + t0 * D[2];
      if (fabs(D[0]) > EPSILON)
         c[1] = P[1] + (c[0] - P[0]) * D[1] / D[0];
      else
         c[1] = P[1] + (c[2] - P[2]) * D[1] / D[2];
      *t = t0;
      }
}

// get_theta_t_dbl returns a double not a Flt like the equiv in height.c
double get_theta_t_dbl(int u, Vec P, Vec D, int theta_dir_flag,
            Vec *theta_norms, double t,
            double mindist, double maxdist)
{
   double d, n;

   /* Find the distance to the current longitude slice */
   d = VecDot(D, theta_norms[u]);
   if (!theta_dir_flag)
      d *= -1.0;
   if (d > 0.0) {
      /* Has a solution in front of the point */
      n = VecDot(P, theta_norms[u]);
      if (theta_dir_flag)
         t = -n / d;
      else
         t = n / d;
      if (t < mindist || t > maxdist) {
         /* Solution is out of range */
         t = MAX_SPH_DIST;
         }
      }
   else
      t = MAX_SPH_DIST;

   return t;
}

/*
   D must be normalized


In order to solve for t in terms of phi, we use the following:
   P = Start point of the ray
   D = Direction of the ray
   X = Distance from origin to the projection of P onto the x-y plane
   Y = Projection of P onto the y-axis

Parameterizing the ray we have:
   X^2 = (P[0] + D[0] * t)^2 + (P[2] + D[2] * t)^2
   Y^2 = (P[2] + D[2] * t)^2

The angle phi can be easily determined from:
   phi = asin(Y / sqrt(X^2 + Y^2))

Rewriting in terms of t we get:
   sin(phi)^2 = Y^2 / (X^2 + Y^2)
or,
   (X^2 + Y^2) sin(phi)^2 - Y^2 = 0

Which can be written as:
   X^2 sin(phi)^2 - Y^2 (1 - sin(phi)^2) = 0
or,
   X^2 sin(phi)^2 - Y^2 cos(phi)^2 = 0

Using the equations above for X^2 and Y^2, this is a quadratic in t.

*/
static double
get_phi_t(int v, Flt *a, Flt *b, Flt *phi_sin,
          Flt last_t, Flt mindist, Flt maxdist)
{
   Flt t, sinphi2, C[3], S[2];
   int j;

   sinphi2 = phi_sin[v] * phi_sin[v];

   C[0] = sinphi2 * (a[0] + b[0]) - b[0];
   C[1] = sinphi2 * (a[1] + b[1]) - b[1];
   C[2] = sinphi2 * (a[2] + b[2]) - b[2];

   j = solve_quadratic(C, S, mindist, maxdist);
   if (j == 2) {
      if (S[0] > last_t)
         if (S[1] > last_t)
            if (S[0] < S[1])
               t = S[0];
            else
               t = S[1];
         else
            t = S[0];
      else if (S[1] > last_t)
         t = S[1];
      else
         t = MAX_SPH_DIST;
      }
   else if (j == 1 && S[0] > last_t)
      t = S[0];
   else
      t = MAX_SPH_DIST;

   return t;
}

static void
determine_phi_flags(Flt tmin, Flt tmid, Flt tmax,
                    Flt phi_min, Flt omega, Flt phi_max,
                    int *phi_dir_flag, int *phi_flip_flag)
{
   if (tmid > tmin && tmid < tmax) {
      if (omega > phi_min) {
         *phi_dir_flag = 1;
         if (omega > phi_max)
            *phi_flip_flag = 1;
         else
            *phi_flip_flag = 0;
         }
      else {
         *phi_dir_flag = 0;
         if (omega < phi_max)
            *phi_flip_flag = 1;
         else
            *phi_flip_flag = 0;
         }
      }
   else {
      *phi_flip_flag = 0;
      if (phi_max > phi_min)
         *phi_dir_flag = 1;
      else
         *phi_dir_flag = 0;
      }
}

static void
precompute_ray_phi_values(Vec P, Vec D, Flt *a, Flt *b)
{
   /* Precompute values for this ray for use in intersection tests
      of the various latitude cones */
   a[0] = D[0] * D[0] + D[2] * D[2];
   b[0] = D[1] * D[1];
   a[1] = 2.0 * (P[0] * D[0] + P[2] * D[2]);
   b[1] = 2.0 * P[1] * D[1];
   a[2] = P[0] * P[0] + P[2] * P[2];
   b[2] = P[1] * P[1];
}

static int
Determine_Hits(Object *obj, Ray *ray, Isect *hit,
               Flt mindist, Flt maxdist, int x, int z)
{
   Vec N, W, U;
   Flt dist;
   triangle triangs[2];
   int i, cnt, Flag = 0;
   HeightData *hf = (HeightData *)obj->o_data;

   cnt = intersect_square(x, z, hf, T_SPH_HEIGHT_FIELD,
                          ray->D, ray->P, triangs);

   for (i=0;i<cnt;i++) {
      dist = triangs[i].dist;
      if (dist > mindist && dist < maxdist) {
         VecAddScaled(ray->P, dist, ray->D, W);
         if ((Global_Shade_Flag & UV_CHECK) &&
             (obj->o_sflag & UV_CHECK)) {
            cartesian_to_geocentric(W, U);
            U[0] /= TWO_PI;
            if (U[0] < 0)
               U[0] = 1.0 + U[0];
            U[1] = 0.5 + U[1] / M_PI;
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

static int
inside_v_bounds(int v, int v0, int v1, int phi_dir_flag)
{
   if (phi_dir_flag)
      /* Moving in positive v */
      return (v0 <= v && v <= v1);
   else
      /* Moving in negative v */
      return (v0 >= v && v >= v1);
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

/* Brute force routine to find intersections.  Try the ray against
   all rectangles within u0 <= theta <= u1 && v0 <= phi <= v1. */
static int
all_possible_hits(Object *obj, Ray *ray, Isect *hit, Flt mindist, Flt maxdist,
                  Vec q0, Vec q, Vec q1, Flt t0, Flt tmid, Flt t1,
                  int theta_dir_flag)
{
   HeightData *hf;
   int i, j;
   int u0, u1, v0, vm, v1;
   int u_steps, v_steps;
   Flt temp, theta, dtheta, phi, dphi;
   int phi_dir_flag, phi_flip_flag;

   hf = (HeightData *)obj->o_data;
   u_steps = hf->xsize;
   v_steps = hf->zsize;

   init_u_variables(q0[0], q1[0], &dtheta, &theta, &u0, &u1,
                    &i, u_steps, theta_dir_flag);

   /* Figure out which direction phi is changing and if it flips
      direction somewhere between the start and end angles */
   determine_phi_flags(t0, tmid, t1, q0[1], q[1], q1[1],
                       &phi_dir_flag, &phi_flip_flag);

   /* Figure out the start/end conditions for movement in phi from
      the beginning of the ray to the highest/lowest latitude. */
   if (phi_flip_flag)
      init_v_variables(q0[1], q[1], &dphi, &phi,
                       &v0, &vm, &j, v_steps, phi_dir_flag);
   init_v_variables(q0[1], q1[1], &dphi, &phi,
                    &v0, &v1, &j, v_steps, phi_dir_flag);

   if (phi_flip_flag) {
      if (v0 > v1) {
         temp = v0;
         v0 = v1;
         v1 = temp;
         }
      if (vm < v0)
         v0 = vm;
      else if (vm > v1)
         v1 = vm;
      }

   if (v0 > 0) v0 -= 1;
   if (v1 < v_steps-1) v1 += 1;

printf("Check all rects: [%d,%d]x[%d,%d]\n", u0, u1, v0, v1);
   /* for (i=u0;i!=u1;) { */
   for (i=0;i<u_steps;i++) {
      for (j=v0;j<=v1;j++)
         if (Determine_Hits(obj, ray, hit, mindist, maxdist, i, j)) {
            printf("Brute force found one at (%d,%d)\n", i, j);
            return 1;
            }
/*
      if (theta_dir_flag)
         i = (i == u_steps - 2 ? 0 : i + 1);
      else
         i = (i == 0 ? u_steps - 2 : i - 1);
*/
      }

   return 0;
}


/* This is the heart of the ray-spherical height field routine.  It
   determines the path of the ray over the surface of a sphere.  As
   the lat/long boxes are determined, they are tested against the ray.

   Special cases:
         1) Through the center of the sphere - there will only be two sectors
            of the sphere that need to be considered.
         2) Parallel to the y-axis.  No change in u, only need to step through
            v to get full coverage.
         3) Parallel to the x-z plane.  No change in v, only need to step through
            u to get full coverage.
*/
int
SphHeightIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                   Flt mindist, Flt maxdist, Isect *hit)
{
   HeightData *hf;
   int u_steps, v_steps;
   Flt high_radius, *phi_sin;
   Vec *theta_norms;

   int theta_dir_flag, theta_flip_flag, phi_dir_flag, phi_flip_flag;
   int u, v, ut, vt, ustep, vstep, u0, v0, u1, v1;
   Flt t, t0, t1, tmid, next_theta_t, next_phi_t;
   Flt omega0, omega1, dir_val;
   Vec p, p0, p1, q, q0, q1;
   Flt theta, dtheta;
   Flt phi, dphi;
   Flt a[3], b[3];
   Vec P, D;

   /* Set up local variables */
   hf = (HeightData *)obj->o_data;
   u_steps     = hf->xsize;
   v_steps     = hf->zsize;
   high_radius = hf->high;
   phi_sin     = hf->phi_sin;
   theta_norms = hf->theta_norms;

   VecCopy(ray->P, P)
   VecCopy(ray->D, D)

   /* Determine the entry and exit points of the ray with the maximal
      sphere of the height field */
   if (!determine_maximal_dists(P, D, high_radius, mindist,
                                maxdist, &t0, &t1))
      return 0;

   VecAddScaled(P, t0, D, p0)
   VecAddScaled(P, t1, D, p1)

   /* Find the angles to the start and end points of the ray */
   cartesian_to_geocentric(p0, q0);
   cartesian_to_geocentric(p1, q1);

   /* Closest approach to the z-axis */
   phi_cusp(p0, D, p, &tmid);
   cartesian_to_geocentric(p, q);
   omega0 = q[0];
   omega1 = q[1];
   tmid   += t0;

#if DBG_PRINT
{
   Vec vtmp;
   Flt ftmp;
VecSub(p, P, vtmp);
ftmp = sqrt(VecDot(vtmp, vtmp));
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g/%g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid, ftmp);
printf("P0: <%g,%g,%g>\nP1: <%g,%g,%g>\nPm: <%g,%g,%g>\n",
       p0[0], p0[1], p0[2], p1[0], p1[1], p1[2], p[0], p[1], p[2]);
}
#endif

   /* Compute ray characteristics needed for solving the quadratics
      that result from every change in the phi direction */
   precompute_ray_phi_values(P, D, a, b);

   /* Check to see if this ray goes to the left or right of the
      origin.  We do this by noting that the right handed cross
      product of the vector from p0 to p1 with the vector from
      p0 to the origin will be positive if the ray points to the
      right of the origin (as viewed from above). */
   if (fabs(omega1) + EPSILON > M_PI_2) {
      u0 = floor((Flt)(u_steps - 1) * q0[0] / TWO_PI);
      if (u0 < 0) u0 += u_steps - 1;
      u1 = floor((Flt)(u_steps - 1) * q1[0] / TWO_PI);
      if (u1 < 0) u1 += u_steps - 1;
      v0 = floor((Flt)(v_steps - 1) * (q0[1] / M_PI + 0.5));
      v1 = floor((Flt)(v_steps - 1) * (q1[1] / M_PI + 0.5));
      if (p[1] < 0.0) {
         for (v=v0;v>=0;v--)
            if (Determine_Hits(obj, ray, hit, mindist, maxdist, u0, v))
               return 1;
         for (v=0;v<=v1;v++)
            if (Determine_Hits(obj, ray, hit, mindist, maxdist, u1, v))
               return 1;
         }
      else {
         for (v=v0;v<=v_steps-1;v++)
            if (Determine_Hits(obj, ray, hit, mindist, maxdist, u0, v))
               return 1;
         for (v=(v_steps - 1);v>=v1;v--)
            if (Determine_Hits(obj, ray, hit, mindist, maxdist, u1, v))
               return 1;
         }
#if DBG_PRINT
printf("No simple hit, u0: %d, u1: %d, v0: %d, v1: %d\n", u0, u1, v0, v1);
{
   Vec vtmp;
   Flt ftmp;
VecSub(p, P, vtmp);
ftmp = sqrt(VecDot(vtmp, vtmp));
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g/%g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid, ftmp);
printf("P0: <%g,%g,%g>\nP1: <%g,%g,%g>\nPm: <%g,%g,%g>\n",
       p0[0], p0[1], p0[2], p1[0], p1[1], p1[2], p[0], p[1], p[2]);
}
#endif
      return 0;
      }
   if (q[2] < EPSILON) {
      /* ray goes really close to the axis, can't rely on
         the calculation for direction below. */
      theta_dir_flag = (q0[0] < q1[0]);
      theta_flip_flag = 1;
      q0[0] += 0.00001;
   } else {
      dir_val = D[2] * p0[0] - D[0] * p0[2];
      if (dir_val > 0.0) {
         theta_dir_flag = 1;
         theta_flip_flag = 0;
         }
      else {
         theta_dir_flag = 0;
         theta_flip_flag = 0;
         }
   }

   /* Figure out the start/end conditions for movement in theta */
   init_u_variables(q0[0], q1[0], &dtheta, &theta, &u0, &u1,
                    &ustep, u_steps, theta_dir_flag);

   /* Figure out which direction phi is changing and if it flips
      direction somewhere between the start and end angles */
   determine_phi_flags(t0, tmid, t1, q0[1], omega1, q1[1],
                       &phi_dir_flag, &phi_flip_flag);

   /* Don't allow phi to change direction right on a phi boundary */
   if (phi_flip_flag) {
      dphi = M_PI / (Flt)(v_steps - 1);
      if (phi_dir_flag)
         phi = dphi * ceil((omega1 - 1.0e-10) / dphi);
      else
         phi = dphi * floor((omega1 + 1.0e-10) / dphi);
      if (fabs(omega1 - phi) < 1.0e-10) {
         /* Adjust phi so it's off the boundary */
         omega1 += (phi_dir_flag ? -1.0e-10 : 1.0e-10);
         }
      }

   /* Figure out the start/end conditions for movement in phi from
      the beginning of the ray to the highest/lowest latitude. */
   if (phi_flip_flag)
      init_v_variables(q0[1], omega1, &dphi, &phi,
                       &v0, &v1, &vstep, v_steps, phi_dir_flag);
   else
      init_v_variables(q0[1], q1[1], &dphi, &phi,
                       &v0, &v1, &vstep, v_steps, phi_dir_flag);

   /* Get the distance to the first points where the ray crosses a
      longitude and latitude line. */
   next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                              theta_norms, t0, mindist, maxdist);

   if (v0 == v1)
      if (phi_flip_flag)
         next_phi_t = tmid;
      else
         next_phi_t = MAX_SPH_DIST;
   else
      next_phi_t = get_phi_t(v0, a, b, phi_sin, t0, mindist, maxdist);

#if DBG_PRINT
printf("dtheta: %g, theta dir: %d, tflag: %d, dphi: %g, phi dir: %d, pflag: %d\n",
       radtodeg(dtheta), theta_dir_flag, theta_flip_flag,
       radtodeg(dphi), phi_dir_flag, phi_flip_flag);
printf("next theta t: %g, next phi t: %g\n", next_theta_t, next_phi_t);
printf("last theta t: %g, next theta t: %g\n",
       get_theta_t(u0-1, P, D, theta_dir_flag,
                   theta_norms, 0.0, mindist, maxdist),
       get_theta_t(u0+1, P, D, theta_dir_flag,
                   theta_norms, 0.0, mindist, maxdist));
printf("last phi t: %g, next phi t: %g\n",
       get_phi_t(v0-1, a, b, phi_sin, 0.0, mindist, maxdist),
       get_phi_t(v0+1, a, b, phi_sin, 0.0, mindist, maxdist));
#endif
#if DBG_PRINT
printf("UV boundaries: [%d, %d], [%d, %d]\n", u0, u1, v0, v1);
#endif
   /* Show the pattern of traversal over the sphere as the ray crosses
      each longitude and latitude line. */
   for (t=0.0,u=u0,v=v0;;) {
      /* Check for an intersection with the rectangle at this
         longitude/latitude combination. */
      ut = u + (theta_dir_flag ? -1 : 0); if (ut < 0) ut = u_steps - 2;
      vt = v + (phi_dir_flag ? -1 : 0); if (vt < 0) vt = 0;

#if DBG_PRINT
printf("u/v: %2d/%2d, t: %g, theta: %g, phi: %g, nt: %g, np: %g\n",
       ut, vt, t, radtodeg(ut * fabs(dtheta)),
       radtodeg(vt * fabs(dphi) - M_PI / 2.0),
       next_theta_t, next_phi_t);
#endif

      if (Determine_Hits(obj, ray, hit, mindist, maxdist, ut, vt)) {
#if 0
if (Shadow_Test) {
printf("Bad shadow(%g): %d/%d from [%d,%d] %d - [%d,%d] %d\nt: %g, t/p: %g/%g, f: %d\n",
       hit->isect_t, u, v, u0, u1, theta_dir_flag, v0, v1, phi_dir_flag,
       t, next_theta_t, next_phi_t, phi_flip_flag);
next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                           theta_norms, 0.0, mindist, maxdist);
next_phi_t = get_phi_t(v, a, b, phi_sin, tmid, mindist, maxdist);
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g, nt: %g, np: %g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid,
       next_theta_t, next_phi_t);
printf("min: %g, max: %g\n", mindist, maxdist);
   }
#endif
         return 1;
         }

      if (u == u1 && (!phi_flip_flag && v == v1))
         break;

      if (!inside_u_bounds(u, u0, u1, theta_dir_flag) ||
          !inside_v_bounds(v, v0, v1, phi_dir_flag)) {
#if 0
printf("Bad u/v: %d/%d from [%d,%d] %d - [%d,%d] %d\nt: %g, t/p: %g/%g, f: %d\n",
       u, v, u0, u1, theta_dir_flag, v0, v1, phi_dir_flag,
       t, next_theta_t, next_phi_t, phi_flip_flag);
next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                           theta_norms, 0.0, mindist, maxdist);
next_phi_t = get_phi_t(v, a, b, phi_sin, tmid, mindist, maxdist);
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g, o1: %g, nt: %g, np: %g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid, radtodeg(omega1),
       next_theta_t, next_phi_t);
   if (all_possible_hits(obj, ray, hit, mindist, maxdist,
                         q0, q, q1, t0, tmid, t1, theta_dir_flag))
      return 1;
   else {
      printf("Brute force failed\n");
      return 0;
      }
#endif
#if DBG_PRINT
printf("Bad u/v\n");
#endif
   return 0;
   }

      if (next_theta_t <= next_phi_t) {
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
         if (u0 == u1) {
            next_theta_t = MAX_SPH_DIST;
            }
         else
            next_theta_t = get_theta_t(u, P, D, theta_dir_flag,
                                       theta_norms, t, mindist, maxdist);
         }
      else {
         /* Check to see if we are ready to either flip directions in phi
            or break out of the loop. */
         t = next_phi_t;
         if (v == v1) {
            if (phi_flip_flag) {
               if (theta_flip_flag) {
                  u = u1;
                  theta_flip_flag = 0;
                  }
#if DBG_PRINT
printf("At u/v: %d/%d @ %g/%g, from [%d,%d] - [%d,%d]\n",
       u, v, next_theta_t, next_phi_t, u0, u1, v0, v1);
#endif
               phi_dir_flag = 1 - phi_dir_flag;
               init_v_variables(omega1, q1[1], &dphi, &phi,
                                &v0, &v1, &vstep, v_steps, phi_dir_flag);
               v = v0;
#if DBG_PRINT
printf("New V boundaries: [%d, %d], vs: %d, dir: %d\n", v, v1, vstep, phi_dir_flag);
#endif
               phi_flip_flag = 0;
               }
            else {
#if DBG_PRINT
printf("Bad v value: %d/%d from [%d,%d] %d - [%d,%d] %d\nt: %g, t/p: %g/%g, f: %d\n",
       u, v, u0, u1, theta_dir_flag, v0, v1, phi_dir_flag,
       t, next_theta_t, next_phi_t, phi_flip_flag);
next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                           theta_norms, 0.0, mindist, maxdist);
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g, nt: %g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid, next_theta_t);
#endif
               return 0;
               }
            }
         else {
            /* Move to the next latitude */
            v   += vstep;
            phi += dphi;
            }

         if (v == v1)
            if (phi_flip_flag)
               next_phi_t = tmid;
            else
               next_phi_t = MAX_SPH_DIST;
         else {
            next_phi_t = get_phi_t(v, a, b, phi_sin, t, mindist, maxdist);
            }
         }
      }

#if DBG_PRINT
if (!Shadow_Test) {
printf("Failed: %d/%d from [%d,%d] %d - [%d,%d] %d\nt: %g, t/p: %g/%g, f: %d\n",
       u, v, u0, u1, theta_dir_flag, v0, v1, phi_dir_flag,
       t, next_theta_t, next_phi_t, phi_flip_flag);
next_theta_t = get_theta_t(u0, P, D, theta_dir_flag,
                           theta_norms, 0.0, mindist, maxdist);
next_phi_t = get_phi_t(v, a, b, phi_sin, tmid, mindist, maxdist);
printf("Q0: <%g,%g,%g> @ %g\nQ1: <%g,%g,%g> @ %g\nQm: <%g,%g,%g> @ %g, nt: %g, np: %g\n",
       radtodeg(q0[0]), radtodeg(q0[1]), q0[2], t0,
       radtodeg(q1[0]), radtodeg(q1[1]), q1[2], t1,
       radtodeg(q[0]), radtodeg(q[1]), q[2], tmid,
       next_theta_t, next_phi_t);
   if (all_possible_hits(obj, ray, hit, mindist, maxdist,
                            q0, q, q1, t0, tmid, t1))
      return 1;
   else {
      printf("Brute force failed\n");
      return 0;
      }
   }
else
   return 0;
#else
   return 0;
#endif
}

void
SphHeightRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, k, u, v;
   Flt u0, du, v0, dv;
   Vec P[3], N[3], t0, t1;
   HeightData *hf = (HeightData *)obj->o_data;
   Poly Polygon;
   Object *tobj;

   u = hf->xsize;
   v = hf->zsize;
   du = 1.0 / (Flt)(u-1);
   dv = 1.0 / (Flt)(v-1);
   for (i=0,v0=0.0;i<v-1;i++,v0+=dv) {
      for (j=0,u0=0.0;j<u-1;j++,u0+=du) {
         indexed_geo_to_cart(hf, j,   i,   P[0]);
         indexed_geo_to_cart(hf, j+1, i,   P[1]);
         indexed_geo_to_cart(hf, j,   i+1, P[2]);

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

         indexed_geo_to_cart(hf, j+1,   i, P[0]);
         indexed_geo_to_cart(hf, j+1, i+1, P[1]);
         indexed_geo_to_cart(hf, j,   i+1, P[2]);

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
