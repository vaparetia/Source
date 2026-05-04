///----------------------------------------------------------------------------
// BP_Camera.cpp
//----------------------------------------------------------------------------

///----------------------------------------------------------------------------
// BP INCLUDES
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Engine/Mechanics/CCRC.h"
#include "BP_RendererDebug.h"
#include "MGS_Common.h"
#include "BP_Camera.h"
#include "BP_Math.h"

//----------------------------------------------------------------------------
// GAME INCLUDES
//----------------------------------------------------------------------------

extern "C"
{
#if MGS_VERSION == 2
   #include "camera.h"
   #include "../../mgs2x/source/user/mode/demo/libdemo.h"
#endif

#if MGS_VERSION == 3
   #include "../../source/user/mode/demo/demo.h"
#ifndef GOLD_VERSION
   extern int gShowDebugCameraCaption;
#endif
   extern int gBP_1stPersonCamera_EnableMovement;
#endif
};

// this is for the vita version, but we'll change it in the pc build too since the data is the same
#define REMOVE_LETTERBOX_OPTION 1

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

#define BP_CAMERA_PRINT_SCRIPT_ID_DATA    0

#define BP_CAMERA_MAX_DEBUG_CAMERA_DEFS   3000
#define BP_CAMERA_MAX_DEBUG_AREA_NAMES    1000

//----------------------------------------------------------------------------
// DEBUG MENU DATA
//----------------------------------------------------------------------------

static const char* gCameraSettings_Mode_MenuStrings[kCSM_Count] =
{
   "Wide-Variable",
   "Wide-ExpandWidth",
   "Wide-ShrinkHeight",
   "Full-Original PS2"
};

//----------------------------------------------------------------------------

static const char* gCameraSettings_Mode_EnumStrings[kCSM_Count] =
{
   "kCSM_Wide_Variable",
   "kCSM_Wide_ExpandWidth",
   "kCSM_Wide_ShrinkHeight",
   "kCSM_Full"
};

//----------------------------------------------------------------------------

static const char* gCameraSettings_Use_Strings[2] =
{
   "Global",
   "Camera"
};

//----------------------------------------------------------------------------

static const char* gCameraSettings_Cinema_ModeStrings[2] =
{
   "FullScreen",  // kCinema_Mode_FullScreen
   "LetterBox"    // kCinema_Mode_WideScreen
};

//----------------------------------------------------------------------------

SBP_CameraDef*       gCameraSettings_ActiveDef = NULL;
SBP_CameraSettings*  gCameraSettings_ActiveSettings = NULL;

static char sFindCache_AreaName[ 16 ] = { 0 };
static int  sFindCache_Start = -1;
static int  sFindCache_End = -1;

int   gCameraSettings_UseCamera = 1;

int   gCameraSettings_EditMode = 0;
int   gCameraSettings_EnableEditCamera = 0;
int   gCameraSettings_EnableEditKeys = 0;

int   gCameraSettings_Cinema_OverrideMode = 0;                 // Debug
int   gCameraSettings_Cinema_Mode = kCinema_Mode_FullScreen;   // From Options
int   gCameraSettings_Cinema_BarsActive = 0;                   // Runtime - true if game wants to display bars
int   gCameraSettings_Cinema_BarsEnabled = 1;                  // Runtime - returns whether or not bars should be rendered
int   gCameraSettings_Cinema_UseFullScreenSettings = 1;        // Runtime - returns whether or not zoomin in full screen view should be used

char  gCameraSettings_MenuItem_NameString[256] = {0};
char  gCameraSettings_MenuItem_IdString[256] = {0};
char  gCameraSettings_MenuItem_FrameString[256] = {0};

int   gCameraSettings_MenuItem_SettingsToUse = 0;
int   gCameraSettings_MenuItem_Name = 0;
int   gCameraSettings_MenuItem_Id = 0;
int   gCameraSettings_MenuItem_AspectMode = 0;
int   gCameraSettings_MenuItem_Ratio = 0;
int   gCameraSettings_MenuItem_OriginX = 0;
int   gCameraSettings_MenuItem_OriginY = 0;
int   gCameraSettings_MenuItem_Reset = 0;

int   gCameraSettings_MenuItem_Copy = 0;
int   gCameraSettings_MenuItem_Paste = 0;

int   gCameraSettings_MenuItem_DumpArea = 0;
int   gCameraSettings_MenuItem_DumpAll = 0;

int   gCameraSettings_MenuItem_Frame = 0;
int   gCameraSettings_MenuItem_InsertKey = 0;
int   gCameraSettings_MenuItem_DeleteKey = 0;
int   gCameraSettings_MenuItem_DeleteAllKeys = 0;
int   gCameraSettings_MenuItem_RestoreToBootDefaults = 0;
int   gCameraSettings_MenuItem_DumpCinemaInfo = 0;

SBP_CameraDef      gCameraSettings_ClipboardDef;

SBP_CameraDef      gCameraSettings_GlobalDef;
SBP_CameraSettings gCameraSettings_GlobalSettings = {0};

SBP_CameraDef      gCameraSettings_CinemaFullScreenDef;
SBP_CameraSettings gCameraSettings_CinemaFullScreenSettings = {0};

SBP_CameraDef      gCameraSettings_CodecDef;
SBP_CameraSettings gCameraSettings_CodecSettings = {0};

EScriptEventType   gCameraSettings_ScriptEventType = kScriptEvent_Default;

real32 const kConvertToWidescreenScale = (16.0f / 9.0f) / (4.0f / 3.0f);

#if BP_ENABLE_CAMERA_DEBUG_MENU
std::vector<SBP_CameraDef> gBP_DebugCameraDefs;
std::vector<std::string>   gBP_DebugAreaNames;
#endif

//----------------------------------------------------------------------------
// PRIVATE FUNCTIONS
//----------------------------------------------------------------------------

// This becomes the x scale for the perspective transform
static
float BP_GetScreenAspectX( const int mode, const real32 ratio )
{
   real32 const originalAspectX = 1.0f;
   real32 const aspectX = originalAspectX / kConvertToWidescreenScale;

   real32 finalAspectX;

   switch( mode )
   {
   case kCSM_Full:
      finalAspectX = originalAspectX;
      break;

   case kCSM_Wide_ExpandWidth:
      finalAspectX = aspectX;
      break;

   case kCSM_Wide_ShrinkHeight:
      finalAspectX = originalAspectX;
      break;

   default:
   case kCSM_Wide_Variable:
      finalAspectX = aspectX + (originalAspectX - aspectX) * ratio;
      break;
   }

   return finalAspectX;
}

//----------------------------------------------------------------------------

