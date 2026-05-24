
//------------------------------------------------------------------------------------------
// BPEEnvironment.h
//------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

//----------------------------------------------------------------------------

// Disable specific warnings for MSDEV

#if defined(WIN32) || defined(_XBOX_VER)

// warning C4786: 'Some STL template class' : identifier was truncated to '255' characters in the debug information.
// Looks like this has been depreciated in VC8.
#pragma warning ( disable : 4786 )

// disable: '<' : signed/unsigned mismatch
#pragma warning ( disable : 4018 )

// disable: conversion from 'x' to 'y', possible loss of data
#pragma warning ( disable : 4267 )

// disable: 'this' : used in base member initializer list
#pragma warning ( disable : 4355 )

// disable : nonstandard extension used : 'argument' : conversion from 'XXX' to 'XXX &' (using temporaries as references)
#pragma warning ( disable : 4239 )

// warning C4251: 'CLASS' : class 'MEMBER VARIABLE CLASS TYPE' needs to have dll-interface to be used by clients of class 'CLASS'
#pragma warning ( disable : 4251 )

// warning C4251: non � DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
// Disabled due to boost signals.
#pragma warning ( disable : 4275 )

// warning C4100: 'argument' : unreferenced formal parameter
#pragma warning ( disable : 4100 )

// warning C4512: 'class' : assignment operator could not be generated
// Disabled due to boost/signal.hpp
#pragma warning ( disable : 4512 )

// warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// We're not using exceptions!
#pragma warning ( disable : 4530 )

// warning C4201: nonstandard extension used : nameless struct/union
// Harmless warning, ANSI compliance only.
#pragma warning ( disable : 4201 )

// warning C4505: 'function' : unreferenced local function has been removed
// Harmless warning, occurs when platform specific functions aren't used for Win32
#pragma warning ( disable : 4505 )

// warning C4561: '__fastcall' incompatible with the '/clr' option: converting to '__stdcall'
#ifdef _MANAGED
   #pragma warning ( disable : 4561 )
#endif

#endif

//------------------------------------------------------------------------------------------
// general defines

#define BPE_STRINGIZE( arg ) #arg

#define _BPE_QUOTE(x) # x
#define BPE_QUOTE(x) _BPE_QUOTE(x)
#define BPE_FILELINE __FILE__ "(" BPE_QUOTE(__LINE__) ") : "
#define BPE_AUTOLINKSYSLIB(libName) comment( lib, BPE_STRINGIZE( libName##.lib ) )

#define BPE_TODOMSG( arg )  message( BPE_FILELINE " TODO: "   arg )
#define BPE_WARNMSG( arg )  message( BPE_FILELINE " WARNING: " arg )
#define BPE_ERRORMSG( arg ) message( BPE_FILELINE " ERROR: "   arg )

//------------------------------------------------------------------------------------------
// Build targets

#define BPE_TARGET_WIN32   0
#define BPE_TARGET_PS3     1
#define BPE_TARGET_RVL     2
#define BPE_TARGET_X360    3
#define BPE_TARGET_VITA       4
#define BPE_TARGET_DREAMCAST  5

#ifdef WIN32
   #define BPE_TARGET BPE_TARGET_WIN32

   #define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( Win32 ##header )
   #define BPE_PLATFORM_SPECIFIC2( path, header) BPE_STRINGIZE( ##path/Win32 ##header )

   #define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( Win32/Win32 ##header )
   #define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/Win32/Win32 ##header )
   #define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/Win32/path2/Win32 ##header )

   // Win32 uses dynamic linking so we don't need to force a reference
   #define BPE_FORCE_REFERENCE   static
   #define BPE_NOINLINE          __declspec(noinline)
   #define BPE_FORCEINLINE       __forceinline

   // Branch prediction
   // Tell compiler that this branch is likely to be taken.
   #define BPE_BRANCH_HINT_TRUE(a)  a
   // Tell compiler that this branch is unlikely to be taken.
   #define BPE_BRANCH_HINT_FALSE(a) a
   // Dynamic branch hinting based on b condition.
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) a

   // Cache prefetch
   // Prefetch disabled on Win32
   #define BPE_DCACHE_PREFETCH(a)

   // Mismatched platform string compares
   #define strcasecmp            _stricmp
   #define strncasecmp           _strnicmp

   #define STD_HASH_MAP          stdext            // Used for MS hash_map

   #define BPE_PATH_SEPARATOR "/"

   #define BPE_PIXELCENTER_MATCH_TEXELCENTER 0

