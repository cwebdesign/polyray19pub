/* defs.h

   Define global types, vector operations.

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Photon section is Copyright (C) 1999, Clyde Meli, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#ifndef __DEFSH
#define __DEFSH 	//CM:200206

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <setjmp.h>
#include <string.h>
#include <float.h>
//#if defined( ZTC ) || defined( DOS386 )
#ifndef __UNIX__
//#include <conio.h>
#endif
//#endif


/*CM added 10/mar/2000 */
#define OUT_TARGA (1)
#define OUT_RAWPPM (2)

/*CM end */

/*CM added 28/feb/2001 */
#if defined( WINTARGET )
#include <windows.h>
#define _WINDOWS
#endif
/*CM end */

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <conio.h> //for _kbhit()
#else
#if __APPLE__
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>


//unsigned long _wait=0;


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


#ifndef SEEK_SET
#define SEEK_SET        0       /* seek from start of file      */
#define SEEK_CUR        1       /* relative to current position */
#define SEEK_END        2       /* relative to end of file      */
#endif

#if defined(__APPLE__)
#define MAC
#endif

#ifdef MAC
#define kbhit Button
#endif

/* Define some global constants */
#define MAXLEVEL             (5)
#define MAX_CONDITION_DEPTH (16)
#define VECTOR_LENGTH        (4)    /* Need to allow homogenous coordinates */
#define POLY_NMAX           (32)    /* max sides to a polygon */
#define PQSIZE             (128)

#define MAX_POLYNOMIAL_ORDER 34

#define PLY_HUGE 1.0e6
#define SMALL 1.0e-3
#define EPSILON 1.0e-8

#ifndef M_PI_4
#define M_PI_4 0.785398163397448309616
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif
#ifndef M_PI
#define M_PI 3.1415926535897932384626
#endif
#ifndef TWO_PI
#define TWO_PI 6.283185207179586476925286766560
#endif
#ifndef M_SQRT2
#define M_SQRT2   1.41421356237309504880
#endif
#define TWO_PI_3  2.0943951023931954923084
#define TWO_PI_43 4.1887902047863909846168

#define RAY_TRACING      1
#define SCAN_CONVERSION  2
#define WIRE_FRAME       3
#define HIDDEN_LINE      4
#define GOURAD_SHADE     5
#define RAW_TRIANGLES    6
#define UV_TRIANGLES     7
#define CSG_TRIANGLES    8
#define MESH_CONVERSION  9
#define LAST_RENDER_MODE 9

#define NONE   0
#define SLABS  1
#define BSP    2

#define DEFAULT_SAMPLES 25
#define DEFAULT_THRESHOLD 0.2

#define SHADOW_CHECK      0x0001
#define REFLECT_CHECK     0x0002
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

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)<(y)?(y):(x))
#define SGN(x) ((x)<0?-1:((x)>0?1:0))
#define ABS(x) ((x)<0?(-(x)):(x))
#define equal(x, y) (fabs((x)-(y)) < EPSILON ? 1 : 0)

/* not used yet! not worth it..... better some processor optimizations..
extern unsigned short sqrtablevalues[16]; -- in speedup.s

for integers only
#define fastsqr(x) ((x)<=16 ? (sqrtablevalues[(x)-1]) : (x)*(x))
*/

typedef double Flt;
typedef long double LFlt;
typedef float fVec[3];
typedef Flt Vec[3];
typedef Flt Matrix[4][4];
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <immintrin.h>
typedef struct  {
    union {
        Matrix f;
        __m128 m[4];
        __m256 n[2];
    };
} SuperMatrix;
#endif

typedef Flt Matrix2[2][2];
typedef Matrix2 MMatrix2[2][2];
typedef struct { unsigned char r, g, b, o; } rgbo;

extern Vec ZeroVector; // = { 0.0, 0.0, 0.0 };


#define disable()
#define enable()

//#define disable() __asm__ __volatile__ ("cli");

