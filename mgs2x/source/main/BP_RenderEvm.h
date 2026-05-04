//----------------------------------------------------------------------------
// BP_RenderEvm.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

void BP_InitEvmShader();
void BP_Evm_FrameBegin();
void BP_Evm_FrameEnd();
void BP_Evm_InitPacket(char* pData);
void BP_Evm_LocalParam(char* pData);
void BP_Evm_Render(char* pData);
void BP_Evm_RenderMulti(char* pData);

void BP_Evm_InitDebugMenu();

//----------------------------------------------------------------------------
