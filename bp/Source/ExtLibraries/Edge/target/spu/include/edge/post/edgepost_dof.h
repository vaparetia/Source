/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_POST_DOF_H__
#define __EDGE_POST_DOF_H__

#include <spu_intrinsics.h>
#include "edge/edge_stdint.h"
#include "edge/post/edgepost.h"


extern "C"
{
	// array with default Depth of Field sample offsets
	
	// array size if 16, each element is { offset_x, offset_y, 0, 0 }
	extern const vec_float4 g_edgePostDefaultDof16Taps[];

	// array size if 36, each element is { offset_x, offset_y, 0, 0 }
	extern const vec_float4 g_edgePostDefaultDof36Taps[];

	// Combine a color buffer and a fuzziness buffer in a suitable format for input to Depth of Field function
	// color is 32bit per pixel
	// fuzziness is output from edgePostExtractFarFuzziness
	void edgePostInitializeDofInputBuffer( void* output, const void* colors, const void* fuzziness, uint32_t count );

	// Combine a color buffer and a fuzziness buffer in a suitable format for input to Depth of Field function
	// color is four floats per pixel
	// fuzziness is output from edgePostExtractFarFuzziness
	void edgePostInitializeDofInputBufferF4( void* output, const void* colors, const void* fuzziness, uint32_t count );

	// Calculate Depth of field ( one scanline )
	// Colour is 4 floats, alpha is fuzziness
	// Output is argb, 8 bit per channel
	// This function is optimized for 16 taps, tapOffsetTable must be 16 vectors
	void edgePostDof( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable );
	
	// Calculate Depth of field ( one scanline )
	// Colour is 4 floats, alpha is fuzziness
	// Output is argb, 16 bit per channel in format 0:5:11
	// This function is optimized for 16 taps, tapOffsetTable must be 16 vectors
	void edgePostDof_FX16( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable );

	// Calculate Depth of field ( one scanline )
	// Colour is 4 floats, alpha is fuzziness
	// Output is argb, 8 bit per channel
	// This function accept a Variable Sample Count, as such can be used for depth Of Field with more then 16 taps.
	void edgePostDofVsc( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable, unsigned numSamples );

	// Calculate Depth of field ( one scanline )
	// Colour is 4 floats, alpha is fuzziness
	// Output is argb, 16 bit per channel in format 0:5:11
	// This function accept a Variable Sample Count, as such can be used for depth Of Field with more then 16 taps.
	void edgePostDofVsc_FX16( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable, unsigned numSamples );

	// Extract near fuzziness
	// Input is one channel float scanline with depth per pixel
	// Output is one channel float scanline with fuzziness per pixel
	void edgePostExtractNearFuzziness( void* output, const void* input, uint32_t count, float nearFuzzy, float nearSharp, float maxFuzziness );
	
	// Extract far fuzziness and combine with near fuzziness
	// Output is one channel float scanline with fuzziness per pixel
	void edgePostExtractFarFuzziness( void* output, const void* input, const void* nearFuzziness, uint32_t count, float farSharp, float farFuzzy, float maxFuzziness, float defaultLeakFactor = .4f);
}



#endif // __EDGE_POST_DOF_H__
