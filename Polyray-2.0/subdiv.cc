/* subdiv.cc

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

#include <vector>

#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "bound.h"
#include "scan.h"
#include "vector.h"
#include "eval.h"
#include "runtime_state.h"
#include "symtab.h"
#include "triangle.h"
#include "subdiv.h"
#include "polygonsplitcontext.h"
#include "factory.h"
#include "util.h"

//readded this for now
#define VERT(i, a) ((verts[vbuffer[i]])[a])



// The original functions become member methods

/* Turn a (possibly cocave) polygon into a set of triangles.  The
   vertex list out_verts must already be sufficiently large to
   accomodate (cnt - 2) triangles.  Each triangle is returned as
   a set of three vertex indices. */
   /** @brief Split a polygon into triangles using the legacy in-place splitter.
    *  @param cnt       Number of polygon vertices.
    *  @param verts     Vertex array.
    *  @param x_axis    Projection axis used as x during splitting.
    *  @param y_axis    Projection axis used as y during splitting.
    *  @param out_cnt   Output triangle count.
    *  @param out_verts Output triangle-index array.
    */
void Split_Polygon(int cnt, fVec* verts, int x_axis, int y_axis,
    int& out_cnt, int** out_verts)
{
    //    convert fVec* verts to std::vector<fVect>
    out_cnt = 0; // Set output count to 0 for now, will be updated by the splitter. no triangles to start with
    //std::vector<fVec> fv;
    
    openpolyray::PolygonSplitContext splitter(verts, x_axis, y_axis, cnt);
    splitter.split(cnt, out_cnt, out_verts); //void split(const std::vector<fVec>&input_data, int& out_cnt, int** out_verts) 

}

/* Turn a (possibly cocave) polygon into a set of triangles.  The
   vertex list out_verts must already be sufficiently large to
   accomodate (cnt - 2) triangles.  Each triangle is returned as
   a set of three vertex indices. */
   /** @brief Split a polygon into triangles using the legacy in-place splitter.
    *  @param cnt       Number of polygon vertices.
    *  @param verts     Vertex array.
    *  @param x_axis    Projection axis used as x during splitting.
    *  @param y_axis    Projection axis used as y during splitting.
    *  @param out_cnt   Output triangle count.
    *  @param out_verts Output triangle-index array.
    */
void cpp_Split_Polygon(int cnt, fVec* verts, int x_axis, int y_axis,
    int& out_cnt, std::vector<std::array<int, 3>>& out_verts)
{
    //    convert fVec* verts to std::vector<fVect>
    out_cnt = 0; // Set output count to 0 for now, will be updated by the splitter. no triangles to start with
    //std::vector<fVec> fv;

    openpolyray::PolygonSplitContext splitter(verts, x_axis, y_axis, cnt);
    splitter.cpp_split(cnt, out_cnt, out_verts); //void split(const std::vector<fVec>&input_data, int& out_cnt, int** out_verts) 

}

//old code is still left here for testing purposes (Old_ prefix)
#define VERT(i, a) ((verts[vbuffer[i]])[a])

/* Storage for polygon indices */
static int* vbuffer = nullptr;
static int* poly_end = nullptr;
static int axis1, axis2;

/** @brief Find the left most vertex in the polygon that has vertices m ... n. */
static int Old_leftmost_vertex(int m, int n, fVec* verts)
{
    int l, i;
    double x;

    /* Assume the first vertex is the farthest to the left */
    l = m;
    x = VERT(m, axis1);

    /* Now see if any of the others are farther to the left */
    for (i = m + 1;i <= n;i++)
        if (VERT(i, axis1) < x) {
            l = i;
            x = VERT(i, axis1);
        }
    return l;
}
/** @brief Given the leftmost vertex in a polygon, this routine finds another vertex
   can be used to safely split the polygon. */
