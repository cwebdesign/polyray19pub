/** @file defs3.h
 *  @brief Global types, constants, vector macros, and scene-graph structs.
 *
 *  This is the single most-included header in the renderer.  It defines:
 *  - Scalar and vector typedefs (@c Flt, @c Vec, @c fVec)
 *  - All scene-graph structs (@c t_object, @c t_ray, @c t_isect)
 *  - Vector arithmetic macros (VecAdd, VecDot)
 *  - Numeric constants and floating-point comparison utilities
 *
 *  Polyray MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

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
#pragma once
#ifndef __DEFSH
#define __DEFSH 	//CM:200206

#include <cstdint>//for int32_t

#include <iostream>
#include <array>


#if  __has_include(<numbers>) && !defined(__clang__)
#include <numbers>
#endif//numbers

#include <ios>
#include <cstdlib>
#include <variant>

#include <cmath>
#include <setjmp.h>
#include <string.h>
#include <float.h>

#ifdef __sun
#include <sys/filio.h> //for FIONREAD
#endif


/** @brief Output image format selection. */
enum class OUT_FORMAT {
    OUT_TARGA  = 1, /**< 24-bit Targa (.tga) output. */
    OUT_RAWPPM = 2  /**< Raw PPM (.ppm) output. */
};

/** @brief Linkage qualifier for functions that must be visible during unit testing.
 *
 *  Expands to nothing (external linkage) when @c TESTING is defined, so that
 *  normally-@c static functions are reachable from GTest.  Otherwise expands
 *  to @c static.
 */
#ifdef TESTING
  #define UNIT_STATIC(x) x
#else
#define UNIT_STATIC(x) static
#endif
/*CM end */

#ifdef __APPLE__
#define MAC
#endif

/*CM added 28/feb/2001 // 15/11/2024 */
#if defined( WINTARGET )
#include <windows.h>
#define _WINDOWS
#endif
/*CM end */

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _kbhit()

using UWORD32 = uint32_t;//not a type
using UWORD64 = uint64_t;
#else
#if __APPLE__
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


//unsigned long _wait=0;
using UWORD32 = uint32_t;//not a type
using UWORD64 = uint64_t;

/** @brief Non-blocking keyboard-hit test (macOS implementation).
 *  @return 1 if a key is waiting in stdin, 0 otherwise.
 */
static int _kbhit(void)
{
  struct termios oldt, newt;
  int oldf;


  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);


  int ch = getchar();


  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);


  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }


  return 0;
}

#else
//linux kbhit from  https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
//added the includes
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
/** @brief Non-blocking keyboard-hit test (Linux implementation).
 *  @return Number of bytes waiting in stdin (non-zero means a key is ready).
 */
inline int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (!initialized) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#endif
#endif

//CM added 4/jan/2024
/** @brief Block until the user presses Enter (cross-platform pause helper). */
inline void defs3_hitanykey(void)
{
    //slog(std::make_tuple("Press ENTER to continue\n");
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //polyray::pause();//cm160519
}
/*CM end */

//#include "polyraync2.tab.h" //todo:for STRING. later change to enum class for exper_node_struct
constexpr int TAB_STRING = 453;//todo:the above.

/* Define some global constants */
constexpr int MAXLEVEL            = 5;   /**< Maximum ray-recursion depth. */
constexpr int MAX_CONDITION_DEPTH = 16;  /**< Maximum nesting depth of @c if/@c else blocks in the scene language. */
constexpr int VECTOR_LENGTH       = 4;   /**< Component count including the homogeneous coordinate. */
constexpr int POLY_NMAX           = 32;  /**< Maximum number of sides in a polygon primitive. */
constexpr int PQSIZE              = 128; /**< Priority queue capacity for the BVH enqueue module. */

constexpr int MAX_POLYNOMIAL_ORDER = 34; /**< Highest polynomial degree supported by the implicit-surface solver. */

constexpr int YYEOL = -1; /**< Lexer sentinel: end of current input line. */

constexpr float PLY_HUGE    = 1.0e6F;  /**< Practical infinity for ray distances and bounding volumes. */
constexpr float SMALL       = 1.0e-3F; /**< Small tolerance used for near-clip and geometry tests. */
constexpr float PLY_EPSILON = 1.0e-8F; /**< General floating-point epsilon for surface intersection tests. */

#if  __has_include(<numbers>) && !defined(__clang__)
//using PY prefix for M_PI etc due to older gcc
constexpr float PYM_PI = std::numbers::pi_v<float>;
constexpr float PYM_PI_4 = std::numbers::pi_v<float> /4;
constexpr float PYM_PI_3 = std::numbers::pi_v<float> /3;
constexpr float PYM_PI_2 = std::numbers::pi_v<float> /2;
constexpr float TWO_PI = 2*std::numbers::pi_v<float>;
constexpr float TWO_PI_3 = 2*std::numbers::pi_v<float> /3;//historic polyray name. actually 4*PI/3

#else //old style
#undef PYM_PI
constexpr float PYM_PI = 3.1415926535897932384626433832795029L;
//#if defined(_MSC_VER)
//#ifndef M_PI_4
constexpr float PYM_PI_4 = 0.7853981633974483096156608458198757L;
//0.785398163397448309616;
//#endif
//#ifndef M_PI_2
constexpr float PYM_PI_2 = 1.57079632679489661923;
//#endif

#ifndef TWO_PI
constexpr float TWO_PI = 6.283185207179586476925286766560;
#endif



#endif //has_include numbers

constexpr double PYTWO_PI_3 = 2.0943951023931954923084F;
constexpr double PYTWO_PI_43 = 4.1887902047863909846168F; 
//historic polyray name. actually 4*PI/3

constexpr double PLY_SQRT2 = std::numbers::sqrt2;// 1.41421356237309504880F;
constexpr double PLY_SQRT3 = std::numbers::sqrt3;// 1.73206;//for superq.cc

constexpr float BARY_VAL1 = -0.005;
constexpr float BARY_VAL2 = 1.0001;


/** @brief Type-punning union for IEEE 754 single-precision float bit manipulation.
 *
 *  Allows safe extraction of sign, exponent, and mantissa bits for
 *  ULP-based floating-point comparison.  Based on the technique at
 *  https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
 */
union Float_t
{
    Float_t(float num = 0.0f) : f(num) {}
    bool    Negative()    const { return i < 0; }                        /**< True when the float is negative. */
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }         /**< Raw 23-bit mantissa field. */
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }            /**< Raw 8-bit biased exponent field. */

    int32_t i; /**< Integer overlay for bit-level access. */
    float   f; /**< Floating-point value. */
#ifdef _DEBUG
    struct
    {   // Bitfields for exploration. Do not use in production code.
        uint32_t mantissa : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } parts;
#endif
};
/** @brief Compare two floats for near-equality using an absolute threshold and a ULP distance.
 *
 *  Returns true if |A−B| <= @p maxDiff (near-zero guard) OR the ULP distance
 *  between A and B is <= @p maxUlpsDiff.
 *  @param A            First value.
 *  @param B            Second value.
 *  @param maxDiff      Absolute difference threshold (handles near-zero).
 *  @param maxUlpsDiff  Maximum allowed ULP distance.
 *  @return             True when A and B are considered equal.
 */
