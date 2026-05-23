/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

/*
 * MLAA works by detecting aliased lines and smoothing them through a blending process.
 *
 * A reference paper on the subject can be found at: 
 * http://visual-computing.intel-research.net/publications/mlaa.pdf
 *
 * Note that this implementation is significantly differently, to improve image quality 
 * and performance.
 *
 * Usually, MLAA will make a horizontal and a vertical pass. Of the two, the vertical pass is less
 * SPU friendly, so we have opted to perform two horizontal passes and to transpose the image 
 * in-between. 
 *
 * - MLAA Passes - 
 *  The MLAA passes operate on complete scanlines, with the fetches interleaved with the computation.
 *  There are five steps to the processing:
 *  1) Marking discontinuities, a.k.a. edge detection
 *  2) Detecting separation lines
 *  3) Detecting features and finding their parameters
 *  4) Sort features by type
 *  5) Blending
 *
 *  The first step is basically an edge-detection, marking wether a pixel lies on a 
 *  horizontal or vertical edge. We process four lines at once, to be able to work on 
 *  4x4 blocks, which allows a fast pixel-transpose for the vertical edge-detection.
 *  As the results of this process do not change between horizontal and vertical MLAA
 *  passes, we keep the flags between passes and only execute this function during the
 *  horizontal pass.
 *  (For those interested: This function also stores the sum of the color-components
 *  for each pixel, with the lower 4 bits masked out, leaving 6 bits precision. Together 
 *  with the edge information, this is stored in the 8 bits of the alpha channel)
 *
 *  The second step runs through the marked lines and looks for any sequence of horizontal
 *  edge pixels, called separation lines. For each separation line, we store where it
 *  starts, and how long it is. Furthermore, we write the position of each pixel in its
 *  separation line (or -1), the sum of its color components and its edge flags into the
 *  scanline, replacing the old color values, which are copied into the blendLines.
 *  At this point, separation lines of length one are directly blended in the buffer, to
 *  avoid running them through the entire pipeline.
 *
 *  The feature detection uses the start and length information from the previous pass to
 *  jump from separation line to separation line. Within a separation line, candidate
 *  pixels for height-computation are detected and evaluated, resulting in the creation
 *  of actual features, having extends and heights.
 *
 *  To turn the features into blend-jobs, each is evaluated and turned into up to two Z or U
 *  shapes. Some features are even discarded altogether.
 *
 *  As a final step, the blend jobs are processed and the resulting lines are written back to XDR.
 *
 * - Transpose Passes -
 *  After each MLAA pass, the image is 'transposed', to unify the horizontal and vertical operation.
 *  As our images are not square, actual in-place transposing is rather hard, so we only transpose
 *  within 128x128 blocks and then piece the complete vertical lines together using DMA-lists.
 *
 */

#include <spu_intrinsics.h>
#include <stdlib.h>
#include <spu_mfcio.h>


#include <edge/edge_stdint.h>
#include <edge/edge_assert.h>

#include <edge/post/edgepost_mlaa.h>
#include <edge/post/edgepost_mlaa_memory_layout.h>
#include <edge/post/edgepost_inplace_transpose.h>

static EdgePostMlaaMemoryLayout *g_pMem;
static uint8_t g_Debug;


#define ENABLE_DEBUG_FEATURES 1
#define CLEAR_ALPHA 0
#define CLEAR_ALPHA_VALUE 0x00

#define MAX_UNDERFETCH 1
#define MAX_OVERFETCH 2

#define SHOW_EDGES 1			// this is not a flag but a mode. Do not change.
#define SHOW_SEPARATION_LINES 2	// this is not a flag but a mode. Do not change.

// WARNING: Setting this to SHOW_SEPARATION_LINES changes the algorithm slightly and costs performance.
#define DEBUG_MODE SHOW_EDGES


// Development flags. No user serviceable parts inside.
#define EXTENDED_BUFFER_TRACKING 0
#define HORIZONTAL_TAG 0x80000000
#define VERTICAL_TAG 0x40000000


#define LINE_STATE_LOADED 1
#define LINE_STATE_THRESHOLDED 2
#define LINE_STATE_MARKED 3
#define LINE_STATE_SEPLINES 4
#define LINE_STATE_PROCESSED 5
#define BLEND_STATE_COPIED 1
#define BLEND_STATE_BLENDED 2
#define BLEND_STATE_STORED 3

extern "C" void edgePostMlaaMarkDiscontinuities(
	uint32_t* line0, uint32_t* line1, uint32_t* line2, uint32_t* line3, uint32_t* line4,
	uint32_t count);

extern "C" vec_int4 edgePostMlaaFindSeparationLines(
	uint32_t *__restrict line0,
	uint32_t *__restrict line1,
	uint32_t *__restrict line2,
	uint32_t *__restrict line3,
	vec_int4*__restrict lineLength0,
	vec_int4*__restrict lineLength1,
	vec_int4*__restrict lineLength2,
	vec_int4*__restrict lineLength3,
	uint32_t *__restrict copy0,
	uint32_t *__restrict copy1,
	uint32_t *__restrict copy2,
	uint32_t *__restrict copy3,
	const uint32_t length,
	const uint32_t flags);

extern "C" vec_int4* edgePostMlaaComputeBounds(
	uint32_t previous_param,
	uint32_t current_param,
	uint32_t next_param,
	uint32_t second_next_param,
	uint32_t lineLength,
	uint32_t blendJobs,
	uint32_t mode,
	uint32_t valid_pixels,
	uint32_t width);

extern "C" void edgePostMlaaCompressSeparationLines(
	uint32_t *__restrict copy0, 
	uint32_t *__restrict copy1, 
	uint32_t *__restrict copy2, 
	uint32_t *__restrict copy3,
	uint32_t *__restrict line4,
	vec_int4*__restrict lineLength0,
	vec_int4*__restrict lineLength1,
	vec_int4*__restrict lineLength2,
	vec_int4*__restrict lineLength3,
	uint32_t *__restrict validPixels0,
	uint32_t *__restrict validPixels1,
	uint32_t *__restrict validPixels2,
	uint32_t *__restrict validPixels3,
	const uint32_t minLineLength,
	vec_int4 bufferEnds);


extern "C" void edgePostMlaaBlendZ(vec_int4* buffer, const uint32_t jobs, const vec_uchar16* unalignedTab);
extern "C" void edgePostMlaaBlendU(vec_int4* buffer, const uint32_t jobs, const vec_uchar16* unalignedTab);

extern "C" vec_uint4 edgePostMlaaFilterBlendJobs(vec_int4* buffer, uint32_t jobs, uint32_t* line0, uint32_t* line1, uint32_t maxJobs);
extern "C" void edgePostMlaaWriteThreshold(uint32_t* ptr, uint32_t length, uint32_t factor, uint32_t base);

#if CLEAR_ALPHA
extern "C" void edgePostMlaaClearAlpha(uint32_t *buffer, uint32_t size, uint32_t clearValue);
#endif

// used to switch select the right fetch function for each pass.
typedef void (*EdgeMlaaFetchFunction)(    register uint32_t imageEa,
									  const register int32_t index,
									  const register uint32_t pitch,
									  const register uint32_t width,
									  const register uint32_t height,
									  const register uint32_t transposedPitch,
									  const register uint32_t transposedBlockOffset,
									  const register bool     first,
									  const register  int32_t scratchIndex);


static void _edgePostMlaaDmaBarrier()
{
	const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	mfc_barrier(31);
	if( interrupts_enabled )
	{
		spu_ienable();
	}
}

__attribute__ ((unused)) __attribute__((noinline)) static void _edgePostMlaaDmaWaitMayStall(uint32_t tag)
{
	EDGE_DMA_WAIT_TAG_STATUS_ALL(tag<<g_pMem->BaseDmaTag);
}

__attribute__ ((unused)) __attribute__((noinline)) static void _edgePostMlaaDmaWaitShouldNotStall(uint32_t tag)
{
	EDGE_DMA_WAIT_TAG_STATUS_ALL(tag<<g_pMem->BaseDmaTag);
}

__attribute__((noinline)) static void _edgePostMlaaAlignedMemcpy(void* dst, const void* src, uint32_t size)
{
	register qword qsrc = (qword)(vec_uint4){(uint32_t)src,0,0,0};
	register qword qdst = (qword)(vec_uint4){(uint32_t)dst,0,0,0};

	while(size>95)
	{
		const register qword v0 = si_lqd(qsrc,0x00);
		const register qword v1 = si_lqd(qsrc,0x10);
		const register qword v2 = si_lqd(qsrc,0x20);
		const register qword v3 = si_lqd(qsrc,0x30);
		const register qword v4 = si_lqd(qsrc,0x40);
		const register qword v5 = si_lqd(qsrc,0x50);

		si_stqd(v0,qdst,0x00);
		si_stqd(v1,qdst,0x10);
		si_stqd(v2,qdst,0x20);
		si_stqd(v3,qdst,0x30);
		si_stqd(v4,qdst,0x40);
		si_stqd(v5,qdst,0x50);

		qsrc = si_ai(qsrc,0x60);
		qdst = si_ai(qdst,0x60);
		size-=96;
	}
	while(size>15)
	{
		const register qword v0 = si_lqd(qsrc,0x00);
		si_stqd(v0,qdst,0x00);
		qsrc = si_ai(qsrc,0x10);
		qdst = si_ai(qdst,0x10);
		size-=16;

	}
	while(size)
	{
		const register qword s = si_lqd(qsrc,0);
		const register qword d = si_lqd(qdst,0);
		const register qword m = si_cbd(qsrc,0);
		const register qword qsrc_offsetted = si_ai(qsrc,0x0D);
		const register qword s_rot = si_rotqby(s,qsrc_offsetted);
		const register qword v = si_shufb(s_rot,d,m);
		si_stqd(v,qdst,0);
		qsrc = si_ai(qsrc,0x01);
		qdst = si_ai(qdst,0x01);
		--size;
	}
}

static inline void _edgePostMlaaSetParamStatus(const register uint32_t buffer, const register uint32_t state)
{
	const register qword qbuffer = si_from_uint(buffer);
	const register qword qstate = si_from_uint(state);
	const register qword insertMask = si_cbd(qbuffer,0);
	const register qword currentState = *(qword*) g_pMem->ParamStatus;
	const register qword newState = si_shufb(qstate,currentState,insertMask);
	*(qword*) g_pMem->ParamStatus = newState;
}

static inline void _edgePostMlaaSetBlendStatus(const register uint32_t buffer, const register uint32_t state)
{
	const register qword qbuffer = si_from_uint(buffer);
	const register qword qstate = si_from_uint(state);
	const register qword insertMask = si_cbd(qbuffer,8);
	const register qword currentState = *(qword*) g_pMem->ParamStatus;
	const register qword newState = si_shufb(qstate,currentState,insertMask);
	*(qword*) g_pMem->ParamStatus = newState;
}

static inline uint32_t _edgePostMlaaGetParamStatus(const register uint32_t buffer)
{
	const register qword qbuffer = si_from_uint(buffer);
	const register qword offsettedBuffer = si_ai(qbuffer,-3);
	const register qword currentState = *(qword*) g_pMem->ParamStatus;
	const register qword rotatedEntry = si_rotqby(currentState,offsettedBuffer);
	const qword maskedEntry = si_andi(rotatedEntry,0xff);
	return *(uint32_t*)&maskedEntry;
}

static inline uint32_t _edgePostMlaaGetBlendStatus(const register uint32_t buffer)
{	
	const register qword qbuffer = si_from_uint(buffer);
	const register qword offsettedBuffer = si_ai(qbuffer,5);
	const register qword currentState = *(qword*) g_pMem->ParamStatus;
	const register qword rotatedEntry = si_rotqby(currentState,offsettedBuffer);
	const qword maskedEntry = si_andi(rotatedEntry,0xff);
	return *(uint32_t*)&maskedEntry;
}


#if ENABLE_DEBUG_FEATURES

#if DEBUG_MODE == SHOW_EDGES
#define ARGB(a,r,g,b)	(((a)<<24)|((r)<<16)|((g)<<8)|(b))
static void _edgePostMlaaColorEdges(const register uint32_t* src, register uint32_t* __restrict dst, const register uint32_t size)
{
	for(register uint32_t i=0; i<size; ++i)
	{
		const register bool alreadyWritten = (src[i]&0xff000000) == 0xff000000;
		const register bool red = alreadyWritten?false:src[i]&0x80000000;
		const register bool green = alreadyWritten?false:src[i]&0x40000000;
		dst[i]=(red||green)?ARGB(255,red?255:0,green?255:0,0):src[i];
	}
}
#endif

#if DEBUG_MODE == SHOW_SEPARATION_LINES
static void _edgePostMlaaMarkSepLines(const uint32_t index, int pass)
{
	uint32_t register pixel_count = 0; 
	uint32_t register line = 0;

	const register uint32_t valid_pixels = 0x00FFFFFF&g_pMem->ValidPixels[index];
	const register uint32_t word_offset = g_pMem->ValidPixels[index] >> 24;

	while(pixel_count < valid_pixels)
	{
		const register vec_uint4 sep_line_info = (vec_uint4)g_pMem->SepLineStartAndLength[index&3][line];
		const register uint32_t offset_and_length = sep_line_info[word_offset];
		const register uint32_t length = (0xFFFF & offset_and_length);
		const register uint32_t offset = offset_and_length >> 18;

		g_pMem->BlendBuffer[index][offset]|=pass?0x00FF0000:0x0000FF00;
		for(uint32_t i = 1; i < length; ++i )
		{
			register uint32_t pixel = g_pMem->BlendBuffer[index][offset+i];
			pixel&=pass?0xFF00FFFF:0xFFFF00FF;
			pixel|=pass?0x00800000:0x00008000;
			g_pMem->BlendBuffer[index][offset+i]=pixel;
		}
		if(length>0)
		{
			register uint32_t pixel = g_pMem->BlendBuffer[index][offset+length];
			pixel&=pass?0xFF00FFFF:0xFFFF00FF;
			pixel|=pass?0x00D00000:0x0000D000;
			g_pMem->BlendBuffer[index][offset+length]=pixel;
		}

		++line;
		pixel_count+=length+1;
	}
}
#endif

