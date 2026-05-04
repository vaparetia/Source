//----------------------------------------------------------------------------
// CCGEffect.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CCGEffect.h"

//----------------------------------------------------------------------------

#include "Engine/Mechanics/TTokenSet.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

static TTokenSet<EState> skStateTokens[] =
{
   { "AlphaBlendEnable",   kState_AlphaBlendEnable    },
   { "AlphaTestEnable",    kState_AlphaTestEnable     },
   { "AlphaRef",           kState_AlphaRef            },
   { "AlphaFunc",          kState_AlphaFunc           },
   { "BlendFunc",          kState_BlendFunc           },
   { "BlendFuncSeparate",  kState_BlendFuncSeparate   },
   { "ZEnable",            kState_ZEnable             },
   { "ZWriteEnable",       kState_ZWriteEnable        },
   { "ColorMask",          kState_ColorMask           },
   { "VertexShader",       kState_VertexShader        },
   { "PixelShader",        kState_PixelShader         },
   { "CullFaceEnable",     kState_CullFaceEnable      },
   { "CullFace",           kState_CullFace            },

   { "Texture",            kState_Texture             },
   { "MinFilter",          kState_MinFilter           },
   { "MagFilter",          kState_MagFilter           },
   { "MipFilter",          kState_MipFilter           },
   { "AddressU",           kState_WrapU               },
   { "AddressV",           kState_WrapV               },
   { "AddressW",           kState_WrapW               },
   { "Convolution",        kState_Convolution         },

   { NULL,                 kState_Invalid             }
};

//----------------------------------------------------------------------------

CCGEffectBase::CCGEffectBase(std::string const & name, std::string const & semantic)
:  mName(name)
,  mSemantic(semantic)
{
}

//----------------------------------------------------------------------------

CCGEffectState::CCGEffectState(std::string const & name, std::string const & semantic)
:  CCGEffectBase(name, semantic)
,  mState(skStateTokens->GetTokenNoCase(name.c_str()))
{
}

//----------------------------------------------------------------------------

CCGEffectState::~CCGEffectState()
{
}

//----------------------------------------------------------------------------

CCGEffectData::CCGEffectData(EType type)
:  mType(type)
{
}

//----------------------------------------------------------------------------

CCGEffectData::~CCGEffectData()
{
}

//----------------------------------------------------------------------------

CCGEffectDataNumeric::CCGEffectDataNumeric(EType type)
:  CCGEffectData(type)
{
   memset(&mData8, 0, sizeof(mData8));
   memset(&mData32, 0, sizeof(mData32));
}

//----------------------------------------------------------------------------

CCGEffectDataNumeric::~CCGEffectDataNumeric()
{
}

//----------------------------------------------------------------------------

CCGEffectDataProgram::CCGEffectDataProgram(std::vector<CCGEffectNamedData> const & programParameters)
:  CCGEffectData(kType_Program)
,  mProgramParameters(programParameters)
,  mData()
,  mSize(0)
{
}

void CCGEffectDataProgram::SetBinaryShader(uint32 size, const char * pData)
{
   mSize = size;

   mData.reset(new char[mSize]);
   memcpy(mData.get(), pData, mSize);
}

//----------------------------------------------------------------------------

CCGEffectDataProgram::~CCGEffectDataProgram()
{
}

//----------------------------------------------------------------------------

CCGEffectDataString::CCGEffectDataString(std::string const & data)
:  CCGEffectData(kType_String)
,  mData(data)
{
}

//----------------------------------------------------------------------------

CCGEffectDataString::~CCGEffectDataString()
{
}

//----------------------------------------------------------------------------

CCGEffectStateAssignment::CCGEffectStateAssignment(CCGEffectState const & state,
                                                   CCGEffectData* pData)
:  mState(state)
,  mpData(pData)
{
}

//----------------------------------------------------------------------------

CCGEffectStateAssignment::~CCGEffectStateAssignment()
{
}

//----------------------------------------------------------------------------

CCGEffectStateAssignmentContainer::CCGEffectStateAssignmentContainer()
{
}

//----------------------------------------------------------------------------

