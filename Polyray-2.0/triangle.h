// This is a personal academic project. Dear PVS-Studio, please check it.

//(C) C. Meli 2020-2024 Code originally from OpenPolyray project
// OpenPolyray
// Copyright(C) 2020-2024 C. Meli(vvx9jddv54@liamekaens.com)
//triangle.h
#if !defined(__TRIANGLE_DEFS)
#define __TRIANGLE_DEFS

//#include <gsl/gsl> //for now
#include <span>
#include <array>
#include <iostream>

#include "defs3.h"
#include "io_ply.h"

//ugly,hack, from old Polyray
//todo:change
//#define VERT(i, a) ((verts[vbuffer[i]])[a])

//class for the triangular polygons
namespace openpolyray

{

#ifdef TESTING
#define PRIVATE public:
#else
#define PRIVATE private:
#endif

struct RetTri
{
	int count;
	int *out_verts; //ugly, just for now
};

class Triangle
	{
	public:
		Triangle(int _cnt,int x_axis, int y_axis)
		{
			cnt=_cnt;
			/* Make sure there is storage for intermediate vertices */
			vbuffer = (int *)malloc(3 * cnt * sizeof(int));
			if (cnt < 2) serror("Split_Polygon:cnt must be at least 2");//cm, avoid buffer overrun

			poly_end = (int *)malloc(cnt * sizeof(int));
			if (vbuffer == NULL || poly_end == NULL)
			   serror("Failed to allocate glyph polygon buffer");

			/* Set the coordinates that we will perform the splitting on. */
			axis1 = x_axis;
			axis2 = y_axis;

			// temporary count of triangles
			temporary_cnt = 0;

			/* Initialize the polygon splitter */
			poly_end[0] = -1;
			poly_end[1] = cnt-1;

			/* Start with a strict identity of vertices in verts and vertices in
			   the polygon buffer */
			for (int i=0;i<cnt;i++)
              vbuffer[i] = i;
		}
		

		~Triangle()
		{
			   /* Free up the temporary stacks used for polygon decomposition */
			   free(vbuffer);
			   vbuffer = nullptr;
			   free(poly_end);
			   poly_end = nullptr;
		}

       // Flt VERT(int i, int a) const noexcept {
		//	return verts[vbuffer[i]])[a];
		//}

		RetTri PolygonSplitter(int cnt, fVec *verts, int **out_verts);		
		/* Test polygon vertices to see if they are linear */
		int linear_vertices(int m, int n, fVec *verts)
		{
		   /* Not doing anything right now */
		   return 0;
	    }
	    RetTri add_new_triangle(int m, int **out_verts);
	    int leftmost_vertex(int m, int n, fVec *verts);
	    int split_vertex(int l, int la, int lb, int m, int n, fVec *verts);
	    void perform_split(int m, int m1, int n, int n1);

		void setvbuffer(int a, int b, int c) {
			vbuffer[0] = a;
			vbuffer[1] = b;
			vbuffer[2] = c;
		}


		PRIVATE


			// Polygon Indices Storage
			int *vbuffer = nullptr;
			int *poly_end = nullptr;
			int axis1, axis2;
			int cnt;
			int temporary_cnt = 0;;

	};


} //namespace

#endif