//#define enable() __asm__ __volatile__ ("sti");

#if defined( __GNUC__ ) && defined ( __DJGPP__ ) /* a DOS optimization for DJGPP */

#define times2(arg1, arg2) \
__asm__ ( \
  "leal (%0,%0,1),%0" \
  : "=r" (arg2) \
  : "0" (arg1) );

#define times3(arg1, arg2) \
__asm__ ( \
  "leal (%0,%0,2),%0" \
  : "=r" (arg2) \
  : "0" (arg1) );

#define times4(arg1, arg2) \
__asm__ ( \
  "leal (%0,%0,3),%0" \
  : "=r" (arg2) \
  : "0" (arg1) );


#define times5(arg1, arg2) \
__asm__ ( \
  "leal (%0,%0,4),%0" \
  : "=r" (arg2) \
  : "0" (arg1) );

#define times9(arg1, arg2) \
__asm__ ( \
   "leal (%0,%0,8),%0" \
   : "=r" (arg2) \
   : "0" (arg1) );

#else

  #define times2(arg1, arg2) (arg2)= (long)(arg1) * 2L;
  #define times3(arg1, arg2) (arg2)= (long)(arg1) * 3L;
  #define times4(arg1, arg2) (arg2)= (long)(arg1) * 4L;
  #define times5(arg1, arg2) (arg2)= (long)(arg1) * 5L;
  #define times6(arg1, arg2) (arg2)= (long)(arg1) * 6L;
  #define times7(arg1, arg2) (arg2)= (long)(arg1) * 7L;
  #define times8(arg1, arg2) (arg2)= (long)(arg1) * 8L;
  #define times9(arg1, arg2) (arg2)= (long)(arg1) * 9L;


#endif

#define RECIPROCAL(a) (1.0/(a))

//#define MUL(a,b) ((a)*(b))
//#define HALF(a) ((a)*0.5))
//#define TWICE(a) ((a)*2.0)



#define degtorad(x)     (((Flt)(x))*M_PI/180.0)
#define radtodeg(x)     (((Flt)(x))*180.0/M_PI)

#define MakeVector(x, y, z, v) {(v)[0]=(x);(v)[1]=(y);(v)[2]=(z);}

#define MakeVectorZero(v) {(v)[0]=(v)[1]=(v)[2]=0.0;}

#define MakeVectorZero1(v) {(v)[0]=(v)[1]=(v)[2]=0.0;}

//#define MakeVectorZero(v) {memcpy((v),ZeroVector,sizeof(ZeroVector));}

//#define MakeVectorZero1(v) {memcpy((v),ZeroVector,sizeof(ZeroVector));}



#define VecScale(S,a)   {(a)[0] *= S ; (a)[1] *= S ; (a)[2] *= S;}
#define VecNegate(a)  {(a)[0]=0-(a)[0];\
                       (a)[1]=0-(a)[1];\
                       (a)[2]=0-(a)[2];}
#define VecClose(a, b)  ( (fabs((a)[0] - (b)[0]) < EPSILON) &&\
                          (fabs((a)[1] - (b)[1]) < EPSILON) &&\
                          (fabs((a)[2] - (b)[2]) < EPSILON) ? 1 : 0)
#define VecDot(a,b)   ((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a)     (sqrt(VecDot(a,a)))
#define VecCopy(a,b)    {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
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
typedef struct t_object Object;
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

/* Basic primitive types: */
#define T_NULL              (0)
#define T_BLOB              (1)
#define T_BOX               (2)
#define T_BEZIER            (3)
#define T_CONE              (4)
#define T_CSG               (5)
#define T_CYLINDER          (6)
#define T_CYL_HEIGHT_FIELD  (7)
#define T_DISC              (8)
#define T_FUNCTION          (9)
#define T_GLYPH            (10)
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

#define FIRST_OBJECT_TYPE (1)
#define LAST_OBJECT_TYPE  (29)

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

struct t_ray {
   Vec P;   /* Start location of the ray */
   Vec D;   /* Direction of the ray */
   };

