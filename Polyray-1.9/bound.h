#if !defined(__POLYRAY_BOUNDS_DEFS)
#define __POLYRAY_BOUNDS_DEFS

typedef struct t_angle_bounds AngleBounds;
struct t_angle_bounds {
   fVec U, V;         /* Basis vectors relating an object to a light */
   float umin, umax;  /* Angular bounds along the U axis */
   float vmin, vmax;  /* Angular bounds along the V axis */
   };

/* This type overlays the standard object type - it is used to hold
   several primitive (or composite) objects in a single bounded
   container.  The type, bound, and parent components are the same
   size and offsets as in the Object data structure. */
typedef struct t_composite_object {
   unsigned short   o_type;    /* Holds the 'type' of the object */
   bbox_info        o_bnd;     /* Bounding box limits */
   Object          *o_parent;  /* Parent object */
   unsigned short   c_size;    /* Number of subobjects held */
   Object         **c_object;  /* Array of subobjects */
   AngleBounds     *c_lbnd;    /* Light/Eye angle bounds */
   } CompositeObject;

extern void get_bounds(Object *obj, bbox_info *box);
extern int calc_triangle_bounds(TriangleObject *tri_obj, bbox_info *box);
extern void recompute_bbox(bbox_info *, Transform *);
extern void recompute_inverse_bbox(bbox_info *, Transform *);
extern void bbox_intersect(bbox_info *, bbox_info *, bbox_info *);
extern void bbox_union(bbox_info *, bbox_info *, bbox_info *);
extern int determine_start(Vec P, Vec D,  Flt bounds[2][3], Flt *min, Flt *max);
extern int BoundIntersect(Viewpoint *, BinTree *, Ray *, Flt, Flt, Isect *);
extern void BuildBoundingSlabs(BinTree *);
extern void AddEyeObjects(Object *obj, objlistptr eyeprims);
extern void AddLightObjects(BinTree *Root);

extern void BuildAreaTestHarness(void);
extern void BuildAreaTableTestHarness(void);

#endif /* __POLYRAY_BOUNDS_DEFS */
