/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ATOMIC_H__
#define __EDGE_ATOMIC_H__

//////////////////////////////////////////////////////////////////////////

#if defined(__SPU__)

	#include <cell/atomic.h>
	
	#define EDGE_ATOMIC_LOCK_LINE_64( ls, ea )						cellAtomicLockLine64( ls, ea )
	#define EDGE_ATOMIC_STORE_CONDITIONAL_64( ls, ea, value )		cellAtomicStoreConditional64( ls, ea, value )
	#define EDGE_ATOMIC_OR_32( ls, ea, value )						cellAtomicOr32( ls, ea, value )
	#define EDGE_ATOMIC_INCR_32( ls, ea )							cellAtomicIncr32( ls, ea )
	#define EDGE_ATOMIC_DECR_32( ls, ea )							cellAtomicDecr32( ls, ea )
	#define EDGE_ATOMIC_ADD_32( ls, ea, value )						cellAtomicAdd32( ls, ea, value )
	#define EDGE_ATOMIC_SUB_32( ls, ea, value )						cellAtomicSub32( ls, ea, value )

#else

	#error EDGE_ATOMIC functions not defined

#endif

//////////////////////////////////////////////////////////////////////////

#endif	//__EDGE_ATOMIC_H__
