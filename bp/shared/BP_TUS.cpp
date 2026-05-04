//----------------------------------------------------------------------------
// BP_TUS.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/COsContext.h"

#include "BP_FileSupport.h"
#include "BP_EndianSupport.h"
#include "BP_Network.h"
#include "ITUSInterface.h"
#include "BP_TUS.h"

#include <stdio.h>
#include "Engine/System/CSyncEvent.h"

#if BP_VITA
#  include "Engine/System/VTAThreadPriorities.h"
#  include "CSceTUS.h"
#endif

#if BP_WIN32
#  include <windows.h>
#endif

#if BP_PS3
#  include <sys/ppu_thread.h>
#  include "sys/synchronization.h"
#  include "CSceTUS.h"
#endif

ITUSInterface *gpTUS = NULL;

namespace
{
   int sTUS_PSN_Netcheck_Handle = 0;
   bool sTUS_PSN_Wants_Signin = false;
}

class CFileBasedTUS : public ITUSInterface
{
public:
   virtual STransfarringError DeleteSlotData( int const slotIndex );
   virtual STransfarringError SetSlotData( int const slotIndex, void const *pBuffer, size_t const size, STUSSlotInfo const &pSlotInfo );
   virtual STransfarringError GetSlotData( int const slotIndex, void *pBufferOut, size_t const sizeOut, STUSSlotInfo *pSlotInfoOut );
   virtual STransfarringError GetSlotInfos( STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots );

   virtual STransfarringError IncrementSlotVariable( int const slotIndex, unsigned long long incrementBy, unsigned long long *pOutOldValue, unsigned long long *pOutNewValue );
private:
   static void CreateDataNameForSlot( int const slotIndex, char *pDataNameOut );
   static void CreateCounterNameForSlot( int const slotIndex, char *pDataNameOut );

   typedef char TFileNameStr[255];

};

void CFileBasedTUS::CreateDataNameForSlot( int const slotIndex, char *pDataNameOut )
{
   sprintf( pDataNameOut, "tusdataslot%d.dat", slotIndex );
}

void CFileBasedTUS::CreateCounterNameForSlot( int const slotIndex, char *pDataNameOut )
{
   sprintf( pDataNameOut, "tuscountslot%d.dat", slotIndex );
}

STransfarringError CFileBasedTUS::DeleteSlotData( int const slotIndex )
{
   TFileNameStr filename;
   CreateDataNameForSlot( slotIndex, filename );

   FILE *fp = fopen( filename, "r+b" );

   if ( fp )
   {
      uint8 slotValid = 0;

      fwrite( &slotValid, 1, 1, fp );
      fclose( fp );
   }

   STransfarringError error;
   SET_TRANSFARRING_ERROR_SUCCESS(error);
   return error;
}

STransfarringError CFileBasedTUS::SetSlotData( int const slotIndex, void const *pBuffer, size_t const size, STUSSlotInfo const &pSlotInfo )
{
   STransfarringError error;

   TFileNameStr filename;
   CreateDataNameForSlot( slotIndex, filename );

   FILE *fp = fopen( filename, "wb" );
   if ( fp )
   {
      uint8 slotValid = 0xFF;
      fwrite( &slotValid, 1, 1, fp );
      fwrite( pSlotInfo.mAccessoryData, 1, sizeof( pSlotInfo.mAccessoryData ), fp );
      fwrite( pBuffer, 1, size, fp );
      fclose( fp );

      SET_TRANSFARRING_ERROR_SUCCESS(error);
   }
   else
   {
      SET_TRANSFARRING_ERROR(error, kTE_Unknown, 0);
   }

   return error;
}

STransfarringError CFileBasedTUS::GetSlotData( int const slotIndex, void *pBufferOut, size_t const sizeOut, STUSSlotInfo *pSlotInfoOut )
{
   STransfarringError currentError;

   TFileNameStr filename;
   CreateDataNameForSlot( slotIndex, filename );

   FILE *fp = fopen( filename, "rb" );

   if ( fp )
   {
      SET_TRANSFARRING_ERROR(currentError, kTE_Unknown, 0);
      uint8 slotValid = 0;
      fread( &slotValid, 1, 1, fp );

      if ( slotValid )
      {
         size_t const accessorySize = fread( pSlotInfoOut->mAccessoryData, 1, sizeof( pSlotInfoOut->mAccessoryData ), fp );

         if ( accessorySize != sizeof( pSlotInfoOut->mAccessoryData ) )
         {
            SET_TRANSFARRING_ERROR(currentError, kTE_Unknown, 0);
         }
         else
         {
            size_t const actualReadSize = fread( pBufferOut, 1, sizeOut, fp );

            SET_TRANSFARRING_ERROR_SUCCESS(currentError);
         }
      }
      else
      {
         SET_TRANSFARRING_ERROR(currentError, kTE_GetSlotData_SlotNotFound, 0);
      }

      fclose( fp );
      return currentError;
   }
   else
   {
      SET_TRANSFARRING_ERROR(currentError, kTE_GetSlotData_SlotNotFound, 0);
   }

   return currentError;
}

