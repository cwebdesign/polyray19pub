/* bound.c

  Copyright (C) 1991-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#include "defs.h"
#include "bound.h"
#include "vector.h"
#include "io.h"
#include "memory.h"
#include "symtab.h"
#include "intersec.h"
#include "light.h"
#include "unixcompat.h"

#define X 0
#define Y 1
#define Z 2
typedef int VecI[3];
typedef struct {
  fVec slab_num;
  fVec slab_den;
  VecI nonzero;
  VecI positive;
  } RAYINFO;

static int primcount = 0;

/* Priority queue types and variables */
typedef struct t_qelem {
   Flt              q_key;
   Object          *q_obj;
   CompositeObject *q_cdp;
   } Qelem;

static int Axis = 0;

int
calc_triangle_bounds(TriangleObject *tri_obj, bbox_info *box)
{
   int i, i1, i2, i3;
   fVec mins, maxs, *V, D;
   float t;

   i1 = tri_obj->o_vert[0];
   i2 = tri_obj->o_vert[1];
   i3 = tri_obj->o_vert[2];

   V = tri_obj->o_parent->o_vertices->V;

   /* Check to see if this triangle is degenerate */
   VecSub(V[i1], V[i2], D);
   if (VecDot(D, D) < EPSILON*EPSILON)
      return 0;
   VecSub(V[i1], V[i3], D);
   if (VecDot(D, D) < EPSILON*EPSILON)
      return 0;
   VecSub(V[i2], V[i3], D);
   if (VecDot(D, D) < EPSILON*EPSILON)
      return 0;

   for (i=0;i<3;i++) {
      mins[i] = MIN(V[i1][i], MIN(V[i2][i], V[i3][i]));
      maxs[i] = MAX(V[i1][i], MAX(V[i2][i], V[i3][i]));
      }
   VecCopy(mins, box->lower_left)
   VecSub(maxs, mins, box->lengths)
   for (i=0;i<3;i++) {
      t = 2.0e-6 * MAX(fabs(mins[i]), fabs(maxs[i]));
      if (t == 0.0) t = EPSILON;
      box->lower_left[i] -= t;
      box->lengths[i] += 2 * t;
      }

   return 1;
}

void
get_bounds(Object *obj, bbox_info *box)
{
   VecCopy(obj->o_bnd.lower_left, box->lower_left);
   VecCopy(obj->o_bnd.lengths, box->lengths);
}

/* Figure out a bounding box after a transformation */
void
recompute_bbox(bbox_info *bbox, Transform *trans)
{
   Vec lower_left, lengths, corner;
   Vec mins, maxs;
   int i, j;

   VecCopy(bbox->lower_left, lower_left);
   VecCopy(bbox->lengths, lengths);
   TxVector(mins, lower_left, trans);
   VecCopy(mins, maxs);
   for (i=1;i<8;i++) {
      VecCopy(lower_left, corner);
      corner[0] += ((i & 1) ? lengths[0] : 0.0);
      corner[1] += ((i & 2) ? lengths[1] : 0.0);
      corner[2] += ((i & 4) ? lengths[2] : 0.0);
      TxVector(corner, corner, trans);
      for (j=0;j<3;j++) {
         if (corner[j] < mins[j]) mins[j] = corner[j];
         if (corner[j] > maxs[j]) maxs[j] = corner[j];
         }
      }
   VecCopy(mins, bbox->lower_left);
   VecSub(maxs, mins, bbox->lengths);
}

/* Figure out a bounding box after applying the inverse of
   a transformation */
void
recompute_inverse_bbox(bbox_info *bbox, Transform *trans)
{
   Vec lower_left, lengths, corner;
   Vec mins, maxs;
   int i, j;

   VecCopy(bbox->lower_left, lower_left);
   VecCopy(bbox->lengths, lengths);
   InvTxVector(mins, lower_left, trans);
   VecCopy(mins, maxs);
   for (i=1;i<8;i++) {
      VecCopy(lower_left, corner);
      corner[0] += ((i & 1) ? lengths[0] : 0.0);
      corner[1] += ((i & 2) ? lengths[1] : 0.0);
      corner[2] += ((i & 4) ? lengths[2] : 0.0);
      InvTxVector(corner, corner, trans);
      for (j=0;j<3;j++) {
         if (corner[j] < mins[j]) mins[j] = corner[j];
         if (corner[j] > maxs[j]) maxs[j] = corner[j];
         }
      }
   VecCopy(mins, bbox->lower_left);
   VecSub(maxs, mins, bbox->lengths);
}

/* Figures out the box that is the intersection of two boxes */
void
bbox_intersect(bbox_info *box1, bbox_info *box2, bbox_info *result)
{
   Vec lower_left1, lower_left2;
   Vec upper_right1, upper_right2;
   int i;

   /* Determine the min and max values of the box along each axis */
   VecCopy(box1->lower_left, lower_left1);
   VecCopy(box1->lower_left, upper_right1);
   VecAdd(upper_right1, box1->lengths, upper_right1);
   VecCopy(box2->lower_left, lower_left2);
   VecCopy(box2->lower_left, upper_right2);
   VecAdd(upper_right2, box2->lengths, upper_right2);

   /* Determine the intersection of the two boxes */
   for (i=0;i<3;i++) {
      result->lower_left[i] = MAX(lower_left1[i], lower_left2[i]);
      result->lengths[i] = MIN(upper_right1[i], upper_right2[i]);
      }
   VecSub(result->lengths, result->lower_left, result->lengths);

   /* Now test to see if they intersect at all */
   for (i=0;i<3;i++)
      if (result->lengths[i] < 0.0) {
         /* Intersecting bounding box is null (note that zero width bounding
            boxes are ok - its possible we had a polygon that was aligned
            with one axis */
         MakeVector(-PLY_HUGE, -PLY_HUGE, -PLY_HUGE, result->lower_left);
         MakeVector(EPSILON, EPSILON, EPSILON, result->lengths);
         return;
         }
}

