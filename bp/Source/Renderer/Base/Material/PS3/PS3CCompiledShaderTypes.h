//----------------------------------------------------------------------------
// PS3CCompiledShaderTypes.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

struct SShaderPassData
{
   enum EFlags
   {
      kColorMaskR                   =  (1 << 0),
      kColorMaskG                   =  (1 << 1),
      kColorMaskB                   =  (1 << 2),
      kColorMaskA                   =  (1 << 3),
      kDepthTest                    =  (1 << 4),
      kAlphaToCoverage              =  (1 << 5),
      kWaitForRenderTargetResult    =  (1 << 6),
      kForceDisableAA               =  (1 << 7)
   };

   SShaderPassData()
   :  mData(0)
   {
      mData |= kColorMaskR|kColorMaskG|kColorMaskB|kColorMaskA;
      mData |= kDepthTest;
   }

   int GetFlag(EFlags const flag) const
   {
      return mData & flag;
   }

   void SetFlag(EFlags const flag, bool const value)
   {
      if( value )
         mData |= flag;
      else
         mData &= ~flag;
   }

#ifndef SPU
   void PutTo(COutputStream & stream) const
   {
      stream.WriteUint32(mData);
   }
#endif

   uint32 mData;
};

struct SShaderStateData
{
   uint16   mSetStateCommandBufferSize;
   uint16   mResetStateCommandBufferSize;
};

struct SShaderFragmentData
{
   uint16   mSize;
   uint16   mFragmentParametersOffset;
   uint16   mFragmentParametersSize;
   uint16   mSamplerParametersOffset;
   uint16   mSamplerParametersSize;
   uint16   mFragmentProgramConfigOffset;
   uint16   mFragmentProgramUCodeOffset;
   uint16   mFragmentProgramUCodeSize;
   uint16   mShaderConstantParametersOffset;
   uint16   mShaderConstantParametersSize;
};

struct SShaderVertexData
{
   uint16   mSize;
   uint16   mPad0;
   uint16   mVertexParametersOffset;
   uint16   mVertexParametersSize;
   uint16   mVertexProgramConfigOffset;
   uint16   mVertexProgramUCodeOffset;
   uint16   mShaderConstantParametersOffset;
   uint16   mShaderConstantParametersSize;
};

struct SShaderPassHeader
{
   SShaderPassData   mPassData;
   uint32            mStateDataOffset;
   uint32            mFragmentDataOffset;
   uint32            mVertexDataOffset;
   uint16            mCommandBufferHoleSize;
   uint16            mPad0;
};

struct SShaderCombinationHeader
{
   uint32      mPassCount;
   uint32      mPassOffsets[1]; // actually up to mPassCount number of offsets
};

struct SShaderHashEntry
{
   uint32      mHash;
   uint32      mOffset;
};

struct SShaderPackageHeader
{
   uint32      mFourCC;
   uint32      mVersion;
   uint32      mCombinationCount;
   uint32      mCombinationsOffset;
   uint32      mHashTableOffset;
   uint32      mFragmentDataOffset;
   uint32      mVertexDataOffset;
   uint32      mStateDataOffset;
};