// this gets multiplied by viewport width/height and then becomes the y scale of the perspective transform.
static
float BP_GetScreenAspectY( const int mode, const real32 ratio )
{
   real32 const originalAspectY = PIXEL_ASPECT;
   real32 const aspectY = originalAspectY * kConvertToWidescreenScale;

   real32 finalAspectY;

   switch( mode )
   {
   case kCSM_Full:
      finalAspectY = originalAspectY;
      break;

   case kCSM_Wide_ExpandWidth:
      finalAspectY = originalAspectY;
      break;

   case kCSM_Wide_ShrinkHeight:
      finalAspectY = aspectY;
      break;

   default:
   case kCSM_Wide_Variable:
      finalAspectY = originalAspectY + (aspectY - originalAspectY) * ratio;
      break;
   }

   return finalAspectY;
}

//----------------------------------------------------------------------------

static 
void BP_CameraSettings_ComputeAuxiliary( SBP_CameraSettings* pSettings )
{
   // Lookup def
   const SBP_CameraDef* def = pSettings->def;
   if( def == NULL )
   {
      def = &gCameraSettings_GlobalDef;
   }

   // Compute aspect from mode and ratio
   pSettings->aspectX = BP_GetScreenAspectX( def->mCameraMode, def->mCameraRatio );
   pSettings->aspectY = BP_GetScreenAspectY( def->mCameraMode, def->mCameraRatio );

   // Compute X offset as ratio of FULL to WIDE so we never show more than WIDE screen
   const real32 WideAspectX = BP_GetScreenAspectX( kCSM_Wide_Variable, 0 );
   pSettings->offsetX = def->mCameraOriginX * (1.0f - (pSettings->aspectX / WideAspectX));

   // Compute Y offset as ratio of WIDE to FULL so we never show more than original FULL screen
   const real32 fullAspectY = BP_GetScreenAspectY( kCSM_Full, 0 );
   pSettings->offsetY = def->mCameraOriginY * (1.0f - (pSettings->aspectY / fullAspectY));
}

//----------------------------------------------------------------------------

#if BP_ENABLE_CAMERA_DEBUG_MENU

static
const char* BP_Camera_AddAreaName( const char* areaName )
{
   // Make sure array doesn't re-allocate since debug info keeps pointers to items!
   if( gBP_DebugAreaNames.size() >= BP_CAMERA_MAX_DEBUG_AREA_NAMES )
   {
      // Please increase BP_CAMERA_MAX_DEBUG_AREA_NAMES
      BP_BREAK;
      return NULL;
   }

   // NOTE: We can't just keep a pointer to areaName since it's only temporary for this area.
   // Instead we have to keep around a cached copy so that it stays valid between area loads

   // Search for cached area name string
   const int nameCount = gBP_DebugAreaNames.size();
   for( int i = 0; i < nameCount; i++ )
   {
      const char* debugAreaName = gBP_DebugAreaNames[i].c_str();
      if( strcmp( areaName, debugAreaName ) == 0 )
      {
         return debugAreaName;
      }
   }

   // Add new area name
   gBP_DebugAreaNames.push_back( areaName );
   return gBP_DebugAreaNames[nameCount].c_str();
}

//----------------------------------------------------------------------------

SBP_CameraDef* BP_Camera_AddDef( const u_int cameraName, const u_int cameraId )
{
   XASSERT( cameraName != 0, "Bad camera!" );

   // Make sure array doesn't re-allocate since debug info keeps pointers to items!
   if( gBP_DebugCameraDefs.size() >= BP_CAMERA_MAX_DEBUG_CAMERA_DEFS )
   {
      // Please increase BP_CAMERA_MAX_DEBUG_CAMERA_DEFS
      BP_BREAK;
      return NULL;
   }

   // NOTE: We can't just keep a pointer to areaName since it's only temporary for this area.
   // Instead we have to keep around a cached copy so that it stays valid between area loads
   const char* areaName = BP_Camera_AddAreaName( GM_GetArea() );
   if( areaName == NULL )
   {
      return NULL;
   }

   // Create new def with default settings
   const int index = gBP_DebugCameraDefs.size();
   SBP_CameraDef def;
   def.mAreaName = areaName;
   def.mCameraName = cameraName;
   def.mCameraId = cameraId;
   gBP_DebugCameraDefs.push_back(def);
   return &gBP_DebugCameraDefs[index];
}

#endif

//----------------------------------------------------------------------------

static void recache_area( char const *areaName )
{
   if ( strcmp( areaName, sFindCache_AreaName ) )
   {
      // find cache is broken, let's fix it
      strcpy( sFindCache_AreaName, areaName );

      // Here's the static list
      SBP_CameraDef* defs  = (SBP_CameraDef*)BP_Camera_GetDefs();
      int            count = BP_Camera_GetDefsCount();

      for ( sFindCache_Start = 0; sFindCache_Start < count; ++sFindCache_Start )
      {
         if ( strstr(defs[sFindCache_Start].mAreaName, areaName) )
         {
            // Found our start
            break;
         }
      }

      if ( sFindCache_Start == count )
      {
         // Nothing.
         sFindCache_End = sFindCache_Start;
      }
      else
      {
         // Find the end
         for ( sFindCache_End = sFindCache_Start + 1; sFindCache_End < count; ++sFindCache_End )
         {
            if ( !strstr(defs[sFindCache_End].mAreaName, areaName) )
            {
               // Found our end
               break;
            }
         }

         if ( sFindCache_End != count )
         {
            // If we have room to spare, make sure there's not a restart!
            int restart;

            for ( restart = sFindCache_End + 1; restart < count; ++restart )
            {
               if ( strstr(defs[restart].mAreaName, areaName) )
               {
                  // Found our restart
                  printf( "BP CAMERA: WARNING %s start/end/RESTART %d/%d/%d\n", areaName, sFindCache_Start, sFindCache_End, restart );

                  break;
               }
            }

            if ( restart != count )
            {
               sFindCache_Start = sFindCache_End = -1;
            }
         }
      }

      printf( "BP CAMERA: Cached start/end for area %s: %d %d\n", areaName, sFindCache_Start, sFindCache_End );

   }
}

//----------------------------------------------------------------------------

