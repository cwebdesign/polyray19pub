/** @file triangle.cc
 *  @brief Polygon-to-triangle decomposition for the openpolyray namespace.
 *
 *  Implements the Triangle class, which decomposes an arbitrary convex or
 *  concave polygon into a set of non-overlapping triangles using a recursive
 *  left-most-vertex splitting strategy.  The decomposition works on an
 *  indirect index buffer (@c vbuffer) so no vertex data is copied; only
 *  integer indices are rearranged.
 *
 *  Polyray - MIT Licensed Revival
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

#include <iostream>
//#include "defs3.h"
//#include "display.h"
//#include "memory.h"
#include "io_ply.h"
#include "triangle.h"

namespace openpolyray
{

/** @brief Decompose a polygon into triangles by recursive leftmost-vertex splitting.
 *
 *  Iterates over sub-polygons tracked in @c poly_end[], repeatedly finding
 *  the leftmost vertex and splitting the polygon there until every piece is a
 *  triangle.  Degenerate (collinear) triangles are silently skipped via
 *  linear_vertices().  Each valid triangle is appended to @p out_verts via
 *  add_new_triangle().
 *
 *  @param cnt        Number of vertices in the polygon (must be >= 2).
 *  @param verts      Array of vertex positions indexed by @c vbuffer[].
 *  @param out_verts  Caller-supplied pointer to a 2-D triangle buffer;
 *                    each entry is an int[3] of vertex indices.  Updated
 *                    in-place as new triangles are emitted.
 *  @return           @c RetTri containing the final triangle count and the
 *                    (possibly reallocated) @p out_verts pointer.
 */
RetTri Triangle::PolygonSplitter(int cnt, fVec* verts, int** out_verts)
{
  RetTri out;
  /* Split and push polygons until they turn into triangles */
  for (int i=1;i>0;) {
     int m = poly_end[i-1] + 1;
     int n = poly_end[i];

     if (n - m == 2) {
        if (!linear_vertices(m, n, verts)) {
		   auto ret=add_new_triangle(m, out_verts);
		   temporary_cnt=ret.count;
		   *out_verts=ret.out_verts;
	    }
        i = i - 1;
        }
     else {
        int l = leftmost_vertex(m, n, verts);
        int la = (l == n ? m : l + 1);
        int lb = (l == m ? n : l - 1);
        int ls = split_vertex(l, la, lb, m, n, verts);
        int m1,n1;
        if (ls == la || ls == lb) {
           m1 = (la < lb ? la : lb);
           n1 = (la < lb ? lb : la);
           }
        else {
           m1 = (l < ls ? l : ls);
           n1 = (l < ls ? ls : l);
           }
        perform_split(m, m1, n, n1);
        poly_end[i++] = m + n1 - m1;
        poly_end[i] = n + 2;
        }
     }
     out.count=temporary_cnt;
     out.out_verts=*out_verts;
     return out;

} //PolygonSplitter

/** @brief Append the triangle at vbuffer[m..m+2] to the output buffer.
 *
 *  Reads three consecutive entries from @c vbuffer starting at @p m and
 *  writes them as a new triangle row into @p out_verts[temporary_cnt],
 *  then increments @c temporary_cnt.  Does nothing when @p out_verts is null.
 *
 *  @param m         Starting index into @c vbuffer for the triangle vertices.
 *  @param out_verts Pointer to the caller's 2-D triangle index buffer.
 *  @return          @c RetTri with the updated triangle count and
 *                   the current @p out_verts pointer.
 */
RetTri Triangle::add_new_triangle(int m, int **out_verts)
{
   RetTri out;
   out.out_verts=*out_verts;

   if (out_verts != nullptr) {
/* printf("New: %d, %d, %d\n", vbuffer[m], vbuffer[m+1], vbuffer[m+2]); */
      int i = temporary_cnt;
      out_verts[i][0] = vbuffer[m  ];
      out_verts[i][1] = vbuffer[m+1];
      out_verts[i][2] = vbuffer[m+2];
      temporary_cnt ++;
      }
   out.count=temporary_cnt;
   return out;
}

