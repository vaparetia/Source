/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#include <cell/spurs/workload.h>
#include <string.h>
#include "edge/edge_assert.h"
#include "edge/post/edgePost.h"
#include "edge/post/edgePost_ppu.h"


// default predefined tile candidates
// This candidate work fine for 720p targets
static const uint32_t kDefaultTileCandidates[] = 
{
	160, 90,
	80, 90,
	80, 60,
	80, 45,
	80, 30,
	80, 15,
	40, 22,
	20, 11,
};

// number of predefined candidates
const uint32_t kDefaultTileCandidateCount = 
	( sizeof(kDefaultTileCandidates) / sizeof(kDefaultTileCandidates[0])) / 2;


static inline uint32_t align( const uint32_t value, uint32_t alignment)
{
	return ( value + (alignment - 1)) & ~(alignment - 1);
}


EdgePostImage::EdgePostImage( EdgePostTileDir dir, void* pEa, uint16_t w, uint16_t h, uint8_t ps, uint8_t borderX, uint8_t borderY, uint8_t multiplier)
	:	m_dir(dir),
		m_ea(uint32_t(pEa)), 
		m_pitch(w * ps), 
		m_width(w), 
		m_height(h), 
		m_pixelSize(ps),
		m_multiplier( multiplier)
{
	m_borders[0] = borderX;
	m_borders[1] = borderY;

	// output tiles are not allowed to have borders
	EDGE_ASSERT( !(( dir == kEdgePostOutputTile) && (m_borders[0]!=0)));
	EDGE_ASSERT( !(( dir == kEdgePostOutputTile) && (m_borders[1]!=0)));
}

EdgePostImage::EdgePostImage(uint16_t w, uint16_t h, uint8_t ps, uint8_t borderX, uint8_t borderY, uint8_t multiplier)
	:	m_dir(kEdgePostScratchTile),
		m_ea(0xffffffff), 
		m_pitch(w * ps), 
		m_width(w), 
		m_height(h), 
		m_pixelSize(ps),
		m_multiplier( multiplier)
{
	m_borders[0] = borderX;
	m_borders[1] = borderY;
}


// Select best tile configuration based on free local store
uint32_t edgePostSelectTileSize
		( 
			const uint32_t*			pCandidates,				// pool of candidate tile sizes
			uint32_t				numCandidates,				// number of candidate
			const EdgePostImage*	pImages,					// input image dimensions ( first entry is output, subsequents are inputs )
			uint32_t				numImages,					// number of input images
			uint32_t				heapSize					// amount of free space in LS for tiles data
		)
{
	EDGE_ASSERT( numImages >= 1 );
	EDGE_ASSERT( numCandidates >= 1 );
	EDGE_ASSERT( pImages );

	for ( uint32_t i = 0; i < numCandidates; ++i )
	{
		// get current candidate size
		uint32_t cx = pCandidates[ i * 2];
		uint32_t cy = pCandidates[ i * 2 + 1];

		// Does it divide completely
		if ( ( pImages[0].m_width % cx) != 0 || ( pImages[0].m_height % cy) != 0 )
			continue;

		// How many tiles does that give?
		uint32_t outTilesX = pImages[0].m_width / cx;
		uint32_t outTilesY = pImages[0].m_height / cy;

		// Too many ?
		if ( outTilesX > 255 || outTilesY > 255 )
			continue;

		// needs double buffering only if its more then one tile
		const uint32_t numBuffers = ( outTilesX * outTilesY ) > 1 ? 2 : 1;

		// keep track of required space
		uint32_t space = 0;

		// check validity of this tile
		uint32_t y = 0;
		for ( ; y < numImages; ++y )
		{
			// if not perfect divisor discard
			if ( ( pImages[y].m_width % outTilesX) != 0 || ( pImages[y].m_height % outTilesY) != 0 )
				break;
			
			// get candidate size
			uint32_t sx = pImages[y].m_width / outTilesX;
			uint32_t sy = pImages[y].m_height / outTilesY;

			// Account for borders
			sx += 2 * pImages[y].m_borders[0];
			sy += 2 * pImages[y].m_borders[1];

			// if tile pitch not 16 byte aligned discard
			uint32_t inTilePitch = sx * pImages[y].m_pixelSize;
			if ( ( inTilePitch & 0xf) != 0 )
				break;
			
			uint32_t multiplier = pImages[y].m_multiplier;

			// number of buffers per tile
			uint32_t numTilebuffers = pImages[y].m_dir == kEdgePostScratchTile ? 1 : numBuffers;

			// dma list size is required only with output tiles
			uint32_t dmalistsize = pImages[y].m_dir == kEdgePostOutputTile ? align( sy * 8, 16) * numTilebuffers : 0;

			// accumulate storage size
			space += ( multiplier * inTilePitch * sy * numTilebuffers ) + dmalistsize;
		}
		if ( numImages != y )
			continue;

		// check if there's enough local store for current configuration
		if ( space > heapSize )
			continue;

		// we have a valid tile !
		return i;
	}

	// couldn't find valid tile !
	EDGE_ASSERT( 0 && "No valid tile size candidates!");
	return 0xffffffff;
}

void edgePostSetupStage
			( 
				EdgePostProcessStage*	pStage,						// state structure to setup
				const EdgePostImage*	pImages,					// input image dimensions ( first entry is output, subsequents are inputs )
				uint32_t				numImages,					// number of input images
				const void*				pEffectCode,				// pointer to embedded spu code
				uint32_t				effectCodeSize,				// with bss
				uint32_t				effectCodeDmaSize,			// without bss
				const void*				pStageParameter,			// EA of parameters
				uint32_t				stageParameterSize,			// size of parameters
				const uint32_t*			pCandidates,				// pool of candidate tile sizes ( or default )
				uint32_t				numCandidates,				// number of candidate
				uint32_t				totalHeapSize
			)
{
	EDGE_ASSERT( pStage );
	EDGE_ASSERT( ( numImages >= 1 ) && pImages );
	EDGE_ASSERT( ( uint32_t(pEffectCode) & 0xf) == 0 );
	EDGE_ASSERT( effectCodeDmaSize <= effectCodeSize );
	EDGE_ASSERT( ( uint32_t(pStageParameter) & 0xf) == 0 );
	EDGE_ASSERT( ( stageParameterSize & 0xf) == 0 );

	// align code size to 16 bytes
	effectCodeSize = align( effectCodeSize, 16);
	effectCodeDmaSize = align( effectCodeDmaSize, 16);

	// reset
	memset( pStage, 0, sizeof(EdgePostProcessStage));

	// heap size based on input parameter
	uint32_t heapSize = totalHeapSize - ( effectCodeSize + stageParameterSize);
	
	// candidates
	if ( !pCandidates )
	{
		pCandidates = kDefaultTileCandidates;
		numCandidates = kDefaultTileCandidateCount;
	}

	// select tile sizes
	uint32_t candidateIndex = edgePostSelectTileSize( pCandidates, numCandidates, pImages, numImages, heapSize );
	uint32_t cx = pCandidates[ candidateIndex * 2];
	uint32_t cy = pCandidates[ candidateIndex * 2 + 1];

	// get number of tiles
	pStage->numTileX = pImages[0].m_width / cx;
	pStage->numTileY = pImages[0].m_height / cy;

	// setup sources
	for ( uint32_t u = 0; u < numImages; ++u )
	{
		EDGE_ASSERT( pImages[u].m_multiplier != 0 );

		pStage->sources[u].addressEa = pImages[u].m_ea;
		pStage->sources[u].pitch = pImages[u].m_pitch;
		pStage->sources[u].bytesPerPixel = pImages[u].m_pixelSize;
		pStage->sources[u].width = pImages[u].m_width / pStage->numTileX;
		pStage->sources[u].height = pImages[u].m_height / pStage->numTileY;
		pStage->sources[u].borderWidth = pImages[u].m_borders[0];
		pStage->sources[u].borderHeight = pImages[u].m_borders[1];
		pStage->sources[u].multiplier = pImages[u].m_multiplier;
		pStage->sources[u].type = (uint8_t)pImages[u].m_dir;
	}

	// setup general section
	pStage->rsxLabelAddress = 0;
	pStage->rsxLabelValue = 0;
	pStage->effectCodeEa = (uint32_t)pEffectCode;
	pStage->effectCodeSize = effectCodeSize;
	pStage->effectCodeDmaSize = effectCodeDmaSize;
	pStage->stageParametersEa = (uint32_t)pStageParameter;
	pStage->stageParametersSize = stageParameterSize;
}
