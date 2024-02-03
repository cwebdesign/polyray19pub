/* particle.c

   Processing for particle systems

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
#include "symtab.h"
#include "vector.h"
#include "psupport.h"
#include "builder.h"
#include "eval.h"
#include "intersec.h"
#include "particle.h"

Particle *
CreateParticle()
{
   Particle *particle;

   particle = (Particle *)polyray_malloc(sizeof(Particle));
   if (particle == NULL)
      error("Failed to create particle");
   particle->Birth         = NULL;
   particle->Death         = NULL;
   particle->Count         = NULL;
   particle->P             = NULL;
   particle->V             = NULL;
   particle->A             = NULL;
   particle->Avoid         = NULL;
   particle->Flock         = NULL;
   particle->obj_name      = NULL;
   particle->next          = NULL;
   particle->particle_name = NULL;
   return particle;
}

Particle *
CopyParticle(char *name)
{
   int c;
   char *part_name;
   Particle *new_part, *old_part;

   /* If the particle system has already been created and placed
      on the stack, then we only need to return a pointer to it. */
   for (old_part=Particles; old_part!=NULL; old_part=old_part->next)
      if (old_part->particle_name != NULL &&
          strcmp(name, old_part->particle_name) == 0)
         return old_part;

   /* We don't have one of these already, so we need to instantiate
      the particle and put it onto the particle generator stack */
   Lookup_Definition(name, &c, (void *)&old_part);
   if (c != T_PARTICLE)
      error("Particle '%s' not found in symbol table\n", name);
   new_part = (Particle *)polyray_malloc(sizeof(Particle));
   if (new_part == NULL)
      error("Failed to create particle");
   part_name = polyray_malloc(strlen(name) + 1);
   if (part_name == NULL) error("Out of memory");
   strcpy(part_name, name);
   new_part->Birth         = copy_node(old_part->Birth);
   new_part->Death         = copy_node(old_part->Death);
   new_part->Count         = copy_node(old_part->Count);
   new_part->P             = copy_node(old_part->P);
   new_part->V             = copy_node(old_part->V);
   new_part->A             = copy_node(old_part->A);
   new_part->Avoid         = copy_node(old_part->Avoid);
   new_part->Flock         = copy_node(old_part->Flock);
   new_part->obj_name      = copy_node(old_part->obj_name);
   new_part->particle_name = part_name;
   new_part->next          = NULL;

   /* Put the new particle generator on the stack */
   new_part->next = Particles;
   Particles = new_part;

   /* Return the particle generator */
   return new_part;
}

void
InsertParticle(Particle *particle)
{
   particle->next = Particles;
   Particles = particle;
}

void
SetParticleP(Particle *particle, NODE_PTR P)
{
   deallocate_node(particle->P);
   particle->P = P;
}

void
SetParticleV(Particle *particle, NODE_PTR V)
{
   deallocate_node(particle->V);
   particle->V = V;
}

void
SetParticleA(Particle *particle, NODE_PTR A)
{
   deallocate_node(particle->A);
   particle->A = A;
}

void
SetParticleAvoid(Particle *particle, NODE_PTR Avoid)
{
   deallocate_node(particle->Avoid);
   particle->Avoid = Avoid;
}

void
SetParticleFlock(Particle *particle, NODE_PTR Flock)
{
   deallocate_node(particle->Flock);
   particle->Flock = Flock;
}

void
SetParticleBirth(Particle *particle, NODE_PTR Birth)
{
   deallocate_node(particle->Birth);
   particle->Birth = Birth;
}

void
SetParticleDeath(Particle *particle, NODE_PTR Death)
{
   deallocate_node(particle->Death);
   particle->Death = Death;
}

void
SetParticleCount(Particle *particle, NODE_PTR Count)
{
   deallocate_node(particle->Count);
   particle->Count = Count;
}

void
SetParticleObjName(Particle *particle, NODE_PTR obj_name)
{
   deallocate_node(particle->obj_name);
   particle->obj_name = obj_name;
}

