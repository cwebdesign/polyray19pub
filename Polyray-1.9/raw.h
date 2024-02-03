#if !defined(__POLYRAY_RAW_DEFS)
#define __POLYRAY_RAW_DEFS

typedef struct {
   BinTree objs; /* Bounding hierarchy of raw triangles */
   float smooth; /* Maximum angle to smooth */
   } RawData;

#define MAXTRILINE 512

/* Data structure for a stack of vertices */
typedef struct VecVerts_struct VecVerts;
struct VecVerts_struct {
   float V[4];
   VecVerts *next;
   };

/* Data structure for a stack of faces */
typedef struct Face_struct Faces;
struct Face_struct {
   int vcount;
   long *verts, *tverts, *nverts;
   Texture *texture;
   Faces *next;
   };

typedef struct triverts_struct triverts;
struct triverts_struct {
   fVec V[3], N[3], U[3];
   Texture *texture;
   triverts *next;
   };

/* Stack of raw triangle collections.  Each collection is associated
   with a single texture name.  (currently unused) */
typedef struct trivertstack_struct trivstack;
struct trivertstack_struct {
   Texture *texture;   /* Texture to apply to all triangles */
   triverts *verts;    /* List of triangles */
   Faces *fstack;      /* Stack of face indices */
   long tcount;        /* Number of triangles in this raw object */
   int nflag;          /* Are there vertex normals? */
   int uvflag;         /* Is there u/v information for the vertices? */
   trivstack *next;    /* Next bag of triangles */
   };

extern Object *MakeRaw(Object *, char *, Flt);
extern Faces *new_face(int vcount, float *v);
extern VecVerts *new_vecvert(float x, float y, float z, float w);
extern trivstack *new_tristack(trivstack *tristack);

#endif /* __POLYRAY_RAW_DEFS */