inline bool AlmostEqualUlpsAndAbs(float A, float B,
    float maxDiff, int maxUlpsDiff)
{
    // Check if the numbers are really close -- needed
    // when comparing numbers near zero.
    float absDiff = fabs(A - B);
    if (absDiff <= maxDiff)
        return true;

    Float_t uA(A);
    Float_t uB(B);

    // Different signs means they do not match.
    if (uA.Negative() != uB.Negative())
        return false;

    // Find the difference in ULPs.
    int ulpsDiff = std::abs(uA.i - uB.i);
    if (ulpsDiff <= maxUlpsDiff)
        return true;
    return false;
}
////code from: https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/?utm_source=pocket_mylist
/** @brief Compare two floats for near-equality using an absolute and a relative threshold.
 *
 *  Returns true if |A−B| <= @p maxDiff OR |A−B| <= max(|A|,|B|) * @p maxRelDiff.
 *  @param A          First value.
 *  @param B          Second value.
 *  @param maxDiff    Absolute difference threshold (handles near-zero).
 *  @param maxRelDiff Relative threshold; defaults to @c FLT_EPSILON.
 *  @return           True when A and B are considered equal.
 */
inline bool AlmostEqualRelativeAndAbs(float A, float B,
    float maxDiff, float maxRelDiff = std::numeric_limits<float>::epsilon())
    //FLT_EPSILON)
    //might have problem with negative nos, have to abs them for it to work! cm
{
    // Check if the numbers are really close -- needed
    // when comparing numbers near zero.
    float diff = fabs(A - B);
    if (diff <= maxDiff)
        return true;

    A = fabs(A);
    B = fabs(B);
    float largest = (B > A) ? B : A;

    if (diff <= largest * maxRelDiff)
        return true;
    return false;
}

//template <class Type, class Container = std::deque<Type> > class stack;

/** @brief Rendering mode selection. */
enum class rmode {
    RAY_TRACING      = 1, /**< Full recursive ray tracing. */
    SCAN_CONVERSION  = 2, /**< Polygon scan-line rasterisation. */
    WIRE_FRAME       = 3, /**< Wireframe edge rendering. */
    HIDDEN_LINE      = 4, /**< Hidden-line removal. */
    GOURAD_SHADE     = 5, /**< Gouraud-shaded scan conversion. */
    RAW_TRIANGLES    = 6, /**< Output raw triangle geometry. */
    UV_TRIANGLES     = 7, /**< Output UV-mapped triangles. */
    CSG_TRIANGLES    = 8, /**< Output CSG-clipped triangles. */
    MESH_CONVERSION  = 9, /**< Convert object to mesh. */
    LAST_RENDER_MODE = 9  /**< Sentinel — highest valid mode index. */
};

/** @brief BVH acceleration structure selection. */
enum class slabbsp {
    NONE  = 0, /**< No acceleration structure; brute-force ray testing. */
    SLABS = 1, /**< Slab-based bounding volume hierarchy. */
    BSP   = 2  /**< Binary space partitioning tree. */
};

#define SHADOW_CHECK      0x0001
//#define REFLECT_CHECK     0x0002
constexpr unsigned short REFLECT_CHECK = 0x0002;
#define TRANSMIT_CHECK    0x0004
#define TWO_SIDED_SURFS   0x0008
#define UV_CHECK          0x0010
#define NORMAL_CORRECT    0x0020
#define CAST_SHADOW       0x0040
#define SMOOTH_FLAG       0x0080
#define ADAPTIVE_UV       0x0100
#define PARTICLE_FLAG     0x0200
#define UNSET_SFLAG       0x8000
#define ALL_SHADE_FLAGS (SHADOW_CHECK | REFLECT_CHECK | TRANSMIT_CHECK |\
                         TWO_SIDED_SURFS | UV_CHECK | NORMAL_CORRECT)

//#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)<(y)?(y):(x))

/** @brief Return true when @p x > @p y (i.e. the pair is out of ascending order). */
#ifdef __clang__
inline bool out_of_order(int x, int y)     { return y < x; }
/** @brief Return true when @p x <= @p y (pair is in ascending order). */
inline bool not_out_of_order(int x, int y) { return !out_of_order(x, y); }
/** @brief Return the larger of @p x and @p y (stable: prefers @p x on tie). */
inline int  stablemax(int x, int y)        { return out_of_order(x, y) ? x : y; }
/** @brief Return the smaller of @p x and @p y.
 *  @param x  First value.
 *  @param y  Second value.
 *  @return   min(x, y).
 */
inline int  MIN(int x, int y)              { return out_of_order(x, y) ? y : x; }
#else
/** @brief Return true when @p x > @p y (generic C++20 version). */
inline bool out_of_order(auto x, auto y)     { return y < x; }
/** @brief Return true when @p x <= @p y (generic C++20 version). */
inline bool not_out_of_order(auto x, auto y) { return !out_of_order(x, y); }
/** @brief Return the larger of @p x and @p y (generic C++20 version). */
auto inline stablemax(auto x, auto y)        { return out_of_order(x, y) ? x : y; }
/** @brief Return the smaller of @p x and @p y (generic C++20 version).
 *  @param x  First value.
 *  @param y  Second value.
 *  @return   min(x, y).
 */
auto inline PLY_MIN(auto x, auto y)          { return out_of_order(x, y) ? y : x; }
auto inline PLY_MIN(auto x, auto y, auto z) { return PLY_MIN(x, PLY_MIN(y, z)); }
#endif //clang

//todo: phase these out
//#define SGN(x) ((x)<0?-1:((x)>0?1:0))
//source of this SGN: https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c/10133700
#undef SGN
/** @brief Return the mathematical sign of @p val: -1, 0, or +1.
 *  @tparam T   Any ordered type.
 *  @param  val Value to test.
 *  @return     -1 if val < 0, 0 if val == 0, +1 if val > 0.
 */
template <typename T> int SGN(T val) {
    return (T(0) < val) - (val < T(0));
}

#define ABS(x) (std::abs(x))
#define POLYRAYequal(x, y) (std::fabs((x)-(y)) < PLY_EPSILON ? 1 : 0)


using Flt  = double;       /**< Primary scalar floating-point type used throughout the renderer. */
using LFlt = long double;  /**< Extended-precision scalar for Sturm-sequence polynomial solvers. */
typedef Flt fVec[3];       /**< 3-component single-precision-compatible vector (Flt[3]). */
typedef Flt Vec[3];        /**< 3-component world/object space vector (Flt[3]). */

using NuVec = std::array<Flt, 3>;
using NuMatrix = std::array<fVec,3>;

typedef Flt Matrix[4][4];
//#include <xmmintrin.h>
#if  __has_include(<immintrin.h>)
#include <immintrin.h>
struct SuperMatrix {
    union {
        Matrix f;
        __m128 m[4];
        __m256 n[2];
    };
};
#endif

typedef Flt Matrix2[2][2];
typedef Matrix2 MMatrix2[2][2];
/** @brief Packed RGBA pixel (8 bits per channel). */
typedef struct { unsigned char r, g, b, o; } rgbo;

