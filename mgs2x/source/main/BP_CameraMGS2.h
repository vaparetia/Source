#pragma once

//----------------------------------------------------------------------------
// BP_CameraMGS2.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// TYPES
//----------------------------------------------------------------------------

typedef struct _GM_CameraSet  MGS_Camera;

//extern float P_GRAVITY;


//----------------------------------------------------------------------------
// C FUNCTIONS
//----------------------------------------------------------------------------

void GV_NearExp4BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 );
void GV_NearExp8BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 );
void GV_NearExp16BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 );
void GV_NearTimeBP( SBP_CameraSettings* set1, SBP_CameraSettings* set2, int time );

//----------------------------------------------------------------------------
// C++ FUNCTIONS
//----------------------------------------------------------------------------

#ifdef __cplusplus

const SBP_CameraDef*    BP_Camera_GetDefs();
const int               BP_Camera_GetDefsCount();

const SBP_CinemaDef*    BP_Cinema_GetDefs();
const int               BP_Cinema_GetDefsCount();

const SBP_CutsceneDef*  BP_Cutscene_GetDefs();
const int               BP_Cutscene_GetDefsCount();

#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
