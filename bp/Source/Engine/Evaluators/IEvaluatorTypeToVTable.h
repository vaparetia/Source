//------------------------------------------------------------------------------------------
// IEvaluatorTypeToVTable.h
// Type to vtable conversion for linear copies of evaluator properties.
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CLinearCopyStream;
class IEvaluatorTypeToVTable;
class IEvaluator;

//------------------------------------------------------------------------------------------

class ENGINE_API IEvaluatorTypeToVTable
{
public:
   // Inserts uint32 ptr to 'GetValue/ApplyModifier' virtual function.
   // Returns buffer 'ptr' to vtable, which is offset so that 'GetValue' points at the correct function.
   // All other vtable entries are invalid as we're only creating a partial vtable with 1 entry.

   // We don't have constructors for evaluators on the SPU, we don't have the original vtables either.
   // This means that we have to build our own vtables for evaluators.

   // i.e.
   // IFloatEvaluator vtable has 7 entries. We only care about the 'GetValue' function.
   // Assume our 'GetValue' function is located at is 0x00008080 on the SPU.
   // Buffer pos at 'InsertFakeVTable' function call is 0x1018. 
   // 
   // Buf pos     virtual function        value in buffer
   // 0x0ffc      ~IEvaluator             undefined
   // 0x1000      Initialize              undefined
   // 0x1004      ApplyProperties         undefined
   // 0x1008      PostLoadUpdate          undefined
   // 0x100c      GetType                 undefined
   // 0x1010      CopyToLinearStream      undefined
   // 0x1014      GetClassType            undefined
   // 0x1018      GetValue                0x00008080  <- Added by InsertFakeVTable
   // 
   // We write 0x00008080 into the buffer, and return 0x00000ffc (ptr to vtable start).
   // The caller uses this value (0x0ffc) to replace the original vtable ptr stored at '*(uint32*)(this)' in the class copy.
   // This relocates the vtable so that it will work on the SPU.
   // Note, this example is SPU specific. Will be different on Win32.
   // Note, we're also ignoring the target offset which would be added to these values.
   // Note, 'undefined' means that the value is not used and is whatever data was stored in the buffer.

   virtual uint32  InsertFakeVTable(CLinearCopyStream &stream, IEvaluator const *pEvaluator) const = 0;
};

//------------------------------------------------------------------------------------------

// This instance can be used by x86 and PPU
extern ENGINE_API IEvaluatorTypeToVTable const * gpMainCPU_EvaluatorTypeToVTable;
// This instance can be used by the SPU only
extern ENGINE_API IEvaluatorTypeToVTable const * gpSPU_EvaluatorTypeToVTable;

// Get address of evaluator buffer on SPU
ENGINE_API int get_spu_evaluator_buffer_address();
//------------------------------------------------------------------------------------------


