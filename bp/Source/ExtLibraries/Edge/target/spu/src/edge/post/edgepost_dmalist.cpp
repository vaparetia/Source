/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge/edge_stdint.h"
#include <spu_intrinsics.h>
#include "edge/edge_printf.h"
#include "edge/edge_assert.h"
#include "edge/post/edgePost.h"
#include "edgepost_internals.h"

static void _edgePostBuildDmaList( EdgeDmaListElement* pDmaList, uint32_t size, uint32_t base, uint32_t stride, uint32_t count )
{
	count = ( count + 1) >> 1;
	qword list = si_from_uint( (uint32_t)pDmaList);
	vec_uint4 element = { size, base, size, base + stride};
	const vec_uint4 increment = { 0, stride << 1, 0, stride << 1};

	for ( uint32_t j = 0; j < count; ++j )
	{
		vec_uint4 _element = spu_add( element, increment );
		qword _list = si_ai( list, 16U );
		si_stqd( (qword)element, list, 0 );
		element = _element;
		list = _list;
	}
}

// Build a dma list to bring tile data from main memory to LS ( or viceversa )
void _edgePostBuildTileDmaList( EdgeDmaListElement* pDmaList, uint32_t tileX, uint32_t tileY, uint32_t sourceIndex, const EdgePostProcessStage& stage)
{
	const EdgePostSourceTile& source = stage.sources[ sourceIndex];
	uint32_t realSourceTileWidth = source.width + source.borderWidth * 2;
	uint32_t realSourceTileHeight = source.height + source.borderHeight * 2;
	uint32_t sourceTileStartEa = source.addressEa +
		( tileY * source.height - source.borderHeight) * source.pitch + 
		( tileX * source.width - source.borderWidth) * source.bytesPerPixel;
	uint32_t srcTileStride = realSourceTileWidth * source.bytesPerPixel;

	// double-check assumptions
	EDGE_ASSERT( ( srcTileStride & 0xf) == 0 );
	EDGE_ASSERT( ( source.pitch & 0xf) == 0 );
	EDGE_ASSERT( ( sourceTileStartEa & 0xf) == 0 );

	// build dmalist
	_edgePostBuildDmaList( pDmaList, srcTileStride, sourceTileStartEa, source.pitch, realSourceTileHeight );
}
