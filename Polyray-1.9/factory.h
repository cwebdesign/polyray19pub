#if !defined(__POLYRAY_FACTORY)
#define __POLYRAY_FACTORY

#include <memory>
#include <vector>
#include "defs3.h"
#include "raw.h" //for VecVerts

Special_Surface* FactorySpecial_Surface();
tstackptr Factorytexture_stack();
txstackptr Factorytransform_stack();
texture_map_entries Factorytexture_map();
texture_fn_entries Factorytexture_fn();
map_entries Factorycolor_map_entry();
HeightData *FactoryHeightData();
Texture* FactoryTexture();
std::unique_ptr<Transform> FactoryTransform();
PolynomialData* FactoryPolynomialData();
PolyData* FactoryPolyData();
RawData* FactoryRawData();
GlyphData* FactoryGlyphData();
RevolveData* FactoryRevolveData();
SweepData* FactorySweepData();
GridData* FactoryGridData();
FunctionData* FactoryFunctionData();
HypertextureData* FactoryHypertextureData();
ParametricData* FactoryParametricData();
csgnodeptr Factorycsgnode();
VecVerts* FactoryVecVerts();
Object* FactoryObject();
Object* FactoryObjectArr2();
ostackptr FactoryObject_stack();
NODE_PTR FactoryNODEPTR();
bbox_info* FactoryBBOXINFO();
SphereData* FactorySphereData();
TriangleObject* FactoryTriangleObject();
ObjectVertices* FactoryObjectVertices();
Light* FactoryObjectLight();
t_point_light* FactoryObjectPointLight();
t_spot_light* FactoryObjectSpotLight();
t_depth_light* FactoryObjectDepthLight();
t_textured_light* FactoryObjectTexturedLight();
FLARECOMP* FactoryObjectFLARECOMPLight();
PolyAlightData* FactoryObjectPolyAlightDataLight();
Img *FactoryImg();

ConeData* FactoryConeData();
BoxData* FactoryBoxData();
CylData* FactoryCylData();
DiscData* FactoryDiscData();
TorusData* FactoryTorusData();
ParabolaData* FactoryParabolaData();
SuperQData* FactorySuperQData();
TriData* FactoryTriData();
NurbData* FactoryNurbData();
BezierData* FactoryBezierData();


Object** FactoryObjPtrArr(long);
std::shared_ptr<Object[]> FactoryObjSharedPtrArr(long);
std::vector<Object> FactoryObjectVec(long size);
std::vector<std::unique_ptr<Object>> FactoryUObjectVec(long size);
std::unique_ptr<Object> FactoryUObject();
std::unique_ptr<struct exper_node_struct> FactoryUPTR();


#endif /* __POLYRAY_FACTORY */

