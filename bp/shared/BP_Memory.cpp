//----------------------------------------------------------------------------
// BP_Memory.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_Memory.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_RendererDebug.h"
#include "BP_Math.h"
#include "Engine/System/CSyncCriticalSection.h"

//----------------------------------------------------------------------------
// TYPES
//----------------------------------------------------------------------------

// Memory header which sits in front of returned allocation
struct SBP_Memory_Hdr
{
   SBP_Memory_Hdr*   prev;       // Previous allocation in list
   SBP_Memory_Hdr*   next;       // Next allocation in list
   void*             ptr;        // Pointer to platform allocated memory
   void*             data;       // Pointer to returned memory
   int               size;       // Size of allocation
   EMemoryType       type;       // Type of allocation
   EMemoryCategory   category;   // Category of allocation
};

#define BP_MEMORY_HDR_SIZE    sizeof(SBP_Memory_Hdr)

//----------------------------------------------------------------------------

// Memory tracking list
struct SBP_Memory_List
{
   SBP_Memory_Hdr*   head;       // head allocation
   int               allocCount; // number of allocations in list
   int               allocSize;  // total memory allocated in list
};

#if BP_VITA
#define MEMORY_STATS_X 230
#else
#define MEMORY_STATS_X 470
#endif

//----------------------------------------------------------------------------
// DATA
//----------------------------------------------------------------------------

static SBP_Memory_List  gBP_Memory_List[kMT_Count][kMC_Count] = {0};
static int              gBP_Memory_ShowStats = 0;
static char *                 sBP_Memory_DgStart = NULL;
static int                    sBP_Memory_DgSize = 0;
static CSyncCriticalSection   sBP_Memory_CriticalSection;

static const char*      gBP_Memory_Types[kMT_Count] = 
{ 
   "Permanent", 
   "Resident", 
   "Normal",
   "DgPacket"
};

static const char*      gBP_Memory_Categories[] = 
{ 
   "GV-Heap", 
   "Scratchpad", 
   "Slot", 
   "Temp", 
   "Sound",
   "Movie",
   "Stream",
   "Font",
   "Renderer",
   "EndFrame",
   "PShade", 
   "Geo-LoadedCollision", 
   "Geo-LoadedGroup", 
   "Geo-DynamicCollision", 
   "Geo-DynamicGeom" 
};

//----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//----------------------------------------------------------------------------

// Platform memory allocation
static 
void* BP_Memory_Platform_Alloc( int size, int alignment )  
{ 
#if defined(BP_PS3) || defined(BP_VITA)
   return memalign( alignment, size );
#else
   return _aligned_malloc( size, alignment );
#endif
}

//----------------------------------------------------------------------------

// Platform memory free
static 
void BP_Memory_Platform_Free( void* data )   
{ 
#if defined(BP_PS3) || defined(BP_VITA)
   free( data ); 
#else
   _aligned_free( data );
#endif
}

//----------------------------------------------------------------------------

// Returns associated linked list of memory type and category
static inline 
SBP_Memory_List& BP_Memory_GetList( EMemoryType type, EMemoryCategory category )
{
   BPE_ASSERT_NO_MSG( (type >= 0) && (type < kMT_Count) );
   BPE_ASSERT_NO_MSG( (category >= 0) && (category < kMC_Count) );

   SBP_Memory_List& list = gBP_Memory_List[type][category];
   return list;
}

//----------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//----------------------------------------------------------------------------

void  BP_Memory_InitDebugMenu( void )
{
#if BP_ENABLE_DEBUG_MENU
   // Make sure these arrays are in sync with the enums!
   BPE_ASSERT_NO_MSG( (sizeof(gBP_Memory_Types) / sizeof(gBP_Memory_Types[0])) == kMT_Count );
   BPE_ASSERT_NO_MSG( (sizeof(gBP_Memory_Categories) / sizeof(gBP_Memory_Categories[0])) == kMC_Count );

   // Create BP memory menu
   int const menu = BP_DebugMenu_AddMenu( "BP Memory", -1 );
   BP_DebugMenu_AddSeparator( menu );
   BP_DebugMenu_AddBool( menu, "Show Stats", &gBP_Memory_ShowStats );
#endif
}

//----------------------------------------------------------------------------

void BP_Memory_GetStats( EMemoryType type, EMemoryCategory category, int* allocCount, int* allocSize )
{
   // Get list stats
   SBP_Memory_List& list = BP_Memory_GetList( type, category );
   if( allocCount )
   {
      *allocCount = list.allocCount;
   }
   if( allocSize )
   {
      *allocSize = list.allocSize;
   }
}

//----------------------------------------------------------------------------

void BP_Memory_GetStatsForCategory( EMemoryCategory category, int *pAllocCount, int *pAllocSize )
{
   int totalAllocCount = 0;
   int totalAllocSize = 0;

   for ( int i = 0; i < kMT_Count; ++i )
   {
      SBP_Memory_List const &list = BP_Memory_GetList( EMemoryType(i), category );

      totalAllocCount += list.allocCount;
      totalAllocSize += list.allocSize;
   }

   if ( pAllocCount )
   {
      *pAllocCount = totalAllocCount;
   }

   if ( pAllocSize )
   {
      *pAllocSize = totalAllocSize;
   }
}