#endif

static inline void _edgePostFillEvenSizedDmaListAligned(
	register EdgeDmaListElement* list,
	register const uint32_t elements,
	register const uint32_t startEa,
	register const uint32_t eaUpdate,
	register const uint32_t fetchSize)
{
	register const vec_uint4 element_update = {0,2*eaUpdate,0,2*eaUpdate};
	register vec_uint4 two_elements         = {fetchSize,startEa,fetchSize,startEa+eaUpdate};
	register vec_uint4 buffer = {(uint32_t)list,0,0,0};

	for(register int32_t i = elements; i > 0; i-=2)
	{
		si_stqd((qword)two_elements,(qword)buffer,0);
		two_elements+=element_update;
		buffer=(vec_uint4)si_ai((qword)buffer,0x10);
	}
}

static void _edgePostMlaaFetchLineTransposed(
								         register uint32_t imageEa,  // in transposed mode, this always points to the start of a block. use BlockOffset to start within a block
								   const register int32_t  index,
								   const register uint32_t pitch, // in transposed mode, this is the "virtual" pitch, e.g 768 for a 720p target.
								   const register uint32_t width, // this is the actual number of bytes pixels to fetch
								   const register uint32_t height,
								   const register uint32_t transposedPitch, //in transposed mode, this is the pitch of the original image.
								   const register uint32_t transposedBlockOffset, //this is the offset within a transposed block where we start.
								   const register bool     first,
								   const register  int32_t scratchIndex)
{
	(void)width;
	const register uint32_t buffer_line=index&BUFFER_LINE_MASK;
	const register uint32_t tag = (scratchIndex>-1)?scratchIndex+2*BUFFER_LINES:buffer_line;

	uint32_t *targetBuffer = (scratchIndex>-1)?g_pMem->ScratchBufferMapping[scratchIndex]:g_pMem->ParameterBuffer[buffer_line];	 

	register uint32_t fetched_line_status = LINE_STATE_MARKED;
	if(1==first)
	{
		fetched_line_status =((index < (int32_t)height) && (index >= 0))?LINE_STATE_THRESHOLDED:LINE_STATE_LOADED;
	}
	
	if(scratchIndex<0)
	{
		_edgePostMlaaSetParamStatus(buffer_line, fetched_line_status);
	}

	if(__builtin_expect((g_pMem->MinFetch<=index) && (g_pMem->MaxFetch>=index),1))
	{
		const register int32_t  total_block_offset   = transposedBlockOffset+index;
		const register bool     negative             = total_block_offset < 0;
		const register uint32_t abs_tbo              = abs(total_block_offset);

		const register int32_t abs_horizontal_block = (abs_tbo) / g_pMem->TransposeBlockSize;
		const register int32_t abs_vertical_line    = (abs_tbo) % g_pMem->TransposeBlockSize;

		// calculate the offset in 128*128 blocks.
		const register int32_t horizontal_block     = negative?-(abs_horizontal_block+1):abs_horizontal_block;
		// calculate the vertical line within this block
		const register int32_t vertical_line        = negative?g_pMem->TransposeBlockSize-abs_vertical_line:abs_vertical_line;


		const register uint32_t ea=
			horizontal_block * g_pMem->TransposeBlockSize * 4 // pick the right block to start with
			+ vertical_line * transposedPitch						// pick the right line within the block.
			+ imageEa;

		//we have to piece it together
		const register uint32_t transfers = pitch / (g_pMem->TransposeBlockSize * 4);

		_edgePostFillEvenSizedDmaListAligned(
			g_pMem->DmaList[2*buffer_line+1],
			transfers,
			ea,
			g_pMem->TransposeBlockSize*transposedPitch,	// jump to the next block
			g_pMem->TransposeBlockSize*4);				// copy a whole block line (we may want to limit this for the last iteration)

		{ 
			const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();
			EDGE_DMA_LIST_GET(
				targetBuffer,
				0,
				g_pMem->DmaList[2*buffer_line+1],
				transfers*sizeof(EdgeDmaListElement),
				tag+g_pMem->BaseDmaTag,
				0,0);
			if( interrupts_enabled )
			{
				spu_ienable();
			}
		}
	}
}

static void _edgePostMlaaFetchLineLinear(
	      register uint32_t imageEa,  // in transposed mode, this always points to the start of a block. use BlockOffset to start within a block
	const register int32_t  index,
	const register uint32_t pitch, // in transposed mode, this is the "virtual" pitch, e.g 768 for a 720p target.
	const register uint32_t width, // this is the actual number of bytes pixels to fetch
	const register uint32_t height,
	const register uint32_t transposedPitch, //in transposed mode, this is the pitch of the original image.
	const register uint32_t transposedBlockOffset, //this is the offset within a transposed block where we start.
	const register bool     first,
	const register  int32_t scratchIndex)
{
	(void) transposedPitch;
	(void) transposedBlockOffset;
	const register uint32_t buffer_line=index&BUFFER_LINE_MASK;
	const register uint32_t tag = (scratchIndex>-1)?scratchIndex+2*BUFFER_LINES:buffer_line;

	uint32_t *targetBuffer = (scratchIndex>-1)?g_pMem->ScratchBufferMapping[scratchIndex]:g_pMem->ParameterBuffer[buffer_line];
	
	register uint32_t fetched_line_status = LINE_STATE_MARKED;
	if(1==first)
	{
		fetched_line_status =((index < (int32_t)height) && (index >= 0))?LINE_STATE_THRESHOLDED:LINE_STATE_LOADED;
	}

	if(scratchIndex<0)
	{
		_edgePostMlaaSetParamStatus(buffer_line, fetched_line_status);
	}

	if(__builtin_expect((g_pMem->MinFetch<=index) && (g_pMem->MaxFetch>=index),1))
	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		const register uint32_t ea=index*pitch+imageEa;
		EDGE_DMA_GET(targetBuffer, ea, width, tag+g_pMem->BaseDmaTag, 0, 0);
		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}
}


