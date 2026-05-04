//----------------------------------------------------------------------------
// BP_SaveLoadMGS.cpp
// MGS-specific emulation layer between PS2 saveload I/O and CP4 platforms.
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "BP_SaveLoadMGS.h"
#include "BP_TrophySystem.h"

extern "C" void MGS_SaveStatus_Close();

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"

#include "MGS_SysCommon.h"

#if BP_PS3
#include "Transfarring_PS3.h"
#include "Transfarring_PS3CGlue.h"
#endif

#if BP_VITA
#include "TransfarringVTA.h"
#endif

extern "C" {
#include "linkvar.h"
}

#if BP_PS3
#include "Transfarring_PS3.h"
#endif

#if BP_VITA
#include "TransfarringVTA.h"
#include "TransfarringVTACGlue.h"
#endif

#define SCE_RDONLY 0x0001
#define SCE_WRONLY 0x0002
#define SCE_ENODEV 19

//----------------------------------------------------------------------------

extern "C" char bp_basefilename[0x30];
extern "C" char ofilename[0x30];   //used to search for (VR?) data from a different product code, irrelevant for us but is searched when starting 'missions'.

//----------------------------------------------------------------------------

static CMGS_SaveStatus gMGS_SaveStatus;

#ifdef BP_PS3
BPE_CTASSERT( sizeof( gMGS_SaveStatus.mSaveDirectory ) == CELL_SAVEDATA_DIRNAME_SIZE );
#endif

BPE_CTASSERT( sizeof( sv_bp_linkvars ) <= MAX_BP_LINKVARBUF );

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#if MGS_VERSION==2
static char const skSGS_Title_NA[] = "METAL GEAR SOLID 2";
static char const skSGS_Title_EU[] = "METAL GEAR SOLID 2";
static char const skSGS_Title_J[]  = "METAL GEAR SOLID 2";
#elif MGS_VERSION==3
static char const skSGS_Title_NA[] = "METAL GEAR SOLID 3";
static char const skSGS_Title_EU[] = "METAL GEAR SOLID 3";
static char const skSGS_Title_J[]  = "METAL GEAR SOLID 3";
#endif

static char const skSGS_Subtitle_E[] = "Save data";
static char const skSGS_Subtitle_F[] = "Sauvegarde";
static char const skSGS_Subtitle_I[] = "Dati salvati";
static char const skSGS_Subtitle_G[] = "Speicherdaten";
static char const skSGS_Subtitle_S[] = "Datos guardados";
static char const skSGS_Subtitle_J[] = "セーブデータ";

static char const skSGS_Subtitle_Mission_E[] = "Mission data";
static char const skSGS_Subtitle_Tales_E[] = "Snake Tales data";
static char const skSGS_Subtitle_Photo_E[] = "Photo data";
static char const skSGS_Subtitle_MG1_E[] = "METAL GEAR DATA";
static char const skSGS_Subtitle_MG2_E[] = "METAL GEAR 2 SOLID SNAKE DATA";

static char const skSGS_Subtitle_Mission_F[] = "Données de mission";
static char const skSGS_Subtitle_Tales_F[] = "Données Snake Tales";
static char const skSGS_Subtitle_Photo_F[] = "Données photo";
static char const skSGS_Subtitle_MG1_F[] = "DONNÉES METAL GEAR";
static char const skSGS_Subtitle_MG2_F[] = "DONNEÉES METAL GEAR 2 SOLID SNAKE";

static char const skSGS_Subtitle_Mission_I[] = "Dati missione";
static char const skSGS_Subtitle_Tales_I[] = "Dati Snake Tales";
static char const skSGS_Subtitle_Photo_I[] = "Dati fotografie";
static char const skSGS_Subtitle_MG1_I[] = "DATI METAL GEAR";
static char const skSGS_Subtitle_MG2_I[] = "DATI METAL GEAR 2 SOLID SNAKE";

static char const skSGS_Subtitle_Mission_G[] = "Missionsdaten";
static char const skSGS_Subtitle_Tales_G[] = "Daten zu Snake Tales";
static char const skSGS_Subtitle_Photo_G[] = "Fotodaten";
static char const skSGS_Subtitle_MG1_G[] = "METAL GEAR-DATEN";
static char const skSGS_Subtitle_MG2_G[] = "METAL GEAR 2 SOLID SNAKE-DATEN";

static char const skSGS_Subtitle_Mission_S[] = "Datos de la misión";
static char const skSGS_Subtitle_Tales_S[] = "Datos de historias de Snake";
static char const skSGS_Subtitle_Photo_S[] = "Datos de fotos";
static char const skSGS_Subtitle_MG1_S[] = "DATOS DE METAL GEAR";
static char const skSGS_Subtitle_MG2_S[] = "DATOS DE METAL GEAR 2 SOLID SNAKE";

static char const skSGS_Subtitle_Mission_J[] = "ミッションのデータ";
static char const skSGS_Subtitle_Tales_J[] = "スネークテイルズのデータ";
static char const skSGS_Subtitle_Photo_J[] = "写真のデータ";
static char const skSGS_Subtitle_MG1_J[] = "METAL GEARのデータ";
static char const skSGS_Subtitle_MG2_J[] = "METAL GEAR 2 SOLID SNAKEのデータ";

static char const skSGS_Unknown[] = "Unknown";

char const * save_data_get_save_game_localized_string(ESaveGameString const string, EInternalSaveData const whichSaveData)
{
   switch (string)
   {
   case kSGS_Title:
      switch (gpOsContext->mBuildSKU)
      {
      case CBaseOsContext::kBS_Japan:
         {
            switch (gpOsContext->mLanguage)
            {
            case CBaseOsContext::kL_Japanese:
               return skSGS_Title_J;

            default: // Default to English
               return skSGS_Title_NA;
            }
            break;
         }
         break;

      case CBaseOsContext::kBS_USA:
         return skSGS_Title_NA;

      case CBaseOsContext::kBS_Europe:
         return skSGS_Title_EU;

      }

   case kSGS_Subtitle:
      switch( whichSaveData )
      {
      case kISD_Game:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_S;

         default: // Default to English
            return skSGS_Subtitle_E;
         }
         break;
      case kISD_VR:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_Mission_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_Mission_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_Mission_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_Mission_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_Mission_S;

         default: // Default to English
            return skSGS_Subtitle_Mission_E;
         }
         break;
      case kISD_SnakeTales:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_Tales_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_Tales_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_Tales_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_Tales_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_Tales_S;

         default: // Default to English
            return skSGS_Subtitle_Tales_E;
         }
         break;
      case kISD_Photo:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_Photo_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_Photo_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_Photo_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_Photo_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_Photo_S;

         default: // Default to English
            return skSGS_Subtitle_Photo_E;
         }
         break;
      case kISD_MG1:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_MG1_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_MG1_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_MG1_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_MG1_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_MG1_S;

         default: // Default to English
            return skSGS_Subtitle_MG1_E;
         }
         break;
      case kISD_MG2:
         switch (gpOsContext->mLanguage)
         {
         case CBaseOsContext::kL_Japanese:
            return skSGS_Subtitle_MG2_J;

         case CBaseOsContext::kL_French:
            return skSGS_Subtitle_MG2_F;

         case CBaseOsContext::kL_Italian:
            return skSGS_Subtitle_MG2_I;

         case CBaseOsContext::kL_German:
            return skSGS_Subtitle_MG2_G;

         case CBaseOsContext::kL_Spanish:
            return skSGS_Subtitle_MG2_S;

         default: // Default to English
            return skSGS_Subtitle_MG2_E;
         }
         break;
      default:
         BPE_VERIFY( false, false, "TODO" );
      }
   }
   return skSGS_Unknown;
}

//----------------------------------------------------------------------------

#ifdef BP_360
//XBox360 restricted to weenie 64x64 icon files, so here are alternate headers.

