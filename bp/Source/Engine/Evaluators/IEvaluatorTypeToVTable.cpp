//------------------------------------------------------------------------------------------
// IEvaluatorTypeToVTable.h
// Type to vtable conversion for linear copies of evaluator properties.
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "IEvaluatorTypeToVTable.h"
#include "IEvaluator.h"
#include "CEvaluatorUpdateData.h"
#include "Engine/Streams/CLinearCopyStream.h"

//------------------------------------------------------------------------------------------

class CMainCPU_EvaluatorTypeToVTable : public IEvaluatorTypeToVTable
{
public:
   CMainCPU_EvaluatorTypeToVTable() {};
   uint32 InsertFakeVTable(CLinearCopyStream &stream, IEvaluator const *pEvaluator) const
   {
#if BPE_TARGET != BPE_TARGET_PS3
      // For MSDEV we'll just use the real vtable
      uint32 *pClassPtr = (uint32*)pEvaluator;
      /*
      switch (pEvaluator->GetClassType())
      {
         case IEvaluator::kECT_Float:
         {
            IFloatEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
         case IEvaluator::kECT_Bool:
         {
            IBoolEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
         case IEvaluator::kECT_Vector:
         {
            IVectorEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
         case IEvaluator::kECT_Color:
         {
            IColorEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
         case IEvaluator::kECT_Modifier:
         {
            IModifierEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
         case IEvaluator::kECT_Emitter:
         {
            IEmitterEvaluator const *pCastEval = static_cast<IFloatEvaluator const *>(pEvaluator);
            pClassPtr = (uint32*)pEvaluator;
         }
         break;
      }
      */
      stream.Put(0xBEEFDEAD);
      return *pClassPtr;

#else
      stream.Put(0xBEEFDEAD);
      return 0;
#endif
      
   }
};

//------------------------------------------------------------------------------------------

struct SSPUTypeToVTableEntry
{
   uint32   mSPU_LS_GetValueFunctionAddress;
   uint32   mSPU_LS_InitializerFunctionAddress;
   uint32   mEvaluatorType;
};

static SSPUTypeToVTableEntry skAllSPUEvaluators[] =
{
#if BPE_TARGET == BPE_TARGET_PS3
   #include "Game/SPU_Particles/SPU_Particles_eval_funcs.h"
#endif
   {
      NULL, // Terminator
      NULL,
   },
};


//------------------------------------------------------------------------------------------

static void evaluator_spu_functions_from_type(uint32 const type, uint32 &getValueFunc, uint32 &initializerFunc)
{
   getValueFunc = 0;
   initializerFunc = 0;
   // Slow linear search method for now.
   SSPUTypeToVTableEntry *pEntry = skAllSPUEvaluators;
   while (pEntry->mEvaluatorType != NULL)
   {
      if (pEntry->mEvaluatorType == type)
      {
         getValueFunc = pEntry->mSPU_LS_GetValueFunctionAddress;
         initializerFunc = pEntry->mSPU_LS_InitializerFunctionAddress;
         return;
      }
      pEntry++;
   }
   bpe_debugger_printf("Can't find evaluator SPU function pointers for 0x%08x\n", type);
   return;
}

//------------------------------------------------------------------------------------------