/* Figures out the box that is the union of two boxes */
void
bbox_union(bbox_info *box1, bbox_info *box2, bbox_info *result)
{
   Vec lower_left1, lower_left2;
   Vec upper_right1, upper_right2;
   int i;

   /* Determine the min and max values of the box along each axis */
   VecCopy(box1->lower_left, lower_left1);
   VecCopy(box1->lower_left, upper_right1);
   VecAdd(upper_right1, box1->lengths, upper_right1);
   VecCopy(box2->lower_left, lower_left2);
   VecCopy(box2->lower_left, upper_right2);
   VecAdd(upper_right2, box2->lengths, upper_right2);

   /* Determine the intersection of the two boxes */
   for (i=0;i<3;i++) {
      result->lower_left[i] = MIN(lower_left1[i], lower_left2[i]);
      result->lengths[i] = MAX(upper_right1[i], upper_right2[i]);
      }
   VecSub(result->lengths, result->lower_left, result->lengths);
}

/* A generally useful function - determines the entry and exit points
   of a ray with a box having corners at "bounds".  The values of entry
   and exit are constrained to lie within the input values of "mind" and
   "maxd". */
int
determine_start(Vec P, Vec D,  Flt bounds[2][3],
                Flt *mind, Flt *maxd)
{
   int i;
   Flt t, dir, pos;
   Flt tmin = *mind;
   Flt tmax = *maxd;

   for (i=0;i<3;i++) {
      dir = D[i];
      pos = P[i];
      if (dir < -EPSILON) {
         t = (bounds[0][i] - pos) / dir;
         if (t < tmin)
            return 0;
         if (t <= tmax)
            tmax = t;
         t = (bounds[1][i] - pos) / dir;
         if (t >= tmin) {
            if (t > tmax)
               return 0;
            tmin = t;
            }
         }
      else if (dir > EPSILON) {
         t = (bounds[1][i] - pos) / dir;
         if (t < tmin)
            return 0;
         if (t <= tmax)
            tmax = t;
         t = (bounds[0][i] - pos) / dir;
         if (t >= tmin) {
            if (t > tmax)
               return 0;
            tmin = t;
            }
         }
      else if (pos < bounds[0][i] || pos > bounds[1][i])
         return 0;
      }
   *mind = tmin;
   *maxd = tmax;
   return 1;
}

void
AddEyeObjects(Object *obj, objlistptr eyeprims)
{
   int i;
   fVec lows, highs;
   CompositeObject *cdp;
   bbox_info box;

   /* Make sure there are objects to work with */
   if (obj == NULL)
      return;

   get_bounds(obj, &box);
   VecCopy(box.lower_left, lows);
   VecAdd(lows, box.lengths, highs);
   for (i=0;i<3;i++)
      if (Eye.view_from[i] < lows[i] ||
          Eye.view_from[i] > highs[i])
         goto add_obj;

   if (obj->o_type == T_COMPOSITE) {
      cdp = (CompositeObject *)obj;
      for (i=0;i<cdp->c_size;i++)
         AddEyeObjects(cdp->c_object[i], eyeprims);
      return;
      }
add_obj:
   eyeprims->list = push_object(eyeprims->list, obj);
   eyeprims->count++;
}

void
AddLightObjects(BinTree *Root)
{
   ostackptr objs;
   long ocnt, tcnt;

   objs = Root->members.list;
   ocnt = Root->members.count;

   /* Put references to the primitives into the array */
   for (tcnt=0,objs=Root->members.list;tcnt<ocnt;tcnt++,objs=objs->next)
      if (objs->element->o_type == T_LIGHT) {
         Add_To_Lights(Copy_Light(objs->element->o_data,
                                  objs->element->o_trans));
         Root->lights.list = push_object(Root->lights.list, objs->element);
         Root->lights.count++;
         }
}

#if 0
static int
Compute_View_Basis(Vec From, Vec At, Vec Dir, Vec Right, Vec Up)
{
   Flt len;
   Vec U, V, C, D;

   VecSub(At, From, D);
   len = sqrt(VecDot(D, D));
   if (len < EPSILON)
      /* No distance between from and at - maximal bounds */
      return 0;
   len = 1.0 / len;
   VecScale(len, D);

   VecCross(Eye.view_up, D, U);
   /* Really should check U to ensure that view_up and D weren't in exactly
      the same direction */
   VecNormalize(U);

   VecCross(D, U, V);
   VecNormalize(V);

   VecCopy(D, Dir);
   VecCopy(U, Right);
   VecCopy(V, Up);

   return 1;
}
#endif

