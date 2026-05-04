//----------------------------------------------------------------------------
// BP_BuildDefines.h
//----------------------------------------------------------------------------

#pragma once

#include "MGS_SysCommon.h"

//----------------------------------------------------------------------------
// Performance
//----------------------------------------------------------------------------

#if !defined(GOLD_VERSION) && (defined(BP_PS3) || defined(BP_360))
#define BP_ENABLE_ONSCREEN_PROFILER 1
#endif

//----------------------------------------------------------------------------
// Debug
//----------------------------------------------------------------------------

#ifndef GOLD_VERSION
#if BP_VITA
#define BP_ENABLE_PROFILE_MARKERS         1
#else
#define BP_ENABLE_PROFILE_MARKERS         0
#define BP_ENABLE_DEBUG_PRIM              1
#endif
#define BP_ENABLE_RENDER_LABELS           1
#define BP_ENABLE_DEBUG_MENU              1
#endif

#if BPE_TARGET == BPE_TARGET_X360
//#define BP_ENABLE_MEMORY_TRACKING   1
#endif

#define BP_ENABLE_DEBUG_CAMERA() BP_ENABLE_DEBUG_MENU
#define BP_ENABLE_DEBUG_HELPER() BP_ENABLE_DEBUG_MENU

//----------------------------------------------------------------------------
// Texture tool
//----------------------------------------------------------------------------
#ifndef GOLD_VERSION
#if defined( BP_WIN32 ) || defined( BP_PS3 )
#define BP_ENABLE_TEXTURE_TOOL      1
#endif
#else
#define BP_ENABLE_TEXTURE_TOOL      0
#endif

//----------------------------------------------------------------------------
// Collision
//----------------------------------------------------------------------------

// NOTE: Enabling "DRAW" and "STATS_DETAILED" adds a few ms cpu overhead!
#if BP_ENABLE_DEBUG_MENU
//#define BP_ENABLE_DEBUG_COLL_DRAW            1
#define BP_ENABLE_DEBUG_COLL_STATS_OVERVIEW  1
//#define BP_ENABLE_DEBUG_COLL_STATS_DETAILED  1
#endif

#define BP_ENABLE_FAST_GEOM   1

//----------------------------------------------------------------------------
// Render
//----------------------------------------------------------------------------

#ifdef BP_PS3
#define BP_ENABLE_MLAA                       0
#endif

#define BP_ENABLE_DEBUG_PRIM_GPU_MARKER      0
#define BP_ENABLE_DEBUG_MESH_GPU_MARKER      0
#define BP_ENABLE_DEBUG_TEXTURE_GPU_MARKER   0

#define BP_USE_NEW_FONT_SYSTEM()             1

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererDefines.h"
