//------------------------------------------------------------------------------------------
// BPERendererAPI.h
// Copyright 2004
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#define BPE_RENDERER_TYPE_PROGSHADER 0
#define BPE_RENDERER_TYPE_PLATFORMSPECIFIC 1

#if ( ( BPE_TARGET==BPE_TARGET_PS3 ) || ( BPE_TARGET == BPE_TARGET_WIN32 ) || ( BPE_TARGET == BPE_TARGET_X360 ) || ( BPE_TARGET==BPE_TARGET_VITA ) || ( BPE_TARGET==BPE_TARGET_DREAMCAST ) )
#  define BPE_RENDERER_PATH(header) BPE_STRINGIZE( ProgShader/PS ##header )
#  define BPE_RENDERER_PATH2(path,header) BPE_STRINGIZE( path/ProgShader/PS ##header )
#  define BPE_RENDERER_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/ProgShader/path2/PS ##header )
#  define BPE_RENDERER_TYPE BPE_RENDERER_TYPE_PROGSHADER
#elif ( BPE_TARGET==BPE_TARGET_RVL )
#  define BPE_RENDERER_PATH(header) BPE_PLATFORM_PATH(header)
#  define BPE_RENDERER_PATH2(path,header) BPE_PLATFORM_PATH2(path,header)
#  define BPE_RENDERER_COMPLEX_PATH(path1,path2,header) BPE_PLATFORM_COMPLEX_PATH(path1,path2,header)
#  define BPE_RENDERER_TYPE BPE_RENDERER_TYPE_PLATFORMSPECIFIC
#else
#  error Unknown platform.
#endif

#if BPE_TARGET == BPE_TARGET_WIN32

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif
#else

#ifdef RENDERER_EXPORTS
#define RENDERER_API
#else
#define RENDERER_API
#endif

#endif
