/* builder.cc

   Build, copy, and print functions for expression trees

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

#include <memory>
#include <map>
#include <iostream>
#include <string>
#include <string_view>
#include <gsl/gsl>

#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "image.h"
#include "builder.h"
#include "polyray.tab.h"
#include "spline.h"
#include "util.h"
#include "factory.h"

Img* SafeReadImage(char *filename, std::string errmsg);

Img* SafeReadImage(char *filename, std::string errmsg)
{
   Img* hfimg=nullptr;
   auto ret = ReadImage(filename);
   if (ret.has_value())
      hfimg = ret.value();
   else {
      serror("%s",(errmsg+": "+ret.error()).c_str());
      exit(1);//not really needed
   }
   return hfimg;
}

#ifndef TESTING
static
#endif
/**
 * @brief Deep-copy a linked list of color-map entries.
 *
 * @param cnode Head of the color-map entry list to copy.
 * @return Head of the copied color-map entry list.
 */
map_entries copy_cmap_node(map_entries cnode)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"copy_cmap_node\n"<<std::flush;
   #endif
   map_entries head, last, temp, new_node;
   temp = cnode;
   head = nullptr;
   last = nullptr;
   while (temp != nullptr) {
      #ifdef DEBUG
      std::cout<<"allocating color map\n";
      #endif
      new_node = Factorycolor_map_entry();
      if (new_node == nullptr)
         serror("Failed to copy a color map entry\n");
      new_node->p0 = temp->p0;
      new_node->p1 = temp->p1;
      VecCopy(temp->v0, new_node->v0);
      VecCopy(temp->v1, new_node->v1);
      new_node->t0 = temp->t0;
      new_node->t1 = temp->t1;
      new_node->next = nullptr;
      if (head == nullptr) {
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

/**
 * @brief Free a linked list of color-map entries.
 *
 * @param cnode Head of the color-map entry list to free.
 * @return No return value.
 */
void deallocate_cmap_node(map_entries cnode)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"builder::deallocate_cmap_node\n"<<std::flush; 
   #endif
   map_entries head, temp;

   head = cnode;
   while (head != nullptr) {
      temp = head;
      head = head->next;
      delete temp;
      }
}

#ifndef TESTING
static
#endif
/**
 * @brief Deep-copy a linked list of expression-list entries.
 *
 * @param entries Head of the list to copy.
 * @return Head of the copied list.
 */
LIST_PTR copy_list(LIST_PTR entries)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"builder::copy_list\n"<<std::flush;
   #endif
   LIST_PTR head, last, temp, new_node;
   temp = entries;
   head = nullptr;
   last = nullptr;
   while (temp != nullptr) {
      new_node = make_list_node(copy_node(temp->element));
      if (new_node == nullptr)
         serror("Failed to copy an array entry\n");
      new_node->next = nullptr;
      if (head == nullptr) {
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

/**
 * @brief Adapter that deep-copies a parse-tree node passed as `void *`.
 *
 * @param vnode Opaque pointer to the node to copy.
 * @return Deep copy of the supplied node.
 */
NODE_PTR copy_node_void(void *vnode)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"builder::copy_node_void\n"<<std::flush;
   #endif
  NODE_PTR node=(NODE_PTR)vnode;
  return copy_node(node);
}

/**
 * @brief Copy string expression payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_string(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data=std::get<std::string>(node->exper_data);
}
/**
 * @brief Copy image expression payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_image(NODE_PTR new_node, NODE_PTR node)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"builder::do_image\n"<<std::flush;
   #endif
   Img *tmp = FactoryImg();
   *tmp = *std::get<Img*>(node->exper_data);
   tmp->copy = 1;
   new_node->exper_data = tmp;
}

/**
 * @brief Copy environment-map payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_environment(NODE_PTR new_node, NODE_PTR node)
{
   auto pics = std::get<Img**>(node->exper_data);
   auto tmp = (Img **)polyray_malloc(6 * sizeof(Img *));
   if (tmp == nullptr) serror("Failed to allocate environment data\n");
   for (int i=0;i<6;i++) {
      tmp[i] = FactoryImg();
      *tmp[i] = *pics[i]; //-V1004
/* printf("Copy environment image %d, %p -> %p\n", i, pics[i], tmp[i]); */
      tmp[i]->copy = 1;
   }
   new_node->exper_data = tmp;
}
/**
 * @brief Copy scalar expression payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_val_exper(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data = std::get<Flt>(node->exper_data);
}
/**
 * @brief Copy fixed vector payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_vec_exper(NODE_PTR new_node, NODE_PTR node)
{
   NuVec tmp = std::get<NuVec>(node->exper_data);
   new_node->exper_data = tmp;
}
/**
 * @brief Copy vector-expression child-node payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_vector_exper(NODE_PTR new_node, NODE_PTR node)
{
   auto vecarr=std::get<std::array<NODE_PTR, VECTOR_LENGTH>>(node->exper_data);
   new_node->exper_data = vecarr;
}
/**
 * @brief Copy polynomial coefficient payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_coeff_exper(NODE_PTR new_node, NODE_PTR node)
{
   auto coeff=std::get<coeff_node>(node->exper_data);
   new_node->exper_data = coeff;
}
/**
 * @brief Copy spline payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_spline_exper(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data = copy_spline_node(std::get<spline_node*>(node->exper_data));
}
/**
 * @brief Copy a single child-node payload used by unary expression nodes.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_tanh(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data = copy_node(std::get<NODE_PTR>(node->exper_data));
}
/**
 * @brief Copy color-map payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_color_map(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data = copy_cmap_node(std::get<map_entries>(node->exper_data));
}
/**
 * @brief Copy array payload data into a new node.
 *
 * @param new_node Destination node receiving copied payload data.
 * @param node Source node providing payload data.
 * @return No return value.
 */
