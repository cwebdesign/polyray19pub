/* blob.c

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
#include "intersec.h"
#include "symtab.h"
#include "mcube.h"
#include "vector.h"
#include "bound.h"
#include "roots.h"
#include "blob.h"

typedef struct {
   int type, index;
   float bound;
   } Blob_Interval;

typedef struct t_blobdata {
   int count;
   Flt threshold;
   Blob_Element *list;
   Blob_Interval *intervals;
   int Sturm_Flag;
   } Blob;

/* Maximum # of intervals from a component */
#define MAX_INTERVALS 64

/* Standard density function */
#define DFN 0

void BlobRender(Viewpoint *, BinTree *, Object *obj);
int BlobIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int BlobInside(Object *obj, Vec Pos);
int BlobNormal(Object *obj, Vec P, Vec N);
void BlobDelete(Object *object);

ObjectProcs BlobProcs = {
   BlobRender,
   NULL,
   GenericInitialize,
   BlobIntersect,
   BlobInside,
   GenericCopy,
   BlobDelete,
   };

/* This routine builds the static information needed by the blob/ray
   intersection code when it goes looking for search intervals. */
static void
add_blob_component(Blob *blob, blobstackptr temp,
                   int *index, Vec mins, Vec maxs)
{
   int i, j;
   Vec bot, top, axis;
   Flt t, rad, mrad, coeff;
   Transform *trans;
   bbox_info bnd;

   if (temp->elem.radius2 < 0.0)
      perror("Degenerate blob element\n");

   i = *index;

   /* Keep track of bounding information */
   rad = temp->elem.radius2;

   /* Compute information specific to each blob
      component type */
   if (temp->elem.type == T_SPHERICAL_BLOB) {
      MakeVector(temp->elem.pos[0]-rad, temp->elem.pos[1]-rad,
                 temp->elem.pos[2]-rad, bnd.lower_left);
      MakeVector(2.0*rad, 2.0*rad, 2.0*rad, bnd.lengths);
#if 1
      trans = NULL;
#else
      trans = (Transform *)polyray_malloc(sizeof(Transform));
      if (trans == NULL)
         error("Failed to allocate blob component\n");
      MakeVector(1, 0.7 + 0.5*polyray_random(), 1, S);
      Get_Scaling_Transformation (trans, S);
printf("S: <%g,%g,%g>\n", S[0], S[1], S[2]);
printf("bnd: <%g,%g,%g> - <%g,%g,%g>\n",
       bnd.lower_left[0],
       bnd.lower_left[1],
       bnd.lower_left[2],
       bnd.lengths[0],
       bnd.lengths[1],
       bnd.lengths[2]);
      recompute_inverse_bbox(&bnd, trans);
printf("bnd: <%g,%g,%g> - <%g,%g,%g>\n",
       bnd.lower_left[0],
       bnd.lower_left[1],
       bnd.lower_left[2],
       bnd.lengths[0],
       bnd.lengths[1],
       bnd.lengths[2]);
#endif
      if (i==0)
         /* Compute bounding information */
         for (j=0;j<3;j++) {
            mins[j] = bnd.lower_left[j];
            maxs[j] = bnd.lower_left[j] + bnd.lengths[j];
            }
      else
         for (j=0;j<3;j++) {
            mins[j] = MIN(mins[j], bnd.lower_left[j]);
            maxs[j] = MAX(maxs[j], bnd.lower_left[j] + bnd.lengths[j]);
            }

      /* Store general blob information */
      coeff = temp->elem.coeffs[2];
      rad *= rad;
      VecCopy(temp->elem.pos, blob->list[i].pos);
      blob->list[i].type      = T_SPHERICAL_BLOB;
      blob->list[i].trans     = trans;
      blob->list[i].radius2   = rad;
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(5 * sizeof(Flt));

      i++;
      }
   else if (temp->elem.type == T_CYLINDRICAL_BLOB) {
      /* Find the axis and axis length */
      trans = (Transform *)polyray_malloc(sizeof(Transform));
      if (trans == NULL)
         error("Failed to allocate blob component\n");
      VecCopy(temp->elem.pos, bot);
      VecCopy(temp->elem.dir, top);
      VecSub(top, bot, axis);
      t = VecNormalize(axis);
      if (t < EPSILON)
          error("Degenerate cylindrical blob\n");
      VecNegate(bot);
      Get_Coordinate_Transform(trans, bot, axis, 1.0, 1.0);
      VecNegate(bot);

      MakeVector(-rad, -rad, -rad, bnd.lower_left);
      MakeVector(2.0*rad, 2.0*rad, t+2.0*rad, bnd.lengths);
      recompute_inverse_bbox(&bnd, trans);
      if (i==0)
         /* Compute bounding information */
         for (j=0;j<3;j++) {
            mins[j] = bnd.lower_left[j];
            maxs[j] = bnd.lower_left[j] + bnd.lengths[j];
            }
      else
         for (j=0;j<3;j++) {
            mins[j] = MIN(mins[j], bnd.lower_left[j]);
            maxs[j] = MAX(maxs[j], bnd.lower_left[j] + bnd.lengths[j]);
            }
      /* Store general blob information.  This is where we build the
         three pieces of a cylindrical blob - the cylinder itself,
         together with the two hemispherical caps. */
      coeff = temp->elem.coeffs[2];
      rad *= rad;
      blob->list[i].type      = T_CYLINDRICAL_BLOB;
      blob->list[i].len       = t;
      blob->list[i].trans     = trans;
      blob->list[i].radius2   = rad;
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(5 * sizeof(Flt));
      i++;
      blob->list[i].type      = T_HEMISPHERICAL_BLOB;
      VecCopy(top,  blob->list[i].pos);
      VecCopy(axis, blob->list[i].dir);
      blob->list[i].len       = VecDot(top, axis);
      blob->list[i].trans     = NULL;
      blob->list[i].radius2   = rad;
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(5 * sizeof(Flt));
      i++;
      blob->list[i].type      = T_HEMISPHERICAL_BLOB;
      VecNegate(axis);
      VecCopy(bot,  blob->list[i].pos);
      VecCopy(axis, blob->list[i].dir);
      blob->list[i].len       = VecDot(bot, axis);
      blob->list[i].trans     = NULL;
      blob->list[i].radius2   = rad;
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(5 * sizeof(Flt));
      i++;
      }
   else if (temp->elem.type == T_PLANAR_BLOB) {
      trans = NULL;
      VecCopy(temp->elem.dir, blob->list[i].dir);
      t = VecNormalize(blob->list[i].dir);
      blob->list[i].len = temp->elem.len / t;
      /* Compute bounding information */
      for (j=0;j<3;j++) {
         mins[j] = -PLY_HUGE;
         maxs[j] = PLY_HUGE;
         }
      /* Store general blob information */
      coeff = temp->elem.coeffs[2];
      blob->list[i].type      = T_PLANAR_BLOB;
      blob->list[i].trans     = NULL;
      blob->list[i].radius2   = rad;
      rad *= rad;
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(5 * sizeof(Flt));
      i++;
      }
   else if (temp->elem.type == T_TOROIDAL_BLOB) {
      /* Find the axis and axis length */
      trans = (Transform *)polyray_malloc(sizeof(Transform));
      if (trans == NULL)
         error("Failed to allocate blob component\n");

      VecCopy(temp->elem.pos, bot);
      VecCopy(temp->elem.dir, axis);
      t = VecNormalize(axis);
      VecNegate(bot);
      Get_Coordinate_Transform(trans, bot, axis, 1.0, 1.0);
      VecNegate(bot);
      mrad = temp->elem.len;
      MakeVector(-(mrad+rad), -(mrad+rad), -rad, bnd.lower_left);
      MakeVector(2.0*(mrad+rad), 2.0*(mrad+rad), 2.0*rad, bnd.lengths);
      recompute_inverse_bbox(&bnd, trans);
      if (i==0)
         /* Compute bounding information */
         for (j=0;j<3;j++) {
            mins[j] = bnd.lower_left[j];
            maxs[j] = bnd.lower_left[j] + bnd.lengths[j];
            }
      else
         for (j=0;j<3;j++) {
            mins[j] = MIN(mins[j], bnd.lower_left[j]);
            maxs[j] = MAX(maxs[j], bnd.lower_left[j] + bnd.lengths[j]);
            }
      /* Store general blob information. */
      coeff = temp->elem.coeffs[2];
      rad  *= rad;
      mrad *= mrad;
      blob->list[i].type      = T_TOROIDAL_BLOB;
      blob->list[i].trans     = trans;
      blob->list[i].len       = mrad;   /* Major axis squared */
      blob->list[i].radius2   = rad;    /* Minor axis squared */
      blob->list[i].coeffs[2] = coeff;
      blob->list[i].coeffs[1] = -(2.0 * coeff) / rad;
      blob->list[i].coeffs[0] = coeff / (rad * rad);
      blob->list[i].tcoeffs   = (Flt *)polyray_malloc(17 * sizeof(Flt));
      i++;
      }
   *index = i;
}

