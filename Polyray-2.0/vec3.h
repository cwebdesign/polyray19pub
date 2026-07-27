/** @file vec3.h
 *  @brief Three-component floating-point vector class for the openpolyray namespace.
 *
 *  Vec3 stores its components in an @c alignas(16) @c std::array<Flt,3> (or
 *  an SSE @c __m128 register when @c VEC3_USE_SSE is defined) and provides:
 *  - Constructors from raw pointers, component triples, and legacy Polyray types
 *    (@c Vec, @c NuVec / @c cfVec / @c fVec).
 *  - Arithmetic operators, length/normalisation, component-wise min/max,
 *    cross product, dot product, and scalar triple product.
 *
 *  Random Vec3 sampling helpers live separately in @c vec3_random.h (free
 *  functions taking a @c Urandomdat&), keeping this type RNG-free.
 *
 *  Implementations live in @c vec3.cc / @c vec3.inl.h.
 *
 *  (C) C. Meli 2020-2026  (OpenPolyray project)
 */
#if !defined(__VEC3_DEFS)
#define __VEC3_DEFS

#include <gsl/gsl>
#include <span>
#include <array>
#include <iostream>
#include <algorithm>
#include <stdint.h>
#include <cmath>

//#include "rgb.h"
#include "defs3.h"

//#define VEC3_USE_SSE
 

// Shuffle helpers.
// Examples: SHUFFLE3(v, 0,1,2) leaves the vector unchanged.
//           SHUFFLE3(v, 0,0,0) splats the X coord out.
#define SHUFFLE3(V, X,Y,Z) float3(_mm_shuffle_ps((V).m, (V).m, _MM_SHUFFLE(Z,Z,Y,X)))
#define VM_INLINE   __forceinline

namespace openpolyray

{

#ifdef TESTING
#define PRIVATE public:
#else
#define PRIVATE private:
#endif

#ifdef VEC3_USE_SSE
	/** @brief Three-component vector backed by an SSE @c __m128 register.
	 *
	 *  Active only when @c VEC3_USE_SSE is defined.  Provides the same public
	 *  API as the scalar branch below but uses SSE intrinsics for storage.
	 */
	class Vec3
	{
	public:
		/** @brief Default constructor — zero-initialises all three components. */
		VM_INLINE Vec3()
		{
			e[0] = e[1] = e[2] = 0;
		}
		/** @brief Construct from a raw @c float pointer (reads p[0..2]).
		 *  @param p  Pointer to at least three floats.
		 */
		explicit Vec3(const float* p) {
			e = _mm_set_ps(p[2], p[2], p[1], p[0]);
		}
		/** @brief Construct from three explicit component values.
		 *  @param e0  X component.  @param e1  Y component.  @param e2  Z component.
		 */
		Vec3(Flt e0, Flt e1, Flt e2)
		{
			e = _mm_set_ps(e2, e2, e1, e0);
		}
		/** @brief Copy constructor — copies all three components.
		 *  @param v  Source vector.
		 */
		Vec3(const Vec3& v) =default;

