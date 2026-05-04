//----------------------------------------------------------------------------
// BP_RenderPatch.h
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

void AS_Patch_InitDebugMenu();
void BP_InitPatchShader();
void BP_PatchInitFrame(char* pData);
void BP_PatchInit(char* pData);
void BP_PatchInitRender(char* pData);
void BP_PatchRender(char* pData);
void BP_PatchFinalizeFrame(char* pData);

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
