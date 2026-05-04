//----------------------------------------------------------------------------
// X360CookedShaderFormat.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "X360CookedShaderFormat.h"

#include "Engine/Streams/CMemoryInputStream.h"
#include "Engine/Resource/CResourceFactory.h"

//----------------------------------------------------------------------------

SParam_X360::SParam_X360(CInputStream & stream)
: mCRC(stream.ReadUint32())
, mContext(stream.ReadUint32())
, mVertexRegisterIndex(stream.ReadUint8())
, mVertexRegisterCount(stream.ReadUint8())
, mPixelRegisterIndex(stream.ReadUint8())
, mPixelRegisterCount(stream.ReadUint8())
, paramName(stream.ReadString())
{
}

void SParam_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mCRC);
   stream.Put((uint32)mContext);
   stream.Put(mVertexRegisterIndex);
   stream.Put(mVertexRegisterCount);
   stream.Put(mPixelRegisterIndex);
   stream.Put(mPixelRegisterCount);
   stream.Put(paramName);
}

//----------------------------------------------------------------------------

SamplerState_X360::SamplerState_X360(CInputStream & stream)
: mRegister(stream.ReadUint32())
, mType(stream.ReadUint32())
, mValue(stream.ReadUint32())
{
}

void SamplerState_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mRegister);
   stream.Put(mType);
   stream.Put(mValue);
}

//----------------------------------------------------------------------------

RenderState_X360::RenderState_X360(CInputStream & stream)
: mType(stream.ReadUint32())
, mValue(stream.ReadUint32())
{
}

void RenderState_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mType);
   stream.Put(mValue);
}

//----------------------------------------------------------------------------

VertexShader_X360::VertexShader_X360(CInputStream & stream)
: mVertexShaderCode(stream)
, mHash(stream.ReadUint32())
{
}

void VertexShader_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mVertexShaderCode);
   stream.Put(mHash);
}

//----------------------------------------------------------------------------

PixelShader_X360::PixelShader_X360(CInputStream & stream)
: mPixelShaderCode(stream)
, mHash(stream.ReadUint32())
{
}

void PixelShader_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mPixelShaderCode);
   stream.Put(mHash);
}

//----------------------------------------------------------------------------

Params_X360::Params_X360(CInputStream & stream)
: mParams(stream)
, mHash(stream.ReadUint32())
{
}

void Params_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mParams);
   stream.Put(mHash);
}

//----------------------------------------------------------------------------

SamplerStates_X360::SamplerStates_X360(CInputStream & stream)
: mSamperStates(stream)
, mSamperStates_Restore(stream)
, mHash(stream.ReadUint32())
{
}

void SamplerStates_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mSamperStates);
   stream.Put(mSamperStates_Restore);
   stream.Put(mHash);
}

//----------------------------------------------------------------------------

RenderStates_X360::RenderStates_X360(CInputStream & stream)
: mRenderStates(stream)
, mRenderStates_Restore(stream)
, mHash(stream.ReadUint32())
{
}

void RenderStates_X360::PutTo(COutputStream & stream) const
{
   stream.Put(mRenderStates);
   stream.Put(mRenderStates_Restore);
   stream.Put(mHash);
}

//----------------------------------------------------------------------------

Pass_360::Pass_360(CInputStream & stream)
: mVertexShader_Index(stream.ReadInt32())
, mPixelShader_Index(stream.ReadInt32())
, mSamperStates_Index(stream.ReadInt32())
, mRenderStates_Index(stream.ReadInt32())
, mParams_Index(stream.ReadInt32())
{
}

void Pass_360::PutTo(COutputStream & stream) const
{
   stream.Put(mVertexShader_Index);
   stream.Put(mPixelShader_Index);
   stream.Put(mSamperStates_Index);
   stream.Put(mRenderStates_Index);
   stream.Put(mParams_Index);
}

//----------------------------------------------------------------------------

Technique_360::Technique_360(CInputStream & stream)
: mName(stream.ReadString())
, mPasses(stream)
{
}

void Technique_360::PutTo(COutputStream & stream) const
{
   stream.Put(mName);
   stream.Put(mPasses);
}

//----------------------------------------------------------------------------

EffectCombination_360::EffectCombination_360(CInputStream & stream)
: mHash(stream.ReadUint32())
, mTechniques(stream)
{
}

void EffectCombination_360::PutTo(COutputStream & stream) const
{
   stream.Put(mHash);
   stream.Put(mTechniques);
}


//----------------------------------------------------------------------------

Effect_360::Effect_360(CInputStream & stream)
: mName(stream.ReadString())
, mCombinations(stream)
, mVertexShader(stream)
, mPixelShader(stream)
, mSamperStates(stream)
, mRenderStates(stream)
, mParams(stream)
{
}

void Effect_360::PutTo(COutputStream & stream) const
{
   stream.WriteUint32('EFCT');
   stream.Put(mName);
   stream.Put(mCombinations);
   stream.Put(mVertexShader);
   stream.Put(mPixelShader);
   stream.Put(mSamperStates);
   stream.Put(mRenderStates);
   stream.Put(mParams);
}

//----------------------------------------------------------------------------

EffectCombination_360 const * Effect_360::GetCombination(uint32 hash) const
{
   foreach(EffectCombination_360 const & combination, mCombinations )
   {
      if( combination.mHash == hash )
         return &combination;
   }

   return NULL;
}

EffectCombination_360 * Effect_360::Combination(uint32 hash)
{
   return const_cast<EffectCombination_360*>(GetCombination(hash));
}

//----------------------------------------------------------------------------

void Effect_360::FEffectFactory(SFactoryResourceBuildData & buildData, SFactoryReturnResource & returnResource)
{
   CMemoryInputStream stream(buildData.mpMemory, buildData.mSize, CMemoryInputStream::kOwner_App);
   
   uint32 fourCC = stream.ReadUint32();
   BPE_VERIFY(fourCC == 'EFCT', false, "Invalid effect resource");

   returnResource.mpResource = new Effect_360(stream);
}
