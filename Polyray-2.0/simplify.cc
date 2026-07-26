/* simplify.cc

   Algebraic equation -> polynomial coefficient translation

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
#include "defs3.h"
#include "polyray.tab.h"
#include "io_ply.h"
#include "memory.h"
#include "builder.h"
#include "roots.h"
#include "eval.h"
#include "simplify.h"

/** @brief Copy TERM coefficient data from one node into another node.
 *  @param new_node Destination node whose expression payload is updated.
 *  @param node     Source TERM node providing the coefficient data.
 *  @return No return value.
 */
static void do_coeff_exp(NODE_PTR new_node, NODE_PTR node)
{
   auto coeff=std::get<coeff_node>(node->exper_data);
   new_node->exper_data = coeff;
}

/* Simplify additive terms. */
/** @brief Simplify an addition node when its children are compatible TERM nodes.
 *  @param node PLUS_EXPER node to simplify.
 *  @return The simplified node, which may be collapsed into a TERM node.
 */
static NODE_PTR simplify_plus(NODE_PTR node)
{
   NODE_PTR left = node->left;
   NODE_PTR right = node->right;

   if (left->exper_type == TERM &&
       right->exper_type == TERM) {
       auto lcoeff = std::get<coeff_node>(left->exper_data);
       auto rcoeff = std::get<coeff_node>(right->exper_data);

       if (lcoeff.x_power == rcoeff.x_power &&
           lcoeff.y_power == rcoeff.y_power &&
           lcoeff.z_power == rcoeff.z_power) {
               node->exper_type = TERM;
               try_catch(do_coeff_exp, node, left);
               auto nodecoeff = std::get<coeff_node>(node->exper_data);
               nodecoeff.coeff = lcoeff.coeff + rcoeff.coeff;
               node->exper_data = nodecoeff;
               node->left = nullptr;
               node->right = nullptr;
               delete left;
               delete right;
               return node;
      }
   }
   
   return node;
}

/* Simplify multiplicative terms. */
/** @brief Simplify a multiplication node by combining terms or distributing over sums.
 *  @param node TIMES_EXPER node to simplify.
 *  @return Simplified expression node.
 */
static NODE_PTR simplify_times(NODE_PTR node)
{
   NODE_PTR left, right, t0, t1, t2, t3;
   left = node->left;
   right = node->right;

   if (left->exper_type == TERM)
      if (right->exper_type == TERM) {
         node->exper_type = TERM;
         auto lcoeff=std::get<coeff_node>(left->exper_data);
         auto rcoeff=std::get<coeff_node>(right->exper_data);
         coeff_node nc;
         nc.coeff = lcoeff.coeff *
                                        rcoeff.coeff;
         nc.x_power = lcoeff.x_power +
                                          rcoeff.x_power;
         nc.y_power = lcoeff.y_power +
                                          rcoeff.y_power;
         nc.z_power = lcoeff.z_power +
                                          rcoeff.z_power;
         node->exper_data = nc;
         node->left = nullptr;
         node->right = nullptr;
         delete left;
         delete right;
         return node;
         }
      else if (right->exper_type == PLUS_EXPER) {
         t0 = simplify(make_node(TIMES_EXPER, copy_node(left), right->left), 0);
         t1 = simplify(make_node(TIMES_EXPER, left, right->right), 0);
         delete right;
         node->exper_type = PLUS_EXPER;
         node->left = t0;
         node->right = t1;
         return node;
         }
      else
         return node;
   else if (right->exper_type == TERM)
      if (left->exper_type == PLUS_EXPER) {
         t0 = simplify(make_node(TIMES_EXPER, left->left, copy_node(right)), 0);
         t1 = simplify(make_node(TIMES_EXPER, left->right, right), 0);
         delete left;
         node->exper_type = PLUS_EXPER;
         node->left = t0;
         node->right = t1;
         return node;
         }
      else
         return node;
   else if (left->exper_type == PLUS_EXPER &&
            right->exper_type == PLUS_EXPER) {
      t0 = simplify(make_node(TIMES_EXPER, copy_node(left->left),
                              copy_node(right->left)), 0);
      t1 = simplify(make_node(TIMES_EXPER, copy_node(left->left),
                              copy_node(right->right)), 0);
      t2 = simplify(make_node(TIMES_EXPER, copy_node(left->right),
                              copy_node(right->left)), 0);
      t3 = simplify(make_node(TIMES_EXPER, copy_node(left->right),
                              copy_node(right->right)), 0);
      deallocate_node(node);
      return make_node(PLUS_EXPER, t0,
                make_node(PLUS_EXPER, t1,
                   make_node(PLUS_EXPER, t2, t3)));
      }
   else
      return node;
}

