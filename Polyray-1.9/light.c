/* light.c

  Compute color and intensity for a variety of light types

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
#include "vector.h"
#include "light.h"
#include "alight.h"
#include "scan.h"
#include "image.h"
#include "builder.h"
#include "eval.h"
#include "symtab.h"
#include "intersec.h"
#include "shade.h"

Light *Current_Light = NULL;

/* Define the distinct types of lights */
#define T_POINT_LIGHT       (150)
#define T_SPOT_LIGHT        (151)
#define T_TEXTURED_LIGHT    (152)
#define T_DIRECTIONAL_LIGHT (153)
#define T_DEPTH_LIGHT       (154)

struct t_depth_light {
   NODE_PTR color;
   Img *light_depth;
   Vec light_from;
   Vec light_at;
   Vec light_up;
   Flt light_angle;
   Flt light_aspect;
   float light_bias;
   Transform *WS;
   };

typedef struct t_flare_component FLARE;
struct t_flare_component {
   NODE_PTR color; /* Color function for this flare */
   int count;      /* Total number of flares with this light */
   float spacing;  /* Power function for lens spacing */
   int seed;       /* Random number seed for building flare spacing */
   float min_rad;  /* Smallest flare size (fraction of image width [0, 1]) */
   float max_rad;  /* Largest flare size */
   float concave;  /* Ratio of concave to convex flares */
   float radius;   /* Size of glow around the light */
   };

struct t_textured_light {
   NODE_PTR color;    /* Run-time color for light */
   Vec dcolor;        /* If color doesn't change then this one is faster */
   Flt radius;        /* Radius when used as an area light */
   FLARE *lens_flare; /* Linked list of flares attached to this light */
   Transform *tx;
   PolyAlightData *alight; /* Data used if this is a polygonal light */
   };

struct t_spot_light {
   Vec light_pos, light_dir, light_color;
   Flt Coef, Radius, Falloff;
   };

struct t_point_light {
   Vec light_pos, light_color;
   };

#define SHADOW_FLAG 0x01

/* Prototypes for the primitive operators */
void LightRender(Viewpoint *, BinTree *, Object *);
int LightIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int LightInside(Object *, Vec);
void LightCopy(Object *, Object *);
void LightDelete(Object *);

ObjectProcs LightProcs = {
   LightRender,
   NULL,
   GenericInitialize,
   LightIntersect,
   LightInside,
   LightCopy,
   LightDelete,
   };

int
LightIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   return 0;
}

int
LightInside(Object *obj, Vec P)
{
   return 0;
}

void
LightCopy(Object *objin, Object *objout)
{
   objout->o_data = Copy_Light((Light *)objin->o_data, NULL);
   objout->o_copy = 0;
}

void
LightDelete(Object *object)
{
   struct t_depth_light *tlight3;
   struct t_textured_light *tlight4;
   Light *light = (Light *)object->o_data;

   if (object->o_copy == 0) {
      /* Delete the light information */
      switch (light->type) {
         case T_POINT_LIGHT:
         case T_SPOT_LIGHT:
         case T_DIRECTIONAL_LIGHT:
            break;
         case T_TEXTURED_LIGHT:
            tlight4 = ((struct t_textured_light *)light->data);
            if (tlight4->tx != NULL)
               polyray_free(tlight4->tx);
            deallocate_node(tlight4->color);
            if (tlight4->lens_flare != NULL) {
               deallocate_node(tlight4->lens_flare->color);
               polyray_free(tlight4->lens_flare);
               }
            if (tlight4->alight != NULL) {
               PolyAlightData *alight = tlight4->alight;
               polyray_free(alight->nbuf);
               polyray_free(alight->obuf);
               polyray_free(alight->sbuf1);
               polyray_free(alight->sbuf2);
               polyray_free(alight->points);
               polyray_free(alight);
               }
            break;
         case T_DEPTH_LIGHT:
            tlight3 = ((struct t_depth_light *)light->data);
            polyray_free(tlight3->WS);
            break;
         default:
            error("Bad light type in Delete_Light");
         }
      polyray_free(light->data);
      polyray_free(light);
      }
}

void
LightRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   ;
}

Object *
MakeLight(Object *object, Light *light)
{
   object->o_type  = T_LIGHT;
   object->o_procs = &LightProcs;
   object->o_data  = light;
   return object;
}

void
Initialize_Light_Caches(void)
{
   #ifdef DEBUG_FN_CALLS
	printf("Initialize_Light_Caches\n");
   #endif
   int j;
   Light *light;

   for (light=Lights;light!=NULL;light=light->next) {
      for (j=0;j<MAX_CACHE_BLOCKING;j++)
         light->cache[j] = NULL;
      }
}

Object *
Get_Light_Blocker(Light *light, int depth)
{
   if (depth >= MAX_CACHE_BLOCKING)
      return NULL;
   else
      return light->cache[depth];
}

void
Set_Light_Blocker(Light *light, int depth, Object *obj)
{
   if (depth >= MAX_CACHE_BLOCKING)
      return;
   else
      light->cache[depth] = obj;
}

void
Terminate_Light_Caches(void)
{
   ;
}

void
Get_Light_Pos(Light *light, Vec Pos)
{
   struct t_point_light *tlight1;
   struct t_spot_light *tlight2;
   struct t_depth_light *tlight3;
   struct t_textured_light *tlight4;
   int ltype;

   ltype = light->type;
   if (ltype == T_POINT_LIGHT) {
      tlight1 = (struct t_point_light *)light->data;
      VecCopy(tlight1->light_pos, Pos);
      }
   else if (ltype == T_SPOT_LIGHT) {
      tlight2 = (struct t_spot_light *)light->data;
      VecCopy(tlight2->light_pos, Pos);
      }
   else if (ltype == T_DEPTH_LIGHT) {
      tlight3 = (struct t_depth_light *)light->data;
      VecCopy(tlight3->light_from, Pos);
      }
   else if (ltype == T_TEXTURED_LIGHT) {
      tlight4 = (struct t_textured_light *)light->data;
      if (tlight4->alight != NULL) {
         PolyAlightData *alight = tlight4->alight;

         VecAdd(alight->upper_right, alight->lower_left, Pos)
         VecScale(0.5, Pos)
         }
      else if (tlight4->tx == NULL)
         MakeVector(0, 0, 0, Pos)
      else
         MakeVector(tlight4->tx->matrix[3][0],
                    tlight4->tx->matrix[3][1],
                    tlight4->tx->matrix[3][2],
                    Pos)
      }
   else if (ltype == T_DIRECTIONAL_LIGHT) {
      tlight1 = (struct t_point_light *)light->data;
      MakeVector(0, 0, 0, Pos);
      VecAddScaled(Pos, PLY_HUGE, tlight1->light_pos, Pos);
      }
   else
      error("Bad light type: %d in Get_Light_Pos\n", ltype);
}

