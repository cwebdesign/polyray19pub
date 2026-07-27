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
#include <utility>
#include "defs3.h"
#include "object_dispatch.h"
#include "io_ply.h"
#include "memory.h"
#include "runtime_state.h"
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
#include "psupport.h"//for create_string
#include "raw.h"
#include "revolve.h"
#include "sphere.h"
#include "superq.h"
#include "sweep.h"
#include "torus.h"
#include "tri.h"

#define ACTION(x) { if (check_condition()) { x } }
#define alloca malloc
#define yyerror serror

struct def_tok_struct {
   int type;
   void *data;
   } temp_def;
static Contour *cl, *contours;
static int gcount;

void start_include(std::string);

#line 145 "polyray.tab.cc"

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

#include "polyray.tab.h"
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
  YYSYMBOL_NURB2 = 276,                    /* NURB2  */
  YYSYMBOL_277_ = 277,                     /* '%'  */
  YYSYMBOL_UMINUS = 278,                   /* UMINUS  */
  YYSYMBOL_279_ = 279,                     /* ','  */
  YYSYMBOL_280_ = 280,                     /* ':'  */
  YYSYMBOL_281_ = 281,                     /* '?'  */
  YYSYMBOL_282_ = 282,                     /* '|'  */
  YYSYMBOL_YYACCEPT = 283,                 /* $accept  */
  YYSYMBOL_scene = 284,                    /* scene  */
  YYSYMBOL_285_1 = 285,                    /* $@1  */
  YYSYMBOL_elementlist = 286,              /* elementlist  */
  YYSYMBOL_element = 287,                  /* element  */
  YYSYMBOL_include_statement = 288,        /* include_statement  */
  YYSYMBOL_defined_token = 289,            /* defined_token  */
  YYSYMBOL_definition_types = 290,         /* definition_types  */
  YYSYMBOL_definition = 291,               /* definition  */
  YYSYMBOL_particle_decls = 292,           /* particle_decls  */
  YYSYMBOL_particle_decl = 293,            /* particle_decl  */
  YYSYMBOL_particle = 294,                 /* particle  */
  YYSYMBOL_295_2 = 295,                    /* $@2  */
  YYSYMBOL_object = 296,                   /* object  */
  YYSYMBOL_297_3 = 297,                    /* $@3  */
  YYSYMBOL_298_4 = 298,                    /* $@4  */
  YYSYMBOL_object_modifier_decls = 299,    /* object_modifier_decls  */
  YYSYMBOL_object_modifier_decl = 300,     /* object_modifier_decl  */
  YYSYMBOL_uv_information = 301,           /* uv_information  */
  YYSYMBOL_root_solver = 302,              /* root_solver  */
  YYSYMBOL_object_decls = 303,             /* object_decls  */
  YYSYMBOL_shape_decl = 304,               /* shape_decl  */
  YYSYMBOL_camera_exper = 305,             /* camera_exper  */
  YYSYMBOL_camera_expers = 306,            /* camera_expers  */
  YYSYMBOL_camera = 307,                   /* camera  */
  YYSYMBOL_flare_options = 308,            /* flare_options  */
  YYSYMBOL_flare_option = 309,             /* flare_option  */
  YYSYMBOL_light_modifier_decl = 310,      /* light_modifier_decl  */
  YYSYMBOL_311_5 = 311,                    /* $@5  */
  YYSYMBOL_light_modifier_decls = 312,     /* light_modifier_decls  */
  YYSYMBOL_depth_light_modifier = 313,     /* depth_light_modifier  */
  YYSYMBOL_depth_light_modifiers = 314,    /* depth_light_modifiers  */
  YYSYMBOL_haze_statement = 315,           /* haze_statement  */
  YYSYMBOL_light = 316,                    /* light  */
  YYSYMBOL_317_6 = 317,                    /* @6  */
  YYSYMBOL_318_7 = 318,                    /* @7  */
  YYSYMBOL_draw_statement = 319,           /* draw_statement  */
  YYSYMBOL_background = 320,               /* background  */
  YYSYMBOL_surface_declaration = 321,      /* surface_declaration  */
  YYSYMBOL_surface_declarations = 322,     /* surface_declarations  */
  YYSYMBOL_surface = 323,                  /* surface  */
  YYSYMBOL_324_8 = 324,                    /* $@8  */
  YYSYMBOL_325_9 = 325,                    /* $@9  */
  YYSYMBOL_texture_map_element = 326,      /* texture_map_element  */
  YYSYMBOL_texture_map_elements = 327,     /* texture_map_elements  */
  YYSYMBOL_texture_fn_element = 328,       /* texture_fn_element  */
  YYSYMBOL_texture_fn_elements = 329,      /* texture_fn_elements  */
  YYSYMBOL_texture_map = 330,              /* texture_map  */
  YYSYMBOL_texture_modifier_decls = 331,   /* texture_modifier_decls  */
  YYSYMBOL_texture_modifier_decl = 332,    /* texture_modifier_decl  */
  YYSYMBOL_texture_declarations = 333,     /* texture_declarations  */
  YYSYMBOL_texture_declaration = 334,      /* texture_declaration  */
  YYSYMBOL_texture = 335,                  /* texture  */
  YYSYMBOL_336_10 = 336,                   /* $@10  */
  YYSYMBOL_337_11 = 337,                   /* $@11  */
  YYSYMBOL_texture_list = 338,             /* texture_list  */
  YYSYMBOL_transform_declaration = 339,    /* transform_declaration  */
  YYSYMBOL_transform_declarations = 340,   /* transform_declarations  */
  YYSYMBOL_transform = 341,                /* transform  */
  YYSYMBOL_342_12 = 342,                   /* $@12  */
  YYSYMBOL_343_13 = 343,                   /* $@13  */
  YYSYMBOL_bezier_points = 344,            /* bezier_points  */
  YYSYMBOL_bezier = 345,                   /* bezier  */
  YYSYMBOL_blob = 346,                     /* blob  */
  YYSYMBOL_347_14 = 347,                   /* $@14  */
  YYSYMBOL_blobelements = 348,             /* blobelements  */
  YYSYMBOL_blobelement = 349,              /* blobelement  */
  YYSYMBOL_box = 350,                      /* box  */
  YYSYMBOL_cone = 351,                     /* cone  */
  YYSYMBOL_csg = 352,                      /* csg  */
  YYSYMBOL_353_15 = 353,                   /* $@15  */
  YYSYMBOL_csg_tree = 354,                 /* csg_tree  */
  YYSYMBOL_cylinder = 355,                 /* cylinder  */
  YYSYMBOL_cylindrical_height_field = 356, /* cylindrical_height_field  */
  YYSYMBOL_cylindrical_height_fn = 357,    /* cylindrical_height_fn  */
  YYSYMBOL_disc = 358,                     /* disc  */
  YYSYMBOL_function = 359,                 /* function  */
  YYSYMBOL_gridded = 360,                  /* gridded  */
  YYSYMBOL_object_list = 361,              /* object_list  */
  YYSYMBOL_height_field = 362,             /* height_field  */
  YYSYMBOL_height_fn = 363,                /* height_fn  */
  YYSYMBOL_hypertexture = 364,             /* hypertexture  */
  YYSYMBOL_lathe = 365,                    /* lathe  */
  YYSYMBOL_366_16 = 366,                   /* $@16  */
  YYSYMBOL_light_object = 367,             /* light_object  */
  YYSYMBOL_nurb = 368,                     /* nurb  */
  YYSYMBOL_nurb2 = 369,                    /* nurb2  */
  YYSYMBOL_parabola = 370,                 /* parabola  */
  YYSYMBOL_parametric = 371,               /* parametric  */
  YYSYMBOL_polygon = 372,                  /* polygon  */
  YYSYMBOL_373_17 = 373,                   /* $@17  */
  YYSYMBOL_polynomial = 374,               /* polynomial  */
  YYSYMBOL_patch_vertex = 375,             /* patch_vertex  */
  YYSYMBOL_ppatch = 376,                   /* ppatch  */
  YYSYMBOL_377_18 = 377,                   /* $@18  */
  YYSYMBOL_raw = 378,                      /* raw  */
  YYSYMBOL_smooth_height_field = 379,      /* smooth_height_field  */
  YYSYMBOL_smooth_height_fn = 380,         /* smooth_height_fn  */
  YYSYMBOL_smooth_cheight_field = 381,     /* smooth_cheight_field  */
  YYSYMBOL_smooth_cheight_fn = 382,        /* smooth_cheight_fn  */
  YYSYMBOL_smooth_sheight_field = 383,     /* smooth_sheight_field  */
  YYSYMBOL_smooth_sheight_fn = 384,        /* smooth_sheight_fn  */
  YYSYMBOL_sphere = 385,                   /* sphere  */
  YYSYMBOL_spherical_height_field = 386,   /* spherical_height_field  */
  YYSYMBOL_spherical_height_fn = 387,      /* spherical_height_fn  */
  YYSYMBOL_superq = 388,                   /* superq  */
  YYSYMBOL_contour = 389,                  /* contour  */
  YYSYMBOL_390_19 = 390,                   /* $@19  */
  YYSYMBOL_glyph_contours = 391,           /* glyph_contours  */
  YYSYMBOL_glyph = 392,                    /* glyph  */
  YYSYMBOL_393_20 = 393,                   /* $@20  */
  YYSYMBOL_sweep = 394,                    /* sweep  */
  YYSYMBOL_395_21 = 395,                   /* $@21  */
  YYSYMBOL_torus = 396,                    /* torus  */
  YYSYMBOL_fexper = 397,                   /* fexper  */
  YYSYMBOL_point = 398,                    /* point  */
  YYSYMBOL_sexper = 399,                   /* sexper  */
  YYSYMBOL_pointlist = 400,                /* pointlist  */
  YYSYMBOL_expression = 401,               /* expression  */
  YYSYMBOL_expression_list = 402,          /* expression_list  */
  YYSYMBOL_conditional = 403,              /* conditional  */
  YYSYMBOL_map_entry = 404,                /* map_entry  */
  YYSYMBOL_map_entries = 405,              /* map_entries  */
  YYSYMBOL_frame_decl = 406,               /* frame_decl  */
  YYSYMBOL_end_frame_decl = 407,           /* end_frame_decl  */
  YYSYMBOL_start_frame_decl = 408,         /* start_frame_decl  */
  YYSYMBOL_total_frames_decl = 409,        /* total_frames_decl  */
  YYSYMBOL_frame_time_decl = 410,          /* frame_time_decl  */
  YYSYMBOL_outfile = 411,                  /* outfile  */
  YYSYMBOL_flush_statement = 412,          /* flush_statement  */
  YYSYMBOL_system_call = 413,              /* system_call  */
  YYSYMBOL_statement = 414,                /* statement  */
  YYSYMBOL_if_else_part = 415,             /* if_else_part  */
  YYSYMBOL_416_22 = 416,                   /* $@22  */
  YYSYMBOL_if_statement = 417,             /* if_statement  */
  YYSYMBOL_418_23 = 418                    /* $@23  */
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
#define YYLAST   2534

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  283
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  136
/* YYNRULES -- Number of rules.  */
#define YYNRULES  432
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1052

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   514


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
       2,     2,     2,   268,     2,     2,     2,   277,   252,     2,
     256,   257,   262,   260,   279,   261,   263,   264,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   280,     2,
     266,   253,   265,   281,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   254,     2,   255,   267,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   258,   282,   259,   269,     2,     2,     2,
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
     273,   274,   275,   276,   278
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   409,   409,   409,   414,   415,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   429,   430,   432,   433,   434,
     439,   451,   453,   455,   457,   459,   461,   463,   468,   471,
     474,   477,   480,   483,   486,   492,   495,   498,   502,   506,
     509,   512,   516,   523,   524,   528,   530,   532,   534,   536,
     538,   540,   542,   544,   550,   549,   553,   559,   558,   562,
     565,   564,   572,   573,   577,   581,   584,   586,   588,   592,
     594,   596,   597,   599,   601,   607,   608,   613,   618,   623,
     626,   629,   632,   640,   642,   644,   649,   650,   654,   655,
     656,   657,   658,   659,   660,   661,   662,   663,   664,   665,
     666,   667,   668,   669,   670,   671,   672,   673,   674,   675,
     676,   677,   678,   679,   680,   681,   682,   683,   684,   685,
     686,   687,   688,   689,   690,   694,   696,   701,   703,   705,
     707,   709,   711,   713,   715,   725,   730,   734,   738,   743,
     749,   752,   754,   759,   760,   764,   768,   769,   773,   775,
     777,   779,   781,   783,   785,   790,   792,   795,   797,   800,
     799,   802,   805,   807,   809,   813,   815,   820,   821,   825,
     827,   829,   831,   833,   836,   838,   840,   842,   847,   848,
     852,   857,   859,   861,   864,   867,   869,   872,   875,   881,
     880,   884,   886,   888,   891,   895,   894,   902,   905,   910,
     915,   917,   919,   921,   923,   925,   927,   929,   931,   933,
     935,   937,   939,   941,   943,   945,   947,   949,   951,   953,
     955,   957,   959,   961,   963,   965,   967,   969,   972,   975,
     980,   981,   986,   985,   989,   992,   991,   998,  1004,  1006,
    1012,  1017,  1019,  1024,  1026,  1031,  1032,  1036,  1044,  1046,
    1048,  1052,  1054,  1059,  1063,  1065,  1067,  1069,  1071,  1074,
    1076,  1079,  1085,  1084,  1088,  1091,  1090,  1097,  1099,  1104,
    1106,  1108,  1110,  1115,  1116,  1121,  1120,  1124,  1127,  1126,
    1133,  1135,  1140,  1148,  1147,  1156,  1157,  1161,  1163,  1165,
    1168,  1171,  1177,  1182,  1188,  1188,  1195,  1197,  1200,  1204,
    1207,  1210,  1213,  1216,  1222,  1228,  1232,  1239,  1242,  1249,
    1252,  1258,  1263,  1269,  1276,  1286,  1292,  1298,  1305,  1311,
    1310,  1321,  1326,  1331,  1339,  1347,  1353,  1359,  1358,  1370,
    1375,  1381,  1391,  1390,  1397,  1400,  1406,  1412,  1418,  1425,
    1428,  1435,  1438,  1444,  1448,  1455,  1458,  1464,  1470,  1473,
    1480,  1483,  1489,  1495,  1494,  1509,  1510,  1515,  1514,  1530,
    1529,  1540,  1546,  1558,  1570,  1585,  1588,  1594,  1596,  1598,
    1601,  1604,  1607,  1609,  1611,  1613,  1615,  1617,  1619,  1621,
    1623,  1625,  1628,  1630,  1632,  1634,  1636,  1638,  1640,  1643,
    1645,  1647,  1649,  1651,  1653,  1656,  1659,  1661,  1664,  1669,
    1671,  1676,  1678,  1680,  1682,  1684,  1686,  1688,  1690,  1692,
    1697,  1701,  1709,  1711,  1717,  1718,  1719,  1720,  1724,  1729,
    1735,  1740,  1745,  1751,  1760,  1765,  1770,  1771,  1776,  1775,
    1778,  1783,  1782
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
  "LTEQ_SYM", "GTEQ_SYM", "EQUAL_SYM", "NEQUAL_SYM", "NURB2", "'%'",
  "UMINUS", "','", "':'", "'?'", "'|'", "$accept", "scene", "$@1",
  "elementlist", "element", "include_statement", "defined_token",
  "definition_types", "definition", "particle_decls", "particle_decl",
  "particle", "$@2", "object", "$@3", "$@4", "object_modifier_decls",
  "object_modifier_decl", "uv_information", "root_solver", "object_decls",
  "shape_decl", "camera_exper", "camera_expers", "camera", "flare_options",
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
  "light_object", "nurb", "nurb2", "parabola", "parametric", "polygon",
  "$@17", "polynomial", "patch_vertex", "ppatch", "$@18", "raw",
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

#define YYPACT_NINF (-884)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-364)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -884,    22,  2194,  -884,  1859,   -61,  -203,   884,  1859,  1859,
    1859,  1859,  1859,  -180,  -102,  1077,  -158,  -145,  -130,  -123,
    -884,  1859,  1740,  1859,   102,   -93,   -97,  1859,   -92,  2194,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
     -86,  -884,  -884,  -884,   -71,  -884,   -69,  -884,  -884,   -64,
    -884,  1859,  1748,  1859,  1859,  1859,   440,  -884,   -40,  -884,
    -884,  -884,  -884,  -884,  1058,  -884,  1058,  -884,  1859,   -89,
     440,   -73,   440,  -884,  -884,  -884,   -65,  1748,  -884,  1859,
     -59,  -884,  -884,  -884,  -884,  -884,   852,  1859,   -52,  -884,
     226,  1859,  -884,  -884,   195,  -884,   -46,  1859,  1859,  1859,
     440,  -213,  1748,  1748,  2173,  -124,   -32,   918,   125,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,  1058,  1058,   -30,
     -26,   -21,   -14,  -884,   -15,   -12,    -7,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,   440,  -884,  -884,   -22,  1859,  1859,
    1859,  2195,  -176,   -20,  1859,  1704,  1259,   298,  1859,   -17,
    1859,   430,  1058,  1058,  -245,  -884,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  -884,   551,  1859,  -884,  -234,   451,
    1027,  -230,  -884,  1859,   -10,  -884,  -884,  1859,  1859,  1859,
    1859,  1859,  1748,  1748,  1859,  1859,  -884,   480,   114,   114,
    -228,  -228,  -228,   -32,  -228,  -884,  -884,  -884,  -884,  -884,
      -1,  -884,  -884,  -884,    14,  1859,  -884,   -16,     3,  -884,
    1859,  -884,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    -884,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,     8,  1259,  -884,
    -884,  -884,  -884,  -884,  -884,  -107,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  1859,  1859,
    1859,   -66,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1169,  -884,  -884,  -884,  -884,  -884,  1859,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,    34,  -884,   440,
    1859,     6,  1058,  1058,  -884,  -884,  -884,   657,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,     9,  -884,
    -884,  -884,  -884,    10,  -884,  -884,  -884,  -884,    12,  -884,
    1859,  -884,  -884,  1859,  1859,  -884,   440,  -884,   440,   440,
     440,   440,   440,  -884,  -884,   213,   525,  -884,  -884,  -884,
     327,  1859,  -884,  -192,  -884,   -95,   -95,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  -884,  1859,  -884,  -884,  -884,  1859,
    1859,  2095,  -884,    15,   -25,    21,    23,   440,    24,    27,
      28,    32,   440,  -884,    36,  -884,    38,   440,    39,    40,
      43,   440,  1859,    44,   440,    54,  -884,    55,    56,    74,
      75,    76,    77,    80,    82,    86,    96,   103,   111,  -884,
    1259,  -107,  -107,  -884,    85,   115,   440,  -884,  -884,  -884,
    -884,   121,  -884,  -884,   124,  -884,  -884,  -884,   126,   129,
    -884,  -884,  -884,   440,   440,   440,   440,   440,   440,   440,
     440,   440,  -884,  -884,   131,  1859,  -884,  -884,  1859,  -884,
    -884,  1859,  1859,  1859,  1859,  1859,  1859,  -884,  -884,  -884,
    1859,  1859,  1859,   398,  2224,   605,  1859,  -884,  1859,   316,
     941,   -68,   -68,  1859,   -68,   -24,   -24,  1859,  -884,    13,
    -884,   132,  -884,    -1,   -91,  1859,  1859,  1859,  -884,   -36,
       7,  -884,  -884,  -884,   440,  -884,  -884,  -884,  -884,   133,
    -884,  2194,  -884,   200,  1859,  -884,  1859,  1859,  1859,  1859,
    1859,  1859,   241,    52,  1859,  1859,  1859,  1859,   135,   136,
    -884,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,   629,  -884,  -107,  -107,  -107,  -107,
    -107,  1859,  1859,  1859,  1859,  1859,  1859,   137,   440,    26,
     138,   139,  -884,   140,   145,  -884,   146,  -884,   148,  -884,
    -884,  -884,  1859,  2235,   738,  1859,  1859,  1859,  1859,   108,
    1859,  1859,  1859,   339,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,  1859,  1859,  -884,  -884,  -884,   154,   155,  1090,
    -884,   158,  -884,  -884,  -884,   161,  1101,  -884,   296,  1859,
    -884,  1859,  1859,  1859,  1859,  -884,  -884,  -884,   162,  -884,
    -884,  -884,  -884,  -884,  1859,  2070,  -884,  -884,   165,   827,
    -884,   167,   169,   171,   172,   177,  1859,  -884,   241,  -884,
      52,   179,   181,   182,   199,  1859,  1859,  1859,  -884,   203,
     206,   207,   208,   215,   216,   217,  -884,  -884,   218,   220,
     222,  -884,    16,  -160,  -160,   238,  -884,  -884,  -884,   227,
     229,   233,   235,  1859,  -884,  1859,  1859,  1859,  1859,  1859,
    1859,  2246,  -884,  -884,  1859,  1132,   440,   440,   440,   -46,
    1246,   440,   440,  1859,  1859,  1859,  1859,  1859,   440,   440,
     440,   440,   440,   440,   440,  1323,  1356,  1446,   440,   -68,
     -68,     1,   -68,  1859,   -68,   236,   240,  -884,   245,  -884,
    1859,  1508,  -884,  2095,  1859,  1859,  1859,  1859,  1859,   246,
    -884,   247,  1859,  1859,  1859,  1859,  1859,   248,  -884,  -884,
    1859,  1859,  1859,  1859,   250,    57,  -884,   253,  1859,  1859,
    1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,   254,   104,   257,  -884,   259,  -884,   262,   265,
    -884,  1038,  1859,  -216,  1859,   440,   440,   440,   440,   440,
    1859,  1859,  1859,  -884,   266,  -884,  -884,  -884,  -884,   -68,
    1859,  1859,  -884,  1859,  -884,   269,   270,   271,   275,   277,
     827,  1859,  -884,  1688,   284,  -884,   289,  -884,   290,  1714,
     291,   293,  -884,  1859,  1859,  1859,   294,  1714,  -884,  1771,
    -884,  1820,  -884,  1831,   295,   297,   299,   300,  -884,   301,
     302,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  1859,  -884,
    -884,  1859,  1859,  1859,  1859,  -884,   440,  -884,  1859,   440,
     440,   440,  1876,   -68,   304,  -884,   305,   440,  1859,  1859,
    1859,  1859,  1859,  -884,   306,  1859,  1859,  1859,  1859,  1859,
    -884,  1859,  -884,   307,  -884,  1859,  1859,  1859,  1859,  -884,
    1859,  1859,  1859,  1859,  1859,   313,   440,  -884,  -884,  -884,
     315,  -884,  -884,   317,   318,  -884,   320,   367,   787,  2257,
    1859,  -884,   -68,  1859,   344,   345,   346,   347,   349,  1859,
     350,  -884,  -884,   253,   352,  1859,   353,  1859,   354,   357,
     360,   362,  1859,  -884,   365,   366,  -884,   368,  1859,  1859,
    1859,  1859,  1859,  -884,  -884,   440,   394,   377,  1859,  1859,
    1859,  1859,  1859,  -884,  1859,  1859,   253,  1859,  -884,  1859,
    1859,  1859,  1859,   253,  1859,  1859,  1859,  -884,  -884,  -884,
     384,   385,  -884,  1859,   388,  -884,   389,   395,  -884,   405,
    -884,   407,  1955,   409,  -884,  -884,  -884,  2060,   410,  -884,
    1859,  1859,   413,  1859,  1859,  1859,  1859,  1859,  1859,  1859,
    1859,  1859,   414,   443,  1859,  -884,  -884,  -884,   420,   445,
    2087,   447,  2108,  -884,  1859,  -884,   448,  1859,  1859,  1859,
    1859,  1859,  -884,  1859,  -884,   440,   440,   440,  2134,  -884,
    1859,   440
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
      11,   414,   415,   416,   417,    17,    10,    18,    12,   389,
       0,   391,   398,   390,     0,   396,     0,   392,   397,   395,
     393,     0,     0,     0,     0,     0,   199,    25,     0,    24,
      27,    21,    23,    22,     0,    26,     0,   195,     0,   191,
     363,     0,   362,   418,   424,   421,     0,     0,    20,     0,
     182,    57,    60,   423,   422,    54,     0,     0,     0,   419,
       0,     0,   189,   420,     0,     4,     0,     0,     0,     0,
     399,     0,     0,     0,     0,     0,   381,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     234,     0,     0,   243,   264,     0,   277,    37,    34,    31,
      28,    30,    29,    32,    33,    35,   179,   193,     0,     0,
       0,     0,     0,   184,     0,   294,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   168,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   143,     0,     0,   412,     0,     0,
       0,     0,   368,     0,     0,   409,   367,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   382,     0,   379,   380,
     376,   377,   378,   374,   375,    41,    39,   232,   235,   262,
       0,   265,   275,   278,     0,     0,   192,     0,     0,   431,
       0,   181,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     332,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    86,   321,
      88,    89,    90,    91,    95,     0,    92,    93,    94,    96,
      97,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   115,   116,   113,   114,   117,
     118,   119,   120,   121,   122,    98,   123,   124,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    62,    71,    75,    64,    65,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    43,   198,
       0,   185,     0,     0,    38,    36,   425,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,     0,   136,
     137,   138,   139,     0,   141,   142,   145,   144,     0,   384,
       0,   413,   385,     0,     0,   394,   400,   401,   403,   402,
     404,   405,   406,   407,   408,     0,     0,   372,   231,   231,
       0,     0,   238,     0,   246,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   177,     0,   196,   178,   194,     0,
       0,     0,   183,     0,     0,     0,   305,   364,     0,     0,
       0,     0,   311,   357,     0,   315,     0,   318,     0,     0,
       0,   326,     0,     0,   329,   334,   336,     0,   339,     0,
     343,     0,   348,     0,     0,     0,     0,     0,     0,    58,
      87,     0,     0,   303,   295,     0,    76,    73,    83,    85,
      84,    66,    70,    72,     0,    69,    79,    80,     0,     0,
      81,    61,    63,    49,    50,    45,    52,    46,    51,    53,
      47,    48,    55,    44,   187,     0,    42,    40,     0,   159,
     158,     0,     0,     0,     0,     0,     0,   190,   167,   161,
       0,     0,     0,     0,     0,     0,     0,   369,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   254,     0,
     246,     0,   244,     0,     0,     0,     0,     0,   273,     0,
       0,   169,   170,   171,   172,   173,   174,   175,   176,     0,
     180,     0,   427,   430,     0,   283,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     327,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   300,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   155,     0,
       0,   162,   166,     0,     0,   165,     0,   140,     0,   383,
     386,   387,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   233,   230,   236,     0,     0,     0,
     267,   259,   256,   255,   241,   261,     0,   263,   253,     0,
     239,     0,     0,     0,     0,   266,   245,   247,   269,   271,
     272,   276,   274,   279,     0,     0,   428,   432,     0,     0,
     292,     0,     0,     0,     0,     0,     0,   355,   358,   313,
     312,     0,     0,     0,     0,     0,     0,     0,   335,     0,
       0,     0,     0,     0,     0,     0,   347,   352,     0,     0,
       0,   296,   301,   297,   298,   299,   302,    74,    67,     0,
      77,     0,     0,     0,   147,     0,     0,     0,     0,     0,
       0,     0,   373,   370,     0,   204,   205,   206,   200,     0,
     208,   209,   210,     0,     0,     0,     0,     0,   216,   217,
     219,   220,   218,   221,   222,   224,   226,     0,   229,     0,
       0,     0,     0,     0,     0,     0,   248,   252,     0,   251,
       0,     0,   426,     0,     0,     0,     0,     0,     0,   284,
     285,     0,     0,     0,     0,     0,     0,     0,   356,   314,
       0,     0,     0,     0,     0,   330,   365,   328,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   163,     0,   156,     0,     0,
     388,     0,     0,     0,     0,   212,   215,   213,   211,   214,
       0,     0,     0,   257,     0,   260,   268,   242,   240,     0,
       0,     0,   270,     0,   429,     0,     0,     0,     0,     0,
       0,     0,   306,   307,     0,   304,   309,   353,     0,   317,
       0,     0,   325,     0,     0,     0,     0,   338,   340,   341,
     344,   345,   349,   350,     0,     0,     0,     0,    78,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   160,
     146,     0,     0,     0,     0,   371,   203,   202,     0,   207,
     223,   225,   228,     0,     0,   249,     0,   197,     0,     0,
       0,     0,     0,   286,     0,     0,     0,     0,     0,     0,
     319,     0,   333,     0,   366,     0,     0,     0,     0,   359,
       0,     0,     0,     0,     0,     0,   148,   153,   149,   151,
       0,   150,   154,     0,     0,   135,     0,     0,   362,     0,
       0,   258,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   293,   310,   354,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   361,     0,     0,    82,     0,     0,     0,
       0,     0,     0,   410,   201,   227,     0,     0,     0,     0,
       0,     0,     0,   287,     0,     0,   320,     0,   331,     0,
       0,     0,     0,   360,     0,     0,     0,   186,   152,   157,
       0,     0,   237,     0,   282,   281,     0,     0,   288,     0,
     308,     0,   323,     0,   342,   346,   351,     0,     0,   188,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   280,   289,   290,     0,     0,
       0,     0,     0,    68,     0,   411,     0,     0,     0,     0,
       0,     0,   164,     0,   291,   316,   322,   337,     0,   250,
       0,   324
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -884,  -884,  -884,  -246,   -28,  -884,   -49,   -74,  -884,  -884,
     183,     2,  -884,    11,  -884,  -884,   441,  -314,  -884,  -884,
    -884,  -884,   544,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,   576,  -884,  -884,  -884,  -884,  -884,   348,
    -376,  -884,  -884,   210,  -884,    -5,  -884,     4,   228,  -884,
    -884,  -884,   -57,  -884,  -884,  -884,  -305,   340,  -140,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,   -84,  -884,  -884,  -884,
    -884,  -361,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -669,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,  -884,  -884,    81,  -884,  -884,  -884,
    -884,  -884,  -884,  -884,   668,  1515,  -210,  -883,    -4,     5,
     -77,  -183,    31,  -884,  -884,  -884,  -884,  -884,  -884,  -884,
    -884,    17,  -884,  -884,  -884,  -884
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    29,    30,    31,    76,   137,    32,   337,
     338,   138,   157,   139,   155,   156,   322,   323,   324,   325,
     267,   268,   184,   185,    35,   793,   870,   498,   589,   347,
     407,   224,    36,    37,   165,   146,    38,    39,   625,   509,
     140,   388,   389,   392,   393,   634,   635,   141,   524,   646,
     519,   520,   142,   390,   394,   631,   528,   529,   143,   395,
     396,   994,   270,   271,   659,   759,   760,   272,   273,   274,
     275,   454,   276,   277,   278,   279,   280,   281,   670,   282,
     283,   284,   285,   945,   286,   287,   288,   289,   290,   291,
     677,   292,   558,   293,   432,   294,   295,   296,   297,   298,
     299,   300,   301,   302,   303,   304,   667,   898,   668,   305,
     552,   306,   952,   307,   761,   776,   416,   777,    82,   111,
     115,   187,   188,    40,    41,    42,    43,    44,    45,    46,
      47,   543,   657,   753,    48,   411
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      66,   105,   145,    80,    33,   371,   774,   458,   472,    67,
     152,    80,   346,    34,   518,   943,   327,    96,    80,   128,
     186,   397,     3,   369,    16,    17,   119,   375,   398,   399,
      67,    33,   424,   425,   193,   194,   195,   328,   186,   125,
      34,   877,   192,   435,   436,   370,   438,   400,   440,   193,
     442,   163,   329,   215,   216,    77,   330,   110,   114,   116,
     117,   118,   976,   878,    68,   522,   193,   401,    93,   983,
     144,    69,   144,   525,    80,   526,    87,   641,   331,   642,
      70,   229,   643,   151,   332,    80,    94,   523,   344,   345,
     574,   575,    69,    80,   202,   203,    88,   110,   402,   326,
      91,    70,   579,   189,   190,   110,   164,   580,   114,   151,
     333,   403,   343,    92,   191,   207,   208,   209,   210,   211,
     212,   213,   214,   144,   144,   383,   384,   527,   327,   135,
     136,   644,   525,   459,   526,    95,   472,   862,   404,   632,
     633,   863,   131,    71,    80,   134,   202,   203,   864,   451,
      80,    72,    73,   100,   339,    74,    80,   204,   144,   144,
      75,   102,   452,   101,    71,   334,   104,    80,   645,    80,
     106,   460,    72,    73,   902,   525,   127,   526,    80,   129,
     130,    75,   327,    16,    17,   107,   527,   108,   335,   376,
     148,   535,   109,   378,   379,   380,   381,   382,   151,   151,
     385,   386,   166,   167,   168,   169,   149,   499,   186,   170,
     171,   326,   132,   133,   150,   692,   693,   694,   695,   696,
     154,    80,   119,   651,   652,   652,    80,   160,   217,   527,
      80,   417,   218,    80,    80,    80,   422,   219,   417,   417,
     405,   427,   220,   221,    80,   431,   222,   377,   434,   417,
     417,   223,   417,   391,   417,   545,   417,   225,    80,   230,
     202,   203,   340,   409,   656,   326,   653,   449,   486,   487,
     336,   204,   637,   406,   172,   865,   577,   578,   579,   173,
     866,   666,   410,   580,   704,   485,   453,   867,   500,   501,
     844,   502,   174,   482,   544,   655,    67,   175,   176,   868,
     546,   328,   547,   548,    80,   456,   549,   550,    80,    80,
     327,   551,    80,   177,   178,   553,   329,   554,   555,   556,
     330,   605,   557,   560,   473,   474,   475,   476,   477,   478,
     479,   480,   481,   561,   562,   563,    80,   576,   144,   144,
     179,   180,   331,   606,   607,   577,   578,   579,   332,   608,
     609,   161,   580,   564,   565,   566,   567,   511,    69,   568,
     181,   569,   723,   869,   719,   570,   503,    70,   119,   504,
     505,   610,    49,    50,   333,   571,   122,   123,   124,   119,
     724,   125,   572,   542,   647,   120,   121,   122,   123,   124,
     573,   126,   125,   326,   581,    80,   534,   417,    80,   611,
     582,    80,   126,   583,    51,   584,    80,   206,   585,    52,
     586,   639,   654,    33,   675,   676,   703,   705,   706,   707,
      53,   182,    34,   512,   708,   709,   725,   710,    80,   334,
      71,   612,   513,   739,   740,   514,   613,   742,    72,    73,
     743,   750,   162,   614,   754,   183,   762,    75,   763,   615,
     764,   765,   335,   515,   627,   628,   766,   630,   770,   616,
     771,   772,   453,   453,   641,    54,   642,   119,    55,   643,
     617,   618,   619,   120,   121,   122,   123,   124,   773,   620,
     125,    80,   778,   726,   588,   779,   780,   781,    80,    80,
     126,    80,    80,   506,   782,   783,   784,   785,   647,   786,
      67,   787,   603,   727,   604,   580,   788,    56,   789,   629,
     621,   516,   790,   636,   791,   819,   135,   136,   644,   820,
     483,    80,    80,    80,   821,   830,   831,   837,   517,   843,
     129,   130,   845,   861,   336,    57,   871,    58,   872,   622,
     623,   873,    80,    33,   874,   883,    80,    80,   888,   889,
     890,    80,    34,    80,   891,    59,   892,    60,   166,   167,
     168,   169,    69,   896,   669,   170,   171,    80,   897,   899,
     900,    70,   901,   905,   909,   624,   910,    80,   911,   912,
     913,   914,    80,   932,   933,   939,   947,   453,   453,   453,
     453,   453,   958,    61,   959,    62,   960,   961,   711,   962,
      63,   715,   716,   717,   718,    64,   720,   721,   722,   728,
     729,   730,   731,   732,   733,   734,   735,   736,   737,   738,
     371,    65,   963,   968,   969,   970,   971,   105,   972,   974,
     172,   975,   977,   979,    71,   173,   980,    80,    80,   981,
      80,   982,    72,    73,   984,   985,   342,   986,   174,   992,
     751,    75,   119,   175,   176,   599,   993,    33,   120,   121,
     122,   123,   124,  1010,  1011,   125,    34,  1013,  1014,   177,
     178,    80,    80,    80,  1015,   126,    81,    83,    84,    85,
      86,   769,   813,   814,  1016,   816,  1017,   818,  1019,  1021,
     488,    99,  1024,  1034,   119,   103,   179,   180,  1035,  1037,
     120,   121,   122,   123,   124,   119,    80,   125,   372,   450,
     801,   120,   121,   122,   123,   124,   181,   126,   125,   805,
     806,   807,   808,   809,  1038,   542,  1040,  1043,   126,   367,
     373,   269,   489,   640,   119,   387,   530,   510,   817,   636,
     120,   121,   122,   123,   124,   815,   893,   125,   638,   768,
     803,    80,    80,    80,    80,    33,     0,   126,     0,   833,
      80,     0,   884,     0,    34,     0,   839,     0,     0,     0,
     824,     0,     0,     0,   847,     0,   849,   182,   851,   119,
     853,   490,    80,     0,     0,   120,   121,   122,   123,   124,
     507,     0,   125,     0,     0,     0,     0,     0,   876,     0,
     879,   183,   126,     0,   508,     0,   880,   881,   882,   491,
     366,     0,     0,     0,     0,     0,     0,   227,   228,   887,
       0,     0,     0,     0,     0,   492,   931,   493,     0,     0,
     494,     0,     0,     0,   348,   349,   350,   351,   352,    80,
     354,    80,   356,   357,   358,   359,   360,   361,   362,   363,
       0,   365,   495,     0,   368,     0,     0,     0,   916,   119,
      49,    50,   601,     0,     0,   120,   121,   122,   123,   124,
     928,     0,   125,   755,   929,   966,     0,   135,   136,   496,
       0,   576,   126,     0,   602,    80,   691,     0,    80,   577,
     578,   579,    51,     0,    80,     0,   580,    52,     0,     0,
     413,   414,     0,     0,   418,     0,    80,     0,    53,   423,
       0,     0,   426,     0,   428,   429,   497,    49,    50,   433,
       0,     0,     0,   437,     0,   439,   965,   441,     0,   443,
       0,   445,   446,   447,   448,    80,     0,     0,     0,     0,
       0,    80,     0,     0,     0,     0,   605,     0,    80,    51,
       0,     0,     0,    54,    52,     0,    55,     0,    80,     0,
       0,     0,     0,     0,    80,    53,     0,     0,   606,   607,
       0,     0,     0,  1002,   608,   609,   756,     0,   457,     0,
    1007,     0,   463,   464,     0,   466,   467,   468,   469,   470,
       0,     0,   119,     0,     0,    56,   610,     0,   120,   121,
     122,   123,   124,   713,     0,   125,     0,     0,    78,    80,
      54,     0,     0,    55,  1030,   126,  1032,   714,     0,     0,
       0,     0,   757,    57,   611,    58,     0,     0,     0,     0,
       0,     0,     0,     0,  1045,  1046,  1047,  1048,     0,     0,
       0,   119,  -363,    59,   758,    60,  1051,   120,   121,   122,
     123,   124,    56,     0,   125,     0,   612,     0,     0,   521,
       0,   613,     0,     0,   126,   531,   532,     0,   614,     0,
       0,   537,     0,     0,   615,     0,     0,   539,     0,     0,
      57,    61,    58,    62,   616,     0,     0,     0,    63,     0,
       0,    49,    50,    64,     0,   617,   618,   619,     0,     0,
      59,     0,    60,     0,   620,     0,   119,     0,     0,    65,
      49,    50,   120,   121,   122,   123,   124,     0,     0,   125,
       0,     0,     0,    51,     0,     0,     0,     0,    52,   126,
       0,   158,     0,     0,     0,   621,     0,     0,    61,    53,
      62,     0,    51,     0,     0,    63,     0,    52,     0,     0,
      64,     0,     0,     0,     0,     0,     0,     0,    53,   590,
       0,     0,   593,     0,   622,   623,    65,     0,   596,   597,
     598,     0,   119,     0,     0,     0,     0,     0,   120,   121,
     122,   123,   124,     0,    54,   125,     0,    55,     0,    16,
      17,     0,     0,     0,   308,   126,     0,   205,    19,    20,
     626,    89,     0,    54,     0,     0,    55,     0,     0,     0,
       0,     0,   658,     0,     0,   661,   662,   663,     0,     0,
       0,     0,   671,     0,   673,     0,    56,   309,   310,   678,
     679,   680,   681,   682,   683,   684,   685,   686,   687,     0,
     689,   690,     0,     0,     0,    56,     0,     0,     0,     0,
     698,   699,   700,   701,    57,     0,    58,     0,     0,     0,
       0,   129,   130,     0,     0,     0,     0,     0,   131,   132,
     133,   134,     0,    57,    59,    58,    60,     0,   135,   136,
       0,   119,     0,     0,   308,     0,     0,   120,   121,   122,
     123,   124,   119,    59,   125,    60,     0,     0,   120,   121,
     122,   123,   124,   875,   126,   125,   374,   745,     0,     0,
       0,   748,    61,     0,    62,   126,     0,   309,   310,    63,
       0,     0,     0,     0,    64,     0,     0,     0,     0,     0,
       0,    61,     0,    62,   767,     0,   311,   312,    63,   313,
      65,   314,   315,    64,   119,     0,     0,     0,     0,     0,
     120,   121,   122,   123,   124,   119,     0,   125,     0,    65,
       0,   120,   121,   122,   123,   124,     0,   126,   125,   741,
       0,   792,     0,   794,   795,   796,   797,   798,   126,   131,
     744,     0,   134,     0,     0,     0,   119,     0,     0,   135,
     136,   316,   120,   121,   122,   123,   124,     0,     0,   125,
     317,   318,     0,   319,   320,     0,     0,     0,     0,   126,
       0,   802,     0,     0,   321,     0,     0,     0,   822,     0,
       0,     0,   825,     0,     0,     0,   311,   312,   471,   313,
     832,   314,   315,   835,   836,     0,     0,     0,   838,   840,
     841,   842,     0,     0,     0,     0,   846,   848,     0,   850,
       0,   852,     0,   854,     0,   856,   857,   858,   859,   860,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   131,
       0,     0,   134,     0,     0,     0,     0,     0,     0,   135,
     136,   316,     0,     0,     0,     0,     0,     0,   885,   886,
     317,   318,     0,   319,   320,     0,     0,     0,     0,   894,
     119,     0,     0,     0,   321,     0,   120,   121,   122,   123,
     124,     0,   903,   125,     0,     0,     0,     0,     0,     0,
       0,     0,    79,   126,     0,   804,     0,     0,     0,   915,
      90,   917,   918,   919,   920,   921,   922,    98,     0,   923,
     924,   925,   926,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   934,     0,   936,   937,
       0,     0,     0,   940,   941,   942,     0,   944,     0,   946,
       0,     0,     0,   948,   949,   950,   951,   119,     0,   954,
     955,   956,   957,   120,   121,   122,   123,   124,     0,     0,
     125,     0,     0,   147,     0,     0,     0,     0,     0,     0,
     126,   967,   810,     0,   153,     0,     0,     0,     0,     0,
     119,     0,   159,     0,     0,   978,   120,   121,   122,   123,
     124,     0,     0,   125,     0,     0,   987,   988,   989,   990,
       0,     0,     0,   126,     0,   811,     0,   996,   997,   998,
     999,     0,  1000,  1001,     0,     0,     0,  1003,  1004,  1005,
    1006,     0,     0,  1008,  1009,     0,     0,     0,     0,     0,
       0,  1012,     0,   226,     0,     0,     0,     0,     0,   231,
       0,     0,     0,     0,     0,   341,     0,     0,  1022,  1023,
       0,     0,  1026,  1027,  1028,  1029,   353,  1031,   355,  1033,
       0,     0,  1036,     0,     0,     0,     0,   364,     0,     0,
     119,     0,  1042,     0,     0,  1044,   120,   121,   122,   123,
     124,  1049,     0,   125,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   126,   232,   812,     0,     0,   233,     0,
     234,     0,     0,     0,     0,   235,   236,     0,     0,     0,
     408,     0,     0,   237,     0,   412,     0,     0,     0,   415,
     238,     0,   419,   420,   421,     0,     0,     0,     6,     0,
       7,   239,   119,   430,     0,     0,     0,     0,   120,   121,
     122,   123,   124,    49,    50,   125,     0,   444,     0,     0,
       0,    49,    50,     0,     0,   126,     0,   823,     0,   240,
       0,     0,   241,     0,   242,     0,     0,   243,   244,     0,
       0,     0,   245,     0,     0,    51,     0,     0,     0,     0,
      52,   246,     0,    51,     0,     0,    15,     0,    52,     0,
       0,    53,     0,   455,     0,     0,     0,   461,   462,    53,
       0,   465,     0,     0,   247,     0,     0,     0,     0,     0,
       0,     0,   248,   249,     0,     0,   250,     0,     0,     0,
       0,     0,     0,     0,     0,   484,   251,   252,     0,     0,
       0,     0,     0,     0,    97,   253,    54,     0,     0,    55,
       0,     0,     0,     0,    54,     0,     0,    55,     0,     0,
       0,   254,   255,   256,   257,   258,   259,     0,     0,     0,
     260,   261,    49,    50,     0,     0,    22,     0,     0,   262,
       0,     0,     0,     0,     0,     0,   263,     0,    56,     0,
     264,     0,     0,     0,   533,     0,    56,   536,    26,     0,
     538,   265,     0,     0,    51,   540,     0,     0,     0,    52,
       0,     0,     0,     0,     0,     0,    57,     0,    58,     0,
      53,     0,   119,     0,    57,     0,    58,   559,   120,   121,
     122,   123,   124,     0,     0,   125,    59,     0,    60,     0,
       0,     0,     0,     0,    59,   126,    60,   895,   119,     0,
       0,     0,     0,     0,   120,   121,   122,   123,   124,     0,
     266,   125,     0,     0,     0,    54,     0,     0,    55,     0,
       0,   126,     0,  -362,    61,     0,    62,     0,     0,     0,
     587,    63,    61,     0,   112,     0,    64,   591,   592,    63,
     594,   595,     0,     0,    64,     0,   113,     0,     0,     0,
       0,     0,    65,     0,     0,   119,     0,    56,     0,     0,
      65,   120,   121,   122,   123,   124,     0,     0,   125,     0,
     648,   649,   650,     0,     0,     0,     0,     0,   126,     0,
     906,     0,     0,     0,     0,    57,     0,    58,     0,     0,
       0,   660,     0,     0,     0,   664,   665,     0,     0,     0,
     672,     0,   674,     0,   119,    59,     0,    60,     0,     0,
     120,   121,   122,   123,   124,   119,   688,   125,     0,     4,
       0,   120,   121,   122,   123,   124,   697,   126,   125,   907,
       0,   702,     0,     0,     0,     0,     0,     0,   126,     0,
     908,     0,     0,    61,     4,    62,     0,     0,     0,     0,
      63,     5,     0,     0,     6,    64,     7,     0,     0,     0,
     119,     0,     0,     8,     0,     9,   120,   121,   122,   123,
     124,    65,     0,   125,    10,     0,     5,     0,     0,     6,
       0,     7,    11,   126,     0,   930,   746,   747,     8,   749,
       9,     0,    12,     0,     0,     0,     0,     0,     0,    10,
      13,     0,     0,     0,    14,     0,     0,    11,     0,     0,
       0,     0,    15,     0,     0,     0,     0,    12,     0,     0,
     559,   775,     0,     0,     0,    13,     0,     0,     0,    14,
       0,    16,    17,     0,     0,     0,    18,    15,     0,   119,
      19,    20,     0,     4,     0,   120,   121,   122,   123,   124,
       0,    21,   125,     0,     0,   799,    16,    17,     0,     0,
       0,    18,   126,     0,  1018,    19,    20,     0,     0,     0,
       0,     0,     0,     0,     0,     5,    21,     0,     6,     0,
       7,     0,     0,     0,     0,     0,     0,     8,     0,     9,
       0,     0,    22,     0,     0,     0,    23,    24,    10,     0,
     826,   827,   828,   829,     0,    25,    11,     0,     0,   834,
       0,     0,     0,     0,    26,     0,    12,    22,    27,     0,
       0,    23,    24,     0,    13,     0,     0,     0,    14,     0,
      25,   855,     0,     0,     0,     0,    15,     0,    28,    26,
       0,     0,     0,    27,   119,     0,     0,     0,     0,     0,
     120,   121,   122,   123,   124,    16,    17,   125,     0,   752,
      18,     0,     0,    28,    19,    20,     0,   126,     0,  1020,
       0,   119,     0,     0,     0,    21,     0,   120,   121,   122,
     123,   124,     0,   541,   125,     0,     0,     0,   559,     0,
     904,     0,   119,     0,   126,     0,  1039,     0,   120,   121,
     122,   123,   124,     0,     0,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   126,    22,  1041,   119,   927,
      23,    24,     0,     0,   120,   121,   122,   123,   124,    25,
       0,   125,     0,     0,   935,     0,     0,   938,    26,     0,
       0,   126,    27,  1050,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   953,     0,   119,     0,     0,
     196,     0,    28,   120,   121,   122,   123,   124,   197,   198,
     125,     0,     0,     0,     0,   199,   200,   201,     0,   119,
     126,     0,     0,     0,   973,   120,   121,   122,   123,   124,
     197,   198,   125,     0,     0,     0,     0,   199,   200,   201,
       0,     0,   126,     0,     0,     0,     0,   991,   119,     0,
       0,   600,     0,   995,   120,   121,   122,   123,   124,   119,
       0,   125,   712,     0,     0,   120,   121,   122,   123,   124,
     119,   126,   125,   800,     0,     0,   120,   121,   122,   123,
     124,   119,   126,   125,   964,     0,     0,   120,   121,   122,
     123,   124,     0,   126,   125,     0,     0,     0,  1025,     0,
       0,     0,     0,     0,   126
};

