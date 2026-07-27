//(C) C. Meli 2020-2026 Code originally from OpenPolyray project
// OpenPolyray
// Copyright(C) 2020-2026 C. Meli(vvx9jddv54@liamekaens.com)
#pragma once
#if !defined(__VEC32_DEFS)
#define _VEC32_DEFS

#include <iostream>
#include <algorithm>
//#include <vector>
//#include <memory>

//#include "rgb.h"
#include <gsl/gsl>

#ifdef VEC3_USE_SSE                                                                    
#include <immintrin.h>
#endif

namespace openpolyray
{
	inline  Vec3::Vec3(Flt e0, Flt e1, Flt e2)
	{
		e[0] = e0; e[1] = e1; e[2] = e2;
	}

	inline const Vec3& Vec3::operator+() const { return *this; }
	inline Vec3 Vec3::operator-() const { return Vec3(-e[0], -e[1], -e[2]); }

	inline Flt Vec3::length(void) const { return std::sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]); }

	inline Flt Vec3::squaredLength(void) const { return e[0] * e[0] + e[1] * e[1] + e[2] * e[2]; }

	inline void Vec3::makeUnitVector(void) { *this = *this / (*this).length(); }

	inline Flt Vec3::minComp(void) const {
		Flt temp = e[0];
		if (e[1] < temp) temp = e[1];
		if (e[2] < temp) temp = e[2];
		return temp;
	}

	inline Flt Vec3::maxComp(void) const {
		Flt temp = e[0];
		if (e[1] > temp) temp = e[1];
		if (e[2] > temp) temp = e[2];
		return temp;
	}

	inline Flt Vec3::maxAbsComp(void) const {
		Flt temp = std::fabs(e[0]);
		if (std::fabs(e[1]) > temp) temp = std::fabs(e[1]);
		if (std::fabs(e[2]) > temp) temp = std::fabs(e[2]);
		return temp;
	}

	inline Flt Vec3::minAbsComp(void) const {
		Flt temp = std::fabs(e[0]);
		if (std::fabs(e[1]) < temp) temp = std::fabs(e[1]);
		if (std::fabs(e[2]) < temp) temp = std::fabs(e[2]);
		return temp;
	}

	inline int Vec3::indexOfMinComp() const {
		int index = 0;
		Flt temp = e[0];
		if (e[1] < temp) { temp = e[1]; index = 1; }
		if (e[2] < temp) index = 2;
		return index;
	}

	inline int Vec3::indexOfMinAbsComp() const {
		int index = 0;
		Flt temp = std::fabs(e[0]);
		if (std::fabs(e[1]) < temp) { temp = std::fabs(e[1]); index = 1; }
		if (std::fabs(e[2]) < temp) index = 2;
		return index;
	}

	inline void Vec3::AddScaled(Vec3 a, Flt b, Vec3 c)
	{
		e[0] = (a)[0] + (b) * (c)[0]; 
		e[1] = (a)[1] + (b) * (c)[1]; 
		e[2] = (a)[2] + (b) * (c)[2];
	}


	inline bool operator==(const Vec3& v1, const Vec3& v2) {
		if (v1.e[0] != v2.e[0] ||
			v1.e[1] != v2.e[1] ||
			v1.e[2] != v2.e[2]) return false;
		return true;
	}

	inline bool operator!=(const Vec3& v1, const Vec3& v2) {
		return !(v1 == v2);
	}

	inline int Vec3::indexOfMaxComp() const {
		int index = 0;
		Flt temp = e[0];
		if (e[1] > temp) { temp = e[1]; index = 1; }
		if (e[2] > temp) index = 2;
		return index;
	}

	inline int Vec3::indexOfMaxAbsComp() const {
		int index = 0;
		Flt temp = std::fabs(e[0]);
		if (std::fabs(e[1]) > temp) { temp = std::fabs(e[1]); index = 1; }
		if (std::fabs(e[2]) > temp) index = 2;
		return index;
	}

	inline Vec3 operator*(Flt scalar, const Vec3& vec)
	{
		return Vec3(vec.e[0] * scalar, vec.e[1] * scalar, vec.e[2] * scalar);
	}

	inline Vec3 operator*(const Vec3& vec, Flt scalar)
	{
		return Vec3(static_cast<double>(vec.e[0]) * double(scalar),
			static_cast<double>(vec.e[1]) * scalar,
			static_cast<double>(vec.e[2]) * scalar);
	}

	inline Vec3 operator/(const Vec3& vec, Flt scalar)
	{
		return Vec3(vec.e[0] / scalar, vec.e[1] / scalar, vec.e[2] / scalar);
	}

	inline Vec3 operator+(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
	}

	inline Vec3 operator-(const Vec3& v1, const Vec3& v2)
	{
		return Vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
	}

	inline Vec3& Vec3::operator+=(const Vec3& v2)
	{
		*this = *this + v2;
		return *this;
	}

	inline Vec3& Vec3::operator=(const Vec3& v2)
	{
		e[0] = v2.e[0]; e[1] = v2.e[1]; e[2] = v2.e[2];
		return *this;
	}

	inline Vec3& Vec3::operator-=(const Vec3& v2)
	{
		*this = *this - v2;
		return *this;
	}

	inline Vec3& Vec3::operator*=(Flt t)
	{
		*this = *this * t;
		return *this;
	}

	inline Vec3& Vec3::operator/=(Flt t)
	{
		*this = *this / t;
		return *this;
	}

	inline Flt dot(const Vec3& v1, const Vec3& v2)
	{
		//std::vector<float> a{ static_cast<float>(v1.x()),static_cast<float>(v1.y()),static_cast<float>(v1.z()) };
		//std::vector<float> b{ static_cast<float>(v2.x()),static_cast<float>(v2.y()),static_cast<float>(v2.z()) };
		//return dotProductFma(a, b);
		//float dotProductFma(const std::vector<float>&a, const std::vector<float>&b)

		return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
	}

	inline Vec3 cross(const Vec3& v1, const Vec3& v2)
	{
		Vec3 temp;
		temp.e[0] = v1.y() * v2.z() - v1.z() * v2.y();
		temp.e[1] = v1.z() * v2.x() - v1.x() * v2.z();
		temp.e[2] = v1.x() * v2.y() - v1.y() * v2.x();
		return temp;
	}

	inline Vec3 unitVector(const Vec3& v)
	{
		const Flt length = v.length();
		if (length == 0)
			return v; // Vec3(1.0, 0.0, 0.0);
		else
			return v / length;
	}

	inline Vec3 minVec(const Vec3& v1, const Vec3& v2)
	{
		Vec3 vec(v1);
		if (v2.x() < v1.x()) vec.setX(v2.x());
		if (v2.y() < v1.y()) vec.setY(v2.y());
		if (v2.z() < v1.z()) vec.setZ(v2.z());
		return vec;
	}

	inline Vec3 maxVec(const Vec3& v1, const Vec3& v2)
	{
		Vec3 vec(v1);
		if (v2.x() > v1.x()) vec.setX(v2.x());
		if (v2.y() > v1.y()) vec.setY(v2.y());
		if (v2.z() > v1.z()) vec.setZ(v2.z());
		return vec;
	}

	//Normalise vector vec polyray style 1/sqrt(len)
	//some implementations of normalisation do 1/len instead
	inline Flt Vec3::VecNormalize(Vec3& vec)
	{
		Flt len = dot(vec, vec);
		if (len < PLY_EPSILON) {
			len = 1.0;
			vec.set(1.0, 0, 0.0);
		}
		else {
			const Flt slen = std::sqrt(len);
			vec = vec * (1.0f / slen);
		}
		return len;
	}

	//Normalise this vector
	inline void Vec3::Normalize(void)
	{
		Flt len = length();
		if (len < PLY_EPSILON) {
			len = 1.0;
			set(1.0, 0, 0.0);
		}
		else {
			const Flt slen = std::sqrt(len);
			e[0] *= 1.0f / slen;
			e[1] *= 1.0f / slen;
			e[2] *= 1.0f / slen;
		}
		return;
	}

	inline bool Vec3::near_zero() const {
		// Return true if the vector is close to zero in all dimensions.
		const auto s = 1e-8;
		return (std::fabs(e[0]) < s) && (std::fabs(e[1]) < s) &&
			(std::fabs(e[2]) < s);
	}

	inline Flt tripleProduct(const Vec3& v1, const Vec3& v2, const Vec3& v3)
	{
		return dot((cross(v1, v2)), v3);
	}

	inline std::istream& operator>>(std::istream& is, Vec3& vec)
	{
		return (is >> vec[0] >> vec[1] >> vec[2]);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vec3& vec)
	{
		return (os << vec[0] << vec[1] << vec[2]);
	}

	inline Vec3 reflect(const Vec3& in, const Vec3& normal)
	{
		// assumes unit length normal
		return in - normal * (2 * dot(in, normal));
	}

	using point3 = Vec3;//type alias

}

#endif