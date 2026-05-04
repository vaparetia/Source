//----------------------------------------------------------------------------
// ShaderSetup.cpp
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ShaderSetup.h"

//----------------------------------------------------------------------------

#include "algorithm"
#include "string.h"

#ifndef SHADER_MEASURE_MODE
#include "cell/dma.h"
#include "cell/gcm_spu.h"
#endif

#ifndef SPU
#define spu_printf printf
#endif

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/PS3/PS3CCompiledShaderTypes.h"

//----------------------------------------------------------------------------

//#define VERBOSE
//#define VERBOSE_NOTFOUND
//#define VERBOSE_COMMAND_BUFFER_HOLE

//----------------------------------------------------------------------------

namespace
{
   #if !defined(WIN32) && !defined(LINUX)
   #define ENDSWAP(a)	a
   #else

   #define ENDSWAP(a) convert_endianness(a)
      template< typename T > inline T convert_endianness( const T value)
      {
         if ( sizeof( T ) == 1 )
         {
            return value;
         }
         if ( sizeof( T ) == 2 )
         {
            return ( ((value & 0x00FF) << 8)
               | ((value & 0xFF00) >> 8) );
         }
         if ( sizeof( T ) == 4 )
         {
            return ( ((value & 0x000000FF) << 24)
               | ((value & 0x0000FF00) << 8)
               | ((value & 0x00FF0000) >> 8)
               | ((value & 0xFF000000) >> 24) );
         }
         if ( sizeof( T ) == 8 )
         {
            T result = value;
            for ( int ii = 0; ii < 4; ++ii )
            {
               char ch = *( (( char* ) &result) + ii );
               *( (( char* ) &result) +      ii  ) = *( (( char* ) &result) + (7 - ii) );
               *( (( char* ) &result) + (7 - ii) ) = ch;
            }
            return result;
         }
         // exception
         return value;
      }

   #endif

   //----------------------------------------------------------------------------

   struct SParameterSortPred
   {
      bool operator() (uint32* pLhs, uint32* pRhs) const
      {
         return *pLhs < *pRhs;
      }
   };

   //----------------------------------------------------------------------------

   void BuildParameterList(uint8* parameters, int const parametersSize, uint32** pParameters, int * pParameterCount)
   {
      union
      {
         uint8* pCurrentParameterU8;
         uint32* pCurrentParameterU32;
      };

      pCurrentParameterU8 = parameters;

      uint8* parametersEnd = parameters + parametersSize;
      while( pCurrentParameterU8 < parametersEnd )
      {
         uint32* pParameterBeginPtr  = pCurrentParameterU32;

         uint32 const crc = *pCurrentParameterU32++;
         uint32 const size = *pCurrentParameterU32++;

         // skip data of the parameter
         pCurrentParameterU8 += size;

         int const parameterCount = *pParameterCount;

         for( int i = 0; ; ++i )
         {
            if( i < parameterCount )
            {
               uint32 const parameterCRC = *(pParameters[i]);
               if( parameterCRC == crc )
                  break;
            }
            else
            {
               pParameters[parameterCount] = pParameterBeginPtr;
               (*pParameterCount)++;
               break;
            }
         }

      }
   }

   //----------------------------------------------------------------------------

   template<class CONTEXT_TYPE> __inline void SetVertexParameter( CONTEXT_TYPE & context, uint32 const parameterCRC, uint32 const valueCount, uint32 const reg, uint32** & pCurrentParameter, uint32** pParametersEnd )
   {
      while( pCurrentParameter < pParametersEnd )
      {
         uint32* pParameter = *pCurrentParameter;

         uint32 const dataCRC = *(*(pCurrentParameter));
         void* pParameterData = (pParameter + 2);

         if( dataCRC == parameterCRC )
         {
#ifdef VERBOSE_SET
            spu_printf("Set Vertex Parameter: %8.8x reg: %d\n", parameterCRC, reg);
#endif
            context.SetVertexProgramConstants(reg, valueCount, (real32 const *)pParameterData);

            ++pCurrentParameter;
            break;
         }
         else if( dataCRC < parameterCRC )
         {
            pCurrentParameter++;
         }
         else // (dataCRC > parameterCRC)
         {
#ifdef VERBOSE_NOTFOUND
            spu_printf("Vertex Parameter: %8.8x (%d) not found\n", parameterCRC, reg);
#endif
            break;
         }
      }
   }

