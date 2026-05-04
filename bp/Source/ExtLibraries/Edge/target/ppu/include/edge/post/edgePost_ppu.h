/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef EDGEPOST_PPU_H
#define EDGEPOST_PPU_H

#include "edge/edge_stdint.h"
#include "edge/edge_stdbool.h"
#include "edge/post/edgePost.h"

// tile direction
enum EdgePostTileDir
{
	kEdgePostInputTile = 1,
	kEdgePostOutputTile,
	kEdgePostScratchTile,
};

// Image descriptor
struct EdgePostImage
{
	EdgePostImage(){}
	EdgePostImage(EdgePostTileDir dir, void* pEa, uint16_t w, uint16_t h, uint8_t ps, uint8_t borderX = 0, uint8_t borderY = 0, uint8_t multiplier = 1);
	EdgePostImage(uint16_t w, uint16_t h, uint8_t ps, uint8_t borderX = 0, uint8_t borderY = 0, uint8_t multiplier = 1);

	EdgePostTileDir m_dir;
	uint32_t m_ea;
	uint32_t m_pitch;
	uint16_t m_width;
	uint16_t m_height;
	uint8_t	m_pixelSize;
	uint8_t	m_borders[2];
	uint8_t	m_multiplier;
};


// Select a tile candidate dimension
uint32_t edgePostSelectTileSize
			( 
				const uint32_t*			pCandidates,				// pool of candidate tile sizes.
				uint32_t				numCandidates,				// number of candidate
				const EdgePostImage*	pImages,					// input image dimensions ( first entry is output, subsequents are inputs )
				uint32_t				numImages,					// number of input images
				uint32_t				heapSize					// amount of free space in LS for tiles data
			);


// Setup a stage
void edgePostSetupStage
			( 
				EdgePostProcessStage*	pStage,						// state structure to setup
				const EdgePostImage*	pImages,					// input image dimensions ( first entry is output, subsequents are inputs )
				uint32_t				numImages,					// number of input images
				const void*				pEffectCode = 0,			// pointer to embedded spu code
				uint32_t				effectCodeSize = 0,			// with bss
				uint32_t				effectCodeDmaSize = 0,		// without bss
				const void*				pStageParameter = 0,		// EA of parameters
				uint32_t				stageParameterSize = 0,		// size of parameters
				const uint32_t*			pCandidates = 0,			// pool of candidate tile sizes ( or default )
				uint32_t				numCandidates = 0,			// number of candidates
				uint32_t				totalHeapSize = EDGEPOST_DEFAULT_HEAP_SIZE
			);

// Initialize workload
void edgePostInitializeWorkload( EdgePostWorkload* pWorkload, const EdgePostProcessStage* pStages, uint16_t stageCount );


// Query if workload is finished
bool edgePostIsWorkloadFinished( EdgePostWorkload* pWorkload );


// Stall for a workload
void edgePostStallForWorkload( EdgePostWorkload* pWorkload );


#endif // EDGEPOST_H