/* Determine the position, intensity, and color of a point light at P */
static Flt
Point_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   struct t_point_light *tlight;

   tlight = (struct t_point_light *)light->data;
   VecCopy(tlight->light_color, C);
   VecCopy(tlight->light_pos, L);
   *rad = 0.0;
   return 1.0;
}

void
Set_Light_Shadow(int shadow_flag)
{
   Current_Light->flags = shadow_flag;
}

void
Add_To_Lights(Light *light)
{
   int j;

   /* If we have an area light, then we transform the points that
      define it's polygon, as well as the basis vectors. */
   if (light->type == T_TEXTURED_LIGHT) {
      struct t_textured_light *tlight;
      PolyAlightData *alight;
      int i;

      tlight = (struct t_textured_light *)light->data;
      alight = tlight->alight;

      if (alight != NULL) {
         for (i=0;i<alight->npoints;i++) {
            fTxVec(alight->points[i], alight->points[i], tlight->tx);
            }
         TxDirection(alight->ubasis, alight->ubasis, tlight->tx);
         TxDirection(alight->vbasis, alight->vbasis, tlight->tx);
         TxVec(alight->lower_left, alight->lower_left, tlight->tx);
         TxVec(alight->upper_right, alight->upper_right, tlight->tx);
         }
      }

   for (j=0;j<MAX_CACHE_BLOCKING;j++)
      light->cache[j] = NULL;
   light->next = Lights;
   Lights = light;
   nLights++;
}

Light *
Copy_Light(Light *light, Transform *tx)
{
   Light *new_light;
   void  *data=NULL;
   struct t_point_light    *tlight1;
   struct t_spot_light     *tlight2;
   struct t_depth_light    *tlight3, *dlight;
   struct t_textured_light *tlight4;
   Viewpoint eye;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   if (new_light == NULL)
      error("Failed to allocate light");

   switch (light->type) {
   case T_POINT_LIGHT:
      data = tlight1 = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
      if (tlight1 == NULL)
         error("Failed to allocate point light");
      memcpy(tlight1, light->data, sizeof(struct t_point_light));
      TxVector(tlight1->light_pos,
               ((struct t_point_light *)light->data)->light_pos,
               tx)
      break;
   case T_SPOT_LIGHT:
      data = tlight2 = (struct t_spot_light *)polyray_malloc(sizeof(struct t_spot_light));
      if (tlight2 == NULL)
         error("Failed to allocate spotlight");
      memcpy(tlight2, light->data, sizeof(struct t_spot_light));
      TxVector(tlight2->light_pos,
               ((struct t_spot_light *)light->data)->light_pos,
               tx)
      TxDirection(tlight2->light_dir,
                  ((struct t_spot_light *)light->data)->light_dir,
                  tx)
      (void)VecNormalize(tlight2->light_dir);
      break;
   case T_TEXTURED_LIGHT:
      data = tlight4 = (struct t_textured_light *)polyray_malloc(sizeof(struct t_textured_light));
      if (tlight4 == NULL)
         error("Failed to allocate textured light");
      memcpy(tlight4, light->data, sizeof(struct t_textured_light));
      if (tlight4->tx != NULL) {
         tlight4->tx = Get_Transformation();
         Compose_Transformations(tlight4->tx, ((struct t_textured_light *)light->data)->tx);
         if (tx != NULL)
            Compose_Transformations(tlight4->tx, tx);
         }
      else if (tx != NULL) {
         tlight4->tx = Get_Transformation();
         Compose_Transformations(tlight4->tx, tx);
         }
      if (tlight4->lens_flare != NULL) {
         FLARE *tflare;
         tflare = polyray_malloc(sizeof(FLARE));
         memcpy(tflare, tlight4->lens_flare, sizeof(FLARE));
         tflare->color = copy_node(tlight4->lens_flare->color);
         tlight4->lens_flare = tflare;
         }
      if (tlight4->alight != NULL) {
         PolyAlightData *alight;
         alight = (PolyAlightData *)polyray_malloc(sizeof(PolyAlightData));
         memcpy(alight, tlight4->alight, sizeof(PolyAlightData));
         alight->nbuf   = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
         alight->obuf   = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
         alight->sbuf1  = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
         alight->sbuf2  = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
         alight->points = (fVec *)polyray_malloc(alight->npoints * sizeof(fVec));
         memcpy(alight->points, tlight4->alight->points, alight->npoints * sizeof(fVec));
         tlight4->alight = alight;
         }
      break;
   case T_DIRECTIONAL_LIGHT:
      data = tlight1 = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
      if (tlight1 == NULL)
         error("Failed to allocate light");
      memcpy(tlight1, light->data, sizeof(struct t_point_light));
      TxDirection(tlight1->light_pos,
                  ((struct t_spot_light *)light->data)->light_pos,
                  tx)
      break;
   case T_DEPTH_LIGHT:
      data = tlight3 = (struct t_depth_light *)polyray_malloc(sizeof(struct t_depth_light));
      if (tlight3 == NULL)
         error("Failed to allocate light");
      dlight = (struct t_depth_light *)light->data;
      memcpy(tlight3, light->data, sizeof(struct t_depth_light));
      TxVector(tlight3->light_from, dlight->light_from, tx);
      TxVector(tlight3->light_at, dlight->light_at, tx);
      TxVector(tlight3->light_up, dlight->light_up, tx);
      VecCopy(tlight3->light_from, eye.view_from);
      VecCopy(tlight3->light_at, eye.view_at);
      VecCopy(tlight3->light_up, eye.view_up);
      eye.view_aspect = tlight3->light_aspect;
      eye.view_xres   = tlight3->light_depth->width;
      eye.view_yres   = tlight3->light_depth->length;
      eye.view_angle  = tlight3->light_angle;
      eye.view_hither = SMALL;
      eye.view_yon    = PLY_HUGE;
      tlight3->WS = Normalize_View(&eye);
      break;
   default:
      error("Bad light type in Copy_Light");
   }

   if (new_light == NULL)
      error("Failed to allocate light");
   new_light->type  = light->type;
   new_light->flags = light->flags;
   new_light->data  = data;
   new_light->next  = NULL;

   return new_light;
}