static int Old_split_vertex(int l, int la, int lb, int m, int n, fVec* verts)
{
    int t, k, lpu, lpl;
    double yu, yl;

    yu = MAX(VERT(l, axis2), MAX(VERT(la, axis2), VERT(lb, axis2)));
    yl = PLY_MIN(VERT(l, axis2), PLY_MIN(VERT(la, axis2), VERT(lb, axis2)));
    if (VERT(lb, axis2) > VERT(la, axis2)) {
        lpu = lb;
        lpl = la;
    }
    else {
        lpu = la;
        lpl = lb;
    }
    t = (VERT(lb, axis1) > VERT(la, axis1) ? lb : la);
    /* Should this be: k=m;k<=n;k++? (was k<n) */
    for (k = m;k <= n;k++)
        if (k != la && k != l && k != lb)
            if (VERT(k, axis2) <= yu &&
                VERT(k, axis2) >= yl &&
                VERT(k, axis1) < VERT(t, axis1) &&

                ((VERT(k, axis2) - VERT(l, axis2)) *
                    (VERT(lpu, axis1) - VERT(l, axis1))) <=
                ((VERT(lpu, axis2) - VERT(l, axis2)) *
                    (VERT(k, axis1) - VERT(l, axis1))) &&

                ((VERT(k, axis2) - VERT(l, axis2)) *
                    (VERT(lpl, axis1) - VERT(l, axis1))) >=
                ((VERT(lpl, axis2) - VERT(l, axis2)) *
                    (VERT(k, axis1) - VERT(l, axis1))))
                t = k;
    return t;
}
/** @brief Test polygon vertices to see if they are linear */
static int Old_linear_vertices(int m, int n, fVec* verts)
{
    /* Not doing anything right now */
    return 0;
}
/** @brief Shift vertex indices around to make two polygons out of one. */
static void Old_perform_split(int m, int m1, int n, int n1)
{
    int i, j, k;
    int* vb1, * vb2;

    k = n + 3 - m1;
    /* Move the new polygon up over the place the current one sits */
    for (j = m1, vb1 = &vbuffer[m1 + k], vb2 = &vbuffer[m1];
        j <= n1;j++, vb1++, vb2++)
        *vb1 = *vb2;

    /* Move top part of remaining polygon */
    for (j = n, vb1 = &vbuffer[n + 2], vb2 = &vbuffer[n];
        j >= n1;j--, vb1--, vb2--)
        *vb1 = *vb2;

    /* Move bottom part of remaining polygon */
    k = n1 - m1 + 1;
    for (j = m1, vb1 = &vbuffer[m1 + k], vb2 = &vbuffer[m1];
        j >= m;j--, vb1--, vb2--)
        *vb1 = *vb2;

    /* Copy the new polygon so that it sits before the remaining polygon */
    i = n + 3 - m1;
    k = m - m1;
    for (j = m1, vb1 = &vbuffer[m1 + k], vb2 = &vbuffer[m1 + i];
        j <= n1;j++, vb1++, vb2++)
        *vb1 = *vb2;
}
/** @brief Copy an indirectly referenced triangle into the output triangle buffer */
static void Old_add_new_triangle(int m, int* out_cnt, int** out_verts)
{
    int i;

    if (out_verts != NULL) {
        /* printf("New: %d, %d, %d\n", vbuffer[m], vbuffer[m+1], vbuffer[m+2]); */
        i = *out_cnt;
        out_verts[i][0] = vbuffer[m];
        out_verts[i][1] = vbuffer[m + 1];
        out_verts[i][2] = vbuffer[m + 2];
        *out_cnt += 1;
    }
}

/** @brief Turn a (possibly cocave) polygon into a set of triangles.  The
   vertex list out_verts must already be sufficiently large to
   accomodate (cnt - 2) triangles.  Each triangle is returned as
   a set of three vertex indices. */
void Old_Split_Polygon(int cnt, fVec* verts, int x_axis, int y_axis,
    int* out_cnt, int** out_verts)
{
    int i, m, m1, n, n1;
    int l, la, lb, ls;

    /* Make sure there is storage for intermediate vertices */
    vbuffer = (int*)malloc(3 * cnt * sizeof(int));
    if (cnt < 2) serror("Split_Polygon:cnt must be at least 2");//cm, avoid buffer overrun

    poly_end = (int*)malloc(cnt * sizeof(int));
    if (vbuffer == NULL || poly_end == NULL)
        serror("Failed to allocate glyph polygon buffer");

    /* Set the coordinates that we will perform the splitting on. */
    axis1 = x_axis;
    axis2 = y_axis;

    /* No triangles to start with */
    *out_cnt = 0;

    /* Initialize the polygon splitter */
    poly_end[0] = -1;
    poly_end[1] = cnt - 1;

    /* Start with a strict identity of vertices in verts and vertices in
       the polygon buffer */
    for (i = 0;i < cnt;i++)
        vbuffer[i] = i;

    /* Split and push polygons until they turn into triangles */
    for (i = 1;i > 0;) {
        m = poly_end[i - 1] + 1;
        n = poly_end[i];

        if (n - m == 2) {
            if (!Old_linear_vertices(m, n, verts))
                Old_add_new_triangle(m, out_cnt, out_verts);
            i = i - 1;
        }
        else {
            l = Old_leftmost_vertex(m, n, verts);
            la = (l == n ? m : l + 1);
            lb = (l     == m ? n : l - 1);
            ls = Old_split_vertex(l, la, lb, m, n, verts);
            if (ls == la || ls == lb) {
                m1 = (la < lb ? la : lb);
                n1 = (la < lb ? lb : la);
            }
            else {
                m1 = (l < ls ? l : ls);
                n1 = (l < ls ? ls : l);
            }
            Old_perform_split(m, m1, n, n1);
            poly_end[i++] = m + n1 - m1;
            poly_end[i] = n + 2;

        }
    }

    /* Free up the temporary stacks used for polygon decomposition */
    free(vbuffer);
    vbuffer = NULL;
    free(poly_end);
    poly_end = NULL;
}



/* Shift vertex indices around to make two polygons out of one. */
/** @brief Rearrange the temporary polygon buffer after choosing a split.
 *  @param m   Start index of the current polygon.
 *  @param m1  Start index of the split span.
 *  @param n   End index of the current polygon.
 *  @param n1  End index of the split span.
 */
static void perform_split(int m, int m1, int n, int n1)
{
    throw std::runtime_error("NO PERFORM SPLIT");
}

/* Copy an indirectly referenced triangle into the output triangle buffer */
#ifndef TESTING
static
#endif
/** @brief Copy a triangle from the temporary polygon buffer into the caller's output array. Only here for testing purposes.
 *  @param m         Start index of the triangle in the temporary buffer.
 *  @param out_cnt   Output triangle count updated in place.
 *  @param out_verts Output triangle-index array.
 */
   void add_new_triangle(int m, int* out_cnt, int** out_verts)
{
    *out_cnt = 0;
    int cnt = 1;
    fVec* vert = (fVec*)polyray_malloc(sizeof(fVec) * cnt);
    vert[0][0] = 0.0f; vert[0][1] = 0.0f; vert[0][2] = 0.0f; // fVec is Flt[3]: indices 0-2 only
    int x_axis = 0;
    int y_axis = 1;
    openpolyray::PolygonSplitContext splitter(vert, x_axis, y_axis, cnt);
    splitter.split(cnt, *out_cnt, out_verts); // populates vbuffer, then emits triangles via add_new_triangle internally
    polyray_free(vert);
}
    







#ifndef TESTING
static
#endif
/** @brief Apply a displacement expression to a sampled vertex.
 *  @param displacement Displacement expression node.
 *  @param vert         Vertex updated in place.
 */
void displace_vertex(NODE_PTR displacement, Vertex* vert)
{
   Vec vtemp;
   Flt ftemp;
   NODE_PTR ntemp;
   int t;
   struct subst_struct subst;

   /* Build a substitution structure to evaluate the special texture with */
   VecCopy(vert->P, subst.P);
   MakeVector(0, 0, 0, subst.PT);
   VecCopy(vert->U, subst.U);
   VecCopy(vert->W, subst.W);
   VecCopy(vert->N, subst.N);
   MakeVector(0, 0, 1, subst.I);

   if ((t = eval_node(&subst, displacement, &ftemp, vtemp, &ntemp)) == 2)
      VecAdd(vert->W, vtemp, vert->W)
   else if (t == 1)
      VecAddScaled(vert->W, ftemp, subst.N, vert->W)
   else
      serror("Bad displacement function\n");
}

#ifndef TESTING
static
#endif
/** @brief Recompute all the vertex normals of a polygon by averaging its face normal.
 *  @param p Polygon whose normals are updated in place.
 */
void average_normals(Poly& p)
{
   if (p.n < 3) return;
   Vec v0, v1, v2;
   VecSub(p.vertices[1].W, p.vertices[0].W, v0);
   VecSub(p.vertices[2].W, p.vertices[0].W, v1);
   VecCross(v1, v0, v2);

   // if we were to check for collinearity here 
   // const Flt area2 = VecDot(v2, v2);
   //const bool collinear = area2 <= PLY_EPSILON * PLY_EPSILON;
   // but it's not the place for it

   VecNormalize(v2);
   for (int i=0;i<p.n;i++) {
      p.vertices[i].N[0] += 0.5 * v2[0];
      p.vertices[i].N[1] += 0.5 * v2[1];
      p.vertices[i].N[2] += 0.5 * v2[2];
      fVecNormalize(p.vertices[i].N);
      }
   //if (--i != p.n - 1) serror("theres an error with i in average_normals, it should be n-1=%d but is %d\n", p.n-1, i);
   
}

#ifndef TESTING
static
#endif
/** @brief Accumulate one mesh-face normal contribution into a vertex normal.
 *  @param V      Vertex-position grid.
 *  @param usteps Number of horizontal subdivisions.
 *  @param vsteps Number of vertical subdivisions.
 *  @param i0     First vertex u index.
 *  @param j0     First vertex v index.
 *  @param i1     Second vertex u index.
 *  @param j1     Second vertex v index.
 *  @param i2     Third vertex u index.
 *  @param j2     Third vertex v index.
 *  @param N      Accumulated normal vector updated in place.
 *  @return 1 when the triangle indices are valid and a normal is added, 0 otherwise.
 */