#elif defined(_PS3)
   #define BPE_TARGET BPE_TARGET_PS3

   #define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( PS3 ##header )
   #define BPE_PLATFORM_SPECIFIC2( path, header ) BPE_STRINGIZE( path/PS3 ##header )

   #define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( PS3/PS3 ##header )
   #define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/PS3/PS3 ##header )
   #define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/PS3/path2/PS3 ##header )

   // Currently just building under win32 so we'll just leave it global
   #define BPE_FORCE_REFERENCE   __attribute__((used))
   #define BPE_NOINLINE          __attribute__((noinline))

#ifndef __cplusplus//soren added to get inline functions to work on PS3
   #define BPE_FORCEINLINE       static inline __attribute__((always_inline))
#else
   #define BPE_FORCEINLINE       inline __attribute__((always_inline))
#endif

// Branch prediction
   // Tell compiler that this branch is likely to be taken.
   #define BPE_BRANCH_HINT_TRUE(a)  __builtin_expect(a, 1)
   // Tell compiler that this branch is unlikely to be taken.
   #define BPE_BRANCH_HINT_FALSE(a) __builtin_expect(a, 0)
   // Dynamic branch hinting based on b condition.
#ifdef __SPU__   
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) __builtin_expect(a, b)
#else
   // PPU doesn't support dynamic branch hinting.
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) a
#endif

   // Cache prefetch
#ifdef __SPU__   
   // No cache prefetch on SPU (no cache!)
   #define BPE_DCACHE_PREFETCH(a)   
#else
   // PPU
   #define BPE_DCACHE_PREFETCH(a)   __builtin_prefetch(a)
#endif

   #define STD_HASH_MAP          std                     // Used for MS hash_map

   #define BPE_PATH_SEPARATOR "/"

   #define BPE_PIXELCENTER_MATCH_TEXELCENTER 0
#elif defined( SN_TARGET_PSP2 )
#  define BPE_TARGET BPE_TARGET_VITA
#  define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( VTA ##header )
#  define BPE_PLATFORM_SPECIFIC2( path, header ) BPE_STRINGIZE( path/VTA ##header )

#  define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( VTA/VTA ##header )
#  define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/VTA/VTA ##header )
#  define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/VTA/path2/VTA ##header )

#  define BPE_FORCE_REFERENCE   __attribute__((used))
#  define BPE_NOINLINE          __attribute__((noinline))

#  ifndef __cplusplus//soren added to get inline functions to work on PS3
#     define BPE_FORCEINLINE       static inline __attribute__((always_inline))
#  else
#     define BPE_FORCEINLINE       inline __attribute__((always_inline))
#  endif
// Branch prediction
// Tell compiler that this branch is likely to be taken.
#  define BPE_BRANCH_HINT_TRUE(a)  a
// Tell compiler that this branch is unlikely to be taken.
#  define BPE_BRANCH_HINT_FALSE(a) a
// Dynamic branch hinting based on b condition.
#  define BPE_BRANCH_HINT_DYNAMIC(a, b) a

// Cache prefetch
// Prefetch disabled on RVL
#  define BPE_DCACHE_PREFETCH(a) __builtin_pld( a )

#  define STD_HASH_MAP          std                     // Used for MS hash_map
#  define BPE_PATH_SEPARATOR "/"
#  define BPE_PIXELCENTER_MATCH_TEXELCENTER 0

#elif defined(RVL)
   #define BPE_TARGET BPE_TARGET_RVL
   #define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( RVL ##header )
   #define BPE_PLATFORM_SPECIFIC2( path, header ) BPE_STRINGIZE( path/RVL ##header )

   #define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( RVL/RVL ##header )
   #define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/RVL/RVL ##header )
   #define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/RVL/path2/RVL ##header )

   #define BPE_FORCE_REFERENCE   __declspec(force_export)
   #define BPE_NOINLINE          
   #define BPE_FORCEINLINE       inline

   // Branch prediction
   // Tell compiler that this branch is likely to be taken.
   #define BPE_BRANCH_HINT_TRUE(a)  a
   // Tell compiler that this branch is unlikely to be taken.
   #define BPE_BRANCH_HINT_FALSE(a) a
   // Dynamic branch hinting based on b condition.
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) a

   // Cache prefetch
   // Prefetch disabled on RVL
   #define BPE_DCACHE_PREFETCH(a)

   #define STD_HASH_MAP          Metrowerks

   #define BPE_PATH_SEPARATOR "/"

   #define BPE_PIXELCENTER_MATCH_TEXELCENTER 0

