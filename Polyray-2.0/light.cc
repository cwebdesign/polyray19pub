/* light.cc

  Compute color and intensity for a variety of light types

  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", (C), WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <cstdint>
#include <utility>

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "vector.h"
#include "light.h"
#include "alight.h"
#include "scan.h"
#include "image.h"
#include "builder.h"
#include "eval.h"
#include "runtime_state.h"
#include "symtab.h"
#include "intersec.h"
#include "shade.h"
#include "factory.h"

Light *Current_Light = nullptr;

// Define the distinct types of lights 
enum class LightType : std::uint8_t {
    T_POINT_LIGHT       = 150,
    T_SPOT_LIGHT        = 151,
    T_TEXTURED_LIGHT    = 152,
    T_DIRECTIONAL_LIGHT = 153,
    T_DEPTH_LIGHT       = 154
};


constexpr int SHADOW_FLAG=0x01;

/* Prototypes for the primitive operators */
void LightRender(Viewpoint *, BinTree *, Object *);
int LightIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int LightInside(Object *, Vec);
void LightCopy(Object *, Object *);


openpolyray::dispatch::ObjectProcs LightProcs = {
   .render = LightRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = LightIntersect,
   .inside = LightInside,
   .copy = LightCopy,
   .del = LightDelete,
   };

/**
 * @brief Object destructor proc for a light — frees per-type sub-data then the Light shell.
 * No-op for copies (@c o_copy != 0) because they share the Light* with the original.
 * @param object  The light Object whose @c o_data holds the Light struct.
 */
void LightDelete(Object* object)
{
    struct t_depth_light* tlight3;
    struct t_textured_light* tlight4;
    Light* light = (Light*)object->o_data;

    if (object->o_copy == 0) {
        /* Delete the light information */
        switch (static_cast<LightType>(light->type)) {
        case LightType::T_POINT_LIGHT:
        case LightType::T_SPOT_LIGHT:
        case LightType::T_DIRECTIONAL_LIGHT:
            break;
        case LightType::T_TEXTURED_LIGHT:
            tlight4 = ((struct t_textured_light*)light->data);
            if (tlight4->tx != nullptr)
                delete tlight4->tx;
            deallocate_node(tlight4->color);
            if (tlight4->lens_flare != nullptr) {
                deallocate_node(tlight4->lens_flare->color);
                polyray_free(tlight4->lens_flare);
            }
            if (tlight4->alight != nullptr) {
                PolyAlightData* alight = tlight4->alight;
                polyray_free(alight->points);
                delete alight;
            }
            break;
        case LightType::T_DEPTH_LIGHT:
            tlight3 = ((struct t_depth_light*)light->data);
            delete tlight3->WS;
            break;
        default:
            serror("Bad light type in Delete_Light");
        }
        delete light->data;
        delete light;
    }
}


/** @brief Ray-intersection stub — lights have no geometry; always returns 0. */
int LightIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
               Flt mindist, Flt maxdist, Isect *hit)
{
   return 0;
}

/** @brief CSG inside-test stub — always returns 0; lights are not CSG primitives. */
int LightInside(Object *obj, Vec P)
{
   return 0;
}

/**
 * @brief Object copy proc for a light — deep-copies the Light struct into @p objout.
 * Sets @c o_copy = 0 on the output so LightDelete will run full cleanup for the copy.
 */
void LightCopy(Object *objin, Object *objout)
{
   objout->o_data = Copy_Light((Light *)objin->o_data, NULL);
   objout->o_copy = 0;
}


/** @brief Scan-conversion render stub — lights produce no rasterised geometry. */
void LightRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   ;
}

/**
 * @brief Wires a parsed @p light into a scene Object, setting type and procs.
 * @param object  Pre-allocated Object shell to populate.
 * @param light   Light struct to attach as @c o_data.
 * @return The populated @p object pointer.
 */
Object *MakeLight(Object *object, Light *light)
{
   object->o_type  = ShapeType::Light;
   object->o_procs = &LightProcs;
   object->o_data  = light;
   return object;
}

/** @brief Zeroes the per-depth shadow-cache pointers for every light in the scene list. */
void Initialize_Light_Caches(void)
{
   #ifdef DEBUG_FN_CALLS
	std::cout<<"Initialize_Light_Caches\n";
   #endif
   int j;
   Light *light;

   for (light= RuntimeState::Lights;light!=nullptr;light=light->next) {
      for (j=0;j<MAX_CACHE_BLOCKING;++j)
         light->cache[j] = nullptr;
      }
}

/**
 * @brief Returns the cached shadow-blocker Object at recursion @p depth for @p light.
 * @return nullptr if @p depth >= MAX_CACHE_BLOCKING.
 */
Object *Get_Light_Blocker(Light *light, int depth)
{
   if (depth >= MAX_CACHE_BLOCKING)
      return nullptr;
   else
      return light->cache[depth];
}

/**
 * @brief Stores @p obj as the shadow-blocker cache entry at recursion @p depth for @p light.
 * No-op if @p depth >= MAX_CACHE_BLOCKING.
 */
void Set_Light_Blocker(Light *light, int depth, Object *obj)
{
   if (depth >= MAX_CACHE_BLOCKING)
      return;
   else
      light->cache[depth] = obj;
}

/** @brief No-op stub — reserved for future shadow-cache cleanup. */
void Terminate_Light_Caches(void)
{
   ;
}