static
SBP_CameraDef* BP_Camera_FindDef( const u_int cameraName, const int cameraId )
{
   int i;

   // Lookup area name
   const char* areaName = GM_GetArea();

   // Use cached?
   if( ( gCameraSettings_ActiveDef ) && ( gCameraSettings_ActiveDef->mAreaName ) )
   {
      // Match?
      if(      (strstr(gCameraSettings_ActiveDef->mAreaName, areaName) != NULL)  // NOTE: substring search allows for comma separated area list 
            && (gCameraSettings_ActiveDef->mCameraName == cameraName) 
            && (gCameraSettings_ActiveDef->mCameraId == cameraId) )
      {
         return gCameraSettings_ActiveDef;
      }
   }

   recache_area( areaName );

#if BP_ENABLE_CAMERA_DEBUG_MENU
   // Use dynamic debug list
   int            count = gBP_DebugCameraDefs.size();
   SBP_CameraDef* defs  = count ? &gBP_DebugCameraDefs[0] : NULL;
#else
   // Use statically defined list
   SBP_CameraDef* defs  = (SBP_CameraDef*)BP_Camera_GetDefs();
   int            count = BP_Camera_GetDefsCount();

   if ( sFindCache_Start != -1 )
   {
      defs = defs + sFindCache_Start;
      count = sFindCache_End - sFindCache_Start;
   }
#endif

   // Is this a cinema camera?
   if( gBP_Demo_Active )
   {
      // Search for the first key frame before or at the current cinema time
      SBP_CameraDef* cinemaDef  = NULL;
      int            cinemaFrame = -1;
      for( i = 0; i < count; i++ )
      {
         // Found match?
         SBP_CameraDef& def = defs[i];
         int id = (int)def.mCameraId;
         if(      (strstr(def.mAreaName, areaName) != NULL)    // NOTE: substring search allows for comma separated area list 
               && (def.mCameraName == cameraName) 
               && (id >= cinemaFrame)
               && (id <= cameraId ) )   // cameraId is gBP_Demo_Frame for cinemas
         {
            cinemaDef   = &def;
            cinemaFrame = id;
         }
      }
      return cinemaDef;
   }
   else
   {
      // Search all camera definition data
      for( i = 0; i < count; i++ )
      {
         // Found match?
         SBP_CameraDef& def = defs[i];
         if(      (strstr(def.mAreaName, areaName) != NULL)   // NOTE: substring search allows for comma separated area list 
               && (def.mCameraName == cameraName) 
               && (def.mCameraId == cameraId) )
         {
            return &def;
         }
      }
   }

   // Not found
   return NULL;
}

//----------------------------------------------------------------------------
// CINEMA BARS FUNCTIONS
//----------------------------------------------------------------------------

void BP_Camera_Cinemabars_SetState( const int active, const int barTop, const int barBot, const float alpha )
{
   // Update Y offset to center cinema view on original PS2 letter box image
   float center = (barTop + barBot) / 2.0f;
   float offset = (center - 224.0f) / 224.0f;
   gCameraSettings_Cinema_BarsActive = active;
   gCameraSettings_CinemaFullScreenDef.mCameraOriginY = offset * 4.0f;
   BP_CameraSettings_ComputeAuxiliary( &gCameraSettings_CinemaFullScreenSettings );
}

//----------------------------------------------------------------------------

int BP_Camera_Cinemabars_AreEnabled()
{
   return gCameraSettings_Cinema_BarsEnabled;
}

//----------------------------------------------------------------------------

int BP_Camera_Cinema_IsFullScreen()
{
   return (gCameraSettings_Cinema_Mode == kCinema_Mode_FullScreen);
}

//----------------------------------------------------------------------------

static 
float BP_Float_Lerp( const float a, const float b, const float t )
{
   bp_math_assert( BP_Float_Check(a) );
   bp_math_assert( BP_Float_Check(b) );
   bp_math_assert( BP_Float_Check(t) );

   return a + ( t * ( b - a ) );
}

//----------------------------------------------------------------------------

static
SBP_CameraSettings* BP_Camera_GetActiveSettings()
{
   // Use global by default
   SBP_CameraSettings* pSettings = &gCameraSettings_GlobalSettings;

   // Use active camera?
   if( gCameraSettings_UseCamera )
   {
      // Active camera settings?
      if( gCameraSettings_ActiveSettings )
      {
         pSettings = gCameraSettings_ActiveSettings;
      }
   }

   // Use cinema full screen (zoomed in) in settings?
   if( gCameraSettings_Cinema_UseFullScreenSettings )
   {
      pSettings = &gCameraSettings_CinemaFullScreenSettings;
   }

   return pSettings;
}

//----------------------------------------------------------------------------

static
SBP_CameraDef* BP_Camera_GetActiveDef()
{
   SBP_CameraSettings* pActiveSettings = BP_Camera_GetActiveSettings();
   if( pActiveSettings )
   {
      return pActiveSettings->def;
   }
   else
   {
      return NULL;
   }
}

//----------------------------------------------------------------------------
// PUBLIC FUNCTIONS
//----------------------------------------------------------------------------

float BP_GetScreenAspectX()
{
   const SBP_CameraSettings* pSettings = BP_Camera_GetActiveSettings();
   if( pSettings->aspectX != 0.0f )
   {
      return pSettings->aspectX;
   }
   else
   {
      return gCameraSettings_GlobalSettings.aspectX;
   }
}

//----------------------------------------------------------------------------

float BP_GetScreenAspectY()
{
   const SBP_CameraSettings* pSettings = BP_Camera_GetActiveSettings();
   if( pSettings->aspectY != 0.0f )
   {
      return pSettings->aspectY;
   }
   else
   {
      return gCameraSettings_GlobalSettings.aspectY;
   }
}

//----------------------------------------------------------------------------

float BP_GetScreenOffsetX()
{
   const SBP_CameraSettings* pSettings = BP_Camera_GetActiveSettings();
   return pSettings->offsetX;
}

//----------------------------------------------------------------------------

float BP_GetScreenOffsetY()
{
   const SBP_CameraSettings* pSettings = BP_Camera_GetActiveSettings();
   return pSettings->offsetY;
}

//----------------------------------------------------------------------------
// DEBUG MENU FUNCTIONS
//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_Modify(int data)
{
   // Lookup active
   SBP_CameraSettings* pActiveSettings = BP_Camera_GetActiveSettings();
   SBP_CameraDef*      pActiveDef      = BP_Camera_GetActiveDef();

   // Active?
   if( pActiveDef && pActiveSettings )
   {
      // Recompute auxiliary
      BP_CameraSettings_ComputeAuxiliary( pActiveSettings );
   }

   return 0;
}

//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_Reset(int data)
{
   // Lookup active
   SBP_CameraSettings* pActiveSettings = BP_Camera_GetActiveSettings();
   SBP_CameraDef*      pActiveDef      = BP_Camera_GetActiveDef();

   // Active?
   if( pActiveDef && pActiveSettings )
   {
      // Reset def
      pActiveDef->SetDefaults();

      // Recompute auxiliary
      BP_CameraSettings_ComputeAuxiliary( pActiveSettings );
   }

   return 0;
}