STransfarringError CFileBasedTUS::GetSlotInfos( STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots )
{
   STransfarringError error;
   uint8 validSlots = 0;

   for ( int i = 0; i < 8; ++i )
   {
      TFileNameStr filename;
      CreateDataNameForSlot( i, filename );

      FILE *fp = fopen( filename, "rb" );
      if ( fp )
      {
         uint8 slotValid = 0;
         fread( &slotValid, 1, 1, fp );

         if ( slotValid )
         {
            size_t const accessorySize = fread( pSlotInfosOut[i].mAccessoryData, 1, sizeof( pSlotInfosOut[i].mAccessoryData ), fp );

            if ( accessorySize == sizeof( pSlotInfosOut[i].mAccessoryData ) )
            {
               // Read a slot info!

               validSlots |= ( 1 << i );
            }
         }

         fclose( fp );
      }
   }

   *pValidSlots = validSlots;

   SET_TRANSFARRING_ERROR_SUCCESS(error);
   return error;
}

STransfarringError CFileBasedTUS::IncrementSlotVariable( int const slotIndex, unsigned long long incrementBy, unsigned long long *pOutOldValue, unsigned long long *pOutNewValue )
{
   STransfarringError error;

   TFileNameStr filename;
   CreateCounterNameForSlot( slotIndex, filename );

   unsigned long long initialValue = 0;

   // If the file exists, read the initial value.
   // If it doesn't exist, the initial value is zero
   FILE *fp = fopen( filename, "r+b" );
   if ( fp )
   {
      if ( fread( &initialValue, sizeof( initialValue ), 1, fp ) )
      {
         // We read an item, let's make sure it's big-endian
         BP_BE_Swap8Bytes_Inp( &initialValue );
      }
      else
      {
         // Set it back to 0 just in case
         initialValue = 0;
      }

      // Seek back to the beginning of the file so we can overwrite it.
      fseek( fp, 0, SEEK_SET );
   }
   else
   {
      // File didn't exist, let's create it.
      fp = fopen( filename, "wb" );
   }

   if ( fp )
   {
      // If we have a valid file at this point, it means we can increment and write
      unsigned long long const newValue = initialValue + incrementBy;
      unsigned long long newValue_BE;

      BP_BE_Swap8Bytes( &newValue_BE, &newValue );

      fwrite( &newValue_BE, sizeof( newValue_BE ), 1, fp );

      fclose( fp );

      if ( pOutOldValue )
      {
         *pOutOldValue = initialValue;
      }

      if ( pOutNewValue )
      {
         *pOutNewValue = newValue;
      }

      SET_TRANSFARRING_ERROR_SUCCESS(error);
   }
   else
   {
      SET_TRANSFARRING_ERROR(error, kTE_Unknown, 0);
   }

   return error;
}

namespace NTUSWorkerThread
{
   enum EWorkType
   {
      kWT_SetSlotData,
      kWT_GetSlotData,
      kWT_DeleteSlot,
      kWT_GetSlotInfos,
      kWT_IncrementSlotVariable,
   };

   bool sInit = false;
#if BP_PS3
   // PS3's version of CSyncEvent does not behave the same as Vita or Win32
   // However, since there are systems depending on it, it's better to implement
   // the platform-specific behavior here rather than change the original.
   //
   // Specifically, PS3's version does not handle auto-reset events.

   sys_event_flag_t sPS3Event = 0xFFFFFFFF;
   uint64 const skPS3StartWorkFlag = 1ULL;

   sys_ppu_thread_t sPS3ThreadId = -1;
#else
   CSyncEvent sEvent( /* manualReset? No */ false, /* initialState */ false );
#endif
   volatile bool sThreadBusy = false;
   uint64 sCurrentWorkItem = 1;

