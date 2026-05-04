//----------------------------------------------------------------------------
// BP_NetworkVTA.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/COsContext.h"

#include "BP_Network.h"

#include <stdio.h>
#include <sdk_version.h>
#include <np.h>
#include <np/np_manager.h>
#include <netcheck_dialog.h>
#include <apputil.h>

#include "BP_CommonDialog.h"
#include "Transfarring_UI_Strings.h"
#include "BP_Misc.h"

#if MGS_VERSION==2

static const SceNpCommunicationId sNpCommunicationId = {
   {'N', 'P', 'W', 'R', '0', '2', '3', '8', '0'},
   '\0',
   0,
   0
};

/***
SceNpCommunicationPassphrase

480c4e2056b6a272276b35c7b4d500982537c4d8db644e3d289b70ad98bc61f43e4ff9f708f7b124a5d5aee092d4ec614cdef552379274eaa79c7b91b56d855d6603574bf8d3d0403673ddce1637f8d2a5f4273aad83625e5119a4217d43272e8c41137e41520c8b0f7afe1fda8e89dff90deb11b24f9061fbe2aa9acfbdaa44
***/

static const SceNpCommunicationPassphrase sNpCommunicationPassphrase = {
   {
      0x48,0x0c,0x4e,0x20,0x56,0xb6,0xa2,0x72,
         0x27,0x6b,0x35,0xc7,0xb4,0xd5,0x00,0x98,
         0x25,0x37,0xc4,0xd8,0xdb,0x64,0x4e,0x3d,
         0x28,0x9b,0x70,0xad,0x98,0xbc,0x61,0xf4,
         0x3e,0x4f,0xf9,0xf7,0x08,0xf7,0xb1,0x24,
         0xa5,0xd5,0xae,0xe0,0x92,0xd4,0xec,0x61,
         0x4c,0xde,0xf5,0x52,0x37,0x92,0x74,0xea,
         0xa7,0x9c,0x7b,0x91,0xb5,0x6d,0x85,0x5d,
         0x66,0x03,0x57,0x4b,0xf8,0xd3,0xd0,0x40,
         0x36,0x73,0xdd,0xce,0x16,0x37,0xf8,0xd2,
         0xa5,0xf4,0x27,0x3a,0xad,0x83,0x62,0x5e,
         0x51,0x19,0xa4,0x21,0x7d,0x43,0x27,0x2e,
         0x8c,0x41,0x13,0x7e,0x41,0x52,0x0c,0x8b,
         0x0f,0x7a,0xfe,0x1f,0xda,0x8e,0x89,0xdf,
         0xf9,0x0d,0xeb,0x11,0xb2,0x4f,0x90,0x61,
         0xfb,0xe2,0xaa,0x9a,0xcf,0xbd,0xaa,0x44
   }
};

/***
SceNpCommunicationSignature

b9dde13b01000000000000008a4869c65431c16011a963d549e0e75395fa7fde4148e18d538b73cef2d05163c0160aa048e65a462aaec8e7539af9c4db08b115af84160916240769c466d38aa1008d587be3c16fe5bd25c294ba9b4ca3b3ccbfee29ced2d8e6d11efadec35d485970130c8394e4ede4baa2c30b8a79d840aae54e15ce9a1090424b4a18442a0ea3e7a6264e3073391491b31b4150bb36753cf5
***/

static const SceNpCommunicationSignature sNpCommunicationSignature = {
   {
      0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
         0x00,0x00,0x00,0x00,0x8a,0x48,0x69,0xc6,
         0x54,0x31,0xc1,0x60,0x11,0xa9,0x63,0xd5,
         0x49,0xe0,0xe7,0x53,0x95,0xfa,0x7f,0xde,
         0x41,0x48,0xe1,0x8d,0x53,0x8b,0x73,0xce,
         0xf2,0xd0,0x51,0x63,0xc0,0x16,0x0a,0xa0,
         0x48,0xe6,0x5a,0x46,0x2a,0xae,0xc8,0xe7,
         0x53,0x9a,0xf9,0xc4,0xdb,0x08,0xb1,0x15,
         0xaf,0x84,0x16,0x09,0x16,0x24,0x07,0x69,
         0xc4,0x66,0xd3,0x8a,0xa1,0x00,0x8d,0x58,
         0x7b,0xe3,0xc1,0x6f,0xe5,0xbd,0x25,0xc2,
         0x94,0xba,0x9b,0x4c,0xa3,0xb3,0xcc,0xbf,
         0xee,0x29,0xce,0xd2,0xd8,0xe6,0xd1,0x1e,
         0xfa,0xde,0xc3,0x5d,0x48,0x59,0x70,0x13,
         0x0c,0x83,0x94,0xe4,0xed,0xe4,0xba,0xa2,
         0xc3,0x0b,0x8a,0x79,0xd8,0x40,0xaa,0xe5,
         0x4e,0x15,0xce,0x9a,0x10,0x90,0x42,0x4b,
         0x4a,0x18,0x44,0x2a,0x0e,0xa3,0xe7,0xa6,
         0x26,0x4e,0x30,0x73,0x39,0x14,0x91,0xb3,
         0x1b,0x41,0x50,0xbb,0x36,0x75,0x3c,0xf5
   }
};

