/*
   pexper.cc

  Polyray - MIT Licensed Revival
  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Copyright (C) 1999-2026, Clyde Meli, All rights reserved.

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
#include <utility>
#include <memory>
#include <map>
#include <format> // Requires C++20 or newer

#include "defs3.h"
#include "io_ply.h"
#include "memory.h"
#include "builder.h"
#include "spline.h"
#include "psupport.h"
#include "symtab.h"
#include "parse.h"
#include "colour.h"
#include "polyray.tab.h"

struct lookup_table_struct {
   char *name;
   int val;
   };

constexpr int UNDEFINED = -1;

std::map<std::string,int> exper_fns {
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

//! Lookup Expression Function by Name
/*!
      Searches the expression function map for a function name and returns its ID.
      \param name Function name to lookup
      \return Function ID from exper_fns map, or UNDEFINED if not found
*/
static int
lookup(std::string name)
{
   auto res=exper_fns.find(name);
   if (res != exper_fns.end()) //found
     return res->second;
   return UNDEFINED;
}

//! Check and Construct Expression with Zero Arguments
/*!
      Validates a function name and constructs a parse node for zero-argument expressions.
      Handles predefined constants like I, N, P, coordinates, random values, and named colors.
      \param name Function or constant name to check
      \return Parse node for the expression, or error exit
*/
NODE_PTR
check_term0(char *name)
{
   NODE_PTR result=nullptr;
   Vec tmp;
   std::string sname{name};
   int arg_name = lookup(sname);

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
      result = make_node(arg_name, nullptr, nullptr);
      break;
   case UNDEFINED:
   {
      NuVec tmp;
      /* This is either a predefined color, or an undefined token */
      if (SearchColorByName(name, tmp) == 0) {
          serror(std::format("Token undefined: \"{}\"\n", name));
         }
      else {
         result = make_vec_node(tmp[0], tmp[1], tmp[2]);
         }
      break;
   }
   default:
   {
      NuVec tmp;
      if (SearchColorByName(name, tmp) == 0) {
         // Not the correct # of arguments to this token 
         serror(std::format("Wrong # of arguments (0) for: \"{}\"\n", name));
         }
      else {
         result = make_vec_node(tmp[0], tmp[1], tmp[2]);
         }
      break;
   }
   }
   return result;
}

//! Check and Construct Expression with One Argument
/*!
      Validates a function name and constructs a parse node for single-argument expressions.
      Handles math functions (sin, cos, etc.), image operations, and object bounding box queries.
      \param name Function name to check
      \param arg1 First argument parse node
      \return Parse node for the expression, or error exit
*/
static NODE_PTR
check_term1(char *name, NODE_PTR arg1)
{
   NODE_PTR result=nullptr;
   char *tstr;
   Object *obj;
   Vec tempv;
   std::string sname{name};
   int arg_name = lookup(sname);

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
      result = make_node(TIMES_EXPER, make_value_node(180.0/PYM_PI),
                         arg1);
      break;
   case IMAGE:
      if (C_create_string(arg1, &tstr)) {
         result = make_image_node(tstr, nullptr);
         polyray_free(tstr);
         deallocate_node(arg1);
         }
      else {
         serror("Non-string used for image\n");
         }
      break;
   case DNOISE:
   case FBM:
   case FNOISE:
      result = make_node(arg_name, arg1, nullptr);
      break;
   case RADIANS:
      result = make_node(TIMES_EXPER, make_value_node(PYM_PI/180.0),
                         arg1);
      break;
   case TRACE:
      result = make_node(TRACE, nullptr, arg1);
      break;
   case MAXT:
   case MINT:
      /* Determine the lower left or upper right of an
         objects bounding box */
         int c;
      if (C_create_string(arg1, &tstr)) {
         Lookup_Definition(tstr, &c, (void **)&obj);
         if (c != std::to_underlying(ShapeType::Object))
            serror("Object '%s' not found in symbol table\n", tstr);
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
         serror("MIN/MAX of an object requires predefined object");
      break;
   case UNDEFINED:
      serror(std::format("Token undefined: \"{}\"\n", name));
   default:
      /* Not the correct # of arguments to this token */
      serror(std::format("Wrong # of arguments (1) for: \"{}\"\n", name));
   }
   return result;
}

