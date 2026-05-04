/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_LZO_ALIGNED_H__
#define __EDGE_LZO_ALIGNED_H__

 //////////////////////////////////////////////////////////////////////////

// aligned
#ifdef __GNUC__
    #define EDGELZO_ALIGNED(alignment)   __attribute__((__aligned__((alignment))))
#elif defined(_MSC_VER)
    #define EDGELZO_ALIGNED(alignment)   __declspec(align(alignment))
#else
    #error   Expecting either __GNUC__ or _MSC_VER to be defined
#endif

//////////////////////////////////////////////////////////////////////////

#endif /* __EDGE_LZO_ALIGNED_H__ */