//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_Copy(int data)
{
   // Lookup active
   SBP_CameraSettings* pActiveSettings = BP_Camera_GetActiveSettings();
   SBP_CameraDef*      pActiveDef      = BP_Camera_GetActiveDef();

   // Active?
   if( pActiveDef && pActiveSettings )
   {
      // Copy
      gCameraSettings_ClipboardDef.mCameraMode    = pActiveDef->mCameraMode;
      gCameraSettings_ClipboardDef.mCameraRatio   = pActiveDef->mCameraRatio;
      gCameraSettings_ClipboardDef.mCameraOriginX = pActiveDef->mCameraOriginX;
      gCameraSettings_ClipboardDef.mCameraOriginY = pActiveDef->mCameraOriginY;
   }

   return 0;
}

//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_Paste(int data)
{
   // Lookup active
   SBP_CameraSettings* pActiveSettings = BP_Camera_GetActiveSettings();
   SBP_CameraDef*      pActiveDef      = BP_Camera_GetActiveDef();

   // Active?
   if( pActiveDef && pActiveSettings )
   {
      // Paste
      pActiveDef->mCameraMode     = gCameraSettings_ClipboardDef.mCameraMode; 
      pActiveDef->mCameraRatio    = gCameraSettings_ClipboardDef.mCameraRatio; 
      pActiveDef->mCameraOriginX  = gCameraSettings_ClipboardDef.mCameraOriginX; 
      pActiveDef->mCameraOriginY  = gCameraSettings_ClipboardDef.mCameraOriginY; 

      // Recompute auxiliary
      BP_CameraSettings_ComputeAuxiliary( pActiveSettings );
   }

   return 0;
}

//----------------------------------------------------------------------------

#if BP_ENABLE_CAMERA_DEBUG_MENU

static
int BP_DebugMenu_ActionCallback_CameraSettings_InsertKey(int data)
{
   // Active?
   if( gCameraSettings_ActiveDef && gCameraSettings_ActiveSettings )
   {
      // Skip if there's already a key for this frame
      SBP_CameraDef* def = BP_Camera_FindDef( gBP_Demo_Id, gBP_Demo_Frame );
      if( (def) && (def->mCameraId == gBP_Demo_Frame) )
      {
         return 0;
      }

      // Add new key for current frame
      SBP_CameraDef* newDef = BP_Camera_AddDef( gBP_Demo_Id, gBP_Demo_Frame );

      // Copy previous key frame to new key frame?
      if( newDef && def )
      {
         newDef->mCameraMode    = def->mCameraMode;
         newDef->mCameraRatio   = def->mCameraRatio;
         newDef->mCameraOriginX = def->mCameraOriginX;
         newDef->mCameraOriginY = def->mCameraOriginY;
      }

      // Clear active pointers
      // (BP_Camera_SetActiveCamera will set them up again)
      gCameraSettings_ActiveSettings->def = NULL;
      gCameraSettings_ActiveDef = NULL;
   }

   return 0;
}
#endif
//----------------------------------------------------------------------------

#if BP_ENABLE_CAMERA_DEBUG_MENU

static
int BP_DebugMenu_ActionCallback_CameraSettings_DeleteKey(int all)
{
   // Active?
   if( gCameraSettings_ActiveDef && gCameraSettings_ActiveSettings )
   {
      // Keep going
      while(1)
      {
         // Find key for current frame
         SBP_CameraDef* def = BP_Camera_FindDef( gBP_Demo_Id, all ? 0x00ffffff : gBP_Demo_Frame );
         if(def)
         {
            // Compute index of def
            const int index = ((int)def - (int)&gBP_DebugCameraDefs[0]) / sizeof(SBP_CameraDef);
            XASSERT( (index >= 0), "index out of range!" );
            XASSERT( (index < gBP_DebugCameraDefs.size()), "index out of range!" );

            // Erase def
            gBP_DebugCameraDefs.erase( gBP_DebugCameraDefs.begin() + index );
         }
         
         // Exit loop?
         if( (def == NULL) || (all == 0) )
         {
            break;
         }
      }

      // Clear active pointers
      // (BP_Camera_SetActiveCamera will set them up again)
      gCameraSettings_ActiveSettings->def = NULL;
      gCameraSettings_ActiveDef = NULL;
   }

   return 0;
}
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_CAMERA_DEBUG_MENU

static
void BP_Camera_GetDefsForArea( std::vector<SBP_CameraDef>& defs, const char* areaName, std::vector<SBP_CameraDef*>& areaDefs )
{
   // Loop over all defs
   for( int i = 0; i < defs.size(); i++ )
   {
      // Output this def?
      SBP_CameraDef& def = defs[i];
      const u_int id = def.mCameraId;
      if(         (strstr( def.mAreaName, areaName ) != NULL)        // NOTE: substring search allows for comma separated area list 
         && (     ( ((id & 0xff000000) == 0) && (id & 0x00ffffff) )  // Cinema camera not on 1st frame?
               || (def.mCameraRatio   != 0.0f)        // Not default values?
               || (def.mCameraOriginX != 0.0f) 
               || (def.mCameraOriginY != 0.0f) ) )
      {
         // Add to list
         areaDefs.push_back( &def );
      }
   }
}

//----------------------------------------------------------------------------

static
void BP_Camera_DumpDef( const SBP_CameraDef& def )
{
   // Output to TTY
   printf( "   SBP_CameraDef(" );
   printf( " kCamDefV4," );
   printf( " \"%s\",", def.mAreaName );
   printf( " 0x%08x,", def.mCameraName );
   if( def.mCameraId & 0xff000000 )
   {
      printf( " 0x%08x,", def.mCameraId );   // Script camera id
   }
   else
   {
      printf( " %10d,", def.mCameraId );     // Cinema camera frame
   }
   printf( " %s,", gCameraSettings_Mode_EnumStrings[ def.mCameraMode ] );
   printf( " %1.2ff,", def.mCameraRatio );
   printf( " %1.2ff,", def.mCameraOriginX );
   printf( " %1.2ff", def.mCameraOriginY );
   printf( " ),\n" );
}

//----------------------------------------------------------------------------