   // Work buffers for slot operations
   // These are both input and output
   uint8 sSlotStorage[ 64 * 1024 ];
   STUSSlotInfo sSlotInfoStorage[ 8 ]; // [0] is used for Get/Set Slot Data

   volatile bool sWantsTUSThreadDead = false;

   // Work inputs
   EWorkType sWorkInput_Type;
   int sWorkInput_SlotIndex;
   uint64 sWorkInput_IncrementBy;

   // Work other output
   uint8 sWorkOutput_SlotInfoValid;
   uint64 sWorkOutput_IncrementPriorValue;
   uint64 sWorkOutput_IncrementNewValue;

   STransfarringError sTUSError;

   void ThreadFunction();

   // Implementing platform-specific thread function wrappers below.
#if BP_VITA
   SceInt32 ThreadFunctionVita( SceSize argSize, void *pArgBlock )
   {
      ThreadFunction();
      return 0;
   }
#endif

#if BP_PS3
   void ThreadFunctionPS3( uint64_t arg )
   {
      ThreadFunction();
      sys_ppu_thread_exit( 0 );
   }
#endif

#if BP_WIN32
   DWORD WINAPI ThreadFunctionWin32( LPVOID )
   {
      ThreadFunction();
      return 0;
   }
#endif

   void Init()
   {
      if ( !sInit )
      {
         sInit = true;

         sThreadBusy = false;

#if BP_PS3
         sys_event_flag_attribute_t eventFlagAttr;

         sys_event_flag_attribute_initialize( eventFlagAttr );
         sys_event_flag_attribute_name_set( eventFlagAttr.name, "TUSWait" );

         int eventFlagResult = sys_event_flag_create( &sPS3Event, &eventFlagAttr, 0 );
         BPE_VERIFY( eventFlagResult == CELL_OK, false, "Couldn't create TUS waiter flag" );
#else
         sEvent.Reset();
#endif

#if BP_VITA
         SceUID threadId = sceKernelCreateThread( "TUS", &ThreadFunctionVita, NVtaThreadPriorities::kPriorityTUS, 
            96 * 1024 /* 96k stack */ , 
            0, SCE_KERNEL_CPU_MASK_USER_ALL, NULL );

         BPE_VERIFY( threadId > 0, false, "Couldn't create TUS Thread" );

         sceKernelStartThread( threadId, 0, NULL );

#elif BP_WIN32
         HANDLE threadHandle = CreateThread( NULL, 0, &ThreadFunctionWin32, NULL, 0, NULL );

         BPE_VERIFY( threadHandle != BPE_INVALID_HANDLE_VALUE, false, "Couldn't create TUS thread" );

         CloseHandle( threadHandle );
#elif BP_PS3
         int const kThreadPriority = 1001;
         int const kStackSize = 128 * 1024;

         int ret = sys_ppu_thread_create(&sPS3ThreadId, &ThreadFunctionPS3, 0 /* arg */, kThreadPriority, kStackSize, SYS_PPU_THREAD_CREATE_JOINABLE /* flags */, "TUS");

         BPE_VERIFY( ret == CELL_OK, false, "Couldn't create TUS Thread!" );
#else
#  error Unsupported platform!
#endif
      }
   }

   void ThreadFunction()
   {
      // This thread simply waits for work, does the work, then waits again.

      for ( ;; )
      {
         // Wait until we have work pending
#if BP_PS3
         uint64_t ps3Result = 0;
         sys_event_flag_wait( sPS3Event, skPS3StartWorkFlag, SYS_EVENT_FLAG_WAIT_AND | SYS_EVENT_FLAG_WAIT_CLEAR, &ps3Result, SYS_NO_TIMEOUT );
#else
         sEvent.Wait();
#endif

         if ( sWantsTUSThreadDead )
         {
            return;
         }

         // Do the appropriate work
         switch ( sWorkInput_Type )
         {
         case kWT_SetSlotData:
            sTUSError = gpTUS->SetSlotData( sWorkInput_SlotIndex, sSlotStorage, sizeof( sSlotStorage ), sSlotInfoStorage[0] );
            break;
         case kWT_GetSlotData:
            memset( sSlotStorage, 0, sizeof( sSlotStorage ) );
            sTUSError = gpTUS->GetSlotData( sWorkInput_SlotIndex, sSlotStorage, sizeof( sSlotStorage ), &( sSlotInfoStorage[0] ) );
            break;
         case kWT_GetSlotInfos:
            sWorkOutput_SlotInfoValid = 0;
            sTUSError = gpTUS->GetSlotInfos( sSlotInfoStorage, &sWorkOutput_SlotInfoValid );
            break;
         case kWT_IncrementSlotVariable:
            sTUSError = gpTUS->IncrementSlotVariable( sWorkInput_SlotIndex, sWorkInput_IncrementBy, &sWorkOutput_IncrementPriorValue, &sWorkOutput_IncrementNewValue );
            break;
         case kWT_DeleteSlot:
            sTUSError = gpTUS->DeleteSlotData( sWorkInput_SlotIndex );
            break;
         }

         // Tell the main thread we're no longer busy (note that the main thread actually
         // sets then when work is pending)
         sThreadBusy = false;
      }
   }