static void _edgePostMlaaStoreLine(
	      register uint32_t imageEa,  //in transposed mode, this always points to the start of a block. use BlockOffset to start within a block
	const register int32_t index,
	const register uint32_t pitch, //in transposed mode, this is the "virtual" pitch, e.g 768 for a 720p target.
	const register uint32_t width, // this is the actual number of bytes pixels to store
	const register uint32_t transposedPitch, //in transposed mode, this is the pitch of the original image.
	const register uint32_t transposedBlockOffset, //this is the offset within a transposed block where we start.
	const register uint32_t pass,
	const register bool first)
{
	(void)first;
	if(0>index)
		return;
	const register uint32_t buffer_line=index&BUFFER_LINE_MASK;

#if EXTENDED_BUFFER_TRACKING
	EDGE_ASSERT(BLEND_STATE_BLENDED == _edgePostMlaaGetBlendStatus(buffer_line));
#endif

#if CLEAR_ALPHA
	if(!first) 
	{
		edgePostMlaaClearAlpha(&g_pMem->BlendBuffer[buffer_line][0],width,CLEAR_ALPHA_VALUE);
	}
#endif

	if(0 == pass)
	{
		const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		const register uint32_t ea=index*pitch+imageEa;
		EDGE_DMA_PUT(&g_pMem->BlendBuffer[buffer_line][0],ea,width,buffer_line+8,0,0);
		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}
	else
	{
		const register uint32_t horizontal_block = (transposedBlockOffset + index) / g_pMem->TransposeBlockSize;
		const register uint32_t vertical_line    = (transposedBlockOffset + index) % g_pMem->TransposeBlockSize;
		const register uint32_t ea=
			horizontal_block * g_pMem->TransposeBlockSize * 4 // pick the right block to start with
			+ vertical_line * transposedPitch						// pick the right line within the block.
			+ imageEa;

		//we have to piece it together
		const register uint32_t transfers = pitch / (g_pMem->TransposeBlockSize * 4);

		_edgePostFillEvenSizedDmaListAligned(
			g_pMem->DmaList[2*buffer_line],
			transfers,
			ea,
			g_pMem->TransposeBlockSize*transposedPitch,
			g_pMem->TransposeBlockSize*4);

		{
			const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();
			EDGE_DMA_LIST_PUT(
				&g_pMem->BlendBuffer[buffer_line][0],
				0,
				g_pMem->DmaList[2*buffer_line],
				transfers*sizeof(EdgeDmaListElement),
				buffer_line+8+g_pMem->BaseDmaTag,
				0,0);
			if( interrupts_enabled ) 
			{
				spu_ienable();
			}
		}
	}
#if EXTENDED_BUFFER_TRACKING
	_edgePostMlaaSetBlendStatus(buffer_line, BLEND_STATE_STORED);
#endif
}


static inline uint32_t _edgePostMlaaComputeBounds(
	register const uint32_t* __restrict const previous_param,
	register const uint32_t* __restrict const current_param,
	register const uint32_t* __restrict const next_param,
	register const uint32_t* __restrict const second_next_param,
	register const vec_int4* __restrict const lineLength,
	register       vec_int4*                  blendJobs,
	register const uint32_t                   mode,
	register const uint32_t                   pixels,
	register const uint32_t                   width)
{
	// these two parameters are used for debugging purposes at times.
	const register vec_int4* final_job_ptr=edgePostMlaaComputeBounds(
		(uint32_t)previous_param,
		(uint32_t)current_param,
		(uint32_t)next_param,
		(uint32_t)second_next_param,
		(uint32_t)lineLength,
		(uint32_t)blendJobs,
		mode,
		pixels,
		width
		);
	const register uint32_t jobs_created=uint32_t(final_job_ptr-blendJobs);

	return jobs_created;
}


static void _edgePostMlaaFindSeparationLines(
	const register uint32_t line0_index, 
	const register uint32_t pitch,
	const register bool     first)
{
	const register uint32_t line1_index = (line0_index+1)&BUFFER_LINE_MASK;
	const register uint32_t line2_index = (line0_index+2)&BUFFER_LINE_MASK;
	const register uint32_t line3_index = (line0_index+3)&BUFFER_LINE_MASK;

	const register uint32_t pixel_pitch = pitch>>2; 

	const bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1; 
	spu_idisable();

	// as we will now write to the blendBuffer, we need to make sure the old contents are written
	// the DMA tags for storing start with 8, thus the 256
	const register uint32_t mask=(256<<line0_index)|(256<<line1_index)|(256<<line2_index)|(256<<line3_index);
	_edgePostMlaaDmaWaitShouldNotStall(mask);

	if( interrupts_enabled ) 
	{
		spu_ienable();
	}

#if EXTENDED_BUFFER_TRACKING
	EDGE_ASSERT(LINE_STATE_MARKED == _edgePostMlaaGetParamStatus(line0_index));
	EDGE_ASSERT(LINE_STATE_MARKED == _edgePostMlaaGetParamStatus(line1_index));
	EDGE_ASSERT(LINE_STATE_MARKED == _edgePostMlaaGetParamStatus(line2_index));
	EDGE_ASSERT(LINE_STATE_MARKED == _edgePostMlaaGetParamStatus(line3_index));

	EDGE_ASSERT(BLEND_STATE_STORED == _edgePostMlaaGetBlendStatus(line0_index));
	EDGE_ASSERT(BLEND_STATE_STORED == _edgePostMlaaGetBlendStatus(line1_index));
	EDGE_ASSERT(BLEND_STATE_STORED == _edgePostMlaaGetBlendStatus(line2_index));
	EDGE_ASSERT(BLEND_STATE_STORED == _edgePostMlaaGetBlendStatus(line3_index));

	_edgePostMlaaSetParamStatus(line0_index, LINE_STATE_SEPLINES);
	_edgePostMlaaSetParamStatus(line1_index, LINE_STATE_SEPLINES);
	_edgePostMlaaSetParamStatus(line2_index, LINE_STATE_SEPLINES);
	_edgePostMlaaSetParamStatus(line3_index, LINE_STATE_SEPLINES);

	_edgePostMlaaSetBlendStatus(line0_index, BLEND_STATE_COPIED);
	_edgePostMlaaSetBlendStatus(line1_index, BLEND_STATE_COPIED);
	_edgePostMlaaSetBlendStatus(line2_index, BLEND_STATE_COPIED);
	_edgePostMlaaSetBlendStatus(line3_index, BLEND_STATE_COPIED);
#endif

#if ENABLE_DEBUG_FEATURES && (DEBUG_MODE == SHOW_EDGES )
	if(0!=(g_Debug&EDGE_POST_MLAA_MODE_SHOW_EDGES))
	{
		_edgePostMlaaColorEdges(g_pMem->ParameterBuffer[line0_index],g_pMem->BlendBuffer[line0_index],pixel_pitch);
		_edgePostMlaaColorEdges(g_pMem->ParameterBuffer[line1_index],g_pMem->BlendBuffer[line1_index],pixel_pitch);
		_edgePostMlaaColorEdges(g_pMem->ParameterBuffer[line2_index],g_pMem->BlendBuffer[line2_index],pixel_pitch);
		_edgePostMlaaColorEdges(g_pMem->ParameterBuffer[line3_index],g_pMem->BlendBuffer[line3_index],pixel_pitch);
		return;
	}
#endif

	const register uint32_t flags = (first)?HORIZONTAL_TAG:VERTICAL_TAG;

	g_pMem->SepLineBufferEnds = edgePostMlaaFindSeparationLines(
		g_pMem->ParameterBuffer[line0_index],
		g_pMem->ParameterBuffer[line1_index],
		g_pMem->ParameterBuffer[line2_index],
		g_pMem->ParameterBuffer[line3_index],
		g_pMem->SepLineStartAndLength[line0_index&3],
		g_pMem->SepLineStartAndLength[line1_index&3],
		g_pMem->SepLineStartAndLength[line2_index&3],
		g_pMem->SepLineStartAndLength[line3_index&3],
		g_pMem->BlendBuffer[line0_index], //copy was moved here (for free!)
		g_pMem->BlendBuffer[line1_index],
		g_pMem->BlendBuffer[line2_index],
		g_pMem->BlendBuffer[line3_index],
		pixel_pitch,
		flags);
}