/* Starting with the density function: (1-r^2)^2, we have a field
   that varies in strength from 1 at r = 0 to 0 at r = 1.  By
   substituting r/rad for r, we can adjust the range of influence
   of a particular component.  By multiplication by coeff, we can
   adjust the amount of total contribution, giving the formula:
      coeff * (1 - (r/rad)^2)^2
   This varies in strength from coeff at r = 0, to 0 at r = rad. */
Object *
MakeBlob(Object *object, Flt threshold, blobstackptr bloblist,
         int npoints, int sflag)
{
   Blob *blob;
   int i, cnt;
   Vec mins, maxs;
   blobstackptr temp;

   blob = (Blob *)polyray_malloc(sizeof(Blob));
   if (blob == NULL)
      error("Failed to allocate blob data\n");
   object->o_type = T_BLOB;
   object->o_procs = &BlobProcs;
   object->o_uv_steps[0] = 20;
   object->o_uv_steps[1] = 20;
   object->o_uv_steps[2] = 20;

   /* First figure out how many components there will be */
   for (temp=bloblist,cnt=0;temp!=NULL;temp=temp->next)
      if (temp->elem.type == T_SPHERICAL_BLOB ||
          temp->elem.type == T_PLANAR_BLOB ||
          temp->elem.type == T_TOROIDAL_BLOB)
         cnt++;
      else if (temp->elem.type == T_CYLINDRICAL_BLOB)
         cnt += 3;
      else
         error("Invalid blob component type");

   blob->threshold = threshold;
   blob->list = (Blob_Element *)polyray_malloc(cnt * sizeof(Blob_Element));
   if (blob->list == NULL)
      error("Failed to allocate blob data\n");
   blob->count = cnt;
   blob->Sturm_Flag = sflag;
   /* Initialize the blob data */
   for(i=0,cnt=0;i<npoints;i++) {
      add_blob_component(blob, bloblist, &cnt, mins, maxs);
      temp = bloblist;
      bloblist = bloblist->next;
      polyray_free(temp);
      }

   /*  Allocate memory for intersection intervals */
   blob->intervals = (Blob_Interval *)polyray_malloc(2 * cnt *
                                                     sizeof(Blob_Interval));
   if (blob->intervals == NULL)
      error("Failed to allocate blob data\n");

   /* Store bounding information */
   VecCopy(mins, object->o_bnd.lower_left);
   VecSub(maxs, mins, object->o_bnd.lengths);

   object->o_data = (void *)blob;
   return object;
}

void
Set_Blob_Solver(Object *obj, int Sturm_Flag)
{
   Blob *blob = (Blob *)obj->o_data;
   blob->Sturm_Flag = Sturm_Flag;
}

void
BlobDelete(Object *object)
{
   int i;
   Blob *blob = (Blob *)object->o_data;

   if (object->o_copy == 0) {
      for (i=0;i<blob->count;i++) {
         if (blob->list[i].trans != NULL)
            polyray_free(blob->list[i].trans);
         polyray_free(blob->list[i].tcoeffs);
         }
      polyray_free(blob->list);
      polyray_free(blob->intervals);
      polyray_free(blob);
      }
}

