/** @file wavefrnt.cc
 *  @brief Wavefront OBJ mesh loader - parses geometry and converts to Polyray triangles.
 *
 *  Reads a Wavefront @c .obj file line by line, accumulating vertex positions
 *  (@c v), vertex normals (@c vn), texture coordinates (@c vt), material
 *  assignments (@c usemtl), and polygon faces (@c f) into linked-list stacks.
 *  When the file is fully consumed, make_triangles() fans each polygon into
 *  triangles using Split_Polygon() and attaches them to the scene Object.
 *
 *  @note The face parser (@c f lines) is functional but the vertex / normal /
 *        texture-coordinate parsers currently call @c exit(0) after printing
 *        diagnostics - the vertex reading path is still work-in-progress.
 *
 *  Polyray - MIT Licensed Revival
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
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <sstream>
#include <string>

#include "defs3.h"
#include "memory.h"
#include "io_ply.h"
#include "symtab.h"
#include "raw.h"
#include "subdiv.h"
#include "bound.h"
#include "wavefrnt.h"
#include "unixcompat.h"
#include "factory.h"


#define MAX_VERTICES_PER_FACE 16     /**< Maximum vertices allowed per OBJ face. */

static long vertex_count;         /**< Running count of @c v entries read. */
static long vertex_texture_count; /**< Running count of @c vt entries read. */
static long vertex_normal_count;  /**< Running count of @c vn entries read. */
static long face_count;           /**< Running count of @c f entries read. */
static Texture *current_texture;  /**< Most-recently set @c usemtl texture; NULL if none. */

std::string rbuf;          /**< Current line buffer read from the .obj file. */
static int rbuf_offset = 0; /**< Read position within @c rbuf. */
static int rbuf_length = 0; /**< Length of the current @c rbuf content. */

/** @brief Advance @c rbuf_offset past whitespace and OBJ line-continuation characters.
 *
 *  Handles the backslash-newline (@c \\) continuation by reading the next line
 *  from @p file into @c rbuf and resetting the offset.  Skips spaces and tabs
 *  in the current line.
 *  @param file  Open .obj file stream; advanced only on a continuation.
 *  @return      1 if more data remains on the current logical line; 0 on EOF/error.
 */
static int whitespace_skip(std::ifstream *file)
{
   while (rbuf_offset < rbuf_length) {
     if (rbuf[rbuf_offset] == '\\') {
         /* Continuation character, get the next line */
         if (std::getline(*file, rbuf)) {
            rbuf_offset = 0;
            rbuf_length = static_cast<int>(rbuf.length());
            }
         else {
            rbuf.clear();
            rbuf_offset = 0;
            rbuf_length = 0;
            return 0;
            }
         }
      else if (rbuf[rbuf_offset] == ' ' || rbuf[rbuf_offset] == '\t') {
         rbuf_offset++;
         }
      else
         break;
   }
   return 1;
}


/** @brief Try to parse a full @c v/vt/vn triplet from a face token.
 *  @param token  Face token text from the current @c f line.
 *  @param v      Output: position index.
 *  @param vt0    Output: texture coordinate index (valid when return >= 2).
 *  @param vn0    Output: normal index (valid when return == 3).
 *  @return       Fields consumed: 1 (@c v only), 2 (@c v/vt), or 3 (@c v/vt/vn).
 */
int handletris(const std::string& token, long &v, long &vt0, long &vn0);

/** @brief Compatibility overload preserving the historical stream-based signature.
 *  @param filep  Stream positioned at a face token.
 *  @param v      Output: position index.
 *  @param vt0    Output: texture coordinate index.
 *  @param vn0    Output: normal index.
 *  @return       Fields consumed: 1, 2, or 3.
 */
int handletris(std::ifstream *filep, long &v, float &vt0, float &vn0);

/** @brief Try to parse a @c v/vt or @c v//vn pair from a face token.
 *  @param token     Face token text from the current @c f line.
 *  @param oneortwo  1 = expect one slash (@c v/vt); 2 = expect two (@c v//vn).
 *  @param a         Output: position index.
 *  @param b         Output: secondary index (texture or normal depending on mode).
 *  @return          2 if the pair was read successfully; 1 if only @p a was consumed.
 */
int handleduo(const std::string& token, int oneortwo, long &a, long &b);

