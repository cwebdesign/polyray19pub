/*
   pexper.c

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
#include "builder.h"
#include "spline.h"
#include "psupport.h"
#include "symtab.h"
#include "parse.h"
#include "ytab.h"

typedef struct lookup_table_struct {
   char *name;
   int val;
   } fn_table;
static char errbuf[80] ;
#define END(v) (v - 1 + sizeof(v) / sizeof(v[0]))
#ifndef UNDEFINED
#define UNDEFINED -1
#endif

static fn_table exper_fns[] = {
      {"I",                      I_EXPER},
      {"N",                      N_EXPER},
      {"P",                      P_EXPER},
      {"U",                      U_EXPER},
      {"W",                      W_EXPER},
      {"acos",                   ACOS},
      {"asin",                   ASIN},
      {"atan",                   ATAN},
      {"atan2",                  ATAN_TWO},
      {"bias",                   BIAS},
      {"brownian",               FBM},
      {"ceil",                   CEIL},
      {"color_map",              COLOR_MAP},
      {"color_wheel",            COLOR_WHEEL},
      {"concat",                 CONCAT},
      {"cos",                    COS},
      {"cosh",                   COSH},
      {"cylindrical_bumpmap",    CYLINDRICAL_BUMPMAP},
      {"cylindrical_imagemap",   CYLINDRICAL_IMAGEMAP},
      {"cylindrical_indexed_map",CYLINDRICAL_INDEXED},
      {"degrees",                DEGREES},
      {"dnoise",                 DNOISE},
      {"environment",            ENVIRONMENT},
      {"environment_map",        ENVIRONMENT_MAP},
      {"exp",                    EXP},
      {"fabs",                   FABS},
      {"floor",                  FLOOR},
      {"fmod",                   FMOD},
      {"fnoise",                 FNOISE},
      {"gain",                   GAIN},
      {"heightmap",              HEIGHT_MAP},
      {"image",                  IMAGE},
      {"indexed_map",            INDEXED_MAP},
      {"legendre",               LEGENDRE},
      {"ln",                     LN},
      {"log",                    LOG},
      {"max",                    MAXT},
      {"min",                    MINT},
      {"opacity",                OPACITY},
      {"planar_bumpmap",         PLANAR_BUMPMAP},
      {"planar_imagemap",        PLANAR_IMAGEMAP},
      {"pow",                    POWER_EXPER},
      {"radians",                RADIANS},
      {"ramp",                   RAMP},
      {"random",                 RANDOM},
      {"reflect",                REFLECT},
      {"ripple",                 RIPPLE},
      {"sawtooth",               SAWTOOTH},
      {"sin",                    SIN},
      {"sinh",                   SINH},
      {"spherical_bumpmap",      SPHERICAL_BUMPMAP},
      {"spherical_imagemap",     SPHERICAL_IMAGEMAP},
      {"spherical_indexed_map",  SPHERICAL_INDEXED},
      {"spline",                 SPLINE},
      {"sqrt",                   SQRT},
      {"tan",                    TAN},
      {"tanh",                   TANH},
      {"trace",                  TRACE},
      {"u",                      UU_EXPER},
      {"v",                      UV_EXPER},
      {"visible",                VISIBLE},
      {"w",                      UW_EXPER},
      {"wave",                   WAVE},
      {"x",                      X_EXPER},
      {"y",                      Y_EXPER},
      {"z",                      Z_EXPER}
      };
typedef struct lookup_table_struct *tabptr;

static int
lookup(char *name)
{
   tabptr low  = exper_fns;
   tabptr high = END(exper_fns);
   tabptr mid;
   int c;

   while (low <= high) {
      mid = low + (high - low) / 2;
      if ((c = strcmp(mid->name, name)) == 0) {
         return mid->val;
         }
      else if (c < 0)
         low = mid + 1;
      else
         high = mid - 1;
      }
   return UNDEFINED;
}

NODE_PTR
check_term0(char *name)
{
   NODE_PTR result;
   Vec tmp;
   int arg_name = lookup(name);

   switch (arg_name) {
   case I_EXPER:
   case N_EXPER:
   case P_EXPER:
   case UU_EXPER:
   case UV_EXPER:
   case UW_EXPER:
   case U_EXPER:
   case RANDOM:
   case W_EXPER:
   case X_EXPER:
   case Y_EXPER:
   case Z_EXPER:
   case OPACITY:
      result = make_node(arg_name, NULL, NULL);
      break;
   case UNDEFINED:
      /* This is either a predefined color, or an undefined token */
      if (LookupColorByName(name, tmp) == 0) {
         sprintf(errbuf, "Token undefined: \"%s\"\n", name);
         error(errbuf);
         }
      else {
         result = make_vec_node(tmp[0], tmp[1], tmp[2]);
         }
      break;
   default:
      if (LookupColorByName(name, tmp) == 0) {
         /* Not the correct # of arguments to this token */
         sprintf(errbuf, "Wrong # of arguments (0) for: \"%s\"\n", name);
         error(errbuf);
         }
      else {
         result = make_vec_node(tmp[0], tmp[1], tmp[2]);
         }
      break;
   }
   return result;
}