static void
Set_Composite_Bounds(CompositeObject *cp)
{
   int i, j, len;
   Flt tmin, tmax;
   bbox_info box1, box2;

   /* Compute the bounds of the composite based on it's children */
   len = cp->c_size;

   get_bounds(cp->c_object[0], &box1);
   VecAdd(box1.lower_left, box1.lengths, box1.lengths)
   for (i=1;i<len;i++) {
      get_bounds(cp->c_object[i], &box2);
      for (j=0;j<3;j++) {
         tmin = box2.lower_left[j];
         tmax = tmin + box2.lengths[j];
         if (tmin < box1.lower_left[j])
            box1.lower_left[j] = tmin;
         if (tmax > box1.lengths[j])
            box1.lengths[j] = tmax;
         }
      }
   VecCopy(box1.lower_left, cp->o_bnd.lower_left)
   VecSub(box1.lengths, box1.lower_left, cp->o_bnd.lengths)

#if 0
   /* If optimization is set to 3 or higher then compute uv bounds
      to the eye.  If 4 or higher then compute them to each light. */
   if (Optimizer > 3) {
      cp->c_lbnd = (AngleBounds *)polyray_malloc((nLights + 1) *
                                                 sizeof(AngleBounds));
      /* To the eye */
      VecAddScaled(cp->o_bnd.lower_left, 0.5, cp->o_bnd.lengths, C);
      Compute_View_Basis(Eye.view_from, C, D, U, V);

      /* Step through the corners of the bounding box and determine the
         angle between it and D. */
      umin = PLY_HUGE; umax = -PLY_HUGE;
      vmin = PLY_HUGE; vmax = -PLY_HUGE;
      VecCopy(cp->o_bnd.lower_left, lower_left);
      VecCopy(cp->o_bnd.lengths, lengths);
      for (i=1;i<8;i++) {
         VecCopy(lower_left, C);
         C[0] += ((i & 1) ? lengths[0] : 0.0);
         C[1] += ((i & 2) ? lengths[1] : 0.0);
         C[2] += ((i & 4) ? lengths[2] : 0.0);

         /* Determine offset in U direction */
         VecSub(Eye.view_from, C, T);
         VecNormalize(T);
         temp1 = VecDot(T, U);
         if (temp1 < umin) umin = temp1;
         if (temp1 > umax) umax = temp1;

         /* Determine offset in V direction */
         VecSub(Eye.view_from, C, T);
         VecNormalize(T);
         temp1 = VecDot(T, U);
         if (temp1 < vmin) vmin = temp1;
         if (temp1 > vmax) vmax = temp1;
         }
      VecCopy(U, cp->c_lbnd[0].U);
      VecCopy(V, cp->c_lbnd[0].V);
      cp->c_lbnd[0].umin = umin;
      cp->c_lbnd[0].umax = umax;
      cp->c_lbnd[0].vmin = vmin;
      cp->c_lbnd[0].vmax = vmax;

      if (Optimizer > 4) {
         /* Now to each light */
         }
      }
   else
#endif
      cp->c_lbnd = NULL;
}

/* Compare two slabs. */
static int
#if defined( VISUALC )
__cdecl
#endif
compslabs(void const *in_a, void const *in_b)
{
   Object **a, **b;
   bbox_info box1, box2;
   Flt am, bm;

   a = (Object **)in_a;
   b = (Object **)in_b;

   get_bounds((*a), &box1);
   get_bounds((*b), &box2);

   am = 2.0 * box1.lower_left[Axis] +
              box1.lengths[Axis];
   bm = 2.0 * box2.lower_left[Axis] +
              box2.lengths[Axis];

   if (am < bm)
      return -1;
   else if (am == bm)
      return 0;
   else
      return 1;
}

static void
FindAxis(Object **Prims, long first, long last)
{
  bbox_info bbox;
  Vec mins, maxs;
  long i;
  Flt d = -PLY_HUGE, e;

  MakeVector( PLY_HUGE, PLY_HUGE, PLY_HUGE, mins);
  MakeVector(-PLY_HUGE,-PLY_HUGE,-PLY_HUGE, maxs);

  for (i=first;i<last;i++) {
     get_bounds(Prims[i], &bbox);
      if (bbox.lower_left[X] < mins[X])
         mins[X] = bbox.lower_left[X];
      if (bbox.lower_left[X] + bbox.lengths[X] > maxs[X])
         maxs[X] = bbox.lower_left[X];
      if (bbox.lower_left[Y] < mins[Y])
         mins[Y] = bbox.lower_left[Y];
      if (bbox.lower_left[Y] + bbox.lengths[Y] > maxs[Y])
         maxs[Y] = bbox.lower_left[Y];
      if (bbox.lower_left[Z] < mins[Z])
         mins[Z] = bbox.lower_left[Z];
      if (bbox.lower_left[Z] + bbox.lengths[Z] > maxs[Z])
         maxs[Z] = bbox.lower_left[Z];
      }

   e = maxs[X] - mins[X];
   if (e > d) {
      d = e;
      Axis = 0;
      }
   e = maxs[Y] - mins[Y];
   if (e > d) {
      d = e;
      Axis = 1;
      }
   e = maxs[Z] - mins[Z];
   if (e > d) {
      d = e;
      Axis = 2;
      }
}


/* Return the surface area of the bounding box */
static Flt
SurfaceArea(Vec lengths)
{
   Flt temp= 2.0*(lengths[X] * lengths[Y] + lengths[X] * lengths[Z] +
               lengths[Y] * lengths[Z]);
#ifdef DEBUG_FN_CALLS
   printf("temp=%f\n",temp);
#endif
   return temp;
}

#if 0
/* Calculate the bounding box containing Prims[first] through Prims[last-1] */
static void
CalcBounds(bbox_info *bounds, Object **Prims,
           long first, long last)
{
    long i;
    Flt tmin, tmax;
    Vec bmin, bmax;
    bbox_info bbox;

    bmin[X] = bmin[Y] = bmin[Z] = +PLY_HUGE;
    bmax[X] = bmax[Y] = bmax[Z] = -PLY_HUGE;

   for (i = first; i < last; i++) {
      get_bounds(Prims[i], &bbox);

      tmin = bbox.lower_left[X];
      tmax = tmin + bbox.lengths[X];

      if (tmin < bmin[X]) bmin[X] = tmin;
      if (tmax > bmax[X]) bmax[X] = tmax;

      tmin = bbox.lower_left[Y];
      tmax = tmin + bbox.lengths[Y];

      if (tmin < bmin[Y]) bmin[Y] = tmin;
      if (tmax > bmax[Y]) bmax[Y] = tmax;

      tmin = bbox.lower_left[Z];
      tmax = tmin + bbox.lengths[Z];

      if (tmin < bmin[Z]) bmin[Z] = tmin;
      if (tmax > bmax[Z]) bmax[Z] = tmax;
      }

   VecCopy(bmin, bounds->lower_left);
   VecSub(bmax, bmin, bounds->lengths);
}
#endif

