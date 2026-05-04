//----------------------------------------------------------------------------
// BP_RenderObj.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

void BP_InitObjOptCmfShader();
void BP_Obj_OptCmf_InitPacket(char* pData);
void BP_Obj_OptCmf_PassParam(char* pData);
void BP_Obj_OptCmf_LocalParam(char* pData);
void BP_Obj_OptCmf_Render(char* pData);

//----------------------------------------------------------------------------