__attribute__((always_inline)) static inline void _edgePostMlaaCompressSepLines(
	const register uint32_t line0_index,
	const register uint32_t pass)
{
	(void)pass;
	const register uint32_t line1_index = (line0_index+1)&BUFFER_LINE_MASK;
	const register uint32_t line2_index = (line0_index+2)&BUFFER_LINE_MASK;
	const register uint32_t line3_index = (line0_index+3)&BUFFER_LINE_MASK;
	const register uint32_t line4_index = (line0_index+4)&BUFFER_LINE_MASK;

#if EXTENDED_BUFFER_TRACKING
	EDGE_ASSERT(BLEND_STATE_STORED  > _edgePostMlaaGetBlendStatus(line0_index));
	EDGE_ASSERT(BLEND_STATE_STORED  > _edgePostMlaaGetBlendStatus(line1_index));
	EDGE_ASSERT(BLEND_STATE_STORED  > _edgePostMlaaGetBlendStatus(line2_index));
	EDGE_ASSERT(BLEND_STATE_STORED  > _edgePostMlaaGetBlendStatus(line3_index));
	EDGE_ASSERT(LINE_STATE_SEPLINES > _edgePostMlaaGetParamStatus(line4_index));
#endif

	{
		const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();
		
		_edgePostMlaaDmaWaitShouldNotStall(1<<line4_index);

		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}
#if ENABLE_DEBUG_FEATURES && (DEBUG_MODE == SHOW_EDGES )
	if(0!=(g_Debug&EDGE_POST_MLAA_MODE_SHOW_EDGES))
	{
		g_pMem->ValidPixels[line0_index] = 0;
		g_pMem->ValidPixels[line1_index] = 0;
		g_pMem->ValidPixels[line2_index] = 0;
		g_pMem->ValidPixels[line3_index] = 0;
		return;
	}
#endif

	edgePostMlaaCompressSeparationLines(
		g_pMem->BlendBuffer[line0_index],
		g_pMem->BlendBuffer[line1_index],
		g_pMem->BlendBuffer[line2_index],
		g_pMem->BlendBuffer[line3_index],
		g_pMem->ParameterBuffer[line4_index],
		g_pMem->SepLineStartAndLength[line0_index&3],
		g_pMem->SepLineStartAndLength[line1_index&3],
		g_pMem->SepLineStartAndLength[line2_index&3],
		g_pMem->SepLineStartAndLength[line3_index&3],
		&g_pMem->ValidPixels[line0_index],
		&g_pMem->ValidPixels[line1_index],
		&g_pMem->ValidPixels[line2_index],
		&g_pMem->ValidPixels[line3_index],
#if ENABLE_DEBUG_FEATURES && (DEBUG_MODE == SHOW_SEPARATION_LINES)
		0,	// if we removed the single pixel features, we would be unable to see them.
#else
		1,	// blend single pixel features directly.
#endif
		g_pMem->SepLineBufferEnds);

#if ENABLE_DEBUG_FEATURES && (DEBUG_MODE == SHOW_SEPARATION_LINES )
	if(0!=(g_Debug&EDGE_POST_MLAA_MODE_SHOW_EDGES))
	{
		_edgePostMlaaMarkSepLines(line0_index,pass);
		_edgePostMlaaMarkSepLines(line1_index,pass);
		_edgePostMlaaMarkSepLines(line2_index,pass);
		_edgePostMlaaMarkSepLines(line3_index,pass);
		return;
	}
#endif
}

static void _edgePostMlaaMarkDiscontinuities(
	const register uint32_t line0_index,
	const register uint32_t pitch,
	const register bool     first,
	const register uint16_t threshold0,
	const register uint16_t threshold1)
{

	const register uint32_t line1_index = (line0_index+1)&BUFFER_LINE_MASK;
	const register uint32_t line2_index = (line0_index+2)&BUFFER_LINE_MASK;
	const register uint32_t line3_index = (line0_index+3)&BUFFER_LINE_MASK;
	const register uint32_t line4_index = (line0_index+4)&BUFFER_LINE_MASK;

	const register uint32_t pixel_pitch = pitch>>2; 

	const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	if(first)
	{		
		{
			_edgePostMlaaDmaWaitShouldNotStall(1<<line0_index);
			if(LINE_STATE_THRESHOLDED > _edgePostMlaaGetParamStatus(line0_index)){
				_edgePostMlaaSetParamStatus(line0_index,LINE_STATE_THRESHOLDED);
				edgePostMlaaWriteThreshold(g_pMem->ParameterBuffer[line0_index],pixel_pitch,threshold1,threshold0);
			}
			_edgePostMlaaDmaWaitShouldNotStall(1<<line1_index);
			if(LINE_STATE_THRESHOLDED > _edgePostMlaaGetParamStatus(line1_index)){
				_edgePostMlaaSetParamStatus(line1_index,LINE_STATE_THRESHOLDED);
				edgePostMlaaWriteThreshold(g_pMem->ParameterBuffer[line1_index],pixel_pitch,threshold1,threshold0);
			}
			_edgePostMlaaDmaWaitShouldNotStall(1<<line2_index);
			if(LINE_STATE_THRESHOLDED > _edgePostMlaaGetParamStatus(line2_index)){
				_edgePostMlaaSetParamStatus(line2_index,LINE_STATE_THRESHOLDED);
				edgePostMlaaWriteThreshold(g_pMem->ParameterBuffer[line2_index],pixel_pitch,threshold1,threshold0);
			}
			_edgePostMlaaDmaWaitShouldNotStall(1<<line3_index);
			if(LINE_STATE_THRESHOLDED > _edgePostMlaaGetParamStatus(line3_index)){
				_edgePostMlaaSetParamStatus(line3_index,LINE_STATE_THRESHOLDED);
				edgePostMlaaWriteThreshold(g_pMem->ParameterBuffer[line3_index],pixel_pitch,threshold1,threshold0);
			}
			_edgePostMlaaDmaWaitShouldNotStall(1<<line4_index);
			if(LINE_STATE_THRESHOLDED > _edgePostMlaaGetParamStatus(line4_index)){
				_edgePostMlaaSetParamStatus(line4_index,LINE_STATE_THRESHOLDED);
				edgePostMlaaWriteThreshold(g_pMem->ParameterBuffer[line4_index],pixel_pitch,threshold1,threshold0);
			}
		}

		{
			edgePostMlaaMarkDiscontinuities(
				g_pMem->ParameterBuffer[line0_index],
				g_pMem->ParameterBuffer[line1_index],
				g_pMem->ParameterBuffer[line2_index],
				g_pMem->ParameterBuffer[line3_index],
				g_pMem->ParameterBuffer[line4_index],
				pixel_pitch);
		}

#if EXTENDED_BUFFER_TRACKING
		_edgePostMlaaSetParamStatus(line0_index, LINE_STATE_MARKED);
		_edgePostMlaaSetParamStatus(line1_index, LINE_STATE_MARKED);
		_edgePostMlaaSetParamStatus(line2_index, LINE_STATE_MARKED);
		_edgePostMlaaSetParamStatus(line3_index, LINE_STATE_MARKED);
#endif

	}
	else
	{
		const register uint32_t mask=(1<<line1_index)|(1<<line2_index)|(1<<line3_index)|(1<<line4_index);
		_edgePostMlaaDmaWaitShouldNotStall(mask);
	}

	if( interrupts_enabled )
	{
		spu_ienable();
	}
}


