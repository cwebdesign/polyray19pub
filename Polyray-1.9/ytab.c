/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "polyray.y"

/*

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/
#include "defs.h"
#include "io.h"
#include "memory.h"
#include "psupport.h"
#include "symtab.h"
#include "texture.h"
#include "particle.h"
#include "light.h"
#include "parse.h"
#include "vector.h"
#include "eval.h"
#include "builder.h"

/* Include files for the various shapes */
#include "bezier.h"
#include "blob.h"
#include "box.h"
#include "cone.h"
#include "csg.h"
#include "cylinder.h"
#include "disc.h"
#include "function.h"
#include "glyph.h"
#include "gridded.h"
#include "height.h"
#include "hypertex.h"
#include "parabola.h"
#include "parametr.h"
#include "poly.h"
#include "polynom.h"
#include "raw.h"
#include "revolve.h"
#include "sphere.h"
#include "superq.h"
#include "sweep.h"
#include "torus.h"
#include "tri.h"

#define ACTION(x) { if (check_condition()) { x } }
#define alloca malloc
#define yyerror error

struct def_tok_struct {
   int type;
   void *data;
   } temp_def;
static Contour *cl, *contours;
static int gcount;

#line 137 "ytab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "ytab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ACCELERATION = 3,               /* ACCELERATION  */
  YYSYMBOL_ACOS = 4,                       /* ACOS  */
  YYSYMBOL_AMBIENT = 5,                    /* AMBIENT  */
  YYSYMBOL_AND_EXPER = 6,                  /* AND_EXPER  */
  YYSYMBOL_ANGLE = 7,                      /* ANGLE  */
  YYSYMBOL_ANTIALIAS = 8,                  /* ANTIALIAS  */
  YYSYMBOL_ANTIALIAS_THRESHOLD = 9,        /* ANTIALIAS_THRESHOLD  */
  YYSYMBOL_APERTURE = 10,                  /* APERTURE  */
  YYSYMBOL_ARRAY = 11,                     /* ARRAY  */
  YYSYMBOL_ASSIGNMENT = 12,                /* ASSIGNMENT  */
  YYSYMBOL_ASIN = 13,                      /* ASIN  */
  YYSYMBOL_ASPECT = 14,                    /* ASPECT  */
  YYSYMBOL_AT = 15,                        /* AT  */
  YYSYMBOL_ATAN = 16,                      /* ATAN  */
  YYSYMBOL_ATAN_TWO = 17,                  /* ATAN_TWO  */
  YYSYMBOL_AVOID = 18,                     /* AVOID  */
  YYSYMBOL_BACKGROUND = 19,                /* BACKGROUND  */
  YYSYMBOL_BEZIER = 20,                    /* BEZIER  */
  YYSYMBOL_BIAS = 21,                      /* BIAS  */
  YYSYMBOL_BIRTH = 22,                     /* BIRTH  */
  YYSYMBOL_BLINN = 23,                     /* BLINN  */
  YYSYMBOL_BLOB = 24,                      /* BLOB  */
  YYSYMBOL_BOUNDING_BOX = 25,              /* BOUNDING_BOX  */
  YYSYMBOL_BOX = 26,                       /* BOX  */
  YYSYMBOL_BRILLIANCE = 27,                /* BRILLIANCE  */
  YYSYMBOL_BUMP_SCALE = 28,                /* BUMP_SCALE  */
  YYSYMBOL_CEIL = 29,                      /* CEIL  */
  YYSYMBOL_CHECKER = 30,                   /* CHECKER  */
  YYSYMBOL_CHEIGHT_FIELD = 31,             /* CHEIGHT_FIELD  */
  YYSYMBOL_CHEIGHT_FN = 32,                /* CHEIGHT_FN  */
  YYSYMBOL_COLOR = 33,                     /* COLOR  */
  YYSYMBOL_COLOR_MAP = 34,                 /* COLOR_MAP  */
  YYSYMBOL_COLOR_WHEEL = 35,               /* COLOR_WHEEL  */
  YYSYMBOL_CONCAT = 36,                    /* CONCAT  */
  YYSYMBOL_CONCAVE = 37,                   /* CONCAVE  */
  YYSYMBOL_CONDITIONAL_EXPER = 38,         /* CONDITIONAL_EXPER  */
  YYSYMBOL_CONE = 39,                      /* CONE  */
  YYSYMBOL_CONTOUR = 40,                   /* CONTOUR  */
  YYSYMBOL_COOK = 41,                      /* COOK  */
  YYSYMBOL_COS = 42,                       /* COS  */
  YYSYMBOL_COSH = 43,                      /* COSH  */
  YYSYMBOL_COUNT = 44,                     /* COUNT  */
  YYSYMBOL_CSG = 45,                       /* CSG  */
  YYSYMBOL_CYLINDER = 46,                  /* CYLINDER  */
  YYSYMBOL_CYLINDRICAL_BUMPMAP = 47,       /* CYLINDRICAL_BUMPMAP  */
  YYSYMBOL_CYLINDRICAL_IMAGEMAP = 48,      /* CYLINDRICAL_IMAGEMAP  */
  YYSYMBOL_CYLINDRICAL_INDEXED = 49,       /* CYLINDRICAL_INDEXED  */
  YYSYMBOL_DEATH = 50,                     /* DEATH  */
  YYSYMBOL_DEFINE = 51,                    /* DEFINE  */
  YYSYMBOL_DEGREES = 52,                   /* DEGREES  */
  YYSYMBOL_DEPTH = 53,                     /* DEPTH  */
  YYSYMBOL_DEPTHMAPPED_LIGHT = 54,         /* DEPTHMAPPED_LIGHT  */
  YYSYMBOL_DIFFUSE = 55,                   /* DIFFUSE  */
  YYSYMBOL_DIRECTIONAL_LIGHT = 56,         /* DIRECTIONAL_LIGHT  */
  YYSYMBOL_DISC = 57,                      /* DISC  */
  YYSYMBOL_DISPLACE = 58,                  /* DISPLACE  */
  YYSYMBOL_DITHER = 59,                    /* DITHER  */
  YYSYMBOL_DIV_EXPER = 60,                 /* DIV_EXPER  */
  YYSYMBOL_DNOISE = 61,                    /* DNOISE  */
  YYSYMBOL_DOT_EXPER = 62,                 /* DOT_EXPER  */
  YYSYMBOL_DRAW = 63,                      /* DRAW  */
  YYSYMBOL_ELSE = 64,                      /* ELSE  */
  YYSYMBOL_END_FRAME = 65,                 /* END_FRAME  */
  YYSYMBOL_ENVIRONMENT = 66,               /* ENVIRONMENT  */
  YYSYMBOL_ENVIRONMENT_MAP = 67,           /* ENVIRONMENT_MAP  */
  YYSYMBOL_EQUAL_EXPER = 68,               /* EQUAL_EXPER  */
  YYSYMBOL_EXP = 69,                       /* EXP  */
  YYSYMBOL_EXPRESSION_SYM = 70,            /* EXPRESSION_SYM  */
  YYSYMBOL_FABS = 71,                      /* FABS  */
  YYSYMBOL_FBM = 72,                       /* FBM  */
  YYSYMBOL_FERRARI = 73,                   /* FERRARI  */
  YYSYMBOL_FILE_FLUSH = 74,                /* FILE_FLUSH  */
  YYSYMBOL_FLARE = 75,                     /* FLARE  */
  YYSYMBOL_FLOCK = 76,                     /* FLOCK  */
  YYSYMBOL_FLOOR = 77,                     /* FLOOR  */
  YYSYMBOL_FNOISE = 78,                    /* FNOISE  */
  YYSYMBOL_FOCAL_DISTANCE = 79,            /* FOCAL_DISTANCE  */
  YYSYMBOL_FMOD = 80,                      /* FMOD  */
  YYSYMBOL_FRAME = 81,                     /* FRAME  */
  YYSYMBOL_FRAME_TIME = 82,                /* FRAME_TIME  */
  YYSYMBOL_FREQUENCY = 83,                 /* FREQUENCY  */
  YYSYMBOL_FROM = 84,                      /* FROM  */
  YYSYMBOL_FUNCTION = 85,                  /* FUNCTION  */
  YYSYMBOL_GAIN = 86,                      /* GAIN  */
  YYSYMBOL_GAUSSIAN = 87,                  /* GAUSSIAN  */
  YYSYMBOL_GLYPH = 88,                     /* GLYPH  */
  YYSYMBOL_GREATER_EXPER = 89,             /* GREATER_EXPER  */
  YYSYMBOL_GRIDDED = 90,                   /* GRIDDED  */
  YYSYMBOL_GTEQ_EXPER = 91,                /* GTEQ_EXPER  */
  YYSYMBOL_HAZE = 92,                      /* HAZE  */
  YYSYMBOL_HEIGHT_FIELD = 93,              /* HEIGHT_FIELD  */
  YYSYMBOL_HEIGHT_FN = 94,                 /* HEIGHT_FN  */
  YYSYMBOL_HEIGHT_MAP = 95,                /* HEIGHT_MAP  */
  YYSYMBOL_HEXAGON = 96,                   /* HEXAGON  */
  YYSYMBOL_HITHER = 97,                    /* HITHER  */
  YYSYMBOL_HYPERTEXTURE = 98,              /* HYPERTEXTURE  */
  YYSYMBOL_I_EXPER = 99,                   /* I_EXPER  */
  YYSYMBOL_IF = 100,                       /* IF  */
  YYSYMBOL_IMAGE = 101,                    /* IMAGE  */
  YYSYMBOL_IMAGE_FORMAT = 102,             /* IMAGE_FORMAT  */
  YYSYMBOL_IMAGE_WINDOW = 103,             /* IMAGE_WINDOW  */
  YYSYMBOL_INCLUDE = 104,                  /* INCLUDE  */
  YYSYMBOL_INDEXED = 105,                  /* INDEXED  */
  YYSYMBOL_INDEXED_MAP = 106,              /* INDEXED_MAP  */
  YYSYMBOL_LATHE = 107,                    /* LATHE  */
  YYSYMBOL_LAYERED = 108,                  /* LAYERED  */
  YYSYMBOL_LEGENDRE = 109,                 /* LEGENDRE  */
  YYSYMBOL_LENSES = 110,                   /* LENSES  */
  YYSYMBOL_LESS_EXPER = 111,               /* LESS_EXPER  */
  YYSYMBOL_LIGHT = 112,                    /* LIGHT  */
  YYSYMBOL_LN = 113,                       /* LN  */
  YYSYMBOL_LOG = 114,                      /* LOG  */
  YYSYMBOL_LOOKUP_FUNCTION = 115,          /* LOOKUP_FUNCTION  */
  YYSYMBOL_LTEQ_EXPER = 116,               /* LTEQ_EXPER  */
  YYSYMBOL_MAXT = 117,                     /* MAXT  */
  YYSYMBOL_MAX_SAMPLES = 118,              /* MAX_SAMPLES  */
  YYSYMBOL_MAX_TRACE_DEPTH = 119,          /* MAX_TRACE_DEPTH  */
  YYSYMBOL_MICROFACET = 120,               /* MICROFACET  */
  YYSYMBOL_MINT = 121,                     /* MINT  */
  YYSYMBOL_MINUS_EXPER = 122,              /* MINUS_EXPER  */
  YYSYMBOL_N_EXPER = 123,                  /* N_EXPER  */
  YYSYMBOL_NO_SHADOW = 124,                /* NO_SHADOW  */
  YYSYMBOL_NOEVAL = 125,                   /* NOEVAL  */
  YYSYMBOL_NOISE = 126,                    /* NOISE  */
  YYSYMBOL_NORMAL = 127,                   /* NORMAL  */
  YYSYMBOL_NOT_EXPER = 128,                /* NOT_EXPER  */
  YYSYMBOL_NUM = 129,                      /* NUM  */
  YYSYMBOL_NURB = 130,                     /* NURB  */
  YYSYMBOL_OBJECT = 131,                   /* OBJECT  */
  YYSYMBOL_OBJECT_SYM = 132,               /* OBJECT_SYM  */
  YYSYMBOL_OCTAVES = 133,                  /* OCTAVES  */
  YYSYMBOL_OR_EXPER = 134,                 /* OR_EXPER  */
  YYSYMBOL_OPACITY = 135,                  /* OPACITY  */
  YYSYMBOL_OUTFILE = 136,                  /* OUTFILE  */
  YYSYMBOL_P_EXPER = 137,                  /* P_EXPER  */
  YYSYMBOL_PARABOLA = 138,                 /* PARABOLA  */
  YYSYMBOL_PARAMETRIC = 139,               /* PARAMETRIC  */
  YYSYMBOL_PARTICLE = 140,                 /* PARTICLE  */
  YYSYMBOL_PARTICLE_SYM = 141,             /* PARTICLE_SYM  */
  YYSYMBOL_PATCH = 142,                    /* PATCH  */
  YYSYMBOL_PHASE = 143,                    /* PHASE  */
  YYSYMBOL_PHONG = 144,                    /* PHONG  */
  YYSYMBOL_PIXEL_ENCODING = 145,           /* PIXEL_ENCODING  */
  YYSYMBOL_PIXELSIZE = 146,                /* PIXELSIZE  */
  YYSYMBOL_PLANAR_BUMPMAP = 147,           /* PLANAR_BUMPMAP  */
  YYSYMBOL_PLANAR_IMAGEMAP = 148,          /* PLANAR_IMAGEMAP  */
  YYSYMBOL_PLANE = 149,                    /* PLANE  */
  YYSYMBOL_PLUS_EXPER = 150,               /* PLUS_EXPER  */
  YYSYMBOL_POINT = 151,                    /* POINT  */
  YYSYMBOL_POLYGON = 152,                  /* POLYGON  */
  YYSYMBOL_POLYNOMIAL = 153,               /* POLYNOMIAL  */
  YYSYMBOL_POSITION = 154,                 /* POSITION  */
  YYSYMBOL_POSITION_FUNCTION = 155,        /* POSITION_FUNCTION  */
  YYSYMBOL_POSITION_SCALE = 156,           /* POSITION_SCALE  */
  YYSYMBOL_POWER_EXPER = 157,              /* POWER_EXPER  */
  YYSYMBOL_RADIANS = 158,                  /* RADIANS  */
  YYSYMBOL_RAMP = 159,                     /* RAMP  */
  YYSYMBOL_RANDOM = 160,                   /* RANDOM  */
  YYSYMBOL_RAW = 161,                      /* RAW  */
  YYSYMBOL_REFLECT = 162,                  /* REFLECT  */
  YYSYMBOL_REFLECTION = 163,               /* REFLECTION  */
  YYSYMBOL_REITZ = 164,                    /* REITZ  */
  YYSYMBOL_RESOLUTION = 165,               /* RESOLUTION  */
  YYSYMBOL_RIPPLE = 166,                   /* RIPPLE  */
  YYSYMBOL_ROOT_SOLVER = 167,              /* ROOT_SOLVER  */
  YYSYMBOL_ROTATE = 168,                   /* ROTATE  */
  YYSYMBOL_SAWTOOTH = 169,                 /* SAWTOOTH  */
  YYSYMBOL_SCALE = 170,                    /* SCALE  */
  YYSYMBOL_SEED = 171,                     /* SEED  */
  YYSYMBOL_SHADING_FLAGS = 172,            /* SHADING_FLAGS  */
  YYSYMBOL_SHEAR = 173,                    /* SHEAR  */
  YYSYMBOL_SIN = 174,                      /* SIN  */
  YYSYMBOL_SINH = 175,                     /* SINH  */
  YYSYMBOL_SIZE = 176,                     /* SIZE  */
  YYSYMBOL_SMOOTH_HEIGHT_FIELD = 177,      /* SMOOTH_HEIGHT_FIELD  */
  YYSYMBOL_SMOOTH_HEIGHT_FN = 178,         /* SMOOTH_HEIGHT_FN  */
  YYSYMBOL_SMOOTH_CHEIGHT_FIELD = 179,     /* SMOOTH_CHEIGHT_FIELD  */
  YYSYMBOL_SMOOTH_CHEIGHT_FN = 180,        /* SMOOTH_CHEIGHT_FN  */
  YYSYMBOL_SMOOTH_SHEIGHT_FIELD = 181,     /* SMOOTH_SHEIGHT_FIELD  */
  YYSYMBOL_SMOOTH_SHEIGHT_FN = 182,        /* SMOOTH_SHEIGHT_FN  */
  YYSYMBOL_SPACING = 183,                  /* SPACING  */
  YYSYMBOL_SPECIAL = 184,                  /* SPECIAL  */
  YYSYMBOL_SPECIAL_SURFACE_SYM = 185,      /* SPECIAL_SURFACE_SYM  */
  YYSYMBOL_SHEIGHT_FIELD = 186,            /* SHEIGHT_FIELD  */
  YYSYMBOL_SHEIGHT_FN = 187,               /* SHEIGHT_FN  */
  YYSYMBOL_SPHERICAL_BUMPMAP = 188,        /* SPHERICAL_BUMPMAP  */
  YYSYMBOL_SPHERICAL_IMAGEMAP = 189,       /* SPHERICAL_IMAGEMAP  */
  YYSYMBOL_SPHERICAL_INDEXED = 190,        /* SPHERICAL_INDEXED  */
  YYSYMBOL_SPLINE = 191,                   /* SPLINE  */
  YYSYMBOL_SPOT_LIGHT = 192,               /* SPOT_LIGHT  */
  YYSYMBOL_SQRT = 193,                     /* SQRT  */
  YYSYMBOL_SPECULAR = 194,                 /* SPECULAR  */
  YYSYMBOL_SPHERE = 195,                   /* SPHERE  */
  YYSYMBOL_START_FRAME = 196,              /* START_FRAME  */
  YYSYMBOL_STATIC = 197,                   /* STATIC  */
  YYSYMBOL_STRING = 198,                   /* STRING  */
  YYSYMBOL_STURM = 199,                    /* STURM  */
  YYSYMBOL_SUBSCRIPT_EXPER = 200,          /* SUBSCRIPT_EXPER  */
  YYSYMBOL_SUMMED = 201,                   /* SUMMED  */
  YYSYMBOL_SUPERQ = 202,                   /* SUPERQ  */
  YYSYMBOL_SURFACE = 203,                  /* SURFACE  */
  YYSYMBOL_SURFACE_SYM = 204,              /* SURFACE_SYM  */
  YYSYMBOL_SYSTEM = 205,                   /* SYSTEM  */
  YYSYMBOL_SWEEP = 206,                    /* SWEEP  */
  YYSYMBOL_TAN = 207,                      /* TAN  */
  YYSYMBOL_TANH = 208,                     /* TANH  */
  YYSYMBOL_TERM = 209,                     /* TERM  */
  YYSYMBOL_TEXTURE = 210,                  /* TEXTURE  */
  YYSYMBOL_TEXTURE_MAP = 211,              /* TEXTURE_MAP  */
  YYSYMBOL_TEXTURE_MAP_SYM = 212,          /* TEXTURE_MAP_SYM  */
  YYSYMBOL_TEXTURE_SYM = 213,              /* TEXTURE_SYM  */
  YYSYMBOL_TEXTURED_LIGHT = 214,           /* TEXTURED_LIGHT  */
  YYSYMBOL_TIMES_EXPER = 215,              /* TIMES_EXPER  */
  YYSYMBOL_TOKEN = 216,                    /* TOKEN  */
  YYSYMBOL_TORUS = 217,                    /* TORUS  */
  YYSYMBOL_TOTAL_FRAMES = 218,             /* TOTAL_FRAMES  */
  YYSYMBOL_TRACE = 219,                    /* TRACE  */
  YYSYMBOL_TRANSFORM = 220,                /* TRANSFORM  */
  YYSYMBOL_TRANSFORM_SYM = 221,            /* TRANSFORM_SYM  */
  YYSYMBOL_TRANSLATE = 222,                /* TRANSLATE  */
  YYSYMBOL_TRANSMISSION = 223,             /* TRANSMISSION  */
  YYSYMBOL_TURBULENCE = 224,               /* TURBULENCE  */
  YYSYMBOL_UMINUS_EXPER = 225,             /* UMINUS_EXPER  */
  YYSYMBOL_UP = 226,                       /* UP  */
  YYSYMBOL_U_EXPER = 227,                  /* U_EXPER  */
  YYSYMBOL_UU_EXPER = 228,                 /* UU_EXPER  */
  YYSYMBOL_UV_EXPER = 229,                 /* UV_EXPER  */
  YYSYMBOL_UW_EXPER = 230,                 /* UW_EXPER  */
  YYSYMBOL_U_STEPS = 231,                  /* U_STEPS  */
  YYSYMBOL_V_STEPS = 232,                  /* V_STEPS  */
  YYSYMBOL_UV = 233,                       /* UV  */
  YYSYMBOL_UV_STEPS = 234,                 /* UV_STEPS  */
  YYSYMBOL_UV_BOUNDS = 235,                /* UV_BOUNDS  */
  YYSYMBOL_VELOCITY = 236,                 /* VELOCITY  */
  YYSYMBOL_VIETA = 237,                    /* VIETA  */
  YYSYMBOL_VIEWPOINT = 238,                /* VIEWPOINT  */
  YYSYMBOL_VISIBLE = 239,                  /* VISIBLE  */
  YYSYMBOL_VAL_EXPER = 240,                /* VAL_EXPER  */
  YYSYMBOL_VEC_EXPER = 241,                /* VEC_EXPER  */
  YYSYMBOL_VECTOR_EXPER = 242,             /* VECTOR_EXPER  */
  YYSYMBOL_WAVE = 243,                     /* WAVE  */
  YYSYMBOL_W_EXPER = 244,                  /* W_EXPER  */
  YYSYMBOL_W_STEPS = 245,                  /* W_STEPS  */
  YYSYMBOL_X_EXPER = 246,                  /* X_EXPER  */
  YYSYMBOL_X_OFFSET = 247,                 /* X_OFFSET  */
  YYSYMBOL_Y_EXPER = 248,                  /* Y_EXPER  */
  YYSYMBOL_Y_OFFSET = 249,                 /* Y_OFFSET  */
  YYSYMBOL_YON = 250,                      /* YON  */
  YYSYMBOL_Z_EXPER = 251,                  /* Z_EXPER  */
  YYSYMBOL_252_ = 252,                     /* '&'  */
  YYSYMBOL_253_ = 253,                     /* '='  */
  YYSYMBOL_254_ = 254,                     /* '['  */
  YYSYMBOL_255_ = 255,                     /* ']'  */
  YYSYMBOL_256_ = 256,                     /* '('  */
  YYSYMBOL_257_ = 257,                     /* ')'  */
  YYSYMBOL_258_ = 258,                     /* '{'  */
  YYSYMBOL_259_ = 259,                     /* '}'  */
  YYSYMBOL_260_ = 260,                     /* '+'  */
  YYSYMBOL_261_ = 261,                     /* '-'  */
  YYSYMBOL_262_ = 262,                     /* '*'  */
  YYSYMBOL_263_ = 263,                     /* '.'  */
  YYSYMBOL_264_ = 264,                     /* '/'  */
  YYSYMBOL_265_ = 265,                     /* '>'  */
  YYSYMBOL_266_ = 266,                     /* '<'  */
  YYSYMBOL_267_ = 267,                     /* '^'  */
  YYSYMBOL_268_ = 268,                     /* '!'  */
  YYSYMBOL_269_ = 269,                     /* '~'  */
  YYSYMBOL_AND_SYM = 270,                  /* AND_SYM  */
  YYSYMBOL_OR_SYM = 271,                   /* OR_SYM  */
  YYSYMBOL_LTEQ_SYM = 272,                 /* LTEQ_SYM  */
  YYSYMBOL_GTEQ_SYM = 273,                 /* GTEQ_SYM  */
  YYSYMBOL_EQUAL_SYM = 274,                /* EQUAL_SYM  */
  YYSYMBOL_NEQUAL_SYM = 275,               /* NEQUAL_SYM  */
  YYSYMBOL_276_ = 276,                     /* '%'  */
  YYSYMBOL_UMINUS = 277,                   /* UMINUS  */
  YYSYMBOL_278_ = 278,                     /* ','  */
  YYSYMBOL_279_ = 279,                     /* ':'  */
  YYSYMBOL_280_ = 280,                     /* '?'  */
  YYSYMBOL_281_ = 281,                     /* '|'  */
  YYSYMBOL_YYACCEPT = 282,                 /* $accept  */
  YYSYMBOL_scene = 283,                    /* scene  */
  YYSYMBOL_284_1 = 284,                    /* $@1  */
  YYSYMBOL_elementlist = 285,              /* elementlist  */
  YYSYMBOL_element = 286,                  /* element  */
  YYSYMBOL_include_statement = 287,        /* include_statement  */
  YYSYMBOL_defined_token = 288,            /* defined_token  */
  YYSYMBOL_definition_types = 289,         /* definition_types  */
  YYSYMBOL_definition = 290,               /* definition  */
  YYSYMBOL_particle_decls = 291,           /* particle_decls  */
  YYSYMBOL_particle_decl = 292,            /* particle_decl  */
  YYSYMBOL_particle = 293,                 /* particle  */
  YYSYMBOL_294_2 = 294,                    /* $@2  */
  YYSYMBOL_object = 295,                   /* object  */
  YYSYMBOL_296_3 = 296,                    /* $@3  */
  YYSYMBOL_297_4 = 297,                    /* $@4  */
  YYSYMBOL_object_modifier_decls = 298,    /* object_modifier_decls  */
  YYSYMBOL_object_modifier_decl = 299,     /* object_modifier_decl  */
  YYSYMBOL_uv_information = 300,           /* uv_information  */
  YYSYMBOL_root_solver = 301,              /* root_solver  */
  YYSYMBOL_object_decls = 302,             /* object_decls  */
  YYSYMBOL_shape_decl = 303,               /* shape_decl  */
  YYSYMBOL_camera_exper = 304,             /* camera_exper  */
  YYSYMBOL_camera_expers = 305,            /* camera_expers  */
  YYSYMBOL_camera = 306,                   /* camera  */
  YYSYMBOL_flare_options = 307,            /* flare_options  */
  YYSYMBOL_flare_option = 308,             /* flare_option  */
  YYSYMBOL_light_modifier_decl = 309,      /* light_modifier_decl  */
  YYSYMBOL_310_5 = 310,                    /* $@5  */
  YYSYMBOL_light_modifier_decls = 311,     /* light_modifier_decls  */
  YYSYMBOL_depth_light_modifier = 312,     /* depth_light_modifier  */
  YYSYMBOL_depth_light_modifiers = 313,    /* depth_light_modifiers  */
  YYSYMBOL_haze_statement = 314,           /* haze_statement  */
  YYSYMBOL_light = 315,                    /* light  */
  YYSYMBOL_316_6 = 316,                    /* @6  */
  YYSYMBOL_317_7 = 317,                    /* @7  */
  YYSYMBOL_draw_statement = 318,           /* draw_statement  */
  YYSYMBOL_background = 319,               /* background  */
  YYSYMBOL_surface_declaration = 320,      /* surface_declaration  */
  YYSYMBOL_surface_declarations = 321,     /* surface_declarations  */
  YYSYMBOL_surface = 322,                  /* surface  */
  YYSYMBOL_323_8 = 323,                    /* $@8  */
  YYSYMBOL_324_9 = 324,                    /* $@9  */
  YYSYMBOL_texture_map_element = 325,      /* texture_map_element  */
  YYSYMBOL_texture_map_elements = 326,     /* texture_map_elements  */
  YYSYMBOL_texture_fn_element = 327,       /* texture_fn_element  */
  YYSYMBOL_texture_fn_elements = 328,      /* texture_fn_elements  */
  YYSYMBOL_texture_map = 329,              /* texture_map  */
  YYSYMBOL_texture_modifier_decls = 330,   /* texture_modifier_decls  */
  YYSYMBOL_texture_modifier_decl = 331,    /* texture_modifier_decl  */
  YYSYMBOL_texture_declarations = 332,     /* texture_declarations  */
  YYSYMBOL_texture_declaration = 333,      /* texture_declaration  */
  YYSYMBOL_texture = 334,                  /* texture  */
  YYSYMBOL_335_10 = 335,                   /* $@10  */
  YYSYMBOL_336_11 = 336,                   /* $@11  */
  YYSYMBOL_texture_list = 337,             /* texture_list  */
  YYSYMBOL_transform_declaration = 338,    /* transform_declaration  */
  YYSYMBOL_transform_declarations = 339,   /* transform_declarations  */
  YYSYMBOL_transform = 340,                /* transform  */
  YYSYMBOL_341_12 = 341,                   /* $@12  */
  YYSYMBOL_342_13 = 342,                   /* $@13  */
  YYSYMBOL_bezier_points = 343,            /* bezier_points  */
  YYSYMBOL_bezier = 344,                   /* bezier  */
  YYSYMBOL_blob = 345,                     /* blob  */
  YYSYMBOL_346_14 = 346,                   /* $@14  */
  YYSYMBOL_blobelements = 347,             /* blobelements  */
  YYSYMBOL_blobelement = 348,              /* blobelement  */
  YYSYMBOL_box = 349,                      /* box  */
  YYSYMBOL_cone = 350,                     /* cone  */
  YYSYMBOL_csg = 351,                      /* csg  */
  YYSYMBOL_352_15 = 352,                   /* $@15  */
  YYSYMBOL_csg_tree = 353,                 /* csg_tree  */
  YYSYMBOL_cylinder = 354,                 /* cylinder  */
  YYSYMBOL_cylindrical_height_field = 355, /* cylindrical_height_field  */
  YYSYMBOL_cylindrical_height_fn = 356,    /* cylindrical_height_fn  */
  YYSYMBOL_disc = 357,                     /* disc  */
  YYSYMBOL_function = 358,                 /* function  */
  YYSYMBOL_gridded = 359,                  /* gridded  */
  YYSYMBOL_object_list = 360,              /* object_list  */
  YYSYMBOL_height_field = 361,             /* height_field  */
  YYSYMBOL_height_fn = 362,                /* height_fn  */
  YYSYMBOL_hypertexture = 363,             /* hypertexture  */
  YYSYMBOL_lathe = 364,                    /* lathe  */
  YYSYMBOL_365_16 = 365,                   /* $@16  */
  YYSYMBOL_light_object = 366,             /* light_object  */
  YYSYMBOL_nurb = 367,                     /* nurb  */
  YYSYMBOL_parabola = 368,                 /* parabola  */
  YYSYMBOL_parametric = 369,               /* parametric  */
  YYSYMBOL_polygon = 370,                  /* polygon  */
  YYSYMBOL_371_17 = 371,                   /* $@17  */
  YYSYMBOL_polynomial = 372,               /* polynomial  */
  YYSYMBOL_patch_vertex = 373,             /* patch_vertex  */
  YYSYMBOL_ppatch = 374,                   /* ppatch  */
  YYSYMBOL_375_18 = 375,                   /* $@18  */
  YYSYMBOL_raw = 376,                      /* raw  */
  YYSYMBOL_smooth_height_field = 377,      /* smooth_height_field  */
  YYSYMBOL_smooth_height_fn = 378,         /* smooth_height_fn  */
  YYSYMBOL_smooth_cheight_field = 379,     /* smooth_cheight_field  */
  YYSYMBOL_smooth_cheight_fn = 380,        /* smooth_cheight_fn  */
  YYSYMBOL_smooth_sheight_field = 381,     /* smooth_sheight_field  */
  YYSYMBOL_smooth_sheight_fn = 382,        /* smooth_sheight_fn  */
  YYSYMBOL_sphere = 383,                   /* sphere  */
  YYSYMBOL_spherical_height_field = 384,   /* spherical_height_field  */
  YYSYMBOL_spherical_height_fn = 385,      /* spherical_height_fn  */
  YYSYMBOL_superq = 386,                   /* superq  */
  YYSYMBOL_contour = 387,                  /* contour  */
  YYSYMBOL_388_19 = 388,                   /* $@19  */
  YYSYMBOL_glyph_contours = 389,           /* glyph_contours  */
  YYSYMBOL_glyph = 390,                    /* glyph  */
  YYSYMBOL_391_20 = 391,                   /* $@20  */
  YYSYMBOL_sweep = 392,                    /* sweep  */
  YYSYMBOL_393_21 = 393,                   /* $@21  */
  YYSYMBOL_torus = 394,                    /* torus  */
  YYSYMBOL_fexper = 395,                   /* fexper  */
  YYSYMBOL_point = 396,                    /* point  */
  YYSYMBOL_sexper = 397,                   /* sexper  */
  YYSYMBOL_pointlist = 398,                /* pointlist  */
  YYSYMBOL_expression = 399,               /* expression  */
  YYSYMBOL_expression_list = 400,          /* expression_list  */
  YYSYMBOL_conditional = 401,              /* conditional  */
  YYSYMBOL_map_entry = 402,                /* map_entry  */
  YYSYMBOL_map_entries = 403,              /* map_entries  */
  YYSYMBOL_frame_decl = 404,               /* frame_decl  */
  YYSYMBOL_end_frame_decl = 405,           /* end_frame_decl  */
  YYSYMBOL_start_frame_decl = 406,         /* start_frame_decl  */
  YYSYMBOL_total_frames_decl = 407,        /* total_frames_decl  */
  YYSYMBOL_frame_time_decl = 408,          /* frame_time_decl  */
  YYSYMBOL_outfile = 409,                  /* outfile  */
  YYSYMBOL_flush_statement = 410,          /* flush_statement  */
  YYSYMBOL_system_call = 411,              /* system_call  */
  YYSYMBOL_statement = 412,                /* statement  */
  YYSYMBOL_if_else_part = 413,             /* if_else_part  */
  YYSYMBOL_414_22 = 414,                   /* $@22  */
  YYSYMBOL_if_statement = 415,             /* if_statement  */
  YYSYMBOL_416_23 = 416                    /* $@23  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2689

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  282
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  135
/* YYNRULES -- Number of rules.  */
#define YYNRULES  430
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1035

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   513


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   268,     2,     2,     2,   276,   252,     2,
     256,   257,   262,   260,   278,   261,   263,   264,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   279,     2,
     266,   253,   265,   280,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   254,     2,   255,   267,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   258,   281,   259,   269,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   270,   271,   272,
     273,   274,   275,   277
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   400,   400,   400,   405,   406,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   420,   421,   423,   424,   425,
     430,   436,   438,   440,   442,   444,   446,   448,   453,   456,
     459,   462,   465,   468,   471,   477,   480,   483,   487,   491,
     494,   497,   501,   508,   509,   513,   515,   517,   519,   521,
     523,   525,   527,   529,   535,   534,   538,   544,   543,   547,
     550,   549,   557,   558,   562,   566,   569,   571,   573,   577,
     579,   581,   582,   584,   586,   592,   593,   598,   603,   608,
     611,   614,   617,   625,   627,   629,   634,   635,   639,   640,
     641,   642,   643,   644,   645,   646,   647,   648,   649,   650,
     651,   652,   653,   654,   655,   656,   657,   658,   659,   660,
     661,   662,   663,   664,   665,   666,   667,   668,   669,   670,
     671,   672,   673,   674,   678,   680,   685,   687,   689,   691,
     693,   695,   697,   699,   709,   714,   718,   722,   727,   733,
     736,   738,   743,   744,   748,   752,   753,   757,   759,   761,
     763,   765,   767,   769,   774,   776,   779,   781,   784,   783,
     786,   789,   791,   793,   797,   799,   804,   805,   809,   811,
     813,   815,   817,   820,   822,   824,   826,   831,   832,   836,
     841,   843,   845,   848,   851,   853,   856,   859,   865,   864,
     868,   870,   872,   875,   879,   878,   886,   889,   894,   899,
     901,   903,   905,   907,   909,   911,   913,   915,   917,   919,
     921,   923,   925,   927,   929,   931,   933,   935,   937,   939,
     941,   943,   945,   947,   949,   951,   953,   956,   959,   964,
     965,   970,   969,   973,   976,   975,   982,   988,   990,   996,
    1001,  1003,  1008,  1010,  1015,  1016,  1020,  1028,  1030,  1032,
    1036,  1038,  1043,  1047,  1049,  1051,  1053,  1055,  1058,  1060,
    1063,  1069,  1068,  1072,  1075,  1074,  1081,  1083,  1088,  1090,
    1092,  1094,  1099,  1100,  1105,  1104,  1108,  1111,  1110,  1117,
    1119,  1124,  1132,  1131,  1140,  1141,  1145,  1147,  1149,  1152,
    1155,  1161,  1166,  1172,  1172,  1179,  1181,  1184,  1188,  1191,
    1194,  1197,  1200,  1206,  1212,  1216,  1223,  1226,  1233,  1236,
    1242,  1247,  1253,  1260,  1270,  1276,  1282,  1289,  1295,  1294,
    1305,  1310,  1315,  1323,  1329,  1335,  1334,  1346,  1351,  1357,
    1367,  1366,  1373,  1376,  1382,  1388,  1394,  1401,  1404,  1411,
    1414,  1420,  1424,  1431,  1434,  1440,  1446,  1449,  1456,  1459,
    1465,  1471,  1470,  1485,  1486,  1491,  1490,  1506,  1505,  1516,
    1522,  1534,  1546,  1558,  1561,  1567,  1569,  1571,  1574,  1577,
    1580,  1582,  1584,  1586,  1588,  1590,  1592,  1594,  1596,  1598,
    1601,  1603,  1605,  1607,  1609,  1611,  1613,  1616,  1618,  1620,
    1622,  1624,  1626,  1629,  1632,  1634,  1637,  1642,  1644,  1649,
    1651,  1653,  1655,  1657,  1659,  1661,  1663,  1665,  1670,  1674,
    1682,  1684,  1690,  1691,  1692,  1693,  1697,  1702,  1708,  1713,
    1718,  1724,  1733,  1738,  1743,  1744,  1749,  1748,  1751,  1756,
    1755
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ACCELERATION", "ACOS",
  "AMBIENT", "AND_EXPER", "ANGLE", "ANTIALIAS", "ANTIALIAS_THRESHOLD",
  "APERTURE", "ARRAY", "ASSIGNMENT", "ASIN", "ASPECT", "AT", "ATAN",
  "ATAN_TWO", "AVOID", "BACKGROUND", "BEZIER", "BIAS", "BIRTH", "BLINN",
  "BLOB", "BOUNDING_BOX", "BOX", "BRILLIANCE", "BUMP_SCALE", "CEIL",
  "CHECKER", "CHEIGHT_FIELD", "CHEIGHT_FN", "COLOR", "COLOR_MAP",
  "COLOR_WHEEL", "CONCAT", "CONCAVE", "CONDITIONAL_EXPER", "CONE",
  "CONTOUR", "COOK", "COS", "COSH", "COUNT", "CSG", "CYLINDER",
  "CYLINDRICAL_BUMPMAP", "CYLINDRICAL_IMAGEMAP", "CYLINDRICAL_INDEXED",
  "DEATH", "DEFINE", "DEGREES", "DEPTH", "DEPTHMAPPED_LIGHT", "DIFFUSE",
  "DIRECTIONAL_LIGHT", "DISC", "DISPLACE", "DITHER", "DIV_EXPER", "DNOISE",
  "DOT_EXPER", "DRAW", "ELSE", "END_FRAME", "ENVIRONMENT",
  "ENVIRONMENT_MAP", "EQUAL_EXPER", "EXP", "EXPRESSION_SYM", "FABS", "FBM",
  "FERRARI", "FILE_FLUSH", "FLARE", "FLOCK", "FLOOR", "FNOISE",
  "FOCAL_DISTANCE", "FMOD", "FRAME", "FRAME_TIME", "FREQUENCY", "FROM",
  "FUNCTION", "GAIN", "GAUSSIAN", "GLYPH", "GREATER_EXPER", "GRIDDED",
  "GTEQ_EXPER", "HAZE", "HEIGHT_FIELD", "HEIGHT_FN", "HEIGHT_MAP",
  "HEXAGON", "HITHER", "HYPERTEXTURE", "I_EXPER", "IF", "IMAGE",
  "IMAGE_FORMAT", "IMAGE_WINDOW", "INCLUDE", "INDEXED", "INDEXED_MAP",
  "LATHE", "LAYERED", "LEGENDRE", "LENSES", "LESS_EXPER", "LIGHT", "LN",
  "LOG", "LOOKUP_FUNCTION", "LTEQ_EXPER", "MAXT", "MAX_SAMPLES",
  "MAX_TRACE_DEPTH", "MICROFACET", "MINT", "MINUS_EXPER", "N_EXPER",
  "NO_SHADOW", "NOEVAL", "NOISE", "NORMAL", "NOT_EXPER", "NUM", "NURB",
  "OBJECT", "OBJECT_SYM", "OCTAVES", "OR_EXPER", "OPACITY", "OUTFILE",
  "P_EXPER", "PARABOLA", "PARAMETRIC", "PARTICLE", "PARTICLE_SYM", "PATCH",
  "PHASE", "PHONG", "PIXEL_ENCODING", "PIXELSIZE", "PLANAR_BUMPMAP",
  "PLANAR_IMAGEMAP", "PLANE", "PLUS_EXPER", "POINT", "POLYGON",
  "POLYNOMIAL", "POSITION", "POSITION_FUNCTION", "POSITION_SCALE",
  "POWER_EXPER", "RADIANS", "RAMP", "RANDOM", "RAW", "REFLECT",
  "REFLECTION", "REITZ", "RESOLUTION", "RIPPLE", "ROOT_SOLVER", "ROTATE",
  "SAWTOOTH", "SCALE", "SEED", "SHADING_FLAGS", "SHEAR", "SIN", "SINH",
  "SIZE", "SMOOTH_HEIGHT_FIELD", "SMOOTH_HEIGHT_FN",
  "SMOOTH_CHEIGHT_FIELD", "SMOOTH_CHEIGHT_FN", "SMOOTH_SHEIGHT_FIELD",
  "SMOOTH_SHEIGHT_FN", "SPACING", "SPECIAL", "SPECIAL_SURFACE_SYM",
  "SHEIGHT_FIELD", "SHEIGHT_FN", "SPHERICAL_BUMPMAP", "SPHERICAL_IMAGEMAP",
  "SPHERICAL_INDEXED", "SPLINE", "SPOT_LIGHT", "SQRT", "SPECULAR",
  "SPHERE", "START_FRAME", "STATIC", "STRING", "STURM", "SUBSCRIPT_EXPER",
  "SUMMED", "SUPERQ", "SURFACE", "SURFACE_SYM", "SYSTEM", "SWEEP", "TAN",
  "TANH", "TERM", "TEXTURE", "TEXTURE_MAP", "TEXTURE_MAP_SYM",
  "TEXTURE_SYM", "TEXTURED_LIGHT", "TIMES_EXPER", "TOKEN", "TORUS",
  "TOTAL_FRAMES", "TRACE", "TRANSFORM", "TRANSFORM_SYM", "TRANSLATE",
  "TRANSMISSION", "TURBULENCE", "UMINUS_EXPER", "UP", "U_EXPER",
  "UU_EXPER", "UV_EXPER", "UW_EXPER", "U_STEPS", "V_STEPS", "UV",
  "UV_STEPS", "UV_BOUNDS", "VELOCITY", "VIETA", "VIEWPOINT", "VISIBLE",
  "VAL_EXPER", "VEC_EXPER", "VECTOR_EXPER", "WAVE", "W_EXPER", "W_STEPS",
  "X_EXPER", "X_OFFSET", "Y_EXPER", "Y_OFFSET", "YON", "Z_EXPER", "'&'",
  "'='", "'['", "']'", "'('", "')'", "'{'", "'}'", "'+'", "'-'", "'*'",
  "'.'", "'/'", "'>'", "'<'", "'^'", "'!'", "'~'", "AND_SYM", "OR_SYM",
  "LTEQ_SYM", "GTEQ_SYM", "EQUAL_SYM", "NEQUAL_SYM", "'%'", "UMINUS",
  "','", "':'", "'?'", "'|'", "$accept", "scene", "$@1", "elementlist",
  "element", "include_statement", "defined_token", "definition_types",
  "definition", "particle_decls", "particle_decl", "particle", "$@2",
  "object", "$@3", "$@4", "object_modifier_decls", "object_modifier_decl",
  "uv_information", "root_solver", "object_decls", "shape_decl",
  "camera_exper", "camera_expers", "camera", "flare_options",
  "flare_option", "light_modifier_decl", "$@5", "light_modifier_decls",
  "depth_light_modifier", "depth_light_modifiers", "haze_statement",
  "light", "@6", "@7", "draw_statement", "background",
  "surface_declaration", "surface_declarations", "surface", "$@8", "$@9",
  "texture_map_element", "texture_map_elements", "texture_fn_element",
  "texture_fn_elements", "texture_map", "texture_modifier_decls",
  "texture_modifier_decl", "texture_declarations", "texture_declaration",
  "texture", "$@10", "$@11", "texture_list", "transform_declaration",
  "transform_declarations", "transform", "$@12", "$@13", "bezier_points",
  "bezier", "blob", "$@14", "blobelements", "blobelement", "box", "cone",
  "csg", "$@15", "csg_tree", "cylinder", "cylindrical_height_field",
  "cylindrical_height_fn", "disc", "function", "gridded", "object_list",
  "height_field", "height_fn", "hypertexture", "lathe", "$@16",
  "light_object", "nurb", "parabola", "parametric", "polygon", "$@17",
  "polynomial", "patch_vertex", "ppatch", "$@18", "raw",
  "smooth_height_field", "smooth_height_fn", "smooth_cheight_field",
  "smooth_cheight_fn", "smooth_sheight_field", "smooth_sheight_fn",
  "sphere", "spherical_height_field", "spherical_height_fn", "superq",
  "contour", "$@19", "glyph_contours", "glyph", "$@20", "sweep", "$@21",
  "torus", "fexper", "point", "sexper", "pointlist", "expression",
  "expression_list", "conditional", "map_entry", "map_entries",
  "frame_decl", "end_frame_decl", "start_frame_decl", "total_frames_decl",
  "frame_time_decl", "outfile", "flush_statement", "system_call",
  "statement", "if_else_part", "$@22", "if_statement", "$@23", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-856)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-362)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -856,    38,  2091,  -856,  1752,   228,  -216,  1159,  1752,  1752,
    1752,  1752,  1752,  -212,  -144,  1211,  -181,  -172,  -152,  -170,
    -856,  1752,  1266,  1752,    51,  -160,  -148,  1752,  -146,  2091,
    -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -128,  -856,  -856,  -856,  -126,  -856,  -124,  -856,  -856,  -113,
    -856,  1752,  1347,  1752,  1752,  1752,  2189,  -856,   333,  -856,
    -856,  -856,  -856,  -856,   961,  -856,   961,  -856,  1752,  -129,
    2189,  -127,  2189,  -856,  -856,  -856,  -121,  1347,  -856,  1752,
    -118,  -856,  -856,  -856,  -856,  -856,   674,  1752,  -104,  -856,
     250,  1752,  -856,  -856,   561,  -856,   -75,  1752,  1752,  1752,
    2189,  -229,  1347,  1347,  1386,  -218,   -66,   751,   411,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,   961,   961,   -93,
     -45,   -44,   -54,  -856,   -40,   -39,   -38,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  2189,  -856,  -856,   -86,  1752,  1752,
    1752,  2107,   -11,   -50,  1752,  2306,  1898,   207,  1752,   -46,
    1752,   426,   961,   961,  -247,  -856,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  -856,   197,  1752,  -856,  -238,   496,
     822,  -235,  -856,  1752,   -89,  -856,  -856,  1752,  1752,  1752,
    1752,  1752,  1347,  1347,  1752,  1752,  -856,  1533,     8,     8,
    -186,  -186,  -186,   -66,  -186,  -856,  -856,  -856,  -856,  -856,
     -17,  -856,  -856,  -856,    14,  1752,  -856,   -31,   -25,  -856,
    1752,  -856,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    -856,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,     5,  1898,  -856,  -856,
    -856,  -856,  -856,  -856,  -117,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  1752,  1752,  1752,   -64,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
     238,  -856,  -856,  -856,  -856,  -856,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,   111,  -856,  2189,  1752,   -23,
     961,   961,  -856,  -856,  -856,  1279,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,   -12,  -856,  -856,  -856,
    -856,    -1,  -856,  -856,  -856,  -856,    17,  -856,  1752,  -856,
    -856,  1752,  1752,  -856,  2189,  -856,  2189,  2189,  2189,  2189,
    2189,  -856,  -856,   560,   435,  -856,  -856,  -856,   213,  1752,
    -856,  -230,  -856,  -138,  -138,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  -856,  1752,  -856,  -856,  -856,  1752,  1752,  1910,
    -856,    23,   -18,    27,    30,  2189,    39,    53,    54,    55,
    2189,  -856,    57,  -856,    62,  2189,    66,    67,    70,  2189,
    1752,    80,  2189,    81,  -856,    85,    87,    88,    93,    94,
      95,    96,    98,    99,   100,   101,  -856,  1898,  -117,  -117,
    -856,    52,   102,  2189,  -856,  -856,  -856,  -856,   105,  -856,
    -856,   106,  -856,  -856,  -856,   108,   109,  -856,  -856,  -856,
    2189,  2189,  2189,  2189,  2189,  2189,  2189,  2189,  2189,  -856,
    -856,   110,  1752,  -856,  -856,  1752,  -856,  -856,  1752,  1752,
    1752,  1752,  1752,  1752,  -856,  -856,  -856,  1752,  1752,  1752,
     271,   522,   586,  1752,  -856,  1752,   397,   776,   -68,   -68,
    1752,   -68,   -56,   -56,  1752,  -856,    15,  -856,   112,  -856,
     -17,   -95,  1752,  1752,  1752,  -856,   -76,   -69,  -856,  -856,
    -856,  2189,  -856,  -856,  -856,  -856,   114,  -856,  2091,  -856,
     203,  1752,  -856,  1752,  1752,  1752,  1752,  1752,  1752,   242,
      45,  1752,  1752,  1752,  1752,   120,   122,  -856,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
      89,  -856,  -117,  -117,  -117,  -117,  -117,  1752,  1752,  1752,
    1752,  1752,  1752,   123,  2189,    34,   129,   131,  -856,   134,
     137,  -856,   140,  -856,   141,  -856,  -856,  -856,  1752,  1194,
     612,  1752,  1752,  1752,  1752,   133,  1752,  1752,  1752,   881,
    1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    -856,  -856,  -856,   142,   143,   891,  -856,   144,  -856,  -856,
    -856,   149,   971,  -856,     2,  1752,  -856,  1752,  1752,  1752,
    1752,  -856,  -856,  -856,   150,  -856,  -856,  -856,  -856,  -856,
    1752,  1840,  -856,  -856,   151,  1062,  -856,   155,   156,   157,
     158,   159,  1752,  -856,   242,  -856,    45,   160,   161,   164,
     167,  1752,  1752,  1752,  -856,   168,   172,   175,   177,   179,
     183,   186,  -856,  -856,   189,   190,  -856,    18,  -167,  -167,
     208,  -856,  -856,  -856,   198,   199,   201,   204,  1752,  -856,
    1752,  1752,  1752,  1752,  1752,  1752,  1767,  -856,  -856,  1752,
     992,  2189,  2189,  2189,   -75,  1226,  2189,  2189,  1752,  1752,
    1752,  1752,  1752,  2189,  2189,  2189,  2189,  2189,  2189,  2189,
    1363,  1422,  1460,  2189,   -68,   -68,   -26,   -68,  1752,   -68,
     206,   212,  -856,   214,  -856,  1752,  1488,  -856,  1910,  1752,
    1752,  1752,  1752,  1752,   215,  -856,   216,  1752,  1752,  1752,
    1752,  1752,   217,  -856,  -856,  1752,  1752,  1752,  1752,   222,
     258,  -856,   224,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  1752,  1752,  1752,  1752,   225,    32,   226,  -856,
     227,  -856,   229,   230,  -856,  2079,  1752,  -191,  1752,  2189,
    2189,  2189,  2189,  2189,  1752,  1752,  1752,  -856,   231,  -856,
    -856,  -856,  -856,   -68,  1752,  1752,  -856,  1752,  -856,   233,
     235,   236,   237,   243,  1062,  1752,  -856,  1588,   244,  -856,
     245,  -856,   251,  1622,   263,   277,  -856,  1752,  1752,  1752,
     278,  1622,  -856,  1813,  -856,  1877,  -856,  1938,   279,   281,
     283,  -856,   284,   285,  1752,  1752,  1752,  1752,  1752,  1752,
    1752,  1752,  -856,  -856,  1752,  1752,  1752,  1752,  -856,  2189,
    -856,  1752,  2189,  2189,  2189,  1983,   -68,   287,  -856,   288,
    2189,  1752,  1752,  1752,  1752,  1752,  -856,   294,  1752,  1752,
    1752,  1752,  1752,  -856,  1752,  -856,   296,  -856,  1752,  1752,
    1752,  1752,  -856,  1752,  1752,  1752,  1752,   299,  2189,  -856,
    -856,  -856,   301,  -856,  -856,   304,   306,  -856,   308,   261,
    2147,  2165,  1752,  -856,   -68,  1752,   309,   310,   311,   312,
     314,  1752,   315,  -856,  -856,   224,   317,  1752,   318,  1752,
     323,   340,   341,   344,  1752,  -856,   346,  -856,   347,  1752,
    1752,  1752,  1752,  1752,  -856,  -856,  2189,   371,   349,  1752,
    1752,  1752,  1752,  1752,  -856,  1752,  1752,   224,  1752,  -856,
    1752,  1752,  1752,  1752,   224,  1752,  1752,  -856,  -856,  -856,
     351,   353,  -856,  1752,   354,  -856,   357,   359,  -856,   363,
    -856,   365,  2013,   366,  -856,  -856,  -856,   370,  -856,  1752,
    1752,   379,  1752,  1752,  1752,  1752,  1752,  1752,  1752,  1752,
     382,   394,  1752,  -856,  -856,  -856,   383,   384,  2039,   388,
    -856,  1752,  -856,   392,  1752,  1752,  1752,  1752,  -856,  1752,
    -856,  2189,  2189,  2189,  -856
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     0,     1,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,     0,     0,
      56,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       5,    13,     8,    19,    16,     7,    15,    14,     9,     6,
      11,   412,   413,   414,   415,    17,    10,    18,    12,   387,
       0,   389,   396,   388,     0,   394,     0,   390,   395,   393,
     391,     0,     0,     0,     0,     0,   198,    25,     0,    24,
      27,    21,    23,    22,     0,    26,     0,   194,     0,   190,
     361,     0,   360,   416,   422,   419,     0,     0,    20,     0,
     181,    57,    60,   421,   420,    54,     0,     0,     0,   417,
       0,     0,   188,   418,     0,     4,     0,     0,     0,     0,
     397,     0,     0,     0,     0,     0,   379,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     233,     0,     0,   242,   263,     0,   276,    37,    34,    31,
      28,    30,    29,    32,    33,    35,   178,   192,     0,     0,
       0,     0,     0,   183,     0,   293,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   167,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   142,     0,     0,   410,     0,     0,
       0,     0,   366,     0,     0,   407,   365,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   380,     0,   377,   378,
     374,   375,   376,   372,   373,    41,    39,   231,   234,   261,
       0,   264,   274,   277,     0,     0,   191,     0,     0,   429,
       0,   180,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     330,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    86,   320,    88,
      89,    90,    91,    95,     0,    92,    93,    94,    96,    97,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   114,   115,   112,   113,   116,   117,   118,
     119,   120,   121,    98,   122,   123,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    62,    71,    75,    64,    65,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    43,   197,     0,   184,
       0,     0,    38,    36,   423,     0,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,     0,   135,   136,   137,
     138,     0,   140,   141,   144,   143,     0,   382,     0,   411,
     383,     0,     0,   392,   398,   399,   401,   400,   402,   403,
     404,   405,   406,     0,     0,   370,   230,   230,     0,     0,
     237,     0,   245,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   176,     0,   195,   177,   193,     0,     0,     0,
     182,     0,     0,     0,   304,   362,     0,     0,     0,     0,
     310,   355,     0,   314,     0,   317,     0,     0,     0,   324,
       0,     0,   327,   332,   334,     0,   337,     0,   341,     0,
     346,     0,     0,     0,     0,     0,    58,    87,     0,     0,
     302,   294,     0,    76,    73,    83,    85,    84,    66,    70,
      72,     0,    69,    79,    80,     0,     0,    81,    61,    63,
      49,    50,    45,    52,    46,    51,    53,    47,    48,    55,
      44,   186,     0,    42,    40,     0,   158,   157,     0,     0,
       0,     0,     0,     0,   189,   166,   160,     0,     0,     0,
       0,     0,     0,     0,   367,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   253,     0,   245,     0,   243,
       0,     0,     0,     0,     0,   272,     0,     0,   168,   169,
     170,   171,   172,   173,   174,   175,     0,   179,     0,   425,
     428,     0,   282,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   325,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   299,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   154,     0,     0,   161,   165,     0,
       0,   164,     0,   139,     0,   381,   384,   385,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     232,   229,   235,     0,     0,     0,   266,   258,   255,   254,
     240,   260,     0,   262,   252,     0,   238,     0,     0,     0,
       0,   265,   244,   246,   268,   270,   271,   275,   273,   278,
       0,     0,   426,   430,     0,     0,   291,     0,     0,     0,
       0,     0,     0,   353,   356,   312,   311,     0,     0,     0,
       0,     0,     0,     0,   333,     0,     0,     0,     0,     0,
       0,     0,   345,   350,     0,     0,   295,   300,   296,   297,
     298,   301,    74,    67,     0,    77,     0,     0,     0,   146,
       0,     0,     0,     0,     0,     0,     0,   371,   368,     0,
     203,   204,   205,   199,     0,   207,   208,   209,     0,     0,
       0,     0,     0,   215,   216,   218,   219,   217,   220,   221,
     223,   225,     0,   228,     0,     0,     0,     0,     0,     0,
       0,   247,   251,     0,   250,     0,     0,   424,     0,     0,
       0,     0,     0,     0,   283,   284,     0,     0,     0,     0,
       0,     0,     0,   354,   313,     0,     0,     0,     0,     0,
     328,   363,   326,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   162,
       0,   155,     0,     0,   386,     0,     0,     0,     0,   211,
     214,   212,   210,   213,     0,     0,     0,   256,     0,   259,
     267,   241,   239,     0,     0,     0,   269,     0,   427,     0,
       0,     0,     0,     0,     0,     0,   305,   306,     0,   303,
     308,   351,     0,   316,     0,     0,   323,     0,     0,     0,
       0,   336,   338,   339,   342,   343,   347,   348,     0,     0,
       0,    78,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   159,   145,     0,     0,     0,     0,   369,   202,
     201,     0,   206,   222,   224,   227,     0,     0,   248,     0,
     196,     0,     0,     0,     0,     0,   285,     0,     0,     0,
       0,     0,     0,   318,     0,   331,     0,   364,     0,     0,
       0,     0,   357,     0,     0,     0,     0,     0,   147,   152,
     148,   150,     0,   149,   153,     0,     0,   134,     0,     0,
     360,     0,     0,   257,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   292,   309,   352,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   359,     0,    82,     0,     0,
       0,     0,     0,     0,   408,   200,   226,     0,     0,     0,
       0,     0,     0,     0,   286,     0,     0,   319,     0,   329,
       0,     0,     0,     0,   358,     0,     0,   185,   151,   156,
       0,     0,   236,     0,   281,   280,     0,     0,   287,     0,
     307,     0,   322,     0,   340,   344,   349,     0,   187,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   279,   288,   289,     0,     0,     0,     0,
      68,     0,   409,     0,     0,     0,     0,     0,   163,     0,
     290,   315,   321,   335,   249
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -856,  -856,  -856,    -2,   -28,  -856,   -55,   -72,  -856,  -856,
     342,     4,  -856,    78,  -856,  -856,   409,  -313,  -856,  -856,
    -856,  -856,   497,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,   526,  -856,  -856,  -856,  -856,  -856,   297,
    -376,  -856,  -856,   163,  -856,   -53,  -856,   -48,   169,  -856,
    -856,  -856,  -154,  -856,  -856,  -856,  -310,   300,  -136,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  -134,  -856,  -856,  -856,
    -856,  -286,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -666,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,  -856,  -856,    29,  -856,  -856,  -856,  -856,
    -856,  -856,  -856,   807,  1687,  -219,  -855,    -4,   -30,   -62,
    -180,   -10,  -856,  -856,  -856,  -856,  -856,  -856,  -856,  -856,
     -57,  -856,  -856,  -856,  -856
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    29,    30,    31,    76,   137,    32,   335,
     336,   138,   157,   139,   155,   156,   320,   321,   322,   323,
     266,   267,   184,   185,    35,   787,   863,   495,   585,   345,
     405,   224,    36,    37,   165,   146,    38,    39,   621,   506,
     140,   386,   387,   390,   391,   630,   631,   141,   521,   642,
     516,   517,   142,   388,   392,   627,   525,   526,   143,   393,
     394,   984,   269,   270,   655,   754,   755,   271,   272,   273,
     274,   451,   275,   276,   277,   278,   279,   280,   666,   281,
     282,   283,   284,   937,   285,   286,   287,   288,   289,   673,
     290,   555,   291,   430,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   663,   891,   664,   303,   549,
     304,   944,   305,   756,   771,   414,   772,    82,   111,   115,
     187,   188,    40,    41,    42,    43,    44,    45,    46,    47,
     540,   653,   748,    48,   409
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      66,   105,   324,    80,   145,   769,    33,   469,   369,   455,
     344,    80,   515,   128,    16,    17,   186,    96,    80,   367,
     325,   395,   373,   422,   423,   152,   192,   519,   396,   397,
     522,   193,   523,    33,   433,   434,   935,   436,     3,   438,
     368,   440,    77,   193,    87,   163,    93,   398,   520,   193,
     194,   195,   202,   203,    88,   215,   216,   110,   114,   116,
     117,   118,   204,   186,    94,   855,   870,   399,   119,   856,
     144,   164,   144,   637,    80,   638,   857,    91,   639,   191,
      34,   125,   967,   151,   524,    80,    92,   871,    95,   974,
     342,   343,   522,    80,   523,   575,   101,   110,   400,   522,
     576,   523,   100,   189,   190,   110,   341,    34,   114,   151,
     102,   401,   104,   324,   326,   207,   208,   209,   210,   211,
     212,   213,   214,   144,   144,   135,   136,   640,   106,   327,
     107,   325,   108,   328,   469,   456,   628,   629,   402,   448,
     381,   382,   131,   109,    80,   134,   524,   129,   130,   148,
      80,   149,   449,   524,   337,   329,    80,   150,   144,   144,
     154,   330,   570,   571,   641,   217,   324,    80,   375,    80,
     637,   895,   638,   457,   160,   639,    16,    17,    80,   186,
     532,   202,   203,   647,   325,   132,   133,   331,   119,   374,
     649,   204,   225,   376,   377,   378,   379,   380,   151,   151,
     383,   384,   220,   858,   166,   167,   168,   169,   859,   496,
     326,   170,   171,   218,   219,   860,   648,   648,   221,   222,
     223,    80,   135,   136,   640,   327,    80,   861,   230,   328,
      80,   415,   338,    80,    80,    80,   420,   389,   415,   415,
     403,   425,   332,   508,    80,   429,   229,   407,   432,   415,
     415,   329,   415,   408,   415,   482,   415,   330,    80,   202,
     203,   542,   119,   306,   446,   333,   497,   652,   483,   484,
     122,   123,   124,   404,   633,   125,   172,   498,   573,   574,
     575,   173,   662,   331,   126,   576,   687,   688,   689,   690,
     691,   862,   699,   324,   174,   499,   307,   308,    67,   175,
     176,   541,    80,   453,   572,   543,    80,    80,   544,   509,
      80,   325,   573,   574,   575,   177,   178,   545,   510,   576,
      67,   511,   470,   471,   472,   473,   474,   475,   476,   477,
     478,   546,   547,   548,    80,   550,   144,   144,   332,   512,
     551,   572,   179,   180,   552,   553,   686,   334,   554,   573,
     574,   575,   450,    68,   623,   624,   576,   626,   557,   558,
      69,   333,   181,   559,   500,   560,   561,   501,   502,    70,
     479,   562,   563,   564,   565,   161,   566,   567,   568,   569,
     577,   539,    69,   578,   579,   643,   580,   581,   582,   714,
     635,    70,   650,    80,   531,   415,    80,   513,   671,    80,
     672,   698,   601,    67,    80,   309,   310,   700,   311,   701,
     312,   313,   702,    33,   514,   703,   129,   130,   704,   705,
     734,   735,   737,   182,   602,   603,    80,   738,   745,   749,
     604,   605,    71,   757,   758,   759,   760,   761,   765,   766,
      72,    73,   767,   334,    74,   768,   773,   183,   131,    75,
     774,   134,   606,   775,    71,   776,   364,   777,   135,   136,
     314,   778,    72,    73,   779,    69,   162,   780,   781,   315,
     316,    75,   317,   318,    70,   576,   782,   783,    80,   784,
     607,   584,   785,   319,   813,    80,    80,    34,    80,    80,
     814,   838,   815,   824,   825,   831,    67,   468,   643,   599,
     837,   600,   839,   854,   864,   865,   625,   866,   867,   876,
     632,   881,   608,   882,   883,   884,   954,   609,    80,    80,
      80,   885,   889,   890,   610,   119,   450,   450,   595,   892,
     611,   120,   121,   122,   123,   124,   651,    71,   125,    80,
     612,   893,    33,    80,    80,    72,    73,   126,    80,   127,
      80,   613,   614,   615,    75,   894,   898,   902,    69,   903,
     616,   904,   905,   906,    80,   924,   925,    70,   166,   167,
     168,   169,   931,    80,   939,   170,   171,   949,    80,   950,
     807,   808,   951,   810,   952,   812,   953,   959,   960,   961,
     962,   617,   963,   965,   706,   966,   968,   710,   711,   712,
     713,   970,   715,   716,   717,   723,   724,   725,   726,   727,
     728,   729,   730,   731,   732,   733,    34,   369,   971,   972,
     618,   619,   973,   105,   975,   976,   982,   983,   665,   999,
      71,  1000,  1002,    80,    80,  1003,    80,  1004,    72,    73,
     172,  1005,   340,  1006,  1008,   173,   746,    75,  1009,  1022,
     450,   450,   450,   450,   450,    33,   620,  1012,   174,   877,
    1021,  1024,  1025,   175,   176,   119,  1027,    80,    80,    80,
    1029,   120,   121,   122,   123,   124,   447,   480,   125,   177,
     178,   268,   365,   636,   507,   811,   634,   126,   809,   119,
     886,   818,   206,   763,   527,   120,   121,   122,   123,   124,
     504,    80,   125,     0,   797,   795,   179,   180,     0,     0,
       0,   126,     0,   505,   799,   800,   801,   802,   803,     0,
     539,     0,   923,     0,     0,     0,   181,     0,     0,    34,
       0,     0,     0,     0,   632,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   764,     0,    80,    80,    80,    80,
     119,     0,    33,   370,   827,    80,   120,   121,   122,   123,
     124,   833,     0,   125,     0,     0,     0,     0,     0,   841,
     957,   843,   126,   845,   371,   847,   119,    80,     0,   596,
       0,   601,   120,   121,   122,   123,   124,   182,     0,   125,
       0,     0,   869,     0,   872,     0,     0,     0,   126,     0,
     873,   874,   875,   602,   603,     0,     0,     0,     0,   604,
     605,   183,     0,   880,   119,    81,    83,    84,    85,    86,
     120,   121,   122,   123,   124,     0,    34,   125,     0,     0,
      99,   606,     0,    80,   103,    80,   126,     0,     0,   503,
     119,     0,     0,   597,     0,     0,   120,   121,   122,   123,
     124,   908,     0,   125,     0,     0,     0,     0,     0,   607,
       0,     0,   126,   920,   598,     0,   119,   921,     0,     0,
       0,     0,   120,   121,   122,   123,   124,   708,    80,   125,
       0,    80,     0,     0,     0,     0,     0,    80,   126,     0,
     709,   608,     0,     0,     0,     0,   609,     0,     0,    80,
       0,     0,     0,   610,   718,     0,     0,     0,     0,   611,
       0,     0,     0,     0,    49,    50,     0,     0,   956,   612,
       0,     0,   719,     0,     0,     0,     0,    80,   119,     0,
     613,   614,   615,    80,   120,   121,   122,   123,   124,   616,
      80,   125,     0,     0,     0,     0,    51,     0,     0,    80,
     126,    52,   158,     0,     0,    80,   227,   228,     0,     0,
       0,     0,    53,     0,   992,     0,     0,     0,   720,     0,
     617,     0,     0,   346,   347,   348,   349,   350,     0,   352,
       0,   354,   355,   356,   357,   358,   359,   360,   361,     0,
     363,     0,     0,   366,    49,    50,     0,     0,    80,   618,
     619,     0,     0,  1018,     0,   119,     0,    54,     0,     0,
      55,   120,   121,   122,   123,   124,     0,     0,   125,     0,
       0,  1031,  1032,  1033,     0,   721,    51,   126,     0,   205,
       0,    52,     0,     0,     0,   622,     0,     0,     0,   411,
     412,     0,    53,   416,     0,   722,     0,     0,   421,    56,
       0,   424,     0,   426,   427,     0,     0,     0,   431,     0,
       0,     0,   435,     0,   437,     0,   439,     0,   441,     0,
     443,   444,   445,     0,     0,     0,   119,    57,     0,    58,
       0,     0,   120,   121,   122,   123,   124,    54,     0,   125,
      55,     0,    16,    17,     0,    49,    50,    59,   126,    60,
     372,    19,    20,     0,     0,     0,     0,     0,   750,     0,
       0,     0,     0,     0,     0,   454,     0,     0,     0,   460,
     461,     0,   463,   464,   465,   466,   467,    51,     0,    56,
       0,     0,    52,     0,     0,    61,     0,    62,     0,     0,
       0,     0,    63,    53,     0,   119,     0,    64,     0,     0,
       0,   120,   121,   122,   123,   124,     0,    57,   125,    58,
       0,     0,    65,     0,   129,   130,     0,   126,     0,   736,
       0,   131,   132,   133,   134,     0,     0,    59,     0,    60,
       0,   135,   136,     0,     0,     0,     0,     0,    54,     0,
       0,    55,    49,    50,     0,     0,   518,     0,     0,     0,
       0,     0,   528,   529,     0,     0,     0,     0,   534,     0,
       0,   751,     0,     0,   536,    61,     0,    62,     0,     0,
       0,     0,    63,     0,    51,   119,     0,    64,     0,    52,
      56,   120,   121,   122,   123,   124,     0,     0,   125,     0,
      53,     0,    65,     0,    49,    50,   119,   126,     0,   739,
       0,     0,   120,   121,   122,   123,   124,   752,    57,   125,
      58,     0,     0,     0,     0,     0,     0,     0,   126,     0,
     796,     0,     0,     0,     0,     0,    51,     0,    59,   753,
      60,    52,     0,    78,     0,    54,     0,     0,    55,     0,
       0,     0,    53,     0,     0,   586,     0,     0,   589,    49,
      50,     0,     0,     0,   592,   593,   594,     0,     0,     0,
       0,     0,   485,     0,     0,     0,    61,     0,    62,     0,
       0,     0,     0,    63,     0,     0,     0,    56,    64,     0,
       0,    51,     0,     0,     0,    89,    52,    54,     0,     0,
      55,     0,     0,    65,     0,     0,     0,    53,   654,     0,
       0,   657,   658,   659,   486,    57,     0,    58,   667,     0,
     669,     0,     0,     0,     0,   674,   675,   676,   677,   678,
     679,   680,   681,   682,   683,    59,   685,    60,     0,    56,
      49,    50,     0,     0,     0,   693,   694,   695,   696,     0,
      97,     0,    54,     0,     0,    55,     0,     0,     0,     0,
       0,     0,     0,   487,     0,     0,     0,    57,     0,    58,
       0,     0,    51,    61,     0,    62,     0,    52,     0,     0,
      63,     0,     0,     0,     0,    64,     0,    59,    53,    60,
       0,   488,     0,     0,    56,     0,     0,     0,     0,     0,
      65,     0,   740,     0,     0,     0,   743,   489,   119,   490,
       0,   707,   491,     0,   120,   121,   122,   123,   124,     0,
       0,   125,    57,     0,    58,    61,     0,    62,     0,   762,
     126,     0,    63,    54,   492,     0,    55,    64,     0,     0,
     119,     0,    59,     0,    60,     0,   120,   121,   122,   123,
     124,     0,    65,   125,     0,     0,     0,     0,     0,   135,
     136,   493,   126,     0,   798,   786,     0,   788,   789,   790,
     791,   792,     0,     0,     0,    56,     0,     0,     0,     0,
      61,     0,    62,     0,     0,     0,     0,    63,     0,     0,
       0,     0,    64,     0,     0,     0,     0,     0,   494,     0,
       0,     0,     0,    57,     0,    58,     0,    65,     0,     0,
       0,     0,   816,     0,     0,     0,   819,     0,     0,     0,
       0,     0,     0,    59,   826,    60,     0,   829,   830,     0,
       0,     0,   832,   834,   835,   836,     0,     0,     0,     0,
     840,   842,     0,   844,     0,   846,     0,   848,     0,   850,
     851,   852,   853,     0,     0,     0,     0,     0,     0,     0,
       0,    61,     0,   112,     0,     0,     0,     0,    63,     0,
       0,     0,     0,    64,     0,   113,     0,   119,     0,     0,
       0,   878,   879,   120,   121,   122,   123,   124,    65,     0,
     125,     0,   887,     0,     0,     0,     0,     0,     0,   126,
     119,   804,     0,   196,     0,   896,   120,   121,   122,   123,
     124,   197,   198,   125,     0,     0,     0,     0,   199,   200,
     201,   907,   126,   909,   910,   911,   912,   913,   914,     0,
       0,   915,   916,   917,   918,     0,   119,     0,     0,     0,
       0,     0,   120,   121,   122,   123,   124,     0,   926,   125,
     928,   929,     0,     0,    79,   932,   933,   934,   126,   936,
     805,   938,    90,     0,     0,   940,   941,   942,   943,    98,
       0,   946,   947,   948,   119,     0,     0,     0,     0,     0,
     120,   121,   122,   123,   124,     0,     0,   125,     0,     0,
       0,     0,   958,     0,     0,     0,   126,     0,   806,     0,
       0,     0,   119,     0,     0,     0,   969,     0,   120,   121,
     122,   123,   124,     0,     0,   125,   977,   978,   979,   980,
       0,     0,     0,     0,   126,   147,   817,   986,   987,   988,
     989,     0,   990,   991,     0,     0,   153,   993,   994,   995,
     996,     0,   997,   998,   159,    49,    50,   119,   385,     0,
    1001,     0,     0,   120,   121,   122,   123,   124,     0,     0,
     125,     0,     0,     0,     0,     0,  1010,  1011,     0,   126,
    1014,  1015,  1016,  1017,     0,  1019,  1020,    51,     0,  1023,
       0,     0,    52,     0,     0,     0,     0,     0,  1028,     0,
       0,  1030,     0,    53,     0,   226,  1034,     0,     0,     0,
       0,   231,   119,     0,     0,     0,     0,   339,   120,   121,
     122,   123,   124,     0,     0,   125,     0,     0,   351,     4,
     353,     0,     0,     0,   126,     0,   888,     0,     0,   362,
       0,     0,     0,     0,     0,     0,   119,     0,    54,     0,
       0,    55,   120,   121,   122,   123,   124,     0,     0,   125,
       0,     5,     0,     0,     6,     0,     7,     0,   126,     0,
    -360,     0,     0,     8,     0,     9,     0,     0,     0,     0,
       0,     0,   406,     0,    10,     0,     0,   410,     0,     0,
      56,   413,    11,   306,   417,   418,   419,     0,     0,     4,
       0,     0,    12,     0,     0,   428,     0,     0,     0,     0,
      13,     0,     0,     0,    14,     0,     0,     0,    57,   442,
      58,     0,    15,     0,     0,     0,   307,   308,     0,     0,
       0,     5,     0,     0,     6,     0,     7,     0,    59,     0,
      60,    16,    17,     8,     0,     9,    18,     0,     0,     0,
      19,    20,     0,     0,    10,     0,     0,     0,     0,     0,
       0,    21,    11,   452,     0,     0,     0,   458,   459,     0,
       0,   462,    12,     0,     0,     0,    61,     0,    62,     0,
      13,     0,     0,    63,    14,     0,     0,     0,    64,     0,
       0,   119,    15,     0,   794,   481,     0,   120,   121,   122,
     123,   124,    22,    65,   125,     0,    23,    24,     0,     0,
       0,    16,    17,   126,     0,    25,    18,     0,     0,     0,
      19,    20,     0,     0,    26,     0,     0,     0,    27,     0,
       0,    21,     0,     0,     0,   309,   310,   119,   311,     0,
     312,   313,     0,   120,   121,   122,   123,   124,    28,     0,
     125,     0,     0,     0,   530,     0,     0,   533,     0,   126,
     535,   899,     0,     0,     0,   537,     0,     0,     0,   747,
       0,     0,    22,     0,     0,     0,    23,    24,   131,     0,
       4,   134,     0,     0,     0,    25,     0,   556,   135,   136,
     314,     0,     0,     0,    26,     0,     0,     0,    27,   315,
     316,   119,   317,   318,     0,     0,     0,   120,   121,   122,
     123,   124,     5,   319,   125,     6,     0,     7,    28,     0,
       0,     0,     0,   126,     8,   900,     9,     0,     0,     0,
       0,     0,     0,     0,     0,    10,     0,     0,   538,   583,
       0,     0,     0,    11,     0,     0,   587,   588,     0,   590,
     591,     0,     0,    12,     0,     0,     0,     0,     0,     0,
       0,    13,   119,     0,     0,    14,     0,     0,   120,   121,
     122,   123,   124,    15,     0,   125,     0,     0,     0,   644,
     645,   646,     0,     0,   126,     0,   901,     0,     0,     0,
       0,     0,    16,    17,     0,     0,     0,    18,     0,     0,
     656,    19,    20,     0,   660,   661,     0,   119,     0,   668,
       0,   670,    21,   120,   121,   122,   123,   124,     0,     0,
     125,     0,     0,     0,     0,   684,     0,     0,     0,   126,
       0,   922,     0,     0,   692,     0,     0,   119,     0,   697,
       0,     0,     0,   120,   121,   122,   123,   124,     0,     0,
     125,     0,     0,    22,     0,     0,     0,    23,    24,   126,
       0,  1007,     0,   119,     0,     0,    25,     0,     0,   120,
     121,   122,   123,   124,     0,    26,   125,     0,     0,    27,
       0,     0,     0,     0,     0,   126,     0,  1026,     0,     0,
       0,     0,     0,     0,   741,   742,   232,   744,     0,    28,
     233,     0,   234,   119,     0,     0,     0,   235,   236,   120,
     121,   122,   123,   124,   868,   237,   125,     0,     0,     0,
       0,     0,   238,     0,     0,   126,     0,     0,   556,   770,
       6,   119,     7,   239,     0,     0,     0,   120,   121,   122,
     123,   124,   197,   198,   125,     0,     0,     0,     0,   199,
     200,   201,     0,   126,     0,     0,     0,     0,     0,     0,
       0,   240,   793,     0,   241,     0,   242,     0,     0,   243,
     244,   119,  -361,     0,   245,     0,     0,   120,   121,   122,
     123,   124,     0,   246,   125,     0,     0,     0,    15,   119,
       0,     0,   955,   126,     0,   120,   121,   122,   123,   124,
       0,     0,   125,     0,     0,     0,   247,   820,   821,   822,
     823,   126,     0,   119,   248,   249,   828,     0,   250,   120,
     121,   122,   123,   124,     0,     0,   125,     0,   251,   252,
       0,     0,     0,     0,     0,   126,     0,   253,   849,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   254,   255,   256,   257,   258,   259,     0,
       0,     0,   260,   261,     0,     0,     0,     0,    22,     0,
       0,   262,     0,     0,     0,     0,     0,     0,   263,     0,
       0,     0,   264,     0,     0,     0,     0,     0,     0,     0,
      26,     0,     0,   265,   556,     0,   897,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   919,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   927,
       0,     0,   930,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     945,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   964,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     981,     0,     0,     0,     0,     0,   985,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1013
};

