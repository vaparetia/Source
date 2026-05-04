//----------------------------------------------------------------------------
// ShaderBinaryWriter.cpp
// Copyright 2010
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "ShaderBinaryWriter.h"

//----------------------------------------------------------------------------

#include "boost/shared_ptr.hpp"

#include "Engine/Math/CMatrix4.h"
#include "Engine/Mechanics/CCRC.h"

#include "Engine/Streams/CLabeledOffsetStream.h"
#include "Engine/Streams/CDiskOutputStream.h"

#include "Renderer/Base/Material/PS3/PS3CCompiledShaderTypes.h"
#include "Renderer/Base/Material/ProgShader/CShaderParameterBuffer.h"
#include "Renderer/Base/Material/ProgShader/PSCShader.h"

#include "Tools/AssetToolMPP/Shader/CCGEffect.h"
#include "Tools/AssetToolMPP/cgb/cgbLevelC.h"

#include "gcm_tool.h"

#define SHADER_MEASURE_MODE
#include "Renderer/Renderer_SPU/ShaderSetup.cpp"

//----------------------------------------------------------------------------

// MSDEV - Bug with template generation and anon namespaces
namespace NShaderBinaryWriter {}
using namespace NShaderBinaryWriter;

namespace NShaderBinaryWriter
{
   class CBinaryChunkWriter
   {
   public:
      CBinaryChunkWriter()
         :  mTotalDataCount(0)
      {
      }

      uint32 AddData(void const * pData, uint32 const dataSize );

      void const * const GetData() { return mStorage.GetData(); }
      int const GetDataSize() const { return mStorage.GetDataSize(); }

      int const GetTotalDataCount() const { return mTotalDataCount; }
      int const GetUniqueDataCount() const { return mOffsetMap.size(); }

   private:
      typedef std::map<uint32, uint32> TCRCToOffsetMap;

      int                        mTotalDataCount;
      TCRCToOffsetMap            mOffsetMap;
      CGrowableMemoryOutStream   mStorage;
   };
}

//----------------------------------------------------------------------------

uint32 CBinaryChunkWriter::AddData(void const * pData, uint32 const dataSize )
{
   mTotalDataCount++;

   uint32 const crc = CCRC::CalculateCRC32(pData, dataSize);

   TCRCToOffsetMap::const_iterator found = mOffsetMap.find(crc);
   if( found != mOffsetMap.end() )
   {
      return found->second;
   }
   else
   {
      uint32 const offset = mStorage.GetWrittenBytes();

      mStorage.Put(pData, dataSize);
      mStorage.AddAlignmentPadding(16);

      mOffsetMap.insert(TCRCToOffsetMap::value_type(crc, offset));

      return offset;
   }
}

//----------------------------------------------------------------------------

namespace NShaderBinaryWriter
{
   static int const sVertexRegisterStrideByType[] =
   {
# define CG_DATATYPE_MACRO(name, compiler_name, enum_name, base_name, ncols, nrows, pc) (ncols > 0) ? ncols : 1,
#include <Cg/cg_datatypes.h>
# undef CG_DATATYPE_MACRO
   };

   static int const sVertexRegisterValueCountByType[] =
   {
# define CG_DATATYPE_MACRO(name, compiler_name, enum_name, base_name, ncols, nrows, pc) ((ncols > 0) ? ncols : 1) * nrows,
#include <Cg/cg_datatypes.h>
# undef CG_DATATYPE_MACRO
   };

   struct SVertexParameter
   {
      SVertexParameter()
         :  mCRC(0)
         ,  mNumFloats(0)
         ,  mRegister(0)
         ,  mPad(0)
      {
      }

      void PutTo(COutputStream & stream) const
      {
         stream.WriteUint32(mCRC);
         stream.WriteUint32(mNumFloats);
         stream.WriteUint32(mRegister);
         stream.WriteUint32(mPad);
      }

      bool const operator < (SVertexParameter const & rhs) const
      {
         return mCRC < rhs.mCRC;
      }

      uint32   mCRC;
      uint32   mNumFloats;
      uint32   mRegister;
      uint32   mPad;
   };

   struct SFragmentParameter
   {
      void PutTo(COutputStream & stream) const
      {
         stream.WriteUint32(mCRC);
         stream.WriteUint16(mDataOffset);
         stream.WriteUint16(mDestOffsets.size());

         for( int i = 0; i < mDestOffsets.size(); ++i )
            stream.WriteUint16(mDestOffsets[i]);
      }

      bool const operator < (SFragmentParameter const & rhs) const
      {
         return mCRC < rhs.mCRC;
      }

      uint32   mCRC;
      uint16   mDataOffset;
      std::vector<uint16> mDestOffsets;
   };

   struct SSamplerParameter
   {
      void PutTo(COutputStream & stream) const
      {
         stream.WriteUint32(mCRC);
         stream.WriteUint16(mIndex);
         stream.WriteUint8(mMinFilter);
         stream.WriteUint8(mMagFilter);
         stream.WriteUint8(mConv);
         stream.WriteUint8(mWrapU);
         stream.WriteUint8(mWrapV);
         stream.WriteUint8(mWrapW);
      }

      bool const operator < (SSamplerParameter const & rhs) const
      {
         return mCRC < rhs.mCRC;
      }

      uint32   mCRC;
      uint16   mIndex;
      uint8    mMinFilter;
      uint8    mMagFilter;
      uint8    mConv;
      uint8    mWrapU;
      uint8    mWrapV;
      uint8    mWrapW;
   };

   CCGEffectTexture const * GetTextureFromSampler(CCGEffectCombination const & combination, const char * const pSamplerName)
   {
      CCGEffectSampler const * pSampler = combination.GetSamplerByName(pSamplerName);
      if( pSampler )
      {
         foreach(CCGEffectStateAssignment const & assignment, pSampler->mStateAssignments.mData)
         {
            if( assignment.mState.mName.c_str() == bpe::istring("Texture") )
            {
               CCGEffectDataString const * pData = static_cast<CCGEffectDataString const *>(assignment.mpData.get());
               return combination.GetTextureByName(pData->mData.c_str());
            }
         }
      }

      return NULL;
   }

   void ConvertVertexParameters(CellCgbProgram & program, std::vector<SVertexParameter> & vertexParameters)
   {
      int const parameterCount = cellCgbMapGetLength(&program);

      int const kNameBufferSize = 256;
      char name[kNameBufferSize];

      for( int parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex )
      {
         // only process input parameters
         if (cellCgbLevelCMapGetDirection(&program, parameterIndex) == CG_IN)
         {
            // skip over "varying" parameters, these are streams
            CGenum const variability = (CGenum)cellCgbLevelCMapGetVariability(&program, parameterIndex);
            if( variability == CG_UNIFORM )
            {
               uint32_t actualNameLength = kNameBufferSize;
               cellCgbMapGetName(&program, parameterIndex, name, &actualNameLength);

               uint16_t resource = cellCgbLevelCMapGetCgResource(&program, parameterIndex);
               uint16_t type = cellCgbLevelCMapGetCgType(&program, parameterIndex);

               uint16_t regIndex;
               cellCgbMapGetVertexUniformRegister(&program, parameterIndex, &regIndex, NULL);

               // skip over parameters with no assigned registers (we can get these for top level array parameters when the array is not continously packed)
               if( regIndex == 0xFFFF )
               {
                  continue;
               }

               int const valueCount = cellCgbLevelCMapGetValueCount(&program, parameterIndex);

               bool const isArray = cellCgbLevelCMapIsArray(&program, parameterIndex);

               if( !isArray )
               {
                  SVertexParameter parameter;
                  parameter.mCRC = CShaderCRCs::GetParameterCRC(name);
                  parameter.mNumFloats = valueCount;
                  parameter.mRegister = regIndex;
                  vertexParameters.push_back(parameter);

#ifdef VERBOSE_CONVERSION
                  printf("Vertex Parameter: %s (%8.8x), Register: %d Values: %d Res: %d Type: %d\n", name, parameter.mCRC, parameter.mRegister, parameter.mNumFloats, resource, type);
#endif
               }
               else
               {
                  int currentRegister = regIndex;

                  int const registerStride = sVertexRegisterStrideByType[type - CG_TYPE_START_ENUM - 1];
                  int const valueCountForArrayElement = sVertexRegisterValueCountByType[type - CG_TYPE_START_ENUM - 1];

                  char arrayElementName[kNameBufferSize];

                  for( int arrayElement = 0; arrayElement < valueCount; ++arrayElement )
                  {
                     sprintf(arrayElementName, "%s[%d]", name, arrayElement);

                     SVertexParameter parameter;
                     parameter.mCRC = CShaderCRCs::GetParameterCRC(arrayElementName);
                     parameter.mNumFloats = valueCountForArrayElement;
                     parameter.mRegister = currentRegister;

                     vertexParameters.push_back(parameter);

#ifdef VERBOSE_CONVERSION
                     printf("Vertex Parameter: %s (%8.8x), Register: %d Values: %d Res: %d Type: %d\n", arrayElementName, parameter.mCRC, parameter.mRegister, parameter.mNumFloats, resource, type);
#endif
                     currentRegister += registerStride;
                  }
               }

            }
         }
      }
   }

#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

   static inline uint8 const GetFilterEnum(uint32 const value)
   {
      switch(value)
      {
      case GL_LINEAR:
         return CELL_GCM_TEXTURE_LINEAR;

      case GL_NEAREST:
         return CELL_GCM_TEXTURE_NEAREST;

      default:
         return (uint8)value;
      }
   }

#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_CLAMP                        0x2900
#define GL_MIRRORED_REPEAT              0x8370
#define GL_MIRROR_CLAMP_EXT             0x8742
#define GL_MIRROR_CLAMP_TO_EDGE_EXT     0x8743
#define GL_MIRROR_CLAMP_TO_BORDER_EXT	0x8912
#define GL_CLAMP_TO_BORDER				0x812D

   static inline uint8 const GetWrapEnum(uint32 const value)
   {
      switch(value)
      {
      case GL_REPEAT:
         return CELL_GCM_TEXTURE_WRAP;

      case GL_CLAMP_TO_EDGE:
         return CELL_GCM_TEXTURE_CLAMP_TO_EDGE;

      case GL_MIRRORED_REPEAT:
         return CELL_GCM_TEXTURE_MIRROR;

      case GL_CLAMP_TO_BORDER:
         return CELL_GCM_TEXTURE_BORDER;

      case GL_CLAMP:
         return CELL_GCM_TEXTURE_CLAMP;

      default:
         return CELL_GCM_TEXTURE_WRAP;
      }
   }

