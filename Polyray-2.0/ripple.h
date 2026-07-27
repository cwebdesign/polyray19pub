// This is a personal academic project. Dear PVS-Studio, please check it.

//(C) C. Meli 2020-2024 Code originally from OpenPolyray project
// OpenPolyray
// Copyright(C) 2020-2024 C. Meli(vvx9jddv54@liamekaens.com)
//ripple.h
#if !defined(__RIPPLE_DEFS)
#define __RIPPLE_DEFS

//#include <gsl/gsl> //for now
#include <span>
#include <array>
#include <iostream>

#include "defs3.h"
#include "io_ply.h"
#include "vec3.h"

//class for the ripple, originating from eval.cc
namespace openpolyray

{

#ifdef TESTING
#define PRIVATE public:
#else
#define PRIVATE private:
#endif

	/* Create a random number between 0 and 1 */
#define random_number ((Flt)rand() / (Flt)RAND_MAX)

	const int MAX_RIPPLE_CENTERS = 5;

struct Ret
{
	int count;
};

class Ripple
	{
	public:
		Ripple(void)
		{
			init_ripples();
		}
		
		Vec3 ripples(Vec P, Vec N, Flt freq, Flt phase, Flt scale);
		void init_ripples(void);

		~Ripple()
		{
			  
		}

	   int get_ripple_init_flag() const { return ripple_init_flag; }
	   Vec3 get_ripple_centers(int i) const { return ripple_centers[i]; }
       // Flt VERT(int i, int a) const noexcept {
		//	return verts[vbuffer[i]])[a];
		//}

		


		PRIVATE


			// Polygon Indices Storage
			/* Ripple/Wave variables */
			
		// UNIT_STATIC
				int ripple_init_flag = 0;
		  //  UNIT_STATIC
				Vec3 ripple_centers[MAX_RIPPLE_CENTERS];

	};


} //namespace

#endif
