#pragma once
#if !defined(__POLYGONSPLITCONTEXT_DEFS)
#define __POLYGONSPLITCONTEXT_DEFS

#include <vector>
#include <array>

#include "defs3.h"
//#include "memory.h"
//#include "io_ply.h"
//#include "bound.h"
//#include "scan.h"
//#include "vector.h"
//#include "eval.h"
//#include "runtime_state.h"
//#include "symtab.h"
//#include "triangle.h"
//#include "subdiv.h"




#ifdef TESTING

#endif


namespace openpolyray
{

    class PolygonSplitContext {
    private:
        // Storage for polygon indices 
        std::vector<int> vbuffer;
        std::vector<int> poly_end;
        int axis1 = 0;
        int axis2 = 1;
        const fVec* verts;
    public:
        /** @brief Construct a polygon splitting context for a projected polygon.
         *  @param _verts   Pointer to the caller's vertex array; must remain valid
         *                  for the entire lifetime of this object.
         *  @param _x_axis  Component index (0, 1, or 2) of each fVec treated as the
         *                  projected x-axis during splitting.
         *  @param _y_axis  Component index (0, 1, or 2) of each fVec treated as the
         *                  projected y-axis during splitting.
         *  @param count    Number of vertices in the polygon; used to reserve internal
         *                  working storage (vbuffer reserves 3*count, poly_end reserves count).
         *  @note This object is single-use.  Calling split() more than once on the same
         *        instance appends to stale internal state and produces incorrect results.
         */
        PolygonSplitContext(const fVec* _verts, int _x_axis, int _y_axis, int count)
            : verts(_verts),
            axis1(_x_axis), axis2(_y_axis) {

            vbuffer.reserve(3 * count);
            poly_end.reserve(count);
        }

        /** @brief Split a polygon into triangles, appending results to the caller's output array.
         *  @param cnt        Number of vertices in the input polygon (vertices are drawn
         *                    from the verts array supplied at construction, indexed 0..cnt-1).
         *  @param out_cnt    Caller's triangle count accumulator; incremented by one for each
         *                    triangle produced, regardless of whether @p out_verts is null.
         *  @param out_verts  Pre-allocated array of at least (cnt - 2) int[3] entries into
         *                    which triangle vertex indices are written.  Pass null to count
         *                    triangles without writing them.
         *  @note Single-use: do not call this method twice on the same context object.
         *  @return No return value.
         */
        void split(const int cnt, int& out_cnt, int** out_verts);

        /** @brief Split a polygon into triangles, appending results to the caller's output array.
         *  @param cnt        Number of vertices in the input polygon (vertices are drawn
         *                    from the verts array supplied at construction, indexed 0..cnt-1).
         *  @param out_cnt    Caller's triangle count accumulator; incremented by one for each
         *                    triangle produced, regardless of whether @p out_verts is null.
         *  @param out_verts  Pre-allocated array of at least (cnt - 2) int[3] entries into
         *                    which triangle vertex indices are written.  Pass null to count
         *                    triangles without writing them.
         *  @note Single-use: do not call this method twice on the same context object.
         *  @return No return value.
         */
        void cpp_split(const int cnt, int& out_cnt, std::vector<std::array<int, 3>>& out_verts);
    private:



// Get a polygon vertex like the old VERT Polyray macro  
        /** @brief Read one projected component from a buffered polygon vertex.
         *  @param i  Index into the temporary polygon buffer.
         *  @param a  Axis/component index to read from the source vertex.
         *  @return The selected component value for the buffered vertex.
         */
        inline Flt getVERT(int i, int a) const //was VERT(i,a)
        {
            return (verts[vbuffer[i]])[a];
        }

        /* Test polygon vertices to see if they are linear */
        /** @brief Stub: intended to detect collinear (degenerate) polygon vertices;
         *         currently always returns 0, so no triangles are ever skipped.
         *  @param m  First vbuffer index of the sub-polygon.
         *  @param n  Last vbuffer index of the sub-polygon.
         *  @return   Always 0 — collinearity detection is not yet implemented.
         */
        bool linear_vertices(int m, int n) const;
            // called only when a subpolygon has been reduced to 3 vertices
        


        /* Given the leftmost vertex in a polygon, this routine finds another vertex
   can be used to safely split the polygon. */
        /** @brief Choose a safe split vertex for polygon triangulation.
         *  @param l   vbuffer index of the leftmost vertex.
         *  @param la  vbuffer index of the adjacent vertex on one side of @p l.
         *  @param lb  vbuffer index of the adjacent vertex on the other side of @p l.
         *  @param m   First vbuffer index of the sub-polygon.
         *  @param n   Last vbuffer index of the sub-polygon.
         *  @return    vbuffer index of the chosen split vertex.
         */
        int split_vertex(int l, int la, int lb, int m, int n);

        // Shift vertex indices around to make two polygons out of one.
        /** @brief Rearrange vbuffer so the chosen diagonal splits one polygon into two
         *         contiguous sub-polygons, each ready for the next triangulation step.
         *  @param m   First vbuffer index of the current polygon.
         *  @param m1  First vbuffer index of the split span (diagonal start).
         *  @param n   Last vbuffer index of the current polygon.
         *  @param n1  Last vbuffer index of the split span (diagonal end).
         */
        void perform_split(int m, int m1, int n, int n1);

    public:
        // The original functions become member methods
        /* Find the left most vertex in the polygon that has vertices m ... n. */
        /** @brief Find the vertex with the smallest axis1 component within a polygon slice.
         *  @param m  First vbuffer index of the sub-polygon.
         *  @param n  Last vbuffer index of the sub-polygon.
         *  @return   vbuffer index in [m, n] of the vertex with the smallest axis1 component.
         */
        int leftmost_vertex(int m, int n);

        /** @brief Append the triangle at vbuffer[m..m+2] to the caller's output array.
         *  @param m          First vbuffer index of the triangle (reads m, m+1, m+2).
         *  @param out_cnt    Triangle count incremented in place; always updated even when
         *                    @p out_verts is null.
         *  @param out_capacity Maximum number of indices in out_verts
         *  @param out_verts  Output array receiving the three vertex indices; if null the
         *                    triangle is counted but not written (no-op write path).
         */
        void add_new_triangle(int m, int& out_cnt, int out_capacity, int** out_verts);

        /** @brief Append the triangle at vbuffer[m..m+2] to the caller's output std::array. using C++ array
         *  @param m          First vbuffer index of the triangle (reads m, m+1, m+2).
         *  @param out_cnt    Triangle count incremented in place; always updated even when
         *                    @p out_verts is null.
         *  @param out_capacity Maximum number of indices in out_verts
         *  @param out_verts  Output array receiving the three vertex indices; if null the
         *                    triangle is counted but not written (no-op write path).
         */
        void cpp_add_new_triangle(int m, int& out_cnt, int out_capacity, std::vector<std::array<int, 3>>& out_verts);
      

    };//end class


}; // namespace Polyray




#endif /* __POLYGONSPLITCONTEXT__ */