static
void BP_Camera_DumpArea( std::vector<SBP_CameraDef>& defs, const char* areaName, const char* fullAreaName )
{
   // Collect defs for this area
   std::vector<SBP_CameraDef*> areaDefs;
   areaDefs.reserve( defs.size() );
   BP_Camera_GetDefsForArea( defs, areaName, areaDefs );

   // Skip if no defs for this area
   if( areaDefs.size() == 0 )
   {
      return;
   }
      
   // Loop over all cameras defs
   printf( "\n" );
#if MGS_VERSION == 2
   printf( "//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA \"%s\" *****", fullAreaName );
#else
   printf( "//****** MGS3 BEGIN CAMERA SETTINGS FOR AREA \"%s\" *****", fullAreaName );
#endif
   printf( "\n" );
   for( int i = 0; i < areaDefs.size(); i++ )
   {
      // Output to TTY
      const SBP_CameraDef& def = *areaDefs[i];
      BP_Camera_DumpDef( def );
   }
#if MGS_VERSION == 2
   printf( "//****** MGS2 END CAMERA SETTINGS FOR AREA \"%s\" *****", areaName );
#else
   printf( "//****** MGS3 END CAMERA SETTINGS FOR AREA \"%s\" *****", areaName );
#endif

   printf( "\n" );
}

#endif

//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_Dump(int all)
{
#if BP_ENABLE_CAMERA_DEBUG_MENU
   // Make sorted copy of all defs
   std::vector<SBP_CameraDef> sortedDefs = gBP_DebugCameraDefs;
   std::sort( sortedDefs.begin(), sortedDefs.end() );

   // Dump all?
   printf( "\n\n" );
   if( all )
   {
      // Make sorted copy of all area names
      std::vector<std::string> sortedAreas = gBP_DebugAreaNames;
      std::sort( sortedAreas.begin(), sortedAreas.end() );

      // Dump all areas
      for( int i = 0; i < sortedAreas.size(); i++ )
      {
         const char* areaName = sortedAreas[i].c_str();
         BP_Camera_DumpArea( sortedDefs, areaName, areaName );
      }
   }
   else
   {
      // Create full area name with additional demo info
      char fullAreaName[256];
      if( gBP_Demo_Active )
      {
         sprintf( fullAreaName, "%s, 0x%08x, %s", GM_GetArea(), gBP_Demo_Id, gBP_Demo_StreamName );
      }
      else
      {
         strcpy( fullAreaName, GM_GetArea() );
      }

      // Dump current area
      BP_Camera_DumpArea( sortedDefs, GM_GetArea(), fullAreaName );
   }
   printf( "\n\n" );
#endif

   return 0;
}

//----------------------------------------------------------------------------

static
int BP_DebugMenu_ActionCallback_CameraSettings_DumpCinemaInfo(int data)
{
#if BP_ENABLE_CAMERA_DEBUG_MENU

   // Output to TTY
   printf( "\n\n" );
#if MGS_VERSION == 2
   printf( "{ \"%s\", 0x%08x, \"%s\", kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGS2 CINEMA", GM_GetArea(), gBP_Demo_Id, gBP_Demo_StreamName );
#else
   printf( "{ \"%s\", 0x%08x, \"%s\", kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGS3 CINEMA", GM_GetArea(), gBP_Demo_Id, gBP_Demo_StreamName );
#endif
   printf( "\n\n" );
#endif

   return 0;
}

//----------------------------------------------------------------------------

void BP_Camera_SetSettingsToBootDefaults()
{
#if BP_ENABLE_CAMERA_DEBUG_MENU
   
   // Free
   gBP_DebugCameraDefs.clear();
   gBP_DebugAreaNames.clear();

   // Pre-allocate
   gBP_DebugCameraDefs.reserve(BP_CAMERA_MAX_DEBUG_CAMERA_DEFS);
   gBP_DebugAreaNames.reserve(BP_CAMERA_MAX_DEBUG_AREA_NAMES);

   // TTY
   printf( "\n\n" );
   printf( "BP_Camera_SetSettingsToBootDefaults()\n" );
   printf( "BEGIN\n" );

   // Copy static list to dynamic list
   const SBP_CameraDef* defs  = BP_Camera_GetDefs();
   const int            count = BP_Camera_GetDefsCount();
   for( int i = 0; i < count; i++ )
   {
      // Lookup def
      const SBP_CameraDef& defA = defs[i];

      // Check for dupe entry
      for( int j = 0; j < gBP_DebugCameraDefs.size(); j++ )
      {
         // Lookup def
         const SBP_CameraDef& defB = gBP_DebugCameraDefs[j];

         // Matching camera id?
         if(      (strstr( defA.mAreaName, defB.mAreaName ) != NULL)
               && (defA.mCameraName == defB.mCameraName)
               && (defA.mCameraId   == defB.mCameraId) )
         {
            // TTY
            printf( " WARNING: found dupe entry!!!\n" );
            printf( "    defA == " ); BP_Camera_DumpDef( defA );
            printf( "    defB == " ); BP_Camera_DumpDef( defB );
         }
      }

      // Add area name
      BP_Camera_AddAreaName( defA.mAreaName );

      // Add def
      gBP_DebugCameraDefs.push_back( defA );
   }

   // TTY
   printf( "END\n" );
   printf( "\n\n" );

#endif
}

//----------------------------------------------------------------------------

void BP_Camera_Init()
{
   // Init global defaults
   gCameraSettings_GlobalSettings.def = &gCameraSettings_GlobalDef;
   BP_CameraSettings_ComputeAuxiliary( &gCameraSettings_GlobalSettings );

   // Init special case codec camera
   gCameraSettings_CodecDef.mCameraRatio = 1.0f;
   gCameraSettings_CodecSettings.def = &gCameraSettings_CodecDef;
   BP_CameraSettings_ComputeAuxiliary( &gCameraSettings_CodecSettings );

   // Init special case cinema no bars camera
   gCameraSettings_CinemaFullScreenDef.mCameraRatio = 1.0f;
   gCameraSettings_CinemaFullScreenDef.mCameraOriginY = -0.57142859f;
   gCameraSettings_CinemaFullScreenSettings.def = &gCameraSettings_CinemaFullScreenDef;
   BP_CameraSettings_ComputeAuxiliary( &gCameraSettings_CinemaFullScreenSettings );

   BP_Camera_SetSettingsToBootDefaults();
}

//----------------------------------------------------------------------------

static int BP_DebugMenu_ActionCallback_CameraSettings_RestoreToBootDefaults(int data)
{
   BP_Camera_SetSettingsToBootDefaults();
   return 0;
}

//----------------------------------------------------------------------------

