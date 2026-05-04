/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEGEOM_CONFIG_H
#define EDGEGEOM_CONFIG_H

//
// Toggle C++ reference implementations of various Edge functions.
// - A value of 1 means "use the optimized, intrinsics implementation"
// - A value of 0 means "use the reference C++ implementation"
//
// This is mainly of use for debugging, if you suspect a bug in Edge,
// or wish to track your data through Edge.  Note that the C++
// versions are *much* slower than the intrinsic versions; they should
// never ever be used in release code!
//

#define EDGEGEOMCULLOCCLUDEDTRIANGLES_INTRINSICS 1
#define EDGEGEOMCULLTRIANGLES_INTRINSICS 1
#define EDGEGEOMNORMALIZEUNIFORMTABLE_INTRINSICS 1
#define EDGEGEOMSKINVERTEXES_INTRINSICS 1
#define EDGEGEOMTRANSFORMVERTEXES_INTRINSICS 1
// Internal functions
#define COMPRESSVERTEXESBYDESCRIPTION_INTRINSICS 1
#define CULLOCCLUDEDTRIANGLES_INTRINSICS 1
#define DECOMPRESSBLENDSHAPERUNTABLE_INTRINSICS 1
#define DECOMPRESSINDEXES_INTRINSICS 1
#define DECOMPRESSVERTEXESBYDESCRIPTION_INTRINSICS 1
#define	TRANSFORMVERTEXESFORCULL_INTRINSICS 1

//
// Disable unused features to reduce ELF size
//

// Safe to disable if all vertex formats are decompressed/compressed using built-in
// vertex formats (or custom callbacks).  This will not affect blend shapes, which
// always use the generic stream-description-based decompression code.
#define ENABLE_DECOMPRESS_BY_DESCRIPTION 1 // Code savings: ~6.5K
#define ENABLE_COMPRESS_BY_DESCRIPTION 1 // Code savings: ~5K

// Safe to disable if you're not using 4x RGMS multisampling on any render targets.
#define ENABLE_4XRGMS_CULLING 1  // Code savings: ~1K

// Safe to disable if you're not using Edge's compressed index buffer format
#define ENABLE_INDEX_DECOMPRESSION 1 // Code savings: ~3K


#endif // EDGEGEOM_CONFIG_H