   uint64 _GameThread_FlagWorkToStart()
   {
      // This function assumes that a check has been made that the thread is not busy
      sThreadBusy = true;
      ++sCurrentWorkItem;
      SET_TRANSFARRING_ERROR_SUCCESS(sTUSError);

#if BP_PS3
      sys_event_flag_set( sPS3Event, skPS3StartWorkFlag );
#else
      sEvent.Set();
#endif

      return sCurrentWorkItem;
   }

   uint64 _GameThread_MakeErrorWork( STransfarringError error )
   {
      // This makes an immediate TUS error condition and doesn't actually cause
      // any work to happen on the other thread

      ++sCurrentWorkItem;
      sTUSError = error;

      return sCurrentWorkItem;
   }

   bool _GameThread_CanPostWork()
   {
      return !sThreadBusy;
   }

   // _GameThread_TryConsumeWork
   //
   // This tries to consume the work specified by workItem.  It also verifies 
   // that the work type is what we think it is.
   // Returns true either on error or work complete
   // Returns false if the thread is busy on the current work item.
   bool _GameThread_TryConsumeWork( uint64 const workItem, EWorkType const workType )
   {
      if ( sCurrentWorkItem != workItem || sWorkInput_Type != workType )
      {
         SET_TRANSFARRING_ERROR(sTUSError, kTE_Unknown, 0);
         return true;
      }

      return !sThreadBusy;
   }

   uint64 GameThread_Post_GetSlotData( int const slotIndex )
   {
      if ( !_GameThread_CanPostWork() ) return 0;

      sWorkInput_Type = kWT_GetSlotData;
      sWorkInput_SlotIndex = slotIndex;

      return _GameThread_FlagWorkToStart();
   }

   uint64 GameThread_Post_SetSlotData( int const slotIndex, void const *slotData, int const slotSize, STUSSlotInfo const &slotInfo )
   {
      if ( !_GameThread_CanPostWork() ) return 0;

      BPE_VERIFY( slotSize <= sizeof( sSlotStorage ), false, "Slot storage out of bounds" );

      if ( slotSize > sizeof( sSlotStorage ) )
      {
         STransfarringError error;
         SET_TRANSFARRING_ERROR(error, kTE_SetSlotData_SizeTooLarge, 0);
         return _GameThread_MakeErrorWork( error );
      }
      else
      {
         sWorkInput_Type = kWT_SetSlotData;
         sWorkInput_SlotIndex = slotIndex;

         memset( sSlotStorage, 0, sizeof( sSlotStorage ) );
         memcpy( sSlotStorage, slotData, slotSize );
         sSlotInfoStorage[0] = slotInfo;

         return _GameThread_FlagWorkToStart();
      }      
   }

   uint64 GameThread_Post_GetSlotInfos()
   {
      if ( !_GameThread_CanPostWork() ) return 0;

      sWorkInput_Type = kWT_GetSlotInfos;
      memset( sSlotStorage, 0, sizeof( sSlotStorage ) );
      sWorkOutput_SlotInfoValid = 0;

      return _GameThread_FlagWorkToStart();
   }

   uint64 GameThread_Post_IncrementSlotVariable(int const slotIndex, uint64 incrementBy)
   {
      if ( !_GameThread_CanPostWork() ) return 0;

      sWorkInput_Type = kWT_IncrementSlotVariable;
      sWorkInput_IncrementBy = incrementBy;
      sWorkInput_SlotIndex = slotIndex;
      sWorkOutput_IncrementPriorValue = 0;
      sWorkOutput_IncrementNewValue = 0;

      return _GameThread_FlagWorkToStart();
   }