   //----------------------------------------------------------------------------

   // Specialized version for measure context because we don't actually have any parameters provided.
   __inline void SetVertexParameter( cell::Gcm::Measure::CellGcmContext & context, uint32 const /*parameterCRC*/, uint32 const valueCount, uint32 const /*reg*/, uint32** & /*pCurrentParameter*/, uint32** /*pParametersEnd*/ )
   {
      context.SetVertexProgramConstants(0, valueCount, NULL);
   }

   //----------------------------------------------------------------------------

   template<class CONTEXT_TYPE> void SetVertexParameters( CONTEXT_TYPE & context, uint32** pParameters, uint32** pParametersEnd, uint8* pBaseAddress )
   {
      SShaderVertexData const * pHeader = (SShaderVertexData const*)pBaseAddress;

      uint32** pCurrentParameter = pParameters;

      uint32* pVertexParameter = (uint32*)(pBaseAddress + ENDSWAP(pHeader->mVertexParametersOffset));
      uint32* pVertexParameterEnd = pVertexParameter + ENDSWAP(pHeader->mVertexParametersSize) / 4;

      while( pVertexParameter < pVertexParameterEnd )
      {
         uint32 const parameterCRC = ENDSWAP(*pVertexParameter++);
         uint32 const valueCount = ENDSWAP(*pVertexParameter++);
         uint32 const reg = ENDSWAP(*pVertexParameter++); 

         // skip over padding (this will fall away later when this code is more optimized)
         pVertexParameter++;

         SetVertexParameter(context, parameterCRC, valueCount, reg, pCurrentParameter, pParametersEnd);
      }
   }

   //----------------------------------------------------------------------------

   void PatchFragmentProgram(uint32** pParameters, uint32** pParametersEnd, uint8* pBaseAddress, uint8* pFragmentMemory)
   {
      SShaderFragmentData const * pHeader = (SShaderFragmentData const *)pBaseAddress;
      uint32** pCurrentParameter = pParameters;

      uint8* pFragmentParameterU8 = pBaseAddress + pHeader->mFragmentParametersOffset;
      uint8 * pFragmentParameterEnd = pFragmentParameterU8 + pHeader->mFragmentParametersSize;
      while( pFragmentParameterU8 < pFragmentParameterEnd )
      {
         uint32 const parameterCRC = ((uint32)pFragmentParameterU8[0] << 24) | ((uint32)pFragmentParameterU8[1] << 16) | ((uint32)pFragmentParameterU8[2] << 8) | (uint32)pFragmentParameterU8[3];
         pFragmentParameterU8 += 4;

         uint16 const dataOffset = *((uint16*)pFragmentParameterU8); 
         pFragmentParameterU8 += 2;

         uint16 const destOffsetCount = *((uint16*)pFragmentParameterU8); 
         pFragmentParameterU8 += 2;

         while( pCurrentParameter < pParametersEnd )
         {
            uint32 const dataCRC = *(*(pCurrentParameter));

            if( dataCRC == parameterCRC )
            {
               uint32* pParameter = *pCurrentParameter;
               real32* pParameterData = (real32*)((uint8*)(pParameter + 2) + dataOffset);
               real32 parameterData0 = cellGcmSwap16Float32(*pParameterData++);
               real32 parameterData1 = cellGcmSwap16Float32(*pParameterData++);
               real32 parameterData2 = cellGcmSwap16Float32(*pParameterData++);
               real32 parameterData3 = cellGcmSwap16Float32(*pParameterData++);

               for( int offsetIdx = 0 ; offsetIdx < destOffsetCount; ++offsetIdx )
               {
                  uint16 const destOffset = *((uint16*)pFragmentParameterU8);
                  pFragmentParameterU8 += 2;

                  real32* pDestData = (real32*)(pFragmentMemory + destOffset);
                  *pDestData++ = parameterData0;
                  *pDestData++ = parameterData1;
                  *pDestData++ = parameterData2;
                  *pDestData++ = parameterData3;
               }
               // IMPORTANT: It is key for us not to advance the data ptr here, because we can have multiple fragment parameters access the same data (for example when the data is a matrix)
               // In the case where the next parameter doesn't use this data the else case directly below will take care of advancing the data.
               break;
            }
            else if( dataCRC < parameterCRC )
            {
               pCurrentParameter++;
            }
            else // (dataCRC > parameterCRC)
            {
#ifdef VERBOSE_NOTFOUND
               spu_printf("Fragment Parameter: %8.8x not found\n", parameterCRC);
#endif
               // if parameter not found, skip over offsets.
               pFragmentParameterU8 += destOffsetCount * 2;
               break;
            }
         }
      }
   }

