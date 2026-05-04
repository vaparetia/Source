//----------------------------------------------------------------------------
// Main.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"

//----------------------------------------------------------------------------

#include "NRenderQueue.h"

//----------------------------------------------------------------------------

union SParameters
{
   qword args;
   uint64_t u64[2];
};

int const kQueueDmaTag = 10;

int const kScratchMemSize = 128 * 1024;
uint8 sScratchMem[kScratchMemSize];

uint8* sScratchMemEnd = sScratchMem + kScratchMemSize;

//----------------------------------------------------------------------------

using namespace NRenderQueue;

//----------------------------------------------------------------------------

int cellSpursTaskMain(qword argTask, uint64_t argTaskset)
{
   //spu_printf("Renderer_SPU: Startup\n");

   SParameters parameters;
   parameters.args = argTask;
   
   uint64_t queueEA = parameters.u64[0];

   CellSpursLFQueuePopContainer popContainer;
   SRenderCmd cmd;

   cellSpursLFQueuePopContainerInitialize(&popContainer, &cmd, kQueueDmaTag);

   bool shouldTerminate = false;

   while(!shouldTerminate)
   {
      if( cellSpursLFQueueTryPopBegin(queueEA, &popContainer) == CELL_OK )
      {
         cellSpursLFQueuePopEnd(queueEA, &popContainer);

         if( ProcessCommand(&cmd, sScratchMem) == kPR_Terminate )
            shouldTerminate = true;
      }

      unsigned int queueSize;
      cellSpursLFQueueSize(queueEA, &queueSize);

      // Only allow yielding to other tasks if we have no more work to do
      if( queueSize == 0 )
      {
         int res = cellSpursTaskPoll();

         if( res != 0 )
         {
            //spu_printf("Begin Yield\n");

            cellSpursYield();

            cellSpursLFQueueSize(queueEA, &queueSize);
            //spu_printf("End Yield, Items: %d\n", queueSize);
         }
      }
   }

   spu_printf("Renderer_SPU: Shutdown\n");
   cellSpursTaskExit(0);
   return 0;
}