static const yytype_int16 yycheck[] =
{
       4,    29,    76,     7,     2,   188,   675,    73,   322,    70,
      87,    15,   257,     2,   390,   898,   156,    21,    22,    68,
     254,     7,     0,   257,   131,   132,   254,   257,    14,    15,
      70,    29,   242,   243,   279,   112,   113,     3,   254,   267,
      29,   257,   255,   253,   254,   279,   256,    33,   258,   279,
     260,   100,    18,   127,   128,   258,    22,    61,    62,    63,
      64,    65,   945,   279,   125,   257,   279,    53,   198,   952,
      74,   132,    76,   168,    78,   170,   256,   168,    44,   170,
     141,   257,   173,    87,    50,    89,   216,   279,   162,   163,
     451,   452,   132,    97,   270,   271,   198,   101,    84,   156,
     258,   141,   262,   107,   108,   109,   101,   267,   112,   113,
      76,    97,   161,   258,   109,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   202,   203,   222,   268,   220,
     221,   222,   168,   199,   170,   258,   450,    33,   124,   515,
     516,    37,   210,   204,   148,   213,   270,   271,    44,   256,
     154,   212,   213,    51,   158,   216,   160,   281,   162,   163,
     221,   258,   269,   256,   204,   131,   258,   171,   259,   173,
     256,   237,   212,   213,   843,   168,   216,   170,   182,   203,
     204,   221,   322,   131,   132,   256,   222,   256,   154,   193,
     279,   401,   256,   197,   198,   199,   200,   201,   202,   203,
     204,   205,     7,     8,     9,    10,   279,   347,   254,    14,
      15,   268,   211,   212,   279,   576,   577,   578,   579,   580,
     279,   225,   254,   259,   529,   530,   230,   279,   258,   222,
     234,   235,   258,   237,   238,   239,   240,   258,   242,   243,
     226,   245,   256,   258,   248,   249,   258,   257,   252,   253,
     254,   258,   256,   254,   258,   280,   260,   279,   262,   279,
     270,   271,   279,   279,    64,   322,   259,   259,   342,   343,
     236,   281,   259,   259,    79,   171,   260,   261,   262,    84,
     176,    40,   279,   267,   258,   279,   275,   183,   279,   279,
     233,   279,    97,   259,   279,   541,    70,   102,   103,   195,
     279,     3,   279,   279,   308,   309,   279,   279,   312,   313,
     450,   279,   316,   118,   119,   279,    18,   279,   279,   279,
      22,     5,   279,   279,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   279,   279,   279,   340,   252,   342,   343,
     145,   146,    44,    27,    28,   260,   261,   262,    50,    33,
      34,   125,   267,   279,   279,   279,   279,    30,   132,   279,
     165,   279,    23,   259,   256,   279,   370,   141,   254,   373,
     374,    55,    33,    34,    76,   279,   262,   263,   264,   254,
      41,   267,   279,   411,   524,   260,   261,   262,   263,   264,
     279,   277,   267,   450,   279,   399,   400,   401,   402,    83,
     279,   405,   277,   279,    65,   279,   410,   282,   279,    70,
     279,   279,   279,   411,   279,   279,   279,   279,   279,   279,
      81,   226,   411,    96,   279,   279,    87,   279,   432,   131,
     204,   115,   105,   279,   279,   108,   120,   279,   212,   213,
     279,   279,   216,   127,   279,   250,   279,   221,   279,   133,
     279,   279,   154,   126,   511,   512,   279,   514,   279,   143,
     279,   279,   451,   452,   168,   126,   170,   254,   129,   173,
     154,   155,   156,   260,   261,   262,   263,   264,   279,   163,
     267,   485,   279,   144,   488,   279,   279,   279,   492,   493,
     277,   495,   496,   280,   279,   279,   279,   279,   638,   279,
      70,   279,   506,   164,   508,   267,   279,   168,   279,   513,
     194,   184,   279,   517,   279,   279,   220,   221,   222,   279,
     337,   525,   526,   527,   279,   279,   279,   279,   201,   279,
     203,   204,   279,   279,   236,   196,   279,   198,   279,   223,
     224,   279,   546,   541,   279,   279,   550,   551,   279,   279,
     279,   555,   541,   557,   279,   216,   279,   218,     7,     8,
       9,    10,   132,   279,   553,    14,    15,   571,   279,   279,
     279,   141,   279,   279,   279,   259,   279,   581,   279,   279,
     279,   279,   586,   279,   279,   279,   279,   576,   577,   578,
     579,   580,   279,   254,   279,   256,   279,   279,   602,   279,
     261,   605,   606,   607,   608,   266,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   620,   621,   622,   623,
     803,   282,   255,   279,   279,   279,   279,   655,   279,   279,
      79,   279,   279,   279,   204,    84,   279,   641,   642,   279,
     644,   279,   212,   213,   279,   279,   216,   279,    97,   255,
     654,   221,   254,   102,   103,   257,   279,   655,   260,   261,
     262,   263,   264,   279,   279,   267,   655,   279,   279,   118,
     119,   675,   676,   677,   279,   277,     8,     9,    10,    11,
      12,   670,   739,   740,   279,   742,   279,   744,   279,   279,
      33,    23,   279,   279,   254,    27,   145,   146,   255,   279,
     260,   261,   262,   263,   264,   254,   710,   267,   257,   268,
     714,   260,   261,   262,   263,   264,   165,   277,   267,   723,
     724,   725,   726,   727,   279,   753,   279,   279,   277,   185,
     279,   155,    75,   523,   254,   255,   396,   389,   743,   743,
     260,   261,   262,   263,   264,   741,   830,   267,   520,   668,
     719,   755,   756,   757,   758,   753,    -1,   277,    -1,   763,
     764,    -1,   819,    -1,   753,    -1,   770,    -1,    -1,    -1,
     753,    -1,    -1,    -1,   778,    -1,   780,   226,   782,   254,
     784,   124,   786,    -1,    -1,   260,   261,   262,   263,   264,
     265,    -1,   267,    -1,    -1,    -1,    -1,    -1,   802,    -1,
     804,   250,   277,    -1,   279,    -1,   810,   811,   812,   152,
     259,    -1,    -1,    -1,    -1,    -1,    -1,   149,   150,   823,
      -1,    -1,    -1,    -1,    -1,   168,   883,   170,    -1,    -1,
     173,    -1,    -1,    -1,   166,   167,   168,   169,   170,   843,
     172,   845,   174,   175,   176,   177,   178,   179,   180,   181,
      -1,   183,   195,    -1,   186,    -1,    -1,    -1,   862,   254,
      33,    34,   257,    -1,    -1,   260,   261,   262,   263,   264,
     874,    -1,   267,    46,   878,   932,    -1,   220,   221,   222,
      -1,   252,   277,    -1,   279,   889,   257,    -1,   892,   260,
     261,   262,    65,    -1,   898,    -1,   267,    70,    -1,    -1,
     232,   233,    -1,    -1,   236,    -1,   910,    -1,    81,   241,
      -1,    -1,   244,    -1,   246,   247,   259,    33,    34,   251,
      -1,    -1,    -1,   255,    -1,   257,   930,   259,    -1,   261,
      -1,   263,   264,   265,   266,   939,    -1,    -1,    -1,    -1,
      -1,   945,    -1,    -1,    -1,    -1,     5,    -1,   952,    65,
      -1,    -1,    -1,   126,    70,    -1,   129,    -1,   962,    -1,
      -1,    -1,    -1,    -1,   968,    81,    -1,    -1,    27,    28,
      -1,    -1,    -1,   977,    33,    34,   149,    -1,   310,    -1,
     984,    -1,   314,   315,    -1,   317,   318,   319,   320,   321,
      -1,    -1,   254,    -1,    -1,   168,    55,    -1,   260,   261,
     262,   263,   264,   265,    -1,   267,    -1,    -1,   124,  1013,
     126,    -1,    -1,   129,  1018,   277,  1020,   279,    -1,    -1,
      -1,    -1,   195,   196,    83,   198,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1038,  1039,  1040,  1041,    -1,    -1,
      -1,   254,   255,   216,   217,   218,  1050,   260,   261,   262,
     263,   264,   168,    -1,   267,    -1,   115,    -1,    -1,   391,
      -1,   120,    -1,    -1,   277,   397,   398,    -1,   127,    -1,
      -1,   403,    -1,    -1,   133,    -1,    -1,   409,    -1,    -1,
     196,   254,   198,   256,   143,    -1,    -1,    -1,   261,    -1,
      -1,    33,    34,   266,    -1,   154,   155,   156,    -1,    -1,
     216,    -1,   218,    -1,   163,    -1,   254,    -1,    -1,   282,
      33,    34,   260,   261,   262,   263,   264,    -1,    -1,   267,
      -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    70,   277,
      -1,   279,    -1,    -1,    -1,   194,    -1,    -1,   254,    81,
     256,    -1,    65,    -1,    -1,   261,    -1,    70,    -1,    -1,
     266,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    81,   491,
      -1,    -1,   494,    -1,   223,   224,   282,    -1,   500,   501,
     502,    -1,   254,    -1,    -1,    -1,    -1,    -1,   260,   261,
     262,   263,   264,    -1,   126,   267,    -1,   129,    -1,   131,
     132,    -1,    -1,    -1,    25,   277,    -1,   279,   140,   141,
     259,   124,    -1,   126,    -1,    -1,   129,    -1,    -1,    -1,
      -1,    -1,   544,    -1,    -1,   547,   548,   549,    -1,    -1,
      -1,    -1,   554,    -1,   556,    -1,   168,    58,    59,   561,
     562,   563,   564,   565,   566,   567,   568,   569,   570,    -1,
     572,   573,    -1,    -1,    -1,   168,    -1,    -1,    -1,    -1,
     582,   583,   584,   585,   196,    -1,   198,    -1,    -1,    -1,
      -1,   203,   204,    -1,    -1,    -1,    -1,    -1,   210,   211,
     212,   213,    -1,   196,   216,   198,   218,    -1,   220,   221,
      -1,   254,    -1,    -1,    25,    -1,    -1,   260,   261,   262,
     263,   264,   254,   216,   267,   218,    -1,    -1,   260,   261,
     262,   263,   264,   265,   277,   267,   279,   639,    -1,    -1,
      -1,   643,   254,    -1,   256,   277,    -1,    58,    59,   261,
      -1,    -1,    -1,    -1,   266,    -1,    -1,    -1,    -1,    -1,
      -1,   254,    -1,   256,   666,    -1,   167,   168,   261,   170,
     282,   172,   173,   266,   254,    -1,    -1,    -1,    -1,    -1,
     260,   261,   262,   263,   264,   254,    -1,   267,    -1,   282,
      -1,   260,   261,   262,   263,   264,    -1,   277,   267,   279,
      -1,   703,    -1,   705,   706,   707,   708,   709,   277,   210,
     279,    -1,   213,    -1,    -1,    -1,   254,    -1,    -1,   220,
     221,   222,   260,   261,   262,   263,   264,    -1,    -1,   267,
     231,   232,    -1,   234,   235,    -1,    -1,    -1,    -1,   277,
      -1,   279,    -1,    -1,   245,    -1,    -1,    -1,   750,    -1,
      -1,    -1,   754,    -1,    -1,    -1,   167,   168,   259,   170,
     762,   172,   173,   765,   766,    -1,    -1,    -1,   770,   771,
     772,   773,    -1,    -1,    -1,    -1,   778,   779,    -1,   781,
      -1,   783,    -1,   785,    -1,   787,   788,   789,   790,   791,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   210,
      -1,    -1,   213,    -1,    -1,    -1,    -1,    -1,    -1,   220,
     221,   222,    -1,    -1,    -1,    -1,    -1,    -1,   820,   821,
     231,   232,    -1,   234,   235,    -1,    -1,    -1,    -1,   831,
     254,    -1,    -1,    -1,   245,    -1,   260,   261,   262,   263,
     264,    -1,   844,   267,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,     7,   277,    -1,   279,    -1,    -1,    -1,   861,
      15,   863,   864,   865,   866,   867,   868,    22,    -1,   871,
     872,   873,   874,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   888,    -1,   890,   891,
      -1,    -1,    -1,   895,   896,   897,    -1,   899,    -1,   901,
      -1,    -1,    -1,   905,   906,   907,   908,   254,    -1,   911,
     912,   913,   914,   260,   261,   262,   263,   264,    -1,    -1,
     267,    -1,    -1,    78,    -1,    -1,    -1,    -1,    -1,    -1,
     277,   933,   279,    -1,    89,    -1,    -1,    -1,    -1,    -1,
     254,    -1,    97,    -1,    -1,   947,   260,   261,   262,   263,
     264,    -1,    -1,   267,    -1,    -1,   958,   959,   960,   961,
      -1,    -1,    -1,   277,    -1,   279,    -1,   969,   970,   971,
     972,    -1,   974,   975,    -1,    -1,    -1,   979,   980,   981,
     982,    -1,    -1,   985,   986,    -1,    -1,    -1,    -1,    -1,
      -1,   993,    -1,   148,    -1,    -1,    -1,    -1,    -1,   154,
      -1,    -1,    -1,    -1,    -1,   160,    -1,    -1,  1010,  1011,
      -1,    -1,  1014,  1015,  1016,  1017,   171,  1019,   173,  1021,
      -1,    -1,  1024,    -1,    -1,    -1,    -1,   182,    -1,    -1,
     254,    -1,  1034,    -1,    -1,  1037,   260,   261,   262,   263,
     264,  1043,    -1,   267,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   277,    20,   279,    -1,    -1,    24,    -1,
      26,    -1,    -1,    -1,    -1,    31,    32,    -1,    -1,    -1,
     225,    -1,    -1,    39,    -1,   230,    -1,    -1,    -1,   234,
      46,    -1,   237,   238,   239,    -1,    -1,    -1,    54,    -1,
      56,    57,   254,   248,    -1,    -1,    -1,    -1,   260,   261,
     262,   263,   264,    33,    34,   267,    -1,   262,    -1,    -1,
      -1,    33,    34,    -1,    -1,   277,    -1,   279,    -1,    85,
      -1,    -1,    88,    -1,    90,    -1,    -1,    93,    94,    -1,
      -1,    -1,    98,    -1,    -1,    65,    -1,    -1,    -1,    -1,
      70,   107,    -1,    65,    -1,    -1,   112,    -1,    70,    -1,
      -1,    81,    -1,   308,    -1,    -1,    -1,   312,   313,    81,
      -1,   316,    -1,    -1,   130,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   138,   139,    -1,    -1,   142,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   340,   152,   153,    -1,    -1,
      -1,    -1,    -1,    -1,   124,   161,   126,    -1,    -1,   129,
      -1,    -1,    -1,    -1,   126,    -1,    -1,   129,    -1,    -1,
      -1,   177,   178,   179,   180,   181,   182,    -1,    -1,    -1,
     186,   187,    33,    34,    -1,    -1,   192,    -1,    -1,   195,
      -1,    -1,    -1,    -1,    -1,    -1,   202,    -1,   168,    -1,
     206,    -1,    -1,    -1,   399,    -1,   168,   402,   214,    -1,
     405,   217,    -1,    -1,    65,   410,    -1,    -1,    -1,    70,
      -1,    -1,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,
      81,    -1,   254,    -1,   196,    -1,   198,   432,   260,   261,
     262,   263,   264,    -1,    -1,   267,   216,    -1,   218,    -1,
      -1,    -1,    -1,    -1,   216,   277,   218,   279,   254,    -1,
      -1,    -1,    -1,    -1,   260,   261,   262,   263,   264,    -1,
     276,   267,    -1,    -1,    -1,   126,    -1,    -1,   129,    -1,
      -1,   277,    -1,   279,   254,    -1,   256,    -1,    -1,    -1,
     485,   261,   254,    -1,   256,    -1,   266,   492,   493,   261,
     495,   496,    -1,    -1,   266,    -1,   268,    -1,    -1,    -1,
      -1,    -1,   282,    -1,    -1,   254,    -1,   168,    -1,    -1,
     282,   260,   261,   262,   263,   264,    -1,    -1,   267,    -1,
     525,   526,   527,    -1,    -1,    -1,    -1,    -1,   277,    -1,
     279,    -1,    -1,    -1,    -1,   196,    -1,   198,    -1,    -1,
      -1,   546,    -1,    -1,    -1,   550,   551,    -1,    -1,    -1,
     555,    -1,   557,    -1,   254,   216,    -1,   218,    -1,    -1,
     260,   261,   262,   263,   264,   254,   571,   267,    -1,    19,
      -1,   260,   261,   262,   263,   264,   581,   277,   267,   279,
      -1,   586,    -1,    -1,    -1,    -1,    -1,    -1,   277,    -1,
     279,    -1,    -1,   254,    19,   256,    -1,    -1,    -1,    -1,
     261,    51,    -1,    -1,    54,   266,    56,    -1,    -1,    -1,
     254,    -1,    -1,    63,    -1,    65,   260,   261,   262,   263,
     264,   282,    -1,   267,    74,    -1,    51,    -1,    -1,    54,
      -1,    56,    82,   277,    -1,   279,   641,   642,    63,   644,
      65,    -1,    92,    -1,    -1,    -1,    -1,    -1,    -1,    74,
     100,    -1,    -1,    -1,   104,    -1,    -1,    82,    -1,    -1,
      -1,    -1,   112,    -1,    -1,    -1,    -1,    92,    -1,    -1,
     675,   676,    -1,    -1,    -1,   100,    -1,    -1,    -1,   104,
      -1,   131,   132,    -1,    -1,    -1,   136,   112,    -1,   254,
     140,   141,    -1,    19,    -1,   260,   261,   262,   263,   264,
      -1,   151,   267,    -1,    -1,   710,   131,   132,    -1,    -1,
      -1,   136,   277,    -1,   279,   140,   141,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    51,   151,    -1,    54,    -1,
      56,    -1,    -1,    -1,    -1,    -1,    -1,    63,    -1,    65,
      -1,    -1,   192,    -1,    -1,    -1,   196,   197,    74,    -1,
     755,   756,   757,   758,    -1,   205,    82,    -1,    -1,   764,
      -1,    -1,    -1,    -1,   214,    -1,    92,   192,   218,    -1,
      -1,   196,   197,    -1,   100,    -1,    -1,    -1,   104,    -1,
     205,   786,    -1,    -1,    -1,    -1,   112,    -1,   238,   214,
      -1,    -1,    -1,   218,   254,    -1,    -1,    -1,    -1,    -1,
     260,   261,   262,   263,   264,   131,   132,   267,    -1,   259,
     136,    -1,    -1,   238,   140,   141,    -1,   277,    -1,   279,
      -1,   254,    -1,    -1,    -1,   151,    -1,   260,   261,   262,
     263,   264,    -1,   258,   267,    -1,    -1,    -1,   843,    -1,
     845,    -1,   254,    -1,   277,    -1,   279,    -1,   260,   261,
     262,   263,   264,    -1,    -1,   267,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   277,   192,   279,   254,   874,
     196,   197,    -1,    -1,   260,   261,   262,   263,   264,   205,
      -1,   267,    -1,    -1,   889,    -1,    -1,   892,   214,    -1,
      -1,   277,   218,   279,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   910,    -1,   254,    -1,    -1,
     257,    -1,   238,   260,   261,   262,   263,   264,   265,   266,
     267,    -1,    -1,    -1,    -1,   272,   273,   274,    -1,   254,
     277,    -1,    -1,    -1,   939,   260,   261,   262,   263,   264,
     265,   266,   267,    -1,    -1,    -1,    -1,   272,   273,   274,
      -1,    -1,   277,    -1,    -1,    -1,    -1,   962,   254,    -1,
      -1,   257,    -1,   968,   260,   261,   262,   263,   264,   254,
      -1,   267,   257,    -1,    -1,   260,   261,   262,   263,   264,
     254,   277,   267,   257,    -1,    -1,   260,   261,   262,   263,
     264,   254,   277,   267,   257,    -1,    -1,   260,   261,   262,
     263,   264,    -1,   277,   267,    -1,    -1,    -1,  1013,    -1,
      -1,    -1,    -1,    -1,   277
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   284,   285,     0,    19,    51,    54,    56,    63,    65,
      74,    82,    92,   100,   104,   112,   131,   132,   136,   140,
     141,   151,   192,   196,   197,   205,   214,   218,   238,   286,
     287,   288,   291,   294,   296,   307,   315,   316,   319,   320,
     406,   407,   408,   409,   410,   411,   412,   413,   417,    33,
      34,    65,    70,    81,   126,   129,   168,   196,   198,   216,
     218,   254,   256,   261,   266,   282,   401,    70,   125,   132,
     141,   204,   212,   213,   216,   221,   289,   258,   124,   398,
     401,   397,   401,   397,   397,   397,   397,   256,   198,   124,
     398,   258,   258,   198,   216,   258,   401,   124,   398,   397,
      51,   256,   258,   397,   258,   287,   256,   256,   256,   256,
     401,   402,   256,   268,   401,   403,   401,   401,   401,   254,
     260,   261,   262,   263,   264,   267,   277,   216,   289,   203,
     204,   210,   211,   212,   213,   220,   221,   290,   294,   296,
     323,   330,   335,   341,   401,   290,   318,   398,   279,   279,
     279,   401,   403,   398,   279,   297,   298,   295,   279,   398,
     279,   125,   216,   289,   402,   317,     7,     8,     9,    10,
      14,    15,    79,    84,    97,   102,   103,   118,   119,   145,
     146,   165,   226,   250,   305,   306,   254,   404,   405,   401,
     401,   402,   255,   279,   403,   403,   257,   265,   266,   272,
     273,   274,   270,   271,   281,   279,   282,   401,   401,   401,
     401,   401,   401,   401,   401,   290,   290,   258,   258,   258,
     256,   258,   258,   258,   314,   279,   398,   397,   397,   257,
     279,   398,    20,    24,    26,    31,    32,    39,    46,    57,
      85,    88,    90,    93,    94,    98,   107,   130,   138,   139,
     142,   152,   153,   161,   177,   178,   179,   180,   181,   182,
     186,   187,   195,   202,   206,   217,   276,   303,   304,   316,
     345,   346,   350,   351,   352,   353,   355,   356,   357,   358,
     359,   360,   362,   363,   364,   365,   367,   368,   369,   370,
     371,   372,   374,   376,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   392,   394,   396,    25,    58,
      59,   167,   168,   170,   172,   173,   222,   231,   232,   234,
     235,   245,   299,   300,   301,   302,   335,   341,     3,    18,
      22,    44,    50,    76,   131,   154,   236,   292,   293,   401,
     279,   398,   216,   289,   290,   290,   257,   312,   397,   397,
     397,   397,   397,   398,   397,   398,   397,   397,   397,   397,
     397,   397,   397,   397,   398,   397,   259,   305,   397,   257,
     279,   404,   257,   279,   279,   257,   401,   257,   401,   401,
     401,   401,   401,   403,   403,   401,   401,   255,   324,   325,
     336,   254,   326,   327,   337,   342,   343,     7,    14,    15,
      33,    53,    84,    97,   124,   226,   259,   313,   398,   279,
     279,   418,   398,   397,   397,   398,   399,   401,   397,   398,
     398,   398,   401,   397,   399,   399,   397,   401,   397,   397,
     398,   401,   377,   397,   401,   399,   399,   397,   399,   397,
     399,   397,   399,   397,   398,   397,   397,   397,   397,   259,
     299,   256,   269,   296,   354,   398,   401,   397,    73,   199,
     237,   398,   398,   397,   397,   398,   397,   397,   397,   397,
     397,   259,   300,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   259,   293,   398,   279,   290,   290,    33,    75,
     124,   152,   168,   170,   173,   195,   222,   259,   310,   341,
     279,   279,   279,   401,   401,   401,   280,   265,   279,   322,
     322,    30,    96,   105,   108,   126,   184,   201,   323,   333,
     334,   397,   257,   279,   331,   168,   170,   222,   339,   340,
     340,   397,   397,   398,   401,   399,   398,   397,   398,   397,
     398,   258,   287,   414,   279,   280,   279,   279,   279,   279,
     279,   279,   393,   279,   279,   279,   279,   279,   375,   398,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   354,   354,   252,   260,   261,   262,
     267,   279,   279,   279,   279,   279,   279,   398,   401,   311,
     397,   398,   398,   397,   398,   398,   397,   397,   397,   257,
     257,   257,   279,   401,   401,     5,    27,    28,    33,    34,
      55,    83,   115,   120,   127,   133,   143,   154,   155,   156,
     163,   194,   223,   224,   259,   321,   259,   335,   335,   401,
     335,   338,   323,   323,   328,   329,   401,   259,   331,   279,
     326,   168,   170,   173,   222,   259,   332,   341,   398,   398,
     398,   259,   339,   259,   279,   286,    64,   415,   397,   347,
     398,   397,   397,   397,   398,   398,    40,   389,   391,   296,
     361,   397,   398,   397,   398,   279,   279,   373,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   397,   398,   397,
     397,   257,   354,   354,   354,   354,   354,   398,   397,   397,
     397,   397,   398,   279,   258,   279,   279,   279,   279,   279,
     279,   401,   257,   265,   279,   401,   401,   401,   401,   256,
     401,   401,   401,    23,    41,    87,   144,   164,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   279,
     279,   279,   279,   279,   279,   397,   398,   398,   397,   398,
     279,   401,   259,   416,   279,    46,   149,   195,   217,   348,
     349,   397,   279,   279,   279,   279,   279,   397,   389,   296,
     279,   279,   279,   279,   375,   398,   398,   400,   279,   279,
     279,   279,   279,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   397,   308,   397,   397,   397,   397,   397,   398,
     257,   401,   279,   405,   279,   401,   401,   401,   401,   401,
     279,   279,   279,   335,   335,   330,   335,   328,   335,   279,
     279,   279,   397,   279,   414,   397,   398,   398,   398,   398,
     279,   279,   397,   401,   398,   397,   397,   279,   397,   401,
     397,   397,   397,   279,   233,   279,   397,   401,   397,   401,
     397,   401,   397,   401,   397,   398,   397,   397,   397,   397,
     397,   279,    33,    37,    44,   171,   176,   183,   195,   259,
     309,   279,   279,   279,   279,   265,   401,   257,   279,   401,
     401,   401,   401,   279,   335,   397,   397,   401,   279,   279,
     279,   279,   279,   349,   397,   279,   279,   279,   390,   279,
     279,   279,   375,   397,   398,   279,   279,   279,   279,   279,
     279,   279,   279,   279,   279,   397,   401,   397,   397,   397,
     397,   397,   397,   397,   397,   397,   397,   398,   401,   401,
     279,   335,   279,   279,   397,   398,   397,   397,   398,   279,
     397,   397,   397,   400,   397,   366,   397,   279,   397,   397,
     397,   397,   395,   398,   397,   397,   397,   397,   279,   279,
     279,   279,   279,   255,   257,   401,   335,   397,   279,   279,
     279,   279,   279,   398,   279,   279,   400,   279,   397,   279,
     279,   279,   279,   400,   279,   279,   279,   397,   397,   397,
     397,   398,   255,   279,   344,   398,   397,   397,   397,   397,
     397,   397,   401,   397,   397,   397,   397,   401,   397,   397,
     279,   279,   397,   279,   279,   279,   279,   279,   279,   279,
     279,   279,   397,   397,   279,   398,   397,   397,   397,   397,
     401,   397,   401,   397,   279,   255,   397,   279,   279,   279,
     279,   279,   397,   279,   397,   401,   401,   401,   401,   397,
     279,   401
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   283,   285,   284,   286,   286,   287,   287,   287,   287,
     287,   287,   287,   287,   287,   287,   287,   287,   287,   287,
     288,   289,   289,   289,   289,   289,   289,   289,   290,   290,
     290,   290,   290,   290,   290,   291,   291,   291,   291,   291,
     291,   291,   291,   292,   292,   293,   293,   293,   293,   293,
     293,   293,   293,   293,   295,   294,   294,   297,   296,   296,
     298,   296,   299,   299,   300,   300,   300,   300,   300,   300,
     300,   300,   300,   300,   300,   300,   300,   301,   301,   301,
     301,   301,   301,   302,   302,   302,   303,   303,   304,   304,
     304,   304,   304,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   304,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   304,   304,   304,   304,   304,   304,   304,   304,
     304,   304,   304,   304,   304,   305,   305,   305,   305,   305,
     305,   305,   305,   305,   305,   305,   305,   305,   305,   305,
     305,   305,   305,   306,   306,   307,   308,   308,   309,   309,
     309,   309,   309,   309,   309,   310,   310,   310,   310,   311,
     310,   310,   310,   310,   310,   310,   310,   312,   312,   313,
     313,   313,   313,   313,   313,   313,   313,   313,   314,   314,
     315,   316,   316,   316,   316,   316,   316,   316,   316,   317,
     316,   316,   316,   316,   316,   318,   316,   319,   319,   320,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     321,   321,   321,   321,   321,   321,   321,   321,   321,   321,
     322,   322,   324,   323,   323,   325,   323,   326,   327,   327,
     328,   329,   329,   330,   330,   331,   331,   332,   332,   332,
     332,   332,   332,   333,   334,   334,   334,   334,   334,   334,
     334,   334,   336,   335,   335,   337,   335,   338,   338,   339,
     339,   339,   339,   340,   340,   342,   341,   341,   343,   341,
     344,   344,   345,   347,   346,   348,   348,   349,   349,   349,
     349,   349,   350,   351,   353,   352,   354,   354,   354,   354,
     354,   354,   354,   354,   355,   356,   356,   357,   357,   358,
     358,   359,   360,   361,   361,   362,   363,   363,   364,   366,
     365,   367,   368,   368,   369,   370,   371,   373,   372,   374,
     375,   375,   377,   376,   378,   378,   379,   380,   380,   381,
     381,   382,   382,   383,   383,   384,   384,   385,   386,   386,
     387,   387,   388,   390,   389,   391,   391,   393,   392,   395,
     394,   396,   397,   398,   399,   400,   400,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   402,
     402,   403,   403,   403,   403,   403,   403,   403,   403,   403,
     404,   404,   405,   405,   406,   406,   406,   406,   407,   408,
     409,   410,   411,   411,   412,   413,   414,   414,   416,   415,
     415,   418,   417
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
       1,     1,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     8,     2,     2,     2,     2,
       4,     2,     2,     1,     2,     4,     2,     0,     2,     2,
       2,     2,     4,     2,     2,     2,     4,     8,     1,     0,
       5,     1,     2,     4,    12,     2,     2,     2,     0,     2,
       2,     2,     2,     2,     2,     2,     2,     1,     2,     0,
       6,     4,     2,     5,     3,     4,    12,     5,    13,     0,
       5,     2,     4,     3,     5,     0,     5,    10,     4,     2,
       2,     6,     4,     4,     2,     2,     2,     4,     2,     2,
       2,     3,     3,     3,     3,     3,     2,     2,     2,     2,
       2,     2,     2,     4,     2,     4,     2,     6,     4,     2,
       2,     0,     0,     5,     1,     0,     5,     9,     1,     3,
       3,     1,     3,     1,     4,     2,     0,     1,     2,     4,
      12,     2,     2,     2,     1,     2,     2,     4,     6,     2,
       4,     2,     0,     5,     1,     0,     5,     1,     3,     2,
       4,     2,     2,     1,     2,     0,     5,     1,     0,     5,
       3,     1,    10,     0,     5,     1,     3,     5,     6,     8,
       8,    10,     4,     8,     0,     2,     3,     3,     3,     3,
       2,     3,     3,     1,     6,     2,     6,     6,    10,     6,
       8,     2,     4,     1,     2,     2,    14,     6,     2,     0,
       9,     1,    14,    10,    16,     6,     2,     0,     5,     2,
       3,     7,     0,     7,     2,     4,     2,    14,     6,     2,
       6,     6,    10,     2,     6,     6,    10,     4,     2,     6,
       6,    10,     4,     0,     5,     1,     2,     0,     4,     0,
       9,     8,     1,     1,     1,     1,     3,     3,     3,     5,
       7,     9,     4,     7,     3,     3,     3,     3,     3,     3,
       3,     2,     3,     6,     4,     4,     6,     6,     8,     1,
       1,     1,     1,     1,     4,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
       9,    13,     1,     2,     1,     1,     1,     1,     2,     2,
       2,     2,     2,     2,     2,     4,     3,     1,     0,     3,
       0,     0,     7
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
#line 409 "polyray.y"
     { condition_flags[0] = 1; }
#line 2615 "polyray.tab.cc"
    break;

  case 14: /* element: light  */
#line 428 "polyray.y"
     { ACTION(Add_To_Lights((yyvsp[0].lgt));) }
#line 2621 "polyray.tab.cc"
    break;

  case 16: /* element: object  */
#line 431 "polyray.y"
     { ACTION(Add_To_BinTree(RuntimeState::scene.Root, (yyvsp[0].obj));) }
#line 2627 "polyray.tab.cc"
    break;

  case 19: /* element: particle  */
#line 435 "polyray.y"
     { ACTION(InsertParticle((yyvsp[0].part));) }
#line 2633 "polyray.tab.cc"
    break;

  case 20: /* include_statement: INCLUDE STRING  */
#line 440 "polyray.y"
      {
          if ((yyvsp[0].name) == nullptr)
              serror("polyray.tab.cc::include_statement missing include name\n");

          std::string sname{ (yyvsp[0].name) };
          ACTION(start_include(sname);)
          polyray_free((yyvsp[0].name));
       }
#line 2646 "polyray.tab.cc"
    break;

  case 21: /* defined_token: SURFACE_SYM  */
#line 452 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2652 "polyray.tab.cc"
    break;

  case 22: /* defined_token: TEXTURE_SYM  */
#line 454 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2658 "polyray.tab.cc"
    break;

  case 23: /* defined_token: TEXTURE_MAP_SYM  */
#line 456 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2664 "polyray.tab.cc"
    break;

  case 24: /* defined_token: OBJECT_SYM  */
#line 458 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2670 "polyray.tab.cc"
    break;

  case 25: /* defined_token: EXPRESSION_SYM  */
#line 460 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2676 "polyray.tab.cc"
    break;

  case 26: /* defined_token: TRANSFORM_SYM  */
#line 462 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2682 "polyray.tab.cc"
    break;

  case 27: /* defined_token: PARTICLE_SYM  */
#line 464 "polyray.y"
      { (yyval.name) = (yyvsp[0].name); }
#line 2688 "polyray.tab.cc"
    break;

  case 28: /* definition_types: surface  */
#line 469 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Surface);
               temp_def.data = (yyvsp[0].surf);) }
