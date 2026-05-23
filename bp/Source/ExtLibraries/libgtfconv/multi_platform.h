/*  SCE CONFIDENTIAL
 *  PLAYSTATION(R)3 Programmer Tool Runtime Library 240.001
 *  Copyright (C) 2008 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#ifndef __GTFCONV_MULTI_PLATFORM_H__
#define __GTFCONV_MULTI_PLATFORM_H__

#ifndef __ENABLE_MULTI_PLATFORM__
#define __ENABLE_MULTI_PLATFORM__

/* multi platform flag */
#ifdef _WIN32
#define __WIN32__
#else
#ifdef __CELLOS_LV2__
#define __CELL_PPU__
#else
#define __LINUX_GCC__
#endif
#endif


/* for windows only */
#ifdef __WIN32__

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

typedef _int8 int8_t;
typedef _int16 int16_t; 
typedef _int32 int32_t;
typedef _int64 int64_t; 
typedef unsigned _int8 uint8_t;
typedef unsigned _int16 uint16_t; 
typedef unsigned _int32 uint32_t;
typedef unsigned _int64 uint64_t;

#endif /* __WIN32__ */

/* for linux only */
#ifdef __LINUX_GCC__

#include <stdint.h>

typedef long LONG;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef uint32_t BOOL;
typedef uint32_t UINT;

#endif /* __LINUX_GCC__ */

/* for cell only */
#ifdef __CELL_PPU__

typedef long LONG;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef uint32_t BOOL;
typedef uint32_t UINT;

#endif /* __CELL_PPU__ */

/* VS2003 */
#ifdef __WIN32__
#if _MSC_VER < 1400
#define vsnprintf _vsnprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif
#endif

#endif /* __ENABLE_MULTI_PLATFORM__ */

#endif /* __GTFCONV_MULTI_PLATFORM_H__ */
