//----------------------------------------------------------------------------
// BP_Memory.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// TYPES
//----------------------------------------------------------------------------

// Memory type
// NOTE: If you add to this list, don't forget to update the array
//       "gBP_Memory_Types" in BP_Memory.cpp
typedef enum kMemoryType
{
   kMT_Permanent,    // Owner is responsible for freeing
   kMT_Resident,     // Gets free'd from GV_ResetResidentMemory
   kMT_Normal,       // Gets free'd from GV_ResetMemory
   kMT_DgPacket,     // Gets free'd when DG_ResizePacketMemory(0) is called

   kMT_Count,
   kMT_FirstValid = 0
} EMemoryType;

//----------------------------------------------------------------------------

// Memory sub category
// NOTE: If you add to this list, don't forget to update the array
//       "gBP_Memory_Categories" in BP_Memory.cpp
typedef enum kMemoryCategory
{
   kMC_GVHeap,                   // Game heap
   kMC_ScratchPad,               // Game scratch pad
   kMC_Slot,                     // FS slot
   kMC_Temp,                     // Temp storage

   // Sound related allocations.
   kMC_Sound,
   kMC_Movie,
   kMC_Stream,
   kMC_Font,

   // Rendering related allocations.
   kMC_Renderer,
   kMC_EndFrame,
   kMC_PShade,

   // MGS3 Specific ones below this point
   kMC_Geo_LoadedCollision,      // Geo fast poly loaded collision
   kMC_Geo_LoadedGroup,          // Geo fast poly loaded group
   kMC_Geo_DynamicCollision,     // Geo fast poly dynamic collision
   kMC_Geo_DynamicGeom,          // Geo fast poly dynamic geom

   kMC_Count,
   kMC_FirstValid = 0
} EMemoryCategory;

//----------------------------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------------------------

void        BP_Memory_InitDebugMenu    ( void );
void        BP_Memory_GetStats         ( EMemoryType type, EMemoryCategory category, int* allocCount, int* allocSize );
void        BP_Memory_GetStatsForCategory  
                                       ( EMemoryCategory category, int *pAllocCount, int *pAllocSize );
int         BP_Memory_GetMemoryUsageForCategory 
                                       ( EMemoryCategory category );
void        BP_Memory_ShowStats        ( void );
void        BP_Memory_SanityCheck      ( void );

void*       BP_Memory_Alloc            ( int size, int alignment, EMemoryType type, EMemoryCategory category );
void*       BP_Memory_Calloc           ( int size, int alignment, EMemoryType type, EMemoryCategory category );
void        BP_Memory_Free             ( void* data );   

void        BP_Memory_FreeType         ( EMemoryType type );

const char* BP_Memory_GetTypeName      ( EMemoryType type );
const char* BP_Memory_GetCategoryName  ( EMemoryCategory category );

int         BP_Memory_GetAllocSize     ( void* data );
void*       BP_Memory_GetHeadAlloc     ( EMemoryType type, EMemoryCategory category );
void*       BP_Memory_GetNextAlloc     ( void* data );

// Returns Permanent on unknown
enum kMemoryType 
            BP_Memory_Classify         ( void *data );

void        BP_Memory_SetDgPacketMemoryRange( void *start, int size );

//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//----------------------------------------------------------------------------