extern Vec ZeroVector; // = { 0.0, 0.0, 0.0 };



#define disable()
#define enable()

//#define disable() __asm__ __volatile__ ("cli");

//#define enable() __asm__ __volatile__ ("sti");


  #define times2(arg1, arg2) (arg2)= (long)(arg1) * 2L;
  #define times3(arg1, arg2) (arg2)= (long)(arg1) * 3L;
  #define times4(arg1, arg2) (arg2)= (long)(arg1) * 4L;
  #define times5(arg1, arg2) (arg2)= (long)(arg1) * 5L;
  #define times6(arg1, arg2) (arg2)= (long)(arg1) * 6L;
  #define times7(arg1, arg2) (arg2)= (long)(arg1) * 7L;
  #define times8(arg1, arg2) (arg2)= (long)(arg1) * 8L;
  #define times9(arg1, arg2) (arg2)= (long)(arg1) * 9L;

/** @brief Compute the reciprocal of @p a.
 *  @param a  Divisor; must not be zero.
 *  @return   1.0 / a.
 */
inline double RECIPROCAL(double a)  {
   return 1.0/(a);
}

//#define MUL(a,b) ((a)*(b))
//#define HALF(a) ((a)*0.5))
//#define TWICE(a) ((a)*2.0)



#define degtorad(x)     (((Flt)(x))*PYM_PI/180.0)
#define radtodeg(x)     (((Flt)(x))*180.0/PYM_PI)

#define MakeVector(x, y, z, v) {(v)[0]=(x);(v)[1]=(y);(v)[2]=(z);}

#define MakeVectorZero(v) {(v)[0]=(v)[1]=(v)[2]=0.0;}

#define MakeVectorZero1(v) {(v)[0]=(v)[1]=(v)[2]=0.0;}

//#define MakeVectorZero(v) {memcpy((v),ZeroVector,sizeof(ZeroVector));}

//#define MakeVectorZero1(v) {memcpy((v),ZeroVector,sizeof(ZeroVector));}



#define VecScale(S,a)   {(a)[0] *= S ; (a)[1] *= S ; (a)[2] *= S;}
#define VecNegate(a)  {(a)[0]=0-(a)[0];\
                       (a)[1]=0-(a)[1];\
                       (a)[2]=0-(a)[2];}
#define VecClose(a, b)  ( (fabs((a)[0] - (b)[0]) < PLY_EPSILON) &&\
                          (fabs((a)[1] - (b)[1]) < PLY_EPSILON) &&\
                          (fabs((a)[2] - (b)[2]) < PLY_EPSILON) ? 1 : 0)
#define VecDot(a,b)   ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a)     (sqrt(VecDot(a,a)))
#define VecCopy(a,b)    {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}

template<typename T> void NuVecCopy(T a, NuVec& b) //copies a TO b
{
    b = {};
    b[0] = a[0]; b[1] = a[1]; b[2] = a[2];
}
template<typename T> void NuVec2Vec(T a, Vec& b)
{
    b[0] = a[0]; b[1] = a[1]; b[2] = a[2];
}


#define VecAdd(a,b,c)    {(c)[0]=(a)[0]+(b)[0];\
                          (c)[1]=(a)[1]+(b)[1];\
                          (c)[2]=(a)[2]+(b)[2];}
#define VecAddScaled(a,b,c,d) {(d)[0]=(a)[0]+(b)*(c)[0];\
                               (d)[1]=(a)[1]+(b)*(c)[1];\
                               (d)[2]=(a)[2]+(b)*(c)[2];}
#define VecSub(a,b,c)    {(c)[0]=(a)[0]-(b)[0];\
                          (c)[1]=(a)[1]-(b)[1];\
                          (c)[2]=(a)[2]-(b)[2];}
#define VecComb(A,a,B,b,c) {(c)[0]=(A)*(a)[0]+(B)*(b)[0];\
                            (c)[1]=(A)*(a)[1]+(B)*(b)[1];\
                            (c)[2]=(A)*(a)[2]+(B)*(b)[2];}
#define VecAddS(A,a,b,c)   {(c)[0]=(A)*(a)[0]+(b)[0];\
                            (c)[1]=(A)*(a)[1]+(b)[1];\
                            (c)[2]=(A)*(a)[2]+(b)[2];}
#define VecCross(a,b,c)    {(c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1];\
                            (c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2];\
                            (c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0];}
#define VecHalf(a, b, c) { (a)[0] = 0.5 * ((b)[0] + (c)[0]);\
                           (a)[1] = 0.5 * ((b)[1] + (c)[1]);\
                           (a)[2] = 0.5 * ((b)[2] + (c)[2]);}

/* Definitions to make transformations cleaner */
#define TxVector(a, b, c) TxVec(a, b, c);
#define TxDirection(a, b, c) TxVec3(a, b, c);
#define InvTxVector(a, b, c) InvTxVec(a, b, c);
#define InvTxVector1(a, b, c) InvTxVec1(a, b, c);
#define InvTxDirection(a, b, c) InvTxVec3(a, b, c);

/* Forward declarations of some fundamental types */
//typedef struct t_object Object;
using Object = struct t_object;
using ObjectPtr = Object *;
typedef struct t_object_tree BinTree;
typedef struct t_texture Texture;
typedef struct t_surface Surface;
typedef struct t_special_surface Special_Surface;
typedef struct t_noise_surface Noise_Surface;
typedef struct Transformation_Struct Transform;
typedef struct t_ray Ray;
typedef struct t_viewpoint Viewpoint;
typedef struct t_objectprocs ObjectProcs;
typedef struct t_isect Isect;
typedef struct csgnode *csgnodeptr;
typedef struct color_map_entry *map_entries;
typedef struct texture_map_struct *texture_map_entries;
typedef struct texture_fn_struct *texture_fn_entries;
typedef struct subst_struct *SUBST_PTR;
typedef struct exper_node_struct *NODE_PTR;
typedef struct exper_list_struct *LIST_PTR;
typedef struct blob_list_struct *blobstackptr;
typedef struct object_stack_struct *ostackptr;
typedef struct object_list_struct objlist, *objlistptr;
typedef struct transform_stack_struct *txstackptr;
typedef struct texture_stack_struct *tstackptr;


/** @brief Geometric data for a sphere or ellipsoid primitive. */
typedef struct {
    Vec sph_center;  /**< Centre point in object space. */
    Flt sph_radius;  /**< Radius. */
    Flt sph_radius2; /**< Radius squared (precomputed for intersection tests). */
} SphereData;

/** @brief This is a placeholder for primitive data for Box */
struct BoxData {
    Flt bounds[2][3];
};

struct SuperQData {
    Flt n, e;
};




constexpr int DEFAULT_SAMPLES=25;
constexpr Flt DEFAULT_THRESHOLD=0.2;