static void do_array(NODE_PTR new_node, NODE_PTR node)
{
   new_node->exper_data = copy_list(std::get<LIST_PTR>(node->exper_data));
}
/* Do a "deep" copy of a branch of the parse tree. */
/**
 * @brief Deep-copy a parse-tree branch.
 *
 * @param node Root of the branch to copy.
 * @return Root of the copied branch, or `NULL` when `node` is null.
 */
NODE_PTR copy_node(NODE_PTR node)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"builder::copy_node\n"<<std::flush;
   #endif
   NODE_PTR new_node;

   if (node == NULL)
      return NULL;
   new_node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   #ifdef DEBUG
   std::cout<<"returned\n"<<std::flush;
   #endif
   if (new_node == nullptr)
      serror("Failed to allocate a node\n");
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
      try_catch(do_string,new_node,node);
      break;
   case IMAGE:
      try_catch(do_image,new_node,node);
      break;
   case ENVIRONMENT:
      try_catch(do_environment,new_node,node);
      break;
   case VAL_EXPER:
      try_catch(do_val_exper,new_node,node);
      break;
   case VEC_EXPER:
      try_catch(do_vec_exper,new_node,node);
      break;
   case RIPPLE:
   case VECTOR_EXPER:
      try_catch(do_vector_exper,new_node,node);
      break;
   case TERM:
      try_catch(do_coeff_exper,new_node,node);
      break;
   case SPLINE:
      try_catch(do_spline_exper,new_node,node);
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
      try_catch(do_tanh,new_node,node);
      break;
   case COLOR_MAP:
      try_catch(do_color_map,new_node,node);
      break;
   case ARRAY:
      try_catch(do_array,new_node,node);
      break;
   default:
      error((char*)"Bad node type in copy_node: %d\n", node->exper_type);
   }
   new_node->left = copy_node(node->left);
   new_node->right = copy_node(node->right);
   return new_node;
}



/* Deallocate all of the memory on a term list. */
/**
 * @brief Free every node in an expression-list chain.
 *
 * @param term_list Head of the list to free.
 * @return No return value.
 */
void
deallocate_list(LIST_PTR term_list)
{
   LIST_PTR temp_list;

   while (term_list!=nullptr) {
      deallocate_node(term_list->element);
      temp_list = term_list;
      term_list = term_list->next;
      //polyray_free(temp_list);
      }
}

/**
 * @brief Release image buffers owned by an image structure.
 *
 * @param image Image whose internal buffers are freed.
 * @return No return value.
 */
static void free_image_memory(Img *image)
{
   if (image == nullptr)
      return;
/* printf("Image->copy = %d\n", image->copy); */
   if (image->image != nullptr) {
      for (unsigned int i=0;i<image->length;i++)
         if (image->image[i] != nullptr)
            polyray_free(image->image[i]);
      polyray_free(image->image);
   }
   if (image->cmap != nullptr)
      polyray_free(image->cmap);
}

/* Free up all of the memory used by a branch of the parse tree. */
/**
 * @brief Free a parse-tree branch and any owned payload data.
 *
 * @param node Root of the branch to free.
 * @return No return value.
 */