/**
 * @brief Returns the world-space position of @p light, dispatching on light type.
 * For directional lights returns a point at PLY_HUGE distance along the light direction.
 * For polygon area lights returns the centroid of the polygon bounding box.
 * @return NuVec world-space position.
 */
NuVec Get_Light_Pos(Light *light)
{
   struct t_point_light *tlight1;
   struct t_spot_light *tlight2;
   struct t_depth_light *tlight3;
   struct t_textured_light *tlight4;
   
   NuVec retPos;

   LightType ltype = static_cast<LightType>(light->type);
   if (ltype == LightType::T_POINT_LIGHT) {
      tlight1 = (struct t_point_light *)light->data;
      VecCopy(tlight1->light_pos, retPos);
      }
   else if (ltype == LightType::T_SPOT_LIGHT) {
      tlight2 = (struct t_spot_light *)light->data;
      VecCopy(tlight2->light_pos, retPos);
      }
   else if (ltype == LightType::T_DEPTH_LIGHT) {
      tlight3 = (struct t_depth_light *)light->data;
      VecCopy(tlight3->light_from, retPos);
      }
   else if (ltype == LightType::T_TEXTURED_LIGHT) {
      tlight4 = (struct t_textured_light *)light->data;
      if (tlight4->alight != nullptr) {
         PolyAlightData *alight = tlight4->alight;

         VecAdd(alight->upper_right, alight->lower_left, retPos)
         VecScale(0.5, retPos)
         }
      else if (tlight4->tx == nullptr)
         MakeVector(0, 0, 0, retPos)
      else
         MakeVector(tlight4->tx->matrix[3][0],
                    tlight4->tx->matrix[3][1],
                    tlight4->tx->matrix[3][2],
                    retPos)
      }
   else if (ltype == LightType::T_DIRECTIONAL_LIGHT) {
      tlight1 = (struct t_point_light *)light->data;
      MakeVector(0, 0, 0, retPos);
      VecAddScaled(retPos, PLY_HUGE, tlight1->light_pos, retPos);
      }
   else
      serror("Bad light type: %d in Get_Light_Pos\n", ltype);
   return retPos;
}

/**
 * @brief Determines the position and color of a point light; intensity is always 1.
 * @param light  The light (must be T_POINT_LIGHT).
 * @param W      World-space point being shaded (unused for point lights).
 * @param C      Out: light color.
 * @param L      Out: light position.
 * @param rad    Out: light radius (set to 0).
 * @return 1.0 always.
 */
static Flt Point_Light(Light *light, Vec W, Vec C, Vec L, Flt *rad)
{
   struct t_point_light *tlight;

   tlight = (struct t_point_light *)light->data;
   VecCopy(tlight->light_color, C);
   VecCopy(tlight->light_pos, L);
   *rad = 0.0;
   return 1.0;
}

/** @brief Sets the shadow flag on the parser-active light (@c Current_Light). */
void Set_Light_Shadow(int shadow_flag)
{
   Current_Light->flags = shadow_flag;
}

/**
 * @brief Appends @p light to the front of RuntimeState::Lights.
 * For polygon area lights, transforms all polygon points and basis vectors
 * through the light's transform before insertion.
 */
void Add_To_Lights(Light *light)
{
   int j;

   /* If we have an area light, then we transform the points that
      define it's polygon, as well as the basis vectors. */
   if (light->type == std::to_underlying(LightType::T_TEXTURED_LIGHT)) { //todo:T_TEXTURED_LIGHT to something T_AREA_TEXTURED_LIGHT


      struct t_textured_light* tlight = (struct t_textured_light *)light->data;
      PolyAlightData* alight = tlight->alight;

      if (alight != nullptr) {
         for (int i=0;i<alight->npoints;i++) {
            fTxVec(alight->points[i], alight->points[i], tlight->tx);
            }
         TxDirection(alight->ubasis, alight->ubasis, tlight->tx);
         TxDirection(alight->vbasis, alight->vbasis, tlight->tx);
         TxVec(alight->lower_left, alight->lower_left, tlight->tx);
         TxVec(alight->upper_right, alight->upper_right, tlight->tx);
         }
      }

   for (j=0;j<MAX_CACHE_BLOCKING;j++)
      light->cache[j] = nullptr;
   light->next = RuntimeState::Lights;
   RuntimeState::Lights = light;
   RuntimeState::nLights++;
}

/**
 * @brief Deep-copies @p light, optionally composing @p tx into the copy's transform.
 * All per-type sub-structs are duplicated; color expression nodes are cloned via copy_node().
 * @param light  Source light to copy.
 * @param tx     Additional transform to compose into the copy (may be nullptr).
 * @return Newly allocated Light copy.
 */