/* Basic primitive types: */
constexpr int T_NULL =             (0);
constexpr int T_BLOB =             (1);
constexpr int T_BOX =              (2);
constexpr int T_BEZIER =           (3);
constexpr int T_CONE =             (4);
constexpr int T_CSG =              (5);
constexpr int T_CYLINDER =         (6);
constexpr int T_CYL_HEIGHT_FIELD = (7);
constexpr int T_DISC =             (8);
constexpr int T_FUNCTION =         (9);
constexpr int T_GLYPH =           (10);
#define T_GRIDDED          (11)
#define T_HEIGHT_FIELD     (12)
#define T_HYPERTEXTURE     (13)
#define T_LIGHT            (14)
#define T_NURB             (15)
#define T_PARABOLA         (16)
#define T_PARAMETRIC       (17)
#define T_POLY             (18)
#define T_POLYNOMIAL       (19)
#define T_RAW_TRIANGLES    (20)
#define T_REVOLVE          (21)
#define T_SPHERE           (22)
#define T_SPH_HEIGHT_FIELD (23)
#define T_SUPERQ           (24)
#define T_SWEEP            (25)
#define T_TORUS            (26)
#define T_TRI              (27)
/* Specialized object types: */
#define T_COMPOSITE     (28)
#define T_POLYGON       (29)

constexpr int FIRST_OBJECT_TYPE = (1);
constexpr int LAST_OBJECT_TYPE = (29);

/* Define the types of entries in a CSG tree */
#define T_BASE_OBJECT    (50)
#define T_UNION          (51)
#define T_INTERSECTION   (52)
#define T_INVERSE        (53)
#define T_CLIP           (54)
#define T_MERGE          (55)

/* Define the distinct types of textures */
#define T_PLAIN          (100)
#define T_CHECKER        (101)
#define T_SPECIAL        (102)
#define T_HEXAGON        (103)
#define T_NOISE          (104)
#define T_LAYERED        (105)
#define T_INDEXED        (106)
#define T_SUMMED         (107)

/* Define the types of entries in the symbol table */
#define T_STRING         (160)     /* Character string */
#define T_OBJECT         (161)     /* A object definition */
#define T_SURFACE        (162)     /* A surface definition */
#define T_TEXTURE        (163)     /* A texture definition */
#define T_EXPRESSION     (164)     /* An expression definition */
#define T_TRANSFORM      (165)     /* A transformation definition */
#define T_TEXTURE_MAP    (166)     /* A texture map definition */
#define T_PARTICLE       (167)     /* A particle generator */

/** @brief A ray defined by an origin and a direction. */
struct t_ray {
   Vec P; /**< Origin (start location) of the ray in world space. */
   Vec D; /**< Direction of the ray (not necessarily unit length). */
   };

/** @brief A 4×4 affine transform stored alongside its inverse. */
struct Transformation_Struct {
   Matrix matrix;  /**< Forward transformation matrix. */
   Matrix inverse; /**< Inverse of @c matrix, kept in sync. */
   };


/** @brief Geometric data for a cone or truncated cone primitive. */
struct ConeData {
    short int closed; /**< Non-zero when the cone ends are capped. */
    Vec top, bot;     /**< Centre points of the top and bottom caps. */
    Flt trad, brad;   /**< Radii of the top and bottom caps. */
    Flt dist;         /**< Distance between top and bottom centres. */
    Transform trans;  /**< Transform from canonical to world space. */
};

/** @brief This is a placeholder for primitive data for Cylinder */
struct CylData {
   short int closed;
   Vec bot, top;
   Flt radius;
   Transform trans;
};

/** @brief This is a placeholder for primitive data for Disc */
struct DiscData {
   Vec center;
   Vec normal;
   Flt iradius, oradius;
   Flt iradius2, oradius2, d;
};

/** @brief Primitive data for a torus shape.
 *
 *  The torus is defined by a @c center point, a normalised axis @c dir,
 *  a major radius @c r0, and a minor (tube) radius @c r1.  Both radii are
 *  stored pre-squared to avoid repeated multiplications during intersection.
 *  @c Sturm_Flag selects the quartic root-finder used by the intersect
 *  routine (0 = Ferrari, 1 = Vieta, 2 = Sturm–Bisection).
 *  @c trans maps world-space rays into the canonical torus frame.
 */
struct TorusData {
   Vec center, dir; /**< Stored center and normalised axis. */
   Flt r0, r1;      /**< Major and minor radii stored SQUARED. */
   int Sturm_Flag;  /**< Quartic solver selector: 0=Ferrari, 1=Vieta, 2=Sturm. */
   Transform trans; /**< Canonical-space transform. */
};

/** @brief Geometric data for a paraboloid primitive. */
struct ParabolaData {
    short int closed; /**< Reserved; cap flag (not currently used). */
    Vec top, bot;     /**< Apex and base-centre points in world space. */
    Flt radius;       /**< Radius of the base circle. */
    Transform trans;  /**< Transform from world space to canonical paraboloid space. */
};

/** @brief In-memory representation of a loaded image (Targa or PPM). */
struct Img {
   char *filename;        /**< Source file path. */
   int copy;              /**< 1 when this is a shallow copy that does not own its data. */
   int bytes_per_pixel;             /**< Bytes per pixel. */
   int cflag;             /**< 1 when this is a colour-mapped (paletted) image. */
   unsigned width;        /**< Width in pixels. */
   unsigned length;       /**< Height in pixels. */
   unsigned scanline_order;        /**< Scan-line order: 0 = top-to-bottom, 1 = bottom-to-top. */
   unsigned pixel_subtype;        /**< Pixel sub-type identifier. */
   unsigned cmlen;        /**< Number of entries in the colour map. */
   unsigned cmsiz;        /**< Bytes per colour-map entry. */
   unsigned char *cmap;   /**< Colour map array (palette). */
   unsigned char **image; /**< Row pointers into the pixel data. */
   };

/** @brief Evaluated (numeric) surface shading parameters for a single intersection. */
struct t_surface {
   fVec  Ka_color;                 /**< Ambient colour (RGB). */
   float Ka_scale;                 /**< Ambient intensity multiplier. */
   float Kb_power;                 /**< Brilliance exponent modifying diffuse response. */
   fVec  Kd_color;                 /**< Diffuse colour (RGB). */
   float Kd_scale;                 /**< Diffuse intensity scale. */
   fVec  Ks_color;                 /**< Specular highlight colour (RGB). */
   float Ks_scale;                 /**< Specular intensity scale. */
   fVec  Kr_color;                 /**< Reflection colour filter (RGB). */
   float Kr_scale;                 /**< Reflectivity; 0 = no reflection. */
   fVec  Kt_color;                 /**< Transmission colour filter (RGB). */
   float Kt_scale;                 /**< Transmissivity; 0 = opaque. */
   float (*D)(Vec, Vec, Vec, Flt); /**< Microfacet distribution function pointer (nullptr = Phong). */
   float D_coeff;                  /**< Pre-computed coefficient for the distribution function. */
   float ior;                      /**< Index of refraction (1.0 = air). */
   };

/** @brief A spline curve definition used for lathe/revolve and sweep paths. */
struct spline_node {
   int      spline_type; /**< Interpolation type (e.g. Bezier, B-spline). */
   int      ctl_count;   /**< Number of control points. */
   int      copy_flag;   /**< Non-zero when this node does not own its arrays. */
   NODE_PTR param;       /**< Parameter expression for animated splines. */
   Vec *ctl_points;      /**< Array of @c ctl_count control points. */
   Vec *ctl_derivs;      /**< Derivative vectors at each control point. */
   Flt  param_dist;      /**< Total arc-length parameter range. */
   Flt *ctl_params;      /**< Parameter values at each control point. */
   };

