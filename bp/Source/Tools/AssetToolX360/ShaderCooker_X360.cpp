//----------------------------------------------------------------------------
// ShaderCooker_X360.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------
#include "stdafx.h"

//----------------------------------------------------------------------------
#include "boost/shared_ptr.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/scoped_array.hpp"

#include "Engine/System/CFileUtils.h"
#include "Engine/Mechanics/TComPtr.h"
#include "Engine/Mechanics/CCRC.h"

#include "Renderer/Base/Material/X360/X360CookedShaderFormat.h"

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3dx9effect.h>
#include <d3dx9shader.h>
#include <xgraphics.h>
#include <fxl.h>

class CShaderCRCs
{
protected:
   CShaderCRCs() {};

public:
   static uint32 const GetParameterCRC(const char * pParameterName)
   {
      uint32 crcValue = 0xFFFFFFFF;

      if (pParameterName) 
      {
         while(*pParameterName) 
         {
            uint32 tableTemp = (uint32)((crcValue & 0xff) ^ (uint8)*pParameterName);

            for (int bitLoop = 0; bitLoop < 8; bitLoop++) 
            {
               if (tableTemp & 1)
                  tableTemp = (tableTemp >> 1) ^ 0xedb88320;
               else
                  tableTemp >>= 1;
            }

            crcValue = (crcValue >> 8) ^ tableTemp;
            pParameterName++;
         }
      }

      return crcValue;
   }
};

void ComputeHash( VertexShader_X360& item )
{
   if( item.mVertexShaderCode.size() )
      item.mHash = CCRC::CalculateCRC32(&item.mVertexShaderCode[0], item.mVertexShaderCode.size()); 
}

void ComputeHash( PixelShader_X360& item )
{
   if( item.mPixelShaderCode.size() )
      item.mHash = CCRC::CalculateCRC32(&item.mPixelShaderCode[0], item.mPixelShaderCode.size()); 
}

void ComputeHash( Params_X360& item )
{
   if( item.mParams.size() )
      item.mHash = CCRC::CalculateCRC32(&item.mParams[0], item.mParams.size()); 
}

void ComputeHash( SamplerStates_X360& item )
{
   if( item.mSamperStates.size() )
      item.mHash = CCRC::CalculateCRC32(&item.mSamperStates[0], item.mSamperStates.size()); 
}

void ComputeHash( RenderStates_X360& item )
{
   if( item.mRenderStates.size() )
      item.mHash = CCRC::CalculateCRC32(&item.mRenderStates[0], item.mRenderStates.size()); 
}

inline void AddVertexPixelParams(char const * paramName, uint32 const parameterCRC, CUSTOMPARAMETER_CONTEXT paramContext, Params_X360 & params,
                                 uint32 vRegisterIndex, uint32 vRegisterCount,
                                 uint32 pRegisterIndex, uint32 pRegisterCount)
{
   params.mParams.push_back( SParam_X360(parameterCRC, paramContext, vRegisterIndex, vRegisterCount, pRegisterIndex, pRegisterCount) );
   params.mParams.back().paramName = paramName;
}

template <class myType>
void RemoveIfDuplicateAndReAssignIndex( myType & object, std::vector<myType> & vectorObject, int32 & index )
{
   ComputeHash( object );
   int iLoop = 0;
   // Be careful to not loop over the item we just added, it is ourself
   int endSize = vectorObject.size()-1;
   for( ; iLoop<endSize; ++iLoop )
   {
      if( object == vectorObject[iLoop] )
      {
         break;
      }
   }
   if( iLoop != endSize )
   {
      // we can delete this entry, point array index to iLoop
      vectorObject.erase( vectorObject.end()-1 );
      index = iLoop;
   }
}

#define X360TOOLSAPI extern "C" __declspec(dllexport)

//----------------------------------------------------------------------------

