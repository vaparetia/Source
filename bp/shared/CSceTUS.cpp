//----------------------------------------------------------------------------
// CSceTUS.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/COsContext.h"

#include "BP_FileSupport.h"
#include "BP_EndianSupport.h"
#include "BP_Network.h"
#include "ITUSInterface.h"
#include "CSceTUS.h"

#include <stdio.h>
#include "Engine/System/CSyncEvent.h"

#include "BP_TUS.h"

#if BP_VITA
#  include "Engine/System/VTAThreadPriorities.h"
#include <libhttp/libhttp_error.h>
#include "TransfarringVTA.h"
#endif

#if BP_PS3
#include "Transfarring_PS3.h"
#endif

#define TUS_DEBUG
#ifdef TUS_DEBUG
#  define tus_log_printf printf
#else
#  define tus_log_printf(x,...) (0)
#endif

namespace
{
#if BP_VITA
   int const skThreadPriority = NVtaThreadPriorities::kPriorityTUS;
#endif

#if BP_PS3
   int const skThreadPriority = 1001;

   static inline int sceNpTusCreateRequest( int32_t titleCtxId )
   {
      return sceNpTusCreateTransactionCtx( titleCtxId );
   }

   static inline int sceNpTusDeleteRequest( int32_t requestId )
   {
      return sceNpTusDestroyTransactionCtx( requestId );
   }

#  define SCE_OK (0)
#endif
}

// Debug menu
extern int gBP_MakeBadCloudData;
extern int gBP_Transfarring_FailDeleteSlotData;
extern int gBP_Transfarring_FailGetSlotData;
extern int gBP_Transfarring_FailSetSlotData;
extern int gBP_Transfarring_FailGetSlotInfos;
extern int gBP_Transfarring_FailIncrementSlotVariable;

static void debug_menu_value_to_error( STransfarringError * const pError, int val )
{
   /*
   static char const* errorTUS[] = 
   { 
   "No Override", 
   "Authentication Error", 
   "No Such Title",
   "Service Not Started",
   "Service Ended",
   "Service Maintenance",
   };*/

   int adjustedVal = 0;

   switch( val )
   {
   case 1:
      adjustedVal = SCE_NP_COMMUNITY_ERROR_INVALID_SIGNATURE;
      break;
   case 2:
      adjustedVal = SCE_NP_COMMUNITY_SERVER_ERROR_NO_SUCH_TITLE;
      break;
   case 3:
      adjustedVal = SCE_NP_COMMUNITY_SERVER_ERROR_TITLE_USER_STORAGE_BEFORE_SERVICE;
      break;
   case 4:
      adjustedVal = SCE_NP_COMMUNITY_SERVER_ERROR_TITLE_USER_STORAGE_END_OF_SERVICE;
      break;
   case 5:
      adjustedVal = SCE_NP_COMMUNITY_SERVER_ERROR_TITLE_USER_STORAGE_MAINTENANCE;
      break;
   }

   SET_TRANSFARRING_ERROR( *pError, kTE_Unknown, adjustedVal );
}

CSceTUS::CSceTUS()
: mTitleContext( -1 )
, mLastRequestID( -1 )
{
   // NOTE - CTOR CALLED IN MAIN THREAD
   memset( &mTitleContextCachedID, 0, sizeof( mTitleContextCachedID ) );

#if BP_VITA
   BPE_CHECK_SCE( sceNpTusInit( skThreadPriority, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL ) );
#elif BP_PS3
   BPE_CHECK_SCE( sceNpTusInit( skThreadPriority ) );
#else
#  error Unsupported platform
#endif
}

CSceTUS::~CSceTUS()
{
}

bool CSceTUS::EnsureValidTitleContext( STransfarringError * const pError )
{
   SceNpId const *pLoggedInId = BP_Network_GetNPID();

   // First, early out if our cached credentials are still valid
   if ( mTitleContext > 0 && pLoggedInId )
   {
      if ( memcmp( pLoggedInId, &mTitleContextCachedID, sizeof( mTitleContextCachedID ) ) == 0 )
      {
         // Cached ID is the same
         return true;
      }
   }


   // Now if we're here, this means that we need a new title context

   // Destroy the old one
   if ( mTitleContext > 0 )
   {
      sceNpTusDestroyTitleCtx( mTitleContext );
      mTitleContext = -1;
   }

   // Make a new one
   if ( pLoggedInId )
   {
      SceNpCommunicationConfig const *config = BP_Network_GetTUSNPConfig();

      // Cache our user ID
      memcpy( &mTitleContextCachedID, pLoggedInId, sizeof( mTitleContextCachedID ) );
      mTitleContext = BPE_CHECK_SCE( sceNpTusCreateTitleCtx( config->commId, config->commPassphrase, pLoggedInId ) );
      tus_log_printf( "TUS: Title context: %d\n", mTitleContext );
   
      if ( mTitleContext < 0 && pError )
      {
         SET_TRANSFARRING_ERROR( *pError, kTE_Unknown, mTitleContext );
      }
   }
   else
   {
      if ( pError )
      {
         SET_TRANSFARRING_ERROR( *pError, kTE_Offline, 0 );
      }
   }

   return mTitleContext > 0;
}