   //----------------------------------------------------------------------------

   template<class CONTEXT_TYPE> __inline void SetSampler( CONTEXT_TYPE & context, uint32** &pCurrentParameter, uint32** pParametersEnd, uint32 const parameterCRC, uint16 const index, uint8 const /*wrapU*/, uint8 const /*wrapV*/, uint8 const wrapW, uint8 const minFilter, uint8 const magFilter, uint8 const conv )
   {
      while( pCurrentParameter < pParametersEnd )
      {
         uint32 const dataCRC = *(*(pCurrentParameter));

         if( dataCRC == parameterCRC )
         {
            uint32* pParameter = *pCurrentParameter;

            bool const srgb = *(pParameter + 2);
            bool const clampU = *(pParameter + 3);
            bool const clampV = *(pParameter + 4);
            
            uint8 const wrapU = clampU ? CELL_GCM_TEXTURE_CLAMP_TO_EDGE : CELL_GCM_TEXTURE_WRAP;
            uint8 const wrapV = clampV ? CELL_GCM_TEXTURE_CLAMP_TO_EDGE : CELL_GCM_TEXTURE_WRAP;

            CellGcmTexture* pTexture = (CellGcmTexture*)(pParameter + 5);

            int maxAniso = CELL_GCM_TEXTURE_MAX_ANISO_1;

            bool const hasMips = (pTexture->mipmap > 1);
            if(hasMips)
            {
               maxAniso = CELL_GCM_TEXTURE_MAX_ANISO_4;
            }

            context.SetTexture(index, pTexture);

            real32 const minLOD = 0.0f;
            real32 const maxLOD = pTexture->mipmap;

            context.SetTextureControl(index, CELL_GCM_TRUE, (uint32_t)(minLOD * 256.0f), (uint32_t)(maxLOD * 256.0f), maxAniso);

            context.SetTextureAddress(index, 
               wrapU, wrapV, wrapW, 
               CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, 
               CELL_GCM_TEXTURE_ZFUNC_LESS, 
               srgb ? (CELL_GCM_TEXTURE_GAMMA_R|CELL_GCM_TEXTURE_GAMMA_G|CELL_GCM_TEXTURE_GAMMA_B) : 0);


            uint8 const actualMinFilter = ( minFilter == CELL_GCM_TEXTURE_LINEAR && hasMips ) ? CELL_GCM_TEXTURE_LINEAR_LINEAR : minFilter;

            real32 const kDefaultLODBias = -0.26f;
            context.SetTextureFilter(index, (int)(kDefaultLODBias * 256.0f) & 0x1fff, actualMinFilter, magFilter, conv);

            // NOTE: The higher the slope the more pixels are sampled bilinear instead of trilinear.
            // A slope of 1.0 would mean every pixel was trilinear, A slope of "infinity" would mean every pixel was bilinear.
            // The hardware default value for slope is 8 (which really turns into 2.0), we're setting it to 22 (which turns into 6.0).
            context.SetTextureOptimization(index, 22, CELL_GCM_TEXTURE_ISO_HIGH, CELL_GCM_TEXTURE_ANISO_HIGH);
#ifdef VERBOSE_SET
            spu_printf("SetSampler: crc: %8.8x unit %d\n", parameterCRC, index);
#endif
            ++pCurrentParameter;
            break;
         }
         else if( dataCRC < parameterCRC )
         {
            pCurrentParameter++;
         }
         else // (dataCRC > parameterCRC)
         {
#ifdef VERBOSE_NOTFOUND
            spu_printf("SetSampler: crc: %8.8x unit %d not set.\n", parameterCRC, index);
#endif
            break;
         }
      }
   }