#line 2695 "polyray.tab.cc"
    break;

  case 29: /* definition_types: texture  */
#line 472 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Texture);
               temp_def.data = (yyvsp[0].text);) }
#line 2702 "polyray.tab.cc"
    break;

  case 30: /* definition_types: texture_map  */
#line 475 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Texture_Map);
               temp_def.data = (yyvsp[0].text_map);) }
#line 2709 "polyray.tab.cc"
    break;

  case 31: /* definition_types: object  */
#line 478 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Object);
               temp_def.data = (yyvsp[0].obj);) }
#line 2716 "polyray.tab.cc"
    break;

  case 32: /* definition_types: transform  */
#line 481 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Transform);
               temp_def.data = (yyvsp[0].trns);) }
#line 2723 "polyray.tab.cc"
    break;

  case 33: /* definition_types: expression  */
#line 484 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Expression);
               temp_def.data = (yyvsp[0].exper);) }
#line 2730 "polyray.tab.cc"
    break;

  case 34: /* definition_types: particle  */
#line 487 "polyray.y"
      { ACTION(temp_def.type = std::to_underlying(ShapeType::Particle);
               temp_def.data = (yyvsp[0].part);) }
#line 2737 "polyray.tab.cc"
    break;

  case 35: /* definition: DEFINE defined_token definition_types  */
