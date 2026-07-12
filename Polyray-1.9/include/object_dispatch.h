#pragma once

namespace openpolyray::dispatch {

/** @brief Virtual function table for a scene primitive.
 *
 *  Every primitive type sets exactly one @c ObjectProcs and stores a pointer
 *  to it in @c t_object::o_procs.  Generic fallbacks (GenericRender, GenericCopy,
 *  GenericDelete, GenericInitialize) are used when a primitive does not need
 *  a custom implementation.
 */
struct t_objectprocs {
   /** @brief Tessellate the object into polygons and render via scan conversion.
    *  @param eye   Active viewpoint.
    *  @param root  Scene BVH.
    *  @param obj   This object.
    */
   void (*render)(Viewpoint *, BinTree *, Object *) = nullptr;

   /** @brief Evaluate object-space position, normal and UV at parametric (u,v).
    *  @param obj   This object.
    *  @param u     Horizontal parametric coordinate [0,1].
    *  @param v     Vertical parametric coordinate [0,1].
    *  @param vert  Output vertex receiving position, normal, UV, and world position.
    */
   void (*evaluate)(Object *, Flt, Flt, Vertex *) = nullptr;

   /** @brief Perform any one-time post-creation initialisation.
    *  @param obj  Newly constructed object.
    *  @return     Non-zero on success.
    */
   int  (*initialize)(Object *) = nullptr;

   /** @brief Find all ray-object intersections within [mindist, maxdist].
    *  @param Eye      Active viewpoint.
    *  @param obj      This object.
    *  @param ray      Incoming ray.
    *  @param mindist  Near clip distance.
    *  @param maxdist  Far clip distance.
    *  @param hit      Output intersection accumulator.
    *  @return         Non-zero when at least one hit was recorded.
    */
   int  (*intersect)(Viewpoint *Eye, Object *, Ray *, Flt, Flt, Isect *) = nullptr;

   /** @brief Test whether a point is inside the object's volume.
    *  @param obj  This object.
    *  @param P    Point in world space.
    *  @return     Non-zero when the point is inside.
    */
   int  (*inside)(Object *, Vec) = nullptr;

   /** @brief Deep-copy object-specific data from @p src to @p dst.
    *  @param src  Source object.
    *  @param dst  Destination object.
    */
   void (*copy)(Object *, Object *) = nullptr;

   /** @brief Free object-specific data owned by @p obj. */
   void (*del)(Object *) = nullptr;
   };
  
}//namespace


/** @brief Common header shared by all object types (including TriangleObject).
 *
 *  Kept POD-like so that @c t_object and @c TriangleObject can alias the
 *  same memory layout up to this point.
 */
struct t_base {
	unsigned short  o_type;    /**< Primitive type tag (T_SPHERE, T_BOX, ...). */
	bbox_info       o_bnd;     /**< Axis-aligned bounding box. */
	Object* o_parent;  /**< Owning/parent object, or nullptr. */
	Texture* o_texture; /**< Texture applied to this object. */
	Transform* o_trans;   /**< Accumulated world-space transform (lazily allocated). */
};
/** @brief Full scene primitive object (extends t_base with ray-tracing data). */
struct t_object : t_base {
	openpolyray::dispatch::ObjectProcs* o_procs;    /**< Virtual function table for this primitive type. */
	float                         o_dither;   /**< Per-object stochastic dither amount. */
	unsigned short                o_copy;     /**< Non-zero when this is a shared copy (data not owned). */
	unsigned short                o_sflag;    /**< Shading flags (SHADOW_CHECK, REFLECT_CHECK, ...). */
	csgnode* o_csg_tree; /**< CSG operation tree rooted at this object, or nullptr. */
	std::array<unsigned short, 3>  o_uv_steps; /**< Tessellation resolution along U, V, and W. */
	std::array<float, 4>           o_uv_bounds;/**< UV parameter bounds: [u_min, u_max, v_min, v_max]. */
	NODE_PTR                      o_displace; /**< Surface displacement expression, or nullptr. */
	void* o_data;     /**< Type-specific shape data (owned unless o_copy is set). */
	ObjectVertices* o_vertices; /**< Pre-computed polygon mesh data, or nullptr. */
};

/** @brief Lightweight triangle primitive that overlays the t_base header.
 *
 *  Shares the @c t_base memory layout with @c t_object so a @c TriangleObject*
 *  can be cast to @c Object* safely up to the @c t_base fields.
 *  The @c o_parent field is used to find the owning object for texturing.
 */
struct TriangleObject : t_base {
	std::array<long, 3> o_vert;  /**< Indices into the parent object's vertex array. */
	std::array<long, 3> o_nvert; /**< Indices into the parent object's normal array. */
};