/** @brief Polymorphic texture object, dispatching via function pointers. */
struct t_texture {
   unsigned short type;      /**< Texture kind (T_PLAIN, T_CHECKER, T_SPECIAL, …). */
   short copy_flag;          /**< Non-zero when this texture does not own its @c data. */
   void (*del)(Texture *);   /**< Destructor: frees type-specific @c data. */
   /** @brief Evaluate the texture at a surface point and return shading parameters.
    *  @param eye        Active viewpoint.
    *  @param obj        Object being shaded.
    *  @param tex        This texture.
    *  @param P          Object-space intersection point.
    *  @param N          Surface normal in world space.
    *  @param I          Incident ray direction.
    *  @param UV         UV/W texture coordinates.
    *  @param u          Scalar U coordinate.
    *  @param v          Scalar V coordinate.
    *  @param lindex     Light index (-1 for ambient/global evaluation).
    *  @return           Pointer to an evaluated Surface, or nullptr.
    */
   Surface *(*eval)(Viewpoint *, Object *, Texture *,
                    Vec, Vec, Vec, Vec, float, float, int);
   Transform *t_trans; /**< Optional texture-space transform. */
   void *data;         /**< Type-specific texture data (owned unless copy_flag is set). */
   };

/** @brief Data for a two-texture checkerboard pattern. */
struct Checker {
   Texture *text1, *text2;       /**< Alternating textures for the checker cells. */
   int repeat_flag1, repeat_flag2; /**< Per-texture UV repetition flags. */
   };//Checker;

/** @brief Data for a layered (stacked) texture that composites multiple textures. */
struct Layered {
   int copy_flag;   /**< Non-zero when this struct does not own its texture list. */
   Surface surf;    /**< Scratch surface accumulator for layer blending. */
   tstackptr texts; /**< Linked list of textures applied bottom-to-top. */
   };//Layered;

/** @brief Data for an index-mapped texture selected by an expression. */
struct Indexed {
   int copy_flag;             /**< Non-zero when this struct does not own its data. */
   Surface surf;              /**< Scratch surface accumulator. */
   NODE_PTR exper;            /**< Expression that produces the map lookup index. */
   texture_map_entries texts; /**< Texture map list indexed by @c exper. */
   };//Indexed;

/** @brief Data for a summed (blended) texture list driven by per-entry functions. */
struct Summed {
   int copy_flag;            /**< Non-zero when this struct does not own its data. */
   Surface surf;             /**< Scratch surface for blending results. */
   texture_fn_entries texts; /**< Linked list of function-keyed texture entries. */
   };

/** @brief Data for a three-texture hexagonal tiling pattern. */
struct Hexagon {
   Texture *text1, *text2, *text3;           /**< The three alternating textures. */
   int repeat_flag1, repeat_flag2, repeat_flag3; /**< Per-texture UV repetition flags. */
   };//Hexagon;

/** @brief Camera and viewport definition for a render. */
struct t_viewpoint {
   int          view_x0;       /**< Pixel X origin of the render region. */
   int          view_y0;       /**< Pixel Y origin of the render region. */
   int          view_xl;       /**< Width of the render region in pixels. */
   int          view_yl;       /**< Height of the render region in pixels. */
   unsigned int view_xres;     /**< Full horizontal resolution in pixels. */
   unsigned int view_yres;     /**< Full vertical resolution in pixels. */
   unsigned int view_ystart;   /**< First scanline to render (for band rendering). */
   unsigned int view_yend;     /**< Last scanline to render (inclusive). */
   Vec view_from;              /**< Camera position in world space. */
   Vec view_at;                /**< Look-at point in world space. */
   Vec view_up;                /**< Camera up vector. */
   Flt view_angle;             /**< Horizontal field-of-view angle in degrees. */
   Flt view_hither;            /**< Near clip distance. */
   Flt view_yon;               /**< Far clip distance. */
   Flt view_aperture;          /**< Lens aperture diameter for depth-of-field. */
   Flt view_aspect;            /**< Pixel aspect ratio (width/height). */
   Flt view_focaldist;         /**< Focal distance for depth-of-field. */
   Transform *WS;              /**< World-to-screen transform. */
   float **ZBuffer;            /**< Depth buffer (scan-conversion path). */
   rgbo  **SBuffer;            /**< Colour buffer (scan-conversion path). */
   int   *edgey, *edgex;      /**< Edge tables for polygon scan-conversion. */
   };

/** @brief UV subdivision bounds and depth limits used during adaptive tessellation. */
struct UVBounds {
   float u[2], v[2]; /**< Lower [0] and upper [1] bounds on the U and V parameters. */
   int min_depth[2]; /**< Minimum subdivision depth along U and V. */
   int cur_depth[2]; /**< Current subdivision depth along U and V. */
   int max_depth[2]; /**< Maximum allowed subdivision depth along U and V. */
   Object *obj;      /**< Object being tessellated. */
   void *data;       /**< Object-specific subdivision state. */
   };

/** @brief A single patch vertex carrying position, normal, and UV coordinates. */
struct UVVert {
   fVec  pos;  /**< Object-space position. */
   fVec  norm; /**< Surface normal. */
   float u, v; /**< Parametric UV coordinates. */
   };

using VertexPtr = struct Vertex *;
/** @brief A fully transformed vertex carrying coordinates in multiple spaces. */
struct Vertex {
   fVec  S; /**< Screen-space coordinates. */
   fVec  W; /**< World-space coordinates. */
   fVec  P; /**< Object-space coordinates. */
   fVec  N; /**< World-space surface normal. */
   fVec  U; /**< Texture UV/W coordinates. */
   float w; /**< Homogeneous scale factor. */
   };

/** @brief A polygon of up to POLY_NMAX vertices, used in rasterisation. */
struct Poly {
   int n;                      /**< Actual number of sides (vertices). */
   Vertex vertices[POLY_NMAX]; /**< Vertex array (only indices 0..n-1 are valid). */
   };

/** @brief Shape types for individual blob field components. */
enum class BlobType {
    T_SPHERICAL_BLOB,    /**< Spherically symmetric field component. */
    T_CYLINDRICAL_BLOB,  /**< Cylindrically symmetric field component. */
    T_HEMISPHERICAL_BLOB,/**< Hemispherical field component. */
    T_PLANAR_BLOB,       /**< Planar (half-space) field component. */
    T_BOX_BLOB,          /**< Box-shaped field component. */
    T_CONICAL_BLOB,      /**< Conical field component. */
    T_TOROIDAL_BLOB      /**< Toroidal field component. */
};