/* Determine the interval over which the ray is in a spherical blob component.
   This is in essence a copy of the ray/sphere intersection routine, however
   in this case since we need intervals of influence, we do not keep single
   hits, and if the ray starts inside the sphere, then we use the origin of the
   ray as the start of the interval.  */
static int
spherical_interval(Blob_Element *element, Ray *ray,
                   Flt mindist, Flt *i0, Flt *i1)
{
   Vec P, D;
   Flt *tcoeffs, c0, c1, c2;
   Flt disc, t0, t1, d0, d1, d;

   /* First transform to component space */
   TxVector(P, ray->P, element->trans);
   TxDirection(D, ray->D, element->trans);
   d = VecNormalize(D);

   VecSub(P, element->pos, P);
   /*
   VecSub(ray->P, element->pos, P);
   VecCopy(ray->D, D);
   */
   t0 = VecDot(P, P);
   t1 = VecDot(P, D);
   disc = t1 * t1 - t0 + element->radius2;

   /* Calculate the interval of influence */
   if (disc < EPSILON)
      return 0;
   disc = sqrt(disc);
   d1 = -t1 + disc;
   if (d1 < mindist)
      d1 = mindist;
   d0 = -t1 - disc;
   if (d0 < mindist) d0 = mindist;
   if (d1 == d0)
      return 0;
   else if (d1 < d0) {
      disc = d0;
      d0 = d1;
      d1 = disc;
      }

   /* Calculate the density equation over the interval */
   c0 = element->coeffs[0];
   c1 = element->coeffs[1];
   c2 = element->coeffs[2];

   tcoeffs = element->tcoeffs;
#if DFN == 0
   tcoeffs[0] = c0;
   tcoeffs[1] = 4*c0*t1;
   tcoeffs[2] = (4*c0*t1*t1 + c1 + 2*c0*t0);
   tcoeffs[3] = 2*(c1*t1 + 2*c0*t0*t1);
   tcoeffs[4] = c2 + c1*t0 + c0*t0*t0;
#else
   tcoeffs[0] = 0.0;
   tcoeffs[1] = 0.0;
   tcoeffs[2] = -c2 / element->radius2;
   tcoeffs[3] = -c2 * 2.0 * t1 / element->radius2;
   tcoeffs[4] =  c2 * (1.0 - t0 / element->radius2);
#endif

   *i0 = d0 / d;
   *i1 = d1 / d;

   return 1;
}

/* Determine the interval over which the ray is in a hemispherical
   blob component.  This looks a lot like the spherical blob component
   code - the big difference is that we chop down the interval over
   a sphere by looking at which hemisphere the hit points are in. */
static int
hemispherical_interval(Blob_Element *element, Ray *ray,
                       Flt mindist, Flt *i0, Flt *i1)
{
   Vec V, P0, P1;
   Flt b, d0, d1, d2;
   int f0, f1;

   if (!spherical_interval(element, ray, mindist, &d0, &d1))
      return 0;

   VecSub(element->pos, ray->P, V);
   b = VecDot(V, ray->D);

   /* We now have the hit points with the sphere.  Process
      them further to see if they are both on one side of the
      hemisphere, or if they are on opposite sides */
   VecAddScaled(ray->P, d0, ray->D, P0);
   VecAddScaled(ray->P, d1, ray->D, P1);

   VecSub(P0, element->pos, V);
   b = VecDot(V, element->dir);
   f0 = (b >= 0 ? 1 : 0);
   VecSub(P1, element->pos, V);
   b = VecDot(V, element->dir);
   f1 = (b >= 0 ? 1 : 0);

   if (f0 == 0)
      if (f1 == 0)
         return 0;
      else {
         /* P1 is above, P0 below */
         b = VecDot(element->dir, ray->D);
         if (fabs(b) > EPSILON) {
            d2 = (element->len - VecDot(element->dir, ray->P)) / b;
            if (d2 > d1) {
               d0 = d1;
               d1 = d2;
               }
            else
               d0 = d2;
            }
         }
   else if (f1 == 0) {
      /* P0 above, P1 below */
      b = VecDot(element->dir, ray->D);
      if (fabs(b) > EPSILON) {
         d2 = (element->len - VecDot(element->dir, ray->P)) / b;
         if (d2 < d0) {
            d1 = d0;
            d0 = d2;
            }
         else
            d1 = d2;
         }
      }

   *i0 = d0;
   *i1 = d1;
   return 1;
}

/* Determine the interval over which the ray is in a cylindrical blob
   component.  This is in essence a copy of the ray/cylinder intersection
   routine, however in this case since we need intervals of influence, we do
   not keep single hits, if the ray starts inside the sphere, then we use the
   origin of the ray as the start of the interval, and we treat the cylinder
   as if it were closed on the ends in order to bound the interval properly. */