#if BP_VITA //AndyO: SDK360?
static const SceNpCommunicationConfig s_npCommunicationConfig =
{
   &sNpCommunicationId,
   &sNpCommunicationPassphrase,
   &sNpCommunicationSignature
};
#endif

#endif

//----------------------------------------------------------------------------

#if MGS_VERSION==3

static const SceNpCommunicationId sNpCommunicationId = {
   {'N', 'P', 'W', 'R', '0', '2', '3', '8', '1'},
   '\0',
   0,
   0
};

/***
SceNpCommunicationPassphrase

ba7e4c519a1c1d581f42ea4bd7e70aa805cdaca1072ef5a2182d2fe1b47c8da7493808f82978cfffce0ff8d7297c9a2cfdea63f85001edeecd6d065855c21d24af7e19e9c795b11bdf354727cfaa8eeabca8e281c7097531d439aefe51eedba96f2f0978335ac6e4db4b4f3661278465b611c7e37e0a2b2dd5dbcc4c1b30b207
***/

static const SceNpCommunicationPassphrase sNpCommunicationPassphrase = {
   {
      0xba,0x7e,0x4c,0x51,0x9a,0x1c,0x1d,0x58,
         0x1f,0x42,0xea,0x4b,0xd7,0xe7,0x0a,0xa8,
         0x05,0xcd,0xac,0xa1,0x07,0x2e,0xf5,0xa2,
         0x18,0x2d,0x2f,0xe1,0xb4,0x7c,0x8d,0xa7,
         0x49,0x38,0x08,0xf8,0x29,0x78,0xcf,0xff,
         0xce,0x0f,0xf8,0xd7,0x29,0x7c,0x9a,0x2c,
         0xfd,0xea,0x63,0xf8,0x50,0x01,0xed,0xee,
         0xcd,0x6d,0x06,0x58,0x55,0xc2,0x1d,0x24,
         0xaf,0x7e,0x19,0xe9,0xc7,0x95,0xb1,0x1b,
         0xdf,0x35,0x47,0x27,0xcf,0xaa,0x8e,0xea,
         0xbc,0xa8,0xe2,0x81,0xc7,0x09,0x75,0x31,
         0xd4,0x39,0xae,0xfe,0x51,0xee,0xdb,0xa9,
         0x6f,0x2f,0x09,0x78,0x33,0x5a,0xc6,0xe4,
         0xdb,0x4b,0x4f,0x36,0x61,0x27,0x84,0x65,
         0xb6,0x11,0xc7,0xe3,0x7e,0x0a,0x2b,0x2d,
         0xd5,0xdb,0xcc,0x4c,0x1b,0x30,0xb2,0x07
   }
};

/***
SceNpCommunicationSignature

b9dde13b0100000000000000ad3948c7b8b6ad9222db2fcbc951b06ecfccef890313e4b0ab80b3ed0679524f46f916c9ef9d2b589059a06b78bd9d7f56ebc9ff3c709cc1d1abbee30756f5c5cb2c351d2d0eabdbac298d431c3cadd023231c5b8d0d42f50481092e7d29ff8b480d09b89e9e26c0455aba166c1576ed28c0662d972f2b44dc33613293333537a766ed16539ab528dcf3e09916ba15210980971b
***/

