#pragma once

//----------------------------------------------------------------------------
// BP_Camera.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#if BP_ENABLE_DEBUG_MENU
#  define BP_ENABLE_CAMERA_DEBUG_MENU 0
#endif

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

// Camera settings mode
enum ECameraSettingsMode
{
   kCSM_Wide_Variable,
   kCSM_Wide_ExpandWidth,
   kCSM_Wide_ShrinkHeight,
   kCSM_Full,

   kCSM_Count
};

// Script event types
enum EScriptEventType
{
   kScriptEvent_Default,
   kScriptEvent_EnterTrap,
   kScriptEvent_LeaveTrap,

   kScriptEvent_Count
};

//----------------------------------------------------------------------------
// STRUCTURES
//----------------------------------------------------------------------------

#ifdef __cplusplus

// Version enums to allow unique constructors
enum ECamDefVersion1
{
   kCamDefV1
};
enum ECamDefVersion2
{
   kCamDefV2
};

enum ECamDefVersion3
{
   kCamDefV3
};

enum ECamDefVersion4
{
   kCamDefV4
};

// Init camera settings util structure
struct SBP_CameraDef
{
   // Constructors
   SBP_CameraDef()
   {
      mAreaName   = 0;
      mCameraName = 0;
      mCameraId   = 0;
      SetDefaults();
   }
   
   SBP_CameraDef(ECamDefVersion1, const char* areaName, u_int cameraName, u_int cameraOldId, int cameraMode, float cameraRatio, float cameraOriginX, float cameraOriginY )
   {
      mAreaName       = areaName;
      mCameraName     = cameraName;
      mCameraId       = cameraOldId;
      mCameraMode     = cameraMode;
      mCameraRatio    = cameraRatio;
      mCameraOriginX  = cameraOriginX;
      mCameraOriginY  = cameraOriginY;

      // Convert old cinema tick to new cinema frame
      if( ( mCameraId & 0xff000000 ) == 0 )
      {
         mCameraId /= 5;
      }
   }

   SBP_CameraDef(ECamDefVersion2, const char* areaName, u_int cameraName, u_int cameraScriptOldId, u_int cameraScriptNewId, int cameraMode, float cameraRatio, float cameraOriginX, float cameraOriginY )
   {
      mAreaName       = areaName;
      mCameraName     = cameraName;
      mCameraId       = cameraScriptNewId;
      mCameraMode     = cameraMode;
      mCameraRatio    = cameraRatio;
      mCameraOriginX  = cameraOriginX;
      mCameraOriginY  = cameraOriginY;

      // Convert old cinema tick to new cinema frame
      if( ( mCameraId & 0xff000000 ) == 0 )
      {
         mCameraId /= 5;
      }
   }

   SBP_CameraDef(ECamDefVersion3, const char* areaName, u_int cameraName, u_int cameraScriptOldId, u_int cameraScriptNewId, int cameraMode, float cameraRatio, float cameraOriginX, float cameraOriginY )
   {
      mAreaName       = areaName;
      mCameraName     = cameraName;
      mCameraId       = cameraScriptNewId;
      mCameraMode     = cameraMode;
      mCameraRatio    = cameraRatio;
      mCameraOriginX  = cameraOriginX;
      mCameraOriginY  = cameraOriginY;

      // Convert old cinema tick to new cinema frame
      if( ( mCameraId & 0xff000000 ) == 0 )
      {
         mCameraId /= 5;
      }
   }

   SBP_CameraDef(ECamDefVersion4, const char* areaName, u_int cameraName, u_int cameraId, int cameraMode, float cameraRatio, float cameraOriginX, float cameraOriginY )
   {
      mAreaName       = areaName;
      mCameraName     = cameraName;
      mCameraId       = cameraId;
      mCameraMode     = cameraMode;
      mCameraRatio    = cameraRatio;
      mCameraOriginX  = cameraOriginX;
      mCameraOriginY  = cameraOriginY;
   }

   void SetDefaults()
   {
      mCameraMode     = kCSM_Wide_Variable;
      mCameraRatio    = 0.0f;
      mCameraOriginX  = 0.0f;
      mCameraOriginY  = 0.0f;
   }

   bool const operator < (SBP_CameraDef const & rhs) const
   {
      // 1. Sort by area name
      int areaCompare = strcmp( mAreaName, rhs.mAreaName );
      if( areaCompare < 0 )
      {
         return 1;
      }
      else if( areaCompare > 0 )
      {
         return 0;
      }

      // 2. Sort by camera name
      if( mCameraName < rhs.mCameraName )
      {
         return 1;
      }
      else if( mCameraName > rhs.mCameraName )
      {
         return 0;
      }

      // 3. Sort by camera id
      if( mCameraId < rhs.mCameraId )
      {
         return 1;
      }

      return 0;
   }