   //----------------------------------------------------------------------------

   __inline void SetSampler( cell::Gcm::Measure::CellGcmContext & context, uint32** & /*pCurrentParameter*/, uint32** /*pParametersEnd*/, uint32 const /*parameterCRC*/, uint16 const /*index*/, uint8 const /*wrapU*/, uint8 const /*wrapV*/, uint8 const /*wrapW*/, uint8 const /*minFilter*/, uint8 const /*magFilter*/, uint8 const /*conv*/)
   {
      context.SetTexture(0, NULL);
      context.SetTextureControl(0, 0, 0, 0, 0);
      context.SetTextureAddress(0, 0, 0, 0, 0, 0, 0);
      context.SetTextureFilter(0, 0, 0, 0, 0);
      context.SetTextureOptimization(0, 0, 0, 0);
   }

   //----------------------------------------------------------------------------

   template<class CONTEXT_TYPE> void SetSamplers( uint32** pParameters, uint32** pParametersEnd, uint8* pBaseAddress, CONTEXT_TYPE & context )
   {
      SShaderFragmentData const * pHeader = (SShaderFragmentData const*)pBaseAddress;

      uint32** pCurrentParameter = pParameters;

      union
      {
         uint8* pSamplerParameterU8;
         uint16* pSamplerParameterU16;
         uint32* pSamplerParameterU32;
      } ptr;

      ptr.pSamplerParameterU8 = pBaseAddress + ENDSWAP(pHeader->mSamplerParametersOffset);
      uint8 * pSamplerParameterEnd = ptr.pSamplerParameterU8 + ENDSWAP(pHeader->mSamplerParametersSize);
      while( ptr.pSamplerParameterU8 < pSamplerParameterEnd )
      {
         uint32 const parameterCRC = ENDSWAP(*ptr.pSamplerParameterU32++);

         uint16 const index = ENDSWAP(*ptr.pSamplerParameterU16++);

         uint8 const minFilter = *ptr.pSamplerParameterU8++;
         uint8 const magFilter = *ptr.pSamplerParameterU8++;
         uint8 const conv = *ptr.pSamplerParameterU8++;
         uint8 const wrapU = *ptr.pSamplerParameterU8++;
         uint8 const wrapV = *ptr.pSamplerParameterU8++;
         uint8 const wrapW = *ptr.pSamplerParameterU8++;

         SetSampler(context, pCurrentParameter, pParametersEnd, parameterCRC, index, wrapU, wrapV, wrapW, minFilter, magFilter, conv);
      }
   }

   //----------------------------------------------------------------------------