static const SceNpCommunicationSignature sNpCommunicationSignature = {
   {
      0xb9,0xdd,0xe1,0x3b,0x01,0x00,0x00,0x00,
         0x00,0x00,0x00,0x00,0xad,0x39,0x48,0xc7,
         0xb8,0xb6,0xad,0x92,0x22,0xdb,0x2f,0xcb,
         0xc9,0x51,0xb0,0x6e,0xcf,0xcc,0xef,0x89,
         0x03,0x13,0xe4,0xb0,0xab,0x80,0xb3,0xed,
         0x06,0x79,0x52,0x4f,0x46,0xf9,0x16,0xc9,
         0xef,0x9d,0x2b,0x58,0x90,0x59,0xa0,0x6b,
         0x78,0xbd,0x9d,0x7f,0x56,0xeb,0xc9,0xff,
         0x3c,0x70,0x9c,0xc1,0xd1,0xab,0xbe,0xe3,
         0x07,0x56,0xf5,0xc5,0xcb,0x2c,0x35,0x1d,
         0x2d,0x0e,0xab,0xdb,0xac,0x29,0x8d,0x43,
         0x1c,0x3c,0xad,0xd0,0x23,0x23,0x1c,0x5b,
         0x8d,0x0d,0x42,0xf5,0x04,0x81,0x09,0x2e,
         0x7d,0x29,0xff,0x8b,0x48,0x0d,0x09,0xb8,
         0x9e,0x9e,0x26,0xc0,0x45,0x5a,0xba,0x16,
         0x6c,0x15,0x76,0xed,0x28,0xc0,0x66,0x2d,
         0x97,0x2f,0x2b,0x44,0xdc,0x33,0x61,0x32,
         0x93,0x33,0x35,0x37,0xa7,0x66,0xed,0x16,
         0x53,0x9a,0xb5,0x28,0xdc,0xf3,0xe0,0x99,
         0x16,0xba,0x15,0x21,0x09,0x80,0x97,0x1b
   }
};

#if BP_VITA //AndyO: SDK360?
static const SceNpCommunicationConfig s_npCommunicationConfig =
{
   &sNpCommunicationId,
   &sNpCommunicationPassphrase,
   &sNpCommunicationSignature
};
#endif

#endif

namespace
{
   SceNpServiceState sServiceState = SCE_NP_SERVICE_STATE_UNKNOWN;
   bool sUserNPIDValid = false;
   SceNpId sUserNPID;
   char sUserName[SCE_SYSTEM_PARAM_USER_NAME_MAXSIZE]; // Local user name
   static bool sNetworkInitialized = false;
   static int sNetcheckDialogHandle = 0;

   static int _BP_Network_GetAgeForSKU()
   {
      switch ( gpOsContext->mBuildSKU )
      {
      case COsContext::kBS_USA:
         // ESRB M - 17+
         return 17;
      case COsContext::kBS_Europe:
         // PEGI 18+
         return 18;
      case COsContext::kBS_Japan:
         // MGS2 Standalone is CERO C - 15+
         // Everything else (Bundle, MGS3 Standalone) is CERO D - 17+

#if MGS_VERSION==2
         if ( BP_IsDownloadableVersion() )
         {
            // MGS2 downloadable - CERO C
            return 15;
         }
#elif MGS_VERSION==3
         // Do nothing for 3
#else
#  error Unknown MGS version
#endif

         // CERO D - 17+
         return 17;
      }

      // If all else fails, 18
      return 18;
   }

   static int _BP_Network_IsSignedOutOrSignedInWithValidAge()
   {
      int isRestricted = 0;
      int age = 0;

      // Returns true if not signed in!

      if ( SCE_OK == sceNpManagerGetContentRatingFlag( &isRestricted, &age ) )
      {
         if ( isRestricted && age < _BP_Network_GetAgeForSKU() )
         {
            printf( "PSN AGE: FAIL %d < %d\n", age, _BP_Network_GetAgeForSKU() );

            return 0;
         }
         else
         {
            printf( "PSN AGE: PASS (ir: %d age: %d wa: %d)\n", isRestricted, age, _BP_Network_GetAgeForSKU() );

            return 1;
         }
      }
      else
      {
         printf( "PSN AGE: API FAIL\n" );
         return 1;
      }
   }

   void myNpServiceStateCallback(
      SceNpServiceState _state,
      int retCode,
      void *userdata
      )
   {
      SceNpServiceState const state = 
         _BP_Network_IsSignedOutOrSignedInWithValidAge() ? _state : SCE_NP_SERVICE_STATE_UNKNOWN;

      if ( state >= SCE_NP_SERVICE_STATE_ONLINE && sServiceState < SCE_NP_SERVICE_STATE_ONLINE )
      {
         SceNpManagerCacheParam cache;

         sUserNPIDValid = false;
         if ( SCE_OK == sceNpManagerGetNpId( &sUserNPID ) )
         {
            // sUserNPID.handle.data is a string, so if it's
            // empty we have an invalid id.
            if ( sUserNPID.handle.data[0] )
            {
               sUserNPIDValid = true;
            }
         }

         if ( !sUserNPIDValid )
         {
            sServiceState = SCE_NP_SERVICE_STATE_UNKNOWN;
            return;
         }
      }
      else if( state < SCE_NP_SERVICE_STATE_ONLINE && sServiceState >= SCE_NP_SERVICE_STATE_ONLINE )
      {
         BP_CommonDialog_WantsMessageDialog(GetTransfarringString(kTString_PsnConnectionLost), kMDL_OK);
      }
      sServiceState = state;
   }
}