static const yytype_int16 yycheck[] =
{
       4,    29,   156,     7,    76,   671,     2,   320,   188,    73,
     257,    15,   388,    68,   131,   132,   254,    21,    22,   257,
     156,     7,   257,   242,   243,    87,   255,   257,    14,    15,
     168,   278,   170,    29,   253,   254,   891,   256,     0,   258,
     278,   260,   258,   278,   256,   100,   198,    33,   278,   278,
     112,   113,   270,   271,   198,   127,   128,    61,    62,    63,
      64,    65,   280,   254,   216,    33,   257,    53,   254,    37,
      74,   101,    76,   168,    78,   170,    44,   258,   173,   109,
       2,   267,   937,    87,   222,    89,   258,   278,   258,   944,
     162,   163,   168,    97,   170,   262,   256,   101,    84,   168,
     267,   170,    51,   107,   108,   109,   161,    29,   112,   113,
     258,    97,   258,   267,     3,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   220,   221,   222,   256,    18,
     256,   267,   256,    22,   447,   199,   512,   513,   124,   256,
     202,   203,   210,   256,   148,   213,   222,   203,   204,   278,
     154,   278,   269,   222,   158,    44,   160,   278,   162,   163,
     278,    50,   448,   449,   259,   258,   320,   171,   257,   173,
     168,   837,   170,   237,   278,   173,   131,   132,   182,   254,
     399,   270,   271,   259,   320,   211,   212,    76,   254,   193,
     259,   280,   278,   197,   198,   199,   200,   201,   202,   203,
     204,   205,   256,   171,     7,     8,     9,    10,   176,   345,
       3,    14,    15,   258,   258,   183,   526,   527,   258,   258,
     258,   225,   220,   221,   222,    18,   230,   195,   278,    22,
     234,   235,   278,   237,   238,   239,   240,   254,   242,   243,
     226,   245,   131,    30,   248,   249,   257,   278,   252,   253,
     254,    44,   256,   278,   258,   278,   260,    50,   262,   270,
     271,   279,   254,    25,   259,   154,   278,    64,   340,   341,
     262,   263,   264,   259,   259,   267,    79,   278,   260,   261,
     262,    84,    40,    76,   276,   267,   572,   573,   574,   575,
     576,   259,   258,   447,    97,   278,    58,    59,    70,   102,
     103,   278,   306,   307,   252,   278,   310,   311,   278,    96,
     314,   447,   260,   261,   262,   118,   119,   278,   105,   267,
      70,   108,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   278,   278,   278,   338,   278,   340,   341,   131,   126,
     278,   252,   145,   146,   278,   278,   257,   236,   278,   260,
     261,   262,   274,   125,   508,   509,   267,   511,   278,   278,
     132,   154,   165,   278,   368,   278,   278,   371,   372,   141,
     259,   278,   278,   278,   278,   125,   278,   278,   278,   278,
     278,   409,   132,   278,   278,   521,   278,   278,   278,   256,
     278,   141,   278,   397,   398,   399,   400,   184,   278,   403,
     278,   278,     5,    70,   408,   167,   168,   278,   170,   278,
     172,   173,   278,   409,   201,   278,   203,   204,   278,   278,
     278,   278,   278,   226,    27,    28,   430,   278,   278,   278,
      33,    34,   204,   278,   278,   278,   278,   278,   278,   278,
     212,   213,   278,   236,   216,   278,   278,   250,   210,   221,
     278,   213,    55,   278,   204,   278,   259,   278,   220,   221,
     222,   278,   212,   213,   278,   132,   216,   278,   278,   231,
     232,   221,   234,   235,   141,   267,   278,   278,   482,   278,
      83,   485,   278,   245,   278,   489,   490,   409,   492,   493,
     278,   233,   278,   278,   278,   278,    70,   259,   634,   503,
     278,   505,   278,   278,   278,   278,   510,   278,   278,   278,
     514,   278,   115,   278,   278,   278,   255,   120,   522,   523,
     524,   278,   278,   278,   127,   254,   448,   449,   257,   278,
     133,   260,   261,   262,   263,   264,   538,   204,   267,   543,
     143,   278,   538,   547,   548,   212,   213,   276,   552,   216,
     554,   154,   155,   156,   221,   278,   278,   278,   132,   278,
     163,   278,   278,   278,   568,   278,   278,   141,     7,     8,
       9,    10,   278,   577,   278,    14,    15,   278,   582,   278,
     734,   735,   278,   737,   278,   739,   278,   278,   278,   278,
     278,   194,   278,   278,   598,   278,   278,   601,   602,   603,
     604,   278,   606,   607,   608,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   538,   797,   278,   278,
     223,   224,   278,   651,   278,   278,   255,   278,   550,   278,
     204,   278,   278,   637,   638,   278,   640,   278,   212,   213,
      79,   278,   216,   278,   278,    84,   650,   221,   278,   255,
     572,   573,   574,   575,   576,   651,   259,   278,    97,   813,
     278,   278,   278,   102,   103,   254,   278,   671,   672,   673,
     278,   260,   261,   262,   263,   264,   267,   335,   267,   118,
     119,   155,   185,   520,   387,   738,   517,   276,   736,   254,
     824,   748,   281,   664,   394,   260,   261,   262,   263,   264,
     265,   705,   267,    -1,   714,   709,   145,   146,    -1,    -1,
      -1,   276,    -1,   278,   718,   719,   720,   721,   722,    -1,
     748,    -1,   876,    -1,    -1,    -1,   165,    -1,    -1,   651,
      -1,    -1,    -1,    -1,   738,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   666,    -1,   750,   751,   752,   753,
     254,    -1,   748,   257,   758,   759,   260,   261,   262,   263,
     264,   765,    -1,   267,    -1,    -1,    -1,    -1,    -1,   773,
     924,   775,   276,   777,   278,   779,   254,   781,    -1,   257,
      -1,     5,   260,   261,   262,   263,   264,   226,    -1,   267,
      -1,    -1,   796,    -1,   798,    -1,    -1,    -1,   276,    -1,
     804,   805,   806,    27,    28,    -1,    -1,    -1,    -1,    33,
      34,   250,    -1,   817,   254,     8,     9,    10,    11,    12,
     260,   261,   262,   263,   264,    -1,   748,   267,    -1,    -1,
      23,    55,    -1,   837,    27,   839,   276,    -1,    -1,   279,
     254,    -1,    -1,   257,    -1,    -1,   260,   261,   262,   263,
     264,   855,    -1,   267,    -1,    -1,    -1,    -1,    -1,    83,
      -1,    -1,   276,   867,   278,    -1,   254,   871,    -1,    -1,
      -1,    -1,   260,   261,   262,   263,   264,   265,   882,   267,
      -1,   885,    -1,    -1,    -1,    -1,    -1,   891,   276,    -1,
     278,   115,    -1,    -1,    -1,    -1,   120,    -1,    -1,   903,
      -1,    -1,    -1,   127,    23,    -1,    -1,    -1,    -1,   133,
      -1,    -1,    -1,    -1,    33,    34,    -1,    -1,   922,   143,
      -1,    -1,    41,    -1,    -1,    -1,    -1,   931,   254,    -1,
     154,   155,   156,   937,   260,   261,   262,   263,   264,   163,
     944,   267,    -1,    -1,    -1,    -1,    65,    -1,    -1,   953,
     276,    70,   278,    -1,    -1,   959,   149,   150,    -1,    -1,
      -1,    -1,    81,    -1,   968,    -1,    -1,    -1,    87,    -1,
     194,    -1,    -1,   166,   167,   168,   169,   170,    -1,   172,
      -1,   174,   175,   176,   177,   178,   179,   180,   181,    -1,
     183,    -1,    -1,   186,    33,    34,    -1,    -1,  1002,   223,
     224,    -1,    -1,  1007,    -1,   254,    -1,   126,    -1,    -1,
     129,   260,   261,   262,   263,   264,    -1,    -1,   267,    -1,
      -1,  1025,  1026,  1027,    -1,   144,    65,   276,    -1,   278,
      -1,    70,    -1,    -1,    -1,   259,    -1,    -1,    -1,   232,
     233,    -1,    81,   236,    -1,   164,    -1,    -1,   241,   168,
      -1,   244,    -1,   246,   247,    -1,    -1,    -1,   251,    -1,
      -1,    -1,   255,    -1,   257,    -1,   259,    -1,   261,    -1,
     263,   264,   265,    -1,    -1,    -1,   254,   196,    -1,   198,
      -1,    -1,   260,   261,   262,   263,   264,   126,    -1,   267,
     129,    -1,   131,   132,    -1,    33,    34,   216,   276,   218,
     278,   140,   141,    -1,    -1,    -1,    -1,    -1,    46,    -1,
      -1,    -1,    -1,    -1,    -1,   308,    -1,    -1,    -1,   312,
     313,    -1,   315,   316,   317,   318,   319,    65,    -1,   168,
      -1,    -1,    70,    -1,    -1,   254,    -1,   256,    -1,    -1,
      -1,    -1,   261,    81,    -1,   254,    -1,   266,    -1,    -1,
      -1,   260,   261,   262,   263,   264,    -1,   196,   267,   198,
      -1,    -1,   281,    -1,   203,   204,    -1,   276,    -1,   278,
      -1,   210,   211,   212,   213,    -1,    -1,   216,    -1,   218,
      -1,   220,   221,    -1,    -1,    -1,    -1,    -1,   126,    -1,
      -1,   129,    33,    34,    -1,    -1,   389,    -1,    -1,    -1,
      -1,    -1,   395,   396,    -1,    -1,    -1,    -1,   401,    -1,
      -1,   149,    -1,    -1,   407,   254,    -1,   256,    -1,    -1,
      -1,    -1,   261,    -1,    65,   254,    -1,   266,    -1,    70,
     168,   260,   261,   262,   263,   264,    -1,    -1,   267,    -1,
      81,    -1,   281,    -1,    33,    34,   254,   276,    -1,   278,
      -1,    -1,   260,   261,   262,   263,   264,   195,   196,   267,
     198,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   276,    -1,
     278,    -1,    -1,    -1,    -1,    -1,    65,    -1,   216,   217,
     218,    70,    -1,   124,    -1,   126,    -1,    -1,   129,    -1,
      -1,    -1,    81,    -1,    -1,   488,    -1,    -1,   491,    33,
      34,    -1,    -1,    -1,   497,   498,   499,    -1,    -1,    -1,
      -1,    -1,    33,    -1,    -1,    -1,   254,    -1,   256,    -1,
      -1,    -1,    -1,   261,    -1,    -1,    -1,   168,   266,    -1,
      -1,    65,    -1,    -1,    -1,   124,    70,   126,    -1,    -1,
     129,    -1,    -1,   281,    -1,    -1,    -1,    81,   541,    -1,
      -1,   544,   545,   546,    75,   196,    -1,   198,   551,    -1,
     553,    -1,    -1,    -1,    -1,   558,   559,   560,   561,   562,
     563,   564,   565,   566,   567,   216,   569,   218,    -1,   168,
      33,    34,    -1,    -1,    -1,   578,   579,   580,   581,    -1,
     124,    -1,   126,    -1,    -1,   129,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   124,    -1,    -1,    -1,   196,    -1,   198,
      -1,    -1,    65,   254,    -1,   256,    -1,    70,    -1,    -1,
     261,    -1,    -1,    -1,    -1,   266,    -1,   216,    81,   218,
      -1,   152,    -1,    -1,   168,    -1,    -1,    -1,    -1,    -1,
     281,    -1,   635,    -1,    -1,    -1,   639,   168,   254,   170,
      -1,   257,   173,    -1,   260,   261,   262,   263,   264,    -1,
      -1,   267,   196,    -1,   198,   254,    -1,   256,    -1,   662,
     276,    -1,   261,   126,   195,    -1,   129,   266,    -1,    -1,
     254,    -1,   216,    -1,   218,    -1,   260,   261,   262,   263,
     264,    -1,   281,   267,    -1,    -1,    -1,    -1,    -1,   220,
     221,   222,   276,    -1,   278,   698,    -1,   700,   701,   702,
     703,   704,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     254,    -1,   256,    -1,    -1,    -1,    -1,   261,    -1,    -1,
      -1,    -1,   266,    -1,    -1,    -1,    -1,    -1,   259,    -1,
      -1,    -1,    -1,   196,    -1,   198,    -1,   281,    -1,    -1,
      -1,    -1,   745,    -1,    -1,    -1,   749,    -1,    -1,    -1,
      -1,    -1,    -1,   216,   757,   218,    -1,   760,   761,    -1,
      -1,    -1,   765,   766,   767,   768,    -1,    -1,    -1,    -1,
     773,   774,    -1,   776,    -1,   778,    -1,   780,    -1,   782,
     783,   784,   785,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   254,    -1,   256,    -1,    -1,    -1,    -1,   261,    -1,
      -1,    -1,    -1,   266,    -1,   268,    -1,   254,    -1,    -1,
      -1,   814,   815,   260,   261,   262,   263,   264,   281,    -1,
     267,    -1,   825,    -1,    -1,    -1,    -1,    -1,    -1,   276,
     254,   278,    -1,   257,    -1,   838,   260,   261,   262,   263,
     264,   265,   266,   267,    -1,    -1,    -1,    -1,   272,   273,
     274,   854,   276,   856,   857,   858,   859,   860,   861,    -1,
      -1,   864,   865,   866,   867,    -1,   254,    -1,    -1,    -1,
      -1,    -1,   260,   261,   262,   263,   264,    -1,   881,   267,
     883,   884,    -1,    -1,     7,   888,   889,   890,   276,   892,
     278,   894,    15,    -1,    -1,   898,   899,   900,   901,    22,
      -1,   904,   905,   906,   254,    -1,    -1,    -1,    -1,    -1,
     260,   261,   262,   263,   264,    -1,    -1,   267,    -1,    -1,
      -1,    -1,   925,    -1,    -1,    -1,   276,    -1,   278,    -1,
      -1,    -1,   254,    -1,    -1,    -1,   939,    -1,   260,   261,
     262,   263,   264,    -1,    -1,   267,   949,   950,   951,   952,
      -1,    -1,    -1,    -1,   276,    78,   278,   960,   961,   962,
     963,    -1,   965,   966,    -1,    -1,    89,   970,   971,   972,
     973,    -1,   975,   976,    97,    33,    34,   254,   255,    -1,
     983,    -1,    -1,   260,   261,   262,   263,   264,    -1,    -1,
     267,    -1,    -1,    -1,    -1,    -1,   999,  1000,    -1,   276,
    1003,  1004,  1005,  1006,    -1,  1008,  1009,    65,    -1,  1012,
      -1,    -1,    70,    -1,    -1,    -1,    -1,    -1,  1021,    -1,
      -1,  1024,    -1,    81,    -1,   148,  1029,    -1,    -1,    -1,
      -1,   154,   254,    -1,    -1,    -1,    -1,   160,   260,   261,
     262,   263,   264,    -1,    -1,   267,    -1,    -1,   171,    19,
     173,    -1,    -1,    -1,   276,    -1,   278,    -1,    -1,   182,
      -1,    -1,    -1,    -1,    -1,    -1,   254,    -1,   126,    -1,
      -1,   129,   260,   261,   262,   263,   264,    -1,    -1,   267,
      -1,    51,    -1,    -1,    54,    -1,    56,    -1,   276,    -1,
     278,    -1,    -1,    63,    -1,    65,    -1,    -1,    -1,    -1,
      -1,    -1,   225,    -1,    74,    -1,    -1,   230,    -1,    -1,
     168,   234,    82,    25,   237,   238,   239,    -1,    -1,    19,
      -1,    -1,    92,    -1,    -1,   248,    -1,    -1,    -1,    -1,
     100,    -1,    -1,    -1,   104,    -1,    -1,    -1,   196,   262,
     198,    -1,   112,    -1,    -1,    -1,    58,    59,    -1,    -1,
      -1,    51,    -1,    -1,    54,    -1,    56,    -1,   216,    -1,
     218,   131,   132,    63,    -1,    65,   136,    -1,    -1,    -1,
     140,   141,    -1,    -1,    74,    -1,    -1,    -1,    -1,    -1,
      -1,   151,    82,   306,    -1,    -1,    -1,   310,   311,    -1,
      -1,   314,    92,    -1,    -1,    -1,   254,    -1,   256,    -1,
     100,    -1,    -1,   261,   104,    -1,    -1,    -1,   266,    -1,
      -1,   254,   112,    -1,   257,   338,    -1,   260,   261,   262,
     263,   264,   192,   281,   267,    -1,   196,   197,    -1,    -1,
      -1,   131,   132,   276,    -1,   205,   136,    -1,    -1,    -1,
     140,   141,    -1,    -1,   214,    -1,    -1,    -1,   218,    -1,
      -1,   151,    -1,    -1,    -1,   167,   168,   254,   170,    -1,
     172,   173,    -1,   260,   261,   262,   263,   264,   238,    -1,
     267,    -1,    -1,    -1,   397,    -1,    -1,   400,    -1,   276,
     403,   278,    -1,    -1,    -1,   408,    -1,    -1,    -1,   259,
      -1,    -1,   192,    -1,    -1,    -1,   196,   197,   210,    -1,
      19,   213,    -1,    -1,    -1,   205,    -1,   430,   220,   221,
     222,    -1,    -1,    -1,   214,    -1,    -1,    -1,   218,   231,
     232,   254,   234,   235,    -1,    -1,    -1,   260,   261,   262,
     263,   264,    51,   245,   267,    54,    -1,    56,   238,    -1,
      -1,    -1,    -1,   276,    63,   278,    65,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    -1,    -1,   258,   482,
      -1,    -1,    -1,    82,    -1,    -1,   489,   490,    -1,   492,
     493,    -1,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   100,   254,    -1,    -1,   104,    -1,    -1,   260,   261,
     262,   263,   264,   112,    -1,   267,    -1,    -1,    -1,   522,
     523,   524,    -1,    -1,   276,    -1,   278,    -1,    -1,    -1,
      -1,    -1,   131,   132,    -1,    -1,    -1,   136,    -1,    -1,
     543,   140,   141,    -1,   547,   548,    -1,   254,    -1,   552,
      -1,   554,   151,   260,   261,   262,   263,   264,    -1,    -1,
     267,    -1,    -1,    -1,    -1,   568,    -1,    -1,    -1,   276,
      -1,   278,    -1,    -1,   577,    -1,    -1,   254,    -1,   582,
      -1,    -1,    -1,   260,   261,   262,   263,   264,    -1,    -1,
     267,    -1,    -1,   192,    -1,    -1,    -1,   196,   197,   276,
      -1,   278,    -1,   254,    -1,    -1,   205,    -1,    -1,   260,
     261,   262,   263,   264,    -1,   214,   267,    -1,    -1,   218,
      -1,    -1,    -1,    -1,    -1,   276,    -1,   278,    -1,    -1,
      -1,    -1,    -1,    -1,   637,   638,    20,   640,    -1,   238,
      24,    -1,    26,   254,    -1,    -1,    -1,    31,    32,   260,
     261,   262,   263,   264,   265,    39,   267,    -1,    -1,    -1,
      -1,    -1,    46,    -1,    -1,   276,    -1,    -1,   671,   672,
      54,   254,    56,    57,    -1,    -1,    -1,   260,   261,   262,
     263,   264,   265,   266,   267,    -1,    -1,    -1,    -1,   272,
     273,   274,    -1,   276,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    85,   705,    -1,    88,    -1,    90,    -1,    -1,    93,
      94,   254,   255,    -1,    98,    -1,    -1,   260,   261,   262,
     263,   264,    -1,   107,   267,    -1,    -1,    -1,   112,   254,
      -1,    -1,   257,   276,    -1,   260,   261,   262,   263,   264,
      -1,    -1,   267,    -1,    -1,    -1,   130,   750,   751,   752,
     753,   276,    -1,   254,   138,   139,   759,    -1,   142,   260,
     261,   262,   263,   264,    -1,    -1,   267,    -1,   152,   153,
      -1,    -1,    -1,    -1,    -1,   276,    -1,   161,   781,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   177,   178,   179,   180,   181,   182,    -1,
      -1,    -1,   186,   187,    -1,    -1,    -1,    -1,   192,    -1,
      -1,   195,    -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     214,    -1,    -1,   217,   837,    -1,   839,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   867,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   882,
      -1,    -1,   885,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     903,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   931,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     953,    -1,    -1,    -1,    -1,    -1,   959,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,  1002
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   283,   284,     0,    19,    51,    54,    56,    63,    65,
      74,    82,    92,   100,   104,   112,   131,   132,   136,   140,
     141,   151,   192,   196,   197,   205,   214,   218,   238,   285,
     286,   287,   290,   293,   295,   306,   314,   315,   318,   319,
     404,   405,   406,   407,   408,   409,   410,   411,   415,    33,
      34,    65,    70,    81,   126,   129,   168,   196,   198,   216,
     218,   254,   256,   261,   266,   281,   399,    70,   125,   132,
     141,   204,   212,   213,   216,   221,   288,   258,   124,   396,
     399,   395,   399,   395,   395,   395,   395,   256,   198,   124,
     396,   258,   258,   198,   216,   258,   399,   124,   396,   395,
      51,   256,   258,   395,   258,   286,   256,   256,   256,   256,
     399,   400,   256,   268,   399,   401,   399,   399,   399,   254,
     260,   261,   262,   263,   264,   267,   276,   216,   288,   203,
     204,   210,   211,   212,   213,   220,   221,   289,   293,   295,
     322,   329,   334,   340,   399,   289,   317,   396,   278,   278,
     278,   399,   401,   396,   278,   296,   297,   294,   278,   396,
     278,   125,   216,   288,   400,   316,     7,     8,     9,    10,
      14,    15,    79,    84,    97,   102,   103,   118,   119,   145,
     146,   165,   226,   250,   304,   305,   254,   402,   403,   399,
     399,   400,   255,   278,   401,   401,   257,   265,   266,   272,
     273,   274,   270,   271,   280,   278,   281,   399,   399,   399,
     399,   399,   399,   399,   399,   289,   289,   258,   258,   258,
     256,   258,   258,   258,   313,   278,   396,   395,   395,   257,
     278,   396,    20,    24,    26,    31,    32,    39,    46,    57,
      85,    88,    90,    93,    94,    98,   107,   130,   138,   139,
     142,   152,   153,   161,   177,   178,   179,   180,   181,   182,
     186,   187,   195,   202,   206,   217,   302,   303,   315,   344,
     345,   349,   350,   351,   352,   354,   355,   356,   357,   358,
     359,   361,   362,   363,   364,   366,   367,   368,   369,   370,
     372,   374,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   390,   392,   394,    25,    58,    59,   167,
     168,   170,   172,   173,   222,   231,   232,   234,   235,   245,
     298,   299,   300,   301,   334,   340,     3,    18,    22,    44,
      50,    76,   131,   154,   236,   291,   292,   399,   278,   396,
     216,   288,   289,   289,   257,   311,   395,   395,   395,   395,
     395,   396,   395,   396,   395,   395,   395,   395,   395,   395,
     395,   395,   396,   395,   259,   304,   395,   257,   278,   402,
     257,   278,   278,   257,   399,   257,   399,   399,   399,   399,
     399,   401,   401,   399,   399,   255,   323,   324,   335,   254,
     325,   326,   336,   341,   342,     7,    14,    15,    33,    53,
      84,    97,   124,   226,   259,   312,   396,   278,   278,   416,
     396,   395,   395,   396,   397,   399,   395,   396,   396,   396,
     399,   395,   397,   397,   395,   399,   395,   395,   396,   399,
     375,   395,   399,   397,   397,   395,   397,   395,   397,   395,
     397,   395,   396,   395,   395,   395,   259,   298,   256,   269,
     295,   353,   396,   399,   395,    73,   199,   237,   396,   396,
     395,   395,   396,   395,   395,   395,   395,   395,   259,   299,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   259,
     292,   396,   278,   289,   289,    33,    75,   124,   152,   168,
     170,   173,   195,   222,   259,   309,   340,   278,   278,   278,
     399,   399,   399,   279,   265,   278,   321,   321,    30,    96,
     105,   108,   126,   184,   201,   322,   332,   333,   395,   257,
     278,   330,   168,   170,   222,   338,   339,   339,   395,   395,
     396,   399,   397,   396,   395,   396,   395,   396,   258,   286,
     412,   278,   279,   278,   278,   278,   278,   278,   278,   391,
     278,   278,   278,   278,   278,   373,   396,   278,   278,   278,
     278,   278,   278,   278,   278,   278,   278,   278,   278,   278,
     353,   353,   252,   260,   261,   262,   267,   278,   278,   278,
     278,   278,   278,   396,   399,   310,   395,   396,   396,   395,
     396,   396,   395,   395,   395,   257,   257,   257,   278,   399,
     399,     5,    27,    28,    33,    34,    55,    83,   115,   120,
     127,   133,   143,   154,   155,   156,   163,   194,   223,   224,
     259,   320,   259,   334,   334,   399,   334,   337,   322,   322,
     327,   328,   399,   259,   330,   278,   325,   168,   170,   173,
     222,   259,   331,   340,   396,   396,   396,   259,   338,   259,
     278,   285,    64,   413,   395,   346,   396,   395,   395,   395,
     396,   396,    40,   387,   389,   295,   360,   395,   396,   395,
     396,   278,   278,   371,   395,   395,   395,   395,   395,   395,
     395,   395,   395,   395,   396,   395,   257,   353,   353,   353,
     353,   353,   396,   395,   395,   395,   395,   396,   278,   258,
     278,   278,   278,   278,   278,   278,   399,   257,   265,   278,
     399,   399,   399,   399,   256,   399,   399,   399,    23,    41,
      87,   144,   164,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   278,   278,   278,   278,   278,   278,
     395,   396,   396,   395,   396,   278,   399,   259,   414,   278,
      46,   149,   195,   217,   347,   348,   395,   278,   278,   278,
     278,   278,   395,   387,   295,   278,   278,   278,   278,   373,
     396,   396,   398,   278,   278,   278,   278,   278,   278,   278,
     278,   278,   278,   278,   278,   278,   395,   307,   395,   395,
     395,   395,   395,   396,   257,   399,   278,   403,   278,   399,
     399,   399,   399,   399,   278,   278,   278,   334,   334,   329,
     334,   327,   334,   278,   278,   278,   395,   278,   412,   395,
     396,   396,   396,   396,   278,   278,   395,   399,   396,   395,
     395,   278,   395,   399,   395,   395,   395,   278,   233,   278,
     395,   399,   395,   399,   395,   399,   395,   399,   395,   396,
     395,   395,   395,   395,   278,    33,    37,    44,   171,   176,
     183,   195,   259,   308,   278,   278,   278,   278,   265,   399,
     257,   278,   399,   399,   399,   399,   278,   334,   395,   395,
     399,   278,   278,   278,   278,   278,   348,   395,   278,   278,
     278,   388,   278,   278,   278,   373,   395,   396,   278,   278,
     278,   278,   278,   278,   278,   278,   278,   395,   399,   395,
     395,   395,   395,   395,   395,   395,   395,   395,   395,   396,
     399,   399,   278,   334,   278,   278,   395,   396,   395,   395,
     396,   278,   395,   395,   395,   398,   395,   365,   395,   278,
     395,   395,   395,   395,   393,   396,   395,   395,   395,   278,
     278,   278,   278,   278,   255,   257,   399,   334,   395,   278,
     278,   278,   278,   278,   396,   278,   278,   398,   278,   395,
     278,   278,   278,   278,   398,   278,   278,   395,   395,   395,
     395,   396,   255,   278,   343,   396,   395,   395,   395,   395,
     395,   395,   399,   395,   395,   395,   395,   395,   395,   278,
     278,   395,   278,   278,   278,   278,   278,   278,   278,   278,
     395,   395,   278,   396,   395,   395,   395,   395,   399,   395,
     395,   278,   255,   395,   278,   278,   278,   278,   395,   278,
     395,   399,   399,   399,   395
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   282,   284,   283,   285,   285,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     287,   288,   288,   288,   288,   288,   288,   288,   289,   289,
     289,   289,   289,   289,   289,   290,   290,   290,   290,   290,
     290,   290,   290,   291,   291,   292,   292,   292,   292,   292,
     292,   292,   292,   292,   294,   293,   293,   296,   295,   295,
     297,   295,   298,   298,   299,   299,   299,   299,   299,   299,
     299,   299,   299,   299,   299,   299,   299,   300,   300,   300,
     300,   300,   300,   301,   301,   301,   302,   302,   303,   303,
     303,   303,   303,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   303,   303,   303,   303,   303,   303,   303,
     303,   303,   303,   303,   304,   304,   304,   304,   304,   304,
     304,   304,   304,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   305,   305,   306,   307,   307,   308,   308,   308,
     308,   308,   308,   308,   309,   309,   309,   309,   310,   309,
     309,   309,   309,   309,   309,   309,   311,   311,   312,   312,
     312,   312,   312,   312,   312,   312,   312,   313,   313,   314,
     315,   315,   315,   315,   315,   315,   315,   315,   316,   315,
     315,   315,   315,   315,   317,   315,   318,   318,   319,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   320,
     320,   320,   320,   320,   320,   320,   320,   320,   320,   321,
     321,   323,   322,   322,   324,   322,   325,   326,   326,   327,
     328,   328,   329,   329,   330,   330,   331,   331,   331,   331,
     331,   331,   332,   333,   333,   333,   333,   333,   333,   333,
     333,   335,   334,   334,   336,   334,   337,   337,   338,   338,
     338,   338,   339,   339,   341,   340,   340,   342,   340,   343,
     343,   344,   346,   345,   347,   347,   348,   348,   348,   348,
     348,   349,   350,   352,   351,   353,   353,   353,   353,   353,
     353,   353,   353,   354,   355,   355,   356,   356,   357,   357,
     358,   359,   360,   360,   361,   362,   362,   363,   365,   364,
     366,   367,   367,   368,   369,   371,   370,   372,   373,   373,
     375,   374,   376,   376,   377,   378,   378,   379,   379,   380,
     380,   381,   381,   382,   382,   383,   384,   384,   385,   385,
     386,   388,   387,   389,   389,   391,   390,   393,   392,   394,
     395,   396,   397,   398,   398,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   400,   400,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   402,   402,
     403,   403,   404,   404,   404,   404,   405,   406,   407,   408,
     409,   409,   410,   411,   412,   412,   414,   413,   413,   416,
     415
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     4,     3,     4,     4,
       5,     4,     5,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     0,     5,     1,     0,     5,     1,
       0,     5,     1,     2,     1,     1,     2,     4,    12,     2,
       2,     1,     2,     2,     4,     1,     2,     4,     6,     2,
       2,     2,     8,     2,     2,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     8,     2,     2,     2,     2,     4,
       2,     2,     1,     2,     4,     2,     0,     2,     2,     2,
       2,     4,     2,     2,     2,     4,     8,     1,     0,     5,
       1,     2,     4,    12,     2,     2,     2,     0,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     0,     6,
       4,     2,     5,     3,     4,    12,     5,    13,     0,     5,
       2,     4,     3,     5,     0,     5,    10,     4,     2,     2,
       6,     4,     4,     2,     2,     2,     4,     2,     2,     2,
       3,     3,     3,     3,     3,     2,     2,     2,     2,     2,
       2,     2,     4,     2,     4,     2,     6,     4,     2,     2,
       0,     0,     5,     1,     0,     5,     9,     1,     3,     3,
       1,     3,     1,     4,     2,     0,     1,     2,     4,    12,
       2,     2,     2,     1,     2,     2,     4,     6,     2,     4,
       2,     0,     5,     1,     0,     5,     1,     3,     2,     4,
       2,     2,     1,     2,     0,     5,     1,     0,     5,     3,
       1,    10,     0,     5,     1,     3,     5,     6,     8,     8,
      10,     4,     8,     0,     2,     3,     3,     3,     3,     2,
       3,     3,     1,     6,     2,     6,     6,    10,     6,     8,
       2,     4,     1,     2,     2,    14,     6,     2,     0,     9,
       1,    14,    10,     6,     2,     0,     5,     2,     3,     7,
       0,     7,     2,     4,     2,    14,     6,     2,     6,     6,
      10,     2,     6,     6,    10,     4,     2,     6,     6,    10,
       4,     0,     5,     1,     2,     0,     4,     0,     9,     8,
       1,     1,     1,     1,     3,     3,     3,     5,     7,     9,
       4,     7,     3,     3,     3,     3,     3,     3,     3,     2,
       3,     6,     4,     4,     6,     6,     8,     1,     1,     1,
       1,     1,     4,     1,     1,     1,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     2,     9,    13,
       1,     2,     1,     1,     1,     1,     2,     2,     2,     2,
       2,     2,     2,     4,     3,     1,     0,     3,     0,     0,
       7
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* $@1: %empty  */
#line 400 "polyray.y"
     { condition_flags[0] = 1; }
#line 2629 "ytab.c"
    break;

  case 14: /* element: light  */
#line 419 "polyray.y"
     { ACTION(Add_To_Lights((yyvsp[0].lgt));) }
#line 2635 "ytab.c"
    break;

  case 16: /* element: object  */
#line 422 "polyray.y"
     { ACTION(Add_To_BinTree(&Root, (yyvsp[0].obj));) }
#line 2641 "ytab.c"
    break;

  case 19: /* element: particle  */
#line 426 "polyray.y"
     { ACTION(InsertParticle((yyvsp[0].part));) }
#line 2647 "ytab.c"
    break;

  case 20: /* include_statement: INCLUDE STRING  */
#line 431 "polyray.y"
      { //printf("doing ACTION include_file action to %s\n",yyvsp[0].name);
		  /*ACTION(include_file_action((char *)POLYRAY_PATH_STRING, (yyvsp[0].name));)*/
		   extern void start_include(char *name);
		  ACTION(start_include(yyvsp[0].name);)
        /*polyray_free((yyvsp[0].name));*/ }
#line 2654 "ytab.c"
    break;

  case 21: /* defined_token: SURFACE_SYM  */
#line 437 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2660 "ytab.c"
    break;

  case 22: /* defined_token: TEXTURE_SYM  */
#line 439 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2666 "ytab.c"
    break;

  case 23: /* defined_token: TEXTURE_MAP_SYM  */
#line 441 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2672 "ytab.c"
    break;

  case 24: /* defined_token: OBJECT_SYM  */
#line 443 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2678 "ytab.c"
    break;

  case 25: /* defined_token: EXPRESSION_SYM  */
#line 445 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2684 "ytab.c"
    break;

  case 26: /* defined_token: TRANSFORM_SYM  */
#line 447 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2690 "ytab.c"
    break;

  case 27: /* defined_token: PARTICLE_SYM  */
#line 449 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2696 "ytab.c"
    break;

  case 28: /* definition_types: surface  */
#line 454 "polyray.y"
      { ACTION(temp_def.type = T_SURFACE;
               temp_def.data = (yyvsp[0].surf);) }
#line 2703 "ytab.c"
    break;

  case 29: /* definition_types: texture  */
#line 457 "polyray.y"
      { ACTION(temp_def.type = T_TEXTURE;
               temp_def.data = (yyvsp[0].text);) }
#line 2710 "ytab.c"
    break;

  case 30: /* definition_types: texture_map  */
#line 460 "polyray.y"
      { ACTION(temp_def.type = T_TEXTURE_MAP;
               temp_def.data = (yyvsp[0].text_map);) }
#line 2717 "ytab.c"
    break;

  case 31: /* definition_types: object  */
#line 463 "polyray.y"
      { ACTION(temp_def.type = T_OBJECT;
               temp_def.data = (yyvsp[0].obj);) }
#line 2724 "ytab.c"
    break;

  case 32: /* definition_types: transform  */
#line 466 "polyray.y"
      { ACTION(temp_def.type = T_TRANSFORM;
               temp_def.data = (yyvsp[0].trns);) }
#line 2731 "ytab.c"
    break;

  case 33: /* definition_types: expression  */
#line 469 "polyray.y"
      { ACTION(temp_def.type = T_EXPRESSION;
               temp_def.data = (yyvsp[0].exper);) }
#line 2738 "ytab.c"
    break;

  case 34: /* definition_types: particle  */
#line 472 "polyray.y"
      { ACTION(temp_def.type = T_PARTICLE;
               temp_def.data = (yyvsp[0].part);) }
#line 2745 "ytab.c"
    break;

  case 35: /* definition: DEFINE defined_token definition_types  */
#line 478 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 0);) }
#line 2752 "ytab.c"
    break;

  case 36: /* definition: STATIC DEFINE defined_token definition_types  */
#line 481 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 0);) }
#line 2759 "ytab.c"
    break;

  case 37: /* definition: DEFINE TOKEN definition_types  */
#line 484 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 0);)
        polyray_free((yyvsp[-1].name)); }
