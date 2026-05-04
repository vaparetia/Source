//----------------------------------------------------------------------------
// BP_BaseRenderer.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
// Additionally you should not include any other headers at all here, this file must stand on it's own.
// Any structs used by these functions should be defined here.
//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

// BP: If you change this here you must also change it to the same value (0 or 1) in:
// BP\Source\ExtLibraries\Edge\target\spu\include\edge\post\edgepost_mlaa_memory_layout.h   
// BP\Source\ExtLibraries\Edge\target\spu\src\edge\post\mlaa\edgepost_mlaa_find_separation_lines.spa
#define EDGE_MLAA_SUPPORT_1080  0 

//----------------------------------------------------------------------------
// Initialization, Settings
//----------------------------------------------------------------------------

// Initialize BP systems, should be first thing called in the app.
void BP_InitializeSystems();

// Shutdown BP systems, should be last things called in app
void BP_ShutdownSystems();

//----------------------------------------------------------------------------
// Resource functions
//----------------------------------------------------------------------------

void BP_VerifyEmptyStageAssetCache( const unsigned int cacheTag );
void BP_ForceClearStageAssetCache();
void BP_ClearStageAssetCacheByTag( const unsigned int cacheTag );
void BP_AddReferenceToLastStageAssetCacheEntry( const unsigned int cacheTag, const char * const stagePath );
void BP_AddResourceToStageAssetCache( const unsigned int cacheTag, const char * const stagePath, const char * const name, void * const memory, const int size );

void * BP_ConstructMesh(const char* name, void* memory, int size);
void BP_BindMGSMesh(const unsigned int assetCacheTag, const char* pCurrentPath, unsigned int resourceId, unsigned int mgsAddr);
unsigned int BP_GetMeshByMGSAddr(unsigned int mgsAddr);
void BP_DestroyMeshes(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd);
int BP_GetLoadedMeshCount();

void * BP_ConstructTexture(const char* name, void* memory, int size);
void BP_BindMGSTexture(const unsigned int assetCacheTag, const char * pCurrentPath, unsigned int triId, unsigned int texId, unsigned int mgsAddr);
void BP_BindMGSImage(const unsigned int assetCacheTag, const char * pCurrentPath, unsigned int imageId, unsigned int mgsAddr);
void BP_TextureSetResidentTexture(unsigned int texture);

#if BP_ENABLE_TEXTURE_TOOL
void BP_ResetTextureByMGSAddr(unsigned int mgsAddr, const char* path);
#endif

unsigned int BP_GetTextureByMGSAddr(unsigned int mgsAddr);
const char* BP_GetTextureNameByMGSAddr(unsigned int mgsAddr);

void BP_DestroyTextures(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd);
void BP_FlushDestroyedTextures();
int BP_GetLoadedTextureCount();

void BP_DestroyResources(unsigned int mgsAddrBegin, unsigned int mgsAddrEnd);
void BP_FlushDestroyedResources();

//----------------------------------------------------------------------------
// Render functions
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
