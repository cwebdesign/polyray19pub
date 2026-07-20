/** @file vec2.h
 *  @brief Two-component floating-point vector class for the openpolyray namespace.
 *
 *  Vec2 stores its components in a @c std::array<Flt,2> and provides the
 *  standard complement of arithmetic operators, length/normalisation helpers,
 *  stream I/O, and component-wise @c min / @c max / @c dot free functions.
 *  Equality comparison uses AlmostEqualRelativeAndAbs() with a tolerance of 0.1.
 *
 *  (C) C. Meli 2020-2026  (OpenPolyray project)
 */
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#if !defined(__VEC2_DEFS)
#define __VEC2_DEFS

// #include "export.h"

// #ifndef POLYRAY_USE_MODULES

#include <gsl/gsl>
#include <array>

#include "defs3.h"



namespace openpolyray
{

#ifdef TESTING
#define PRIVATE public:
#else
#define PRIVATE private:
#endif


	/** @brief Two-component vector of type @c Flt.
	 *
	 *  Provides value-semantics arithmetic, in-place normalisation, stream I/O,
	 *  and a fuzzy equality operator (tolerance 0.1).  The backing storage
	 *  @c data is @c public for direct array access; @c debug is private
	 *  (exposed as public when compiled with @c TESTING defined).
	 */
	class Vec2
	{
	public:
		/** @brief Default constructor — initialises both components to zero. */
		Vec2()
		{
			data[0] = data[1] = 0;
			debug = false;
		}
		/** @brief Destructor — trivial; no heap resources owned. */
		~Vec2(void) {};

		void setdebug(void);

		/** @brief Construct from two explicit component values.
		 *  @param f0  X component.
		 *  @param f1  Y component.
		 */
		Vec2(Flt f0, Flt f1)
		{
			data[0] = f0;
			data[1] = f1;
			debug = false;
		}

		/** @brief Copy constructor.
		 *  @param v  Source vector; copies both components and the debug flag.
		 */
		Vec2(const Vec2& v)
		{
			data[0] = v.data[0];
			data[1] = v.data[1];
			debug = v.debug;
		}

		/** @brief Return the X (first) component.  @return data[0]. */
		Flt x() const
		{
			return data[0];
		}

		/** @brief Return the Y (second) component.  @return data[1]. */
		Flt y() const
		{
			return data[1];
		}

		/** @brief Set the X component.  @param f  New value for data[0]. */
		void set_x(Flt f)
		{
			data[0] = f;
		}

		/** @brief Set the Y component.  @param f  New value for data[1]. */
		void set_y(Flt f)
		{
			data[1] = f;
		}

		/*probably not needed:
		  Vec2 clamp(Vec2 x, Vec2 minVal, Vec2 maxVal)
		{
			if (x < minVal) return minVal;
			if (x > maxVal) return maxVal;
			return x;
		}	
		*/

		/** @brief Return the Euclidean length (L2 norm) of the vector.
		 *  @return √(x² + y²).
		 */
		Flt length() const
		{
			return std::sqrt(squared_length());
		}

		/** @brief Return the squared Euclidean length (avoids a sqrt).
		 *  @return x² + y².
		 */
		Flt squared_length() const
		{
			return data[0] * data[0] + data[1] * data[1];
		}

		/** @brief Normalise the vector in place so its length becomes 1.
		 *
		 *  A zero-length vector is left unchanged.
		 */
		void normalize();

		/** @brief Pseudo-randomly scramble both components in place.
		 *
		 *  Applies a deterministic but non-invertible mixing step (multiply by
		 *  large constants, keep only the fractional part).  Used for jitter /
		 *  hash-like operations where true randomness is not required.
		 */
		void scramble();


		/** @brief Unary negation — return a vector with both components negated.
		 *  @return Vec2(−x, −y).
		 */
		Vec2 operator-() const
		{
			return Vec2(-data[0], -data[1]);
		}

		/** @brief Read-only component access.
		 *  @param i  Index (0 = x, 1 = y).
		 *  @return   Component value at @p i.
		 */
		Flt operator[](int i) const
		{
			return data[i];
		}

		/** @brief Writable component access.
		 *  @param i  Index (0 = x, 1 = y).
		 *  @return   Reference to the component at @p i.
		 */
		Flt& operator[](int i)
		{
			return data[i];
		}