Light *
light_action1(Vec color, Vec pos)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   new_light->type = T_POINT_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   VecCopy(color, data->light_color);
   VecCopy(pos, data->light_pos);
   Current_Light = new_light;
   return new_light;
}

Light *
light_action2(Vec pos)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   MakeVector(1.0, 1.0, 1.0, data->light_color);
   VecCopy(pos, data->light_pos);
   new_light->type = T_POINT_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   Current_Light = new_light;
   return new_light;
}

Light *
SetSpotLight(Vec color, Vec pos, Vec at, Flt coef, Flt radius, Flt falloff)
{
   Light *new_light;
   struct t_spot_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_spot_light *)polyray_malloc(sizeof(struct t_spot_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   VecCopy(color, data->light_color);
   VecCopy(pos, data->light_pos);
   VecSub(at, pos, data->light_dir);
   (void)VecNormalize(data->light_dir);
   data->Coef    = coef;
   data->Radius  = cos(radius * M_PI / 180.0);
   data->Falloff = cos(falloff * M_PI / 180.0);
   new_light->type = T_SPOT_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   Current_Light = new_light;
   return new_light;
}

Light *
light_action3(void)
{
   Light *new_light;
   struct t_textured_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_textured_light *)
          polyray_malloc(sizeof(struct t_textured_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   MakeVector(1, 1, 1, data->dcolor); /* Default color is white */
   data->color      = NULL;
   data->radius     = 0.0;
   data->tx         = NULL;
   data->lens_flare = NULL;
   data->alight     = NULL;

   new_light->type  = T_TEXTURED_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data  = (void *)data;
   new_light->next  = NULL;
   Current_Light = new_light;
   return new_light;
}

Light *
light_action4(Vec dir)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   MakeVector(1.0, 1.0, 1.0, data->light_color);
   VecCopy(dir, data->light_pos);
   new_light->type = T_DIRECTIONAL_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   Current_Light = new_light;
   return new_light;
}

Light *
light_action5(Vec color, Vec dir)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_point_light *)polyray_malloc(sizeof(struct t_point_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   VecCopy(color, data->light_color);
   VecCopy(dir, data->light_pos);
   new_light->type = T_DIRECTIONAL_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   Current_Light = new_light;
   return new_light;
}

Light *
light_action6(void)
{
   Light *new_light;
   struct t_depth_light *data;

   new_light = (Light *)polyray_malloc(sizeof(struct t_light));
   data = (struct t_depth_light *)
          polyray_malloc(sizeof(struct t_depth_light));
   if (new_light == NULL || data == NULL)
      error("Failed to allocate light");
   data->color = NULL;
   data->light_depth = NULL;
   MakeVector(0, 50, 0, data->light_from);
   MakeVector(0, 0, 0, data->light_at);
   MakeVector(0, 0, 1, data->light_up);
   data->light_aspect = 1.0;
   data->light_angle  = degtorad(22.5);
   data->light_bias = rayeps;
   data->WS = NULL;
   new_light->type = T_DEPTH_LIGHT;
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = NULL;
   Current_Light = new_light;
   return new_light;
}

void
DepthLight1(Flt ang)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_angle = degtorad(ang/2.0);
}

void
DepthLight2(Flt asp)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_aspect = asp;
}

void
DepthLight3(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_at);
}

void
DepthLight4(NODE_PTR exper)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->color = exper;
}

void
DepthLight5(char *filename)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_depth = TGAReadImage(filename);
}

void
DepthLight6(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_from);
}

void
DepthLight7(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_up);
}

void
DepthLight8()
{
   Viewpoint eye;
   struct t_depth_light *tlight;
   int xres, yres;

   tlight = (struct t_depth_light *)Current_Light->data;

   if (tlight->light_depth == NULL) {
      xres = 256;
      yres = 256;
      }
   else {
      xres = tlight->light_depth->width;
      yres = tlight->light_depth->length;
      }

   VecCopy(tlight->light_from, eye.view_from);
   VecCopy(tlight->light_at, eye.view_at);
   VecCopy(tlight->light_up, eye.view_up);
   eye.view_aspect = tlight->light_aspect;
   eye.view_xres   = xres;
   eye.view_yres   = yres;
   eye.view_angle  = tlight->light_angle;
   eye.view_hither = SMALL;
   eye.view_yon    = PLY_HUGE;
   tlight->WS = Normalize_View(&eye);
}

void
DepthLight9(Flt d)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_bias = d;
}

void
Set_Light_Color(NODE_PTR exper)
{
   struct t_textured_light *tlight;
   Flt ftemp;
   NODE_PTR tnode;

   tlight = (struct t_textured_light *)Current_Light->data;

   if (eval_node(NULL, exper, &ftemp, tlight->dcolor, &tnode) != 2) {
      /* Run-time color */
      if (tlight->color != NULL)
         deallocate_node(tlight->color);
      tlight->color = exper;
      }
   else {
      /* Color doesn't change */
      deallocate_node(exper);
      deallocate_node(tlight->color);
      tlight->color = NULL;
      }
}

void
Set_Light_Radius(Flt rad)
{
   struct t_textured_light *tlight;

   tlight = (struct t_textured_light *)Current_Light->data;
   tlight->radius = rad;

   if (tlight->alight != NULL) {
      PolyAlightData *alight = tlight->alight;
      polyray_free(alight->nbuf);
      polyray_free(alight->obuf);
      polyray_free(alight->sbuf1);
      polyray_free(alight->sbuf2);
      polyray_free(alight->points);
      polyray_free(alight);
      }
}

