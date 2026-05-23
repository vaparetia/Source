//----------------------------------------------------------------------------
// PS3CCompiledShader.h
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Material/PS3/PS3CCompiledShaderTypes.h"

//----------------------------------------------------------------------------

struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

//----------------------------------------------------------------------------

class CCompiledShader : public SShaderCombinationHeader
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
      return (SShaderPassHeader const *)(pBaseAddress + mPassOffsets[passIdx]);
   }

};

//----------------------------------------------------------------------------

class CCompiledShaderPackage
{
public:
   static void Factory(SFactoryResourceBuildData &buildData, SFactoryReturnResource &returnResource);

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

   CCompiledShader const * GetCombinationByHash(uint32 const hash) const;

   CCompiledShaderPackage(void* pData);

private:

   void FixupOffsets();
   
public:
   SShaderPackageHeader*   mpPackage;
};