/* Simplify exponentiated terms */
/** @brief Simplify a power expression, optionally applying a leading unary minus.
 *  @param node       POWER_EXPER node to simplify.
 *  @param minus_flag Non-zero when the resulting expression should be negated.
 *  @return Simplified expression node, or nullptr on unrecoverable failure.
 */
static NODE_PTR simplify_power(NODE_PTR node, int minus_flag)
{
   NODE_PTR left, right, t0, t1, t2, head;
   int n, i;
   unsigned long j;

   left = node->left;
   right = node->right;

   if (right->exper_type != TERM) {
      serror("Invalid expression as power");
      return node;
      }
   else if (std::get<coeff_node>(right->exper_data).coeff == 0.0) {
      node->exper_type = TERM;
      coeff_node nc;
      nc.coeff = (minus_flag ? -1.0 : 1.0);
      nc.x_power = 0.0;
      nc.y_power = 0.0;
      nc.z_power = 0.0;
      node->exper_data = nc;
      node->left = nullptr;
      node->right = nullptr;
      deallocate_node(left);
      delete right;
      return node;
      }
   else if (std::get<coeff_node>(right->exper_data).coeff == 1.0) {
      delete right;
      delete node;
      return simplify(left, minus_flag);
      }
   else if (left->exper_type == TERM) {
      node->exper_type = TERM;
      coeff_node nc;
      auto lcoeff=std::get<coeff_node>(left->exper_data);
      auto rcoeff=std::get<coeff_node>(right->exper_data);
      nc.coeff = pow(lcoeff.coeff,
                                         rcoeff.coeff) *
                                          (minus_flag ? -1.0 : 1.0);
      nc.x_power = lcoeff.x_power *
                                       rcoeff.coeff;
      nc.y_power = lcoeff.y_power *
                                       rcoeff.coeff;
      nc.z_power = lcoeff.z_power *
                                       rcoeff.coeff;
      node->exper_data = nc;
      node->left = nullptr;
      node->right = nullptr;
      delete left;
      delete right;
      return node;
      }
   else if (left->exper_type == PLUS_EXPER) {
      head = nullptr;
      //auto lcoeff=std::get<coeff_node>(left->exper_data);
      auto rcoeff=std::get<coeff_node>(right->exper_data);
      n = (int)(rcoeff.coeff);
      for (i=0;i<=n;i++) {
         j = binomial(n, i);
         t0 = simplify(make_node(POWER_EXPER, copy_node(left->left),
                                 make_value_term_node((Flt)i)), 0);
         t1 = simplify(make_node(POWER_EXPER, copy_node(left->right),
                                 make_value_term_node((Flt)(n-i))), 0);
         t2 = make_node(TIMES_EXPER, make_value_term_node((Flt)j),
                        make_node(TIMES_EXPER, t0, t1));
         if (head == NULL)
            head = t2;
         else
            head = make_node(PLUS_EXPER, t2, head);
         }
      deallocate_node(node);
      return simplify(head, minus_flag);
      }
   else {
      serror("Simplification failed\n");
      return NULL;
      }
}

void HandleLeftValExper(NODE_PTR node)
{
    coeff_node nc{};// = std::get<coeff_node>(node->exper_data);
    nc.coeff =
        std::get<coeff_node>(node->left->exper_data).coeff /
        std::get<coeff_node>(node->right->exper_data).coeff;
    nc.x_power = 0.0;
    nc.y_power = 0.0;
    nc.z_power = 0.0;
    node->exper_data = nc;
    node->exper_type = TERM;   // or Flt+VAL_EXPER, depending on intended representation
        
    deallocate_node(node->left);
    deallocate_node(node->right);    
}

void HandleLeftNotVal(NODE_PTR& node, NODE_PTR tnode)
{
    node->exper_type = TIMES_EXPER;
    node->right = node->left;
    node->left = tnode;
    coeff_node tc = std::get<coeff_node>(tnode->exper_data);
    tc.coeff = 1.0 / tc.coeff;
    tnode->exper_data = tc;
    node = simplify_times(node);
}

/* Once a parse tree has been created we need to convert it into a form
   that can be more easily manipulated. */