#line 2767 "ytab.c"
    break;

  case 38: /* definition: STATIC DEFINE TOKEN definition_types  */
#line 488 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 0);)
        polyray_free((yyvsp[-1].name)); }
#line 2775 "ytab.c"
    break;

  case 39: /* definition: DEFINE NOEVAL defined_token definition_types  */
#line 492 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 1);) }
#line 2782 "ytab.c"
    break;

  case 40: /* definition: STATIC DEFINE NOEVAL defined_token definition_types  */
#line 495 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 1);) }
#line 2789 "ytab.c"
    break;

  case 41: /* definition: DEFINE NOEVAL TOKEN definition_types  */
#line 498 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 1);)
        polyray_free((yyvsp[-1].name)); }
#line 2797 "ytab.c"
    break;

  case 42: /* definition: STATIC DEFINE NOEVAL TOKEN definition_types  */
#line 502 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 1);)
        polyray_free((yyvsp[-1].name)); }
#line 2805 "ytab.c"
    break;

  case 45: /* particle_decl: BIRTH expression  */
#line 514 "polyray.y"
      { ACTION(SetParticleBirth(CurrentParticle, (yyvsp[0].exper));) }
#line 2811 "ytab.c"
    break;

  case 46: /* particle_decl: DEATH expression  */