#elif defined(_XBOX_VER)
   #define BPE_TARGET BPE_TARGET_X360
   #define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( X360 ##header )
   #define BPE_PLATFORM_SPECIFIC2( path, header ) BPE_STRINGIZE( path/X360 ##header )

   #define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( X360/X360 ##header )
   #define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/X360/X360 ##header )
   #define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/X360/path2/X360 ##header )

   #define BPE_FORCE_REFERENCE
   #define BPE_NOINLINE          __declspec(noinline)
   #define BPE_FORCEINLINE       __forceinline

   // Alignment
   #define BPE_ALIGNED(alignment)   __declspec(align(alignment))
   #define BPE_ALIGNED_PS3(alignment)

   // Branch prediction
   // Tell compiler that this branch is likely to be taken.
   #define BPE_BRANCH_HINT_TRUE(a)  a
   // Tell compiler that this branch is unlikely to be taken.
   #define BPE_BRANCH_HINT_FALSE(a) a
   // Dynamic branch hinting based on b condition.
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) a

   // Cache prefetch
   #define BPE_DCACHE_PREFETCH(a) __dcbt( (int)(a), 0 )

   // Mismatched platform string compares
   #define strcasecmp            _stricmp
   #define strncasecmp           _strnicmp

   #define STD_HASH_MAP          std

   #define BPE_PATH_SEPARATOR "\\"

   #define BPE_PIXELCENTER_MATCH_TEXELCENTER 0 //be sure to set D3DRS_HALFPIXELOFFSET to true

#elif defined(__DREAMCAST__)
   #define BPE_TARGET BPE_TARGET_DREAMCAST
   #define BPE_PLATFORM_SPECIFIC( header ) BPE_STRINGIZE( DC ##header )
   #define BPE_PLATFORM_SPECIFIC2( path, header ) BPE_STRINGIZE( path/DC ##header )

   #define BPE_PLATFORM_PATH(header) BPE_STRINGIZE( DC/DC ##header )
   #define BPE_PLATFORM_PATH2(path,header) BPE_STRINGIZE( path/DC/DC ##header )
   #define BPE_PLATFORM_COMPLEX_PATH(path1,path2,header) BPE_STRINGIZE( path1/DC/path2/DC ##header )

   #define BPE_FORCE_REFERENCE   __attribute__((used))
   #define BPE_NOINLINE          __attribute__((noinline))
   #define BPE_FORCEINLINE       inline __attribute__((always_inline))

   #define BPE_BRANCH_HINT_TRUE(a)  __builtin_expect(a, 1)
   #define BPE_BRANCH_HINT_FALSE(a) __builtin_expect(a, 0)
   #define BPE_BRANCH_HINT_DYNAMIC(a, b) a

   #define BPE_DCACHE_PREFETCH(a)   __builtin_prefetch(a)

   #define STD_HASH_MAP          std
   #define BPE_PATH_SEPARATOR "/"
   #define BPE_PIXELCENTER_MATCH_TEXELCENTER 0

#else
   #error Unknown compile target!
#endif

//------------------------------------------------------------------------------------------
// Endian-ness

#define BPE_ENDIAN_LITTLE 0
#define BPE_ENDIAN_BIG    1

#if BPE_TARGET==BPE_TARGET_WIN32 || BPE_TARGET==BPE_TARGET_VITA || BPE_TARGET==BPE_TARGET_DREAMCAST
#define BPE_ENDIAN BPE_ENDIAN_LITTLE
#elif ( BPE_TARGET == BPE_TARGET_PS3 || BPE_TARGET==BPE_TARGET_RVL || BPE_TARGET==BPE_TARGET_X360 )
#define BPE_ENDIAN BPE_ENDIAN_BIG
#else
#error Unknown target for endian macros!
#endif

