/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <math.h>
#include "edge/post/edgePost.h"
#include "edge/post/edgePost_spu.h"

/*
void printvector( const char* msg, const vec_float4 v )
{
	spu_printf( "%s %f,%f,%f,%f\n", msg, 
		spu_extract( v, 0),
		spu_extract( v, 1),
		spu_extract( v, 2),
		spu_extract( v, 3));
}
*/

static vec_float4 edgePostNearestSample_C( const vec_float4* pSourceTex, const vec_float4 uv, uint32_t stride )
{
	// convert uv to (signed) integer
	vec_int4 iuv = spu_convts( uv, 0 );

	// calculate offset from base
	int offset = 16 * spu_extract( iuv, 0) + int(stride) * spu_extract( iuv, 1);

	// load
	return (vec_float4)si_lqx( si_from_ptr( pSourceTex), si_from_int( offset) );
}

static float max( float a, float b)
{
	return a > b ? a : b;
}

static float min( float a, float b)
{
	return a < b ? a : b;
}

void edgePostMotionblur_C( void* output, const void* input, const void* motionBuffer, uint32_t stride, uint32_t count )
{
	uint32_t* pOutput = (uint32_t*)output;
	const vec_float4* pInput = (const vec_float4*)input;
	const uint32_t* pMotion = (const uint32_t*)motionBuffer;

	for ( uint32_t c = 0; c < count; ++c )
	{
		// read centre sample and center motion
		vec_float4 centre = pInput[c];
		uint32_t pm = pMotion[c];

		// unpack motion
		vec_uint4 im = { ( pm >> 24 ) & 0xff, ( pm >> 16 ) & 0xff, ( pm >> 8 ) & 0xff, pm & 0xff };
		vec_float4 motion = spu_mul( spu_convtf( im, 8), spu_splats( 256.0f/255.0f));

		float mx = ( spu_extract( motion, 0) * 2.f) - 1.f;
		float my = ( spu_extract( motion, 1) * 2.f) - 1.f;
		float ma = min( sqrtf( mx*mx + my*my), 1.f);
		vec_float4 motionVector = { mx, my, .0f, .0f }; 
		motionVector = spu_mul( motionVector, spu_splats( 2.f ) );

		// sample along motion vector
		vec_float4 rgb = centre;
		for ( int i = 1; i < 8; ++i )
		{
			vec_float4 uv = spu_mul( motionVector, spu_splats( float(i) ) );
			vec_float4 sample = edgePostNearestSample_C( &pInput[c], uv, stride );
			rgb = spu_add( rgb, sample );
		}
		for ( int i = 1; i < 8; ++i )
		{
			vec_float4 uv = spu_mul( motionVector, spu_splats( float(-i) ) );
			vec_float4 sample = edgePostNearestSample_C( &pInput[c], uv, stride );
			rgb = spu_add( rgb, sample );
		}
		rgb = spu_mul( rgb, spu_splats( 1.f / 15.f ));


		// insert alpha
		float alpha = max( ma, spu_extract( centre, 0));
		rgb = spu_insert( alpha, rgb, 0);

		// convert to integer
		vec_uint4 irgb = spu_convtu( spu_mul( rgb, spu_splats( 255.0f/256.0f)), 8);

		// convert/store
		// output
		uint32_t a = spu_extract( irgb, 0 );
		uint32_t r = spu_extract( irgb, 1 );
		uint32_t g = spu_extract( irgb, 2 );
		uint32_t b = spu_extract( irgb, 3 );
		uint32_t argb = a << 24 | r << 16 | g << 8 | b;
		pOutput[c] = argb;

	}
}
