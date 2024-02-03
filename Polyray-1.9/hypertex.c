/* hypertex.c

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
#include "builder.h"
#include "bound.h"
#include "hypertex.h"
#include "function.h"
#include "eval.h"
#include "shade.h"
#include "trace.h"
#include "csg.h"

/* #define DENSITY_NORMALS 1 */
typedef struct {
   int flag;            /* Set to 0 if the object hasn't been initialized */
   NODE_PTR fn;         /* Symbolic function */
   Vec deltas;          /* Size of each voxel */
   int sizes[3];        /* Number of sides of the containing box */
   } HypertextureData;

void HypertextureRender(Viewpoint *, BinTree *, Object *);
int HypertextureIntersect(Viewpoint *Eye, Object *, Ray *, Flt, Flt, Isect *);
int HypertextureInside(Object *, Vec);
void HypertextureCopy(Object *, Object *);
void HypertextureDelete(Object *);

ObjectProcs HypertextureProcs = {
   HypertextureRender,
   NULL,
   GenericInitialize,
   HypertextureIntersect,
   HypertextureInside,
   GenericCopy,
   HypertextureDelete,
   };

#ifdef DENSITY_NORMALS
static int
HypertextureNormal(Object *obj, Vec P, Vec N)
{
   int i;
   Flt fval;
   Vec vval;
   struct subst_struct subst, *sp;
   HypertextureData *FnData = (HypertextureData *)obj->o_data;
   NODE_PTR exper = FnData->fn;

   /* Solve for the partial with respect to x, then y, then z */
   sp = &subst;
   VecCopy(P, subst.P);
   MakeVector(1, 0, 0, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[0] = (i == 1 ? fval : vval[0]);
   MakeVector(0, 1, 0, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[1] = (i == 1 ? fval : vval[1]);
   MakeVector(0, 0, 1, subst.PT);
   i = eval_node_dx(sp, exper, &fval, vval);
   N[2] = (i == 1 ? fval : vval[2]);

   /* Make the gradient point out of the Hypertexture */
   VecScale(-1.0, N);
   (void)VecNormalize(N);

   return 1;
}
#endif

static void
InitializeHypertexture(Object *obj)
{
   HypertextureData *FnData = (HypertextureData *)obj->o_data;

   FnData->flag = 1;

   FnData->sizes[0] = obj->o_uv_steps[0];
   FnData->sizes[1] = obj->o_uv_steps[1];
   FnData->sizes[2] = obj->o_uv_steps[2];

   FnData->deltas[0] = obj->o_bnd.lengths[0] / (Flt)(FnData->sizes[0] - 1);
   FnData->deltas[1] = obj->o_bnd.lengths[1] / (Flt)(FnData->sizes[1] - 1);
   FnData->deltas[2] = obj->o_bnd.lengths[2] / (Flt)(FnData->sizes[2] - 1);
}

static void
diffuse_lighting(Viewpoint *Eye, Object *obj,
                 Flt Kd_scale, fVec Kd_color,
                 Vec W, Vec I, Vec dcolor)
{
   Ray tray;
   Light *light;
   Flt d, t, tmin, intensity, radius;
   Vec N, SV, light_pos, light_color, L;
   int i, j;

   /* Diffuse part */
   MakeVector(0, 0, 0, dcolor)
   if (Kd_scale == 0.0)
      return;

   VecCopy(I, N)
   VecNegate(N)
   for (light=Lights,j=0;light!=NULL;light=light->next,j++) {
      VecCopy(W, tray.P);
      intensity = Light_Color(light, W, light_color, light_pos, &radius);
      if (ABS(intensity) < EPSILON)
         continue;
      MakeVector(1.0, 1.0, 1.0, SV);
      nShadows++;
      #ifdef DEBUG
      printf("\nnShadows in hypertex=%ul\n",nShadows);
      printf("press enter\n");getchar();
      #endif
      VecSub(light_pos, W, L);
      t = VecNormalize(L);
      if ((d = VecDot(N, L)) <= 0) {
         d = 0.1;
         }
      else {
         d = (d + 0.1) / 1.1;
         }

      VecCopy(L, tray.D);
      if (Rendering_Method == SCAN_CONVERSION)
          /* The polygons can be pretty far from the real surface,
             add in a big interval before looking for intersections. */
          tmin = 0.1;
      else
         tmin = rayeps;
      if (!(obj->o_sflag & SHADOW_CHECK) || !light->flags ||
          Shadow(Eye, light, &tray, tmin, t, radius, SV)) {
         d *= intensity;
         for (i=0;i<3;i++)
             dcolor[i] += SV[i] * d * Kd_color[i] * light_color[i];
         }
      }
   VecScale(Kd_scale, dcolor)
}

int
HypertextureIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                      Flt mindist, Flt maxdist, Isect *hit)
{
   HypertextureData *FnData = (HypertextureData *)obj->o_data;
   NODE_PTR exper;
   Flt mind, maxd, idepth, itemp;
   int i, index, Flag, high_steps;
   int x[3], steps[3], highs[3];
   float step_res, alpha, surf_alpha, dx[3];
   bbox_info bbox;
   Flt fx[3], lastf, boundbox[2][3];
   Vec P, D, hitpos, pDX[3], nxp[3];
   Vec W, N, I, color, scolor, dcolor, StartP;
   fVec V;
   Surface *surf;
   Ray new_ray;
   unsigned long nr;
   unsigned short old_sflags;

   exper = FnData->fn;
   VecCopy(ray->P, P);
   VecCopy(ray->D, D);

   /* See if this Hypertexture has been initialized yet */
   if (FnData->flag == 0)
      InitializeHypertexture(obj);

   /* Find where we hit the bounding box around the height field */
   mind = mindist;
   maxd = maxdist;
   VecCopy(obj->o_bnd.lower_left, bbox.lower_left);
   VecCopy(obj->o_bnd.lengths, bbox.lengths);
   recompute_inverse_bbox(&bbox, obj->o_trans);
   VecCopy(bbox.lower_left, boundbox[0]);
   VecAdd(boundbox[0], bbox.lengths, boundbox[1]);
   if (determine_start(P, D, boundbox, &mind, &maxd))
      VecAddScaled(P, mind, D, hitpos)
   else
      return 0;

   Compute_Step_Values(FnData->deltas, FnData->sizes, D, steps, highs, dx);
   for (i=0;i<3;i++) {
      VecCopy(D, pDX[i])
      VecScale(dx[i], pDX[i]);
      }

   high_steps =
      MAX(obj->o_uv_steps[0], MAX(obj->o_uv_steps[1], obj->o_uv_steps[2]));
   step_res   = 1.0 / (float)high_steps;

   Compute_DDA_Start(FnData->deltas, FnData->sizes, &obj->o_bnd,
                     hitpos, D, x, fx);
   for (i=0;i<3;i++)
      VecAddScaled(hitpos, fx[i], D, nxp[i]);
   VecCopy(hitpos, StartP)

   /* Now walk the voxels using a 3D-DDA */
   Flag   = 0;
   idepth = PLY_HUGE;
   lastf = -PLY_HUGE;
   MakeVector(0, 0, 0, color)
   alpha = 1.0; /* Assume we are totally transparent to start with */
   for (;;) {
      /* Determine which direction to step */
      if ((fx[0] < fx[1]) && (fx[0] < fx[2]))
         index = 0;
      else if (fx[1] < fx[2])
         index = 1;
      else
         index = 2;

      /* Accumulate colors and opacity here */
      VecCopy(hitpos, P)
      VecCopy(hitpos, V)
      TxVector(W, P, obj->o_trans);
#ifdef DENSITY_NORMALS
      HypertextureNormal(obj, P, N);
      TxNormal(N, N, obj->o_trans);
#endif

      if (obj->o_parent == NULL ||
          Inside_CSG_Node(obj->o_csg_tree, W)) {
         /* Passed CSG checks, now do shading */
         VecSub(W, Eye->view_from, I)
         itemp = VecNormalize(I);
         surf = find_surface(Eye, obj, obj->o_texture, W, N, I, V, 0);
         if (surf->Kt_scale > 1.0 - EPSILON)
            surf_alpha = 1.0;
         else if (surf->Kt_scale < EPSILON)
            surf_alpha = 0.0;
         else
            surf_alpha = 1.0 - pow(1.0 - surf->Kt_scale, 100.0 * step_res);
         if (surf->Kt_scale < 1.0) {
            if (!Flag) {
               /* We are starting to accumulate color.  Use this
                  as the intersection point for the object. */
               idepth = itemp;
               Flag = 1;
               }
            /* Update the value of alpha */
            alpha      = surf_alpha * alpha;
            surf_alpha = 1.0 - surf_alpha;

            /* Do shading of this piece of the hypertexture */
            old_sflags = obj->o_sflag;
            obj->o_sflag = 0;
#ifdef DENSITY_NORMALS
            /* Ambient, diffuse, and specular shading */
            ShadeSurface(Eye, obj, surf, 0, 1.0, 1.0, I, W, N, scolor, NULL);
#else
            /* Ambient/Diffuse only shading */
            VecCopy(surf->Ka_color, scolor)
            VecScale(surf->Ka_scale, scolor)
            diffuse_lighting(Eye, obj, surf->Kd_scale, surf->Kd_color,
                             W, I, dcolor);
            VecAdd(dcolor, scolor, scolor)
#endif
            obj->o_sflag = obj->o_sflag;
            VecScale(surf_alpha, scolor)
            VecAdd(color, scolor, color)
            }
         }
      else {
         /* Since this point was chopped out by CSG, we make it
            completely transparent */
         alpha = 1.0;
         }

      /* Step to the next cell */
      x[index] += steps[index];
      if ((maxdist < fx[index]))
         break;
      else if ((x[index] == highs[index]))
         break;
      else if ((alpha < 0.001))
         break;

      fx[index] += dx[index];
      VecCopy(nxp[index], hitpos)
      VecAdd(nxp[index], pDX[index], nxp[index])
      }

   if (!Flag)
      return 0;

   /* See if we need to add any background coloring */
   if (Shadow_Test) {
      MakeVector(alpha, itemp, 0, color)
      }
   else if (alpha > 0.001) {
      VecAddScaled(ray->P, maxd, ray->D, new_ray.P)
      VecCopy(D, new_ray.D)
      /* Transform the ray back into world coordinates */
      TxVector(new_ray.P, new_ray.P, obj->o_trans);
      TxDirection(new_ray.D, new_ray.D, obj->o_trans);
      VecNormalize(new_ray.D);
      old_sflags = obj->o_sflag;
      obj->o_sflag = UNSET_SFLAG;
      Trace(NULL, 1, 1.0, &new_ray, scolor, &lastf, 1.0, &nr);
      obj->o_sflag = old_sflags;
      VecAddScaled(color, alpha, scolor, color);
      }

   if (hit->flag == 0 || mind < hit->isect_t) {
      hit->flag    = 1;
      hit->obj     = obj;
      hit->isect_t = idepth;
      hit->texture = NULL;
      VecCopy(color, hit->U);
      TxVector(hit->W, StartP, obj->o_trans)
      return 1;
      }
   else
      return 0;
}

Object *
MakeHypertexture(Object *object, NODE_PTR data)
{
   HypertextureData *FnData = (HypertextureData *)object->o_data;

   object->o_type = T_HYPERTEXTURE;
   object->o_procs = &HypertextureProcs;
   FnData = (HypertextureData *)polyray_malloc(sizeof(HypertextureData));
   if (FnData == NULL)
      error("Failed to allocate Hypertexture information");
   FnData->fn     = data;
   FnData->flag   = 0;
   object->o_data = FnData;
   MakeVector(-1, -1, -1, object->o_bnd.lower_left);
   MakeVector( 2,  2,  2, object->o_bnd.lengths);
   object->o_uv_steps[0] = 20;
   object->o_uv_steps[1] = 20;
   object->o_uv_steps[2] = 20;
   return object;
}

void
HypertextureDelete(object)
   Object *object;
{
   HypertextureData *FnData = (HypertextureData *)object->o_data;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0)
      return;

   /* Free the symbolic Hypertexture */
   deallocate_node(FnData->fn);

   /* Free the Hypertexture structure itself */
   polyray_free(FnData);
}

int
HypertextureInside(Object *obj, Vec P)
{
   /* After a bunch of trials of ways to do coherent CSG with
      hypertexture, it appears the only one that doesn't hose
      up the entire program is to only allow CSG to remove
      parts of a hypertexture.  You will not see pieces of
      another surface within a hypertexture as a result of
      a CSG operation. */
   return 0;
}

void
HypertextureRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   /* Haven't figured out how to scan convert a hypertexture yet.
      Perhaps render the sides of the bounding box and call the
      tracer recursively for each pixel. */
   ;
}