struct Transformation_Struct {
   Matrix matrix;
   Matrix inverse;
   };

typedef struct Img {
   char *filename;       /* Name of the file this the image came from */
   int copy;             /* Set to 1 if this is a copy of a image */
   int psize;            /* # of bytes per pixel */
   int cflag;            /* 1 if this is a color mapped image */
   unsigned width;       /* # of pixels wide */
   unsigned length;      /* # of pixels high */
   unsigned orien;       /* 0 = top to bottom, 1 = bottom to top */
   unsigned ftype;       /* Targa type */
   unsigned cmlen;       /* # of entries in the color map */
   unsigned cmsiz;       /* # of bytes per pixel in each color map entry */
   unsigned char *cmap;  /* The color map for this image */
   unsigned char **image;/* The image */
   } Img;

struct t_surface {
   fVec Ka_color;   /* Ambient color */
   float Ka_scale;  /* Ambient multiplier */
   float Kb_power;  /* Brilliance modifier to diffuse */
   fVec Kd_color;   /* Diffuse color */
   float Kd_scale;  /* Diffuse scale */
   fVec Ks_color;   /* Specular color */
   float Ks_scale;  /* Specular scale */
   fVec Kr_color;   /* Reflection color */
   float Kr_scale;  /* Reflection scale */
   fVec Kt_color;   /* Transmission color */
   float Kt_scale;  /* Transmission scale */
   float (*D)(Vec, Vec, Vec, Flt); /* Microfacet distribution functions */
   float D_coeff;   /* Coefficient for the microfacet distribution */
   float ior;       /* index of refraction */
   };

struct t_texture {
   /* Texture type */
   unsigned short type;

   /* Is this a copy? */
   short copy_flag;

   /* Delete this texture */
   void (*del)(Texture *);

   /* Evaluate a texture function for a given point */
   Surface *(*eval)(Viewpoint *, Object *, Texture *,
                    Vec, Vec, Vec, Vec, float, float, int);

   /* Transforms specific to this texture */
   Transform *t_trans;

   /* Texture specific data */
   void *data;
   };

typedef struct checker_struct {
   Texture *text1, *text2;
   int repeat_flag1, repeat_flag2;
   } Checker;

typedef struct layered_struct {
   int copy_flag;    /* Is this a copy of another layered texture? */
   Surface surf;     /* Holder for results of texture calculations */
   tstackptr texts;  /* List of textures */
   } Layered;

typedef struct indexed_struct {
   int copy_flag;             /* Is this a copy of another layered texture? */
   Surface surf;              /* Holder for results of texture calculations */
   NODE_PTR exper;            /* Lookup function for the texture map */
   texture_map_entries texts; /* List of textures */
   } Indexed;

typedef struct summed_struct {
   int copy_flag;
   Surface surf;
   texture_fn_entries texts;
   } Summed;

typedef struct hexagon_struct {
   Texture *text1, *text2, *text3;
   int repeat_flag1, repeat_flag2, repeat_flag3;
   } Hexagon;

struct t_viewpoint {
/* CM modify 10/mar/2000. Changing view_xres/yres to unsigned */
/* CM modify 28/feb/2002. Changing view_ystart/end to unsigned */
   int view_x0;
   int view_y0;
   int view_xl;
   int view_yl;
   unsigned int view_xres;
   unsigned int view_yres;
   unsigned int view_ystart;
   unsigned int view_yend;
   Vec view_from;
   Vec view_at;
   Vec view_up;
   Flt view_angle;
   Flt view_hither;
   Flt view_yon;
   Flt view_aperture;
   Flt view_aspect;
   Flt view_focaldist;
   Transform *WS;
   float **ZBuffer;
   rgbo  **SBuffer;
   int   *edgey, *edgex;
   };