CCGEffectStateAssignment const * CCGEffectStateAssignmentContainer::GetByName(std::string const & name) const
{
   foreach(CCGEffectStateAssignment const & stateAssignment, mData)
   {
      if( stateAssignment.mState.mName == name )
         return &stateAssignment;
   }

   return NULL;
}



//----------------------------------------------------------------------------

CCGEffectNamedData::CCGEffectNamedData(std::string const & name,
                                         CCGEffectData* pData)
:  mName(name)
,  mpData(pData)
{
}

//----------------------------------------------------------------------------

CCGEffectNamedData::~CCGEffectNamedData()
{
}

//----------------------------------------------------------------------------

CCGEffectTexture::CCGEffectTexture(std::string const & name,
                                   std::string const & semantic,
                                   std::vector<CCGEffectNamedData> const & annotations)
:  CCGEffectBase(name, semantic)
,  mAnnotations(annotations)
{
}

//----------------------------------------------------------------------------

CCGEffectTexture::~CCGEffectTexture()
{
}

//----------------------------------------------------------------------------

CCGEffectSampler::CCGEffectSampler(std::string const & name,
                                   std::string const & semantic,
                                   CCGEffectStateAssignmentContainer const & stateAssignments)
:  CCGEffectBase(name, semantic)
,  mStateAssignments(stateAssignments)
{
}

//----------------------------------------------------------------------------

CCGEffectSampler::~CCGEffectSampler()
{
}

//----------------------------------------------------------------------------

CCGEffectPass::CCGEffectPass(std::string const & name,
                             std::string const & semantic,
                             CCGEffectStateAssignmentContainer const & stateAssignments,
                             std::vector<CCGEffectNamedData> const & annotations)
:  CCGEffectBase(name, semantic)
,  mStateAssignments(stateAssignments)
,  mAnnotations(annotations)
{
}

//----------------------------------------------------------------------------

CCGEffectPass::~CCGEffectPass()
{
}

//----------------------------------------------------------------------------

CCGEffectTechnique::CCGEffectTechnique(std::string const & name,
                                       std::string const & semantic,
                                       std::vector<CCGEffectPass> const & passes)
:  CCGEffectBase(name, semantic)
,  mPasses(passes)
{
}

//----------------------------------------------------------------------------

CCGEffectTechnique::~CCGEffectTechnique()
{
}

//----------------------------------------------------------------------------

CCGEffectCombination::CCGEffectCombination(uint32 hash,
                                           std::vector<CCGEffectTechnique> const & techniques,
                                           std::vector<CCGEffectSampler> const & samplers,
                                           std::vector<CCGEffectTexture> const & textures)
:  mHash(hash)
,  mTechniques(techniques)
,  mSamplers(samplers)
,  mTextures(textures)
{
}

//----------------------------------------------------------------------------

CCGEffectCombination::~CCGEffectCombination()
{
}

//----------------------------------------------------------------------------

CCGEffectSampler const * CCGEffectCombination::GetSamplerByName(char const * const pName) const
{
   foreach(CCGEffectSampler const & sampler, mSamplers)
   {
      if( strcmp(sampler.mName.c_str(), pName) == 0)
         return &sampler;
   }

   return NULL;
}

//----------------------------------------------------------------------------

CCGEffectTexture const * CCGEffectCombination::GetTextureByName(char const * const pName) const
{
   foreach(CCGEffectTexture const & texture, mTextures)
   {
      if( strcmp(texture.mName.c_str(), pName) == 0)
         return &texture;
   }

   return NULL;
}

//----------------------------------------------------------------------------

CCGEffect::CCGEffect(std::vector<CCGEffectCombination> const & combinations)
:  mCombinations(combinations)
{
}

//----------------------------------------------------------------------------

CCGEffect::~CCGEffect()
{
}

//----------------------------------------------------------------------------

CCGEffectCombination const * CCGEffect::GetCombination(uint32 hash) const
{
   foreach(CCGEffectCombination const & combination, mCombinations )
   {
      if( combination.mHash == hash )
         return &combination;
   }

   return NULL;
}

//----------------------------------------------------------------------------

CCGEffectCombination * CCGEffect::Combination(uint32 hash)
{
   return const_cast<CCGEffectCombination*>(GetCombination(hash));
}