void
Set_Light_Polygon(Flt ures, Flt vres, Flt adaptive_depth, Flt jitter)
{
   struct t_textured_light *tlight;
   PolyAlightData *alight;
   int i;

   tlight = (struct t_textured_light *)Current_Light->data;

   if (tlight->alight != NULL) {
      PolyAlightData *alight = tlight->alight;
      polyray_free(alight->nbuf);
      polyray_free(alight->obuf);
      polyray_free(alight->sbuf1);
      polyray_free(alight->sbuf2);
      polyray_free(alight->points);
      polyray_free(alight);
      }
   tlight->radius = 0.0;
   alight = (PolyAlightData *)polyray_malloc(sizeof(PolyAlightData));
   tlight->alight = alight;
   alight->ures   = MAX(1, (int)ures);
   alight->vres   = MAX(1, (int)vres);
   alight->adaptive_depth = MAX(0, (int)adaptive_depth);
   alight->jitter  = jitter;
   MakeVector(0, 0, 0, alight->lower_left)
   MakeVector(1, 0, 1, alight->upper_right)
   MakeVector(1, 0, 0, alight->ubasis)
   MakeVector(0, 0, 1, alight->vbasis)
   alight->nbuf    = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
   alight->obuf    = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
   alight->sbuf1   = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
   alight->sbuf2   = (Vec *)polyray_malloc((alight->vres+1) * sizeof(Vec));
   for (i=0;i<alight->ures;i++) {
      MakeVector(0, 0, 0, alight->nbuf[i])
      MakeVector(0, 0, 0, alight->obuf[i])
      MakeVector(0, 0, 0, alight->sbuf1[i])
      MakeVector(0, 0, 0, alight->sbuf2[i])
      }

   alight->npoints = 4;
   alight->points  = (fVec *)polyray_malloc(alight->npoints * sizeof(fVec));
   MakeVector(0, 0, 0, alight->points[0])
   MakeVector(0, 0, 1, alight->points[1])
   MakeVector(1, 0, 1, alight->points[2])
   MakeVector(1, 0, 0, alight->points[3])

   MakeVector(0, 1, 0, alight->normal)
   alight->d = 0.0;
   alight->u = 0;
   alight->v = 2;
}

void
Transform_Light(Transform *t)
{
   struct t_textured_light *tlight;

   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, t);
}

void
Shear_Light(Flt xy, Flt xz, Flt yx, Flt yz, Flt zx, Flt zy)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, &trans);
}

void
Translate_Light(Vec v)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Translation_Transformation(&trans, v);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, &trans);
}

void
Rotate_Light(Vec v)
{
   Transform trans;
   Vec vt;
   struct t_textured_light *tlight;

   VecCopy(v, vt);
   VecScale(M_PI/180.0, vt);
   Get_Rotation_Transformation(&trans, vt);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, &trans);
}

void
Rotate_Axis_Light(Vec v, Flt ang)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Rotate_Transform(&trans, v, M_PI * ang / 180.0);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, &trans);
}

void
Scale_Light(Vec v)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Scaling_Transformation (&trans, v);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == NULL) tlight->tx = Get_Transformation();
   Compose_Transformations(tlight->tx, &trans);
}

/* Determine the position, intensity, and color of a directional light at P */
static Flt
Directional_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   struct t_point_light *tlight;

   tlight = (struct t_point_light *)light->data;
   VecCopy(tlight->light_color, C);
   VecAddScaled(W, PLY_HUGE, tlight->light_pos, L);
   *rad = 0.0;
   return 1.0;
}


/* Cubic interpolation between 0 at low and 1 at high, sampled at 'x'.
   It is assumed that high >= low. */
static Flt
cubic_spline(Flt low, Flt high, Flt x)
{

   if (x < low)
      return 0.0;
   else if (x > high)
      return 1.0;
   if (high == low)
      return 0.0;
   x = (x - low) / (high - low);
   return (3 - 2 * x) * x * x;
}

/* Determine the position, intensity, and color of a spot light at P */
static Flt
Spot_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   Vec D;
   Flt len, costheta, atten;
   struct t_spot_light *tlight;

   tlight = (struct t_spot_light *)light->data;
   VecCopy(tlight->light_color, C);
   VecCopy(tlight->light_pos, L);
   VecSub(tlight->light_pos, W, D);
   len = sqrt(VecDot(D, D));
   if (len < EPSILON) return 1;
   D[0] /= len; D[1] /= len; D[2] /= len;
   costheta = -VecDot(tlight->light_dir, D);
   if (costheta <= 0.0) return 0.0;
   atten = pow(costheta, tlight->Coef);
   if (tlight->Radius > 0.0)
      atten *= cubic_spline(tlight->Falloff, tlight->Radius, costheta);
   *rad = 0.0;
   return atten;
}

/* Determine the position, intensity, and color of a textured light at P */
static Flt
Textured_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   Vec P;
   int i;
   NODE_PTR ntemp;
   Flt ftemp;
   Vec vtemp;
   struct subst_struct subst;
   struct t_textured_light *tlight;

   /* Get the info for this light */
   tlight = (struct t_textured_light *)light->data;

   /* Apply the transformation to the texture */
   if (tlight->alight != NULL) {
      PolyAlightData *alight = tlight->alight;

      VecAdd(alight->upper_right, alight->lower_left, L)
      VecScale(0.5, L)
      InvTxVector1(P, W, tlight->tx);
      }
   else if (tlight->tx == NULL) {
      MakeVector(0, 0, 0, L);
      VecCopy(W, P);
      }
   else {
      /* Copy the translation component of the transformation into
         the light position */
      MakeVector(tlight->tx->matrix[3][0],
                 tlight->tx->matrix[3][1],
                 tlight->tx->matrix[3][2],
                 L);
      InvTxVector1(P, W, tlight->tx);
      }

   if (tlight->color == NULL) {
      /* No color function - this implies that a quicker color can be used */
      VecCopy(tlight->dcolor, C);
      }
   else {
      /* Build a substitution structure to evaluate the color function */
      VecCopy(P, subst.P);
      MakeVector(0, 0, 0, subst.PT);
      VecCopy(W, subst.W)
      VecSub(L, W, subst.N)
      if (tlight->tx)
         InvTxDirection(subst.N, subst.N, tlight->tx)
      VecNormalize(subst.N);
      spherical_imagemap(subst.N, &subst.U[0], &subst.U[1]);
      VecCopy(subst.N, subst.I)
      VecNegate(subst.I)
      i = eval_node(&subst, tlight->color, &ftemp, vtemp, &ntemp);
      if (i == 1)
         MakeVector(ftemp, ftemp, ftemp, C)
      else if (i == 2)
         VecCopy(vtemp, C)
      else
         error("Invalid lighting function (not vector or float) for light\n");
      }

   *rad = tlight->radius;
   return 1.0;
}