/** @brief Compatibility overload preserving the historical stream-based signature.
 *  @param filep     Stream positioned at a face token.
 *  @param oneortwo  1 = expect one slash; 2 = expect two slashes.
 *  @param a         Output: first parsed index.
 *  @param b         Output: second parsed index.
 *  @return          2 if both indices were consumed; 1 otherwise.
 */
int handleduo(std::ifstream *filep, int oneortwo, float &a, float &b);


/** @brief Parse one face-vertex reference from a face token into its three indices.
 *
 *  Tries each of the four OBJ face-vertex formats in order:
 *  -# @c v/vt/vn  - position, texture, and normal index
 *  -# @c v//vn    - position and normal only
 *  -# @c v/vt     - position and texture only
 *  -# @c v        - position only
 *
 *  Calls serror() if none of the formats match.
 *
 *  @param token  Face token text from the current @c f line.
 *  @param v      Output: 0-based vertex position index (converted from OBJ 1-based).
 *  @param vt     Output: 0-based texture coordinate index; 0 if absent.
 *  @param vn     Output: 0-based vertex normal index; 0 if absent.
 */
#ifndef TESTING
static
#endif
void read_vertex(const std::string& token, long& v, long& vt, long& vn)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wavefrnt::read_vertex\n";
   //openpolyray::polyray_pause();
   #endif
   long v0, vt0, vn0;

   if (handletris(token, v, vt0, vn0)==3) {
      vt = vt0;
      vn = vn0;
      return;
   }
   if (handleduo(token, 2, v0, vn0)==2) {
      v = v0;
      vn = vn0;
      vt = 0L;
      return;
   }
   if (handleduo(token, 1, v0, vt0)==2) {
      vn = 0L;
      v = v0;
      vt = vt0;
      return;
   }
   std::istringstream stream(token);
   if (stream >> v0) {
      v = v0;
      vt = 0L;
      vn = 0L;
      return;
   }

   serror("Bad vertex data\n");
}

/** @brief Try to parse a full @c v/vt/vn triplet from a face token (implementation).
 *
 *  Splits the token at slash boundaries and parses all three indices when the
 *  token matches the full @c v/vt/vn form.
 *  @param token  Face token text from the current @c f line.
 *  @param v      Output: position index as read from the stream.
 *  @param vt0    Output: texture coordinate index (set when return >= 2).
 *  @param vn0    Output: normal index (set when return == 3).
 *  @return       1, 2, or 3 indicating the number of @c / -separated fields parsed.
 */
int handletris(const std::string& token, long &v, long &vt0, long &vn0)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wavefrnt::handletris\n";
   #endif
   std::size_t slash1 = token.find('/');
   if (slash1 == std::string::npos)
      return std::istringstream(token) >> v ? 1 : 0;

   std::size_t slash2 = token.find('/', slash1 + 1);
   if (slash2 == std::string::npos) {
      std::istringstream vstream(token.substr(0, slash1));
      std::istringstream vtstream(token.substr(slash1 + 1));
      if ((vstream >> v) && (vtstream >> vt0))
         return 2;
      return 1;
   }

   if (slash2 == slash1 + 1)
      return 1;

   std::istringstream vstream(token.substr(0, slash1));
   std::istringstream vtstream(token.substr(slash1 + 1, slash2 - slash1 - 1));
   std::istringstream vnstream(token.substr(slash2 + 1));
   if ((vstream >> v) && (vtstream >> vt0) && (vnstream >> vn0))
      return 3;

   return 1;
}

int handletris(std::ifstream *filep, long &v, float &vt0, float &vn0)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wavefrnt::handletris\n";
   #endif
   *filep >> v;
   char c;
   if ((*filep >> c) && (c == '/')) {
      *filep >> vt0;
      char c;
      if ((*filep >> c) && (c == '/')) {
         *filep >> vn0;
         return 3;
      }
      return 2;
   }
   return 1;
}

/** @brief Try to parse a @c v/vt or @c v//vn pair from a face token (implementation).
 *
 *  Reads the token text directly from the current logical line instead of the
 *  file stream. This avoids losing sync after @c getline() has advanced the
 *  underlying stream to the next line.
 *
 *  @param token     Face token text from the current @c f line.
 *  @param oneortwo  1 = single-slash (@c v/vt) mode; 2 = double-slash (@c v//vn) mode.
 *  @param a         Output: first index (position).
 *  @param b         Output: second index (texture or normal); valid when return == 2.
 *  @return          2 if both indices were consumed; 1 if only @p a was read.
 */
