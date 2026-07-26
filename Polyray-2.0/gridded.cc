/* gridded.cc

  Polyray MIT Licensed Revival
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

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "runtime_state.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "gridded.h"
#include "bound.h"
#include "image.h"
#include "factory.h"
#include "util.h"

void GridRender(Viewpoint *, BinTree *, Object *);




openpolyray::dispatch::ObjectProcs GridProcs = {
   .render = GridRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = GridIntersect,
   .inside = GridInside,
   .copy = GenericCopy,
   .del = GridDelete,
   };

/**
 * @brief CSG inside test — always returns 0 because gridded objects are not CSG primitives.
 * @param obj  The grid object (unused).
 * @param P    Point to test (unused).
 * @return 0 always.
 */
int
GridInside(Object *obj, Vec P)
{
   /* Not a csg primitive */
   return 0;
}

/**
 * @brief Constructs a gridded object from an image index-map and a list of prototype objects.
 *
 * The image is read as an 8-bit index buffer: each pixel value selects which prototype object
 * occupies that grid cell. The object list is stored as an array of BVH slabs (one per prototype).
 *
 * @param object    Pre-allocated Object to populate.
 * @param filename  Path to the image file whose pixel values encode the grid index map.
 * @param objs      Parser-supplied stack of prototype objects (consumed and freed here).
 * @return The populated @p object pointer.
 */
Object *MakeGrid(Object *object, char *filename, ostackptr objs)
{
   GridData *grid;
   int OldOptim;
   int i, j, obj_cnt;
   int width, height;
   Flt x, xdelta, y, ydelta, findx;
   ostackptr last_obj, temp_obj;
   Object *obj;
   Img *gridimg;

   object->o_type = ShapeType::Gridded;
   object->o_procs = &GridProcs;

   /* Attempt to allocate memory for this primitive */
   grid = FactoryGridData();

   /* Count up the # of objects that were parsed in */
   for (temp_obj=objs,obj_cnt=0;temp_obj!=NULL;temp_obj=temp_obj->next)
      ++obj_cnt;
   /* Store the list objects within the grids' data structure */
   grid->obj_cnt = obj_cnt;
   grid->objs = (BinTree *)polyray_malloc(obj_cnt * sizeof(BinTree));
   if (grid->objs == nullptr)
      serror("Failed to allocate gridded object data");
   for (i=0;i<obj_cnt;++i)
      Initialize_BinTree(grid->objs[i]);
   for (temp_obj=objs,i=0;temp_obj!=nullptr;++i) {
      last_obj = temp_obj;
      obj = temp_obj->element;
      Add_To_BinTree(grid->objs[obj_cnt-i-1], obj);
      OldOptim = RuntimeState::settings.Optimizer;
      RuntimeState::settings.Optimizer = 1;
      BuildBoundingSlabs(grid->objs[obj_cnt-i-1]);//old code &
      RuntimeState::settings.Optimizer = OldOptim;
      temp_obj = temp_obj->next;
      polyray_free(last_obj);
      }

   // Buffer the grid image into temporary storage - it wastes space, but
   //   allows the use of a standard image reader routine 
   auto ret = ReadImage(filename);
   if (ret.has_value())
     gridimg = ret.value();
   else
     serror("%s",("MakeGridd: "+ret.error()).c_str());

   grid->xsize = gridimg->width;
   grid->zsize = gridimg->length;

   grid->data = (unsigned char **)polyray_malloc(gridimg->length *
                                                 sizeof(unsigned char *));
   if (grid->data == NULL)
      serror("Failed to allocate grid data\n");
   ydelta = 1.0 / (Flt)gridimg->length;
   xdelta = 1.0 / (Flt)gridimg->width;
   height = gridimg->length;
   width  = gridimg->width;
   for (i=0,y=0.0;i<height;i++,y+=ydelta) {
      grid->data[i] = (unsigned char *)polyray_malloc(width *
                                                      sizeof(unsigned char));
      if (grid->data[i] == NULL)
         serror("Failed to allocate grid->data[%d]\n", i);
      /* Read in row of grid data. */
      for (j=0,x=0.0;j<width;j++,x+=xdelta) {
         if (!lookup_index(gridimg, x, y, 0, &findx)) {
             FreeImg(gridimg);
             serror("Failed to determine grid index at %dx%d", j, i);
             }
         grid->data[i][j] = (unsigned char)findx;
         }
      }
   FreeImg(gridimg);

   MakeVector(0, 0, 0, grid->boundbox[0]);
   MakeVector(gridimg->width, 1, gridimg->length, grid->boundbox[1]);

   /* Set the data pointer for this object */
   object->o_data = (void *)grid;

   MakeVector(0.0, 0.0, 0.0, object->o_bnd.lower_left);
   MakeVector(gridimg->width, 1.0, gridimg->length, object->o_bnd.lengths);

   return object;
}