static NODE_PTR
check_term1(char *name, NODE_PTR arg1)
{
   NODE_PTR result;
   char *tstr;
   Object *obj;
   Vec tempv;
   int c, arg_name = lookup(name);

   switch (arg_name) {
   case ACOS:
   case ASIN:
   case ATAN:
   case CEIL:
   case COS:
   case COSH:
   case EXP:
   case FABS:
   case FLOOR:
   case LN:
   case LOG:
   case RAMP:
   case RANDOM:
   case RIPPLE:
   case SAWTOOTH:
   case SIN:
   case SINH:
   case SQRT:
   case TAN:
   case TANH:
   case WAVE:
      result = make_fn1_node(arg_name, arg1);
      break;
   case DEGREES:
      result = make_node(TIMES_EXPER, make_value_node(180.0/M_PI),
                         arg1);
      break;
   case IMAGE:
      if (create_string(arg1, &tstr)) {
         result = make_image_node(tstr, NULL);
         polyray_free(tstr);
         deallocate_node(arg1);
         }
      else {
         sprintf(errbuf, "Non-string used for image\n");
         error(errbuf);
         }
      break;
   case DNOISE:
   case FBM:
   case FNOISE:
      result = make_node(arg_name, arg1, NULL);
      break;
   case RADIANS:
      result = make_node(TIMES_EXPER, make_value_node(M_PI/180.0),
                         arg1);
      break;
   case TRACE:
      result = make_node(TRACE, NULL, arg1);
      break;
   case MAXT:
   case MINT:
      /* Determine the lower left or upper right of an
         objects bounding box */
      if (create_string(arg1, &tstr)) {
         Lookup_Definition(tstr, &c, (void *)&obj);
         if (c != T_OBJECT)
            error("Object '%s' not found in symbol table\n", tstr);
         if (arg_name == MINT) {
            VecCopy(obj->o_bnd.lower_left, tempv)
            result = make_vec_node(tempv[0], tempv[1], tempv[2]);
            }
         else {
            VecAdd(obj->o_bnd.lower_left, obj->o_bnd.lengths, tempv)
            result = make_vec_node(tempv[0], tempv[1], tempv[2]);
            }
         polyray_free(tstr);
         deallocate_node(arg1);
         }
      else
         error("MIN/MAX of an object requires predefined object");
      break;
   case UNDEFINED:
      sprintf(errbuf, "Token undefined: \"%s\"\n", name);
      error(errbuf);
   default:
      /* Not the correct # of arguments to this token */
      sprintf(errbuf, "Wrong # of arguments (1) for: \"%s\"\n", name);
      error(errbuf);
   }
   return result;
}

static NODE_PTR
check_term2(char *name, NODE_PTR arg1, NODE_PTR arg2)
{
   NODE_PTR result;
   char *tstr;
   int arg_name = lookup(name);

   switch (arg_name) {
   case ATAN_TWO:
   case BIAS:
   case GAIN:
   case DNOISE:
   case FNOISE:
   case FBM:
   case FMOD:
   case MAXT:
   case MINT:
   case POWER_EXPER:
   case VISIBLE:
   case ENVIRONMENT_MAP:
   case REFLECT:
   case TRACE:
      result = make_node(arg_name, arg1, arg2);
      break;
   case SPLINE:
      result = make_spline_node(NULL, arg1, arg2, NULL);
      break;
   case HEIGHT_MAP:
   case INDEXED_MAP:
   case PLANAR_IMAGEMAP:
   case SPHERICAL_IMAGEMAP:
   case SPHERICAL_INDEXED:
   case CYLINDRICAL_IMAGEMAP:
   case CYLINDRICAL_INDEXED:
   case PLANAR_BUMPMAP:
   case SPHERICAL_BUMPMAP:
   case CYLINDRICAL_BUMPMAP:
      result = make_fn3_node(arg_name, arg1, arg2, NULL);
      break;
   case IMAGE:
      if (create_string(arg1, &tstr)) {
         result = make_image_node(tstr, arg2);
         polyray_free(tstr);
         deallocate_node(arg1);
         }
      else {
         sprintf(errbuf, "Non-string used for image\n");
         error(errbuf);
         }
      break;
   case UNDEFINED:
      sprintf(errbuf, "Token undefined: \"%s\"\n", name);
      error(errbuf);
   default:
      /* Not the correct # of arguments to this token */
      sprintf(errbuf, "Wrong # of arguments (2) for: \"%s\"\n", name);
      error(errbuf);
   }
   return result;
}