static /*inline*/ void _edgePostMlaaProcessLine(
	const register uint32_t line0_index,
	const register bool     first,
	const register uint32_t width)
{

	//first we have to make sure all transfers to the interesting buffers are done
	const register uint32_t line1_index=(line0_index+1)&BUFFER_LINE_MASK;
	const register uint32_t line2_index=(line0_index+2)&BUFFER_LINE_MASK;
	const register uint32_t line3_index=(line0_index+3)&BUFFER_LINE_MASK;

	g_pMem->Jobs = 0;

#if EXTENDED_BUFFER_TRACKING
	EDGE_ASSERT(LINE_STATE_MARKED <= _edgePostMlaaGetParamStatus(line0_index));
	EDGE_ASSERT(LINE_STATE_SEPLINES == _edgePostMlaaGetParamStatus(line1_index));
	EDGE_ASSERT(LINE_STATE_MARKED <= _edgePostMlaaGetParamStatus(line2_index));
	EDGE_ASSERT(LINE_STATE_MARKED <= _edgePostMlaaGetParamStatus(line3_index));
#endif

	_edgePostMlaaSetParamStatus(line1_index, LINE_STATE_PROCESSED);

	if(0==(0x00ffffff&g_pMem->ValidPixels[line1_index])) return; //first length, mask out first byte which is used differently.

	const register uint32_t jobs=_edgePostMlaaComputeBounds(
		g_pMem->ParameterBuffer[line0_index],
		g_pMem->ParameterBuffer[line1_index],
		g_pMem->ParameterBuffer[line2_index],
		g_pMem->ParameterBuffer[line3_index],
		g_pMem->SepLineStartAndLength[line1_index&3],
		g_pMem->BlendJobs,
		first,
		g_pMem->ValidPixels[line1_index],
		width);	

	EDGE_ASSERT(jobs<=EDGE_MLAA_SEPLINEQWORDS);
	EDGE_ASSERT(jobs>0);

	g_pMem->Jobs = jobs;
}


#define A	0,1,2,3
#define B	4,5,6,7
#define C	8,9,10,11
#define D	12,13,14,15
#define a	16,17,18,19
#define b	20,21,22,23
#define c	24,25,26,27
#define d	28,29,30,31
#define O   128,128,128,128

static const vec_uchar16 unalignedShuffles[] =
{
	{A, B, C, D},
	{B, C, D, a},
	{C, D, a, b},
	{D, a, b, c},
	{a, b, c, d},
	{A, a, b, c},
	{A, B, a, b},
	{A, B, C, a},
	{A, B, C, D},
	{d, B, C, D},
	{c, d, C, D},
	{b, c, d, D},
};


static void _edgePostMlaaBlendJobs(
   const register uint32_t line1,
   const register uint32_t line2)
{	
	const register uint32_t line1_index=line1&BUFFER_LINE_MASK;
	const register uint32_t line2_index=line2&BUFFER_LINE_MASK;

#if EXTENDED_BUFFER_TRACKING
	EDGE_ASSERT((-1 == g_pMem->Jobs) || (LINE_STATE_PROCESSED == _edgePostMlaaGetParamStatus(line1_index)));
	EDGE_ASSERT((-1 == g_pMem->Jobs) || (BLEND_STATE_COPIED == _edgePostMlaaGetBlendStatus(line1_index)));
	EDGE_ASSERT((-1 == g_pMem->Jobs) || (BLEND_STATE_COPIED == _edgePostMlaaGetBlendStatus(line2_index)));
#endif

	_edgePostMlaaSetBlendStatus(line1_index, (-1 == g_pMem->Jobs)?BLEND_STATE_STORED:BLEND_STATE_BLENDED);

#if ENABLE_DEBUG_FEATURES
	if(0!=(g_Debug&EDGE_POST_MLAA_MODE_SHOW_EDGES)) return;
#endif

	if(__builtin_expect(0 >= g_pMem->Jobs,0)) return;

	register vec_uint4 zu_jobs;
	zu_jobs=edgePostMlaaFilterBlendJobs(
		g_pMem->BlendJobs,
		g_pMem->Jobs,
		g_pMem->BlendBuffer[line1_index],
		g_pMem->BlendBuffer[line2_index],
		2*EDGE_MLAA_SEPLINEQWORDS); //LINE_WIDTH only defines offsets in memory, not performance

	const register uint32_t z_jobs=zu_jobs[0];
	const register uint32_t u_jobs=zu_jobs[1];

	if(z_jobs)
	{
		edgePostMlaaBlendZ(&g_pMem->BlendJobs[(2*EDGE_MLAA_SEPLINEQWORDS-1)*2],z_jobs,unalignedShuffles);
	}

	if(u_jobs)
	{
		edgePostMlaaBlendU(&g_pMem->BlendJobs[(2*EDGE_MLAA_SEPLINEQWORDS-1)*2+1],u_jobs,unalignedShuffles);
	}
}