int CSceTUS::AllocRequestID()
{
   // If we're allocating a new request, we're done with the prior one.
   // We only allow one request at a time.

   if ( mLastRequestID > 0 )
   {
      BPE_CHECK_SCE( sceNpTusDeleteRequest( mLastRequestID ) );
   }

   if ( EnsureValidTitleContext(NULL) )
   {
      mLastRequestID = BPE_CHECK_SCE( sceNpTusCreateRequest( mTitleContext ) );
      tus_log_printf( "TUS: Created request: ID %d\n", mLastRequestID );
   }
   else
   {
      mLastRequestID = SCE_NP_COMMUNITY_ERROR_INVALID_ID;
      tus_log_printf( "TUS: Couldn't create request: %d\n", mLastRequestID );
   }

   return mLastRequestID;
}

STransfarringError CSceTUS::DeleteSlotData( int const slotIndex )
{
   STransfarringError error;
   if ( !EnsureValidTitleContext(&error) )
   {
      return error;
   }
   else if( gBP_Transfarring_FailDeleteSlotData )
   {
      debug_menu_value_to_error( &error, gBP_Transfarring_FailDeleteSlotData );
      return error;
   }

   SceNpTusSlotId deletingSlots[] = { slotIndex };

   int result = sceNpTusDeleteMultiSlotData( 
      AllocRequestID(), 
      BP_Network_GetNPID(), // NP ID
      deletingSlots, 
      BPE_ARRAY_SIZE( deletingSlots ),
      NULL );

   SET_TRANSFARRING_ERROR(error, ( result == SCE_OK ) ? kTE_Success : kTE_Unknown, result);
   
   return error;
}

STransfarringError CSceTUS::SetSlotData( int const slotIndex, void const *pBuffer, size_t const size, STUSSlotInfo const &pSlotInfo )
{
   STransfarringError error;
   if ( !EnsureValidTitleContext(&error) )
   {
      return error;
   }
   else if( gBP_Transfarring_FailSetSlotData )
   {
      debug_menu_value_to_error( &error, gBP_Transfarring_FailSetSlotData );
      return error;
   }

   // Copy our accessory data into the TUS info
   SceNpTusDataInfo info = { sizeof( pSlotInfo.mAccessoryData ) };
   if( !gBP_MakeBadCloudData )
   {
      memcpy( info.data, pSlotInfo.mAccessoryData, sizeof( pSlotInfo.mAccessoryData ) );
   }
   else
   {
      SCloudAccessoryData badAccessoryData;
      badAccessoryData.mCookie = 0xFFFFFFFF;
      badAccessoryData.mRegion = 0xAAAAAAAA;
      badAccessoryData.mSaveFile.mSaveIndex = 666;
      badAccessoryData.mSaveFile.mSaveType = kST_MaxSaveTypes;
      badAccessoryData.mSaveFile.mDifficulty = 88;
      badAccessoryData.mSaveFile.mSaveDate.mDay = 99;
      badAccessoryData.mSaveFile.mSaveDate.mMonth = 55;
      badAccessoryData.mSaveFile.mSaveDate.mYear = 1776;
      badAccessoryData.mSaveFile.mStageIndex = 0xBBBBBBBB;
      badAccessoryData.mSaveFile.mPlayTime = 0xCCCCCCCC;
      memset( badAccessoryData.mSaveDataHash, 0xEEEEEEEE, sizeof(badAccessoryData.mSaveDataHash) );
      memset( badAccessoryData.mSaveFile.transfarringID, 0xDDDDDDDD, sizeof(badAccessoryData.mSaveFile.transfarringID) );
      memset( badAccessoryData.mSaveFile.cloudVersionHash, 0x66666666, sizeof(badAccessoryData.mSaveFile.cloudVersionHash) );
      memset( badAccessoryData.mSaveFile.psnAccountHash, 0x44444444, sizeof(badAccessoryData.mSaveFile.psnAccountHash) );
      badAccessoryData.mSaveFile.mClearCount = 77;
      badAccessoryData = CTransfarringManager::Instance()->EndianSwapAccessoryData( badAccessoryData );

      memcpy( info.data, &badAccessoryData, sizeof( badAccessoryData ) );
   }

   int result = sceNpTusSetData( 
      AllocRequestID(),
      BP_Network_GetNPID(), // NP ID
      slotIndex,
      size,
      size,
      pBuffer,
      &info,
      sizeof( info ),
#if BP_VITA
      NULL, // last changed author
      NULL, // last changed date
#endif
      NULL );

   tus_log_printf( "TUS: SetSlotData: np:%8.8x s:%d i:%8.8x si:%d r:%d\n",
      BP_Network_GetNPID(), 
      slotIndex,
      pBuffer,
      size,
      result );

   SET_TRANSFARRING_ERROR(error, ( result >= 0 ) ? kTE_Success : kTE_Unknown, result);
   return error;
}