#line 493 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 0);) }
#line 2744 "polyray.tab.cc"
    break;

  case 36: /* definition: STATIC DEFINE defined_token definition_types  */
#line 496 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 0);) }
#line 2751 "polyray.tab.cc"
    break;

  case 37: /* definition: DEFINE TOKEN definition_types  */
#line 499 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 0);)
        polyray_free((yyvsp[-1].name)); }
#line 2759 "polyray.tab.cc"
    break;

  case 38: /* definition: STATIC DEFINE TOKEN definition_types  */
#line 503 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 0);)
        polyray_free((yyvsp[-1].name)); }
#line 2767 "polyray.tab.cc"
    break;

  case 39: /* definition: DEFINE NOEVAL defined_token definition_types  */
#line 507 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 1);) }
#line 2774 "polyray.tab.cc"
    break;

  case 40: /* definition: STATIC DEFINE NOEVAL defined_token definition_types  */
#line 510 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 1);) }
#line 2781 "polyray.tab.cc"
    break;

  case 41: /* definition: DEFINE NOEVAL TOKEN definition_types  */
#line 513 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 0, 1);)
        polyray_free((yyvsp[-1].name)); }
#line 2789 "polyray.tab.cc"
    break;

  case 42: /* definition: STATIC DEFINE NOEVAL TOKEN definition_types  */
#line 517 "polyray.y"
      { ACTION(Insert_Definition((yyvsp[-1].name), temp_def.type,
                                 temp_def.data, 1, 1);)
        polyray_free((yyvsp[-1].name)); }
#line 2797 "polyray.tab.cc"
    break;

  case 45: /* particle_decl: BIRTH expression  */
#line 529 "polyray.y"
      { ACTION(SetParticleBirth(CurrentParticle, (yyvsp[0].exper));) }
#line 2803 "polyray.tab.cc"
    break;

  case 46: /* particle_decl: DEATH expression  */
#line 531 "polyray.y"
      { ACTION(SetParticleDeath(CurrentParticle, (yyvsp[0].exper));) }
#line 2809 "polyray.tab.cc"
    break;

  case 47: /* particle_decl: POSITION expression  */
#line 533 "polyray.y"
      { ACTION(SetParticleP(CurrentParticle, (yyvsp[0].exper));) }
#line 2815 "polyray.tab.cc"
    break;

  case 48: /* particle_decl: VELOCITY expression  */
#line 535 "polyray.y"
      { ACTION(SetParticleV(CurrentParticle, (yyvsp[0].exper));) }
#line 2821 "polyray.tab.cc"
    break;

  case 49: /* particle_decl: ACCELERATION expression  */
#line 537 "polyray.y"
      { ACTION(SetParticleA(CurrentParticle, (yyvsp[0].exper));) }
#line 2827 "polyray.tab.cc"
    break;

  case 50: /* particle_decl: AVOID expression  */
#line 539 "polyray.y"
      { ACTION(SetParticleAvoid(CurrentParticle, (yyvsp[0].exper));) }
#line 2833 "polyray.tab.cc"
    break;

  case 51: /* particle_decl: FLOCK expression  */
#line 541 "polyray.y"
      { ACTION(SetParticleFlock(CurrentParticle, (yyvsp[0].exper));) }
#line 2839 "polyray.tab.cc"
    break;

  case 52: /* particle_decl: COUNT expression  */
#line 543 "polyray.y"
      { ACTION(SetParticleCount(CurrentParticle, (yyvsp[0].exper));) }
#line 2845 "polyray.tab.cc"
    break;

  case 53: /* particle_decl: OBJECT expression  */
#line 545 "polyray.y"
      { ACTION(SetParticleObjName(CurrentParticle, (yyvsp[0].exper));) }
#line 2851 "polyray.tab.cc"
    break;

  case 54: /* $@2: %empty  */
#line 550 "polyray.y"
     { ACTION(CurrentParticle = CreateParticle();) }
#line 2857 "polyray.tab.cc"
    break;

  case 55: /* particle: PARTICLE '{' $@2 particle_decls '}'  */
#line 552 "polyray.y"
     { ACTION((yyval.part) = CurrentParticle;) }
#line 2863 "polyray.tab.cc"
    break;

  case 56: /* particle: PARTICLE_SYM  */
#line 554 "polyray.y"
      { ACTION((yyval.part) = CopyParticle((yyvsp[0].name));) }
#line 2869 "polyray.tab.cc"
    break;

  case 57: /* $@3: %empty  */
#line 559 "polyray.y"
      { ACTION(Object_Stack = push_object(Object_Stack, object_action1());) }
#line 2875 "polyray.tab.cc"
    break;

  case 58: /* object: OBJECT '{' $@3 object_decls '}'  */
#line 561 "polyray.y"
      { ACTION((yyval.obj) = pop_object(&Object_Stack);) }
#line 2881 "polyray.tab.cc"
    break;

  case 59: /* object: OBJECT_SYM  */
#line 563 "polyray.y"
      { ACTION((yyval.obj) = object_action2((yyvsp[0].name));) }
#line 2887 "polyray.tab.cc"
    break;

  case 60: /* $@4: %empty  */
#line 565 "polyray.y"
      { ACTION(Object_Stack =
               push_object(Object_Stack, object_action2((yyvsp[-1].name)));) }
#line 2894 "polyray.tab.cc"
    break;

  case 61: /* object: OBJECT_SYM '{' $@4 object_modifier_decls '}'  */
#line 568 "polyray.y"
      { ACTION((yyval.obj) = pop_object(&Object_Stack);) }
#line 2900 "polyray.tab.cc"
    break;

  case 64: /* object_modifier_decl: texture  */
#line 578 "polyray.y"
      { ACTION(if (Object_Stack->element->o_texture != nullptr)
                        TextureDelete(Object_Stack->element->o_texture);
                     Object_Stack->element->o_texture = (yyvsp[0].text);) }
#line 2908 "polyray.tab.cc"
    break;

  case 65: /* object_modifier_decl: transform  */
#line 582 "polyray.y"
      { ACTION(TransformObject(Object_Stack->element, (yyvsp[0].trns));
               delete (yyvsp[0].trns);) }
#line 2915 "polyray.tab.cc"
    break;

  case 66: /* object_modifier_decl: ROTATE point  */
#line 585 "polyray.y"
      { ACTION(RotateObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2921 "polyray.tab.cc"
    break;

  case 67: /* object_modifier_decl: ROTATE point ',' fexper  */
#line 587 "polyray.y"
      { ACTION(RotateAxisObject(Object_Stack->element, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 2927 "polyray.tab.cc"
    break;

  case 68: /* object_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 590 "polyray.y"
      { ACTION(ShearObject(Object_Stack->element, (yyvsp[-10].flt), (yyvsp[-8].flt),
                           (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 2934 "polyray.tab.cc"
    break;

  case 69: /* object_modifier_decl: TRANSLATE point  */
#line 593 "polyray.y"
      { ACTION(TranslateObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2940 "polyray.tab.cc"
    break;

  case 70: /* object_modifier_decl: SCALE point  */
#line 595 "polyray.y"
      { ACTION(ScaleObject(Object_Stack->element, (yyvsp[0].vec));) }
#line 2946 "polyray.tab.cc"
    break;

  case 72: /* object_modifier_decl: SHADING_FLAGS fexper  */
#line 598 "polyray.y"
      { ACTION(Object_Stack->element->o_sflag = (int)(yyvsp[0].flt);) }
#line 2952 "polyray.tab.cc"
    break;

  case 73: /* object_modifier_decl: DITHER fexper  */
#line 600 "polyray.y"
      { ACTION(Object_Stack->element->o_dither = (yyvsp[0].flt);) }
#line 2958 "polyray.tab.cc"
    break;

  case 74: /* object_modifier_decl: BOUNDING_BOX point ',' point  */
#line 602 "polyray.y"
     { ACTION(VecCopy((yyvsp[-2].vec), Object_Stack->element->o_bnd.lower_left);
              VecCopy((yyvsp[0].vec), Object_Stack->element->o_bnd.lengths);
              VecSub(Object_Stack->element->o_bnd.lengths,
                     Object_Stack->element->o_bnd.lower_left,
                     Object_Stack->element->o_bnd.lengths);) }
#line 2968 "polyray.tab.cc"
    break;

  case 76: /* object_modifier_decl: DISPLACE expression  */
#line 609 "polyray.y"
      { ACTION(Object_Stack->element->o_displace = (yyvsp[0].exper);) }
#line 2974 "polyray.tab.cc"
    break;

  case 77: /* uv_information: UV_STEPS fexper ',' fexper  */
#line 614 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[-2].flt);
               Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 2983 "polyray.tab.cc"
    break;

  case 78: /* uv_information: UV_STEPS fexper ',' fexper ',' fexper  */
#line 619 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[-4].flt);
               Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[-2].flt);
               Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 2992 "polyray.tab.cc"
    break;

  case 79: /* uv_information: U_STEPS fexper  */
#line 624 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[0] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 2999 "polyray.tab.cc"
    break;

  case 80: /* uv_information: V_STEPS fexper  */
#line 627 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[1] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3006 "polyray.tab.cc"
    break;

  case 81: /* uv_information: W_STEPS fexper  */
#line 630 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_steps[2] = (int)(yyvsp[0].flt);
               Object_Stack->element->o_sflag &= ~ADAPTIVE_UV;) }
#line 3013 "polyray.tab.cc"
    break;

  case 82: /* uv_information: UV_BOUNDS fexper ',' fexper ',' fexper ',' fexper  */
#line 633 "polyray.y"
      { ACTION(Object_Stack->element->o_uv_bounds[0] = (yyvsp[-6].flt);
               Object_Stack->element->o_uv_bounds[1] = (yyvsp[-4].flt);
               Object_Stack->element->o_uv_bounds[2] = (yyvsp[-2].flt);
               Object_Stack->element->o_uv_bounds[3] = (yyvsp[0].flt);) }
#line 3022 "polyray.tab.cc"
    break;

  case 83: /* root_solver: ROOT_SOLVER FERRARI  */
#line 641 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 0);) }
#line 3028 "polyray.tab.cc"
    break;

  case 84: /* root_solver: ROOT_SOLVER VIETA  */
#line 643 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 1);) }
#line 3034 "polyray.tab.cc"
    break;

  case 85: /* root_solver: ROOT_SOLVER STURM  */
#line 645 "polyray.y"
      { ACTION(root_solver_action(Object_Stack->element, 2);) }
#line 3040 "polyray.tab.cc"
    break;

  case 125: /* camera_exper: ANGLE fexper  */
#line 695 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_angle = degtorad((yyvsp[0].flt)/2.0 );) }
#line 3046 "polyray.tab.cc"
    break;

  case 126: /* camera_exper: ANTIALIAS fexper  */
#line 697 "polyray.y"
     { ACTION(RuntimeState::settings.antialias = (int)(yyvsp[0].flt);
              if (RuntimeState::settings.antialias < 0 || RuntimeState::settings.antialias > 4)
                 serror("Antialias value of %d is not between 0 and 4",
                       RuntimeState::settings.antialias);)}
#line 3055 "polyray.tab.cc"
    break;

  case 127: /* camera_exper: ANTIALIAS_THRESHOLD fexper  */
#line 702 "polyray.y"
     { ACTION(RuntimeState::settings.antialias_threshold = (yyvsp[0].flt);) }
#line 3061 "polyray.tab.cc"
    break;

  case 128: /* camera_exper: APERTURE fexper  */
#line 704 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_aperture = (yyvsp[0].flt);) }
#line 3067 "polyray.tab.cc"
    break;

  case 129: /* camera_exper: ASPECT fexper  */
#line 706 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_aspect = (yyvsp[0].flt);) }
#line 3073 "polyray.tab.cc"
    break;

  case 130: /* camera_exper: AT point  */
#line 708 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), RuntimeState::scene.Eye.view_at);) }
#line 3079 "polyray.tab.cc"
    break;

  case 131: /* camera_exper: FOCAL_DISTANCE fexper  */
#line 710 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_focaldist = (yyvsp[0].flt);) }
#line 3085 "polyray.tab.cc"
    break;

  case 132: /* camera_exper: FROM point  */
#line 712 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), RuntimeState::scene.Eye.view_from);) }
#line 3091 "polyray.tab.cc"
    break;

  case 133: /* camera_exper: HITHER fexper  */
#line 714 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_hither = (yyvsp[0].flt);) }
#line 3097 "polyray.tab.cc"
    break;

  case 134: /* camera_exper: IMAGE_FORMAT fexper  */
#line 716 "polyray.y"
     { ACTION(if ((int)((yyvsp[0].flt)) == 0)
                RuntimeState::settings.DepthRender = 0;
             else if ((int)((yyvsp[0].flt)) == 1) {
                RuntimeState::settings.pixel_encoding = 0;
                RuntimeState::settings.DepthRender = 1;
                }
             else
                serror("image_format must be either 0 (normal) or 1 (depth)");
              ) }
#line 3111 "polyray.tab.cc"
    break;

  case 135: /* camera_exper: IMAGE_WINDOW fexper ',' fexper ',' fexper ',' fexper  */
#line 726 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_x0 = (int) (yyvsp[-6].flt);
              RuntimeState::scene.Eye.view_y0 = (int) (yyvsp[-4].flt);
              RuntimeState::scene.Eye.view_xl = (int) (yyvsp[-2].flt);
              RuntimeState::scene.Eye.view_yl = (int) (yyvsp[0].flt);) }
#line 3120 "polyray.tab.cc"
    break;

  case 136: /* camera_exper: MAX_SAMPLES fexper  */
#line 731 "polyray.y"
     { ACTION(RuntimeState::settings.maxsamples = (int)(yyvsp[0].flt);
              if (RuntimeState::settings.maxsamples < 0)
                 serror("maxsamples must be greater than 0");)}
#line 3128 "polyray.tab.cc"
    break;

  case 137: /* camera_exper: MAX_TRACE_DEPTH fexper  */
#line 735 "polyray.y"
     { ACTION(RuntimeState::settings.maxlevel = (int)(yyvsp[0].flt);
              if (RuntimeState::settings.maxlevel < 1 || RuntimeState::settings.maxlevel > 63)
                 serror("maxlevel must be between 1 and 63");)}
#line 3136 "polyray.tab.cc"
    break;

  case 138: /* camera_exper: PIXEL_ENCODING fexper  */
#line 739 "polyray.y"
     { ACTION(RuntimeState::settings.pixel_encoding = (int)(yyvsp[0].flt);
              if (RuntimeState::settings.pixel_encoding != 0 && RuntimeState::settings.pixel_encoding != 1)
                 serror("Pixel encoding of %d is not one of: 0 [none], 1 [RLE]",
                       RuntimeState::settings.pixel_encoding);) }
#line 3145 "polyray.tab.cc"
    break;

  case 139: /* camera_exper: PIXELSIZE fexper  */
#line 744 "polyray.y"
     { ACTION(RuntimeState::settings.pixelsize = (int)(yyvsp[0].flt);
              if (RuntimeState::settings.pixelsize != 8 && RuntimeState::settings.pixelsize != 16 &&
                  RuntimeState::settings.pixelsize != 24 && RuntimeState::settings.pixelsize != 32)
                 serror("Pixelsize of %d is not one of: 8, 16, 24, 32",
                       RuntimeState::settings.pixelsize);) }
#line 3155 "polyray.tab.cc"
    break;

  case 140: /* camera_exper: RESOLUTION fexper ',' fexper  */
#line 750 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_xres = (int) (yyvsp[-2].flt);
              RuntimeState::scene.Eye.view_yres = (int) (yyvsp[0].flt);) }
#line 3162 "polyray.tab.cc"
    break;

  case 141: /* camera_exper: UP point  */
#line 753 "polyray.y"
     { ACTION(VecCopy((yyvsp[0].vec), RuntimeState::scene.Eye.view_up);) }
#line 3168 "polyray.tab.cc"
    break;

  case 142: /* camera_exper: YON fexper  */
#line 755 "polyray.y"
     { ACTION(RuntimeState::scene.Eye.view_yon = (yyvsp[0].flt);) }
#line 3174 "polyray.tab.cc"
    break;

  case 148: /* flare_option: COLOR expression  */
#line 774 "polyray.y"
      { ACTION(Set_Flare_Color((yyvsp[0].exper));) }
#line 3180 "polyray.tab.cc"
    break;

  case 149: /* flare_option: COUNT fexper  */
#line 776 "polyray.y"
      { ACTION(Set_Flare_Count((yyvsp[0].flt));) }
#line 3186 "polyray.tab.cc"
    break;

  case 150: /* flare_option: SPACING fexper  */
#line 778 "polyray.y"
      { ACTION(Set_Flare_Spacing((yyvsp[0].flt));) }
#line 3192 "polyray.tab.cc"
    break;

  case 151: /* flare_option: SEED fexper  */
#line 780 "polyray.y"
      { ACTION(Set_Flare_Seed((int)(yyvsp[0].flt));) }
#line 3198 "polyray.tab.cc"
    break;

  case 152: /* flare_option: SIZE fexper ',' fexper  */