   // Data
   const char* mAreaName;                    // Area name
   u_int       mCameraName;                  // Camera name
   u_int       mCameraId;                    // Camera id
   int         mCameraMode;                  // Camera mode
   float       mCameraRatio;                 // Camera ratio (0=HD, 1 = full)
   float       mCameraOriginX;               // Camera origin X (-1 = left, 0 = center, -1 = right)
   float       mCameraOriginY;               // Camera origin Y (-1 = top,  0 = center, +1 = bottom)
};

#else

// Init camera settings util structure
typedef struct _SBP_CameraDef
{
   // Data
   const char* mAreaName;        // Area name
   u_int       mCameraName;      // Camera name
   u_int       mCameraId;        // Camera id
   int         mCameraMode;      // Camera mode
   float       mCameraRatio;     // Camera ratio (0=HD, 1 = full)
   float       mCameraOriginX;   // Camera origin X (-1 = left, 0 = center, -1 = right)
   float       mCameraOriginY;   // Camera origin Y (-1 = top,  0 = center, +1 = bottom)
} SBP_CameraDef;

#endif

//----------------------------------------------------------------------------

// Tweakable camera settings
typedef ALIGN16_DECL(struct) _SBP_CameraSettings
{
   // Static
   SBP_CameraDef* def;        // Definition
   u_int          id;         // Id

   // Auxiliary vars computed at runtime and used by perspective matrix
   float          offsetX;    // Offset X
   float          offsetY;    // Offset Y
   float          aspectX;    // Aspect X
   float          aspectY;    // Aspect Y

} SBP_CameraSettings;

//----------------------------------------------------------------------------
// CINEMA OVERRIDES
//----------------------------------------------------------------------------

// Cinema mode
enum ECinemaMode
{
   kCinema_Mode_FullScreen,   // Full screen zoomed in with no cinema bars
   kCinema_Mode_WideScreen,   // Wide screen (with optional cinema bars)
   kCinema_Mode_Options,      // Wide screen = no zoom, Full screen = zoom
};

// Cinema bar control
enum ECinemaBars
{
   kCinema_Bars_Options,      // Wide screen = On, Full screen = Off
   kCinema_Bars_Off,          // Force bars always off
   kCinema_Bars_On            // Force bars always on
};

//----------------------------------------------------------------------------

// Cinema definition
typedef struct _SBP_CinemaDef
{
   const char* mAreaName;     // Area name
   u_int       mCinemaId;     // Id
   const char* mCinemaName;   // Name of cinema
   int         mCinemaBars;   // Cinema bars state
   int         mCinemaMode;   // Cinema mode state
} SBP_CinemaDef;

//----------------------------------------------------------------------------

// Cut scene definition
typedef struct _SBP_CutsceneDef
{
   const char* mAreaName;     // Area name
   u_int       mCameraName;   // Camera name
   u_int       mCameraId;     // Camera id
   int         mCinemaBars;   // Cinema bars state
   int         mCinemaMode;   // Cinema mode state
} SBP_CutsceneDef;

//----------------------------------------------------------------------------
// GAME INCLUDES
//----------------------------------------------------------------------------

#if MGS_VERSION == 2
#include "BP_CameraMGS2.h"
#endif

#if MGS_VERSION == 3
#include "BP_CameraMGS3.h"
#endif


//----------------------------------------------------------------------------
// C FUNCTIONS
//----------------------------------------------------------------------------

// Misc
void BP_Camera_Init();
void BP_Camera_InitDebugMenu();
void BP_Camera_Update();

// Rendering
float BP_GetScreenAspectX();
float BP_GetScreenAspectY();
float BP_GetScreenOffsetX();
float BP_GetScreenOffsetY();

// Cinema
void BP_Camera_Cinemabars_SetState( const int active, const int barTop, const int barBot, const float alpha );
int BP_Camera_Cinemabars_AreEnabled();
int BP_Camera_Cinema_IsFullScreen();

// Camera
void BP_Camera_SetScriptEventType( const int scriptEventType );
void BP_Camera_SetIdFromScript( MGS_Camera* cam );
void BP_Camera_SetIdFromDemo( MGS_Camera* cam );
u_int BP_Camera_GetCameraId();
u_int BP_Camera_GetDemoId();
void BP_Camera_SetActiveCamera( MGS_Camera* cam );
void BP_Camera_SetCodecCamera();

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
