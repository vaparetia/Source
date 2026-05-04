//------------------------------------------------------------------------------------------
// IEvaluatorFactory.h
// Interface class for building evaluators
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class IFloatEvaluator;
class IBoolEvaluator;
class IVectorEvaluator;
class IColorEvaluator;
class IModifierPVEvaluator;
class IEmitterPVEvaluator;
class ICustomEvaluator;
class IUserDataEvaluator;
class CInputStream;
class CEvaluatorAllocator;

//------------------------------------------------------------------------------------------

class ENGINE_API IEvaluatorFactory
{
public:
   virtual ~IEvaluatorFactory() {};

   virtual IFloatEvaluator *        BuildFloatEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IBoolEvaluator *         BuildBoolEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IVectorEvaluator *       BuildVectorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IColorEvaluator *        BuildColorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IModifierPVEvaluator *   BuildModifierPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IEmitterPVEvaluator *    BuildEmitterPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual IUserDataEvaluator *     BuildUserDataEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
   virtual ICustomEvaluator *       BuildCustomEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const = 0;
private:
};

//------------------------------------------------------------------------------------------