void BP_Camera_InitDebugMenu()
{
#if BP_ENABLE_CAMERA_DEBUG_MENU
   // Camera Debug options
   {
      int const cameraMenu = BP_DebugMenu_AddMenu("Camera", -1);
      
      BP_DebugMenu_AddBool(cameraMenu, "Cinema mode: Override options", &gCameraSettings_Cinema_OverrideMode);
      BP_DebugMenu_SetEnabled( BP_DebugMenu_AddEnum(cameraMenu, "Cinema mode", gCameraSettings_Cinema_ModeStrings, &gCameraSettings_Cinema_Mode, 0, 1 ), &gCameraSettings_Cinema_OverrideMode);

      BP_DebugMenu_AddSeparator(cameraMenu);
      BP_DebugMenu_AddBool(cameraMenu, "Enable Edit Mode", &gCameraSettings_EditMode);

      BP_DebugMenu_AddSeparator(cameraMenu);
      gCameraSettings_MenuItem_SettingsToUse = BP_DebugMenu_AddEnum(cameraMenu, "Settings to use/edit", gCameraSettings_Use_Strings, &gCameraSettings_UseCamera, 0, 1);
      gCameraSettings_MenuItem_Name          = BP_DebugMenu_AddString(cameraMenu, "Name", gCameraSettings_MenuItem_NameString);
      gCameraSettings_MenuItem_Id            = BP_DebugMenu_AddString(cameraMenu, "Id", gCameraSettings_MenuItem_IdString);
      gCameraSettings_MenuItem_AspectMode    = BP_DebugMenu_AddEnum (cameraMenu, "Aspect mode", gCameraSettings_Mode_MenuStrings, &gCameraSettings_GlobalDef.mCameraMode, 0, kCSM_Count-1);
      gCameraSettings_MenuItem_Ratio         = BP_DebugMenu_AddFloat(cameraMenu, "Ratio", &gCameraSettings_GlobalDef.mCameraRatio, 0.0f, 10.0f, 0.01f, 0.1f);
      gCameraSettings_MenuItem_OriginX       = BP_DebugMenu_AddFloat(cameraMenu, "OriginX", &gCameraSettings_GlobalDef.mCameraOriginX, -10.0f, 10.0f, 0.01f, 0.1f);
      gCameraSettings_MenuItem_OriginY       = BP_DebugMenu_AddFloat(cameraMenu, "OriginY", &gCameraSettings_GlobalDef.mCameraOriginY, -10.0f, 10.0f, 0.01f, 0.1f);

      BP_DebugMenu_AddSeparator(cameraMenu);
      gCameraSettings_MenuItem_Reset       = BP_DebugMenu_AddAction(cameraMenu, "Reset settings", BP_DebugMenu_ActionCallback_CameraSettings_Reset, 0);
      gCameraSettings_MenuItem_Copy        = BP_DebugMenu_AddAction(cameraMenu, "Copy settings", BP_DebugMenu_ActionCallback_CameraSettings_Copy, 0);
      gCameraSettings_MenuItem_Paste       = BP_DebugMenu_AddAction(cameraMenu, "Paste settings", BP_DebugMenu_ActionCallback_CameraSettings_Paste, 0);
      
      BP_DebugMenu_AddSeparator(cameraMenu);
      gCameraSettings_MenuItem_DumpArea    = BP_DebugMenu_AddAction(cameraMenu, "Dump area cameras to TTY", BP_DebugMenu_ActionCallback_CameraSettings_Dump, 0);
      gCameraSettings_MenuItem_DumpAll     = BP_DebugMenu_AddAction(cameraMenu, "Dump all game cameras to TTY", BP_DebugMenu_ActionCallback_CameraSettings_Dump, 1);
      
      BP_DebugMenu_AddSeparator(cameraMenu);
      gCameraSettings_MenuItem_Frame         = BP_DebugMenu_AddString(cameraMenu, "Cinema: key frame", gCameraSettings_MenuItem_FrameString);
      gCameraSettings_MenuItem_InsertKey     = BP_DebugMenu_AddAction(cameraMenu, "Cinema: insert key", BP_DebugMenu_ActionCallback_CameraSettings_InsertKey, 0);
      gCameraSettings_MenuItem_DeleteKey     = BP_DebugMenu_AddAction(cameraMenu, "Cinema: delete key", BP_DebugMenu_ActionCallback_CameraSettings_DeleteKey, 0);
      gCameraSettings_MenuItem_DeleteAllKeys = BP_DebugMenu_AddAction(cameraMenu, "Cinema: delete all keys", BP_DebugMenu_ActionCallback_CameraSettings_DeleteKey, 1);
      gCameraSettings_MenuItem_RestoreToBootDefaults = BP_DebugMenu_AddAction(cameraMenu, "Cinema: restore all keys to boot defaults", BP_DebugMenu_ActionCallback_CameraSettings_RestoreToBootDefaults, 0);
      gCameraSettings_MenuItem_DumpCinemaInfo = BP_DebugMenu_AddAction(cameraMenu, "Cinema: dump name to TTY", BP_DebugMenu_ActionCallback_CameraSettings_DumpCinemaInfo, 0);

      // Enable items
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_SettingsToUse,  &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Name,           &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Id,             &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_AspectMode,     &gCameraSettings_EnableEditCamera );

      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Ratio,          &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_OriginX,        &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_OriginY,        &gCameraSettings_EnableEditCamera );

      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Reset,          &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Copy,           &gCameraSettings_EnableEditCamera );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Paste,          &gCameraSettings_EnableEditCamera );

      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_Frame,                 &gCameraSettings_EnableEditKeys );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_InsertKey,             &gCameraSettings_EnableEditKeys );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_DeleteKey,             &gCameraSettings_EnableEditKeys );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_DeleteAllKeys,         &gCameraSettings_EnableEditKeys );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_RestoreToBootDefaults, &gCameraSettings_EnableEditKeys );
      BP_DebugMenu_SetEnabled( gCameraSettings_MenuItem_DumpCinemaInfo,        &gCameraSettings_EnableEditKeys );

      // Callback
      BP_DebugMenu_SetCallback( gCameraSettings_MenuItem_AspectMode,   &BP_DebugMenu_ActionCallback_CameraSettings_Modify, 0 );
      BP_DebugMenu_SetCallback( gCameraSettings_MenuItem_Ratio,        &BP_DebugMenu_ActionCallback_CameraSettings_Modify, 0 );
      BP_DebugMenu_SetCallback( gCameraSettings_MenuItem_OriginX,      &BP_DebugMenu_ActionCallback_CameraSettings_Modify, 0 );
      BP_DebugMenu_SetCallback( gCameraSettings_MenuItem_OriginY,      &BP_DebugMenu_ActionCallback_CameraSettings_Modify, 0 );

#if MGS_VERSION == 3
      BP_DebugMenu_AddSeparator(cameraMenu);
      BP_DebugMenu_AddBool(cameraMenu, "Debug Camera Caption", &gShowDebugCameraCaption );
      BP_DebugMenu_AddBool(cameraMenu, "Allow 1st person movement", &gBP_1stPersonCamera_EnableMovement );