/* Determine the position, intensity, and color of a depth mapped light */
static Flt
Depth_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   Vec D, S;
   Flt d0, d1, u, v, w;
   int i;
   NODE_PTR ntemp;
   Flt ftemp;
   Vec vtemp;
   Transform *tx;
   struct subst_struct subst;
   struct t_depth_light *tlight;

   /* Get the info for this light */
   tlight = (struct t_depth_light *)light->data;
   *rad = 0.0;
   VecCopy(tlight->light_from, L);

   /* First check to see if we are visible to the light */
   VecSub(L, W, D);
   d0 = sqrt(VecDot(D, D)); /* Distance to the point we are shading */
   tx = tlight->WS;
   u  = 0.0; v = 0.0;
   if (tx != NULL && tlight->light_depth != NULL) {
      w = W[0] * tx->matrix[0][3] +
          W[1] * tx->matrix[1][3] +
          W[2] * tx->matrix[2][3] +
                 tx->matrix[3][3];
      TxVec(S, W, tx);
      w = 1.0 / w;
      VecScale(w, S);
      u = S[0] / (tlight->light_depth->width - 1);
      v = S[1] / (tlight->light_depth->length - 1);
      if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0) {
         lookup_height(tlight->light_depth, u, 1-v, 0, &d1);
         if (tlight->light_depth->psize != 32)
            d1 += 129.0; /* Why 129 instead of 128?  Seems something broke so that
                            extra 1 is necessary to get the depth right. */
         if (d1 < d0 - tlight->light_bias) {
            /* Shadowed */
            MakeVector(0, 0, 0, C);
            return 0.0;
            }
         }
      }

   if (tlight->color == NULL)
      MakeVector(1, 1, 1, C)
   else {
      /* Evaluate the color function for this light */
      VecCopy(W, subst.P);
      MakeVector(0, 0, 0, subst.PT);
      MakeVector(u, v, 0, subst.U);
      VecCopy(W, subst.W);
      VecSub(L, W, subst.N);
      VecNormalize(subst.N);
      VecCopy(subst.N, subst.I);
      VecNegate(subst.I);
      i = eval_node(&subst, tlight->color, &ftemp, vtemp, &ntemp);
      if (i == 1)
         MakeVector(ftemp, ftemp, ftemp, C)
      else if (i == 2)
         VecCopy(vtemp, C)
      else
         error("Invalid lighting function (not vector or float) for light\n");
      }
   return 1.0;
}

Flt
Light_Color(Light *light, Vec W, Vec light_color, Vec light_pos, Flt *radius)
{
   int ltype;
   Flt intensity=0.0;

   /* Determine the intensity/color of light from this light source */
   ltype = light->type;
   if (ltype == T_POINT_LIGHT)
      intensity = Point_Light(light, W, light_color, light_pos, radius);
   else if (ltype == T_SPOT_LIGHT)
      intensity = Spot_Light(light, W, light_color, light_pos, radius);
   else if (ltype == T_DEPTH_LIGHT)
      intensity = Depth_Light(light, W, light_color, light_pos, radius);
   else if (ltype == T_TEXTURED_LIGHT)
      intensity = Textured_Light(light, W, light_color, light_pos, radius);
   else if (ltype == T_DIRECTIONAL_LIGHT)
      intensity = Directional_Light(light, W, light_color, light_pos, radius);
   else
      error("Bad light type: %d in Shade\n", ltype);

   return intensity;
}

/* Determine the colors from the direction of all the lights */
void
Get_Light_Colors(Viewpoint *Eye, Vec W, Vec *light_colors)
{
   Light *light;
   Ray tray;
   Flt t, tmin, intensity, radius;
   int j;
   Vec SV, light_pos, light_color;

   for (light=Lights,j=0;light!=NULL;light=light->next,j++) {
      VecCopy(W, tray.P);
      intensity = Light_Color(light, W, light_color, light_pos, &radius);
      if (!(light->flags & SHADOW_FLAG)) {
          light_colors[j][0] = light_color[0];
          light_colors[j][1] = light_color[1];
          light_colors[j][2] = light_color[2];
      } else if (ABS(intensity) < EPSILON) {
          MakeVector(0, 0, 0, light_colors[j]);
      } else {
          VecSub(light_pos, W, tray.D);
          t = VecNormalize(tray.D);
          tmin = ((Rendering_Method == SCAN_CONVERSION) ? 0.1 : rayeps);
          nShadows++;
          #ifdef DEBUG
          printf("\nnShadows in light=%ul\n",nShadows);
          printf("press enter\n");getchar();
          #endif
          if (Shadow(Eye, light, &tray, tmin, t, radius, SV)) {
             light_colors[j][0] = SV[0] * light_color[0];
             light_colors[j][1] = SV[1] * light_color[1];
             light_colors[j][2] = SV[2] * light_color[2];
             }
          else
             MakeVector(0, 0, 0, light_colors[j]);
      }
   }
}

void
Initialize_Lights()
{
   Lights = NULL;
   nLights = 0;
}

