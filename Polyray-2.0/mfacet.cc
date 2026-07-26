/* mfacet.cc
  Polyray - MIT Licensed Revival
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
#include "defs3.h"
#include "vector.h"
#include "mfacet.h"

//! Initialize Phong Distribution Parameter
/*!
      Calculates the Phong exponent from a roughness angle.
      \param beta Roughness angle in radians
      \return Phong exponent (Ns parameter), or FLT_MAX if beta <= 0, 0 if beta >= pi/4
*/
float D_Phong_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   if (beta >= PYM_PI_4) return 0.0;
   return -(log(2.0) / log(cos(2.0 * beta)));
}

//! Phong Microfacet Distribution Function
/*!
      Computes the Phong distribution for specular surface reflection.
      \param N Surface normal vector
      \param L Light direction vector
      \param V View direction vector
      \param Ns Phong exponent (roughness parameter)
      \return Distribution value (0 to 1), or 0 if reflection direction is invalid
*/
float D_Phong(Vec N, Vec L, Vec V, Flt Ns)
{
   Vec VN;
   Flt RvL;

   SpecularDirection(V, N, VN);
   RvL = VecDot(VN, L);
   if (RvL < 0.0) return 0.0;
   return pow(RvL, Ns);
}

//! Initialize Blinn Distribution Parameter
/*!
      Calculates the Blinn exponent from a roughness angle.
      \param beta Roughness angle in radians
      \return Blinn exponent parameter, or FLT_MAX if beta <= 0, 0 if beta >= pi/2
*/
float D_Blinn_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   if (beta >= PYM_PI_2) return 0.0;
   return -(log(2.0) / log(cos(beta)));
}

//! Blinn Microfacet Distribution Function
/*!
      Computes the Blinn distribution for specular surface reflection.
      Uses the half-vector between view and light directions.
      \param N Surface normal vector
      \param L Light direction vector
      \param V View direction vector
      \param Ns Blinn exponent (roughness parameter)
      \return Distribution value (0 to 1), or 0 if normal is invalid
*/
float D_Blinn(Vec N, Vec L, Vec V, Flt Ns)
{
   Vec VH;
   Flt NH;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   return pow(NH, Ns);
}

//! Initialize Gaussian Distribution Parameter
/*!
      Calculates the Gaussian roughness parameter from a roughness angle.
      \param beta Roughness angle in radians
      \return Gaussian roughness parameter (C1), or FLT_MAX if beta <= 0
*/
float
D_Gaussian_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   return sqrt(log(2.0)) / beta;
}

//! Gaussian Microfacet Distribution Function
/*!
      Computes the Gaussian distribution for specular surface reflection.
      \param N Surface normal vector
      \param L Light direction vector
      \param V View direction vector
      \param C1 Gaussian roughness parameter
      \return Distribution value based on Gaussian falloff from the reflection direction
*/
float
D_Gaussian(Vec N, Vec L, Vec V, Flt C1)
{
   Vec VH;
   Flt NH, temp;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   temp = acos(NH) * C1;
   return exp(-(temp * temp));
}

//! Initialize Reitz Distribution Parameter
/*!
      Calculates the Reitz roughness parameter from a roughness angle.
      \param beta Roughness angle in radians
      \return Reitz roughness parameter (C2_2), or 0 if beta <= 0
*/
float
D_Reitz_Init(Flt beta)
{
   Flt cos_beta;
   if (beta <= 0.0) return 0.0;
   cos_beta = cos(beta);
   return (cos_beta * cos_beta - 1.0) / (cos_beta * cos_beta - sqrt(2.0));
}

//! Reitz Microfacet Distribution Function
/*!
      Computes the Reitz (Beckmann) distribution for specular surface reflection.
      \param N Surface normal vector
      \param L Light direction vector
      \param V View direction vector
      \param C2_2 Reitz roughness parameter squared
      \return Distribution value based on Reitz formula
*/
float
D_Reitz(Vec N, Vec L, Vec V, Flt C2_2)
{
   Vec VH;
   Flt NH, temp;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   temp = C2_2 / ((NH * NH * (C2_2 - 1.0)) + 1.0);
   return temp * temp;
}

//! Initialize Cook-Torrance Distribution Parameter
/*!
      Calculates the Cook-Torrance roughness parameter from a roughness angle.
      \param beta Roughness angle in radians
      \return Cook-Torrance roughness parameter (m_2), or 0 if beta <= 0, FLT_MAX if beta >= pi/2
*/
float
D_Cook_Init(Flt beta)
{
   Flt tan_beta;
   if (beta <= 0.0) return 0.0f;
   if (beta >= PYM_PI_2) return FLT_MAX;
   tan_beta = tan(beta);
   return -(tan_beta * tan_beta) / log(pow(cos(beta), 4.0) / 2.0);
}

//! Cook-Torrance Microfacet Distribution Function
/*!
      Computes the Cook-Torrance distribution for specular surface reflection.
      More physically accurate model for rough surfaces.
      \param N Surface normal vector
      \param L Light direction vector
      \param V View direction vector
      \param m_2 Cook-Torrance roughness parameter squared
      \return Distribution value based on Cook-Torrance formula, or 0 if reflection is invalid
*/
float
D_Cook(Vec N, Vec L, Vec V, Flt m_2)
{
   Vec VH;
   Flt NH, temp;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   temp = -(1.0 - NH * NH) / (NH * NH * m_2);
   return exp(temp) / (4.0 * PYM_PI * m_2 * pow(NH, 4.0));
}