		/** @brief Component-wise add-assign.
		 *  @param v  Vector to add.
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator+=(const Vec2& v)
		{
			data[0] += v.data[0];
			data[1] += v.data[1];
			return *this;
		}

		/** @brief Component-wise subtract-assign.
		 *  @param v  Vector to subtract.
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator-=(const Vec2& v)
		{
			data[0] -= v.data[0];
			data[1] -= v.data[1];
			return *this;
		}

		/** @brief Scalar add-assign — adds @p f to both components.
		 *  @param f  Scalar to add.
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator+=(const Flt f)
		{
			data[0] += f;
			data[1] += f;
			return *this;
		}

		/** @brief Scalar subtract-assign — subtracts @p f from both components.
		 *  @param f  Scalar to subtract.
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator-=(const Flt f)
		{
			data[0] -= f;
			data[1] -= f;
			return *this;
		}

		/** @brief Scalar multiply-assign — scales both components by @p f.
		 *  @param f  Scale factor.
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator*=(const Flt f)
		{
			data[0] *= f;
			data[1] *= f;
			return *this;
		}

		/** @brief Scalar divide-assign — divides both components by @p f.
		 *  @param f  Divisor (must be non-zero).
		 *  @return   Reference to @c *this.
		 */
		Vec2& operator/=(const Flt f)
		{
			data[0] /= f;
			data[1] /= f;
			return *this;
		}
	public:
		std::array<Flt, 2> data; /**< Component storage: data[0]=x, data[1]=y. */
	PRIVATE
		bool debug; /**< When @c true, extra diagnostic output may be emitted. */
	};//class

	/** @brief Pseudo-randomly scramble both components in place.
	 *
	 *  Multiplies each component by a large constant and retains only the
	 *  fractional part, producing a cheap but non-invertible hash-like mix.
	 *  The two constants (1234.12345054321 and 7654.54321012345) are chosen
	 *  empirically for reasonable distribution on [0, 1).
	 */
	inline void Vec2::scramble()
	{
		Flt _x;
		Flt _y = data[0];

		_x = data[1] * 1234.12345054321f;
		data[0] = _x - (int)_x;
		_y = _y * 7654.54321012345f;
		data[1] = _y - (int)_y;
	}

	/** @brief Enable debug mode for this vector instance. */
	inline void Vec2::setdebug(void)
	{
		debug = true;
	}
		
	/** @brief Normalise the vector so its length equals 1.
	 *
	 *  Divides both components by the current length.  If the length is zero
	 *  the vector is left unchanged to avoid division by zero.
	 */
	inline void Vec2::normalize()
	{
		Flt len = length();
		if (len > 0)
		{
			Flt k = 1.0f / len;
			data[0] *= k;
			data[1] *= k;
		}
	}

	/** @brief Fuzzy equality — true when both components are within 0.1 of each other.
	 *  @param v1  Left-hand vector.
	 *  @param v2  Right-hand vector.
	 *  @return    @c true if each component pair satisfies AlmostEqualRelativeAndAbs(0.1).
	 */
	inline bool operator==(const Vec2& v1, const Vec2& v2)
	{
		//Util u; u.IsAlm..
		return (AlmostEqualRelativeAndAbs(v1[0], v2[0], 0.1) &&
			AlmostEqualRelativeAndAbs(v1[1], v2[1], 0.1));
	}

	/** @brief Fuzzy inequality — true when any component differs by more than 0.1.
	 *  @param v1  Left-hand vector.
	 *  @param v2  Right-hand vector.
	 *  @return    @c true if any component pair fails AlmostEqualRelativeAndAbs(0.1).
	 */
	inline bool operator!=(const Vec2& v1, const Vec2& v2)
	{
		//Util u;
		return (!AlmostEqualRelativeAndAbs(v1[0], v2[0], 0.1) ||
			!AlmostEqualRelativeAndAbs(v1[1], v2[1], 0.1));
	}

	/** @brief Stream extraction — reads two whitespace-separated @c Flt values into @p v.
	 *  @param is  Input stream.
	 *  @param v   Vector to populate.
	 *  @return    Reference to @p is.
	 */
	inline std::istream& operator>>(std::istream& is, Vec2& v)
	{
		return (is >> v[0] >> v[1]);
	}

	/** @brief Stream insertion — writes both components to @p os.
	 *  @param os  Output stream.
	 *  @param v   Vector to write.
	 *  @return    Reference to @p os.
	 */
	inline std::ostream& operator<<(std::ostream& os, const Vec2& v)
	{
		return (os << v[0] << v[1]);
	}