/* Generates a table of bound box surface areas */
static void
BuildAreaTable(Object **Prims, long a, long b, Flt *areas)
{
   long i, imin, dir;
   Vec bmin, bmax, lengths;
   Flt tmin, tmax;
   bbox_info bbox;
#ifdef DEBUG_FN_CALLS
   printf("BuildAreaTable: a=%ld b=%ld\n",a,b);
#endif
   if (a < b) {
      imin = a;
      dir = +1;
      }
   else {
      imin = b;
      dir = -1;
      }
#ifdef DEBUG_FN_CALLS
   printf("imin=%ld dir=%ld\n",imin,dir);
#endif
   bmin[X] = bmin[Y] = bmin[Z] = +PLY_HUGE;
   bmax[X] = bmax[Y] = bmax[Z] = -PLY_HUGE;

   for (i = a; i != (b + dir); i += dir) {
      get_bounds(Prims[i], &bbox);

      tmin = bbox.lower_left[X];
      tmax = tmin + bbox.lengths[X];
#ifdef DEBUG_FN_CALLS
      printf("i=%d tmin=%f tmax=%f\n",i,tmin,tmax);
#endif

      if (tmin < bmin[X]) bmin[X] = tmin;
      if (tmax > bmax[X]) bmax[X] = tmax;

      tmin = bbox.lower_left[Y];
#ifdef DEBUG_FN_CALLS
      printf("2bbox.lengths[Y]=%f",bbox.lengths[Y]);
#endif
      tmax = tmin + bbox.lengths[Y];
#ifdef DEBUG_FN_CALLS
      printf("2tmin=%f tmax=%f\n",tmin,tmax);//2tmin and tmax ARE different in c++
#endif

      if (tmin < bmin[Y]) bmin[Y] = tmin;
      if (tmax > bmax[Y]) bmax[Y] = tmax;

      tmin = bbox.lower_left[Z];
      tmax = tmin + bbox.lengths[Z];
#ifdef DEBUG_FN_CALLS
      printf("3tmin=%f tmax=%f\n",tmin,tmax);
#endif

      if (tmin < bmin[Z]) bmin[Z] = tmin;
      if (tmax > bmax[Z]) bmax[Z] = tmax;

      VecSub(bmax, bmin, lengths);
#ifdef DEBUG_FN_CALLS
      printf("LX=%f LY=%f LZ=%f\n",lengths[X],lengths[Y],lengths[Z]);
#endif

      areas[i-imin] = SurfaceArea(lengths);
      }
}

static void BuildAreaTableTest(Object **Prims, long a, long b, Flt *areas)
{
   long i, imin, dir;
   Vec bmin, bmax, lengths;
   Flt tmin, tmax;
   bbox_info bbox;

   if (a < b) {
      imin = a;
      dir = +1;
      }
   else {
      imin = b;
      dir = -1;
      }

   bmin[X] = bmin[Y] = bmin[Z] = +PLY_HUGE;
   bmax[X] = bmax[Y] = bmax[Z] = -PLY_HUGE;

   for (i = a; i != (b + dir); i += dir) {
      get_bounds(Prims[i], &bbox);

      tmin = bbox.lower_left[X];
      tmax = tmin + bbox.lengths[X];

      if (tmin < bmin[X]) bmin[X] = tmin;
      if (tmax > bmax[X]) bmax[X] = tmax;

      tmin = bbox.lower_left[Y];
      tmax = tmin + bbox.lengths[Y];

      if (tmin < bmin[Y]) bmin[Y] = tmin;
      if (tmax > bmax[Y]) bmax[Y] = tmax;

      tmin = bbox.lower_left[Z];
      tmax = tmin + bbox.lengths[Z];

      if (tmin < bmin[Z]) bmin[Z] = tmin;
      if (tmax > bmax[Z]) bmax[Z] = tmax;
#ifdef DEBUG_FN_CALLS
      printf("bmin <%f,%f,%f>\n",bmin[0],bmin[1],bmin[2]);
      printf("bmax <%f,%f,%f>\n",bmax[0],bmax[1],bmax[2]);
#endif
      VecSub(bmax, bmin, lengths);
#ifdef DEBUG_FN_CALLS
      printf("lengths <%f,%f,%f>",lengths[0],lengths[1],lengths[2]);
#endif
      areas[i-imin] = SurfaceArea(lengths);
      }
}

