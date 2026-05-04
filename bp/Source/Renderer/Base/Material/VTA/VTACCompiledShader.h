//----------------------------------------------------------------------------
// VTACCompiledShader.h
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

#include "Engine\Mechanics\IObject.h"

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

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

#if 0
struct SShaderStateData
{
   uint16   mSetStateCommandBufferSize;
   uint16   mResetStateCommandBufferSize;
};
#endif

// Paired with Tools\AssetToolMPP\Shader\ShaderBinaryWriterVTA.cpp
struct SVertexParameter
{
   uint32   mCRC;
   uint8    mParameterIndex;
   uint8    mBackendRegister;
   uint8    mTotalFloatParameterCount;
   uint8    mPad;
};

// Paired with Tools\AssetToolMPP\Shader\ShaderBinaryWriterVTA.cpp
struct SVertexSemantic
{
   uint8    mVDUType; // One of kVDU_, or 0xFF if invalid
   uint8    mRegisterIndex;
   uint8    mGxmSemantic;
   uint8    mGxmSemanticIndexCount;
};

// Paired with Tools\AssetToolMPP\Shader\ShaderBinaryWriterVTA.cpp
struct SFragmentParameter
{
   uint32   mCRC;
   uint8    mParameterIndex;
   uint8    mBackendRegister;
   uint8    mTotalFloatParameterCount;
   uint8    mPad;
};

struct SShaderVertexData
{
   uint16   mSize;
   uint16   mPad0;
   uint16   mVertexParametersOffset;
   uint16   mVertexParametersSize;
   uint16   mVertexProgramSemanticsOffset;
   uint16   mVertexProgramUCodeOffset;
   uint16   mShaderConstantParametersOffset;
   uint16   mShaderConstantParametersSize;
   uint32   mRuntimeShaderPatcherId; // This is set at run-time
};

struct SShaderFragmentData
{
   uint16   mSize;
   uint16   mFragmentParametersOffset;
   uint16   mFragmentParametersSize;
   uint16   mSamplerParametersOffset;
   uint16   mSamplerParametersSize;
//   uint16   mFragmentProgramConfigOffset;
   uint16   mFragmentProgramUCodeOffset;
   uint16   mFragmentProgramUCodeSize;
   uint16   mShaderConstantParametersOffset;
   uint16   mShaderConstantParametersSize;
   uint16   mPad0;
   uint32   mRuntimeShaderPatcherId; // This is set at run-time
};

struct SShaderHashEntry
{
   uint32      mHash;
   uint32      mOffset;
};

struct SShaderCombinationHeader
{
   uint32      mPassCount;
   uint32      mPassOffsets[1]; // actually up to mPassCount number of offsets
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
//   uint32      mStateDataOffset;
};

struct SShaderPassHeader
{
   SShaderPassData   mPassData;
//   uint32            mStateDataOffset;
   uint32            mFragmentDataOffset;
   uint32            mVertexDataOffset;
//   uint16            mCommandBufferHoleSize;
//   uint16            mPad0;
};

class CCompiledShader
{
public:
   // Start using compiled shader, returns number of passes
   int const Begin() const;
   // Finish compiled shader
   void End() const;

   // Start using pass, returns pass type
   int const BeginPass(int const currentPass) const;
   void EndPass() const;

   void BeginShaderBatch(uint8* pMaterialConstantParameters, int const materialConstantParametersSize,
      uint8* pMaterialRuntimeParametersShared, int const materialRuntimeParametersSharedSize,
      uint8* pMaterialRuntimeParametersUnshared, int const materialRuntimeParametersUnsharedSize) const;

   void EndShaderBatch() const;

   SShaderPassHeader const * GetPassHeader(int const passIdx) const 
   {
      uint8* pBaseAddress = (uint8*)this;
      return (SShaderPassHeader const *)(pBaseAddress + mHeader.mPassOffsets[passIdx]);
   }

   int GetPassCount() const { return mHeader.mPassCount; }

   void FixupHeader();
private:
   SShaderCombinationHeader mHeader;
};

//----------------------------------------------------------------------------

class CCompiledShaderPackage : public IObject
{
public:
   static void Factory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

   ~CCompiledShaderPackage();

   int const GetCombinationCount() const
   { 
      return mpPackage->mCombinationCount; 
   }

   SShaderHashEntry const * GetHashTable() const 
   { 
      return (SShaderHashEntry const *)((uint8*)mpPackage + mpPackage->mHashTableOffset); 
   }

   uint32 GetHash(int const index) const
   {
      return GetHashTable()[index].mHash;
   }

   CCompiledShader const * GetCombination(int const index) const
   {
      uint32 const combinationOffset = GetHashTable()[index].mOffset;
      return (CCompiledShader const *)((uint8*)mpPackage + mpPackage->mCombinationsOffset + combinationOffset); 
   }

   CCompiledShader * Combination(int const index) 
   {
      return const_cast<CCompiledShader *>( GetCombination( index ) );
   }

   CCompiledShader const * GetCombinationByHash(uint32 const hash) const;

   CCompiledShaderPackage(void* pData);

private:
   void FixupOffsets();

public:
   SShaderPackageHeader*   mpPackage;
};