class CSPU_EvaluatorTypeToVTable : public IEvaluatorTypeToVTable
{
public:
   CSPU_EvaluatorTypeToVTable() {};
   uint32 InsertFakeVTable(CLinearCopyStream &stream, IEvaluator const *pEvaluator) const
   {
      // All evaluator 'GetValue' functions are offset by entry count in vtable.
      // You might need to reverse engineer code dumps to figure out vtable offsets.
      int vtableOffset = 0;
      IEvaluator::STypeInfo typeInfo;
      pEvaluator->GetInfo(typeInfo);
      switch (typeInfo.mClassType)
      {
         case IEvaluator::kECT_Float:
         {
            vtableOffset = -7 * (int) sizeof(void*);
         }
         break;
         case IEvaluator::kECT_Bool:
         {
            vtableOffset = -7* (int) sizeof(void*);
         }
         break;
         case IEvaluator::kECT_Vector:
         {
            vtableOffset = -7 * (int) sizeof(void*);
         }
         break;
         case IEvaluator::kECT_Color:
         {
            vtableOffset = -7 * (int) sizeof(void*);
         }
         break;
         case IEvaluator::kECT_Modifier:
         {
            vtableOffset = -7 * (int) sizeof(void*);
         }
         break;
         case IEvaluator::kECT_Emitter:
         {
            vtableOffset = -7 * (int) sizeof(void*);
         }
         break;
         default:
            BPE_VERIFYA(false, "Unknown evaluator type.");
            break;
      }
      uint32 vtablePtr = stream.BufferPosToBufferPointer(stream.mCurrentBufferPos + vtableOffset);

      uint32 spu_GetValueFunc;
      uint32 spu_InitializerFunc;
      evaluator_spu_functions_from_type(pEvaluator->GetType(), spu_GetValueFunc, spu_InitializerFunc);
      stream.Put(spu_InitializerFunc);
      stream.Put(spu_GetValueFunc);
      
      return vtablePtr;
   }
};

//------------------------------------------------------------------------------------------
#include "Engine/Math/CRandom.h"

void linear_copy_stream_test(IEvaluator *pEvaluator)
{
   if (!pEvaluator) return;

   uint8 buffer[8192];
   CLinearCopyStream stream(sizeof(buffer), buffer, (uint32) buffer);

   IEvaluator *pCopy = (IEvaluator*) pEvaluator->CopyToLinearStream(stream, *gpMainCPU_EvaluatorTypeToVTable);
   
   IEvaluator::STypeInfo typeInfo;
   pEvaluator->GetInfo(typeInfo);   

   if ((stream.mCurrentBufferPos > 0) && (typeInfo.mClassType == IEvaluator::kECT_Vector))
   {
      IVectorEvaluator const *pEvalCopy = static_cast<IVectorEvaluator const*>(pCopy);
      CRandom  random(99);
      uint8 dataBlockMem[1024];
      CEvaluatorDataBlock dataBlock(dataBlockMem);
      CEvaluatorUpdateData updateData(&random, dataBlock, 0, 0, 0);
      CVector3 val = pEvalCopy->GetValue(updateData);
   }
   if ((stream.mCurrentBufferPos > 0) && (typeInfo.mClassType == IEvaluator::kECT_Float))
   {
      IFloatEvaluator const *pEvalCopy = static_cast<IFloatEvaluator const*>(pCopy);
      CRandom  random(99);
      uint8 dataBlockMem[1024];
      CEvaluatorDataBlock dataBlock(dataBlockMem);
      CEvaluatorUpdateData updateData(&random, dataBlock, 0, 0, 0);
      real32 val = pEvalCopy->GetValue(updateData);
   }
}

//------------------------------------------------------------------------------------------
static CMainCPU_EvaluatorTypeToVTable const  sMainCPU_EvaluatorTypeToVTable;
static CSPU_EvaluatorTypeToVTable const      sSPU_EvaluatorTypeToVTable;

IEvaluatorTypeToVTable const * gpMainCPU_EvaluatorTypeToVTable = &sMainCPU_EvaluatorTypeToVTable;
IEvaluatorTypeToVTable const * gpSPU_EvaluatorTypeToVTable = &sSPU_EvaluatorTypeToVTable;

//------------------------------------------------------------------------------------------

int get_spu_evaluator_buffer_address()
{
#if BPE_TARGET == BPE_TARGET_PS3
   #include "Game/SPU_Particles/SPU_Particles_evaluator_buffer.h"
   return _SPU_Particles_elf_lsSPUEvaluatorBuffer_address;
#else
   return 0;
#endif
}

//------------------------------------------------------------------------------------------