void
Deallocate_Lights()
{
   int j;
   Light *temp_light;
   struct t_textured_light *tlight;
   struct t_depth_light *tdlight;
   Img *img;

   while (Lights != NULL) {
      if (Lights->type == T_TEXTURED_LIGHT) {
         tlight = (struct t_textured_light *)Lights->data;
         deallocate_node(tlight->color);
         if (tlight->tx != NULL)
             polyray_free(tlight->tx);
         if (tlight->lens_flare != NULL) {
            deallocate_node(tlight->lens_flare->color);
            polyray_free(tlight->lens_flare);
            }
         if (tlight->alight != NULL) {
            PolyAlightData *alight = tlight->alight;
            polyray_free(alight->nbuf);
            polyray_free(alight->obuf);
            polyray_free(alight->sbuf1);
            polyray_free(alight->sbuf2);
            polyray_free(alight->points);
            polyray_free(alight);
            }
         }
      else if (Lights->type == T_DEPTH_LIGHT) {
         tdlight = (struct t_depth_light *)Lights->data;
         deallocate_node(tdlight->color);
         img = tdlight->light_depth;
         if (img->copy == 0) {
            /* Deallocate the z-buffer */
            polyray_free(img->filename);
            for (j=0;j<img->length;j++)
               polyray_free(img->image[j]);
            polyray_free(img->image);
            if (img->cmap != NULL)
               polyray_free(img->cmap);
            polyray_free(img);
            }
         if (tdlight->WS != NULL)
             polyray_free(tdlight->WS);
         }
      polyray_free(Lights->data);
      temp_light = Lights;
      Lights = Lights->next;
      polyray_free(temp_light);
      }
   nLights = 0;
}

/* Will add a lens flare to the current light (must be a textured light) */
void
Create_Lens_Flare()
{
   struct t_textured_light *tlight;
   FLARE *flare;

   tlight = (struct t_textured_light *)Current_Light->data;

   if (tlight->lens_flare != NULL) {
      deallocate_node(tlight->lens_flare->color);
      polyray_free(tlight->lens_flare);
      }

   /* Add this flare to the existing list of flares */
   flare = polyray_malloc(sizeof(FLARE));
   flare->color    = NULL; /* Which will become white */
   flare->count    = 10;
   flare->spacing  = 1.0;
   flare->seed     = 0;
   flare->min_rad  = 0.005;
   flare->max_rad  = 0.05;
   flare->concave  = 0.75;
   flare->radius   = 0.0;

   tlight->lens_flare = flare;
}

void
Set_Flare_Color(NODE_PTR color)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   if (flare->color != NULL)
      deallocate_node(flare->color);
   flare->color = color;
}

void
Set_Flare_Count(int count)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   if (count < 0) {
      warning("Must be a positive number of flares for a lens (reset to 1)");
      count = 1;
      }
   flare->count = count;
}

void
Set_Flare_Spacing(Flt spacing)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->spacing = spacing;
}

void
Set_Flare_Seed(int seed)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->seed = seed;
}

void
Set_Flare_Size(Flt min_rad, Flt max_rad)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->min_rad = min_rad;
   flare->max_rad = max_rad;
}

void
Set_Flare_Concave(Flt concave_ratio)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->concave = concave_ratio;
}

void
Set_Flare_Sphere(Flt radius)
{
   struct t_textured_light *tlight;
   FLARE *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->radius = radius;
}

static void
Draw_Lens_Flares(Viewpoint *eye, FLARE *flare, Vec C, Vec L)
{
   Flt opac, ftemp;
   NODE_PTR ntemp;
   Vec Cf, D, P;
   fVec S0, S1, C0, C1, P0, P1;
   float rad, w, x0, y0, dist;
   int color_flag, concave_flag, count;
   struct subst_struct subst, *sp;
   int i, j, k;
   int ylow, yhigh, xs, ys;

   /* These variables are only used for wireframe rendering */
   float x1, y1;
   Flt t, dt;
   int steps = 8;

   /* Transform the light location into screen space */
   VecSub(L, eye->view_from, D);
   dist = VecLen(D) * SGN(VecDot(D, ViewVec));
   if (dist <= 0)
      /* Light is behind the eye - no lens flares */
      return;

   /* sp is in case we need to evaluate colors on the fly. */
   sp = &subst;

   /* If the flare has a defined color then use it rather than the
      color of the light */
   if (flare->color != NULL) {
      i = eval_node(NULL, flare->color, &ftemp, D, &ntemp);
      if (i == 2) {
         VecCopy(D, C0)
         color_flag = 1; /* Solid color for flare */
         }
      else {
         color_flag = 2; /* Need to evaluate color for each pixel */
         reset_subst(sp);
         }
      }
   else {
      VecCopy(C, C0);
      color_flag = 0;
      }

   /* Project onto the screen */
   w = tx_point(eye->WS, L, S0);
   w = (w != 0.0 ? 1.0 / w : 1.0);
   VecScale(w, S0);

   /* Should be able to select flares that are on one side or the other
      of the screen center - the line below forces them to be on both sides. */
   xs = eye->view_xres / 2;
   ys = eye->view_yres / 2;
   S0[0] -= xs;
   S0[1] -= ys;
   MakeVector(-S0[0], -S0[1], S0[2], S1);
   /* MakeVector(0, 0, 0, S1); */

   /* Draw a line between the two points */
#if 0
   MakeVector(1, 1, 1, C0);
   MakeVector(1, 1, 1, C1);
   draw_2dline(eye, S0, C0, 0.0, S1, C1, 0.0);
#endif

   /* Reset the random number generator */
   srand(flare->seed);

   /* Copy the color into the variables we use for drawing */
   count = flare->count;
   if (flare->radius != 0) count++;
   for (i=0;i<count;i++) {
      if (flare->radius != 0 && i == 0) {
         dist = 1.0;
         concave_flag = (flare->radius > 0 ? 1 : 0);
         rad = fabs(flare->radius) * eye->view_xres;
      }
      else {
         dist = pow(((float)rand() / (float)RAND_MAX), flare->spacing);
         dist *= ((rand() > RAND_MAX/2) ? 1.0 : -1.0);
         concave_flag = (((float)rand() / (float)RAND_MAX) < flare->concave ? 1 : 0);
         rad = ((float)rand() / (float)RAND_MAX);
         rad *= (flare->max_rad - flare->min_rad);
         rad += flare->min_rad;
         rad *= eye->view_xres;
      }
      VecCopy(S0, P);
      VecScale(dist, P);

      /* Keep the flare drawing within the bounds of the current image
         window */
      ylow  = P[1] - rad + ys;
      yhigh = P[1] + rad + ys;
      if (yhigh < win.y0 || ylow > win.y1) {
         /* Flare isn't visible on this strip of the screen */
         continue;
         }
      else {
         ylow = MAX(ylow, win.y0);
         yhigh = MIN(yhigh, win.y1);
         }

      if (Rendering_Method == WIRE_FRAME ||
          Rendering_Method == HIDDEN_LINE) {
         /* Draw a circle at the radius of the lens flare */
         x0  = P[0] + rad + xs;
         y0  = P[1] + ys;
         MakeVector(1, 1, 1, C0);
         MakeVector(1, 1, 1, C1);
         for (j=0,t=dt=TWO_PI/(Flt)steps;j<steps;j++,t+=dt) {
            x1 = P[0] + rad * cos(t) + xs;
            y1 = P[1] + rad * sin(t) + ys;
            if ((y0 < ylow && y1 < ylow) || (y0 > yhigh && y1 > yhigh)) {
               x0 = x1;
               y0 = y1;
               continue;
               }
            MakeVector(x0, y0, 0, P0);
            MakeVector(x1, y1, 0, P1);
            draw_2dline(eye, P0, C0, 0.0, P1, C1, 0.0);
            x0 = x1;
            y0 = y1;
            }
         }
      else {
         /* Brute force - check every pixel in the box around the center
            of the flare */
         for (j=-rad;j<=rad;j++) {
            y0 = P[1] + j + ys;
            if (y0 < ylow || y0 > yhigh)
               continue;
            for (k=-rad;k<=rad;k++) {
               x0 = P[0] + k + xs;
               if (x0 < win.x0 || x0 > win.x1)
                  continue;
               dist = sqrt((Flt)k * (Flt)k + (Flt)j * (Flt)j) / rad;
               if (dist > 1.0) {
                  /* Need to handle partial coverage of a pixel here... */
                  continue;
                  }
               if (color_flag == 0 || color_flag == 1) {
                  opac = (concave_flag ? dist : 1.0 - dist);
                  VecCopy(C0, Cf)
                  }
               else {
                  subst.U[0] = (concave_flag ? dist : 1.0 - dist);
                  subst.U[1] = atan2((double)k, (double)j);
                  subst.U[2] = i;
                  subst.P[0] = concave_flag;
                  if (eval_node(sp, flare->color, &opac, Cf, &ntemp) != 2) {
                     error("Bad color expression in lens flare");
                     }
                  }

               /* Now draw the colored point */
               draw_point(eye, x0, y0, 0.0, Cf, opac);
               }
            }
         }
      }
}