static int
cylindrical_interval(Blob_Element *element, Ray *ray, Flt *i0, Flt *i1)
{
   Flt a, b, c, d0, d1, d2, t0, t1, t2, disc;
   Flt c0, c1, c2;
   Flt len, z0, z1, *tcoeffs;
   Vec P, D;

   /* First transform to canonical cylinder space */
   TxVector(P, ray->P, element->trans);
   TxDirection(D, ray->D, element->trans);

   t0 = -PLY_HUGE;
   t1 = PLY_HUGE;

   d0 = -PLY_HUGE;
   d1 =  PLY_HUGE;

   len = element->len;
   
   /* Determine where the ray hits the sides of
      the cylinder. */
   a = P[0] * P[0] + P[1] * P[1];
   b = P[0] * D[0] + P[1] * D[1];
   c = D[0] * D[0] + D[1] * D[1];
   if (a > EPSILON) {
      disc = b * b - c * (a - element->radius2);
      if (disc < 0.0)
         /* No hits with the cyl */
         return 0;

      disc = sqrt(disc);
      d0 = (-b - disc) / c;
      d1 = (-b + disc) / c;

      z0 = P[2] + d0 * D[2];
      z1 = P[2] + d1 * D[2];

      if ((z0 > len && z1 > len) || (z0 < 0.0 && z1 < 0.0))
         /* Hits all above or all below the cylinder */
         return 0;

      if (d0 > d1) {
         d2 = d1;
         d1 = d0;
         d0 = d2;
         }
      }

   /* Check to see if the ray hits the top or bottom
      of the can.  If so and these hits are better than
      the ones from the sides, then use them */
   if (D[2] > 0) {
      /* Ray goes from bottom to top of can */
      d2 = -P[2] / D[2];
      if (d2 > d0)
         d0 = d2;
      d2 = (len - P[2]) / D[2];
      if (d2 < d1)
         d1 = d2;
      }
   else if (D[2] < 0) {
      /* Ray goes from top to bottom of can */
      d2 = (len - P[2]) / D[2];
      if (d2 > d0)
         d0 = d2;
      d2 = -P[2] / D[2];
      if (d2 < d1)
         d1 = d2;
      }
   else if (P[2] < 0 || P[2] > len)
      /* Completely misses the cyl */
      return 0;

   /* Calculate the density equation over the interval */
   c0 = element->coeffs[0];
   c1 = element->coeffs[1];
   c2 = element->coeffs[2];

   if (c > EPSILON) {
      t0 = a;
      t1 = b;
      t2 = c;
      }
   else {
      t0 = a;
      t1 = 0;
      t2 = 0;
      }

   /* Save the coefficients for this component */
   tcoeffs = element->tcoeffs;
   tcoeffs[0] = c0*t2*t2;
   tcoeffs[1] = 4*c0*t1*t2;
   tcoeffs[2] = (4*c0*t1*t1 + c1*t2 + 2*c0*t0*t2);
   tcoeffs[3] = 2*(c1*t1 + 2*c0*t0*t1);
   tcoeffs[4] = c2 + c1*t0 + c0*t0*t0;

   *i0 = d0;
   *i1 = d1;
   return 1;
}

/* Determine the interval over which the ray is in a planar blob component.
   This involves finding the point of intersection of the ray and the plane,
   as well as the point of intersection of the ray and a parallel plane that
   is offset from the original by the radius of this component. */
static int
planar_interval(Blob_Element *element, Ray *ray,
                Flt mindist, Flt *i0, Flt *i1)
{
   Vec D;
   Flt t0, t1, t2, d0, d1, temp;
   Flt *tcoeffs, c0, c1, c2;

   VecCopy(element->dir, D);
   t0 = VecDot(D, ray->P) + element->len;
   t2 = VecDot(D, ray->D);

   if (fabs(t2) < EPSILON) {
      /* Ray is parallel to the plane */
      t2 = 0;
      d0 = mindist;
      d1 = PLY_HUGE;
      }
   else {
      d0 = -(t0 + element->len) / t2;
      d1 = d0 + element->radius2 / t2;
      if (d1 < mindist)
         d1 = mindist;
      if (d0 < mindist) d0 = mindist;
      if (d1 == d0)
         return 0;
      else if (d1 < d0) {
         temp = d0;
         d0 = d1;
         d1 = temp;
         }
      }

   /* Calculate the density equation over the interval */
   c0 = element->coeffs[0];
   c1 = element->coeffs[1];
   c2 = element->coeffs[2];

   t1 = t0 * t2;
   t0 = t0 * t0;
   t2 = t2 * t2;

   tcoeffs = element->tcoeffs;
   tcoeffs[0] = c0*t2*t2;
   tcoeffs[1] = 4*c0*t1*t2;
   tcoeffs[2] = (4*c0*t1*t1 + c1*t2 + 2*c0*t0*t2);
   tcoeffs[3] = 2*(c1*t1 + 2*c0*t0*t1);
   tcoeffs[4] = c2 + c1*t0 + c0*t0*t0;

   *i0 = d0;
   *i1 = d1;
   return 1;
}

/* Determine the interval(s) over which the ray is in a toroidal blob component.
   This involves finding the points of intersection of the ray and a torus,
   then sorting the intersection points so that we get either 0, 1, or 2
   intervals where the ray is inside the torus. */
