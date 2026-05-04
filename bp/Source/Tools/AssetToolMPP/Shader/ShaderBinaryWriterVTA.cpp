//----------------------------------------------------------------------------
// ShaderBinaryWriterVTA.cpp
// Copyright 2011
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

#include <gxm/program.h>
//----------------------------------------------------------------------------

namespace NShaderBinaryWriterVTA {}

using namespace NShaderBinaryWriterVTA;

static void ThrowManagedException( char const *text )
{
   System::String ^str = gcnew System::String( text );

   throw gcnew System::Exception( str );
}

template <class T>
static uint16 const SafeCastU16( char const *desc, T const &input )
{
   if ( input < 0 || input > 65536 )
   {
      char error[ 255 ];
      sprintf( error, "Casting %s to U16 failed - out of range (%d)\n", desc, int( input ) );
   }

   return uint16( input );
}

template <class T>
static uint8 const SafeCastU8( char const *desc, T const &input )
{
   if ( input < 0 || input > 255 )
   {
      char error[ 255 ];
      sprintf( error, "Casting %s to U8 failed - out of range (%d)\n", desc, int( input ) );
   }

   return uint8( input );
}

namespace NShaderBinaryWriterVTA
{
   class CBinaryChunkWriter
   {
   public:
      CBinaryChunkWriter()
         :  mTotalDataCount(0)
      {
         mStorage.SetEndian( COutputStream::kOE_LittleEndian );
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

namespace NShaderBinaryWriterVTA
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

   // Paired with Renderer\Base\Material\VTA\VTACCompiledShader.h
   struct SVertexParameter
   {
      SVertexParameter( uint32 crc, int parameterIndex, int reg, int const floatParamCount)
         : mCRC(crc)
         , mParameterIndex( SafeCastU8( "SVertexParameter::parameterIndex", parameterIndex ) )
         , mRegister(SafeCastU8( "SVertexParameter::mRegister", reg ))
         , mTotalFloatParameterCount( SafeCastU8( "SVertexParameter::mtotalFloatParamaeterCount", floatParamCount ) )
         , mPad0( 0 )
      {
      }

      void PutTo(COutputStream & stream) const
      {
         stream.WriteUint32(mCRC);
         stream.WriteUint8(mParameterIndex);
         stream.WriteUint8(mRegister);
         stream.WriteUint8( mTotalFloatParameterCount );
         stream.WriteUint8( mPad0 );
      }

      bool const operator < (SVertexParameter const & rhs) const
      {
         return mCRC < rhs.mCRC;
      }

      uint32   mCRC;
      uint8    mParameterIndex;
      uint8    mRegister;
      uint8    mTotalFloatParameterCount;
      uint8   mPad0;
   };

   // Paired with Renderer\Base\Material\VTA\VTACCompiledShader.h
   struct SVertexSemantic
   {
      SVertexSemantic( int vduType, int registerIndex, SceGxmParameterSemantic semantic, int const semanticIndex )
         : mVDUType( SafeCastU8( "vduType", vduType ) )
         , mRegisterIndex( SafeCastU8( "registerIndex", registerIndex ) )
         , mGxmSemantic( SafeCastU8( "gxmSemantic", semantic ) )
         , mGxmSemanticIndex( SafeCastU8( "gxmSemanticIndex", semanticIndex ) )
      {
      }

      void PutTo( COutputStream &stream ) const
      {
         stream.WriteUint8( mVDUType );
         stream.WriteUint8( mRegisterIndex );
         stream.WriteUint8( mGxmSemantic );
         stream.WriteUint8( mGxmSemanticIndex );
      }

      uint8    mVDUType; // One of kVDU_, or 0xFF if invalid
      uint8    mRegisterIndex;
      uint8    mGxmSemantic;
      uint8    mGxmSemanticIndex;
   };
   
   // Paired with Renderer\Base\Material\VTA\VTACCompiledShader.h
   typedef SVertexParameter SFragmentParameter;

   struct SSamplerParameter
   {
      void PutTo(COutputStream & stream) const
      {
         stream.WriteUint32(mCRC);
         stream.WriteUint16(mIndex);
         stream.WriteUint8(mMinFilter);
         stream.WriteUint8(mMagFilter);
//         stream.WriteUint8(mConv);
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
//      uint8    mConv;
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

   static inline int get_gxm_param_total_float_count( SceGxmProgramParameter const *gxmParam )
   {
      int const arrayCount = sceGxmProgramParameterGetArraySize( gxmParam );
      if ( arrayCount > 1 )
      {
         // If it's an array, then it's always 4 components
         return arrayCount * 4;
      }
      else
      {
         return sceGxmProgramParameterGetComponentCount( gxmParam );
      }
   }

   static EVertexDataUsage sem_to_vdu( SceGxmParameterSemantic const sem, int index )
   {
      switch ( sem )
      {
      case SCE_GXM_PARAMETER_SEMANTIC_POSITION:
         return kVDU_Position;
      case SCE_GXM_PARAMETER_SEMANTIC_BLENDWEIGHT:
         return kVDU_BlendWeight;
      case SCE_GXM_PARAMETER_SEMANTIC_NORMAL:
         return kVDU_Normal;
      case SCE_GXM_PARAMETER_SEMANTIC_COLOR:
         switch ( index )
         {
         case 0:
            return kVDU_Color0;
         case 1:
            return kVDU_Color1;
         }
         break;
      case SCE_GXM_PARAMETER_SEMANTIC_FOGCOORD:
         return kVDU_FogCoord;
      case SCE_GXM_PARAMETER_SEMANTIC_POINTSIZE:
         return kVDU_PSize;
      case SCE_GXM_PARAMETER_SEMANTIC_BLENDINDICES:
         return kVDU_BlendIndices;
      case SCE_GXM_PARAMETER_SEMANTIC_TEXCOORD:
         if ( index <= 5 )
         {
            return EVertexDataUsage( kVDU_TexCoord0 + index );
         }
         break;
      case SCE_GXM_PARAMETER_SEMANTIC_TANGENT:
         return kVDU_Tangent;
      case SCE_GXM_PARAMETER_SEMANTIC_BINORMAL:
         return kVDU_Binormal;
      }

      printf( "Invalid sem/index combo: %d %d\n", sem, index );
      BPE_VERIFYA( false, "Invalid sem/index combo" );
      return kVDU_Position;
   }


   void ConvertVertexParameters(SceGxmProgram * program, std::vector<SVertexParameter> & vertexParameters, std::vector<SVertexSemantic> &vertexSemantics, TVTARegisterMap const &registers )
   {
      int const parameterCount = sceGxmProgramGetParameterCount( program );

//      int const kNameBufferSize = 256;
//      char name[kNameBufferSize];

      for( int parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex )
      {
         // only process input parameters
//         if (cellCgbLevelCMapGetDirection(&program, parameterIndex) == CG_IN)
         {
            SceGxmProgramParameter const *parameter = sceGxmProgramGetParameter( program, parameterIndex );
            SceGxmParameterCategory const category = sceGxmProgramParameterGetCategory( parameter );

            // We only want to write uniform parameters that live in the default uniform buffer
            // If our uniform buffer index is max, which means that it's in the default uniform buffer
            // This is undocumented.
            if( category == SCE_GXM_PARAMETER_CATEGORY_UNIFORM && sceGxmProgramParameterGetContainerIndex( parameter ) >= SCE_GXM_MAX_UNIFORM_BUFFERS )
            {
               char const *name = sceGxmProgramParameterGetName( parameter );

               int const bufferOffset = sceGxmProgramParameterGetResourceIndex( parameter );
               SceGxmParameterType const type = sceGxmProgramParameterGetType( parameter );
               int const valueCount = sceGxmProgramParameterGetComponentCount(parameter);
               int const arraySize = sceGxmProgramParameterGetArraySize( parameter );

               TVTARegisterMap::const_iterator found = registers.find( name );

               uint16 virtualRegisterIndex = ( found == registers.end() ) ? gkUint16Max : found->second;

               SVertexParameter parameter( 
                  CShaderCRCs::GetParameterCRC( name ),
                  parameterIndex, 
                  virtualRegisterIndex,
                  get_gxm_param_total_float_count( parameter ) );

               vertexParameters.push_back(parameter);

#ifdef VERBOSE_CONVERSION
               printf("Vertex Parameter: %s (%8.8x), Register: %d Values: %d Res: %d Type: %d\n", name, parameter.mCRC, parameter.mRegister, parameter.mNumFloats, resource, type);
#endif

            }
            else if ( category == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE )
            {
              SceGxmParameterSemantic semantic = sceGxmProgramParameterGetSemantic( parameter );

              if ( semantic != SCE_GXM_PARAMETER_SEMANTIC_NONE )
              {
                 EVertexDataUsage vdu = sem_to_vdu( semantic, sceGxmProgramParameterGetSemanticIndex( parameter ) );
                 int const registerIndex = sceGxmProgramParameterGetResourceIndex( parameter );

                 vertexSemantics.push_back( SVertexSemantic( vdu, registerIndex, semantic, sceGxmProgramParameterGetSemanticIndex( parameter ) ) );
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
         return SCE_GXM_TEXTURE_FILTER_LINEAR;

      case GL_NEAREST:
         return SCE_GXM_TEXTURE_FILTER_POINT;

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
         return SCE_GXM_TEXTURE_ADDR_REPEAT;

      case GL_CLAMP_TO_EDGE:
         return SCE_GXM_TEXTURE_ADDR_CLAMP_IGNORE_BORDER ;

      case GL_MIRRORED_REPEAT:
         return SCE_GXM_TEXTURE_ADDR_MIRROR ;

      case GL_CLAMP_TO_BORDER:
         return SCE_GXM_TEXTURE_ADDR_CLAMP_FULL_BORDER ;

      case GL_CLAMP:
         return SCE_GXM_TEXTURE_ADDR_CLAMP ;

      default:
         return SCE_GXM_TEXTURE_ADDR_REPEAT ;
      }
   }

   void ConvertSamplerSettings(CCGEffectSampler const & sampler, SSamplerParameter & parameter)
   {
      parameter.mMinFilter = SCE_GXM_TEXTURE_FILTER_LINEAR;
      parameter.mMagFilter = SCE_GXM_TEXTURE_FILTER_LINEAR;
//      parameter.mConv = CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX;
      parameter.mWrapU = SCE_GXM_TEXTURE_ADDR_REPEAT;
      parameter.mWrapV = SCE_GXM_TEXTURE_ADDR_REPEAT;
      parameter.mWrapW = SCE_GXM_TEXTURE_ADDR_REPEAT;

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

#if 0
         case kState_Convolution:
            parameter.mConv = pNumeric->mData32.i1;
            break;
#endif
         case kState_Invalid:
            bpe_debugger_printf("Invalid Sampler State: %s\n", stateAssignment.mState.mName.c_str());
            break;
         }
      }
   }

   void ConvertSamplerParameter(CCGEffectCombination const & combination, SceGxmProgramParameter const *gxmParameter, int const parameterIndex, std::vector<SSamplerParameter> & samplerParameters)
   {
      char const *name = sceGxmProgramParameterGetName( gxmParameter );

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
            parameter.mIndex = parameterIndex; // sceGxmProgramParameterGetResourceIndex( gxmParameter );
            ConvertSamplerSettings(*pSampler, parameter);

            samplerParameters.push_back(parameter);

#ifdef VERBOSE_CONVERSION
            printf("Sampler[%d]: %s (%8.8x) min: %d mag: %d\n", parameter.mIndex, safeSemantic.c_str(), parameter.mCRC, parameter.mMinFilter, parameter.mMagFilter);
#endif
         }
      }
   }

   void ConvertFragmentParameter(SceGxmProgramParameter const *gxmParameter, int const parameterIndex, std::vector<SFragmentParameter> & fragmentParameters, TVTARegisterMap const &registers )
   {
      char const *name = sceGxmProgramParameterGetName( gxmParameter );

      BPE_ASSERT( NULL == strchr( name, '[' ), "Found weird array thing" );

      TVTARegisterMap::const_iterator found = registers.find( name );

      uint16 virtualRegisterIndex = ( found == registers.end() ) ? gkUint16Max : found->second;

      SFragmentParameter parameter( CShaderCRCs::GetParameterCRC(name), parameterIndex, virtualRegisterIndex, get_gxm_param_total_float_count( gxmParameter ) );

      fragmentParameters.push_back(parameter);
   }

   void ConvertFragmentParameters(CCGEffectCombination const & combination, SceGxmProgram const * program, TVTARegisterMap const &registers, std::vector<SFragmentParameter> & fragmentParameters, std::vector<SSamplerParameter> & samplerParameters)
   {
      int const parameterCount = sceGxmProgramGetParameterCount( program );

      for( int parameterIndex = 0; parameterIndex < parameterCount; ++parameterIndex )
      {
         // Only process parameters that are input parameters
//         if (cellCgbLevelCMapGetDirection(&program, parameterIndex) == CG_IN)
         {
            // Only care about uniform parameters
            SceGxmProgramParameter const *gxmParameter = sceGxmProgramGetParameter( program, parameterIndex );
            SceGxmParameterCategory const category = sceGxmProgramParameterGetCategory( gxmParameter );
            
            if ( category == SCE_GXM_PARAMETER_CATEGORY_SAMPLER )
            {
               ConvertSamplerParameter( combination, gxmParameter, parameterIndex, samplerParameters );
            }
            else if( category == SCE_GXM_PARAMETER_CATEGORY_UNIFORM )
            {
               ConvertFragmentParameter(gxmParameter, parameterIndex, fragmentParameters, registers);
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

#if 0
   void SetupGCMContext(cell::Gcm::Unsafe::CellGcmContext* pContext, void* pData, int size, CellGcmContextCallback callback)
   {
      pContext->begin = pContext->current = (uint32_t*)pData;
      pContext->end = pContext->begin + size;
      pContext->callback = callback;
   }
#endif
   void ConvertPass(CCGEffectCombination const & combination, CCGEffectPass const & pass, TVTARegisterMap const &registers, CLabeledOffsetStream & shaderBinary, CBinaryChunkWriter & fragmentShaderChunks, CBinaryChunkWriter & vertexShaderChunks, CBinaryChunkWriter & stateDataChunks)
   {
      SShaderPassData passData;

      std::vector<SVertexParameter> vertexParameters;
      std::vector<SVertexSemantic> vertexSemantics;
      std::vector<SFragmentParameter> fragmentParameters;
      std::vector<SSamplerParameter> samplerParameters;

//      int const kStateCommandBufferMaxSize = 2 * 1024;
//      uint32 stateSetBufferMemory[kStateCommandBufferMaxSize];
//      cell::Gcm::Unsafe::CellGcmContext stateSetContext;
//      SetupGCMContext(&stateSetContext, (uint32_t*)stateSetBufferMemory, kStateCommandBufferMaxSize, NULL);

//      uint32 stateResetBufferMemory[kStateCommandBufferMaxSize];
//      cell::Gcm::Unsafe::CellGcmContext stateResetContext;
//      SetupGCMContext(&stateResetContext, (uint32_t*)stateResetBufferMemory, kStateCommandBufferMaxSize, NULL);

      int const kShaderConstantBufferMaxSize = 16 * 1024;
      uint8 fragmentShaderConstantBufferMemory[kShaderConstantBufferMaxSize];
      CShaderParameterBuffer fragmentShaderConstantBuffer(fragmentShaderConstantBufferMemory);
      uint8 vertexShaderConstantBufferMemory[kShaderConstantBufferMaxSize];
      CShaderParameterBuffer vertexShaderConstantBuffer(vertexShaderConstantBufferMemory);

      void const * pFragmentProgramUCode = NULL;
      uint32 fragmentProgramUCodeSize = 0;

      void const * pVertexProgramUCode = NULL;
      uint32 vertexProgramUCodeSize = 0;
#if 0
      CellCgbFragmentProgramConfiguration fragmentProgramConfig;

      CellCgbVertexProgramConfiguration vertexProgramConfig;

      uint32 alphaFunc = CELL_GCM_GREATER;
      uint32 alphaRef = 0;
      bool hasAlphaTest = false;
#endif

      foreach(CCGEffectStateAssignment const & stateAssignment, pass.mStateAssignments.mData)
      {
         CCGEffectDataNumeric const * pNumeric = static_cast<CCGEffectDataNumeric const *>(stateAssignment.mpData.get());
         CCGEffectDataProgram const * pProgram = static_cast<CCGEffectDataProgram const *>(stateAssignment.mpData.get());

         switch(stateAssignment.mState.mState)
         {
         case kState_AlphaBlendEnable:
#if 0
            if( pNumeric->mData8.b1 != false )
            {
               stateSetContext.SetBlendEnable(CELL_GCM_TRUE);
               stateResetContext.SetBlendEnable(CELL_GCM_FALSE);
            }
#endif
            break;

         case kState_AlphaTestEnable:
#if 0
            if( pNumeric->mData8.b1 != false )
            {
               stateSetContext.SetAlphaTestEnable(CELL_GCM_TRUE);
               stateResetContext.SetAlphaTestEnable(CELL_GCM_FALSE);
            }
#endif
            break;

         case kState_AlphaRef:
#if 0
            alphaRef = (uint32)(pNumeric->mData32.f1 * 255.0f);
            hasAlphaTest = true;
#endif
            break;

         case kState_AlphaFunc:
#if 0
            alphaFunc = (uint32)pNumeric->mData32.f1;
            hasAlphaTest = true;
#endif
            break;

         case kState_BlendFunc:
#if 0
            stateSetContext.SetBlendFunc(pNumeric->mData32.i2[0], pNumeric->mData32.i2[1], pNumeric->mData32.i2[0], pNumeric->mData32.i2[1]);
#endif
            break;

         case kState_BlendFuncSeparate:
#if 0
            stateSetContext.SetBlendFunc(pNumeric->mData32.i4[0], pNumeric->mData32.i4[1], pNumeric->mData32.i4[2], pNumeric->mData32.i4[3]);
#endif
            break;

         case kState_ZEnable:
#if 0
            passData.SetFlag(SShaderPassData::kDepthTest, pNumeric->mData8.b1);
#endif
            break;

         case kState_ZWriteEnable:
#if 0
            if( pNumeric->mData8.b1 != true )
            {
               stateSetContext.SetDepthMask(CELL_GCM_FALSE);
               stateResetContext.SetDepthMask(CELL_GCM_TRUE);
            }
#endif
            break;

         case kState_ColorMask:
#if 0
            passData.SetFlag(SShaderPassData::kColorMaskR, pNumeric->mData8.b4[0]);
            passData.SetFlag(SShaderPassData::kColorMaskG, pNumeric->mData8.b4[1]);
            passData.SetFlag(SShaderPassData::kColorMaskB, pNumeric->mData8.b4[2]);
            passData.SetFlag(SShaderPassData::kColorMaskA, pNumeric->mData8.b4[3]);
#endif
            break;

         case kState_VertexShader:
            {
               SceGxmProgram *vertexProgram = (SceGxmProgram *) pProgram->mData.get();

               pVertexProgramUCode = vertexProgram;
               vertexProgramUCodeSize = sceGxmProgramGetSize( vertexProgram );

               ConvertVertexParameters(vertexProgram, vertexParameters, vertexSemantics, registers);
            }
            break;

         case kState_PixelShader:
            {
               SceGxmProgram *fragmentProgram = (SceGxmProgram *) pProgram->mData.get();
#if 0
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

#endif
               pFragmentProgramUCode = fragmentProgram;
               fragmentProgramUCodeSize = sceGxmProgramGetSize( fragmentProgram );

               ConvertProgramParameters(pProgram, fragmentShaderConstantBuffer);

               ConvertFragmentParameters(combination, fragmentProgram, registers, fragmentParameters, samplerParameters);
            }

            break;

         case kState_CullFaceEnable:
#if 0
            stateSetContext.SetCullFaceEnable(pNumeric->mData8.b1);
            stateResetContext.SetCullFaceEnable(CELL_GCM_TRUE);
#endif
            break;

         case kState_CullFace:
#if 0
            stateSetContext.SetCullFace( pNumeric->mData32.i1 );
            stateResetContext.SetCullFace( CELL_GCM_BACK );
#endif
            break;

         case kState_Invalid:
            bpe_debugger_printf("CCompiledShader::ConvertPass: Unsupported state: %s\n", stateAssignment.mState.mName.c_str());
            break;

         }
      }

#if 0
      if( hasAlphaTest )
      {
         stateSetContext.SetAlphaFunc(alphaFunc, alphaRef);
         stateResetContext.SetAlphaFunc(CELL_GCM_GREATER, 0);
      }
#endif

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
#if 0
         int const stateSetSize = (stateSetContext.current - stateSetContext.begin) * sizeof(uint32);
         int const stateResetSize = (stateResetContext.current - stateResetContext.begin) * sizeof(uint32);
#endif
         CGrowableMemoryOutStream vertexParameterStream;
         vertexParameterStream.SetEndian( COutputStream::kOE_LittleEndian );
         {
            std::sort(vertexParameters.begin(), vertexParameters.end());
            for( int i = 0; i < vertexParameters.size(); ++i )
               vertexParameterStream.Put(vertexParameters[i]);

            // Make the "zero" parameter last
            SVertexParameter const last( 0, 0, 0, 0 );
            vertexParameterStream.Put( last );
         }

         CGrowableMemoryOutStream vertexSemanticStream;
         vertexSemanticStream.SetEndian( COutputStream::kOE_LittleEndian );
         {
            for ( int i = 0; i < vertexSemantics.size(); ++i )
            {
               vertexSemanticStream.Put( vertexSemantics[i] );
            }

            // Make last parameter
            SVertexSemantic const lastSemantic( 0xFF, 0,SCE_GXM_PARAMETER_SEMANTIC_NONE, 0 );
            vertexSemanticStream.Put( lastSemantic );
         }

         CGrowableMemoryOutStream fragmentParameterStream;
         fragmentParameterStream.SetEndian( COutputStream::kOE_LittleEndian );
         {
            std::sort(fragmentParameters.begin(), fragmentParameters.end());
            for( int i = 0; i < fragmentParameters.size(); ++i )
               fragmentParameterStream.Put(fragmentParameters[i]);

            // Make a zero crc param last
            SFragmentParameter const last( 0, 0, 0, 0 );
            fragmentParameterStream.Put( last );
         }

         CGrowableMemoryOutStream samplerParameterStream;
         samplerParameterStream.SetEndian( COutputStream::kOE_LittleEndian );
         {
            std::sort(samplerParameters.begin(), samplerParameters.end());
            for( int i = 0; i < samplerParameters.size(); ++i )
               samplerParameterStream.Put(samplerParameters[i]);
         }

#if 0
         // State Set Data
         uint32 stateDataOffset;
         {
            CLabeledOffsetStream stateDataStream;
            stateDataStream.SetEndian( COutputStream::kOE_LittleEndian );

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

#endif

         CLabeledOffsetStream fragmentDataStream;
         fragmentDataStream.SetEndian( COutputStream::kOE_LittleEndian );

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
//            fragmentDataStream.AddOffset16("FragmentConfig", "Start");
            // uint16   mFragmentProgramUCodeOffset;
            fragmentDataStream.AddOffset16("FragmentUCode", "Start");
            // uint16   mFragmentProgramUCodeSize;
            fragmentDataStream.WriteUint16((uint16)fragmentProgramUCodeSize);
            // uint16   mShaderConstantParametersOffset;
            fragmentDataStream.AddOffset16("ShaderConstants", "Start");
            // uint16   mShaderConstantParametersSize;
            fragmentDataStream.WriteUint16(fragmentShaderConstantBuffer.GetSize());
            // uint16   mPad0;
            fragmentDataStream.WriteUint16( 0xBABE /* pad */ );
            // uint32   mRuntimeShaderPatcherId; // This is set at run-time
            fragmentDataStream.WriteUint32( 0x00000000 );

            fragmentDataStream.AddLabel("FragmentParameters");
            if( fragmentParameterStream.GetDataSize() > 0 )
               fragmentDataStream.Put(fragmentParameterStream.GetData(), fragmentParameterStream.GetDataSize());

            fragmentDataStream.AddLabel("SamplerParameters");
            if( samplerParameterStream.GetDataSize() > 0 )
               fragmentDataStream.Put(samplerParameterStream.GetData(), samplerParameterStream.GetDataSize());

#if 0
            fragmentDataStream.AddLabel("FragmentConfig");
            fragmentDataStream.Put(&fragmentProgramConfig, sizeof(fragmentProgramConfig));
#endif

            fragmentDataStream.AddAlignmentPadding( 8 );
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
         vertexDataStream.SetEndian( COutputStream::kOE_LittleEndian );

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
            // uint16   mVertexProgramSemanticsOffset;
            vertexDataStream.AddOffset16("VertexSemantics", "Start");
            // uint16   mVertexProgramUCodeOffset;
            vertexDataStream.AddOffset16("VertexUCode", "Start");
            // uint16   mShaderConstantParametersOffset;
            vertexDataStream.AddOffset16("ShaderConstants", "Start");
            // uint16   mShaderConstantParametersSize;
            vertexDataStream.WriteUint16(vertexShaderConstantBuffer.GetSize());
            // uint32   mRuntimeShaderPatcherId; // This is set at run-time
            vertexDataStream.WriteUint32( 0x00000000 );

            vertexDataStream.AddLabel("VertexParameters");
            vertexDataStream.Put(vertexParameterStream.GetData(), vertexParameterStream.GetDataSize());

            vertexDataStream.AddLabel("VertexSemantics");
            vertexDataStream.Put(vertexSemanticStream.GetData(), vertexSemanticStream.GetDataSize() );

            vertexDataStream.AddAlignmentPadding( 8 );
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
         finalStream.SetEndian( COutputStream::kOE_LittleEndian );

         // write header
         {
            //SPassData   mPassData;
            finalStream.Put(passData);

#if 0
            //uint32      mStateDataOffset;
            finalStream.WriteUint32(stateDataOffset);
#endif
            //uint32      mFragmentDataOffset;
            finalStream.WriteUint32(fragmentDataOffset);
            //uint32      mVertexDataOffset;
            finalStream.WriteUint32(vertexDataOffset);
            //uint32      mRegisterDataOffset;
            finalStream.WriteUint32(0); // all point to the same reg data
#if 0
            //uint16      mCommandBufferHoleSize;
            int const commandBufferHoleSize = MeasureShaderCommandBufferHole(pShaderVertexData, pShaderFragmentData);
            finalStream.WriteUint16(commandBufferHoleSize);
            //uin16       mPad0
            finalStream.WriteUint16(0xBABE);
#endif

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

void WriteShaderBinaryVTA(std::vector<CCGEffectCombination> const & effectCombinations, std::string const & outputPath,
                          TVTARegisterMap const &registers )
{
   //_CrtSetBreakAlloc(992);

   CBinaryChunkWriter fragmentShaderChunks;
   CBinaryChunkWriter vertexShaderChunks;
   CBinaryChunkWriter stateChunks;

   CLabeledOffsetStream shaderBinary;
   shaderBinary.SetEndian( COutputStream::kOE_LittleEndian );

   shaderBinary.AddLabel("Start");


   // Write fourCC
   // uint32      mFourCC;
   shaderBinary.WriteUint32('CFX!');
   // Write version
   // uint32      mVersion;
   // DON'T FORGET TO UPDATE AssetTypes.cs's version!
   shaderBinary.WriteUint32(4);

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

#if 0
   // Write offset to State Data
   // uint32      mStateDataOffset;
   shaderBinary.AddOffset32("StateData", "Start");
#endif

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
            ConvertPass(combination, pass, registers, shaderBinary, fragmentShaderChunks, vertexShaderChunks, stateChunks);
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
#if 0
   shaderBinary.AddAlignmentPadding(16);
   shaderBinary.AddLabel("StateData");
   shaderBinary.Put(stateChunks.GetData(), stateChunks.GetDataSize());
#endif

   System::Console::WriteLine("Fragment Shader Blocks: {0}/{1}", fragmentShaderChunks.GetUniqueDataCount(), fragmentShaderChunks.GetTotalDataCount());
   System::Console::WriteLine("Vertex Shader Blocks: {0}/{1}", vertexShaderChunks.GetUniqueDataCount(), vertexShaderChunks.GetTotalDataCount());
#if 0
   System::Console::WriteLine("State Blocks: {0}/{1}", stateChunks.GetUniqueDataCount(), stateChunks.GetTotalDataCount());
#endif

   shaderBinary.PatchOffsets();

   // Write binary file
   CDiskOutputStream outStream(outputPath.c_str());
   outStream.Put(shaderBinary.GetData(), shaderBinary.GetDataSize());
}