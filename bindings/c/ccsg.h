#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
// Opaque Types
//--------------------------------------------------------------------------------------------------
typedef struct CCSG_World    CCSG_World;
typedef struct CCSG_Face     CCSG_Face;
typedef struct CCSG_Brush    CCSG_Brush;
typedef struct CCSG_Fragment CCSG_Fragment;

typedef struct CCSG_BrushSet CCSG_BrushSet;

//--------------------------------------------------------------------------------------------------
// Types
//--------------------------------------------------------------------------------------------------
typedef int32_t CCSG_Volume;
typedef CCSG_Volume (*CCSG_VolumeOperation)(CCSG_Volume);

// Must be 16 byte aligned
typedef void *(*CCSG_AllocateFunction)(size_t in_size);
typedef void (*CCSG_FreeFunction)(void *in_block);
typedef void *(*CCSG_AlignedAllocateFunction)(size_t in_size, size_t in_alignment);
typedef void (*CCSG_AlignedFreeFunction)(void *in_block);

//--------------------------------------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------------------------------------
void
CCSG_RegisterCustomAllocator(CCSG_AllocateFunction in_alloc,
                             CCSG_FreeFunction in_free,
                             CCSG_AlignedAllocateFunction in_aligned_alloc,
                             CCSG_AlignedFreeFunction in_aligned_free);

//--------------------------------------------------------------------------------------------------
// CCSG_World Methods
//--------------------------------------------------------------------------------------------------
CCSG_World*
CCSG_World_Create();

void
CCSG_World_Destroy(CCSG_World *world);

CCSG_Brush*
CCSG_World_First(CCSG_World *world);

CCSG_Brush*
CCSG_World_Next(CCSG_World *world, CCSG_Brush *brush);

void
CCSG_World_Remove(CCSG_World *world, CCSG_Brush *brush);

CCSG_Brush*
CCSG_World_Add(CCSG_World *world);

CCSG_BrushSet*
CCSG_World_Rebuild(CCSG_World *world);

void
CCSG_World_SetVoidVolume(CCSG_World *world, CCSG_Volume void_volume);

CCSG_Volume
CCSG_World_GetVoidVolume(const CCSG_World *world);

void*
CCSG_WorldGetUserData(const CCSG_World *world);

void
CCSG_World_SetUserData(CCSG_World *world, void *user_data);

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif