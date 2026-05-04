//------------------------------------------------------------------------------------------
// TransfarringVTA.cpp
//
// Vita transfarring functions
//------------------------------------------------------------------------------------------

#include <libnetctl.h>
#include <common_dialog.h>
#include <netcheck_dialog.h>
#include <apputil.h>

#include "Engine/Stdafx.h"
#include "Engine/System/COsContext.h"
#include "TransfarringVTA.h"
#include "BP_SaveLoadMGS.h"
#include "BP_Memory.h"
#include "BP_Network.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "BP_MD5.h"

//#include "cdbios.h"
#include "StdAfx.h"

#include <np/np_manager.h>
#include <kernel/openpsid.h>

#include <sstream>

#if MGS_VERSION == 3

extern "C" {
#include "../../source/user/arai/mc/mc.h"
#include "../../source/user/arai/mc/mc_actor_ext.h"
#include "libcdvd.h"
}

#endif

extern "C" {
#include "md5.h"
}

#if MGS_VERSION == 3
   #define PS3_WPAKEY      "3308CAD2BA1AD28CF2D845A4239AEAEB"
#else
   #define PS3_WPAKEY      "2186CAD2BA1AD28CF2D845A4239AEAEA"
#endif

//"Title Ids" used as passwords for Wifi transfarring
//Used to stop connections between different SKUS
//Keep in sync with Transfarring_PS3.cpp
#if MGS_VERSION == 2
static char const skPS3TitleId_JP[] = "BLJM61011";
static char const skPS3TitleId_NA[] = "BLUS30857";
static char const skPS3TitleId_EU[] = "BLES01429";
#else
static char const skPS3TitleId_JP[] = "BLJM61002";
static char const skPS3TitleId_NA[] = "BLUS30848";
static char const skPS3TitleId_EU[] = "BLES01410";
#endif

#define WIFI_PROTOCOL_VERSION 6

const char kMessageSendFileToVita = 0x3E;
const char kMessageSendFileToPS3 = 0x7A;
const char kMessageSendFileListToPS3 = 0x1E;
const char kMessageKillConnection = 0x2F;
const char kMessageReceivedFile = 0x0B;
const char kMessageUnlinkFile = 0x4C;
const char kMessageFileUnlinked = 0x5D;
const char kMessageInvalid = 0xFF;

static const SceUInt32 skDelayNetworkThreadTimeMilliseconds = 5;
static const SceUInt32 skDelayNetworkThreadTimeMicroseconds = skDelayNetworkThreadTimeMicroseconds * 1000;

static const uint32_t skNumNetworkUserThreads = 1;
static const uint32_t skNumNetworkWorkerThreads = 1;

static int32_t VTANetworkThread(uint32_t arg);
static void ps3connect_netctl_callback(int eventType, void *arg);
static void CreateSaveFileFromCodedString(const char* name, TransfarringSaveFile* file);

// Loading
#if MGS_VERSION == 2

static MCMAN_WORK sMCMan;

#endif

#if MGS_VERSION == 3

static GV_HANDLE sMCActor;

#endif

const int kSendDataSize = sizeof(TRANSFARRING_GAME_DATA);

static SceUID gMemoryCardMutex;
static SceNetId s_sock = -1;
static int sCallbackID = -1;

CTransfarringManager* CTransfarringManager::m_instance = NULL;

// CTransfarringManager
CTransfarringManager::CTransfarringManager()
: m_bNeedsRealDelete(false)
, m_pRuntimeWorkArea(NULL)
, m_pThreadContext(NULL)
, mIsNetworkInitialized( false )
, mIsNetworkThreadRunning( false )
, m_isConnectedToPS3(false)
, m_hasDisconnectedFromPS3(false)
, m_status(kVTS_Uninitialized)
, m_LocalFileList(NULL)
, m_AllowIDUpdates(true)
, m_numPhases(0)
, m_curPhase(0)
, m_curProgress(0)
, m_errorSyncEvent(false, false)
, m_bIsWaitingForErrorResolution(false)
, m_bUsingExternalMemoryCardActor(false)
, mGameThreadWorkInput_WorkType(kTW_Idle)
, mGameThreadWorkInput_SaveType(kST_Game)
, mGameThreadWorkInput_NonGameSaveType( kNGST_MaxNonGameSaveTypes )
, mGameThreadWorkInput_SaveIndex(-1)
, mGameThreadWorkInput_ConflictResolution(kCCR_Invalid)
, mGameThreadWorkInput_SyncAllTypes(false)
, mGameThreadWorkInput_TreatVersionMismatchesAsConflicts(false)
, mNetworkThreadWorkInput_WorkType( kNW_Idle )
, mNetworkThreadWorkPosted( false )
, mNetworkThreadSyncEvent( false, false )
, mNeedsMemoryCardCleanup( false )
, mAreFileListsValid( false )
, mShouldOverrideSaveDate( false )
{
   sceKernelCreateMutex("TransfarringSaveLoadMutex", SCE_KERNEL_MUTEX_ATTR_TH_FIFO, 0, NULL);

   mIDMap.resize( kST_MaxSaveTypes, TIDMapEntry() );

   SET_TRANSFARRING_ERROR_INVALID(m_wifiError);

   m_errorSyncEvent.Reset();
   mNetworkThreadSyncEvent.Reset();

   memset( mGameThreadWorkInput_TransfarringID, 0, sizeof(mGameThreadWorkInput_TransfarringID) );

   memset( mSaltString, 0, sizeof(mSaltString) );
   sprintf( mSaltString, 
      "%8.8x%8.8x%8.8x%8.8x", 
      WIFI_PROTOCOL_VERSION, 
      WIFI_PROTOCOL_VERSION + 0x621fdf0d, 
      WIFI_PROTOCOL_VERSION + 0x2058cf71, 
      WIFI_PROTOCOL_VERSION + 0xdf4b920f );

   switch( gpOsContext->mBuildSKU )
   {
   case CBaseOsContext::kBS_USA:
      mPS3SKU = skPS3TitleId_NA;
      break;
   case CBaseOsContext::kBS_Europe:
      mPS3SKU = skPS3TitleId_EU;
      break;
   case CBaseOsContext::kBS_Japan:
      mPS3SKU = skPS3TitleId_JP;
      break;
   default:
      BPE_ASSERT( false, "TransfarringVTA was given a bad SKU" );
   }

   memset( &mOverrideSaveDate, 0, sizeof(mOverrideSaveDate) );
   memset( mLoadedTransfarringID, 0, sizeof(mLoadedTransfarringID) );
}

CTransfarringManager::~CTransfarringManager()
{
   m_instance = NULL;
   sceKernelDeleteMutex(gMemoryCardMutex);
   m_LocalFileList.ClearSaveFiles();
   m_CloudFileList.ClearCloudFiles();
}

CTransfarringManager* CTransfarringManager::Instance()
{
   return m_instance;
}

void CTransfarringManager::InitSingleton()
{
   BPE_VERIFY( CTransfarringManager::m_instance == NULL, false, "Can't call transfarring init twice" );

   m_instance = new CTransfarringManager();
}

bool CTransfarringManager::NeedsRealDelete()
{
   return m_bNeedsRealDelete;
}

void CTransfarringManager::EndianSwapGameData(TRANSFARRING_GAME_DATA* dst, const TRANSFARRING_GAME_DATA* src, ESaveType saveType) const
{
   // No endian swapping done on Vita, just copy
   if( src != dst )
   {
      memcpy( dst, src, sizeof(TRANSFARRING_GAME_DATA) );
   }
}

void CTransfarringManager::GetConsoleOpenPSID(unsigned char* dst)
{

//We are not using the console PSID anymore so in order to avoid a TRC violation
//Calls to this function will zero out memory instead of calling sceKernelGetOpenPsId().

//    SceKernelOpenPsId psid;
//    memset(&psid, 0, sizeof(psid));
//    sceKernelGetOpenPsId(&psid);
// 
//    BPE_CTASSERT(sizeof(psid) == 16);
// 
//    memcpy(dst, &psid, sizeof(psid)); // 16 bytes of hash data

   memset(dst, 0, sizeof(SceKernelOpenPsId));
}

void CTransfarringManager::GetPSNAccountHash( bool useCached, unsigned char* dst, unsigned char* transfarringID )
{
   char psnAccountName[SCE_NP_ONLINEID_MAX_LENGTH+1];
   memset( psnAccountName, 0, sizeof(psnAccountName) );

   int ret = -1;

   if( useCached 
   	|| BP_Network_Vita_IsPSNSignedIn() // If we're signed in (But not connected) we can use cache
	)
   {
      SceNpManagerCacheParam param;
      ret = sceNpManagerGetCachedParam(&param);

      if( ret == SCE_OK )
      {
         strcpy( psnAccountName, param.npId.handle.data );
      }
   }
   else
   {
      // AS(FR) - Realistically, this else should only get called 
	  // when offline, which means that in real life, npid will always be
	  // NULL.  Since we're trying to minimize changes close to ship,
	  // this is not getting changed
      const SceNpId* npid = BP_Network_GetNPID();
      
      if( npid )
      {
         strcpy( psnAccountName, npid->handle.data );

         ret = SCE_OK;
      }
   }

   if( ret == SCE_OK )
   {
      int psnIDLen = strlen( psnAccountName );
      if (psnIDLen > 0)
      {
         md5_state_s state;
         md5_init( &state );

         // Append the transfarring ID
         md5_append( &state, (const md5_byte_t*)transfarringID, TransfarringDefines::kHashLengthBytes );

         // Append the passphrase
         unsigned char passphrase[TransfarringDefines::kHashLengthBytes];
         GetPSNAccountPassphrase( passphrase );
         md5_append( &state, (const md5_byte_t*)passphrase, TransfarringDefines::kHashLengthBytes );

         // Append the PSN account name
         md5_append( &state, (const md5_byte_t*)psnAccountName, psnIDLen );

         md5_finish( &state, (md5_byte_t*)dst );
      }
      else
      {
         memset(dst, 0, TransfarringDefines::kHashLengthBytes);
      }
   }
   else
   {
      memset(dst, 0, TransfarringDefines::kHashLengthBytes);
   }
}

void CTransfarringManager::InitNetwork()
{
   if( !mIsNetworkInitialized )
   {
      CreateNetworkThread();
   }
}

void CTransfarringManager::CreateNetworkThread()
{
   uint32_t workSize = sceUltUlthreadRuntimeGetWorkAreaSize( skNumNetworkUserThreads, skNumNetworkWorkerThreads );

   SceUltUlthreadRuntimeOptParam param;
   sceUltUlthreadRuntimeOptParamInitialize( &param );

   param.workerThreadPriority  = SCE_KERNEL_DEFAULT_PRIORITY_USER - 1;

   m_pRuntimeWorkArea = BP_Memory_Alloc( workSize, 8, kMT_Permanent, kMC_Temp );
   int success = sceUltUlthreadRuntimeCreate( 
      &m_NetworkULTRuntime, 
      "Transfarring_ULT_runtime", 
      skNumNetworkUserThreads, 
      skNumNetworkWorkerThreads, 
      m_pRuntimeWorkArea, 
      &param );

   uint32 const skSizeContext = 32 * 1024; // Can't really find any information on how big this should be other than "more than 512"
   m_pThreadContext = BP_Memory_Alloc( skSizeContext, 8, kMT_Permanent, kMC_Temp ); 

   success = sceUltUlthreadCreate(
         &m_NetworkThread,
         "Transfarring_NetworkThread",
         VTANetworkThread,
         0, 
         m_pThreadContext,
         skSizeContext,
         &m_NetworkULTRuntime,
         NULL );

   mIsNetworkInitialized = true;
}

STransfarringError CTransfarringManager::PostNetworkThreadWork_LoadFileList()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if( CanPostNetworkThreadWork() )
   {
      m_status = kVTS_LoadingFileList;

      mNetworkThreadWorkInput_WorkType = kNW_LoadFileList;

      mAreFileListsValid = false;

      SET_TRANSFARRING_ERROR_SUCCESS( error );

      FinalizePostedWork_NetworkThread();
   }
   else
   {
      SET_TRANSFARRING_ERROR( error, kTE_Unknown, 0 );
   }

   return error;
}

STransfarringError CTransfarringManager::PostNetworkThreadWork_UnloadFileList()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   BPE_ASSERT(mNeedsMemoryCardCleanup == true, "WiFi transfarring uninitialized, tried to shut it down");

   if( CanPostNetworkThreadWork() )
   {
      m_status = kVTS_UnloadFileList;

      mNetworkThreadWorkInput_WorkType = kNW_UnloadFileList;

      SET_TRANSFARRING_ERROR_SUCCESS( error );

      FinalizePostedWork_NetworkThread();
   }
   else
   {
      SET_TRANSFARRING_ERROR( error, kTE_Unknown, 0 );
   }

   return error;
}

STransfarringError CTransfarringManager::PostNetworkThreadWork_ConnectToPS3()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if( !m_isConnectedToPS3 && CanPostNetworkThreadWork() )
   {
      m_status = kVTS_ConnectingToPS3;

      mNetworkThreadWorkInput_WorkType = kNW_ConnectToPS3;

      SET_TRANSFARRING_ERROR_SUCCESS( error );

      FinalizePostedWork_NetworkThread();
   }
   else
   {
      SET_TRANSFARRING_ERROR( error, kTE_Unknown, 0 );
   }

   return error;
}

STransfarringError CTransfarringManager::PostNetworkThreadWork_DisconnectFromPS3()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if( m_isConnectedToPS3 && CanPostNetworkThreadWork() )
   {
      m_status = kVTS_DisconnectingFromPS3;

      mNetworkThreadWorkInput_WorkType = kNW_DisconnectFromPS3;

      SET_TRANSFARRING_ERROR_SUCCESS( error );

      FinalizePostedWork_NetworkThread();
   }
   else
   {
      SET_TRANSFARRING_ERROR( error, kTE_Unknown, 0 );
   }

   return error;
}

STransfarringError CTransfarringManager::PostNetworkThreadWork_CleanupSystem()
{
   STransfarringError error;
   SET_TRANSFARRING_ERROR_INVALID( error );

   if( CanPostNetworkThreadWork() )
   {
      m_status = kVTS_CleaningUpSystem;

      mNetworkThreadWorkInput_WorkType = kNW_CleanupSystem;

      SET_TRANSFARRING_ERROR_SUCCESS( error );

      FinalizePostedWork_NetworkThread();
   }
   else
   {
      SET_TRANSFARRING_ERROR( error, kTE_Unknown, 0 );
   }

   return error;
}

bool CTransfarringManager::IsConnectedToPS3()
{
   return m_isConnectedToPS3;
}

#if MGS_VERSION == 2

void CTransfarringManager::CreateMemoryCardActor_NetworkThread()
{
   NewMCMan(&sMCMan);

   // Check the memory card
   bool checked = false;
   while (!checked)
   {
      LockSaveLoadMutex();
      checked = MCManChecked() != 0;
      UnlockSaveLoadMutex();
      sceKernelDelayThread(5 * 1000);
   }
   checked = false;

   SetMemoryCardActor(&sMCMan);

   mNeedsMemoryCardCleanup = true;
}

void CTransfarringManager::DestroyMemoryCardActor_NetworkThread()
{
   LockSaveLoadMutex();
   GV_DestroyActor(&sMCMan);
   UnlockSaveLoadMutex();

   SetMemoryCardActor(NULL);

   m_LocalFileList.ClearSaveFiles();
   m_CloudFileList.ClearCloudFiles();

   mNeedsMemoryCardCleanup = false;
}
#endif

#if MGS_VERSION == 3

void CTransfarringManager::CreateMemoryCardActor_NetworkThread()
{
   sMCActor = NewMcActor();
   
   SetMemoryCardActor(&sMCActor);

   mNeedsMemoryCardCleanup = true;
}