#if MGS_VERSION==2
#include "SaveIcon/ICON_TANKER_X360.PNG.h"
#include "SaveIcon/ICON_PLANT_X360.PNG.h"
#include "SaveIcon/ICON_VR_X360.PNG.h"
#include "SaveIcon/ICON_SNAKETALES_X360.PNG.h"
#include "SaveIcon/ICON_PHOTO_X360.PNG.h"
BPE_CTASSERT( sizeof(binary_ICON_TANKER_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_PLANT_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_VR_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_SNAKETALES_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_PHOTO_PNG_data) <= 15616 );
// Put JP icons in namespace to avoid conflicts
namespace NSaveIconsJP
{
   #include "SaveIcon/JP/ICON_TANKER_X360.PNG.h"
   #include "SaveIcon/JP/ICON_PLANT_X360.PNG.h"
   #include "SaveIcon/JP/ICON_VR_X360.PNG.h"
   #include "SaveIcon/JP/ICON_SNAKETALES_X360.PNG.h"
   #include "SaveIcon/JP/ICON_PHOTO_X360.PNG.h"
   BPE_CTASSERT( sizeof(binary_ICON_TANKER_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_PLANT_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_VR_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_SNAKETALES_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_PHOTO_PNG_data) <= 15616 );
}

#elif MGS_VERSION==3
#include "SaveIcon/ICON_GAME_X360.PNG.h"
#include "SaveIcon/ICON_PHOTO_X360.PNG.h"
#include "SaveIcon/ICON_MG1_X360.PNG.h"
#include "SaveIcon/ICON_MG2_X360.PNG.h"
BPE_CTASSERT( sizeof(binary_ICON_GAME_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_PHOTO_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_MG1_PNG_data) <= 15616 );
BPE_CTASSERT( sizeof(binary_ICON_MG2_PNG_data) <= 15616 );
// Put JP icons in namespace to avoid conflicts
namespace NSaveIconsJP
{
   #include "SaveIcon/JP/ICON_GAME_X360.PNG.h"
   #include "SaveIcon/JP/ICON_PHOTO_X360.PNG.h"
   #include "SaveIcon/JP/ICON_MG1_X360.PNG.h"
   #include "SaveIcon/JP/ICON_MG2_X360.PNG.h"
   BPE_CTASSERT( sizeof(binary_ICON_GAME_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_PHOTO_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_MG1_PNG_data) <= 15616 );
   BPE_CTASSERT( sizeof(binary_ICON_MG2_PNG_data) <= 15616 );
}

#endif

#else

#if MGS_VERSION==2
#include "SaveIcon/ICON_TANKER.PNG.h"
#include "SaveIcon/ICON_PLANT.PNG.h"
#include "SaveIcon/ICON_VR.PNG.h"
#include "SaveIcon/ICON_SNAKETALES.PNG.h"
#include "SaveIcon/ICON_PHOTO.PNG.h"

// Put JP icons in namespace to avoid conflicts
namespace NSaveIconsJP
{
   #include "SaveIcon/JP/ICON_TANKER.PNG.h"
   #include "SaveIcon/JP/ICON_PLANT.PNG.h"
   #include "SaveIcon/JP/ICON_VR.PNG.h"
   #include "SaveIcon/JP/ICON_SNAKETALES.PNG.h"
   #include "SaveIcon/JP/ICON_PHOTO.PNG.h"
}

#elif MGS_VERSION==3
#include "SaveIcon/ICON_GAME.PNG.h"
#include "SaveIcon/ICON_MG1.PNG.h"
#include "SaveIcon/ICON_MG2.PNG.h"
#include "SaveIcon/ICON_PHOTO.PNG.h"

// Put JP icons in namespace to avoid conflicts
namespace NSaveIconsJP
{
   #include "SaveIcon/JP/ICON_GAME.PNG.h"
   #include "SaveIcon/JP/ICON_MG1.PNG.h"
   #include "SaveIcon/JP/ICON_MG2.PNG.h"
   #include "SaveIcon/JP/ICON_PHOTO.PNG.h"
}

#endif

#endif

#define BP_MC_ICON_FILENAME "ICON0.PNG"

#if MGS_VERSION==2
#define BP_SAVE_GAME_SIZE (29098+MAX_BP_LINKVARBUF)
#elif MGS_VERSION==3
#define BP_SAVE_GAME_SIZE (18812+MAX_BP_LINKVARBUF)
#endif

// Vita save header is larger to accomodate the need to store fast-access transfarring information
#if BP_VITA
#define BP_SAVE_HEADER_SIZE 1024
#else
#define BP_SAVE_HEADER_SIZE 32
#endif

#if MGS_VERSION==2
#define BP_SAVE_PHOTO_SIZE (720*1280 + 4)
#elif MGS_VERSION==3
#define BP_SAVE_PHOTO_SIZE (720*1280 + 4)
#endif
#define BP_DATA_FILENAME_LENGTH (28+1)

#if MGS_VERSION==2
#define BP_DATA_FILENAME_FIRSTCHAR 'D'
#define BP_DATA_FILENAME_FIRSTCHARSTR "D"
#define BP_PS2_ICON_FILENAME "mgs2.ico"

//If this errors out on compile, it means that a change in trophy size resulted in change in SBP_LinkVars size
//In that case, appropriately adjust the size of STrophyDeltaBitfield in linkvar.h
BPE_CTASSERT( kTRP_Count == 46 );

#elif MGS_VERSION==3
#define BP_DATA_FILENAME_FIRSTCHAR '_'
#define BP_DATA_FILENAME_FIRSTCHARSTR "_"
#define BP_PS2_ICON_FILENAME "mgs3.ico"

//If this errors out on compile, it means that a change in trophy size resulted in change in SBP_LinkVars size
//In that case, appropriately adjust the size of STrophyDeltaBitfield in linkvar.h
BPE_CTASSERT( kTRP_Count == 46 );

#endif

static char sBP_SaveHeaderBuffer[BP_SAVE_HEADER_SIZE] = { 0 };
static char sBP_SaveDataBuffer[BP_SAVE_GAME_SIZE < BP_SAVE_PHOTO_SIZE ? BP_SAVE_PHOTO_SIZE : BP_SAVE_GAME_SIZE] = { 0 };

typedef enum
{
   kEMCF_Data,
   kEMCF_Info,
   kEMCF_Icon
} EExternalMCFile;