//----------------------------------------------------------------------------

int BP_Memory_GetMemoryUsageForCategory( EMemoryCategory category )
{
   int allocSize = 0;

   BP_Memory_GetStatsForCategory( category, NULL, &allocSize );
   return allocSize;
}

//----------------------------------------------------------------------------

void  BP_Memory_ShowStats( void )
{
   // Show all stats?
   if( gBP_Memory_ShowStats )
   {
      int currentX = MEMORY_STATS_X;
      int currentY = 40;

      // Loop over all types
      for( int type = 0; type < kMT_Count; type++ )
      {
         // Loop over all categories for type
         for( int category = 0; category < kMC_Count; category++ )
         {
            // Allocations present for type?
            SBP_Memory_List& list = BP_Memory_GetList( EMemoryType( type ), EMemoryCategory( category ) );
            if( list.allocCount )
            {
               // Show stats
               char info[256];
               sprintf( info, "%%(Type:%10s) (Category:%20s) (Allocs %4d) (Mem %7.1fK)", BP_Memory_GetTypeName(EMemoryType(type) ), BP_Memory_GetCategoryName(EMemoryCategory( category) ), list.allocCount, (float)list.allocSize/1024.0f );
               BP_Debug_DrawString(info, &currentX, &currentY, 0, CColor(255,255,255,255));
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_Memory_SanityCheck( void )
{
#ifdef BPE_DEBUG
   // Make sure these arrays are in sync with the enums!
   BPE_ASSERT_NO_MSG( (sizeof(gBP_Memory_Types) / sizeof(gBP_Memory_Types[0])) == kMT_Count );
   BPE_ASSERT_NO_MSG( (sizeof(gBP_Memory_Categories) / sizeof(gBP_Memory_Categories[0])) == kMC_Count );

   // Loop over all types
   for( int _type = 0; _type < kMT_Count; _type++ )
   {
      EMemoryType const type = EMemoryType( _type );
      // Loop over all categories for type
      for( int _category = 0; _category < kMC_Count; _category++ )
      {
         EMemoryCategory const category = EMemoryCategory( _category );
         // Check list
         int allocCount = 0;
         int allocSize  = 0;
         SBP_Memory_List& list = BP_Memory_GetList( type, category );
         
         // Check all allocations for type/category
         SBP_Memory_Hdr* hdr = list.head;
         while( hdr )
         {
            // Check header
            void* data = (char*)hdr + BP_MEMORY_HDR_SIZE;
            BPE_ASSERT_NO_MSG( hdr->type     == type );
            BPE_ASSERT_NO_MSG( hdr->category == category );
            BPE_ASSERT_NO_MSG( hdr->data     == data );

            // Check linked list ptrs
            if( list.head == hdr )
            {
               BPE_ASSERT_NO_MSG( hdr->prev == NULL );
            }
            if( hdr->next )
            {
               BPE_ASSERT_NO_MSG( hdr->next->prev == hdr );
            }
            if( hdr->prev )
            {
               BPE_ASSERT_NO_MSG( hdr->prev->next == hdr );
            }

            // Track list stats
            allocCount++;
            allocSize += hdr->size;

            // Next allocation
            hdr = hdr->next;
         }

         // Check list stats
         BPE_ASSERT_NO_MSG( list.allocCount == allocCount );
         BPE_ASSERT_NO_MSG( list.allocSize == allocSize );
      }
   }
#endif
}

//----------------------------------------------------------------------------

void *BP_Memory_Alloc( int size, int alignment, EMemoryType type, EMemoryCategory category )
{
   CSyncCriticalSectionLocker lock(&sBP_Memory_CriticalSection);

#ifdef BPE_DEBUG
   BP_Memory_SanityCheck();
#endif

   // Compute header size rounded up to requested allocation alignment 
   // so that it can sit right behind the returned data.
   int hdrSize = BP_MEMORY_HDR_SIZE;
   hdrSize += alignment-1;
   hdrSize &= ~(alignment-1);
   BPE_ASSERT_NO_MSG( hdrSize >= BP_MEMORY_HDR_SIZE );

   // Allocate hdr and requested memory
   char* ptr = (char*)BP_Memory_Platform_Alloc( hdrSize + size, alignment );

   // Compute data and hdr ptrs
   char* data = ptr + hdrSize;
   SBP_Memory_Hdr* hdr = (SBP_Memory_Hdr*)(data - BP_MEMORY_HDR_SIZE);

   // Make sure returned data is correctly aligned
   BPE_ASSERT_NO_MSG( ((int)data & (alignment-1)) == 0);

   // Setup hdr
   hdr->ptr  = ptr;
   hdr->data = data;
   hdr->size = size;
   hdr->type = type;
   hdr->category = category;

   // Add to head of associated linked list
   SBP_Memory_List& list = BP_Memory_GetList( type, category );
   hdr->prev = NULL;
   hdr->next = list.head;
   if( list.head )
   {
      list.head->prev = hdr;
   }
   list.head = hdr;

   // Update list stats
   list.allocCount++;
   list.allocSize += size;

#ifdef BPE_DEBUG
   BP_Memory_SanityCheck();
#endif

   return data;
}

//----------------------------------------------------------------------------

void *BP_Memory_Calloc( int size, int alignment, EMemoryType type, EMemoryCategory category )
{
   // Allocate and clear data
   void* pData = BP_Memory_Alloc( size, alignment, type, category );
   if( pData )
   {
      memset( pData, 0, size );
   }
   return pData;
}

//----------------------------------------------------------------------------

void  BP_Memory_Free( void* data )   
{
   // Handle NULL
   if(!data)
   {
      return;
   }

   CSyncCriticalSectionLocker lock(&sBP_Memory_CriticalSection);

#ifdef BPE_DEBUG
   BP_Memory_SanityCheck();
#endif

   // Get header
   SBP_Memory_Hdr* hdr = (SBP_Memory_Hdr*)((char*)data - BP_MEMORY_HDR_SIZE);
   BPE_ASSERT_NO_MSG( hdr->data == data );

#ifdef BPE_DEBUG
   // Trash memory to help catch any alloc/free bugs
   float* dst = (float*)data;
   for( int i = 0; i < (hdr->size/4); i++ )
   {
      BP_Float_SetInvalid( &dst[i] );
   }
#endif

   // Remove from associated linked list
   SBP_Memory_List& list = BP_Memory_GetList( hdr->type, hdr->category );
   if( list.head == hdr )
   {
      list.head = hdr->next;
   }      
   if( hdr->prev )
   {
      hdr->prev->next = hdr->next;
   }
   if( hdr->next )
   {
      hdr->next->prev = hdr->prev;
   }

   // Update list stats
   list.allocCount--;
   list.allocSize -= hdr->size;

   // Free
   BP_Memory_Platform_Free( hdr->ptr );

#ifdef BPE_DEBUG
   BP_Memory_SanityCheck();
#endif
}

//----------------------------------------------------------------------------

void  BP_Memory_FreeType( EMemoryType type )
{
   CSyncCriticalSectionLocker lock(&sBP_Memory_CriticalSection);

   // Loop over all categories for type
   for( int category = 0; category < kMC_Count; category++ )
   {
      // Free all allocations in list
      SBP_Memory_List& list = BP_Memory_GetList( type, EMemoryCategory( category ) );
      while( list.head )
      {
         BPE_ASSERT_NO_MSG( list.head->type == type );
         BPE_ASSERT_NO_MSG( list.head->category == category );
         BP_Memory_Free( list.head->data );
      }
   }
}

//----------------------------------------------------------------------------

const char* BP_Memory_GetTypeName( EMemoryType type )
{
   BPE_ASSERT_NO_MSG( (type >= 0) && (type < kMT_Count) );
   return gBP_Memory_Types[type];
}

//----------------------------------------------------------------------------

const char* BP_Memory_GetCategoryName( EMemoryCategory category )
{
   BPE_ASSERT_NO_MSG( (category >= 0) && (category < kMC_Count) );
   return gBP_Memory_Categories[category];
}

//----------------------------------------------------------------------------

int BP_Memory_GetAllocSize( void* data )
{
   // Get header
   SBP_Memory_Hdr* hdr = (SBP_Memory_Hdr*)((char*)data - BP_MEMORY_HDR_SIZE);
   BPE_ASSERT_NO_MSG( hdr->data == data );

   return hdr->size;
}

//----------------------------------------------------------------------------

void* BP_Memory_GetHeadAlloc( EMemoryType type, EMemoryCategory category )
{
   SBP_Memory_List& list = BP_Memory_GetList( type, category );
   if( list.head )
   {
      return list.head->data;
   }
   else
   {
      return NULL;
   }
}

//----------------------------------------------------------------------------

void* BP_Memory_GetNextAlloc( void* data )
{
   // Get header
   SBP_Memory_Hdr* hdr = (SBP_Memory_Hdr*)((char*)data - BP_MEMORY_HDR_SIZE);
   BPE_ASSERT_NO_MSG( hdr->data == data );

   // Return next alloc
   if( hdr->next )
   {
      return hdr->next->data;
   }
   else
   {
      return NULL;
   }
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
enum kMemoryType BP_Memory_Classify( void *data )
{
   char *pcData = reinterpret_cast<char *>( data );

   if ( pcData >= RESIDENT_ADDR && pcData < RESIDENT_BOTTOM )
   {
      return kMT_Resident;
   }
   else if ( pcData >= MEM_ADDR && pcData < MEM_BOTTOM )
   {
      return kMT_Normal;
   }
   else if ( sBP_Memory_DgStart != NULL && pcData >= sBP_Memory_DgStart && pcData < sBP_Memory_DgStart + sBP_Memory_DgSize )
   {
      return kMT_DgPacket;
   }
   else
   {
      return kMT_Permanent;
   }
}
#endif

//----------------------------------------------------------------------------

void BP_Memory_SetDgPacketMemoryRange( void *start, int size )
{
   sBP_Memory_DgStart = reinterpret_cast<char *>( start );
   sBP_Memory_DgSize = size;
}

