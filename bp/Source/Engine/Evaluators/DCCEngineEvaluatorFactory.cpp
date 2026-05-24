//----------------------------------------------------------------------------
// DCCEngineEvaluatorFactory.cpp
// Minimal evaluator factory stub for Dreamcast.
// No asset loading in Phase 5, so all Build* methods return NULL.
//----------------------------------------------------------------------------

#ifdef __DREAMCAST__

#include "Engine/StdAfx.h"
#include "Engine/Evaluators/CEngineEvaluatorFactory.h"

class DCEvaluatorFactory : public IEvaluatorFactory
{
public:
   virtual IFloatEvaluator *      BuildFloatEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IBoolEvaluator *       BuildBoolEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IVectorEvaluator *     BuildVectorEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IColorEvaluator *      BuildColorEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IModifierPVEvaluator * BuildModifierPVEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IEmitterPVEvaluator *  BuildEmitterPVEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual IUserDataEvaluator *   BuildUserDataEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
   virtual ICustomEvaluator *     BuildCustomEvaluator(uint32 const, int const, CInputStream &, CEvaluatorAllocator &) const { return NULL; }
};

static DCEvaluatorFactory sDCEvaluatorFactory;

IEvaluatorFactory & GetEngineEvaluatorFactory()
{
   return sDCEvaluatorFactory;
}

#endif // __DREAMCAST__
