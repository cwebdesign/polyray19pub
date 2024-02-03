#include "defs.h"
#include "memory.h"
#include "io.h"
#include "scan.h"
#include "mcube.h"
#include "vector.h"

/* The values of each of the unique hot/cold vertex
   combinations of a cube.  Under solid rotations of
   a cube, all 256 possible colorings of the vertices
   of a cube with two colors (hot/cold) can be transformed
   into one of these 23 base cases.  */
#if 0
static unsigned char base_corners[23] =
   { 0x00, 0x01, 0x03, 0x09, 0x81, 0x07, 0x43, 0x29,
     0x0f, 0x17, 0x27, 0x47, 0x87, 0x69, 0xa5, 0x1f,
     0x97, 0x67, 0x3f, 0x6f, 0xe7, 0x7f, 0xff};
#endif

/* List of the permutations of the vertices of a cube
   when it is rotated from one position to another */
static unsigned char permute_list[24][8] = {
   {0, 1, 2, 3, 4, 5, 6, 7},
   {4, 5, 0, 1, 6, 7, 2, 3},
   {6, 7, 4, 5, 2, 3, 0, 1},
   {2, 3, 6, 7, 0, 1, 4, 5},
   {2, 0, 3, 1, 6, 4, 7, 5},
   {3, 2, 1, 0, 7, 6, 5, 4},
   {1, 3, 0, 2, 5, 7, 4, 6},
   {1, 5, 3, 7, 0, 4, 2, 6},
   {5, 4, 7, 6, 1, 0, 3, 2},
   {4, 0, 6, 2, 5, 1, 7, 3},
   {1, 0, 5, 4, 3, 2, 7, 6},
   {2, 6, 0, 4, 3, 7, 1, 5},
   {4, 6, 5, 7, 0, 2, 1, 3},
   {7, 3, 5, 1, 6, 2, 4, 0},
   {7, 5, 6, 4, 3, 1, 2, 0},
   {7, 6, 3, 2, 5, 4, 1, 0},
   {0, 4, 1, 5, 2, 6, 3, 7},
   {0, 2, 4, 6, 1, 3, 5, 7},
   {3, 1, 7, 5, 2, 0, 6, 4},
   {5, 1, 4, 0, 7, 3, 6, 2},
   {6, 4, 2, 0, 7, 5, 3, 1},
   {3, 7, 2, 6, 1, 5, 0, 4},
   {6, 2, 7, 3, 4, 0, 5, 1},
   {5, 7, 1, 3, 4, 6, 0, 2}};

/* For each of the 256 possible hot/cold colorings of
   the vertices of a cube, this table relates that
   coloring to one of the 23 unique base cases.  The
   table contains the index of the permutation that
   takes this case to the base case, followed by the
   index of the base case that it is taken to. */
