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

typedef struct CCSG_BrushSet  CCSG_BrushSet;
typedef struct CCSG_BrushVec  CCSG_BrushVec;
typedef struct CCSG_RayHitVec CCSG_RayHitVec;
typedef struct CCSG_FaceVec   CCSG_FaceVec;
typedef struct CCSG_PlaneVec  CCSG_PlaneVec;

//--------------------------------------------------------------------------------------------------
// Reinterpreted Types - Must maintain these in sync with csg types
//--------------------------------------------------------------------------------------------------
typedef int CCSG_Volume;
typedef CCSG_Volume (*CCSG_VolumeOperation)(CCSG_Volume);
typedef float CCSG_Vec3[3];
typedef float CCSG_Mat4[16];

typedef struct CCSG_Plane {
    CCSG_Vec3 normal;
    float offset;
} CCSG_Plane;

typedef struct CCSG_Ray {
    CCSG_Vec3 origin, direction;
} CCSG_Ray;

typedef struct CCSG_RayHit {
    CCSG_Brush *brush;
    CCSG_Face *face;
    CCSG_Fragment *fragment;
    float parameter;
    CCSG_Vec3 position;
} CCSG_RayHit;

typedef struct CCSG_Box {
    CCSG_Vec3 min, max;
} CCSG_Box;

typedef struct CCSG_Vertex {
    CCSG_Face *faces[3];
    CCSG_Vec3 position;
} CCSG_Vertex;

typedef struct CCSG_Triangle {
    int i, j, k;
} CCSG_Triangle;

//--------------------------------------------------------------------------------------------------
// Memory
//--------------------------------------------------------------------------------------------------
typedef void *(*CCSG_AllocateFunction)(size_t in_size); // Must be 16 byte aligned
typedef void (*CCSG_FreeFunction)(void *in_block);
typedef void *(*CCSG_AlignedAllocateFunction)(size_t in_size, size_t in_alignment);
typedef void (*CCSG_AlignedFreeFunction)(void *in_block);

void
CCSG_RegisterCustomAllocator(CCSG_AllocateFunction in_alloc,
                             CCSG_FreeFunction in_free,
                             CCSG_AlignedAllocateFunction in_aligned_alloc,
                             CCSG_AlignedFreeFunction in_aligned_free);

//--------------------------------------------------------------------------------------------------
// STL Container Methods
//--------------------------------------------------------------------------------------------------
void
CCSG_BrushSet_Destroy(CCSG_BrushSet *set);

void
CCSG_BrushVec_Destroy(CCSG_BrushVec *vec);

void
CCSG_RayHitVec_Destroy(CCSG_RayHitVec *vec);

void
CCSG_FaceVec_Destroy(CCSG_FaceVec *vec);

void
CCSG_PlaneVec_Destroy(CCSG_PlaneVec *vec);

//--------------------------------------------------------------------------------------------------
// CCSG_World
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

CCSG_BrushVec*
CCSG_World_QueryPoint(CCSG_World *world, const CCSG_Vec3 *point);

CCSG_BrushVec*
CCSG_World_QueryBox(CCSG_World *world, const CCSG_Box *box);

CCSG_RayHitVec*
CCSG_World_QueryRay(CCSG_World *world, const CCSG_Ray *ray);

CCSG_BrushVec*
CCSG_World_QueryFrustum(CCSG_World *world, const CCSG_Mat4 *view_projection);

void*
CCSG_WorldGetUserData(const CCSG_World *world);

void
CCSG_World_SetUserData(CCSG_World *world, void *user_data);

//--------------------------------------------------------------------------------------------------
// CCSG_Brush
//--------------------------------------------------------------------------------------------------
void
CCSG_Brush_SetPlanes(CCSG_Brush *brush, const CCSG_PlaneVec *planes);

const CCSG_PlaneVec*
CCSG_Brush_GetPlanes(const CCSG_Brush *brush);

void
CCSG_Brush_SetVolumeOperation(CCSG_Brush *brush, const CCSG_VolumeOperation *operation);

const CCSG_FaceVec*
CCSG_Brush_GetFaces(const CCSG_Brush *brush);

//--------------------------------------------------------------------------------------------------
// CCSG_Fragment
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// CCSG_Face
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif