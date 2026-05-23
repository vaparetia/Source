/* SCE CONFIDENTIAL
* PlayStation(R)Edge 1.2.0
* Copyright (C) 2010 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

#ifndef EDGEPOST_H
#define EDGEPOST_H

#include "edge/edge_stdint.h"
#include "edge/edge_stdbool.h"


#define EDGEPOST_DEFAULT_HEAP_SIZE		1024 * 220			// default SPU heap size

// stage flags
#define EDGE_POST_WRITE_RSX_LABEL		0x01				// If flag is set unlock RSX label
#define EDGE_POST_BREAKPOINT			0x02				// If flag is set breakpoint before each tile for the current stage ( debugging aid )

// tile type
#define EDGE_POST_TILE_INPUT			1
#define EDGE_POST_TILE_OUTPUT			2
#define EDGE_POST_TILE_SCRATCH			3

// Maximum number of Tiles per stage
#define EDGE_POST_MAX_TILES				4

// Describe tile configuration for an image
// 16 bytes
struct EdgePostSourceTile
{
	uint32_t addressEa;				// address of input image or NULL if disabled
	uint32_t pitch;					// pitch of input image
	uint16_t width;					// width of tile
	uint16_t height;				// height of tile
	uint8_t borderWidth;			// border in X ( in pixels )
	uint8_t borderHeight;			// border in Y ( in scanline )
	uint8_t bytesPerPixel;			// byte per pixels
	uint8_t multiplier:6;			// LS space multiplier, the space in LS allocated for this tile will be tileSize * multiplier
	uint8_t type:2;					// tile type								
} __attribute__((__aligned__(16)));



// Stage parameters
// 128 bytes
struct EdgePostProcessStage
{
	uint8_t numTileX;				// number of tile in X
	uint8_t numTileY;				// number of tile in Y
	uint8_t pad;
	uint8_t flags;					// generic flags
	uint32_t rsxLabelAddress;		// effective address of RSX label to sync with
	uint32_t rsxLabelValue;			// value to be set 
	uint32_t effectCodeEa;			// effective address of effect code
	
	uint32_t effectCodeSize;		// amount of bytes to be allocated to hold the effect code
	uint32_t effectCodeDmaSize;		// size of DMA to bring effect code in Local Store
	uint32_t stageParametersEa;		// user parameters
	uint32_t stageParametersSize;	// value to be set 
	
	uint32_t user0;					// user data
	uint32_t user1;					// user data
	uint32_t user2;					// user data
	uint32_t user3;					// user data

	union // other generic user data
	{
		uint32_t   userDataI[4];
		float      userDataF[4];
	};

	// source/sink informations
	EdgePostSourceTile sources[ EDGE_POST_MAX_TILES ];
} __attribute__((__aligned__(128)));



// A chain of effect 
// 16 bytes
struct EdgePostWorkload
{
	// mutable 64 bytes ( accessed using atomic operations )
	uint16_t stage;					// current stage
	uint16_t tile;					// next free tile
	uint16_t tileCompleted;			// number of completed tiles
	uint16_t tileCount;				// number of tiles in current stage

	// read-only from SPU
	uint16_t stageCount;			// number of post-processing stages
	uint16_t pad;
	uint32_t processStageArrayEa;	// points to array of processing stages ( readonly )
} __attribute__((__aligned__(16)));

#endif // EDGEPOST_H
