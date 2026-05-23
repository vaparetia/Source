//------------------------------------------------------------------------------------------
// IEvaluator.cpp
// Interface classes for evaluators
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "Engine/Evaluators/IEvaluator.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"

//------------------------------------------------------------------------------------------

IEvaluator::STypeInfo::STypeInfo()
{
   mTimeRangeStart = gkReal32Min;
   mTimeRangeEnd = gkReal32Max;
   mClassType = kECT_Invalid;
   mIsConst = false;
}

//------------------------------------------------------------------------------------------

IEvaluator::STypeInfo::~STypeInfo()
{
}

//------------------------------------------------------------------------------------------

bool IEvaluator::UpdateProperties(  IEvaluator * pEvaluator,
                                    CGameObjectApplyPropertiesData &applyProperties, 
                                    uint16 const size,
                                    CEvaluatorAllocator &allocator)
{
   uint32 const type = applyProperties.mStream.ReadUint32();

   if (!pEvaluator)
   {
      // NULL evaluator, don't update
      applyProperties.mStream.Get(NULL, size - 4);
      return false;
   }

   if (pEvaluator->GetType() != type)
   {
      // Skip data
      applyProperties.mStream.Get(NULL, size - 4);
      return false;
   }

   pEvaluator->ApplyProperties(applyProperties, allocator);

   return true;
}

//------------------------------------------------------------------------------------------

void IFloatEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Float;
}

//------------------------------------------------------------------------------------------

void IBoolEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Bool;
}

//------------------------------------------------------------------------------------------

void IVectorEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Vector;
}

//------------------------------------------------------------------------------------------

void IColorEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Color;
}

//------------------------------------------------------------------------------------------

void IModifierPVEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Modifier;
}

//------------------------------------------------------------------------------------------

void IEmitterPVEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Emitter;
}

//------------------------------------------------------------------------------------------

void ICustomEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_Custom;
}
//------------------------------------------------------------------------------------------

void IUserDataEvaluator::GetInfo(STypeInfo &typeInfo) const
{
   typeInfo = STypeInfo();
   typeInfo.mClassType = kECT_UserData;
}

//------------------------------------------------------------------------------------------

CSimpleModifierTemporaryAllocator::~CSimpleModifierTemporaryAllocator()
{
   delete mpDeleteMemPtr;
}

//------------------------------------------------------------------------------------------

int32 CSimpleModifierTemporaryAllocator::Allocate(int32 const size)
{
   BPE_VERIFY((mMemRemaining - size) >= 0, false, "Temporary allocator out of buffer space.");

   int32 const offset = (int32) (mpCurrentMemPtr - mpOriginalMemPtr);
   mpCurrentMemPtr += size;
   mMemRemaining -= size;

   // Return offset
   return offset;
}

//------------------------------------------------------------------------------------------