void CTransfarringManager::DestroyMemoryCardActor_NetworkThread()
{
   LockSaveLoadMutex();
   GV_ActorKill(sMCActor);
   UnlockSaveLoadMutex();

   SetMemoryCardActor(NULL);

   mNeedsMemoryCardCleanup = false;
}

#endif

static int32_t VTANetworkThread(uint32_t arg)
{
   (void)arg;
   return CTransfarringManager::Instance()->NetworkThread();
}

// Network thread
int32_t CTransfarringManager::NetworkThread()
{
   m_status = kVTS_Idle;

   mIsNetworkThreadRunning = true;
   mNetworkThreadWorkPosted = false;

   for( ;; )
   {
      // Process posted network thread work
      if( mNetworkThreadWorkPosted )
      {
         ProcessNetworkThreadWork_NetworkThread();
         m_status = kVTS_Idle;
      }

      // If connected to PS3 constantly
      if( m_isConnectedToPS3 )
      {
         // Receive ping message
         char msg = kMessageInvalid;
         ENetworkOperationResult res = ReceiveAndVerifyPacket_NetworkThread( &msg, sizeof(msg), false );
         if( res == kNOR_Disconnected || res == kNOR_VersionMismatch )
         {
            DisconnectFromPS3_NetworkThread();

            if( res == kNOR_VersionMismatch )
            {
               SET_TRANSFARRING_ERROR( m_wifiError, kTE_WifiVersionMismatch, 0 );
               WaitForErrorResolution_NetworkThread();
            }

            continue;
         }

         // Process message work type
         ProcessNetworkMessage_NetworkThread( msg );

         // Check the network state callback
         sceNetCtlCheckCallback();

         // If the access point was closed, disconnect
         if( !m_isConnectedToPS3 )
         {
            DisconnectFromPS3_NetworkThread();
         }

         // Wait for a bit and poll again
         mNetworkThreadSyncEvent.Wait( skDelayNetworkThreadTimeMilliseconds );
      }
      else
      {
         mNetworkThreadSyncEvent.Wait();
      }
   }

   mIsNetworkThreadRunning = false;

   m_status = kVTS_Uninitialized;

   return 0;
}

bool CTransfarringManager::CanPostNetworkThreadWork() const
{
   return !mNetworkThreadWorkPosted;
}

void CTransfarringManager::ProcessNetworkThreadWork_NetworkThread()
{
   if( mNetworkThreadWorkInput_WorkType == kNW_ConnectToPS3 )
   {
      if( !mAreFileListsValid )
      {
         LoadFileLists_NetworkThread();
      }

      ConnectToPS3_NetworkThread();
   }
   else if( mNetworkThreadWorkInput_WorkType == kNW_DisconnectFromPS3 )
   {
      DisconnectFromPS3_NetworkThread();
   }
   else if( mNetworkThreadWorkInput_WorkType == kNW_LoadFileList )
   {
      LoadFileLists_NetworkThread();
   }
   else if( mNetworkThreadWorkInput_WorkType == kNW_UnloadFileList )
   {
      UnloadFileLists_NetworkThread();
   }
   else if( mNetworkThreadWorkInput_WorkType == kNW_CleanupSystem )
   {
      CleanupSystem_NetworkThread();
   }

   mNetworkThreadWorkInput_WorkType = kNW_Idle;
   mNetworkThreadWorkPosted = false;
}

void CTransfarringManager::FinalizePostedWork_NetworkThread()
{
   // Start up network thread again
   mNetworkThreadWorkPosted = true;
   mNetworkThreadSyncEvent.Set();
}

void CTransfarringManager::LoadFileLists_NetworkThread()
{
   if( !mNeedsMemoryCardCleanup )
   {
      CreateMemoryCardActor_NetworkThread();
   }

   // Get the local file list
   m_LocalFileList.ClearSaveFiles();
   m_LocalFileList.LoadAllFileLists_SubThread();

   // TODO: This is not thread safe; TUS functions must be called from the game thread. Remove when cloud syncing is added to UI side.
   // Get the cloud file list
   STransfarringError cloudError;
   SET_TRANSFARRING_ERROR_INVALID(cloudError);
   if (BP_Network_IsSignedIn())
   {
      do
      {
         cloudError = ProcessGetCloudFileList();
         sceKernelDelayThread( skDelayNetworkThreadTimeMicroseconds );
      } while (cloudError.mHighLevelError == kTE_Processing);

      if (cloudError.mHighLevelError != kTE_Success)
      {
         //SET_TRANSFARRING_ERROR(m_wifiError, cloudError.mHighLevelError, 0);
         //WaitForErrorResolution();
      }
   }

   mAreFileListsValid = true;
}

void CTransfarringManager::UnloadFileLists_NetworkThread()
{
   m_LocalFileList.ClearSaveFiles();
   m_CloudFileList.ClearCloudFiles();

   DestroyMemoryCardActor_NetworkThread();

   mAreFileListsValid = false;
}