   uint64 GameThread_Post_DeleteSlotData(int const slotIndex)
   {
      if ( !_GameThread_CanPostWork() ) return 0;

      sWorkInput_Type = kWT_DeleteSlot;
      sWorkInput_SlotIndex = slotIndex;

      return _GameThread_FlagWorkToStart();
   }

   bool GameThread_Consume_GetSlotData( uint64 workItem, STransfarringError *pError, void *pBuffer, STUSSlotInfo *pSlotInfo )
   {
      if ( !_GameThread_TryConsumeWork( workItem, kWT_GetSlotData ) ) return false;

      *pError = sTUSError;
      if ( sTUSError.mHighLevelError == kTE_Success )
      {
         memcpy( pBuffer, sSlotStorage, sizeof( sSlotStorage ) );
         *pSlotInfo = sSlotInfoStorage[0];
      }
      else
      {
         memset( pBuffer, 0, sizeof( sSlotStorage ) );
         memset( pSlotInfo, 0, sizeof( *pSlotInfo ) );
      }

      return true;
   }

   bool GameThread_Consume_SetSlotData( uint64 workItem, STransfarringError *pError )
   {
      if ( !_GameThread_TryConsumeWork( workItem, kWT_SetSlotData ) ) return false;

      *pError = sTUSError;

      return true;
   }

   bool GameThread_Consume_GetSlotInfos( uint64 workItem, STransfarringError *pError, STUSSlotInfo *pSlotInfosOut, uint8 *pValidSlots )
   {
      if ( !_GameThread_TryConsumeWork( workItem, kWT_GetSlotInfos ) ) return false;

      *pError = sTUSError;
      memcpy( pSlotInfosOut, sSlotInfoStorage, sizeof( sSlotInfoStorage ) );
      *pValidSlots = sWorkOutput_SlotInfoValid;

      return true;
   }

   bool GameThread_Consume_IncrementSlotVariable( uint64 workItem, STransfarringError *pError, unsigned long long *pOptionalOutOldValue, unsigned long long *pOutNewValue )
   {
      if ( !_GameThread_TryConsumeWork( workItem, kWT_IncrementSlotVariable ) ) return false;

      *pError = sTUSError;
      
      if ( pOptionalOutOldValue )
      {
         *pOptionalOutOldValue = sWorkOutput_IncrementPriorValue;
      }

      *pOutNewValue = sWorkOutput_IncrementNewValue;

      return true;
   }

   bool GameThread_Consume_DeleteSlotData( uint64 workItem, STransfarringError *pError )
   {
      if ( !_GameThread_TryConsumeWork( workItem, kWT_DeleteSlot ) ) return false;

      *pError = sTUSError;

      return true;
   }
}

//----------------------------------------------------------------------------

void BP_TUS_Init()
{
#if BP_VITA || BP_PS3
   static CSceTUS sSCETUS;

   gpTUS = &sSCETUS;
#else
   static CFileBasedTUS sFileBasedTUS;

   gpTUS = &sFileBasedTUS;
#endif

   NTUSWorkerThread::Init();
}

//----------------------------------------------------------------------------

uint64 BP_TUS_Post_GetSlotData( int const slotIndex )
{
   return NTUSWorkerThread::GameThread_Post_GetSlotData( slotIndex );
}

//----------------------------------------------------------------------------

uint64 BP_TUS_Post_SetSlotData( int const slotIndex, void const *slotData, int const slotSize, void const * accessoryData )
{
   STUSSlotInfo slotInfo;

   memcpy( slotInfo.mAccessoryData, accessoryData, sizeof( slotInfo.mAccessoryData ) );

   return NTUSWorkerThread::GameThread_Post_SetSlotData( slotIndex, slotData, slotSize, slotInfo );
}

//----------------------------------------------------------------------------

uint64 BP_TUS_Post_GetSlotInfos()
{
   return NTUSWorkerThread::GameThread_Post_GetSlotInfos();
}

//----------------------------------------------------------------------------

uint64 BP_TUS_Post_IncrementSlotVariable(int const slotIndex, unsigned long long incrementBy)
{
   return NTUSWorkerThread::GameThread_Post_IncrementSlotVariable(slotIndex, incrementBy);
}