int handleduo(const std::string& token, int oneortwo, long &a, long &b)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wavefrnt::handleduo\n";
   #endif
   std::size_t slash1 = token.find('/');
   if (slash1 == std::string::npos)
      return 1;

   std::size_t slash2 = token.find('/', slash1 + 1);
   if (oneortwo == 2) {
      if (slash2 != slash1 + 1)
         return 1;

      std::istringstream astream(token.substr(0, slash1));
      std::istringstream bstream(token.substr(slash2 + 1));
      if ((astream >> a) && (bstream >> b))
         return 2;
      return 1;
   }

   if (slash2 != std::string::npos)
      return 1;

   std::istringstream astream(token.substr(0, slash1));
   std::istringstream bstream(token.substr(slash1 + 1));
   if ((astream >> a) && (bstream >> b))
      return 2;

   return 1;
}

int handleduo(std::ifstream *filep, int oneortwo, float &a, float &b)
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wavefrnt::handleduo\n";
   #endif
   *filep >> a;
   char c;
   if ((*filep >> c) && (c == '/')) {
      if (oneortwo == 2) {
         if ((*filep >> c) && (c == '/')) {
            *filep >> b;
            return 2;
         }
      }
      *filep >> b;
      return 2;
   }

   return 1;
}

/** @brief Parse one OBJ @c f line and return a heap-allocated @c Faces record.
 *
 *  Reads up to @c MAX_VERTICES_PER_FACE vertex references from the current
 *  position in @c rbuf using read_vertex().  Allocates a @c Faces struct and
 *  its @c verts, @c tverts (if any UV indices were present), and @c nverts (if
 *  any normal indices were present) arrays via @c polyray_malloc.  Converts
 *  1-based OBJ indices to 0-based, handling negative (relative) indices.
 *
 *  Emits swarning() and returns @c nullptr if fewer than 3 vertices are found.
 *  Face-vertex tokens are parsed from the current contents of @c rbuf rather
 *  than the underlying file stream so the parser stays aligned with the line
 *  already fetched by @c getline().
 *  @return       Pointer to a newly allocated @c Faces struct, or @c nullptr on error.
 */
static Faces *read_face()
{
   #ifdef DEBUG_FN_CALLS
   std::cout<<"wave_frnt::read_face\n";
   //openpolyray::polyray_pause();
   #endif
   Faces *face;
   int i, vcount, vtp_flag, vnp_flag;
   long v[MAX_VERTICES_PER_FACE];
   long vt[MAX_VERTICES_PER_FACE];
   long vn[MAX_VERTICES_PER_FACE];
   std::istringstream line(rbuf.substr(rbuf_offset));
   std::string token;

   vtp_flag = 0;
   vnp_flag = 0;
   for (vcount=0;
        vcount<MAX_VERTICES_PER_FACE && (line >> token);
        vcount++) {
      read_vertex(token, v[vcount], vt[vcount], vn[vcount]);
      if (vt[vcount] != 0)
         vtp_flag = 1;
      if (vn[vcount] != 0)
         vnp_flag = 1;
      }
   #ifdef DEBUG
   std::cout<<"vcount="<<vcount<<"\n";
   #endif
   if (vcount == MAX_VERTICES_PER_FACE)
      swarning("Too many vertices in a face");
   else if (vcount < 3) {
      swarning("Too few vertices in a face");
      return nullptr;
      }

   face = (Faces*)polyray_malloc(sizeof(Faces));
   face->verts = (long*)polyray_malloc(vcount * sizeof(long));
   if (vtp_flag)
      face->tverts = (long*)polyray_malloc(vcount * sizeof(long));
   else
      face->tverts = NULL;
   if (vnp_flag)
      face->nverts = (long*)polyray_malloc(vcount * sizeof(long));
   else
      face->nverts = NULL;
   face->vcount = vcount;
   for (i=0;i<vcount;i++) {
      if (v[i] > 0)
         face->verts[i] = v[i] - 1;
      else
         face->verts[i] = vertex_count + v[i];
      if (vtp_flag) {
         if (vt[i] > 0)
            face->tverts[i] = vt[i] - 1;
         else
            face->tverts[i] = vertex_texture_count + vt[i];
         }
      if (vnp_flag) {
         if (vn[i] > 0)
            face->nverts[i] = vn[i] - 1;
         else
            face->nverts[i] = vertex_normal_count + vn[i];
         }
      }

   face->next = NULL;
   return face;
}