#endif
   }
#endif
}

//----------------------------------------------------------------------------


void BP_Camera_Update()
{
#if !REMOVE_LETTERBOX_OPTION
   // Update cinema bars from options
   if( !gCameraSettings_Cinema_OverrideMode )
   {
#  if MGS_VERSION == 2
      gCameraSettings_Cinema_Mode = ((GM_Configuration&GM_CONFIG_CUTSCENES_LETTERBOXED) != 0);
#  endif

#  if MGS_VERSION == 3
      gCameraSettings_Cinema_Mode = ((GM_Configuration & GM_CONFIG_SOUND_5_1CHANL) != 0 );
#  endif
   }
#endif

#if BP_ENABLE_CAMERA_DEBUG_MENU
   // Lookup active def
   SBP_CameraDef* def = NULL;
   if( gCameraSettings_UseCamera )
   {
      // Lookup camera def
      def = gCameraSettings_ActiveDef;
      if( def == NULL )
      {
         def = &gCameraSettings_GlobalDef;
      }

      // Cinema?
      if( gBP_Demo_Active )
      {
         // Edit cinema camera?
         if( gCameraSettings_Cinema_UseFullScreenSettings )
         {
            def = &gCameraSettings_CinemaFullScreenDef;
         }

         // Cinema info
         BP_DebugMenu_SetName( gCameraSettings_MenuItem_Name, "Cinema name" );
         BP_DebugMenu_SetName( gCameraSettings_MenuItem_Id, "Cinema key frame" );
         sprintf( gCameraSettings_MenuItem_NameString, "0x%08x (%s)", gBP_Demo_Id, gBP_Demo_StreamName );
         sprintf( gCameraSettings_MenuItem_IdString, "%d", def->mCameraId );
      }
      else
      {
         // Camera info
         BP_DebugMenu_SetName( gCameraSettings_MenuItem_Name, "Camera name" );
         BP_DebugMenu_SetName( gCameraSettings_MenuItem_Id, "Camera id" );
         sprintf( gCameraSettings_MenuItem_NameString, "0x%08x", def->mCameraName );
         sprintf( gCameraSettings_MenuItem_IdString, "0x%08x", def->mCameraId );
      }
   }
   else
   {
      // Global info
      def = &gCameraSettings_GlobalDef;
      BP_DebugMenu_SetName( gCameraSettings_MenuItem_Name, "Global name" );
      BP_DebugMenu_SetName( gCameraSettings_MenuItem_Id, "Global id" );
      sprintf( gCameraSettings_MenuItem_NameString, "0x%08x", def->mCameraName );
      sprintf( gCameraSettings_MenuItem_IdString, "0x%08x", def->mCameraId );
   }

   // Update cinema info
   if( gBP_Demo_Active )
   {
      sprintf( gCameraSettings_MenuItem_FrameString, "%d", gBP_Demo_Frame);
   }
   else
   {
      sprintf( gCameraSettings_MenuItem_FrameString, "" );
   }

   // Update debug menu vars
   BP_DebugMenu_SetEnumVar( gCameraSettings_MenuItem_AspectMode, &def->mCameraMode );
   BP_DebugMenu_SetFloatVar( gCameraSettings_MenuItem_Ratio, &def->mCameraRatio );
   BP_DebugMenu_SetFloatVar( gCameraSettings_MenuItem_OriginX, &def->mCameraOriginX );
   BP_DebugMenu_SetFloatVar( gCameraSettings_MenuItem_OriginY, &def->mCameraOriginY );

   // Update enabled items
   gCameraSettings_EnableEditCamera = gCameraSettings_EditMode && def && (def->mCameraMode == kCSM_Wide_Variable);
   gCameraSettings_EnableEditKeys   = gCameraSettings_EditMode && gBP_Demo_Active;
#endif
}

//----------------------------------------------------------------------------

void BP_Camera_SetScriptEventType( const int scriptEventType )
{
   gCameraSettings_ScriptEventType = (EScriptEventType)scriptEventType;
}

//----------------------------------------------------------------------------

void BP_Camera_SetId( MGS_Camera* cam, const u_int id )
{
   // Set id
   XASSERT( cam != NULL, "Camera is NULL?!!" );

   // Leave camera settings untouched if this came from a "leave trap" script event
   // since this happens after the script event "enter trap" happens which sets
   // up the new camera. This fixes the dodgey 1 frame glitches that would
   // happen otherwise as the old camera settings are used for a single frame
   // on the new camera. 
   // See trap.c for calls to "BP_Camera_SetScriptEventType"
   if( gCameraSettings_ScriptEventType == kScriptEvent_LeaveTrap )
   {
      return;
   }

   // Set id
   cam->bp_settings.id = id;

   // Lookup def
   SBP_CameraDef* def = cam->name ? BP_Camera_FindDef( cam->name, id ) : NULL;

#if BP_ENABLE_CAMERA_DEBUG_MENU
   // No def?
   if( ( def == NULL ) && ( cam->name ) && ( gCameraSettings_EditMode ) )
   {
      // Cinema?
      if( gBP_Demo_Active )
      {
         // Create def for first frames of cinema
         def = BP_Camera_AddDef( cam->name, -1 );
      }
      else
      {
         // Create def for in game camera
         def = BP_Camera_AddDef( cam->name, id );
      }
   }
#endif

   // Not found, so use global def
   if( def == NULL )
   {
      def = &gCameraSettings_GlobalDef;
   }

   // Switching camera view?
   if( cam->bp_settings.def != def )
   {
      // Reset camera settings
      cam->bp_settings.def = def;
      BP_CameraSettings_ComputeAuxiliary( &cam->bp_settings );
   }
}

//----------------------------------------------------------------------------

u_int BP_Camera_GetScriptId()
{
   // Lookup command bytes
   const char* scriptStart;
   const char* scriptEnd;
   GCL_GetOptionBytes( &scriptStart, &scriptEnd );

   // Compute hash
   const int length = (int)scriptEnd - (int)scriptStart + 1;
   u_int hash = CCRC::CalculateCRC32( scriptStart, length );

#if BP_CAMERA_PRINT_SCRIPT_ID_DATA

   printf("\n");
   printf("New:");
   GCL_GetOptionBytes( &scriptStart, &scriptEnd );

   printf( " Hash=0x%08x,", hash );
   printf( " Data=" );
   for( int i = 0; i < length; i++ )
   {
      if( i != 0 )
      {
         printf( ", " );
      }
      printf( "0x%02x", (unsigned char)scriptStart[i] );
   }
   printf("\n");

#endif

   return hash;
}

//----------------------------------------------------------------------------