void deallocate_node(NODE_PTR node)
{
   int i;
   Img **tmp;

   if (node == nullptr)
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
         deallocate_node(std::get<NODE_PTR>(node->exper_data));
         break;
      case ARRAY:
         deallocate_list(std::get<LIST_PTR>(node->exper_data));
         break;
      case STRING:
         break;
      case COLOR_MAP:
         deallocate_cmap_node(std::get<map_entries>(node->exper_data));
         break;
      case IMAGE:
         { Img *img = std::get<Img*>(node->exper_data);
           if (img != nullptr && img->copy == 0)
              free_image_memory(img);
           if (img != nullptr)
              delete img;
         }
         break;
      case ENVIRONMENT:
         { tmp = (Img **)std::get<Img**>(node->exper_data);
           for (i=0;tmp != nullptr && i<6;i++) {
              if (tmp[i] != nullptr && tmp[i]->copy == 0) {
/* printf("Free environment image %d, %p\n", i, tmp[i]); */
                 free_image_memory(tmp[i]);
                 }
              if (tmp[i] != nullptr)
                 delete tmp[i];
              }
           if (tmp != nullptr)
              polyray_free(tmp);
           tmp=nullptr;
         }
      break;
      case SPLINE:
         deallocate_spline_node(node);
      break;
      case RIPPLE:
      case VECTOR_EXPER: {
         auto v = std::get<std::array<NODE_PTR, VECTOR_LENGTH>>(node->exper_data);
         deallocate_node(v[0]);
         deallocate_node(v[1]);
         deallocate_node(v[2]);
         deallocate_node(v[3]);
         break;
      }
      default:
         std::string out="Deallocation of node of type ";
         out+=int2string(node->exper_type);
         out+=" is not possible\n";
         serror(out);
         //error("Can't deallocate node of type: %d\n", node->exper_type);
      }
      deallocate_node(node->left);
      deallocate_node(node->right);
      //polyray_free(node);
      }
}

/* Allocate memory for a node of a parse tree. */
/**
 * @brief Allocate a generic parse-tree node.
 *
 * @param type Expression type stored in the node.
 * @param left Left child node.
 * @param right Right child node.
 * @return Newly allocated node.
 */