static NODE_PTR
check_term3(char *name, NODE_PTR arg1, NODE_PTR arg2, NODE_PTR arg3)
{
   NODE_PTR result;
   int arg_name = lookup(name);

   switch (arg_name) {
   case SPLINE:
      result = make_spline_node(arg1, arg2, arg3, NULL);
      break;
   case COLOR_WHEEL:
   case INDEXED_MAP:
   case HEIGHT_MAP:
   case LEGENDRE:
   case PLANAR_IMAGEMAP:
   case SPHERICAL_IMAGEMAP:
   case SPHERICAL_INDEXED:
   case CYLINDRICAL_IMAGEMAP:
   case CYLINDRICAL_INDEXED:
   case PLANAR_BUMPMAP:
   case SPHERICAL_BUMPMAP:
   case CYLINDRICAL_BUMPMAP:
      result = make_fn3_node(arg_name, arg1, arg2, arg3);
      break;
   case UNDEFINED:
      sprintf(errbuf, "Token undefined: \"%s\"\n", name);
      error(errbuf);
   default:
      /* Not the correct # of arguments to this token */
      sprintf(errbuf, "Wrong # of arguments (3) for: \"%s\"\n", name);
      error(errbuf);
   }
   return result;
}

static NODE_PTR
check_term4(char *name, NODE_PTR arg1, NODE_PTR arg2, NODE_PTR arg3, NODE_PTR arg4)
{
   NODE_PTR result;
   int arg_name = lookup(name);

   switch (arg_name) {
   case RIPPLE:
      result = make_vector4_node(arg1, arg2, arg3, arg4);
      result->exper_type = RIPPLE;
      break;
   case SPLINE:
      result = make_spline_node(arg1, arg2, arg3, arg4);
      break;
   case UNDEFINED:
      sprintf(errbuf, "Token undefined: \"%s\"\n", name);
      error(errbuf);
   default:
      /* Not the correct # of arguments to this token */
      sprintf(errbuf, "Wrong # of arguments (4) for: \"%s\"\n", name);
      error(errbuf);
   }
   return result;
}

NODE_PTR
check_term(char *name, LIST_PTR args)
{
   LIST_PTR temp;
   NODE_PTR result, arg[6];
   int i, arg_name;

   arg_name = lookup(name);
   if (arg_name == CONCAT)
      result = make_string_node(build_string(args));
   else {
      for (i=0,temp=args;temp!=NULL;i++,temp=temp->next)
         if (i < 6)
            arg[i] = temp->element;
   switch (i) {
      case 0:
         result = check_term0(name);
         break;
      case 1:
         result = check_term1(name, arg[0]);
         break;
      case 2:
         result = check_term2(name, arg[0], arg[1]);
         break;
      case 3:
         result = check_term3(name, arg[0], arg[1], arg[2]);
         break;
      case 4:
         result = check_term4(name, arg[0], arg[1], arg[2], arg[3]);
         break;
      default:
         arg_name = lookup(name);
         if ((arg_name == ENVIRONMENT) && (i == 6)) {
            char *file0, *file1, *file2, *file3, *file4, *file5;

            if (create_string(arg[0], &file0) &&
                create_string(arg[1], &file1) &&
                create_string(arg[2], &file2) &&
                create_string(arg[3], &file3) &&
                create_string(arg[4], &file4) &&
                create_string(arg[5], &file5)) {
               result = make_environ_node(file0, file1, file2,
                                          file3, file4, file5);
               polyray_free(file0); polyray_free(file1);
               polyray_free(file2); polyray_free(file3);
               polyray_free(file4); polyray_free(file5);
               }
            else
               error("Non-string argument to ENVIRONMENT\n");
            for (i=0;i<6;i++)
               deallocate_node(arg[i]);
            }
         else {
            sprintf(errbuf, "Too many arguments (%d) for %s\n", i, name);
            error(errbuf);
            }
      } }

   /* Clean up the memory used to hold the arguments */
   while (args != NULL) {
      temp = args;
      args = args->next;
      polyray_free(temp);
      }

   return result;
}