void BuildAreaTableTestHarness(void)
{
#ifdef DEBUG_FN_CALLS
    printf("BuildAreaTableTestHarness started\n");
#endif
    Object **Prims = (Object **)polyray_malloc(2 * sizeof(Object *));
      if (Prims == NULL)
         error("Insufficient memory to build bounding slabs\n");
    int tcnts=0;
    Object *single = (Object *)polyray_malloc(sizeof(Object));
    single->o_type = T_BEZIER;
    bbox_info boundary;
    MakeVector(-10,-10,-10,boundary.lower_left);
    MakeVector(10,10,10,boundary.lengths);
    single->o_bnd = boundary;
    single->o_copy=0;
    /*

   unsigned short  o_type;         * Holds the 'type' of the object *
   bbox_info       o_bnd;          /* Bounding box limits *
   Object         *o_parent;       /* Parent object *
   Texture        *o_texture;      /* Texture characteristics *
   ObjectProcs    *o_procs;        /* Routines specific to this object type *
   float           o_dither;       /* Random dithering for this object *
   Transform      *o_trans;        /* Transforms operating on this object *
   unsigned short  o_copy;         /* Is this just a copy of another object? *
   unsigned short  o_sflag;        /* Which shading options to use *
   csgnode        *o_csg_tree;     /* Pointer to CSG affecting this object *
   unsigned short  o_uv_steps[3];  /* Resolution along u/v/w axes *
   float           o_uv_bounds[4]; /* u,v coordinate bounds *
   NODE_PTR        o_displace;     /* Surface displacement function *
   void           *o_data;         /* Object specific data *
   ObjectVertices *o_vertices;     /* Polygon mesh information */
      /* Put references to the primitives into the array */
   Prims[0]=single;
   Flt *area_test;
   int size=1;
   area_test  = (Flt *)polyray_malloc(size * sizeof(Flt));
   BuildAreaTableTest(Prims, 0, 0, area_test);
}

static int
SortAndSplit(Object **Root, Object **Prims, long *nPrims,
             long first, long last)
{
   CompositeObject *cp;
   long size, i, m;
   Flt *area_left, *area_right;
   Flt best_index, new_index;
#ifdef DEBUG_FN_CALLS
   printf("SortAndSplit: nPrims=%ld first=%ld last=%ld\n",*nPrims,first,last);
#endif

   if ((Check_Abort_Flag == 1) && _kbhit())
      longjmp(abort_environ, 1);

   size = last - first;

   if (size <= 0)
       return 1;

   FindAxis(Prims, first, last);

   qsort((char *) (Prims + first), size, sizeof (Object *), compslabs);

   /* area_left[] and area_right[] hold the surface areas of the bounding
      boxes to the left and right of any given point. e.g. area_left[i] holds
      the surface area of the bounding box containing Prims 0 through i and
      area_right[i] holds the surface area of the box containing Prims
      i through size-1 */
   area_left  = (Flt *)polyray_malloc(size * sizeof(Flt));
   area_right = (Flt *)polyray_malloc(size * sizeof(Flt));
   if (area_left == NULL || area_right == NULL)
      error("failed to allocate bounding slab memory");

   /* precalculate the areas for speed */
   BuildAreaTable(Prims, first, last-1, area_left);
   BuildAreaTable(Prims, last-1, first, area_right);
#ifdef DEBUG_FN_CALLS
   printf("area_right[0]=%f\n",area_right[0]);
#endif
   best_index = area_right[0] * (size - 3.0);
#ifdef DEBUG_FN_CALLS
   printf("best_index=%f\n",best_index);
#endif
   m = -1;

   /* Find the most effective point to split. The best location will be
      the one that minimizes the function N1*A1 + N2*A2 where N1 and N2
      are the number of objects in the two groups and A1 and A2 are the
      surface areas of the bounding boxes of the two groups */
   for (i = 0; i < size-1; i++) {
      new_index = (i+1)*area_left[i] + (size-1-i)*area_right[i+1];
      if (new_index < best_index) {
         best_index = new_index;
         m = i + first;
         }
      }

   polyray_free(area_left);
   polyray_free(area_right);

   /* See if we should stop sorting objects */
   if (size <= clustersize || m < 0) {
      /* build a box to contain them */
      cp = (CompositeObject *)polyray_malloc(sizeof(CompositeObject));
      cp->o_type   = T_COMPOSITE;
      cp->o_parent = NULL;
      cp->c_size   = size;
      cp->c_object = (Object **)polyray_malloc(size * sizeof(Object *));
      for (i=0;i<size;i++) {
         cp->c_object[i] = Prims[first+i];
         }

      Set_Composite_Bounds(cp);

      *Root = (Object *)cp;
      if (*nPrims < primcount) {
         Prims[*nPrims] = (Object *)cp;
         *nPrims += 1;
         return 1;
         }
      else
         error("Bounding Slab creation failure - increase clustersize\n");
      }
   else {
      SortAndSplit(Root, Prims, nPrims, first, m + 1);
      SortAndSplit(Root, Prims, nPrims, m + 1, last);
      return 0;
      }
   return -1;
}

void
BuildBoundingSlabs(BinTree *Root)
{
   long low, high;
   ostackptr objs;
   long ocnt, tcnt;
   Object **Prims;

   /* Make sure there are objects to work with */
   if (Root->members.count == 0)
      return;
   else if (Optimizer == 1) {
      objs = Root->members.list;
      ocnt = Root->members.count;
      /* Make sure there are objects to work with */
      if (ocnt == 0)
         return;

      /* Allocate an array to reference the objects */
      Prims = (Object **)polyray_malloc(2 * ocnt * sizeof(Object *));
      if (Prims == NULL)
         error("Insufficient memory to build bounding slabs\n");

      /* Put references to the primitives into the array */
      for (low=0,tcnt=0,objs=Root->members.list;low<ocnt;low++,objs=objs->next)
         if (objs->element->o_type != T_LIGHT)
            Prims[tcnt++] = objs->element;

      /* Now build the hierarchical set of objects */
      high = tcnt;  /* The # of objects may have changed due to lights */
      low  = 0;
      primcount = 2 * tcnt;
      while (SortAndSplit(&Root->slab_root, Prims, &tcnt, low, high) == 0) {
         low  = high;
         high = tcnt;
         }

      /* Free up the temporary array of object references */
      polyray_free(Prims);
      }
   else
      error("Unknown optimizer\n");

#if 0
printf("Extent of scene: <%g, %g, %g> -> <%g, %g, %g>\n",
       Root->slab_root->o_bnd.lower_left[X],
       Root->slab_root->o_bnd.lower_left[Y],
       Root->slab_root->o_bnd.lower_left[Z],
       Root->slab_root->o_bnd.lower_left[X]+Root->slab_root->o_bnd.lengths[X],
       Root->slab_root->o_bnd.lower_left[Y]+Root->slab_root->o_bnd.lengths[Y],
       Root->slab_root->o_bnd.lower_left[Z]+Root->slab_root->o_bnd.lengths[Z]);
#endif
}

