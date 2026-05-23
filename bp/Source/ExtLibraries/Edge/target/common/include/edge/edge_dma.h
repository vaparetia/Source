/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_DMA_H__
#define __EDGE_DMA_H__

//////////////////////////////////////////////////////////////////////////

#if defined(__SPU__)

	#include <cell/dma.h>
	#define EDGE_DMA_GET( ls, ea, size, tag, tid, rid )					cellDmaGet( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_GETB( ls, ea, size, tag, tid, rid )				cellDmaGetb( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_GETF( ls, ea, size, tag, tid, rid )				cellDmaGetf( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_LARGE_GET( ls, ea, size, tag, tid, rid )			cellDmaLargeGet( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_SMALL_GET( ls, ea, size, tag, tid, rid )			cellDmaSmallGet( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_SMALL_GETF( ls, ea, size, tag, tid, rid )			cellDmaSmallGetf( (ls), (ea), (size), (tag), (tid), (rid) )
    #define EDGE_DMA_LIST_GET( ls, ea, list, listSize, tag, tid, rid )	cellDmaListGet( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
    #define EDGE_DMA_LIST_GETB( ls, ea, list, listSize, tag, tid, rid )	cellDmaListGetb( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
	#define EDGE_DMA_LIST_GETF( ls, ea, list, listSize, tag, tid, rid )	cellDmaListGetf( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
    #define EDGE_DMA_GETLLAR( ls, ea, tid, rid )                        cellDmaGetllar( (ls), (ea), (tid), (rid) )

	#define EDGE_DMA_PUT( ls, ea, size, tag, tid, rid )					cellDmaPut( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_PUTB( ls, ea, size, tag, tid, rid )				cellDmaPutb( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_PUTF( ls, ea, size, tag, tid, rid )				cellDmaPutf( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_LARGE_PUT( ls, ea, size, tag, tid, rid )			cellDmaLargePut( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_SMALL_PUT( ls, ea, size, tag, tid, rid )			cellDmaSmallPut( (ls), (ea), (size), (tag), (tid), (rid) )
	#define EDGE_DMA_SMALL_PUTF( ls, ea, size, tag, tid, rid )			cellDmaSmallPutf( (ls), (ea), (size), (tag), (tid), (rid) )
    #define EDGE_DMA_LIST_PUT( ls, ea, list, listSize, tag, tid, rid )	cellDmaListPut( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
	#define EDGE_DMA_LIST_PUTB( ls, ea, list, listSize, tag, tid, rid )	cellDmaListPutb( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
	#define EDGE_DMA_LIST_PUTF( ls, ea, list, listSize, tag, tid, rid )	cellDmaListPutf( (ls), (ea), (list), (listSize), (tag), (tid), (rid) )
    #define EDGE_DMA_PUTLLC( ls, ea, tid, rid )                         cellDmaPutllc( (ls), (ea), (tid), (rid) )
    #define EDGE_DMA_PUTLLUC( ls, ea, tid, rid )                        cellDmaPutlluc( (ls), (ea), (tid), (rid) )

	#define EDGE_DMA_PUT_UINT32( value, ea, tag, tid, rid)				cellDmaPutUint32( (value), (ea), (tag), (tid), (rid) )

    #define EDGE_DMA_WAIT_ATOMIC_STATUS( )                              cellDmaWaitAtomicStatus( )
    #define EDGE_DMA_WAIT_TAG_STATUS_ALL( mask )						cellDmaWaitTagStatusAll( (mask) )

	typedef CellDmaListElement EdgeDmaListElement;

#else

	#error EDGE_DMA functions not defined

#endif

//////////////////////////////////////////////////////////////////////////

#endif	//__EDGE_DMA_H__