void
Draw_Flares(Viewpoint *eye)
{
   Light *light;
   struct t_textured_light *tlight;
   Flt intensity, radius;
   Vec W, C, L;

   /* Step through each light and see if it has any associated flares */
   for (light=Lights;light!=NULL;light=light->next) {
      if (light->type == T_TEXTURED_LIGHT) {
         tlight = (struct t_textured_light *)light->data;
         if (tlight->lens_flare != NULL) {
            /* First see how the light projects onto the screen */
            MakeVector(0, 0, 0, W);
            intensity = Textured_Light(light, W, C, L, &radius);
            if ((Optimizer > 0) &&
                ((Rendering_Method == RAY_TRACING) ||
                 ((Rendering_Method == GOURAD_SHADE ||
                   Rendering_Method == SCAN_CONVERSION) &&
                  (Global_Shade_Flag &
                   (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK)))))
               if (!Check_Visibility(eye->view_from, L))
                  continue;
             VecScale(intensity, C);
             Draw_Lens_Flares(eye, tlight->lens_flare, C, L);
            }
         }
      }
}

/***********************************************************************
 * Shadow(ray, hit, tmax)
 *
 * Returns true if we are unshadowed/partially shadowed.  Returns the
 * primitive in the "hit" buffer.
 *
 * Note: the return value of this procedure is a bit strange, as well
 * as the name.  Should probably be changed.
 ***********************************************************************/
static float hex_circle[19][2] =
   { 0.750, 0.433, 0.000, 0.866,-0.750, 0.433,-0.750,-0.433, 0.000,
    -0.866, 0.750,-0.433, 0.000, 0.000, 0.750, 0.000, 0.375, 0.650,
    -0.375, 0.650,-0.750, 0.000,-0.375,-0.650, 0.375,-0.650, 0.375,
     0.216, 0.000, 0.433,-0.375, 0.217,-0.375,-0.216, 0.000,-0.433,
     0.375,-0.271};
#define SHADOW_RAY_JITTER 0.2 /* 0.075 */

static void
jittered_hex_ray(int index, Vec L, Flt radius, Flt distance, Vec D)
{
   float t0, t1, t2, t3, t4;
   float deltax, deltay, mu;
   Vec tD;

   deltax = SHADOW_RAY_JITTER * (0.5 - polyray_random());
   deltay = SHADOW_RAY_JITTER * (0.5 - polyray_random());
   MakeVector(radius*(hex_circle[index][0] + deltax),
              radius*(hex_circle[index][1] + deltay),
              distance, tD);
   t0 = L[0];
   t1 = L[1];
   t2 = L[2];
   mu = sqrt(t1 * t1 + t2 * t2);
   t3 = -t0 * tD[0] / mu;
   t4 = tD[1] / mu;
   MakeVector(mu * tD[0] + t0 * tD[2],
              t1 * t3 + t2 * t4 + t1 * tD[2],
              t2 * t3 - t1 * t4 + t2 * tD[2],
              D);
   VecNormalize(D);
}

