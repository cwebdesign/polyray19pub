/* polygonsplitcontext.cc

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

#include <vector>
#include <cassert>
#include <gsl/assert>
#include <cmath>

#include "defs3.h"
#include "io_ply.h"
#include "runtime_state.h"
#include "polygonsplitcontext.h"



namespace openpolyray
{

   
    int PolygonSplitContext::split_vertex(int l, int la, int lb, int m, int n)
    {
        const double y_l = getVERT(l, axis2);
        const double y_la = getVERT(la, axis2);
        const double y_lb = getVERT(lb, axis2);

        const double yu = std::max({ y_l, y_la, y_lb });
        const double yl = PLY_MIN(y_l, y_la, y_lb);

        const int lpu = (y_lb > y_la) ? lb : la;
        const int lpl = (y_lb > y_la) ? la : lb;

        int t = (getVERT(lb, axis1) > getVERT(la, axis1)) ? lb : la;

        for (int k = m; k <= n; ++k) {
            const bool is_original_vertex =
                k == la || k == l || k == lb;

            if (is_original_vertex)
                continue;

            const double k_x = getVERT(k, axis1);
            const double k_y = getVERT(k, axis2);

            const bool within_y_range =
                k_y <= yu && k_y >= yl;

            const bool left_of_current_candidate =
                k_x < getVERT(t, axis1);

            const bool below_upper_edge =
                ((k_y - y_l) * (getVERT(lpu, axis1) - getVERT(l, axis1))) <=
                ((getVERT(lpu, axis2) - y_l) * (k_x - getVERT(l, axis1)));

            const bool above_lower_edge =
                ((k_y - y_l) * (getVERT(lpl, axis1) - getVERT(l, axis1))) >=
                ((getVERT(lpl, axis2) - y_l) * (k_x - getVERT(l, axis1)));

            const bool is_valid_split_candidate =
                within_y_range &&
                left_of_current_candidate &&
                below_upper_edge &&
                above_lower_edge;

            if (is_valid_split_candidate) {
                t = k;
            }
        }

        return t;
    }

        void PolygonSplitContext::perform_split(int poly_start, int split_start, int poly_end, int split_end)
        {
            assert(poly_start <= split_start);
            assert(split_start <= split_end);
            assert(split_end <= poly_end);

            assert(poly_start >= 0);
            assert(poly_end >= 0);

            assert(poly_end < static_cast<int>(vbuffer.size()));
            // perform_split writes up to n+2
            assert((poly_end + 2) < static_cast<int>(vbuffer.size()));

            int i, j, k;
            int* vb1, * vb2;



            k = poly_end + 3 - split_start;
            /* Move the new polygon up over the place the current one sits */
            for (j = split_start, vb1 = &vbuffer[split_start + k], vb2 = &vbuffer[split_start];
                j <= split_end; j++, vb1++, vb2++)
                *vb1 = *vb2;

            /* Move top part of remaining polygon */
            for (j = poly_end, vb1 = &vbuffer[poly_end + 2], vb2 = &vbuffer[poly_end];
                j >= split_end; j--, vb1--, vb2--)
                *vb1 = *vb2;

            /* Move bottom part of remaining polygon */
            k = split_end - split_start + 1;
            for (j = split_start, vb1 = &vbuffer[split_start + k], vb2 = &vbuffer[split_start];
                j >= poly_start; j--, vb1--, vb2--)
                *vb1 = *vb2;

            /* Copy the new polygon so that it sits before the remaining polygon */
            i = poly_end + 3 - split_start;
            k = poly_start - split_start;
            for (j = split_start, vb1 = &vbuffer[split_start + k], vb2 = &vbuffer[split_start + i];
                j <= split_end; j++, vb1++, vb2++)
                *vb1 = *vb2;
        }


        int PolygonSplitContext::leftmost_vertex(int m, int n)
        {

            // Assume the first vertex is the farthest to the left
            int l = m;
            auto x = getVERT(m, axis1);
            // Now see if any of the others are farther to the left 
            for (int i = m + 1; i <= n; i++)
                if (getVERT(i, axis1) < x) {
                    l = i;
                    x = getVERT(i, axis1);
                }
            return l;
        }


        void PolygonSplitContext::add_new_triangle(int m, int& out_cnt, int out_capacity, int** out_verts) {
            //called like this:  add_new_triangle(m, out_cnt, out_verts)            

            if (out_verts == nullptr) return;
            if (out_cnt >= out_capacity) {
                serror("Can't add new triangle outside of existing capacity");
            }
            /* printf("New: %d, %d, %d\n", vbuffer[m], vbuffer[m+1], vbuffer[m+2]); */
            int i = out_cnt;
            out_verts[i][0] = vbuffer[m];
            out_verts[i][1] = vbuffer[m + 1];
            out_verts[i][2] = vbuffer[m + 2];
            out_cnt += 1;
            
        }

        void PolygonSplitContext::cpp_add_new_triangle(int m, int& out_cnt, int out_capacity, std::vector<std::array<int, 3>>& out_verts) {
            //called like this:  add_new_triangle(m, out_cnt, out_verts)            
            if (out_cnt >= out_capacity) {
                serror("Can't add new triangle outside of existing capacity");
            }
            /* printf("New: %d, %d, %d\n", vbuffer[m], vbuffer[m+1], vbuffer[m+2]); */
            int i = out_cnt;
            out_verts[i][0] = vbuffer[m];
            out_verts[i][1] = vbuffer[m + 1];
            out_verts[i][2] = vbuffer[m + 2];
            out_cnt += 1;
        }
        
        // Triangulates a simple polygon by recursively splitting it from a reflex/leftmost vertex.
        // This is not BSP polygon-plane splitting.
        void PolygonSplitContext::split(const int cnt, int& out_cnt, int** out_verts) {
            Expects(out_verts != nullptr);
            if (cnt < 3) { //guard small/invalid polygons, exit early in this case
                out_cnt = 0;
                return;
            }
            // Initialize the polygon splitter 
            poly_end.clear();
            vbuffer.clear();
            poly_end.push_back(-1);
            poly_end.push_back(cnt - 1);

            // Start with a strict identity of vertices in verts and vertices in the polygon buffer
            for (int i = 0; i < cnt; i++)
                vbuffer.push_back(i);
            // perform_split writes up to index n+3 (<= cnt+2 on the first split, growing by 2
            // per subsequent split).  The original C code used malloc(3*cnt) which made those
            // slots valid; resize to the same bound so operator[] stays within size().
            // reserve(3*count) in the constructor means this resize never reallocates.
            vbuffer.resize(3 * cnt);

            int maxtriangles = cnt - 2;


            /* Split and push polygons until they turn into triangles */
            for (int i = 1; i > 0;) {
                int m = poly_end[i - 1] + 1;
                int n = poly_end[i];

                if (n - m == 2) {
                    if (!linear_vertices(m, n))
                        add_new_triangle(m, out_cnt, maxtriangles, out_verts);
                    i--;
                }
                else {
                    int l = leftmost_vertex(m, n);
                    int la = (l == n ? m : l + 1);
                    int lb = (l == m ? n : l - 1);
                    int ls = split_vertex(l, la, lb, m, n);
                    int m1, n1;
                    if (ls == la || ls == lb) {
                        m1 = (la < lb ? la : lb);
                        n1 = (la < lb ? lb : la);
                    }
                    else {
                        m1 = (l < ls ? l : ls);
                        n1 = (l < ls ? ls : l);
                    }
                    Ensures(l >= m && l <= n);
                    Ensures(la >= m && la <= n);
                    Ensures(lb >= m && lb <= n);
                    Ensures(ls >= m && ls <= n);
                    perform_split(m, m1, n, n1);
                    poly_end[i] = m + n1 - m1;   // update existing slot in place - still in bounds
                    poly_end.push_back(n + 2);   // append the new sub-polygon end - extends size()
                    Ensures(poly_end[i] >= m);
                    Ensures(poly_end[i] < static_cast<int>(vbuffer.size()));
                    i++;                         // advance i to point at the entry we just pushed
                    
                } //rnf ig
            }//end for

        }//end split

        bool PolygonSplitContext::linear_vertices(int m, int n) const
        {
            // This function is currently only expected to be called for triangles.
            assert(n - m == 2);  
            if (!RuntimeState::settings.check_colineartriangles) return false;

            const int a = vbuffer[m];
            const int b = vbuffer[m + 1];
            const int c = vbuffer[m + 2];

            const double ax = getVERT(a, axis1);
            const double ay = getVERT(a, axis2);

            const double bx = getVERT(b, axis1);
            const double by = getVERT(b, axis2);

            const double cx = getVERT(c, axis1);
            const double cy = getVERT(c, axis2);

            const double abx = bx - ax;
            const double aby = by - ay;
            const double acx = cx - ax;
            const double acy = cy - ay;

            const double area2 = abx * acy - aby * acx;

            const double scale =
                std::max({ std::abs(abx), std::abs(aby), std::abs(acx), std::abs(acy), 1.0 });

            constexpr double EPS = 1.0e-12;

            return std::abs(area2) <= EPS * scale * scale;
        }

        void PolygonSplitContext::cpp_split(const int cnt, int& out_cnt, std::vector<std::array<int, 3>>& out_verts) {
            if (cnt < 3) { //guard small/invalid polygons
                out_cnt = 0;
                return;
            }
            // Initialize the polygon splitter 
            poly_end.clear();
            vbuffer.clear();
            poly_end.push_back(-1);
            poly_end.push_back(cnt - 1);

            // Start with a strict identity of vertices in verts and vertices in the polygon buffer
            for (int i = 0; i < cnt; i++)
                vbuffer.push_back(i);
            // perform_split writes up to index n+3 (cnt+2 on the first split, growing by 2
            // per subsequent split).  The original C code used malloc(3*cnt) which made those
            // slots valid; resize to the same bound so operator[] stays within size().
            // reserve(3*count) in the constructor means this resize never reallocates.
            vbuffer.resize(3 * cnt);

            int maxtriangles = cnt - 2;

            /* Split and push polygons until they turn into triangles */
            for (int i = 1; i > 0;) {
                int m = poly_end[i - 1] + 1;
                int n = poly_end[i];

                if (n - m == 2) {
                    if (!linear_vertices(m, n))
                        cpp_add_new_triangle(m, out_cnt, maxtriangles, out_verts);
                    i--;
                }
                else {
                    int l = leftmost_vertex(m, n);
                    int la = (l == n ? m : l + 1);
                    int lb = (l == m ? n : l - 1);
                    int ls = split_vertex(l, la, lb, m, n);
                    int m1, n1;
                    if (ls == la || ls == lb) {
                        m1 = (la < lb ? la : lb);
                        n1 = (la < lb ? lb : la);
                    }
                    else {
                        m1 = (l < ls ? l : ls);
                        n1 = (l < ls ? ls : l);
                    }
                    Ensures(l >= m && l <= n);
                    Ensures(la >= m && la <= n);
                    Ensures(lb >= m && lb <= n);
                    Ensures(ls >= m && ls <= n);
                    perform_split(m, m1, n, n1);
                    poly_end[i] = m + n1 - m1;   // update existing slot in place - still in bounds
                    poly_end.push_back(n + 2);   // append the new sub-polygon end - extends size()
                    Ensures(poly_end[i] >= m);
                    Ensures(poly_end[i] < static_cast<int>(vbuffer.size()));
                    i++;                         // advance i to point at the entry we just pushed

                } //rnf ig
            }//end for

        }//end split


}; // namespace Polyray

// The original functions become member methods are found in subdiv.cc not here


