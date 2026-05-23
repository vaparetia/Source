//------------------------------------------------------------------------------------------
// CEngineEvaluatorFactory.h
// Interface class for building engine evaluators
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "IEvaluatorFactory.h"

//------------------------------------------------------------------------------------------
// You can derive from this to add your own evaluators
class ENGINE_API CEngineEvaluatorFactory : public IEvaluatorFactory
{
public:
   virtual IFloatEvaluator *        BuildFloatEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IBoolEvaluator *         BuildBoolEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IVectorEvaluator *       BuildVectorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IColorEvaluator *        BuildColorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IModifierPVEvaluator *   BuildModifierPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IEmitterPVEvaluator *    BuildEmitterPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual IUserDataEvaluator *     BuildUserDataEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
   virtual ICustomEvaluator *       BuildCustomEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const;
};

//------------------------------------------------------------------------------------------
// Get an instance of the factory if you don't need to derive from it
ENGINE_API IEvaluatorFactory & GetEngineEvaluatorFactory();

//------------------------------------------------------------------------------------------