//ugly hack
#define VERT(i, a) ((vertsarr[vbuffer[i]])[a])

/** @brief Find the leftmost vertex in the sub-polygon [m, n].
 *
 *  Scans vertices @p m through @p n (inclusive) via @c vbuffer[] and
 *  returns the index into @c vbuffer of whichever vertex has the smallest
 *  @c axis1 coordinate.  Ties are broken in favour of the earlier index.
 *
 *  @param m        First vbuffer index of the sub-polygon.
 *  @param n        Last vbuffer index of the sub-polygon.
 *  @param vertsarr Array of vertex positions; accessed via the VERT macro.
 *  @return         vbuffer index in [m, n] of the leftmost vertex.
 */
int Triangle::leftmost_vertex(int m, int n, fVec *vertsarr)
{
   int l, i;
   double x;

   /* Assume the first vertex is the farthest to the left */
   l = m;
   x = VERT(m, axis1);

   /* Now see if any of the others are farther to the left */
   for (i=m+1;i<=n;i++)
      if (VERT(i, axis1) < x) {
         l = i;
         x = VERT(i, axis1);
         }
   return l;
}



/** @brief Find a vertex that can be used to safely split the polygon at vertex @p l.
 *
 *  Starting from @p la or @p lb (neighbours of the leftmost vertex @p l),
 *  searches all remaining vertices in [m, n] for one that lies strictly inside
 *  the cone formed by the edges l->la and l->lb and has a smaller @c axis1
 *  coordinate than the current best candidate.  If no interior vertex is found,
 *  returns whichever of @p la / @p lb is farther to the right.
 *
 *  @param l    vbuffer index of the leftmost vertex.
 *  @param la   vbuffer index of the vertex clockwise from @p l.
 *  @param lb   vbuffer index of the vertex counter-clockwise from @p l.
 *  @param m    First vbuffer index of the sub-polygon.
 *  @param n    Last vbuffer index of the sub-polygon.
 *  @param vertsarr  Array of vertex positions; accessed via the VERT macro.
 *  @return     vbuffer index of the chosen split vertex.
 */
int Triangle::split_vertex(int l, int la, int lb, int m, int n, fVec *vertsarr)
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
   for (k=m;k<=n;k++)
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

/** @brief Rearrange @c vbuffer to split one polygon into two contiguous sub-polygons.
 *
 *  In-place rearrangement of the index buffer so that the range [m, n] is
 *  replaced by two back-to-back sub-polygon ranges that share the edge
 *  (vbuffer[m1], vbuffer[n1]).  After the call, PolygonSplitter() pushes
 *  both halves onto its implicit stack via @c poly_end[].
 *
 *  @param m   First index of the current polygon in @c vbuffer.
 *  @param m1  First index of the shared (split) edge.
 *  @param n   Last index of the current polygon in @c vbuffer.
 *  @param n1  Last index of the shared (split) edge.
 */
void Triangle:: perform_split(int m, int m1, int n, int n1)
{
   int i, j, k;
   int *vb1, *vb2;

   k = n + 3 - m1;
   /* Move the new polygon up over the place the current one sits */
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1];
        j<=n1;j++,vb1++,vb2++)
      *vb1 = *vb2;

   /* Move top part of remaining polygon */
   for (j=n,vb1=&vbuffer[n+2],vb2=&vbuffer[n];
        j>=n1;j--,vb1--,vb2--)
      *vb1 = *vb2;

   /* Move bottom part of remaining polygon */
   k = n1 - m1 + 1;
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1];
        j>=m;j--,vb1--,vb2--)
      *vb1 = *vb2;

   /* Copy the new polygon so that it sits before the remaining polygon */
   i = n + 3 - m1;
   k = m - m1;
   for (j=m1,vb1=&vbuffer[m1+k],vb2=&vbuffer[m1+i];
        j<=n1;j++,vb1++,vb2++)
      *vb1 = *vb2;
}


}//namespace