//-------------------------------------------------------------------------------------------------------
// release build specific defines

#ifdef NDEBUG
   #define BPE_RELEASE
   #define BPE_BUILDTYPE "Release"
   #define BPE_AUTOLINKLIB(libName) comment( lib, BPE_STRINGIZE( libName##.lib ) )

//-------------------------------------------------------------------------------------------------------
// debug build specific defines

#else
   #define BPE_DEBUG
   #define BPE_BUILDTYPE "Debug"
   #define BPE_AUTOLINKLIB(libName) comment( lib, BPE_STRINGIZE( libName##_Debug.lib ) )
#endif

//-------------------------------------------------------------------------------------------------------
// global typedefs

#ifndef int8_16_32_64_defined
#define int8_16_32_64_defined
typedef unsigned char      uint8;
typedef unsigned short     uint16;
// On LP64 (64-bit Linux/macOS), unsigned long is 8 bytes; use unsigned int to keep uint32 at 4.
// int32 stays as long to preserve overload resolution on the host (avoids ambiguity with int).
#if defined(__LP64__)
typedef unsigned int       uint32;
#else
typedef unsigned long      uint32;
#endif
typedef long               int32;

typedef char               int8;
typedef short              int16;

#if BPE_TARGET != BPE_TARGET_WIN32
typedef unsigned long long   uint64;
#else
typedef unsigned __int64   uint64;
#endif

#if BPE_TARGET != BPE_TARGET_WIN32
typedef long long          int64;
#else
typedef __int64            int64;
#endif


#endif


typedef float              real32;
typedef double             real64;

//-------------------------------------------------------------------------------------------------------
// Platform-specific includes

#if BPE_TARGET == BPE_TARGET_RVL
#include <revolution/os.h>
#elif BPE_TARGET == BPE_TARGET_VITA
#include <scetypes.h>
#elif BPE_TARGET == BPE_TARGET_DREAMCAST
#include <kos/mutex.h>
#endif

//-------------------------------------------------------------------------------------------------------
// Replacements for windows typedefs
typedef void *                      BPE_HANDLE;       // Equivalent to windows HANDLE
#define BPE_INVALID_HANDLE_VALUE    ((BPE_HANDLE)-1)

#if BPE_TARGET == BPE_TARGET_WIN32
#define BPE_WINAPI                  __stdcall
#else
#define BPE_WINAPI
#endif

typedef struct _BPE_GUID {                            // Equivalent to windows GUID
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} BPE_GUID;

#if BPE_TARGET == BPE_TARGET_WIN32

typedef struct _RTL_CRITICAL_SECTION_DEBUG   RTL_CRITICAL_SECTION_DEBUG;

typedef struct _BPE_RTL_CRITICAL_SECTION {
    RTL_CRITICAL_SECTION_DEBUG *DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    int32      LockCount;
    int32      RecursionCount;
    BPE_HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    BPE_HANDLE LockSemaphore;
    uint32 *   SpinCount;        // force size on 64-bit systems when packed
} BPE_RTL_CRITICAL_SECTION;

typedef BPE_RTL_CRITICAL_SECTION BPE_CRITICAL_SECTION;

#elif BPE_TARGET == BPE_TARGET_PS3

// Reduce includes by duplicating PS3 lwmutex structs
typedef struct {
   volatile uint32 owner;
	volatile uint32 waiter;
} _bpe_sys_lwmutex_lock_info_t;

typedef union {
	_bpe_sys_lwmutex_lock_info_t info;
	volatile uint64 all_info;
} _bpe_sys_lwmutex_variable_t;

// sys_lwmutex
typedef struct _BPE_CRITICAL_SECTION {
	_bpe_sys_lwmutex_variable_t lock_var;
	uint32 attribute;
	uint32 recursive_count;
	uint32 sleep_queue; //_sys_sleep_queue_t
	uint32 pad;
} BPE_CRITICAL_SECTION;

#elif BPE_TARGET==BPE_TARGET_RVL

typedef OSMutex BPE_CRITICAL_SECTION;

#elif BPE_TARGET==BPE_TARGET_X360

#define BPE_CRITICAL_SECTION CRITICAL_SECTION

