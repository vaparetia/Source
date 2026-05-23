//----------------------------------------------------------------------------
// CD3DCachedDevice.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CD3DCachedDevice.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"

//----------------------------------------------------------------------------

//#define STATE_CACHING_ENABLED

//----------------------------------------------------------------------------

CD3DCachedDevice::CD3DCachedDevice(TComPtr<IDirect3DDevice9> const & device)
:  mDevice(device)
{
   InvalidateCachedState();
}
//----------------------------------------------------------------------------

void CD3DCachedDevice::InvalidateCachedState()
{
   mBoundTextureState.resize(16, NULL);
   mRenderState.clear();
   mRenderState.resize(512, std::make_pair(false, (DWORD)0));
   mTextureStageState.clear();
   mSamplerState.clear();

   mpIndexBuffer = NULL;
   mStreamSources.clear();
   mStreamSources.resize(32, SStreamSourceInfo(NULL, -1, -1));
   
   mPixelShaderConstants.clear();
   mPixelShaderConstants.resize(1024, CVector4::Zero());
   mVertexShaderConstants.clear();
   mVertexShaderConstants.resize(1024, CVector4::Zero());
   
   mpPixelShader = NULL;
   mpVertexShader = NULL;
   mpVertexDecl = NULL;
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::BeginScene()
{
   mCalls = SCounters();
   mUncachedCalls = SCounters();
   
   return D3DDeviceUncached()->BeginScene();
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::EndScene()
{
   return D3DDeviceUncached()->EndScene();
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
   mCalls.mSetRenderState++;

#ifdef STATE_CACHING_ENABLED

   std::pair<bool, DWORD> & cachedState = mRenderState[state];
   
   if( cachedState.first )
   {
      if( cachedState.second == value )
      {
         return S_OK;
      }
   }

   cachedState.first = true;
   cachedState.second = value;
#endif

   mUncachedCalls.mSetRenderState++;
   
   return D3DDeviceUncached()->SetRenderState(state, value);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
   mCalls.mSetTextureStageState++;

#ifdef STATE_CACHING_ENABLED
   TTextureStageState const key = std::make_pair(stage, type);
   TTextureStageStateMap::iterator found = mTextureStageState.find(key);

   if( found != mTextureStageState.end() )
   {
      if( found->second == value )
      {
         return S_OK;
      }
      else
      {
         found->second = value;
      }
   }
   else
   {
      mTextureStageState.insert(std::make_pair(key, value));
   }
#endif

   mUncachedCalls.mSetTextureStageState++;

   return D3DDeviceUncached()->SetTextureStageState(stage, type, value);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
   mCalls.mSetSamplerState++;

#ifdef STATE_CACHING_ENABLED
   TSamplerState const key = std::make_pair(sampler, type);
   TSamplerStateMap::iterator found = mSamplerState.find(key);

   if( found != mSamplerState.end() )
   {
      if( found->second == value )
      {
         return S_OK;
      }
      else
      {
         found->second = value;
      }
   }
   else
   {
      mSamplerState.insert(std::make_pair(key, value));
   }
#endif

   mUncachedCalls.mSetSamplerState++;
   return D3DDeviceUncached()->SetSamplerState(sampler, type, value);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetPixelShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount)
{
   mCalls.mSetPixelConstant++;
   BPE_VERIFY(startRegister >= 0 && (startRegister + vector4fCount) < mPixelShaderConstants.size(), false, "out of bounds");

#ifdef STATE_CACHING_ENABLED
   if( memcmp( &mPixelShaderConstants[startRegister], pConstantData, sizeof(float) * 4 * vector4fCount ) == 0 )
      return S_OK;

   memcpy(&mPixelShaderConstants[startRegister], pConstantData, sizeof(float) * 4 * vector4fCount);
#endif
   
   mUncachedCalls.mSetPixelConstant++;
   return D3DDeviceUncached()->SetPixelShaderConstantF(startRegister, pConstantData, vector4fCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetPixelShaderConstantI( UINT registerIndex,
                                                   CONST INT *pConstantData,
                                                   UINT registerCount )
{
   return D3DDeviceUncached()->SetPixelShaderConstantI(registerIndex,
                                                       pConstantData,
                                                       registerCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetPixelShaderConstantB( UINT registerIndex,
                                                   CONST BOOL *pConstantData,
                                                   UINT registerCount )
{
   return D3DDeviceUncached()->SetPixelShaderConstantB(registerIndex,
                                                       pConstantData,
                                                       registerCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetVertexShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount)
{
   mCalls.mSetVertexConstant++;
   BPE_VERIFY(startRegister >= 0 && (startRegister + vector4fCount) < mVertexShaderConstants.size(), false, "out of bounds");

#ifdef STATE_CACHING_ENABLED
   if( memcmp( &mVertexShaderConstants[startRegister], pConstantData, sizeof(float) * 4 * vector4fCount ) == 0 )
      return S_OK;

   memcpy(&mVertexShaderConstants[startRegister], pConstantData, sizeof(float) * 4 * vector4fCount);
#endif
   
   mUncachedCalls.mSetVertexConstant++;
   return D3DDeviceUncached()->SetVertexShaderConstantF(startRegister, pConstantData, vector4fCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetVertexShaderConstantI( UINT registerIndex,
                                                    CONST INT *pConstantData,
                                                    UINT registerCount)
{
   return D3DDeviceUncached()->SetVertexShaderConstantI(registerIndex,
                                                        pConstantData,
                                                        registerCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetVertexShaderConstantB( UINT registerIndex,
                                                    CONST BOOL *pConstantData,
                                                    UINT registerCount )
{
   return D3DDeviceUncached()->SetVertexShaderConstantB(registerIndex,
                                                        pConstantData,
                                                        registerCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
   mCalls.mSetIndex++;

#ifdef STATE_CACHING_ENABLED
   if( mpIndexBuffer == pIndexData )
   {
      return S_OK;
   }

   mpIndexBuffer = pIndexData;
#endif

   mUncachedCalls.mSetIndex++;
   return D3DDeviceUncached()->SetIndices(pIndexData);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9* pStreamData, UINT offsetInBytes, UINT stride)
{
   mCalls.mSetVertexStream++;

   SStreamSourceInfo stream(pStreamData, offsetInBytes, stride);

#ifdef STATE_CACHING_ENABLED
   if( mStreamSources[streamNumber] == stream )
   {
      return S_OK;
   }

   mStreamSources[streamNumber] = stream;
#endif

   mUncachedCalls.mSetVertexStream++;

   return D3DDeviceUncached()->SetStreamSource(streamNumber, pStreamData, offsetInBytes, stride);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::DrawPrimitive(D3DPRIMITIVETYPE primitiveType, UINT startVertex, UINT primitiveCount)
{
   mCalls.mDrawPrimitive++;
   return D3DDeviceUncached()->DrawPrimitive(primitiveType, startVertex, primitiveCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, void const* pVertexStreamZeroData, UINT vertexStreamZeroStride)
{
   mCalls.mDrawPrimitiveUP++;
   return D3DDeviceUncached()->DrawPrimitiveUP(primitiveType, primitiveCount, pVertexStreamZeroData, vertexStreamZeroStride);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT primCount)
{
   mCalls.mDrawIndexedPrimitive++;
   return D3DDeviceUncached()->DrawIndexedPrimitive(type, baseVertexIndex, minVertexIndex, numVertices, startIndex, primCount);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetPixelShader(IDirect3DPixelShader9* pShader)
{
   mCalls.mSetPixelShader++;

#ifdef STATE_CACHING_ENABLED
   if( mpPixelShader == pShader )
      return S_OK;

   mpPixelShader = pShader;
#endif

   mUncachedCalls.mSetPixelShader++;

   return D3DDeviceUncached()->SetPixelShader(pShader);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetVertexShader(IDirect3DVertexShader9* pShader)
{
   mCalls.mSetVertexShader++;

#ifdef STATE_CACHING_ENABLED
   if( mpVertexShader == pShader )
      return S_OK;

   mpVertexShader = pShader;
#endif

   mUncachedCalls.mSetVertexShader++;
   
   return D3DDeviceUncached()->SetVertexShader(pShader);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
   mCalls.mSetVertexDecl++;

#ifdef STATE_CACHING_ENABLED
   if( mpVertexDecl == pDecl )
      return S_OK;

   mpVertexDecl = pDecl;
#endif

   mUncachedCalls.mSetVertexDecl++;
   
   return D3DDeviceUncached()->SetVertexDeclaration(pDecl);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetFVF(DWORD fvf)
{
   return D3DDeviceUncached()->SetFVF(fvf);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetTexture(DWORD stage, IDirect3DBaseTexture9* pTexture)
{
   BPE_VERIFY( stage < mBoundTextureState.size(), false, "invalid index or array to small" );

   mCalls.mSetTexture++;

#ifdef STATE_CACHING_ENABLED
   if( pTexture == mBoundTextureState[stage] )
   {
      return S_OK;
   }

   mBoundTextureState[stage] = pTexture;
#endif

   mUncachedCalls.mSetTexture++;
   return D3DDeviceUncached()->SetTexture(stage, pTexture);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
   mCalls.mSetRenderTarget++;
   mUncachedCalls.mSetRenderTarget++;
   return D3DDeviceUncached()->SetRenderTarget(renderTargetIndex, pRenderTarget);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetViewport( D3DVIEWPORT9 const *pViewport )
{
   return D3DDeviceUncached()->SetViewport( pViewport );
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
   mCalls.mSetDepthBuffer++;
   mUncachedCalls.mSetDepthBuffer++;
   return D3DDeviceUncached()->SetDepthStencilSurface(pNewZStencil);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetTransform(D3DTRANSFORMSTATETYPE type, CONST D3DMATRIX * matrix)
{
   return D3DDeviceUncached()->SetTransform(type, matrix);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetMaterial(CONST D3DMATERIAL9 *pMaterial)
{
   return D3DDeviceUncached()->SetMaterial(pMaterial);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetLight(DWORD index, CONST D3DLIGHT9 *pLight)
{
   return D3DDeviceUncached()->SetLight(index, pLight);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::LightEnable(DWORD index, BOOL enable)
{
   return D3DDeviceUncached()->LightEnable(index, enable);
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::SetNPatchMode(FLOAT numSegments)
{
   return D3DDeviceUncached()->SetNPatchMode(numSegments);
}