static void _edgePostMlaaCopyFromScratchBuffer(
	const register uint32_t slot,
	const register  int32_t line,
	const register uint32_t height,
	const register uint32_t pass)
{
	const register uint32_t line_index = line & BUFFER_LINE_MASK;
	register uint32_t* const old_ptr = g_pMem->ParameterBuffer[line_index];

	g_pMem->ParameterBuffer[line_index] = g_pMem->ScratchBufferMapping[slot];
	g_pMem->ScratchBufferMapping[slot] = old_ptr;

	// first check that the transfer is complete.

	{
		const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
		spu_idisable();

		_edgePostMlaaDmaWaitShouldNotStall(1<<(slot+2*BUFFER_LINES));

		if( interrupts_enabled ) 
		{
			spu_ienable();
		}
	}

	register uint32_t fetched_line_status = LINE_STATE_MARKED;
	if(1==pass)
	{
		fetched_line_status =((line < (int32_t)height) && (line >= 0))?LINE_STATE_THRESHOLDED:LINE_STATE_LOADED;
	}

	_edgePostMlaaSetParamStatus(line_index, fetched_line_status);
}

#define FETCH_LINE(number) fetchFunction(imageEa,number,pitch,width,height,transposedPitch,transposedBlockOffset,pass,-1)
#define STORE_LINE(number) _edgePostMlaaStoreLine(destEa,number,pitch,width,transposedPitch,transposedBlockOffset,pass,first)
#define PROCESS_LINE(a,b,c,d) EDGE_ASSERT(b<height); _edgePostMlaaProcessLine((a)&BUFFER_LINE_MASK,first,width)
#define MARK_LINES(a,b,c,d,e) _edgePostMlaaMarkDiscontinuities((a)&BUFFER_LINE_MASK,width,first,threshold0,threshold1)
#define BLEND_JOBS(a,b) _edgePostMlaaBlendJobs((a)&BUFFER_LINE_MASK,(b)&BUFFER_LINE_MASK)
#define FIND_SEP(a,b,c,d) _edgePostMlaaFindSeparationLines((a)&BUFFER_LINE_MASK,width,first)
#define COMPRESS_SEP(a,b,c,d,e) _edgePostMlaaCompressSepLines((a)&BUFFER_LINE_MASK,pass)
#define FETCH_SCRATCH(slot,number) fetchFunction(imageEa,number,pitch,width,height,transposedPitch,transposedBlockOffset,first,slot)
#define COPY_SCRATCH(slot,number) _edgePostMlaaCopyFromScratchBuffer(slot,number,height,pass);

/*
 * Erroneous over- and underfetch is controlled by the MAX_UNDERFETCH and MAX_OVERFETCH constants,
 * and the checking for this is transparent to the loop. This allows this to look a lot simpler than it
 * actually is. 
 */

static void _edgePostMlaaDmaLoop(
	      register uint32_t imageEa,
	      register uint32_t destEa,
	const register uint32_t width,
	const register  int32_t height,
	const register uint32_t pitch,
	const register uint32_t transposedPitch,
	const register uint32_t transposedBlockOffset,
	const register uint32_t pass,
	const register uint16_t threshold0,
	const register uint16_t threshold1,
	const register uint32_t spuId,
	const register bool lastSpu,
	const register bool first,
	const register uint32_t	baseDmaTag)
{
	EdgeMlaaFetchFunction fetchFunction = (0==pass)?_edgePostMlaaFetchLineLinear:_edgePostMlaaFetchLineTransposed;	

#if ENABLE_DEBUG_FEATURES && (DEBUG_MODE == SHOW_EDGES )
	if(0!=(g_Debug&EDGE_POST_MLAA_MODE_SHOW_EDGES) && !first)
	{
		return;
	}
#endif

	// these are used by the FetchLine/StoreLine functions/
	g_pMem->MinFetch = (0==spuId)?0:-MAX_UNDERFETCH;
	g_pMem->MaxFetch = (lastSpu)?height:(height+MAX_OVERFETCH);
	g_pMem->BaseDmaTag = baseDmaTag;

	register vec_uchar16 paramInit = {
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		LINE_STATE_PROCESSED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED,
		BLEND_STATE_STORED};

	*(vec_uchar16*)g_pMem->ParamStatus = paramInit;

	for(uint32_t i = 0; i< BUFFER_LINES; ++i)
	{
		g_pMem->ParameterBuffer[i] = g_pMem->ParameterBufferPool[i];
	}
	g_pMem->ScratchBufferMapping[0] = g_pMem->ParameterBufferPool[BUFFER_LINES];
	g_pMem->ScratchBufferMapping[1] = g_pMem->ParameterBufferPool[BUFFER_LINES+1];


	register int32_t line = (0==spuId)?0:-1;	// inner SPUs overfetch to smooth the transitions.

	FETCH_LINE(line + 0);
	FETCH_LINE(line + 1);
	FETCH_LINE(line + 2);
	FETCH_LINE(line + 3);
	FETCH_LINE(line + 4);

	_edgePostMlaaDmaWaitMayStall(1<<(line&BUFFER_LINE_MASK));
	MARK_LINES(line + 0,line + 1,line + 2,line + 3,line + 4);


	_edgePostMlaaAlignedMemcpy(g_pMem->ParameterBuffer[(line-1)&BUFFER_LINE_MASK],g_pMem->ParameterBuffer[line&BUFFER_LINE_MASK],width);
	_edgePostMlaaSetParamStatus((line-1)&BUFFER_LINE_MASK,LINE_STATE_PROCESSED);


	FETCH_LINE(line + 5);
	FETCH_LINE(line + 6);

	//prevent the first blend from being processed (-1)
	g_pMem->Jobs = -1;

	for(; line < height-4; line+=4)
	{
		FETCH_SCRATCH(0,line+7);
		FETCH_SCRATCH(1,line+8);

		FIND_SEP(line,line+1,line+2,line+3); // waits for previous stores
		BLEND_JOBS(line-1,line);


		if(1 == pass) _edgePostMlaaDmaBarrier();
		STORE_LINE(line-4);
		STORE_LINE(line-3);
		STORE_LINE(line-2);
		STORE_LINE(line-1);
		_edgePostMlaaDmaBarrier();

		COMPRESS_SEP(line,line+1,line+2,line+3,line+4);
		
		PROCESS_LINE(line-1,line,line+1,line+2);
		BLEND_JOBS(line,line+1);


		PROCESS_LINE(line,line+1,line+2,line+3);
		BLEND_JOBS(line+1,line+2);

		COPY_SCRATCH(0,line+7); // waits for previous scratch fetches
		COPY_SCRATCH(1,line+8);
		MARK_LINES(line+4,line+5,line+6,line+7,line+8); //waits for previous direct fetches
		PROCESS_LINE(line+1,line+2,line+3,line+4);
		BLEND_JOBS(line+2,line+3);
		PROCESS_LINE(line+2,line+3,line+4,line+5);
		//The blend of line+3,line+4 needs to wait until after line+4 was copied.
		FETCH_LINE(line+9);
		FETCH_LINE(line+10);
	}

	for(; line < height; line+=4)
	{
		FETCH_SCRATCH(0,line+7); 
		FETCH_SCRATCH(1,line+8);

		FIND_SEP(line,line+1,line+2,line+3);
		BLEND_JOBS(line-1,line);


		if(1 == pass) _edgePostMlaaDmaBarrier();

		STORE_LINE(line-4);
		STORE_LINE(line-3);
		STORE_LINE(line-2);
		STORE_LINE(line-1);
		_edgePostMlaaDmaBarrier();

		COMPRESS_SEP(line,line+1,line+2,line+3,line+4);

		PROCESS_LINE(line-1,line,line+1,line+2);
		BLEND_JOBS(line,line+1);


		if((line+1)<height)
		{
			PROCESS_LINE(line,line+1,line+2,line+3);
			BLEND_JOBS(line+1,line+2);
		}

		if((line+2)<height)
		{
			COPY_SCRATCH(0,line+7);
			COPY_SCRATCH(1,line+8);
			MARK_LINES(line+4,line+5,line+6,line+7,line+8);
			PROCESS_LINE(line+1,line+2,line+3,line+4);
			FETCH_LINE(line+9);
			BLEND_JOBS(line+2,line+3);

		}
		if((line+3)<height) 
		{
			PROCESS_LINE(line+2,line+3,line+4,line+5);
		}
		//The blend of line+3,line+4 needs to wait until after line+4 was copied.
		FETCH_LINE(line+10);
	}

	if(line-4 < height) {STORE_LINE(line-4);}
	if(line-3 < height) {STORE_LINE(line-3);}
	if(line-2 < height) {STORE_LINE(line-2);}
	if(line-1 < height) 
	{
		BLEND_JOBS(line-1,line);
		STORE_LINE(line-1);
	}

}