/** @brief A single field component of a blob implicit surface. */
struct Blob_Element {
   BlobType  type;     /**< Shape of this component (sphere, cylinder, …). */
   Vec       pos;      /**< Centre of a spherical or toroidal component. */
   Vec       dir;      /**< Far end of a conical/cylindrical component, or torus normal. */
   Flt       len;      /**< Cylinder length, or major radius of a toroidal component. */
   Transform *trans;   /**< Transform into canonical component space. */
   Flt       radius2;  /**< Squared radius of influence (density goes to zero at this distance). */
   Flt       coeffs[3];/**< Coefficients of the quartic density function. */
   Flt       *tcoeffs; /**< Temporary coefficient storage during ray/blob intersection. */
   };//Blob_Element;

/** @brief Singly-linked list node wrapping a single blob field component. */
struct blob_list_struct {
   Blob_Element elem; /**< The blob field component. */
   blobstackptr next; /**< Next node in the list, or nullptr. */
   };

/** @brief A variable-length list of 3-D control points (used for Bezier patches). */
typedef struct VList {
   int  count;   /**< Number of points currently stored. */
   Vec *points;  /**< Heap-allocated point array (capacity managed by the caller). */
   } VList;

/** @brief A single glyph outline contour for the font/glyph primitive. */
typedef struct {
   int   count;   /**< Number of control points in this contour. */
   fVec *points;  /**< X,Y coordinates; Z encodes the on/off-curve flag (0 = on-curve). */
   } Contour;

/** @brief Virtual function table for a scene primitive.
 *
 *  Every primitive type sets exactly one @c ObjectProcs and stores a pointer
 *  to it in @c t_object::o_procs.  Generic fallbacks (GenericRender, GenericCopy,
 *  GenericDelete, GenericInitialize) are used when a primitive does not need
 *  a custom implementation.
 */
struct t_objectprocs {
   /** @brief Tessellate the object into polygons and render via scan conversion.
    *  @param eye   Active viewpoint.
    *  @param root  Scene BVH.
    *  @param obj   This object.
    */
   void (*render)(Viewpoint *, BinTree *, Object *);

   /** @brief Evaluate object-space position, normal and UV at parametric (u,v).
    *  @param obj   This object.
    *  @param u     Horizontal parametric coordinate [0,1].
    *  @param v     Vertical parametric coordinate [0,1].
    *  @param vert  Output vertex receiving position, normal, UV, and world position.
    */
   void (*evaluate)(Object *, Flt, Flt, Vertex *);

   /** @brief Perform any one-time post-creation initialisation.
    *  @param obj  Newly constructed object.
    *  @return     Non-zero on success.
    */
   int  (*initialize)(Object *);

   /** @brief Find all ray–object intersections within [mindist, maxdist].
    *  @param Eye      Active viewpoint.
    *  @param obj      This object.
    *  @param ray      Incoming ray.
    *  @param mindist  Near clip distance.
    *  @param maxdist  Far clip distance.
    *  @param hit      Output intersection accumulator.
    *  @return         Non-zero when at least one hit was recorded.
    */
   int  (*intersect)(Viewpoint *Eye, Object *, Ray *, Flt, Flt, Isect *);

   /** @brief Test whether a point is inside the object's volume.
    *  @param obj  This object.
    *  @param P    Point in world space.
    *  @return     Non-zero when the point is inside.
    */
   int  (*inside)(Object *, Vec);

   /** @brief Deep-copy object-specific data from @p src to @p dst.
    *  @param src  Source object.
    *  @param dst  Destination object.
    */
   void (*copy)(Object *, Object *);

   /** @brief Free object-specific data owned by @p obj. */
   void (*del)(Object *);
   };

/** @brief A node in a Constructive Solid Geometry (CSG) operation tree. */
struct csgnode {
   int        type;         /**< CSG operation: T_UNION, T_INTERSECTION, T_INVERSE, T_CLIP, T_MERGE, or T_BASE_OBJECT. */
   csgnodeptr parent;       /**< Parent node, or nullptr for the root. */
   void      *left, *right; /**< Child nodes (csgnodeptr) or leaf objects (Object*). */
   };

/** @brief Axis-aligned bounding box stored as a corner point and extents. */
struct bbox_info {
   fVec lower_left; /**< Minimum corner of the bounding box in world space. */
   fVec lengths;    /**< Extents (width, height, depth) along each axis. */
   };

/** @brief Pre-computed vertex arrays for a polygon-mesh object. */
struct ObjectVertices {
   unsigned  n;  /**< Number of vertices. */
   fVec     *V;  /**< World-space vertex positions (array of @c n). */
   fVec     *N;  /**< Vertex normals (array of @c n). */
   fVec     *U;  /**< Texture UV/W coordinates (array of @c n). */
   };

/** @brief Common header shared by all object types (including TriangleObject).
 *
 *  Kept POD-like so that @c t_object and @c TriangleObject can alias the
 *  same memory layout up to this point.
 */
struct t_base {
   unsigned short  o_type;    /**< Primitive type tag (T_SPHERE, T_BOX, …). */
   bbox_info       o_bnd;     /**< Axis-aligned bounding box. */
   Object         *o_parent;  /**< Owning/parent object, or nullptr. */
   Texture        *o_texture; /**< Texture applied to this object. */
   Transform      *o_trans;   /**< Accumulated world-space transform (lazily allocated). */
};
/** @brief Full scene primitive object (extends t_base with ray-tracing data). */
struct t_object : t_base {
   ObjectProcs                  *o_procs;    /**< Virtual function table for this primitive type. */
   float                         o_dither;   /**< Per-object stochastic dither amount. */
   unsigned short                o_copy;     /**< Non-zero when this is a shared copy (data not owned). */
   unsigned short                o_sflag;    /**< Shading flags (SHADOW_CHECK, REFLECT_CHECK, …). */
   csgnode                      *o_csg_tree; /**< CSG operation tree rooted at this object, or nullptr. */
   std::array<unsigned short,3>  o_uv_steps; /**< Tessellation resolution along U, V, and W. */
   std::array<float,4>           o_uv_bounds;/**< UV parameter bounds: [u_min, u_max, v_min, v_max]. */
   NODE_PTR                      o_displace; /**< Surface displacement expression, or nullptr. */
   void                         *o_data;     /**< Type-specific shape data (owned unless o_copy is set). */
   ObjectVertices               *o_vertices; /**< Pre-computed polygon mesh data, or nullptr. */
   };

/** @brief Lightweight triangle primitive that overlays the t_base header.
 *
 *  Shares the @c t_base memory layout with @c t_object so a @c TriangleObject*
 *  can be cast to @c Object* safely up to the @c t_base fields.
 *  The @c o_parent field is used to find the owning object for texturing.
 */
struct TriangleObject : t_base {
   long o_vert[3];  /**< Indices into the parent object's vertex array. */
   long o_nvert[3]; /**< Indices into the parent object's normal array. */
   };

/** @brief Record of a ray–surface intersection (hit record). */
struct t_isect {
   int      flag;    /**< Non-zero when this record holds a valid hit. */
   Flt      isect_t; /**< Ray parameter (distance) at the intersection point. */
   fVec     U;       /**< Intersection point in object UV/W coordinates. */
   Vec      N;       /**< Surface normal in world space. */
   Vec      W;       /**< Intersection point in world space. */
   Object  *obj;     /**< The primitive that was hit. */
   Texture *texture; /**< Texture to evaluate at this hit (may differ from obj->o_texture). */
   };

