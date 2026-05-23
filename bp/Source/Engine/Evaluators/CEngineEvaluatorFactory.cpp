//------------------------------------------------------------------------------------------
// CEngineEvaluatorFactory.cpp
// Interface class for building engine evaluators
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "CEngineEvaluatorFactory.h"
#include "IEvaluator.h"
#include "IEvaluatorFactory.h"
#include "CRegisteredEvaluatorFactories.h"
#include "Engine/GameObjectSystem/CGameObjectApplyPropertiesData.h"

#if !defined(SPU)
// Not required for SPU

//------------------------------------------------------------------------------------------
// All engine evaluators added to these
CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gboolEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gmodifierPVEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gemitterPVEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories gcustomEvaluatorFactoriesEngine;
CRegisteredEvaluatorFactories guserDataEvaluatorFactoriesEngine;

//------------------------------------------------------------------------------------------

IFloatEvaluator * CEngineEvaluatorFactory::BuildFloatEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gfloatEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IFloatEvaluator * const>(pEvaluator);
}
 
//------------------------------------------------------------------------------------------

IBoolEvaluator * CEngineEvaluatorFactory::BuildBoolEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gboolEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IBoolEvaluator * const>(pEvaluator);
}
 
//------------------------------------------------------------------------------------------

IVectorEvaluator * CEngineEvaluatorFactory::BuildVectorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gvector3EvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   // Disable testing code.
   return static_cast<IVectorEvaluator * const>(pEvaluator);
}
 
//------------------------------------------------------------------------------------------

IColorEvaluator * CEngineEvaluatorFactory::BuildColorEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gcolorEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IColorEvaluator * const>(pEvaluator);
}

//------------------------------------------------------------------------------------------

IModifierPVEvaluator * CEngineEvaluatorFactory::BuildModifierPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gmodifierPVEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IModifierPVEvaluator * const>(pEvaluator);
}

//------------------------------------------------------------------------------------------

IEmitterPVEvaluator * CEngineEvaluatorFactory::BuildEmitterPVEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gemitterPVEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IEmitterPVEvaluator * const>(pEvaluator);
}

//------------------------------------------------------------------------------------------

ICustomEvaluator * CEngineEvaluatorFactory::BuildCustomEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = gcustomEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<ICustomEvaluator * const>(pEvaluator);
}

//------------------------------------------------------------------------------------------

IUserDataEvaluator * CEngineEvaluatorFactory::BuildUserDataEvaluator(uint32 const type, int const size, CInputStream &inStream, CEvaluatorAllocator &allocator) const
{
   IEvaluator * const pEvaluator = guserDataEvaluatorFactoriesEngine.BuildEvaluator(type, inStream, allocator, *this);
   // Skip evaluator data if we couldn't build it
   if (!pEvaluator) inStream.Get(NULL, size - 4); // Skip size associated with type
   return static_cast<IUserDataEvaluator * const>(pEvaluator);
}

//------------------------------------------------------------------------------------------
// Create instance of factory
static CEngineEvaluatorFactory gEngineEvalutorFactory;

//------------------------------------------------------------------------------------------

IEvaluatorFactory & GetEngineEvaluatorFactory()
{
   return gEngineEvalutorFactory;
}

#endif //!defined(SPU)

//------------------------------------------------------------------------------------------

#include "boost/scoped_ptr.hpp"
#include "CEvaluatorAllocator.h"
#include "CEvaluatorUpdateData.h"
#include "IEvaluatorTypeToVTable.h"
#include "Engine/Math/CRandom.h"
#include "Engine/Math/MathUtils.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/Math/CPerlinNoise.h"
#include "Engine/Graphics/CColorf.h"
#include "Engine/Streams/CLinearCopyStream.h"

//------------------------------------------------------------------------------------------
#if defined(SPU)
// Disable unneeded code on SPUs
#define NO_EVALUATOR_REGISTRATION
#define NO_EVALUATOR_CONSTRUCTOR
#define NO_EVALUATOR_TYPE
#define NO_EVALUATOR_LOADER

#endif
//------------------------------------------------------------------------------------------