void CTransfarringManager::ConnectToPS3_NetworkThread()
{
   SceNetCheckDialogParam netcheckParam;
   SceNetCheckDialogPS3ConnectParam ps3Param;
   SceNetCheckDialogPS3ConnectInfo ps3info;

   // Set up PS3 connection parameters
   memset(&ps3Param, 0, sizeof(ps3Param));
   ps3Param.action = SCE_NETCHECK_DIALOG_PS3_CONNECT_ACTION_ENTER;
   strncpy(ps3Param.ssid, mPS3SKU, sizeof(ps3Param.ssid));
   strncpy(ps3Param.wpaKey, PS3_WPAKEY, sizeof(ps3Param.wpaKey));
   strncpy(ps3Param.titleId, mPS3SKU, sizeof(ps3Param.titleId));

   // Set up network check dialog
   sceNetCheckDialogParamInit(&netcheckParam);
   netcheckParam.mode = SCE_NETCHECK_DIALOG_MODE_PS3_CONNECT;
   netcheckParam.ps3ConnectParam = &ps3Param;

   int ret = 0;

   // It's possible that a common dialog is up from an error.
   do
   {
      ret = sceNetCheckDialogInit(&netcheckParam);
      sceKernelDelayThread( skDelayNetworkThreadTimeMicroseconds );
   } while( ret != SCE_OK );

   SceCommonDialogStatus cdStatus;
   SceNetCheckDialogResult netcheckResult;

   // Spin until the network check dialog is done
   for (;;)
   {
      cdStatus = sceNetCheckDialogGetStatus();
      if (cdStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
      {
         break;
      }
   }

   ret = sceNetCheckDialogGetResult(&netcheckResult);

   ret = sceNetCheckDialogGetPS3ConnectInfo(&ps3info);

   ret = sceNetCheckDialogTerm();

   if (netcheckResult.result == SCE_COMMON_DIALOG_RESULT_OK)
   {
      // Connected to the PS3 successfully
      // Register callback so we can detect when we lose our connection
      ret = sceNetCtlInetRegisterCallback(ps3connect_netctl_callback, NULL, &sCallbackID);

      // AS(JM) If we have an old socket, close it
      if ( s_sock != -1 )
      {
         sceNetSocketClose( s_sock );
      }

      s_sock = sceNetSocket("PS3Connect", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);

      SceNetSockaddrIn sendsin;
      memset(&sendsin, 0, sizeof(sendsin));
      sendsin.sin_family = SCE_NET_AF_INET;
      sendsin.sin_addr.s_addr = ps3info.inaddr.s_addr;
      sendsin.sin_port = sceNetHtons(5000);

      static const int skTimeoutUs = 90 * 1000000;   

      // AS(JM) - Make sure to set 90 sec timeout for send and recv
      {
         int timeout = skTimeoutUs;

         sceNetSetsockopt(s_sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_RCVTIMEO,
            &timeout, sizeof(timeout) );
      }

      {
         int timeout = skTimeoutUs;

         sceNetSetsockopt(s_sock, SCE_NET_SOL_SOCKET, SCE_NET_SO_SNDTIMEO,
            &timeout, sizeof(timeout) );
      }
      ret = sceNetConnect(s_sock, (SceNetSockaddr *)&sendsin, sizeof(sendsin));

      if( ret == SCE_OK )
      {
         m_isConnectedToPS3 = true;
         m_hasDisconnectedFromPS3 = false;
      }
      else
      {
         SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToConnectToPS3, 0 );
         WaitForErrorResolution_NetworkThread();
      }
   }
   else
   {
      SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToConnectToPS3, 0 );
      WaitForErrorResolution_NetworkThread();
   }
}

void CTransfarringManager::DisconnectFromPS3_NetworkThread()
{
   if(!m_hasDisconnectedFromPS3)
   {
      m_hasDisconnectedFromPS3 = true;

      // Send a message that kill the connection
      char killMsg = kMessageKillConnection;
      SendPacket_NetworkThread( &killMsg, sizeof(killMsg) );

      SceNetCheckDialogParam netcheckParam;
      SceNetCheckDialogPS3ConnectParam ps3Param;

      // Set up PS3 connection parameters
      memset(&ps3Param, 0, sizeof(ps3Param));
      ps3Param.action = SCE_NETCHECK_DIALOG_PS3_CONNECT_ACTION_LEAVE;
      strncpy(ps3Param.ssid, mPS3SKU, sizeof(ps3Param.ssid));
      strncpy(ps3Param.wpaKey, PS3_WPAKEY, sizeof(ps3Param.wpaKey));
      strncpy(ps3Param.titleId, mPS3SKU, sizeof(ps3Param.titleId));

      // Spin until success or error. Cancelling is not a valid option.
      for(;;)
      {
         // Set up network check dialog
         sceNetCheckDialogParamInit(&netcheckParam);
         netcheckParam.mode = SCE_NETCHECK_DIALOG_MODE_PS3_CONNECT;
         netcheckParam.ps3ConnectParam = &ps3Param;

         int ret = 0;

         // It's possible that a common dialog is up from an error.
         do
         {
            ret = sceNetCheckDialogInit(&netcheckParam);
            sceKernelDelayThread( skDelayNetworkThreadTimeMicroseconds );
         } while( ret != SCE_OK );

         SceCommonDialogStatus cdStatus;
         SceNetCheckDialogResult netcheckResult;

         // Spin until the network check dialog is done
         for (;;)
         {
            cdStatus = sceNetCheckDialogGetStatus();
            if (cdStatus == SCE_COMMON_DIALOG_STATUS_FINISHED)
            {
               break;
            }
         }

         ret = sceNetCheckDialogGetResult(&netcheckResult);
         ret = sceNetCheckDialogTerm();

         if( netcheckResult.result == SCE_COMMON_DIALOG_RESULT_OK )
         {
            break;
         }
         else if( netcheckResult.result != SCE_COMMON_DIALOG_RESULT_USER_CANCELED )
         {
            SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToDisconnectFromPS3, 0 );
            WaitForErrorResolution_NetworkThread();
            break;
         }
      }

      sceNetSocketClose(s_sock);
      s_sock = -1;

      sceNetCtlInetUnregisterCallback(sCallbackID);
      sCallbackID = -1;

      m_isConnectedToPS3 = false;
   }
}

