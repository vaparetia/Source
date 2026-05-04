//----------------------------------------------------------------------------
// CCGEffect.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

//----------------------------------------------------------------------------

class CResId;
struct SFactoryResourceBuildData;
struct SFactoryReturnResource;

//----------------------------------------------------------------------------

class CCGEffectBase
{
public:
   CCGEffectBase(std::string const & name, std::string const & semantic);

public:
   std::string mName;
   std::string mSemantic;
};

//----------------------------------------------------------------------------

enum EState
{
   // render states
   kState_AlphaBlendEnable,
   kState_AlphaTestEnable,
   kState_AlphaRef,
   kState_AlphaFunc,
   kState_BlendFunc,
   kState_BlendFuncSeparate,

   kState_ZEnable,

   kState_ZWriteEnable,
   kState_ColorMask,

   kState_VertexShader,
   kState_PixelShader,

   kState_CullFaceEnable,
   kState_CullFace,

   // texture stage states
   kState_Texture,
   kState_MinFilter,
   kState_MagFilter,
   kState_MipFilter,
   kState_WrapU,
   kState_WrapV,
   kState_WrapW,
   kState_Convolution,

   kState_Invalid = -1
};

class CCGEffectState : public CCGEffectBase
{
public:
   CCGEffectState(std::string const & name, std::string const & semantic);
   ~CCGEffectState();

public:
   EState   mState;
};

//----------------------------------------------------------------------------

class CCGEffectData
{
public:
   enum EType
   {
      kType_Bool1,
      kType_Bool2,
      kType_Bool3,
      kType_Bool4,

      kType_Int1,
      kType_Int2,
      kType_Int3,
      kType_Int4,

      kType_Float1,
      kType_Float2,
      kType_Float3,
      kType_Float4,

      kType_Float3x3,
      kType_Float3x4,
      kType_Float4x4,

      kType_String,

      kType_Program,
      kType_Texture,
      kType_Sampler2D,

      kType_Invalid = -1
   };

   virtual ~CCGEffectData();

protected:
   CCGEffectData(EType type);
   
public:
   EType    mType;
};

//----------------------------------------------------------------------------

class CCGEffectNamedData
{
public:
   CCGEffectNamedData(std::string const & name, CCGEffectData* pData);
   ~CCGEffectNamedData();

public:
   std::string                      mName;
   boost::shared_ptr<CCGEffectData> mpData;
};

//----------------------------------------------------------------------------

class CCGEffectDataNumeric : public CCGEffectData
{
public:
   CCGEffectDataNumeric(EType type);
   virtual ~CCGEffectDataNumeric();

public:
   // bool data
   union
   {
      bool     b1;
      bool     b2[2];
      bool     b3[3];
      bool     b4[4];
   } mData8;

   union
   {
      union
      {
         int      i1;
         int      i2[2];
         int      i3[3];
         int      i4[4];
      };

      union
      {
         real32   f1;
         real32   f2[2];
         real32   f3[3];
         real32   f4[4];
         real32   f3x3[3][3];
         real32   f3x4[3][4];
         real32   f4x4[4][4];
      };
   } mData32;
};

//----------------------------------------------------------------------------

class CCGEffectDataProgram : public CCGEffectData
{
public:
   CCGEffectDataProgram(std::vector<CCGEffectNamedData> const & programParameters);
   virtual ~CCGEffectDataProgram();

   void SetBinaryShader(uint32 size, const char * pData);

public:
   std::vector<CCGEffectNamedData>   mProgramParameters;

   uint32                              mSize;
   boost::scoped_ptr<char>             mData;
};

//----------------------------------------------------------------------------

class CCGEffectDataString : public CCGEffectData
{
public:
   CCGEffectDataString(std::string const & data);
   virtual ~CCGEffectDataString();

public:
   std::string mData;
};

//----------------------------------------------------------------------------

class CCGEffectStateAssignment
{
public:
   CCGEffectStateAssignment(CCGEffectState const & state,
                            CCGEffectData* pData);
   ~CCGEffectStateAssignment();

public:
   CCGEffectState                   mState;
   boost::shared_ptr<CCGEffectData> mpData;
};

//----------------------------------------------------------------------------

class CCGEffectStateAssignmentContainer
{
public:
   CCGEffectStateAssignmentContainer();

   CCGEffectStateAssignment const * GetByName(std::string const & name) const;

public:
   std::vector<CCGEffectStateAssignment> mData;
};

//----------------------------------------------------------------------------

class CCGEffectTexture : public CCGEffectBase
{
public:
   CCGEffectTexture(std::string const & name,
                    std::string const & semantic,
                    std::vector<CCGEffectNamedData> const & annotations);
   ~CCGEffectTexture();

public:
   std::vector<CCGEffectNamedData> mAnnotations;
};

//----------------------------------------------------------------------------

class CCGEffectSampler : public CCGEffectBase
{
public:
   CCGEffectSampler(std::string const & name,
                    std::string const & semantic,
                    CCGEffectStateAssignmentContainer const & stateAssignments);
   ~CCGEffectSampler();

public:
   CCGEffectStateAssignmentContainer mStateAssignments;
};

//----------------------------------------------------------------------------

class CCGEffectPass : public CCGEffectBase
{
public:
   CCGEffectPass(std::string const & name,
                 std::string const & semantic,
                 CCGEffectStateAssignmentContainer const & stateAssignments,
                 std::vector<CCGEffectNamedData> const & annotations);
   ~CCGEffectPass();

public:
   CCGEffectStateAssignmentContainer mStateAssignments;
   std::vector<CCGEffectNamedData> mAnnotations;
};

//----------------------------------------------------------------------------

class CCGEffectTechnique : public CCGEffectBase
{
public:
   CCGEffectTechnique(std::string const & name,
                      std::string const & semantic,
                      std::vector<CCGEffectPass> const & passes);
   ~CCGEffectTechnique();

public:
   std::vector<CCGEffectPass> mPasses;
};

//----------------------------------------------------------------------------

class CCGEffectCombination
{
public:
   CCGEffectCombination(uint32 hash,
                        std::vector<CCGEffectTechnique> const & techniques,
                        std::vector<CCGEffectSampler> const & samplers,
                        std::vector<CCGEffectTexture> const & textures);
   ~CCGEffectCombination();

   CCGEffectSampler const * GetSamplerByName(char const * const pName) const;
   CCGEffectTexture const * GetTextureByName(char const * const pName) const;

public:
   uint32                              mHash;
   std::vector<CCGEffectTechnique>   mTechniques;
   std::vector<CCGEffectSampler>     mSamplers;
   std::vector<CCGEffectTexture>     mTextures;
};

//----------------------------------------------------------------------------

class CCGEffect
{
public:
   CCGEffect(std::vector<CCGEffectCombination> const & combinations);
   virtual ~CCGEffect();

   CCGEffectCombination const * GetCombination(uint32 hash) const;
   CCGEffectCombination * Combination(uint32 hash);

public:
   std::vector<CCGEffectCombination> mCombinations;
};