u_int BP_Camera_GetCameraId()
{
   return BP_Camera_GetScriptId();
}

//----------------------------------------------------------------------------

u_int BP_Camera_GetDemoId()
{
   // Use new method (hash demo stream filename)
   const char* start  = gBP_Demo_StreamName;
   const int   length = strlen( gBP_Demo_StreamName );
   const u_int hash   = CCRC::CalculateCRC32( start, length );

   return hash;
}

//----------------------------------------------------------------------------

void BP_Camera_SetIdFromScript( MGS_Camera* cam )
{
   // Compute options id
   u_int newId = BP_Camera_GetCameraId();

   // Set id
   BP_Camera_SetId( cam, newId );
}

//----------------------------------------------------------------------------

void BP_Camera_SetIdFromDemo( MGS_Camera* cam )
{
   // Set name
   cam->name = gBP_Demo_Id;
   
   // Set id
   BP_Camera_SetId( cam, gBP_Demo_Frame );
}

//----------------------------------------------------------------------------

void BP_Camera_SetCinemaInfo( void )
{
#if REMOVE_LETTERBOX_OPTION
   // make sure this is set appropriately
   gCameraSettings_Cinema_Mode = kCinema_Mode_FullScreen;
#endif

   // Setup default enabling of cinema bars:
   // - Always enable for in game cut scenes (ie. when a demo is not active)
   // - Always enable when options are set to wide screen (letterbox) mode
   gCameraSettings_Cinema_BarsEnabled           = (!gBP_Demo_Active) || (gCameraSettings_Cinema_Mode == kCinema_Mode_WideScreen);
   
   // Setup default "use full screen" (ie.zoomed in area of view iniside original PS2 black bars) camera:
   // - Enabled if original PS2 bars are wanting to be displayed, options are wide screen, and a demo is active!
   gCameraSettings_Cinema_UseFullScreenSettings = (gCameraSettings_Cinema_BarsActive) && (gCameraSettings_Cinema_Mode == kCinema_Mode_FullScreen) && (gBP_Demo_Active);

   // Override with special case cinema settings?
   if( gBP_Demo_Active )
   {
      // Search cinema defs to see if we should override bars/full screen
      const SBP_CinemaDef* def   = BP_Cinema_GetDefs();
      const int            count = BP_Cinema_GetDefsCount();
      for( int i = 0; i < count; i++, def++ )
      {
         // Found matching cinema?
         if( gBP_Demo_Id == def->mCinemaId )
         {
            // Set cinema bars
            switch( def->mCinemaBars )
            {
            case kCinema_Bars_Options:
               gCameraSettings_Cinema_BarsEnabled = (gCameraSettings_Cinema_Mode == kCinema_Mode_WideScreen);
               break;
            case kCinema_Bars_Off:
               gCameraSettings_Cinema_BarsEnabled = 0;
               break;
            case kCinema_Bars_On:
               gCameraSettings_Cinema_BarsEnabled = 1;
               break;
            }

            // Set full screen
            switch( def->mCinemaMode )
            {
            case kCinema_Mode_Options:
               gCameraSettings_Cinema_UseFullScreenSettings = (gCameraSettings_Cinema_Mode == kCinema_Mode_FullScreen);
               BP_Camera_Cinemabars_SetState( 0, 10, 330, 0.0f );
               break;
            case kCinema_Mode_WideScreen:
               gCameraSettings_Cinema_UseFullScreenSettings = 0;
               break;
            case kCinema_Mode_FullScreen:
               gCameraSettings_Cinema_UseFullScreenSettings = 1;
               break;
            }

            // Done
            return;
         }
      }
   }
   else
   {
      // Lookup camera info
      const char* areaName   = GM_GetArea();
      const u_int cameraName = gCameraSettings_ActiveDef->mCameraName;
      const u_int cameraId   = gCameraSettings_ActiveDef->mCameraId;

      // Search cutscene defs to see if we should override bars/full screen
      const SBP_CutsceneDef* def = BP_Cutscene_GetDefs();
      const int            count = BP_Cutscene_GetDefsCount();
      for( int i = 0; i < count; i++, def++ )
      {
         // Found?
         if(   (def->mCameraName == cameraName) 
            && (def->mCameraId   == cameraId)
            && (strcmp(def->mAreaName, areaName) == 0) )
         {
            // Set cinema bars
            switch( def->mCinemaBars )
            {
            case kCinema_Bars_Options:
               gCameraSettings_Cinema_BarsEnabled = (gCameraSettings_Cinema_Mode == kCinema_Mode_WideScreen);
               break;
            case kCinema_Bars_Off:
               gCameraSettings_Cinema_BarsEnabled = 0;
               break;
            case kCinema_Bars_On:
               gCameraSettings_Cinema_BarsEnabled = 1;
               break;
            }

            // Set full screen
            switch( def->mCinemaMode )
            {
            case kCinema_Mode_Options:
               gCameraSettings_Cinema_UseFullScreenSettings = (gCameraSettings_Cinema_Mode == kCinema_Mode_FullScreen);
               BP_Camera_Cinemabars_SetState( 0, 10, 330, 0.0f );
               break;
            case kCinema_Mode_WideScreen:
               gCameraSettings_Cinema_UseFullScreenSettings = 0;
               break;
            case kCinema_Mode_FullScreen:
               gCameraSettings_Cinema_UseFullScreenSettings = 1;
               break;
            }

            // Done
            return;
         }
      }
   }
}

//----------------------------------------------------------------------------

void BP_Camera_SetActiveCamera( MGS_Camera* cam )
{
   // If specifying a camera, make sure to re-look up id so
   // def is set for cameras which are not driven by script (eg. when backing up against objects)
   if(cam)
   {
      BP_Camera_SetId( cam, cam->bp_settings.id );
   }

   // Track camera settings?
   if( (cam) && (cam->bp_settings.def) )
   {
      // Use camera settings?
      gCameraSettings_ActiveDef      = cam->bp_settings.def;
      gCameraSettings_ActiveSettings = &cam->bp_settings;
   }
   else
   {
      // Use global settings
      gCameraSettings_ActiveDef      = &gCameraSettings_GlobalDef;
      gCameraSettings_ActiveSettings = &gCameraSettings_GlobalSettings;
   }

   // Setup cinema info
   BP_Camera_SetCinemaInfo();
}

//----------------------------------------------------------------------------

void BP_Camera_SetCodecCamera()
{
   // Use PS23 settings
   gCameraSettings_ActiveDef      = &gCameraSettings_CodecDef;
   gCameraSettings_ActiveSettings = &gCameraSettings_CodecSettings;
}

//----------------------------------------------------------------------------
