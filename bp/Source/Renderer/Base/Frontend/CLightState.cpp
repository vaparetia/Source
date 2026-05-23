//------------------------------------------------------------------------------------------
// CLightState.cpp
// Bluepoint
// Copyright 2004
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CLightState.h"

//------------------------------------------------------------------------------------------

#include "Engine/Graphics/CSphericalHarmonicsHelper.h"
#include "Engine/Graphics/CLight.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

CLightState::CLightState()
: mAmbient(CVector3::kConstructUninitialized)
, mAmbientOcclusion(CVector3::kConstructUninitialized)
{
   Reset();
}

//------------------------------------------------------------------------------------------

CLightState::~CLightState()
{
}

//------------------------------------------------------------------------------------------

void CLightState::Reset()
{
   mDynamicLights.clear();
   mStaticLights.clear();
   mAmbient = CVector3::Zero();
   mAmbientOcclusion = CVector3::Zero();
   mDynamicLightsTextureDir.reset();
   mDynamicLightsTextureColor.reset();
}

//------------------------------------------------------------------------------------------

void CLightState::AddLight( CLight const * const pLight )
{
   if (pLight->GetType() == CLight::kType_Occlusion)
   {
      mAmbientOcclusion += pLight->GetColor();
   }
   else if (pLight->GetType() == CLight::kType_Ambient)
   {
      mAmbient += pLight->GetColor();
   }
   else if (pLight->IsStaticLight())
   {
      if (mStaticLights.size() < mStaticLights.capacity())
      {
         mStaticLights.push_back(pLight);
      }
   }
   else
   {
      // Must be a dynamic light
      if (mDynamicLights.size() < mDynamicLights.capacity())
      {
         mDynamicLights.push_back(pLight);
      }
   }
}

//------------------------------------------------------------------------------------------

int CLightState::GetNumLights() const
{
   return mStaticLights.size() + mDynamicLights.size();
}

//------------------------------------------------------------------------------------------

CLight const * CLightState::GetLight(int const index) const
{
   // Return static lights first
   if (index >= mStaticLights.size())
   {
      return mDynamicLights[index - mStaticLights.size()];
   }
   return mStaticLights[index];
}

//------------------------------------------------------------------------------------------

CLightSets::CLightSets()
{
   // Initialize to max combination of lightsets
   mLightSets.resize(mLightSets.capacity(), CLightState());
}

//------------------------------------------------------------------------------------------

CLightSets::~CLightSets()
{
}

//------------------------------------------------------------------------------------------

void CLightSets::Reset()
{
   foreach(CLightState &state, mLightSets)
   {
      state.Reset();
   }
}

//------------------------------------------------------------------------------------------


