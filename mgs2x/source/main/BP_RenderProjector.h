//----------------------------------------------------------------------------
// BP_RenderProjector.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------------
void BP_InitProjectorShader();

//Shadow stage
void BP_Projector_ShadowStage_StartPacket();

void BP_Projector_ShadowStage_InitPacket(char* pData);
void BP_Projector_ShadowStage_PostZScreenPacket();
void BP_Projector_ShadowStage_ShadowProjStart(char* pData);

void BP_Projector_ShadowStage_EndPacket();
//Spot stage
void BP_Projector_SpotStage_StartPacket();

void BP_Projector_SpotStage_InitPacket(char* pData);
void BP_Projector_SpotStage_PostZScreenPacket(char* pData);
void BP_Projector_SpotStage_ShadowProjStart(char* pData);

void BP_Projector_SpotStage_EndPacket();

#if BP_VITA
void BP_ProjectorNew_StartPacket(char* pData);
void BP_ProjectorNew_InitPacket(char* pData);
void BP_ProjectorNew_SpotProjStart(char* pData);
void BP_ProjectorNew_ShadowProjStart(char* pData);
void BP_ProjectorNew_EndPacket();
#endif

//Draw functions
void BP_Projector_LocalParam(char* pData);
void BP_Projector_Render(char* pData);
//
void BP_Projector_InitDebugMenu();

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
