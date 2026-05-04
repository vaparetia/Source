//----------------------------------------------------------------------------
// X360CookedShaderFormat.h
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

#include "Engine/StlExtras/vector_s.h"

#define FXLPCONTEXT_VERTEXSHADERCONSTANTF           0x0001
#define FXLPCONTEXT_PIXELSHADERCONSTANTF            0x0002
#define FXLPCONTEXT_VERTEXSHADERCONSTANTI           0x0004
#define FXLPCONTEXT_PIXELSHADERCONSTANTI            0x0008
#define FXLPCONTEXT_VERTEXSHADERCONSTANTB           0x0010
#define FXLPCONTEXT_PIXELSHADERCONSTANTB            0x0020
#define FXLPCONTEXT_VERTEXSHADERSAMPLER             0x0040
#define FXLPCONTEXT_PIXELSHADERSAMPLER              0x0080
typedef unsigned int CUSTOMPARAMETER_CONTEXT;

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

//----------------------------------------------------------------------------

class RENDERER_API SParam_X360
{
public:
   SParam_X360(uint32 crc, CUSTOMPARAMETER_CONTEXT context, uint8 vertexRegisterIndex, uint8 vertexRegisterCount, uint8 pixelRegisterIndex, uint8 pixelRegisterCount)
      : mCRC(crc)
      , mContext(context)
      , mVertexRegisterIndex(vertexRegisterIndex)
      , mVertexRegisterCount(vertexRegisterCount)
      , mPixelRegisterIndex(pixelRegisterIndex)
      , mPixelRegisterCount(pixelRegisterCount)
   {
   }

   SParam_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator < (SParam_X360 const & rhs) const
   {
      return mCRC < rhs.mCRC;
   }

   bool const operator == (SParam_X360 const & rhs) const
   {
      return mCRC == rhs.mCRC && mContext == rhs.mContext;
   }
public:
   uint32 mCRC;
   CUSTOMPARAMETER_CONTEXT mContext;
   uint8 mVertexRegisterIndex;
   uint8 mVertexRegisterCount;
   uint8 mPixelRegisterIndex;
   uint8 mPixelRegisterCount;
   std::string paramName;
};

class RENDERER_API SamplerState_X360
{
public:
   SamplerState_X360(uint32 reg, uint32 type, uint32 value)
      : mRegister(reg)
      , mType(type)
      , mValue(value)
   {
   }

   SamplerState_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (SamplerState_X360 const & rhs) const
   {
      return mRegister == rhs.mRegister && mType == rhs.mType && mValue == rhs.mValue;
   }

public:
   uint32 mRegister;
   uint32 mType;
   uint32 mValue;
};

class RENDERER_API RenderState_X360
{
public:
   RenderState_X360(uint32 type, uint32 value)
      : mType(type)
      , mValue(value)
   {
   }

   RenderState_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (RenderState_X360 const & rhs) const
   {
      return mType == rhs.mType && mValue == rhs.mValue;
   }

public:
   uint32 mType;
   uint32 mValue;
};

class RENDERER_API VertexShader_X360
{
public:
   VertexShader_X360()
      : mHash(0)
   {
   }
   VertexShader_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (VertexShader_X360 const & rhs) const
   {
      return mHash == rhs.mHash && mVertexShaderCode == rhs.mVertexShaderCode;
   }

public:
   bpe::vector_s<char> mVertexShaderCode;
   uint32 mHash;
};

class RENDERER_API PixelShader_X360
{
public:
   PixelShader_X360()
      : mHash(0)
   {
   }
   PixelShader_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (PixelShader_X360 const & rhs) const
   {
      return mHash == rhs.mHash && mPixelShaderCode == rhs.mPixelShaderCode;
   }

public:
   bpe::vector_s<char> mPixelShaderCode;
   uint32 mHash;
};

class RENDERER_API Params_X360
{
public:
   Params_X360()
      : mHash(0)
   {
   }
   Params_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (Params_X360 const & rhs) const
   {
      return mHash == rhs.mHash && mParams == rhs.mParams;
   }

public:
   bpe::vector_s<SParam_X360> mParams;

   uint32 mHash;
};

class RENDERER_API SamplerStates_X360
{
public:
   SamplerStates_X360()
      : mHash(0)
   {
   }
   SamplerStates_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (SamplerStates_X360 const & rhs) const
   {
      // Note: no need to check mSamperStates_Restore, it is created from mSamperStates
      return mHash == rhs.mHash && mSamperStates == rhs.mSamperStates;
   }

public:
   bpe::vector_s<SamplerState_X360> mSamperStates;
   bpe::vector_s<SamplerState_X360> mSamperStates_Restore;

   uint32 mHash;
};

class RENDERER_API RenderStates_X360
{
public:
   RenderStates_X360()
      : mHash(0)
   {
   }
   RenderStates_X360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   bool const operator == (RenderStates_X360 const & rhs) const
   {
      // Note: no need to check mRenderStates_Restore, it is created from mRenderStates
      return mHash == rhs.mHash && mRenderStates == rhs.mRenderStates;
   }

public:
   bpe::vector_s<RenderState_X360> mRenderStates;
   bpe::vector_s<RenderState_X360> mRenderStates_Restore;

   uint32 mHash;
};

class RENDERER_API Pass_360
{
public:
   Pass_360()
   {
   }
   Pass_360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

public:
   int32 mVertexShader_Index;
   int32 mPixelShader_Index;
   int32 mSamperStates_Index;
   int32 mRenderStates_Index;
   int32 mParams_Index;
};

class RENDERER_API Technique_360
{
public:
   Technique_360()
   {
   }
   Technique_360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

public:
   std::string mName;
   bpe::vector_s<Pass_360> mPasses;
};

class RENDERER_API EffectCombination_360
{
public:
   EffectCombination_360()
   {
   }
   EffectCombination_360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

public:
   uint32 mHash;
   bpe::vector_s<Technique_360> mTechniques;
};

class RENDERER_API Effect_360
{
public:
   Effect_360()
   {
   }
   virtual ~Effect_360(){}

   Effect_360(CInputStream & stream);
   void PutTo(COutputStream & stream) const;

   EffectCombination_360 const * GetCombination(uint32 hash) const;
   EffectCombination_360 * Combination(uint32 hash);

    static void FEffectFactory(SFactoryResourceBuildData & buildData, SFactoryReturnResource & returnResource);
public:
   std::string mName;
   bpe::vector_s<EffectCombination_360> mCombinations;

   bpe::vector_s<VertexShader_X360> mVertexShader;
   bpe::vector_s<PixelShader_X360> mPixelShader;
   bpe::vector_s<SamplerStates_X360> mSamperStates;
   bpe::vector_s<RenderStates_X360> mRenderStates;
   bpe::vector_s<Params_X360> mParams;
};