static ParticleObject *
CreateParticleObject(Particle *particle, SUBST_PTR subst)
{
   ParticleObject *pobj;
   Flt fval;
   NODE_PTR nval;

   pobj = (ParticleObject *)polyray_malloc(sizeof(ParticleObject));
   if (particle == NULL)
      error("Failed to create particle wrapper");
   pobj->parent = particle;
   pobj->age = 0.0;
   pobj->next = NULL;

   if (particle->P == NULL ||
       eval_node(subst, particle->P, &fval, pobj->P, &nval) != 2)
      MakeVector(0, 0, 0, pobj->P)
   if (particle->V == NULL ||
       eval_node(subst, particle->V, &fval, pobj->V, &nval) != 2)
      MakeVector(0, 0, 0, pobj->V)
   return pobj;
}

static void
NewParticleObject(Particle *particle, SUBST_PTR subst)
{
   ParticleObject *pobj;

   pobj = CreateParticleObject(particle, subst);
   pobj->next = ParticleObjects;
   ParticleObjects = pobj;
}

void
DeleteParticle(Particle *particle)
{
   deallocate_node(particle->P);
   deallocate_node(particle->V);
   deallocate_node(particle->A);
   deallocate_node(particle->Birth);
   deallocate_node(particle->Death);
   deallocate_node(particle->Avoid);
   deallocate_node(particle->Count);
   deallocate_node(particle->obj_name);
   if (particle->particle_name != NULL)
      polyray_free(particle->particle_name);
   polyray_free(particle);
}

void
FreeParticles()
{
   Particle *last;
   ParticleObject *lasto;

   while (ParticleObjects != NULL) {
      lasto = ParticleObjects;
      ParticleObjects = ParticleObjects->next;
      polyray_free(lasto);
      }
   ParticleObjects = NULL;
   while (Particles != NULL) {
      last = Particles;
      Particles = Particles->next;
      DeleteParticle(last);
      }
   Particles = NULL;
}

static void
CreateParticleWrapper(Viewpoint *Eye, SUBST_PTR subst, ParticleObject *part)
{
   Flt fval, dist, tdist;
   Vec vval, V, R;
   NODE_PTR nval;
   char *objname;
   Object *obj;
   Isect hit;
   Ray path;

   /* This particle is still alive, build a wrapper and figure
      out where it's current position should be. */
   if (eval_node(subst, part->parent->obj_name, &fval, vval, &nval) == 3 &&
       create_string(nval, &objname)) {
      obj = object_action2(objname);
      obj->o_sflag |= PARTICLE_FLAG;
      polyray_free(objname);
      }
   else {
      message("Failed determine object name from:");
      show_node(part->parent->obj_name);
      error("Missing object name");
      }

   /* Move it into position */
   TranslateObject(obj, part->P);

   /* Add the particle's wrapper to the database */
   Add_To_BinTree(&Root, obj);

   /* Move the particle into position for the next frame */
   if (part->parent->A == NULL ||
       eval_node(subst, part->parent->A, &fval, vval, &nval) != 2)
      MakeVector(0, 0, 0, vval)
   VecScale(frame_time, vval);
   VecAdd(part->V, vval, part->V)

   if (part->parent->Avoid != NULL) {
      VecCopy(part->V, vval);
      dist  = VecNormalize(vval);
      tdist = frame_time * dist;
      VecCopy(part->P, path.P);
      VecCopy(vval, path.D);
      if (Intersect(Eye, &Root, &path, rayeps, tdist, &hit) &&
          (hit.isect_t < frame_time * dist)) {
#if 0
printf("H: %g/%g - N: <%g,%g,%g>, Nlen: %g, vlen: %g\n", hit.isect_t, tdist,
       hit.N[0], hit.N[1], hit.N[2], VecLen(hit.N), VecLen(vval));
#endif
         /* There is a collision within the distance this particle
            will travel over the course of the next frame */
         VecCopy(vval, V);
         VecNegate(V);
         VecNormalize(hit.N);
         if (VecDot(V, hit.N) < 0.0) {
            VecNegate(hit.N)
            }
         SpecularDirection(V, hit.N, R);
         VecCopy(R, part->V);
         VecScale(dist, part->V);
         dist = tdist - hit.isect_t;
         VecAddScaled(hit.W, dist, R, part->P);
#if 0
printf("V: <%g,%g,%g> -> <%g,%g,%g>\n",
       vval[0], vval[1], vval[2], R[0], R[1], R[2]);
printf("dist: %g\n", dist);
#endif
         }
   /* Normal movement, set new position */
      else
         VecAddScaled(part->P, frame_time, part->V, part->P)
      }
   else
      VecAddScaled(part->P, frame_time, part->V, part->P)
}

