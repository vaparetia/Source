//------------------------------------------------------------------------------------------
// EvaluatorUtils.h
// Helper functions for evaluators
// Bluepoint
// Copyright 2009
//------------------------------------------------------------------------------------------

#include "Engine/Evaluators/IEvaluator.h"
#include <boost/optional.hpp>

//------------------------------------------------------------------------------------------

namespace EvaluatorUtils
{
   ENGINE_API boost::optional<real32>  GetFloatFromUserData(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier);
   // This version returns 0.0f if a float type associated with the evaluator tree cannot be found.
   ENGINE_API real32                   GetFloatFromUserData_Always(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier);
   
   ENGINE_API char const *             GetStringFromUserData(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier);
   // This version returns an empty string if a float type associated with the evaluator tree cannot be found.
   ENGINE_API char const *             GetStringFromUserData_Always(CEvaluatorUpdateData const &updateData, IUserDataEvaluator const * pEvaluator, char const * const pIdentifier);   
   
} // namespace EvaluatorUtils

