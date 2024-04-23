#include <assert.h>
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
//C_CPP_PTR_CONVERT(CCSG_BrushSet, csg::fragment_t)

#undef C_CPP_PTR_CONVERT

//--------------------------------------------------------------------------------------------------
// CCSG_World Methods
//--------------------------------------------------------------------------------------------------
CCSG_World*
CCSG_World_Create() { return toC(new csg::world_t()); }

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
CCSG_World_Rebuild(CCSG_World *world)
{

}

void
CCSG_World_SetVoidVolume(CCSG_World *world, CCSG_Volume void_volume)
{

}

CCSG_Volume
CCSG_World_GetVoidVolume(const CCSG_World *world)
{

}

void*
CCSG_WorldGetUserData(const CCSG_World *world)
{

}

void
CCSG_World_SetUserData(CCSG_World *world, void *user_data)
{

}