struct perm_table_struct {
   unsigned char perm;
   unsigned char base;
   } corner_perm[256] = {
     0, 0,  0, 1,  6, 1,  0, 2,  3, 1,  4, 2,  6, 3,  0, 5,
     5, 1,  0, 3,  6, 2,  6, 5,  3, 2,  4, 5,  5, 5,  0, 8,
     1, 1,  9, 2,  1, 3, 16, 5,  9, 3, 17, 5,  4, 7,  0, 9,
     1, 4, 16, 6,  6, 6,  6,11,  3, 6,  4,10,  5,12,  0,15,
     8, 1, 16, 3,  7, 2, 10, 5,  3, 4, 17, 6,  7, 6,  0,10,
    18, 3,  0, 7,  7, 5,  6, 9,  5, 6,  4,12,  5,11,  6,15,
     1, 2,  1, 5, 19, 5,  1, 8,  1, 6,  1,11, 19,12, 16,15,
     8, 6,  1,12,  7,10, 10,15,  4,14, 17,17,  7,17,  0,18,
     2, 1, 17, 3,  2, 4,  0, 6, 11, 2, 11, 5, 11, 6,  0,11,
    21, 3,  5, 7, 18, 6,  6,12,  3, 5,  3, 9,  3,10,  4,15,
    12, 2,  9, 5, 12, 6,  9,10, 20, 5,  9, 8, 20,12, 17,15,
    20, 6,  9,12,  2,14, 16,17,  3,11, 11,15,  3,17,  4,18,
     2, 3,  8, 7, 19, 6, 10,12, 22, 6, 11,12,  3,14,  0,17,
     2, 7,  0,13,  7,12,  6,16,  3,12,  3,16,  5,17,  0,19,
    12, 5,  1, 9, 12,11,  1,15, 12,10,  9,15, 12,17,  9,18,
    12,12,  1,16, 19,17, 16,19, 20,17, 17,19,  3,20,  0,21,
    13, 1,  0, 4,  7, 3, 10, 6,  3, 3,  4, 6,  3, 7,  0,12,
    13, 2, 21, 6, 18, 5,  6,10, 21, 5,  4,11,  5, 9,  5,15,
    12, 3,  9, 6,  1, 7, 16,12, 11, 7, 17,12,  3,13,  0,16,
    13, 6,  1,14, 18,12,  6,17, 21,12,  4,17,  5,16,  6,19,
    14, 2, 23, 6, 23, 5, 10,11, 14, 6,  0,14, 23,12, 10,17,
    13, 5, 13,12,  7, 8,  7,15, 13,10, 21,17, 18,15,  6,18,
     8, 5,  1,10,  8, 9, 19,15,  8,12,  1,17,  8,16,  1,19,
     8,11,  8,17, 23,15,  7,18, 13,17,  1,20,  7,19,  6,21,
     2, 2,  2, 6, 15, 6,  6,14, 22, 5, 11,10, 22,12, 11,17,
    15, 5, 15,12, 15,11, 18,17,  3, 8,  3,15, 21,15,  3,18,
     2, 5,  2,11,  2,12,  9,17,  2, 9, 20,15,  2,16,  9,19,
     2,10,  2,17, 15,17,  2,20, 22,15, 11,18,  3,19,  3,21,
    14, 5, 14,12, 14,10, 23,17, 14,11, 22,17, 14,17,  0,20,
    13, 9, 13,16, 13,15, 18,19, 15,15, 21,19, 13,18,  5,21,
     2, 8, 12,15,  8,15,  1,18,  2,15, 12,18, 12,19,  1,21,
    14,15,  2,19, 14,18,  8,21,  2,18,  2,21, 13,21,  0,22};

/* For each of the 23 base colorings, this table gives the
   number of triangles that are needed to separate hot from
   cold, along with a list of vertex pairs that make up the
   triangles. */
/* Note: should be able to use this table together with the
   permutation tables to generate a full (256) entry table
   of triangles for every possible hot/cold cube. */
#if 0
unsigned char edge_table[] = {
/*  0 */   0,
/*  1 */   1, 0, 2, 0, 1, 0, 4,
/*  2 */   2, 0, 2, 1, 5, 0, 4, 0, 2, 1, 3, 1, 5,
/*  3 */   2, 0, 2, 0, 1, 0, 4, 1, 3, 2, 3, 3, 7,
/*  4 */   2, 0, 2, 0, 1, 0, 4, 3, 7, 6, 7, 5, 7,
/*  5 */   3, 0, 4, 2, 6, 1, 5, 2, 6, 2, 3, 1, 5,
              1, 5, 2, 3, 1, 3,
/*  6 */   3, 2, 6, 4, 6, 6, 7, 0, 4, 0, 2, 1, 5,
              0, 2, 1, 3, 1, 5,
/*  7 */   3, 0, 2, 0, 1, 0, 4, 1, 5, 5, 7, 4, 5,
              2, 3, 3, 7, 1, 3,
/*  8 */   2, 0, 4, 2, 6, 1, 5, 1, 5, 2, 6, 3, 7,
/*  9 */   4, 2, 6, 4, 5, 4, 6, 2, 6, 1, 5, 4, 5,
              2, 6, 2, 3, 1, 5, 2, 3, 1, 3, 1, 5,
/* 10 */   4, 0, 4, 2, 6, 4, 5, 2, 6, 5, 7, 4, 5,
              2, 6, 2, 3, 5, 7, 2, 3, 1, 3, 5, 7,
/* 11 */   4, 0, 4, 1, 3, 1, 5, 0, 4, 2, 3, 1, 3,
              0, 4, 4, 6, 2, 3, 4, 6, 6, 7, 2, 3,
/* 12 */   4, 0, 4, 2, 6, 1, 5, 2, 6, 2, 3, 1, 5,
              1, 5, 2, 3, 1, 3, 3, 7, 6, 7, 5, 7,
/* 13 */   4, 0, 4, 4, 6, 4, 5, 0, 1, 1, 5, 1, 3,
              0, 2, 2, 3, 2, 6, 5, 7, 6, 7, 3, 7,
/* 14 */   4, 0, 4, 2, 6, 4, 5, 4, 5, 2, 6, 6, 7,
              0, 1, 1, 5, 2, 3, 1, 5, 3, 7, 2, 3,
/* 15 */   3, 4, 6, 2, 6, 4, 5, 2, 6, 1, 5, 4, 5,
              1, 5, 2, 6, 3, 7,
/* 16 */   3, 2, 6, 6, 7, 4, 6, 4, 5, 5, 7, 1, 5,
              1, 3, 3, 7, 2, 3,
/* 17 */   3, 0, 4, 4, 6, 4, 5, 1, 3, 6, 7, 2, 3,
              1, 3, 5, 7, 6, 7,
/* 18 */   2, 4, 6, 2, 6, 3, 7, 4, 6, 3, 7, 5, 7,
/* 19 */   2, 0, 4, 4, 6, 4, 5, 3, 7, 5, 7, 6, 7,
/* 20 */   2, 0, 4, 4, 6, 4, 5, 1, 3, 3, 7, 2, 3,
/* 21 */   1, 3, 7, 5, 7, 6, 7,
/* 22 */   0 };