typedef struct {
   float u[2], v[2]; /* Lower and upper bounds on u and v */
   int min_depth[2]; /* Minimum amount we will subdivide */
   int cur_depth[2]; /* Current depth of subdivision */
   int max_depth[2]; /* Absolute farthest we will subdivide */
   Object *obj;      /* Object that we are currently chopping up */
   void *data;       /* Object specific data to help subdivision */
   } UVBounds;

/* Structure to hold patch vertex information */
typedef struct uvvert {
   fVec pos, norm;
   float u, v;
   } UVVert;

typedef struct Vertex_Struct *VertexPtr;
typedef struct Vertex_Struct{
   fVec  S; /* screen space coordinates */
   fVec  W; /* world space coordinates */
   fVec  P; /* object space coordinates */
   fVec  N; /* world space normal */
   fVec  U; /* u/v/w coordinates */
   float w; /* Homogenous scale factor */
   } Vertex;

typedef struct {
   int n;                      /* number of sides */
   Vertex vertices[POLY_NMAX]; /* vertices */
   } Poly;

#define T_SPHERICAL_BLOB      0
#define T_CYLINDRICAL_BLOB    1
#define T_HEMISPHERICAL_BLOB  2
#define T_PLANAR_BLOB         3
#define T_BOX_BLOB            4
#define T_CONICAL_BLOB        5
#define T_TOROIDAL_BLOB       6

/* Blob types */
typedef struct {
   short int type;   /* One of the blob component types listed above */
   Vec pos;          /* Center of a spherical/toroidal component     */
   Vec dir;          /* Far end of conical/cylindrical component     */
   Flt len;          /* Length of a cylindrical component, major
                        radius of a toroidal component               */
   Transform *trans; /* Transform into canonical component space     */
   Flt radius2;      /* Distance at which density goes to zero       */
   Flt coeffs[3];    /* Coefficients of the quartic density func     */
   Flt *tcoeffs;     /* Temp storage while doing ray/blob hits       */
   } Blob_Element;

struct blob_list_struct {
   Blob_Element elem;
   blobstackptr next;
   };

typedef struct Vector_List {
   int count;
   Vec *points;
   } VList;

/* Glyph contour data type */
typedef struct {
   int count;    /* Number of points in the contour */
   fVec *points; /* X,Y coordinates with Z = On/off curve flag */
   } Contour;

/* Define the structure of a primitive object */
struct t_objectprocs {
  /* Routines to assist in rendering an object as polygons */
     /* Default routine if adaptive ones are NULL */
     void (*render)(Viewpoint *, BinTree *, Object *);

     /* Determine vertex values at a particular u-v coordinate */
     void (*evaluate)(Object *, float, float, Vertex *);

  /* Initialize internal data after creation and updates
     to the various fields. */
  int  (*initialize)(Object *);

  /* Return ray-surface intersections */
  int  (*intersect)(Viewpoint *Eye, Object *, Ray *, Flt, Flt, Isect *);

  /* Determine if a point is 'inside' */
  int  (*inside)(Object *, Vec);

  /* Copy object specific information */
  void (*copy)(Object *, Object *);

  /* Delete object specific information */
  void (*del)(Object *);
  };

typedef struct csgnode {
   int type;
   csgnodeptr parent;
   void *left, *right;
   } csgnode;

typedef struct t_bounding_box_info {
   fVec lower_left, lengths;
   } bbox_info;

typedef struct t_points_polygon {
   unsigned n;
   fVec *V; /* World coordinates */
   fVec *N; /* Normals */
   fVec *U; /* u/v/w coordinates */
   } ObjectVertices;