   void ConvertSamplerSettings(CCGEffectSampler const & sampler, SSamplerParameter & parameter)
   {
      parameter.mMinFilter = CELL_GCM_TEXTURE_LINEAR;
      parameter.mMagFilter = CELL_GCM_TEXTURE_LINEAR;
      parameter.mConv = CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX;
      parameter.mWrapU = CELL_GCM_TEXTURE_WRAP;
      parameter.mWrapV = CELL_GCM_TEXTURE_WRAP;
      parameter.mWrapW = CELL_GCM_TEXTURE_WRAP;

      foreach(CCGEffectStateAssignment const & stateAssignment, sampler.mStateAssignments.mData)
      {
         CCGEffectDataNumeric const * pNumeric = static_cast<CCGEffectDataNumeric const *>(stateAssignment.mpData.get());

         switch(stateAssignment.mState.mState)
         {
         case kState_Texture:
            break;

         case kState_MinFilter:
            parameter.mMinFilter = GetFilterEnum(pNumeric->mData32.i1);
            break;

         case kState_MagFilter:
            parameter.mMagFilter = GetFilterEnum(pNumeric->mData32.i1);
            break;

         case kState_MipFilter:
            // Silently ignore the mip filter for now
            break;

         case kState_WrapU:
            parameter.mWrapU = GetWrapEnum(pNumeric->mData32.i1);
            break;

         case kState_WrapV:
            parameter.mWrapV = GetWrapEnum(pNumeric->mData32.i1);
            break;

         case kState_WrapW:
            parameter.mWrapW = GetWrapEnum(pNumeric->mData32.i1);
            break;

         case kState_Convolution:
            parameter.mConv = pNumeric->mData32.i1;
            break;

         case kState_Invalid:
            bpe_debugger_printf("Invalid Sampler State: %s\n", stateAssignment.mState.mName.c_str());
            break;
         }
      }
   }

   void ConvertSamplerParameter(CCGEffectCombination const & combination, CellCgbProgram & program, int const parameterIndex, std::vector<SSamplerParameter> & samplerParameters)
   {
      int const kNameBufferSize = 256;
      char name[kNameBufferSize];

      uint32_t actualNameLength = kNameBufferSize;
      cellCgbMapGetName(&program, parameterIndex, name, &actualNameLength);

      CCGEffectSampler const * pSampler = combination.GetSamplerByName(name);
      if( pSampler != NULL )
      {
         CCGEffectTexture const * textureParameter = GetTextureFromSampler(combination, name);
         if( textureParameter != NULL )
         {
            std::string const & safeSemantic = !textureParameter->mSemantic.empty() ? textureParameter->mSemantic : textureParameter->mName;

            // Add parameter for texture
            SSamplerParameter parameter;
            parameter.mCRC = CShaderCRCs::GetParameterCRC(safeSemantic.c_str());
            parameter.mIndex = cellCgbMapGetValue(&program, parameterIndex);
            ConvertSamplerSettings(*pSampler, parameter);

            samplerParameters.push_back(parameter);

#ifdef VERBOSE_CONVERSION
            printf("Sampler[%d]: %s (%8.8x) min: %d mag: %d\n", parameter.mIndex, safeSemantic.c_str(), parameter.mCRC, parameter.mMinFilter, parameter.mMagFilter);
#endif
         }
      }
   }

   void ConvertFragmentParameter(CellCgbProgram & program, int const parameterIndex, std::vector<SFragmentParameter> & fragmentParameters)
   {
      int const kNameBufferSize = 256;
      char name[kNameBufferSize];

      uint32_t actualNameLength = kNameBufferSize;
      cellCgbMapGetName(&program, parameterIndex, name, &actualNameLength);

      uint16_t offsets[64];
      uint32_t offsetCount = 64;
      cellCgbMapGetFragmentUniformOffsets(&program, parameterIndex, offsets, &offsetCount);
      if( offsetCount > 0 )
      {
         CGtype const type = (CGtype)cellCgbLevelCMapGetCgType(&program, parameterIndex);
         int const valueCount = cellCgbLevelCMapGetValueCount(&program, parameterIndex);

         SFragmentParameter parameter;
         parameter.mCRC = CShaderCRCs::GetParameterCRC(name);

         bool arrayParameter = false;

         // calculate data offset
         {
            parameter.mDataOffset = 0;

            std::string indexStr = name;
            int const foundBegin = indexStr.rfind('[');
            if( foundBegin != std::string::npos )
            {
               int const foundEnd = indexStr.find(']', foundBegin);
               if( foundEnd != std::string::npos )
               {
                  std::string parentName = std::string(name).substr(0, foundBegin);

                  indexStr = indexStr.substr(foundBegin + 1, foundEnd - foundBegin);
                  int const arrayIndex = atoi(indexStr.c_str());

                  parameter.mCRC = CShaderCRCs::GetParameterCRC(parentName.c_str());
                  parameter.mDataOffset = arrayIndex * sizeof(real32) * valueCount;

                  arrayParameter = true;
#ifdef VERBOSE_CONVERSION
                  printf("Fragment Parameter: %s (%8.8x) Parent: %s Index: %d DataOffset: %d\n", name, parameter.mCRC, parentName.c_str(), arrayIndex, parameter.mDataOffset);
#endif
               }
            }
         }

#ifdef VERBOSE_CONVERSION
         if( !arrayParameter )
            printf("Fragment Parameter: %s (%8.8x), Type: %d Value Count: %d\n", name, parameter.mCRC, type, valueCount);
#endif

         for( int i = 0; i < offsetCount; ++i )
         {
#ifdef VERBOSE_CONVERSION
            if( i == 0 )
               printf("Offsets:\n");

            printf("   %d\n", offsets[i]);
#endif
            parameter.mDestOffsets.push_back(offsets[i]);
         }

         fragmentParameters.push_back(parameter);
      }
   }