NODE_PTR make_node(int type, NODE_PTR left, NODE_PTR right)
{
   NODE_PTR node = FactoryNODEPTR(); //(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
#ifdef DEBUG
   //std::cout << "mnode0b\n" << std::flush;
   //smessage("returned generic\n");
#endif
   if (node == nullptr)
      serror("Failed to allocate a node\n");
   node->exper_type = type;
   node->left = left;
   node->right = right;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a floating
   point number. */
/**
 * @brief Allocate a scalar literal expression node.
 *
 * @param value Scalar value stored in the node.
 * @return Newly allocated value node.
 */
NODE_PTR make_value_node(Flt value)
{
   //std::cout << "mvnode1\n" << std::flush;
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   //std::cout << "mvnode2\n" << std::flush;
#ifdef DEBUG
   //smessage("returned valnode\n");
   #endif
   if (node == nullptr)
      serror("Failed to allocate a value node\n");
   //std::cout << "mvnode3\n" << std::flush;
   node->exper_type = VAL_EXPER;
   #ifdef DEBUG
   //smessage("builder::make_value_node: going to put a Flt in exper_data\n");
   #endif
   exdata temp{value};
   node->exper_data = temp; //strcopyaight assignment crashes MSYS2
   node->left = nullptr;
   node->right = nullptr;
   #ifdef DEBUG
   std::cout<<"returning from make_value_node\n";
   #endif
   return node;
}

/* Allocate memory for a node of a parse tree that contains a string */
/**
 * @brief Allocate a string literal expression node from a C++ string.
 *
 * @param value String value stored in the node.
 * @return Newly allocated string node.
 */
NODE_PTR make_string_node(std::string value)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   #ifdef DEBUG
   std::cout<<"returned snstring\n"<<std::flush;
   #endif
   if (node == nullptr)
      serror("Failed to allocate a string node\n");

   node->exper_type = STRING;
   node->exper_data=value;
   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a string */
/**
 * @brief Allocate a string literal expression node from a C string.
 *
 * @param value String value stored in the node.
 * @return Newly allocated string node.
 */
NODE_PTR make_string_node(char *value)
{
   std::string svalue {value};
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   #ifdef DEBUG
   std::cout<<"returned sn\n"<<std::flush;
   #endif
   if (node == nullptr)
      serror("Failed to allocate a string node\n");

   node->exper_type = STRING;
   node->exper_data = svalue;
   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/**
 * @brief Allocate an image expression node and load its image data.
 *
 * @param filename Image filename to load.
 * @param support Optional support node attached as the left child.
 * @return Newly allocated image node.
 */
NODE_PTR make_image_node(char *filename, NODE_PTR support)
{
   NODE_PTR node;

   node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   #ifdef DEBUG
   std::cout<<"returned imnode\n"<<std::flush;
   #endif
   if (node == nullptr)
      serror("Failed to allocate an image node\n");
   node->exper_type = IMAGE;
   node->exper_data = SafeReadImage(filename,"Image file");
   node->left  = support;
   node->right = nullptr;
   return node;
}

#ifdef TESTING
/**
 * @brief Allocate a mock image expression node without loading image data.
 *
 * @param filename Unused image filename placeholder.
 * @param support Optional support node attached as the left child.
 * @return Newly allocated mock image node.
 */
NODE_PTR make_image_nodemock(char* filename, NODE_PTR support)
{
    NODE_PTR node;

    node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
#ifdef DEBUG
    std::cout << "returned imnode\n" << std::flush;
#endif
    if (node == nullptr)
        serror("Failed to allocate an image node\n");
    node->exper_type = IMAGE;
    node->left = support;
    node->right = nullptr;
    return node;
}
#endif

/**
 * @brief Allocate an environment-map node from six image filenames.
 *
 * @param file0 First environment face filename.
 * @param file1 Second environment face filename.
 * @param file2 Third environment face filename.
 * @param file3 Fourth environment face filename.
 * @param file4 Fifth environment face filename.
 * @param file5 Sixth environment face filename.
 * @return Newly allocated environment node.
 */
NODE_PTR make_environ_node(char *file0, char *file1, char *file2,
                  char *file3, char *file4, char *file5)
{
   NODE_PTR node;

   node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   #ifdef DEBUG
   std::cout<<"returned enode\n"<<std::flush;
   #endif
   if (node == nullptr)
      serror("Failed to allocate an image node\n");
   node->exper_type = ENVIRONMENT;
   Img** images = (Img **)polyray_malloc(6 * sizeof(Img *));
   if (images == nullptr)
     serror("Failed to allocate environment buffer\n");
   node->exper_data = images;

   node->left  = nullptr;
   node->right = nullptr;
   images[0] = SafeReadImage(file0,"Environment image");
   images[1] = SafeReadImage(file1,"Environment image");
   images[2] = SafeReadImage(file2,"Environment image");
   images[3] = SafeReadImage(file3,"Environment image");
   images[4] = SafeReadImage(file4,"Environment image");
   images[5] = SafeReadImage(file5,"Environment image");
   return node;
}

#ifdef TESTING
/**
 * @brief Allocate a mock environment-map node without loading image data.
 *
 * @param file0 Unused first face filename placeholder.
 * @param file1 Unused second face filename placeholder.
 * @param file2 Unused third face filename placeholder.
 * @param file3 Unused fourth face filename placeholder.
 * @param file4 Unused fifth face filename placeholder.
 * @param file5 Unused sixth face filename placeholder.
 * @return Newly allocated mock environment node.
 */
NODE_PTR make_environ_nodemock(char* file0, char* file1, char* file2,
    char* file3, char* file4, char* file5)
{
    NODE_PTR node;

    node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
#ifdef DEBUG
    std::cout << "returned enode\n" << std::flush;
#endif
    if (node == nullptr)
        serror("Failed to allocate an image node\n");
    node->exper_type = ENVIRONMENT;

    node->left = nullptr;
    node->right = nullptr;
 
    return node;
}
#endif

/**
 * @brief Allocate a three-component vector expression node.
 *
 * @param node0 First component expression.
 * @param node1 Second component expression.
 * @param node2 Third component expression.
 * @return Newly allocated vector-expression node.
 */
NODE_PTR make_vector3_node(NODE_PTR node0, NODE_PTR node1, NODE_PTR node2)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
#ifdef DEBUG
   smessage("returned v3node\n");
   #endif
   if (node == nullptr) serror("Failed to allocate a vector node\n");
   node->exper_type = VECTOR_EXPER;

   //auto vv = std::make_shared<vvarr>();
   auto vv = new vvarr();
   *vv = { node0, node1, node2, nullptr};
   node->exper_data=*vv;
   delete vv;

   node->left = nullptr;
   node->right = nullptr;
   #ifdef DEBUG
   smessage("returning from make_vector3_node\n");
   #endif
   return node;
}

/**
 * @brief Allocate a four-component vector expression node.
 *
 * @param node0 First component expression.
 * @param node1 Second component expression.
 * @param node2 Third component expression.
 * @param node3 Fourth component expression.
 * @return Newly allocated vector-expression node.
 */
NODE_PTR make_vector4_node(NODE_PTR node0, NODE_PTR node1,
                  NODE_PTR node2, NODE_PTR node3)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr) serror("Failed to allocate a vector node\n");
   node->exper_type = VECTOR_EXPER;
   auto vv = std::make_shared<vvarr>();
   *vv = { node0, node1, node2, node3};
   node->exper_data=*vv;

   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/**
 * @brief Allocate a fixed three-value vector literal node.
 *
 * @param val0 First vector component.
 * @param val1 Second vector component.
 * @param val2 Third vector component.
 * @return Newly allocated vector literal node.
 */
