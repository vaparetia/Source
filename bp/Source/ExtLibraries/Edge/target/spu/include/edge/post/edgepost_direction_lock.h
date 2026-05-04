/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef EDGEPOST_DIRECTION_LOCK_H
#define EDGEPOST_DIRECTION_LOCK_H

/*
 * If an SPU operation is DMA bound, interleaving reads and loads is not necessarily optimal.
 * XDR memory is split into 16 banks and 2 ports, interleaved at 128 byte boundaries. While 
 * load balancing between banks and ports is important, it is not the only factor affecting
 * bandwidth. Each port has a transfer direction, either reading or writing. Switching between
 * these two directions is not free and - if done excessively - will degrade performance.
 *
 * This primitive is intended to be used by SPU operations executing on multiple SPUs, which
 * quickly alternate between reading and writing on the same XDR port(s).
 *
 * The Set* functions are used to request a transfer direction for the lock. If they are called
 * with the 'block' parameter set, they will busy-wait until the desired direction can be set.
 * Once the SPU does not want to control the direction anymore, the edgeDirectionLockRelease
 * function should be called, so that direction changes can be performed.
 *
 * On the PPU:
 *  The direction lock is a single 128 byte aligned 32bit memory location in XDR. 
 *  As it is accessed through atomic operations, it is recommended that a single cache-line is 
 *  exclusively reserved for the lock, to prevent false sharing.
 *
 * On the SPU:
 *  Each SPU maintains its own EdgeDirectionLock, which contains both a pointer to the
 *  actual XDR counter, as well as the current direction, as seen from this SPU.
 *  As we use atomic accesses, a 128 byte aligned 128 byte buffer is needed as well in Local 
 *  Store. This temporary buffer can safely be clobbered between calls and can thus be reused,
 *  e.g., for other atomic accesses.
 *
 * Impact on PPU and other SPUs:
 * As using this primitive reduces the overall load on the memory system, it will not 
 * negatively impact the performance of other threads. On the other hand, non-syncronized
 * threads will still cause a certain amount of contention.
 *
 * WARNING:
 * This primitive allows the optimization of a very specific case, i.e. trying to saturate
 * XDR bandwidth with concurrent reads and writes. As such, it can be very beneficial in 
 * situations where port contention is the problem, but it is by no means a general
 * method to sync DMAs. Indeed, syncing DMAs to minimize port contention will negatively
 * impact performance in the vast majority of cases, because it effectively disables the 
 * MFC queues.
 */


#ifdef __SPU__

#include <stdint.h>
typedef struct EdgeDirectionLock
{
	uint64_t ea;				// 128 byte aligned XDR memory
	int32_t  currentDirection;	// -1 is reading +1 is writing 0 is inactive
	volatile int32_t *buffer;	// 128 byte of 128byte aligned LS memory
} EdgeDirectionLock __attribute__((aligned(16)));

#ifdef __cplusplus
extern "C" {
#endif

void _edgeDirectionLockInitialize(EdgeDirectionLock* lock, uint64_t ea, volatile int32_t* atomic_buffer);
int _edgeDirectionLockSetToWrite(EdgeDirectionLock *lock, const int block);
int _edgeDirectionLockSetToRead(EdgeDirectionLock *lock, const int block);
int _edgeDirectionLockRelease(EdgeDirectionLock *lock, const int block);

#ifdef __cplusplus
}
#endif //__cplusplus

#else

#ifdef __cplusplus
extern "C" {
#endif

void _edgeDirectionLockInitialize(volatile int32_t* buffer)

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

#endif //EDGEPOST_DIRECTION_LOCK_H