/**
 * @brief Frees all dynamic storage owned by a gridded object.
 *
 * Releases each row of elevation data, the object BVH slab array, and the GridData struct.
 * No-op for copies (@c o_copy != 0) because they share data with the original.
 *
 * @param object  The grid Object to deallocate.
 */
void GridDelete(Object *obj)
{
   GridData *grid = static_cast<GridData*>(obj->o_data);

   // Only delete the memory if this is the original 
   if (obj->o_copy != 0) return;

   /* Free the height data */
   for (int i=0;i<grid->zsize;++i)
      polyray_free(grid->data[i]);
   polyray_free(grid->data);

   /* Delete the list of objects */
   for (int i=0;i<grid->obj_cnt;i++)
      Delete_BinTree(&grid->objs[i]);
   polyray_free(grid->objs);

   // Free the height field structure itself 
   delete static_cast<GridData*>(obj->o_data);
   obj->o_data = nullptr;
}

/**
 * @brief Tests a ray against the prototype object stored at grid cell (@p x, @p z).
 *
 * Offsets the ray origin into the cell's local frame before calling Intersect(), then
 * untranslates the resulting world-space hit position.
 *
 * @param Eye      Active viewpoint.
 * @param grid     Grid data containing the index map and per-prototype BVH slabs.
 * @param P        Ray origin in grid space.
 * @param D        Ray direction (normalised).
 * @param x        Grid cell column index.
 * @param z        Grid cell row index.
 * @param mindist  Minimum valid intersection distance.
 * @param maxdist  Maximum valid intersection distance.
 * @param hit      Output intersection record; updated on a closer hit.
 * @return 1 if an intersection was found, 0 otherwise.
 */
static int
get_grid_intersections(Viewpoint *Eye, GridData *grid, Vec P, Vec D,
                       int x, int z,
                       Flt mindist, Flt maxdist,
                       Isect *hit)
{
   int ind;
   Vec tP;
   Ray tray;
   int flag = 0;

   /* Look for an intersection at grid point (x1, z1) */

   if (x >= 0 && x < grid->xsize &&
       z >= 0 && z < grid->zsize) {
      ind = grid->data[z][x];
      if (ind < grid->obj_cnt) {
         VecCopy(D, tray.D);
         MakeVector(-x, 0, -z, tP);
         VecAdd(P, tP, tray.P);
         MakeVector(P[0]-x, P[1], P[2]-z, tray.P);
         if (Intersect(Eye, &grid->objs[ind], &tray, mindist, maxdist, hit)) {
            VecSub(hit->W, tP, hit->W);
            flag = 1;
            }
         }
      }
   return flag;
}

/**
 * @brief Ray-grid intersection using a 2-D DDA traversal.
 *
 * Clips the ray to the grid bounding box, then walks cells in X/Z ray order.
 * For each cell whose Y extent overlaps the ray, calls get_grid_intersections().
 * Returns on the first hit found (no sorting — the DDA order is front-to-back).
 * If the grid object has a transform, the hit position and normal are mapped back
 * to world space before returning.
 *
 * @param Eye      Active viewpoint.
 * @param obj      The grid Object (carries GridData and optional transform).
 * @param ray      Query ray in world space.
 * @param mindist  Minimum valid intersection distance.
 * @param maxdist  Maximum valid intersection distance.
 * @param hit      Output intersection record.
 * @return 1 if an intersection was found, 0 otherwise.
 */
int GridIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Vec hitpos;
   Flt mind, maxd;
   int x, z, posY;
   int stepX=0, stepZ=0, outX=0, outZ=0;
   float tDX=0.0, tDZ=0.0;
   Flt tX, tZ;
   Vec nxp, nzp, pDX, pDZ;
   GridData *grid = (GridData*)obj->o_data;
   Vec P, D;

   VecCopy(ray->P, P);
   VecCopy(ray->D, D);

   mind = mindist;
   maxd = maxdist;
   if (determine_start(P, D, grid->boundbox, &mind, &maxd)) {
      VecAddScaled(P, mind, D, hitpos);
      }
   else
      return 0;

   if (D[0] < 0.0) {
      stepX = -1;
      outX  = -1;
      tDX   = -1.0 / D[0];
      }
   else if (D[0] > 0.0) {
      stepX = 1;
      outX  = grid->xsize;
      tDX   = 1.0 / D[0];
      }

   if (D[2] < 0.0) {
      stepZ = -1;
      outZ  = -1;
      tDZ   = -1.0 / D[2];
      }
   else if (D[2] > 0.0) {
      stepZ = 1;
      outZ = grid->zsize;
      tDZ = 1.0 / D[2];
      }

   pDX[0] = D[0] * tDX;
   pDX[1] = D[1] * tDX;
   pDX[2] = D[2] * tDX;
   pDZ[0] = D[0] * tDZ;
   pDZ[1] = D[1] * tDZ;
   pDZ[2] = D[2] * tDZ;

   /* Are we going up or down? */
   posY = (D[1] > 0.0);

   /* Figure out what pixel we are starting at */
   x = hitpos[0];
   z = hitpos[2];

   if (D[0] < 0.0)
      tX = ((Flt)x - hitpos[0]) / D[0];
   else if (D[0] > 0.0)
      tX = ((Flt)(x+1) - hitpos[0]) / D[0];
   else
      tX = PLY_HUGE;

   if (D[2] < 0.0)
      tZ = ((Flt)z  - hitpos[2]) / D[2];
   else if (D[2] > 0.0)
      tZ = ((Flt)(z+1) - hitpos[2]) / D[2];
   else
      tZ = PLY_HUGE;

   VecAddScaled(hitpos, tX, D, nxp);
   VecAddScaled(hitpos, tZ, D, nzp);

   /* Now that all of the information has been set up, lets do the DDA. */