void BP_Network_Init()
{
   if ( !sNetworkInitialized )
   {
      sNetworkInitialized = true;

      BPE_CHECK_SCE( sceNpInit( &s_npCommunicationConfig, NULL ) );

      sUserNPIDValid = false;
      memset( &sUserNPID, 1, sizeof( sUserNPID ) );

      BPE_CHECK_SCE( sceNpRegisterServiceStateCallback( &myNpServiceStateCallback, NULL ) );

      sceAppUtilSystemParamGetString(SCE_SYSTEM_PARAM_ID_USER_NAME, (SceChar8*)sUserName, SCE_SYSTEM_PARAM_USER_NAME_MAXSIZE);
   }
}

void BP_Network_HeartBeat()
{
   int const returnValue = sceNpCheckCallback();

   if ( returnValue < 0 )
   {
      printf( "BP_Network_HeartBeat() - Error code: %d\n", returnValue );
   }
}

int BP_Network_IsSignedIn()
{
   return ( sServiceState >= SCE_NP_SERVICE_STATE_ONLINE ) && sUserNPIDValid;
}

int BP_Network_Vita_IsPSNSignedIn()
{
   // AS(FR) - Returns true if we're in the Signed In state, regardless of 
   // connected state.  Other signin states check connected.
   return ( sServiceState >= SCE_NP_SERVICE_STATE_SIGNED_IN );
}

SceNpCommunicationConfig const *BP_Network_GetTUSNPConfig()
{
   return &s_npCommunicationConfig;
}

SceNpId const *BP_Network_GetNPID()
{
   // Return NP ID if the user is signed in, or NULL
   // This is useful to pass NULL to NP functions

   return BP_Network_IsSignedIn() ? ( &sUserNPID ) : NULL;
}

void BP_Network_GetNPIDCached( SceNpId *id, int *pIsValid )
{
   *id = sUserNPID;
   *pIsValid = sUserNPIDValid;
}

int BP_Network_Dialog_BeginPSN()
{
   if ( sceNetCheckDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_NONE )
   {
      printf( "BP_Network_Dialog_BeginPSN: Common dialog is still running\n" );
      return 0;
   }

   SceNetCheckDialogParam param;

   sceNetCheckDialogParamInit( &param );
   param.mode = SCE_NETCHECK_DIALOG_MODE_PSN_ONLINE;
   param.npCommunicationId = *( BP_Network_GetTUSNPConfig()->commId );

   int result = sceNetCheckDialogInit( &param );
   if ( result == SCE_OK )
   {
      return ++sNetcheckDialogHandle;
   }
   else
   {
      printf( "BP_Network_Dialog_BeginPSN: Error %8.8x %d starting dialog!\n", result, result );
      return 0;
   }
}

int BP_Network_Dialog_IsStillProcessing( int handle )
{
   if ( sNetcheckDialogHandle == handle )
   {
      return sceNetCheckDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_RUNNING;
   }
   else
   {
      return 0;
   }
}

int BP_Network_Dialog_GetResults( int handle, int *result )
{
   BPE_VERIFY( !BP_Network_Dialog_IsStillProcessing( handle ), false, "Can't get netcheck results while still processing" );

   // Set the result just in case someone's checking it wrong
   *result = -1;

   if ( handle == sNetcheckDialogHandle && sceNetCheckDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED )
   {
      SceNetCheckDialogResult dlgResult = { -1 };
      int sceResult = sceNetCheckDialogGetResult( &dlgResult );
      BPE_CHECK_SCE( sceNetCheckDialogTerm() );

      if ( sceResult >= 0 )
      {

         // If we're checking the results and the age doesn't work out, then
         // put up a message box!

         if ( !_BP_Network_IsSignedOutOrSignedInWithValidAge() )
         {
            BP_CommonDialog_WantsMessageDialog( BP_GetCustomOverrideString( "TRANSFARRING", "PSN_JAILBAIT" ), kMDL_OK);
         }

         *result = dlgResult.result;
         return 1;
      }
      else
      {
         return 0;
      }
   }
   else
   {
      // Invalid handle?  bad status?
      return 0;
   }
}

const char* BP_Network_GetCurrentUserName()
{
   return sUserName;
}