#elif BPE_TARGET==BPE_TARGET_VITA

typedef struct _BPE_CRITICAL_SECTION
{
   // This is used as the lightweight mutex work area. See SceKernelLwMutexWork
   SceInt64 data[4];
} BPE_CRITICAL_SECTION;

#elif BPE_TARGET==BPE_TARGET_DREAMCAST

typedef mutex_t BPE_CRITICAL_SECTION;

#else
#  error Unknown platform!
#endif

//-------------------------------------------------------------------------------------------------------

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

//-----------------------------------------------------------------------------
// provide access to debug heap APIs and structures

#if BPE_TARGET == BPE_TARGET_WIN32

#ifdef BPE_DEBUG
#include <crtdbg.h>
#endif

#define BPE_SET_BREAKPOINT             __debugbreak()
#define BPE_SET_BREAKPOINT_ALWAYS      __debugbreak()

#elif BPE_TARGET==BPE_TARGET_PS3

#if defined(SPU)
#define BPE_SET_BREAKPOINT             spu_hcmpeq(0, 0);
#define BPE_SET_BREAKPOINT_ALWAYS      spu_hcmpeq(0, 0);
#else
#define BPE_SET_BREAKPOINT             __asm__ volatile("trap")
#define BPE_SET_BREAKPOINT_ALWAYS      __asm__ volatile("trap")
#endif

#elif BPE_TARGET==BPE_TARGET_RVL

#define BPE_SET_BREAKPOINT             asm { opword 0 }
#define BPE_SET_BREAKPOINT_ALWAYS      asm { opword 0 }

#elif BPE_TARGET==BPE_TARGET_X360

#define BPE_SET_BREAKPOINT             __debugbreak()
#define BPE_SET_BREAKPOINT_ALWAYS      __debugbreak()

#elif BPE_TARGET==BPE_TARGET_VITA
#define BPE_SET_BREAKPOINT             __builtin_breakpoint(0)
#define BPE_SET_BREAKPOINT_ALWAYS      __builtin_breakpoint(0)

#elif BPE_TARGET==BPE_TARGET_DREAMCAST
#define BPE_SET_BREAKPOINT             __builtin_trap()
#define BPE_SET_BREAKPOINT_ALWAYS      __builtin_trap()

#else
#  error Unknown platform!
#endif

#define BPE_EMIT _emit

//-----------------------------------------------------------------------------
// reset BPE_SET_BREAKPOINT to perform no action on a Release build

#ifndef BPE_DEBUG
#undef BPE_SET_BREAKPOINT
#define BPE_SET_BREAKPOINT ((void)0)
#endif

//-----------------------------------------------------------------------------
// disable copy and assignment for the contained object

#define BPE_UNREF( argn ) ((void)argn)

#define BPE_DISABLE_OBJECT_COPY( objtype ) \
   private: \
   explicit objtype ( objtype const & )

#define BPE_DISABLE_OBJECT_ASSIGN( objtype ) \
   private: \
   objtype & operator=( objtype const & )

#define BPE_DISABLE_COPY_AND_ASSIGNMENT( objtype ) \
   BPE_DISABLE_OBJECT_COPY( objtype ); \
   BPE_DISABLE_OBJECT_ASSIGN( objtype )

//-----------------------------------------------------------------------------
// helper functions
#define BPE_ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define BPE_MAKE_FOURCC(ch0, ch1, ch2, ch3) ((uint32)(uint8)(ch0) | ((uint32)(uint8)(ch1) << 8) | ((uint32)(uint8)(ch2) << 16) | ((uint32)(uint8)(ch3) << 24 ))

// Turn offset into ptr (offset is added to "ptr to offset", if offset is 0, the return is NULL)
#define BPE_OFFSET_GET_POINTER(type,offset)              \
   (                                                     \
   (offset)? (type*)((int32)(&offset)+(int32)(offset))   \
   : (type*)0                                            \
   )  

//-----------------------------------------------------------------------------

#ifdef _MANAGED
#define MANAGED_PUBLIC public
#else
#define MANAGED_PUBLIC
#endif

//----------------------------------------------------------------------------
// Other header files to define global behavior

#if BPE_TARGET==BPE_TARGET_RVL
#include <stddef.h>
#endif

//----------------------------------------------------------------------------