top_of_loop:
   if (tX < tZ) {
      if (( posY && hitpos[1]<=1 && nxp[1]>=0) ||
          (!posY && hitpos[1]>=0 && nxp[1]<=1))
         if (get_grid_intersections(Eye, grid, P, D, x, z, mindist, maxdist, hit)) {
            if (obj->o_trans) {
               TxVector(hit->W, hit->W, obj->o_trans);
               TxNormal(hit->N, hit->N, obj->o_trans);
               }
            return 1;
            }
      x += stepX;
      if (x == outX)
         return 0;
      tX += tDX;
      VecCopy(nxp, hitpos);
      VecAdd(nxp, pDX, nxp);
      }
   else {
      if (( posY && hitpos[1]<=1 && nzp[1]>=0) ||
          (!posY && hitpos[1]>=0 && nzp[1]<=1))
         if (get_grid_intersections(Eye, grid, P, D, x, z, mindist, maxdist, hit)) {
            if (obj->o_trans) {
               TxVector(hit->W, hit->W, obj->o_trans);
               TxNormal(hit->N, hit->N, obj->o_trans);
               }
            return 1;
            }
      z += stepZ;
      if (z == outZ)
         return 0;
      tZ += tDZ;
      VecCopy(nzp, hitpos);
      VecAdd(nzp, pDZ, nzp);
      }
   if ( posY && (hitpos[1] <= 1)) goto top_of_loop;
   if (!posY && (hitpos[1] >= 0)) goto top_of_loop;

   return 0;
}

/**
 * @brief Scan-conversion render pass for a gridded object.
 *
 * Iterates every grid cell, translates the cell's prototype object to its (i, j) position
 * in grid space (composing with the grid's own transform if present), then delegates to
 * that prototype's render proc. The prototype's transform pointer is saved and restored
 * around each call so the prototype is not permanently modified.
 *
 * @param eye   Active viewpoint.
 * @param Root  Scene BVH root (passed through to sub-object render procs).
 * @param obj   The grid Object.
 */
void GridRender(Viewpoint *eye, BinTree *Root, Object *obj) //needs a test!
{
   int i, j, ind;
   Object *new_obj;
   ostackptr objs;
   Transform tx, old_tx, *sav_tx;
   Vec txvec;
   GridData *grid = (GridData*)obj->o_data;

   txvec[1] = 0;
   for (i=0;i<grid->xsize;i++) {
      txvec[0] = i;
      for (j=0;j<grid->zsize;j++) {
          if ((RuntimeState::Check_Abort_Flag != 0) && _kbhit())
              throw std::runtime_error("ABORT");

         txvec[2] = j;
         ind = grid->data[j][i];
         if (ind < grid->obj_cnt &&
             (objs = grid->objs[ind].members.list) != nullptr) {
            while (objs != nullptr) {
               new_obj = objs->element;
               sav_tx = new_obj->o_trans;
               Get_Translation_Transformation(&tx, txvec);
               //Get_Translation_TransformationCPP(&tx, txvec);
               if (new_obj->o_trans) {
                  old_tx = *(new_obj->o_trans);
                  Compose_Transformations(old_tx, tx);
                  tx = old_tx;
                  }
               if (obj->o_trans)
                  Compose_Transformations(tx, * obj->o_trans);
               new_obj->o_trans = &tx;
#if 0
/* Need code to recompute the bounding box for this element of
   the gridded object, project it onto the screen, and if not
   visible then to skip it. */
               /* See if the object is visible on the screen */
               BboxScreenSize(eye, &obj->o_bnd, &width, &height);
               if (width > 0 && height > 0)
#endif
                  new_obj->o_procs->render(eye, Root, new_obj);
               new_obj->o_trans = sav_tx;
               objs = objs->next;
               }
            }
         }
      }
}