   void ConvertFragmentParameters(CCGEffectCombination const & combination, CellCgbProgram & program, std::vector<SFragmentParameter> & fragmentParameters, std::vector<SSamplerParameter> & samplerParameters)
   {
      int const parameterCount = cellCgbMapGetLength(&program);

      for( int parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex )
      {
         // Only process parameters that are input parameters
         if (cellCgbLevelCMapGetDirection(&program, parameterIndex) == CG_IN)
         {
            // Only care about uniform parameters
            CGenum const variability = (CGenum)cellCgbLevelCMapGetVariability(&program, parameterIndex);
            if( variability == CG_UNIFORM )
            {
               CGtype const type = (CGtype)cellCgbLevelCMapGetCgType(&program, parameterIndex);

               switch( type )
               {
               case CG_SAMPLERRECT:
               case CG_SAMPLER1D:
               case CG_SAMPLER2D:
               case CG_SAMPLER3D:
               case CG_SAMPLERCUBE:
                  ConvertSamplerParameter(combination, program, parameterIndex, samplerParameters);
                  break;

               default:
                  ConvertFragmentParameter(program, parameterIndex, fragmentParameters);
                  break;
               }
            }
         }
      }
   }

   void ConvertProgramParameters(CCGEffectDataProgram const * pProgram, CShaderParameterBuffer & shaderConstantBuffer)
   {
      for( int i = 0; i < pProgram->mProgramParameters.size(); ++i )
      {
         CCGEffectNamedData const & parameter = pProgram->mProgramParameters[i];
         CCGEffectDataNumeric const * pNumericData = (CCGEffectDataNumeric const *)parameter.mpData.get();

         uint32 const crc = CShaderCRCs::GetParameterCRC(parameter.mName.c_str());
         switch(parameter.mpData->mType)
         {
         case CCGEffectData::kType_Float1:
            shaderConstantBuffer.AddParameter(crc, pNumericData->mData32.f1);
            break;

         case CCGEffectData::kType_Float2:
            shaderConstantBuffer.AddParameter(crc, *reinterpret_cast<CVector2 const*>(pNumericData->mData32.f2));
            break;

         case CCGEffectData::kType_Float3:
            shaderConstantBuffer.AddParameter(crc, *reinterpret_cast<CVector3 const*>(pNumericData->mData32.f3));
            break;

         case CCGEffectData::kType_Float4:
            shaderConstantBuffer.AddParameter(crc, *reinterpret_cast<CVector4 const*>(pNumericData->mData32.f4) );
            break;

         case CCGEffectData::kType_Float3x4:
            shaderConstantBuffer.AddParameter(crc, *reinterpret_cast<CMatrix34 const*>(pNumericData->mData32.f3x4) );
            break;

         case CCGEffectData::kType_Float4x4:
            shaderConstantBuffer.AddParameter(crc, *reinterpret_cast<CMatrix4 const*>(pNumericData->mData32.f4x4) );
            break;

         default:
            BPE_VERIFYA(false, "Invalid program parameter type");
            break;
         }

      }
   }

   void SetupGCMContext(cell::Gcm::Unsafe::CellGcmContext* pContext, void* pData, int size, CellGcmContextCallback callback)
   {
      pContext->begin = pContext->current = (uint32_t*)pData;
      pContext->end = pContext->begin + size;
      pContext->callback = callback;
   }