int
Shadow(Viewpoint *eye, Light *light, Ray *ray,
       Flt tmin, Flt tmax,
       Flt radius, Vec SV)
{
   Ray jray;
   Isect thit;
   Flt t, dmax, light_left, fscale;
   Flt theta, lradius, ldistance;
   Surface *surf;
   int i, r, rmax, unblocked, cnt = 0;
   int old_test = Shadow_Test;
   Vec Filt[19], fcolor;
   Object *cobj;
   extern int recursion_depth;

   if (!(Global_Shade_Flag & SHADOW_CHECK)) {
      MakeVector(1.0, 1.0, 1.0, SV);
      return 1;
      }

   Shadow_Test = 1;

   /* If this is a polygonal area light, then we jump to the routine
      specifically built to deal with it. */
   if (light != NULL && light->type == T_TEXTURED_LIGHT) {
      struct t_textured_light *tlight;
      PolyAlightData *alight;

      tlight = (struct t_textured_light *)light->data;
      alight = tlight->alight;
      if (alight != NULL) {
         i = PolygonLight(eye, light, alight, tmin, ray->P, SV);
         Shadow_Test = old_test;
         return i;
         }
      }

   if (radius > 0.0) {
      /* Spherical light source, up to 19 rays to test blocking */
      rmax = 19;
      theta = asin(radius / tmax);
      lradius = radius * cos(theta);
      ldistance = tmax - lradius * sin(theta);
      }
   else
      /* Point light source, only one ray to test shadowing */
      rmax = 1;

   for (r=0,unblocked=rmax;r<rmax;r++) {
      /* Up to 19 samples of the light source */
      light_left = 1.0;
      VecCopy(ray->P, jray.P);
      dmax = tmax;
      cnt = 0;

      if (rmax > 1)
         jittered_hex_ray(r, ray->D, lradius, ldistance, jray.D);
      else
         VecCopy(ray->D, jray.D);

      /* Since there may be several semi-transparent occluding
         surfaces between the initial point and the light, we
         loop until we either are completely blocked or we get
         very close to the light. */
      MakeVector(1.0, 1.0, 1.0, Filt[r]);
      while (light_left > SMALL && tmax > SMALL && cnt < 10) {
         if (light != NULL && cnt == 0) {
             totalShadows++;
             cobj = Get_Light_Blocker(light, recursion_depth);
             thit.flag = 0;
             if (cobj != NULL &&
                 find_object_intersections(eye, cobj, &jray, tmin,
                                           dmax, &thit)) {
               if ((Global_Shade_Flag & TRANSMIT_CHECK) &&
                   (thit.obj->o_sflag & TRANSMIT_CHECK)) {
                  t = thit.isect_t;
                  VecNormalize(thit.N);
                  surf = find_surface(eye, thit.obj, thit.texture, thit.W,
                                      thit.N, jray.D, thit.U, 7);
                  if (surf->Kt_scale == 0.0 ||
                      (surf->Kt_color[0] == 0 &&
                       surf->Kt_color[1] == 0 &&
                       surf->Kt_color[2] == 0)) {
                     /* Completely blocked */
                     totalShadowCaches++;
                     light_left = 0.0;
                     break;
                     }
                  }
               else {
                  totalShadowCaches++;
                  light_left = 0.0;
                  break;
                  }
               }
             }

          if (Intersect(eye, &Root, &jray, tmin, dmax, &thit)) {
            /* See if there is any color left after this hit */
            if (thit.obj->o_type == T_HYPERTEXTURE) {
               /* Special case, the color and opacity have already
                  been calculated for us.  */
               fscale = thit.U[0];
               MakeVector(1, 1, 1, fcolor)
               t = thit.U[1];
               }
            else if ((Global_Shade_Flag & TRANSMIT_CHECK) &&
                (thit.obj->o_sflag & TRANSMIT_CHECK)) {
               t = thit.isect_t;
               VecNormalize(thit.N);
               surf = find_surface(eye, thit.obj, thit.texture, thit.W,
                                   thit.N, jray.D, thit.U, 7);
               if (surf->Kt_scale == 0.0 ||
                   (surf->Kt_color[0] == 0 &&
                    surf->Kt_color[1] == 0 &&
                    surf->Kt_color[2] == 0)) {
                  /* Completely blocked */
                  if (cnt == 0 && light != NULL)
                     Set_Light_Blocker(light, recursion_depth, thit.obj);
                  light_left = 0.0;
                  break;
                  }
               VecCopy(surf->Kt_color, fcolor)
               fscale = surf->Kt_scale;
               }
            else {
               /* If this is the first hit in the loop then we can
                  cache this object as the blocker of the light */
               if (cnt == 0 && light != NULL)
                  Set_Light_Blocker(light, recursion_depth, thit.obj);
               light_left = 0.0;
               break;
               }

            /* Reduce the amount of light by the filter color associated
               with the thing it hit */
            for (i=0;i<3;i++)
               Filt[r][i] *= fscale * fcolor[i];
            light_left = VecDot(Filt[r], Filt[r]);

            /* Move up a little closer to the light */
            VecCopy(thit.W, jray.P);
            dmax = dmax - t;
            cnt++;
            }
         else {
            if (cnt == 0 && light != NULL)
               Set_Light_Blocker(light, recursion_depth, NULL);
            break;
            }
         }

      /* See if there is an early out based on complete occlusion by
         the light source */
      if (light_left < SMALL)
         MakeVector(0.0, 0.0, 0.0, Filt[r])
      if (Filt[r][0] < 1.0 || Filt[r][1] < 1.0 || Filt[r][2] < 1.0)
         unblocked--;
      if (r == 6) {
         if (unblocked == 12) {
            unblocked = 0;
            break;
            }
         else if (unblocked == 19)
            break;
         }
      }

   /* Compute the actual filter amount based on the number of
      samples of the light we took. */
   if (rmax == 1)
      VecCopy(Filt[0], SV)
   else {
      MakeVector(0.0, 0.0, 0.0, SV);
      for (i=0;i<r;i++)
         VecAdd(Filt[i], SV, SV);
      t = 1.0 / (Flt)r; /* (Flt)unblocked / ((Flt)rmax * r); */
      VecScale(t, SV);
      }
   Shadow_Test = old_test;
   if (SV[0] == 0.0 && SV[1] == 0.0 && SV[2] == 0.0)
      return 0;
   else
      return 1;
}