#include "Float/CFEInitialRandom_float.h"
#include "Float/CFERandom_float.h"
#include "Float/CFEConst_float.h"
#include "Float/CFETimeProvider_float.h"
#include "Float/CFESpline_float.h"
#include "Float/CFEPulse_float.h"
#include "Float/CFESampleAndHold_float.h"
#include "Float/CFEMathOperator_float.h"
#include "Float/CFEInitialRandomS_float.h"
#include "Float/CFERandomS_float.h"
#include "Float/CFEInitialRandomV_float.h"
#include "Float/CFERandomV_float.h"
#include "Float/CFEInitialRandomVS_float.h"
#include "Float/CFERandomVS_float.h"
#include "Float/CFEInitialGaussianRandomS_float.h"
#include "Float/CFEGaussianRandomS_float.h"
#include "Float/CFEInitialGaussianRandom_float.h"
#include "Float/CFEGaussianRandom_float.h"
#include "Float/CFEKeepInitial_float.h"
#include "Float/CFETimeProviderEx_float.h"
#include "Float/CFESin_float.h"
#include "Float/CFESplineCR3_float.h"
#include "Float/CFESplineCR4_float.h"
#include "Float/CFEInitialRandomBinary_float.h"
#include "Float/CFERandomBinary_float.h"
#include "Float/CFEClampMinMax_float.h"
#include "Bool/CBEConvertFloatToBool_bool.h"
#include "Vector3/CVEConst_vector.h"
#include "Vector3/CVEBuildVector.h"
#include "Vector3/CVERandom_vector.h"
#include "Vector3/CVEInitialRandom_vector.h"
#include "Vector3/CVECircle_vector.h"
#include "Vector3/CVECircleTangent_vector.h"
#include "Vector3/CVESpline_vector.h"
#include "Vector3/CVESampleAndHold_vector.h"
#include "Vector3/CVEBuildFromFloat_vector.h"
#include "Color/CCEConst_color.h"
#include "Color/CCEBuildColorRGBA.h"
#include "Color/CCEInitialRandomHSVS_color.h"
#include "Color/CCEScaleIA_color.h"
#include "Color/CCEScaleRGBA_color.h"
#include "Color/CCEMathOperator_color.h"
#include "Color/CCESpline_color.h"
#include "Color/CCEKeepInitial_color.h"
#include "Color/CCEBuildColorHSVA.h"
#include "Color/CCESplineCR3_color.h"
#include "Color/CCESplineCR4_color.h"
#include "ModifierPV/CMPVEWind_modifierPV.h"
#include "ModifierPV/CMPVEGravity_modifierPV.h"
#include "ModifierPV/CMPVESwirl_modifierPV.h"
#include "ModifierPV/CMPVETurbulence_modifierPV.h"
#include "ModifierPV/CMPVEAttractor_modifierPV.h"
#include "ModifierPV/CMPVEVelocityDamp_modifierPV.h"
#include "EmitterPV/CEPVESphere_emitterPV.h"
#include "EmitterPV/CEPVEAngleSphere_emitterPV.h"
#include "EmitterPV/CEPVESimple_emitterPV.h"
#include "EmitterPV/CEPVEDisc_emitterPV.h"
#include "./float/CFERangeScale_float.h"
#include "./float/CFEFollowValue_float.h"
#include "./float/CFESmoothStep_float.h"
#include "./color/CCEInterpolateSpeed_color.h"
#include "./vector3/CVEMathOperator_vector.h"
#include "./float/CFEAccumulate_float.h"
#include "./userData/CUDEFloat_userData.h"
#include "./userData/CUDEString_userData.h"
#include "./userData/CUDEBranch2_userData.h"
#include "./vector3/CVEGaussianRandom_vector.h"
#include "./float/CFETimeStep_float.h"
#include "./float/CFEDistance_float.h"
//#CodeGenIncludeTag# - Do not modify this comment, used by code gen.

//------------------------------------------------------------------------------------------