struct t_object {
   unsigned short  o_type;         /* Holds the 'type' of the object */
   bbox_info       o_bnd;          /* Bounding box limits */
   Object         *o_parent;       /* Parent object */
   Texture        *o_texture;      /* Texture characteristics */
   ObjectProcs    *o_procs;        /* Routines specific to this object type */
   float           o_dither;       /* Random dithering for this object */
   Transform      *o_trans;        /* Transforms operating on this object */
   unsigned short  o_copy;         /* Is this just a copy of another object? */
   unsigned short  o_sflag;        /* Which shading options to use */
   csgnode        *o_csg_tree;     /* Pointer to CSG affecting this object */
   unsigned short  o_uv_steps[3];  /* Resolution along u/v/w axes */
   float           o_uv_bounds[4]; /* u,v coordinate bounds */
   NODE_PTR        o_displace;     /* Surface displacement function */
   void           *o_data;         /* Object specific data */
   ObjectVertices *o_vertices;     /* Polygon mesh information */
   };

/* Object look-alike for storing polygons.  The parent object is used
   for performing any texturing */
typedef struct t_triangle_object {
   unsigned short  o_type;     /* Holds the 'type' of the object */
   bbox_info       o_bnd;      /* Bounding box limits */
   Object         *o_parent;   /* Parent object */
   Texture        *o_texture;  /* Texture characteristics */
   long            o_vert[3];  /* Vertices of the triangle */
   long            o_nvert[3]; /* Vertex normals */
   } TriangleObject;

struct t_isect {
   int flag;         /* Is this a valid hit? */
   Flt isect_t;      /* Distance to the point of intersection */
   fVec U;           /* Point of intersection in u/v/w coordinates */
   Vec N;            /* Normal to the surface in world coordinates */
   Vec W;            /* Point of intersection in world coordinates */
   Object *obj;      /* The primitive object that was hit */
   Texture *texture; /* Texture for this object */
   };

typedef struct coeff_node_struct {
   float coeff;
   float x_power;
   float y_power;
   float z_power;
   } coeff_node;

typedef struct ivalue_node_struct {
   Flt low, high;
   } ivalue_node;

typedef struct func_node_struct {
   int func_type;
   LIST_PTR params;
   } func_node;

typedef struct cond_node_struct {
   NODE_PTR condition, exper1, exper2;
   } cond_node;

struct color_map_entry {
   float p0, p1; /* Start and end values for this entry */
   fVec v0, v1; /* Start and end colors */
   float t0, t1; /* Start and end transmission values */
   map_entries next;
   };

struct texture_map_struct {
   float p0, p1;        /* Start and end values for this entry */
   Texture *t0, *t1; /* Start and end textures */
   texture_map_entries next;
   };

struct texture_fn_struct {
   NODE_PTR fn; /* Evaluation function */
   Texture *t0; /* Textures */
   texture_fn_entries next;
   };

struct exper_node_struct {
   int exper_type;      /* Type of node, values defined in 'polyray.y' */
   union {
      char        *str;               /* Text string */
      coeff_node  coeff;              /* Floating point/Polynomial entry */
      func_node   funct;              /* Arbitrary function */
      LIST_PTR    array;              /* Array of expressions */
      Flt         value;              /* Floating point value */
      NODE_PTR    param;              /* Pointer to a function argument */
      NODE_PTR    vec[VECTOR_LENGTH]; /* vector of nodes */
      Vec         v;                  /* Vector of floats */
      map_entries cmap;               /* Color map */
      Img         *image;             /* An image map */
      Img         **images;           /* An environment map */
      void        *data;              /* Spline points + parameters */
      } exper_data;
   NODE_PTR left, right;
   };

struct exper_list_struct {
   NODE_PTR element;
   LIST_PTR next;
   };

struct subst_struct {
   Vec U, UT;   /* Natural u/v/w coordinates for the primitive */
   Vec P, PT;   /* Object coordinates and rates of change */
   Vec W, N, I; /* World, Normal, Incident vectors */
   };

/* The following structure is used for the "special" texture.
   the actual values for all of the surface components are
   calculated at run-time for each object using the following
   information: point of intersection, normal to the surface,
   incident vector of the current ray, and the index of the
   current light being examined (-1 if not a light modifiable
   component, such as ambient lighting).  The only component that
   cannot be altered at run-time is the microfacet distribution
   function. */
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