// used to compute width of transposed image
static inline uint32_t _edgePostMlaaAlign( const uint32_t value, uint32_t alignment)
{
	return ( value + (alignment - 1)) & ~(alignment - 1);
}


void edgePostMlaaPass(EdgePostMlaaMemoryLayout* layout, const EdgePostMlaaTaskParameters* parameter, uint32_t pass, uint32_t first, uint32_t baseDmaTag)
{
	EDGE_ASSERT(baseDmaTag<15); //make sure we have enough tags available.
	g_pMem = layout;
	g_Debug = parameter->mode;
	g_pMem->TransposeBlockSize = (parameter->mode&EDGE_POST_MLAA_MODE_TRANSPOSE_64)?64:128;

	const register bool last_spu = parameter->spuId==parameter->spuCount-1;
	if(0==pass)
	{
		//lines are actually line...
		const register uint32_t lines_per_spu = parameter->imageHeight / parameter->spuCount;
		const register uint32_t lines_for_last_spu = 
			parameter->imageHeight - (parameter->spuCount - 1) * lines_per_spu;

		const register uint32_t address_offset = parameter->spuId * lines_per_spu * parameter->imagePitch;

		_edgePostMlaaDmaLoop(
			parameter->imageAddress+address_offset,
			parameter->destAddress+address_offset,
			parameter->imageWidth*4,
			last_spu?lines_for_last_spu:lines_per_spu,
			parameter->imagePitch,
			0, //this is only needed for working on transposed buffers.
			0, //this is only needed for working on transposed buffers.
			pass,
			parameter->parameter0,
			parameter->parameter1,
			parameter->spuId,
			last_spu,
			first,
			baseDmaTag);
	}
	else
	{
		//here things are a bit different, as we're on a tiled image

		const register uint32_t lines_per_spu = parameter->imageWidth / parameter->spuCount;
		const register uint32_t lines_for_last_spu = 
			parameter->imageWidth - (parameter->spuCount - 1) * lines_per_spu;

		// The line this SPU starts at. This is not very useful by itself, as we need a transpose block boundary.
		const register uint32_t base_address_offset = parameter->spuId * lines_per_spu;
		// Get the distance to the previous block boundary
		const register uint32_t block_offset = base_address_offset % g_pMem->TransposeBlockSize;
		// ...and the block boundary itself
		const register uint32_t address_offset = sizeof(uint32_t) * (base_address_offset - block_offset);

		const register uint32_t transpose_height = _edgePostMlaaAlign(parameter->imageHeight,g_pMem->TransposeBlockSize);

		_edgePostMlaaDmaLoop(
			parameter->destAddress+address_offset, // offset to the block boundary
			parameter->destAddress+address_offset,
			parameter->imageHeight*4,
			last_spu?lines_for_last_spu:lines_per_spu,
			transpose_height*4,
			parameter->imagePitch, //this is only needed for working on transposed buffers.
			block_offset,  // pass the offset from the block to the actual line
			pass,
			parameter->parameter0,
			parameter->parameter1,
			parameter->spuId,
			last_spu,
			first,
			baseDmaTag);

	}
}

// we use this struct to pass the parameters to the the writeThreshold operation that is performed in the first transpose.
struct _EdgeMlaaTransposeThresholdingParameter
{
	uint32_t factor;
	uint32_t base;
};


static void _edgePostMlaaTransposeThresholdingFunction(void *param, void *data, size_t dataSize)
{
	_EdgeMlaaTransposeThresholdingParameter* typedParam = (_EdgeMlaaTransposeThresholdingParameter*)param;
	edgePostMlaaWriteThreshold((uint32_t*)data, dataSize/sizeof(uint32_t), typedParam->factor, typedParam->base);
}

void edgePostTransposePass(EdgePostInplaceTranposeMemoryLayout* layout, const EdgePostMlaaTaskParameters* parameter, uint32_t first, uint32_t maxSpus, uint32_t baseDmaTag)
{
	EDGE_ASSERT(maxSpus < 3); // it makes no sense to run this on more than two SPUs really.
	g_Debug = parameter->mode;

	const uint32_t block_size = (parameter->mode&EDGE_POST_MLAA_MODE_TRANSPOSE_64)?64:128;

	const uint32_t transpose_height = _edgePostMlaaAlign(parameter->imageHeight,block_size);
	const uint32_t lines_per_spu=transpose_height/2; //we only use two SPUs.

	_EdgeMlaaTransposeThresholdingParameter thresholdingParam;
	thresholdingParam.base = parameter->parameter0;
	thresholdingParam.factor = parameter->parameter1;	

	if(parameter->spuId<maxSpus)
	{
		edgePostTransposeInPlace(
			layout,
			parameter->imageAddress,
			parameter->destAddress,
			parameter->imagePitch,			
			lines_per_spu,
			block_size,
			parameter->spuId,
			parameter->directionLockAddress,
			first?_edgePostMlaaTransposeThresholdingFunction:NULL,
			&thresholdingParam,
			baseDmaTag);

		//if we only have one SPU, we need to do the lower parts as well.
		if((1==maxSpus)||(1==parameter->spuCount))
		{
			edgePostTransposeInPlace(
				layout,
				parameter->imageAddress,
				parameter->destAddress,
				parameter->imagePitch,
				lines_per_spu,
				block_size,
				1,
				parameter->directionLockAddress,
				first?_edgePostMlaaTransposeThresholdingFunction:NULL,
				&thresholdingParam,
				baseDmaTag);
		}
	}
}