#line 516 "polyray.y"
      { ACTION(SetParticleDeath(CurrentParticle, (yyvsp[0].exper));) }
#line 2817 "ytab.c"
    break;

  case 47: /* particle_decl: POSITION expression  */
#line 518 "polyray.y"
      { ACTION(SetParticleP(CurrentParticle, (yyvsp[0].exper));) }
#line 2823 "ytab.c"
    break;

  case 48: /* particle_decl: VELOCITY expression  */
#line 520 "polyray.y"
      { ACTION(SetParticleV(CurrentParticle, (yyvsp[0].exper));) }
#line 2829 "ytab.c"
    break;

  case 49: /* particle_decl: ACCELERATION expression  */
#line 522 "polyray.y"
      { ACTION(SetParticleA(CurrentParticle, (yyvsp[0].exper));) }
#line 2835 "ytab.c"
    break;

  case 50: /* particle_decl: AVOID expression  */
#line 524 "polyray.y"
      { ACTION(SetParticleAvoid(CurrentParticle, (yyvsp[0].exper));) }
#line 2841 "ytab.c"
    break;

  case 51: /* particle_decl: FLOCK expression  */
#line 526 "polyray.y"
      { ACTION(SetParticleFlock(CurrentParticle, (yyvsp[0].exper));) }
