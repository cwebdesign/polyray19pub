/* simplifier.c

   Algebraic equation -> polynomial coefficient translation

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
#include "ytab.h"
#include "io.h"
#include "memory.h"
#include "builder.h"
#include "roots.h"
#include "eval.h"
#include "simplify.h"

/* Simplify additive terms. */
static NODE_PTR
simplify_plus(NODE_PTR node)
{
   NODE_PTR left, right;
   left = node->left;
   right = node->right;

   if (left->exper_type == TERM &&
       right->exper_type == TERM &&
       left->exper_data.coeff.x_power == right->exper_data.coeff.x_power &&
       left->exper_data.coeff.y_power == right->exper_data.coeff.y_power &&
       left->exper_data.coeff.z_power == right->exper_data.coeff.z_power) {
      node->exper_type = TERM;
      node->exper_data.coeff.coeff = left->exper_data.coeff.coeff +
                                     right->exper_data.coeff.coeff;
      node->exper_data.coeff.x_power = left->exper_data.coeff.x_power;
      node->exper_data.coeff.y_power = left->exper_data.coeff.y_power;
      node->exper_data.coeff.z_power = left->exper_data.coeff.z_power;
      node->left = NULL;
      node->right = NULL;
      polyray_free(left);
      polyray_free(right);
      return node;
      }
   else
      return node;
}