//----------------------------------------------------------------------------

uint64 BP_TUS_Post_DeleteSlotData(int const slotIndex)
{
   return NTUSWorkerThread::GameThread_Post_DeleteSlotData(slotIndex);
}

//----------------------------------------------------------------------------

int BP_TUS_Consume_GetSlotData( uint64 workItem, STransfarringError *pError, void *pBuffer, void *accessoryData )
{
   return NTUSWorkerThread::GameThread_Consume_GetSlotData( workItem, pError, pBuffer,
      reinterpret_cast<STUSSlotInfo *>( accessoryData ) );
}

//----------------------------------------------------------------------------

int BP_TUS_Consume_SetSlotData( uint64 workItem, STransfarringError *pError )
{
   return NTUSWorkerThread::GameThread_Consume_SetSlotData( workItem, pError );
}

//----------------------------------------------------------------------------

int BP_TUS_Consume_GetSlotInfos( uint64 workItem, STransfarringError *pError, void *pSlotInfosOut, uint8 *pValidSlots )
{
   return NTUSWorkerThread::GameThread_Consume_GetSlotInfos( 
      workItem, 
      pError, 
      reinterpret_cast<STUSSlotInfo *>( pSlotInfosOut ), 
      pValidSlots );
}

//----------------------------------------------------------------------------

int BP_TUS_Consume_IncrementSlotVariable( uint64 workItem, STransfarringError *pError, unsigned long long *pOptionalOutOldValue, unsigned long long *pOutNewValue )
{
   return NTUSWorkerThread::GameThread_Consume_IncrementSlotVariable( 
      workItem, 
      pError, 
      pOptionalOutOldValue,
      pOutNewValue );
}

//----------------------------------------------------------------------------

int BP_TUS_Consume_DeleteSlotData( unsigned long long workItem, STransfarringError *pError )
{
   return NTUSWorkerThread::GameThread_Consume_DeleteSlotData( workItem, pError );
}

//----------------------------------------------------------------------------

void BP_TUS_WantsSignin()
{
   sTUS_PSN_Wants_Signin = true;
}

//----------------------------------------------------------------------------

void BP_TUS_HeartBeat()
{
   // If we're running the netcheck box, keep checking until it's done

   if ( sTUS_PSN_Netcheck_Handle )
   {
      if ( !BP_Network_Dialog_IsStillProcessing( sTUS_PSN_Netcheck_Handle ) )
      {
         int result = -1;

         BP_Network_Dialog_GetResults( sTUS_PSN_Netcheck_Handle, &result );
         sTUS_PSN_Netcheck_Handle = 0;
      }
   }
   else if ( sTUS_PSN_Wants_Signin )
   {
      if ( !BP_Network_IsSignedIn() )
      {
         sTUS_PSN_Netcheck_Handle = BP_Network_Dialog_BeginPSN();
      }
      sTUS_PSN_Wants_Signin = false;
   }
#if 0
   // JM - HANDY TO PUT DEBUG CODE HERE
   //      Anything in here can be replaced
   else if ( BP_Network_IsSignedIn() )
   {
      static uint64 sIncrementHandle = -1;
         
      if ( sIncrementHandle == -1 )
      {
         sIncrementHandle = BP_TUS_Post_IncrementSlotVariable( 1, 1 );
      }

      if ( sIncrementHandle > 0 )
      {
         int error;
     
         uint64 newValue = 0;

         if ( BP_TUS_Consume_IncrementSlotVariable( sIncrementHandle, &error, NULL, &newValue ) )
         {
            printf( "Consuem done! Err: %d Val: %d\n", error, uint32( newValue ) );
            sIncrementHandle = 0;
         }
      }
   }
#endif
}

#if BP_PS3

void BP_TUS_ForceTerm()
{
   // Tell it to die
   NTUSWorkerThread::sWantsTUSThreadDead = true;

   // Tell it we have work for it.  In this case, the work is 
   // to die.
#if BP_PS3
   sys_event_flag_set( NTUSWorkerThread::sPS3Event, NTUSWorkerThread::skPS3StartWorkFlag );
#else
   sEvent.Set();
#endif

   // Wait for the thread to terminate
   uint64_t exitStatus = 0;
   sys_ppu_thread_join( NTUSWorkerThread::sPS3ThreadId, &exitStatus );

   // Now terminate TUS
   sceNpTusTerm();
}

#endif