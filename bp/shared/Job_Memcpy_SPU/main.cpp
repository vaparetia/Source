#include <cell/spurs/job_chain.h>
#include <cell/spurs/job_context.h>
#include <cell/dma.h>
#include <spu_printf.h>

//NOTE: DMA Tags 0, 1, 2 and 31 are used by JobQueue implementation.

struct Buffer
{
   uint32_t bufferLS;
   uint32_t size;
   uint32_t dmaTag;
   int      hasData;
};

void cellSpursJobMain2(CellSpursJobContext2* context, CellSpursJob256 *job256)
{
   CellSpursJob128 *job = (CellSpursJob128 *)job256;

   uint32_t const size = job->workArea.userData[2];

   uint32_t currentDestEA = job->workArea.userData[0];
   uint32_t const destEndEA = currentDestEA + size;

   uint32_t currentSourceEA = job->workArea.userData[1];
   uint32_t const sourceEndEA = currentSourceEA + size;

   //spu_printf("Starting SPU memcpy src: %8.8x dst: %8.8x size: %8.8x\n", currentSourceEA, currentDestEA, size);

   int const kScratchBufferSize = 64 * 1024;
   int const kBufferSize = kScratchBufferSize / 2;
   
   Buffer buffers[2];
   
   buffers[0].bufferLS = (uint32_t)context->sBuffer;
   buffers[0].dmaTag = 10;
   buffers[0].size = 0;
   buffers[0].hasData = 1;
   
   buffers[1].bufferLS = (uint32_t)context->sBuffer + kBufferSize;
   buffers[1].dmaTag = 11;
   buffers[1].size = 0;
   buffers[1].hasData = 0;

   int currentBufferIdx = 0;

   while( currentDestEA < destEndEA )
   {
      Buffer & buffer = buffers[currentBufferIdx];
      
      // Do we have data to output?
      if( buffer.hasData )
      {
         if( buffer.size )
         {
            // wait for last read to finish
            cellDmaWaitTagStatusAll(1 << buffer.dmaTag);

            // initiate write
            //spu_printf("SPU memcpy[%d]: %8.8x <- %8.8x (size: %8.8x)\n", currentBufferIdx, currentDestEA, buffer.bufferLS, buffer.size);
            cellDmaLargePut(buffer.bufferLS, currentDestEA, buffer.size, buffer.dmaTag, 0, 0);

            // advance write ptr
            currentDestEA += buffer.size;
         }
         buffer.hasData = 0;
      }
      else
      {
         // wait for last write to finish
         cellDmaWaitTagStatusAll(1 << buffer.dmaTag);

         buffer.size = sourceEndEA - currentSourceEA;
         buffer.size = (buffer.size > kBufferSize) ? kBufferSize : buffer.size;
         
         if( buffer.size )
         {
            // initiate new read
            //spu_printf("SPU memcpy[%d]: %8.8x -> %8.8x (size: %8.8x)\n", currentBufferIdx, currentSourceEA, buffer.bufferLS, buffer.size);
            cellDmaLargeGet(buffer.bufferLS, currentSourceEA, buffer.size, buffer.dmaTag, 0, 0);

            // advance read ptr
            currentSourceEA += buffer.size;
         }

         buffer.hasData = 1;
      }

      currentBufferIdx = (currentBufferIdx + 1) & 1;
   }

   // Wait for any remaining writes to finish
   cellDmaWaitTagStatusAll(1 << buffers[0].dmaTag | 1 << buffers[1].dmaTag);

}

