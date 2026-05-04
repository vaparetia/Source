/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2009 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifdef __SPU__

#include <sys/return_code.h>
#include <edge/post/edgepost_direction_lock.h>


#include <spu_intrinsics.h>
#include <spu_mfcio.h>

static inline int _edgeTryReleaseDirectonLock(EdgeDirectionLock *lock, const int block)
{
	if(0==lock->currentDirection)
		return 1;

	register const int interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	register int result = 0;
	register uint32_t atomic_status;

	do
	{
		mfc_getllar(lock->buffer,lock->ea,0,0);
		atomic_status = spu_readch(MFC_RdAtomicStat);

		lock->buffer[0]-=lock->currentDirection;
		mfc_putllc(lock->buffer,lock->ea,0,0);
		atomic_status=spu_readch(MFC_RdAtomicStat);
		result = (0 == atomic_status)?1:0;

	} while (__builtin_expect(block&&!result,0));

	if (__builtin_expect(interrupts_enabled,0))
	{
		spu_ienable();
	}

	lock->currentDirection=__builtin_expect(result,1)?0:lock->currentDirection; // change if successful.
	return result;
}

static inline void _edgeReleaseDirectonLockAndSet(EdgeDirectionLock *lock, const int dir)
{
	int released = (0 == lock->currentDirection)?1:0;
	int set      = (dir == lock->currentDirection)?1:0;

	register const int interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	register uint32_t atomic_status;

	while(!set)	
	{
		mfc_getllar(lock->buffer,lock->ea,0,0);
		atomic_status = spu_readch(MFC_RdAtomicStat);

		int released_this_iteration = 0;

		lock->buffer[0]-=released?0:lock->currentDirection;
		released_this_iteration = !released;

		const int set_this_iteration = 
			(0==lock->buffer[0]) | // use bit operator to safe branches.
			((dir<0)?(lock->buffer[0]<0):(lock->buffer[0]>0));

		lock->buffer[0]+=set_this_iteration?dir:0;

		mfc_putllc(lock->buffer,lock->ea,0,0);
		atomic_status=spu_readch(MFC_RdAtomicStat);
		released |= released_this_iteration&!atomic_status ;
		set |= set_this_iteration&!atomic_status;
	}

	if (__builtin_expect(interrupts_enabled,0))
	{
		spu_ienable();
	}

	lock->currentDirection=dir; // change if successful.
}


static inline int _edgeTryUpdateDirectionLock(EdgeDirectionLock *lock, const int block, const int dir)
{
	if(dir==lock->currentDirection)
		return 1;

	register const int interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	register int result = 0;
	register uint32_t atomic_status;

	do
	{
		mfc_getllar(lock->buffer,lock->ea,0,0);
		atomic_status = spu_readch(MFC_RdAtomicStat);

		if(
			(0==lock->buffer[0]) || 
			((dir<0)?(lock->buffer[0]<0):(lock->buffer[0]>0))
			)
		{
			lock->buffer[0]+=dir;
			mfc_putllc(lock->buffer,lock->ea,0,0);
			atomic_status=spu_readch(MFC_RdAtomicStat);
			result = (0 == atomic_status)?1:0;
		}
	} while (__builtin_expect(block&&!result,0));

	if (interrupts_enabled)
	{
		spu_ienable();
	}

	lock->currentDirection=__builtin_expect(result,1)?dir:lock->currentDirection; // change if successful.
	return result;

}

int _edgeDirectionLockSetToWrite(EdgeDirectionLock *lock, const int block)
{
	if(!block)
	{
		register const int updated=_edgeTryReleaseDirectonLock(lock,block)&&_edgeTryUpdateDirectionLock(lock,block,1);
		return updated?CELL_OK:EAGAIN;
	}
	else
	{
		_edgeReleaseDirectonLockAndSet(lock,1);
		return CELL_OK;
	}
}

int _edgeDirectionLockSetToRead(EdgeDirectionLock *lock, const int block)
{
	if(!block)
	{
		register const int updated=_edgeTryReleaseDirectonLock(lock,block)&&_edgeTryUpdateDirectionLock(lock,block,-1);
		return updated?CELL_OK:EAGAIN;
	}
	else
	{
		_edgeReleaseDirectonLockAndSet(lock,-1);
	}
	
	return CELL_OK;
}

int _edgeDirectionLockRelease(EdgeDirectionLock *lock, const int block)
{
	register const int updated=_edgeTryReleaseDirectonLock(lock,block);
	return updated?CELL_OK:EAGAIN;
}

void _edgeDirectionLockInitialize(EdgeDirectionLock* lock, uint64_t ea, volatile int32_t* atomic_buffer)
{
	lock->ea               = ea;
	lock->currentDirection = 0;
	lock->buffer           = atomic_buffer;
}

#else

#include <ppu_intrinsics.h>

void _edgeDirectionLockInitialize(volatile int32_t* lock)
{
	*lock=0;
	__lwsync();
}
#endif