unsigned int edge_table_indices[23] =
   {   0,   1,   8,  21,  34,  47,  66,  85, 104, 117,
     142, 167, 192, 217, 242, 267, 286, 305, 324, 337,
     350, 363, 370};
#else
unsigned char edge_table[] = {
/*  0 */   0,
/*  1 */   1, 0, 2, 0, 1, 0, 4,
/*  2 */   2, 0, 2, 1, 5, 0, 4, 0, 2, 1, 3, 1, 5,
/*  3 */   2, 0, 2, 0, 1, 0, 4, 1, 3, 2, 3, 3, 7,
/*  4 */   2, 0, 2, 0, 1, 0, 4, 3, 7, 6, 7, 5, 7,
/*  5 */   3, 0, 4, 2, 6, 1, 5, 2, 6, 2, 3, 1, 5,
              1, 5, 2, 3, 1, 3,
/*  6 */   3, 2, 6, 4, 6, 6, 7, 0, 4, 0, 2, 1, 5,
              0, 2, 1, 3, 1, 5,
/*  7 */   3, 0, 2, 0, 1, 0, 4, 1, 5, 5, 7, 4, 5,
              2, 3, 3, 7, 1, 3,
/*  8 */   2, 0, 4, 2, 6, 1, 5, 1, 5, 2, 6, 3, 7,
/*  9 */   4, 2, 6, 4, 5, 4, 6, 2, 6, 1, 5, 4, 5,
              2, 6, 2, 3, 1, 5, 2, 3, 1, 3, 1, 5,
/* 10 */   4, 0, 4, 2, 6, 4, 5, 2, 6, 5, 7, 4, 5,
              2, 6, 2, 3, 5, 7, 2, 3, 1, 3, 5, 7,
/* 11 */   4, 0, 4, 1, 3, 1, 5, 0, 4, 2, 3, 1, 3,
              0, 4, 4, 6, 2, 3, 4, 6, 6, 7, 2, 3,
/* 12 */   4, 0, 4, 2, 6, 1, 5, 2, 6, 2, 3, 1, 5,
              1, 5, 2, 3, 1, 3, 3, 7, 6, 7, 5, 7,
/* 13 */   4, 0, 1, 0, 2, 0, 4, 1, 5, 4, 5, 5, 7,
              1, 3, 2, 3, 3, 7, 2, 6, 4, 6, 6, 7,
/* 14 */   4, 0, 1, 2, 3, 2, 6, 0, 1, 0, 4, 2, 6,
              1, 5, 4, 5, 6, 7, 1, 5, 3, 7, 6, 7,
/* 15 */   3, 4, 6, 2, 6, 4, 5, 2, 6, 1, 5, 4, 5,
              1, 5, 2, 6, 3, 7,
/* 16 */   5, 1, 5, 1, 3, 2, 3, 1, 5, 2, 3, 2, 6,
              1, 5, 2, 6, 4, 5, 2, 6, 4, 5, 4, 6,
              3, 7, 5, 7, 6, 7,
/* 17 */   5, 1, 3, 4, 5, 5, 7, 1, 3, 0, 4, 4, 5,
              0, 4, 1, 3, 2, 3, 0, 4, 2, 3, 4, 6,
              2, 3, 4, 6, 6, 7,
/* 18 */   2, 4, 6, 2, 6, 3, 7, 4, 6, 3, 7, 5, 7,
#if 1 /* Corrected ordering of vertices */
/* 19 */   4, 0, 4, 5, 7, 4, 5, 0, 4, 3, 7, 5, 7,
              0, 4, 6, 7, 3, 7, 0, 4, 4, 6, 6, 7,
#else /* Old ordering of vertices */
/* 19 */   4, 0, 4, 4, 5, 5, 7, 0, 4, 5, 7, 3, 7,
              0, 4, 6, 7, 3, 7, 0, 4, 4, 6, 6, 7,
#endif
/* 20 */   2, 0, 4, 4, 6, 4, 5, 1, 3, 3, 7, 2, 3,
/* 21 */   1, 3, 7, 5, 7, 6, 7,
/* 22 */   0 };