/** @brief Simplify a parse-tree expression into TERM/PLUS/TIMES polynomial form.
 *  @param node       Root of the parse-tree expression.
 *  @param minus_flag Non-zero when the expression should be negated during simplification.
 *  @return Simplified expression tree.
 */
NODE_PTR simplify(NODE_PTR node, int minus_flag)
{
   Flt fval;
   Vec vval;
   NODE_PTR tnode;

   if (eval_node(NULL, node, &fval, vval, &tnode) == 1) {
      deallocate_node(node);
      node = make_value_term_node((minus_flag ? -1 : 1) * fval);
      return node;
      }

   switch(node->exper_type) {
   case X_EXPER: {
      node->exper_type = TERM;
      coeff_node nc;
      nc.coeff   = (minus_flag?-1.0:1.0);
      nc.x_power = 1.0;
      nc.y_power = 0.0;
      nc.z_power = 0.0;
      node->exper_data = nc;
      return node;
   }
   case Y_EXPER: {
      node->exper_type = TERM;
      coeff_node nc;
      nc.coeff   = (minus_flag?-1.0:1.0);
      nc.x_power = 0.0;
      nc.y_power = 1.0;
      nc.z_power = 0.0;
      node->exper_data = nc;
      return node;
   }
   case Z_EXPER: {
      node->exper_type = TERM;
      coeff_node nc;
      nc.coeff   = (minus_flag?-1.0:1.0);
      nc.x_power = 0.0;
      nc.y_power = 0.0;
      nc.z_power = 1.0;
      node->exper_data = nc;
      return node;
   }
   case VAL_EXPER: {
      node->exper_type = TERM;
      coeff_node nc;
      if (!std::holds_alternative<Flt>(node->exper_data)) {
          serror("Bad VAL_EXPER payload, variant index=%d\n\n\n", (int)node->exper_data.index());
      }
      nc.coeff   = (minus_flag?-1.0:1.0) *
                                       std::get<Flt>(node->exper_data);
      nc.x_power = 0.0;
      nc.y_power = 0.0;
      nc.z_power = 0.0;
      node->exper_data = nc;
      return node;
   }
   case TERM:
      if (minus_flag) { 
        coeff_node nc;
        nc = std::get<coeff_node>(node->exper_data);
        nc.coeff *= -1.0;
        node->exper_data = nc;
      }
      return node;
   case PLUS_EXPER:
      node->left = simplify(node->left, minus_flag);
      node->right = simplify(node->right, minus_flag);
      return node;
   case MINUS_EXPER:
      node->exper_type = PLUS_EXPER;
      node->left = simplify(node->left, minus_flag);
      node->right = simplify(node->right, 1 - minus_flag);
      node = simplify_plus(node);
      return node;
   case TIMES_EXPER:
      node->left = simplify(node->left, minus_flag);
      node->right = simplify(node->right, 0);
      node = simplify_times(node);
      return node;
   case DIV_EXPER:
      node->left = simplify(node->left, minus_flag);
      tnode = simplify(node->right, 0);     
      //this code used to use VAL_EXPER. Now it's mostly just a pre - simplification / runtime - expression form, not a polynomial - normal - form node and
      //we use TERM
      if (tnode->exper_type == TERM &&
          std::get<coeff_node>(tnode->exper_data).coeff != 0.0) {
          node->right = tnode; //node->right has not been updated to tnode yet so we have to do it before we call HandleLeftValExper
         if (node->left->exper_type == TERM) {
             HandleLeftValExper(node);
            }
         else {
             HandleLeftNotVal(node, tnode);
            }
         }
      else
         node->right = tnode;
         
      return node;
   case POWER_EXPER:
      node->left = simplify(node->left, 0);
      node->right = simplify(node->right, 0);
      return simplify_power(node, minus_flag);
   case UMINUS_EXPER:
      tnode = simplify(node->left, 1 - minus_flag);
      delete node;
      return tnode;
   default:
      return node;
   }
}

/* Simple insertion sort of the expressions on the list. */
/** @brief Sort and combine additive TERM nodes in descending power order.
 *  @param term_list Linked list of expression terms to sort and combine.
 *  @return Head of the sorted, combined list.
 */
