/* height.cc

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

#include <gsl/gsl>

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "intersec.h"
#include "runtime_state.h"
#include "symtab.h"
#include "scan.h"
#include "vector.h"
#include "bound.h"
#include "height.h"
#include "eval.h"
#include "builder.h"
#include "image.h"
#include "util.h" //InvertMatrix
#include "factory.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _getch()
#endif
constexpr int ALT_BOXSIZE = 2;



void HeightRender(Viewpoint *, BinTree *, Object *);
int HeightIntersect(Viewpoint *, Object *, Ray *, Flt, Flt, Isect *);

openpolyray::dispatch::ObjectProcs HeightProcs = {
   .render = HeightRender,
   .evaluate = nullptr,
   .initialize = GenericInitialize,
   .intersect = HeightIntersect,
   .inside = HeightInside,
   .copy = GenericCopy,
   .del = HeightDelete,
   };

void indexed_cyl_to_cart(HeightData *hf, int u, int v, Vec P)
{
   Flt r, t0, t1;

   r  = hf->data[v][u];
   P[1] = v;
   P[0] = r *  hf->theta_norms[u][2]; /* cos(Q[0]); */
   P[2] = r * -hf->theta_norms[u][0]; /* sin(Q[0]); */
}

void indexed_geo_to_cart(HeightData *hf, int u, int v, Vec P)
{
   Flt r, t0, t1;

   r  = hf->data[v][u];
   t1 = hf->phi_sin[v];         /* sin(Q[1]); */
   P[1] = r * t1;
   t0 = -hf->theta_norms[u][0]; /* sin(Q[0]); */
   t1 = hf->phi_cos[v];         /* cos(Q[1]) */
   P[2] = r * t0 * t1;
   t0 = hf->theta_norms[u][2];  /* cos(Q[0]); */
   P[0] = r * t0 * t1;
}

void create_angle_tables(ShapeType hf_type, int u_steps, int v_steps,
                    Flt **phi_sin, Flt **phi_cos,
                    Vec **theta_norms)
{
   int i;
   Flt phi, dphi, theta, dtheta;

   if (hf_type == ShapeType::Sph_Height_Field) {
      *phi_sin = (Flt *)polyray_malloc(v_steps * sizeof(Flt));
      *phi_cos = (Flt *)polyray_malloc(v_steps * sizeof(Flt));
      dphi = PYM_PI / (double)(v_steps - 1);
      for (i=0,phi=-PYM_PI/2.0;i<v_steps;i++,phi+=dphi) {
         (*phi_sin)[i] = sin(phi);
         (*phi_cos)[i] = cos(phi);
         }
      }
   else {
      *phi_sin = nullptr;
      *phi_cos = nullptr;
      }

   *theta_norms = (Vec *)polyray_malloc(u_steps * sizeof(Vec));
   dtheta = TWO_PI / (double)(u_steps - 1);
   for (i=0,theta=0.0;i<u_steps;i++,theta+=dtheta) {
      (*theta_norms)[i][0] = -sin(theta);
      (*theta_norms)[i][1] =  0.0;
      (*theta_norms)[i][2] =  cos(theta);
      }
}

/* Initialize variables for walking longitude lines */
void init_u_variables(Flt thetas, Flt thetae,
                 Flt *dtheta, Flt *theta0,
                 int *u0, int *u1, int *u,
                 int u_steps, int dir_flag)
{
   *dtheta = TWO_PI / (double)(u_steps-1);
   if (dir_flag) {
      *u0 = ceil(thetas / *dtheta);
      *u1 = ceil(thetae / *dtheta);
      *u  = 1;
      *theta0 = *dtheta * (float)*u0;
      }
   else {
      *u0 = floor(thetas / *dtheta);
      *u1 = floor(thetae / *dtheta);
      *theta0 = *dtheta * (float)*u0;
      if (1 || *u0 != *u1) {
         *u  = -1;
         *dtheta = -*dtheta;
         }
      else {
         *u0 = ceil(thetas / *dtheta);
         *u1 = ceil(thetae / *dtheta);
         *u = 1;
         }
      }

   if (*u0 < 0)
      *u0 = u_steps + *u0 - 1;
   if (*u1 < 0)
      *u1 = u_steps + *u1 - 1;
}