NODE_PTR
make_vec_node(Flt val0, Flt val1, Flt val2)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr) serror("Failed to allocate a vector node\n");
   node->exper_type = VEC_EXPER;
   node->exper_data = NuVec { val0, val1, val2 };
   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/* Allocate memory for a node of a parse tree that contains a floating
   point number. */
/**
 * @brief Allocate a polynomial term node with a constant coefficient.
 *
 * @param value Constant coefficient value.
 * @return Newly allocated term node.
 */
NODE_PTR
make_value_term_node(Flt value)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   if (node == nullptr)
      serror("Failed to allocate a value node\n");
   node->exper_type = TERM;
   node->exper_data = coeff_node { value, 0.0, 0.0, 0.0};
   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/* Allocate a single element of an expression list. */
/**
 * @brief Allocate one list element for an expression array.
 *
 * @param node Expression stored in the list element.
 * @return Newly allocated list node.
 */
LIST_PTR
make_list_node(NODE_PTR node)
{
   LIST_PTR temp = (LIST_PTR)polyray_malloc(sizeof(struct exper_list_struct));
   if (temp == nullptr)
      serror("Failed to allocate list node\n");
   temp->element = node;
   temp->next = nullptr;
   return temp;
}

/* Allocate memory for a node of a parse tree. */
/**
 * @brief Allocate an array expression node.
 *
 * @param elist Linked list of array elements.
 * @return Newly allocated array node.
 */
NODE_PTR make_array_node(LIST_PTR elist)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr) serror("Failed to allocate a node\n");
   node->exper_type = ARRAY;
   node->exper_data = elist;
   node->left  = nullptr;
   node->right = nullptr;
   return node;
}

/* Allocate memory for a function node */
/**
 * @brief Allocate a unary function-expression node.
 *
 * @param fntype Expression type for the function.
 * @param exper Operand expression.
 * @return Newly allocated function node.
 */
NODE_PTR
make_fn1_node(int fntype, NODE_PTR exper)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr)
      serror("Failed to allocate a function(1) node\n");
   node->exper_type = fntype;
   node->exper_data = exper;
   node->left = nullptr;
   node->right = nullptr;
   return node;
}

/* Allocate memory for a function node */
/**
 * @brief Allocate a binary function-expression node.
 *
 * @param fntype Expression type for the function.
 * @param exper1 First operand expression.
 * @param exper2 Second operand expression.
 * @return Newly allocated function node.
 */
NODE_PTR
make_fn2_node(int fntype, NODE_PTR exper1, NODE_PTR exper2)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));
   if (node == NULL)
      serror("Failed to allocate a function(2) node\n");
   node->exper_type = fntype;
   node->left = exper1;
   node->right = exper2;
   return node;
}

/* Allocate memory for a function node */
/**
 * @brief Allocate a ternary function-expression node.
 *
 * @param fntype Expression type for the function.
 * @param exper1 First operand expression stored in payload data.
 * @param exper2 Second operand expression stored as left child.
 * @param exper3 Third operand expression stored as right child.
 * @return Newly allocated function node.
 */
NODE_PTR make_fn3_node(int fntype, NODE_PTR exper1, NODE_PTR exper2, NODE_PTR exper3)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr)
      serror("Failed to allocate a value node\n");
   node->exper_type = fntype;
   node->exper_data = exper1;
   node->left  = exper2;
   node->right = exper3;
   return node;
}

/* Allocate memory for a function node */
/**
 * @brief Allocate a conditional expression node.
 *
 * @param condition Condition expression stored in payload data.
 * @param exper1 Expression used when the condition is true.
 * @param exper2 Expression used when the condition is false.
 * @return Newly allocated conditional node.
 */
NODE_PTR make_cond_node(NODE_PTR condition, NODE_PTR exper1, NODE_PTR exper2)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr)
      serror("Failed to allocate a value node\n");
   node->exper_type = CONDITIONAL_EXPER;
   node->exper_data = condition;
   node->left  = exper1;
   node->right = exper2;
   return node;
}