static int
torus_intervals(Blob_Element *element, Ray *ray,
                Flt mindist, Flt *i0, Flt *i1)
{
   Flt coeff[5], ocoeffs[9], Depths[4];
   Flt dist, c0, c1, c2, *tcoeffs;
   Vec P, D;
   int i, j, hitcnt, cnt;
   Flt t1, t2, t3, t4, t5, t6;

   /* First transform to canonical torus space */
   TxVector(P, ray->P, element->trans);
   TxDirection(D, ray->D, element->trans);

   dist = VecNormalize(D);

   t1 = VecDot(D, P);
   t2 = VecDot(P, P);
   t4 = element->len;
   t5 = element->radius2;
   t3 = t4 + t5;
   t6 = t2 - t3;

   coeff[0] = 1.0;
   coeff[1] = 4.0 * t1;
   coeff[2] = 2.0 * t6 + 4.0 * (t1 * t1 + t4 * D[2] * D[2]);
   coeff[3] = 4.0 * t1 * t6 + 8.0 * t4 * P[2] * D[2];
   coeff[4] = t6 * t6 - 4.0 * t4 * (t5 - P[2] * P[2]);

   /* For now, just use Vieta's method - if this proves to
      give insufficient accuracy, then use Sturm sequences */
   hitcnt = solve_quartic1(coeff, Depths, -PLY_HUGE, PLY_HUGE);

   if (hitcnt == 0 || hitcnt == 1)
      /* Best of all possible worlds - no contribution of the
         toroidal component */
      return 0;
   else if (hitcnt == 2) {
      /* One interval, make sure they are sorted */
      if (fabs(Depths[0] - Depths[1]) < EPSILON)
         /* Twin root, ignore it */
         return 0;
      if (Depths[0] > Depths[1]) {
         t1 = Depths[1];
         Depths[1] = Depths[0];
         Depths[0] = t1;
         }
      if (Depths[1] < mindist)
         return 0;
      if (Depths[0] < mindist)
         Depths[0] = mindist;
      i0[0] = Depths[0];
      i1[0] = Depths[1];
      cnt = 1;
      }
   else {
      /* Do a nice little bubble sort to make sure the roots
         are in the proper order */
      for (i=0;i<hitcnt-1;i++)
         for (j=i+1;j<hitcnt;j++)
            if (Depths[i] > Depths[j]) {
               t1 = Depths[i];
               Depths[i] = Depths[j];
               Depths[j] = t1;
               }
      if (hitcnt == 3) {
         /* Split this into two distinct intervals - that way we
            can pretend there were really four roots */
         Depths[3] = Depths[2] + EPSILON;
         Depths[2] = Depths[1] - EPSILON;
         }
      if (Depths[3] < mindist)
         return 0;
      if (Depths[2] < mindist)
         Depths[2] = mindist;
      if (Depths[1] < mindist) {
         i0[0] = Depths[2];
         i1[0] = Depths[3];
         cnt = 1;
         }
      else {
         if (Depths[0] < mindist)
            Depths[0] = mindist;
         i0[0] = Depths[0];
         i1[0] = Depths[1];
         i0[1] = Depths[2];
         i1[1] = Depths[3];
         cnt = 2;
         }
      }

   /* Calculate the density equation over the interval,
         d = c2 * r^4 + c1 * r^2 + c0
      This is slow and complicated, as the density equation is a quartic,
      and the equation of the torus is also a quartic.  The result is an
      order 16 polynomial.  So far numerical inaccuracies have prevented
      consistent results when raytracing toroidal blob components. */
   c0 = element->coeffs[0];
   c1 = element->coeffs[1];
   c2 = element->coeffs[2];

   tcoeffs = element->tcoeffs;
   for (i=0;i<16;i++)
      tcoeffs[i] = 0;
   tcoeffs[16] = c2;
   for (i=0;i<=8;i++)
      ocoeffs[i] = 0;

   for (i=0;i<=4;i++)
      for (j=0;j<=4;j++)
         ocoeffs[i+j] += coeff[i] * coeff[j];

   for (i=0;i<=8;i++)
      tcoeffs[8+i] += c1 * ocoeffs[i];

   for (i=0;i<=8;i++)
      for (j=0;j<=8;j++)
         tcoeffs[i+j] += c0 * ocoeffs[i] * ocoeffs[j];

   return cnt;
}

static void
insert_interval(Blob_Interval *intervals, int i,
                int *count, Flt t0, Flt t1)
{
   int j, k, cnt = *count;

   /* Store the points of intersection of this blob with the ray.  Keep track
      of: whether this is the start or end point of the hit, which component
      was pierced by the ray, and the point on the ray that the hit occured */
   for (k=0;k<cnt && t0>intervals[k].bound;k++);
   if (k<cnt) {
      /* This hit point is smaller than one that already exists - bump the
         rest and insert it here */
      for (j=cnt;j>k;j--)
         memcpy(&intervals[j], &intervals[j-1],
                sizeof(Blob_Interval));
      intervals[k].type  = 0;
      intervals[k].index = i;
      intervals[k].bound = t0;
      cnt++;
      for (k=k+1;k<cnt && t1>intervals[k].bound;k++);
      if (k<cnt) {
         for (j=cnt;j>k;j--)
            memcpy(&intervals[j], &intervals[j-1],
                   sizeof(Blob_Interval));
         intervals[k].type  = 1;
         intervals[k].index = i;
         intervals[k].bound = t1;
         }
      else {
         intervals[cnt].type  = 1;
         intervals[cnt].index = i;
         intervals[cnt].bound = t1;
         }
      cnt++;
      }
   else {
      /* Just plop the start and end points at
         the end of the list */
      intervals[cnt].type  = 0;
      intervals[cnt].index = i;
      intervals[cnt].bound = t0;
      cnt++;
      intervals[cnt].type  = 1;
      intervals[cnt].index = i;
      intervals[cnt].bound = t1;
      cnt++;
      }
   *count = cnt;
}

/* Make a sorted list of points along the ray that the various blob
   components start and stop adding their influence.  It would take
   a very complex blob (with many components along the current ray)
   to warrant the overhead of using a faster sort technique. */
static int
determine_influences(Ray *ray, Blob *blob, Flt mindist)
{
   int i, j, icnt, cnt;
   Flt t0[MAX_INTERVALS], t1[MAX_INTERVALS];
   Blob_Interval *intervals = blob->intervals;
   Blob_Element *temp;

   for (i=0,cnt=0,temp=blob->list;i<blob->count;i++,temp++) {
      if (temp->type == T_SPHERICAL_BLOB) {
         if ((icnt = spherical_interval(temp, ray, mindist, t0, t1)) == 0)
            continue;
         }
      else if (temp->type == T_CYLINDRICAL_BLOB) {
         if ((icnt = cylindrical_interval(temp, ray, t0, t1)) == 0)
            continue;
         }
      else if (temp->type == T_HEMISPHERICAL_BLOB) {
         if ((icnt = hemispherical_interval(temp, ray, mindist, t0, t1)) == 0)
            continue;
         }
      else if (temp->type == T_PLANAR_BLOB) {
         if ((icnt = planar_interval(temp, ray, mindist, t0, t1)) == 0)
            continue;
         }
      else if (temp->type == T_TOROIDAL_BLOB) {
         if ((icnt = torus_intervals(temp, ray, mindist, t0, t1)) == 0)
            continue;
         }
      else
         error("Bad blob component type: %d in determine_influences\n",
               temp->type);

      /* Make sure all of the intervals we collected are actually somewhere
         within the part of the ray we are looking at */
      for (j=0;j<icnt;j++) {
         if (t1[j] < mindist)
            /* Both hits out of range */
            break;
         if (t0[j] < mindist)
            /* Interval starts inside the component */
            t0[j] = mindist;

         /* Seems to be a valid interval, put it in the list */
         insert_interval(intervals, i, &cnt, t0[j], t1[j]);
         }
      }
   return cnt;
}

