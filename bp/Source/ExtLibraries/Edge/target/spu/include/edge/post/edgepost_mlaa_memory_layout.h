/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGE_POST_MLAA_MEMORYLAYOUT_H
#define EDGE_POST_MLAA_MEMORYLAYOUT_H

/*
 * The task switches between MLAA and transpose mode, with both modes using healthy amounts of LS.
 * Thus, all globals are gathered here so that we can switch between memory layouts.
 *
 * It should be noted that the system does not work with 1920 pixel scanlines.
 */

#include "edgepost_direction_lock.h"
#include "edgepost_inplace_transpose.h"

// JDW: Set to enable support for 1080p.  This reduces the size of the memory layout to fit a maximum line-width of
// 1920.  Besides the obvious benefit of allowing MLAA on a 1080p buffer, this has a few minor drawbacks:
// 1) It's possible (but unlikely) that very noisy scenes may miss some edges.  I haven't found any normal scene where
//    this can occur though, and if it does it's localized and mostly unnoticable.
// 2) There's a couple extra instructions per pixel in edgePostMlaaFindSeparationLines.
// 3) 1080p is 2.25x as many pixels as 720p, so it's proportionately slower.  But you're also saving proportionately by
//    replacing MSAA.
#define EDGE_MLAA_SUPPORT_1080  0 // JDW: If you change this here you must also change it in edgepost_mlaa_find_separation_lines.spa

#define EDGE_MLAA_ALIGNED(a) __attribute__((aligned(a)))

#define LINE_WIDTH			(EDGE_MLAA_SUPPORT_1080 ? 1920 : 1280) // JDW: If you change this here you must also change it in edgepost_mlaa_find_separation_lines.spa
#define BUFFER_LINES		8
#define BUFFER_LINE_MASK	(BUFFER_LINES-1)
#define PARAMETER_POOL_SIZE 10

// JDW: Maximum number of separator line qwords.  Reducing this saves space, but may result in errors if there are a
// lot of separator lines on a single scanline.
//   The memory cost is proportional to 128*EDGE_MLAA_SEPLINEQWORDS.  At 1920 width, the original value (LINE_WIDTH/2)
// cost us 120K, in total over by about 60K.  A value of 50% of the original leaves us enough room to fit and seems to
// be enough for even pathological cases (e.g. a screen full of thin vertical or horizontal stripes).  25% seems to be
// enough for all but the most pathological cases and leaves a lot of memory to spare (say, to allow for non-optimized
// builds).
#define EDGE_MLAA_SEPLINEQWORDS  (EDGE_MLAA_SUPPORT_1080 ? (LINE_WIDTH/4) : (LINE_WIDTH/2)) // JDW: If you change this here you must also change it in edgepost_mlaa_find_separation_lines.spa

typedef struct EdgePostMlaaMemoryLayout
{
	uint8_t  ParamStatus[BUFFER_LINES] EDGE_MLAA_ALIGNED(16);			// This is used to track the status of the lines currently mapped into ParameterBuffer
	uint8_t  BlendStatus[BUFFER_LINES];									// This is used to track the status of the blend lines.
	
	uint32_t* ParameterBuffer[BUFFER_LINES];							// We map/unmap/remap ParameterBuffer lines when needed, and this is the mapping table.
	uint32_t* ScratchBufferMapping[PARAMETER_POOL_SIZE-BUFFER_LINES];	// Lines currently not mapped to ParameterBuffer are here.

	vec_int4 SepLineBufferEnds;											// End-pointers for separation-lines before compression.
	uint32_t ParameterBufferPool[PARAMETER_POOL_SIZE][LINE_WIDTH] EDGE_MLAA_ALIGNED(128);	//these are the separate parameters.
	uint32_t BlendBuffer[BUFFER_LINES][LINE_WIDTH] EDGE_MLAA_ALIGNED(128);					//these are the blended lines and thus the output.
	vec_int4 SepLineStartAndLength[4][EDGE_MLAA_SEPLINEQWORDS+1]EDGE_MLAA_ALIGNED(128);		//start position and length of separation lines, in order. +1 because findSepLines may write behind the last element.
	uint32_t ValidPixels[BUFFER_LINES];									//number of valid sep-line pixels per scanline. Used to control computeBounds.

	vec_int4 BlendJobs[4*EDGE_MLAA_SEPLINEQWORDS];						//buffer for jobs to be given to blend. Only one needed.

	EdgeDmaListElement DmaList[2*BUFFER_LINES][LINE_WIDTH/EDGEPOST_TRANSPOSE_MIN_BLOCK_SIZE]; //each buffer line gets its own get and put lists.

	int32_t		Jobs;			// Number of jobs found for the current line.
	int32_t		MinFetch;		// Smallest valid line index to fetch. Lines with smaller indices are not fetched.
	int32_t		MaxFetch;		// Largest valid line index to fetch. Lines with larger indices are not fetched.
	uint32_t	BaseDmaTag;		// Smallest DMA tag to be used.
	uint32_t	TransposeBlockSize;	// A transposed block contains TransposeBlockSize*TransposeBlockSize pixels.
} EdgePostMlaaMemoryLayout;

#endif //EDGE_POST_MLAA_MEMORYLAYOUT_H
