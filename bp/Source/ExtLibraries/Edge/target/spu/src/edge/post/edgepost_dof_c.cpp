/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/post/edgePost.h"
#include "edge/post/edgePost_spu.h"
#include "edge/edge_stdint.h"
#include "edge/edge_assert.h"


/*
// Scaled sample points
static const int kSampleCount = 16;
const vec_float4 g_defaultDofTaps[kSampleCount] = {
	TAP_OFFSET( -0.258220f, 0.543813f ),
	TAP_OFFSET( -0.434058f, 0.381764f ),
	TAP_OFFSET( 0.056296f, 0.616842f ),
	TAP_OFFSET( -0.706061f, 0.132693f ),
	TAP_OFFSET( -0.202318f, 0.239745f ),
	TAP_OFFSET( -0.764672f, -0.063198f ),
	TAP_OFFSET( 0.263764f, 0.776956f ),
	TAP_OFFSET( -0.438999f, -0.007373f ),
	TAP_OFFSET( 0.414320f, 0.433819f ),
	TAP_OFFSET( -0.181898f, 0.029110f ),
	TAP_OFFSET( 0.077827f, 0.103592f ),
	TAP_OFFSET( -0.833748f, -0.325049f ),
	TAP_OFFSET( 0.070210f, 0.375632f ),
	TAP_OFFSET( -0.261129f, -0.348633f ),
	TAP_OFFSET( -0.500331f, -0.333446f ),
	TAP_OFFSET( -0.050543f, 0.843876f ),
};
*/

// 16 taps default offsets
#define TAP_OFFSET( A, B ) { .5f + ( A * 6.f), .5f + ( B * 6.f), .0f, .0f }
const vec_float4 g_edgePostDefaultDof16Taps[] = 
{
	TAP_OFFSET( 0.402850, 0.040140 ),
	TAP_OFFSET( 0.127312, 0.372079 ),
	TAP_OFFSET( -0.287164, -0.523500 ),
	TAP_OFFSET( -0.200276,  0.553908 ),
	TAP_OFFSET( 0.177847, 0.554831 ),
	TAP_OFFSET( -0.494969, 0.287847 ),
	TAP_OFFSET( 0.314259, 0.663152 ),
	TAP_OFFSET( 0.438827,  -0.395499 ),
	TAP_OFFSET( -0.542696, 0.512252 ),
	TAP_OFFSET( 0.399431, -0.649687 ),
	TAP_OFFSET( -0.595748, -0.099768 ),
	TAP_OFFSET( -0.141366, 0.091103 ),
	TAP_OFFSET( 0.721309, 0.206925 ),
	TAP_OFFSET( -0.434476, -0.411616 ),
	TAP_OFFSET( 0.715196, -0.266536 ),
	TAP_OFFSET( 0.178104, 0.747583 ),
};
#undef TAP_OFFSET

#define TAP_OFFSET( A, B ) { A*1.5f, B*1.5f, .0f, .0f }
const vec_float4 g_edgePostDefaultDof36Taps[] = 
{
	TAP_OFFSET( -3, -1 ),
	TAP_OFFSET( -3, 0 ),
	TAP_OFFSET( -3, 1 ),
	TAP_OFFSET( -2, -2 ),
	TAP_OFFSET( -2, -1 ),
	TAP_OFFSET( -2, 0 ),
	TAP_OFFSET( -2, 1 ),
	TAP_OFFSET( -2, 2 ),
	TAP_OFFSET( -1, -3 ),
	TAP_OFFSET( -1, -2 ),
	TAP_OFFSET( -1, -1 ),
	TAP_OFFSET( -1, 0 ),
	TAP_OFFSET( -1, 1 ),
	TAP_OFFSET( -1, 2 ),
	TAP_OFFSET( -1, 3 ),
	TAP_OFFSET( 0, -3 ),
	TAP_OFFSET( 0, -2 ),
	TAP_OFFSET( 0, -1 ),
	TAP_OFFSET( 0, 1 ),
	TAP_OFFSET( 0, 2 ),
	TAP_OFFSET( 0, 3 ),
	TAP_OFFSET( 1, -3 ),
	TAP_OFFSET( 1, -2 ),
	TAP_OFFSET( 1, -1 ),
	TAP_OFFSET( 1, 0 ),
	TAP_OFFSET( 1, 1 ),
	TAP_OFFSET( 1, 2 ),
	TAP_OFFSET( 1, 3 ),
	TAP_OFFSET( 2, -2 ),
	TAP_OFFSET( 2, -1 ),
	TAP_OFFSET( 2, 0 ),
	TAP_OFFSET( 2, 1 ),
	TAP_OFFSET( 2, 2 ),
	TAP_OFFSET( 3, -1 ),
	TAP_OFFSET( 3, 0 ),
	TAP_OFFSET( 3, 1 ),
};
#undef TAP_OFFSET

inline vec_float4 floorf4( vec_float4 x )
{
	vec_int4   xi, xi1;
	vec_uint4  inrange;
	vec_float4 truncated, truncated1;

	// Find truncated value and one less.

	inrange = spu_cmpabsgt( (vec_float4)spu_splats(0x4b000000), x );

	xi = spu_convts( x, 0 );
	xi1 = spu_add( xi, -1 );

	truncated = spu_sel( x, spu_convtf( xi, 0 ), inrange );
	truncated1 = spu_sel( x, spu_convtf( xi1, 0 ), inrange );

	// If truncated value is greater than input, subtract one.

	return spu_sel( truncated, truncated1, spu_cmpgt( truncated, x ) );
}

static vec_float4 edgePostNearestSample_C( const vec_float4* pSourceTex, const vec_float4 uv, uint32_t stride )
{
#if 0
	vec_int4 iuv = spu_convts( floorf4( uv), 0 );
#else
	// convert uv to (signed) integer
	vec_int4 iuv = spu_convts( uv, 0 );
#endif

	// calculate offset from base
	int offset = 16 * spu_extract( iuv, 0) + int(stride) * spu_extract( iuv, 1);

	// load
	return (vec_float4)si_lqx( si_from_ptr( pSourceTex), si_from_int( offset) );
}

static void UnpackFuzzinessData( const uint32_t src, float& fuzz, float& depth, float& leakFactor )
{
	uint16_t ifuzz = src >> 16;
	uint8_t idepth = src & 0xff;
	uint8_t ileakFactor = ( src >> 8) & 0xff;
	fuzz = float(ifuzz) * ( 1.f / 65535.f );
	depth = float(idepth) * ( 1.f / 255.f );
	leakFactor = float(ileakFactor) * ( 1.f / 255.f );
}

static uint32_t PackFuzzinessData( float fuzz, float depth, float leakFactor )
{
	uint32_t ifuzz = uint32_t( fuzz * 65535.f);
	uint32_t idepth = uint32_t( depth * 255.f );
	uint32_t ileakFactor = uint32_t( leakFactor * 255.f );
	uint32_t r = ( ifuzz << 16 ) | ( ileakFactor << 8 ) | ( idepth );
	return r;
}

extern "C"
void edgePostDofVsc_C( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable, int numSamples )
{
	uint32_t* pOutput = (uint32_t*)output;
	vec_float4* pInput = (vec_float4*)input;

	for ( uint32_t c = 0; c < count; ++c )
	{
		// Read centre sample
		vec_float4 centre = pInput[c];
		float centreFuz;
		float centreDepth;
		float centreLeakFactor;
		UnpackFuzzinessData( spu_extract( (vec_uint4)centre, 0), centreFuz, centreDepth, centreLeakFactor );

		// Start with centre sample
		vec_float4 accumulator = centre;
		float fuzzAccumulator = 1.f;

		// Sample points
		for ( int i = 0; i < numSamples; ++i )
		{
			vec_float4 uv = spu_mul( spu_splats( centreFuz), tapOffsetTable[i]);
			vec_float4 sample = edgePostNearestSample_C( &pInput[c], uv, stride );
			
			// extract fuzziness information
			float sampleFuz;
			float sampleDepth;
			float sampleLeakFactor;
			UnpackFuzzinessData( spu_extract( (vec_uint4)sample, 0), sampleFuz, sampleDepth, sampleLeakFactor );
			
			// adjust fuzziness 
			sampleFuz = sampleDepth < centreDepth ? sampleFuz * centreLeakFactor : sampleFuz;

			accumulator = spu_madd( sample, spu_splats(sampleFuz), accumulator );
			fuzzAccumulator += sampleFuz;
		}

		// normalize kernel result
		float iFuzzAccumulator = 1.f / fuzzAccumulator;
		accumulator = spu_mul( accumulator, spu_splats( iFuzzAccumulator) );

		// insert fuzziness into alpha of accumulator to be used as a blending factor
		accumulator = spu_insert( centreFuz, accumulator, 0);

		// convert to integer
		vec_uint4 p = spu_convtu( spu_mul( accumulator, spu_splats( 255.0f/256.0f)), 8);

		// output
		uint32_t a = spu_extract( p, 0 );
		uint32_t r = spu_extract( p, 1 );
		uint32_t g = spu_extract( p, 2 );
		uint32_t b = spu_extract( p, 3 );
		uint32_t argb = a << 24 | r << 16 | g << 8 | b;
		pOutput[c] = argb;
	}
}