#line 782 "polyray.y"
      { ACTION(Set_Flare_Size((yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3204 "polyray.tab.cc"
    break;

  case 153: /* flare_option: CONCAVE fexper  */
#line 784 "polyray.y"
      { ACTION(Set_Flare_Concave((yyvsp[0].flt));) }
#line 3210 "polyray.tab.cc"
    break;

  case 154: /* flare_option: SPHERE fexper  */
#line 786 "polyray.y"
      { ACTION(Set_Flare_Sphere((yyvsp[0].flt));) }
#line 3216 "polyray.tab.cc"
    break;

  case 155: /* light_modifier_decl: COLOR expression  */
#line 791 "polyray.y"
      { ACTION(Set_Light_Color((yyvsp[0].exper));) }
#line 3222 "polyray.tab.cc"
    break;

  case 156: /* light_modifier_decl: SPHERE point ',' fexper  */
#line 793 "polyray.y"
      { ACTION(Translate_Light(toNuVec((yyvsp[-2].vec)));
               Set_Light_Radius((yyvsp[0].flt));) }
#line 3229 "polyray.tab.cc"
    break;

  case 157: /* light_modifier_decl: POLYGON fexper ',' fexper ',' fexper ',' fexper  */
#line 796 "polyray.y"
      { ACTION(Set_Light_Polygon((yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3235 "polyray.tab.cc"
    break;

  case 158: /* light_modifier_decl: NO_SHADOW  */
#line 798 "polyray.y"
      { ACTION(Set_Light_Shadow(0);) }
#line 3241 "polyray.tab.cc"
    break;

  case 159: /* $@5: %empty  */
#line 800 "polyray.y"
      { ACTION(Create_Lens_Flare();) }
#line 3247 "polyray.tab.cc"
    break;

  case 161: /* light_modifier_decl: transform  */
#line 803 "polyray.y"
      { ACTION(Transform_Light((yyvsp[0].trns));
               delete (yyvsp[0].trns);) }
#line 3254 "polyray.tab.cc"
    break;

  case 162: /* light_modifier_decl: ROTATE point  */
#line 806 "polyray.y"
      { ACTION(Rotate_Light(toNuVec((yyvsp[0].vec)));) }
#line 3260 "polyray.tab.cc"
    break;

  case 163: /* light_modifier_decl: ROTATE point ',' fexper  */
#line 808 "polyray.y"
      { ACTION(Rotate_Axis_Light((yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3266 "polyray.tab.cc"
    break;

  case 164: /* light_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 811 "polyray.y"
      { ACTION(Shear_Light((yyvsp[-10].flt), (yyvsp[-8].flt), (yyvsp[-6].flt),
                           (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3273 "polyray.tab.cc"
    break;

  case 165: /* light_modifier_decl: TRANSLATE point  */
#line 814 "polyray.y"
      { ACTION(Translate_Light(toNuVec((yyvsp[0].vec)));) }
#line 3279 "polyray.tab.cc"
    break;

  case 166: /* light_modifier_decl: SCALE point  */
#line 816 "polyray.y"
      { ACTION(Scale_Light((yyvsp[0].vec));) }
#line 3285 "polyray.tab.cc"
    break;

  case 169: /* depth_light_modifier: ANGLE fexper  */
#line 826 "polyray.y"
      { ACTION(DepthLight1((yyvsp[0].flt));) }
#line 3291 "polyray.tab.cc"
    break;

  case 170: /* depth_light_modifier: ASPECT fexper  */
#line 828 "polyray.y"
      { ACTION(DepthLight2((yyvsp[0].flt));) }
#line 3297 "polyray.tab.cc"
    break;

  case 171: /* depth_light_modifier: AT point  */
#line 830 "polyray.y"
      { ACTION(DepthLight3((yyvsp[0].vec));) }
#line 3303 "polyray.tab.cc"
    break;

  case 172: /* depth_light_modifier: COLOR expression  */
#line 832 "polyray.y"
      { ACTION(DepthLight4((yyvsp[0].exper));) }
#line 3309 "polyray.tab.cc"
    break;

  case 173: /* depth_light_modifier: DEPTH sexper  */
#line 834 "polyray.y"
      { ACTION(DepthLight5((yyvsp[0].name));
               polyray_free((yyvsp[0].name));) }
#line 3316 "polyray.tab.cc"
    break;

  case 174: /* depth_light_modifier: FROM point  */
#line 837 "polyray.y"
      { ACTION(DepthLight6((yyvsp[0].vec));) }
#line 3322 "polyray.tab.cc"
    break;

  case 175: /* depth_light_modifier: HITHER fexper  */
#line 839 "polyray.y"
     { ACTION(DepthLight9((yyvsp[0].flt));) }
#line 3328 "polyray.tab.cc"
    break;

  case 176: /* depth_light_modifier: UP point  */
#line 841 "polyray.y"
      { ACTION(DepthLight7((yyvsp[0].vec));) }
#line 3334 "polyray.tab.cc"
    break;

  case 177: /* depth_light_modifier: NO_SHADOW  */
#line 843 "polyray.y"
      { ACTION(Set_Light_Shadow(0);) }
#line 3340 "polyray.tab.cc"
    break;

  case 180: /* haze_statement: HAZE fexper ',' fexper ',' point  */
#line 853 "polyray.y"
     { ACTION(haze_action((yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].vec));) }
#line 3346 "polyray.tab.cc"
    break;

  case 181: /* light: LIGHT point ',' point  */
#line 858 "polyray.y"
     { ACTION((yyval.lgt) = light_action1((yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 3352 "polyray.tab.cc"
    break;

  case 182: /* light: LIGHT point  */
#line 860 "polyray.y"
     { ACTION((yyval.lgt) = light_action2((yyvsp[0].vec));) }
#line 3358 "polyray.tab.cc"
    break;

  case 183: /* light: LIGHT NO_SHADOW point ',' point  */
#line 862 "polyray.y"
     { ACTION((yyval.lgt) = light_action1((yyvsp[-2].vec), (yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3365 "polyray.tab.cc"
    break;

  case 184: /* light: LIGHT NO_SHADOW point  */
#line 865 "polyray.y"
     { ACTION((yyval.lgt) = light_action2((yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3372 "polyray.tab.cc"
    break;

  case 185: /* light: SPOT_LIGHT point ',' point  */
#line 868 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight(RuntimeState::White, (yyvsp[-2].vec), (yyvsp[0].vec), 10.0, 30, 45);) }
#line 3378 "polyray.tab.cc"
    break;

  case 186: /* light: SPOT_LIGHT point ',' point ',' point ',' fexper ',' fexper ',' fexper  */
#line 870 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight((yyvsp[-10].vec), (yyvsp[-8].vec), (yyvsp[-6].vec), (yyvsp[-4].flt),
                           (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3385 "polyray.tab.cc"
    break;

  case 187: /* light: SPOT_LIGHT NO_SHADOW point ',' point  */
#line 873 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight(RuntimeState::White, (yyvsp[-2].vec), (yyvsp[0].vec), 10.0, 30, 45);
              Set_Light_Shadow(0);) }
#line 3392 "polyray.tab.cc"
    break;

  case 188: /* light: SPOT_LIGHT NO_SHADOW point ',' point ',' point ',' fexper ',' fexper ',' fexper  */
#line 877 "polyray.y"
     { ACTION((yyval.lgt) = SetSpotLight((yyvsp[-10].vec), (yyvsp[-8].vec), (yyvsp[-6].vec), (yyvsp[-4].flt),
                           (yyvsp[-2].flt), (yyvsp[0].flt));
              Set_Light_Shadow(0);) }
#line 3400 "polyray.tab.cc"
    break;

  case 189: /* @6: %empty  */
#line 881 "polyray.y"
     { ACTION((yyval.lgt) = light_action3();) }
#line 3406 "polyray.tab.cc"
    break;

  case 190: /* light: TEXTURED_LIGHT '{' @6 light_modifier_decls '}'  */
#line 883 "polyray.y"
     { ACTION((yyval.lgt) = Current_Light;) }
#line 3412 "polyray.tab.cc"
    break;

  case 191: /* light: DIRECTIONAL_LIGHT point  */
#line 885 "polyray.y"
     { ACTION((yyval.lgt) = light_action4((yyvsp[0].vec));) }
#line 3418 "polyray.tab.cc"
    break;

  case 192: /* light: DIRECTIONAL_LIGHT point ',' point  */
#line 887 "polyray.y"
     { ACTION((yyval.lgt) = light_action5((yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 3424 "polyray.tab.cc"
    break;

  case 193: /* light: DIRECTIONAL_LIGHT NO_SHADOW point  */
#line 889 "polyray.y"
     { ACTION((yyval.lgt) = light_action4((yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3431 "polyray.tab.cc"
    break;

  case 194: /* light: DIRECTIONAL_LIGHT NO_SHADOW point ',' point  */
#line 892 "polyray.y"
     { ACTION((yyval.lgt) = light_action5((yyvsp[-2].vec), (yyvsp[0].vec));
              Set_Light_Shadow(0);) }
#line 3438 "polyray.tab.cc"
    break;

  case 195: /* @7: %empty  */
#line 895 "polyray.y"
     { ACTION((yyval.lgt) = light_action6();) }
#line 3444 "polyray.tab.cc"
    break;

  case 196: /* light: DEPTHMAPPED_LIGHT '{' @7 depth_light_modifiers '}'  */
#line 897 "polyray.y"
     { ACTION(DepthLight8();
              (yyval.lgt) = Current_Light;) }
#line 3451 "polyray.tab.cc"
    break;

  case 197: /* draw_statement: DRAW fexper ',' fexper ',' fexper ',' expression ',' expression  */
#line 903 "polyray.y"
      { ACTION(draw_action((yyvsp[-8].flt), (yyvsp[-6].flt), (int)(yyvsp[-4].flt),
                           (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3458 "polyray.tab.cc"
    break;

  case 198: /* draw_statement: POINT expression ',' expression  */
#line 906 "polyray.y"
      { ACTION(draw_action(0.0, 0.0, 0, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3464 "polyray.tab.cc"
    break;

  case 199: /* background: BACKGROUND expression  */
#line 911 "polyray.y"
     { ACTION(background_action((yyvsp[0].exper));) }
#line 3470 "polyray.tab.cc"
    break;

  case 200: /* surface_declaration: COLOR expression  */
#line 916 "polyray.y"
      { ACTION(color_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3476 "polyray.tab.cc"
    break;

  case 201: /* surface_declaration: COLOR_MAP '(' map_entries ',' expression ')'  */
#line 918 "polyray.y"
      { ACTION(color_map_action(CurrentSurface, (yyvsp[-3].cmap_entry), (yyvsp[-1].exper));) }
#line 3482 "polyray.tab.cc"
    break;

  case 202: /* surface_declaration: COLOR_MAP '(' map_entries ')'  */
#line 920 "polyray.y"
      { ACTION(color_map_action(CurrentSurface, (yyvsp[-1].cmap_entry), nullptr);) }
#line 3488 "polyray.tab.cc"
    break;

  case 203: /* surface_declaration: AMBIENT expression ',' expression  */
#line 922 "polyray.y"
      { ACTION(ambient_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3494 "polyray.tab.cc"
    break;

  case 204: /* surface_declaration: AMBIENT expression  */
#line 924 "polyray.y"
      { ACTION(ambient_action(CurrentSurface, nullptr, (yyvsp[0].exper));) }
#line 3500 "polyray.tab.cc"
    break;

  case 205: /* surface_declaration: BRILLIANCE expression  */
#line 926 "polyray.y"
      { ACTION(brilliance_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3506 "polyray.tab.cc"
    break;

  case 206: /* surface_declaration: BUMP_SCALE expression  */
#line 928 "polyray.y"
      { ACTION(bump_scale_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3512 "polyray.tab.cc"
    break;

  case 207: /* surface_declaration: DIFFUSE expression ',' expression  */
#line 930 "polyray.y"
      { ACTION(diffuse_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3518 "polyray.tab.cc"
    break;

  case 208: /* surface_declaration: DIFFUSE expression  */
#line 932 "polyray.y"
      { ACTION(diffuse_action(CurrentSurface, nullptr, (yyvsp[0].exper));) }
#line 3524 "polyray.tab.cc"
    break;

  case 209: /* surface_declaration: FREQUENCY expression  */
#line 934 "polyray.y"
      { ACTION(frequency_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3530 "polyray.tab.cc"
    break;

  case 210: /* surface_declaration: LOOKUP_FUNCTION expression  */
#line 936 "polyray.y"
      { ACTION(lookup_function_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3536 "polyray.tab.cc"
    break;

  case 211: /* surface_declaration: MICROFACET PHONG expression  */
#line 938 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, PHONG, (yyvsp[0].exper));) }
#line 3542 "polyray.tab.cc"
    break;

  case 212: /* surface_declaration: MICROFACET BLINN expression  */
#line 940 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, BLINN, (yyvsp[0].exper));) }
#line 3548 "polyray.tab.cc"
    break;

  case 213: /* surface_declaration: MICROFACET GAUSSIAN expression  */
#line 942 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, GAUSSIAN, (yyvsp[0].exper));) }
#line 3554 "polyray.tab.cc"
    break;

  case 214: /* surface_declaration: MICROFACET REITZ expression  */
#line 944 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, REITZ, (yyvsp[0].exper));) }
#line 3560 "polyray.tab.cc"
    break;

  case 215: /* surface_declaration: MICROFACET COOK expression  */
#line 946 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, COOK, (yyvsp[0].exper));) }
#line 3566 "polyray.tab.cc"
    break;

  case 216: /* surface_declaration: MICROFACET expression  */
#line 948 "polyray.y"
      { ACTION(microfacet_action(CurrentSurface, PHONG, (yyvsp[0].exper));) }
#line 3572 "polyray.tab.cc"
    break;

  case 217: /* surface_declaration: NORMAL expression  */
#line 950 "polyray.y"
      { ACTION(normal_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3578 "polyray.tab.cc"
    break;

  case 218: /* surface_declaration: POSITION expression  */
#line 952 "polyray.y"
      { ACTION(position_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3584 "polyray.tab.cc"
    break;

  case 219: /* surface_declaration: OCTAVES expression  */
#line 954 "polyray.y"
      { ACTION(octaves_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3590 "polyray.tab.cc"
    break;

  case 220: /* surface_declaration: PHASE expression  */
#line 956 "polyray.y"
      { ACTION(phase_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3596 "polyray.tab.cc"
    break;

  case 221: /* surface_declaration: POSITION_FUNCTION expression  */
#line 958 "polyray.y"
      { ACTION(position_function_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3602 "polyray.tab.cc"
    break;

  case 222: /* surface_declaration: POSITION_SCALE expression  */
#line 960 "polyray.y"
      { ACTION(position_scale_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3608 "polyray.tab.cc"
    break;

  case 223: /* surface_declaration: REFLECTION expression ',' expression  */
#line 962 "polyray.y"
      { ACTION(reflection_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3614 "polyray.tab.cc"
    break;

  case 224: /* surface_declaration: REFLECTION expression  */
#line 964 "polyray.y"
      { ACTION(reflection_action(CurrentSurface, nullptr, (yyvsp[0].exper));) }
#line 3620 "polyray.tab.cc"
    break;

  case 225: /* surface_declaration: SPECULAR expression ',' expression  */
#line 966 "polyray.y"
      { ACTION(specular_action(CurrentSurface, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3626 "polyray.tab.cc"
    break;

  case 226: /* surface_declaration: SPECULAR expression  */
#line 968 "polyray.y"
      { ACTION(specular_action(CurrentSurface, nullptr, (yyvsp[0].exper));) }
#line 3632 "polyray.tab.cc"
    break;

  case 227: /* surface_declaration: TRANSMISSION expression ',' expression ',' expression  */
#line 970 "polyray.y"
      { ACTION(transmission_action(CurrentSurface, (yyvsp[-4].exper), (yyvsp[-2].exper),
                                   (yyvsp[0].exper));) }
#line 3639 "polyray.tab.cc"
    break;

  case 228: /* surface_declaration: TRANSMISSION expression ',' expression  */
#line 973 "polyray.y"
      { ACTION(transmission_action(CurrentSurface, nullptr,
                                   (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 3646 "polyray.tab.cc"
    break;

  case 229: /* surface_declaration: TURBULENCE expression  */
#line 976 "polyray.y"
      { ACTION(turbulence_action(CurrentSurface, (yyvsp[0].exper));) }
#line 3652 "polyray.tab.cc"
    break;

  case 232: /* $@8: %empty  */
#line 986 "polyray.y"
      { ACTION(surface_action1();) }
#line 3658 "polyray.tab.cc"
    break;

  case 233: /* surface: SURFACE '{' $@8 surface_declarations '}'  */
#line 988 "polyray.y"
     { ACTION((yyval.surf) = CurrentSurface;) }
#line 3664 "polyray.tab.cc"
    break;

  case 234: /* surface: SURFACE_SYM  */
#line 990 "polyray.y"
      { ACTION(surface_action2((yyvsp[0].name)); (yyval.surf) = CurrentSurface;) }
#line 3670 "polyray.tab.cc"
    break;

  case 235: /* $@9: %empty  */
#line 992 "polyray.y"
      { ACTION(surface_action2((yyvsp[-1].name));) }
#line 3676 "polyray.tab.cc"
    break;

  case 236: /* surface: SURFACE_SYM '{' $@9 surface_declarations '}'  */
#line 994 "polyray.y"
      { ACTION((yyval.surf) = CurrentSurface;) }
#line 3682 "polyray.tab.cc"
    break;

  case 237: /* texture_map_element: '[' fexper ',' fexper ',' texture ',' texture ']'  */
#line 999 "polyray.y"
      { ACTION((yyval.text_map) =
               make_texture_map_entry((yyvsp[-7].flt), (yyvsp[-5].flt), (yyvsp[-3].text), (yyvsp[-1].text));) }
#line 3689 "polyray.tab.cc"
    break;

  case 238: /* texture_map_elements: texture_map_element  */
#line 1005 "polyray.y"
      { ACTION((yyval.text_map) = (yyvsp[0].text_map);) }
#line 3695 "polyray.tab.cc"
    break;

  case 239: /* texture_map_elements: texture_map_elements ',' texture_map_element  */
#line 1007 "polyray.y"
      { ACTION((yyval.text_map) =
               texture_map_action2((yyvsp[-2].text_map), (yyvsp[0].text_map));) }
#line 3702 "polyray.tab.cc"
    break;

  case 240: /* texture_fn_element: expression ',' texture  */
#line 1013 "polyray.y"
      { ACTION((yyval.text_fn) = make_texture_fn_entry((yyvsp[-2].exper), (yyvsp[0].text));) }
#line 3708 "polyray.tab.cc"
    break;

  case 241: /* texture_fn_elements: texture_fn_element  */
#line 1018 "polyray.y"
      { ACTION((yyval.text_fn) = (yyvsp[0].text_fn);) }
#line 3714 "polyray.tab.cc"
    break;

  case 242: /* texture_fn_elements: texture_fn_elements ',' texture_fn_element  */
#line 1020 "polyray.y"
      { ACTION((yyval.text_fn) = texture_fn_action2((yyvsp[-2].text_fn), (yyvsp[0].text_fn));) }
#line 3720 "polyray.tab.cc"
    break;

  case 243: /* texture_map: TEXTURE_MAP_SYM  */
#line 1025 "polyray.y"
      { ACTION((yyval.text_map) = texture_map_action1((yyvsp[0].name));) }
#line 3726 "polyray.tab.cc"
    break;

  case 244: /* texture_map: TEXTURE_MAP '(' texture_map_elements ')'  */
#line 1027 "polyray.y"
      { ACTION((yyval.text_map) = (yyvsp[-1].text_map);) }
#line 3732 "polyray.tab.cc"
    break;

  case 247: /* texture_modifier_decl: transform  */
#line 1037 "polyray.y"
      { ACTION(if (Texture_Stack->element->t_trans == nullptr)
                  Texture_Stack->element->t_trans = (yyvsp[0].trns);
               else {
                  Compose_Transformations(* Texture_Stack->element->t_trans,
                                          *(yyvsp[0].trns));
                  delete (yyvsp[0].trns);
                  }) }
#line 3744 "polyray.tab.cc"
    break;

  case 248: /* texture_modifier_decl: ROTATE point  */
#line 1045 "polyray.y"
      { ACTION(TextureRotate(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3750 "polyray.tab.cc"
    break;

  case 249: /* texture_modifier_decl: ROTATE point ',' fexper  */
#line 1047 "polyray.y"
      { ACTION(TextureAxisRotate(Texture_Stack->element, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3756 "polyray.tab.cc"
    break;

  case 250: /* texture_modifier_decl: SHEAR fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper  */
#line 1050 "polyray.y"
      { ACTION(TextureShear(Texture_Stack->element, (yyvsp[-10].flt), (yyvsp[-8].flt),
                            (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3763 "polyray.tab.cc"
    break;

  case 251: /* texture_modifier_decl: TRANSLATE point  */
#line 1053 "polyray.y"
      { ACTION(TextureTranslate(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3769 "polyray.tab.cc"
    break;

  case 252: /* texture_modifier_decl: SCALE point  */
#line 1055 "polyray.y"
      { ACTION(TextureScale(Texture_Stack->element, (yyvsp[0].vec));) }
#line 3775 "polyray.tab.cc"
    break;

  case 254: /* texture_declaration: surface  */
#line 1064 "polyray.y"
      { ACTION(create_plain(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3781 "polyray.tab.cc"
    break;

  case 255: /* texture_declaration: SPECIAL surface  */
#line 1066 "polyray.y"
      { ACTION(create_special(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3787 "polyray.tab.cc"
    break;

  case 256: /* texture_declaration: NOISE surface  */
#line 1068 "polyray.y"
      { ACTION(create_noise(Texture_Stack->element, (yyvsp[0].surf));) }
#line 3793 "polyray.tab.cc"
    break;

  case 257: /* texture_declaration: CHECKER texture ',' texture  */
#line 1070 "polyray.y"
      { ACTION(create_checker(Texture_Stack->element, (yyvsp[-2].text), (yyvsp[0].text));) }
#line 3799 "polyray.tab.cc"
    break;

  case 258: /* texture_declaration: HEXAGON texture ',' texture ',' texture  */
#line 1072 "polyray.y"
      { ACTION(create_hexagon(Texture_Stack->element,
                              (yyvsp[-4].text), (yyvsp[-2].text), (yyvsp[0].text));) }
#line 3806 "polyray.tab.cc"
    break;

  case 259: /* texture_declaration: LAYERED texture_list  */
#line 1075 "polyray.y"
      { ACTION(create_layered(Texture_Stack->element, (yyvsp[0].textlist));) }
#line 3812 "polyray.tab.cc"
    break;

  case 260: /* texture_declaration: INDEXED expression ',' texture_map  */
#line 1077 "polyray.y"
      { ACTION(create_indexed(Texture_Stack->element, (yyvsp[-2].exper),
                              (yyvsp[0].text_map));) }
#line 3819 "polyray.tab.cc"
    break;

  case 261: /* texture_declaration: SUMMED texture_fn_elements  */
#line 1080 "polyray.y"
      { ACTION(create_summed(Texture_Stack->element, (yyvsp[0].text_fn));) }
#line 3825 "polyray.tab.cc"
    break;

  case 262: /* $@10: %empty  */
#line 1085 "polyray.y"
      { ACTION(push_texture(texture_action1());) }
#line 3831 "polyray.tab.cc"
    break;

  case 263: /* texture: TEXTURE '{' $@10 texture_declarations '}'  */
#line 1087 "polyray.y"
      { ACTION((yyval.text) = pop_texture();) }
#line 3837 "polyray.tab.cc"
    break;

  case 264: /* texture: TEXTURE_SYM  */
#line 1089 "polyray.y"
      { ACTION((yyval.text) = texture_action2((yyvsp[0].name));) }
#line 3843 "polyray.tab.cc"
    break;

  case 265: /* $@11: %empty  */
#line 1091 "polyray.y"
      { ACTION(push_texture(texture_action2((yyvsp[-1].name)));) }
#line 3849 "polyray.tab.cc"
    break;

  case 266: /* texture: TEXTURE_SYM '{' $@11 texture_modifier_decls '}'  */
#line 1093 "polyray.y"
      { ACTION((yyval.text) = pop_texture();) }
#line 3855 "polyray.tab.cc"
    break;

  case 267: /* texture_list: texture  */
#line 1098 "polyray.y"
      { ACTION((yyval.textlist) = texture_list_action1((yyvsp[0].text));) }
#line 3861 "polyray.tab.cc"
    break;

  case 268: /* texture_list: texture_list ',' texture  */
#line 1100 "polyray.y"
      { ACTION((yyval.textlist) = texture_list_action2((yyvsp[-2].textlist), (yyvsp[0].text));) }
#line 3867 "polyray.tab.cc"
    break;

  case 269: /* transform_declaration: ROTATE point  */
#line 1105 "polyray.y"
      { ACTION(rotate_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3873 "polyray.tab.cc"
    break;

  case 270: /* transform_declaration: ROTATE point ',' fexper  */
#line 1107 "polyray.y"
      { ACTION(axis_rotate_transform(Current_Transform, (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 3879 "polyray.tab.cc"
    break;

  case 271: /* transform_declaration: SCALE point  */
#line 1109 "polyray.y"
      { ACTION(scale_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3885 "polyray.tab.cc"
    break;

  case 272: /* transform_declaration: TRANSLATE point  */
#line 1111 "polyray.y"
      { ACTION(translate_transform(Current_Transform, (yyvsp[0].vec));) }
#line 3891 "polyray.tab.cc"
    break;

  case 275: /* $@12: %empty  */
#line 1121 "polyray.y"
      { ACTION(Current_Transform = transform_action1();) }
#line 3897 "polyray.tab.cc"
    break;

  case 276: /* transform: TRANSFORM '{' $@12 transform_declarations '}'  */
#line 1123 "polyray.y"
      { ACTION((yyval.trns) = Current_Transform;) }
#line 3903 "polyray.tab.cc"
    break;

  case 277: /* transform: TRANSFORM_SYM  */
#line 1125 "polyray.y"
      { ACTION((yyval.trns) = transform_action2((yyvsp[0].name));) }
#line 3909 "polyray.tab.cc"
    break;

  case 278: /* $@13: %empty  */
#line 1127 "polyray.y"
      { ACTION(Current_Transform = transform_action2((yyvsp[-1].name));) }
#line 3915 "polyray.tab.cc"
    break;

  case 279: /* transform: TRANSFORM_SYM '{' $@13 transform_declarations '}'  */
#line 1129 "polyray.y"
      { ACTION((yyval.trns) = Current_Transform;) }
#line 3921 "polyray.tab.cc"
    break;

  case 280: /* bezier_points: bezier_points ',' point  */
#line 1134 "polyray.y"
      { ACTION((yyval.vecl) = add_bezier_point((yyvsp[-2].vecl), (yyvsp[0].vec));) }
#line 3927 "polyray.tab.cc"
    break;

  case 281: /* bezier_points: point  */
#line 1136 "polyray.y"
      { ACTION((yyval.vecl) = add_bezier_point(nullptr, (yyvsp[0].vec));) }
#line 3933 "polyray.tab.cc"
    break;

  case 282: /* bezier: BEZIER fexper ',' fexper ',' fexper ',' fexper ',' bezier_points  */
#line 1141 "polyray.y"
      { ACTION(MakeBezier(Object_Stack->element,
                          (int)(yyvsp[-8].flt), (yyvsp[-6].flt), (int)(yyvsp[-4].flt),
                          (int)(yyvsp[-2].flt), (yyvsp[0].vecl));) }
#line 3941 "polyray.tab.cc"
    break;

  case 283: /* $@14: %empty  */
#line 1148 "polyray.y"
     { ACTION(npoints = 0;) }
#line 3947 "polyray.tab.cc"
    break;

  case 284: /* blob: BLOB fexper ':' $@14 blobelements  */
#line 1150 "polyray.y"
     { ACTION(MakeBlob(Object_Stack->element, (yyvsp[-3].flt),
                       blob_components, npoints, 1);
              blob_components = nullptr; npoints = 0;) }
#line 3955 "polyray.tab.cc"
    break;

  case 287: /* blobelement: fexper ',' fexper ',' point  */
#line 1162 "polyray.y"
      { ACTION(spherical_component_action((yyvsp[0].vec), (yyvsp[-4].flt), (yyvsp[-2].flt));) }
#line 3961 "polyray.tab.cc"
    break;

  case 288: /* blobelement: SPHERE point ',' fexper ',' fexper  */
#line 1164 "polyray.y"
      { ACTION(spherical_component_action((yyvsp[-4].vec), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3967 "polyray.tab.cc"
    break;

  case 289: /* blobelement: CYLINDER point ',' point ',' fexper ',' fexper  */
#line 1166 "polyray.y"
      { ACTION(cylindrical_component_action((yyvsp[-6].vec), (yyvsp[-4].vec),
                                            (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3974 "polyray.tab.cc"
    break;

  case 290: /* blobelement: PLANE point ',' fexper ',' fexper ',' fexper  */
#line 1169 "polyray.y"
      { ACTION(planar_component_action((yyvsp[-6].vec), (yyvsp[-4].flt),
                                       (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3981 "polyray.tab.cc"
    break;

  case 291: /* blobelement: TORUS point ',' point ',' fexper ',' fexper ',' fexper  */
#line 1172 "polyray.y"
      { ACTION(toroidal_component_action((yyvsp[-8].vec), (yyvsp[-6].vec),
                                         (yyvsp[-4].flt), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 3988 "polyray.tab.cc"
    break;

  case 292: /* box: BOX point ',' point  */
#line 1178 "polyray.y"
      { ACTION(MakeBox(Object_Stack->element, (yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 3994 "polyray.tab.cc"
    break;

  case 293: /* cone: CONE point ',' fexper ',' point ',' fexper  */
#line 1183 "polyray.y"
      { ACTION(MakeCone(Object_Stack->element, (yyvsp[-6].vec),
                        (yyvsp[-4].flt), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4001 "polyray.tab.cc"
    break;

  case 294: /* $@15: %empty  */
#line 1188 "polyray.y"
      { ACTION(ObjectDepth++;) }
#line 4007 "polyray.tab.cc"
    break;

  case 295: /* csg: $@15 csg_tree  */
#line 1190 "polyray.y"
      { ACTION(ObjectDepth--;
               MakeCSG(Object_Stack->element, (yyvsp[0].csgtree));) }
#line 4014 "polyray.tab.cc"
    break;

  case 296: /* csg_tree: '(' csg_tree ')'  */
#line 1196 "polyray.y"
      { ACTION((yyval.csgtree) = (yyvsp[-1].csgtree);) }
#line 4020 "polyray.tab.cc"
    break;

  case 297: /* csg_tree: csg_tree '+' csg_tree  */
#line 1198 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Union), (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4027 "polyray.tab.cc"
    break;

  case 298: /* csg_tree: csg_tree '-' csg_tree  */
#line 1201 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Intersection), (yyvsp[-2].csgtree),
                                make_csg_node(std::to_underlying(ShapeType::Inverse), (yyvsp[0].csgtree), nullptr));) }
#line 4035 "polyray.tab.cc"
    break;

  case 299: /* csg_tree: csg_tree '*' csg_tree  */
#line 1205 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Intersection), (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4042 "polyray.tab.cc"
    break;

  case 300: /* csg_tree: '~' csg_tree  */
#line 1208 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Inverse), (yyvsp[0].csgtree), nullptr);) }
#line 4049 "polyray.tab.cc"
    break;

  case 301: /* csg_tree: csg_tree '&' csg_tree  */
#line 1211 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Clip), (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4056 "polyray.tab.cc"
    break;

  case 302: /* csg_tree: csg_tree '^' csg_tree  */
#line 1214 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Merge), (yyvsp[-2].csgtree), (yyvsp[0].csgtree));) }
#line 4063 "polyray.tab.cc"
    break;

  case 303: /* csg_tree: object  */
#line 1217 "polyray.y"
      { ACTION((yyval.csgtree) =
                  make_csg_node(std::to_underlying(ShapeType::Base_Object), (yyvsp[0].obj), nullptr);) }
#line 4070 "polyray.tab.cc"
    break;

  case 304: /* cylinder: CYLINDER point ',' point ',' fexper  */
#line 1223 "polyray.y"
      { ACTION(MakeCylinder(Object_Stack->element,
                            (yyvsp[-4].vec), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4077 "polyray.tab.cc"
    break;

  case 305: /* cylindrical_height_field: CHEIGHT_FIELD sexper  */
#line 1229 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[0].name), 0,
                             1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4085 "polyray.tab.cc"
    break;

  case 306: /* cylindrical_height_field: CHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1233 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[-4].name), 0,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4093 "polyray.tab.cc"
    break;

  case 307: /* cylindrical_height_fn: CHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1240 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 0, 1.0, 128.0);) }
#line 4100 "polyray.tab.cc"
    break;

  case 308: /* cylindrical_height_fn: CHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1244 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 0, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4107 "polyray.tab.cc"
    break;

  case 309: /* disc: DISC point ',' point ',' fexper  */
#line 1250 "polyray.y"
      { ACTION(MakeDisc(Object_Stack->element,
                        (yyvsp[-4].vec), (yyvsp[-2].vec), 0.0, (yyvsp[0].flt));) }
#line 4114 "polyray.tab.cc"
    break;

  case 310: /* disc: DISC point ',' point ',' fexper ',' fexper  */
#line 1253 "polyray.y"
      { ACTION(MakeDisc(Object_Stack->element,
                        (yyvsp[-6].vec), (yyvsp[-4].vec), (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4121 "polyray.tab.cc"
    break;

  case 311: /* function: FUNCTION expression  */
#line 1259 "polyray.y"
      { ACTION(MakeFunction(Object_Stack->element, (yyvsp[0].exper));) }
#line 4127 "polyray.tab.cc"
    break;

  case 312: /* gridded: GRIDDED sexper ',' object_list  */
#line 1264 "polyray.y"
      { ACTION(MakeGrid(Object_Stack->element, (yyvsp[-2].name), (yyvsp[0].objlist));
        polyray_free((yyvsp[-2].name));) }
#line 4134 "polyray.tab.cc"
    break;

  case 313: /* object_list: object  */
#line 1270 "polyray.y"
      { ACTION(ostackptr ost =
                     (ostackptr)polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == nullptr) serror("Failed to allocate grid object");
                ost->element = (yyvsp[0].obj);
                ost->next    = nullptr;
                (yyval.objlist)  = ost;) }
#line 4145 "polyray.tab.cc"
    break;

  case 314: /* object_list: object_list object  */
#line 1277 "polyray.y"
      { ACTION(ostackptr ost =
                     (ostackptr)polyray_malloc(sizeof(struct object_stack_struct));
                if (ost == nullptr) serror("Failed to allocate grid object");
                ost->element = (yyvsp[0].obj);
                ost->next    = (yyvsp[-1].objlist);
                (yyval.objlist)  = ost;) }
#line 4156 "polyray.tab.cc"
    break;

  case 315: /* height_field: HEIGHT_FIELD sexper  */
#line 1287 "polyray.y"
      { ACTION(MakeHeight(Object_Stack->element, (yyvsp[0].name), 0);
        polyray_free((yyvsp[0].name));) }
#line 4163 "polyray.tab.cc"
    break;

  case 316: /* height_fn: HEIGHT_FN fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1295 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                            (yyvsp[-8].flt), (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt),
                            (yyvsp[0].exper), 0);) }
#line 4171 "polyray.tab.cc"
    break;

  case 317: /* height_fn: HEIGHT_FN fexper ',' fexper ',' expression  */
#line 1299 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt),
                            0.0, 1.0, 0.0, 1.0,
                            (yyvsp[0].exper), 0);) }
#line 4179 "polyray.tab.cc"
    break;

  case 318: /* hypertexture: HYPERTEXTURE expression  */
#line 1306 "polyray.y"
      { ACTION(MakeHypertexture(Object_Stack->element, (yyvsp[0].exper));) }
#line 4185 "polyray.tab.cc"
    break;

  case 319: /* $@16: %empty  */
#line 1311 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec));
            if (plist == nullptr) serror("Failed to allocate lathe data\n");
            pl = plist;) }
#line 4194 "polyray.tab.cc"
    break;

  case 320: /* lathe: LATHE fexper ',' point ',' fexper ',' $@16 pointlist  */
#line 1316 "polyray.y"
   { ACTION(MakeRevolve(Object_Stack->element, (int)(yyvsp[-7].flt),
                        (yyvsp[-5].vec), (int)(yyvsp[-3].flt), plist);) }
#line 4201 "polyray.tab.cc"
    break;

  case 321: /* light_object: light  */
#line 1322 "polyray.y"
   { ACTION(MakeLight(Object_Stack->element, (yyvsp[0].lgt));) }
#line 4207 "polyray.tab.cc"
    break;

  case 322: /* nurb: NURB fexper ',' fexper ',' fexper ',' fexper ',' expression ',' expression ',' expression  */
#line 1328 "polyray.y"
   { ACTION(MakeNurb(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                     (int)(yyvsp[-8].flt), (int)(yyvsp[-6].flt), (yyvsp[-4].exper), (yyvsp[-2].exper),
                     (yyvsp[0].exper));) }
#line 4215 "polyray.tab.cc"
    break;

  case 323: /* nurb: NURB fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1333 "polyray.y"
   { ACTION(MakeNurb(Object_Stack->element, (int)(yyvsp[-8].flt), (int)(yyvsp[-6].flt),
                     (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt), nullptr, nullptr,
                     (yyvsp[0].exper));) }
#line 4223 "polyray.tab.cc"
    break;

  case 324: /* nurb2: NURB2 fexper ',' fexper ',' fexper ',' fexper ',' expression ',' expression ',' expression ',' expression  */
#line 1341 "polyray.y"
   { ACTION(MakeNurbTrimmed(Object_Stack->element, (int)(yyvsp[-14].flt), (int)(yyvsp[-12].flt),
                            (int)(yyvsp[-10].flt), (int)(yyvsp[-8].flt), (yyvsp[-6].exper), (yyvsp[-4].exper),
                            (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4231 "polyray.tab.cc"
    break;

  case 325: /* parabola: PARABOLA point ',' point ',' fexper  */
#line 1348 "polyray.y"
      { ACTION(MakeParabola(Object_Stack->element,
                            (yyvsp[-4].vec), (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4238 "polyray.tab.cc"
    break;

  case 326: /* parametric: PARAMETRIC expression  */
#line 1354 "polyray.y"
      { ACTION(MakeParametric(Object_Stack->element, (yyvsp[0].exper));) }
#line 4244 "polyray.tab.cc"
    break;

  case 327: /* $@17: %empty  */
#line 1359 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            if (npoints < 3)
               serror("polygons must have at least 3 sides\n");
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec)) ;
            if (plist == nullptr) serror("Failed to allocate polygon data\n");
            pl = plist;) }
#line 4255 "polyray.tab.cc"
    break;

  case 328: /* polygon: POLYGON fexper ',' $@17 pointlist  */
#line 1366 "polyray.y"
   { ACTION(MakePoly(Object_Stack->element, (int)(yyvsp[-3].flt), plist);) }
#line 4261 "polyray.tab.cc"
    break;

  case 329: /* polynomial: POLYNOMIAL expression  */
#line 1371 "polyray.y"
      { ACTION(polynomial_action1((yyvsp[0].exper), 1);) }
#line 4267 "polyray.tab.cc"
    break;

  case 330: /* patch_vertex: point ',' point  */
#line 1376 "polyray.y"
      { ACTION(VecCopy((yyvsp[-2].vec), tri_vertex[npoints].pos);
               VecCopy((yyvsp[0].vec), tri_vertex[npoints].norm);
               tri_vertex[npoints].u = PLY_HUGE;
               tri_vertex[npoints].v = PLY_HUGE;
               npoints++;) }
#line 4277 "polyray.tab.cc"
    break;

  case 331: /* patch_vertex: point ',' point UV fexper ',' fexper  */
#line 1382 "polyray.y"
      { ACTION(VecCopy((yyvsp[-6].vec), tri_vertex[npoints].pos);
               VecCopy((yyvsp[-4].vec), tri_vertex[npoints].norm);
               tri_vertex[npoints].u = (yyvsp[-2].flt);
               tri_vertex[npoints].v = (yyvsp[0].flt);
               npoints++;) }
#line 4287 "polyray.tab.cc"
    break;

  case 332: /* $@18: %empty  */
#line 1391 "polyray.y"
      { ACTION(npoints = 0;) }
#line 4293 "polyray.tab.cc"
    break;

  case 333: /* ppatch: PATCH $@18 patch_vertex ',' patch_vertex ',' patch_vertex  */
#line 1393 "polyray.y"
      { ACTION(MakeTri(Object_Stack->element, tri_vertex);) }
#line 4299 "polyray.tab.cc"
    break;

  case 334: /* raw: RAW sexper  */
#line 1398 "polyray.y"
      { ACTION(MakeRaw(Object_Stack->element, (yyvsp[0].name), 0.0);
        polyray_free((yyvsp[0].name));) }
#line 4306 "polyray.tab.cc"
    break;

  case 335: /* raw: RAW sexper ',' fexper  */
#line 1401 "polyray.y"
      { ACTION(MakeRaw(Object_Stack->element, (yyvsp[-2].name), (yyvsp[0].flt));
        polyray_free((yyvsp[-2].name));) }
#line 4313 "polyray.tab.cc"
    break;

  case 336: /* smooth_height_field: SMOOTH_HEIGHT_FIELD sexper  */
#line 1407 "polyray.y"
      { ACTION(MakeHeight(Object_Stack->element, (yyvsp[0].name), 1);
        polyray_free((yyvsp[0].name));) }
#line 4320 "polyray.tab.cc"
    break;

  case 337: /* smooth_height_fn: SMOOTH_HEIGHT_FN fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' fexper ',' expression  */
#line 1415 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-12].flt), (int)(yyvsp[-10].flt),
                            (yyvsp[-8].flt), (yyvsp[-6].flt), (yyvsp[-4].flt), (yyvsp[-2].flt),
                            (yyvsp[0].exper), 1);) }
#line 4328 "polyray.tab.cc"
    break;

  case 338: /* smooth_height_fn: SMOOTH_HEIGHT_FN fexper ',' fexper ',' expression  */
#line 1419 "polyray.y"
      { ACTION(MakeHeightFn(Object_Stack->element, (int)(yyvsp[-4].flt), (int)(yyvsp[-2].flt),
                            0.0, 1.0, 0.0, 1.0,
                            (yyvsp[0].exper), 1);) }
#line 4336 "polyray.tab.cc"
    break;

  case 339: /* smooth_cheight_field: SMOOTH_CHEIGHT_FIELD sexper  */
#line 1426 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[0].name), 1, 1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4343 "polyray.tab.cc"
    break;

  case 340: /* smooth_cheight_field: SMOOTH_CHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1429 "polyray.y"
      { ACTION(MakeCylHeight(Object_Stack->element, (yyvsp[-4].name), 1,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4351 "polyray.tab.cc"
    break;

  case 341: /* smooth_cheight_fn: SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1436 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 1, 1.0, 128.0);) }
#line 4358 "polyray.tab.cc"
    break;

  case 342: /* smooth_cheight_fn: SMOOTH_CHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1439 "polyray.y"
      { ACTION(MakeCylHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 1, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4365 "polyray.tab.cc"
    break;

  case 343: /* smooth_sheight_field: SMOOTH_SHEIGHT_FIELD sexper  */
#line 1445 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[0].name), 0,
                             1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4373 "polyray.tab.cc"
    break;

  case 344: /* smooth_sheight_field: SMOOTH_SHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1449 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[-4].name), 1,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4381 "polyray.tab.cc"
    break;

  case 345: /* smooth_sheight_fn: SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1456 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 1, 1.0, 128.0);) }
#line 4388 "polyray.tab.cc"
    break;

  case 346: /* smooth_sheight_fn: SMOOTH_SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1459 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 1, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4395 "polyray.tab.cc"
    break;

  case 347: /* sphere: SPHERE point ',' fexper  */
#line 1465 "polyray.y"
      { ACTION(MakeSphere(Object_Stack->element,
                          (yyvsp[-2].vec), (yyvsp[0].flt));) }
#line 4402 "polyray.tab.cc"
    break;

  case 348: /* spherical_height_field: SHEIGHT_FIELD sexper  */
#line 1471 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[0].name), 0, 1.0, 128.0);
        polyray_free((yyvsp[0].name));) }
#line 4409 "polyray.tab.cc"
    break;

  case 349: /* spherical_height_field: SHEIGHT_FIELD sexper ',' fexper ',' fexper  */
#line 1474 "polyray.y"
      { ACTION(MakeSphHeight(Object_Stack->element, (yyvsp[-4].name), 0,
                             (yyvsp[-2].flt), (yyvsp[0].flt));
        polyray_free((yyvsp[-4].name));) }
#line 4417 "polyray.tab.cc"
    break;

  case 350: /* spherical_height_fn: SHEIGHT_FN fexper ',' fexper ',' expression  */
#line 1481 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-4].flt), (yyvsp[-2].flt),
                               (yyvsp[0].exper), 0, 1.0, 128.0);) }
#line 4424 "polyray.tab.cc"
    break;

  case 351: /* spherical_height_fn: SHEIGHT_FN fexper ',' fexper ',' expression ',' fexper ',' fexper  */
#line 1484 "polyray.y"
      { ACTION(MakeSphHeightFn(Object_Stack->element, (yyvsp[-8].flt), (yyvsp[-6].flt),
                               (yyvsp[-4].exper), 0, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4431 "polyray.tab.cc"
    break;

  case 352: /* superq: SUPERQ fexper ',' fexper  */
#line 1490 "polyray.y"
      { ACTION(MakeSuperq(Object_Stack->element, (yyvsp[-2].flt), (yyvsp[0].flt));) }
#line 4437 "polyray.tab.cc"
    break;

  case 353: /* $@19: %empty  */
#line 1495 "polyray.y"
      { ACTION(npoints = (int)(yyvsp[-1].flt);
               if (npoints < 2)
                  serror("contours must have at least 3 sides\n");
               plist = (fVec *)polyray_malloc(((int)(yyvsp[-1].flt) + 1) * sizeof(fVec));
               if (plist == nullptr) serror("Failed to allocate contour data\n");
               pl = plist;) }
#line 4448 "polyray.tab.cc"
    break;

  case 354: /* contour: CONTOUR fexper ',' $@19 pointlist  */
#line 1502 "polyray.y"
      { ACTION(if (gcount == 0) serror("Too many contours for the glyph\n");
               cl->count = (int)(yyvsp[-3].flt);
               cl->points = plist;
               gcount--; cl++;) }
#line 4457 "polyray.tab.cc"
    break;

  case 357: /* $@20: %empty  */
#line 1515 "polyray.y"
      { ACTION(gcount = (int)(yyvsp[0].flt);
               if (gcount < 1)
                  serror("Glyphs must have at least one contour");
               contours = (Contour*)polyray_malloc(gcount * sizeof(Contour));
               if (contours == nullptr)
                  serror("Failed to allocate glyph data");
               cl = contours;) }
#line 4469 "polyray.tab.cc"
    break;

  case 358: /* glyph: GLYPH fexper $@20 glyph_contours  */
#line 1523 "polyray.y"
      { ACTION(if (gcount != 0)
                  serror("Wrong number of contours in glyph\n");
               MakeGlyph(Object_Stack->element, (int)(yyvsp[-2].flt), contours);) }
#line 4477 "polyray.tab.cc"
    break;

  case 359: /* $@21: %empty  */
#line 1530 "polyray.y"
   { ACTION(npoints = (int)(yyvsp[-1].flt);
            plist = (fVec *)polyray_malloc((int)(yyvsp[-1].flt) * sizeof(fVec));
            if (plist == nullptr) serror("Failed to allocate sweep data\n");
            pl = plist;) }
#line 4486 "polyray.tab.cc"
    break;

  case 360: /* sweep: SWEEP fexper ',' point ',' fexper ',' $@21 pointlist  */
#line 1535 "polyray.y"
   { ACTION(MakeSweep(Object_Stack->element, (int)(yyvsp[-7].flt),
                      (yyvsp[-5].vec), (int)(yyvsp[-3].flt), plist);) }
#line 4493 "polyray.tab.cc"
    break;

  case 361: /* torus: TORUS fexper ',' fexper ',' point ',' point  */
#line 1541 "polyray.y"
      { ACTION(MakeTorus(Object_Stack->element, (yyvsp[-6].flt), (yyvsp[-4].flt),
                         (yyvsp[-2].vec), (yyvsp[0].vec));) }
#line 4500 "polyray.tab.cc"
    break;

  case 362: /* fexper: expression  */
#line 1547 "polyray.y"
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(nullptr, (yyvsp[0].exper), &ftmp, vtmp, &tnode) == 1) {
                  deallocate_node((yyvsp[0].exper));
                  (yyval.flt) = ftmp;
                  }
               else {
                  serror("Bad fexper expression\n");
                  }) }
#line 4513 "polyray.tab.cc"
    break;

  case 363: /* point: expression  */
#line 1559 "polyray.y"
      { ACTION(Flt ftmp; Vec vtmp; NODE_PTR tnode;
               if (eval_node(nullptr, (yyvsp[0].exper), &ftmp, vtmp, &tnode) == 2) {
                  VecCopy(vtmp, (yyval.vec));
                  deallocate_node((yyvsp[0].exper));
                  }
               else {
                  serror("Bad point expression\n");
                  }) }
#line 4526 "polyray.tab.cc"
    break;

  case 364: /* sexper: expression  */
#line 1571 "polyray.y"
      { ACTION(std::string stmp;
               if (create_string((yyvsp[0].exper), stmp)) {
                  deallocate_node((yyvsp[0].exper));
                  (yyval.name) = static_cast<char*>(polyray_malloc(stmp.length() + 1));
                  if ((yyval.name) == nullptr)
                     serror("Failed to allocate sexper string\n");
                  memcpy((yyval.name), stmp.c_str(), stmp.length() + 1);                  
                  }
               else {
                  serror("Bad sexper expression\n");
                  }) }
#line 4542 "polyray.tab.cc"
    break;

  case 365: /* pointlist: point  */
#line 1586 "polyray.y"
     { ACTION(if (npoints==0) serror("Too many points for the polygon\n");
              VecCopy((yyvsp[0].vec), (*pl)); npoints--; pl++;) }
#line 4549 "polyray.tab.cc"
    break;

  case 366: /* pointlist: pointlist ',' point  */
#line 1589 "polyray.y"
     { ACTION(if (npoints==0) serror("Too many points for the polygon\n");
              VecCopy((yyvsp[0].vec), (*pl)); npoints--; pl++;) }
#line 4556 "polyray.tab.cc"
    break;

  case 367: /* expression: '(' expression ')'  */
#line 1595 "polyray.y"
      { ACTION((yyval.exper) = (yyvsp[-1].exper);) }
#line 4562 "polyray.tab.cc"
    break;

  case 368: /* expression: '[' expression_list ']'  */
#line 1597 "polyray.y"
      { ACTION((yyval.exper) = make_array_node((yyvsp[-1].elist));) }
#line 4568 "polyray.tab.cc"
    break;

  case 369: /* expression: '<' expression ',' expression '>'  */
#line 1599 "polyray.y"
      { ACTION((yyval.exper) = make_vector3_node((yyvsp[-3].exper), (yyvsp[-1].exper),
                                             make_value_node(0.0));) }
#line 4575 "polyray.tab.cc"
    break;

  case 370: /* expression: '<' expression ',' expression ',' expression '>'  */
#line 1602 "polyray.y"
      { ACTION((yyval.exper) = make_vector3_node((yyvsp[-5].exper), (yyvsp[-3].exper),
                                             (yyvsp[-1].exper));) }
#line 4582 "polyray.tab.cc"
    break;

  case 371: /* expression: '<' expression ',' expression ',' expression ',' expression '>'  */
#line 1605 "polyray.y"
      { ACTION((yyval.exper) = make_vector4_node((yyvsp[-7].exper), (yyvsp[-5].exper),
                                             (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4589 "polyray.tab.cc"
    break;

  case 372: /* expression: expression '[' expression ']'  */
#line 1608 "polyray.y"
      { ACTION((yyval.exper) = make_node(SUBSCRIPT_EXPER, (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4595 "polyray.tab.cc"
    break;

  case 373: /* expression: '(' conditional '?' expression ':' expression ')'  */
#line 1610 "polyray.y"
      { ACTION((yyval.exper) = make_cond_node((yyvsp[-5].exper), (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4601 "polyray.tab.cc"
    break;

  case 374: /* expression: expression '^' expression  */
#line 1612 "polyray.y"
      { ACTION((yyval.exper) = make_node(POWER_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4607 "polyray.tab.cc"
    break;

  case 375: /* expression: expression '%' expression  */
#line 1614 "polyray.y"
      { ACTION((yyval.exper) = make_node(FMOD, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4613 "polyray.tab.cc"
    break;

  case 376: /* expression: expression '*' expression  */
#line 1616 "polyray.y"
      { ACTION((yyval.exper) = make_node(TIMES_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4619 "polyray.tab.cc"
    break;

  case 377: /* expression: expression '.' expression  */
#line 1618 "polyray.y"
      { (yyval.exper) = make_node(DOT_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper)); }
#line 4625 "polyray.tab.cc"
    break;

  case 378: /* expression: expression '/' expression  */
#line 1620 "polyray.y"
      { ACTION((yyval.exper) = make_node(DIV_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4631 "polyray.tab.cc"
    break;

  case 379: /* expression: expression '+' expression  */
#line 1622 "polyray.y"
      { ACTION((yyval.exper) = make_node(PLUS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4637 "polyray.tab.cc"
    break;

  case 380: /* expression: expression '-' expression  */
#line 1624 "polyray.y"
      { ACTION((yyval.exper) = make_node(MINUS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4643 "polyray.tab.cc"
    break;

  case 381: /* expression: '-' expression  */
#line 1626 "polyray.y"
      { ACTION((yyval.exper) = make_node(TIMES_EXPER, make_value_node(-1.0),
                                     (yyvsp[0].exper));)}
#line 4650 "polyray.tab.cc"
    break;

  case 382: /* expression: '|' expression '|'  */
#line 1629 "polyray.y"
      { ACTION((yyval.exper) = make_fn1_node(FABS, (yyvsp[-1].exper));) }
#line 4656 "polyray.tab.cc"
    break;

  case 383: /* expression: COLOR_MAP '(' map_entries ',' expression ')'  */
#line 1631 "polyray.y"
      { ACTION((yyval.exper) = make_cmap_node((yyvsp[-3].cmap_entry), (yyvsp[-1].exper));) }
#line 4662 "polyray.tab.cc"
    break;

  case 384: /* expression: COLOR_MAP '(' map_entries ')'  */
#line 1633 "polyray.y"
      { ACTION((yyval.exper) = make_cmap_node((yyvsp[-1].cmap_entry), nullptr);) }
#line 4668 "polyray.tab.cc"
    break;

  case 385: /* expression: NOISE '(' expression ')'  */
#line 1635 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOISE, (yyvsp[-1].exper), nullptr);) }
#line 4674 "polyray.tab.cc"
    break;

  case 386: /* expression: NOISE '(' expression ',' expression ')'  */
#line 1637 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOISE, (yyvsp[-3].exper), (yyvsp[-1].exper));) }
#line 4680 "polyray.tab.cc"
    break;

  case 387: /* expression: ROTATE '(' expression ',' expression ')'  */
#line 1639 "polyray.y"
      { ACTION((yyval.exper) = make_fn3_node(ROTATE, (yyvsp[-3].exper), (yyvsp[-1].exper), nullptr);) }
#line 4686 "polyray.tab.cc"
    break;

  case 388: /* expression: ROTATE '(' expression ',' expression ',' expression ')'  */
#line 1641 "polyray.y"
      { ACTION((yyval.exper) = make_fn3_node(ROTATE, (yyvsp[-5].exper), (yyvsp[-3].exper),
                                         (yyvsp[-1].exper));) }
#line 4693 "polyray.tab.cc"
    break;

  case 389: /* expression: COLOR  */
#line 1644 "polyray.y"
      { ACTION((yyval.exper) = make_node(COLOR, nullptr, nullptr);) }
#line 4699 "polyray.tab.cc"
    break;

  case 390: /* expression: FRAME  */
#line 1646 "polyray.y"
      { ACTION((yyval.exper) = make_node(FRAME, nullptr, nullptr);) }
#line 4705 "polyray.tab.cc"
    break;

  case 391: /* expression: END_FRAME  */
#line 1648 "polyray.y"
      { ACTION((yyval.exper) = make_node(END_FRAME, nullptr, nullptr);) }
#line 4711 "polyray.tab.cc"
    break;

  case 392: /* expression: START_FRAME  */
#line 1650 "polyray.y"
      { ACTION((yyval.exper) = make_node(START_FRAME, nullptr, nullptr);) }
#line 4717 "polyray.tab.cc"
    break;

  case 393: /* expression: TOTAL_FRAMES  */
#line 1652 "polyray.y"
      { ACTION((yyval.exper) = make_node(TOTAL_FRAMES, nullptr, nullptr);) }
#line 4723 "polyray.tab.cc"
    break;

  case 394: /* expression: TOKEN '(' expression_list ')'  */
#line 1654 "polyray.y"
      { ACTION((yyval.exper) = check_term((yyvsp[-3].name), (yyvsp[-1].elist));)
        polyray_free((yyvsp[-3].name)); }
#line 4730 "polyray.tab.cc"
    break;

  case 395: /* expression: TOKEN  */
#line 1657 "polyray.y"
      { ACTION((yyval.exper) = check_term0((yyvsp[0].name));)
        polyray_free((yyvsp[0].name)); }
#line 4737 "polyray.tab.cc"
    break;

  case 396: /* expression: NUM  */
#line 1660 "polyray.y"
      { ACTION((yyval.exper) = make_value_node((yyvsp[0].flt));) }
#line 4743 "polyray.tab.cc"
    break;

  case 397: /* expression: STRING  */
#line 1662 "polyray.y"
      { ACTION((yyval.exper) = make_string_node((yyvsp[0].name));)
               polyray_free((yyvsp[0].name)); }
#line 4750 "polyray.tab.cc"
    break;

  case 398: /* expression: EXPRESSION_SYM  */
#line 1665 "polyray.y"
      { ACTION((yyval.exper) = exper_action((yyvsp[0].name));) }
#line 4756 "polyray.tab.cc"
    break;

  case 399: /* expression_list: expression  */
#line 1670 "polyray.y"
      { ACTION((yyval.elist) = make_list_node((yyvsp[0].exper));) }
#line 4762 "polyray.tab.cc"
    break;

  case 400: /* expression_list: expression_list ',' expression  */
#line 1672 "polyray.y"
      { ACTION((yyval.elist) = expression_action1((yyvsp[-2].elist), (yyvsp[0].exper));) }
#line 4768 "polyray.tab.cc"
    break;

  case 401: /* conditional: '(' conditional ')'  */
#line 1677 "polyray.y"
      { ACTION((yyval.exper) = (yyvsp[-1].exper);) }
#line 4774 "polyray.tab.cc"
    break;

  case 402: /* conditional: expression '<' expression  */
#line 1679 "polyray.y"
      { ACTION((yyval.exper) = make_node(LESS_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4780 "polyray.tab.cc"
    break;

  case 403: /* conditional: expression '>' expression  */
#line 1681 "polyray.y"
      { ACTION((yyval.exper) = make_node(GREATER_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4786 "polyray.tab.cc"
    break;

  case 404: /* conditional: expression LTEQ_SYM expression  */
#line 1683 "polyray.y"
      { ACTION((yyval.exper) = make_node(LTEQ_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4792 "polyray.tab.cc"
    break;

  case 405: /* conditional: expression GTEQ_SYM expression  */
#line 1685 "polyray.y"
      { ACTION((yyval.exper) = make_node(GTEQ_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4798 "polyray.tab.cc"
    break;

  case 406: /* conditional: expression EQUAL_SYM expression  */
#line 1687 "polyray.y"
      { ACTION((yyval.exper) = make_node(EQUAL_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4804 "polyray.tab.cc"
    break;

  case 407: /* conditional: conditional AND_SYM conditional  */
#line 1689 "polyray.y"
      { ACTION((yyval.exper) = make_node(AND_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4810 "polyray.tab.cc"
    break;

  case 408: /* conditional: conditional OR_SYM conditional  */
#line 1691 "polyray.y"
      { ACTION((yyval.exper) = make_node(OR_EXPER, (yyvsp[-2].exper), (yyvsp[0].exper));) }
#line 4816 "polyray.tab.cc"
    break;

  case 409: /* conditional: '!' conditional  */
#line 1693 "polyray.y"
      { ACTION((yyval.exper) = make_node(NOT_EXPER, (yyvsp[-1].exper), nullptr);) }
#line 4822 "polyray.tab.cc"
    break;

  case 410: /* map_entry: '[' fexper ',' fexper ',' point ',' point ']'  */
#line 1698 "polyray.y"
      { ACTION((yyval.cmap_entry) =
                 map_entry_action1((yyvsp[-7].flt), (yyvsp[-5].flt),
                                   (yyvsp[-3].vec), 0.0, (yyvsp[-1].vec), 0.0);) }
#line 4830 "polyray.tab.cc"
    break;

  case 411: /* map_entry: '[' fexper ',' fexper ',' point ',' fexper ',' point ',' fexper ']'  */
#line 1702 "polyray.y"
      { ACTION((yyval.cmap_entry) =
                 map_entry_action1((yyvsp[-11].flt), (yyvsp[-9].flt),
                                   (yyvsp[-7].vec), (yyvsp[-5].flt),
                                   (yyvsp[-3].vec), (yyvsp[-1].flt));) }
#line 4839 "polyray.tab.cc"
    break;

  case 412: /* map_entries: map_entry  */
#line 1710 "polyray.y"
      { ACTION((yyval.cmap_entry) = (yyvsp[0].cmap_entry);) }
#line 4845 "polyray.tab.cc"
    break;

  case 413: /* map_entries: map_entries map_entry  */
#line 1712 "polyray.y"
      { ACTION((yyval.cmap_entry) = map_entry_action2((yyvsp[-1].cmap_entry),
                                                  (yyvsp[0].cmap_entry));) }
#line 4852 "polyray.tab.cc"
    break;

  case 418: /* end_frame_decl: END_FRAME fexper  */
#line 1725 "polyray.y"
      { ACTION(RuntimeState::animator.end_frame = (int)(yyvsp[0].flt);) }
#line 4858 "polyray.tab.cc"
    break;

  case 419: /* start_frame_decl: START_FRAME fexper  */
#line 1730 "polyray.y"
      { ACTION(RuntimeState::animator.start_frame = (int)(yyvsp[0].flt);
               if (!RuntimeState::Parsed_Flag) RuntimeState::animator.current_frame = RuntimeState::animator.start_frame;) }
#line 4865 "polyray.tab.cc"
    break;

  case 420: /* total_frames_decl: TOTAL_FRAMES fexper  */
#line 1736 "polyray.y"
      { ACTION(RuntimeState::animator.total_frames = (int)(yyvsp[0].flt);) }
#line 4871 "polyray.tab.cc"
    break;

  case 421: /* frame_time_decl: FRAME_TIME fexper  */
#line 1741 "polyray.y"
      { ACTION(RuntimeState::animator.frame_time = (yyvsp[0].flt);) }
#line 4877 "polyray.tab.cc"
    break;

  case 422: /* outfile: OUTFILE TOKEN  */
#line 1746 "polyray.y"
      { ACTION(if (!RuntimeState::Parsed_Flag) {
                  RuntimeState::outfilebase = (yyvsp[0].name);
                  RuntimeState::filebaseflag = 1;
                  })
        polyray_free((yyvsp[0].name)); }
#line 4887 "polyray.tab.cc"
    break;

  case 423: /* outfile: OUTFILE STRING  */
#line 1752 "polyray.y"
      { ACTION(if (!RuntimeState::Parsed_Flag) {
                  RuntimeState::outfilebase = (yyvsp[0].name);
                  RuntimeState::filebaseflag = 1;
                  })
        polyray_free((yyvsp[0].name)); }
#line 4897 "polyray.tab.cc"
    break;

  case 424: /* flush_statement: FILE_FLUSH fexper  */
#line 1761 "polyray.y"
      { ACTION(flush_action((int)(yyvsp[0].flt));) }
#line 4903 "polyray.tab.cc"
    break;

  case 425: /* system_call: SYSTEM '(' expression_list ')'  */
#line 1766 "polyray.y"
      { ACTION(evaluate_system_call((yyvsp[-1].elist));) }
#line 4909 "polyray.tab.cc"
    break;

  case 428: /* $@22: %empty  */
#line 1776 "polyray.y"
     { condition_flags[condition_depth] = 1-condition_flags[condition_depth]; }
#line 4915 "polyray.tab.cc"
    break;

  case 431: /* $@23: %empty  */
#line 1783 "polyray.y"
      { Flt ftmp; Vec vtmp; NODE_PTR tnode;
        if (check_condition()) {
           if (eval_node(nullptr, (yyvsp[-1].exper), &ftmp, vtmp, &tnode) == 1 &&
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
#line 4933 "polyray.tab.cc"
    break;

  case 432: /* if_statement: IF '(' conditional ')' $@23 statement if_else_part  */
#line 1797 "polyray.y"
     { condition_depth--; }
#line 4939 "polyray.tab.cc"
    break;


#line 4943 "polyray.tab.cc"

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

#line 1800 "polyray.y"