static void
enqueue(Qelem **QueuePtr, unsigned *Qsize, unsigned *MaxQueue,
        unsigned *QueueFlag,CompositeObject *cdp, Object *obj,
        float key)
{
   int i, j;
   Qelem *Queue = *QueuePtr, *TQueue, temp_queue;

   (*Qsize)++;
   i = *Qsize;
   if (i > maxQueueSize)
       maxQueueSize = i;
   if (i >= *MaxQueue) {
      /* Overflow condition, reallocate the Queue */
      TQueue = (Qelem *)polyray_malloc((i * 2) * sizeof(Qelem));
      if (TQueue == NULL)
         error("Out of priority queue space at %d entries\n", i * 2);

      /* Copy the old queue contents into the new queue */
      memcpy(TQueue, Queue, i * sizeof(Qelem));

      /* Reset the pointers to the priority queue */
      if (*QueueFlag)
         polyray_free(Queue);
      *QueuePtr = Queue = TQueue;
      *MaxQueue = i * 2;
      *QueueFlag = 1;
      }

   Queue[i].q_key = key;
   Queue[i].q_obj = obj;
   Queue[i].q_cdp = cdp;

   /* If we are using a priority queue, then we make sure to
      adjust the queue after every insertion to retain the sorted
      order of entries */
   for (j=i/2;i>1 && Queue[i].q_key < Queue[j].q_key;i=j,j=j/2) {
      temp_queue = Queue[i];
      Queue[i] = Queue[j];
      Queue[j] = temp_queue;
      }

   nEnqueued++;
}

/***********************************************************************
 * CheckAndEnqueue(obj, maxdist)
 * Check the current ray (as paramaterized with the num and den
 * arrays above) against the bounding volume of obj.
 * If we intersect the bounding volume, then insert it into the
 * priority queue.
 *
 ***********************************************************************/
static void
CheckAndEnqueue(Qelem **QueuePtr, unsigned *Qsize, unsigned *MaxQueue,
                unsigned *QueueFlag, CompositeObject *cdp, Object *obj,
                float maxdist, RAYINFO *rayinfo)
{
   float tmin, tmax;
   float dmin, dmax;
   bbox_info *boxptr;

  nChecked++;

#if 0
   /* Check to see if this composite is within the angular range allowed */
   if (recursion_depth == 0) {
      /* This is a ray cast from the eye */
      }
#endif

   /* Special cases tried and found lacking:
         1) Determine bounds of triangle on the fly to save
            memory (24 bytes/triangle).  Extremely slow, bounding
            times were off by an order of magnitude.
         2) Immediate queuing of triangles without bounds checks
            resulted in over 2-1 increase in intersection tests
            and around a 25% increase in overall runtime. */

   /* Determine the bounding box */
   boxptr = &obj->o_bnd;

   if (rayinfo->nonzero[X]) {
      if (rayinfo->positive[X]) {
         dmin = (boxptr->lower_left[X] - rayinfo->slab_num[X]) *
         rayinfo->slab_den[X];
         dmax = dmin + (boxptr->lengths[X]  * rayinfo->slab_den[X]);
         if (dmax < EPSILON)
            return;
         }
      else {
         dmax = (boxptr->lower_left[X] - rayinfo->slab_num[X]) *
                rayinfo->slab_den[X];
         if (dmax < EPSILON)
            return;
         dmin = dmax + (boxptr->lengths[X]  * rayinfo->slab_den[X]);
         }
      if (dmin > dmax) return;
      }
   else {
      if ((rayinfo->slab_num[X] < boxptr->lower_left[X]) ||
          (rayinfo->slab_num[X] >
           boxptr->lengths[X] + boxptr->lower_left[X])) return;
      dmin = -PLY_HUGE;
      dmax = PLY_HUGE;
      }

   if (rayinfo->nonzero[Y]) {
      if (rayinfo->positive[Y]) {
         tmin = (boxptr->lower_left[Y] - rayinfo->slab_num[Y]) *
                rayinfo->slab_den[Y];
         tmax = tmin + (boxptr->lengths[Y]  * rayinfo->slab_den[Y]);
         }
      else {
         tmax = (boxptr->lower_left[Y] - rayinfo->slab_num[Y]) *
                rayinfo->slab_den[Y];
         tmin = tmax + (boxptr->lengths[Y]  * rayinfo->slab_den[Y]);
         }
      /* unwrap the logic - do the dmin and dmax checks only when tmin and
         tmax actually affect anything, also try to escape ASAP.  Better
         yet, fold the logic below into the two branches above so as to
         compute only what is needed. You might even try tmax < EPSILON
         first (instead of second) for an early quick out */
      if (tmax < dmax) {
         if (tmax < EPSILON)
            return;
         /* check bounds only if tmax changes dmax */
         if (tmin > dmin) {
            if (tmin > tmax)
               return;
            /* do this last in case it's not needed! */
            dmin = tmin;
            }
         else {
            if (dmin > tmax)
               return;
            }
         /* do this last in case it's not needed! */
         dmax = tmax;
         }
      else {
         if (tmin > dmin) {
            if (tmin > dmax) return;
            /* do this last in case it's not needed! */
            dmin = tmin;
            } /* else nothing needs to happen, since dmin and dmax did not
                 change! */
         }
      }
   else {
      if (rayinfo->slab_num[Y] < boxptr->lower_left[Y] ||
          rayinfo->slab_num[Y] > boxptr->lengths[Y] + boxptr->lower_left[Y])
         return;
      }

   if (rayinfo->nonzero[Z] ) {
      if (rayinfo->positive[Z]) {
         tmin = (boxptr->lower_left[Z] - rayinfo->slab_num[Z]) *
                rayinfo->slab_den[Z];
         tmax = tmin + (boxptr->lengths[Z]  * rayinfo->slab_den[Z]);
         }
       else {
          tmax = (boxptr->lower_left[Z] - rayinfo->slab_num[Z]) *
                 rayinfo->slab_den[Z];
          tmin = tmax + (boxptr->lengths[Z]  * rayinfo->slab_den[Z]);
          }
      if (tmax < dmax) {
         if (tmax < EPSILON ) return;
         /* check bounds only if tmax changes dmax */
         if (tmin > dmin) {
            if (tmin > tmax) return;
            /* do this last in case it's not needed! */
            dmin = tmin;
            }
         else {
           if (dmin > tmax) return;
           }
         /* do this last in case it's not needed! */
         dmax = tmax;
         }
      else {
#ifdef DEBUG_FN_CALLS
		  printf("tmax=%f\n", tmax);
#endif
         if (tmin > dmin) {
            if (tmin > dmax) return;
            /* do this last in case it's not needed! */
            dmin = tmin;
            } /* else nothing needs to happen, since dmin and dmax did not
                 change! */
         }
      }
   else {
      if (rayinfo->slab_num[Z] < boxptr->lower_left[Z] ||
          rayinfo->slab_num[Z] > boxptr->lengths[Z] + boxptr->lower_left[Z])
         return;
      }

   /* If the minimum distance to the bounding box is greater than the
      distance to the closest hit found, then we can ignore this object.
      Looking at the statistics this appears to be a slight win, with
      just a few less objects enqueued.  No impact to runtime seen so far. */
   if (dmin > maxdist) return;

   /* The ray has hit the bounding box, let's add this object to the stack. */
   enqueue(QueuePtr, Qsize, MaxQueue, QueueFlag, cdp, obj, dmin);
}

