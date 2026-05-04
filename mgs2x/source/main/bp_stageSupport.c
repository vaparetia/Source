//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#include "MGS_SysCommon.h"

//----------------------------------------------------------------------------

#include "bp_stageFuncs.c"

extern void BP_ClearLocalResourceInfo();

//----------------------------------------------------------------------------

typedef struct
{
   char *   name;
   CHARA*   funcs;
} StageFuncTableMapping;

static StageFuncTableMapping skStageFuncs[] =
{
   #include "bp_stageTable.c"
   { NULL, NULL }
};

//----------------------------------------------------------------------------

extern char *GM_GetArea();

static CHARA skNullFunc = { 0, NULL };

CHARA * BP_GetCurrentStageFunctions()
{
   const char* pCurrentArea = GM_GetArea();

   StageFuncTableMapping* pMapping = skStageFuncs;

   for( ; pMapping->name; ++pMapping )
   {
      if( strcmp(pMapping->name, pCurrentArea) == 0 )
      {
         return pMapping->funcs;
      }
   }

   return &skNullFunc;
}

//----------------------------------------------------------------------------

typedef void (*BSSINITFN)();

#if MGS_VERSION == 3
extern void BP_CallBulletScar_Bss_Init();
extern void BP_CallBulletScar2_Bss_Init();
extern void BP_CallLayoutAnime3D_Bss_Init();
extern void BP_IncontinenceEffect_Bss_Init();
extern void BP_PictureProcessingProg_Bss_Init();
extern void BP_SpiritPhotoMng_Bss_Init();
extern void BP_SpiritPhotoMngFunc_Bss_Init();
extern void BP_SubjectViewTearsEft_Bss_Init();
extern void BP_UnderwaterEft_Bss_Init();
extern void BP_WaterStainEft_Bss_Init();
extern void BP_SetSubView_Bss_Init();
extern void BP_BottleWaterTarget_Bss_Init();
extern void BP_PbeeFar_Bss_Init();
extern void BP_Marsh_Bss_Init();
extern void BP_s151aFall_Bss_Init();
extern void BP_Firethrower_Bss_Init();
extern void BP_PbBind_Bss_Init();
extern void BP_PbCvd_Bss_Init();
extern void BP_PbCvdMdb_Bss_Init();
extern void BP_PbCvdSorrow_Bss_Init();
extern void BP_PbFar_Bss_Init();
extern void BP_PbMiddle_Bss_Init();
extern void BP_CallAct_Bss_Init();
extern void BP_KtuInitialize_Bss_Init();
extern void BP_EneFirethrower_Bss_Init();
extern void BP_Util_Bss_Init();
extern void BP_BeePheromone_Bss_Init();
#else
extern void BP_mpegstr_BSS_Init();
extern void BP_ipustream_BSS_Init();
extern void BP_shipworm_BSS_Init();
#endif

static const BSSINITFN skBssInitFunctions[] =
{
#if MGS_VERSION == 3
   &BP_CallBulletScar_Bss_Init,
   &BP_CallBulletScar2_Bss_Init,
   &BP_CallLayoutAnime3D_Bss_Init,
   &BP_IncontinenceEffect_Bss_Init,
   &BP_PictureProcessingProg_Bss_Init,
   &BP_SpiritPhotoMng_Bss_Init,
   &BP_SpiritPhotoMngFunc_Bss_Init,
   &BP_SubjectViewTearsEft_Bss_Init,
   &BP_UnderwaterEft_Bss_Init,
   &BP_WaterStainEft_Bss_Init,
   &BP_SetSubView_Bss_Init,
   &BP_BottleWaterTarget_Bss_Init,
   &BP_PbeeFar_Bss_Init,
   &BP_Marsh_Bss_Init,
   &BP_s151aFall_Bss_Init,
   &BP_Firethrower_Bss_Init,
   &BP_PbBind_Bss_Init,
   &BP_PbCvd_Bss_Init,
   &BP_PbCvdMdb_Bss_Init,
   &BP_PbCvdSorrow_Bss_Init,
   &BP_PbFar_Bss_Init,
   &BP_PbMiddle_Bss_Init,
   &BP_CallAct_Bss_Init,
   &BP_KtuInitialize_Bss_Init,
   &BP_EneFirethrower_Bss_Init,
   &BP_Util_Bss_Init,
   &BP_BeePheromone_Bss_Init,
#else
   &BP_mpegstr_BSS_Init,
   &BP_ipustream_BSS_Init,
   &BP_shipworm_BSS_Init,
#endif

   NULL
};

//----------------------------------------------------------------------------

void BP_InitStageBSS()
{
   int current = 0;
   while( skBssInitFunctions[current] )
   {
      BSSINITFN function = skBssInitFunctions[current];
      function();

      ++current;
   }

   BP_ClearLocalResourceInfo();
}