		/** @brief Construct from a legacy @c cfVec (const float array[3]).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(const cfVec& v)
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		/** @brief Construct from a legacy Polyray @c Vec (double[3]).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(Vec v)
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		/** @brief Construct from a legacy Polyray @c fVec (float[3]).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(fVec v)
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		/** @brief Copy components of @p v into a @c cfVec.
		 *  @param v    Source Vec3.  @param res  Output @c cfVec (float[3]).
		 */
		void Vec3TocfVec(const Vec3& v, cfVec res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Copy components of @p v into a @c fVec.
		 *  @param v    Source Vec3.  @param res  Output @c fVec (float[3]).
		 */
		void Vec3ToffVec(const Vec3& v, fVec res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Copy components of @p v into a legacy Polyray @c Vec.
		 *  @param v    Source Vec3.  @param res  Output @c Vec (double[3]).
		 */
		void Vec3ToVec(const Vec3& v, Vec res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Return the X component.  @return e[0]. */
		Flt x() const noexcept {
			return e.at(0);
		}
		/** @brief Return the Y component.  @return e[1]. */
		Flt y() const noexcept {
			return e.at(1);
		}
		/** @brief Return the Z component.  @return e[2]. */
		Flt z() const noexcept {
			return e.at(2);
		}

		/** @brief Read-only component access.  @param i Index (0=x,1=y,2=z).  @return e[i]. */
		Flt operator[](int i) const noexcept { return e.at(i); }
		/** @brief Writable component access.  @param i Index (0=x,1=y,2=z).  @return Reference to e[i]. */
		Flt& operator[](int i) noexcept { return e.at(i); }

		/** @brief Return the Euclidean (L2) length √(x²+y²+z²). */
		Flt length() const;
		/** @brief Return the squared length x²+y²+z² (avoids a sqrt). */
		Flt squaredLength() const;

		/** @brief Normalise the vector in place so its length equals 1. */
		void makeUnitVector();
		/** @brief Set all three components to zero. */
		void makeZeroVector() noexcept {
			e[0] = e[1] = e[2] = 0;
		}

		/** @brief Set the X component.  @param _x  New value. */
		void setX(Flt _x) noexcept { e[0] = _x; }
		/** @brief Set the Y component.  @param _y  New value. */
		void setY(Flt _y) noexcept { e[1] = _y; }
		/** @brief Set the Z component.  @param _z  New value. */
		void setZ(Flt _z) noexcept { e[2] = _z; }
		/** @brief Set all three components at once.
		 *  @param _x X value.  @param _y Y value.  @param _z Z value.
		 */
		void set(Flt _x, Flt _y, Flt _z) noexcept { e[0] = _x; e[1] = _y; e[2] = _z; }
		/** @brief Return a copy of @c *this. */
		Vec3 GetVec() const
		{
			return *this;
		}

		/** @brief Set *this = a + b*c. */
		void AddScaled(Vec3 a, Flt b, Vec3 c);

		/** @brief Return the smallest component value. */
		Flt minComp() const;
		/** @brief Return the largest component value. */
		Flt maxComp() const;
		/** @brief Return the largest absolute component value. */
		Flt maxAbsComp() const;
		/** @brief Return the smallest absolute component value. */
		Flt minAbsComp() const;

		/** @brief Return the index (0–2) of the smallest component. */
		int indexOfMinComp() const;
		/** @brief Return the index (0–2) of the smallest absolute component. */
		int indexOfMinAbsComp() const;
		/** @brief Return the index (0–2) of the largest component. */
		int indexOfMaxComp() const;
		/** @brief Return the index (0–2) of the largest absolute component. */
		int indexOfMaxAbsComp() const;

		/** @brief Exact equality test.  @return @c true when all components are equal. */
		friend bool operator==(const Vec3& v1, const Vec3& v2);
		/** @brief Exact inequality test.  @return @c true when any component differs. */
		friend bool operator!=(const Vec3& v1, const Vec3& v2);

		/** @brief Stream extraction — reads three whitespace-separated values. */
		friend std::istream& operator>>(std::istream& is, Vec3& t);
		/** @brief Stream insertion — writes the three components to @p os. */
		friend std::ostream& operator<<(std::ostream& os, const Vec3& t);

		/** @brief Component-wise addition.  @return Vec3(v1+v2). */
		friend Vec3 operator+(const Vec3& v1, const Vec3& v2);
		/** @brief Component-wise subtraction.  @return Vec3(v1−v2). */
		friend Vec3 operator-(const Vec3& v1, const Vec3& v2);
		/** @brief Scalar division.  @return Vec3(vec/scalar). */
		friend Vec3 operator/(const Vec3& vec, Flt scalar);
		/** @brief Vector–scalar multiplication.  @return Vec3(vec*scalar). */
		friend Vec3 operator*(const Vec3& vec, Flt scalar);
		/** @brief Scalar–vector multiplication.  @return Vec3(scalar*vec). */
		friend Vec3 operator*(Flt scalar, const Vec3& vec);

		/** @brief Copy-assign.  @return Reference to @c *this. */
		Vec3& operator=(const Vec3& v2);
		/** @brief Component-wise add-assign.  @return Reference to @c *this. */
		Vec3& operator+=(const Vec3& v2);
		/** @brief Component-wise subtract-assign.  @return Reference to @c *this. */
		Vec3& operator-=(const Vec3& v2);
		/** @brief Scalar multiply-assign.  @param t Scale factor.  @return Reference to @c *this. */
		Vec3& operator*=(const Flt t);
		/** @brief Scalar divide-assign.  @param t Divisor (non-zero).  @return Reference to @c *this. */
		Vec3& operator/=(const Flt t);

		/** @brief Return a unit-length copy of @p v (undefined for zero vector). */
		friend Vec3 unitVector(const Vec3& v);
		/** @brief Component-wise minimum of @p v1 and @p v2. */
		friend Vec3 minVec(const Vec3& v1, const Vec3& v2);
		/** @brief Component-wise maximum of @p v1 and @p v2. */
		friend Vec3 maxVec(const Vec3& v1, const Vec3& v2);
		/** @brief Return the cross product v1 × v2. */
		friend Vec3 cross(const Vec3& v1, const Vec3& v2);
		/** @brief Return the dot product v1 · v2. */
		friend Flt dot(const Vec3& v1, const Vec3& v2);
		/** @brief Return the scalar triple product v1 · (v2 × v3). */
		friend Flt tripleProduct(const Vec3& v1, const Vec3& v2, const Vec3& v3);
		/** @brief Normalise @p v in place and return its original length. */
		Flt VecNormalize(Vec3& v);
		/** @brief Normalise @c *this in place. */
		void Normalize(void);
		/** @brief Return @c true when all components are near zero (< 1e-8). */
		bool near_zero() const;
#else

	/** @brief Three-component vector backed by an @c alignas(16) @c std::array<Flt,3>.
	 *
	 *  Active when @c VEC3_USE_SSE is @b not defined (the normal build).
	 *  Provides constructors from raw pointers, component triples, and the
	 *  legacy Polyray types @c Vec and @c NuVec.  All arithmetic is scalar.
	 */
	class Vec3
	{
	public:
		/** @brief Default constructor — zero-initialises all three components. */
		Vec3() 
			
		{
			e[0] = e[1] = e[2] = 0;
			
		}
		/** @brief Construct from a raw @c float pointer (reads p[0..2]).
		 *  @param p  Pointer to at least three floats.
		 */
		Vec3(const float* p) 
		{
			e[0] = p[0]; e[1] = p[1]; e[2] = p[2];
		}
		/** @brief Construct from three explicit component values.
		 *  @param e0  X component.  @param e1  Y component.  @param e2  Z component.
		 */
		Vec3(Flt e0, Flt e1, Flt e2);
		/** @brief Copy constructor — copies all three components.
		 *  @param v  Source vector.
		 */
		Vec3(const Vec3& v) = default;

		/** @brief Construct from a @c NuVec (std::array<double,3>).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(const NuVec& v) 
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		/** @brief Construct from a legacy Polyray @c Vec (double[3]).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(Vec v)
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		/** @brief Construct from a legacy Polyray @c Vec (double[3]).
		 *  @param v  Source array; elements 0–2 are copied.
		 */
		Vec3(const Vec v)
		{
			e[0] = v[0];
			e[1] = v[1];
			e[2] = v[2];
		}

		//Vec3(fVec v) - commented out: fVec and Vec are the same type now.

		/** @brief Copy components of @p v into a @c NuVec.
		 *  @param v    Source Vec3.  @param res  Output @c NuVec (std::array<double,3>).
		 */
		void Vec3ToNuVec(const Vec3& v, NuVec& res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Copy components of @p v into a @c fVec (float[3]).
		 *  @param v    Source Vec3.  @param res  Output @c fVec.
		 */
		void Vec3ToffVec(const Vec3& v, fVec res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Copy components of @p v into a legacy Polyray @c Vec.
		 *  @param v    Source Vec3.  @param res  Output @c Vec (double[3]).
		 */
		void Vec3ToVec(const Vec3& v, Vec res)
		{
			res[0] = v.x();
			res[1] = v.y();
			res[2] = v.z();
		}

		/** @brief Copy @c *this components into a legacy Polyray @c Vec.
		 *  @param res  Output @c Vec (double[3]).
		 */
		void Vec3ToVec(Vec res) const
		{
			res[0] = x();
			res[1] = y();
			res[2] = z();
		}

		/** @brief Return a @c NuVec copy of @c *this.
		 *  @return @c std::array<double,3> containing {e[0], e[1], e[2]}.
		 */
		NuVec GetNuVec(void) const
		{
			NuVec v;
			v[0]=e[0];
			v[1]=e[1];
			v[2]=e[2];
			return v;
		}

		/** @brief Return the X component.  @return e[0]. */
		Flt x() const noexcept {
			return e.at(0);
		}
		/** @brief Return the Y component.  @return e[1]. */
		Flt y() const noexcept {
			return e.at(1);
		}
		/** @brief Return the Z component.  @return e[2]. */
		Flt z() const noexcept {
			return e.at(2);
		}

		/** @brief Unary plus — returns a const reference to @c *this unchanged. */
		const Vec3& operator+(void) const;
		/** @brief Unary negation — returns a new Vec3 with all components negated. */
		Vec3 operator-(void) const;

		/** @brief Read-only component access.  @param i Index (0=x,1=y,2=z).  @return e[i]. */
		Flt operator[](int i) const noexcept { return e.at(i); }
		/** @brief Writable component access.  @param i Index (0=x,1=y,2=z).  @return Reference to e[i]. */
		Flt& operator[](int i) noexcept { return e.at(i); }

		/** @brief Return the Euclidean (L2) length √(x²+y²+z²). */
		Flt length(void) const;
		/** @brief Return the squared length x²+y²+z² (avoids a sqrt). */
		Flt squaredLength(void) const;

		/** @brief Normalise the vector in place so its length equals 1. */
		void makeUnitVector(void);
		/** @brief Set all three components to zero. */
		void makeZeroVector(void) noexcept {
			e[0] = e[1] = e[2] = 0;
		}

		/** @brief Set the X component.  @param _x  New value. */
		void setX(Flt _x) noexcept { e[0] = _x; }
		/** @brief Set the Y component.  @param _y  New value. */
		void setY(Flt _y) noexcept { e[1] = _y; }
		/** @brief Set the Z component.  @param _z  New value. */
		void setZ(Flt _z) noexcept { e[2] = _z; }
		/** @brief Set all three components at once.
		 *  @param _x X value.  @param _y Y value.  @param _z Z value.
		 */
		void set(Flt _x, Flt _y, Flt _z) noexcept { e[0] = _x; e[1] = _y; e[2] = _z; }
		/** @brief Return a copy of @c *this. */
		Vec3 GetVec() const
		{
			return *this;
		}

		/** @brief Set *this = a + b*c. */
		void AddScaled(Vec3 a, Flt b, Vec3 c);

		/** @brief Return the smallest component value. */
		Flt minComp() const;
		/** @brief Return the largest component value. */
		Flt maxComp() const;
		/** @brief Return the largest absolute component value. */
		Flt maxAbsComp() const;
		/** @brief Return the smallest absolute component value. */
		Flt minAbsComp() const;

		/** @brief Return the index (0–2) of the smallest component. */
		int indexOfMinComp() const;
		/** @brief Return the index (0–2) of the smallest absolute component. */
		int indexOfMinAbsComp() const;
		/** @brief Return the index (0–2) of the largest component. */
		int indexOfMaxComp() const;
		/** @brief Return the index (0–2) of the largest absolute component. */
		int indexOfMaxAbsComp() const;

		/** @brief Exact equality — @c true when all three components are equal. */
		friend bool operator==(const Vec3& v1, const Vec3& v2);
		/** @brief Exact inequality — @c true when any component differs. */
		friend bool operator!=(const Vec3& v1, const Vec3& v2);

		/** @brief Stream extraction — reads three whitespace-separated @c Flt values. */
		friend std::istream& operator>>(std::istream& is, Vec3& t);
		/** @brief Stream insertion — writes the three components to @p os. */
		friend std::ostream& operator<<(std::ostream& os, const Vec3& t);

		/** @brief Component-wise addition.  @return Vec3(v1+v2). */
		friend Vec3 operator+(const Vec3& v1, const Vec3& v2);
		/** @brief Component-wise subtraction.  @return Vec3(v1−v2). */
		friend Vec3 operator-(const Vec3& v1, const Vec3& v2);
		/** @brief Scalar division.  @return Vec3(vec/scalar). */
		friend Vec3 operator/(const Vec3& vec, Flt scalar);
		/** @brief Vector–scalar multiplication.  @return Vec3(vec*scalar). */
		friend Vec3 operator*(const Vec3& vec, Flt scalar);
		/** @brief Scalar–vector multiplication.  @return Vec3(scalar*vec). */
		friend Vec3 operator*(Flt scalar, const Vec3& vec);

		/** @brief Copy-assign all three components.  @return Reference to @c *this. */
		Vec3& operator=(const Vec3& v2);
		/** @brief Component-wise add-assign.  @return Reference to @c *this. */
		Vec3& operator+=(const Vec3& v2);
		/** @brief Component-wise subtract-assign.  @return Reference to @c *this. */
		Vec3& operator-=(const Vec3& v2);
		/** @brief Scalar multiply-assign.  @param t Scale factor.  @return Reference to @c *this. */
		Vec3& operator*=(const Flt t);
		/** @brief Scalar divide-assign.  @param t Divisor (non-zero).  @return Reference to @c *this. */
		Vec3& operator/=(const Flt t);

		/** @brief Return a unit-length copy of @p v (undefined for zero vector). */
		friend Vec3 unitVector(const Vec3& v);
		/** @brief Component-wise minimum of @p v1 and @p v2. */
		friend Vec3 minVec(const Vec3& v1, const Vec3& v2);
		/** @brief Component-wise maximum of @p v1 and @p v2. */
		friend Vec3 maxVec(const Vec3& v1, const Vec3& v2);
		/** @brief Return the cross product v1 × v2. */
		friend Vec3 cross(const Vec3& v1, const Vec3& v2);
		/** @brief Return the dot product v1 · v2. */
		friend Flt dot(const Vec3& v1, const Vec3& v2);
		/** @brief Return the scalar triple product v1 · (v2 × v3). */
		friend Flt tripleProduct(const Vec3& v1, const Vec3& v2, const Vec3& v3);
		/** @brief Normalise @p v in place and return its original length.
		 *  @param v  Vector to normalise in place.
		 *  @return   The length of @p v before normalisation.
		 */
		Flt VecNormalize(Vec3& v);
		/** @brief Normalise @c *this in place. */
		void Normalize(void);
		/** @brief Return @c true when all components are near zero (each < 1e-8). */
		bool near_zero() const;

		
#ifdef VEC3_USE_SSE
			_m128 e; /**< SSE register holding all four lanes (z duplicated into w). */
#else
		alignas(16) std::array<Flt, 3> e; /**< Component storage: e[0]=x, e[1]=y, e[2]=z. */
#endif

		PRIVATE
#endif

		
	};

	}


#include "vec3.inl.h"
#endif