#line 2847 "ytab.c"
    break;

  case 52: /* particle_decl: COUNT expression  */
#line 528 "polyray.y"
      { ACTION(SetParticleCount(CurrentParticle, (yyvsp[0].exper));) }
#line 2853 "ytab.c"
    break;

  case 53: /* particle_decl: OBJECT expression  */
#line 530 "polyray.y"
      { ACTION(SetParticleObjName(CurrentParticle, (yyvsp[0].exper));) }
#line 2859 "ytab.c"
    break;

  case 54: /* $@2: %empty  */
#line 535 "polyray.y"
     { ACTION(CurrentParticle = CreateParticle();) }
#line 2865 "ytab.c"
    break;

  case 55: /* particle: PARTICLE '{' $@2 particle_decls '}'  */
#line 537 "polyray.y"
     { ACTION((yyval.part) = CurrentParticle;) }
#line 2871 "ytab.c"
    break;

  case 56: /* particle: PARTICLE_SYM  */
#line 539 "polyray.y"
      { ACTION((yyval.part) = CopyParticle((yyvsp[0].name));) }
#line 2877 "ytab.c"
    break;

  case 57: /* $@3: %empty  */
#line 544 "polyray.y"
      { ACTION(Object_Stack = push_object(Object_Stack, object_action1());) }