void CTransfarringManager::ProcessNetworkMessage_NetworkThread( const char msg )
{
   int ret = 0;

   if (msg == kMessageSendFileToVita)
   {
      m_status = kVTS_ReceivingSaveFile;

      // Trying to keep this big structure off the stack
      boost::shared_ptr<TRANSFARRING_GAME_DATA> data(new TRANSFARRING_GAME_DATA);

      unsigned char transfarringID[TransfarringDefines::kHashLengthBytes];
      memset(transfarringID, 0, sizeof(transfarringID));

      ESaveType saveType = kST_MaxSaveTypes;

      char pingMessage = kMessageInvalid;
      ENetworkOperationResult res = ReceiveAndVerifyPacket_NetworkThread( &pingMessage, sizeof(pingMessage) );
      if( res == kNOR_Success && pingMessage == kMessageSendFileToVita )
      {
         res = ReceiveAndVerifyPacket_NetworkThread( data.get(), kSendDataSize );

         if( res == kNOR_Success )
         {
            res = ReceiveAndVerifyPacket_NetworkThread( transfarringID, sizeof( transfarringID ) );

            if( res == kNOR_Success )
            {
               res = ReceiveAndVerifyPacket_NetworkThread( &saveType, sizeof(ESaveType) );
            }
         }
      }

      // Only save the file if it was successfully received
      if( res == kNOR_Success && pingMessage == kMessageSendFileToVita )
      {
         HandleIncomingSaveFile( data.get(), true, transfarringID );

         StartProgress( 1 );

         TransfarringID newTID;
         newTID.MakeWifiFromUniqueID((const long long*)transfarringID);
         newTID.m_IsLocked = false;

         ESaveType dummySaveType = kST_MaxSaveTypes;
         int saveIndex = -1;

         GetSaveFileFromUniqueID( (const unsigned char*)newTID.m_UniqueID, dummySaveType, saveIndex );

         if( saveIndex == -1 || dummySaveType == kST_MaxSaveTypes )
         {
            saveIndex = m_LocalFileList.FindFirstAvailableSaveSlot(saveType);
         }

         BPE_ASSERT(saveIndex != -1, "TransfarringVTA received a save file when it had no save slots available, should not be possible");

         SetTransfarringID(newTID, saveType, saveIndex);

         TRANSFARRING_GAME_DATA* dataPtr = data.get();
         STransfarringError saveError = m_LocalFileList.SaveFile_SubThread(*dataPtr, saveType, saveIndex);

         // Notify the PS3 of our result
         char pingMessage = kMessageInvalid;
         char sendSaveIndex = saveIndex;

         if( saveError.mHighLevelError == kTE_Success )
         {
            pingMessage = kMessageReceivedFile;
         }
         else
         {
            sendSaveIndex = -1;
         }

         SendPacket_NetworkThread( &pingMessage, sizeof(pingMessage) );
         SendPacket_NetworkThread( &sendSaveIndex, sizeof(sendSaveIndex) );
      }
      else
      {
         SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToReceive, 0 );
         WaitForErrorResolution_NetworkThread();
      }
   }
   else if (msg == kMessageSendFileToPS3)
   {
      m_status = kVTS_SendingSaveFile;

      ESaveType saveType;
      int saveIndex = -1;

      ENetworkOperationResult res = ReceiveAndVerifyPacket_NetworkThread( &saveType, sizeof( ESaveType ) );

      if( res == kNOR_Success )
      {
         res = ReceiveAndVerifyPacket_NetworkThread( &saveIndex, sizeof( saveIndex ) );
      }

      if( res == kNOR_Success )
      {
         boost::shared_ptr<TRANSFARRING_GAME_DATA> data(new TRANSFARRING_GAME_DATA);
         TRANSFARRING_GAME_DATA* dataPtr = data.get();

         StartProgress( 2 );
         STransfarringError fileError = m_LocalFileList.LoadFile_SubThread(*dataPtr, saveType, saveIndex);

         // Mark as locked
         TransfarringID newTID = GetTransfarringID( saveType, saveIndex );
         if( newTID.m_Type == kTT_WiFi )
         {
            newTID.m_IsLocked = true;
         }
         else
         {
            newTID.MakeNewWifi( true );
         }

         bool newOriginatorData = TryWriteOriginatorData( &dataPtr->mBPLinkvars, saveType, saveIndex, false );
         if( newOriginatorData || BP_IsMD5HashZero( dataPtr->mBPLinkvars.mOriginalPSNAccount ) )
         {
            dataPtr->mBPLinkvars.mTrophyValidFlag = BPLinkvarDefines::kTrophiesInvalid;
         }

         SetTransfarringID( newTID, saveType, saveIndex );

         SendPacket_NetworkThread( dataPtr, sizeof(TRANSFARRING_GAME_DATA) );
         SendPacket_NetworkThread( newTID.m_UniqueID, TransfarringDefines::kHashLengthBytes );

         // Resave
         fileError = m_LocalFileList.SaveFile_SubThread( *dataPtr, saveType, saveIndex );

         char pingMessage = kMessageInvalid;
         res = ReceiveAndVerifyPacket_NetworkThread( &pingMessage, sizeof( pingMessage) );

         if( res == kNOR_Success && pingMessage == kMessageReceivedFile )
         {
            // Delete the file locally
            //m_LocalFileList.DeleteFile_SubThread(saveType, saveIndex);
         }
         else if( res == kNOR_Success && pingMessage == kMessageInvalid )
         {
            SET_TRANSFARRING_ERROR( m_wifiError, kTE_OutOfSpaceRemote, 0 );
            WaitForErrorResolution_NetworkThread();
         }
         else
         {
            SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToSend, 0 );
            WaitForErrorResolution_NetworkThread();
         }
      }
      else
      {
         SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToSend, 0 );
         WaitForErrorResolution_NetworkThread();
      }
   }
   else if (msg == kMessageSendFileListToPS3)
   {
      m_status = kVTS_SendingSaveList;

      // Since this is the first possible message received from the Vita, send a message back so we can ensure
      // we're both on the same WiFi protocol
      char pingMessage = kMessageInvalid;
      SendPacket_NetworkThread( &pingMessage, sizeof(pingMessage) );

      for (int saveType = kST_Game; saveType < kST_MaxSaveTypes; saveType++)
      {
         char smallFileNum = (char)m_LocalFileList.GetNumSaveFiles((ESaveType)saveType);
         SendPacket_NetworkThread( &smallFileNum, sizeof(smallFileNum) );

         for (char i=0; i < smallFileNum; i++)
         {
            TransfarringFileList::TTransfarringSaveFilePtr saveFile = m_LocalFileList.GetSaveFilePtr((ESaveType)saveType, i);

            SendPacket_NetworkThread( saveFile.get(), sizeof(TransfarringSaveFile) );

            char isCorrupt = m_LocalFileList.IsFileCorrupt( (ESaveType)saveType, i );
            SendPacket_NetworkThread( &isCorrupt, sizeof(isCorrupt) );

            if( !isCorrupt )
            {
               TransfarringID tid = GetTransfarringID( (ESaveType)saveType, saveFile->mSaveIndex );
               char isLinked = tid.m_Type != kTT_Invalid;
               SendPacket_NetworkThread( &isLinked, sizeof(isLinked) );
            }
            else
            {
               char isLinked = 0;
               SendPacket_NetworkThread( &isLinked, sizeof(isLinked) );
            }
         }
      }
   }
   else if( msg == kMessageUnlinkFile )
   {
      m_status = kVTS_UnlockingSaveFile;

      ESaveType saveType = kST_MaxSaveTypes;
      int saveIndex = -1;

      ENetworkOperationResult res = ReceiveAndVerifyPacket_NetworkThread( &saveType, sizeof( saveType ) );
      if( res == kNOR_Success )
      {
         res = ReceiveAndVerifyPacket_NetworkThread( &saveIndex, sizeof( saveIndex ) );
      }

      if( res == kNOR_Success )
      {
         TransfarringID curID = GetTransfarringID( saveType, saveIndex );

         BPE_ASSERT(curID.m_Type = kTT_WiFi, "Tried to unlink a non-wifi save file!");

         boost::shared_ptr<TRANSFARRING_GAME_DATA> data(new TRANSFARRING_GAME_DATA);
         TRANSFARRING_GAME_DATA* dataPtr = data.get();

         StartProgress( 2 );

         STransfarringError fileError = m_LocalFileList.LoadFile_SubThread(*dataPtr, saveType, saveIndex);

         if( fileError.mHighLevelError == kTE_Success )
         {
            // Make an invalid transfarring ID
            curID.Invalidate();
            SetTransfarringID(curID, saveType, saveIndex);

            fileError = m_LocalFileList.SaveFile_SubThread(*dataPtr, saveType, saveIndex);
         }
         
         if( fileError.mHighLevelError == kTE_Success )
         {
            char successMsg = kMessageFileUnlinked;
            SendPacket_NetworkThread( &successMsg, sizeof(successMsg) );
         }
         else
         {
            char failureMsg = kMessageInvalid;
            SendPacket_NetworkThread( &failureMsg, sizeof(failureMsg) );

            SET_TRANSFARRING_ERROR( m_wifiError, kTE_FailedToReceive, 0 );
            WaitForErrorResolution_NetworkThread();
         }
      }
      else
      {
         // Handle disconnection
         SET_TRANSFARRING_ERROR( m_wifiError, kTE_Unknown, 0 );
         WaitForErrorResolution_NetworkThread();
      }
   }
   else if( msg == kMessageKillConnection )
   {
      DisconnectFromPS3_NetworkThread();
   }

   m_status = kVTS_Idle;
}

