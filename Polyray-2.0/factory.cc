/** @file factory.cc
 *  @brief Factory functions for heap-allocating every major Polyray data structure.
 *
 *  Each factory zero-initialises the returned object and asserts allocation success
 *  via GSL Ensures before returning.
 *
 *  Polyray MIT Licensed Revival
 *  Copyright (C) 2024-2026, Clyde Meli, All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include <iomanip>
#include <iostream> 
#include <sstream> 
#include <string>
#include <bitset>
#include <vector>
#include <memory>
#include <gsl/gsl>

#include "defs3.h"
#include "raw.h"
#include "factory.h"

/** @brief Allocate and default-initialise a Special_Surface.
 *  @return Pointer to the newly allocated Special_Surface.
 */
Special_Surface* FactorySpecial_Surface()
{
	auto new_ss = new Special_Surface();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_ss != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_ss;
}

/** @brief Allocate and default-initialise a texture_stack_struct.
 *  @return Pointer to the newly allocated texture stack node.
 */
tstackptr Factorytexture_stack()
{
	auto new_t = new texture_stack_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a transform_stack_struct.
 *  @return Pointer to the newly allocated transform stack node.
 */
txstackptr Factorytransform_stack()
{
	auto new_t = new transform_stack_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a texture_map_struct.
 *  @return Pointer to the newly allocated texture map entry.
 */
texture_map_entries Factorytexture_map()
{
	auto new_t = new texture_map_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a texture_fn_struct.
 *  @return Pointer to the newly allocated texture function entry.
 */
texture_fn_entries Factorytexture_fn()
{
	auto new_t = new texture_fn_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise an Img struct.
 *  @return Pointer to the newly allocated Img node.
 */
Img *FactoryImg()
{
	auto new_t = new Img();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	new_t->filename.clear();
	new_t->copy = 0;
	new_t->bytes_per_pixel = 0;
	new_t->cflag = 0;
	new_t->width = 0;
	new_t->length = 0;
	new_t->scanline_order = 0;
	new_t->pixel_subtype = 0;
	new_t->cmlen = 0;
	new_t->cmsiz = 0;
	new_t->cmap = nullptr;
	new_t->image = nullptr;
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}
/** @brief Allocate and default-initialise a HeightData
 *  @return Pointer to the newly allocated HeightData
 */
HeightData *FactoryHeightData()
{
	auto new_t = new HeightData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}
/** @brief Allocate and default-initialise a color_map_entry.
 *  @return Pointer to the newly allocated colour-map entry.
 */
map_entries Factorycolor_map_entry()
{
	auto new_t = new color_map_entry();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a Texture.
 *  @return Pointer to the newly allocated Texture.
 */
Texture* FactoryTexture()
{
	auto new_t = new Texture();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a Transform.
 *  @return Pointer to the newly allocated Transform.
 */
std::unique_ptr<Transform> FactoryTransform()
{
	// Directly return the unique_ptr. 
    // NRVO guarantees zero-copy efficiency.
    return std::make_unique<Transform>(); 
}

/** @brief Allocate and default-initialise a PolynomialData.
 *  @return Pointer to the newly allocated PolynomialData.
 */
PolynomialData *FactoryPolynomialData()
{
	auto new_t = new PolynomialData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a PolyData.
 *  @return Pointer to the newly allocated PolyData.
 */
PolyData* FactoryPolyData()
{
	auto new_t = new PolyData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a RawData.
 *  @return Pointer to the newly allocated RawData.
 */
RawData* FactoryRawData()
{
	auto new_t = new RawData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a GlyphData.
 *  @return Pointer to the newly allocated GlyphData.
 */
GlyphData* FactoryGlyphData()
{
	auto new_t = new GlyphData();
	Ensures(new_t != nullptr);
	return new_t;
}
/** @brief Allocate and default-initialise a RevolveData.
 *  @return Pointer to the newly allocated RevolveData.
 */
RevolveData* FactoryRevolveData()
{
	auto new_t = new RevolveData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a SweepData.
 *  @return Pointer to the newly allocated SweepData.
 */
SweepData* FactorySweepData()
{
	auto new_t = new SweepData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a GridData.
 *  @return Pointer to the newly allocated GridData.
 */
GridData* FactoryGridData()
{
	auto new_t = new GridData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a FunctionData.
 *  @return Pointer to the newly allocated FunctionData.
 */
FunctionData* FactoryFunctionData()
{
	auto new_t = new FunctionData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a HypertextureData.
 *  @return Pointer to the newly allocated HypertextureData.
 */
HypertextureData* FactoryHypertextureData()
{
	auto new_t = new HypertextureData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a ParametricData.
 *  @return Pointer to the newly allocated ParametricData.
 */
ParametricData* FactoryParametricData()
{
	auto new_t = new ParametricData();
	Ensures(new_t != nullptr);
	return new_t;
}

/** @brief Allocate and default-initialise a csgnode.
 *  @return Pointer to the newly allocated CSG node.
 */
csgnodeptr Factorycsgnode()
{
	auto new_t = new csgnode();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_t != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_t;
}

/** @brief Allocate and default-initialise a VecVerts vertex array.
 *  @return Pointer to the newly allocated VecVerts.
 */
VecVerts* FactoryVecVerts()
{
	auto new_p = new VecVerts();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_p != nullptr);
	return new_p;
}


/** @brief Allocate and zero-initialise a single Object.
 *
 *  Sets o_displace, o_parent, and o_texture to nullptr.
 *
 *  @return Pointer to the newly allocated Object.
 */
Object* FactoryObject()
{
	auto new_node = new Object{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	new_node->o_displace = nullptr;
	new_node->o_parent = nullptr;
	new_node->o_texture = nullptr;
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}


/** @brief Allocate and zero-initialise an array of exactly 2 Objects.
 *
 *  Both elements have o_displace, o_parent, and o_texture set to nullptr.
 *
 *  @return Pointer to the newly allocated Object[2] array.
 */
Object* FactoryObjectArr2()
{
	auto new_node = new Object[2]{};// zero-initialize both shells
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	new_node[0].o_displace = nullptr;
	new_node[0].o_parent = nullptr;
	new_node[0].o_texture = nullptr;
	new_node[1].o_displace = nullptr;
	new_node[1].o_parent = nullptr;
	new_node[1].o_texture = nullptr;
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}


/** @brief Allocate a raw array of @p size Object pointers (uninitialized).
 *  @param size  Number of Object* slots to allocate.
 *  @return      Pointer to the newly allocated Object*[size] array.
 */
Object** FactoryObjPtrArr(long size)
{
	Object ** new_node = new ObjectPtr[size];//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate a shared array of @p size default-constructed Objects.
 *  @param size  Number of Objects to allocate.
 *  @return      shared_ptr owning the Object[size] array.
 */
std::shared_ptr<Object[]> FactoryObjSharedPtrArr(long size)
{
	//std::shared_ptr<Object[]> 
	auto new_node = std::make_shared<Object[]>(size);// new ObjectPtr[size];//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Construct a vector of @p size zero-initialised Objects.
 *
 *  Each element has o_displace, o_parent, and o_texture set to nullptr.
 *
 *  @param size  Number of Objects to create.
 *  @return      std::vector containing @p size initialised Objects.
 */
std::vector<Object> FactoryObjectVec(long size)
{
	std::vector<Object> new_node;
	new_node.reserve(size);//works
	for (int i = 0; i < size; i++)
	{
		new_node.push_back(Object());
		new_node[i].o_displace = nullptr;
		new_node[i].o_parent = nullptr;
		new_node[i].o_texture = nullptr;
		new_node[i].o_displace = nullptr;
		new_node[i].o_parent = nullptr;
		new_node[i].o_texture = nullptr;
	}
	
	return new_node;
}

/** @brief Allocate and zero-initialise a single Object as a unique_ptr.
 *
 *  Sets o_displace, o_parent, and o_texture to nullptr.
 *
 *  @return unique_ptr owning the newly allocated Object.
 */
std::unique_ptr<Object> FactoryUObject()
{
	auto new_node = std::make_unique<Object>();
	Ensures(new_node != nullptr);
	new_node->o_displace = nullptr;
	new_node->o_parent = nullptr;
	new_node->o_texture = nullptr;
	return new_node;
}
/** @brief Construct a vector of @p size unique_ptr-owned zero-initialised Objects.
 *  @param size  Number of Objects to create.
 *  @return      std::vector of unique_ptrs, each owning one initialised Object.
 */
std::vector<std::unique_ptr<Object>> FactoryUObjectVec(long size)
{
	std::vector<std::unique_ptr<Object>> new_vec;
	new_vec.reserve(size);//works
	for (int i = 0; i < size; i++)
	{
		auto new_node=FactoryUObject();
		 // unique_ptr cannot be copied, so we must move it
        new_vec.push_back(std::move(new_node));
	}
	
	return new_vec;
}

/** @brief Allocate and default-initialise an object_stack_struct.
 *  @return Pointer to the newly allocated object stack node.
 */
ostackptr FactoryObject_stack()
{
	auto new_node = new object_stack_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise an exper_node_struct (parse-tree node).
 *  @return NODE_PTR pointing to the newly allocated expression node.
 */
NODE_PTR FactoryNODEPTR()
{
	auto new_node = new exper_node_struct();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a bbox_info bounding-box structure.
 *  @return Pointer to the newly allocated bbox_info.
 */
bbox_info* FactoryBBOXINFO()
{
	auto new_node = new bbox_info();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}


/** @brief Allocate and default-initialise a ConeData structure.
 *  @return Pointer to the newly allocated ConeData.
 */
ConeData* FactoryConeData()
{
	auto new_node = new ConeData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a BoxData structure.
 *  @return Pointer to the newly allocated BoxData.
 */
BoxData* FactoryBoxData()
{
	auto new_node = new BoxData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a CylData structure.
 *  @return Pointer to the newly allocated CylData.
 */
CylData* FactoryCylData()
{
	auto new_node = new CylData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a DiscData structure.
 *  @return Pointer to the newly allocated DiscData.
 */
DiscData* FactoryDiscData()
{
	auto new_node = new DiscData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a TorusData structure.
 *  @return Pointer to the newly allocated TorusData.
 */
TorusData* FactoryTorusData()
{
	auto new_node = new TorusData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a SphereData structure.
 *  @return Pointer to the newly allocated SphereData.
 */
SphereData* FactorySphereData()
{
	auto new_node = new SphereData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a ParabolaData structure.
 *  @return Pointer to the newly allocated ParabolaData.
 */
ParabolaData* FactoryParabolaData()
{
	auto new_node = new ParabolaData();
	Ensures(new_node != nullptr);
	return new_node;
}

/** @brief Allocate and zero-initialise a SuperQData.
 *  @return Pointer to the newly allocated SuperQData.
 */
SuperQData* FactorySuperQData()
{
	auto new_node = new SuperQData();// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a TriData.
 *  @return Pointer to the newly allocated TriData.
 */
TriData* FactoryTriData()
{
	auto new_node = new TriData();// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a NurbData.
 *  @return Pointer to the newly allocated NurbData.
 */
NurbData* FactoryNurbData()
{
	auto new_node = new NurbData();// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and default-initialise a BezierData structure.
 *  @return Pointer to the newly allocated BezierData.
 */
BezierData* FactoryBezierData()
{
	auto new_node = new BezierData();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}


/** @brief Allocate and zero-initialise a TriangleObject.
 *  @return Pointer to the newly allocated TriangleObject.
 */
TriangleObject* FactoryTriangleObject()
{
	auto new_node = new TriangleObject{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}



/** @brief Allocate and default-initialise an ObjectVertices structure.
 *  @return Pointer to the newly allocated ObjectVertices.
 */
ObjectVertices* FactoryObjectVertices()
{
	auto new_node = new ObjectVertices();//works
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a Light object.
 *  @return Pointer to the newly allocated Light.
 */
Light* FactoryObjectLight()
{
	auto new_node = new Light{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
t_point_light* FactoryObjectPointLight()
{
	auto new_node = new  t_point_light{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
t_spot_light* FactoryObjectSpotLight()
{
	auto new_node = new  t_spot_light{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
t_depth_light* FactoryObjectDepthLight()
{
	auto new_node = new  t_depth_light{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}


/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
t_textured_light* FactoryObjectTexturedLight()
{
	auto new_node = new  t_textured_light{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}



/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
FLARECOMP* FactoryObjectFLARECOMPLight()
{
	auto new_node = new FLARECOMP{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

/** @brief Allocate and zero-initialise a point Light object.
 *  @return Pointer to the newly allocated Light.
 */
PolyAlightData* FactoryObjectPolyAlightDataLight()
{
	auto new_node = new PolyAlightData{};// zero-initialize shell pointers/state
	//auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	//std::cout<<"hey I used new() in factory\n"<<std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}

//in future use this..
/** @brief Allocate an exper_node_struct as a unique_ptr (future preferred path).
 *  @return unique_ptr owning the newly allocated expression node.
 */
std::unique_ptr<struct exper_node_struct> FactoryUPTR()
{
	auto new_node = std::make_unique<struct exper_node_struct>();
	Ensures(new_node != nullptr);
	//if (new_node == nullptr)
	//   serror("FactoryNODEPTR:Failed to allocate a node\n");
	std::cout << "hey I make_unique in factory\n" << std::flush;
	//NODE_PTR rawptr=new_node.release();
	return new_node;
}
