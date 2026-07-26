/** @file vec3_random.h
 *  @brief Random Vec3 sampling helpers for the openpolyray namespace.
 *
 *  These are free functions that take an explicit @c Urandomdat& generator,
 *  keeping @c Vec3 itself free of any embedded RNG.  They provide the common
 *  Monte-Carlo sampling primitives (uniform component fill, points in the unit
 *  sphere, unit-sphere directions, hemisphere directions) used by stochastic
 *  shading — soft shadows, diffuse bounce, depth-of-field jitter, etc.
 *
 *  Currently unused by the renderer; kept here so the sampling code is ready to
 *  wire in when a path-tracing / stochastic sampling path is added.  Pass the
 *  same @c Urandomdat instance through a sampling pass for reproducibility.
 *
 *  Adapted from "Ray Tracing in One Weekend"
 *  (https://raytracing.github.io/books/RayTracingInOneWeekend.html).
 *
 *  (C) C. Meli 2020-2026  (OpenPolyray project)
 */
#if !defined(__VEC3_RANDOM_DEFS)
#define __VEC3_RANDOM_DEFS

#include "vec3.h"
#include "urandomdat.h"

namespace openpolyray
{
	/** @brief Return a Vec3 with each component uniform in [0, 1].
	 *  @param rng  Random generator to draw from.
	 */
	inline Vec3 randomVec(Urandomdat& rng)
	{
		return Vec3(rng.random(), rng.random(), rng.random());
	}

	/** @brief Return a Vec3 with each component uniform in [@p min, @p max].
	 *  @param rng  Random generator to draw from.
	 *  @param min  Lower bound (inclusive).  @param max  Upper bound (inclusive).
	 */
	inline Vec3 randomVec(Urandomdat& rng, double min, double max)
	{
		return Vec3(rng.random(min, max), rng.random(min, max),
			rng.random(min, max));
	}

	/** @brief Return a point uniformly distributed inside the unit sphere.
	 *
	 *  Uses rejection sampling: draw points in the [-1,1] cube until one lands
	 *  within the unit sphere.
	 *  @param rng  Random generator to draw from.
	 */
	inline Vec3 randomInUnitSphere(Urandomdat& rng)
	{
		for (;;) {
			Vec3 p = randomVec(rng, -1, 1);
			if (p.squaredLength() < 1)
				return p;
		}
	}

	/** @brief Return a random unit-length direction (point on the unit sphere).
	 *  @param rng  Random generator to draw from.
	 */
	inline Vec3 randomUnitVector(Urandomdat& rng)
	{
		return unitVector(randomInUnitSphere(rng));
	}

	/** @brief Return a random direction in the hemisphere around @p normal.
	 *  @param rng     Random generator to draw from.
	 *  @param normal  Hemisphere axis; the result satisfies dot(result, normal) > 0.
	 */
	inline Vec3 randomInHemisphere(Urandomdat& rng, const Vec3& normal)
	{
		Vec3 in_unit_sphere = randomInUnitSphere(rng);
		if (dot(in_unit_sphere, normal) > 0.0)   // same hemisphere as the normal
			return in_unit_sphere;
		else
			return -in_unit_sphere;
	}
}

#endif /* __VEC3_RANDOM_DEFS */