Light *Copy_Light(Light *light, Transform *tx)
{
   //Light *new_light;
   void  *data=nullptr;
   struct t_point_light    *tlight1;
   struct t_spot_light     *tlight2;
   struct t_depth_light    *tlight3, *dlight;
   struct t_textured_light *tlight4;
   Viewpoint eye;

   Light *new_light = FactoryObjectLight();
   if (new_light == nullptr)
      serror("Failed to allocate light");

   switch (static_cast<LightType>(light->type)) {
   case LightType::T_POINT_LIGHT:
      data = tlight1 = FactoryObjectPointLight();
      if (tlight1 == nullptr)
         serror("Failed to allocate point light");
      //memcpy(tlight1, light->data, sizeof(struct t_point_light));
      // Explicitly cast the void* to the correct struct pointer, then dereference and copy
      *tlight1 = *static_cast<const t_point_light*>(light->data);
      TxVector(tlight1->light_pos,
               ((struct t_point_light *)light->data)->light_pos,
               tx)
      break;
   case LightType::T_SPOT_LIGHT:
       data = tlight2 = FactoryObjectSpotLight();
       if (tlight2 == nullptr)
         serror("Failed to allocate spotlight");
      memcpy(tlight2, light->data, sizeof(struct t_spot_light));
      TxVector(tlight2->light_pos,
               ((struct t_spot_light *)light->data)->light_pos,
               tx)
      TxDirection(tlight2->light_dir,
                  ((struct t_spot_light *)light->data)->light_dir,
                  tx)
      (void)VecNormalize(tlight2->light_dir);
      break;
   case LightType::T_TEXTURED_LIGHT:
       data = tlight4 = FactoryObjectTexturedLight();
      if (tlight4 == nullptr)
         serror("Failed to allocate textured light");
      memcpy(tlight4, light->data, sizeof(struct t_textured_light));
      if (tlight4->tx != nullptr) {
         tlight4->tx = Get_Transformation().release();
         Compose_Transformations(* tlight4->tx, *((struct t_textured_light *)light->data)->tx);
         if (tx != nullptr)
            Compose_Transformations(* tlight4->tx, *tx);
         }
      else if (tx != nullptr) {
         tlight4->tx = Get_Transformation().release();
         Compose_Transformations(* tlight4->tx, *tx);
         }
      if (tlight4->lens_flare != nullptr) {
         FLARECOMP *tflare;
         tflare = FactoryObjectFLARECOMPLight();
         memcpy(tflare, tlight4->lens_flare, sizeof(FLARECOMP));
         tflare->color = copy_node(tlight4->lens_flare->color);
         tlight4->lens_flare = tflare;
         }
      if (tlight4->alight != nullptr) {
         PolyAlightData *alight;
         alight = FactoryObjectPolyAlightDataLight();
         memcpy(alight, tlight4->alight, sizeof(PolyAlightData));
         alight->nbuf = {};
         alight->nbuf.resize(alight->vres + 1);
         alight->obuf = alight->nbuf;
         alight->sbuf1 = alight->nbuf;
         alight->sbuf2 = alight->nbuf;

         alight->points = (fVec *)polyray_malloc(alight->npoints * sizeof(fVec));
         memcpy(alight->points, tlight4->alight->points, alight->npoints * sizeof(fVec));
         tlight4->alight = alight;
         }
      break;
   case LightType::T_DIRECTIONAL_LIGHT:
      data = tlight1 = FactoryObjectPointLight();
      if (tlight1 == nullptr)
         serror("Failed to allocate light");
      memcpy(tlight1, light->data, sizeof(struct t_point_light));
      TxDirection(tlight1->light_pos,
                  ((struct t_spot_light *)light->data)->light_pos,
                  tx)
      break;
   case LightType::T_DEPTH_LIGHT:
      data = tlight3 = FactoryObjectDepthLight();
      if (tlight3 == nullptr)
         serror("Failed to allocate light");
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
      tlight3->WS = Normalize_View(&eye).release();
      break;
   default:
      serror("Bad light type in Copy_Light");
   }

   if (new_light ==  nullptr)
      serror("Failed to allocate light");
   new_light->type  = light->type;
   new_light->flags = light->flags;
   new_light->data  = data;
   new_light->next  = nullptr;

   return new_light;
}

/**
 * @brief Parser action: creates a point light with explicit @p color and @p pos.
 * @return New Light* (polyray_malloc); sets Current_Light.
 */
