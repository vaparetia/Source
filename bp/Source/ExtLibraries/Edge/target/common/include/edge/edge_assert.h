/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ASSERT_H__
#define __EDGE_ASSERT_H__

//////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) | defined(__PPU__)
#include <assert.h>
#elif defined(__SPU__)
#include <spu_intrinsics.h>
#endif

#include "edge/edge_printf.h"

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" 
{
#endif	//__cplusplus

//////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)

	#ifdef NDEBUG
		#define EDGE_ASSERT( condition )								\
__pragma(warning(push))													\
__pragma(warning(disable:4127))											\
			do { } while (0)											\
__pragma(warning(pop))
		#define EDGE_ASSERT_MSG( condition, format )					\
__pragma(warning(push))													\
__pragma(warning(disable:4127))											\
			do { } while (0)											\
__pragma(warning(pop))
	#else
		#define EDGE_ASSERT( condition )								\
			do {														\
				assert( (condition) );									\
__pragma(warning(push))													\
__pragma(warning(disable:4127))											\
			} while (0)													\
__pragma(warning(pop))
		#define EDGE_ASSERT_MSG( condition, format )					\
			do															\
			{															\
				if ( __builtin_expect( !( condition ), 0 ) )			\
				{														\
					EDGE_PRINTF format ;								\
					assert( (condition) );								\
				}														\
__pragma(warning(push))													\
__pragma(warning(disable:4127))											\
				} while (0)												\
__pragma(warning(pop))
	#endif

#elif defined(__PPU__)

	#ifdef NDEBUG
		#define EDGE_ASSERT( condition )								\
			do { } while (0)
		#define EDGE_ASSERT_MSG( condition, format )					\
			do { } while (0)
	#else
		#define EDGE_ASSERT( condition )								\
			do { assert( (condition) ); } while (0)
		#define EDGE_ASSERT_MSG( condition, format )					\
			do															\
			{															\
				if ( __builtin_expect( !( condition ), 0 ) )			\
				{														\
					EDGE_PRINTF format ;								\
					assert( (condition) );								\
				}														\
			} while (0)
	#endif

#elif defined(__SPU__)

	#ifdef NDEBUG
		#define EDGE_ASSERT( condition )								\
			do { } while (0)
		#define EDGE_ASSERT_MSG( condition, format )					\
			do { } while (0)
	#else
		#define EDGE_ASSERT( condition )								\
			do															\
			{															\
				if ( __builtin_expect( !( condition ), 0 ) )			\
				{														\
					_edgeAssertionFailure( __FILE__, __LINE__ );		\
				}														\
			} while (0)
		#define EDGE_ASSERT_MSG( condition, format )					\
			do															\
			{															\
				if ( __builtin_expect( !( condition ), 0 ) )			\
				{														\
					EDGE_PRINTF format ;								\
					_edgeAssertionFailure( __FILE__, __LINE__ );		\
				}														\
			} while (0)
	#endif

#else

	#error EDGE_ASSERT and EDGE_ASSERT_MSG not defined

#endif

//////////////////////////////////////////////////////////////////////////

#if defined(__SPU__)

#include <edge/edge_stdint.h> // for uint32_t
void _edgeAssertionFailure( const char* filename, uint32_t lineNumber ) __attribute__((weak)) __attribute__((noinline));
void _edgeAssertionFailure( const char* filename, uint32_t lineNumber )
{
	EDGE_PRINTF( "EDGE ASSERTION FAILURE: %s(%d)\n", filename, lineNumber );
	spu_stop( 0x3FFF );
}

#endif

//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif	//__cplusplus

//////////////////////////////////////////////////////////////////////////

#endif	//__EDGE_ASSERT_H__
