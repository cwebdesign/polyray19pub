/* builder.c

   Build, copy, and print functions for expression trees

  Copyright (C) 1993-1996, Alexander Enzmann, All rights reserved.
  Modifications (c) 2000-2024, C. Meli, All rights reserved.

  This software may be used for any private and non-commercial
  use.

  You may not distribute this software, in whole or in part,
  for any commercial purpose, without the express consent of
  the authors.

  There is no warranty or other guarantee of fitness of this software
  for any purpose.  It is provided solely "as is".

*/

#include "defs.h"
#include "memory.h"
#include "io.h"
#include "image.h"
#include "builder.h"
#include "ytab.h"
#include "spline.h"

static map_entries
copy_cmap_node(map_entries cnode)
{
   map_entries head, last, temp, new_node;
   temp = cnode;
   head = NULL;
   last = NULL;
   while (temp != NULL) {
      new_node = polyray_malloc(sizeof(struct color_map_entry));
      if (new_node == NULL)
         error("Failed to copy a color map entry\n");
      new_node->p0 = temp->p0;
      new_node->p1 = temp->p1;
      VecCopy(temp->v0, new_node->v0);
      VecCopy(temp->v1, new_node->v1);
      new_node->t0 = temp->t0;
      new_node->t1 = temp->t1;
      new_node->next = NULL;
      if (head == NULL) {
         head = new_node;
         last = head;
         }
      else {
         last->next = new_node;
         last = last->next;
         }
      temp = temp->next;
      }
   return head;
}

void
deallocate_cmap_node(map_entries cnode)
{
   map_entries head, temp;

   head = cnode;
   while (head != NULL) {
      temp = head;
      head = head->next;
      polyray_free(temp);
      }
}

static LIST_PTR
copy_list(LIST_PTR entries)
{
   LIST_PTR head, last, temp, new_node;
   temp = entries;
   head = NULL;
   last = NULL;
   while (temp != NULL) {
      new_node = make_list_node(copy_node(temp->element));
      if (new_node == NULL)
         error("Failed to copy an array entry\n");
      new_node->next = NULL;
      if (head == NULL) {
         head = new_node;
         last = head;
         }
      else {
         last->next = new_node;
         last = last->next;
         }
      temp = temp->next;
      }
   return head;
}