void CTransfarringManager::CleanupSystem_NetworkThread()
{
   if( mNeedsMemoryCardCleanup )
   {
      UnloadFileLists_NetworkThread();
   }
}

void CTransfarringManager::SendPacket_NetworkThread( void const * const pData, unsigned int dataSize ) const
{
   // Make the salted hash
   unsigned char localHash[TransfarringDefines::kHashLengthBytes];
   CreateSaltedHash( localHash, pData, dataSize, mSaltString, sizeof(mSaltString) );

   // Send the salted hash
   sceNetSend( s_sock, localHash, sizeof(localHash), 0 );

   // Send the data
   sceNetSend( s_sock, pData, dataSize, 0 );
}

CTransfarringManager::ENetworkOperationResult CTransfarringManager::ReceiveAndVerifyPacket_NetworkThread( void * const pData, unsigned int dataSize, bool shouldWait ) const
{
   ENetworkOperationResult receiveSuccess = kNOR_MaxNumOperations;

   unsigned char hashBuffer[TransfarringDefines::kHashLengthBytes];

   // Receive a hash plus the packet
   int hashSizeToReceive = sizeof(hashBuffer);
   int recvLen = 0;

   do
   {
      recvLen = sceNetRecv( s_sock, hashBuffer, hashSizeToReceive, shouldWait ? SCE_NET_MSG_WAITALL : SCE_NET_MSG_DONTWAIT );
      if( recvLen > 0 )
      {
         hashSizeToReceive -= recvLen;
      }
   } while (recvLen > 0 && hashSizeToReceive > 0);

   if( hashSizeToReceive == 0 )
   {
      // Force a wait on the second recv
      recvLen = sceNetRecv( s_sock, pData, dataSize, SCE_NET_MSG_WAITALL );

      if( recvLen == dataSize )
      {
         // Hash the packet contents
         unsigned char localHash[TransfarringDefines::kHashLengthBytes];

         CreateSaltedHash( localHash, pData, dataSize, mSaltString, sizeof(mSaltString) );

         if( BP_CompareMD5Hashes( localHash, hashBuffer ) )
         {
            receiveSuccess = kNOR_Success;
         }
         else
         {
            receiveSuccess = kNOR_VersionMismatch;
         }
      }
      else
      {
         receiveSuccess = kNOR_Disconnected;
      }
   }
   //AS if recvLen < 0 && shouldWait, then a TCP timeout happened in the first call to sceNetRecv
   else if( recvLen == 0 || ( recvLen < 0 && ( sce_net_errno != SCE_NET_EAGAIN || shouldWait ) ) )
   {
      receiveSuccess = kNOR_Disconnected;
   }
   else if( !shouldWait )
   {
      receiveSuccess = kNOR_NoDataReceived;
   }

   //AS Kill the socket
   if( receiveSuccess == kNOR_Disconnected )
   {
      sceNetSocketClose(s_sock);
      s_sock = -1;
   }

   BPE_VERIFY( receiveSuccess != kNOR_MaxNumOperations, false, "ReceiveAndVerifyPacket_NetworkThread did not handle a message" );
   return receiveSuccess;
}