Light *
light_action1(Vec color, Vec pos)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectPointLight();
   if (new_light == nullptr || data == nullptr)
      serror("Failed to allocate light");
   new_light->type = std::to_underlying(LightType::T_POINT_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   VecCopy(color, data->light_color);
   VecCopy(pos, data->light_pos);
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Parser action: creates a white point light at @p pos.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *light_action2(Vec pos)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectPointLight();
   MakeVector(1.0, 1.0, 1.0, data->light_color);
   VecCopy(pos, data->light_pos);
   new_light->type = std::to_underlying(LightType::T_POINT_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Creates a spot light from position, aim point, and cone parameters.
 * @param color    RGB light color.
 * @param pos      World-space light position.
 * @param at       Look-at point; the cone axis is (at − pos), normalised.
 * @param coef     Cosine-power falloff exponent.
 * @param radius   Half-angle (degrees) of the full-intensity cone.
 * @param falloff  Half-angle (degrees) of the zero-intensity cone edge.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *SetSpotLight(Vec color, Vec pos, Vec at, Flt coef, Flt radius, Flt falloff)
{
   Light *new_light;
   struct t_spot_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectSpotLight();
   VecCopy(color, data->light_color);
   VecCopy(pos, data->light_pos);
   VecSub(at, pos, data->light_dir);
   (void)VecNormalize(data->light_dir);
   data->Coef    = coef;
   data->Radius  = cos(radius * PYM_PI / 180.0);
   data->Falloff = cos(falloff * PYM_PI / 180.0);
   new_light->type = std::to_underlying(LightType::T_SPOT_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Parser action: creates a textured (procedural-color) light with default white color.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *
light_action3(void)
{
   Light *new_light;
   struct t_textured_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectTexturedLight();
   MakeVector(1, 1, 1, data->dcolor); /* Default color is white */
   data->color      = nullptr;
   data->radius     = 0.0;
   data->tx         = nullptr;
   data->lens_flare = nullptr;
   data->alight     = nullptr;

   new_light->type  = std::to_underlying(LightType::T_TEXTURED_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data  = (void *)data;
   new_light->next  = nullptr;
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Parser action: creates a white directional (infinite-distance) light pointing along @p dir.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *
light_action4(Vec dir)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectPointLight();
   MakeVector(1.0, 1.0, 1.0, data->light_color);
   VecCopy(dir, data->light_pos);
   new_light->type = std::to_underlying(LightType::T_DIRECTIONAL_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Parser action: creates a directional light with explicit @p color and @p dir.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *light_action5(Vec color, Vec dir)
{
   Light *new_light;
   struct t_point_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectPointLight();
   VecCopy(color, data->light_color);
   VecCopy(dir, data->light_pos);
   new_light->type = std::to_underlying(LightType::T_DIRECTIONAL_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   Current_Light = new_light;
   return new_light;
}

/**
 * @brief Parser action: creates a depth-mapped shadow light with default parameters.
 * @return New Light* (Factory); sets Current_Light.
 */
Light *light_action6(void)
{
   Light *new_light;
   struct t_depth_light *data;

   new_light = FactoryObjectLight();
   data = FactoryObjectDepthLight();
   data->color = nullptr;
   data->light_depth = nullptr;
   MakeVector(0, 50, 0, data->light_from);
   MakeVector(0, 0, 0, data->light_at);
   MakeVector(0, 0, 1, data->light_up);
   data->light_aspect = 1.0;
   data->light_angle  = degtorad(22.5);
   data->light_bias = RuntimeState::rayeps;
   data->WS = nullptr;
   new_light->type = std::to_underlying(LightType::T_DEPTH_LIGHT);
   new_light->flags = SHADOW_FLAG;
   new_light->data = (void *)data;
   new_light->next = nullptr;
   Current_Light = new_light;
   return new_light;
}

/** @brief Sets the half-angle (degrees) of the current depth light's projection cone. */
void DepthLight1(Flt ang)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_angle = degtorad(ang/2.0);
}

/** @brief Sets the aspect ratio of the current depth light's view frustum. */
void
DepthLight2(Flt asp)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_aspect = asp;
}

/** @brief Sets the look-at point of the current depth light. */
void
DepthLight3(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_at);
}

/** @brief Attaches a run-time color expression to the current depth light. */
void
DepthLight4(NODE_PTR exper)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->color = exper;
}

/** @brief Loads the shadow-depth image from @p filename for the current depth light. */
void DepthLight5(char *filename)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   auto ret = ReadImage(filename);
   if (ret.has_value())
     tlight->light_depth = ret.value();
   else
     serror("%s",("DepthLight5: "+ret.error()).c_str());
}

/** @brief Sets the eye (from) point of the current depth light. */
void
DepthLight6(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_from);
}

/** @brief Sets the up-vector of the current depth light. */
void
DepthLight7(Vec v)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   VecCopy(v, tlight->light_up);
}

/** @brief Finalises the depth light's view-space transform from its current from/at/up/angle settings. */
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
   tlight->WS = Normalize_View(&eye).release();
}

/** @brief Sets the shadow-depth bias of the current depth light. */
void
DepthLight9(Flt d)
{
   struct t_depth_light *tlight;

   tlight = (struct t_depth_light *)Current_Light->data;
   tlight->light_bias = d;
}

/**
 * @brief Sets the run-time color expression on the current textured light.
 * If @p exper evaluates to a constant, stores the result in @c dcolor and frees the node.
 */
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

/**
 * @brief Sets the area-light sphere radius on the current textured light.
 * Frees any existing polygon area-light data if present.
 */
void
Set_Light_Radius(Flt rad)
{
   struct t_textured_light *tlight;

   tlight = (struct t_textured_light *)Current_Light->data;
   tlight->radius = rad;

   if (tlight->alight != nullptr) {
      PolyAlightData *alight = tlight->alight;
      delete (alight->points);
      delete (alight);
      }
}

/**
 * @brief Configures the current textured light as a rectangle polygon area light.
 * Allocates PolyAlightData with a default 1×1 unit quad and shadow sample buffers.
 * @param ures            Number of sample columns.
 * @param vres            Number of sample rows.
 * @param adaptive_depth  Adaptive subdivision depth for shadow sampling.
 * @param jitter          Amount of jitter applied to shadow sample positions.
 */
void Set_Light_Polygon(Flt ures, Flt vres, Flt adaptive_depth, Flt jitter)
{
   struct t_textured_light *tlight;
   PolyAlightData *alight;
   int i;

   tlight = (struct t_textured_light *)Current_Light->data;

   if (tlight->alight != nullptr) {
      PolyAlightData *alight = tlight->alight;
      delete (alight->points);
      delete (alight);
      }
   tlight->radius = 0.0;
   alight = FactoryObjectPolyAlightDataLight();
    tlight->alight = alight;
   alight->ures   = MAX(1, (int)ures);
   alight->vres   = MAX(1, (int)vres);
   alight->adaptive_depth = MAX(0, (int)adaptive_depth);
   alight->jitter  = jitter;
   MakeVector(0, 0, 0, alight->lower_left)
       MakeVector(1, 0, 1, alight->upper_right)
       MakeVector(1, 0, 0, alight->ubasis)
       MakeVector(0, 0, 1, alight->vbasis)
       alight->nbuf = {};
   alight->nbuf.resize(alight->vres + 1);
   alight->obuf = alight->nbuf;
   alight->sbuf1 = alight->nbuf;
   alight->sbuf2 = alight->nbuf;
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

/** @brief Composes transform @p t into the current textured light's accumulated transform. */
void Transform_Light(Transform *t)
{
   struct t_textured_light *tlight;

   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, *t);
}

/**
 * @brief Composes a shear transform into the current textured light's transform.
 * @param xy,xz,yx,yz,zx,zy  Off-diagonal shear coefficients.
 */
void Shear_Light(Flt xy, Flt xz, Flt yx, Flt yz, Flt zx, Flt zy)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Shear_Transformation(&trans, xy, xz, yx, yz, zx, zy);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) 
      tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, trans);
}

/** @brief Composes a translation by @p v into the current textured light's transform. */
void Translate_Light(const NuVec v)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Translation_TransformationCPP(trans, v);

   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) 
      tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, trans);
}

/** @brief Composes an Euler-angle rotation (degrees, X-Y-Z) into the current textured light's transform. */
void Rotate_Light( NuVec vt)
{
   Transform trans;
   struct t_textured_light *tlight;
   VecScale(PYM_PI/180.0, vt);
   Get_Rotation_Transformation_CPP(trans, vt);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) 
      tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, trans);
}

/**
 * @brief Composes an axis-angle rotation into the current textured light's transform.
 * @param v    Rotation axis (need not be normalised).
 * @param ang  Rotation angle in degrees.
 */
void Rotate_Axis_Light(Vec v, Flt ang)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Rotate_Transform(&trans, v, PYM_PI * ang / 180.0);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) 
      tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, trans);
}

/** @brief Composes a non-uniform scale by @p v into the current textured light's transform. */
void
Scale_Light(Vec v)
{
   Transform trans;
   struct t_textured_light *tlight;

   Get_Scaling_Transformation (&trans, v);
   tlight = (struct t_textured_light *)Current_Light->data;
   if (tlight->tx == nullptr) 
      tlight->tx = Get_Transformation().release();
   Compose_Transformations(* tlight->tx, trans);
}

/**
 * @brief Determines the position and color of a directional light; intensity is always 1.
 * @param light  The light (must be T_DIRECTIONAL_LIGHT).
 * @param W      World-space point being shaded; L is set to W + PLY_HUGE * direction.
 * @param C      Out: light color.
 * @param L      Out: effective light position (point at infinity along light direction).
 * @param rad    Out: light radius (set to 0).
 * @return 1.0 always.
 */
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


/**
 * @brief Smooth cubic Hermite interpolation from 0 at @p low to 1 at @p high, evaluated at @p x.
 * Assumes high >= low. Returns 0 if x < low, 1 if x > high.
 */
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

/**
 * @brief Determines the intensity and color of a spot light at @p W.
 * Applies cosine-power falloff and a cubic-spline inner/outer cone transition.
 * @param light  The light (must be T_SPOT_LIGHT).
 * @param W      World-space point being shaded.
 * @param C      Out: light color.
 * @param L      Out: light position.
 * @param rad    Out: light radius (set to 0).
 * @return Attenuated intensity in [0, 1]; 0 if @p W is outside the cone.
 */
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
   if (len < PLY_EPSILON) return 1;
   D[0] /= len; D[1] /= len; D[2] /= len;
   costheta = -VecDot(tlight->light_dir, D);
   if (costheta <= 0.0) return 0.0;
   atten = pow(costheta, tlight->Coef);
   if (tlight->Radius > 0.0)
      atten *= cubic_spline(tlight->Falloff, tlight->Radius, costheta);
   *rad = 0.0;
   return atten;
}

/**
 * @brief Determines the position and color of a textured (procedural-color) light at @p W.
 * Evaluates the run-time color expression in the light's local coordinate frame.
 * @param light  The light (must be T_TEXTURED_LIGHT).
 * @param W      World-space point being shaded.
 * @param C      Out: evaluated light color.
 * @param L      Out: effective light position (centroid for area lights; matrix translation for others).
 * @param rad    Out: light radius from tlight->radius.
 * @return 1.0 always.
 */
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
         serror("Invalid lighting function (not vector or float) for light\n");
      }

   *rad = tlight->radius;
   return 1.0;
}

/**
 * @brief Determines the color of a depth-mapped shadow light at @p W.
 * Projects @p W through the light's view-space transform and compares against
 * the stored depth map to return 0 (shadowed) or 1 (lit).
 * @param light  The light (must be T_DEPTH_LIGHT).
 * @param W      World-space point being shaded.
 * @param C      Out: light color (black if shadowed, otherwise from color expression).
 * @param L      Out: light from-point.
 * @param rad    Out: light radius (set to 0).
 * @return 0.0 if the point is in shadow; 1.0 otherwise.
 */
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
         if (tlight->light_depth->bytes_per_pixel != 32)
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
         serror("Invalid lighting function (not vector or float) for light\n");
      }
   return 1.0;
}

/**
 * @brief Dispatches to the per-type light evaluator to get intensity, color, and position.
 * @param light        Light to evaluate.
 * @param W            World-space point being shaded.
 * @param light_color  Out: RGB color of the light.
 * @param light_pos    Out: world-space light position.
 * @param radius       Out: light radius (0 for point/directional lights).
 * @return Intensity scalar (0 = fully blocked or off-axis for spot).
 */
Flt
Light_Color(Light *light, Vec W, Vec light_color, Vec light_pos, Flt *radius)
{
   #ifdef DEBUG_FN_CALLS
   smessage("light::Light_Color\n");
   #endif
   Flt intensity=0.0;

   /* Determine the intensity/color of light from this light source */
   LightType ltype = static_cast<LightType>(light->type);
   if (ltype == LightType::T_POINT_LIGHT)
      intensity = Point_Light(light, W, light_color, light_pos, radius);
   else if (ltype == LightType::T_SPOT_LIGHT)
      intensity = Spot_Light(light, W, light_color, light_pos, radius);
   else if (ltype == LightType::T_DEPTH_LIGHT)
      intensity = Depth_Light(light, W, light_color, light_pos, radius);
   else if (ltype == LightType::T_TEXTURED_LIGHT)
      intensity = Textured_Light(light, W, light_color, light_pos, radius);
   else if (ltype == LightType::T_DIRECTIONAL_LIGHT)
      intensity = Directional_Light(light, W, light_color, light_pos, radius);
   else
      serror("Bad light type: %d in Shade\n", ltype);

   return intensity;
}

/**
 * @brief Computes the unoccluded lit-color contribution from every scene light at @p W.
 * Traces shadow rays where needed and writes results into @p light_colors.
 * @param Eye          Active viewpoint.
 * @param W            World-space point being shaded.
 * @param light_colors Out: per-light RGB color array (caller must provide enough entries).
 */
void Get_Light_Colors(Viewpoint *Eye, Vec W, Vec *light_colors)
{
   Light *light;
   Ray tray;
   Flt t, tmin, intensity, radius;
   int j;
   Vec SV, light_pos, light_color;

   for (light= RuntimeState::Lights,j=0;light!=NULL;light=light->next,j++) {
      VecCopy(W, tray.P);
      intensity = Light_Color(light, W, light_color, light_pos, &radius);
      if (!(light->flags & SHADOW_FLAG)) {
          light_colors[j][0] = light_color[0];
          light_colors[j][1] = light_color[1];
          light_colors[j][2] = light_color[2];
      } else if (ABS(intensity) < PLY_EPSILON) {
          MakeVector(0, 0, 0, light_colors[j]);
      } else {
          VecSub(light_pos, W, tray.D);
          t = VecNormalize(tray.D);
          tmin = ((runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) ? 0.1 : RuntimeState::rayeps);
          runtimeState::stats.nShadows++;
          #ifdef DEBUG
          printf("\nnShadows in light=%lu\n",runtimeState::stats.nShadows);
          printf("press enter\n");getchar();
          #endif
          auto ret = Shadow(Eye, light, &tray, tmin, t, radius);
          if (ret.i) {
             light_colors[j][0] = ret.ShadowVec[0] * light_color[0];
             light_colors[j][1] = ret.ShadowVec[1] * light_color[1];
             light_colors[j][2] = ret.ShadowVec[2] * light_color[2];
             }
          else
             MakeVector(0, 0, 0, light_colors[j]);
      }
   }
}

/** @brief Resets the global light list and light count to the empty state. */
void Initialize_Lights()
{
   RuntimeState::Lights = nullptr;
   RuntimeState::nLights = 0;
}

/** @brief Frees all lights in RuntimeState::Lights — per-type sub-structs, then each Light shell. */
void Deallocate_Lights()
{
   int j;
   Light *temp_light;
   struct t_textured_light *tlight;
   struct t_depth_light *tdlight;
   Img *img;

   while (RuntimeState::Lights != nullptr) {
      if (static_cast<LightType>(RuntimeState::Lights->type) == LightType::T_TEXTURED_LIGHT) {
         tlight = (struct t_textured_light *)RuntimeState::Lights->data;
         deallocate_node(tlight->color);
         if (tlight->tx != nullptr)
             delete tlight->tx;
         if (tlight->lens_flare != nullptr) {
            deallocate_node(tlight->lens_flare->color);
            delete (tlight->lens_flare);
            }
         if (tlight->alight != nullptr) {
            PolyAlightData* alight = tlight->alight;
            delete (alight->points);
            delete (alight);
            }
         }
      else if (static_cast<LightType>(RuntimeState::Lights->type) == LightType::T_DEPTH_LIGHT) {
         tdlight = (struct t_depth_light *)RuntimeState::Lights->data;
         deallocate_node(tdlight->color);
         img = tdlight->light_depth;
         if (img->copy == 0) {
            /* Deallocate the z-buffer */
            for (j=0;j<img->length;j++)
               polyray_free(img->image[j]);
            polyray_free(img->image);
            if (img->cmap != nullptr)
               polyray_free(img->cmap);
            delete img;
            }
         if (tdlight->WS != nullptr)
             delete tdlight->WS;
         }
      delete RuntimeState::Lights->data;
      temp_light = RuntimeState::Lights;
      RuntimeState::Lights = RuntimeState::Lights->next;
      delete temp_light;
      }
   RuntimeState::nLights = 0;
}