int
BoundIntersect(Viewpoint *Eye, BinTree *World, Ray *world_ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   Qelem base_queue[PQSIZE];
   Qelem *Queue, temp_queue;
   unsigned Qsize, MaxQueue, QueueFlag;
   int i, j;
   Object *cobj, *Root;
   ostackptr tobj1;
   CompositeObject *cdp;
   RAYINFO rayinfo;
   float t, key, min_dist = maxdist;

   /* Note that the priority queue must be allocated locally so that
      intersections tests are reentrant.  Note that for gridded objects
      this routine will be called recursively, and therefore may not
      use any global variables. */
   MaxQueue = PQSIZE;
   Queue = &base_queue[0];
   QueueFlag = 0;
   Qsize = 0;
   totalQueueResets++;

   /* Now go searching for an intersection */
   hit->flag = 0;
   if ((Root = World->slab_root) == NULL)
      return 0;
   if (Root->o_type != T_COMPOSITE) {
      /* For each primitive that was declared find all intersections
         with this ray */
      totalQueues++;
      return find_object_intersections(Eye, Root, world_ray, mindist, maxdist, hit);
      }
   else {
      /* Create the direction vectors for this ray */
      VecCopy(world_ray->P, rayinfo.slab_num)
      if ((rayinfo.nonzero[X] = ((t = world_ray->D[X]) != 0.0)) != 0) {
         rayinfo.slab_den[X] = 1.0 / t;
         rayinfo.positive[X] = (world_ray->D[X] > 0.0);
         }
      if ((rayinfo.nonzero[Y] = ((t = world_ray->D[Y]) != 0.0)) != 0) {
         rayinfo.slab_den[Y] = 1.0 / t;
         rayinfo.positive[Y] = (world_ray->D[Y] > 0.0);
         }
      if ((rayinfo.nonzero[Z] = ((t = world_ray->D[Z]) != 0.0)) != 0) {
         rayinfo.slab_den[Z] = 1.0 / t;
         rayinfo.positive[Z] = (world_ray->D[Z] > 0.0);
         }

      if (World->eyeprims.count < 2 ||
                  ((Eye != NULL) &&
           (world_ray->P[0] != Eye->view_from[0] ||
            world_ray->P[1] != Eye->view_from[1] ||
            world_ray->P[2] != Eye->view_from[2])))
         CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                         NULL, Root, min_dist, &rayinfo);
      else {
         for (tobj1=World->eyeprims.list;tobj1!=NULL;tobj1=tobj1->next)
            CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                            NULL, tobj1->element, min_dist, &rayinfo);
         }

      for (;;) {
         if (Qsize == 0)
            break;

         /* Pull the closest object out of the priority queue  Dequeuing
            objects is done inline (rather than a function call) for
            reasons of speed alone. */
         key  = Queue[1].q_key;
         cobj = Queue[1].q_obj;
         cdp  = Queue[1].q_cdp;
         Queue[1] = Queue[Qsize];
         Qsize--;

         /* After pulling the closest element, we need to reset
            the priority queue so that it is back in order */
         for (i=1,j=2;j<=Qsize;i=j,j=2*j) {
            if (j < Qsize && Queue[j].q_key >= Queue[j+1].q_key)
               j++;
            if (Queue[i].q_key > Queue[j].q_key) {
               temp_queue = Queue[i];
               Queue[i] = Queue[j];
               Queue[j] = temp_queue;
               }
            else
               break;
            }

         if (key > min_dist)
            /* We are guaranteed that there are no primitives
               closer than this one, since the priority queue
               kept things in sorted order */
            break;
         else if (cobj->o_type == T_COMPOSITE) {
            /* if it is in the queue, it got hit.  Check each of
               its children to see if their bounding volumes get hit.
               if so, then push them into the priority queue... */
            cdp = (CompositeObject *)cobj;

            for (i=0;i<cdp->c_size;i++)
               CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                               cdp, cdp->c_object[i], min_dist, &rayinfo);
            }
         else {
            /* Do the ray-surface intersect check */
            totalQueues++;
            if (find_object_intersections(Eye, cobj, world_ray,
                                           MAX(0.9999*key,mindist), min_dist, hit)) {
               min_dist = hit->isect_t;
               /* Don't need to look any farther - we have an opaque
                  surface in a shadow check. */
               if (Shadow_Test &&
                   !((Global_Shade_Flag & TRANSMIT_CHECK) &&
                     (hit->obj->o_sflag & TRANSMIT_CHECK)))
                  break;
               }
            }
         }
      }

   if (QueueFlag)
      polyray_free(Queue);
   return hit->flag;
}

