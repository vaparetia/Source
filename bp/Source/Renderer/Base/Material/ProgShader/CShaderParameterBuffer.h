//----------------------------------------------------------------------------
// CShaderParameterBuffer.h
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Backend/RenderBackendTypes.h"

#include "Engine/Math/CVector2.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"

//----------------------------------------------------------------------------

class CBaseTexture;
class CMatrix34;
class CMatrix4;

//----------------------------------------------------------------------------

class RENDERER_API CShaderParameterBuffer
{
public:
   CShaderParameterBuffer(uint8* pParameters);
   void PutTo(COutputStream & stream) const;

   void AddTexture(uint32 const parameterCRC, CBaseTexture const * pTexture, bool const clampU, bool const clampV, bool const srgb = false);

   void AddParameter(uint32 const parameterCRC, real32 const value) { AddParameter(parameterCRC, CVector4(value, 0.0f, 0.0f, 0.0f)); }
   void AddParameter(uint32 const parameterCRC, CVector2 const & value) { AddParameter(parameterCRC, CVector4(value.GetX(), value.GetY(), 0.0f, 0.0f)); }
   void AddParameter(uint32 const parameterCRC, CVector3 const & value) { AddParameter(parameterCRC, CVector4(value, 1.0f)); }
   void AddParameter(uint32 const parameterCRC, CVector4 const & value);
   void AddParameter(uint32 const parameterCRC, CMatrix34 const & value);
   void AddParameter(uint32 const parameterCRC, CMatrix4 const & value);
   
   void AddParameterGamma(uint32 const parameterCRC, CVector3 const & value, EShaderColorSpace const colorSpace) { AddParameter(parameterCRC, NShaderColorSpaceConversion::ConvertGammaToShaderColorSpace(value, colorSpace)); }
   void AddParameterGamma(uint32 const parameterCRC, CColor const & value, EShaderColorSpace const colorSpace) { AddParameter(parameterCRC, NShaderColorSpaceConversion::ConvertGammaToShaderColorSpace(value, colorSpace).GetVector4()); }
   void AddParameterGamma(uint32 const parameterCRC, CColorf const & value, EShaderColorSpace const colorSpace) { AddParameter(parameterCRC, NShaderColorSpaceConversion::ConvertGammaToShaderColorSpace(value, colorSpace).AsVector4()); }

   uint8* GetStartAddress() const { return reinterpret_cast<uint8*>(mpStartAddress); }
   uint8* GetCurrentParameterPtr() const { return reinterpret_cast<uint8*>(mpParameters); }
   int const GetSize() const { return (mpParameters - mpStartAddress) << 2; }

private:
   uint32*  mpStartAddress;
   uint32*  mpParameters;
};

//----------------------------------------------------------------------------

class RENDERER_API CShaderParameterBufferAllocator
{
public:
   // begins scoped of region memory
   static void BeginRegion();

   // Allocate/reserve a parameter buffer, must occur inside of a region, the size of the parameter 
   // buffer is open-ended as long as it doesn't go over the reserve size of the region, only one buffer can be in an "unfinalized" state at a time.
   static CShaderParameterBuffer const AllocateParameterBuffer();
   
   // finalizes a parameter buffer (meaning it actually sets the size of allocation) based on used size of the buffer
   static void FinalizeParameterBuffer(CShaderParameterBuffer const & parameterBuffer);

   // ends region of scoped memory, a command is put in the SPU queue to free the region
   static void EndRegion();
};