extern "C"
void edgePostDof_C( void* output, const void* input, uint32_t stride, uint32_t count, const vec_float4* tapOffsetTable )
{
	edgePostDofVsc_C( output, input, stride, count, tapOffsetTable, sizeof(g_edgePostDefaultDof16Taps) / sizeof(g_edgePostDefaultDof16Taps[0]) );
}


//	Interleave color and fuzziness
//	Premultiply samples by fuzziness
extern "C"
void edgePostInitializeDofInputBuffer_C( void* output, const void* colors, const void* fuzziness, uint32_t count )
{
	// we need to do this backwards to be able to convert in place
	vec_float4* pOutput = (vec_float4*)output;
	const uint32_t* pColors = (const uint32_t*)colors;
	const float* pFuzziness = (const float*)fuzziness;

	while ( count-- ) 
	{

		uint32_t c = pColors[count];
		float f = pFuzziness[count];

		uint32_t ir = ( c >> 16 ) & 0xff;
		uint32_t ig = ( c >> 8 ) & 0xff;
		uint32_t ib = ( c ) & 0xff;

		vec_uint4 ic = { 0, ir, ig, ib };
		vec_float4 fc = spu_mul( spu_convtf( ic, 8), spu_splats( 256.0f/255.0f));

		pOutput[count] = spu_insert( f, fc, 0);

	}
}

// Extract near fuzziness
extern "C"
void edgePostExtractNearFuzziness_C( void* output, const void* input, uint32_t count, 
						float nearFuzzy, float nearSharp, float maxFuzziness )
{
	float* pOutput = (float*)output;
	float* pInput = (float*)input;

	float k0 = nearSharp - nearFuzzy;
	k0 = k0 == .0f ? .0f : 1.f / k0;

	for ( uint32_t c = 0; c < count; ++c )
	{
		float linearDepth = pInput[c];

		// no fuzziness by default
		float fuzziness = 0;

		if ( linearDepth < nearSharp )
		{
			// near zone, maps to -1..0
			fuzziness = ( linearDepth - nearSharp ) * k0;
			fuzziness = -( fuzziness < -maxFuzziness ? -maxFuzziness : fuzziness);
		}

		pOutput[c] = fuzziness;
	}
}

// Extract Far fuzziness
extern "C"
void edgePostExtractFarFuzziness_C( void* output, const void* input, const void* nearFuzziness, uint32_t count, 
								   float farSharp, float farFuzzy, float maxFuzziness, float defaultLeakFactor )
{
	uint32_t* pOutput = (uint32_t*)output;
	float* pInput = (float*)input;
	float* pNearFuzziness = (float*)nearFuzziness;

	float k1 = farFuzzy - farSharp;
	k1 = k1 == .0f ? .0f : 1.f / k1;

	for ( uint32_t c = 0; c < count; ++c )
	{
		float linearDepth = pInput[c];
		float nearFuz = pNearFuzziness[c];

		float fuzziness = nearFuz;
			
		if ( linearDepth > farSharp )
		{
			// far zone maps to 0..1
			float farFuzziness = ( linearDepth - farSharp ) * k1;
			fuzziness = farFuzziness;

			// clamp to max fuzziness
			fuzziness = fuzziness > maxFuzziness ? maxFuzziness : fuzziness;
		}

		float leakFactor = nearFuz > .0f ? 1.f : defaultLeakFactor;

		// store
		pOutput[c] = PackFuzzinessData( fuzziness, linearDepth, leakFactor );
	}
}
