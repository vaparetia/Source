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
#include "edgepost_internals.h"
#include "edge/edge_dma.h"


// Stall for DMA completion
void _edgePostDmaStall( int tag )
{
	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	// stall for DMA
	EDGE_DMA_WAIT_TAG_STATUS_ALL( 1 << tag );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}
}

void _edgePostDmaLargeGet( void *ls, uint64_t ea, uint32_t size, uint32_t tag )
{
	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	EDGE_DMA_LARGE_GET( ls, ea, size, tag, 0, 0 );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}
}

void _edgePostDmaGet( void *ls, uint64_t ea, uint32_t size, uint32_t tag )
{
	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	EDGE_DMA_GET( ls, ea,	size, tag, 0, 0 );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}
}

void _edgePostDmaListGet( void *ls, const EdgeDmaListElement *list, uint32_t listSize, uint32_t tag )
{
	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	EDGE_DMA_LIST_GET( ls, 0, list, listSize, tag, 0, 0 );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}
}

void _edgePostDmaListPut( const void *ls, const EdgeDmaListElement *list, uint32_t listSize, uint32_t tag )
{
	// disable interrupts
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	EDGE_DMA_LIST_PUT( ls, 0, list, listSize, tag, 0, 0 );

	// re-enable interrupts
	if (interruptsEnabled)
	{
		spu_ienable();
	}
}