/** @brief Convert the accumulated vertex and face stacks into Polyray triangle objects.
 *
 *  1. Allocates and populates @c obj->o_vertices->V (positions) and @c N (normals)
 *     by draining @p vstack and @p nstack respectively via @c polyray_free.
 *  2. For each face in @p fstack, fans the polygon into triangles with
 *     Split_Polygon(), choosing the two projection axes from the face normal.
 *  3. Each valid triangle becomes a @c TriangleObject pushed onto the @c RawData
 *     object list; degenerate triangles (zero bounding box) are discarded.
 *  4. All @c Faces and temporary polygon/triangle index arrays are freed.
 *
 *  Calls serror() if @p vertex_count is zero or if the stacks are inconsistent.
 *
 *  @param obj           Scene Object receiving the generated triangles.
 *  @param vertex_count  Number of entries in @p vstack.
 *  @param normal_count  Number of entries in @p nstack; 0 if no normals were read.
 *  @param fstack        Linked list of @c Faces to tessellate (consumed and freed).
 *  @param vstack        Linked list of vertex positions (consumed and freed).
 *  @param nstack        Linked list of vertex normals (consumed and freed).
 */
static void
make_triangles(Object *obj, long vertex_count, long normal_count,
               Faces *fstack, VecVerts *vstack, VecVerts *nstack)
{
   RawData *raw = (RawData*)obj->o_data;
   fVec *V, *N;
   long tcnt, triangle_count;
   VecVerts *vtemp;
   Faces *ftemp1, *ftemp2;
   TriangleObject *tobj;
   bbox_info box;

   triangle_count = 0;
   if (vertex_count==0) {
      serror("error: vertex_count in wavefrnt::make_triangles should not be 0!");
   }

   /* Now we need to allocate space for the vertices and process
      the face stacks into a set of triangles */
   obj->o_vertices = (ObjectVertices *)polyray_malloc(sizeof(ObjectVertices));
   obj->o_vertices->n = vertex_count;
   V = (fVec *)polyray_malloc(vertex_count * sizeof(fVec));
   obj->o_vertices->V = V;
   if (normal_count > 0) {
      N = (fVec *)polyray_malloc(normal_count * sizeof(fVec));
      obj->o_vertices->N = N;
      }
   else {
      N = NULL;
      obj->o_vertices->N = NULL;
      }
   obj->o_vertices->U = NULL;


   /* Copy the vertices into the V array */
   for (tcnt=vertex_count-1;vstack!=NULL&&tcnt>=0;tcnt--) {
      /* Copy this vertex into the array */
      VecCopy(vstack->V, V[tcnt])
      /* Free up the space used for this vertex */
      vtemp = vstack;
      vstack = vstack->next;
      delete vtemp;
      }
   if (tcnt != -1 || vstack != NULL)
      serror("Didn't properly process .obj vertices");

   /* Copy the normals into the N array */
   for (tcnt=normal_count-1;nstack!=NULL&&tcnt>=0;tcnt--) {
      /* Copy this vertex into the array */
      VecCopy(nstack->V, N[tcnt])
      /* Free up the space used for this vertex */
      vtemp = nstack;
      nstack = nstack->next;
      delete vtemp;
      }
   if (tcnt != -1 || vstack != NULL)
      serror("Didn't properly process .obj vertices");

   /* Create triangles in the form we want them */
   for (ftemp1=fstack,tcnt=0;ftemp1!=NULL;tcnt++) {
      /* We need to turn the face into a set of triangles and
         stuff each one onto the stack */
      fVec V0, V1, Norm, *verts;
      Flt d;
      int i, j, out_n, npoints, **out_verts;

      /* Allocate temporary space to hold the polygon and
         subsequent triangles */
      npoints = ftemp1->vcount;
      verts = (fVec *)polyray_malloc(npoints * sizeof(fVec));
      out_verts = (int **)polyray_malloc((npoints - 2) * sizeof(int *));
      for (j=0;j<npoints-2;j++)
         out_verts[j] = (int *)polyray_malloc(3 * sizeof(int));
      /* Stuff the vertices of the polygon into the array
         verts for subsequent chopping. */
      for (j=0;j<npoints;j++)
         VecCopy(V[ftemp1->verts[j]], verts[j])

      /* Figure out what axes to use when splitting the polygon */
      VecSub(verts[1], verts[0], V0);
      VecSub(verts[2], verts[0], V1);
      VecCross(V1, V0, Norm);
      d = sqrt(VecDot(Norm, Norm));
      if (d < PLY_EPSILON)
         /* Degenerate triangle, ignore the error */
         d = 1.0;
      else
         d = 1.0 / d;
      VecScale(d, Norm);
      if (fabs(Norm[0]) >= fabs(Norm[1]) &&
          fabs(Norm[0]) >= fabs(Norm[2])) {
         i = 1;
         j = 2;
         }
      else if (fabs(Norm[1]) >= fabs(Norm[0]) &&
               fabs(Norm[1]) >= fabs(Norm[2])) {
         i = 0;
         j = 2;
         }
      else {
         i = 0;
         j = 1;
         }
      Split_Polygon(npoints, verts, i, j, out_n, out_verts);

      triangle_count += out_n;
      /* Add all the triangles to the list */
      for (j=0;j<out_n;++j) {
         tobj = FactoryTriangleObject();
         tobj->o_type = ShapeType::Polygon;
         tobj->o_texture = ftemp1->texture;;
         tobj->o_parent = obj;
         tobj->o_vert[0] = ftemp1->verts[out_verts[j][0]];
         tobj->o_vert[1] = ftemp1->verts[out_verts[j][1]];
         tobj->o_vert[2] = ftemp1->verts[out_verts[j][2]];
         if (ftemp1->nverts) {
            tobj->o_nvert[0] = ftemp1->nverts[out_verts[j][0]];
            tobj->o_nvert[1] = ftemp1->nverts[out_verts[j][1]];
            tobj->o_nvert[2] = ftemp1->nverts[out_verts[j][2]];
            }
         else {
            tobj->o_nvert[0] = -1;
            tobj->o_nvert[1] = -1;
            tobj->o_nvert[2] = -1;
            }

         if (calc_triangle_bounds(*tobj, &box)) {
            VecCopy(box.lower_left, tobj->o_bnd.lower_left)
            VecCopy(box.lengths, tobj->o_bnd.lengths)
            raw->objs.members.list = push_object(raw->objs.members.list,
                                                 (Object *)tobj);
            raw->objs.members.count++;
            }
         else
            /* Degenerate triangle, remove it. */
            delete tobj;
         }

      /* Clean up temporary memory */
      for (j=0;j<npoints-2;j++)
         polyray_free(out_verts[j]);
      polyray_free(out_verts);
      polyray_free(verts);

      /* Dispose of the ones we just looked at */
      ftemp2 = ftemp1;
      ftemp1 = ftemp1->next;
      polyray_free(ftemp2->verts);
      if (ftemp2->tverts) polyray_free(ftemp2->tverts);
      if (ftemp2->nverts) polyray_free(ftemp2->nverts);
      polyray_free(ftemp2);
      }

   sstatus("OBJ import summary: %ld vertices, %ld normals, %ld faces, %ld triangles\n",
           vertex_count, normal_count, face_count, triangle_count);
   if (raw->objs.members.count == 0)
      swarning("OBJ import produced no renderable triangles\n");
}

