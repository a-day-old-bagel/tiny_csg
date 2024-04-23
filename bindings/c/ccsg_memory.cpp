#include "ccsg_memory.hpp"

namespace CCSG {

//    // must be at least 8 byte aligned on 32 bit platform and 16 byte aligned on 64 bit platform
//    using AllocateFunction = void *(*)(size_t inSize);
//    using FreeFunction = void (*)(void *inBlock);
//    using AlignedAllocateFunction = void *(*)(size_t inSize, size_t inAlignment);
//    using AlignedFreeFunction = void (*)(void *inBlock);

AllocateFunction Allocate = nullptr;
FreeFunction Free = nullptr;
AlignedAllocateFunction AlignedAllocate = nullptr;
AlignedFreeFunction AlignedFree = nullptr;

}