/* Do a "deep" copy of a branch of the parse tree. */
NODE_PTR
copy_node(NODE_PTR node)
{
   NODE_PTR new_node;
   int i;
   Img **pics, **tmp;

   if (node == NULL)
      return NULL;
   new_node = polyray_malloc(sizeof(struct exper_node_struct));
   if (new_node == NULL)
      error("Failed to allocate a node\n");
   new_node->exper_type = node->exper_type;
   switch (node->exper_type) {
   case ENVIRONMENT_MAP:
   case PLUS_EXPER:
   case MINUS_EXPER:
   case TIMES_EXPER:
   case DIV_EXPER:
   case DOT_EXPER:
   case POWER_EXPER:
   case EQUAL_EXPER:
   case GREATER_EXPER:
   case GTEQ_EXPER:
   case LESS_EXPER:
   case LTEQ_EXPER:
   case NOT_EXPER:
   case OR_EXPER:
   case AND_EXPER:
   case SUBSCRIPT_EXPER:
   case ATAN_TWO:
   case BIAS:
   case GAIN:
   case FMOD:
   case MAXT:
   case MINT:
   case FBM:
   case FNOISE:
   case DNOISE:
   case NOISE:
   case REFLECT:
   case TRACE:
   case VISIBLE:
   case I_EXPER:
   case N_EXPER:
   case P_EXPER:
   case U_EXPER:
   case UU_EXPER:
   case UV_EXPER:
   case UW_EXPER:
   case W_EXPER:
   case X_EXPER:
   case Y_EXPER:
   case Z_EXPER:
   case COLOR:
   case OPACITY:
   case RANDOM:
   case START_FRAME:
   case END_FRAME:
   case TOTAL_FRAMES:
   case FRAME:
      break;
   case STRING:
      new_node->exper_data.str =
         polyray_malloc((strlen(node->exper_data.str) + 1) * sizeof(char));
      if (new_node->exper_data.str == NULL)
         error("Failed to allocate a string node\n");
      strcpy(new_node->exper_data.str, node->exper_data.str);
      break;
   case IMAGE:
      { Img *tmp = (Img *)polyray_malloc(sizeof(Img));
        if (tmp == NULL)
           error("Failed to allocate picture data\n");
        memcpy(tmp, node->exper_data.image, sizeof(Img));
        tmp->copy = 1;
        new_node->exper_data.image = tmp;
      }
      break;
   case ENVIRONMENT:
      { pics = node->exper_data.images;
        tmp = (Img **)polyray_malloc(6 * sizeof(Img *));
        if (tmp == NULL) error("Failed to allocate environment data\n");
        for (i=0;i<6;i++) {
           tmp[i] = (Img *)polyray_malloc(sizeof(Img));
           if (tmp[i] == NULL) error("Failed to allocate environment data\n");
           memcpy(tmp[i], pics[i], sizeof(Img));
/* printf("Copy environment image %d, %p -> %p\n", i, pics[i], tmp[i]); */
           tmp[i]->copy = 1;
           }
        new_node->exper_data.images = tmp;
      }
      break;
   case VAL_EXPER:
      new_node->exper_data.value = node->exper_data.value;
      break;
   case VEC_EXPER:
      VecCopy(node->exper_data.v, new_node->exper_data.v);
      break;
   case RIPPLE:
   case VECTOR_EXPER:
      new_node->exper_data.vec[0] = copy_node(node->exper_data.vec[0]);
      new_node->exper_data.vec[1] = copy_node(node->exper_data.vec[1]);
      new_node->exper_data.vec[2] = copy_node(node->exper_data.vec[2]);
      new_node->exper_data.vec[3] = copy_node(node->exper_data.vec[3]);
      break;
   case TERM:
      new_node->exper_data.coeff.coeff   = node->exper_data.coeff.coeff;
      new_node->exper_data.coeff.x_power = node->exper_data.coeff.x_power;
      new_node->exper_data.coeff.y_power = node->exper_data.coeff.y_power;
      new_node->exper_data.coeff.z_power = node->exper_data.coeff.z_power;
      break;
   case SPLINE:
      new_node->exper_data.data = copy_spline_node(node->exper_data.data);
      break;
   case ROTATE:
   case COLOR_WHEEL:
   case CONDITIONAL_EXPER:
   case ACOS:
   case ASIN:
   case ATAN:
   case CEIL:
   case COS:
   case COSH:
   case EXP:
   case FABS:
   case FLOOR:
   case HEIGHT_MAP:
   case INDEXED_MAP:
   case CYLINDRICAL_INDEXED:
   case SPHERICAL_INDEXED:
   case LEGENDRE:
   case CYLINDRICAL_BUMPMAP:
   case PLANAR_BUMPMAP:
   case SPHERICAL_BUMPMAP:
   case CYLINDRICAL_IMAGEMAP:
   case PLANAR_IMAGEMAP:
   case SPHERICAL_IMAGEMAP:
   case LN:
   case LOG:
   case RAMP:
   case SAWTOOTH:
   case SIN:
   case SINH:
   case SQRT:
   case TAN:
   case TANH:
      new_node->exper_data.param = copy_node(node->exper_data.param);
      break;
   case COLOR_MAP:
      new_node->exper_data.cmap = copy_cmap_node(node->exper_data.cmap);
      break;
   case ARRAY:
      new_node->exper_data.array = copy_list(node->exper_data.array);
      break;
   default:
      error("Bad node type in copy_node: %d\n", node->exper_type);
   }
   new_node->left = copy_node(node->left);
   new_node->right = copy_node(node->right);
   return new_node;
}

/* Deallocate all of the memory on a term list. */
void
deallocate_list(LIST_PTR term_list)
{
   LIST_PTR temp_list;

   while (term_list!=NULL) {
      deallocate_node(term_list->element);
      temp_list = term_list;
      term_list = term_list->next;
      polyray_free(temp_list);
      }
}

static void
free_image_memory(Img *image)
{
unsigned /* CM 9/MAR/2000 */
   int i;

/* printf("Image->copy = %d\n", image->copy); */
   polyray_free(image->filename);
   for (i=0;i<image->length;i++)
      polyray_free(image->image[i]);
   polyray_free(image->image);
   if (image->cmap != NULL)
      polyray_free(image->cmap);
}

/* Free up all of the memory used by a branch of the parse tree. */
void
deallocate_node(NODE_PTR node)
{
   int i;
   Img **tmp;

   if (node == NULL)
      return;
   else {
      switch (node->exper_type) {
      case ENVIRONMENT_MAP:
      case EQUAL_EXPER:
      case LESS_EXPER:
      case LTEQ_EXPER:
      case GREATER_EXPER:
      case GTEQ_EXPER:
      case NOT_EXPER:
      case OR_EXPER:
      case AND_EXPER:
      case I_EXPER:
      case N_EXPER:
      case P_EXPER:
      case U_EXPER:
      case UU_EXPER:
      case UV_EXPER:
      case UW_EXPER:
      case W_EXPER:
      case X_EXPER:
      case Y_EXPER:
      case Z_EXPER:
      case COLOR:
      case OPACITY:
      case RANDOM:
      case START_FRAME:
      case END_FRAME:
      case TOTAL_FRAMES:
      case FRAME:
      case VAL_EXPER:
      case PLUS_EXPER:
      case MINUS_EXPER:
      case TIMES_EXPER:
      case DIV_EXPER:
      case DOT_EXPER:
      case POWER_EXPER:
      case UMINUS_EXPER:
      case TERM:
      case VEC_EXPER:
      case SUBSCRIPT_EXPER:
      case ATAN_TWO:
      case BIAS:
      case GAIN:
      case FMOD:
      case MINT:
      case MAXT:
      case FBM:
      case FNOISE:
      case DNOISE:
      case NOISE:
      case REFLECT:
      case TRACE:
      case VISIBLE:
         break;
      case ROTATE:
      case COLOR_WHEEL:
      case CONDITIONAL_EXPER:
      case CYLINDRICAL_BUMPMAP:
      case CYLINDRICAL_IMAGEMAP:
      case CYLINDRICAL_INDEXED:
      case ACOS:
      case ASIN:
      case ATAN:
      case CEIL:
      case COS:
      case COSH:
      case EXP:
      case FABS:
      case FLOOR:
      case HEIGHT_MAP:
      case INDEXED_MAP:
      case LEGENDRE:
      case LN:
      case LOG:
      case PLANAR_IMAGEMAP:
      case PLANAR_BUMPMAP:
      case RAMP:
      case SAWTOOTH:
      case SIN:
      case SINH:
      case SPHERICAL_BUMPMAP:
      case SPHERICAL_IMAGEMAP:
      case SPHERICAL_INDEXED:
      case SQRT:
      case TAN:
      case TANH:
         deallocate_node(node->exper_data.param);
         break;
      case ARRAY:
         deallocate_list(node->exper_data.array);
         break;
      case STRING:
         polyray_free(node->exper_data.str);
         break;
      case COLOR_MAP:
         deallocate_cmap_node(node->exper_data.cmap);
         break;
      case IMAGE:
         { Img *img = node->exper_data.image;
           if (img->copy == 0)
              free_image_memory(img);
           polyray_free(node->exper_data.image);
         }
         break;  
      case ENVIRONMENT:
         { tmp = (Img **)node->exper_data.images;
           for (i=0;i<6;i++) {
              if (tmp[i]->copy == 0) {
/* printf("Free environment image %d, %p\n", i, tmp[i]); */
                 free_image_memory(tmp[i]);
                 }
              polyray_free(tmp[i]);
              }
           polyray_free(tmp);
         }
      break;
      case SPLINE:
         deallocate_spline_node(node);
      break;
      case RIPPLE:
      case VECTOR_EXPER:
         deallocate_node(node->exper_data.vec[0]);
         deallocate_node(node->exper_data.vec[1]);
         deallocate_node(node->exper_data.vec[2]);
         deallocate_node(node->exper_data.vec[3]);
         break;
      default:
         error("Can't deallocate node of type: %d\n", node->exper_type);
      }
      deallocate_node(node->left);
      deallocate_node(node->right);
      polyray_free(node);
      }
}

/* Allocate memory for a node of a parse tree. */
NODE_PTR
make_node(int type, NODE_PTR left, NODE_PTR right)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL)
      error("Failed to allocate a node\n");
   node->exper_type = type;
   node->left = left;
   node->right = right;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a floating
   point number. */
NODE_PTR
make_value_node(Flt value)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      error("Failed to allocate a value node\n");
   node->exper_type = VAL_EXPER;
   node->exper_data.value = value;
   node->left = NULL;
   node->right = NULL;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a string */
NODE_PTR
make_string_node(char *value)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));
   char *temp_str = polyray_malloc((strlen(value) + 1) * sizeof(char));
   if (node == NULL || temp_str == NULL)
      error("Failed to allocate a string node\n");
   strcpy(temp_str, value);
   node->exper_type = STRING;
   node->exper_data.str = temp_str;
   node->left = NULL;
   node->right = NULL;
   return node;
}

NODE_PTR
make_image_node(char *filename, NODE_PTR support)
{
   NODE_PTR node;

   node = polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      error("Failed to allocate an image node\n");
   node->exper_type = IMAGE;
   node->exper_data.image = TGAReadImage(filename);
   node->left  = support;
   node->right = NULL;
   return node;
}

NODE_PTR
make_environ_node(char *file0, char *file1, char *file2,
                  char *file3, char *file4, char *file5)
{
   NODE_PTR node;

   node = polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      error("Failed to allocate an image node\n");
   node->exper_type = ENVIRONMENT;
   node->exper_data.images = (Img **)polyray_malloc(6 * sizeof(Img *));
   if (node->exper_data.image == NULL)
      error("Failed to allocate environment buffer\n");
   node->left  = NULL;
   node->right = NULL;
   node->exper_data.images[0] = TGAReadImage(file0);
   node->exper_data.images[1] = TGAReadImage(file1);
   node->exper_data.images[2] = TGAReadImage(file2);
   node->exper_data.images[3] = TGAReadImage(file3);
   node->exper_data.images[4] = TGAReadImage(file4);
   node->exper_data.images[5] = TGAReadImage(file5);
   return node;
}

NODE_PTR
make_vector3_node(NODE_PTR node0, NODE_PTR node1, NODE_PTR node2)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL) error("Failed to allocate a vector node\n");
   node->exper_type = VECTOR_EXPER;
   node->exper_data.vec[0] = node0;
   node->exper_data.vec[1] = node1;
   node->exper_data.vec[2] = node2;
   node->exper_data.vec[3] = NULL;

   node->left = NULL;
   node->right = NULL;
   return node;
}

NODE_PTR
make_vector4_node(NODE_PTR node0, NODE_PTR node1,
                  NODE_PTR node2, NODE_PTR node3)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL) error("Failed to allocate a vector node\n");
   node->exper_type = VECTOR_EXPER;
   node->exper_data.vec[0] = node0;
   node->exper_data.vec[1] = node1;
   node->exper_data.vec[2] = node2;
   node->exper_data.vec[3] = node3;

   node->left = NULL;
   node->right = NULL;
   return node;
}

NODE_PTR
make_vec_node(Flt val0, Flt val1, Flt val2)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL) error("Failed to allocate a vector node\n");
   node->exper_type = VEC_EXPER;
   node->exper_data.v[0] = val0;
   node->exper_data.v[1] = val1;
   node->exper_data.v[2] = val2;
   node->left = NULL;
   node->right = NULL;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a floating
   point number. */
NODE_PTR
make_value_term_node(Flt value)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      error("Failed to allocate a value node\n");
   node->exper_type = TERM;
   node->exper_data.coeff.coeff = value;
   node->exper_data.coeff.x_power = 0.0;
   node->exper_data.coeff.y_power = 0.0;
   node->exper_data.coeff.z_power = 0.0;
   node->left = NULL;
   node->right = NULL;
   return node;
}

/* Allocate a single element of an expression list. */
LIST_PTR
make_list_node(NODE_PTR node)
{
   LIST_PTR temp = polyray_malloc(sizeof(struct exper_list_struct));
   if (temp == NULL)
      error("Failed to allocate list node\n");
   temp->element = node;
   temp->next = NULL;
   return temp;
}

/* Allocate memory for a node of a parse tree. */
NODE_PTR
make_array_node(LIST_PTR elist)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL) error("Failed to allocate a node\n");
   node->exper_type = ARRAY;
   node->exper_data.array = elist;
   node->left  = NULL;
   node->right = NULL;
   return node;
}