STransfarringError CSceTUS::GetSlotData( int const slotIndex, void *pBufferOut, size_t const sizeOut, STUSSlotInfo *pSlotInfoOut )
{
   STransfarringError error;
   if ( !EnsureValidTitleContext(&error) )
   {
      return error;
   }
   else if( gBP_Transfarring_FailGetSlotData )
   {
      debug_menu_value_to_error( &error, gBP_Transfarring_FailGetSlotData );
      return error;
   }

   SceNpTusDataStatus status = { 0 };
   memset( pBufferOut, 0, sizeOut );

   int result = sceNpTusGetData( 
      AllocRequestID(),
      BP_Network_GetNPID(), // NP ID
      slotIndex,
      &status, sizeof( status ), 
      pBufferOut, 
      sizeOut, 
      NULL );

   tus_log_printf( "TUS: GetSlotData: np:%8.8x s:%d o:%8.8x so:%d r:%d\n",
      BP_Network_GetNPID(), 
      slotIndex,
      pBufferOut,
      sizeOut,
      result );

   if ( result >= 0 && !gBP_Transfarring_FailGetSlotData )
   {
      memset( pSlotInfoOut->mAccessoryData, 0, sizeof( pSlotInfoOut->mAccessoryData ) );
      memcpy( pSlotInfoOut->mAccessoryData, status.info.data, status.info.infoSize );

      SET_TRANSFARRING_ERROR_SUCCESS(error);
   }
   else
   {
      SET_TRANSFARRING_ERROR(error, kTE_Unknown, result);
   }

   return error;
}

STransfarringError CSceTUS::GetSlotInfos( STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots )
{
   STransfarringError error;
   if ( !EnsureValidTitleContext(&error) )
   {
      return error;
   }
   else if( gBP_Transfarring_FailGetSlotInfos )
   {
      debug_menu_value_to_error( &error, gBP_Transfarring_FailGetSlotInfos );
      return error;
   }

   SceNpTusSlotId const skSlotIdArray[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
   SceNpTusDataStatus statuses[8] = { { 0 } };

   int result = sceNpTusGetMultiSlotDataStatus( 
      AllocRequestID(),
      BP_Network_GetNPID(), // NP ID
      skSlotIdArray,
      statuses,
      sizeof( statuses ), 
      8,
      NULL );

   if (result < 0 && !gBP_Transfarring_FailGetSlotInfos )
   {
#if defined(BP_VITA)
      if (result == SCE_HTTP_ERROR_RESOLVER_ENODNS || result == 0x8041211b) // Undocumented error that occurs on network disconnection
      {
         SET_TRANSFARRING_ERROR(error, kTE_SignedInButDisconnected, result);
      }
      else
      {
         SET_TRANSFARRING_ERROR(error, kTE_Unknown, result);
      }
#else
      SET_TRANSFARRING_ERROR(error, kTE_Unknown, result);
#endif
      return error;
   }
   
   // Valid Slots is a bitfield
   uint8 validSlots = 0;

   // Run through each one and look at the valid slots
   for ( int slotIndex = 0; slotIndex < 8; ++slotIndex )
   {
      STUSSlotInfo &currentOut = pSlotInfosOut[ slotIndex ];

      // Zero out any current accessory data
      memset( currentOut.mAccessoryData, 0, sizeof( currentOut.mAccessoryData ) );

      // If we have data, then mark it and copy it
      if ( statuses[slotIndex].hasData )
      {
         validSlots |= ( 1 << slotIndex );
         memcpy( currentOut.mAccessoryData, statuses[slotIndex].info.data, statuses[slotIndex].info.infoSize );
      }
   }

   *pValidSlots = validSlots;

   SET_TRANSFARRING_ERROR_SUCCESS(error);
   return error;
}

STransfarringError CSceTUS::IncrementSlotVariable( int const slotIndex, unsigned long long incrementBy, unsigned long long *pOutOldValue, unsigned long long *pOutNewValue )
{
   STransfarringError error;
   if ( !EnsureValidTitleContext(&error) )
   {
      return error;
   }
   else if( gBP_Transfarring_FailIncrementSlotVariable )
   {
      debug_menu_value_to_error( &error, gBP_Transfarring_FailIncrementSlotVariable );
      return error;
   }

   SceNpTusVariable variable = { 0 };

   int result = sceNpTusAddAndGetVariable(
      AllocRequestID(),
      BP_Network_GetNPID(), // NP ID
      slotIndex,
      incrementBy, 
#if BP_VITA
      NULL, // last change author
      NULL, // last change date
#endif
      &variable,
      sizeof( variable ),
      NULL );

   if ( result == 0 && !gBP_Transfarring_FailIncrementSlotVariable )
   {
      if ( pOutOldValue )
      {
         *pOutOldValue = variable.oldVariable;
      }

      *pOutNewValue = variable.variable;

      SET_TRANSFARRING_ERROR_SUCCESS(error);
   }
   else
   {
      SET_TRANSFARRING_ERROR(error, kTE_Unknown, result);
   }

   return error;
}