/**
 * @brief Attaches a new default FLARECOMP to the current textured light.
 * Replaces any existing flare. Current_Light must be T_TEXTURED_LIGHT.
 */
void
Create_Lens_Flare()
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;

   tlight = (struct t_textured_light *)Current_Light->data;

   if (tlight->lens_flare != nullptr) {
      deallocate_node(tlight->lens_flare->color);
      polyray_free(tlight->lens_flare);
      }

   /* Add this flare to the existing list of flares */
   flare = (FLARECOMP*)polyray_malloc(sizeof(FLARECOMP));
   flare->color    = nullptr; /* Which will become white */
   flare->count    = 10;
   flare->spacing  = 1.0;
   flare->seed     = 0;
   flare->min_rad  = 0.005;
   flare->max_rad  = 0.05;
   flare->concave  = 0.75;
   flare->radius   = 0.0;

   tlight->lens_flare = flare;
}

/** @brief Sets the color expression on the current light's lens flare component. */
void Set_Flare_Color(NODE_PTR color)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   if (flare->color != nullptr)
      deallocate_node(flare->color);
   flare->color = color;
}

/** @brief Sets the number of lens flare discs (clamped to ≥ 1). */
void Set_Flare_Count(int count)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   if (count < 0) {
      swarning("Must be a positive number of flares for a lens (reset to 1)");
      count = 1;
      }
   flare->count = count;
}

/** @brief Sets the power-distribution exponent controlling flare disc spacing along the lens axis. */
void Set_Flare_Spacing(Flt spacing)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->spacing = spacing;
}

/** @brief Sets the random seed used to generate lens flare disc positions. */
void Set_Flare_Seed(int seed)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->seed = seed;
}

/**
 * @brief Sets the minimum and maximum disc radii as a fraction of image width.
 * @param min_rad  Smallest flare disc radius.
 * @param max_rad  Largest flare disc radius.
 */
void Set_Flare_Size(Flt min_rad, Flt max_rad)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->min_rad = min_rad;
   flare->max_rad = max_rad;
}

/** @brief Sets the fraction of flare discs rendered as concave (vs. convex). */
void Set_Flare_Concave(Flt concave_ratio)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->concave = concave_ratio;
}

/** @brief Sets the radius of the circular glow rendered around the light position. */
void Set_Flare_Sphere(Flt radius)
{
   struct t_textured_light *tlight;
   FLARECOMP *flare;
   tlight = (struct t_textured_light *)Current_Light->data;
   flare  = tlight->lens_flare;
   flare->radius = radius;
}

/**
 * @brief Renders all lens flare discs for @p flare into the current scan strip.
 * Projects @p L onto the screen and places discs along the light-to-centre axis.
 * @param eye    Active viewpoint.
 * @param flare  Flare component describing count, size, spacing, seed, etc.
 * @param C      Light color used when flare has no explicit color expression.
 * @param L      Light world-space position.
 */
static void Draw_Lens_Flares(Viewpoint *eye, FLARECOMP *flare, Vec C, Vec L)
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
   if (flare->color != nullptr) {
      i = eval_node(nullptr, flare->color, &ftemp, D, &ntemp);
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
   w = tx_point(* eye->WS, L, S0);
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
         yhigh = PLY_MIN(yhigh, win.y1);
         }

      if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
          runtimeState::settings.Render_Method == rmode::HIDDEN_LINE) {
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
                     serror("Bad color expression in lens flare");
                     }
                  }

               /* Now draw the colored point */
               draw_point(eye, x0, y0, 0.0, Cf, opac);
               }
            }
         }
      }
}

/**
 * @brief Iterates all scene lights and renders lens flares for any textured lights that have them.
 * @param eye  Active viewpoint.
 */
