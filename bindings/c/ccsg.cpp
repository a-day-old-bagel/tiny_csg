#include <assert.h>
#include <stddef.h>
#include "ccsg.h"

//--------------------------------------------------------------------------------------------------
// Custom Allocator
//--------------------------------------------------------------------------------------------------
#ifdef CSG_CUSTOM_ALLOCATOR_HEADER
#include "ccsg_memory.hpp"
CCSG::AllocateFunction CCSG::Allocate = nullptr;
CCSG::FreeFunction CCSG::Free = nullptr;
CCSG::AlignedAllocateFunction CCSG::AlignedAllocate = nullptr;
CCSG::AlignedFreeFunction CCSG::AlignedFree = nullptr;
void
CCSG_RegisterCustomAllocator(CCSG_AllocateFunction in_alloc,
                             CCSG_FreeFunction in_free,
                             CCSG_AlignedAllocateFunction in_aligned_alloc,
                             CCSG_AlignedFreeFunction in_aligned_free)
{
    CCSG::Allocate = in_alloc;
    CCSG::Free = in_free;
    CCSG::AlignedAllocate = in_aligned_alloc;
    CCSG::AlignedFree = in_aligned_free;
}
#else // CSG_CUSTOM_ALLOCATOR_HEADER
void
CCSG_RegisterCustomAllocator(CCSG_AllocateFunction in_alloc,
                             CCSG_FreeFunction in_free,
                             CCSG_AlignedAllocateFunction in_aligned_alloc,
                             CCSG_AlignedFreeFunction in_aligned_free)
{ assert(!"Must define CSG_CUSTOM_ALLOCATOR_HEADER to register custom allocator!"); }
#endif // CSG_CUSTOM_ALLOCATOR_HEADER

#include "csg.hpp"

//--------------------------------------------------------------------------------------------------
// Safety Checks
//--------------------------------------------------------------------------------------------------
#define SIZE_ASSERT(C_TYPE, CPP_TYPE) static_assert(sizeof(C_TYPE) == sizeof(CPP_TYPE));
#define LAYOUT_ASSERTS(C_TYPE, CPP_TYPE, MEMBER) \
    static_assert(sizeof(C_TYPE) == sizeof(CPP_TYPE)); \
    static_assert(offsetof(C_TYPE, MEMBER) == offsetof(CPP_TYPE, MEMBER));

static_assert(sizeof(glm::vec3) == 3 * sizeof(float));
static_assert(sizeof(glm::mat4) == 16 * sizeof(float));

SIZE_ASSERT(CCSG_Vec3, glm::vec3)
SIZE_ASSERT(CCSG_Mat4, glm::mat4)

LAYOUT_ASSERTS(CCSG_Plane, csg::plane_t, offset)
LAYOUT_ASSERTS(CCSG_Ray, csg::ray_t, direction)
LAYOUT_ASSERTS(CCSG_RayHit, csg::ray_hit_t, position)
LAYOUT_ASSERTS(CCSG_Box, csg::box_t, max)
LAYOUT_ASSERTS(CCSG_Vertex, csg::vertex_t, position)
LAYOUT_ASSERTS(CCSG_Triangle, csg::triangle_t, k)
#undef LAYOUT_ASSERTS
#undef SIZE_ASSERT

//--------------------------------------------------------------------------------------------------
// STL Container Aliases
//--------------------------------------------------------------------------------------------------
using BrushSet = csg::set_t<csg::brush_t*>;
using BrushVec = csg::vector_t<csg::brush_t*>;
using RayHitVec = csg::vector_t<csg::ray_hit_t>;
using FaceVec = csg::vector_t<csg::face_t>;
using PlaneVec = csg::vector_t<csg::plane_t>;

//--------------------------------------------------------------------------------------------------
// C <---> C++ Pointer Cast Helpers
//--------------------------------------------------------------------------------------------------
#define C_CPP_PTR_CONVERT(C_TYPE, CPP_TYPE) \
    static auto toC(const CPP_TYPE *in) { assert(in); return reinterpret_cast<const C_TYPE *>(in); } \
    static auto toCpp(const C_TYPE *in) { assert(in); return reinterpret_cast<const CPP_TYPE *>(in); } \
    static auto toC(CPP_TYPE *in) { assert(in); return reinterpret_cast<C_TYPE *>(in); } \
    static auto toCpp(C_TYPE *in) { assert(in); return reinterpret_cast<CPP_TYPE *>(in); } \

C_CPP_PTR_CONVERT(CCSG_World, csg::world_t)
C_CPP_PTR_CONVERT(CCSG_Face, csg::face_t)
C_CPP_PTR_CONVERT(CCSG_Brush, csg::brush_t)
C_CPP_PTR_CONVERT(CCSG_Fragment, csg::fragment_t)

C_CPP_PTR_CONVERT(CCSG_Plane, csg::plane_t)
C_CPP_PTR_CONVERT(CCSG_Ray, csg::ray_t)
C_CPP_PTR_CONVERT(CCSG_RayHit, csg::ray_hit_t)
C_CPP_PTR_CONVERT(CCSG_Box, csg::box_t)
C_CPP_PTR_CONVERT(CCSG_Vertex, csg::vertex_t)
C_CPP_PTR_CONVERT(CCSG_Triangle, csg::triangle_t)

C_CPP_PTR_CONVERT(CCSG_BrushSet, BrushSet)
C_CPP_PTR_CONVERT(CCSG_BrushVec, BrushVec)
C_CPP_PTR_CONVERT(CCSG_RayHitVec, RayHitVec)
C_CPP_PTR_CONVERT(CCSG_FaceVec, FaceVec)
C_CPP_PTR_CONVERT(CCSG_PlaneVec, PlaneVec)

C_CPP_PTR_CONVERT(CCSG_Vec3, glm::vec3)
C_CPP_PTR_CONVERT(CCSG_Mat4, glm::mat4)

#undef C_CPP_PTR_CONVERT

//--------------------------------------------------------------------------------------------------
// STL Container Methods
//--------------------------------------------------------------------------------------------------
void
CCSG_BrushSet_Destroy(CCSG_BrushSet *set) { delete toCpp(set); }

void
CCSG_BrushVec_Destroy(CCSG_BrushVec *vec) { delete toCpp(vec); }

void
CCSG_RayHitVec_Destroy(CCSG_RayHitVec *vec) { delete toCpp(vec); }

void
CCSG_FaceVec_Destroy(CCSG_FaceVec *vec) { delete toCpp(vec); }

void
CCSG_PlaneVec_Destroy(CCSG_PlaneVec *vec) { delete toCpp(vec); }

//--------------------------------------------------------------------------------------------------
// CCSG_World
//--------------------------------------------------------------------------------------------------
CCSG_World*
CCSG_World_Create() {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        assert(CCSG::Allocate && "Must register custom allocator first if CSG_CUSTOM_ALLOCATOR_HEADER is defined");
#   endif
    return toC(new csg::world_t());
}

void
CCSG_World_Destroy(CCSG_World *world) { delete toCpp(world); }

CCSG_Brush*
CCSG_World_First(CCSG_World *world) { return toC(toCpp(world)->first()); }

CCSG_Brush*
CCSG_World_Next(CCSG_World *world, CCSG_Brush *brush) { return toC(toCpp(world)->next(toCpp(brush))); }

void
CCSG_World_Remove(CCSG_World *world, CCSG_Brush *brush) { toCpp(world)->remove(toCpp(brush)); }

CCSG_Brush*
CCSG_World_Add(CCSG_World *world) { return toC(toCpp(world)->add()); }

CCSG_BrushSet*
CCSG_World_Rebuild(CCSG_World *world) {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        auto brush_set = static_cast<BrushSet*>(CCSG::Allocate(sizeof(BrushSet)));
            ::new (brush_set) BrushSet(toCpp(world)->rebuild());
#   else
        auto brush_set = new BrushSet(toCpp(world)->rebuild());
#   endif
    return toC(brush_set);
}

void
CCSG_World_SetVoidVolume(CCSG_World *world, CCSG_Volume void_volume) { toCpp(world)->set_void_volume(void_volume); }

CCSG_Volume
CCSG_World_GetVoidVolume(const CCSG_World *world) { return toCpp(world)->get_void_volume(); }

CCSG_BrushVec*
CCSG_World_QueryPoint(CCSG_World *world, const CCSG_Vec3 *point) {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        auto brush_vec = static_cast<BrushVec*>(CCSG::Allocate(sizeof(BrushVec)));
            ::new (brush_vec) BrushVec(toCpp(world)->query_point(*toCpp(point)));
#   else
        auto brush_vec = new BrushVec(toCpp(world)->query_point(*toCpp(point)));
#   endif
    return toC(brush_vec);
}

CCSG_BrushVec*
CCSG_World_QueryBox(CCSG_World *world, const CCSG_Box *box) {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        auto brush_vec = static_cast<BrushVec*>(CCSG::Allocate(sizeof(BrushVec)));
            ::new (brush_vec) BrushVec(toCpp(world)->query_box(*toCpp(box)));
#   else
        auto brush_vec = new BrushVec(toCpp(world)->query_box(*toCpp(box)));
#   endif
    return toC(brush_vec);
}

CCSG_RayHitVec*
CCSG_World_QueryRay(CCSG_World *world, const CCSG_Ray *ray) {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        auto ray_hit_vec = static_cast<RayHitVec*>(CCSG::Allocate(sizeof(RayHitVec)));
            ::new (ray_hit_vec) RayHitVec(toCpp(world)->query_ray(*toCpp(ray)));
#   else
        auto ray_hit_vec = new RayHitVec(toCpp(world)->query_ray(*toCpp(ray)));
#   endif
    return toC(ray_hit_vec);
}

CCSG_BrushVec*
CCSG_World_QueryFrustum(CCSG_World *world, const CCSG_Mat4 *view_projection) {
#   ifdef CSG_CUSTOM_ALLOCATOR_HEADER
        auto brush_vec = static_cast<BrushVec*>(CCSG::Allocate(sizeof(BrushVec)));
            ::new (brush_vec) BrushVec(toCpp(world)->query_frustum(*toCpp(view_projection)));
#   else
        auto brush_vec = new BrushVec(toCpp(world)->query_frustum(*toCpp(view_projection)));
#   endif
    return toC(brush_vec);
}

void*
CCSG_WorldGetUserData(const CCSG_World *world) { return std::any_cast<void*>(toCpp(world)->userdata); }

void
CCSG_World_SetUserData(CCSG_World *world, void *user_data) { toCpp(world)->userdata = std::make_any<void*>(user_data); }

//--------------------------------------------------------------------------------------------------
// CCSG_Brush
//--------------------------------------------------------------------------------------------------
const CCSG_PlaneVec*
CCSG_Brush_GetPlanes(const CCSG_Brush *brush) { return toC(&toCpp(brush)->get_planes()); }

//--------------------------------------------------------------------------------------------------
// CCSG_Fragment
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// CCSG_Face
//--------------------------------------------------------------------------------------------------