/*
This procedure generates intervals of influence for each component
of a blob.  After these are made, determine their aggregate effect
on the ray.  As the individual intervals are checked, a quartic (or higher
polynomial if toroidal components are involved) is generated that represents
the density at a particular point on the ray.

After making the substitutions in MakeBlob, there is a formula
for each component that has the form:
   
   c0 * r^4 + c1 * r^2 + c2.

This formula defines the density with respect to distance from
the blob component.  The actual subsitution into this equation
will depend on the nature of the component.  For each of the
defined blob components there are slightly different forms
that describe density as a function of the ray.


   A) Spherical components

   In order to determine the influence on the ray of all of the
   individual components, we start by determining the distance
   from any point on the ray to the specified point.  This can
   be found using the pythagorean theorem, using C as the center
   of this component, P as the start of the ray, and D as the
   direction of travel of the ray:

      r^2 = (t * D + P - C) . (t * D + P - C)

   we insert this equation for each appearance of r^2 in the
   components' formula, giving:

      r^2 = D.D t^2 + 2 t D . (P - C) + (P - C) . (P - C)

   Since the direction vector has been normalized, D.D = 1.
   Using the substitutions:

      t0 = (P - C) . (P - C),
      t1 = 2 D . (P - C)
      t2 = 1

   We can write the formula as:

      r^2 = t0 + t t1 + t^2


   B) Cylindrical components

   Cylindrical blob components are a little more difficult
   since we have to transform into and out of canonical
   space for the cylinder.  This means keeping track of
   a space transformation for the cylinder as well as a
   size transformation for the ray.

      r^2 = (t * D  + P) . (t * D + P)

   For cylinders, we keep track of the values of "P" and "D"
   in cylinder space rather than in blob space.  By doing this,
   the density equations for the ray have the same meaning
   regardless of component type.  In addition, since the z-axis
   part of the ray (in cylinder space) is meaningless, we
   set that part to zero.

   we insert this equation for each appearance of r^2 in the
   components' formula, giving:

      r^2 = D.D t^2 + 2 t D.P + P.P

   Using the substitutions:

      t0 =   P . P
      t1 = 2 D . P
      t2 =   D . D

   We can write the formula as:

      r^2 = t0 + t1 t + t2 t^2


   C) Planar components

   The distance from the point <x0, y0, z0> to the plane
   ax + by + cz + d = 0 is:

      r = | a x0 + b y0 + c z0 + d| / sqrt(a^2 + b^2 + c^2).

   Assuming that the normal to the plane is normalized, the
   denominator above reduces to 1.  Replacing the point by
   the ray P + t * D and the normal to the plane by N,
   we get the distance formula:

      r = N . (t * D + P) + d

   Note: if r < 0 then we are underneath the planar component.
   Rewriting the distance into its squared format gives:

      r^2 = [N . (t * D  + P) + d]^2

   Rearranging terms,

      r^2 = [N.D t + (N.P + d)]^2

   Collecting terms with respect to t:

      r^2 = (N.D)^2 t^2 + 2 (N.D * (N.P + d)) t + (N.P + d)^2

   Using the substitutions:

      t0 =   (N.P + d) * (N.P + d)
      t1 = 2 N.D * (N.P + d)
      t2 =   N.D * N.D

   We can write the formula as:

      r^2 = t0 + t1 t + t2 t^2


   D) Circular components (including toroidal)

   The distance formula will be with respect to the circle
   that is at the center of the torus.  In essence the density
   will be with respect to that circle - by generating the
   distance formula from the closest point to the ray from
   the circle the resulting surface will be toroidal.  A
   slight drawback to this is that the inner radius MUST be
   smaller than the outer radius, otherwise the densities
   from opposite ends of the torus will start to interact.


   Steps to determine the distance to the center circle:

      1) Transform the ray into the canonical space that the
         torus sits in (torus lies in x-y plane with axis
         of rotation aligned with the z axis).

      2) The intervals that the ray is inside the torus are
         determined exactly the same way the ray/torus intersection
         is calculated.  These give intervals over which the torus
         contributes to the density along the ray.  The distance
         of the ray to the central circle of the torus is given
         by:

               r1 = x^4 + c1 x^3 + c2 x^2 + c3 x + c4,

         Where the coefficients are calculated from the following:

               t1 = VecDot(D, P);
               t2 = VecDot(P, P);
               t4 = torus->r0;
               t5 = torus->r1;
               t3 = t4 + t5;
               t6 = t2 - t3;

         Followed by:

               c1 = 4.0 * t1;
               c2 = 2.0 * t6 + 4.0 * (t1 * t1 + t4 * D[2] * D[2]);
               c3 = 4.0 * t1 * t6 + 8.0 * t4 * P[2] * D[2];
               c4 = t6 * t6 - 4.0 * t4 * (t5 - P[2] * P[2]);

         This results in an 8th order polynomial for r^2, and
         finally in a 16th order polynomial for the density.

Finally:

In general toroidal components are not involved, so we will have a quadratic
expression for the r^2 component of the density function.  Once a substitution
for r^2 has been found for an individual component, it is substituted into the
formula for density:

   density = c0 * (r^2)^2 + c1 * r^2 + c2,

or:

   density = c0 * (t0 + 2 t1 t + t2 t^2)^2 +
             c1 * (t0 + 2 t1 t + t2 t^2) +
             c2

Expanding terms and collecting with respect to "t" gives:

   t^4 * c0*t2^2 + 
   t^3 * 4*c0*t1*t2 +
   t^2 * (4*c0*t1^2 + c1*t2 + 2*c0*t0*t2)
   t   * 2*(c1*t1 + 2*c0*t0*t1) + 
   1     c2 + c1*t0 + c0*t0^2 +


This formula can now be solved for "t" by any of the quartic root solvers that
are available.
*/
int
BlobIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Blob *blob = (Blob *)obj->o_data;
   Flt dist, *tcoeffs, coeffs[5];
   Flt roots[16], scoeffs[17];
   Flt low, high;
   int i, j, cnt, scoeff_cnt = 0;
   int Flag = 0;
   Vec P, N, U;
   int root_count, in_flag;
   Blob_Element *element;
   Blob_Interval *intervals = blob->intervals;

   /* Figure out the intervals along the ray where each
      component of the blob has an effect. */
   if ((cnt = determine_influences(ray, blob, mindist)) == 0)
      /* Ray doesn't hit the sphere of influence of any of
         its component elements */
      return 0;

   /* Clear out the coefficients */
   for (i=0;i<4;i++) coeffs[i] = 0.0;
   coeffs[4] = -blob->threshold;
   for (i=0;i<=16;i++) scoeffs[i] = 0.0;

   /* Step through the list of influence points, adding the
      influence of each blob component as it appears */
   for (i=0,in_flag=0;i<cnt;i++) {
      if (intervals[i].type == 0) {
         /* Something is just starting to influence the ray, so calculate its
            coefficients and add them into the pot. */
         in_flag++;
         element = blob->list + intervals[i].index;

         tcoeffs = element->tcoeffs;
         if (element->type == T_SPHERICAL_BLOB ||
             element->type == T_HEMISPHERICAL_BLOB ||
             element->type == T_PLANAR_BLOB ||
             element->type == T_CYLINDRICAL_BLOB) {
            /* Add the coefficients into the equation for the
               section of the ray we are looking at. */
            for (j=0;j<5;j++) coeffs[j] += tcoeffs[j];
            }
         else if (element->type == T_TOROIDAL_BLOB) {
            /* Add the order 16 polynomial to the pot */ 
            for (j=0;j<=16;j++) scoeffs[j] += tcoeffs[j];
            scoeff_cnt++;
            }
         else
            error("Bad element type: %d\n", element->type);
         }
      else {
         /* We are losing the influence of a component, so
            subtract off its coefficients */
         element = blob->list + intervals[i].index;
         tcoeffs = element->tcoeffs;
         if (element->type == T_SPHERICAL_BLOB ||
             element->type == T_HEMISPHERICAL_BLOB ||
             element->type == T_PLANAR_BLOB ||
             element->type == T_CYLINDRICAL_BLOB)
            for (j=0;j<5;j++) coeffs[j] -= tcoeffs[j];
         else if (element->type == T_TOROIDAL_BLOB) {
            /* Subtract off the order 16 coefficients */
            scoeff_cnt--;
            for (j=0;j<=16;j++) scoeffs[j] -= tcoeffs[j];
            }
         else
            error("Bad element type: %d\n", element->type);
         if (--in_flag == 0)
            /* None of the components are currently affecting
               the ray - skip ahead. */
            continue;
         }

      low  = MAX(mindist, intervals[i].bound);
      high = MIN(maxdist, intervals[i+1].bound);

      /* Solve only for roots that are in the interval(s) of influence
         of the active set of blob components.  Use the root solver that
         was specified. */
      if (scoeff_cnt) {
         /* Add in any quartic coefficients */
         for (j=0;j<=4;j++)
            scoeffs[j+12] += coeffs[j];
         root_count = bounded_polysolve(16, scoeffs, roots, low, high);
         for (j=0;j<=4;j++)
            scoeffs[j+12] -= coeffs[j];
         }
      else if (blob->Sturm_Flag == 0)
         /* Use Ferrari's method */
         root_count = solve_quartic(coeffs, roots, low, high);
      else if (blob->Sturm_Flag == 1)
         /* Use Vieta's method */
         root_count = solve_quartic1(coeffs, roots, low, high);
      else
         /* Sturm sequences */
         root_count = bounded_polysolve(4, coeffs, roots, low, high);

      /* See if any of the roots are valid */
      for(j=0;j<root_count;j++) {
         dist = roots[j];
         /* See if it is in a valid part of the current ray. */
         VecAddScaled(ray->P, dist, ray->D, P);
         BlobNormal(obj, P, N);
         MakeVector(0, 0, 0, U);
         Insert_Hit(obj, P, N, dist, U, hit);
         Flag++;
         }
      }

   return (Flag?1:0);
}

