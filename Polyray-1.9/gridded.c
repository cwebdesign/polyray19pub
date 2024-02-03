/* gridded.c

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
#include "gridded.h"
#include "bound.h"
#include "image.h"

typedef struct {
   unsigned char **data; /* Elevation information */
   int obj_cnt;          /* # of distinct objects in the grid */
   BinTree *objs;        /* Array of objects stored in the grid */
   int xsize, zsize;     /* # of points/side */
   Flt boundbox[2][3];   /* Bounds for the entire height field */
   } GridData;

void GridRender(Viewpoint *, BinTree *, Object *);
int GridIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);
int GridInside(Object *obj, Vec P);
void GridDelete(Object *object);

ObjectProcs GridProcs = {
   GridRender,
   NULL,
   GenericInitialize,
   GridIntersect,
   GridInside,
   GenericCopy,
   GridDelete,
   };

int
GridInside(Object *obj, Vec P)
{
   /* Not a csg primitive */
   return 0;
}

Object *
MakeGrid(Object *object, char *filename, ostackptr objs)
{
   GridData *grid;
   int OldOptim;
   int i, j, obj_cnt;
   int width, height;
   Flt x, xdelta, y, ydelta, findx;
   ostackptr last_obj, temp_obj;
   Object *obj;
   Img *gridimg;

   object->o_type = T_GRIDDED;
   object->o_procs = &GridProcs;

   /* Attempt to allocate memory for this primitive */
   if ((grid = (GridData *)polyray_malloc(sizeof(GridData))) == NULL)
      error("Failed to allocate gridded object data\n");

   /* Count up the # of objects that were parsed in */
   for (temp_obj=objs,obj_cnt=0;temp_obj!=NULL;temp_obj=temp_obj->next)
      obj_cnt++;
   /* Store the list objects within the grids' data structure */
   grid->obj_cnt = obj_cnt;
   grid->objs = (BinTree *)polyray_malloc(obj_cnt * sizeof(BinTree));
   if (grid->objs == NULL)
      error("Failed to allocate gridded object data");
   for (i=0;i<obj_cnt;i++)
      Initialize_BinTree(&grid->objs[i]);
   for (temp_obj=objs,i=0;temp_obj!=NULL;i++) {
      last_obj = temp_obj;
      obj = temp_obj->element;
      Add_To_BinTree(&grid->objs[obj_cnt-i-1], obj);
      OldOptim = Optimizer;
      Optimizer = 1;
      BuildBoundingSlabs(&grid->objs[obj_cnt-i-1]);
      Optimizer = OldOptim;
      temp_obj = temp_obj->next;
      polyray_free(last_obj);
      }

   /* Buffer the grid image into temporary storage - it wastes space, but
      allows the use of a standard image reader routine */
   gridimg = TGAReadImage(filename);

   grid->xsize = gridimg->width;
   grid->zsize = gridimg->length;

   grid->data = (unsigned char **)polyray_malloc(gridimg->length *
                                                 sizeof(unsigned char *));
   if (grid->data == NULL)
      error("Failed to allocate grid data\n");
   ydelta = 1.0 / (Flt)gridimg->length;
   xdelta = 1.0 / (Flt)gridimg->width;
   height = gridimg->length;
   width  = gridimg->width;
   for (i=0,y=0.0;i<height;i++,y+=ydelta) {
      grid->data[i] = (unsigned char *)polyray_malloc(width *
                                                      sizeof(unsigned char));
      if (grid->data[i] == NULL)
         error("Failed to allocate grid->data[%d]\n", i);
      /* Read in row of grid data. */
      for (j=0,x=0.0;j<width;j++,x+=xdelta) {
         if (!lookup_index(gridimg, x, y, 0, &findx)) {
             FreeImg(gridimg);
             error("Failed to determine grid index at %dx%d", j, i);
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

/* Deallocate all of the dynamic storage associated with
   a height field object */
void
GridDelete(Object *object)
{
   GridData *grid = (GridData *)object->o_data;
   int i;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0) return;

   /* Free the height data */
   for (i=0;i<grid->zsize;i++)
      polyray_free(grid->data[i]);
   polyray_free(grid->data);

   /* Delete the list of objects */
   for (i=0;i<grid->obj_cnt;i++)
      Delete_BinTree(&grid->objs[i]);
   polyray_free(grid->objs);

   /* Free the height field structure itself */
   polyray_free(grid);
}

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

int
GridIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
              Flt mindist, Flt maxdist, Isect *hit)
{
   Vec hitpos;
   Flt mind, maxd;
   int x, z, posY;
   int stepX, stepZ, outX, outZ;
   float tDX=0.0, tDZ=0.0;
   Flt tX, tZ;
   Vec nxp, nzp, pDX, pDZ;
   GridData *grid = obj->o_data;
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

void
GridRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, ind;
   Object *new_obj;
   ostackptr objs;
   Transform tx, old_tx, *sav_tx;
   Vec txvec;
   GridData *grid = obj->o_data;

   txvec[1] = 0;
   for (i=0;i<grid->xsize;i++) {
      txvec[0] = i;
      for (j=0;j<grid->zsize;j++) {
         if ((Check_Abort_Flag != 0) && _kbhit())
            longjmp(abort_environ, 1);

         txvec[2] = j;
         ind = grid->data[j][i];
         if (ind < grid->obj_cnt &&
             (objs = grid->objs[ind].members.list) != NULL) {
            while (objs != NULL) {
               new_obj = objs->element;
               sav_tx = new_obj->o_trans;
               Get_Translation_Transformation(&tx, txvec);
               if (new_obj->o_trans) {
                  old_tx = *(new_obj->o_trans);
                  Compose_Transformations(&old_tx, &tx);
                  tx = old_tx;
                  }
               if (obj->o_trans)
                  Compose_Transformations(&tx, obj->o_trans);
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