//! Check and Construct Expression with Two Arguments
/*!
      Validates a function name and constructs a parse node for two-argument expressions.
      Handles binary operations like atan2, power, mapping functions, and visibility tests.
      \param name Function name to check
      \param arg1 First argument parse node
      \param arg2 Second argument parse node
      \return Parse node for the expression, or error exit
*/
static NODE_PTR
check_term2(char *name, NODE_PTR arg1, NODE_PTR arg2)
{
   NODE_PTR result=nullptr;
   char *tstr;
   std::string sname{name};
   int arg_name = lookup(sname);

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
      result = make_spline_node(nullptr, arg1, arg2, nullptr);
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
      result = make_fn3_node(arg_name, arg1, arg2, nullptr);
      break;
   case IMAGE:
      if (C_create_string(arg1, &tstr)) {
         result = make_image_node(tstr, arg2);
         polyray_free(tstr);
         deallocate_node(arg1);
         }
      else {
         serror("Non-string used for image\n");
         }
      break;
   case UNDEFINED:
      serror(std::format("Token undefined: \"{}\"\n", name));
   default:
      /* Not the correct # of arguments to this token */
      serror(std::format("Wrong # of arguments (2) for: \"{}\"\n", name));
   }
   return result;
}

//! Check and Construct Expression with Three Arguments
/*!
      Validates a function name and constructs a parse node for three-argument expressions.
      Handles spline functions, mapping operations, and image queries with coordinates.
      \param name Function name to check
      \param arg1 First argument parse node
      \param arg2 Second argument parse node
      \param arg3 Third argument parse node
      \return Parse node for the expression, or error exit
*/
static NODE_PTR
check_term3(char *name, NODE_PTR arg1, NODE_PTR arg2, NODE_PTR arg3)
{
   NODE_PTR result=nullptr;
   std::string sname{name};
   int arg_name = lookup(sname);

   switch (arg_name) {
   case SPLINE:
      result = make_spline_node(arg1, arg2, arg3, nullptr);
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
      serror(std::format("Token undefined: \"{}\"\n", name));
   default:
      /* Not the correct # of arguments to this token */
      serror(std::format("Wrong # of arguments (3) for: \"{}\"\n", name));
   }
   return result;
}

//! Check and Construct Expression with Four Arguments
/*!
      Validates a function name and constructs a parse node for four-argument expressions.
      Handles ripple functions and spline operations with 4 control points.
      \param name Function name to check
      \param arg1 First argument parse node
      \param arg2 Second argument parse node
      \param arg3 Third argument parse node
      \param arg4 Fourth argument parse node
      \return Parse node for the expression, or error exit
*/
static NODE_PTR
check_term4(char *name, NODE_PTR arg1, NODE_PTR arg2, NODE_PTR arg3, NODE_PTR arg4)
{
   NODE_PTR result=nullptr;
   std::string sname{name};
   int arg_name = lookup(sname);

   switch (arg_name) {
   case RIPPLE:
      result = make_vector4_node(arg1, arg2, arg3, arg4);
      result->exper_type = RIPPLE;
      break;
   case SPLINE:
      result = make_spline_node(arg1, arg2, arg3, arg4);
      break;
   case UNDEFINED:
      serror(std::format("Token undefined: \"{}\"\n", name));
   default:
      /* Not the correct # of arguments to this token */
      serror(std::format("Wrong # of arguments (4) for: \"{}\"\n", name));
   }
   return result;
}

//! Check and Construct Expression with Variable Arguments
/*!
      Main dispatcher function that validates a function name and constructs parse nodes
      for expressions with any number of arguments (0-6).
      Automatically routes to appropriate check_termN based on argument count.
      \param name Function name to check
      \param args Linked list of argument parse nodes
      \return Parse node for the expression, or error exit
      \note Handles special case of ENVIRONMENT function with exactly 6 arguments
*/
NODE_PTR
check_term(char *name, LIST_PTR args)
{
   LIST_PTR temp;
   NODE_PTR result=nullptr, arg[6];
   int i=0;
   std::string sname{name};
   int arg_name = lookup(sname);

   if (arg_name == CONCAT)
      result = make_string_node(build_string(args));
   else {
      for (i=0,temp=args;temp!=nullptr;i++,temp=temp->next)
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
         std::string sname{name};
         int arg_name = lookup(sname);
         if ((arg_name == ENVIRONMENT) && (i == 6)) {
            char *file0, *file1, *file2, *file3, *file4, *file5;

            if (C_create_string(arg[0], &file0) &&
                C_create_string(arg[1], &file1) &&
                C_create_string(arg[2], &file2) &&
                C_create_string(arg[3], &file3) &&
                C_create_string(arg[4], &file4) &&
                C_create_string(arg[5], &file5)) {
               result = make_environ_node(file0, file1, file2,
                                          file3, file4, file5);
               polyray_free(file0); polyray_free(file1);
               polyray_free(file2); polyray_free(file3);
               polyray_free(file4); polyray_free(file5);
               }
            else
               serror("Non-string argument to ENVIRONMENT\n");
            for (i=0;i<6;i++)
               deallocate_node(arg[i]);
            }
         else {
            serror(std::format("Too many arguments ({}) for {}\n", i, name));
            }
      } }

   /* Clean up the memory used to hold the arguments */
   while (args != nullptr) {
      temp = args;
      args = args->next;
      polyray_free(temp);
      }

   return result;
}