static LIST_PTR sort_terms(LIST_PTR term_list)
{
   LIST_PTR new_list = nullptr;
   LIST_PTR temp1, temp2, temp3, last;
   NODE_PTR term, test_term;
   int flag;

   temp1 = term_list;
   while (temp1 != nullptr) {
      term = temp1->element;
      if (new_list == nullptr)
         /* nothing to compare against */
         new_list = make_list_node(term);
      else if (term->exper_type != TERM) {
         /* Can only compare base terms, even worse getting here means
            that simplification has failed. */
         temp2 = make_list_node(term);
         temp2->next = new_list;
         new_list = temp2;
         }
      else if (fabs(std::get<coeff_node>(term->exper_data).coeff) < 1.0e-10) {
         /* Non-contributing term, get rid of it. */
         delete term;
         }
      else {
         /* First see if it matches any of the existing terms */
         temp2 = new_list;
         last = new_list;
         flag = 0;
         while (temp2 != nullptr && !flag) {
            test_term = temp2->element;
            if (test_term->exper_type == TERM) {
               coeff_node tc=std::get<coeff_node>(test_term->exper_data);
               coeff_node termc=std::get<coeff_node>(term->exper_data);
               if (tc.x_power ==
                   termc.x_power &&
                   tc.y_power ==
                   termc.y_power &&
                   tc.z_power ==
                   termc.z_power) {
                  /* Can collect these terms into one. */
                  tc.coeff +=
                     termc.coeff;
                  test_term->exper_data = tc;
                  delete term;
                  flag = 1;
                  }
               else if (tc.x_power <
                        termc.x_power ||
                        (tc.x_power ==
                         termc.x_power &&
                         (tc.y_power <
                          termc.y_power ||
                          (tc.y_power ==
                           termc.y_power &&
                           tc.z_power <
                           termc.z_power)))) {
                  /* The next term on the list is greater than this one.
                     Insert it here. */
                  temp3 = make_list_node(term);
                  temp3->next = temp2;
                  if (temp2 == new_list)
                     new_list = temp3;
                  else
                     last->next = temp3;
                  flag = 1;
                  }
               }
            if (temp2 != new_list)
               last = last->next;
            temp2 = temp2->next;
            }
         if (!flag) {
            /* Didn't insert it anywhere, just add to the end of list */
            temp2 = make_list_node(term);
            if (new_list == nullptr)
               new_list = temp2;
            else {
               last = new_list;
               while (last->next != nullptr) last = last->next;
               last->next = temp2;
               }
            }
         }
      temp2 = temp1;
      temp1 = temp1->next;
      polyray_free(temp2);
      }
   /* The list is now sorted. Go through it and remove any terms with
      a zero coefficient. */
   temp1 = new_list;
   last = new_list;
   while (temp1!=nullptr) {
      term = temp1->element;
      if (term->exper_type == TERM &&
          fabs(std::get<coeff_node>(term->exper_data).coeff) < 1.0e-20) {
         /* Non-contributing term. remove it. */
         if (temp1 == new_list) {
            temp1 = temp1->next;
            polyray_free(new_list);
            delete term;
            new_list = temp1;
            last = temp1;
            }
         else {
            temp2 = temp1;
            temp1 = temp1->next;
            last->next = temp1;
            polyray_free(temp2);
            delete term;
            }
         }
      else {
         if (temp1 != new_list) last = last->next;
         temp1 = temp1->next;
         }
      }
   return new_list;
}

/* Separate all additive terms into a single linked list. */
/** @brief Flatten a PLUS_EXPER tree into a linked list of additive terms.
 *  @param node      Expression node to flatten.
 *  @param term_list Output list head updated in place.
 *  @return No return value.
 */
static void collect_terms(NODE_PTR node, LIST_PTR *term_list)
{
   LIST_PTR temp1;
   if (node->exper_type == PLUS_EXPER) {
      collect_terms(node->left, term_list);
      collect_terms(node->right, term_list);
      delete node;
      }
   else {
      if (*term_list == NULL)
         *term_list = make_list_node(node);
      else {
         temp1 = make_list_node(node);
         temp1->next = *term_list;
         *term_list = temp1;
         }
      }
}

/* Collect common terms, sort them, then print them out. */
/** @brief Collect, flatten, and combine additive terms from an expression tree.
 *  @param node Root expression node.
 *  @return Sorted linked list of additive terms.
 */
LIST_PTR collect_additive_terms(NODE_PTR node)
{
   LIST_PTR term_list = NULL;
   /* First take all of the terms in the expression and collect into a
      single linked list. */
   collect_terms(node, &term_list);

   /* Sort them, collecting common terms. */
   term_list = sort_terms(term_list);

   return term_list;
}
