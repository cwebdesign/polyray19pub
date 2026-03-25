//mfacet.c
/*
  Polyray � MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the �Software�), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "defs.h"
#include "vector.h"
#include "mfacet.h"

float
D_Phong_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   if (beta >= M_PI_4) return 0.0;
   return -(log(2.0) / log(cos(2.0 * beta)));
}

float
D_Phong(Vec N, Vec L, Vec V, Flt Ns)
{
   Vec VN;
   Flt RvL;

   SpecularDirection(V, N, VN);
   RvL = VecDot(VN, L);
   if (RvL < 0.0) return 0.0;
   return pow(RvL, Ns);
}

float
D_Blinn_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   if (beta >= M_PI_2) return 0.0;
   return -(log(2.0) / log(cos(beta)));
}

float
D_Blinn(Vec N, Vec L, Vec V, Flt Ns)
{
   Vec VH;
   Flt NH;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   return pow(NH, Ns);
}

float
D_Gaussian_Init(Flt beta)
{
   if (beta <= 0.0) return FLT_MAX;
   return sqrt(log(2.0)) / beta;
}

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

float
D_Reitz_Init(Flt beta)
{
   Flt cos_beta;
   if (beta <= 0.0) return 0.0;
   cos_beta = cos(beta);
   return (cos_beta * cos_beta - 1.0) / (cos_beta * cos_beta - sqrt(2.0));
}

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

float
D_Cook_Init(Flt beta)
{
   Flt tan_beta;
   if (beta <= 0.0) return 0.0f;
   if (beta >= M_PI_2) return FLT_MAX;
   tan_beta = tan(beta);
   return -(tan_beta * tan_beta) / log(pow(cos(beta), 4.0) / 2.0);
}

float
D_Cook(Vec N, Vec L, Vec V, Flt m_2)
{
   Vec VH;
   Flt NH, temp;
   VecH(V,L,VH);
   NH = VecDot(N, VH);
   if (NH < 0.0) return 0.0;
   temp = -(1.0 - NH * NH) / (NH * NH * m_2);
   return exp(temp) / (4.0 * M_PI * m_2 * pow(NH, 4.0));
}