/** @brief A single monomial term in a multivariate polynomial expression. */
struct coeff_node {
   Flt   coeff;   /**< Scalar coefficient. */
   float x_power; /**< Exponent of x. */
   float y_power; /**< Exponent of y. */
   float z_power; /**< Exponent of z. */
   };

/** @brief An interval value node storing a [low, high] range. */
struct ivalue_node {
   Flt low;  /**< Lower bound of the interval. */
   Flt high; /**< Upper bound of the interval. */
   };

/** @brief A built-in or user function call node in the expression tree. */
struct func_node {
   int      func_type; /**< Function identifier (defined in polyray.y). */
   LIST_PTR params;    /**< Linked list of argument expression nodes. */
   };

/** @brief A ternary conditional expression node (condition ? exper1 : exper2). */
struct cond_node_struct {
   NODE_PTR condition; /**< Boolean predicate expression. */
   NODE_PTR exper1;    /**< Expression evaluated when condition is true. */
   NODE_PTR exper2;    /**< Expression evaluated when condition is false. */
   };

/** @brief One entry in a colour map, spanning a parameter interval. */
struct color_map_entry {
   float p0, p1;    /**< Start and end parameter values for this interval. */
   fVec  v0, v1;    /**< RGB colour at the start and end of the interval. */
   float t0, t1;    /**< Transmission (alpha) at the start and end of the interval. */
   map_entries next;/**< Next entry in the map list, or nullptr. */
   };

/** @brief One entry in a texture map, blending between two textures over a parameter interval. */
struct texture_map_struct {
   float               p0, p1; /**< Start and end parameter values for this interval. */
   Texture            *t0, *t1;/**< Textures at the start and end of the interval. */
   texture_map_entries next;   /**< Next entry in the map list, or nullptr. */
   };

/** @brief One entry in a function-keyed texture list. */
struct texture_fn_struct {
   NODE_PTR           fn;   /**< Expression that selects/weights this texture. */
   Texture           *t0;   /**< Texture associated with this entry. */
   texture_fn_entries next; /**< Next entry in the list, or nullptr. */
   };

using vvarr = std::array<NODE_PTR, VECTOR_LENGTH>;
 //  #include "exper_node.h"
//std::string, coeff_node, func_node, LIST_PTR, Flt,
//    NODE_PTR, std::array<NODE_PTR, VECTOR_LENGTH>,
//    NuVec, map_entries, Img*, Img**,
//    void*>;
//enum class ExpTypes { TYPE_STR, TYPE_COEFF, TYPE_FUNC, TYPE_LIST_PTR, TYPE_FLT,
//   TYPE_NODEPTR, TYPE_ARRAYNV,
//   TYPE_NUVEC, TYPE_MAP_ENTRIES, TYPE_IMGP, TYPE_IMGPP, VOIDP };
using exdata = std::variant  <
      //std::string str;
      std::string,               /* Text string */
      coeff_node,               /* Floating point/Polynomial entry */
      func_node,                /* Arbitrary function */
      LIST_PTR,                  /* Array of expressions */
      Flt,                       /* Floating point value */
      NODE_PTR,                  /* Pointer to a function argument */
      vvarr, //NODE_PTR    vec[VECTOR_LENGTH]; /* vector of nodes */
      NuVec,                           /* Vector of floats */
      map_entries ,             /* Color map */
      Img*,             /* An image map */
      Img**,           /* An environment map */
      spline_node*             /* Spline points + parameters */
>;

/** @brief A node in the expression parse tree.
 *
 *  Each node carries a type tag, a variant payload, and optional child pointers
 *  for binary/unary operators.  Leaf nodes (literals, identifiers) have
 *  @c left == nullptr and @c right == nullptr.
 */
struct exper_node_struct {
   int      exper_type;  /**< Node type tag; values defined in polyray.y. */
   exdata   exper_data;  /**< Variant payload (string, scalar, vector, function, …). */
   NODE_PTR left, right; /**< Child sub-expressions, or nullptr for leaves. */
   };

/** @brief A singly-linked list node wrapping a single expression tree node. */
struct exper_list_struct {
   NODE_PTR element; /**< The expression node stored at this list position. */
   LIST_PTR next;    /**< Next list node, or nullptr. */
   };


//template based loosely from https://stackoverflow.com/questions/71076473/how-to-implement-a-try-catch-wrapper-in-c
/** @brief Invoke @p f(n1,n2), swallowing @c std::bad_variant_access exceptions.
 *  @tparam F     Callable type.
 *  @tparam NODE  Expression node pointer type.
 *  @param f      Function to call.
 *  @param n1     First expression node argument.
 *  @param n2     Second expression node argument.
 */
template <typename F, typename NODE> void try_catch(F&& f, NODE n1,NODE n2) {
    try { f(n1,n2); }
    //catch (std::exception & err) { /* ... */ }
    catch (std::bad_variant_access&)
     { std::cout << "our variant doesn't hold what its supposed to at this moment...\n"; }
}

/** @brief Surface substitution data passed to texture evaluators at a hit point. */
struct subst_struct {
   Vec U, UT; /**< Natural UV/W coordinates and their rate of change. */
   Vec P, PT; /**< Object-space hit point and its rate of change. */
   Vec W;     /**< World-space hit point. */
   Vec N;     /**< Surface normal in world space. */
   Vec I;     /**< Incident ray direction in world space. */
   };

/** @brief Procedural surface definition — all shading components are expression trees.
 *
 *  Every field is a @c NODE_PTR that is evaluated at run-time for each
 *  intersection, giving the surface access to the hit point, normal,
 *  incident ray, and current light index.  The one exception is @c D_type,
 *  which selects the microfacet distribution function at parse time.
 */
struct t_special_surface {
   int       S_type;       /* What type of surface is this? */
   NODE_PTR  Position_fn;  /* Function used to determine position */
   NODE_PTR  Pos_scale;    /* Scaling on the position value */
   NODE_PTR  Lookup_fn;    /* Function used to search the color map */
   NODE_PTR  Turbulence;   /* Amount of influence of the noise */
   NODE_PTR  Octaves;      /* Number of octaves of noise to use */
   NODE_PTR  Frequency;    /* Frequency of ripple/wave */
   NODE_PTR  Phase;        /* Phase offset for ripple/wave */
   NODE_PTR  Bump_scale;   /* Amount of bump applied to normal */
   NODE_PTR  body_color;   /* Default color to use */
   NODE_PTR  normal;       /* Expression used to modify the normal */
   NODE_PTR  position;     /* Expression used to modify the object coordinate */
   NODE_PTR  Ka_color;     /* Ambient color */
   NODE_PTR  Ka_scale;     /* Ambient multiplier */
   NODE_PTR  Kb_power;     /* Brilliance power */
   NODE_PTR  Kd_color;     /* Diffuse color */
   NODE_PTR  Kd_scale;     /* Diffuse scale */
   NODE_PTR  Ks_color;     /* Specular color */
   NODE_PTR  Ks_scale;     /* Specular scale */
   NODE_PTR  Kr_color;     /* Reflection color */
   NODE_PTR  Kr_scale;     /* Reflection scale */
   NODE_PTR  Kt_color;     /* Transmission color */
   NODE_PTR  Kt_scale;     /* Transmission scale */
   int D_type;             /* What kind of distribution function? */
   NODE_PTR  D_angle;      /* Coefficient for the microfacet distribution */
   NODE_PTR  ior;          /* index of refraction */
   map_entries map;        /* Color map */
   Surface   surf;         /* Holder for results of texture calculations */
   };