/* Allocate memory for a function node */
NODE_PTR
make_fn1_node(int fntype, NODE_PTR exper)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL)
      error("Failed to allocate a function(1) node\n");
   node->exper_type = fntype;
   node->exper_data.param = exper;
   node->left = NULL;
   node->right = NULL;
   return node;
}

/* Allocate memory for a function node */
NODE_PTR
make_fn2_node(int fntype, NODE_PTR exper1, NODE_PTR exper2)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      error("Failed to allocate a function(2) node\n");
   node->exper_type = fntype;
   node->left = exper1;
   node->right = exper2;
   return node;
}

/* Allocate memory for a function node */
NODE_PTR
make_fn3_node(int fntype, NODE_PTR exper1, NODE_PTR exper2, NODE_PTR exper3)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL)
      error("Failed to allocate a value node\n");
   node->exper_type = fntype;
   node->exper_data.param = exper1;
   node->left  = exper2;
   node->right = exper3;
   return node;
}

/* Allocate memory for a function node */
NODE_PTR
make_cond_node(NODE_PTR condition, NODE_PTR exper1, NODE_PTR exper2)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL)
      error("Failed to allocate a value node\n");
   node->exper_type = CONDITIONAL_EXPER;
   node->exper_data.param = condition;
   node->left  = exper1;
   node->right = exper2;
   return node;
}

/* Allocate memory for a node of a parse tree. */
NODE_PTR
make_cmap_node(map_entries map, NODE_PTR deflt)
{
   NODE_PTR node = polyray_malloc(sizeof(struct exper_node_struct));

   if (node == NULL) error("Failed to allocate a node\n");
   node->exper_type = COLOR_MAP;
   node->exper_data.cmap = map;
   node->left  = deflt;
   node->right = NULL;
   return node;
}

static char *
lookup_fn_name(int fntype)
{
   switch (fntype) {
   case ACOS:
      return "acos";
   case ASIN:
      return "asin";
   case ATAN:
      return "atan";
   case ATAN_TWO:
      return "atan2";
   case BIAS:
      return "bias";
   case CEIL:
      return "ceil";
   case COLOR_MAP:
      return "color_map";
   case COS:
      return "cos";
   case COSH:
      return "cosh";
   case DNOISE:
      return "dnoise";
   case EXP:
      return "exp";
   case FABS:
      return "fabs";
   case FBM:
      return "fbm";
   case FNOISE:
      return "fnoise";
   case FLOOR:
      return "floor";
   case FMOD:
      return "fmod";
   case GAIN:
      return "gain";
   case LN:
      return "ln";
   case LOG:
      return "log";
   case MAXT:
      return "max";
   case MINT:
      return "min";
   case NOISE:
      return "noise";
   case RAMP:
      return "ramp";
   case REFLECT:
      return "reflect";
   case ROTATE:
      return "rotate";
   case SAWTOOTH:
      return "sawtooth";
   case SIN:
      return "sin";
   case SPLINE:
      return "spline";
   case SQRT:
      return "sqrt";
   case TAN:
      return "tan";
   case TANH:
      return "tanh";
   }
   return "Unknown";
}

static void
show_cmap_node(map_entries cnode)
{
   map_entries temp = cnode;
   while (temp != NULL) {
      message("[%g, %g, <%g, %g, %g>, <%g, %g, %g>]\n",
              temp->p0, temp->p1,
              temp->v0[0], temp->v0[1], temp->v0[2],
              temp->v1[0], temp->v1[1], temp->v1[2]);
      temp = temp->next;
      }
}

static void
show_array_node(LIST_PTR list)
{
   LIST_PTR temp = list;
   while (temp != NULL) {
      show_node(temp->element);
      if (temp->next != NULL)
         message(", ");
      temp = temp->next;
      }
}

/* Print the contents of a parse node to the screen. This routine
   recursively*/
