#include "ccsg.h"

#ifdef CCSG_USE_CUSTOM_ALLOCATOR
#include "ccsg_memory.hpp"
#endif

#include "csg.hpp"

#include <assert.h>

//--------------------------------------------------------------------------------------------------
// C <---> C++ Pointer Cast Helpers
//--------------------------------------------------------------------------------------------------
#define FN(name) static auto name

FN(toC)(const csg::world_t *in) { assert(in); return reinterpret_cast<const CCSG_World *>(in); }
FN(toCpp)(const CCSG_World *in) { assert(in); return reinterpret_cast<const csg::world_t *>(in); }
FN(toC)(csg::world_t *in) { assert(in); return reinterpret_cast<CCSG_World *>(in); }
FN(toCpp)(CCSG_World *in) { assert(in); return reinterpret_cast<csg::world_t *>(in); }

FN(toC)(const csg::face_t *in) { assert(in); return reinterpret_cast<const CCSG_Face *>(in); }
FN(toCpp)(const CCSG_Face *in) { assert(in); return reinterpret_cast<const csg::face_t *>(in); }
FN(toC)(csg::face_t *in) { assert(in); return reinterpret_cast<CCSG_Face *>(in); }
FN(toCpp)(CCSG_Face *in) { assert(in); return reinterpret_cast<csg::face_t *>(in); }

FN(toC)(const csg::brush_t *in) { assert(in); return reinterpret_cast<const CCSG_Brush *>(in); }
FN(toCpp)(const CCSG_Brush *in) { assert(in); return reinterpret_cast<const csg::brush_t *>(in); }
FN(toC)(csg::brush_t *in) { assert(in); return reinterpret_cast<CCSG_Brush *>(in); }
FN(toCpp)(CCSG_Brush *in) { assert(in); return reinterpret_cast<csg::brush_t *>(in); }

FN(toC)(const csg::fragment_t *in) { assert(in); return reinterpret_cast<const CCSG_Fragment *>(in); }
FN(toCpp)(const CCSG_Fragment *in) { assert(in); return reinterpret_cast<const csg::fragment_t *>(in); }
FN(toC)(csg::fragment_t *in) { assert(in); return reinterpret_cast<CCSG_Fragment *>(in); }
FN(toCpp)(CCSG_Fragment *in) { assert(in); return reinterpret_cast<csg::fragment_t *>(in); }

#undef FN

#ifdef CCSG_USE_CUSTOM_ALLOCATOR
//--------------------------------------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------------------------------------
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
#else //CCSG_USE_CUSTOM_ALLOCATOR
void
CCSG_RegisterCustomAllocator(CCSG_AllocateFunction in_alloc,
                             CCSG_FreeFunction in_free,
                             CCSG_AlignedAllocateFunction in_aligned_alloc,
                             CCSG_AlignedFreeFunction in_aligned_free)
{ assert(!"Must define CCSG_USE_CUSTOM_ALLOCATOR to register custom allocator!"); }
#endif //CCSG_USE_CUSTOM_ALLOCATOR

//--------------------------------------------------------------------------------------------------
// CCSG_World Methods
//--------------------------------------------------------------------------------------------------
CCSG_World*
CCSG_World_Create()
{
    return toC(new csg::world_t());
}

void
CCSG_World_Destroy(CCSG_World *world)
{
    delete toCpp(world);
}

CCSG_Brush*
CCSG_World_First(CCSG_World *world)
{

}

CCSG_Brush*
CCSG_World_Next(CCSG_World *world, CCSG_Brush *brush)
{

}

void
CCSG_World_Remove(CCSG_World *world, CCSG_Brush *brush)
{

}

CCSG_Brush*
CCSG_World_Add(CCSG_World *world)
{

}

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