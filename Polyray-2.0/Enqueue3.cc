/** @file Enqueue3.cc
 *  @brief Enqueue Module v3 implementation - CompositeObject factory.
 *
 *  Polyray - MIT Licensed Revival
 *  Copyright (C) 2024-2026, Clyde Meli, All rights reserved.

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
#include <queue>
#include <vector>
#include <memory>
#include <stdexcept>
#include <gsl/gsl>
#include "defs3.h"
#include "memory.h"
#include "enqueue.h"


/** @brief Construct and zero-initialise a CompositeObject.
 *
 *  Allocates a new CompositeObject, zeroes all shell pointers and state,
 *  sets the object type to T_COMPOSITE, and records the expected subobject count.
 *
 *  @param size  Number of subobjects to be stored in the composite.
 *  @return      Pointer to the newly allocated CompositeObject.
 */
CompositeObject* FactoryCompositeObject(int size)
{
    auto new_node = new CompositeObject{};// zero-initialize shell pointers/state
    //auto new_node = std::make_unique<struct exper_node_struct>();
    Ensures(new_node != nullptr);
    new_node->o_parent = nullptr;
    new_node->o_texture = nullptr;
    new_node->o_trans = nullptr;
    new_node->c_lbnd = nullptr;

    new_node->o_type = ShapeType::Composite;
    new_node->c_size = size;
    new_node->c_object.clear();//={} gives errors
    //still need to do the equivalent of
    // for (i=0;i<size;i++) {
    //       cp->c_object[i] = Prims[first + i];
    //}
    // but pushing onto the vector instead of array
    return new_node;
}