	/** @brief Component-wise vector addition.
	 *  @param v1  Left operand.  @param v2  Right operand.
	 *  @return    Vec2(v1.x+v2.x, v1.y+v2.y).
	 */
	inline Vec2 operator+(const Vec2& v1, const Vec2& v2)
	{
		return Vec2(v1.data[0] + v2.data[0], v1.data[1] + v2.data[1]);
	}

	/** @brief Component-wise vector subtraction.
	 *  @param v1  Left operand.  @param v2  Right operand.
	 *  @return    Vec2(v1.x−v2.x, v1.y−v2.y).
	 */
	inline Vec2 operator-(const Vec2& v1, const Vec2& v2)
	{
		return Vec2(v1.data[0] - v2.data[0], v1.data[1] - v2.data[1]);
	}

	/** @brief Add scalar @p f to both components of @p v.
	 *  @param v  Vector operand.  @param f  Scalar to add.
	 *  @return   Vec2(v.x+f, v.y+f).
	 */
	inline Vec2 operator+(const Vec2& v, Flt f)
	{
		return Vec2(v.data[0] + f, v.data[1] + f);
	}

	/** @brief Subtract scalar @p f from both components of @p v.
	 *  @param v  Vector operand.  @param f  Scalar to subtract.
	 *  @return   Vec2(v.x−f, v.y−f).
	 */
	inline Vec2 operator-(const Vec2& v, Flt f)
	{
		return Vec2(v.data[0] - f, v.data[1] - f);
	}

	/** @brief Scalar–vector multiplication (scalar on the left).
	 *  @param f  Scale factor.  @param v  Vector to scale.
	 *  @return   Vec2(f·v.x, f·v.y).
	 */
	inline Vec2 operator*(Flt f, const Vec2& v)
	{
		return Vec2(v.data[0] * f, v.data[1] * f);
	}

	/** @brief Vector–scalar multiplication (scalar on the right).
	 *  @param v  Vector to scale.  @param f  Scale factor.
	 *  @return   Vec2(v.x·f, v.y·f).
	 */
	inline Vec2 operator*(const Vec2& v, Flt f)
	{
		return Vec2(v.data[0] * f, v.data[1] * f);
	}

	/** @brief Divide both components of @p v by scalar @p f.
	 *  @param v  Vector to divide.  @param f  Divisor (must be non-zero).
	 *  @return   Vec2(v.x/f, v.y/f).
	 */
	inline Vec2 operator/(const Vec2& v, Flt f)
	{
		return Vec2(v.data[0] / f, v.data[1] / f);
	}

	/** @brief Component-wise minimum of two vectors.
	 *  @param v1  First vector.  @param v2  Second vector.
	 *  @return    Vec2(min(v1.x,v2.x), min(v1.y,v2.y)).
	 */
	inline Vec2 min(const Vec2& v1, const Vec2& v2)
	{
		return Vec2(std::min(v1.x(), v2.x()), std::min(v1.y(), v2.y()));
	}

	/** @brief Component-wise maximum of two vectors.
	 *  @param v1  First vector.  @param v2  Second vector.
	 *  @return    Vec2(max(v1.x,v2.x), max(v1.y,v2.y)).
	 */
	inline Vec2 max(const Vec2& v1, const Vec2& v2)
	{
		return Vec2(std::max(v1.x(), v2.x()), std::max(v1.y(), v2.y()));
	}

	/** @brief Return a unit-length copy of @p v.
	 *
	 *  Divides both components by the Euclidean length.  Behaviour is
	 *  undefined when @p v is the zero vector (division by zero).
	 *  @param v  Vector to normalise (must have non-zero length).
	 *  @return   A new Vec2 with length 1 and the same direction as @p v.
	 */
	inline Vec2 unit(const Vec2& v)
	{
		const Flt k = 1.0f / std::sqrt(v.data[0] * v.data[0] + v.data[1] * v.data[1]);
		return Vec2(v.data[0] * k, v.data[1] * k);
	}

	/** @brief Compute the dot (inner) product of two vectors.
	 *  @param v1  First vector.
	 *  @param v2  Second vector.
	 *  @return    v1.x·v2.x + v1.y·v2.y.
	 */
	inline Flt dot(const Vec2& v1, const Vec2& v2)
	{
		return v1.data[0] * v2.data[0] + v1.data[1] * v2.data[1];
	}


} //namespace

#endif