/* Simplify multiplicative terms. */
static NODE_PTR
simplify_times(NODE_PTR node)
{
   NODE_PTR left, right, t0, t1, t2, t3;
   left = node->left;
   right = node->right;

   if (left->exper_type == TERM)
      if (right->exper_type == TERM) {
         node->exper_type = TERM;
         node->exper_data.coeff.coeff = left->exper_data.coeff.coeff *
                                        right->exper_data.coeff.coeff;
         node->exper_data.coeff.x_power = left->exper_data.coeff.x_power +
                                          right->exper_data.coeff.x_power;
         node->exper_data.coeff.y_power = left->exper_data.coeff.y_power +
                                          right->exper_data.coeff.y_power;
         node->exper_data.coeff.z_power = left->exper_data.coeff.z_power +
                                          right->exper_data.coeff.z_power;
         node->left = NULL;
         node->right = NULL;
         polyray_free((void *)left);
         polyray_free((void *)right);
         return node;
         }
      else if (right->exper_type == PLUS_EXPER) {
         t0 = simplify(make_node(TIMES_EXPER, copy_node(left), right->left), 0);
         t1 = simplify(make_node(TIMES_EXPER, left, right->right), 0);
         polyray_free(right);
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
         polyray_free(left);
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
static NODE_PTR
simplify_power(NODE_PTR node, int minus_flag)
{
   NODE_PTR left, right, t0, t1, t2, head;
   int n, i;
   unsigned long j;

   left = node->left;
   right = node->right;

   if (right->exper_type != TERM) {
      error("Invalid expression as power");
      return node;
      }
   else if (right->exper_data.coeff.coeff == 0.0) {
      node->exper_type = TERM;
      node->exper_data.coeff.coeff = (minus_flag ? -1.0 : 1.0);
      node->exper_data.coeff.x_power = 0.0;
      node->exper_data.coeff.y_power = 0.0;
      node->exper_data.coeff.z_power = 0.0;
      node->left = NULL;
      node->right = NULL;
      deallocate_node(left);
      polyray_free((void *)right);
      return node;
      }
   else if (right->exper_data.coeff.coeff == 1.0) {
      polyray_free(right);
      polyray_free(node);
      return simplify(left, minus_flag);
      }
   else if (left->exper_type == TERM) {
      node->exper_type = TERM;
      node->exper_data.coeff.coeff = pow(left->exper_data.coeff.coeff,
                                         right->exper_data.coeff.coeff) *
                                          (minus_flag ? -1.0 : 1.0);
      node->exper_data.coeff.x_power = left->exper_data.coeff.x_power *
                                       right->exper_data.coeff.coeff;
      node->exper_data.coeff.y_power = left->exper_data.coeff.y_power *
                                       right->exper_data.coeff.coeff;
      node->exper_data.coeff.z_power = left->exper_data.coeff.z_power *
                                       right->exper_data.coeff.coeff;
      node->left = NULL;
      node->right = NULL;
      polyray_free((void *)left);
      polyray_free((void *)right);
      return node;
      }
   else if (left->exper_type == PLUS_EXPER) {
      head = NULL;
      n = (int)(right->exper_data.coeff.coeff);
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
      error("Simplification failed\n");
      return NULL;
      }
}

/* Once a parse tree has been created we need to convert it into a form
   that can be more easily manipulated. */
NODE_PTR
simplify(NODE_PTR node, int minus_flag)
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
   case X_EXPER:
      node->exper_type = TERM;
      node->exper_data.coeff.coeff   = (minus_flag?-1.0:1.0);
      node->exper_data.coeff.x_power = 1.0;
      node->exper_data.coeff.y_power = 0.0;
      node->exper_data.coeff.z_power = 0.0;
      return node;
   case Y_EXPER:
      node->exper_type = TERM;
      node->exper_data.coeff.coeff   = (minus_flag?-1.0:1.0);
      node->exper_data.coeff.x_power = 0.0;
      node->exper_data.coeff.y_power = 1.0;
      node->exper_data.coeff.z_power = 0.0;
      return node;
   case Z_EXPER:
      node->exper_type = TERM;
      node->exper_data.coeff.coeff   = (minus_flag?-1.0:1.0);
      node->exper_data.coeff.x_power = 0.0;
      node->exper_data.coeff.y_power = 0.0;
      node->exper_data.coeff.z_power = 1.0;
      return node;
   case VAL_EXPER:
      node->exper_type = TERM;
      node->exper_data.coeff.coeff   = (minus_flag?-1.0:1.0) *
                                       node->exper_data.value;
      node->exper_data.coeff.x_power = 0.0;
      node->exper_data.coeff.y_power = 0.0;
      node->exper_data.coeff.z_power = 0.0;
      return node;
   case TERM:
      if (minus_flag) node->exper_data.coeff.coeff *= -1.0;
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
      if (tnode->exper_type == VAL_EXPER &&
          tnode->exper_data.coeff.coeff != 0.0) {
         if (node->left->exper_type == VAL_EXPER) {
            node->exper_data.coeff.coeff =
               node->left->exper_data.coeff.coeff /
                  node->right->exper_data.coeff.coeff;
            deallocate_node(node->left);
            deallocate_node(node->right);
            node->exper_type = VAL_EXPER;
            }
         else {
            node->exper_type = TIMES_EXPER;
            node->right = node->left;
            node->left = tnode;
            tnode->exper_data.coeff.coeff = 1.0 / tnode->exper_data.coeff.coeff;
            node = simplify_times(node);
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
      polyray_free(node);
      return tnode;
   default:
      return node;
   }
}

/* Simple insertion sort of the expressions on the list. */
static LIST_PTR
sort_terms(LIST_PTR term_list)
{
   LIST_PTR new_list = NULL;
   LIST_PTR temp1, temp2, temp3, last;
   NODE_PTR term, test_term;
   int flag;

   temp1 = term_list;
   while (temp1 != NULL) {
      term = temp1->element;
      if (new_list == NULL)
         /* nothing to compare against */
         new_list = make_list_node(term);
      else if (term->exper_type != TERM) {
         /* Can only compare base terms, even worse getting here means
            that simplification has failed. */
         temp2 = make_list_node(term);
         temp2->next = new_list;
         new_list = temp2;
         }
      else if (fabs(term->exper_data.coeff.coeff) < 1.0e-10) {
         /* Non-contributing term, get rid of it. */
         polyray_free(term);
         }
      else {
         /* First see if it matches any of the existing terms */
         temp2 = new_list;
         last = new_list;
         flag = 0;
         while (temp2 != NULL && !flag) {
            test_term = temp2->element;
            if (test_term->exper_type == TERM) {
               if (test_term->exper_data.coeff.x_power ==
                   term->exper_data.coeff.x_power &&
                   test_term->exper_data.coeff.y_power ==
                   term->exper_data.coeff.y_power &&
                   test_term->exper_data.coeff.z_power ==
                   term->exper_data.coeff.z_power) {
                  /* Can collect these terms into one. */
                  test_term->exper_data.coeff.coeff +=
                     term->exper_data.coeff.coeff;
                  polyray_free(term);
                  flag = 1;
                  }
               else if (test_term->exper_data.coeff.x_power <
                        term->exper_data.coeff.x_power ||
                        (test_term->exper_data.coeff.x_power ==
                         term->exper_data.coeff.x_power &&
                         (test_term->exper_data.coeff.y_power <
                          term->exper_data.coeff.y_power ||
                          (test_term->exper_data.coeff.y_power ==
                           term->exper_data.coeff.y_power &&
                           test_term->exper_data.coeff.z_power <
                           term->exper_data.coeff.z_power)))) {
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
            if (new_list == NULL)
               new_list = temp2;
            else {
               last = new_list;
               while (last->next != NULL) last = last->next;
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
   while (temp1!=NULL) {
      term = temp1->element;
      if (term->exper_type == TERM &&
          fabs(term->exper_data.coeff.coeff) < 1.0e-20) {
         /* Non-contributing term. remove it. */
         if (temp1 == new_list) {
            temp1 = temp1->next;
            polyray_free(new_list);
            polyray_free(term);
            new_list = temp1;
            last = temp1;
            }
         else {
            temp2 = temp1;
            temp1 = temp1->next;
            last->next = temp1;
            polyray_free(temp2);
            polyray_free(term);
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
static void
collect_terms(NODE_PTR node, LIST_PTR *term_list)
{
   LIST_PTR temp1;
   if (node->exper_type == PLUS_EXPER) {
      collect_terms(node->left, term_list);
      collect_terms(node->right, term_list);
      polyray_free(node);
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
LIST_PTR
collect_additive_terms(NODE_PTR node)
{
   LIST_PTR term_list = NULL;
   /* First take all of the terms in the expression and collect into a
      single linked list. */
   collect_terms(node, &term_list);

   /* Sort them, collecting common terms. */
   term_list = sort_terms(term_list);

   return term_list;
}