void CTransfarringManager::CreateSaltedHash( 
   void * const pHash, 
   void const * const pData, 
   unsigned int dataSize, 
   void const * const pSalt, 
   unsigned int saltSize ) const
{
   // Use the MD5 library directly since we're hashing non-contiguous data
   md5_state_s state;
   md5_init( &state );

   md5_append( &state, (const md5_byte_t*)pSalt, saltSize );
   md5_append( &state, (const md5_byte_t*)pData, dataSize );

   md5_finish( &state, (md5_byte_t*)pHash );
}

void CTransfarringManager::LockSaveLoadMutex()
{
   sceKernelLockMutex(gMemoryCardMutex, 1, NULL);
}

void CTransfarringManager::UnlockSaveLoadMutex()
{
   sceKernelUnlockMutex(gMemoryCardMutex, 1);
}

EVitaTransfarringStatus CTransfarringManager::GetTransfarringStatus()
{
   return m_status;
}

void CTransfarringManager::SetTransfarringStatus(EVitaTransfarringStatus status)
{
   m_status = status;
}

int CTransfarringManager::GetNumLocalSaveFiles(ESaveType saveType)
{
   return m_LocalFileList.GetNumSaveFiles(saveType);
}

CTransfarringManager::TTransfarringSaveFilePtr const CTransfarringManager::GetLocalSaveFile(ESaveType saveType, ConcatenatedSaveIndex saveIndex)
{
   return m_LocalFileList.GetSaveFilePtr(saveType, saveIndex);
}

int CTransfarringManager::GetNumCloudSaveFiles(ESaveType saveType)
{
   return m_CloudFileList.GetNumCloudFilesForType(saveType);
}

CTransfarringManager::TTransfarringSaveFilePtr const CTransfarringManager::GetCloudSaveFile(ESaveType saveType, ConcatenatedSaveIndex saveIndex)
{
   return m_CloudFileList.GetCloudFilePtr(saveType, saveIndex);
}

void CTransfarringManager::NetworkCallback(int eventType, void *arg)
{
   (void)arg;
   if (eventType == SCE_NET_CTL_EVENT_TYPE_DISCONNECTED)
   {
      m_isConnectedToPS3 = false;
   }
   else if (eventType == SCE_NET_CTL_EVENT_TYPE_IPOBTAINED)
   {
      m_isConnectedToPS3 = true;
      m_hasDisconnectedFromPS3 = false;
   }
}

// Handle disconnection
static void ps3connect_netctl_callback(int eventType, void *arg)
{
   CTransfarringManager::Instance()->NetworkCallback(eventType, arg);
}
