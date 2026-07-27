/* ripple.cc

  Polyray - MIT Licensed Revival
  Copyright (C) 2021-2024, Clyde Meli, All rights reserved.

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

#include <iostream>
#include "defs3.h"
//#include "display.h"
//#include "memory.h"
#include "io_ply.h"
#include "ripple.h"
#include "vector.h"
#include "vec3.h"

namespace openpolyray
{
    /** @brief Initialize the pseudo-random ripple centers used by the procedural ripple normal perturbation.
     *  @return No return value.
     */
    void Ripple::init_ripples(void)
    {
        srand(42);
        for (int i = 0; i < MAX_RIPPLE_CENTERS; i++) {
            MakeVector(random_number, random_number, random_number,
                ripple_centers[i]);
            /* Spread them within the unit cube */
            for (int j = 0; j < 3; j++)
                ripple_centers[i][j] = 2.0 * (ripple_centers[i][j] - 0.5);
        }
        ripple_init_flag = 1;
    }

    /** @brief Perturb a surface normal using the procedural ripple field.
     *  @param P      Sample position in object or texture space.
     *  @param N      Input normal vector, updated and normalized before return.
     *  @param freq   Ripple frequency multiplier.
     *  @param phase  Ripple phase offset.
     *  @param scale  Ripple displacement scale.
     *  @return The perturbed normal as a Vec3 value.
     */
    Vec3 Ripple::ripples(Vec P, Vec N, Flt freq, Flt phase, Flt scale)
    {
        int i;
        Vec Pt;
        Flt len, val;        

        if (!ripple_init_flag) init_ripples();

        for (i = 0; i < MAX_RIPPLE_CENTERS; i++) {
            VecCopy(P, Pt);
            VecSub(Pt, ripple_centers[i], Pt);
            len = VecDot(Pt, Pt);
            if (len < PLY_EPSILON)
                len = 1.0;
            else
                len = sqrt(len);

            val = cos(len * freq + phase) * scale /
                (len * (Flt)MAX_RIPPLE_CENTERS);

            VecAddS(val, Pt, N, N);
        }
        VecNormalize(N);
        Vec3 N3{ N };
        return N3;
    } //eval_ripple



}//namespace