void
show_node(NODE_PTR node)
{
   int tflag, i;
   if (node == NULL) {
      message("(NULL)");
      return;
      }

   switch (node->exper_type) {
   case TERM:
      tflag = 0;
      if (node->exper_data.coeff.coeff != 1.0) {
          message("%lg", node->exper_data.coeff.coeff);
          tflag = 1;
          }
      else if (node->exper_data.coeff.x_power == 0.0 &&
               node->exper_data.coeff.y_power == 0.0 &&
               node->exper_data.coeff.z_power == 0.0)
          message("1");
      if (node->exper_data.coeff.x_power > 0.0) {
         if (tflag) message("*");
         if (node->exper_data.coeff.x_power != 1.0)
            message("x^%lg", node->exper_data.coeff.x_power);
         else
            message("x");
         tflag = 1;
         }
      if (node->exper_data.coeff.y_power > 0.0) {
         if (tflag) message("*");
         if (node->exper_data.coeff.y_power != 1)
            message("y^%lg", node->exper_data.coeff.y_power);
         else
            message("y");
         tflag = 1;
         }
      if (node->exper_data.coeff.z_power > 0.0) {
         if (tflag) message("*");
         if (node->exper_data.coeff.z_power != 1)
            message("z^%lg", node->exper_data.coeff.z_power);
         else
            message("z");
         }
      break;
   case I_EXPER:
      message("I");
      break;
   case N_EXPER:
      message("N");
      break;
   case P_EXPER:
      message("P");
      break;
   case U_EXPER:
      message("U", node->exper_data.value);
      break;
   case UU_EXPER:
      message("u", node->exper_data.value);
      break;
   case UV_EXPER:
      message("v", node->exper_data.value);
      break;
   case UW_EXPER:
      message("2", node->exper_data.value);
      break;
   case VAL_EXPER:
      message("%lg", node->exper_data.value);
      break;
   case W_EXPER:
      message("W");
      break;
   case X_EXPER:
      message("x");
      break;
   case Y_EXPER:
      message("y");
      break;
   case Z_EXPER:
      message("z");
      break;
   case COLOR:
      message("color");
      break;
   case OPACITY:
      message("opacity");
      break;
   case RANDOM:
      message("random");
      break;
   case START_FRAME:
      message("start_frame");
      break;
   case END_FRAME:
      message("end_frame");
      break;
   case TOTAL_FRAMES:
      message("total_frames");
      break;
   case FRAME:
      message("frame");
      break;
   case VEC_EXPER:
      message("<");
      for (i=0;i<VECTOR_LENGTH-1;i++) {
         message("%lg", node->exper_data.v[i]);
         if (i < VECTOR_LENGTH-2)
            message(", ");
         }
      message(">");
      break;
   case ENVIRONMENT:
      message("environment(");
      for (i=0;i<6;i++) {
         message("\"%s\"", node->exper_data.images[i]->filename);
         if (i < 5)
            message(", ");
         else
            message(")");
         }
      break;
   case IMAGE:
      message("image(\"%s\")", node->exper_data.image->filename);
      break;
   case INDEXED_MAP:
      message("indexed(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case CYLINDRICAL_INDEXED:
      message("cylindrical_indexed_map(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case SPHERICAL_INDEXED:
      message("spherical_indexed_map(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case HEIGHT_MAP:
      message("heightmap(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case PLANAR_IMAGEMAP:
      message("planar_imagemap(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case CYLINDRICAL_IMAGEMAP:
      message("cylindrical_imagemap(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case SPHERICAL_IMAGEMAP:
      message("spherical_imagemap(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case COLOR_MAP:
      message("color_map(");
      show_cmap_node(node->exper_data.cmap);
      message(")");
      break;
   case ARRAY:
      message("[");
      show_array_node(node->exper_data.array);
      message("]");
      break;
   case STRING:
      message("\"%s\"", node->exper_data.str);
      break;
   case COLOR_WHEEL:
      message("color_wheel(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case CONDITIONAL_EXPER:
      message("(");
      show_node(node->exper_data.param);
      message("?");
      show_node(node->left);
      message(":");
      show_node(node->right);
      message(")");
      break;
   case LEGENDRE:
      message("legendre(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      message(",");
      show_node(node->right);
      message(")");
      break;
   case RIPPLE:
      message("ripple(");
      show_node(node->exper_data.vec[0]);
      message(", ");
      show_node(node->exper_data.vec[1]);
      message(", ");
      show_node(node->exper_data.vec[2]);
      message(", ");
      show_node(node->exper_data.vec[3]);
      message(")");
      break;
   case VECTOR_EXPER:
      message("<");
      show_node(node->exper_data.vec[0]);
      message(", ");
      show_node(node->exper_data.vec[1]);
      message(", ");
      show_node(node->exper_data.vec[2]);
      message(">");
      break;
   case PLUS_EXPER:
      message("(");
      show_node(node->left);
      message("+");
      show_node(node->right);
      message(")");
      break;
   case MINUS_EXPER:
      message("(");
      show_node(node->left);
      message("-");
      show_node(node->right);
      message(")");
      break;
   case TIMES_EXPER:
      message("(");
      show_node(node->left);
      message("*");
      show_node(node->right);
      message(")");
      break;
   case DIV_EXPER:
      message("(");
      show_node(node->left);
      message("/");
      show_node(node->right);
      message(")");
      break;
   case DOT_EXPER:
      message("(");
      show_node(node->left);
      message(".");
      show_node(node->right);
      message(")");
      break;
   case POWER_EXPER:
      message("(");
      show_node(node->left);
      message("^");
      show_node(node->right);
      message(")");
      break;
   case UMINUS_EXPER:
      message("(");
      message("-");
      show_node(node->left);
      message(")");
      break;
   case EQUAL_EXPER:
      show_node(node->left);
      message("==");
      show_node(node->right);
      break;
   case GREATER_EXPER:
      show_node(node->left);
      message(">");
      show_node(node->right);
      break;
   case GTEQ_EXPER:
      show_node(node->left);
      message(">=");
      show_node(node->right);
      break;
   case LESS_EXPER:
      show_node(node->left);
      message("<");
      show_node(node->right);
      break;
   case LTEQ_EXPER:
      show_node(node->left);
      message("<=");
      show_node(node->right);
      break;
   case NOT_EXPER:
      message("!");
      show_node(node->left);
      break;
   case AND_EXPER:
      show_node(node->left);
      message("&&");
      show_node(node->right);
      break;
   case OR_EXPER:
      show_node(node->left);
      message("||");
      show_node(node->right);
      break;
   case ENVIRONMENT_MAP:
      message("environment_map(");
      show_node(node->left);
      message(", ");
      show_node(node->right);
      message(")");
      break;
   case SUBSCRIPT_EXPER:
      show_node(node->left);
      message("[");
      show_node(node->right);
      message("]");
      break;
   case ATAN_TWO:
   case GAIN:
   case BIAS:
   case FNOISE:
   case FBM:
   case DNOISE:
   case NOISE:
      message("%s(", lookup_fn_name(node->exper_type));
      show_node(node->left);
      if (node->right != NULL) {
         message(", ");
         show_node(node->right);
         }
      message(")");
      break;
   case TRACE:
      message("trace");
      if (node->left != NULL) {
         show_node(node->left);
         message(", ");
         }
      show_node(node->right);
      message(")");
      break;
   case ROTATE:
      message("rotate(");
      show_node(node->exper_data.param);
      message(",");
      show_node(node->left);
      if (node->right != NULL) {
         message(", ");
         show_node(node->right);
         }
      message(")");
      break;
   case SPLINE:
      show_spline_node(node);
      break;
   case FMOD:
   case MAXT:
   case MINT:
   case VISIBLE:
   case REFLECT:
      message("%s(", lookup_fn_name(node->exper_type));
      show_node(node->exper_data.param);
      message(", ");
      show_node(node->left);
      if (node->right) {
         message(", ");
         show_node(node->right);
         }
      message(")");
      break;
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
   case SAWTOOTH:
   case SIN:
   case SINH:
   case SQRT:
   case TAN:
   case TANH:
      message("%s(", lookup_fn_name(node->exper_type));
      show_node(node->exper_data.param);
      message(")");
      break;
   default:
      error("Bad node type in show_node: %d\n", node->exper_type);
   }
}

DrawNode *
make_draw_node(Flt low, Flt high, int steps,
               NODE_PTR draw_fn, NODE_PTR color_fn)
{
   DrawNode *node;

   node = polyray_malloc(sizeof(DrawNode));
   node->low = low;
   node->high = high;
   node->steps = steps;
   node->draw_fn = draw_fn;
   node->color_fn = color_fn;
   node->next = NULL;

   return node;
}

void
delete_draw_nodes(DrawNode *node)
{
   DrawNode *tnode;

   while (node!=NULL) {
      deallocate_node(node->draw_fn);
      deallocate_node(node->color_fn);
      tnode = node;
      node  = node->next;
      polyray_free(tnode);
      }
}