   template<class CONTEXT_TYPE> __inline void SetShaderImpl(uint8* pShaderVertexDataLS,
                                                            uint8* pShaderFragmentDataLS,
                                                            uint8* materialConstantParametersLS, int const materialConstantParametersSize,
                                                            uint8* materialRuntimeParametersSharedLS, int const materialRuntimeParametersSharedSize,
                                                            uint8* materialRuntimeParametersUnsharedLS, int const materialRuntimeParametersUnsharedSize,
                                                            uint32 const jumpToNextOffset,
                                                            uint32 const outFragmentProgramOffset,
                                                            CONTEXT_TYPE & context,
                                                            uint8* pFragmentMemory,
                                                            uint8* scratchMemoryLS)
   {
      SShaderFragmentData const * pShaderFragmentDataHeader = (SShaderFragmentData const*)pShaderFragmentDataLS;
      SShaderVertexData const * pShaderVertexDataHeader = (SShaderVertexData const*)pShaderVertexDataLS;
      
      uint8* pFragmentProgramUCode = pShaderFragmentDataLS + pShaderFragmentDataHeader->mFragmentProgramUCodeOffset;
      uint8* pVertexProgramUCode = pShaderVertexDataLS + pShaderVertexDataHeader->mVertexProgramUCodeOffset;

      // Build list of unique parameters sorted by crc, if a parameter is already in the list, it will not be added again.
      uint32** pParameters = (uint32**)scratchMemoryLS;
      uint32** pParametersEnd = NULL;
#ifndef SHADER_MEASURE_MODE
      {
         int parameterCount = 0;

         // Build list, each crc only occurs once.
         BuildParameterList(materialRuntimeParametersUnsharedLS, materialRuntimeParametersUnsharedSize, pParameters, &parameterCount);
         BuildParameterList(materialRuntimeParametersSharedLS, materialRuntimeParametersSharedSize, pParameters, &parameterCount);
         BuildParameterList(materialConstantParametersLS, materialConstantParametersSize, pParameters, &parameterCount);
         BuildParameterList(pShaderFragmentDataLS + pShaderFragmentDataHeader->mShaderConstantParametersOffset, pShaderFragmentDataHeader->mShaderConstantParametersSize, pParameters, &parameterCount);
         BuildParameterList(pShaderVertexDataLS + pShaderVertexDataHeader->mShaderConstantParametersOffset, pShaderVertexDataHeader->mShaderConstantParametersSize, pParameters, &parameterCount);

         // Sort list of parameters by their CRC.
         std::sort(pParameters, pParameters + parameterCount, SParameterSortPred());

         pParametersEnd = pParameters + parameterCount;
         scratchMemoryLS += BPE_PAD_16(parameterCount * sizeof(uint32));
      }
#endif

      // Insert jump to next
      context.SetJumpCommand(jumpToNextOffset);

      // Set vertex program
#ifdef SHADER_MEASURE_MODE
      CellCgbVertexProgramConfiguration vertexConfig = *(CellCgbVertexProgramConfiguration *)(pShaderVertexDataLS + ENDSWAP(pShaderVertexDataHeader->mVertexProgramConfigOffset));
      vertexConfig.instructionCount = ENDSWAP(vertexConfig.instructionCount);
      context.SetVertexProgramLoad(&vertexConfig, pVertexProgramUCode);
#else
      CellCgbVertexProgramConfiguration * pVertexConfig = (CellCgbVertexProgramConfiguration *)(pShaderVertexDataLS + ENDSWAP(pShaderVertexDataHeader->mVertexProgramConfigOffset));
      context.SetVertexProgramLoad(pVertexConfig, pVertexProgramUCode);
#endif

      // Set vertex parameters
      SetVertexParameters(context, pParameters, pParametersEnd, pShaderVertexDataLS);

#ifndef SHADER_MEASURE_MODE
      // Copy initial fragment program to memory
      memcpy(pFragmentMemory, pFragmentProgramUCode, pShaderFragmentDataHeader->mFragmentProgramUCodeSize);

      // Patch fragment program
      PatchFragmentProgram(pParameters, pParametersEnd, pShaderFragmentDataLS, pFragmentMemory);
#endif
      // Set fragment program
      {
         CellCgbFragmentProgramConfiguration fragmentConfig = *(CellCgbFragmentProgramConfiguration*)(pShaderFragmentDataLS + ENDSWAP(pShaderFragmentDataHeader->mFragmentProgramConfigOffset));
         fragmentConfig.offset = outFragmentProgramOffset;

         context.SetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);

         context.SetFragmentProgramLoadLocation(&fragmentConfig, CELL_GCM_LOCATION_LOCAL);
      }

      // Set samplers
      SetSamplers(pParameters, pParametersEnd, pShaderFragmentDataLS, context);
   }

}

//----------------------------------------------------------------------------

