//(C) C. Meli 2021-2024
#if !defined(__URANDOMDAT)
#define __URANDOMDAT

#include <array>
#include <random>

#include "defs3.h"

namespace openpolyray
{
	constexpr int DEFAULT_SEED{ 7373 };
	constexpr int MULTIPLIER{ 18829 }; /* This is 5^15 (mod 65536)     */

#ifdef TESTING
#define PRIVATE public:
#else
#define PRIVATE private:
#endif
    using UWORD64 = uint64_t;
	class Urandomdat
	{
	public:
		Urandomdat(void);
		Urandomdat(UWORD64);//(UWORD64 seed)
		~Urandomdat(void) {};

		Flt polyray_random(void);

		double rand01(void);
		double random(void);
		double random(double x, double y);

		void setdebug(void);

		int debug;
		virtual double max() { return RAND_MAX; }
		std::uniform_real_distribution<double> unif;
		std::mt19937_64 rng;
	private:
		virtual double randinternal(void);
	};

}//namespace
#endif //__URANDOMDAT