/** @brief Parse a Wavefront OBJ file and attach the resulting mesh to @p obj.
 *
 *  Reads the file from the beginning, dispatching on the first token of each
 *  line:
 *  - @c v   - vertex position (accumulated onto @c vstack)
 *  - @c vn  - vertex normal  (accumulated onto @c nstack)
 *  - @c vt  - texture coordinate (currently skipped after diagnostics)
 *  - @c usemtl - sets @c current_texture from the scene symbol table
 *  - @c f   - polygon face (parsed by read_face(), accumulated onto @c fstack)
 *
 *  After all lines are consumed, calls make_triangles() to tessellate the faces
 *  and attach them to @p obj.
 *
 *
 *  @param obj    Pre-allocated scene Object to receive the triangle mesh.
 *  @param filep  Open @c std::ifstream positioned at any point (rewound internally).
 *  @return       Total number of face (@c f) entries successfully parsed.
 */
int
Process_Obj_File(Object *obj, std::ifstream  *filep)
{
   char  tbuf1[MAXTRILINE], tbuf2[MAXTRILINE];
   float v0, v1, v2, v3;
   long lcnt;
   int icnt, ntype;
   int i, j;
   void *texptr;
   VecVerts *vstack, *nstack, *vtemp;
   Faces *fstack, *ftemp1;

   filep->seekg(0,std::ios_base::beg);
   vstack = NULL;
   nstack = NULL;
   fstack = NULL;
   vertex_count = 0;
   vertex_texture_count = 0;
   vertex_normal_count = 0;
   face_count = 0;
   current_texture = NULL;
   /* Read the entire file, processing triangles as we go. */
   for (lcnt=0;;lcnt++) {
      //std::cout<<"lcnt="<<lcnt<<"\n";
      auto foffset = filep->tellg();
      if (! std::getline(*filep, rbuf)) 
         break;
      /* First read in the command for this line */
      std::istringstream line(rbuf);
      std::string ctype;
      if (!(line >> ctype))
         continue;
      //openpolyray::polyray_pause();
      icnt = 1;
      rbuf_length = static_cast<int>(rbuf.length());
      rbuf_offset = static_cast<int>(ctype.length());
      whitespace_skip(filep);

      /* Looking for a statement like: "v x y z w" */
      if (ctype == "v") {
         /* Read a vertex */
         char *tmp = (char*)rbuf.c_str();
         icnt = sscanf(tmp, "v %g %g %g %g", &v0, &v1, &v2, &v3);

         if (icnt == 3 || icnt == 4) {
            /* Valid vertex */
            vtemp = new_vecvert(v0, v1, v2, (icnt == 3 ? 0.0f : v3));
            vtemp->next = vstack;
            vstack = vtemp;
            vertex_count++;
            }
         else
            swarning("Bad vertex");
         continue;
         }

      /* Looking for a statement like: "vn x y z" */
      if (ctype == "vn") {
         /* Read a vertex */
         char *tmp = (char*)rbuf.c_str();
         icnt = sscanf(tmp, "vn %g %g %g", &v0, &v1, &v2);
         if (icnt == 3) {
            /* Valid vertex */
            vtemp = new_vecvert(v0, v1, v2, 0.0 );
            vtemp->next = nstack;
            nstack = vtemp;
            vertex_normal_count++;
            }
         else
            swarning("Bad normal");
         continue;
         }

      /* Looking for a statement like: "vt u v w" */
      if (ctype == "vt") {
         /* For now we are ignoring texture coordinates */
         vertex_texture_count++;
         continue;
         }

      /* Look for: "usemtl texture_name" */
      if (ctype == "usemtl") {
         char *tmp = (char*)rbuf.c_str();
         icnt = sscanf(tmp, "%s %s", tbuf1, tbuf2);
         if (icnt == 2) {
            /* Got a texture name */
            for (i=0,j=strlen(tbuf2);i<j;i++)
               if (tbuf2[i] == '.')
                  tbuf2[i] = '_';
            Lookup_Definition(tbuf2, &ntype, &texptr);
            if (ntype != std::to_underlying(ShapeType::Texture)) {
               swarning("Texture '%s' undefined\n", tbuf2);
               current_texture = nullptr;
               }
            else
               current_texture = (Texture*)texptr;
            }
         else
            swarning("Bad texture (usemtl) name");
         continue;
         }

      if (ctype == "f") {
         /* Read a face */
         ftemp1 = read_face();
         if (ftemp1 != NULL) {
            ftemp1->texture = current_texture;
            ftemp1->next = fstack;
            fstack = ftemp1;
            face_count++;
            }
         else
            swarning("Bad face");
         continue;
         }
      }

   /* Turn the contents of the face stack into triangle objects.  This
      routine removes the memory associated with tristack. */
   make_triangles(obj, vertex_count, vertex_normal_count,
                  fstack, vstack, nstack);

   return face_count;
}