SamplerState_X360 GetDefaultSamplerStateFor(unsigned int reg, D3DSAMPLERSTATETYPE type, unsigned int value)
{
   static const float zero_f = 0.0f;
   static const DWORD zero_f_dw = *(DWORD*)&zero_f;
   unsigned int defaultValue = 0;

   switch( type )
   {
   case D3DSAMP_ADDRESSU:
   case D3DSAMP_ADDRESSV:
   case D3DSAMP_ADDRESSW:
      defaultValue = D3DTADDRESS_WRAP;
      break;
   case D3DSAMP_BORDERCOLOR:
      defaultValue = 0x00000000;
      break;
   case D3DSAMP_MAGFILTER:
   case D3DSAMP_MINFILTER:
   case D3DSAMP_MAGFILTERZ:
   case D3DSAMP_MINFILTERZ:
      defaultValue = D3DTEXF_POINT;
      break;
   case D3DSAMP_MIPFILTER:
      defaultValue = D3DTEXF_NONE;
      break;
   case D3DSAMP_SEPARATEZFILTERENABLE:
   case D3DSAMP_WHITEBORDERCOLORW:
      defaultValue = FALSE;
      break;
   case D3DSAMP_POINTBORDERENABLE:
      defaultValue = TRUE;
      break;
   case D3DSAMP_MIPMAPLODBIAS:
   case D3DSAMP_ANISOTROPYBIAS:
      defaultValue = zero_f_dw;
      break;
   case D3DSAMP_MAXMIPLEVEL:
   case D3DSAMP_HGRADIENTEXPBIAS:
   case D3DSAMP_VGRADIENTEXPBIAS:
      defaultValue = 0;
      break;
   case D3DSAMP_MAXANISOTROPY:
      defaultValue = 1;
      break;
   case D3DSAMP_MINMIPLEVEL:
      defaultValue = 13;
      break;
   case D3DSAMP_TRILINEARTHRESHOLD:
      defaultValue = D3DTRILINEAR_IMMEDIATE;
      break;
   default:
      BPE_VERIFYA(false, "Unsupported samplerstate!");
      break;
   }

   return SamplerState_X360(reg, type, defaultValue );
}

//----------------------------------------------------------------------------

RenderState_X360 GetDefaultRenderStateFor(D3DRENDERSTATETYPE type, unsigned int value)
{
   unsigned int defaultValue = 0;

   switch( type )
   {
   case D3DRS_ZENABLE:
   case D3DRS_ZWRITEENABLE:
      defaultValue = TRUE;
      break;
   case D3DRS_ALPHABLENDENABLE:
   case D3DRS_SEPARATEALPHABLENDENABLE:
   case D3DRS_ALPHATESTENABLE:
      defaultValue = FALSE;
      break;
   case D3DRS_SRCBLEND:
   case D3DRS_SRCBLENDALPHA:
      defaultValue = D3DBLEND_ONE;
      break;
   case D3DRS_DESTBLEND:
   case D3DRS_DESTBLENDALPHA:
      defaultValue = D3DBLEND_ZERO;
      break;
   case D3DRS_COLORWRITEENABLE:
   case D3DRS_COLORWRITEENABLE1:
   case D3DRS_COLORWRITEENABLE2:
   case D3DRS_COLORWRITEENABLE3:
      defaultValue = D3DCOLORWRITEENABLE_ALL;
      break;
   case D3DRS_CULLMODE:
      defaultValue = D3DCULL_CCW;
      break;
   case D3DRS_ALPHAREF:
      defaultValue = 0;
      break;
   case D3DRS_ALPHAFUNC:
      defaultValue = D3DCMP_ALWAYS;
      break;
   default:
      BPE_VERIFYA(false, "Unsupported renderstate!");
      break;
   }

   return RenderState_X360(type, defaultValue );
}

//----------------------------------------------------------------------------

FXLHANDLE FindAnnotationFromParameter(FXLEffect * pEffect, FXLHANDLE handle, FXLPARAMETER_DESC const & desc, char const * pAnnotationName, FXLANNOTATION_DESC & AnnotationDesc)
{
   for( int iAnnotation=0;iAnnotation<desc.Annotations;++iAnnotation )
   {
      FXLHANDLE AnnotationHandle = pEffect->GetAnnotationHandleFromIndex(handle, iAnnotation);
      pEffect->GetAnnotationDesc(AnnotationHandle, &AnnotationDesc);
      if( strcmpi(AnnotationDesc.pName, pAnnotationName) ==0 )
      {
         return AnnotationHandle;
      }
   }
   return 0;
}

//----------------------------------------------------------------------------