struct t_noise_surface {
   int S_type;      /* What type of surface is this? */
   int N_modifier;  /* Function used to modify normal */
   int Position_fn; /* Function used to determine position */
   float Pos_scale;   /* Scaling on the position value */
   int Lookup_fn;   /* Function used to search the color map */
   float Turbulence;  /* Amount of influence of the noise */
   int Octaves;     /* Number of octaves of noise to use */
   float Frequency;   /* Frequency of ripple/wave */
   float Phase;       /* Phase offset for ripple/wave */
   float Bump_scale;  /* Amount of bump applied to normal */
   Vec body_color;  /* Default color if color map fails */
   int Kt_flag;     /* Possibly different filter for transparency */
   map_entries map; /* Color map */
   Surface surf;    /* Holder for results of texture calculations */
   };

struct object_stack_struct {
   Object *element;
   ostackptr next;
   };

struct object_list_struct {
   ostackptr list;
   unsigned long count;
   };

struct transform_stack_struct {
   Transform  *tx;
   txstackptr next;
   };

struct texture_stack_struct {
   Texture *element;
   tstackptr next;
   };

/* Data structure for the output image */
typedef struct Pic {
   char *filename;  /* Name of the output file */
   FILE *filep;     /* File handle for the output file */
   /* CM change to UNSIGNED from SIGNED int 10/mar/2000 */
   unsigned /*CM end */
   int x, y;        /* Width and height */
   int psize;       /* # of bytes per pixel */
   int cflag;       /* 0 = uncompressed, 1 = RLE compressed */
   unsigned char buffer[256][4]; /* Up to 32 bits/pixel */
   int CopyCount;
   int RepeatCount;
   int OutputCount;
   /* CM change to UNSIGNED from SIGNED int 19/jan/2002 to avoid warnings in pic.c */
   unsigned /*CM end */
   int ColumnCount;
   unsigned char *line_flags;   /* Which lines have been rendered? */
   char *resume;                /* Name of old image file */
   FILE *ofilep;                /* File handle for the old image  */
   unsigned long *line_offsets; /* Line offsets in old image file */
   } Pic;

struct t_object_tree {
   Flt bounds[2][3];  /* Extent of the entire bin tree */
   objlist csgprims;  /* List of CSG objects */
   objlist members;   /* List of all of the primitives */
   objlist polyprims; /* List of objects split into polygons */
   objlist eyeprims;  /* List of all bounding boxes that contain the eye */
   objlist lights;    /* List of all lights defined as objects */
   int MaxDepth;      /* Max allowed depth of the tree */
   int MaxListLength; /* Max primitive allowed in a leaf node */
   Object *slab_root; /* Root of the bounding slab tree */
   };

/* Type for particle generators */
typedef struct t_particledata {
   NODE_PTR P, V, A;
   NODE_PTR Birth, Death, Count;
   NODE_PTR Avoid, Flock, obj_name;
   char *particle_name;
   struct t_particledata *next;
   } Particle;

/* Type for an instantiated object */
typedef struct t_particleobj {
   Flt age;
   Vec P, V;
   Particle *parent;
   struct t_particleobj *next;
   } ParticleObject;

typedef struct t_draw_node_struct {
   Flt low, high;
   int steps;
   NODE_PTR draw_fn;
   NODE_PTR color_fn;
   struct t_draw_node_struct *next;
   } DrawNode;

typedef struct {
   Flt x0, x1;  /* left and right */
   Flt y0, y1;  /* top and bottom */
   Flt z0, z1;  /* near and far */
   } Poly_box;

typedef struct {
   int x0, y0;  /* xmin and ymin */
   int x1, y1;  /* xmax and ymax (inclusive) */
   } Window;

typedef struct t_light Light;

#define MAX_CACHE_BLOCKING 10

struct t_light {
   int type;
   int flags;   /* What to do with this light */
   void *data;
   Light *next;
   Object *cache[MAX_CACHE_BLOCKING]; /* Shadow cache */
   };

#endif
