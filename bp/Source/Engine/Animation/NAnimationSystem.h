//----------------------------------------------------------------------------
// NAnimationSystem.h
// Bluepoint
// Copyright 2009
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

namespace NAnimationSystem
{
   enum ERootMotionExtraction
   {
      kRootMotionExtraction_None          = 0,

      kRootMotionExtraction_RotationX     = 1 << 0,
      kRootMotionExtraction_RotationY     = 1 << 1,
      kRootMotionExtraction_RotationZ     = 1 << 2,
      kRootMotionExtraction_Rotation      = kRootMotionExtraction_RotationX | kRootMotionExtraction_RotationY | kRootMotionExtraction_RotationZ,

      kRootMotionExtraction_TranslationX  = 1 << 3,
      kRootMotionExtraction_TranslationY  = 1 << 4,
      kRootMotionExtraction_TranslationZ  = 1 << 5,
      kRootMotionExtraction_Translation   = kRootMotionExtraction_TranslationX | kRootMotionExtraction_TranslationY | kRootMotionExtraction_TranslationZ,

      kRootMotionExtraction_All           = kRootMotionExtraction_Rotation | kRootMotionExtraction_Translation
   };
}