FXLHANDLE FindAnnotationFromPass(FXLEffect * pEffect, FXLHANDLE passHandle, FXLPASS_DESC const & passDesc, char const * pAnnotationName, FXLANNOTATION_DESC & AnnotationDesc)
{
   for( int iAnnotation=0;iAnnotation<passDesc.Annotations;++iAnnotation )
   {
      FXLHANDLE AnnotationHandle = pEffect->GetAnnotationHandleFromIndex(passHandle, iAnnotation);
      pEffect->GetAnnotationDesc(AnnotationHandle, &AnnotationDesc);
      if( strcmpi(AnnotationDesc.pName, pAnnotationName) ==0 )
      {
         return AnnotationHandle;
      }
   }
   return 0;
}

//----------------------------------------------------------------------------
HRESULT OpenAndReadFile(LPCSTR pFileName,void ** ppData, unsigned int * pBytes)
{
   FILE * fp = fopen(pFileName, "rb");
   if( fp == NULL )
   {
      return -1;
   }
   fseek((FILE*) fp, 0, SEEK_END);
   *pBytes = ftell((FILE*) fp);
   fseek((FILE*) fp, 0, SEEK_SET);
   *ppData = new uint8[*pBytes];
   int numberOfBytes = fread((void*)*ppData, *pBytes, 1, fp);
   fclose(fp);

   return S_OK;
}
HRESULT OpenAndWriteFile(LPCSTR pFileName,void * pData, unsigned int Bytes)
{
   HANDLE handle = CreateFile( pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
   if( handle == INVALID_HANDLE_VALUE )
   {
      char shaderDumpDir[MAX_PATH];
      sprintf(shaderDumpDir, "%s\\ShaderDump", getenv("BPE_REPOSITORY"));
      CreateDirectory(shaderDumpDir, NULL);
      handle = CreateFile( pFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
      if( handle == INVALID_HANDLE_VALUE )
         return -1;
   }

   DWORD numberOfBytes;
   if( !WriteFile( handle, pData, (DWORD)Bytes, &numberOfBytes, NULL ) )
   {
      CloseHandle(handle);
      return -1;
   }
   CloseHandle(handle);
   return S_OK;
}
HRESULT CloseFile(LPCVOID pData)
{
   delete [] pData;
   return S_OK;
}
class OurFXIncludeHandler : public ID3DXInclude
{
public:
   STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes, LPSTR pFullPath, DWORD cbFullPath)
   {
      std::string inputFileName = pFileName;
      std::string unifiedPath = CFileUtils::UnifyPath(inputFileName);
      FILE * fp = fopen(unifiedPath.c_str(), "rb");
      if( fp == NULL )
      {
         enum { kBufferSize = 1024 };
         char buffer[kBufferSize];
         if( !GetEnvironmentVariable( "BPE_REPOSITORY", buffer, kBufferSize ) )
         {
            return -1;
         }

         std::string correctedFileName = buffer;
         correctedFileName += "\\EngineSupport\\Shaders\\";
         correctedFileName += pFileName;
         unifiedPath = CFileUtils::UnifyPath(correctedFileName);
         fp = fopen(unifiedPath.c_str(), "rb");
         if( fp == NULL )
         {
            return -1;
         }
      }
      fseek((FILE*) fp, 0, SEEK_END);
      *pBytes = ftell((FILE*) fp);
      fseek((FILE*) fp, 0, SEEK_SET);
      *ppData = new uint8[*pBytes];
      int numberOfBytesRead = 0;
      numberOfBytesRead = fread((void*)*ppData, *pBytes, 1, fp);
      fclose(fp);

      return S_OK;
   }
   STDMETHOD(Close)(LPCVOID pData)
   {
      delete [] pData;
      return S_OK;
   }
};
OurFXIncludeHandler g_FXIncludeHandler;

X360TOOLSAPI bool CreateShaderCombination(unsigned int definesCRC, std::string const & inputPathStr, Effect_360 & effect, std::vector<std::string> const & defineStrings, bool compileDebugShaders, std::string& errorStrings)
{
   effect.mCombinations.push_back(EffectCombination_360());
   EffectCombination_360 & effectCombination = effect.mCombinations.back();
   effectCombination.mHash = definesCRC;

   std::vector<D3DXMACRO> defineMacros;
   for( int i = 0; i < (defineStrings.size() / 2); ++i )
   {
      D3DXMACRO macro;

      macro.Name = defineStrings[i*2 + 0].c_str();
      macro.Definition = defineStrings[i*2 + 1].c_str();
      defineMacros.push_back(macro);
   }
   D3DXMACRO const kPlatformMacro = { "BPE_PLATFORM", "2" };
   defineMacros.push_back( kPlatformMacro );
   D3DXMACRO const kNullTerminator = { NULL, NULL };
   defineMacros.push_back(kNullTerminator);

   void * pCodeData;//TODO: change into a scoped_ptr?
   unsigned int fileSize;
   if( OpenAndReadFile(inputPathStr.c_str(), &pCodeData, &fileSize) != S_OK )
   {
      BPE_VERIFYA( false, "sys_memory_allocate failed" );
      return false;
   }

   char updbFileName[MAX_PATH];
   char updbActualFileName[MAX_PATH];

   DWORD compileFlags = D3DXSHADER_NO_PRESHADER|D3DXSHADER_FXLPARAMETERS_AS_VARIABLE_NAMES|D3DXSHADER_PACKMATRIX_ROWMAJOR;
   D3DXSHADER_COMPILE_PARAMETERS compileParams;
   ZeroMemory( &compileParams, sizeof(compileParams) );
   if( compileDebugShaders )
   {
      char drive[_MAX_DRIVE];
      char dir[_MAX_DIR];
      char name[_MAX_FNAME];
      char ext[_MAX_EXT];
      _splitpath_s( inputPathStr.c_str(), drive, dir, name, ext );

      sprintf(updbActualFileName, "%s\\ShaderDump\\%s_0x%X.updb", getenv("BPE_REPOSITORY"), name, definesCRC);

      compileFlags |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
      compileParams.Flags = D3DXSHADEREX_GENERATE_UPDB;
      compileParams.UPDBPath = updbActualFileName;
   }
   else
   {
      compileParams.Flags |= D3DXSHADEREX_OPTIMIZE_UCODE;
   }
   ID3DXBuffer* pEffectData = NULL;
   ID3DXBuffer* errorMsgs = NULL;
   HRESULT hr = FXLCompileEffectEx( (CHAR*)pCodeData, fileSize,
      &defineMacros[0], &g_FXIncludeHandler, compileFlags,
      &pEffectData, &errorMsgs, &compileParams );
   TComPtr<ID3DXBuffer> mem_pEffectData(pEffectData);
   TComPtr<ID3DXBuffer> mem_errorMsgs(errorMsgs);
   if( FAILED( hr ) )
   {
      errorStrings = (char*)errorMsgs->GetBufferPointer();
      return false;
   }
   if( compileDebugShaders )//output updb info
   {
      if( compileParams.pUPDBBuffer )
      {
         if( compileParams.pUPDBBuffer->GetBufferSize() )
         {
            OpenAndWriteFile(updbActualFileName, compileParams.pUPDBBuffer->GetBufferPointer(), compileParams.pUPDBBuffer->GetBufferSize());
         }
         compileParams.pUPDBBuffer->Release();
         compileParams.pUPDBBuffer = NULL;
      }
   }
   FXLEffect * pEffect;
   if( FAILED( FXLCreateEffect( NULL, pEffectData->GetBufferPointer(), NULL, &pEffect ) ) )
   {
      errorStrings = (char*)errorMsgs->GetBufferPointer();
      return false;
   }
   TComPtr<FXLEffect> comPtrEffect(pEffect);

   FXLEFFECT_DESC effectDesc;
   pEffect->GetEffectDesc(&effectDesc);
   effectCombination.mTechniques.reserve(effectDesc.Techniques);
   for( int iTechnique=0; iTechnique < effectDesc.Techniques; ++iTechnique )
   {
      effectCombination.mTechniques.push_back(Technique_360());
      Technique_360 & effectTechnique = effectCombination.mTechniques[iTechnique];

      FXLHANDLE techHandle = pEffect->GetTechniqueHandleFromIndex(iTechnique);
      FXLTECHNIQUE_DESC techDesc;
      pEffect->GetTechniqueDesc(techHandle, &techDesc);

      effectTechnique.mPasses.reserve(techDesc.Passes);
      for( int iPass=0; iPass < techDesc.Passes; ++iPass )
      {
         effectTechnique.mPasses.push_back(Pass_360());
         Pass_360 & effectPass = effectTechnique.mPasses[iPass];

         effect.mVertexShader.push_back(VertexShader_X360());
         VertexShader_X360 & vertexShader = effect.mVertexShader.back();
         effectPass.mVertexShader_Index = effect.mVertexShader.size()-1;

         effect.mPixelShader.push_back(PixelShader_X360());
         PixelShader_X360 & pixelShader = effect.mPixelShader.back();
         effectPass.mPixelShader_Index = effect.mPixelShader.size()-1;

         effect.mSamperStates.push_back(SamplerStates_X360());
         SamplerStates_X360 & samplerStates = effect.mSamperStates.back();
         effectPass.mSamperStates_Index = effect.mSamperStates.size()-1;

         effect.mRenderStates.push_back(RenderStates_X360());
         RenderStates_X360 & renderStates = effect.mRenderStates.back();
         effectPass.mRenderStates_Index = effect.mRenderStates.size()-1;

         effect.mParams.push_back(Params_X360());
         Params_X360 & params = effect.mParams.back();
         effectPass.mParams_Index = effect.mParams.size()-1;
         /////////////////////////////////////////////////////////////////////////////////

         FXLHANDLE passHandle = pEffect->GetPassHandleFromIndex(techHandle, iPass);
         FXLPASS_DESC passDesc;
         pEffect->GetPassDesc(passHandle, &passDesc);
         //RenderStates
         if( passDesc.RenderStates )
         {
            renderStates.mRenderStates.reserve(passDesc.RenderStates);
            renderStates.mRenderStates_Restore.reserve(passDesc.RenderStates);
            for( int iRenderState=0; iRenderState < passDesc.RenderStates; ++iRenderState )
            {
               D3DRENDERSTATETYPE type;
               unsigned int value;
               pEffect->GetRenderState(passHandle, iRenderState, &type, (DWORD*)&value );
               renderStates.mRenderStates.push_back(RenderState_X360(type, value));
               renderStates.mRenderStates_Restore.push_back(GetDefaultRenderStateFor(type, value));
            }
         }
         //SamplerStates
         if( passDesc.SamplerStates )
         {
            samplerStates.mSamperStates.reserve(passDesc.SamplerStates);
            samplerStates.mSamperStates_Restore.reserve(passDesc.SamplerStates);
            for( int iSamplerState=0; iSamplerState < passDesc.SamplerStates; ++iSamplerState )
            {
               unsigned int reg;
               D3DSAMPLERSTATETYPE type;
               unsigned int value;
               pEffect->GetSamplerState(passHandle, iSamplerState, &reg, &type, (DWORD*)&value );
               samplerStates.mSamperStates.push_back(SamplerState_X360(reg, type, value));
               samplerStates.mSamperStates_Restore.push_back(GetDefaultSamplerStateFor(reg, type, value));
            }
         }

         {
            char * pVertexShaderFunction = (char *)passDesc.pVertexShaderFunction;
            vertexShader.mVertexShaderCode.reserve(passDesc.VertexShaderFunctionSize);
            for( int iVSC=0; iVSC < passDesc.VertexShaderFunctionSize; ++iVSC )
            {
               vertexShader.mVertexShaderCode.push_back(pVertexShaderFunction[iVSC]);
            }
         }
         {
            char * pPixelShaderFunction = (char *)passDesc.pPixelShaderFunction;
            pixelShader.mPixelShaderCode.reserve(passDesc.PixelShaderFunctionSize);
            for( int iPSC=0; iPSC < passDesc.PixelShaderFunctionSize; ++iPSC )
            {
               pixelShader.mPixelShaderCode.push_back(pPixelShaderFunction[iPSC]);
            }
         }

         for( int iParam = 0; iParam < effectDesc.Parameters; ++iParam )
         {
            FXLHANDLE paramHandle = pEffect->GetParameterHandleFromIndex(iParam);

            FXLPARAMETER_CONTEXT paramContext = pEffect->GetParameterContext(passHandle, paramHandle);
            if( paramContext )
            {
               FXLPARAMETER_DESC paramDesc;
               pEffect->GetParameterDesc(paramHandle, &paramDesc);

               switch(paramDesc.Type)
               {
               case FXLDTYPE_SAMPLER:
                  {
                     if( !(paramContext & (FXLPCONTEXT_VERTEXSHADERSAMPLER|FXLPCONTEXT_PIXELSHADERSAMPLER)) )
                     {
                        BPE_VERIFYA(false, "Invalid parameter context for sampler");
                     }

                     unsigned int vRegisterIndex = -1;
                     unsigned int pRegisterIndex = -1;
                     unsigned int vRegisterCount = 0;
                     unsigned int pRegisterCount = 0;
                     if( paramContext & FXLPCONTEXT_VERTEXSHADERSAMPLER )
                     {
                        pEffect->GetParameterRegister(passHandle, paramHandle, FXLPCONTEXT_VERTEXSHADERSAMPLER, &vRegisterIndex, &vRegisterCount);
                     }
                     if( paramContext & FXLPCONTEXT_PIXELSHADERSAMPLER )
                     {
                        pEffect->GetParameterRegister(passHandle, paramHandle, FXLPCONTEXT_PIXELSHADERSAMPLER, &pRegisterIndex, &pRegisterCount);
                     }

                     char scriptString[MAX_PATH];
                     uint32 parameterCRC;

                     {
                        FXLANNOTATION_DESC annotationDesc;
                        FXLHANDLE annotationHandle = FindAnnotationFromParameter(pEffect, paramHandle, paramDesc, "Texture", annotationDesc);
                        if( annotationHandle && annotationDesc.Type == FXLDTYPE_STRING && annotationDesc.Size < MAX_PATH )
                        {
                           pEffect->GetAnnotation(annotationHandle, scriptString);
                           parameterCRC = CShaderCRCs::GetParameterCRC(scriptString);
                           AddVertexPixelParams(scriptString, parameterCRC, paramContext, params, vRegisterIndex, vRegisterCount, pRegisterIndex, pRegisterCount);
                        }
                        else
                        {
                           parameterCRC = CShaderCRCs::GetParameterCRC(paramDesc.pName);
                           AddVertexPixelParams(paramDesc.pName, parameterCRC, paramContext, params, vRegisterIndex, vRegisterCount, pRegisterIndex, pRegisterCount);
                        }
                     }
                  }
                  break;

               default:
                  {
                     if( !(paramContext & (FXLPCONTEXT_VERTEXSHADERCONSTANTF|FXLPCONTEXT_PIXELSHADERCONSTANTF)) )
                     {
                        BPE_VERIFYA(false, "Invalid parameter context for parameter");
                     }

                     unsigned int vRegisterIndex = -1;
                     unsigned int pRegisterIndex = -1;
                     unsigned int vRegisterCount = 0;
                     unsigned int pRegisterCount = 0;
                     if( paramContext & FXLPCONTEXT_VERTEXSHADERCONSTANTF )
                     {
                        pEffect->GetParameterRegister(passHandle, paramHandle, FXLPCONTEXT_VERTEXSHADERCONSTANTF, &vRegisterIndex, &vRegisterCount);
                     }
                     if( paramContext & FXLPCONTEXT_PIXELSHADERCONSTANTF )
                     {
                        pEffect->GetParameterRegister(passHandle, paramHandle, FXLPCONTEXT_PIXELSHADERCONSTANTF, &pRegisterIndex, &pRegisterCount);
                     }

                     uint32 const parameterCRC = CShaderCRCs::GetParameterCRC(paramDesc.pName);
                     AddVertexPixelParams(paramDesc.pName, parameterCRC, paramContext, params, vRegisterIndex, vRegisterCount, pRegisterIndex, pRegisterCount);
                  }
                  break;
               }
            }
         }
         //
         std::sort(params.mParams.begin(), params.mParams.end());
         // now we have to decide what parts are duplicates and assign the correct index numbers to use
         RemoveIfDuplicateAndReAssignIndex( vertexShader, effect.mVertexShader, effectPass.mVertexShader_Index );
         RemoveIfDuplicateAndReAssignIndex( pixelShader, effect.mPixelShader, effectPass.mPixelShader_Index );
         RemoveIfDuplicateAndReAssignIndex( samplerStates, effect.mSamperStates, effectPass.mSamperStates_Index );
         RemoveIfDuplicateAndReAssignIndex( renderStates, effect.mRenderStates, effectPass.mRenderStates_Index );
         RemoveIfDuplicateAndReAssignIndex( params, effect.mParams, effectPass.mParams_Index );
      }
   }

   return true;
}