unsigned int edge_table_indices[23] =
   {   0,   1,   8,  21,  34,  47,  66,  85, 104, 117,
     142, 167, 192, 217, 242, 267, 286, 317, 348, 361,
     386, 399, 406};
#endif

/* This routine determines how many triangles are needed for
   a given hot/cold cube, as well as the appropriate indices */
static void
split_cubes(Viewpoint *eye, BinTree *Root, Vec V[8], Flt C[8],
            unsigned char v, Flt T,
            int (*gradient)(Object *, Vec, Vec),
            Object *obj)
{
   Poly Polygon;
   int i, base, perm, toff, tcnt;
   int indl, indh;
   Vec P[3], N[3], v0, v1;
   Flt t0, t1, ratio0, ratio1;

   base = corner_perm[v].base;
   perm = corner_perm[v].perm;
   toff = edge_table_indices[base];

   for (tcnt=edge_table[toff];tcnt>0;tcnt--) {
      /* Step through all triangles associated with this
         cube.  As they are generated, transform them from
         their base values into their actual values */
      Polygon.n = 3;
      for (i=0;i<3;i++) {
         /* Grab each pair of vertices of this triangle,
            unwind the permutation, then use the unwound
            values to make a triangle in object space */
         indl = edge_table[++toff];
         indh = edge_table[++toff];
         indl = permute_list[perm][indl];
         indh = permute_list[perm][indh];

         /* Interpolate the vertex positions based on the
            ratio of hot to cold */
         if (C[indl] < T) {
            /* Start vertex is hot */
            t0 = T - C[indl];
            t1 = C[indh] - T;
            ratio0 = t1 / (t0 + t1);
            ratio1 = 1.0 - ratio0;
            }
         else {
            /* End vertex is hot */
            t0 = T - C[indh];
            t1 = C[indl] - T;
            ratio0 = t0 / (t0 + t1);
            ratio1 = 1.0 - ratio0;
            }

         VecComb(ratio0, V[indl], ratio1, V[indh], P[i]);

         VecCopy(P[i], Polygon.vertices[i].W);

         if (obj->o_trans)
            InvTxVector(P[i], P[i], obj->o_trans)

         /* Not sure what to do if the call to gradient fails... */
         if (gradient != NULL)
            gradient(obj, P[i], N[i]);

         VecCopy(P[i], Polygon.vertices[i].P)
         }

      /* If a function exists for evaluating the gradient, then
         the values for the normal already exist - transform them
         into the correct orientation. */
      if (gradient != NULL) {
         if (obj->o_trans)
            for (i=0;i<3;i++)
               TxNormal(N[i], N[i], obj->o_trans);
         for (i=0;i<3;i++)
            VecNormalize(N[i]);
         }
      else {
         /* No gradient function, calculate normal based on
            vertex positions */
         VecSub(P[2], P[0], v0);
         VecSub(P[1], P[0], v1);
         VecCross(v0, v1, N[0]);
         VecNormalize(N[0]);
         VecCopy(N[0], N[1]);
         VecCopy(N[0], N[2]);
         }

      /* Copy values into polygon structure for scan conversion */
      for (i=0;i<3;i++) {
         VecCopy(N[i], Polygon.vertices[i].N)
         VecCopy(P[i], Polygon.vertices[i].U)
         }
      scan_convert(eye, Root, obj, NULL, &Polygon);
      }
}