Flt
get_theta_t(int u, Vec P, Vec D, int theta_dir_flag,
            Vec *theta_norms, Flt t, Flt mindist, Flt maxdist)
{
   Flt d, n;

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

static int add_single_normal(HeightData *hf, ShapeType hf_type,
                  int x0, int z0,
                  int x1, int z1,
                  int x2, int z2,
                  Vec N)
{
   float **data = hf->data;
   Vec v0, v1, v2, t0, t1, Nt;

   if (x0 < 0 || z0 < 0 ||
       x1 < 0 || z1 < 0 ||
       x2 < 0 || z2 < 0 ||
       x0 >= hf->xsize || z0 >= hf->zsize ||
       x1 >= hf->xsize || z1 >= hf->zsize ||
       x2 >= hf->xsize || z2 >= hf->zsize) {
      return 0;
      }
   else {
      if (hf_type == ShapeType::Height_Field) {
         MakeVector(x0, data[z0][x0], z0, v0);
         MakeVector(x1, data[z1][x1], z1, v1);
         MakeVector(x2, data[z2][x2], z2, v2);
         }
      else if (hf_type == ShapeType::Sph_Height_Field) {
         indexed_geo_to_cart(hf, x0, z0, v0);
         indexed_geo_to_cart(hf, x1, z1, v1);
         indexed_geo_to_cart(hf, x2, z2, v2);
         }
      else { /* hf_type == T_CYL_HEIGHT_FIELD */
         indexed_cyl_to_cart(hf, x0, z0, v0);
         indexed_cyl_to_cart(hf, x1, z1, v1);
         indexed_cyl_to_cart(hf, x2, z2, v2);
         }

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
void smooth_height_field(HeightData *hf, ShapeType hf_type)
{
   int i, j, k;
   int xsize = hf->xsize;
   int zsize = hf->zsize;
   Flt t;
   Vec N;

   /* First off, allocate all the memory needed to store the
      normal information */
   hf->norm = (fVec **)polyray_malloc(zsize * sizeof(fVec *));
   if (hf->norm == nullptr)
      serror("Failed to allocate hf->norm\n");
   for (i=0; i<zsize; i++) {
       hf->norm[i] = (fVec *)polyray_malloc(hf->xsize * sizeof(fVec));
       if (hf->norm[i] == nullptr)
          serror("Failed to allocate hf->norm[%d]\n", i);
       }

   /* For now we will do it the hard way - by generating the normals
      individually for each elevation point */
   for (i=0;i<zsize;++i) {
      for (j=0;j<xsize;++j) {
         MakeVector(0.0, 0.0, 0.0, N);
         k = 0;
         if (hf_type == ShapeType::Height_Field) {
            k += add_single_normal(hf, hf_type, j, i, j-1, i, j-1, i-1, N);
            k += add_single_normal(hf, hf_type, j, i, j-1, i-1, j, i-1, N);
            k += add_single_normal(hf, hf_type, j, i, j, i-1, j+1, i, N);
            k += add_single_normal(hf, hf_type, j, i, j+1, i, j+1, i+1, N);
            k += add_single_normal(hf, hf_type, j, i, j+1, i+1, j, i+1, N);
            k += add_single_normal(hf, hf_type, j, i, j, i+1, j-1, i, N);
            }
         else {
            k += add_single_normal(hf, hf_type, j, i, j-1, i, j, i-1, N);
            k += add_single_normal(hf, hf_type, j, i, j-1, i+1, j-1, i, N);
            k += add_single_normal(hf, hf_type, j, i, j, i+1, j-1, i+1, N);
            k += add_single_normal(hf, hf_type, j, i, j+1, i, j, i+1, N);
            k += add_single_normal(hf, hf_type, j, i, j+1, i-1, j+1, i, N);
            k += add_single_normal(hf, hf_type, j, i, j, i-1, j+1, i-1, N);
            }

         if (k == 0)
            serror("Failed to find any normals at: (%d, %d)\n", i, j);
         t = 1.0 / (Flt)k;
         VecScale(t, N);
         VecCopy(N, hf->norm[i][j]);
         }
      }
}

void read_height_data(char *filename, float offset, float scale, HeightData *hf)
{
   Img *hfimg=nullptr;

   /* Buffer the height field into an image - it wastes storage, but
      allows the use of a standard image reader routine */
   auto ret = ReadImage(filename);
   if (ret.has_value())
      hfimg = ret.value();
   else
      serror("%s",ret.error().c_str());
      //hfimg = ReadImage(filename);
   
   hf->xsize = hfimg->width;
   hf->zsize = hfimg->length;

   if (hf->xsize < 2 || hf->zsize < 2) {
      FreeImg(hfimg);
      serror("Height field too small: %dx%d", hf->xsize, hf->zsize);
      }

   hf->low  = PLY_HUGE;
   hf->high = -PLY_HUGE;
   hf->storage.resize(static_cast<std::size_t>(hf->xsize) * hf->zsize);
   hf->rows.resize(hf->zsize);
   for (int z = 0; z < hf->zsize; ++z) {
      hf->rows[z] = hf->storage.data() + static_cast<std::size_t>(z) * hf->xsize;
   }
   hf->data = hf->rows.data();

   for (int i=0;i<hf->zsize;++i) {
       /* Rows already alias into hf->storage; no per-row allocation needed. */
       /* Read in row of HF data. */
       for (int j=0;j<hf->xsize;++j) {
          Flt depth = image_height(hfimg, j, i);
          float val = offset + scale * depth;
          hf->data[i][j] = val;
          if (val > hf->high) hf->high = val;
          if (val < hf->low) hf->low = val;
          }
      }

   FreeImg(hfimg);
}

Object *MakeHeight(Object *object, char *filename, int smoothed)
{
   HeightData *hf;
   Vec v;

   object->o_type = ShapeType::Height_Field;
   object->o_procs = &HeightProcs;

   /* Attempt to allocate memory for this primitive */
   hf = FactoryHeightData();
   
   /* Read in the height field information and build the data structures */
   read_height_data(filename, 0.0, 1.0, hf);
   hf->low -= PLY_EPSILON;
   hf->high += PLY_EPSILON;

   /* Check the size */
   if (hf->xsize < 2 || hf->zsize < 2)
      serror("Grid size: (%d,%d) is too small, must be larger than 2\n",
            hf->xsize, hf->zsize);

   /* Some preprocessing to speed up the ray-hf intersection process */
   hf->norm  = nullptr;

   /* These are unused in rectangular height fields */
   hf->phi_sin = nullptr;
   hf->phi_cos = nullptr;
   hf->theta_norms = nullptr;

   if (runtimeState::settings.Render_Method == rmode::WIRE_FRAME ||
       runtimeState::settings.Render_Method == rmode::HIDDEN_LINE ||
       runtimeState::settings.Render_Method == rmode::RAW_TRIANGLES ||
       runtimeState::settings.Render_Method == rmode::CSG_TRIANGLES)
      /* No need for normals if we are only interested vertices of the
         height field.  Note that UV_TRIANGLES requires the smoothed
         vertex normals so it is smoothed. */
      hf->type = 0;
   else if (smoothed) {
      hf->type = 1;
      smooth_height_field(hf, ShapeType::Height_Field);
      }
   else
      hf->type = 0;

   hf->cache_length = 0;
   hf->last_cached  = 0;

   hf->boundbox[0][0] = 0.0;
   hf->boundbox[0][1] = hf->low;
   hf->boundbox[0][2] = 0.0;
   hf->boundbox[1][0] = hf->xsize;
   hf->boundbox[1][1] = hf->high;
   hf->boundbox[1][2] = hf->zsize;

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   /* Transform the box so that 0<=x<=1 and 0<=z<=1 */
   hf->trans = Get_Transformation().release();
   MakeVector(1.0/(Flt)(hf->xsize - 1), 1.0, 1.0/(Flt)(hf->zsize - 1), v);
   Get_Scaling_Transformation(hf->trans, v);

   MakeVector(0, hf->low, 0, object->o_bnd.lower_left);
   MakeVector(1, hf->high-hf->low, 1, object->o_bnd.lengths);

   return object;
}

Object *MakeHeightFn(Object *object, int xsize, int zsize,
             Flt xlow, Flt xhigh, Flt zlow, Flt zhigh,
             NODE_PTR fn, int smoothed)
{
   int i, j;
   Flt vval, val;
   Flt xval, zval;
   Flt deltax, deltaz;
   Flt u, v, du, dv;
   struct subst_struct subst;
   Transform trans;
   NuVec tvec;
   NODE_PTR tnode;

   /* Check the size */
   if (xsize < 2 || zsize < 2)
      serror("Grid size: (%d,%d) is too small, must be larger than 2\n",
            xsize, zsize);

   object->o_type = ShapeType::Height_Field;
   object->o_procs = &HeightProcs;

   /* Should make a check here that the expression is based
      on x and z alone. */

   /* Make sure the area covered is non-zero */
   if (xlow == xhigh || zlow == zhigh)
      serror("Degenerate height field - bound(s) are equal");

   /* Attempt to allocate memory for this primitive */
   HeightData *hf = FactoryHeightData();
   hf->xsize = xsize;
   hf->zsize = zsize;
   hf->low  =  PLY_HUGE;
   hf->high = -PLY_HUGE;

   /* These are unused in rectangular height fields */
   hf->phi_sin = nullptr;
   hf->phi_cos = nullptr;
   hf->theta_norms = nullptr;

   /* Step through each element of the grid and create the height field */
   du = 1.0 / (Flt)(xsize - 1);
   dv = 1.0 / (Flt)(zsize - 1);
   deltax = (xhigh - xlow) / (Flt)(xsize-1);
   deltaz = (zhigh - zlow) / (Flt)(zsize-1);
   MakeVector(0, 0, 0, subst.PT);
   hf->storage.resize(static_cast<std::size_t>(hf->xsize) * hf->zsize);
   hf->rows.resize(hf->zsize);
   for (int z = 0; z < hf->zsize; ++z) {
      hf->rows[z] = hf->storage.data() + static_cast<std::size_t>(z) * hf->xsize;
   }
   hf->data = hf->rows.data();
   subst.P[1] = 0.0;
   for (i=0, zval=zlow, v=0.0; i<zsize; i++, zval+=deltaz, v+=dv) {
       subst.P[2] = zval;
       for (j=0, xval=xlow,u=0.0;j<xsize;j++, xval+=deltax,u+=du) {
          MakeVector(u, v, 0, subst.U);
          subst.P[0] = xval;
          Vec vtvec;
          // Direct, clean conversion
          toVec(tvec, vtvec);//src,dest
          if (eval_node(&subst, fn, &vval, vtvec, &tnode) == 1)
             val = vval;
          else
             val = 0.0;
          hf->data[i][j] = val;
          if (val > hf->high) hf->high = val;
          if (val < hf->low) hf->low = val;
          }
      }
   hf->low -= PLY_EPSILON;
   hf->high += PLY_EPSILON;

   /* If we want to smooth out the height field, then go
      and start calculating normals */
   if (smoothed) {
      hf->type = 1;
      smooth_height_field(hf, ShapeType::Height_Field);
      }
   else {
      hf->type = 0;
      }

   /* Set the data pointer for this object */
   object->o_data = (void *)hf;

   hf->cache_length = 0;
   hf->last_cached  = 0;

   hf->boundbox[0][0] = 0.0;
   hf->boundbox[0][1] = hf->low;
   hf->boundbox[0][2] = 0.0;
   hf->boundbox[1][0] = xsize;
   hf->boundbox[1][1] = hf->high;
   hf->boundbox[1][2] = zsize;

   /* Build a transformation that puts the field where we want it. */
   hf->trans = Get_Transformation().release();
   /* First transform the box so that 0 <= x <= 1 and
      0 <= z <= 1 */
   MakeVector(1.0/(Flt)(xsize - 1), 1.0, 1.0/(Flt)(zsize - 1), tvec);
   Get_Scaling_Transformation_CPP(* hf->trans, tvec);

   /* Now scale back to the size we were told about */
   MakeVector(xhigh-xlow, 1.0, zhigh-zlow, tvec);
   Get_Scaling_Transformation_CPP(trans, tvec);
   Compose_Transformations(* hf->trans, trans);

   /* Finally translate it to the correct position */
   MakeVector(xlow, 0.0, zlow, tvec);
   Get_Translation_Transformation_CPP(trans, tvec);
   Compose_Transformations(* hf->trans, trans);

   MakeVector(xlow, hf->low, zlow-PLY_EPSILON, object->o_bnd.lower_left);
   MakeVector(xhigh-xlow, hf->high-hf->low, zhigh-zlow,
              object->o_bnd.lengths);

   deallocate_node(fn);
   return object;
}

/* Deallocate all of the dynamic storage associated with
   a height field object */
void HeightDelete(Object *object)
{
   HeightData *hf = (HeightData *)object->o_data;
   int i;

   /* Only delete the memory if this is the original */
   if (object->o_copy != 0) return;

   /* Elevation data/rows/storage are vector-backed and released by RAII when
      hf is deleted; only the smoothed-field vertex normals remain on the
      legacy allocator. */
   if (hf->type == 1) {
      /* Smoothed height field: free per-row normals, then the row array. */
      for (i=0;i<hf->zsize;i++)
         polyray_free(hf->norm[i]);
      polyray_free(hf->norm);
      }

   if (hf->phi_sin)
      polyray_free(hf->phi_sin);
   if (hf->phi_cos)
      polyray_free(hf->phi_cos);
   if (hf->theta_norms)
      polyray_free(hf->theta_norms);

   /* Get rid of the transform */
   if (hf->trans)
      delete hf->trans;

   /* Free the height field structure itself */
   delete hf;
}



constexpr int BARYCENTRIC = 1;

/* Intersection of a ray with a triangle using barycentric coordinates.  */
static Flt bary_intersect_triangle(HeightData *hf, Vec D, Vec P, triangle *tri, fVec B[3])
{
   fVec IB[3];
   int ui, vi;
   Flt u, v, d;
   Vec W, N;

   Vec Q;
   Flt a, b, r;

   u = VecDot(tri->N, P) + tri->d;
   v = VecDot(tri->N, D);

   d = -u / v;

   if (d < PLY_EPSILON)
      return PLY_HUGE;

   VecAddScaled(P, d, D, W) /* Point of intersection with triangle */

   /* Try a more computationally expensive method for inside/outside
      testing.  This allows a slop factor to be added. */
   auto ret=InvertMatrix(B);
   //if (!InvertMatrix(B, IB))
   //fVec a1; 
   //a1[0]=ret.out[0][0];
   //IB[0][0]=a1[0];//.. [][1]=[1] etc
   for (int i=0;i<3;i++)
     for (int j=0;j<3;j++)
       IB[i][j]=ret.out[i][j];
      
   if (!ret.i)
     return 0;
   VecSub(W, tri->v1, Q);
   a = VecDot(Q, IB[0]);
   b = VecDot(Q, IB[1]);
   r = 1.0 - a - b;
   if (a > BARY_VAL1 && b > BARY_VAL1 && a + b < BARY_VAL2) {
      if (hf->type == 1) {
         MakeVector(0.0, 0.0, 0.0, N);
         VecAddS(r, tri->n1, N, N);
         VecAddS(a, tri->n2, N, N);
         VecAddS(b, tri->n3, N, N);
         VecNormalize(N);
         VecCopy(N, tri->Ni);
         }
      return d;
      }

   return PLY_HUGE;
}

/* Intersection with a triangle from a height field is really easy,
   because we know that each one is a right triangle that has legs
   of length 1.  For u-v mapping, or smooth height fields, we use the
   point of intersection as a way to interpolate within a single triangle.
   The internal offset within the 1x1 square is:
      (v1[0]-xpos, v1[2]-zpos), or
      (v3[0]-xpos, v1[2]-zpos)
*/
static Flt intersect_triangle(HeightData *hf, Vec D, Vec P, triangle *tri)
{
   Flt u, v, d, x, z;
   Flt i0, i1;
   Vec N;

   u = VecDot(tri->N, P) + tri->d;
   v = VecDot(tri->N, D);

   d = -u / v;

   if (d < PLY_EPSILON)
      return PLY_HUGE;

   x = P[0] + d * D[0];
   z = P[2] + d * D[2];

   if (tri->type == 0 && x >= tri->v1[0] && z >= tri->v1[2] &&
       x + z <= tri->v2[0] + tri->v2[2]) {
      if (hf->type == 1) {
         i0 = x - tri->v1[0];
         i1 = z - tri->v1[2];
         VecComb((1.0 - i0), tri->n1, i0, tri->n2, N);
         VecNormalize(N);
         VecComb((1.0 - i1), N, i1, tri->n3, N);
         VecNormalize(N);
         VecCopy(N, tri->Ni);
         }
      return d;
      }
   if (tri->type == 1 && x <= tri->v2[0] && z <= tri->v2[2] &&
       x + z >= tri->v1[0] + tri->v1[2]) {
      if (hf->type == 1) {
         i0 = x - tri->v3[0];
         i1 = z - tri->v1[2];
         VecComb((1.0 - i0), tri->n3, i0, tri->n2, N);
         VecNormalize(N);
         VecComb(i1, N, (1.0 - i1), tri->n1, N);
         VecNormalize(N);
         VecCopy(N, tri->Ni);
         }
      return d;
      }
   return PLY_HUGE;
}

static triangle *find_cached_triangle(HeightData *hf, 
                     ShapeType hf_type, int x1, int z1,
                     int x2, int z2, int x3, int z3,
                     int triangle_type, fVec B[3])
{
   triangle *tri;
   Flt u, v;
   int i;

   if (hf_type == ShapeType::Height_Field) {
      u = (Flt)x1;
      v = (Flt)z1;
      }
   else if (hf_type == ShapeType::Sph_Height_Field) {
      u = u2theta(hf->xsize, x1);
      v = v2phi(hf->zsize, z1);
      }
   else { /* hf_type == T_CYL_HEIGHT_FIELD */
      u = u2theta(hf->xsize, x1);
      v = z1;
      }

   /* Look for this triangle in the triangle cache */
   for (i=0,tri=&hf->cache[0];i<hf->cache_length;++i,++tri)
      if (tri->type == triangle_type &&
          tri->v1[0] == u) {
         if (hf_type == ShapeType::Height_Field) {
            if (tri->v1[2] == v)
               goto compute_b;
               // return tri;
            }
         else if (tri->v1[1] == v)
            goto compute_b;
            // return tri;
         }

   /* The triangle wasn't cached, so put it there */
   tri = &hf->cache[hf->last_cached];
   hf->last_cached = (hf->last_cached + 1) % MAX_CACHE;
   if (hf->cache_length < MAX_CACHE)
      hf->cache_length++;

   /* Insert all the relevant info for this triangle */
   tri->type = triangle_type;
   if (hf_type == ShapeType::Height_Field) {
      MakeVector(x1, hf->data[z1][x1], z1, tri->v1);
      MakeVector(x2, hf->data[z2][x2], z2, tri->v2);
      MakeVector(x3, hf->data[z3][x3], z3, tri->v3);
      }
   else if (hf_type == ShapeType::Sph_Height_Field) {
      indexed_geo_to_cart(hf, x1, z1, tri->v1);
      indexed_geo_to_cart(hf, x2, z2, tri->v2);
      indexed_geo_to_cart(hf, x3, z3, tri->v3);
      }
   else { /* hf_type == T_CYL_HEIGHT_FIELD */
      indexed_cyl_to_cart(hf, x1, z1, tri->v1);
      indexed_cyl_to_cart(hf, x2, z2, tri->v2);
      indexed_cyl_to_cart(hf, x3, z3, tri->v3);
      }

   if (hf->type == 1) {
      VecCopy(hf->norm[z1][x1], tri->n1);
      VecCopy(hf->norm[z2][x2], tri->n2);
      VecCopy(hf->norm[z3][x3], tri->n3);
      }

compute_b:
   VecSub(tri->v2, tri->v1, B[0]);
   VecSub(tri->v3, tri->v1, B[1]);
   VecCross(B[0], B[1], tri->N);
   if (VecNormalize(tri->N) < PLY_EPSILON)
      return nullptr;
   VecCopy(tri->N, B[2]);
   tri->d = -VecDot(tri->v1, tri->N);

   return tri;
}

/* This routine splits a rectangle from (x,z) -> (x+1,z+1) into
   two right triangles, then checks the ray against them. */
int intersect_square(int x, int z, HeightData *hf, ShapeType hf_type,
                 Vec D, Vec P, triangle *hit_tri)
{
   triangle *tri1, *tri2;
   Flt d1, d2;
   fVec B[3];

   if (x < 0 || x >= hf->xsize-1 || z < 0 || z >= hf->zsize-1)
      return 0;

   if (hf_type == ShapeType::Height_Field) {
      /* Check the ray against the two triangles that make up
         this square. */
      if ( (tri1 = find_cached_triangle(hf, hf_type, x, z, x+1, z, x, z+1, 0, B)))
         d1 = intersect_triangle(hf, D, P, tri1);
      else
         d1 = PLY_HUGE;
      if ( (tri2 = find_cached_triangle(hf, hf_type, x+1, z, x+1, z+1, x, z+1, 1, B)))
         d2 = intersect_triangle(hf, D, P, tri2);
      else
         d2 = PLY_HUGE;
      }
   else {
      /* Check the ray against the two triangles that make up
         this square. */
      if ( (tri1 = find_cached_triangle(hf, hf_type, x, z, x, z+1, x+1, z, 0, B)))
         d1 = bary_intersect_triangle(hf, D, P, tri1, B);
      else
         d1 = PLY_HUGE;
      if ( (tri2 = find_cached_triangle(hf, hf_type, x+1, z, x, z+1, x+1, z+1, 1, B)))
         d2 = bary_intersect_triangle(hf, D, P, tri2, B);
      else
         d2 = PLY_HUGE;
      }

   if (d1 == PLY_HUGE)
      if (d2 == PLY_HUGE)
         return 0;
      else {
         hit_tri[0] = *tri2;
         hit_tri[0].dist = d2;
         return 1;
         }
   else {
      hit_tri[0] = *tri1;
      hit_tri[0].dist = d1;
      if (d2 == PLY_HUGE)
         return 1;
      else {
         hit_tri[1] = *tri2;
         hit_tri[1].dist = d2;
         return 2;
         }
      }
}

int HeightInside(Object *obj, Vec Pos)
{
   triangle tri[2];
   Vec P,P1,D;
   Flt dist;
   int x, z;
   Ensures(obj != nullptr);
   HeightData *hf = (HeightData *)obj->o_data;

   /* It's inside if it is underneath the height field. */
   InvTxVector1(P, Pos, obj->o_trans)
   InvTxVector1(P1, P, hf->trans);

   x = P1[0];
   z = P1[2];
   MakeVector(0, 1, 0, D);
   dist = PLY_HUGE;
   if (P1[1] > hf->data[z  ][x  ] && P1[1] > hf->data[z  ][x+1] &&
       P1[1] > hf->data[z+1][x  ] && P1[1] > hf->data[z+1][x+1])
      return 0;
   else if (P1[1] < hf->data[z  ][x  ] && P1[1] < hf->data[z  ][x+1] &&
            P1[1] < hf->data[z+1][x  ] && P1[1] < hf->data[z+1][x+1])
      return 1;
   else if (intersect_square(x, z, hf, ShapeType::Height_Field, D, P1, tri) > 0)
      return 1;
   else
      return 0;
}

static void Compute_Step_Values(HeightData *hf, Vec D,
                    int *xstep, int *xhigh, float *dx,
                    int *zstep, int *zhigh, float *dz)
{
   int xsize, zsize;

   xsize = hf->xsize-1;
   zsize = hf->zsize-1;

   /* Set the step direction, step size, and exit condition
      for the x-axis */
   if (D[0] < 0.0) {
      *xstep = -1;
      *xhigh = -1;
      *dx    = -1.0 / D[0];
      }
   else if (D[0] > 0.0) {
      *xstep = 1;
      *xhigh = xsize;
      *dx    = 1.0 / D[0];
      }
   else {
      *xstep = 0;
      *xhigh = -1;
      *dx    = 0.0;
      }

   /* Set the step direction, step size, and exit condition
      for the z-axis */
   if (D[2] < 0.0) {
      *zstep = -1;
      *zhigh = -1;
      *dz    = -1.0 / D[2];
      }
   else if (D[2] > 0.0) {
      *zstep = 1;
      *zhigh = zsize;
      *dz    = 1.0 / D[2];
      }
   else {
      *zstep = 0;
      *zhigh = -1;
      *dz    = 0.0;
      }
}

static void Compute_DDA_Start(HeightData *hf, Vec hitpos, Vec D,
                  int *x, Flt *fx, int *z, Flt *fz)
{
   int xsize, zsize;

   xsize = hf->xsize - 1;
   zsize = hf->zsize - 1;

   /* Figure out what pixel we are starting at */
   *x = hitpos[0];
   if (*x >= xsize) *x = xsize-1;
   *z = hitpos[2];
   if (*z >= zsize) *z = zsize-1;

   /* Set the initial difference value along the x-axis */
   if (D[0] < 0.0)
      *fx = (*x - hitpos[0]) / D[0];
   else if (D[0] > 0.0)
      *fx = ((*x+1) - hitpos[0]) / D[0];
   else
      *fx = PLY_HUGE;

   /* Set the initial difference value along the x-axis */
   if (D[2] < 0.0)
      *fz = (*z - hitpos[2]) / D[2];
   else if (D[2] > 0.0)
      *fz = ((*z+1) - hitpos[2]) / D[2];
   else
      *fz = PLY_HUGE;
}

static int Determine_Hits(Object *obj, Ray *ray, Flt t, Isect *hit,
               Flt mindist, Flt maxdist,
               int x, int z, Vec P, Vec D, Flt ty0, Flt ty1)
{
   Vec N, W, U;
   Flt dist;
   triangle triangs[2];
   int i, cnt, Flag = 0;
   HeightData *hf = (HeightData *)obj->o_data;
   Flt y0, y1;

   y0 = ty0 - PLY_EPSILON;
   y1 = ty1 + PLY_EPSILON;

   if ((y0 > hf->data[z  ][x  ] && y0 > hf->data[z  ][x+1] &&
        y0 > hf->data[z+1][x  ] && y0 > hf->data[z+1][x+1]) ||
       (y1 < hf->data[z  ][x  ] && y1 < hf->data[z  ][x+1] &&
        y1 < hf->data[z+1][x  ] && y1 < hf->data[z+1][x+1]))
      return 0;

   cnt = intersect_square(x, z, hf, ShapeType::Height_Field, D, P, 
      triangs);

   for (i=0;i<cnt;i++) {
      dist = triangs[i].dist / t;
      if (dist > mindist && dist < maxdist) {
         VecAddScaled(ray->P, dist, ray->D, W);
         InvTxVector1(U, W, hf->trans);
         U[0] = U[0] / (Flt)(hf->xsize - 1);
         U[1] = U[2] / (Flt)(hf->zsize - 1);
         U[2] = 0.0;
         if (hf->type == 0)
            TxNormal(N, triangs[i].N, hf->trans);
         else
            TxNormal(N, triangs[i].Ni, hf->trans);
         if (Insert_Hit(obj, W, N, dist, U, hit))
            Flag = 1;
         }
      }

   return Flag;
}

/* Find the place where the ray intersects the height field.  It
   is assumed that the ray has already been transformed into object
   space before this routine is called.  Currently only the first
   hit will be returned - the DDA loop is the only thing that needs
   to be modified if more are desired. */
int HeightIntersect(Viewpoint *Eye, Object *obj, Ray *ray,
                Flt mindist, Flt maxdist, Isect *hit)
{
   Vec hitpos;
   Flt mind, maxd;
   int x, z, xhigh, zhigh, up_flag;
   int xstep, zstep, Flag;
   float dx, dz;
   Flt t, fx, fz, y0, y1;
   Vec nxp, nzp, pDX, pDZ;
   Vec P, D;
   HeightData *hf = (HeightData*)obj->o_data;

   /* Transform the ray into height field space */
   InvTxVector1(P, ray->P, hf->trans);
   InvTxDirection(D, ray->D, hf->trans);
   t = VecNormalize(D);

   /* Find where we hit the bounding box around the height field */
   mind = t * mindist;
   maxd = t * maxdist;
   if (determine_start(P, D, hf->boundbox, &mind, &maxd))
      VecAddScaled(P, mind, D, hitpos)
   else
      return 0;

   Compute_Step_Values(hf, D, &xstep, &xhigh, &dx, &zstep, &zhigh, &dz);

   VecCopy(D, pDX);
   VecScale(dx, pDX);
   VecCopy(D, pDZ);
   VecScale(dz, pDZ);

   /* Are we going up or down? */
   up_flag = (D[1] > 0.0);

   /* Figure out the starting conditions on the DDA variables */
   Compute_DDA_Start(hf, hitpos, D, &x, &fx, &z, &fz);
   VecAddScaled(hitpos, fx, D, nxp);
   VecAddScaled(hitpos, fz, D, nzp);

   Flag = 0;

   /* Now that all of the information has been set up,
      lets do the DDA. */
   for (;;) {
      if (fx < fz) {
         if (up_flag) { y0 = hitpos[1]; y1 = nxp[1]; }
         else { y0 = nxp[1]; y1 = hitpos[1]; }

         if (Determine_Hits(obj, ray, t, hit, mindist, maxdist,
                            x, z, P, D, y0, y1)) {
            Flag = 1;
            break;
            }

         x += xstep;
         if (x == xhigh) break;
         fx += dx;
         VecCopy(nxp, hitpos);
         VecAdd(nxp, pDX, nxp);
         }
      else {
         if (up_flag) { y0 = hitpos[1]; y1 = nzp[1]; }
         else { y0 = nzp[1]; y1 = hitpos[1]; }

         if (Determine_Hits(obj, ray, t, hit, mindist, maxdist,
                            x, z, P, D, y0, y1)) {
            Flag = 1;
            break;
            }

         z += zstep;
         if (z == zhigh) break;
         fz += dz;
         VecCopy(nzp, hitpos);
         VecAdd(nzp, pDZ, nzp);
         }

      /* Have we walked outside the bounds of the height field? */
      if (( up_flag && hitpos[1] > hf->high) ||
          (!up_flag && hitpos[1] < hf->low))
         break;
      }
   return Flag;
}

void HeightRender(Viewpoint *eye, BinTree *Root, Object *obj)
{
   int i, j, k, u, v;
   Flt x, z;
   Vec P[3], N[3], U[3], t0, t1;
   HeightData *hf = (HeightData *)obj->o_data;
   Poly Polygon;
   Object *tobj;

   u = hf->xsize;
   v = hf->zsize;
   for (i=0;i<v-1;i++) {
      if (RuntimeState::Check_Abort_Flag > 0 && _kbhit()) {
#if defined( __APPLE__ )
          RuntimeState::Abort_Flag = 1;
#else
#if defined( __linux__ )|| defined(__sun) || defined(__sun__) || defined(__illumos__)
          RuntimeState::Abort_Flag = 1;
#else
          RuntimeState::Abort_Flag = _getch();
#endif
#endif
         break;
         }
      z = (Flt)i;
      for (j=0;j<u-1;j++) {
         x = (Flt)j;

         MakeVector(x,     hf->data[i][j],   z,     P[0]);
         MakeVector(x+1.0, hf->data[i][j+1], z,     P[1]);
         MakeVector(x,     hf->data[i+1][j], z+1.0, P[2]);

         Polygon.n = 3;
         if (hf->type == 0) {
            VecSub(P[2], P[0], t0);
            VecSub(P[1], P[0], t1);
            VecCross(t0, t1, N[0]);
            VecCopy(N[0], N[1]);
            VecCopy(N[0], N[2]);
            }
         else {
            VecCopy(hf->norm[i][j],     N[0]);
            VecCopy(hf->norm[i][j+1],   N[1]);
            VecCopy(hf->norm[i+1][j],   N[2]);
            }

         for (k=0;k<3;k++) {
            MakeVector(P[k][0] / (Flt)(u - 1),
                       P[k][2] / (Flt)(v - 1), 0, U[k]);
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
            VecCopy(U[k], Polygon.vertices[k].U);
            VecCopy(P[k], Polygon.vertices[k].W);
            VecCopy(N[k], Polygon.vertices[k].N);
            }

         scan_convert(eye, Root, obj, nullptr, &Polygon);

         MakeVector(x+1.0, hf->data[i][j+1],   z,     P[0]);
         MakeVector(x+1.0, hf->data[i+1][j+1], z+1.0, P[1]);
         MakeVector(x,     hf->data[i+1][j],   z+1.0, P[2]);

         Polygon.n = 3;

         if (hf->type == 0) {
            VecSub(P[2], P[0], t0);
            VecSub(P[1], P[0], t1);
            VecCross(t0, t1, N[0]);
            VecCopy(N[0], N[1]);
            VecCopy(N[0], N[2]);
            }
         else {
            VecCopy(hf->norm[i][j+1],   N[0]);
            VecCopy(hf->norm[i+1][j+1], N[1]);
            VecCopy(hf->norm[i+1][j],   N[2]);
            }

         for (k=0;k<3;k++) {
            MakeVector(P[k][0] / (Flt)(u - 1),
                       P[k][2] / (Flt)(v - 1), 0, U[k]);
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
            VecCopy(U[k], Polygon.vertices[k].U);
            VecCopy(P[k], Polygon.vertices[k].W);
            VecCopy(N[k], Polygon.vertices[k].N);
            }
         scan_convert(eye, Root, obj, nullptr, &Polygon);
         }
      }
}