int add_mesh_normal(fVec* V, int usteps, int vsteps, int i0, int j0, int i1, int j1, int i2, int j2, Vec N)
{
   Vec v0, v1, v2, t0, t1, Nt;

   if (i0 < 0 || j0 < 0 ||
       i1 < 0 || j1 < 0 ||
       i2 < 0 || j2 < 0 ||
       i0 > usteps || j0 > vsteps ||
       i1 > usteps || j1 > vsteps ||
       i2 > usteps || j2 > vsteps) {
      return 0;
      }
   else {
      VecCopy(V[i0 * (vsteps + 1) + j0], v0);
      VecCopy(V[i1 * (vsteps + 1) + j1], v1);
      VecCopy(V[i2 * (vsteps + 1) + j2], v2);
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
#ifndef TESTING
static
#endif
/** @brief Generate smoothed vertex normals across a regular mesh grid.
 *  @param V      Vertex-position grid.
 *  @param N      Vertex-normal grid updated in place.
 *  @param usteps Number of horizontal subdivisions.
 *  @param vsteps Number of vertical subdivisions.
 */
void smooth_mesh(fVec* V, fVec* N, int usteps, int vsteps)
{
   /* For now we will do it the hard way - by generating the normals
      individually for each elevation point */
   for (int i=0;i<=usteps;i++) {
      for (int j=0;j<=vsteps;j++) {
		 const int vertex_flat_index = i * (vsteps + 1) + j;
         Vec accumulator;
         VecCopy(N[vertex_flat_index], accumulator);
         int count = 1;
         count += add_mesh_normal(V, usteps, vsteps, j, i, j-1, i, j-1, i-1, accumulator);
         count += add_mesh_normal(V, usteps, vsteps, j, i, j-1, i-1, j, i-1, accumulator);
         count += add_mesh_normal(V, usteps, vsteps, j, i, j, i-1, j+1, i, accumulator);
         count += add_mesh_normal(V, usteps, vsteps, j, i, j+1, i, j+1, i+1, accumulator);
         count += add_mesh_normal(V, usteps, vsteps, j, i, j+1, i+1, j, i+1, accumulator);
         count += add_mesh_normal(V, usteps, vsteps, j, i, j, i+1, j-1, i, accumulator);         
         VecScale(1.0 / static_cast<Flt>(count), accumulator);
         VecCopy(accumulator, N[vertex_flat_index]);
		 fVecNormalize(N[vertex_flat_index]);
         }
      }
}

#ifndef TESTING
static
#endif
/** @brief HandleTSphereUVSteps. */
void HandleTSphereUVSteps(int size, int& u_steps, int& v_steps)
{
    if (size < 8) {
        u_steps = 4;
        v_steps = 3;
    }
    else if (size < 16) {
        u_steps = 6;
        v_steps = 4;
    }
    else if (size < 32) {
        u_steps = 8;
        v_steps = 6;
    }
    else if (size < 64) {
        u_steps = 12;
        v_steps = 8;
    }
    else if (size < 128) {
        u_steps = 16;
        v_steps = 8;
    }
    else if (size < 256) {
        u_steps = 32;
        v_steps = 16;
    }
    else {
        u_steps = 48;
        v_steps = 24;
    }
}

#ifndef TESTING
static 
#endif
/** @brief HandleTDiscConeCylinderUVSteps. */
void HandleTDiscConeCylinderUVSteps(int size, int& u_steps, int& v_steps)
{
    if (size < 8) {
        u_steps = 4;
        v_steps = 1;
    }
    else if (size < 16) {
        u_steps = 6;
        v_steps = 1;
    }
    else if (size < 32) {
        u_steps = 8;
        v_steps = 1;
    }
    else if (size < 64) {
        u_steps = 12;
        v_steps = 1;
    }
    else if (size < 128) {
        u_steps = 16;
        v_steps = 2;
    }
    else if (size < 256) {
        u_steps = 32;
        v_steps = 4;
    }
    else {
        u_steps = 48;
        v_steps = 4;
    }
}


/** @brief Recompute UV tessellation density adaptively from projected object size.
 *  @param obj    Object whose adaptive UV settings are consulted.
 *  @param width  Projected screen-space width.
 *  @param height Projected screen-space height.
 *  @return A `retUVSteps` result containing the selected subdivision counts
 *          and whether adaptive UV steps were assigned.
 */
#ifndef TESTING
static
#endif
retUVSteps recompute_uv_steps(Object& obj, int width, int height)
{
   int size;
   retUVSteps ret{ 0 };
   if (!(obj.o_sflag & ADAPTIVE_UV))
      return ret;//0

   size = MAX(width, height);
   switch (obj.o_type) {
      case ShapeType::Sphere:
          HandleTSphereUVSteps(size, ret.u_steps, ret.v_steps);
          ret.adaptiveUVstepsassigned = 1;		  
          return ret;
          break;
      case ShapeType::Disc:
      case ShapeType::Cone:
      case ShapeType::Cylinder:
          HandleTDiscConeCylinderUVSteps(size, ret.u_steps, ret.v_steps);
          ret.adaptiveUVstepsassigned = 1;
          return ret;
          break;
      default:
          return ret;//0
         }
}

/* Subdivide a u-v surface to an exact value */
/** @brief Uniformly tessellate a parametric surface object into quads or mesh triangles.
 *  @param eye  Active viewpoint.
 *  @param Root Scene or mesh output tree.
 *  @param obj  Parametric surface object to subdivide.
 */
void Uniform_Subdivide(Viewpoint *eye, BinTree *Root, Object *obj)
{
   #ifdef DEBUG_FN_CALLS
   smessage("subdiv::Uniform_Subdivide\n");
   #endif
   Object *tobj;
   TriangleObject *obj1, *obj2;
   Vertex Vert;
   Vertex *vertex_row1=nullptr, *vertex_row2=nullptr, *temp_row;
   fVec *V, *N, *U;
   int i, j, displace_flag, internal_abort;
   long n, k;
   int u_steps, v_steps, width, height;
   Flt u, v, u_low, u_high, v_low, v_high;
   Flt delta_u, delta_v;
   bbox_info box;
   Poly P;
   Poly *Polygon = &P;

   if ((runtimeState::Check_Abort_Flag == 1) && _kbhit())
       throw std::runtime_error("Aborted");

   u_steps = obj->o_uv_steps[0];
   v_steps = obj->o_uv_steps[1];

   if (obj->o_uv_bounds[0] == -PLY_HUGE &&
       obj->o_uv_bounds[1] ==  PLY_HUGE &&
       obj->o_uv_bounds[2] == -PLY_HUGE &&
       obj->o_uv_bounds[3] ==  PLY_HUGE) {
      u_low  = 0.0;
      u_high = 1.0;
      v_low  = 0.0;
      v_high = 1.0;
      }
   else {
      u_low  = obj->o_uv_bounds[0];
      u_high = obj->o_uv_bounds[1];
      v_low  = obj->o_uv_bounds[2];
      v_high = obj->o_uv_bounds[3];
      }
   delta_u = (u_high - u_low) / (Flt)u_steps;
   delta_v = (v_high - v_low) / (Flt)v_steps;

   displace_flag = 0;

   if (runtimeState::settings.Render_Method == rmode::MESH_CONVERSION) {
      /* Store the computed information in a more compact form and generate
         the triangle objects associated with the parent object */
      obj->o_vertices = (ObjectVertices *)
                        polyray_malloc(sizeof(ObjectVertices));
      n = (long)(u_steps + 1) * (long)(v_steps + 1);
      V = (fVec *)polyray_malloc(n * sizeof(fVec));
      U = (fVec *)polyray_malloc(n * sizeof(fVec));
      if (V == NULL || U == NULL)
         serror("Out of polygon memory");
      if (obj->o_sflag & SMOOTH_FLAG) {
         N = (fVec *)polyray_malloc(n * sizeof(fVec));
         if (N == nullptr)
            serror("Out of polygon memory");
         }
      else
         N = nullptr;

      /* Store the vertex information */
      for (i=0,k=0,u=u_low;i<=u_steps;i++,u+=delta_u) {
         for (j=0,v=v_low;j<=v_steps;j++,k++,v+=delta_v) {
            obj->o_procs->evaluate(obj, u, v, &Vert);
            for (tobj=obj;tobj!=nullptr;tobj=tobj->o_parent) {
               if (tobj->o_displace) {
                  /* A displacement was found, apply displacement mapping */
                  displace_vertex(tobj->o_displace, &Vert);
                  displace_flag = 1;
                  }
               }
            VecCopy(Vert.W, V[k]);
            if (N != nullptr) VecCopy(Vert.N, N[k]);
            if (U != nullptr) VecCopy(Vert.U, U[k]);
            }
         }

      /* Put the vertex information into the parent object */
      obj->o_vertices->n = n;
      obj->o_vertices->V = V;
      obj->o_vertices->N = N;
      obj->o_vertices->U = U;

      /* Perform normal averaging on a displaced surface */
      if (displace_flag && (N != NULL))
         smooth_mesh(V, N, u_steps, v_steps);

      /* Generate the individual triangle objects */
      for (i=0,k=0,u=u_low;i<u_steps;i++,u+=delta_u) {
         for (j=0,v=v_low;j<v_steps;j++,k++,v+=delta_v) {
            obj1 = FactoryTriangleObject();
            obj2 = FactoryTriangleObject();
            //if (obj1 == nullptr || obj2 == nullptr)
            //   serror("Insufficient polygon memory");

            /* Initialize the first triangle object */
            obj1->o_type = ShapeType::Polygon;
            obj1->o_parent = obj;
            obj1->o_vert[0] = i * (v_steps + 1) + j;
            obj1->o_vert[1] = i * (v_steps + 1) + (j + 1);
            obj1->o_vert[2] = (i + 1) * (v_steps + 1) + (j + 1);
            obj1->o_texture = obj->o_texture;
            if (calc_triangle_bounds(*obj1, &box)) {
               VecCopy(box.lower_left, obj1->o_bnd.lower_left)
               VecCopy(box.lengths, obj1->o_bnd.lengths)
               Root->members.list = push_object(Root->members.list,
                                                (Object *)obj1);
               //printf("obj1->o_bnd.lower_left: %f %f %f\n", obj1->o_bnd.lower_left[0], obj1->o_bnd.lower_left[1], obj1->o_bnd.lower_left[2]);          
               Root->members.count++;
               }
            else
               delete obj1;

            /* Initialize second triangle object */
            obj2->o_type = ShapeType::Polygon;
            obj2->o_parent = obj;
            obj2->o_vert[0] = i * (v_steps + 1) + j;
            obj2->o_vert[1] = (i + 1) * (v_steps + 1) + (j + 1);
            obj2->o_vert[2] = (i + 1) * (v_steps + 1) + j;
            obj2->o_texture = obj->o_texture;
            if (calc_triangle_bounds(*obj2, &box)) {
               VecCopy(box.lower_left, obj2->o_bnd.lower_left)
               VecCopy(box.lengths, obj2->o_bnd.lengths)
               Root->members.list = push_object(Root->members.list,
                                                (Object *)obj2);
               Root->members.count++;
               }
            else
               delete obj2;
            }
         }
      }
   else {      

      /* Since we may abort rendering somewhere within this
         next loop, we want to be able to catch the abort and
         undo the memory allocation */
      try {
         /* See if the object is visible on the screen */
         BboxScreenSize(eye, &obj->o_bnd, &width, &height);
         if (width <= 0 || height <= 0)
            return;

         /* See if we are using an adaptive step size */
         auto ret = recompute_uv_steps(*obj, width, height);
         u_steps = ret.u_steps;
		 v_steps = ret.v_steps;

         if (ret.adaptiveUVstepsassigned) {
            delta_u = (u_high - u_low) / (Flt)u_steps;
            delta_v = (v_high - v_low) / (Flt)v_steps;
            }

         vertex_row1 = (Vertex *)polyray_malloc((v_steps+1) * sizeof(Vertex));
         vertex_row2 = (Vertex *)polyray_malloc((v_steps+1) * sizeof(Vertex));
         if (vertex_row1 == nullptr || vertex_row2 == nullptr) {
             serror("Failed to allocate mesh memory"); exit(-1);
         }

         /* Evaluate the surface over the entire mesh */
         for (i=0,u=u_low;i<=u_steps;i++,u+=delta_u) {
            for (j=0,v=v_low;j<=v_steps;j++,v+=delta_v) {
               obj->o_procs->evaluate(obj, u, v, &vertex_row2[j]);
               for (tobj=obj;tobj!=NULL;tobj=tobj->o_parent) {
                  if (tobj->o_displace != NULL) {
                     /* A displacement was found, apply displacement mapping */
                     displace_vertex(tobj->o_displace, &vertex_row2[j]);
                     displace_flag = 1;
                     }
                  }
               }
            if (i > 0) {
               /* Now have at least two rows of vertices, we can go render
                  the current strip */
               for (j=0,v=v_low;j<v_steps;j++,v+=delta_v) {
                  Polygon->n = 4;
                  memcpy(&Polygon->vertices[0], &vertex_row1[j  ], sizeof(Vertex));
                  memcpy(&Polygon->vertices[1], &vertex_row1[j+1], sizeof(Vertex));
                  memcpy(&Polygon->vertices[2], &vertex_row2[j+1], sizeof(Vertex));
                  memcpy(&Polygon->vertices[3], &vertex_row2[j  ], sizeof(Vertex));
                  if (displace_flag)
                     average_normals(*Polygon);
                  scan_convert(eye, Root, obj, NULL, Polygon);
                  }
               }
            temp_row    = vertex_row2;
            vertex_row2 = vertex_row1;
            vertex_row1 = temp_row;
            }
         internal_abort = 0;
         }
      catch (const std::exception& e) {
         internal_abort = 1;
      }      
      polyray_free(vertex_row1);
      polyray_free(vertex_row2);
      if (internal_abort)
          throw std::runtime_error("Aborted");
      }
}
