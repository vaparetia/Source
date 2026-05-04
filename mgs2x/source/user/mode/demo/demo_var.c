//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demovar.c

*/

#include "BP_Camera.h"//BP_CAMERA


int DM_FrameSkip = 0 ; /* フレームスキップ スキップしたフレーム数が入る */
int DM_CameraFlag = 0;	/* デモのカメラ起動時のGM_SetCameraTypeに */
						/* 属性を追加するかを決定するフラグ。     */


//BP_CAMERA - adding extra demo vars for tracking camera cuts
int   gBP_Demo_Active = 0;
int   gBP_Demo_Id = 0;
int   gBP_Demo_Frame = -1;
char  gBP_Demo_StreamName[256] = {0};
//BP_CAMERA - adding extra demo vars for tracking camera cuts