#ifndef SHADER_MEASURE_MODE
void SetShader(uint8* pShaderFragmentDataLS,
               uint8* pShaderVertexDataLS,
               uint16 const commandBufferHoleSize,
               uint8* materialConstantParametersLS, int const materialConstantParametersSize,
               uint8* materialRuntimeParametersSharedLS, int const materialRuntimeParametersSharedSize,
               uint8* materialRuntimeParametersUnsharedLS, int const materialRuntimeParametersUnsharedSize,
               uint8* scratchMemoryLS,
               uint32 const outFragmentProgramEA,
               uint32 const outFragmentProgramOffset,
               uint32 const outCommandBufferHoleEA,
               uint32 const jumpToNextOffset)
{
   //__asm volatile ("stopd 0,1,1");

   SShaderFragmentData const * pShaderFragmentDataHeader = (SShaderFragmentData const*)pShaderFragmentDataLS;

   int const kDmaTag = 13;

   // Allocate scratch memory for output fragment memory
   uint8* pFragmentMemory = scratchMemoryLS;
   int const alignedFragmentProgramSize = BPE_PAD_16(pShaderFragmentDataHeader->mFragmentProgramUCodeSize);
   scratchMemoryLS += alignedFragmentProgramSize;

   // Align ptr to 128
   scratchMemoryLS = (uint8*)((uint32(scratchMemoryLS) + 127) & ~127);

   // Allocate scratch memory for output command buffer
   cell::Gcm::UnsafeInline::CellGcmContext context;
   {
      //memset(scratchMemoryLS, 0, commandBufferHoleSize);
      cellGcmSetupContextData(&context, (uint32_t*)scratchMemoryLS, commandBufferHoleSize, NULL);
      scratchMemoryLS += commandBufferHoleSize;
   }

   SetShaderImpl(pShaderVertexDataLS,
                 pShaderFragmentDataLS, 
                 materialConstantParametersLS, materialConstantParametersSize, 
                 materialRuntimeParametersSharedLS, materialRuntimeParametersSharedSize,
                 materialRuntimeParametersUnsharedLS, materialRuntimeParametersUnsharedSize,
                 jumpToNextOffset,
                 outFragmentProgramOffset,
                 context,
                 pFragmentMemory, 
                 scratchMemoryLS);

   // (SANITY CHECK) check for overflow
   uint32 const actualCommandBufferSizeInBytes = (context.current - context.begin) * 4;

   if( commandBufferHoleSize < actualCommandBufferSizeInBytes )
   {
      //spu_printf("FATAL ERROR: precalculated command buffer hole size isn't big enough: Calculated: %d Actual: %d!\n", commandBufferHoleSize, actualCommandBufferSizeInBytes);
      return;
   }

   // Fill unused space with nops
   {
      int const bytesFilled = (context.current - context.begin) * 4;
      int const bytesRemaining = commandBufferHoleSize - bytesFilled;
      int const nopsToWrite = bytesRemaining / 4;

      context.SetNopCommand(nopsToWrite);

#ifdef VERBOSE_COMMAND_BUFFER_HOLE
      spu_printf("Context Begin: 0x%8.8x End: 0x%8.8x: Precalculated command buffer hole size: %d Actual: %d Wrote %d nops\n", outCommandBufferHoleEA, outCommandBufferHoleEA + 4 * (context.current - context.begin), pHeader->mCommandBufferHoleSize, actualCommandBufferSizeInBytes, nopsToWrite);
#endif
   }

   // Output fragment program
   cellDmaPut(pFragmentMemory, (uint64_t)outFragmentProgramEA, alignedFragmentProgramSize, kDmaTag, 0, 0);
   
   // Do fenced DMA get from last address written, this makes sure that the data has actually made it to video memory.
   // NOTE: this is unnecessary if the program is in main memory.
   {
      uint32 temp;
      cellDmaSmallGetf(&temp, (uint64_t)outFragmentProgramEA, 4, kDmaTag, 0, 0);
   }

   // DMA out command buffer hole
   {
      // NOTE: the command buffer hole EA is 16 byte aligned
      // We need to take the memory banks into account (128 byte chunks at 128 byte aligned addresses).
      // If the amount of data to be written spans over multiple banks must ensure to write the first memory bank last 
      // to ensure data is not read until it is fully written.

      // Determine how many bytes there is from the start address to the end of the memory bank that start address is in
      uint32 bytesUntilEndOfFirstMemoryBank = ((outCommandBufferHoleEA + 127) & ~127) - outCommandBufferHoleEA;
      
      // In case the address is aligned with the beginning of the memory bank the remaining size is the full memory bank size.
      if( bytesUntilEndOfFirstMemoryBank == 0 )
         bytesUntilEndOfFirstMemoryBank = 128;

      // In the case where the total command buffer size is smaller than the remainder in the current bank we can DMA
      // all of it in a single step because the JTS will be overwritten atomically with the rest of the data.
      if( commandBufferHoleSize < bytesUntilEndOfFirstMemoryBank )
      {
#ifdef VERBOSE_COMMAND_BUFFER_HOLE
         spu_printf("putf: target: 0x%8.8x size: %d\n", outCommandBufferHoleEA, commandBufferHoleSize);
#endif
         cellDmaPutf(context.begin, (uint64_t)outCommandBufferHoleEA, commandBufferHoleSize, kDmaTag, 0, 0);
      }
      // In the case where the total command buffer size is bigger than the remainder in the current bank we must DMA
      // the all the data beginning on the next memory bank out first and AFTERWARDS overwrite the data (including the JTS) 
      // in the first memory bank using a fenced DMA to guarantee order.
      else
      {
#ifdef VERBOSE_COMMAND_BUFFER_HOLE
         spu_printf("put: target: 0x%8.8x size: %d\n", outCommandBufferHoleEA + bytesUntilEndOfFirstMemoryBank, commandBufferHoleSize - bytesUntilEndOfFirstMemoryBank);
#endif
         cellDmaPut(uint32(context.begin) + bytesUntilEndOfFirstMemoryBank, (uint64_t)outCommandBufferHoleEA + bytesUntilEndOfFirstMemoryBank, commandBufferHoleSize - bytesUntilEndOfFirstMemoryBank, kDmaTag, 0, 0);

#ifdef VERBOSE_COMMAND_BUFFER_HOLE
         spu_printf("putf: target: 0x%8.8x size: %d\n", outCommandBufferHoleEA, bytesUntilEndOfFirstMemoryBank);
#endif
         cellDmaPutf(context.begin, (uint64_t)outCommandBufferHoleEA, bytesUntilEndOfFirstMemoryBank, kDmaTag, 0, 0);

#ifdef VERBOSE_COMMAND_BUFFER_HOLE
         // Print out the command buffer we generated in a RSXFifoDisassembler friendly way.
         for( int i = 0; i < (commandBufferHoleSize / 4); ++i )
         {
            uint32* pPtr = (uint32*)context.begin;
            spu_printf("%8.8x %8.8x ....\n", outCommandBufferHoleEA + i * 4, pPtr[i]);
         }
#endif
      }
   }

   cellDmaWaitTagStatusAll(1 << kDmaTag);
   //spu_printf("End Set Shader\n");
}
#endif

//----------------------------------------------------------------------------

#ifdef SHADER_MEASURE_MODE
int const MeasureShaderCommandBufferHole(uint8* vertexShaderDataLS, uint8* fragmentShaderDataLS)
{
   cell::Gcm::Measure::CellGcmContext context;
   context.current = 0;
   context.begin = 0;
   context.end = context.begin + 1024 * 1024;

   SetShaderImpl(vertexShaderDataLS, fragmentShaderDataLS, NULL, 0, NULL, 0, NULL, 0, 0, 0, context, NULL, NULL);
   
   uint32 sizeInBytes = (context.current - context.begin) * 4;
   sizeInBytes = (sizeInBytes + 15) & (~15);
   
   return sizeInBytes;
}
#endif