#line 2883 "ytab.c"
    break;

  case 58: /* object: OBJECT '{' $@3 object_decls '}'  */
#line 546 "polyray.y"
      { ACTION((yyval.obj) = pop_object(&Object_Stack);) }
#line 2889 "ytab.c"
    break;

  case 59: /* object: OBJECT_SYM  */
#line 548 "polyray.y"
      { ACTION((yyval.obj) = object_action2((yyvsp[0].name));) }
#line 2895 "ytab.c"
    break;

  case 60: /* $@4: %empty  */
#line 550 "polyray.y"
      { ACTION(Object_Stack =
               push_object(Object_Stack, object_action2((yyvsp[-1].name)));) }
#line 2902 "ytab.c"
    break;

  case 61: /* object: OBJECT_SYM '{' $@4 object_modifier_decls '}'  */
#line 553 "polyray.y"
      { ACTION((yyval.obj) = pop_object(&Object_Stack);) }
#line 2908 "ytab.c"
    break;

  case 64: /* object_modifier_decl: texture  */
#line 563 "polyray.y"
      { ACTION(if (Object_Stack->element->o_texture != NULL)
                        TextureDelete(Object_Stack->element->o_texture);
                     Object_Stack->element->o_texture = (yyvsp[0].text);) }
#line 2916 "ytab.c"
    break;

  case 65: /* object_modifier_decl: transform  */
#line 567 "polyray.y"
      { ACTION(TransformObject(Object_Stack->element, (yyvsp[0].trns));
               polyray_free((yyvsp[0].trns));) }
#line 2923 "ytab.c"
    break;

  case 66: /* object_modifier_decl: ROTATE point  */