static void
InstantiateParticleObjects(Viewpoint *Eye)
{
   struct subst_struct subst;
   ParticleObject *temp, *last;
   ParticleObject *new_pobj, *tpobj;
   Particle *new_part;
   Flt fval;
   Vec vval;
   NODE_PTR nval;
   char *objname;
   int death_val, i, j;

   /* First walk through existing particle objects to see if they die */
   last = temp = ParticleObjects;
   new_pobj = NULL;
   while (temp != NULL) {
      /* Set the substitiution structure to reflect the current state of
         the particle */
      reset_subst(&subst);
      temp->age += frame_time;
      subst.U[0] = temp->age;
      VecCopy(temp->P, subst.P);
      VecCopy(temp->V, subst.I);

      if ((temp->parent->Death == NULL) ||
          ((death_val = eval_node(&subst, temp->parent->Death,
                                  &fval, vval, &nval)) == 1 &&
           fval == 0)) {
         CreateParticleWrapper(Eye, &subst, temp);
         if (ParticleObjects != temp)
            last = last->next;
         temp = temp->next;
         }
      else {
         /* This particle died, see if it creates a new system, then
            clean up it's memory & remove from the stack. */
         if ((temp->parent->Death != NULL) && (death_val == 3) &&
             create_string(nval, &objname)) {
            /* This particle activates yet another particle system - if
               it's a new particle system then it is added to the stack
               of particle systems. */
            new_part = CopyParticle(objname);
            polyray_free(objname);

            /* Create new particles */
            if (new_part->Count == NULL)
               j = 1;
            else if (eval_node(&subst, new_part->Count,
                               &fval, vval, &nval) == 1)
               j = (int)fval;
            else
               error("Invalid particle count");

            /* Create the new particles using the values for position,
               velocity, acceleration, etc. from the dying particle */
            subst.U[0] = 0;
            for (i=0;i<j;i++) {
               if (new_pobj == NULL) {
                  new_pobj = CreateParticleObject(new_part, &subst);
                  tpobj = new_pobj;
                  }
               else {
                  tpobj->next = CreateParticleObject(new_part, &subst);
                  tpobj = tpobj->next;
                  }
               CreateParticleWrapper(Eye, &subst, tpobj);
               }
            }

         /* Remove this particle object from the list */
         if (ParticleObjects == temp) {
            last = ParticleObjects;
            temp = ParticleObjects = ParticleObjects->next;
            polyray_free(last);
            last = ParticleObjects;
            }
         else {
            last->next = temp->next;
            polyray_free(temp);
            temp = last->next;
            }
         }
      }

   /* Add any new particle objects to the stack */
   if (new_pobj != NULL) {
      last = new_pobj;
      while (last->next != NULL) {
         last = last->next;
         }
      last->next = ParticleObjects;
      ParticleObjects = new_pobj;
      }
}

void
InstantiateParticles(Viewpoint *Eye)
{
   struct subst_struct subst;
   Particle *temp;
   Flt fval;
   Vec vval;
   NODE_PTR nval;
   int i, j=0;
   int c;

   /* Indicate that we are building particles */
   Particle_Test = 1;

   /* First walk through existing particle objects to see if something new
      is born */
   temp = Particles;
   reset_subst(&subst);
   c = 0;
   while (temp != NULL) {
      if ((temp->Birth == NULL && current_frame == start_frame) ||
          (temp->Birth != NULL && eval_node(&subst, temp->Birth, &fval,
                                            vval, &nval) == 1 &&
           fval != 0)) {
         /* Create new particles */
         if (temp->Count == NULL)
            j = 1;
         else if (eval_node(&subst, temp->Count, &fval, vval, &nval) == 1)
            j = (int)fval;
         else
            error("Invalid particle count");

         /* Build the particles */
         for (i=0;i<j;i++)
            NewParticleObject(temp, &subst);
         }
      temp = temp->next;
      c++;
      }

   /* Activate any live particles */
   InstantiateParticleObjects(Eye);

   /* No longer building particles */
   Particle_Test = 0;
}
