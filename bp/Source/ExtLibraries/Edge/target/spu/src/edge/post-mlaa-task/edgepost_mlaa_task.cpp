/* SCE CONFIDENTIAL
 * PlayStation(R)Edge 1.2.0
 * Copyright (C) 2010 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <stdint.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>

#include <edge/edge_atomic.h>
#include <edge/edge_assert.h>
#include <edge/edge_dma.h>

#include <edge/post/edgepost_mlaa.h>
#include <edge/post/edgepost_mlaa_memory_layout.h>

union MemoryLayout{
	EdgePostMlaaMemoryLayout mlaa;
	EdgePostInplaceTranposeMemoryLayout transpose;
} EDGE_MLAA_ALIGNED(128);

static MemoryLayout g_Mem;

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
	(void)argTaskset;

	// although this is a spurs task main and thus should not have interrupts enabled,
	// we handle interrupts properly, in case this gets copied and pasted.
	// In general, removing *all* interrupt handling code from the task will not result
	// in a tangible speed-up. Last time we measured it was about 50us total.

	while (1)
	{
		// Wait for a signal
		{
			const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();
			cellSpursWaitSignal();
			if( interrupts_enabled )
			{
				spu_ienable();
			}
		}

		register const	uint32_t		parameter_ea = spu_extract( (vec_uint4)argTask, 0 );
		EdgePostMlaaTaskParameters	parameter;
		register const	uint32_t		parameter_size=sizeof(parameter);		


		{
			const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();
			EDGE_DMA_GET(&parameter,parameter_ea,parameter_size,0,0,0);
			EDGE_DMA_WAIT_TAG_STATUS_ALL(1); //wait for data to arrive.
			if( interrupts_enabled )
			{
				spu_ienable();
			}
		}

		if(0 == (EDGE_POST_MLAA_MODE_ENABLED&parameter.mode))
		{
			// This is debug functionality which allows the bypassing of the MLAA operation.
			// This will simply cause the source to be copied into the destination.
			const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
			spu_idisable();
			for(register uint32_t i=0;i<parameter.imageHeight;++i)
			{
				const register uint32_t index=i&BUFFER_LINE_MASK;
				EDGE_DMA_GETF(g_Mem.mlaa.ParameterBufferPool[index],parameter.imageAddress+i*parameter.imagePitch,parameter.imageWidth * 4, index,0,0);
				EDGE_DMA_PUTF(g_Mem.mlaa.ParameterBufferPool[index],parameter.destAddress+i*parameter.imagePitch,parameter.imageWidth * 4, index,0,0);
			}
			cellSpursBarrierNotify(parameter.barrierAddress);
			cellSpursBarrierWait(parameter.barrierAddress);
			if( interrupts_enabled )
			{
				spu_ienable();
			}
		}
		else
		{

			// we allow transpose to be performed on a single SPU, to reduce XDR traffic and minimize SPU utilization.
			const uint32_t maxTransposeSpus = (parameter.mode&EDGE_POST_MLAA_MODE_SINGLE_SPU_TRANSPOSE)?1:2;

			edgePostTransposePass(&g_Mem.transpose, &parameter, 1, maxTransposeSpus,  0);
			//after this, we no longer use the input buffer, so we set it to the destination buffer for the other passes
			parameter.imageAddress = parameter.destAddress;


			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(0xffffffff); // flush the queues before we yield
				cellSpursBarrierNotify(parameter.barrierAddress);
				cellSpursBarrierWait(parameter.barrierAddress);
				if( interrupts_enabled ) 
				{
					spu_ienable();
				}
			}


			edgePostMlaaPass(&g_Mem.mlaa, &parameter, 1, 1, 0); // vertical pass.

			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(0xffffffff); // flush the queues before we yield
				cellSpursBarrierNotify(parameter.barrierAddress);
				cellSpursBarrierWait(parameter.barrierAddress);
				if( interrupts_enabled )
				{
					spu_ienable();
				}
			}


			edgePostTransposePass(&g_Mem.transpose, &parameter, 0, maxTransposeSpus, 0);

			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(0xffffffff); // flush the queues before we yield
				cellSpursBarrierNotify(parameter.barrierAddress);
				cellSpursBarrierWait(parameter.barrierAddress);
				if( interrupts_enabled )
				{
					spu_ienable();
				}
			}

			edgePostMlaaPass(&g_Mem.mlaa, &parameter, 0, 0, 0); // horizontal pass.
			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_WAIT_TAG_STATUS_ALL(0xffffffff); // flush the queues before we yield
				cellSpursBarrierNotify(parameter.barrierAddress);
				cellSpursBarrierWait(parameter.barrierAddress);
				if( interrupts_enabled )
				{
					spu_ienable();
				}
			}
		}

		// This is sync'd over all SPUs by the barrier after the horizontal pass
		if (parameter.rsxLabelAddress != 0)
		{
			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_DMA_PUT_UINT32(parameter.rsxLabelValue,parameter.rsxLabelAddress,0,0,0);
				EDGE_DMA_WAIT_TAG_STATUS_ALL(1);
				if( interrupts_enabled )
				{
					spu_ienable();
				}
			}
		}

		if(parameter.taskCounterAddress)
		{
			{
				const register bool interrupts_enabled = spu_readch(SPU_RdMachStat) & 1;
				spu_idisable();
				EDGE_MLAA_ALIGNED(128) uint32_t atomic_buffer[32];
				EDGE_DMA_WAIT_TAG_STATUS_ALL(0xffffffff); //make sure all is done before we let anyone else into LS.
				EDGE_ATOMIC_INCR_32(atomic_buffer,parameter.taskCounterAddress);
				if( interrupts_enabled )
				{
					spu_ienable();
				}
			}
		}
	}
	// WARNING: This must never be reached, as we do not save the context needed for atexit.
	// If you do want this task to exit, please make sure enough context is saved.
	EDGE_ASSERT(!"This task must not exit!");
	return 0; 
}