/* The point P is assumed to already be in object space */
int
BlobNormal(Object *obj, Vec P, Vec N)
{
   Blob *blob = (Blob *)obj->o_data;
   int i;
   Blob_Element *temp;
   Flt val, dist2, dist;
   Flt t0, t1;
   Vec V, V1;
   int flag = 0;

   /*MakeVector(0.0, 0.0, 0.0, N);*/
   MakeVectorZero(N);
   /* For each component that contributes to this point, add
      its bit to the normal */
   temp = blob->list;
   for(i=0;i<blob->count;i++,temp++) {
      if (temp->type == T_SPHERICAL_BLOB) {
         TxVector(V, P, temp->trans);
         VecSub(V, temp->pos, V);
         /* VecSub(P, temp->pos, V); */
         dist2 = VecDot(V, V);
         if (dist2 < temp->radius2 + EPSILON) {
#if DFN == 0
            val = -2.0 * temp->coeffs[0] * dist2 -
                         temp->coeffs[1];
#else
            val = -2.0 * temp->coeffs[0] * dist2;
#endif
            InvTxNormal(V, V, temp->trans);
            VecAddS(val, V, N, N);
            flag = 1;
            }
         }
      else if (temp->type == T_CYLINDRICAL_BLOB) {
         TxVector(V, P, temp->trans);
         if (V[2] > -EPSILON && V[2] < temp->len+EPSILON) {
            V[2] = 0.0;
            dist2 = VecDot(V, V);
            if (dist2 < temp->radius2 + EPSILON) {
               InvTxDirection(V, V, temp->trans);
               val = -2.0 * temp->coeffs[0] * dist2 -
                            temp->coeffs[1];
               VecAddS(val, V, N, N);
               flag = 1;
               }
            }
         }
      else if (temp->type == T_HEMISPHERICAL_BLOB) {
         VecSub(P, temp->pos, V);
         if ((VecDot(V, temp->dir) >= 0.0) &&
             ((dist2 = VecDot(V, V)) < temp->radius2 + EPSILON)) {
            val = -2.0 * temp->coeffs[0] * dist2 -
                         temp->coeffs[1];
            VecAddS(val, V, N, N);
            flag = 1;
            }
         }
      else if (temp->type == T_PLANAR_BLOB) {
         /* Determine gradient for planar blob */
         dist = VecDot(P, temp->dir) + temp->len;
         if (dist >= 0.0 && dist < temp->radius2 + EPSILON) {
            dist2 = dist * dist;
            val = -2.0 * temp->coeffs[0] * dist2 -
                         temp->coeffs[1];
            VecAddS(val, temp->dir, N, N);
            flag = 1;
            }
         }
      else if (temp->type == T_TOROIDAL_BLOB) {
         TxVector(V, P, temp->trans);
         t0 = V[0] * V[0] + V[1] * V[1];
         if (t0 < temp->len) {
            /* Inside center circle */
            t0 = sqrt(t0);
            if (fabs(t0) < EPSILON) t0 = 1.0;
            t1 = sqrt(temp->len);
            dist2 = (t1 - t0);
            dist2 = dist2 * dist2;
            MakeVector(V[0], V[1], 0.0, V1);
            t1 = t1 / t0;
            VecScale(t1, V1);
            VecSub(V, V1, V1);
            }
         else if (t0 > temp->len) {
            /* Outside center circle */
            t0 = sqrt(t0);
            if (fabs(t0) < EPSILON) t0 = 1.0;
            t1 = sqrt(temp->len);
            dist2 = (t0 - t1);
            dist2 = dist2 * dist2;
            MakeVector(V[0], V[1], 0.0, V1);
            t1 = t1 / t0;
            VecScale(t1, V1);
            VecSub(V, V1, V1);
            }
         else {
            MakeVector(0.0, 0.0, V[2], V1);
            dist2 = 0.0;
            }
         dist2 += V[2] * V[2];
         if (dist2 < temp->radius2 + EPSILON) {
            InvTxDirection(V1, V1, temp->trans);
            val = -2.0 * temp->coeffs[0] * dist2 -
                         temp->coeffs[1];
            VecAddS(val, V1, N, N);
            flag = 1;
            }
         }
      else
         error("Bad blob component type in 'BlobNormal'\n");
      }
   if (!flag) {
      MakeVector(0.0, 0.0, -1.0, N);
      }
   return flag;
}