/** @brief Fixed-parameter noise-based surface (compile-time constants, no expression trees). */
struct t_noise_surface {
   int   S_type;      /**< Surface sub-type identifier. */
   int   N_modifier;  /**< Normal modification function selector. */
   int   Position_fn; /**< Position function selector. */
   float Pos_scale;   /**< Scale applied to the position before noise lookup. */
   int   Lookup_fn;   /**< Colour-map lookup function selector. */
   float Turbulence;  /**< Turbulence influence weight. */
   int   Octaves;     /**< Number of noise octaves to sum. */
   float Frequency;   /**< Ripple/wave frequency. */
   float Phase;       /**< Ripple/wave phase offset. */
   float Bump_scale;  /**< Bump-map scale applied to the normal. */
   Vec   body_color;  /**< Fallback colour when the colour map is not used. */
   int   Kt_flag;     /**< Non-zero when transparency uses a separate filter. */
   map_entries map;   /**< Colour map, or nullptr. */
   Surface surf;      /**< Scratch surface accumulator for evaluation results. */
   };

/** @brief Singly-linked stack node holding a pointer to an Object (parser object stack). */
struct object_stack_struct {
   Object   *element; /**< The object stored at this stack position. */
   ostackptr next;    /**< Next node in the stack, or nullptr. */
   };

/** @brief A counted list of objects, used for the various primitive groups in BinTree. */
struct object_list_struct {
   ostackptr     list;  /**< Head of the linked object list. */
   unsigned long count; /**< Number of objects in the list. */
   };

/** @brief Singly-linked stack node holding a transform pointer (parser transform stack). */
struct transform_stack_struct {
   Transform  *tx;   /**< The transform stored at this stack position. */
   txstackptr  next; /**< Next node in the stack, or nullptr. */
   };

/** @brief Singly-linked stack node holding a texture pointer (parser texture stack). */
struct texture_stack_struct {
   Texture  *element; /**< The texture stored at this stack position. */
   tstackptr next;    /**< Next node in the stack, or nullptr. */
   };

/** @brief Output image file handle and RLE encoding state. */
struct Pic {
   char *filename;               /**< Path to the output image file. */
   FILE *filep;                  /**< Open file handle for writing. */
   unsigned int x, y;            /**< Image width and height in pixels. */
   int psize;                    /**< Bytes per pixel (e.g. 3 for RGB). */
   int cflag;                    /**< Compression flag: 0 = raw, 1 = RLE. */
   unsigned char buffer[256][4]; /**< RLE run buffer (up to 32 bits/pixel). */
   int CopyCount;                /**< Length of the current non-run span. */
   int RepeatCount;              /**< Length of the current RLE run. */
   int OutputCount;              /**< Total pixels flushed to the file. */
   unsigned int ColumnCount;     /**< Current column within the scanline. */
   unsigned char *line_flags;    /**< Per-scanline completion flags (for resume). */
   char *resume;                 /**< Path to a partially rendered image to resume, or nullptr. */
   FILE *ofilep;                 /**< File handle for the old (resume) image. */
   unsigned long *line_offsets;  /**< Byte offsets of each scanline in the resume image. */
   };

/** @brief Top-level scene acceleration structure (BVH / slab tree root). */
struct t_object_tree {
   Flt     bounds[2][3]; /**< World-space extent: bounds[0] = min corner, bounds[1] = max corner. */
   objlist csgprims;     /**< All CSG compound objects. */
   objlist members;      /**< All primitive objects in the scene. */
   objlist polyprims;    /**< Objects that have been tessellated into polygons. */
   objlist eyeprims;     /**< Objects whose bounding boxes contain the camera origin. */
   objlist lights;       /**< Objects that act as area/mesh light sources. */
   int     MaxDepth;     /**< Maximum allowed depth of the bounding slab tree. */
   int     MaxListLength;/**< Maximum number of primitives allowed in a leaf node. */
   Object *slab_root;    /**< Root of the bounding slab acceleration tree. */
   };

/** @brief Definition of a particle system generator (expression-driven). */
struct Particle {
   NODE_PTR P;             /**< Position expression. */
   NODE_PTR V;             /**< Velocity expression. */
   NODE_PTR A;             /**< Acceleration expression. */
   NODE_PTR Birth;         /**< Birth-time expression. */
   NODE_PTR Death;         /**< Death-time expression. */
   NODE_PTR Count;         /**< Particle count expression. */
   NODE_PTR Avoid;         /**< Avoidance force expression. */
   NODE_PTR Flock;         /**< Flocking force expression. */
   NODE_PTR obj_name;      /**< Name of the shape to instance per particle. */
   char    *particle_name; /**< Identifier for this particle system. */
   Particle *next;         /**< Next particle system in the scene list. */
   };

/** @brief A live particle instance spawned by a Particle generator. */
struct ParticleObject {
   Flt       age;    /**< Current age of the particle (simulation time units). */
   Vec       P;      /**< Current position. */
   Vec       V;      /**< Current velocity. */
   Particle *parent; /**< Generator that spawned this particle. */
   struct ParticleObject *next; /**< Next live particle in the simulation list. */
   };

/** @brief A queued parametric draw command (scene-language @c draw statement). */
struct DrawNode {
   Flt      low, high; /**< Parameter range [low, high] for the draw sweep. */
   int      steps;     /**< Number of steps across the parameter range. */
   NODE_PTR draw_fn;   /**< Expression mapping parameter to a 3-D point. */
   NODE_PTR color_fn;  /**< Expression mapping parameter to an RGB colour. */
   struct DrawNode *next; /**< Next draw command in the list. */
   };

/** @brief An axis-aligned box defined by three pairs of face distances. */
struct Poly_box {
   Flt x0, x1; /**< Left and right X boundaries. */
   Flt y0, y1; /**< Top and bottom Y boundaries. */
   Flt z0, z1; /**< Near and far Z boundaries. */
   };

/** @brief An integer-coordinate screen-space window rectangle. */
struct Window {
   int x0, y0; /**< Minimum X and Y (inclusive). */
   int x1, y1; /**< Maximum X and Y (inclusive). */
   };

//typedef struct t_light Light;
using Light = struct t_light;

constexpr int MAX_CACHE_BLOCKING = 10;

/** @brief A light source in the scene. */
struct t_light {
   int    type;                          /**< Light type (point, directional, spot, …). */
   int    flags;                         /**< Behaviour flags (casting shadows, etc.). */
   void  *data;                          /**< Type-specific light parameters. */
   Light *next;                          /**< Next light in the scene list. */
   Object *cache[MAX_CACHE_BLOCKING];    /**< Shadow cache: recently shadowing objects. */
   };

#endif