/* Allocate memory for a node of a parse tree. */
/**
 * @brief Allocate a color-map expression node.
 *
 * @param map Linked list of color-map entries.
 * @param deflt Default expression used when the map does not match.
 * @return Newly allocated color-map node.
 */
NODE_PTR
make_cmap_node(map_entries map, NODE_PTR deflt)
{
   NODE_PTR node = FactoryNODEPTR();//(NODE_PTR)polyray_malloc(sizeof(struct exper_node_struct));

   if (node == nullptr) serror("Failed to allocate a node\n");
   node->exper_type = COLOR_MAP;
   node->exper_data = map;
   node->left  = deflt;
   node->right = nullptr;
   return node;
}

//lookup fn - cm080124 on a rainy day
//originally lookup_fn_name
#ifndef TESTING
static
#endif
/**
 * @brief Return the textual name for a function expression type.
 *
 * @param fntype Expression type identifier.
 * @return Human-readable function name, or `"Unknown"`.
 */
std::string lookup_fn(int fntype)
{
   std::map<int,std::string> fnMap {
     { ACOS,        "acos"},
     { ASIN,        "asin"},
     { ATAN,        "atan"},
     { ATAN_TWO,    "atan2"},
     { BIAS,        "bias"},
     { CEIL,        "ceil"},
     { COLOR_MAP,   "color_map"},
     { COS,         "cos"},
     { COSH,        "cosh"},
     { DNOISE,      "dnoise"},
     { EXP,         "exp"},
     { FABS,        "fabs"},
     { FBM,         "fbm"},
     { FNOISE,      "fnoise"},
     { FLOOR,       "floor"},
     { FMOD,        "fmod"},
     { GAIN,        "gain"},
     { LN,          "ln"},
     { LOG,         "log"},
     { MAXT,        "max"},
     { MINT,        "min"},
     { NOISE,       "noise"},
     { RAMP,        "ramp"},
     { REFLECT,     "reflect"},
     { ROTATE,      "rotate"},
     { SAWTOOTH,    "sawtooth"},
     { SIN,         "sin"},
     { SPLINE,      "spline"},
     { SQRT,        "sqrt"},
     { TAN,         "tan"},
     { TANH,        "tanh"}
   };
   auto res=fnMap.find(fntype);
   if (res != fnMap.end()) //found
     return res->second;
   return "Unknown";
}

#ifndef TESTING
static
#endif
/**
 * @brief Print a linked list of color-map entries.
 *
 * @param cnode Head of the color-map list to display.
 * @return No return value.
 */
void show_cmap_node(map_entries cnode)
{
   map_entries temp = cnode;
   while (temp != NULL) {
       std::string out=double2string(temp->p0) + ", " + double2string(temp->p1) + ", <";
       out+= double2string(temp->v0[0]) + ", " + double2string(temp->v0[1]) + ", " + double2string(temp->v0[2]) + ">, <";
       out+= double2string(temp->v1[0]) + ", " + double2string(temp->v1[1]) + ", " + double2string(temp->v1[2]) + ">]\n";
       smessage(out);
          //"[%f, %f, <%f, %f, %f>, <%f, %f, %f>]\n",
          //    temp->p0, temp->p1,
          //    temp->v0[0], temp->v0[1], temp->v0[2],
          //    temp->v1[0], temp->v1[1], temp->v1[2]);
      temp = temp->next;
      }
}

#ifndef TESTING
static
#endif
/**
 * @brief Print the contents of an array-expression list.
 *
 * @param list Head of the array list to display.
 * @return No return value.
 */
void show_array_node(LIST_PTR list)
{
   LIST_PTR temp = list;
   while (temp != NULL) {
      show_node(temp->element);
      if (temp->next != NULL)
         smessage(", ");
      temp = temp->next;
      }
}

/* Print the contents of a parse node to the screen. This routine
   recursively*/
/**
 * @brief Print a parse-tree node and its children.
 *
 * @param node Node to display.
 * @return No return value.
 */