   void ConvertPass(CCGEffectCombination const & combination, CCGEffectPass const & pass, CLabeledOffsetStream & shaderBinary, CBinaryChunkWriter & fragmentShaderChunks, CBinaryChunkWriter & vertexShaderChunks, CBinaryChunkWriter & stateDataChunks)
   {
      SShaderPassData passData;

      std::vector<SVertexParameter> vertexParameters;
      std::vector<SFragmentParameter> fragmentParameters;
      std::vector<SSamplerParameter> samplerParameters;

      int const kStateCommandBufferMaxSize = 2 * 1024;
      uint32 stateSetBufferMemory[kStateCommandBufferMaxSize];
      cell::Gcm::Unsafe::CellGcmContext stateSetContext;
      SetupGCMContext(&stateSetContext, (uint32_t*)stateSetBufferMemory, kStateCommandBufferMaxSize, NULL);

      uint32 stateResetBufferMemory[kStateCommandBufferMaxSize];
      cell::Gcm::Unsafe::CellGcmContext stateResetContext;
      SetupGCMContext(&stateResetContext, (uint32_t*)stateResetBufferMemory, kStateCommandBufferMaxSize, NULL);

      int const kShaderConstantBufferMaxSize = 16 * 1024;
      uint8 fragmentShaderConstantBufferMemory[kShaderConstantBufferMaxSize];
      CShaderParameterBuffer fragmentShaderConstantBuffer(fragmentShaderConstantBufferMemory);
      uint8 vertexShaderConstantBufferMemory[kShaderConstantBufferMaxSize];
      CShaderParameterBuffer vertexShaderConstantBuffer(vertexShaderConstantBufferMemory);

      CellCgbFragmentProgramConfiguration fragmentProgramConfig;
      void const * pFragmentProgramUCode = NULL;
      uint32 fragmentProgramUCodeSize = 0;

      CellCgbVertexProgramConfiguration vertexProgramConfig;
      void const * pVertexProgramUCode = NULL;
      uint32 vertexProgramUCodeSize = 0;

      uint32 alphaFunc = CELL_GCM_GREATER;
      uint32 alphaRef = 0;
      bool hasAlphaTest = false;

      foreach(CCGEffectStateAssignment const & stateAssignment, pass.mStateAssignments.mData)
      {
         CCGEffectDataNumeric const * pNumeric = static_cast<CCGEffectDataNumeric const *>(stateAssignment.mpData.get());
         CCGEffectDataProgram const * pProgram = static_cast<CCGEffectDataProgram const *>(stateAssignment.mpData.get());

         switch(stateAssignment.mState.mState)
         {
         case kState_AlphaBlendEnable:
            if( pNumeric->mData8.b1 != false )
            {
               stateSetContext.SetBlendEnable(CELL_GCM_TRUE);
               stateResetContext.SetBlendEnable(CELL_GCM_FALSE);
            }
            break;

         case kState_AlphaTestEnable:
            if( pNumeric->mData8.b1 != false )
            {
               stateSetContext.SetAlphaTestEnable(CELL_GCM_TRUE);
               stateResetContext.SetAlphaTestEnable(CELL_GCM_FALSE);
            }
            break;

         case kState_AlphaRef:
            alphaRef = (uint32)(pNumeric->mData32.f1 * 255.0f);
            hasAlphaTest = true;
            break;

         case kState_AlphaFunc:
            alphaFunc = (uint32)pNumeric->mData32.f1;
            hasAlphaTest = true;
            break;

         case kState_BlendFunc:
            stateSetContext.SetBlendFunc(pNumeric->mData32.i2[0], pNumeric->mData32.i2[1], pNumeric->mData32.i2[0], pNumeric->mData32.i2[1]);
            break;

         case kState_BlendFuncSeparate:
            stateSetContext.SetBlendFunc(pNumeric->mData32.i4[0], pNumeric->mData32.i4[1], pNumeric->mData32.i4[2], pNumeric->mData32.i4[3]);
            break;

         case kState_ZEnable:
            passData.SetFlag(SShaderPassData::kDepthTest, pNumeric->mData8.b1);
            break;

         case kState_ZWriteEnable:
            if( pNumeric->mData8.b1 != true )
            {
               stateSetContext.SetDepthMask(CELL_GCM_FALSE);
               stateResetContext.SetDepthMask(CELL_GCM_TRUE);
            }
            break;

         case kState_ColorMask:
            passData.SetFlag(SShaderPassData::kColorMaskR, pNumeric->mData8.b4[0]);
            passData.SetFlag(SShaderPassData::kColorMaskG, pNumeric->mData8.b4[1]);
            passData.SetFlag(SShaderPassData::kColorMaskB, pNumeric->mData8.b4[2]);
            passData.SetFlag(SShaderPassData::kColorMaskA, pNumeric->mData8.b4[3]);
            break;

         case kState_VertexShader:
            {
               CellCgbProgram vertexProgram;

               // read CGB header
               cellCgbRead(pProgram->mData.get(), pProgram->mSize, &vertexProgram);

               // retrieve configuration struct
               cellCgbGetVertexConfiguration(&vertexProgram, &vertexProgramConfig);

               // get ptr to shader microcode
               pVertexProgramUCode = cellCgbGetUCode(&vertexProgram);
               vertexProgramUCodeSize = cellCgbGetUCodeSize(&vertexProgram);

               // set the constant values, which are the internal values needed by the program (constants or the default values)
               {
                  int const count = cellCgbGetVertexConstantCount(&vertexProgram);
                  for( int i = 0; i < count; i++ )
                  {
                     // get the constant value and the register it needs to go to
                     const real32 *value = NULL;
                     uint16_t reg = 0;
                     cellCgbGetVertexConstantValues(&vertexProgram, i, &reg, &value);

                     uint32 const fakeCRC = reg;

                     // write parameter
                     SVertexParameter vertexParameter;
                     vertexParameter.mCRC = fakeCRC;
                     vertexParameter.mNumFloats = 4;
                     vertexParameter.mRegister = reg;
                     vertexParameters.push_back(vertexParameter);

                     // write data for parameter to constant buffer
                     vertexShaderConstantBuffer.AddParameter(fakeCRC, CVector4(NEndian::GetSwapped(value[0]), NEndian::GetSwapped(value[1]), NEndian::GetSwapped(value[2]), NEndian::GetSwapped(value[3])));
                  }
               }

               ConvertProgramParameters(pProgram, vertexShaderConstantBuffer);
               ConvertVertexParameters(vertexProgram, vertexParameters);
            }
            break;

         case kState_PixelShader:
            {
               CellCgbProgram fragmentProgram;

               // read CGB header
               cellCgbRead(pProgram->mData.get(), pProgram->mSize, &fragmentProgram);

               // retrieve configuration struct
               cellCgbGetFragmentConfiguration(&fragmentProgram, &fragmentProgramConfig);

               // check if there is an annotation to control the register count of the shader
               for( int annotationIdx = 0; annotationIdx < pass.mAnnotations.size(); ++annotationIdx )
               {
                  CCGEffectNamedData const & annotation = pass.mAnnotations[annotationIdx];
                  if( CStringExtras::CompareCaseInsensitive(annotation.mName, "TempRegisterCount") )
                  {
                     BPE_VERIFY(annotation.mpData->mType == CCGEffectData::kType_Int1, false, "Invalid type for 'TempRegisterCount' annotation, should be Int.");

                     CCGEffectDataNumeric const * pTempRegisterCount = static_cast<CCGEffectDataNumeric const *>(annotation.mpData.get());

                     BPE_VERIFY(pTempRegisterCount->mData32.i1 >= 2 && pTempRegisterCount->mData32.i1 <= 48, false, "Invalid register count specified in 'TempRegisterCount' annotation, must be between 2 and 48");
                     BPE_VERIFY(pTempRegisterCount->mData32.i1 >= fragmentProgramConfig.registerCount, false, "Temp register count must be at least as big as the one in the fragment program");

                     fragmentProgramConfig.registerCount = pTempRegisterCount->mData32.i1;
                  }
                  else if( CStringExtras::CompareCaseInsensitive(annotation.mName, "CLIP0") )
                  {
                     uint32 inputMask = NEndian::GetSwapped((uint32)fragmentProgramConfig.attributeInputMask);
                     inputMask |= CELL_GCM_ATTRIB_OUTPUT_MASK_UC0;
                     fragmentProgramConfig.attributeInputMask = NEndian::GetSwapped(inputMask); 
                  }
               }

               uint32_t controlTxp = CELL_GCM_FALSE;
               fragmentProgramConfig.fragmentControl &= ~CELL_GCM_MASK_SET_SHADER_CONTROL_CONTROL_TXP;
               fragmentProgramConfig.fragmentControl |= controlTxp << CELL_GCM_SHIFT_SET_SHADER_CONTROL_CONTROL_TXP;

               // get ptr and size of shader microcode
               pFragmentProgramUCode = cellCgbGetUCode(&fragmentProgram);
               fragmentProgramUCodeSize = cellCgbGetUCodeSize(&fragmentProgram);

               ConvertProgramParameters(pProgram, fragmentShaderConstantBuffer);
               ConvertFragmentParameters(combination, fragmentProgram, fragmentParameters, samplerParameters);
            }

            break;

         case kState_CullFaceEnable:
            stateSetContext.SetCullFaceEnable(pNumeric->mData8.b1);
            stateResetContext.SetCullFaceEnable(CELL_GCM_TRUE);
            break;

         case kState_CullFace:
            stateSetContext.SetCullFace( pNumeric->mData32.i1 );
            stateResetContext.SetCullFace( CELL_GCM_BACK );
            break;

         case kState_Invalid:
            bpe_debugger_printf("CCompiledShader::ConvertPass: Unsupported state: %s\n", stateAssignment.mState.mName.c_str());
            break;

         }
      }

      if( hasAlphaTest )
      {
         stateSetContext.SetAlphaFunc(alphaFunc, alphaRef);
         stateResetContext.SetAlphaFunc(CELL_GCM_GREATER, 0);
      }

      foreach( CCGEffectNamedData const & annotation, pass.mAnnotations)
      {
         if( annotation.mName == "AlphaToCoverage" )
         {
            passData.SetFlag(SShaderPassData::kAlphaToCoverage, true);
         }
         else if ( annotation.mName == "WaitForRenderTargetResult" )
         {
            passData.SetFlag(SShaderPassData::kWaitForRenderTargetResult, true);
         }
         else if ( annotation.mName == "ForceDisableAA" )
         {
            passData.SetFlag(SShaderPassData::kForceDisableAA, true);
         }
      }

      // write out data in packed format
      {
         int const stateSetSize = (stateSetContext.current - stateSetContext.begin) * sizeof(uint32);
         int const stateResetSize = (stateResetContext.current - stateResetContext.begin) * sizeof(uint32);

         CGrowableMemoryOutStream vertexParameterStream;
         {
            std::sort(vertexParameters.begin(), vertexParameters.end());
            for( int i = 0; i < vertexParameters.size(); ++i )
               vertexParameterStream.Put(vertexParameters[i]);
         }

         CGrowableMemoryOutStream fragmentParameterStream;
         {
            std::sort(fragmentParameters.begin(), fragmentParameters.end());
            for( int i = 0; i < fragmentParameters.size(); ++i )
               fragmentParameterStream.Put(fragmentParameters[i]);
         }

         CGrowableMemoryOutStream samplerParameterStream;
         {
            std::sort(samplerParameters.begin(), samplerParameters.end());
            for( int i = 0; i < samplerParameters.size(); ++i )
               samplerParameterStream.Put(samplerParameters[i]);
         }

         // State Set Data
         uint32 stateDataOffset;
         {
            CLabeledOffsetStream stateDataStream;
            stateDataStream.AddOffset16("SetStateEnd", "SetState");
            stateDataStream.AddOffset16("ResetStateEnd", "ResetState");

            stateDataStream.AddLabel("SetState");
            stateDataStream.Put(stateSetBufferMemory, stateSetSize);
            stateDataStream.AddLabel("SetStateEnd");

            stateDataStream.AddLabel("ResetState");
            stateDataStream.Put(stateResetBufferMemory, stateResetSize);
            stateDataStream.AddLabel("ResetStateEnd");

            stateDataStream.PatchOffsets();
            stateDataOffset = stateDataChunks.AddData(stateDataStream.GetData(), stateDataStream.GetDataSize());
         }

         CLabeledOffsetStream fragmentDataStream;
         uint8* pShaderFragmentData;
         uint32 fragmentDataOffset;
         {
            fragmentDataStream.AddLabel("Start");

            // uint16   mSize;
            fragmentDataStream.AddOffset16("End", "Start");
            // uint16   mFragmentParametersOffset;
            fragmentDataStream.AddOffset16("FragmentParameters", "Start");
            // uint16   mFragmentParametersSize;
            fragmentDataStream.WriteUint16(fragmentParameterStream.GetDataSize());
            // uint16   mSamplerParametersOffset;
            fragmentDataStream.AddOffset16("SamplerParameters", "Start");
            // uint16   mSamplerParametersSize;
            fragmentDataStream.WriteUint16(samplerParameterStream.GetDataSize());
            // uint16   mFragmentProgramConfigOffset;
            fragmentDataStream.AddOffset16("FragmentConfig", "Start");
            // uint16   mFragmentProgramUCodeOffset;
            fragmentDataStream.AddOffset16("FragmentUCode", "Start");
            // uint16   mFragmentProgramUCodeSize;
            fragmentDataStream.WriteUint16((uint16)fragmentProgramUCodeSize);
            // uint16   mShaderConstantParametersOffset;
            fragmentDataStream.AddOffset16("ShaderConstants", "Start");
            // uint16   mShaderConstantParametersSize;
            fragmentDataStream.WriteUint16(fragmentShaderConstantBuffer.GetSize());

            fragmentDataStream.AddLabel("FragmentParameters");
            if( fragmentParameterStream.GetDataSize() > 0 )
               fragmentDataStream.Put(fragmentParameterStream.GetData(), fragmentParameterStream.GetDataSize());

            fragmentDataStream.AddLabel("SamplerParameters");
            if( samplerParameterStream.GetDataSize() > 0 )
               fragmentDataStream.Put(samplerParameterStream.GetData(), samplerParameterStream.GetDataSize());

            fragmentDataStream.AddLabel("FragmentConfig");
            fragmentDataStream.Put(&fragmentProgramConfig, sizeof(fragmentProgramConfig));

            fragmentDataStream.AddLabel("FragmentUCode");
            fragmentDataStream.Put(pFragmentProgramUCode, fragmentProgramUCodeSize);

            fragmentDataStream.AddLabel("ShaderConstants");
            fragmentDataStream.Put(fragmentShaderConstantBuffer);

            fragmentDataStream.AddAlignedLabel("End", 16);

            fragmentDataStream.PatchOffsets();

            pShaderFragmentData = (uint8*)fragmentDataStream.GetData();
            fragmentDataOffset = fragmentShaderChunks.AddData(fragmentDataStream.GetData(), fragmentDataStream.GetDataSize());
         }

         CLabeledOffsetStream vertexDataStream;
         uint8* pShaderVertexData;
         uint32 vertexDataOffset;
         {
            vertexDataStream.AddLabel("Start");

            // uint16   mSize;
            vertexDataStream.AddOffset16("End", "Start");
            // uint16   mPad0;
            vertexDataStream.WriteUint16(0xFEED);
            // uint16   mVertexParametersOffset;
            vertexDataStream.AddOffset16("VertexParameters", "Start");
            // uint16   mVertexParametersSize;
            vertexDataStream.WriteUint16(vertexParameterStream.GetDataSize());
            // uint16   mVertexProgramConfigOffset;
            vertexDataStream.AddOffset16("VertexConfig", "Start");
            // uint16   mVertexProgramUCodeOffset;
            vertexDataStream.AddOffset16("VertexUCode", "Start");
            // uint16   mShaderConstantParametersOffset;
            vertexDataStream.AddOffset16("ShaderConstants", "Start");
            // uint16   mShaderConstantParametersSize;
            vertexDataStream.WriteUint16(vertexShaderConstantBuffer.GetSize());

            vertexDataStream.AddLabel("VertexParameters");
            if( vertexParameterStream.GetDataSize() > 0 )
               vertexDataStream.Put(vertexParameterStream.GetData(), vertexParameterStream.GetDataSize());

            vertexDataStream.AddLabel("VertexConfig");
            vertexDataStream.Put(&vertexProgramConfig, sizeof(vertexProgramConfig));

            vertexDataStream.AddLabel("VertexUCode");
            vertexDataStream.Put(pVertexProgramUCode, vertexProgramUCodeSize);

            vertexDataStream.AddLabel("ShaderConstants");
            vertexDataStream.Put(vertexShaderConstantBuffer);

            vertexDataStream.AddAlignedLabel("End", 16);

            vertexDataStream.PatchOffsets();

            vertexDataOffset = vertexShaderChunks.AddData(vertexDataStream.GetData(), vertexDataStream.GetDataSize());
            pShaderVertexData = (uint8*)vertexDataStream.GetData();
         }

         CGrowableMemoryOutStream finalStream;

         // write header
         {
            //SPassData   mPassData;
            finalStream.Put(passData);

            //uint32      mStateDataOffset;
            finalStream.WriteUint32(stateDataOffset);
            //uint32      mFragmentDataOffset;
            finalStream.WriteUint32(fragmentDataOffset);
            //uint32      mVertexDataOffset;
            finalStream.WriteUint32(vertexDataOffset);

            //uint16      mCommandBufferHoleSize;
            int const commandBufferHoleSize = MeasureShaderCommandBufferHole(pShaderVertexData, pShaderFragmentData);
            finalStream.WriteUint16(commandBufferHoleSize);
            //uin16       mPad0
            finalStream.WriteUint16(0xBABE);

            finalStream.Flush();
         }

         int const finalSize = finalStream.GetDataSize();

         SShaderPassHeader * pHeader = (SShaderPassHeader*)finalStream.GetData();

#ifdef VERBOSE_CONVERSION
         printf("Command buffer hole size: %d\n", pHeader->mCommandBufferHoleSize);
#endif
         shaderBinary.Put(finalStream.GetData(), finalSize);
      }
   }
}