void
Draw_Flares(Viewpoint *eye)
{
   Light *light;
   struct t_textured_light *tlight;
   Flt intensity, radius;
   Vec W, C, L;

   /* Step through each light and see if it has any associated flares */
   for (light= RuntimeState::Lights;light!=NULL;light=light->next) {
      if (static_cast<LightType>(light->type) == LightType::T_TEXTURED_LIGHT) {
         tlight = (struct t_textured_light *)light->data;
         if (tlight->lens_flare != NULL) {
            /* First see how the light projects onto the screen */
            MakeVector(0, 0, 0, W);
            intensity = Textured_Light(light, W, C, L, &radius);
            if ((RuntimeState::settings.Optimizer > 0) &&
                ((runtimeState::settings.Render_Method == rmode::RAY_TRACING) ||
                 ((runtimeState::settings.Render_Method == rmode::GOURAD_SHADE ||
                   runtimeState::settings.Render_Method == rmode::SCAN_CONVERSION) &&
                  (runtimeState::scene.Global_Shade_Flag &
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

/**
 * @brief Builds a jittered sample ray toward a spherical area light.
 * Uses a 19-sample hexagonal pattern with random jitter applied to each sample.
 * @param index     Sample index into the hex pattern [0, 18].
 * @param L         Normalised direction toward the light.
 * @param radius    Projected radius of the light disc at @p distance.
 * @param distance  Distance from the shading point to the light.
 * @param D         Out: normalised jittered ray direction.
 */
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

//struct ShadRet {
//    int i;
//    NuVec ShadowVec;//SV
//};
/**
 * @brief Ray-marches from ray->P toward a light, accumulating filter color through
 * transparent occluders. Returns 1 with a filter if any unblocked light reaches the point.
 * For polygon area lights delegates to PolygonLight(). For spherical area lights fires
 * up to 19 jittered hex-pattern rays.
 * @param eye     Active viewpoint (used for surface evaluation on transmissive hits).
 * @param light   The light casting shadows (may be nullptr to disable caching).
 * @param ray     Shadow ray from the shaded point toward the light.
 * @param tmin    Minimum valid intersection distance (ray epsilon).
 * @param tmax    Distance to the light.
 * @param radius  Light source radius (0 = point, >0 = spherical area light).
 * @return ShadRet with @c i=1 and @c ShadowVec=filter color if lit; @c i=0 if fully shadowed.
 */
ShadRet Shadow(Viewpoint *eye, Light *light, Ray *ray,
       Flt tmin, Flt tmax,
       Flt radius)
{
   Ray jray;
   Isect thit;
   Flt t, dmax, light_left, fscale;
   Flt theta, lradius, ldistance;
   Surface *surf;
   int r, rmax, unblocked, cnt = 0;
   int old_test = runtimeState::Shadow_Test;
   Vec Filt[19], fcolor;
   Object *cobj;

   if (!(runtimeState::scene.Global_Shade_Flag & SHADOW_CHECK)) {
      ShadRet SV;
      MakeVector(1.0, 1.0, 1.0, SV.ShadowVec);
      SV.i = 1;
      return SV;
      }

   runtimeState::Shadow_Test = 1;

   /* If this is a polygonal area light, then we jump to the routine
      specifically built to deal with it. */
   if (light != nullptr && static_cast<LightType>(light->type) == LightType::T_TEXTURED_LIGHT) {
      struct t_textured_light *tlight;
      PolyAlightData *alight;

      tlight = (struct t_textured_light *)light->data;
      alight = tlight->alight;
      if (alight != nullptr) {
         ShadRet SV;
         Vec temp;
         SV.i = PolygonLight(eye, light, alight, tmin, ray->P, temp);
         SV.ShadowVec = toNuVec(temp);
         RuntimeState::Shadow_Test = old_test;
         return SV;
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
         if (light != nullptr && cnt == 0) {
             RuntimeState::stats.totalShadows++;
             cobj = Get_Light_Blocker(light, RuntimeState::recursion_depth);
             thit.flag = 0;
             if (cobj != NULL &&
                 find_object_intersections(eye, cobj, &jray, tmin,
                                           dmax, &thit)) {
               if ((runtimeState::scene.Global_Shade_Flag & TRANSMIT_CHECK) &&
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
                      RuntimeState::stats.totalShadowCaches++;
                     light_left = 0.0;
                     break;
                     }
                  }
               else {
                   RuntimeState::stats.totalShadowCaches++;
                  light_left = 0.0;
                  break;
                  }
               }
             }

          if (Intersect(eye, &RuntimeState::scene.Root, &jray, tmin, dmax, &thit)) {
            /* See if there is any color left after this hit */
            if (thit.obj->o_type == ShapeType::Hypertexture) {
               /* Special case, the color and opacity have already
                  been calculated for us.  */
               fscale = thit.U[0];
               MakeVector(1, 1, 1, fcolor)
               t = thit.U[1];
               }
            else if ((runtimeState::scene.Global_Shade_Flag & TRANSMIT_CHECK) &&
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
                  if (cnt == 0 && light != nullptr)
                     Set_Light_Blocker(light, RuntimeState::recursion_depth, thit.obj);
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
                  Set_Light_Blocker(light, RuntimeState::recursion_depth, thit.obj);
               light_left = 0.0;
               break;
               }

            /* Reduce the amount of light by the filter color associated
               with the thing it hit */
            for (int i=0;i<3;++i)
               Filt[r][i] *= fscale * fcolor[i];
            light_left = VecDot(Filt[r], Filt[r]);

            /* Move up a little closer to the light */
            VecCopy(thit.W, jray.P);
            dmax = dmax - t;
            cnt++;
            }
         else {
            if (cnt == 0 && light != nullptr)
               Set_Light_Blocker(light, RuntimeState::recursion_depth, nullptr);
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
   ShadRet SV;
   if (rmax == 1)
      VecCopy(Filt[0], SV.ShadowVec)
   else {      
      MakeVector(0.0, 0.0, 0.0, SV.ShadowVec);
      for (int i=0;i<r;i++)
         VecAdd(Filt[i], SV.ShadowVec, SV.ShadowVec);
      t = 1.0 / (Flt)r; /* (Flt)unblocked / ((Flt)rmax * r); */
      VecScale(t, SV.ShadowVec);
      }
   RuntimeState::Shadow_Test = old_test;
   if (SV.ShadowVec[0] == 0.0 && SV.ShadowVec[1] == 0.0
       && SV.ShadowVec[2] == 0.0) {
       SV.i = 0;
       return SV;
   }
   SV.i = 1;
   return SV;
}