#if MGS_VERSION==2
//TODO: need to switch b/t tanker and plant icons for game savedata? (to match PS2)
static SMCFile gBP_MCFile_Icon_Game = { BP_MC_ICON_FILENAME,  binary_ICON_TANKER_PNG_data, sizeof(binary_ICON_TANKER_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_Game };
static SMCFile gBP_MCFile_Icon_VR = { BP_MC_ICON_FILENAME,  binary_ICON_VR_PNG_data, sizeof(binary_ICON_VR_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_VR };
static SMCFile gBP_MCFile_Icon_SnakeTales = { BP_MC_ICON_FILENAME,  binary_ICON_SNAKETALES_PNG_data, sizeof(binary_ICON_SNAKETALES_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_SnakeTales };
static SMCFile gBP_MCFile_Icon_Photo = { BP_MC_ICON_FILENAME,  binary_ICON_PHOTO_PNG_data, sizeof(binary_ICON_PHOTO_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_Photo };

// Function that switches save game icons between JP and NA/EU regions for HD Collection/Edition support
static void bp_switch_savedata_icons_for_sku(CBaseOsContext::EBuildSKU const buildSKU)
{
   switch (buildSKU)
   {
   case CBaseOsContext::kBS_Japan:
      // HD Edition icons
      {
         gBP_MCFile_Icon_Game.mData       = NSaveIconsJP::binary_ICON_TANKER_PNG_data;
         gBP_MCFile_Icon_Game.mSize       = sizeof(NSaveIconsJP::binary_ICON_TANKER_PNG_data);
         gBP_MCFile_Icon_VR.mData         = NSaveIconsJP::binary_ICON_VR_PNG_data;
         gBP_MCFile_Icon_VR.mSize         = sizeof(NSaveIconsJP::binary_ICON_VR_PNG_data);
         gBP_MCFile_Icon_SnakeTales.mData = NSaveIconsJP::binary_ICON_SNAKETALES_PNG_data;
         gBP_MCFile_Icon_SnakeTales.mSize = sizeof(NSaveIconsJP::binary_ICON_SNAKETALES_PNG_data);
         gBP_MCFile_Icon_Photo.mData      = NSaveIconsJP::binary_ICON_PHOTO_PNG_data;
         gBP_MCFile_Icon_Photo.mSize      = sizeof(NSaveIconsJP::binary_ICON_PHOTO_PNG_data);
      }
      break;
   default:
      // HD Collection icons
      {
         gBP_MCFile_Icon_Game.mData       = binary_ICON_TANKER_PNG_data;
         gBP_MCFile_Icon_Game.mSize       = sizeof(binary_ICON_TANKER_PNG_data);
         gBP_MCFile_Icon_VR.mData         = binary_ICON_VR_PNG_data;
         gBP_MCFile_Icon_VR.mSize         = sizeof(binary_ICON_VR_PNG_data);
         gBP_MCFile_Icon_SnakeTales.mData = binary_ICON_SNAKETALES_PNG_data;
         gBP_MCFile_Icon_SnakeTales.mSize = sizeof(binary_ICON_SNAKETALES_PNG_data);
         gBP_MCFile_Icon_Photo.mData      = binary_ICON_PHOTO_PNG_data;
         gBP_MCFile_Icon_Photo.mSize      = sizeof(binary_ICON_PHOTO_PNG_data);
      }
      break;
   }
   return;
}

#elif MGS_VERSION==3
static SMCFile gBP_MCFile_Icon_Game = { BP_MC_ICON_FILENAME,  binary_ICON_GAME_PNG_data, sizeof(binary_ICON_GAME_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_Game };
static SMCFile gBP_MCFile_Icon_MG1 = { BP_MC_ICON_FILENAME,  binary_ICON_MG1_PNG_data, sizeof(binary_ICON_MG1_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_MG1 };
static SMCFile gBP_MCFile_Icon_MG2 = { BP_MC_ICON_FILENAME,  binary_ICON_MG2_PNG_data, sizeof(binary_ICON_MG2_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_MG2 };
static SMCFile gBP_MCFile_Icon_Photo = { BP_MC_ICON_FILENAME,  binary_ICON_PHOTO_PNG_data, sizeof(binary_ICON_PHOTO_PNG_data), kMCFT_System, kInvalidAccessTime, kISD_Photo };

// Function that switches save game icons between JP and NA/EU regions for HD Collection/Edition support
static void bp_switch_savedata_icons_for_sku(CBaseOsContext::EBuildSKU const buildSKU)
{
   switch (buildSKU)
   {
   case CBaseOsContext::kBS_Japan:
      // HD Edition icons
      {
         gBP_MCFile_Icon_Game.mData    = NSaveIconsJP::binary_ICON_GAME_PNG_data;
         gBP_MCFile_Icon_Game.mSize    = sizeof(NSaveIconsJP::binary_ICON_GAME_PNG_data);
         gBP_MCFile_Icon_MG1.mData     = NSaveIconsJP::binary_ICON_MG1_PNG_data;
         gBP_MCFile_Icon_MG1.mSize     = sizeof(NSaveIconsJP::binary_ICON_MG1_PNG_data);
         gBP_MCFile_Icon_MG2.mData     = NSaveIconsJP::binary_ICON_MG2_PNG_data;
         gBP_MCFile_Icon_MG2.mSize     = sizeof(NSaveIconsJP::binary_ICON_MG2_PNG_data);
         gBP_MCFile_Icon_Photo.mData   = NSaveIconsJP::binary_ICON_PHOTO_PNG_data;
         gBP_MCFile_Icon_Photo.mSize   = sizeof(NSaveIconsJP::binary_ICON_PHOTO_PNG_data);
      }
      break;
   default:
      // HD Collection icons
      {
         gBP_MCFile_Icon_Game.mData    = binary_ICON_GAME_PNG_data;
         gBP_MCFile_Icon_Game.mSize    = sizeof(binary_ICON_GAME_PNG_data);
         gBP_MCFile_Icon_MG1.mData     = binary_ICON_MG1_PNG_data;
         gBP_MCFile_Icon_MG1.mSize     = sizeof(binary_ICON_MG1_PNG_data);
         gBP_MCFile_Icon_MG2.mData     = binary_ICON_MG2_PNG_data;
         gBP_MCFile_Icon_MG2.mSize     = sizeof(binary_ICON_MG2_PNG_data);
         gBP_MCFile_Icon_Photo.mData   = binary_ICON_PHOTO_PNG_data;
         gBP_MCFile_Icon_Photo.mSize   = sizeof(binary_ICON_PHOTO_PNG_data);
      }
      break;
   }
   return;
}

#endif

//The single kMCFT_SaveGame file, DATA.BIN, contains the actual savegame data, equivalent to the D* base-64 filename
//file on PS2.  We add a MASTER.BIN "info" file that contains the MCMAN_INFODATA information needed to index savegames,
//which was encoded into the data filename on PS2 but cannot be on PS3 because they have awesome MS-DOS filename restrictions
//on their files and insufficient invisible user space to store it in the PARAM.SFO.
static SMCFile gBP_MCFiles[] =
{
   { "DATA.BIN",  &sBP_SaveDataBuffer[0], 0, kMCFT_SaveGame, kInvalidAccessTime },
   { "MASTER.BIN",  &sBP_SaveHeaderBuffer[0], sizeof(sBP_SaveHeaderBuffer), kMCFT_Master, kInvalidAccessTime },
   { BP_MC_ICON_FILENAME,  NULL, 0, kMCFT_System, kInvalidAccessTime },
};

static const int skMCFileCount = sizeof( gBP_MCFiles ) / sizeof( *gBP_MCFiles );

//"Game progress" type savedata records for which there must be at least one present or space for
// one of each on the PS3 HDD to allow the player to start up the game.
static SMCStartupCheckDir gBP_MCStartupCheckDirs[] =
{
#if MGS_VERSION==2
   { "000G", 0 }, //Game
   { "000V", 0 }, //Missions
   { "000S", 0 }, //Snake Tales
#elif MGS_VERSION==3
   { "1G", 0 },   //Game
   { "11", 0 },   //Metal Gear 1
   { "12", 0 },   //Metal Gear 2
#endif
};

static const int skMCStartupCheckDirCount = sizeof( gBP_MCStartupCheckDirs ) / sizeof( *gBP_MCStartupCheckDirs );

//Every savegame has the following files in its save directory regardless of type.
//They are also read / written in this order.
typedef enum
{
   kIMCF_MgsIco,     //mgs*.ico
   kIMCF_IconSys,    //icon.sys
   kIMCF_Padding,    //named same as savegame folder; garbage file only present to make plant savegames the same total size as tanker savegames
   kIMCF_Data,       //actual savegame data, the only one we care about.

   kIMCF_Count,

   kIMCF_FirstValid = 0,
   kIMCF_LastValid = kIMCF_Count-1

} EInternalMCFile;

typedef enum
{
   kOpenMode_Read,
   kOpenMode_Write
} EInternalMCFileOpenMode;

typedef struct _SInternalMCFile
{
   EInternalMCFile   mWhichFile;
   SMCFile *         mpExternalMCFile;
} SInternalMCFile;

//Of the following files, only the Data one actually reads / writes data in our version.
static SInternalMCFile sInternalMCFiles[kIMCF_Count] =
{
   { kIMCF_MgsIco, NULL },          //ignored
   { kIMCF_IconSys, NULL },         //ignored
   { kIMCF_Padding, NULL },         //ignored
   { kIMCF_Data, &gBP_MCFiles[kEMCF_Data] }
};

static SInternalMCFile * spInternalFd = NULL;   //pointer to currently open file (only one allowed at once)
static EInternalMCFileOpenMode sInternalOpenMode = kOpenMode_Read;
static EInternalSaveData sWhichInternalSaveData = kISD_Game;
#if MGS_VERSION==3
static ESaveGameTitle gBP_Save_Game_Title = kSGT_Count;
#endif

static int bp_extract_subdir_suffix( char * const dirsuffix, const char * const path )
{
   //Check against original PS2 base.
   const int basefilenamelen = strlen(bp_basefilename);
   if( ( path[0] == '/' ) && !strncmp( path+1, bp_basefilename, basefilenamelen ) )
   {
      //Matches the pattern "/{PS2 product code}{suffix}"
      //Fill in just the suffix and return true.
      strcpy( dirsuffix, path+1+basefilenamelen );
      return 1;
   }

   //Check against CP4 base.
   const int rootfolderlen = strlen(gMGS_SaveStatus.mSaveDirectoryRoot);
   if( ( path[0] == '/' ) && !strncmp( path+1, gMGS_SaveStatus.mSaveDirectoryRoot, rootfolderlen ) )
   {
      //Matches the pattern "/{CP4 savegame root}{suffix}"
      //Fill in just the suffix and return true.
      strcpy( dirsuffix, path+1+rootfolderlen );
      return 1;
   }

   return 0;
}

static const char * bp_internal_skip_fullpath_mc( const char * const fullpath )
{
   const char * name = fullpath;
   //MGS3 uses fullpaths for filenames, so handle stripping out the parent folder here.
   int basefilenamelen = strlen(bp_basefilename);
   //These fullpaths are in the format "/BASLUS-213591G000004/BASLUS-213591G000004"
   //Product code is BASLUS-21359
   //Version and type is 1G
   //Index is the following 6 digits
   if( ( fullpath[0] == '/' ) && !strncmp(fullpath+1, bp_basefilename, basefilenamelen ) && ( strchr(fullpath+1,'/') == fullpath+1+basefilenamelen+2+6 ) )
   {
      //+1 to skip over first /
      //+2 to skip over version and type
      //+6 to skip over index
      //+1 to skip over second /
      name += 1 + basefilenamelen + 2 + 6 + 1;
   }

   return name;
}

static void bp_internal_get_dir_mc_callback( CMGS_SaveStatus * pSS )
{
   if( pSS->CompletedOK() )
   {
      //Need to convert retrieved savegame folder entries to original PS2 names--
      //PS3/XBox360 savegame prefix will be replaced with PS2 product code.
      for( int i=0; i < MGS_SaveStatus_GetNumDirEntries(); ++i )
      {
#if MGS_VERSION==2
         sceMcTblGetDir * tbl = pSS->mpGetDirTable+i;
         char buf[sizeof(tbl->EntryName)];
         char * entryName = (char*)tbl->EntryName;
#elif MGS_VERSION==3
         SceMc2DirParam * tbl = pSS->mpGetDirTable2+i;
         char buf[sizeof(tbl->name)];
         char * entryName = (char*)tbl->name;
#endif
         const char * newProductCode;
         if( newProductCode = strstr( entryName, pSS->mSaveDirectoryRoot ) )
         {
            const char * suffix = entryName + strlen( pSS->mSaveDirectoryRoot );
            strcpy( buf, bp_basefilename );
            strcat( buf, suffix );
            strcpy( entryName, buf );
         }
      }
   }
}

#if MGS_VERSION==3
static void bp_getFileTime(SceMc2DateParam* OutpPS2Date, const time_t* InpBPDate)
{
#if defined(BP_PS3) || defined (BP_VITA)
   struct tm * systemTime = localtime(InpBPDate);
   OutpPS2Date->year = systemTime->tm_year-100+2000;
   OutpPS2Date->month = systemTime->tm_mon+1;
   OutpPS2Date->day = systemTime->tm_mday;
   OutpPS2Date->hour = systemTime->tm_hour;
   OutpPS2Date->min = systemTime->tm_min;
   OutpPS2Date->sec = systemTime->tm_sec;
#elif defined(BP_360)
   SYSTEMTIME systemTime;
   FileTimeToSystemTime((FILETIME *)InpBPDate, &systemTime);
   OutpPS2Date->year = systemTime.wYear;
   OutpPS2Date->month = systemTime.wMonth;
   OutpPS2Date->day = systemTime.wDay;
   OutpPS2Date->hour = systemTime.wHour;
   OutpPS2Date->min = systemTime.wMinute;
   OutpPS2Date->sec = systemTime.wSecond;
#endif
}
#endif

static void bp_internal_get_savedata_mc_callback( CMGS_SaveStatus * pSS )
{
   if( !pSS->CompletedOK() )
   {
#if MGS_VERSION==3
      //MGS3 does a file listing of the folder it's about to save a new game to.
      //In this case, clear out the "nodata" or "no space" errorcode that we got back.
      if( pSS->mCallbackResult == CMGS_SaveStatus::kCB_NoData || pSS->mCallbackResult == CMGS_SaveStatus::kCB_NoSpace )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
         *pSS->mpGetDirListExistEntries = 0;
      }
      else if( pSS->mCallbackResult == CMGS_SaveStatus::kCB_Corrupted )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
         *pSS->mpGetDirListExistEntries = 1;

         SceMc2DirParam * nextFile = pSS->mpGetDirTable2;
         strncpy( (char*)nextFile->name, "", sizeof( nextFile->name )-1 );
      }
#else
      if( pSS->mCallbackResult == CMGS_SaveStatus::kCB_NoData || pSS->mCallbackResult == CMGS_SaveStatus::kCB_NoSpace )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
         pSS->mGetDirListExistEntries = 0;
      }
      else if( pSS->mCallbackResult == CMGS_SaveStatus::kCB_Corrupted )
      {
         pSS->mCallbackResult = CMGS_SaveStatus::kCB_OK;
         pSS->mGetDirListExistEntries = 1;

         const char * const ps2Filename = (const char*)pSS->mpMCFiles[kEMCF_Info].mData;
         memset( pSS->mpGetDirTable, 0, pSS->mGetDirListMaxEntries * sizeof( *pSS->mpGetDirTable ) );
         sceMcTblGetDir * nextFile = pSS->mpGetDirTable;
         strncpy( (char*)nextFile->EntryName, ps2Filename, sizeof( nextFile->EntryName )-1 );
      }
#endif
   }
   else
   {
      //This step transforms the result of save_data_auto_load() into what the game expects to get back from sceMcGetDir()
      //performed on a savegame subfolder.  If it succeeded, we will have successfully found all the files of gBP_MCFiles[]
      //and read the contents of gBP_MCFiles[kEMCF_Info].
      //We need to fill in the sceMcTblGetDir* array ourselves now.  It should contain one entry, the correct filename
      //of the PS2 savedata file that is used for indexing at this step.
      const char * const ps2Filename = (const char*)pSS->mpMCFiles[kEMCF_Info].mData;

      int sizeAdjust = 0;
      //Hack to fix size to what's expected after taking into account injected BP save data.
      if( sWhichInternalSaveData == kISD_Game
#if MGS_VERSION==2
         || sWhichInternalSaveData == kISD_VR
         || sWhichInternalSaveData == kISD_SnakeTales
#endif
         )
      {
         sizeAdjust = -MAX_BP_LINKVARBUF;
      }

#if MGS_VERSION==2
      pSS->mGetDirListExistEntries = 1;
      memset( pSS->mpGetDirTable, 0, pSS->mGetDirListMaxEntries * sizeof( *pSS->mpGetDirTable ) );
      sceMcTblGetDir * nextFile = pSS->mpGetDirTable;
      strncpy( (char*)nextFile->EntryName, ps2Filename, sizeof( nextFile->EntryName )-1 );
      nextFile->FileSizeByte = pSS->mpMCFiles[kEMCF_Data].mSize + sizeAdjust;
      nextFile->_Modify = pSS->mpMCFiles[kEMCF_Data].mFileModifiedTime;

#elif MGS_VERSION==3
      *pSS->mpGetDirListExistEntries = 1;
      memset( pSS->mpGetDirTable2, 0, pSS->mGetDirListMaxEntries * sizeof( *pSS->mpGetDirTable2 ) );
      SceMc2DirParam * nextFile = pSS->mpGetDirTable2;
      strncpy( (char*)nextFile->name, ps2Filename, sizeof( nextFile->name )-1 );
      if( pSS->mpMCFiles[kEMCF_Data].mInternalFileType == kISD_Photo )
      {
         nextFile->size = pSS->mpMCFiles[kEMCF_Data].mGenericResult;
      }
      else
      {
         nextFile->size = pSS->mpMCFiles[kEMCF_Data].mSize + sizeAdjust;
      }
      bp_getFileTime(&nextFile->modification, &pSS->mpMCFiles[kEMCF_Data].mFileModifiedTime);

      if( pSS->mGetSubdirExactFilename[0] != 0 )
      {
         //HACK to handle weird request for listing of "padding" file when saving in MGS3 album mode.
         //Stored expected result of a dir search for a specific file.  Report it back now.
         strncpy( (char*)nextFile->name, pSS->mGetSubdirExactFilename, sizeof( nextFile->name )-1 );
         nextFile->size = 29;
      }

#endif

   }
}

static void bp_internal_fixup_bp_linkvars( SBP_LinkVars* pBPLV )
{
   if( pBPLV->mVersion < BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_VALID_FLAG )
   {
      // Consider trophies valid for all old versions
      pBPLV->mTrophyValidFlag = BPLinkvarDefines::kTrophiesValid;
   }

   // If the version is zero, this is a new game and the version wasn't set. Don't clear the trophy bitfield.
   if( pBPLV->mVersion != 0 && pBPLV->mVersion < BP_FIRST_LINKVAR_VERSION_WITH_TROPHY_DELTA )
   {
      // Zero out trophy delta bitfield
      memset( &pBPLV->mTrophyDeltaBitfield, 0, sizeof (pBPLV->mTrophyDeltaBitfield) );
   }

   if( pBPLV->mVersion != 0 &&pBPLV->mVersion < BP_FIRST_LINKVAR_VERSION_WITH_ENCRYPTED_PSN_ID )
   {
      // Clear originator data since we're hashing the PSN ID differently now
      memset( pBPLV->mOriginalConsolePSID, 0, sizeof(pBPLV->mOriginalConsolePSID) );
      memset( pBPLV->mOriginalPSNAccount, 0, sizeof(pBPLV->mOriginalPSNAccount) );
      
      //Set the console type to legacy so that we do not disable trophies for old PS3 saves.
#if BP_PS3
      pBPLV->mOriginalConsoleType = BPLinkvarDefines::kConsoleLegacyPS3;
#else
      pBPLV->mOriginalConsoleType = BPLinkvarDefines::kConsoleInvalid;
#endif
   }

   pBPLV->mCookie = BP_LINKVAR_COOKIE;
   pBPLV->mVersion = BP_LINKVAR_VERSION;
   pBPLV->mSize = sizeof( bp_linkvars );
}

static void bp_internal_do_read_savedata( CMGS_SaveStatus * pSS )
{
   BPE_ASSERT_NO_MSG( pSS->mpReadWriteBuf != NULL );
   //"read" from this file.
   memcpy( pSS->mpReadWriteBuf, (char*)spInternalFd->mpExternalMCFile->mData + spInternalFd->mpExternalMCFile->mOpenFileLoc, pSS->mReadWriteSize );
   spInternalFd->mpExternalMCFile->mOpenFileLoc += pSS->mReadWriteSize;

   //Hack to inject BP added persistent storage to the end of the "Data" file, just for the game type save data.
   if( spInternalFd->mWhichFile == kIMCF_Data )
   {
      if( sWhichInternalSaveData == kISD_Game
#if MGS_VERSION==2
         || sWhichInternalSaveData == kISD_VR
         || sWhichInternalSaveData == kISD_SnakeTales
#endif
         )
      {
         if( spInternalFd->mpExternalMCFile->mOpenFileLoc + MAX_BP_LINKVARBUF == spInternalFd->mpExternalMCFile->mSize )
         {
            memcpy( &sv_bp_linkvars, (char*)spInternalFd->mpExternalMCFile->mData + spInternalFd->mpExternalMCFile->mOpenFileLoc, sizeof( sv_bp_linkvars ) );
#ifndef GOLD_VERSION
            //TED - failsafe code to make sure that old unversioned BP linkvar savedata just gets cleared out.  By the time we ship all savedata will be up to date anyway.
            if( sv_bp_linkvars.mCookie != BP_LINKVAR_COOKIE )
            {
               memset( &sv_bp_linkvars, 0, sizeof( sv_bp_linkvars ) );
            }
#endif
            //TODO: Here is where to handle bp linkvar savedata version checking / fixup.

            if( sv_bp_linkvars.mVersion > BP_LINKVAR_VERSION )
            {
               pSS->mCallbackResult = CMGS_SaveStatus::kCB_Corrupted;
               memset( &sv_bp_linkvars, 0, sizeof(sv_bp_linkvars) );
            }
            else
            {
               bp_internal_fixup_bp_linkvars( &sv_bp_linkvars );
            }
         }
      }
   }

}

static void bp_internal_read_savedata_mc_callback( CMGS_SaveStatus * pSS )
{
   if( pSS->CompletedOK() )
   {
      bp_internal_do_read_savedata( pSS );
   }
#if MGS_VERSION==3
   //autoclose the file.
   MGS_SaveStatus_Close();
#endif
}

static void bp_internal_wrote_savedata_mc_callback( CMGS_SaveStatus * pSS )
{
#if MGS_VERSION==3
   //autoclose the file.
   MGS_SaveStatus_Close();
#endif
}

extern "C" void bp_set_current_savegame( const char * const dirsuffix )
{
   EInternalSaveData internalFileType = kISD_Game;
   int dataSize = 0;
   SMCFile * pIconFile = NULL;
   if( strchr( dirsuffix, 'G' ) )
   {
      //Game data
      internalFileType = kISD_Game;
      dataSize = BP_SAVE_GAME_SIZE;
      pIconFile = &gBP_MCFile_Icon_Game;
      sWhichInternalSaveData = kISD_Game;
#if MGS_VERSION==3
      gBP_Save_Game_Title = kSGT_MGS3;
#endif
   }
#if MGS_VERSION==2
   else if( strchr( dirsuffix, 'V' ) )
   {
      //VR data
      internalFileType = kISD_VR;
      dataSize = BP_SAVE_GAME_SIZE;
      pIconFile = &gBP_MCFile_Icon_VR;
      sWhichInternalSaveData = kISD_VR;
   }
   else if( strchr( dirsuffix, 'S' ) )
   {
      //Snake Tales data
      internalFileType = kISD_SnakeTales;
      dataSize = BP_SAVE_GAME_SIZE;
      pIconFile = &gBP_MCFile_Icon_SnakeTales;
      sWhichInternalSaveData = kISD_SnakeTales;
   }
   else if( strchr( dirsuffix, 'P' ) )
   {
      //Photo data
      internalFileType = kISD_Photo;
      dataSize = BP_SAVE_PHOTO_SIZE;
      pIconFile = &gBP_MCFile_Icon_Photo;
      sWhichInternalSaveData = kISD_Photo;
   }
#elif MGS_VERSION==3
   else if( strchr( dirsuffix, 'P' ) )
   {
      //Photo data
      internalFileType = kISD_Photo;
      dataSize = BP_SAVE_PHOTO_SIZE;
      pIconFile = &gBP_MCFile_Icon_Photo;
      sWhichInternalSaveData = kISD_Photo;
      gBP_Save_Game_Title = kSGT_MGS3;
   }
   else if ( dirsuffix[ 1 ] == '1' )
   {
      //MG1 savegame.
      internalFileType = kISD_MG1;
      dataSize = 203;                           // This must match the game data's size!
      pIconFile = &gBP_MCFile_Icon_MG1;
      sWhichInternalSaveData = kISD_MG1;
      gBP_Save_Game_Title = kSGT_MG1;
   }
   else if ( dirsuffix[ 1 ] == '2' )
   {
      //MG2 savegame
      internalFileType = kISD_MG2;
      dataSize = 344;                           // This must match the game data's size!
      pIconFile = &gBP_MCFile_Icon_MG2;      // <- This should be set to something else!
      sWhichInternalSaveData = kISD_MG2;
      gBP_Save_Game_Title = kSGT_MG2;
   }
#endif
   else
   {
      printf("Unknown savegame type: %s\n", dirsuffix );
      BP_TODO_BREAK;
   }

   if( dataSize > sizeof( sBP_SaveDataBuffer ) )
   {
      BP_BREAK;
   }

   //Set up the MCFile array for this type.
   gBP_MCFiles[kEMCF_Data].mSize = dataSize;
   gBP_MCFiles[kEMCF_Data].mInternalFileType = internalFileType;
   gBP_MCFiles[kEMCF_Icon] = *pIconFile;
}

extern "C" void bp_set_current_savegame_adv( const char * const dirsuffix, int dataSize )
{
   bp_set_current_savegame(dirsuffix);
   gBP_MCFiles[kEMCF_Data].mSize = dataSize;
}

//----------------------------------------------------------------------------

void MGS_SaveStatus_Init()
{
   gMGS_SaveStatus.ResetAll(gBP_MCFiles, skMCFileCount, gBP_MCStartupCheckDirs, skMCStartupCheckDirCount);

#if MGS_VERSION==2
   sprintf(gMGS_SaveStatus.mSaveDirectoryRoot, "%s_MGS2", gpOsContext->mTitleId_SaveData);
#elif MGS_VERSION==3
   sprintf(gMGS_SaveStatus.mSaveDirectoryRoot, "%s_MGS3", gpOsContext->mTitleId_SaveData);
#else
#error TODO!
#endif
   // Switch between HD Collection and HD Edition
   bp_switch_savedata_icons_for_sku(gpOsContext->mBuildSKU);

   save_data_init( gMGS_SaveStatus.mSaveDirectoryRoot );

   //This starts up the initial check for existing savedata so we can calculate req. space.
   for( int i=0; i < skMCStartupCheckDirCount; ++i )
   {
      //Calculate required space for each save type before doing the check.
      bp_set_current_savegame( gBP_MCStartupCheckDirs[i].mDirNamePrefix );
      gBP_MCStartupCheckDirs[i].mContentsSizeKB = gMGS_SaveStatus.GetMCFilesTotalSizeInKB();
   }
   save_data_space_check( &gMGS_SaveStatus );
}

extern "C" int MGS_Get360SpaceForFileType(const char* saveType)
{
#if defined(BP_360)
   gMGS_SaveStatus.ResetAll(gBP_MCFiles, skMCFileCount, gBP_MCStartupCheckDirs, skMCStartupCheckDirCount);

#if MGS_VERSION==2
   sprintf(gMGS_SaveStatus.mSaveDirectoryRoot, "%s_MGS2", gpOsContext->mTitleId_SaveData);
#elif MGS_VERSION==3
   sprintf(gMGS_SaveStatus.mSaveDirectoryRoot, "%s_MGS3", gpOsContext->mTitleId_SaveData);
#else
#error TODO!
#endif
   save_data_init( gMGS_SaveStatus.mSaveDirectoryRoot );

   bp_set_current_savegame( saveType );
   const int totalRawSpace = gMGS_SaveStatus.GetMCFilesTotalSizeExcludeSystem();
   return XContentCalculateSize(totalRawSpace, 1); // 1 directory
#else
   return -1;
#endif
}

void MGS_SaveStatus_ResetStatus()
{
   gMGS_SaveStatus.ResetStatus();
}

int MGS_SaveStatus_IsInitialized()
{
   return gMGS_SaveStatus.mpMCFiles!=NULL;
}

int MGS_SaveStatus_IsProcessing()
{
   return gMGS_SaveStatus.mStatus == CMGS_SaveStatus::kS_Processing;
}

int MGS_SaveStatus_IsDone()
{
   return gMGS_SaveStatus.IsDone();
}

int MGS_SaveStatus_CompletedOK()
{
   return gMGS_SaveStatus.CompletedOK();
}

int MGS_SaveStatus_HasNoData()
{
   return gMGS_SaveStatus.mCallbackResult == CMGS_SaveStatus::kCB_NoData;
}

int MGS_SaveStatus_ExtraSpaceReq()
{
   return gMGS_SaveStatus.mHDSizeReqKB;
}

int MGS_SaveStatus_HasNoSpace()
{
   return gMGS_SaveStatus.mCallbackResult == CMGS_SaveStatus::kCB_NoSpace;
}

int MGS_SaveStatus_IsCorrupted()
{
   return gMGS_SaveStatus.mCallbackResult == CMGS_SaveStatus::kCB_Corrupted || 
      gMGS_SaveStatus.mCallbackResult == CMGS_SaveStatus::kCB_CELL_Error;
}

int MGS_SaveStatus_WrongUser()
{
   return gMGS_SaveStatus.mWrongUser;
}

void MGS_SaveStatus_ClearWrongUser()
{
   gMGS_SaveStatus.mWrongUser = 0;
   BP_TrophySystem_Enable();
}

int MGS_SaveStatus_GetNumDirEntries()
{
#if MGS_VERSION==2
   return gMGS_SaveStatus.mGetDirListExistEntries;
#else
   return *gMGS_SaveStatus.mpGetDirListExistEntries;
#endif
}

const char * MGS_SaveStatus_GetSaveDirectoryRoot()
{
   return gMGS_SaveStatus.mSaveDirectoryRoot;
}

void MGS_SaveStatus_SetChosenGameSave( const int saveIndex )
{
   gMGS_SaveStatus.mChosenGameSave = saveIndex;
}

#if MGS_VERSION==2
unsigned int MGS_SaveStatus_GetDir(const char *name, int maxent, sceMcTblGetDir *table)
#elif MGS_VERSION==3
unsigned int MGS_SaveStatus_GetDir2(const char *name, int maxent, SceMc2DirParam *table, int * const pOutEntCount)
#endif
{
   gMGS_SaveStatus.ResetStatus();

   gMGS_SaveStatus.mGetDirListMaxEntries = maxent;
#if MGS_VERSION == 2
   gMGS_SaveStatus.mpGetDirTable = table;
#elif MGS_VERSION == 3
   gMGS_SaveStatus.mpGetDirTable2 = table;
   gMGS_SaveStatus.mpGetDirListExistEntries = pOutEntCount;
#endif

   char fullPath[FILENAME_MAX] = { 0 };
   if( name[0] != '/' )
   {
      //Looking in current folder.  Convert to a full path.
      BPE_ASSERT( strchr( name, '/' ) == 0, "unexpected pattern" );
      sprintf( fullPath, "/%s/%s", gMGS_SaveStatus.mSaveDirectory, name );
   }
   else
   {
      //Already a full path.
      strcpy( fullPath, name );
   }

   char dirsuffix[FILENAME_MAX];
   if( bp_extract_subdir_suffix( dirsuffix, fullPath ) )
   {
      if( !strchr( dirsuffix, '/' ) )
      {
         //Looking for a list of directories / savegames for a particular type.
         //dirsuffix pattern should look something like "000G*"

         //Strip out the wildcard at the end.
         if( dirsuffix[strlen(dirsuffix)-1] != '*' )
            BP_TODO_BREAK;
         dirsuffix[strlen(dirsuffix)-1] = 0;

         //Special case for initial savegame check on game startup-- it looks for all savegames
         //regardless of type using the following wildcard which we need to strip out.
         if( !strcmp( dirsuffix, "????" ) )
         {
            dirsuffix[0] = 0;
         }
         //This should result in a directory "prefix" in the form "000*" where * is [G|V|S|P].
         //The actual savegames for each type will be this followed by a 3-digit index.
         gMGS_SaveStatus.SetChosenGameSaveDirSuffix( dirsuffix );

         save_data_get_dir_list( &gMGS_SaveStatus, bp_internal_get_dir_mc_callback );
         return 1;
      }
      else
      {
         //Getting a directory listing of a subfolder / savegame record.
         //dirsuffix pattern should look something like "G000/D*"
         //This is the interesting case because we have to transform what we've saved on the PS3
         //file system to a pretend listing of our internal filename.
         //Unfortunately we had to save this info in the *contents* of a separate "info" file
         //because the PS3 savedata filenames can't handle it.

         if( gMGS_SaveStatus.mRecreatingDeletedSaveData )
         {
            //MGS3 actually does a directory listing after deleting a folder so we have to pretend
            //that we deleted the data so it will go to the next step of recreating the save.
#if MGS_VERSION==2
            gMGS_SaveStatus.mGetDirListExistEntries = 0;
#else
            *gMGS_SaveStatus.mpGetDirListExistEntries = 0;
#endif
            return 0;
         }

         //Mangle the pattern and start a load of just the header file.
         char * endofpattern = strrchr( dirsuffix, '/' );
         if( strstr( dirsuffix, "/" BP_DATA_FILENAME_FIRSTCHARSTR "*" ) == endofpattern )
         {
            //Generic wildcard search for *data* file (/_* in MGS3 or /D* in MGS2)
            memset( gMGS_SaveStatus.mGetSubdirExactFilename, 0, sizeof( gMGS_SaveStatus.mGetSubdirExactFilename ) );
         }
         else if( !strncmp( name, endofpattern, strlen(endofpattern) ) )
         {
            //Search for specific "padding" file (named same as save folder) used in MGS3 album mode
            //N.B. still named by the PS2 product code, not ours.  We will just report the same name
            //back in the bp_internal_get_savedata_mc_callback()
            strncpy( gMGS_SaveStatus.mGetSubdirExactFilename, endofpattern+1, sizeof( gMGS_SaveStatus.mGetSubdirExactFilename ) );
         }
         else
         {
            //unexpected pattern
            BP_TODO_BREAK;
         }
         //This should leave us with the dirsuffix of the savegame.
         *endofpattern = 0;

         //Set up the folder for the savegame.
         gMGS_SaveStatus.SetChosenGameSaveDirSuffix( dirsuffix );

         //Set up gBP_MCFiles appropriately for this savegame.
         bp_set_current_savegame( dirsuffix );

         //Force the savegame system to only load the "master" file which is our "info" file.
         MGS_SaveStatus_SetChosenGameSave( -1 );
         //For this step, we don't want the wrong user flag to be set since we're just "looking at" the directory contents,
         //not loading it for use by the game.  Temporarily disable wrong user detection.
         gMGS_SaveStatus.mWrongUserDetectDisabled = 1;
         save_data_auto_load( &gMGS_SaveStatus, bp_internal_get_savedata_mc_callback );
         return 1;
      }
   }
#if MGS_VERSION==2
   else if( strstr( name, ofilename ) )
   {
      //This search is done when entering "Missions" and seems to be to check for VR mission data
      //from another PS2 product code.  Instantly return no results for this case.
      gMGS_SaveStatus.mGetDirListExistEntries = 0;
      return 0;
   }
#endif
   else
   {
      //Unexpected pattern.
      BP_TODO_BREAK;
      return 0;
   }
}

int MGS_SaveStatus_GetDirResult()
{
   return MGS_SaveStatus_GetNumDirEntries();
}

void MGS_SaveStatus_ChDir( const char * const path )
{
   gMGS_SaveStatus.ResetStatus();

   char dirsuffix[32];
   if( !bp_extract_subdir_suffix( dirsuffix, path ) )
   {
      //Unexpected case
      BP_TODO_BREAK;
   }

   //Copy across subfolder as a suffix to our saveload interface.
   //This subfolder name, concatenated with the titleid-based root name,
   //is the full folder name that will be used for the savegame.
   gMGS_SaveStatus.SetChosenGameSaveDirSuffix( dirsuffix );

   //set up gBP_MCFiles appropriately for this savegame.
   bp_set_current_savegame( dirsuffix );
}

void MGS_SaveStatus_Open( const char * const fullpath, const int mode )
{
   gMGS_SaveStatus.ResetStatus();

   EInternalMCFile whichFile = kIMCF_Count;

   const char * name = bp_internal_skip_fullpath_mc( fullpath );

   if( spInternalFd )
      BP_BREAK;   //the internal file should not already be open.

   if( mode == SCE_RDONLY )
   {
      sInternalOpenMode = kOpenMode_Read;
   }
   else
   {
      sInternalOpenMode = kOpenMode_Write;
   }

   if( !strcmp( name, BP_PS2_ICON_FILENAME ) )
   {
      whichFile = kIMCF_MgsIco;
   }
   else if( !strcmp( name, "icon.sys" ) )
   {
      whichFile = kIMCF_IconSys;
   }
   else if( !strncmp( name, bp_basefilename, strlen( bp_basefilename ) ) )
   {
      //This filename always starts with the base filename for the game (ps2 product code).
      whichFile = kIMCF_Padding;
   }
   else if( ( name[0] == BP_DATA_FILENAME_FIRSTCHAR ) && ( strlen(name) == BP_DATA_FILENAME_LENGTH ) )
   {
      //This filename starts with D, followed by 28 characters of base64 text data.
      whichFile = kIMCF_Data;
      //We cannot set the actual filename to this string because of filename restrictions on PS3.
      //Instead, we'll use the generic filename "DATA.BIN" and instead copy the contents of this
      //string to the buffer that will be written out externally as "MASTER.BIN".
      if(sInternalOpenMode == kOpenMode_Write)
      {
         SMCFile * pHeaderFile = &gBP_MCFiles[kEMCF_Info];
         strncpy( (char*)pHeaderFile->mData, name, BP_DATA_FILENAME_LENGTH );

#if BP_VITA
         // Zero out the existing transfarring info
         memset((char*)pHeaderFile->mData + 32, 0, BP_SAVE_HEADER_SIZE - 32);

         // Add the transfarring data
         int directoryLen = strlen(gMGS_SaveStatus.mSaveDirectory);
#if MGS_VERSION == 2
         const char* substring = gMGS_SaveStatus.mSaveDirectory + directoryLen - 4;
#endif
#if MGS_VERSION == 3
         const char* substring = gMGS_SaveStatus.mSaveDirectory + directoryLen - 7;
#endif

         char gameType = substring[0];
         substring += 1;

         ESaveType saveTypeIndex = kST_MaxSaveTypes;
         switch(gameType)
         {
         case 'G':
            saveTypeIndex = kST_Game;
            break;
#if MGS_VERSION == 2
         case 'V':
            saveTypeIndex = kST_VR;
            break;
         case 'S':
            saveTypeIndex = kST_SnakeTales;
            break;
#endif
#if MGS_VERSION == 3
#endif
         };

         // Only save Transfarring data for eligible types
         if (saveTypeIndex != kST_MaxSaveTypes)
         {
            char saveIndex[32];
            strcpy(saveIndex, substring);

            int saveIndexNum = Transfarring_SaveIndexStringToInt(saveIndex);

            TransfarringID tID = CTransfarringManager::Instance()->GetTransfarringID(saveTypeIndex, saveIndexNum);
            if (tID.m_UniqueID[0] != 0 && tID.m_UniqueID[1] != 0)
            {
               tID.WriteToString((char*)pHeaderFile->mData + 32);
            }
         }
#endif // BP_VITA
      }
      //else we won't get this information until the savegame listing step.
   }
   else
   {
      printf( "Warning!: Unknown file type\n" );
      whichFile = kIMCF_Data;
      // BP-Dak: This is a very special case on X360, you can sign out of the active profile during a load, this will cause the load to fail
      // and will report back a filename as "". This case was never properly handled on PS2 as it was impossible to "corrupt a save after a directory listing" 
      // without pulling out the memory card (pulling out the memory card will force you to exit the save/load screen and go back to port select on PS2)
      // This is not an unhanded file name case for this condition, it is simple a failure to load after a directory listing has succeeded
      //BP_TODO_BREAK;
   }

   printf( "MGS_SaveStatus_Open: %s %d\n", fullpath, mode );

   spInternalFd = &sInternalMCFiles[whichFile];
   if( spInternalFd->mpExternalMCFile )
   {
      //Initialize the file we're working with.
      spInternalFd->mpExternalMCFile->mOpenFileLoc = 0;
   }
}

void MGS_SaveStatus_Close()
{
   if( !spInternalFd )
      BP_BREAK;

   spInternalFd = NULL;
}

int MGS_SaveStatus_CloseReturnError()
{
   if ( !spInternalFd )
   {
      return -1;
   }
   else
   {
      spInternalFd = NULL;
      return 0;
   }
}

unsigned int MGS_SaveStatus_Write( const void *buff, unsigned int size )
{
   if( !spInternalFd || ( sInternalOpenMode != kOpenMode_Write ) )
      BP_BREAK;

   if( spInternalFd->mpExternalMCFile )
   {
      if( spInternalFd->mpExternalMCFile->mOpenFileLoc + size > spInternalFd->mpExternalMCFile->mSize )
      {
         //internal error - buffer isn't big enough for expected file size.
         BP_BREAK;
      }

      //"write" to this file.
      memcpy( (char*)spInternalFd->mpExternalMCFile->mData + spInternalFd->mpExternalMCFile->mOpenFileLoc, buff, size );
      spInternalFd->mpExternalMCFile->mOpenFileLoc += size;

      //Hack to inject BP added persistent storage to the end of the "Data" file, just for the game type save data.
      if( spInternalFd->mWhichFile == kIMCF_Data )
      {
         if( sWhichInternalSaveData == kISD_Game
#if MGS_VERSION==2
            || sWhichInternalSaveData == kISD_VR
            || sWhichInternalSaveData == kISD_SnakeTales
#endif
            )
         {
            if( spInternalFd->mpExternalMCFile->mOpenFileLoc + MAX_BP_LINKVARBUF == spInternalFd->mpExternalMCFile->mSize )
            {
               //Write this data with the last of the game's savedata being written.
               memset( (char*)spInternalFd->mpExternalMCFile->mData + spInternalFd->mpExternalMCFile->mOpenFileLoc, 0, MAX_BP_LINKVARBUF );
               
               // Originally this was done at load time, but there's cases where you can save a file without loading it,
               // such as by receiving a file from the cloud, or from WiFi transfarring, so we need to do the fixup here too.
               bp_internal_fixup_bp_linkvars( &bp_linkvars );

               memcpy( (char*)spInternalFd->mpExternalMCFile->mData + spInternalFd->mpExternalMCFile->mOpenFileLoc, &bp_linkvars, sizeof( bp_linkvars ) );
               spInternalFd->mpExternalMCFile->mOpenFileLoc += MAX_BP_LINKVARBUF;
            }
         }
      }

      if( ( spInternalFd->mWhichFile == kIMCF_Data ) && ( spInternalFd->mpExternalMCFile->mOpenFileLoc == spInternalFd->mpExternalMCFile->mSize ) 
#if MGS_VERSION==3
         //Hack to allow variable size MGS3 photo saves
         || sWhichInternalSaveData == kISD_Photo
#endif
         )
      {
#if MGS_VERSION==3
         if( sWhichInternalSaveData == kISD_Photo )
         {
            gBP_MCFiles[kEMCF_Data].mSize = size;
         }
#endif

         //Finished writing to the data file (the only one we write externally).  Actually perform the (async) savedata save at this step.
         if ( gMGS_SaveStatus.mRecreatingDeletedSaveData )
         {
            save_data_auto_recreate_save( &gMGS_SaveStatus, bp_internal_wrote_savedata_mc_callback );
         }
         else
         {
            save_data_auto_save( &gMGS_SaveStatus, bp_internal_wrote_savedata_mc_callback );
         }

         //Reset flag for the next time, this sequence is done.
         gMGS_SaveStatus.mRecreatingDeletedSaveData = 0;

         return 1;
      }
      else
      {
         //Not time to perform the asynchronous step yet.
      }
   }
   else
   {
      //This is an ignored file for us.
   }

#if MGS_VERSION==3
   //autoclose the file.
   MGS_SaveStatus_Close();
#endif
   return 0;
}

unsigned int MGS_SaveStatus_Read( void *buff, unsigned int size )
{
   if( !spInternalFd || ( sInternalOpenMode != kOpenMode_Read ) )
      BP_BREAK;

   if( spInternalFd->mpExternalMCFile )
   {
      if( spInternalFd->mpExternalMCFile->mOpenFileLoc + size > spInternalFd->mpExternalMCFile->mSize )
      {
         //internal error - buffer isn't big enough for expected file size.
         BP_BREAK;
      }

      gMGS_SaveStatus.mpReadWriteBuf = buff;
      gMGS_SaveStatus.mReadWriteSize = size;

      //MGS2 shouldn't ever try to read from anything but the D* file.
      BPE_ASSERT( spInternalFd->mWhichFile == kIMCF_Data, "Unexpected file opened for reading!" );
      if( spInternalFd->mpExternalMCFile->mOpenFileLoc == 0 )
      {
         //Performing the first read on the only file read.  Start the asynchronous operation with a callback
         //that will perform the copy to the output buffer when it is done.
         MGS_SaveStatus_SetChosenGameSave( kEMCF_Data );
         gMGS_SaveStatus.mWrongUserDetectDisabled = 0;   //detect wrong user, we're loading data to use!
         save_data_auto_load( &gMGS_SaveStatus, bp_internal_read_savedata_mc_callback );

         return 1;
      }
      else
      {
         //The asynchronous step for the entire file has been completed and we can just copy the data out.
         bp_internal_do_read_savedata( &gMGS_SaveStatus );
      }
   }
   else
   {
      //This is an ignored file for us.
      BP_TODO_BREAK; //should copy the original const data out?
   }

#if MGS_VERSION==3
   //autoclose the file.
   MGS_SaveStatus_Close();
#endif
   return 0;
}

unsigned int MGS_SaveStatus_Read2( const char * const name, void *buff, unsigned int size )
{
   MGS_SaveStatus_Open( name, SCE_RDONLY ); //read mode

   //The read function will autoclose the file, either immediately if it's synchronous or via the callback if asynchronous.
   unsigned int bp_async_flag = MGS_SaveStatus_Read( buff, size );

   return bp_async_flag;
}

unsigned int MGS_SaveStatus_Write2( const char * const name, const void *buff, unsigned int size )
{
   MGS_SaveStatus_Open( name, SCE_WRONLY ); //write mode

   //The write function will autoclose the file, either immediately if it's synchronous or via the callback if asynchronous.
   unsigned int bp_async_flag = MGS_SaveStatus_Write( buff, size );

   return bp_async_flag;
}

unsigned int MGS_SaveStatus_Delete( const char * const fullpath )
{
   const char * const name = bp_internal_skip_fullpath_mc( fullpath );
   BPE_ASSERT( ( name[0] == BP_DATA_FILENAME_FIRSTCHAR ) && ( strlen(name) == BP_DATA_FILENAME_LENGTH ), "unexpected delete pattern" );

   //Delete the whole savegame record.

#ifdef BP_VITA
   if (CTransfarringManager::Instance()->NeedsRealDelete())
   {
#if MGS_VERSION == 3
      char dirsuffix[FILENAME_MAX];
      bp_extract_subdir_suffix(dirsuffix, fullpath);
      char* suffixMinusName = strstr(dirsuffix, name);
      *suffixMinusName = 0;
      gMGS_SaveStatus.SetChosenGameSaveDirSuffix(dirsuffix);
#endif
      save_data_auto_delete( &gMGS_SaveStatus );

      return 1;
   }
   else
   {
      printf("MGS_SaveStatus_Delete: next save will be a recreate_save.\n");
      gMGS_SaveStatus.mRecreatingDeletedSaveData = 1;
      return 0;
   }
#else
   //BP: can't use CellSaveData*Delete API for this functionality because it's impossible to do an "autodelete"
   //that suppresses the PS3 system UI.
   //MGS never deletes a savedata record except to recreate it, so instead we'll set a flag here that the next save operation
   //should instead be s "recreate" operation.
   printf("MGS_SaveStatus_Delete: next save will be a recreate_save.\n");
   gMGS_SaveStatus.mRecreatingDeletedSaveData = 1;
   return 0;   //no async operation!
#endif
}

unsigned int MGS_SaveStatus_GetInfo()
{
   save_data_get_info( &gMGS_SaveStatus );
   return 1;
}

int MGS_SaveStatus_GetInfoResult()
{
   if( gMGS_SaveStatus.mDeviceId != 0 )
   {
      return 0;   //SCE_OK
   }
   else
   {
      return SCE_ENODEV;
   }
}

unsigned int MGS_SaveStatus_Rename( const char * const org, const char * const dst )
{
   //This is used on the data (D*) file of MGS photo savedatas when editing them in photo album mode.
   //We need to update the contents of MASTER.BIN and start an auto_recreate_save operation.
   //We *should* only be writing out the MASTER.BIN file, but there's no interface to do that so
   //to hell with it, it doesn't really hurt to write out the icon and data files as well.
   BPE_ASSERT( strlen(dst) == BP_DATA_FILENAME_LENGTH, "unexpected D* filename!" );
   SMCFile * pHeaderFile = &gBP_MCFiles[kEMCF_Info];
   strncpy( (char*)pHeaderFile->mData, dst, BP_DATA_FILENAME_LENGTH );

   gMGS_SaveStatus.ResetStatus();
   save_data_auto_recreate_save( &gMGS_SaveStatus, NULL );

   return 1;
}

void MGS_SaveStatus_Seek( const int offset, const int mode )
{
   //This is used on icon.sys when saving a photo in MGS2 photo album mode.
   //Basically has no consequences for us since we ignore r/w access to that file anyway.
   if( !spInternalFd )
      BP_BREAK;

   if( spInternalFd->mpExternalMCFile )
   {
      int newOffset = spInternalFd->mpExternalMCFile->mOpenFileLoc;
      switch( mode )
      {
      case SEEK_SET:
         newOffset = offset;
         break;
      case SEEK_CUR:
         newOffset += offset;
         break;
      case SEEK_END:
         newOffset = spInternalFd->mpExternalMCFile->mSize + offset;
         break;
      default:
         BP_BREAK;
      }

      newOffset = bpe::max_val( newOffset, 0 );
      newOffset = bpe::min_val( newOffset, spInternalFd->mpExternalMCFile->mSize );
      spInternalFd->mpExternalMCFile->mOpenFileLoc = (unsigned int)newOffset;
   }
}

#if MGS_VERSION==3
ESaveGameTitle get_save_game_title()
{
   return gBP_Save_Game_Title;
}
#endif

//----------------------------------------------------------------------------