#include "Float/CFEInitialRandom_float.cpp"
#include "Float/CFERandom_float.cpp"
#include "Float/CFEConst_float.cpp"
#include "Float/CFETimeProvider_float.cpp"
#include "Float/CFESpline_float.cpp"
#include "Float/CFEPulse_float.cpp"
#include "Float/CFESampleAndHold_float.cpp"
#include "Float/CFEMathOperator_float.cpp"
#include "Float/CFEInitialRandomS_float.cpp"
#include "Float/CFERandomS_float.cpp"
#include "Float/CFEInitialRandomV_float.cpp"
#include "Float/CFERandomV_float.cpp"
#include "Float/CFEInitialRandomVS_float.cpp"
#include "Float/CFERandomVS_float.cpp"
#include "Float/CFEInitialGaussianRandomS_float.cpp"
#include "Float/CFEGaussianRandomS_float.cpp"
#include "Float/CFEInitialGaussianRandom_float.cpp"
#include "Float/CFEGaussianRandom_float.cpp"
#include "Float/CFEKeepInitial_float.cpp"
#include "Float/CFETimeProviderEx_float.cpp"
#include "Float/CFESin_float.cpp"
#include "Float/CFESplineCR3_float.cpp"
#include "Float/CFESplineCR4_float.cpp"
#include "Float/CFEInitialRandomBinary_float.cpp"
#include "Float/CFERandomBinary_float.cpp"
#include "Float/CFEClampMinMax_float.cpp"
#include "Bool/CBEConvertFloatToBool_bool.cpp"
#include "Vector3/CVEConst_vector.cpp"
#include "Vector3/CVEBuildVector.cpp"
#include "Vector3/CVERandom_vector.cpp"
#include "Vector3/CVEInitialRandom_vector.cpp"
#include "Vector3/CVECircle_vector.cpp"
#include "Vector3/CVECircleTangent_vector.cpp"
#include "Vector3/CVESpline_vector.cpp"
#include "Vector3/CVESampleAndHold_vector.cpp"
#include "Vector3/CVEBuildFromFloat_vector.cpp"
#include "Color/CCEConst_color.cpp"
#include "Color/CCEBuildColorRGBA.cpp"
#include "Color/CCEInitialRandomHSVS_color.cpp"
#include "Color/CCEScaleRGBA_color.cpp"
#include "Color/CCEScaleIA_color.cpp"
#include "Color/CCEMathOperator_color.cpp"
#include "Color/CCESpline_color.cpp"
#include "Color/CCEKeepInitial_color.cpp"
#include "Color/CCEBuildColorHSVA.cpp"
#include "Color/CCESplineCR3_color.cpp"
#include "Color/CCESplineCR4_color.cpp"
#include "ModifierPV/CMPVEWind_modifierPV.cpp"
#include "ModifierPV/CMPVEGravity_modifierPV.cpp"
#include "ModifierPV/CMPVESwirl_modifierPV.cpp"
#include "ModifierPV/CMPVETurbulence_modifierPV.cpp"
#include "ModifierPV/CMPVEAttractor_modifierPV.cpp"
#include "ModifierPV/CMPVEVelocityDamp_modifierPV.cpp"
#include "EmitterPV/CEPVESphere_emitterPV.cpp"
#include "EmitterPV/CEPVEAngleSphere_emitterPV.cpp"
#include "EmitterPV/CEPVESimple_emitterPV.cpp"
#include "EmitterPV/CEPVEDisc_emitterPV.cpp"
#include "./float/CFERangeScale_float.cpp"
#include "./float/CFEFollowValue_float.cpp"
#include "./float/CFESmoothStep_float.cpp"
#include "./color/CCEInterpolateSpeed_color.cpp"
#include "./vector3/CVEMathOperator_vector.cpp"
#include "./float/CFEAccumulate_float.cpp"
#include "./userData/CUDEFloat_userData.cpp"
#include "./userData/CUDEString_userData.cpp"
#include "./userData/CUDEBranch2_userData.cpp"
#include "./vector3/CVEGaussianRandom_vector.cpp"
#include "./float/CFETimeStep_float.cpp"
#include "./float/CFEDistance_float.cpp"
//#CodeGenCppTag# - Do not modify this comment, used by code gen.

//------------------------------------------------------------------------------------------