/* Calculate the field value of a blob - the position vector
   "Pos" must already have been transformed into blob space. */
static Flt
calculate_field_value(Object *obj, Vec Pos)
{
   int i;
   Flt len, density, t0;
   Vec V;
   Blob_Element *ptr;
   Blob *blob = (Blob *)obj->o_data;

   density = 0.0;
   for (i=0,ptr=blob->list;i<blob->count;i++,ptr++) {
      if (ptr->type == T_SPHERICAL_BLOB) {
         TxVector(V, Pos, ptr->trans);
         VecSub(ptr->pos, V, V);
         /* VecSub(ptr->pos, Pos, V); */
         len = VecDot(V, V);
         if (len < ptr->radius2 + EPSILON) {
            /* Inside the radius of influence of this
               component, add it's contribution */
            density += len * (len * ptr->coeffs[0] +
                                    ptr->coeffs[1]) +
                       ptr->coeffs[2];
            }
         }
      else if (ptr->type == T_CYLINDRICAL_BLOB) {
         TxVector(V, Pos, ptr->trans);
         if (V[2] > -EPSILON && V[2] < ptr->len+EPSILON) {
            V[2] = 0.0;
            len = VecDot(V, V);
            if (len <= ptr->radius2 + EPSILON)
               density += len * (len * ptr->coeffs[0] +
                                       ptr->coeffs[1]) +
                          ptr->coeffs[2];
               }
            }
      else if (ptr->type == T_HEMISPHERICAL_BLOB) {
         VecSub(Pos, ptr->pos, V);
         if ((VecDot(V, ptr->dir) >= 0.0) &&
             ((len = VecDot(V, V)) < ptr->radius2 + EPSILON)) {
            density += len * (len * ptr->coeffs[0] +
                                    ptr->coeffs[1]) +
                       ptr->coeffs[2];
            }
         }
      else if (ptr->type == T_PLANAR_BLOB) {
         len = VecDot(Pos, ptr->dir) + ptr->len;
         if (len >= 0.0)
            if (len < ptr->radius2 + EPSILON) {
               len = len * len;
               density += len * (len * ptr->coeffs[0] +
                                       ptr->coeffs[1]) +
                          ptr->coeffs[2];
               }
         }
      else if (ptr->type == T_TOROIDAL_BLOB) {
         TxVector(V, Pos, ptr->trans);
         t0 = V[0] * V[0] + V[1] * V[1];
         if (t0 < ptr->len) {
            /* Inside center circle */
            len = (sqrt(ptr->len) - sqrt(t0));
            len = len * len;
            }
         else if (t0 > ptr->len) {
            /* Outside center circle */
            len = (sqrt(t0) - sqrt(ptr->len));
            len = len * len;
            }
         else
            len = 0.0;
         len += V[2] * V[2];
         if (len < ptr->radius2 + EPSILON)
            density += len * (len * ptr->coeffs[0] +
                                    ptr->coeffs[1]) +
                       ptr->coeffs[2];
         }
      }
  return density;
}

/* Calculate the density at this point, then compare to
   the threshold to see if we are in or out of the blob */
int
BlobInside(Object *obj, Vec Pos)
{
   Vec P;

   InvTxVector1(P, Pos, obj->o_trans)
   return ((calculate_field_value(obj, P) >
            ((Blob *)(obj->o_data))->threshold) ? 1 : 0);
}

static Flt
evaluate_blob(Object *obj, Vec Pos)
{
   Vec P;
   if (obj->o_trans)
      InvTxVector1(P, Pos, obj->o_trans)
   else
      VecCopy(Pos, P);
   return calculate_field_value(obj, P);
}

void
BlobRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   MarchCubes(eye, Root, obj->o_uv_steps[0], obj->o_uv_steps[1],
              obj->o_uv_steps[2], &(obj->o_bnd),
              ((Blob *)(obj->o_data))->threshold,
              evaluate_blob,
              BlobNormal, obj);

}