//----------------------------------------------------------------------------

void WriteShaderBinaryPS3(std::vector<CCGEffectCombination> const & effectCombinations, std::string const & outputPath)
{
   CBinaryChunkWriter fragmentShaderChunks;
   CBinaryChunkWriter vertexShaderChunks;
   CBinaryChunkWriter stateChunks;

   CLabeledOffsetStream shaderBinary;

   shaderBinary.AddLabel("Start");


   // Write fourCC
   // uint32      mFourCC;
   shaderBinary.WriteUint32('CFX!');
   // Write version
   // uint32      mVersion;
   shaderBinary.WriteUint32(0);

   // Write number of combinations
   // uint32      mCombinationCount;
   shaderBinary.WriteUint32(effectCombinations.size());

   // Write offset to Combinations
   // uint32      mCombinationsOffset;
   shaderBinary.AddOffset32("Combinations", "Start");

   // Write offset to Hash table
   // uint32      mHashTableOffset;
   shaderBinary.AddOffset32("Hash Table", "Start");

   // Write offset to Fragment Data
   // uint32      mFragmentDataOffset;
   shaderBinary.AddOffset32("FragmentData", "Start");

   // Write offset to Vertex Data
   // uint32      mVertexDataOffset;
   shaderBinary.AddOffset32("VertexData", "Start");

   // Write offset to State Data
   // uint32      mStateDataOffset;
   shaderBinary.AddOffset32("StateData", "Start");

   // Write combinations
   {
      shaderBinary.AddAlignmentPadding(16);
      shaderBinary.AddLabel("Combinations");

      for( int combinationIdx = 0; combinationIdx < effectCombinations.size(); ++combinationIdx )
      {
         CCGEffectCombination const & combination = effectCombinations[combinationIdx];

         std::string combinationLabel = CStringExtras::Stringize("Combination %d", combinationIdx);
         shaderBinary.AddLabel(combinationLabel);

         CCGEffectTechnique const & technique = combination.mTechniques.front();

         // Write pass count
         shaderBinary.WriteUint32(technique.mPasses.size());

         // Write offset for each pass
         for( int passIdx = 0; passIdx < technique.mPasses.size(); ++passIdx )
         {
            shaderBinary.AddOffset32(CStringExtras::Stringize("Combination %d Pass %d", combinationIdx, passIdx), combinationLabel);
         }

         // Write passes
         for( int passIdx = 0; passIdx < technique.mPasses.size(); ++passIdx )
         {
            shaderBinary.AddLabel(CStringExtras::Stringize("Combination %d Pass %d", combinationIdx, passIdx));

            CCGEffectPass const & pass = technique.mPasses[passIdx];
            ConvertPass(combination, pass, shaderBinary, fragmentShaderChunks, vertexShaderChunks, stateChunks);
         }
      }
   }

   // Write CRC Table
   {
      shaderBinary.AddAlignmentPadding(16);
      shaderBinary.AddLabel("Hash Table");
      for( int combinationIdx = 0; combinationIdx < effectCombinations.size(); ++combinationIdx )
      {
         CCGEffectCombination const & combination = effectCombinations[combinationIdx];
         shaderBinary.WriteUint32(combination.mHash);
         shaderBinary.AddOffset32(CStringExtras::Stringize("Combination %d", combinationIdx), "Combinations");
      }
   }

   // Write fragment data buffer
   shaderBinary.AddAlignmentPadding(16);
   shaderBinary.AddLabel("FragmentData");
   shaderBinary.Put(fragmentShaderChunks.GetData(), fragmentShaderChunks.GetDataSize());

   // Write vertex data buffer
   shaderBinary.AddAlignmentPadding(16);
   shaderBinary.AddLabel("VertexData");
   shaderBinary.Put(vertexShaderChunks.GetData(), vertexShaderChunks.GetDataSize());

   // Write state data buffer
   shaderBinary.AddAlignmentPadding(16);
   shaderBinary.AddLabel("StateData");
   shaderBinary.Put(stateChunks.GetData(), stateChunks.GetDataSize());

   System::Console::WriteLine("Fragment Shader Blocks: {0}/{1}", fragmentShaderChunks.GetUniqueDataCount(), fragmentShaderChunks.GetTotalDataCount());
   System::Console::WriteLine("Vertex Shader Blocks: {0}/{1}", vertexShaderChunks.GetUniqueDataCount(), vertexShaderChunks.GetTotalDataCount());
   System::Console::WriteLine("State Blocks: {0}/{1}", stateChunks.GetUniqueDataCount(), stateChunks.GetTotalDataCount());

   shaderBinary.PatchOffsets();

   // Write binary file
   CDiskOutputStream outStream(outputPath.c_str());
   outStream.Put(shaderBinary.GetData(), shaderBinary.GetDataSize());
}