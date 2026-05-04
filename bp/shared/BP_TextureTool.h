//----------------------------------------------------------------------------
// BP_TextureTool.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

#if BP_ENABLE_TEXTURE_TOOL

void           BP_TextureTool_Init();
void           BP_TextureTool_SetEnable( bool enabled );
bool           BP_TextureTool_GetEnable();
int            BP_TextureTool_RemapPath( char* path );
void           BP_TextureTool_BeginFrame();
void           BP_TextureTool_EndFrame();
CBaseTexture*  BP_TextureTool_GetBPTexture(DG_TEX* pTex);
void           BP_TextureTool_StorePathRemapping( const char* originalPath,
                                                  const char* flatPath );
void           BP_TextureTool_ReloadTextures(bool modifiedOnly);
void           BP_TextureTool_TextureDestroyed(DG_TEX* pTex);
#endif

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------