#if 0
/* If the point P is inside anything, this routine will return 1 and
   the variable obj will point to the object itself (actually to the
   first one found */
static int
BoundInside(Vec P, Object *obj)
{
   /* Check CSG (if any) */
   if (obj->o_parent != NULL &&
       !Inside_CSG_Node(obj->o_csg_tree, P))
         return 0;
   else if (Root->o_type != T_COMPOSITE)
      return (obj->o_procs->inside)(Root, P);
   else {
      return inside_object_tree(Root, P);
      }
}

static int
inside_object_tree(Object *obj, Vec P)
{
   CompositeObject *cobj;
   int i;

   if (Root->o_type != T_COMPOSITE)
      return (Root->o_procs->inside)(Root, P);
   else
      cdp = (CompositeObject *)cobj;
      for (i=0;i<cdp->c_size;i++)
      if (obj->o_parent != NULL)
         return Inside_CSG_Node(Root->o_csg_tree
      else
         return (obj->o_procs->inside)(Root, P);

      /* Create the direction vectors for this ray */
      VecCopy(world_ray->P, rayinfo.slab_num)
      if ((rayinfo.nonzero[X] = ((t = world_ray->D[X]) != 0.0)) != 0) {
         rayinfo.slab_den[X] = 1.0 / t;
         rayinfo.positive[X] = (world_ray->D[X] > 0.0);
         }
      if ((rayinfo.nonzero[Y] = ((t = world_ray->D[Y]) != 0.0)) != 0) {
         rayinfo.slab_den[Y] = 1.0 / t;
         rayinfo.positive[Y] = (world_ray->D[Y] > 0.0);
         }
      if ((rayinfo.nonzero[Z] = ((t = world_ray->D[Z]) != 0.0)) != 0) {
         rayinfo.slab_den[Z] = 1.0 / t;
         rayinfo.positive[Z] = (world_ray->D[Z] > 0.0);
         }

      if (World->eyeprims.count < 2 ||
          world_ray->P[0] != Eye->view_from[0] ||
          world_ray->P[1] != Eye->view_from[1] ||
          world_ray->P[2] != Eye->view_from[2])
         CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                         NULL, Root, min_dist, &rayinfo);
      else {
         for (tobj1=World->eyeprims.list;tobj1!=NULL;tobj1=tobj1->next)
            CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                            NULL, tobj1->element, min_dist, &rayinfo);
         }

      for (;;) {
         if (Qsize == 0)
            break;

         /* Pull the closest object out of the priority queue  Dequeuing
            objects is done inline (rather than a function call) for
            reasons of speed alone. */
         key  = Queue[1].q_key;
         cobj = Queue[1].q_obj;
         cdp  = Queue[1].q_cdp;
         Queue[1] = Queue[Qsize];
         Qsize--;

         /* After pulling the closest element, we need to reset
            the priority queue so that it is back in order */
         for (i=1,j=2;j<=Qsize;i=j,j=2*j) {
            if (j < Qsize && Queue[j].q_key >= Queue[j+1].q_key)
               j++;
            if (Queue[i].q_key > Queue[j].q_key) {
               temp_queue = Queue[i];
               Queue[i] = Queue[j];
               Queue[j] = temp_queue;
               }
            else
               break;
            }

         if (key > min_dist)
            /* We are guaranteed that there are no primitives
               closer than this one, since the priority queue
               kept things in sorted order */
            break;
         else if (cobj->o_type == T_COMPOSITE) {
            /* if it is in the queue, it got hit.  Check each of
               its children to see if their bounding volumes get hit.
               if so, then push them into the priority queue... */
            cdp = (CompositeObject *)cobj;

            for (i=0;i<cdp->c_size;i++)
               CheckAndEnqueue(&Queue, &Qsize, &MaxQueue, &QueueFlag,
                               cdp, cdp->c_object[i], min_dist, &rayinfo);
            }
         else {
            /* Do the ray-surface intersect check */
            totalQueues++;
            if (find_object_intersections(Eye, cobj, world_ray,
                                           MAX(0.9999*key,mindist), min_dist, hit)) {
               min_dist = hit->isect_t;
               /* Don't need to look any farther - we have an opaque
                  surface in a shadow check. */
               if (Shadow_Test &&
                   !((Global_Shade_Flag & TRANSMIT_CHECK) &&
                     (hit->obj->o_sflag & TRANSMIT_CHECK)))
                  break;
               }
            }
         }
      }

   if (QueueFlag)
      polyray_free(Queue);
   return hit->flag;
}
#endif

