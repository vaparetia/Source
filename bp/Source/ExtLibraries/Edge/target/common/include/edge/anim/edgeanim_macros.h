/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_MACROS_H__
#define __EDGE_MACROS_H__

// offsets
#define EDGE_OFFSET_GET_POINTER(type,offset)                                  \
	(                                                                         \
		(offset)? (type*)((intptr_t)(&offset)+(intptr_t)(offset))             \
				: (type*)0                                                    \
	)                                                                         \

// SPU bookmarks
#if defined(__SPU__) && defined (EDGE_ENABLE_BOOKMARKS)
#	define EDGE_BOOKMARK(id)        si_wrch(69, si_from_int((id)))      
#else
#	define EDGE_BOOKMARK(id)        
#endif//__SPU__

// aligned 
#ifdef __GNUC__
#	define EDGE_ALIGNED(alignment)   __attribute__((__aligned__((alignment))))
#elif defined(_MSC_VER)
#	define EDGE_ALIGNED(alignment)   __declspec(align(alignment))
#else
#	error
#endif

// aligned on PS3 only
#if (defined(__PPU__) || defined(__SPU__))
#	define EDGE_ALIGNED_PS3(alignment)   __attribute__((__aligned__((alignment))))
#else
#	define EDGE_ALIGNED_PS3(alignment)
#endif

#ifdef  _MSC_VER
#define __builtin_expect(a, b)	a
#define __restrict__
#endif

// "alignment" must be a power of 2
#define EDGE_IS_ALIGNED(value, alignment)                                   \
	(                                                                       \
		!(((uintptr_t) (value)) & (((uintptr_t) (alignment)) - 1U))         \
	)                                                                       

// if aligned on PS3 only
#if (defined(__PPU__) || defined(__SPU__))
#	define EDGE_IS_ALIGNED_PS3(value, alignment) 	EDGE_IS_ALIGNED(value, alignment) 
#else
#	define EDGE_IS_ALIGNED_PS3(value, alignment)	(true)
#endif

// "alignment" must be a power of 2
#define EDGE_ALIGN(value, alignment)                                        \
	(                                                                       \
		(((uintptr_t) (value)) + (((uintptr_t) (alignment)) - 1U))          \
		& ~(((uintptr_t) (alignment)) - 1U)                                 \
	)

#endif // __MACROS_H__