#line 570 "polyray.y"
      { ACTION(RotateObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2929 "ytab.c"
    break;

  case 67: /* object_modifier_decl: ROTATE point ',' fexper  */
#line 572 "polyray.y"
      { ACTION(RotateAxisObject(Object_Stack->element, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 2935 "ytab.c"
    break;

  case 68: /* object_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 575 "polyray.y"
      { ACTION(ShearObject(Object_Stack->element, (yyvsp[-10].flt), (yyvsp[-8].flt),
                           (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 2942 "ytab.c"
    break;

  case 69: /* object_modifier_decl: TRANSLATE point  */
#line 578 "polyray.y"
      { ACTION(TranslateObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2948 "ytab.c"
    break;

  case 70: /* object_modifier_decl: SCALE point  */
#line 580 "polyray.y"
      { ACTION(ScaleObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2954 "ytab.c"
    break;

  case 72: /* object_modifier_decl: SHADING_FLAGS fexper  */
#line 583 "polyray.y"
      { ACTION(Object_Stack->element->o_sflag = (int)(yyvsp[0].flt);) }
#line 2960 "ytab.c"
    break;

  case 73: /* object_modifier_decl: DITHER fexper  */
#line 585 "polyray.y"
      { ACTION(Object_Stack->element->o_dither = (yyvsp[0].flt);) }
#line 2966 "ytab.c"
    break;

  case 74: /* object_modifier_decl: BOUNDING_BOX point ',' point  */
#line 587 "polyray.y"
     { ACTION(VecCopy((yyvsp[-2].vec), Object_Stack->element->o_bnd.lower_left);
              VecCopy((yyvsp[0].vec), Object_Stack->element->o_bnd.lengths);
              VecSub(Object_Stack->element->o_bnd.lengths,
                     Object_Stack->element->o_bnd.lower_left,
                     Object_Stack->element->o_bnd.lengths);) }
#line 2976 "ytab.c"
    break;

  case 76: /* object_modifier_decl: DISPLACE expression  */
#line 594 "polyray.y"
      { ACTION(Object_Stack->element->o_displace = (yyvsp[0].exper);) }
#line 2982 "ytab.c"
    break;

  case 77: /* uv_information: UV_STEPS fexper ',' fexper  */
#line 599 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[-2].flt);
               Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 2991 "ytab.c"
    break;

  case 78: /* uv_information: UV_STEPS fexper ',' fexper ',' fexper  */
#line 604 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[-4].flt);
               Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[-2].flt);
               Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3000 "ytab.c"
    break;

  case 79: /* uv_information: U_STEPS fexper  */
#line 609 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3007 "ytab.c"
    break;

  case 80: /* uv_information: V_STEPS fexper  */
#line 612 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3014 "ytab.c"
    break;

  case 81: /* uv_information: W_STEPS fexper  */
#line 615 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3021 "ytab.c"
    break;

  case 82: /* uv_information: UV_BOUNDS fexper ',' fexper ',' fexper ',' fexper  */
#line 618 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_bounds[0] = (yyvsp[-6].flt);
               Object_Stack->element->o_uv_bounds[1] = (yyvsp[-4].flt);
               Object_Stack->element->o_uv_bounds[2] = (yyvsp[-2].flt);
               Object_Stack->element->o_uv_bounds[3] = (yyvsp[0].flt);) }
#line 3030 "ytab.c"
    break;

  case 83: /* root_solver: ROOT_SOLVER FERRARI  */
#line 626 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 0);) }
#line 3036 "ytab.c"
    break;

  case 84: /* root_solver: ROOT_SOLVER VIETA  */
#line 628 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 1);) }
#line 3042 "ytab.c"
    break;

  case 85: /* root_solver: ROOT_SOLVER STURM  */
#line 630 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 2);) }
#line 3048 "ytab.c"
    break;

  case 124: /* camera_exper: ANGLE fexper  */
#line 679 "polyray.y"
     { ACTION(Eye.view_angle = degtorad((yyvsp[0].flt)/2.0 );) }
#line 3054 "ytab.c"
    break;

  case 125: /* camera_exper: ANTIALIAS fexper  */
#line 681 "polyray.y"
     { ACTION(antialias = (int)(yyvsp[0].flt);
              if (antialias < 0 || antialias > 4)
                 error("Antialias value of %d is not between 0 and 4",
                       antialias);)}
#line 3063 "ytab.c"
    break;

  case 126: /* camera_exper: ANTIALIAS_THRESHOLD fexper  */
#line 686 "polyray.y"
     { ACTION(antialias_threshold = (yyvsp[0].flt);) }
#line 3069 "ytab.c"
    break;

  case 127: /* camera_exper: APERTURE fexper  */
#line 688 "polyray.y"
     { ACTION(Eye.view_aperture = (yyvsp[0].flt);) }
#line 3075 "ytab.c"
    break;

  case 128: /* camera_exper: ASPECT fexper  */
#line 690 "polyray.y"
     { ACTION(Eye.view_aspect = (yyvsp[0].flt);) }
#line 3081 "ytab.c"
    break;

  case 129: /* camera_exper: AT point  */
#line 692 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), Eye.view_at);) }
#line 3087 "ytab.c"
    break;

  case 130: /* camera_exper: FOCAL_DISTANCE fexper  */
#line 694 "polyray.y"
     { ACTION(Eye.view_focaldist = (yyvsp[0].flt);) }
#line 3093 "ytab.c"
    break;

  case 131: /* camera_exper: FROM point  */
#line 696 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), Eye.view_from);) }
#line 3099 "ytab.c"
    break;

  case 132: /* camera_exper: HITHER fexper  */
#line 698 "polyray.y"
     { ACTION(Eye.view_hither = (yyvsp[0].flt);) }
#line 3105 "ytab.c"
    break;

  case 133: /* camera_exper: IMAGE_FORMAT fexper  */
#line 700 "polyray.y"
     { ACTION(if ((int)((yyvsp[0].flt)) == 0)
                 DepthRender = 0;
              else if ((int)((yyvsp[0].flt)) == 1) {
                 pixel_encoding = 0;
                 DepthRender = 1;
                 }
              else
                 error("image_format must be either 0 (normal) or 1 (depth)");
              ) }
#line 3119 "ytab.c"
    break;

  case 134: /* camera_exper: IMAGE_WINDOW fexper ',' fexper ',' fexper ',' fexper  */
#line 710 "polyray.y"
     { ACTION(Eye.view_x0 = (int) (yyvsp[-6].flt);
              Eye.view_y0 = (int) (yyvsp[-4].flt);
              Eye.view_xl = (int) (yyvsp[-2].flt);
              Eye.view_yl = (int) (yyvsp[0].flt);) }
#line 3128 "ytab.c"
    break;

  case 135: /* camera_exper: MAX_SAMPLES fexper  */
#line 715 "polyray.y"
     { ACTION(maxsamples = (int)(yyvsp[0].flt);
              if (maxsamples < 0)
                 error("maxsamples must be greater than 0");)}
#line 3136 "ytab.c"
    break;

  case 136: /* camera_exper: MAX_TRACE_DEPTH fexper  */
#line 719 "polyray.y"
     { ACTION(maxlevel = (int)(yyvsp[0].flt);
              if (maxlevel < 1 || maxlevel > 63)
                 error("maxlevel must be between 1 and 63");)}
#line 3144 "ytab.c"
    break;

  case 137: /* camera_exper: PIXEL_ENCODING fexper  */
#line 723 "polyray.y"
     { ACTION(pixel_encoding = (int)(yyvsp[0].flt);
              if (pixel_encoding != 0 && pixel_encoding != 1)
                 error("Pixel encoding of %d is not one of: 0 [none], 1 [RLE]",
                       pixel_encoding);) }
#line 3153 "ytab.c"
    break;

  case 138: /* camera_exper: PIXELSIZE fexper  */
#line 728 "polyray.y"
     { ACTION(pixelsize = (int)(yyvsp[0].flt);
              if (pixelsize != 8 && pixelsize != 16 &&
                  pixelsize != 24 && pixelsize != 32)
                 error("Pixelsize of %d is not one of: 8, 16, 24, 32",
                       pixelsize);) }
#line 3163 "ytab.c"
    break;

  case 139: /* camera_exper: RESOLUTION fexper ',' fexper  */
#line 734 "polyray.y"
     { ACTION(Eye.view_xres = (int) (yyvsp[-2].flt);
              Eye.view_yres = (int) (yyvsp[0].flt);) }
#line 3170 "ytab.c"
    break;

  case 140: /* camera_exper: UP point  */
#line 737 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), Eye.view_up);) }
#line 3176 "ytab.c"
    break;

  case 141: /* camera_exper: YON fexper  */
#line 739 "polyray.y"
     { ACTION(Eye.view_yon = (yyvsp[0].flt);) }
#line 3182 "ytab.c"
    break;

  case 147: /* flare_option: COLOR expression  */
#line 758 "polyray.y"
      { ACTION(Set_Flare_Color((yyvsp[0].exper));) }
#line 3188 "ytab.c"
    break;

  case 148: /* flare_option: COUNT fexper  */
#line 760 "polyray.y"
      { ACTION(Set_Flare_Count((yyvsp[0].flt));) }
#line 3194 "ytab.c"
    break;

  case 149: /* flare_option: SPACING fexper  */
#line 762 "polyray.y"
      { ACTION(Set_Flare_Spacing((yyvsp[0].flt));) }
#line 3200 "ytab.c"
    break;

  case 150: /* flare_option: SEED fexper  */
#line 764 "polyray.y"
      { ACTION(Set_Flare_Seed((int)(yyvsp[0].flt));) }
#line 3206 "ytab.c"
    break;

  case 151: /* flare_option: SIZE fexper ',' fexper  */
#line 766 "polyray.y"
      { ACTION(Set_Flare_Size((yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3212 "ytab.c"
    break;

  case 152: /* flare_option: CONCAVE fexper  */
#line 768 "polyray.y"
      { ACTION(Set_Flare_Concave((yyvsp[0].flt));) }
#line 3218 "ytab.c"
    break;

  case 153: /* flare_option: SPHERE fexper  */
#line 770 "polyray.y"
      { ACTION(Set_Flare_Sphere((yyvsp[0].flt));) }
#line 3224 "ytab.c"
    break;

  case 154: /* light_modifier_decl: COLOR expression  */
#line 775 "polyray.y"
      { ACTION(Set_Light_Color((yyvsp[0].exper));) }
#line 3230 "ytab.c"
    break;

  case 155: /* light_modifier_decl: SPHERE point ',' fexper  */
#line 777 "polyray.y"
      { ACTION(Translate_Light((yyvsp[-2].vec));
               Set_Light_Radius((yyvsp[0].flt));) }
#line 3237 "ytab.c"
    break;

  case 156: /* light_modifier_decl: POLYGON fexper ',' fexper ',' fexper ',' fexper  */
#line 780 "polyray.y"
      { ACTION(Set_Light_Polygon((yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3243 "ytab.c"
    break;

  case 157: /* light_modifier_decl: NO_SHADOW  */
#line 782 "polyray.y"
      { ACTION(Set_Light_Shadow(0);) }
#line 3249 "ytab.c"
    break;

  case 158: /* $@5: %empty  */
#line 784 "polyray.y"
      { ACTION(Create_Lens_Flare();) }
#line 3255 "ytab.c"
    break;

  case 160: /* light_modifier_decl: transform  */
#line 787 "polyray.y"
      { ACTION(Transform_Light((yyvsp[0].trns));
               polyray_free((yyvsp[0].trns));) }
#line 3262 "ytab.c"
    break;

  case 161: /* light_modifier_decl: ROTATE point  */
#line 790 "polyray.y"
      { ACTION(Rotate_Light((yyvsp[0].vec));) }
#line 3268 "ytab.c"
    break;

  case 162: /* light_modifier_decl: ROTATE point ',' fexper  */
#line 792 "polyray.y"
      { ACTION(Rotate_Axis_Light((yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3274 "ytab.c"
    break;

  case 163: /* light_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 795 "polyray.y"
      { ACTION(Shear_Light((yyvsp[-10].flt), (yyvsp[-8].flt), (yyvsp[-6].flt),
                           (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3281 "ytab.c"
    break;

  case 164: /* light_modifier_decl: TRANSLATE point  */
#line 798 "polyray.y"
      { ACTION(Translate_Light((yyvsp[0].vec));) }
#line 3287 "ytab.c"
    break;

  case 165: /* light_modifier_decl: SCALE point  */
#line 800 "polyray.y"
      { ACTION(Scale_Light((yyvsp[0].vec));) }
#line 3293 "ytab.c"
    break;

  case 168: /* depth_light_modifier: ANGLE fexper  */
#line 810 "polyray.y"
      { ACTION(DepthLight1((yyvsp[0].flt));) }
#line 3299 "ytab.c"
    break;

  case 169: /* depth_light_modifier: ASPECT fexper  */
#line 812 "polyray.y"
      { ACTION(DepthLight2((yyvsp[0].flt));) }
#line 3305 "ytab.c"
    break;

  case 170: /* depth_light_modifier: AT point  */
#line 814 "polyray.y"
      { ACTION(DepthLight3((yyvsp[0].vec));) }
#line 3311 "ytab.c"
    break;

  case 171: /* depth_light_modifier: COLOR expression  */
#line 816 "polyray.y"
      { ACTION(DepthLight4((yyvsp[0].exper));) }
#line 3317 "ytab.c"
    break;

  case 172: /* depth_light_modifier: DEPTH sexper  */
#line 818 "polyray.y"
      { ACTION(DepthLight5((yyvsp[0].name));
               polyray_free((yyvsp[0].name));) }
#line 3324 "ytab.c"
    break;

  case 173: /* depth_light_modifier: FROM point  */
#line 821 "polyray.y"
      { ACTION(DepthLight6((yyvsp[0].vec));) }
#line 3330 "ytab.c"
    break;

  case 174: /* depth_light_modifier: HITHER fexper  */
#line 823 "polyray.y"
     { ACTION(DepthLight9((yyvsp[0].flt));) }
#line 3336 "ytab.c"
    break;

  case 175: /* depth_light_modifier: UP point  */
#line 825 "polyray.y"
      { ACTION(DepthLight7((yyvsp[0].vec));) }
#line 3342 "ytab.c"
    break;

  case 176: /* depth_light_modifier: NO_SHADOW  */
#line 827 "polyray.y"
      { ACTION(Set_Light_Shadow(0);) }
#line 3348 "ytab.c"
    break;

  case 179: /* haze_statement: HAZE fexper ',' fexper ',' point  */
#line 837 "polyray.y"
     { ACTION(haze_action((yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].vec));) }
#line 3354 "ytab.c"
    break;

  case 180: /* light: LIGHT point ',' point  */
#line 842 "polyray.y"
     { ACTION((yyval.lgt) = light_action1((yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 3360 "ytab.c"
    break;

  case 181: /* light: LIGHT point  */
#line 844 "polyray.y"
     { ACTION((yyval.lgt) = light_action2((yyvsp[0].vec));) }
#line 3366 "ytab.c"
    break;

  case 182: /* light: LIGHT NO_SHADOW point ',' point  */
#line 846 "polyray.y"
     { ACTION((yyval.lgt) = light_action1((yyvsp[-2].vec), (yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3373 "ytab.c"
    break;

  case 183: /* light: LIGHT NO_SHADOW point  */
#line 849 "polyray.y"
     { ACTION((yyval.lgt) = light_action2((yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3380 "ytab.c"
    break;

  case 184: /* light: SPOT_LIGHT point ',' point  */
#line 852 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight(White, (yyvsp[-2].vec), (yyvsp[0].vec), 10.0, 30, 45);) }
#line 3386 "ytab.c"
    break;

  case 185: /* light: SPOT_LIGHT point ',' point ',' point ',' fexper ',' fexper ',' fexper  */
#line 854 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight((yyvsp[-10].vec), (yyvsp[-8].vec), (yyvsp[-6].vec), (yyvsp[-4].flt),
                           (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3393 "ytab.c"
    break;

  case 186: /* light: SPOT_LIGHT NO_SHADOW point ',' point  */
#line 857 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight(White, (yyvsp[-2].vec), (yyvsp[0].vec), 10.0, 30, 45);
              Set_Light_Shadow(0);) }
#line 3400 "ytab.c"
    break;

  case 187: /* light: SPOT_LIGHT NO_SHADOW point ',' point ',' point ',' fexper ',' fexper ',' fexper  */
#line 861 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight((yyvsp[-10].vec), (yyvsp[-8].vec), (yyvsp[-6].vec), (yyvsp[-4].flt),
                           (yyvsp[-2].flt), (yyvsp[0].flt));
              Set_Light_Shadow(0);) }
#line 3408 "ytab.c"
    break;

  case 188: /* @6: %empty  */
#line 865 "polyray.y"
     { ACTION((yyval.lgt) = light_action3();) }
#line 3414 "ytab.c"
    break;

  case 189: /* light: TEXTURED_LIGHT '{' @6 light_modifier_decls '}'  */
#line 867 "polyray.y"
     { ACTION((yyval.lgt) = Current_Light;) }
#line 3420 "ytab.c"
    break;

  case 190: /* light: DIRECTIONAL_LIGHT point  */
#line 869 "polyray.y"
     { ACTION((yyval.lgt) = light_action4((yyvsp[0].vec));) }
#line 3426 "ytab.c"
    break;

  case 191: /* light: DIRECTIONAL_LIGHT point ',' point  */
#line 871 "polyray.y"
     { ACTION((yyval.lgt) = light_action5((yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 3432 "ytab.c"
    break;

  case 192: /* light: DIRECTIONAL_LIGHT NO_SHADOW point  */
#line 873 "polyray.y"
     { ACTION((yyval.lgt) = light_action4((yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3439 "ytab.c"
    break;

  case 193: /* light: DIRECTIONAL_LIGHT NO_SHADOW point ',' point  */
#line 876 "polyray.y"
     { ACTION((yyval.lgt) = light_action5((yyvsp[-2].vec), (yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3446 "ytab.c"
    break;

  case 194: /* @7: %empty  */
#line 879 "polyray.y"
     { ACTION((yyval.lgt) = light_action6();) }
#line 3452 "ytab.c"
    break;

  case 195: /* light: DEPTHMAPPED_LIGHT '{' @7 depth_light_modifiers '}'  */
#line 881 "polyray.y"
     { ACTION(DepthLight8();
              (yyval.lgt) = Current_Light;) }
#line 3459 "ytab.c"
    break;

  case 196: /* draw_statement: DRAW fexper ',' fexper ',' fexper ',' expression ',' expression  */
#line 887 "polyray.y"
      { ACTION(draw_action((yyvsp[-8].flt), (yyvsp[-6].flt), (int)(yyvsp[-4].flt),
                           (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3466 "ytab.c"
    break;

  case 197: /* draw_statement: POINT expression ',' expression  */
#line 890 "polyray.y"
      { ACTION(draw_action(0.0, 0.0, 0, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3472 "ytab.c"
    break;

  case 198: /* background: BACKGROUND expression  */
#line 895 "polyray.y"
     { ACTION(background_action((yyvsp[0].exper));) }
#line 3478 "ytab.c"
    break;

  case 199: /* surface_declaration: COLOR expression  */
#line 900 "polyray.y"
      { ACTION(color_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3484 "ytab.c"
    break;

  case 200: /* surface_declaration: COLOR_MAP '(' map_entries ',' expression ')'  */
#line 902 "polyray.y"
      { ACTION(color_map_action(CurrentSurface, (yyvsp[-3].cmap_entry), (yyvsp[-1].exper));) }
#line 3490 "ytab.c"
    break;

  case 201: /* surface_declaration: COLOR_MAP '(' map_entries ')'  */
#line 904 "polyray.y"
      { ACTION(color_map_action(CurrentSurface, (yyvsp[-1].cmap_entry), NULL);) }
#line 3496 "ytab.c"
    break;

  case 202: /* surface_declaration: AMBIENT expression ',' expression  */
#line 906 "polyray.y"
      { ACTION(ambient_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3502 "ytab.c"
    break;

  case 203: /* surface_declaration: AMBIENT expression  */
#line 908 "polyray.y"
      { ACTION(ambient_action(CurrentSurface, NULL, (yyvsp[0].exper));) }
#line 3508 "ytab.c"
    break;

  case 204: /* surface_declaration: BRILLIANCE expression  */
#line 910 "polyray.y"
      { ACTION(brilliance_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3514 "ytab.c"
    break;

  case 205: /* surface_declaration: BUMP_SCALE expression  */
#line 912 "polyray.y"
      { ACTION(bump_scale_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3520 "ytab.c"
    break;

  case 206: /* surface_declaration: DIFFUSE expression ',' expression  */
#line 914 "polyray.y"
      { ACTION(diffuse_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3526 "ytab.c"
    break;

  case 207: /* surface_declaration: DIFFUSE expression  */
#line 916 "polyray.y"
      { ACTION(diffuse_action(CurrentSurface, NULL, (yyvsp[0].exper));) }
#line 3532 "ytab.c"
    break;

  case 208: /* surface_declaration: FREQUENCY expression  */
#line 918 "polyray.y"
      { ACTION(frequency_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3538 "ytab.c"
    break;

  case 209: /* surface_declaration: LOOKUP_FUNCTION expression  */
#line 920 "polyray.y"
      { ACTION(lookup_function_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3544 "ytab.c"
    break;

  case 210: /* surface_declaration: MICROFACET PHONG expression  */
#line 922 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, PHONG, (yyvsp[0].exper));) }
#line 3550 "ytab.c"
    break;

  case 211: /* surface_declaration: MICROFACET BLINN expression  */
#line 924 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, BLINN, (yyvsp[0].exper));) }
#line 3556 "ytab.c"
    break;

  case 212: /* surface_declaration: MICROFACET GAUSSIAN expression  */
#line 926 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, GAUSSIAN, (yyvsp[0].exper));) }
#line 3562 "ytab.c"
    break;

  case 213: /* surface_declaration: MICROFACET REITZ expression  */
#line 928 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, REITZ, (yyvsp[0].exper));) }
#line 3568 "ytab.c"
    break;

  case 214: /* surface_declaration: MICROFACET COOK expression  */
#line 930 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, COOK, (yyvsp[0].exper));) }
#line 3574 "ytab.c"
    break;

  case 215: /* surface_declaration: MICROFACET expression  */
#line 932 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, PHONG, (yyvsp[0].exper));) }
#line 3580 "ytab.c"
    break;

  case 216: /* surface_declaration: NORMAL expression  */
#line 934 "polyray.y"
      { ACTION(normal_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3586 "ytab.c"
    break;

  case 217: /* surface_declaration: POSITION expression  */
#line 936 "polyray.y"
      { ACTION(position_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3592 "ytab.c"
    break;

  case 218: /* surface_declaration: OCTAVES expression  */
#line 938 "polyray.y"
      { ACTION(octaves_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3598 "ytab.c"
    break;

  case 219: /* surface_declaration: PHASE expression  */
#line 940 "polyray.y"
      { ACTION(phase_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3604 "ytab.c"
    break;

  case 220: /* surface_declaration: POSITION_FUNCTION expression  */
#line 942 "polyray.y"
      { ACTION(position_function_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3610 "ytab.c"
    break;

  case 221: /* surface_declaration: POSITION_SCALE expression  */
#line 944 "polyray.y"
      { ACTION(position_scale_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3616 "ytab.c"
    break;

  case 222: /* surface_declaration: REFLECTION expression ',' expression  */
#line 946 "polyray.y"
      { ACTION(reflection_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3622 "ytab.c"
    break;

  case 223: /* surface_declaration: REFLECTION expression  */
#line 948 "polyray.y"
      { ACTION(reflection_action(CurrentSurface, NULL, (yyvsp[0].exper));) }
#line 3628 "ytab.c"
    break;

  case 224: /* surface_declaration: SPECULAR expression ',' expression  */
#line 950 "polyray.y"
      { ACTION(specular_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3634 "ytab.c"
    break;

  case 225: /* surface_declaration: SPECULAR expression  */
#line 952 "polyray.y"
      { ACTION(specular_action(CurrentSurface, NULL, (yyvsp[0].exper));) }
#line 3640 "ytab.c"
    break;

  case 226: /* surface_declaration: TRANSMISSION expression ',' expression ',' expression  */
#line 954 "polyray.y"
      { ACTION(transmission_action(CurrentSurface, (yyvsp[-4].exper), (yyvsp[-2].exper),
                                   (yyvsp[0].exper));) }
#line 3647 "ytab.c"
    break;

  case 227: /* surface_declaration: TRANSMISSION expression ',' expression  */
#line 957 "polyray.y"
      { ACTION(transmission_action(CurrentSurface, NULL,
                                   (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3654 "ytab.c"
    break;

  case 228: /* surface_declaration: TURBULENCE expression  */
#line 960 "polyray.y"
      { ACTION(turbulence_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3660 "ytab.c"
    break;

  case 231: /* $@8: %empty  */
#line 970 "polyray.y"
      { ACTION(surface_action1();) }
#line 3666 "ytab.c"
    break;

  case 232: /* surface: SURFACE '{' $@8 surface_declarations '}'  */
#line 972 "polyray.y"
     { ACTION((yyval.surf) = CurrentSurface;) }
#line 3672 "ytab.c"
    break;

  case 233: /* surface: SURFACE_SYM  */
#line 974 "polyray.y"
      { ACTION(surface_action2((yyvsp[0].name)); (yyval.surf) = CurrentSurface;) }
#line 3678 "ytab.c"
    break;

  case 234: /* $@9: %empty  */
#line 976 "polyray.y"
      { ACTION(surface_action2((yyvsp[-1].name));) }
#line 3684 "ytab.c"
    break;

  case 235: /* surface: SURFACE_SYM '{' $@9 surface_declarations '}'  */
#line 978 "polyray.y"
      { ACTION((yyval.surf) = CurrentSurface;) }
#line 3690 "ytab.c"
    break;

  case 236: /* texture_map_element: '[' fexper ',' fexper ',' texture ',' texture ']'  */
#line 983 "polyray.y"
      { ACTION((yyval.text_map) =
               make_texture_map_entry((yyvsp[-7].flt), (yyvsp[-5].flt), (yyvsp[-3].text), (yyvsp[-1].text));) }
#line 3697 "ytab.c"
    break;

  case 237: /* texture_map_elements: texture_map_element  */
#line 989 "polyray.y"
      { ACTION((yyval.text_map) = (yyvsp[0].text_map);) }
#line 3703 "ytab.c"
    break;

  case 238: /* texture_map_elements: texture_map_elements ',' texture_map_element  */
#line 991 "polyray.y"
      { ACTION((yyval.text_map) =
               texture_map_action2((yyvsp[-2].text_map), (yyvsp[0].text_map));) }
#line 3710 "ytab.c"
    break;

  case 239: /* texture_fn_element: expression ',' texture  */
#line 997 "polyray.y"
      { ACTION((yyval.text_fn) = make_texture_fn_entry((yyvsp[-2].exper), (yyvsp[0].text));) }
#line 3716 "ytab.c"
    break;

  case 240: /* texture_fn_elements: texture_fn_element  */
#line 1002 "polyray.y"
      { ACTION((yyval.text_fn) = (yyvsp[0].text_fn);) }
#line 3722 "ytab.c"
    break;

  case 241: /* texture_fn_elements: texture_fn_elements ',' texture_fn_element  */
#line 1004 "polyray.y"
      { ACTION((yyval.text_fn) = texture_fn_action2((yyvsp[-2].text_fn), (yyvsp[0].text_fn));) }
#line 3728 "ytab.c"
    break;

  case 242: /* texture_map: TEXTURE_MAP_SYM  */
#line 1009 "polyray.y"
      { ACTION((yyval.text_map) = texture_map_action1((yyvsp[0].name));) }
#line 3734 "ytab.c"
    break;

  case 243: /* texture_map: TEXTURE_MAP '(' texture_map_elements ')'  */
#line 1011 "polyray.y"
      { ACTION((yyval.text_map) = (yyvsp[-1].text_map);) }
#line 3740 "ytab.c"
    break;

  case 246: /* texture_modifier_decl: transform  */
#line 1021 "polyray.y"
      { ACTION(if (Texture_Stack->element->t_trans == NULL)
                  Texture_Stack->element->t_trans = (yyvsp[0].trns);
               else {
                  Compose_Transformations(Texture_Stack->element->t_trans,
                                          (yyvsp[0].trns));
                  polyray_free((yyvsp[0].trns));
                  }) }
#line 3752 "ytab.c"
    break;

  case 247: /* texture_modifier_decl: ROTATE point  */
#line 1029 "polyray.y"
      { ACTION(TextureRotate(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3758 "ytab.c"
    break;

  case 248: /* texture_modifier_decl: ROTATE point ',' fexper  */
#line 1031 "polyray.y"
      { ACTION(TextureAxisRotate(Texture_Stack->element, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3764 "ytab.c"
    break;

  case 249: /* texture_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 1034 "polyray.y"
      { ACTION(TextureShear(Texture_Stack->element, (yyvsp[-10].flt), (yyvsp[-8].flt),
                            (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3771 "ytab.c"
    break;

  case 250: /* texture_modifier_decl: TRANSLATE point  */
#line 1037 "polyray.y"
      { ACTION(TextureTranslate(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3777 "ytab.c"
    break;

  case 251: /* texture_modifier_decl: SCALE point  */
#line 1039 "polyray.y"
      { ACTION(TextureScale(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3783 "ytab.c"
    break;

  case 253: /* texture_declaration: surface  */
#line 1048 "polyray.y"
      { ACTION(create_plain(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3789 "ytab.c"
    break;

  case 254: /* texture_declaration: SPECIAL surface  */
#line 1050 "polyray.y"
      { ACTION(create_special(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3795 "ytab.c"
    break;

  case 255: /* texture_declaration: NOISE surface  */
#line 1052 "polyray.y"
      { ACTION(create_noise(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3801 "ytab.c"
    break;

  case 256: /* texture_declaration: CHECKER texture ',' texture  */
#line 1054 "polyray.y"
      { ACTION(create_checker(Texture_Stack->element, (yyvsp[-2].text), (yyvsp[0].text));) }
#line 3807 "ytab.c"
    break;

  case 257: /* texture_declaration: HEXAGON texture ',' texture ',' texture  */
#line 1056 "polyray.y"
      { ACTION(create_hexagon(Texture_Stack->element,
                              (yyvsp[-4].text), (yyvsp[-2].text), (yyvsp[0].text));) }
#line 3814 "ytab.c"
    break;

  case 258: /* texture_declaration: LAYERED texture_list  */
#line 1059 "polyray.y"
      { ACTION(create_layered(Texture_Stack->element, (yyvsp[0].textlist));) }
#line 3820 "ytab.c"
    break;

  case 259: /* texture_declaration: INDEXED expression ',' texture_map  */
#line 1061 "polyray.y"
      { ACTION(create_indexed(Texture_Stack->element, (yyvsp[-2].exper),
                              (yyvsp[0].text_map));) }
#line 3827 "ytab.c"
    break;

  case 260: /* texture_declaration: SUMMED texture_fn_elements  */
#line 1064 "polyray.y"
      { ACTION(create_summed(Texture_Stack->element, (yyvsp[0].text_fn));) }
#line 3833 "ytab.c"
    break;

  case 261: /* $@10: %empty  */
#line 1069 "polyray.y"
      { ACTION(push_texture(texture_action1());) }
#line 3839 "ytab.c"
    break;

  case 262: /* texture: TEXTURE '{' $@10 texture_declarations '}'  */
#line 1071 "polyray.y"
      { ACTION((yyval.text) = pop_texture();) }
#line 3845 "ytab.c"
    break;

  case 263: /* texture: TEXTURE_SYM  */
#line 1073 "polyray.y"
      { ACTION((yyval.text) = texture_action2((yyvsp[0].name));) }
#line 3851 "ytab.c"
    break;

  case 264: /* $@11: %empty  */
#line 1075 "polyray.y"
      { ACTION(push_texture(texture_action2((yyvsp[-1].name)));) }
#line 3857 "ytab.c"
    break;

  case 265: /* texture: TEXTURE_SYM '{' $@11 texture_modifier_decls '}'  */
#line 1077 "polyray.y"
      { ACTION((yyval.text) = pop_texture();) }
#line 3863 "ytab.c"
    break;

  case 266: /* texture_list: texture  */
#line 1082 "polyray.y"
      { ACTION((yyval.textlist) = texture_list_action1((yyvsp[0].text));) }
#line 3869 "ytab.c"
    break;

  case 267: /* texture_list: texture_list ',' texture  */
#line 1084 "polyray.y"
      { ACTION((yyval.textlist) = texture_list_action2((yyvsp[-2].textlist), (yyvsp[0].text));) }
#line 3875 "ytab.c"
    break;

  case 268: /* transform_declaration: ROTATE point  */
#line 1089 "polyray.y"
      { ACTION(rotate_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3881 "ytab.c"
    break;

  case 269: /* transform_declaration: ROTATE point ',' fexper  */
#line 1091 "polyray.y"
      { ACTION(axis_rotate_transform(Current_Transform, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3887 "ytab.c"
    break;

  case 270: /* transform_declaration: SCALE point  */
#line 1093 "polyray.y"
      { ACTION(scale_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3893 "ytab.c"
    break;

  case 271: /* transform_declaration: TRANSLATE point  */
#line 1095 "polyray.y"
      { ACTION(translate_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3899 "ytab.c"
    break;

  case 274: /* $@12: %empty  */
#line 1105 "polyray.y"
      { ACTION(Current_Transform = transform_action1();) }
#line 3905 "ytab.c"
    break;

  case 275: /* transform: TRANSFORM '{' $@12 transform_declarations '}'  */
#line 1107 "polyray.y"
      { ACTION((yyval.trns) = Current_Transform;) }
#line 3911 "ytab.c"
    break;

  case 276: /* transform: TRANSFORM_SYM  */
#line 1109 "polyray.y"
      { ACTION((yyval.trns) = transform_action2((yyvsp[0].name));) }
#line 3917 "ytab.c"
    break;

  case 277: /* $@13: %empty  */
#line 1111 "polyray.y"
      { ACTION(Current_Transform = transform_action2((yyvsp[-1].name));) }
#line 3923 "ytab.c"
    break;

  case 278: /* transform: TRANSFORM_SYM '{' $@13 transform_declarations '}'  */
#line 1113 "polyray.y"
      { ACTION((yyval.trns) = Current_Transform;) }
#line 3929 "ytab.c"
    break;

  case 279: /* bezier_points: bezier_points ',' point  */
#line 1118 "polyray.y"
      { ACTION((yyval.vecl) = add_bezier_point((yyvsp[-2].vecl), (yyvsp[0].vec));) }
#line 3935 "ytab.c"
    break;

  case 280: /* bezier_points: point  */
#line 1120 "polyray.y"
      { ACTION((yyval.vecl) = add_bezier_point(NULL, (yyvsp[0].vec));) }
#line 3941 "ytab.c"
    break;

  case 281: /* bezier: BEZIER fexper ',' fexper ',' fexper ',' fexper ',' bezier_points  */
#line 1125 "polyray.y"
      { ACTION(MakeBezier(Object_Stack->element,
                          (int)(yyvsp[-8].flt), (yyvsp[-6].flt), (int)(yyvsp[-4].flt),
                          (int)(yyvsp[-2].flt), (yyvsp[0].vecl));) }
#line 3949 "ytab.c"
    break;

  case 282: /* $@14: %empty  */
#line 1132 "polyray.y"
     { ACTION(npoints = 0;) }
#line 3955 "ytab.c"
    break;

  case 283: /* blob: BLOB fexper ':' $@14 blobelements  */
#line 1134 "polyray.y"
     { ACTION(MakeBlob(Object_Stack->element, (yyvsp[-3].flt),
                       blob_components, npoints, 1);
              blob_components = NULL; npoints = 0;) }
#line 3963 "ytab.c"
    break;

  case 286: /* blobelement: fexper ',' fexper ',' point  */
#line 1146 "polyray.y"
      { ACTION(spherical_component_action((yyvsp[0].vec), (yyvsp[-4].flt), (yyvsp[-2].flt));) }
#line 3969 "ytab.c"
    break;

  case 287: /* blobelement: SPHERE point ',' fexper ',' fexper  */
#line 1148 "polyray.y"
      { ACTION(spherical_component_action((yyvsp[-4].vec), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3975 "ytab.c"
    break;

  case 288: /* blobelement: CYLINDER point ',' point ',' fexper ',' fexper  */
#line 1150 "polyray.y"
      { ACTION(cylindrical_component_action((yyvsp[-6].vec), (yyvsp[-4].vec),
                                            (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3982 "ytab.c"
    break;

  case 289: /* blobelement: PLANE point ',' fexper ',' fexper ',' fexper  */
#line 1153 "polyray.y"
      { ACTION(planar_component_action((yyvsp[-6].vec), (yyvsp[-4].flt),
                                       (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3989 "ytab.c"
    break;

  case 290: /* blobelement: TORUS point ',' point ',' fexper ',' fexper ',' fexper  */
#line 1156 "polyray.y"
      { ACTION(toroidal_component_action((yyvsp[-8].vec), (yyvsp[-6].vec),
                                         (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3996 "ytab.c"
    break;

  case 291: /* box: BOX point ',' point  */
#line 1162 "polyray.y"
      { ACTION(MakeBox(Object_Stack->element, (yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 4002 "ytab.c"
    break;

  case 292: /* cone: CONE point ',' fexper ',' point ',' fexper  */
#line 1167 "polyray.y"
      { ACTION(MakeCone(Object_Stack->element, (yyvsp[-6].vec),
                        (yyvsp[-4].flt), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4009 "ytab.c"
    break;

  case 293: /* $@15: %empty  */
#line 1172 "polyray.y"
      { ACTION(ObjectDepth++;) }
#line 4015 "ytab.c"
    break;

  case 294: /* csg: $@15 csg_tree  */
#line 1174 "polyray.y"
      { ACTION(ObjectDepth--;
               MakeCSG(Object_Stack->element, (yyvsp[0].csgtree));) }
#line 4022 "ytab.c"
    break;

  case 295: /* csg_tree: '(' csg_tree ')'  */
#line 1180 "polyray.y"
      { ACTION((yyval.csgtree) = (yyvsp[-1].csgtree);) }
#line 4028 "ytab.c"
    break;

  case 296: /* csg_tree: csg_tree '+' csg_tree  */
#line 1182 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_UNION, (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4035 "ytab.c"
    break;

  case 297: /* csg_tree: csg_tree '-' csg_tree  */
#line 1185 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_INTERSECTION, (yyvsp[-2].csgtree),
                                make_csg_node(T_INVERSE, (yyvsp[0].csgtree), NULL));) }
#line 4043 "ytab.c"
    break;

  case 298: /* csg_tree: csg_tree '*' csg_tree  */
#line 1189 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_INTERSECTION, (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4050 "ytab.c"
    break;

  case 299: /* csg_tree: '~' csg_tree  */
#line 1192 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_INVERSE, (yyvsp[0].csgtree), NULL);) }
#line 4057 "ytab.c"
    break;

  case 300: /* csg_tree: csg_tree '&' csg_tree  */
#line 1195 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_CLIP, (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4064 "ytab.c"
    break;

  case 301: /* csg_tree: csg_tree '^' csg_tree  */
#line 1198 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_MERGE, (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4071 "ytab.c"
    break;

  case 302: /* csg_tree: object  */
#line 1201 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(T_BASE_OBJECT, (yyvsp[0].obj), NULL);) }
#line 4078 "ytab.c"
    break;

  case 303: /* cylinder: CYLINDER point ',' point ',' fexper  */
#line 1207 "polyray.y"
      { ACTION(MakeCylinder(Object_Stack->element,
                            (yyvsp[-4].vec), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4085 "ytab.c"
    break;

  case 304: /* cylindrical_height_field: CHEIGHT_FIELD sexper  */
#line 1213 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[0].name), 0,
                             1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4093 "ytab.c"
    break;

  case 305: /* cylindrical_height_field: CHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1217 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[-4].name), 0,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4101 "ytab.c"
    break;

  case 306: /* cylindrical_height_fn: CHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1224 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 0, 1.0, 128.0);) }
#line 4108 "ytab.c"
    break;

  case 307: /* cylindrical_height_fn: CHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1228 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 0, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4115 "ytab.c"
    break;

  case 308: /* disc: DISC point ',' point ',' fexper  */
#line 1234 "polyray.y"
      { ACTION(MakeDisc(Object_Stack->element,
                        (yyvsp[-4].vec), (yyvsp[-2].vec), 0.0, (yyvsp[0].flt));) }
#line 4122 "ytab.c"
    break;

  case 309: /* disc: DISC point ',' point ',' fexper ',' fexper  */
#line 1237 "polyray.y"
      { ACTION(MakeDisc(Object_Stack->element,
                        (yyvsp[-6].vec), (yyvsp[-4].vec), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4129 "ytab.c"
    break;

  case 310: /* function: FUNCTION expression  */
#line 1243 "polyray.y"
      { ACTION(MakeFunction(Object_Stack->element, (yyvsp[0].exper));) }
#line 4135 "ytab.c"
    break;

  case 311: /* gridded: GRIDDED sexper ',' object_list  */
#line 1248 "polyray.y"
      { ACTION(MakeGrid(Object_Stack->element, (yyvsp[-2].name), (yyvsp[0].objlist));
        polyray_free((yyvsp[-2].name));) }
#line 4142 "ytab.c"
    break;

  case 312: /* object_list: object  */
#line 1254 "polyray.y"
      { ACTION(ostackptr ost =
                     polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == NULL) error("Failed to allocate grid object");
                ost->element = (yyvsp[0].obj);
                ost->next    = NULL;
                (yyval.objlist)  = ost;) }
#line 4153 "ytab.c"
    break;

  case 313: /* object_list: object_list object  */
#line 1261 "polyray.y"
      { ACTION(ostackptr ost =
                     polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == NULL) error("Failed to allocate grid object");
                ost->element = (yyvsp[0].obj);
                ost->next    = (yyvsp[-1].objlist);
                (yyval.objlist)  = ost;) }
#line 4164 "ytab.c"
    break;

  case 314: /* height_field: HEIGHT_FIELD sexper  */
#line 1271 "polyray.y"
      { ACTION(MakeHeight(Object_Stack->element, (yyvsp[0].name), 0);
        polyray_free((yyvsp[0].name));) }
#line 4171 "ytab.c"
    break;

  case 315: /* height_fn: HEIGHT_FN fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1279 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                            (yyvsp[-8].flt), (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt),
                            (yyvsp[0].exper), 0);) }
#line 4179 "ytab.c"
    break;

  case 316: /* height_fn: HEIGHT_FN fexper ',' fexper ',' expression  */
#line 1283 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt),
                            0.0, 1.0, 0.0, 1.0,
                            (yyvsp[0].exper), 0);) }
#line 4187 "ytab.c"
    break;

  case 317: /* hypertexture: HYPERTEXTURE expression  */
#line 1290 "polyray.y"
      { ACTION(MakeHypertexture(Object_Stack->element, (yyvsp[0].exper));) }
#line 4193 "ytab.c"
    break;

  case 318: /* $@16: %empty  */
#line 1295 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec));
            if (plist == NULL) error("Failed to allocate lathe data\n");
            pl = plist;) }
#line 4202 "ytab.c"
    break;

  case 319: /* lathe: LATHE fexper ',' point ',' fexper ',' $@16 pointlist  */
#line 1300 "polyray.y"
   { ACTION(MakeRevolve(Object_Stack->element, (int)(yyvsp[-7].flt),
                        (yyvsp[-5].vec), (int)(yyvsp[-3].flt), plist);) }
#line 4209 "ytab.c"
    break;

  case 320: /* light_object: light  */
#line 1306 "polyray.y"
   { ACTION(MakeLight(Object_Stack->element, (yyvsp[0].lgt));) }
#line 4215 "ytab.c"
    break;

  case 321: /* nurb: NURB fexper ',' fexper ',' fexper ',' fexper ',' expression ',' expression ',' expression  */
#line 1312 "polyray.y"
   { ACTION(MakeNurb(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                     (int)(yyvsp[-8].flt), (int)(yyvsp[-6].flt), (yyvsp[-4].exper), (yyvsp[-2].exper),
                     (yyvsp[0].exper));) }
#line 4223 "ytab.c"
    break;

  case 322: /* nurb: NURB fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1317 "polyray.y"
   { ACTION(MakeNurb(Object_Stack->element, (int)(yyvsp[-8].flt), (int)(yyvsp[-6].flt),
                     (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt), NULL, NULL,
                     (yyvsp[0].exper));) }
#line 4231 "ytab.c"
    break;

  case 323: /* parabola: PARABOLA point ',' point ',' fexper  */
#line 1324 "polyray.y"
      { ACTION(MakeParabola(Object_Stack->element,
                            (yyvsp[-4].vec), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4238 "ytab.c"
    break;

  case 324: /* parametric: PARAMETRIC expression  */
#line 1330 "polyray.y"
      { ACTION(MakeParametric(Object_Stack->element, (yyvsp[0].exper));) }
#line 4244 "ytab.c"
    break;

  case 325: /* $@17: %empty  */
#line 1335 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            if (npoints < 3)
               error("polygons must have at least 3 sides\n");
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec)) ;
            if (plist == NULL) error("Failed to allocate polygon data\n");
            pl = plist;) }
#line 4255 "ytab.c"
    break;

  case 326: /* polygon: POLYGON fexper ',' $@17 pointlist  */
#line 1342 "polyray.y"
   { ACTION(MakePoly(Object_Stack->element, (int)(yyvsp[-3].flt), plist);) }
#line 4261 "ytab.c"
    break;

  case 327: /* polynomial: POLYNOMIAL expression  */
#line 1347 "polyray.y"
      { ACTION(polynomial_action1((yyvsp[0].exper), 1);) }
#line 4267 "ytab.c"
    break;

  case 328: /* patch_vertex: point ',' point  */
#line 1352 "polyray.y"
      { ACTION(VecCopy((yyvsp[-2].vec), tri_vertex[npoints].pos);
               VecCopy((yyvsp[0].vec), tri_vertex[npoints].norm);
               tri_vertex[npoints].u = PLY_HUGE;
               tri_vertex[npoints].v = PLY_HUGE;
               npoints++;) }
#line 4277 "ytab.c"
    break;

  case 329: /* patch_vertex: point ',' point UV fexper ',' fexper  */
#line 1358 "polyray.y"
      { ACTION(VecCopy((yyvsp[-6].vec), tri_vertex[npoints].pos);
               VecCopy((yyvsp[-4].vec), tri_vertex[npoints].norm);
               tri_vertex[npoints].u = (yyvsp[-2].flt);
               tri_vertex[npoints].v = (yyvsp[0].flt);
               npoints++;) }
#line 4287 "ytab.c"
    break;

  case 330: /* $@18: %empty  */
#line 1367 "polyray.y"
      { ACTION(npoints = 0;) }
#line 4293 "ytab.c"
    break;

  case 331: /* ppatch: PATCH $@18 patch_vertex ',' patch_vertex ',' patch_vertex  */
#line 1369 "polyray.y"
      { ACTION(MakeTri(Object_Stack->element, tri_vertex);) }
#line 4299 "ytab.c"
    break;

  case 332: /* raw: RAW sexper  */
#line 1374 "polyray.y"
      { ACTION(MakeRaw(Object_Stack->element, (yyvsp[0].name), 0.0);
        polyray_free((yyvsp[0].name));) }
#line 4306 "ytab.c"
    break;

  case 333: /* raw: RAW sexper ',' fexper  */
#line 1377 "polyray.y"
      { ACTION(MakeRaw(Object_Stack->element, (yyvsp[-2].name), (yyvsp[0].flt));
        polyray_free((yyvsp[-2].name));) }
#line 4313 "ytab.c"
    break;

  case 334: /* smooth_height_field: SMOOTH_HEIGHT_FIELD sexper  */
#line 1383 "polyray.y"
      { ACTION(MakeHeight(Object_Stack->element, (yyvsp[0].name), 1);
        polyray_free((yyvsp[0].name));) }
#line 4320 "ytab.c"
    break;

  case 335: /* smooth_height_fn: SMOOTH_HEIGHT_FN fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1391 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                            (yyvsp[-8].flt), (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt),
                            (yyvsp[0].exper), 1);) }
#line 4328 "ytab.c"
    break;

  case 336: /* smooth_height_fn: SMOOTH_HEIGHT_FN fexper ',' fexper ',' expression  */
#line 1395 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt),
                            0.0, 1.0, 0.0, 1.0,
                            (yyvsp[0].exper), 1);) }
#line 4336 "ytab.c"
    break;

  case 337: /* smooth_cheight_field: SMOOTH_CHEIGHT_FIELD sexper  */
#line 1402 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[0].name), 1, 1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4343 "ytab.c"
    break;

  case 338: /* smooth_cheight_field: SMOOTH_CHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1405 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[-4].name), 1,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4351 "ytab.c"
    break;

  case 339: /* smooth_cheight_fn: SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1412 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 1, 1.0, 128.0);) }
#line 4358 "ytab.c"
    break;

  case 340: /* smooth_cheight_fn: SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1415 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 1, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4365 "ytab.c"
    break;

  case 341: /* smooth_sheight_field: SMOOTH_SHEIGHT_FIELD sexper  */
#line 1421 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[0].name), 0,
                             1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4373 "ytab.c"
    break;

  case 342: /* smooth_sheight_field: SMOOTH_SHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1425 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[-4].name), 1,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4381 "ytab.c"
    break;

  case 343: /* smooth_sheight_fn: SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1432 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 1, 1.0, 128.0);) }
#line 4388 "ytab.c"
    break;

  case 344: /* smooth_sheight_fn: SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1435 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 1, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4395 "ytab.c"
    break;

  case 345: /* sphere: SPHERE point ',' fexper  */
#line 1441 "polyray.y"
      { ACTION(MakeSphere(Object_Stack->element,
                          (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4402 "ytab.c"
    break;

  case 346: /* spherical_height_field: SHEIGHT_FIELD sexper  */
#line 1447 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[0].name), 0, 1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4409 "ytab.c"
    break;

  case 347: /* spherical_height_field: SHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1450 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[-4].name), 0,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4417 "ytab.c"
    break;

  case 348: /* spherical_height_fn: SHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1457 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 0, 1.0, 128.0);) }
#line 4424 "ytab.c"
    break;

  case 349: /* spherical_height_fn: SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1460 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 0, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4431 "ytab.c"
    break;

  case 350: /* superq: SUPERQ fexper ',' fexper  */
#line 1466 "polyray.y"
      { ACTION(MakeSuperq(Object_Stack->element, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4437 "ytab.c"
    break;

  case 351: /* $@19: %empty  */
#line 1471 "polyray.y"
      { ACTION(npoints = (int)(yyvsp[-1].flt);
               if (npoints < 2)
                  error("contours must have at least 3 sides\n");
               plist = (fVec *)polyray_malloc(((int)(yyvsp[-1].flt) + 1) * sizeof(fVec));
               if (plist == NULL) error("Failed to allocate contour data\n");
               pl = plist;) }
#line 4448 "ytab.c"
    break;

  case 352: /* contour: CONTOUR fexper ',' $@19 pointlist  */
#line 1478 "polyray.y"
      { ACTION(if (gcount == 0) error("Too many contours for the glyph\n");
               cl->count = (int)(yyvsp[-3].flt);
               cl->points = plist;
               gcount--; cl++;) }
#line 4457 "ytab.c"
    break;

  case 355: /* $@20: %empty  */
#line 1491 "polyray.y"
      { ACTION(gcount = (int)(yyvsp[0].flt);
               if (gcount < 1)
                  error("Glyphs must have at least one contour");
               contours = polyray_malloc(gcount * sizeof(Contour));
               if (contours == NULL)
                  error("Failed to allocate glyph data");
               cl = contours;) }
#line 4469 "ytab.c"
    break;

  case 356: /* glyph: GLYPH fexper $@20 glyph_contours  */
#line 1499 "polyray.y"
      { ACTION(if (gcount != 0)
                  error("Wrong number of contours in glyph\n");
               MakeGlyph(Object_Stack->element, (int)(yyvsp[-2].flt), contours);) }
#line 4477 "ytab.c"
    break;

  case 357: /* $@21: %empty  */
#line 1506 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec));
            if (plist == NULL) error("Failed to allocate sweep data\n");
            pl = plist;) }
#line 4486 "ytab.c"
    break;

  case 358: /* sweep: SWEEP fexper ',' point ',' fexper ',' $@21 pointlist  */
#line 1511 "polyray.y"
   { ACTION(MakeSweep(Object_Stack->element, (int)(yyvsp[-7].flt),
                      (yyvsp[-5].vec), (int)(yyvsp[-3].flt), plist);) }
#line 4493 "ytab.c"
    break;

  case 359: /* torus: TORUS fexper ',' fexper ',' point ',' point  */
#line 1517 "polyray.y"
      { ACTION(MakeTorus(Object_Stack->element, (yyvsp[-6].flt), (yyvsp[-4].flt),
                         (yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 4500 "ytab.c"
    break;

  case 360: /* fexper: expression  */
#line 1523 "polyray.y"
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(NULL, (yyvsp[0].exper), &ftmp, vtmp, &tnode) == 1) {
                  deallocate_node((yyvsp[0].exper));
                  (yyval.flt) = ftmp;
                  }
               else {
                  error("Bad fexper expression\n");
                  }) }
#line 4513 "ytab.c"
    break;

  case 361: /* point: expression  */
#line 1535 "polyray.y"
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(NULL, (yyvsp[0].exper), &ftmp, vtmp, &tnode) == 2) {
                  VecCopy(vtmp, (yyval.vec));
                  deallocate_node((yyvsp[0].exper));
                  }
               else {
                  error("Bad point expression\n");
                  }) }
#line 4526 "ytab.c"
    break;

  case 362: /* sexper: expression  */
#line 1547 "polyray.y"
      { ACTION(char *stmp;
               if (create_string((yyvsp[0].exper), &stmp)) {
                  deallocate_node((yyvsp[0].exper));
                  (yyval.name) = stmp;
                  }
               else {
                  error("Bad sexper expression\n");
                  }) }
#line 4539 "ytab.c"
    break;

  case 363: /* pointlist: point  */
#line 1559 "polyray.y"
     { ACTION(if (npoints==0) error("Too many points for the polygon\n");
              VecCopy((yyvsp[0].vec), (*pl)); npoints--; pl++;) }
#line 4546 "ytab.c"
    break;

  case 364: /* pointlist: pointlist ',' point  */
#line 1562 "polyray.y"
     { ACTION(if (npoints==0) error("Too many points for the polygon\n");
              VecCopy((yyvsp[0].vec), (*pl)); npoints--; pl++;) }
#line 4553 "ytab.c"
    break;

  case 365: /* expression: '(' expression ')'  */
#line 1568 "polyray.y"
      { ACTION((yyval.exper) = (yyvsp[-1].exper);) }
#line 4559 "ytab.c"
    break;

  case 366: /* expression: '[' expression_list ']'  */
#line 1570 "polyray.y"
      { ACTION((yyval.exper) = make_array_node((yyvsp[-1].elist));) }
#line 4565 "ytab.c"
    break;

  case 367: /* expression: '<' expression ',' expression '>'  */
#line 1572 "polyray.y"
      { ACTION((yyval.exper) = make_vector3_node((yyvsp[-3].exper), (yyvsp[-1].exper),
                                             make_value_node(0.0));) }
#line 4572 "ytab.c"
    break;

  case 368: /* expression: '<' expression ',' expression ',' expression '>'  */
#line 1575 "polyray.y"
      { ACTION((yyval.exper) = make_vector3_node((yyvsp[-5].exper), (yyvsp[-3].exper),
                                             (yyvsp[-1].exper));) }
#line 4579 "ytab.c"
    break;

  case 369: /* expression: '<' expression ',' expression ',' expression ',' expression '>'  */
#line 1578 "polyray.y"
      { ACTION((yyval.exper) = make_vector4_node((yyvsp[-7].exper), (yyvsp[-5].exper),
                                             (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4586 "ytab.c"
    break;

  case 370: /* expression: expression '[' expression ']'  */
#line 1581 "polyray.y"
      { ACTION((yyval.exper) = make_node(SUBSCRIPT_EXPER, (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4592 "ytab.c"
    break;

  case 371: /* expression: '(' conditional '?' expression ':' expression ')'  */
#line 1583 "polyray.y"
      { ACTION((yyval.exper) = make_cond_node((yyvsp[-5].exper), (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4598 "ytab.c"
    break;

  case 372: /* expression: expression '^' expression  */
#line 1585 "polyray.y"
      { ACTION((yyval.exper) = make_node(POWER_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4604 "ytab.c"
    break;

  case 373: /* expression: expression '%' expression  */
#line 1587 "polyray.y"
      { ACTION((yyval.exper) = make_node(FMOD, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4610 "ytab.c"
    break;

  case 374: /* expression: expression '*' expression  */
#line 1589 "polyray.y"
      { ACTION((yyval.exper) = make_node(TIMES_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4616 "ytab.c"
    break;

  case 375: /* expression: expression '.' expression  */
#line 1591 "polyray.y"
      { (yyval.exper) = make_node(DOT_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper)); }
#line 4622 "ytab.c"
    break;

  case 376: /* expression: expression '/' expression  */
#line 1593 "polyray.y"
      { ACTION((yyval.exper) = make_node(DIV_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4628 "ytab.c"
    break;

  case 377: /* expression: expression '+' expression  */
#line 1595 "polyray.y"
      { ACTION((yyval.exper) = make_node(PLUS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4634 "ytab.c"
    break;

  case 378: /* expression: expression '-' expression  */
#line 1597 "polyray.y"
      { ACTION((yyval.exper) = make_node(MINUS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4640 "ytab.c"
    break;

  case 379: /* expression: '-' expression  */
#line 1599 "polyray.y"
      { ACTION((yyval.exper) = make_node(TIMES_EXPER, make_value_node(-1.0),
                                     (yyvsp[0].exper));)}
#line 4647 "ytab.c"
    break;

  case 380: /* expression: '|' expression '|'  */
#line 1602 "polyray.y"
      { ACTION((yyval.exper) = make_fn1_node(FABS, (yyvsp[-1].exper));) }
#line 4653 "ytab.c"
    break;

  case 381: /* expression: COLOR_MAP '(' map_entries ',' expression ')'  */
#line 1604 "polyray.y"
      { ACTION((yyval.exper) = make_cmap_node((yyvsp[-3].cmap_entry), (yyvsp[-1].exper));) }
#line 4659 "ytab.c"
    break;

  case 382: /* expression: COLOR_MAP '(' map_entries ')'  */
#line 1606 "polyray.y"
      { ACTION((yyval.exper) = make_cmap_node((yyvsp[-1].cmap_entry), NULL);) }
#line 4665 "ytab.c"
    break;

  case 383: /* expression: NOISE '(' expression ')'  */
#line 1608 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOISE, (yyvsp[-1].exper), NULL);) }
#line 4671 "ytab.c"
    break;

  case 384: /* expression: NOISE '(' expression ',' expression ')'  */
#line 1610 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOISE, (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4677 "ytab.c"
    break;

  case 385: /* expression: ROTATE '(' expression ',' expression ')'  */
#line 1612 "polyray.y"
      { ACTION((yyval.exper) = make_fn3_node(ROTATE, (yyvsp[-3].exper), (yyvsp[-1].exper), NULL);) }
#line 4683 "ytab.c"
    break;

  case 386: /* expression: ROTATE '(' expression ',' expression ',' expression ')'  */
#line 1614 "polyray.y"
      { ACTION((yyval.exper) = make_fn3_node(ROTATE, (yyvsp[-5].exper), (yyvsp[-3].exper),
                                         (yyvsp[-1].exper));) }
#line 4690 "ytab.c"
    break;

  case 387: /* expression: COLOR  */
#line 1617 "polyray.y"
      { ACTION((yyval.exper) = make_node(COLOR, NULL, NULL);) }
#line 4696 "ytab.c"
    break;

  case 388: /* expression: FRAME  */
#line 1619 "polyray.y"
      { ACTION((yyval.exper) = make_node(FRAME, NULL, NULL);) }
#line 4702 "ytab.c"
    break;

  case 389: /* expression: END_FRAME  */
#line 1621 "polyray.y"
      { ACTION((yyval.exper) = make_node(END_FRAME, NULL, NULL);) }
#line 4708 "ytab.c"
    break;

  case 390: /* expression: START_FRAME  */
#line 1623 "polyray.y"
      { ACTION((yyval.exper) = make_node(START_FRAME, NULL, NULL);) }
#line 4714 "ytab.c"
    break;

  case 391: /* expression: TOTAL_FRAMES  */
#line 1625 "polyray.y"
      { ACTION((yyval.exper) = make_node(TOTAL_FRAMES, NULL, NULL);) }
#line 4720 "ytab.c"
    break;

  case 392: /* expression: TOKEN '(' expression_list ')'  */
#line 1627 "polyray.y"
      { ACTION((yyval.exper) = check_term((yyvsp[-3].name), (yyvsp[-1].elist));)
        polyray_free((yyvsp[-3].name)); }
#line 4727 "ytab.c"
    break;

  case 393: /* expression: TOKEN  */
#line 1630 "polyray.y"
      { ACTION((yyval.exper) = check_term0((yyvsp[0].name));)
        polyray_free((yyvsp[0].name)); }
#line 4734 "ytab.c"
    break;

  case 394: /* expression: NUM  */
#line 1633 "polyray.y"
      { ACTION((yyval.exper) = make_value_node((yyvsp[0].flt));) }
#line 4740 "ytab.c"
    break;

  case 395: /* expression: STRING  */
#line 1635 "polyray.y"
      { ACTION((yyval.exper) = make_string_node((yyvsp[0].name));)
               polyray_free((yyvsp[0].name)); }
#line 4747 "ytab.c"
    break;

  case 396: /* expression: EXPRESSION_SYM  */
#line 1638 "polyray.y"
      { ACTION((yyval.exper) = exper_action((yyvsp[0].name));) }
#line 4753 "ytab.c"
    break;

  case 397: /* expression_list: expression  */
#line 1643 "polyray.y"
      { ACTION((yyval.elist) = make_list_node((yyvsp[0].exper));) }
#line 4759 "ytab.c"
    break;

  case 398: /* expression_list: expression_list ',' expression  */
#line 1645 "polyray.y"
      { ACTION((yyval.elist) = expression_action1((yyvsp[-2].elist), (yyvsp[0].exper));) }
#line 4765 "ytab.c"
    break;

  case 399: /* conditional: '(' conditional ')'  */
#line 1650 "polyray.y"
      { ACTION((yyval.exper) = (yyvsp[-1].exper);) }
#line 4771 "ytab.c"
    break;

  case 400: /* conditional: expression '<' expression  */
#line 1652 "polyray.y"
      { ACTION((yyval.exper) = make_node(LESS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4777 "ytab.c"
    break;

  case 401: /* conditional: expression '>' expression  */
#line 1654 "polyray.y"
      { ACTION((yyval.exper) = make_node(GREATER_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4783 "ytab.c"
    break;

  case 402: /* conditional: expression LTEQ_SYM expression  */
#line 1656 "polyray.y"
      { ACTION((yyval.exper) = make_node(LTEQ_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4789 "ytab.c"
    break;

  case 403: /* conditional: expression GTEQ_SYM expression  */
#line 1658 "polyray.y"
      { ACTION((yyval.exper) = make_node(GTEQ_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4795 "ytab.c"
    break;

  case 404: /* conditional: expression EQUAL_SYM expression  */
#line 1660 "polyray.y"
      { ACTION((yyval.exper) = make_node(EQUAL_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4801 "ytab.c"
    break;

  case 405: /* conditional: conditional AND_SYM conditional  */
#line 1662 "polyray.y"
      { ACTION((yyval.exper) = make_node(AND_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4807 "ytab.c"
    break;

  case 406: /* conditional: conditional OR_SYM conditional  */
#line 1664 "polyray.y"
      { ACTION((yyval.exper) = make_node(OR_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4813 "ytab.c"
    break;

  case 407: /* conditional: '!' conditional  */
#line 1666 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOT_EXPER, (yyvsp[-1].exper), NULL);) }
#line 4819 "ytab.c"
    break;

  case 408: /* map_entry: '[' fexper ',' fexper ',' point ',' point ']'  */
#line 1671 "polyray.y"
      { ACTION((yyval.cmap_entry) =
                 map_entry_action1((yyvsp[-7].flt), (yyvsp[-5].flt),
                                   (yyvsp[-3].vec), 0.0, (yyvsp[-1].vec), 0.0);) }
#line 4827 "ytab.c"
    break;

  case 409: /* map_entry: '[' fexper ',' fexper ',' point ',' fexper ',' point ',' fexper ']'  */
#line 1675 "polyray.y"
      { ACTION((yyval.cmap_entry) =
                 map_entry_action1((yyvsp[-11].flt), (yyvsp[-9].flt),
                                   (yyvsp[-7].vec), (yyvsp[-5].flt),
                                   (yyvsp[-3].vec), (yyvsp[-1].flt));) }
#line 4836 "ytab.c"
    break;

  case 410: /* map_entries: map_entry  */
#line 1683 "polyray.y"
      { ACTION((yyval.cmap_entry) = (yyvsp[0].cmap_entry);) }
#line 4842 "ytab.c"
    break;

  case 411: /* map_entries: map_entries map_entry  */
#line 1685 "polyray.y"
      { ACTION((yyval.cmap_entry) = map_entry_action2((yyvsp[-1].cmap_entry),
                                                  (yyvsp[0].cmap_entry));) }
#line 4849 "ytab.c"
    break;

  case 416: /* end_frame_decl: END_FRAME fexper  */
#line 1698 "polyray.y"
      { ACTION(end_frame = (int)(yyvsp[0].flt);) }
#line 4855 "ytab.c"
    break;

  case 417: /* start_frame_decl: START_FRAME fexper  */
#line 1703 "polyray.y"
      { ACTION(start_frame = (int)(yyvsp[0].flt);
               if (!Parsed_Flag) current_frame = start_frame;) }
#line 4862 "ytab.c"
    break;

  case 418: /* total_frames_decl: TOTAL_FRAMES fexper  */
#line 1709 "polyray.y"
      { ACTION(total_frames = (int)(yyvsp[0].flt);) }
#line 4868 "ytab.c"
    break;

  case 419: /* frame_time_decl: FRAME_TIME fexper  */
#line 1714 "polyray.y"
      { ACTION(frame_time = (yyvsp[0].flt);) }
#line 4874 "ytab.c"
    break;

  case 420: /* outfile: OUTFILE TOKEN  */
#line 1719 "polyray.y"
      { ACTION(if (!Parsed_Flag) {
                  strcpy(outfilebase, (yyvsp[0].name));
                  filebaseflag = 1;
                  })
        polyray_free((yyvsp[0].name)); }
#line 4884 "ytab.c"
    break;

  case 421: /* outfile: OUTFILE STRING  */
#line 1725 "polyray.y"
      { ACTION(if (!Parsed_Flag) {
                  strcpy(outfilebase, (yyvsp[0].name));
                  filebaseflag = 1;
                  })
        polyray_free((yyvsp[0].name)); }
#line 4894 "ytab.c"
    break;

  case 422: /* flush_statement: FILE_FLUSH fexper  */
#line 1734 "polyray.y"
      { ACTION(flush_action((int)(yyvsp[0].flt));) }
#line 4900 "ytab.c"
    break;

  case 423: /* system_call: SYSTEM '(' expression_list ')'  */
#line 1739 "polyray.y"
      { ACTION(evaluate_system_call((yyvsp[-1].elist));) }
#line 4906 "ytab.c"
    break;

  case 426: /* $@22: %empty  */
#line 1749 "polyray.y"
     { condition_flags[condition_depth] = 1-condition_flags[condition_depth]; }
#line 4912 "ytab.c"
    break;

  case 429: /* $@23: %empty  */
#line 1756 "polyray.y"
      { Flt ftmp; Vec vtmp; NODE_PTR tnode;
        if (check_condition()) {
           if (eval_node(NULL, (yyvsp[-1].exper), &ftmp, vtmp, &tnode) == 1 &&
               ftmp != 0.0) {
              condition_flags[++condition_depth] = 1;
              }
           else
              condition_flags[++condition_depth] = 0;
           deallocate_node((yyvsp[-1].exper));
           }
        else
           condition_flags[++condition_depth] = 0;
     }
#line 4930 "ytab.c"
    break;

  case 430: /* if_statement: IF '(' conditional ')' $@23 statement if_else_part  */
#line 1770 "polyray.y"
     { condition_depth--; }
#line 4936 "ytab.c"
    break;


#line 4940 "ytab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1773 "polyray.y"