void
MarchCubes(Viewpoint *eye, BinTree *Root,
           int u_steps, int v_steps, int w_steps,
           bbox_info *bound, Flt T,
           Flt (*evaluate)(Object *, Vec),
           int (*gradient)(Object *, Vec, Vec),
           Object *obj)
{
   int i, j, k, l;
   Flt u, v, w;
   Flt u_min, v_min, w_min;
   Flt delta_u, delta_v, delta_w;
   Vec V[8];
   Flt C[8];
   unsigned char b, f;
#if 0
   /* Enable this if the code below is enabled */
   int j0, k0;
   Vec E;
   Flt **level0, **level1, **levelt;
#endif

   u_min = bound->lower_left[0];
   v_min = bound->lower_left[1];
   w_min = bound->lower_left[2];

   delta_u = bound->lengths[0] / (Flt)u_steps;
   delta_v = bound->lengths[1] / (Flt)v_steps;
   delta_w = bound->lengths[2] / (Flt)w_steps;

#if 0
   /* Allocate two layers of the sampling cube */
   level0 = (Flt **)polyray_malloc((v_steps+1) * sizeof(Flt *));
   level1 = (Flt **)polyray_malloc((v_steps+1) * sizeof(Flt *));
   if (level0 == NULL || level1 == NULL)
      error("Out of memory");
   for (i=0;i<=v_steps;i++) {
      level0[i] = (Flt *)polyray_malloc((w_steps+1) * sizeof(Flt));
      level1[i] = (Flt *)polyray_malloc((w_steps+1) * sizeof(Flt));
      if (level0[i] == NULL || level1[i] == NULL)
         error("Out of memory");
      }

   /* Dump out polygons */
   for (i=0,u=u_min;i<u_steps;i++,u+=delta_u) {
if ((Check_Abort_Flag == 1) && _kbhit())
   longjmp(abort_environ, 1);
      /* Fill in the current layer of values */
      for (j=0,v=v_min;j<=v_steps;j++,v+=delta_v) {
         for (k=0,w=w_min;k<=w_steps;k++,w+=delta_w) {
            MakeVector(u, v, w, E);
            level1[j][k] = evaluate(obj, E);
            }
         }

      /* March cubes across the two layers */
      for (j=0,v=v_min;j<v_steps;j++,v+=delta_v)
         for (k=0,w=w_min;k<w_steps;k++,w+=delta_w) {
            /* Evaluate the function at each corner of
               the current cube */
            for (f=1,l=0,b=0;l<8;l++,f<<=1) {
               MakeVector(u-(l&1?0:delta_u),
                          v-(l&2?0:delta_v),
                          w-(l&4?0:delta_w),
                          V[l]);
               j0 = j + (l&2?1:0);
               k0 = k + (l&4?1:0);
               if (l&1)
                  C[l] = level1[j0][k0];
               else
                  C[l] = level0[j0][k0];
               if (C[l] < T) b |= f;
               }

            /* Given the cube & the threshold, generate
               polygons that split hot vertices from
               cold vertices */
            split_cubes(eye, Root, V, C, b, T, gradient, obj);
            }

      /* Push down the top level and make the bottom one available
         for reuse */
      levelt = level0;
      level0 = level1;
      level1 = levelt;
      }

   /* Deallocate the sampling cube memory */
   for (i=0;i<v_steps;i++) {
      polyray_free(level0[i]);
      polyray_free(level1[i]);
      }
   polyray_free(level0);
   polyray_free(level1);
#else
   /* Earlier code - seems to be faster, even though it
      appears to do twice as much evaluation.  Need to
      figure out why. */
   /* Dump out polygons */
   for (i=0,u=u_min;i<u_steps;i++,u+=delta_u)
      for (j=0,v=v_min;j<v_steps;j++,v+=delta_v)
         for (k=0,w=w_min;k<w_steps;k++,w+=delta_w) {
            /* Evaluate the function at each corner of
               the current cube */
            if (k==0) {
               /* Only evaluate all eight corners the first
                  time through. */
               for (f=1,l=0,b=0;l<8;l++,f<<=1) {
                  MakeVector(u+(l&1?delta_u:0.0),
                             v+(l&2?delta_v:0.0),
                             w+(l&4?delta_w:0.0),
                             V[l]);
                  /* Do the evaluation in object space */
                  C[l] = evaluate(obj, V[l]);
                  if (C[l] < T) b |= f;
                  }
               }
            else {
               /* After the first time we can just move
                  the values from the top row of vertices
                  to the bottom row.  Then calculate the
                  new top row. */
               b >>= 4;
               f = 0x10;
               for (l=0;l<4;l++) {
                  VecCopy(V[l+4], V[l]);
                  C[l] = C[l+4];
                  }
               for (l=4;l<8;l++,f<<=1) {
                  MakeVector(u+(l&1?delta_u:0.0),
                             v+(l&2?delta_v:0.0),
                             w+(l&4?delta_w:0.0),
                             V[l]);
                  /* Do the evaluation in object space */
                  C[l] = evaluate(obj, V[l]);
                  if (C[l] < T) b |= f;
                  }
               }
            /* Given the cube & the threshold, generate
               polygons that split hot vertices from
               cold vertices */
            split_cubes(eye, Root, V, C, b, T, gradient, obj);
            }
#endif

}