void show_node(NODE_PTR node)
{
   if (node == nullptr) {
      smessage("(NULL)");
      return;
      }

   switch (node->exper_type) {
   case TERM:
   {
      int tflag = 0;
      auto cn=std::get<coeff_node>(node->exper_data);
      if (cn.coeff != 1.0) {
          smessage("%lg", cn.coeff);
          tflag = 1;
          }
      else if (cn.x_power == 0.0 &&
               cn.y_power == 0.0 &&
               cn.z_power == 0.0)
          smessage("1");
      if (cn.x_power > 0.0) {
         if (tflag) smessage("*");
         if (cn.x_power != 1.0)
            smessage("x^%lg", cn.x_power);
         else
            smessage("x");
         tflag = 1;
         }
      if (cn.y_power > 0.0) {
         if (tflag) smessage("*");
         if (cn.y_power != 1)
            smessage("y^%lg", cn.y_power);
         else
            smessage("y");
         tflag = 1;
         }
      if (cn.z_power > 0.0) {
         if (tflag) smessage("*");
         if (cn.z_power != 1)
            smessage("z^%lg", cn.z_power);
         else
            smessage("z");
         }
      break;
   }
   case I_EXPER:
      smessage("I");
      break;
   case N_EXPER:
      smessage("N");
      break;
   case P_EXPER:
      smessage("P");
      break;
   case U_EXPER:
   {
      auto value=std::get<Flt>(node->exper_data);
      smessage("U", value);
      break;
   }
   case UU_EXPER:
   {
      auto value=std::get<Flt>(node->exper_data);
      smessage("u", value);
      break;
   }
   case UV_EXPER:
   {
      auto value=std::get<Flt>(node->exper_data);
      smessage("v", value);
      break;
   }
   case UW_EXPER:
   {
      auto value=std::get<Flt>(node->exper_data);
      smessage("2", value);
      break;
   }
   case VAL_EXPER:
   {
      auto value=std::get<Flt>(node->exper_data);
      smessage("%lg", value);
      break;
   }
   case W_EXPER:
      smessage("W");
      break;
   case X_EXPER:
      smessage("x");
      break;
   case Y_EXPER:
      smessage("y");
      break;
   case Z_EXPER:
      smessage("z");
      break;
   case COLOR:
      smessage("color");
      break;
   case OPACITY:
      smessage("opacity");
      break;
   case RANDOM:
      smessage("random");
      break;
   case START_FRAME:
      smessage("start_frame");
      break;
   case END_FRAME:
      smessage("end_frame");
      break;
   case TOTAL_FRAMES:
      smessage("total_frames");
      break;
   case FRAME:
      smessage("frame");
      break;
   case VEC_EXPER:
   {
      smessage("<");
      auto v=std::get<NuVec>(node->exper_data);
      for (int i=0;i<VECTOR_LENGTH;i++) {
         smessage("%lg", v[i]);
         if (i < VECTOR_LENGTH-1)
            smessage(", ");
         }
      smessage(">");
      break;
   }
   case ENVIRONMENT:
   {
      smessage("environment(");
      auto images=std::get<Img**>(node->exper_data);
      for (int i=0;i<6;i++) {
         smessage("\"%s\"", images[i]->filename.c_str());
         if (i < 5)
            smessage(", ");
         else
            smessage(")");
         }
      break;
   }
   case IMAGE:
   {
      auto image=std::get<Img*>(node->exper_data);
      smessage("image(\"%s\")", image->filename.c_str());
      break;
   }
   case INDEXED_MAP:
   {
      smessage("indexed(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case CYLINDRICAL_INDEXED:
   {
      smessage("cylindrical_indexed_map(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case SPHERICAL_INDEXED:
   {
      smessage("spherical_indexed_map(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case HEIGHT_MAP:
   {
      smessage("heightmap(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case PLANAR_IMAGEMAP:
   {
      smessage("planar_imagemap(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case CYLINDRICAL_IMAGEMAP:
   {
      smessage("cylindrical_imagemap(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case SPHERICAL_IMAGEMAP:
   {
      smessage("spherical_imagemap(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case COLOR_MAP:
   {
      smessage("color_map(");
      auto cmap=std::get<map_entries>(node->exper_data);
      show_cmap_node(cmap);
      smessage(")");
      break;
   }
   case ARRAY:
   {
      smessage("[");
      auto arr=std::get<LIST_PTR>(node->exper_data);
      show_array_node(arr);
      smessage("]");
      break;
   }
   case STRING:
      {
      std::string strt=std::get<std::string>(node->exper_data);
      smessage("\""+strt+"\"");
      break;
      }
   case COLOR_WHEEL:
   {
      smessage("color_wheel(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case CONDITIONAL_EXPER:
   {
      smessage("(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage("?");
      show_node(node->left);
      smessage(":");
      show_node(node->right);
      smessage(")");
      break;
   }
   case LEGENDRE:
   {
      smessage("legendre(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      smessage(",");
      show_node(node->right);
      smessage(")");
      break;
   }
   case RIPPLE:
   {
      smessage("ripple(");
      auto vec=std::get<vvarr>(node->exper_data);
      show_node(vec[0]);
      smessage(", ");
      show_node(vec[1]);
      smessage(", ");
      show_node(vec[2]);
      smessage(", ");
      show_node(vec[3]);
      smessage(")");
      break;
   }
   case VECTOR_EXPER:
   {
      smessage("<");
      auto vec=std::get<vvarr>(node->exper_data);
      show_node(vec[0]);
      smessage(", ");
      show_node(vec[1]);
      smessage(", ");
      show_node(vec[2]);
      smessage(">");
      break;
   }
   case PLUS_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage("+");
      show_node(node->right);
      smessage(")");
      break;
   }
   case MINUS_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage("-");
      show_node(node->right);
      smessage(")");
      break;
   }
   case TIMES_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage("*");
      show_node(node->right);
      smessage(")");
      break;
   }
   case DIV_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage("/");
      show_node(node->right);
      smessage(")");
      break;
   }
   case DOT_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage(".");
      show_node(node->right);
      smessage(")");
      break;
   }
   case POWER_EXPER:
   {
      smessage("(");
      show_node(node->left);
      smessage("^");
      show_node(node->right);
      smessage(")");
      break;
   }
   case UMINUS_EXPER:
   {
      smessage("(");
      smessage("-");
      show_node(node->left);
      smessage(")");
      break;
   }
   case EQUAL_EXPER:
   {
      show_node(node->left);
      smessage("==");
      show_node(node->right);
      break;
   }
   case GREATER_EXPER:
   {
      show_node(node->left);
      smessage(">");
      show_node(node->right);
      break;
   }
   case GTEQ_EXPER:
   {
      show_node(node->left);
      smessage(">=");
      show_node(node->right);
      break;
   }
   case LESS_EXPER:
   {
      show_node(node->left);
      smessage("<");
      show_node(node->right);
      break;
   }
   case LTEQ_EXPER:
   {
      show_node(node->left);
      smessage("<=");
      show_node(node->right);
      break;
   }
   case NOT_EXPER:
   {
      smessage("!");
      show_node(node->left);
      break;
   }
   case AND_EXPER:
   {
      show_node(node->left);
      smessage("&&");
      show_node(node->right);
      break;
   }
   case OR_EXPER:
   {
      show_node(node->left);
      smessage("||");
      show_node(node->right);
      break;
   }
   case ENVIRONMENT_MAP:
   {
      smessage("environment_map(");
      show_node(node->left);
      smessage(", ");
      show_node(node->right);
      smessage(")");
      break;
   }
   case SUBSCRIPT_EXPER:
   {
      show_node(node->left);
      smessage("[");
      show_node(node->right);
      smessage("]");
      break;
   }
   case ATAN_TWO:
   case GAIN:
   case BIAS:
   case FNOISE:
   case FBM:
   case DNOISE:
   case NOISE:
   {
      smessage(lookup_fn(node->exper_type));
      smessage("()");
      show_node(node->left);
      if (node->right != nullptr) {
         smessage(", ");
         show_node(node->right);
         }
      smessage(")");
      break;
   }
   case TRACE:
   {
      smessage("trace");
      if (node->left != nullptr) {
         show_node(node->left);
         smessage(", ");
         }
      show_node(node->right);
      smessage(")");
      break;
   }
   case ROTATE:
   {
      smessage("rotate(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(",");
      show_node(node->left);
      if (node->right != nullptr) {
         smessage(", ");
         show_node(node->right);
         }
      smessage(")");
      break;
   }
   case SPLINE:
      show_spline_node(node);
      break;
   case FMOD:
   case MAXT:
   case MINT:
   case VISIBLE:
   case REFLECT:
   {
      smessage(lookup_fn(node->exper_type));
      smessage("(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(", ");
      show_node(node->left);
      if (node->right) {
         smessage(", ");
         show_node(node->right);
         }
      smessage(")");
      break;
   }
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
   {
      smessage(lookup_fn(node->exper_type));
      smessage("(");
      auto param=std::get<NODE_PTR>(node->exper_data);
      show_node(param);
      smessage(")");
      break;
   }
   default:
      serror("Bad node type in show_node: %d\n", node->exper_type);
   }
}

DrawNode *make_draw_node(Flt low, Flt high, int steps,
               NODE_PTR draw_fn, NODE_PTR color_fn)
{
   DrawNode *node;

   node = (DrawNode*)polyray_malloc(sizeof(DrawNode));
   node->low = low;
   node->high = high;
   node->steps = steps;
   node->draw_fn = draw_fn;
   node->color_fn = color_fn;
   node->next = nullptr;

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
