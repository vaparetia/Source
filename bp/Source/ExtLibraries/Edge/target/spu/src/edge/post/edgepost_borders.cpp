/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_stdint.h"
#include <spu_intrinsics.h>
#include <string.h>
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/post/edgePost.h"
#include "edge/post/edgePost_spu.h"
#include "edgepost_internals.h"

// Take leftmost pixel and replicate
static const vec_char16 s_replicateFirst_8 = {	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
static const vec_char16 s_replicateFirst_4 = {	0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03 };
static const vec_char16 s_replicateFirst_2 = {	0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01 };
static const vec_char16 s_replicateFirst_1 = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Take rightmost pixel and replicate four times.
static const vec_char16 s_replicateLast_8 = { 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
static const vec_char16 s_replicateLast_4 = { 0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f };
static const vec_char16 s_replicateLast_2 = { 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f };
static const vec_char16 s_replicateLast_1 = { 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f };

static void _edgePostReplicateLeft( uint32_t scanlineCount, uint8_t* src, uint32_t stride, uint32_t count, const vec_char16 replicateMask )
{
	for ( uint32_t j = 0; j != scanlineCount; ++j )
	{
		vec_char16 border = si_lqd( si_from_ptr( src), 0x00);
		border = si_shufb( border, border, replicateMask);
		vec_char16* dst = (vec_char16*)src - 1;
		for ( uint32_t i = 0; i < count; ++i )
			*dst-- = border;
		src += stride;
	}
}

static void _edgePostReplicateRight( uint32_t scanlineCount, uint8_t* src, uint32_t stride, uint32_t count, const vec_char16 replicateMask )
{
	for ( uint32_t j = 0; j != scanlineCount; ++j )
	{
		vec_char16 border = si_lqd( si_from_ptr( src), 0x00);
		border = si_shufb( border, border, replicateMask);
		vec_char16* dst = (vec_char16*)src + 1;
		for ( uint32_t i = 0; i < count; ++i )
			*dst++ = border;
		src += stride;
	}
}

static void _edgePostReplicateBorders( uint32_t tileX, uint32_t tileY, const EdgePostProcessStage& info, int sourceIndex, uint8_t* inBuffer )
{
	const EdgePostSourceTile& source = info.sources[ sourceIndex];
	const uint32_t overfetchx = source.borderWidth;
	const uint32_t overfetchy = source.borderHeight;
	const uint32_t srcStride = source.bytesPerPixel * ( source.width + 2 * overfetchx);
	const uint32_t tileHeight = source.height;
	const uint32_t fullTileHeight = tileHeight + 2 * overfetchy;

	EDGE_ASSERT( (srcStride & 0xf) == 0);

	if ( overfetchy != 0 )
	{
		if ( tileY == 0 )
		{
			// Replicate top scanline
			for ( uint32_t i = 0; i != overfetchy; ++i )
			{
				edgePostMemCopy( inBuffer + i * srcStride, inBuffer + overfetchy * srcStride, srcStride );
			}
		}

		if ( tileY == uint32_t( info.numTileY - 1) )
		{
			// Replicate bottom scanline
			for ( uint32_t i = 0; i != overfetchy; ++i )
			{
				edgePostMemCopy( inBuffer + ( fullTileHeight - i - 1) * srcStride, 
						inBuffer + ( fullTileHeight - overfetchy - 1) * srcStride, srcStride );
			}
		}
	}

	bool bLeftRightBorders = ( overfetchx != 0 ) && ( ( tileX == 0 ) || ( tileX == uint32_t( info.numTileX - 1 )));
	if ( bLeftRightBorders )
	{

		vec_char16 replicateFirst = s_replicateFirst_4;
		vec_char16 replicateLast = s_replicateLast_4;
		uint32_t qcount = overfetchx;

		switch (source.bytesPerPixel)
		{
		case 8:
			replicateFirst = s_replicateFirst_8;
			replicateLast = s_replicateLast_8;
			qcount >>= 1;
			break;

		case 2:
			replicateFirst = s_replicateFirst_2;
			replicateLast = s_replicateLast_2;
			qcount >>= 3;
			break;

		case 1:
			replicateFirst = s_replicateFirst_1;
			replicateLast = s_replicateLast_1;
			qcount >>= 4;
			break;

		case 4:
			qcount >>= 2;
			break;

		default:
			EDGE_ASSERT(0 && "unreachable");
		}

		if ( tileX == 0 )
			_edgePostReplicateLeft( fullTileHeight, inBuffer + overfetchx * source.bytesPerPixel, srcStride, qcount, replicateFirst );

		if ( tileX == uint32_t( info.numTileX - 1 ))
			_edgePostReplicateRight( fullTileHeight, inBuffer + srcStride - ( overfetchx + 1 ) * source.bytesPerPixel, srcStride, qcount, replicateLast );
	}

}

void _edgePostExpandBorders( uint32_t tileX, uint32_t tileY, const EdgePostProcessStage& stage, 
							uint8_t** pInput, uint8_t** pBorderCorrectedInput )
{
	// Expand borders
	for ( int j = 0; j < EDGE_POST_MAX_TILES; ++j )
	{
		if (pInput[j])
		{
			// Replicate borders if needed
			if (stage.sources[j].type != EDGE_POST_TILE_SCRATCH)
				_edgePostReplicateBorders( tileX, tileY, stage, j, pInput[j] );

			// Shift to account for borders
			const EdgePostSourceTile& source = stage.sources[ j];
			const uint32_t srcStride = source.bytesPerPixel * ( source.width + 2 * source.borderWidth);
			pBorderCorrectedInput[j] = pInput[j] + source.borderHeight * srcStride + source.borderWidth * source.bytesPerPixel;
		}
	}
}
