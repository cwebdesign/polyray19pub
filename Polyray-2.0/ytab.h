/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_POLYRAY_TAB_H_INCLUDED
# define YY_YY_POLYRAY_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ACCELERATION = 258,            /* ACCELERATION  */
    ACOS = 259,                    /* ACOS  */
    AMBIENT = 260,                 /* AMBIENT  */
    AND_EXPER = 261,               /* AND_EXPER  */
    ANGLE = 262,                   /* ANGLE  */
    ANTIALIAS = 263,               /* ANTIALIAS  */
    ANTIALIAS_THRESHOLD = 264,     /* ANTIALIAS_THRESHOLD  */
    APERTURE = 265,                /* APERTURE  */
    ARRAY = 266,                   /* ARRAY  */
    ASSIGNMENT = 267,              /* ASSIGNMENT  */
    ASIN = 268,                    /* ASIN  */
    ASPECT = 269,                  /* ASPECT  */
    AT = 270,                      /* AT  */
    ATAN = 271,                    /* ATAN  */
    ATAN_TWO = 272,                /* ATAN_TWO  */
    AVOID = 273,                   /* AVOID  */
    BACKGROUND = 274,              /* BACKGROUND  */
    BEZIER = 275,                  /* BEZIER  */
    BIAS = 276,                    /* BIAS  */
    BIRTH = 277,                   /* BIRTH  */
    BLINN = 278,                   /* BLINN  */
    BLOB = 279,                    /* BLOB  */
    BOUNDING_BOX = 280,            /* BOUNDING_BOX  */
    BOX = 281,                     /* BOX  */
    BRILLIANCE = 282,              /* BRILLIANCE  */
    BUMP_SCALE = 283,              /* BUMP_SCALE  */
    CEIL = 284,                    /* CEIL  */
    CHECKER = 285,                 /* CHECKER  */
    CHEIGHT_FIELD = 286,           /* CHEIGHT_FIELD  */
    CHEIGHT_FN = 287,              /* CHEIGHT_FN  */
    COLOR = 288,                   /* COLOR  */
    COLOR_MAP = 289,               /* COLOR_MAP  */
    COLOR_WHEEL = 290,             /* COLOR_WHEEL  */
    CONCAT = 291,                  /* CONCAT  */
    CONCAVE = 292,                 /* CONCAVE  */
    CONDITIONAL_EXPER = 293,       /* CONDITIONAL_EXPER  */
    CONE = 294,                    /* CONE  */
    CONTOUR = 295,                 /* CONTOUR  */
    COOK = 296,                    /* COOK  */
    COS = 297,                     /* COS  */
    COSH = 298,                    /* COSH  */
    COUNT = 299,                   /* COUNT  */
    CSG = 300,                     /* CSG  */
    CYLINDER = 301,                /* CYLINDER  */
    CYLINDRICAL_BUMPMAP = 302,     /* CYLINDRICAL_BUMPMAP  */
    CYLINDRICAL_IMAGEMAP = 303,    /* CYLINDRICAL_IMAGEMAP  */
    CYLINDRICAL_INDEXED = 304,     /* CYLINDRICAL_INDEXED  */
    DEATH = 305,                   /* DEATH  */
    DEFINE = 306,                  /* DEFINE  */
    DEGREES = 307,                 /* DEGREES  */
    DEPTH = 308,                   /* DEPTH  */
    DEPTHMAPPED_LIGHT = 309,       /* DEPTHMAPPED_LIGHT  */
    DIFFUSE = 310,                 /* DIFFUSE  */
    DIRECTIONAL_LIGHT = 311,       /* DIRECTIONAL_LIGHT  */
    DISC = 312,                    /* DISC  */
    DISPLACE = 313,                /* DISPLACE  */
    DITHER = 314,                  /* DITHER  */
    DIV_EXPER = 315,               /* DIV_EXPER  */
    DNOISE = 316,                  /* DNOISE  */
    DOT_EXPER = 317,               /* DOT_EXPER  */
    DRAW = 318,                    /* DRAW  */
    ELSE = 319,                    /* ELSE  */
    END_FRAME = 320,               /* END_FRAME  */
    ENVIRONMENT = 321,             /* ENVIRONMENT  */
    ENVIRONMENT_MAP = 322,         /* ENVIRONMENT_MAP  */
    EQUAL_EXPER = 323,             /* EQUAL_EXPER  */
    EXP = 324,                     /* EXP  */
    EXPRESSION_SYM = 325,          /* EXPRESSION_SYM  */
    FABS = 326,                    /* FABS  */
    FBM = 327,                     /* FBM  */
    FERRARI = 328,                 /* FERRARI  */
    FILE_FLUSH = 329,              /* FILE_FLUSH  */
    FLARE = 330,                   /* FLARE  */
    FLOCK = 331,                   /* FLOCK  */
    FLOOR = 332,                   /* FLOOR  */
    FNOISE = 333,                  /* FNOISE  */
    FOCAL_DISTANCE = 334,          /* FOCAL_DISTANCE  */
    FMOD = 335,                    /* FMOD  */
    FRAME = 336,                   /* FRAME  */
    FRAME_TIME = 337,              /* FRAME_TIME  */
    FREQUENCY = 338,               /* FREQUENCY  */
    FROM = 339,                    /* FROM  */
    FUNCTION = 340,                /* FUNCTION  */
    GAIN = 341,                    /* GAIN  */
    GAUSSIAN = 342,                /* GAUSSIAN  */
    GLYPH = 343,                   /* GLYPH  */
    GREATER_EXPER = 344,           /* GREATER_EXPER  */
    GRIDDED = 345,                 /* GRIDDED  */
    GTEQ_EXPER = 346,              /* GTEQ_EXPER  */
    HAZE = 347,                    /* HAZE  */
    HEIGHT_FIELD = 348,            /* HEIGHT_FIELD  */
    HEIGHT_FN = 349,               /* HEIGHT_FN  */
    HEIGHT_MAP = 350,              /* HEIGHT_MAP  */
    HEXAGON = 351,                 /* HEXAGON  */
    HITHER = 352,                  /* HITHER  */
    HYPERTEXTURE = 353,            /* HYPERTEXTURE  */
    I_EXPER = 354,                 /* I_EXPER  */
    IF = 355,                      /* IF  */
    IMAGE = 356,                   /* IMAGE  */
    IMAGE_FORMAT = 357,            /* IMAGE_FORMAT  */
    IMAGE_WINDOW = 358,            /* IMAGE_WINDOW  */
    INCLUDE = 359,                 /* INCLUDE  */
    INDEXED = 360,                 /* INDEXED  */
    INDEXED_MAP = 361,             /* INDEXED_MAP  */
    LATHE = 362,                   /* LATHE  */
    LAYERED = 363,                 /* LAYERED  */
    LEGENDRE = 364,                /* LEGENDRE  */
    LENSES = 365,                  /* LENSES  */
    LESS_EXPER = 366,              /* LESS_EXPER  */
    LIGHT = 367,                   /* LIGHT  */
    LN = 368,                      /* LN  */
    LOG = 369,                     /* LOG  */
    LOOKUP_FUNCTION = 370,         /* LOOKUP_FUNCTION  */
    LTEQ_EXPER = 371,              /* LTEQ_EXPER  */
    MAXT = 372,                    /* MAXT  */
    MAX_SAMPLES = 373,             /* MAX_SAMPLES  */
    MAX_TRACE_DEPTH = 374,         /* MAX_TRACE_DEPTH  */
    MICROFACET = 375,              /* MICROFACET  */
    MINT = 376,                    /* MINT  */
    MINUS_EXPER = 377,             /* MINUS_EXPER  */
    N_EXPER = 378,                 /* N_EXPER  */
    NO_SHADOW = 379,               /* NO_SHADOW  */
    NOEVAL = 380,                  /* NOEVAL  */
    NOISE = 381,                   /* NOISE  */
    NORMAL = 382,                  /* NORMAL  */
    NOT_EXPER = 383,               /* NOT_EXPER  */
    NUM = 384,                     /* NUM  */
    NURB = 385,                    /* NURB  */
    OBJECT = 386,                  /* OBJECT  */
    OBJECT_SYM = 387,              /* OBJECT_SYM  */
    OCTAVES = 388,                 /* OCTAVES  */
    OR_EXPER = 389,                /* OR_EXPER  */
    OPACITY = 390,                 /* OPACITY  */
    OUTFILE = 391,                 /* OUTFILE  */
    P_EXPER = 392,                 /* P_EXPER  */
    PARABOLA = 393,                /* PARABOLA  */
    PARAMETRIC = 394,              /* PARAMETRIC  */
    PARTICLE = 395,                /* PARTICLE  */
    PARTICLE_SYM = 396,            /* PARTICLE_SYM  */
    PATCH = 397,                   /* PATCH  */
    PHASE = 398,                   /* PHASE  */
    PHONG = 399,                   /* PHONG  */
    PIXEL_ENCODING = 400,          /* PIXEL_ENCODING  */
    PIXELSIZE = 401,               /* PIXELSIZE  */
    PLANAR_BUMPMAP = 402,          /* PLANAR_BUMPMAP  */
    PLANAR_IMAGEMAP = 403,         /* PLANAR_IMAGEMAP  */
    PLANE = 404,                   /* PLANE  */
    PLUS_EXPER = 405,              /* PLUS_EXPER  */
    POINT = 406,                   /* POINT  */
    POLYGON = 407,                 /* POLYGON  */
    POLYNOMIAL = 408,              /* POLYNOMIAL  */
    POSITION = 409,                /* POSITION  */
    POSITION_FUNCTION = 410,       /* POSITION_FUNCTION  */
    POSITION_SCALE = 411,          /* POSITION_SCALE  */
    POWER_EXPER = 412,             /* POWER_EXPER  */
    RADIANS = 413,                 /* RADIANS  */
    RAMP = 414,                    /* RAMP  */
    RANDOM = 415,                  /* RANDOM  */
    RAW = 416,                     /* RAW  */
    REFLECT = 417,                 /* REFLECT  */
    REFLECTION = 418,              /* REFLECTION  */
    REITZ = 419,                   /* REITZ  */
    RESOLUTION = 420,              /* RESOLUTION  */
    RIPPLE = 421,                  /* RIPPLE  */
    ROOT_SOLVER = 422,             /* ROOT_SOLVER  */
    ROTATE = 423,                  /* ROTATE  */
    SAWTOOTH = 424,                /* SAWTOOTH  */
    SCALE = 425,                   /* SCALE  */
    SEED = 426,                    /* SEED  */
    SHADING_FLAGS = 427,           /* SHADING_FLAGS  */
    SHEAR = 428,                   /* SHEAR  */
    SIN = 429,                     /* SIN  */
    SINH = 430,                    /* SINH  */
    SIZE = 431,                    /* SIZE  */
    SMOOTH_HEIGHT_FIELD = 432,     /* SMOOTH_HEIGHT_FIELD  */
    SMOOTH_HEIGHT_FN = 433,        /* SMOOTH_HEIGHT_FN  */
    SMOOTH_CHEIGHT_FIELD = 434,    /* SMOOTH_CHEIGHT_FIELD  */
    SMOOTH_CHEIGHT_FN = 435,       /* SMOOTH_CHEIGHT_FN  */
    SMOOTH_SHEIGHT_FIELD = 436,    /* SMOOTH_SHEIGHT_FIELD  */
    SMOOTH_SHEIGHT_FN = 437,       /* SMOOTH_SHEIGHT_FN  */
    SPACING = 438,                 /* SPACING  */
    SPECIAL = 439,                 /* SPECIAL  */
    SPECIAL_SURFACE_SYM = 440,     /* SPECIAL_SURFACE_SYM  */
    SHEIGHT_FIELD = 441,           /* SHEIGHT_FIELD  */
    SHEIGHT_FN = 442,              /* SHEIGHT_FN  */
    SPHERICAL_BUMPMAP = 443,       /* SPHERICAL_BUMPMAP  */
    SPHERICAL_IMAGEMAP = 444,      /* SPHERICAL_IMAGEMAP  */
    SPHERICAL_INDEXED = 445,       /* SPHERICAL_INDEXED  */
    SPLINE = 446,                  /* SPLINE  */
    SPOT_LIGHT = 447,              /* SPOT_LIGHT  */
    SQRT = 448,                    /* SQRT  */
    SPECULAR = 449,                /* SPECULAR  */
    SPHERE = 450,                  /* SPHERE  */
    START_FRAME = 451,             /* START_FRAME  */
    STATIC = 452,                  /* STATIC  */
    STRING = 453,                  /* STRING  */
    STURM = 454,                   /* STURM  */
    SUBSCRIPT_EXPER = 455,         /* SUBSCRIPT_EXPER  */
    SUMMED = 456,                  /* SUMMED  */
    SUPERQ = 457,                  /* SUPERQ  */
    SURFACE = 458,                 /* SURFACE  */
    SURFACE_SYM = 459,             /* SURFACE_SYM  */
    SYSTEM = 460,                  /* SYSTEM  */
    SWEEP = 461,                   /* SWEEP  */
    TAN = 462,                     /* TAN  */
    TANH = 463,                    /* TANH  */
    TERM = 464,                    /* TERM  */
    TEXTURE = 465,                 /* TEXTURE  */
    TEXTURE_MAP = 466,             /* TEXTURE_MAP  */
    TEXTURE_MAP_SYM = 467,         /* TEXTURE_MAP_SYM  */
    TEXTURE_SYM = 468,             /* TEXTURE_SYM  */
    TEXTURED_LIGHT = 469,          /* TEXTURED_LIGHT  */
    TIMES_EXPER = 470,             /* TIMES_EXPER  */
    TOKEN = 471,                   /* TOKEN  */
    TORUS = 472,                   /* TORUS  */
    TOTAL_FRAMES = 473,            /* TOTAL_FRAMES  */
    TRACE = 474,                   /* TRACE  */
    TRANSFORM = 475,               /* TRANSFORM  */
    TRANSFORM_SYM = 476,           /* TRANSFORM_SYM  */
    TRANSLATE = 477,               /* TRANSLATE  */
    TRANSMISSION = 478,            /* TRANSMISSION  */
    TURBULENCE = 479,              /* TURBULENCE  */
    UMINUS_EXPER = 480,            /* UMINUS_EXPER  */
    UP = 481,                      /* UP  */
    U_EXPER = 482,                 /* U_EXPER  */
    UU_EXPER = 483,                /* UU_EXPER  */
    UV_EXPER = 484,                /* UV_EXPER  */
    UW_EXPER = 485,                /* UW_EXPER  */
    U_STEPS = 486,                 /* U_STEPS  */
    V_STEPS = 487,                 /* V_STEPS  */
    UV = 488,                      /* UV  */
    UV_STEPS = 489,                /* UV_STEPS  */
    UV_BOUNDS = 490,               /* UV_BOUNDS  */
    VELOCITY = 491,                /* VELOCITY  */
    VIETA = 492,                   /* VIETA  */
    VIEWPOINT = 493,               /* VIEWPOINT  */
    VISIBLE = 494,                 /* VISIBLE  */
    VAL_EXPER = 495,               /* VAL_EXPER  */
    VEC_EXPER = 496,               /* VEC_EXPER  */
    VECTOR_EXPER = 497,            /* VECTOR_EXPER  */
    WAVE = 498,                    /* WAVE  */
    W_EXPER = 499,                 /* W_EXPER  */
    W_STEPS = 500,                 /* W_STEPS  */
    X_EXPER = 501,                 /* X_EXPER  */
    X_OFFSET = 502,                /* X_OFFSET  */
    Y_EXPER = 503,                 /* Y_EXPER  */
    Y_OFFSET = 504,                /* Y_OFFSET  */
    YON = 505,                     /* YON  */
    Z_EXPER = 506,                 /* Z_EXPER  */
    AND_SYM = 507,                 /* AND_SYM  */
    OR_SYM = 508,                  /* OR_SYM  */
    LTEQ_SYM = 509,                /* LTEQ_SYM  */
    GTEQ_SYM = 510,                /* GTEQ_SYM  */
    EQUAL_SYM = 511,               /* EQUAL_SYM  */
    NEQUAL_SYM = 512,              /* NEQUAL_SYM  */
    UMINUS = 513                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 341 "polyray.y"

   Vec vec;
   VList *vecl;
   Flt flt;
   Flt *fltptr;
   Object *obj;
   NODE_PTR exper;
   LIST_PTR elist;
   char *name;
   void *data;
   csgnodeptr csgtree;
   Special_Surface *surf;
   Texture *text;
   texture_map_entries text_map;
   texture_fn_entries text_fn;
   map_entries cmap_entry;
   Transform *trns;
   ostackptr objlist;
   tstackptr textlist;
   Particle *part;
   Light *lgt;

#line 345 "ytab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_POLYRAY_TAB_H_INCLUDED  */
