//------------------------------------------------------------------------------------------
// CD3DCachedDevice.h
// Bluepoint
// Copyright 2011
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include <xtl.h>

#include "Engine/Math/CVector4.h"
#include "Engine/Mechanics/TComPtr.h"

//----------------------------------------------------------------------------

struct SVertexShaderBinding
{
   IDirect3DVertexDeclaration9* pVertexDecl;
   IDirect3DPixelShader9* pPixelShader;
   IDirect3DVertexShader9* pVertexShader;
   IDirect3DVertexShader9*	pBoundVertexShader;

   char* pNewMemory;
};

//----------------------------------------------------------------------------

class CD3DCachedDevice
{
public:
   CD3DCachedDevice(TComPtr<IDirect3DDevice9> const & device);

   void    InvalidateCachedState();     // Call this after using external library calls (such as CD3DFont)
   void    InvalidateCachedState2();     // Call this after using external library calls (such as CD3DFont)
   HRESULT BeginScene();
   HRESULT EndScene();

   HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
   HRESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

   HRESULT SetPixelShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount);

   HRESULT SetPixelShaderConstantI(UINT registerIndex, CONST INT *pConstantData, UINT registerCount);

   HRESULT SetPixelShaderConstantB(UINT registerIndex, CONST BOOL *pConstantData, UINT registerCount);
   
   HRESULT SetVertexShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount);

   HRESULT SetVertexShaderConstantI(UINT registerIndex, CONST INT *pConstantData, UINT registerCount);

   HRESULT SetVertexShaderConstantB(UINT registerIndex, CONST BOOL *pConstantData, UINT registerCount);

   HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData);
   HRESULT SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9* pStreamData, UINT offsetInBytes, UINT stride);

   HRESULT DrawPrimitive(D3DPRIMITIVETYPE primitiveType, UINT startVertex, UINT primitiveCount);
   HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, void const* pVertexStreamZeroData, UINT vertexStreamZeroStride);
   HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT primCount);

   HRESULT SetPixelShader(IDirect3DPixelShader9* pShader);

   HRESULT SetVertexShader(IDirect3DVertexShader9* pShader);
   HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);

   HRESULT SetFVF(DWORD fvf);
   HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture9* pTexture);

   HRESULT SetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9* pRenderTarget);

   HRESULT SetViewport( D3DVIEWPORT9 const *pViewport );

   HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);
private:
   void GetVertexShaderBinding();
   
private:
   TComPtr<IDirect3DDevice9>  mDevice;

   IDirect3DBaseTexture9*              mBoundTextureState[14];
   DWORD                               mRenderState[209];
   DWORD                               mSamplerState[14][13];//Sampler unit, state

   IDirect3DIndexBuffer9*              mpIndexBuffer;
   DWORD                               mStreamStrides[12];

   IDirect3DPixelShader9*              mpPixelShader;
   IDirect3DVertexShader9*             mpVertexShader;
   IDirect3DVertexDeclaration9*        mpVertexDecl;

   std::vector<SVertexShaderBinding> m_boundVertexShaders;
   int mLastBoundMatch;
};

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
   if( mRenderState[state] == value )
   {
      return S_OK;
   }
   mRenderState[state] = value;
   return mDevice->SetRenderState(state, value);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
   if( mSamplerState[sampler][type] == value )
   {
      return S_OK;
   }
   mSamplerState[sampler][type] = value;
   return mDevice->SetSamplerState(sampler, type, value);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetTexture(DWORD stage, IDirect3DBaseTexture9* pTexture)
{
   if( mBoundTextureState[stage] == pTexture )
   {
      return S_OK;
   }
   mBoundTextureState[stage] = pTexture;
   return mDevice->SetTexture(stage, pTexture);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::DrawPrimitive(D3DPRIMITIVETYPE primitiveType, UINT startVertex, UINT primitiveCount)
{
   //GetVertexShaderBinding();
   return mDevice->DrawPrimitive(primitiveType, startVertex, primitiveCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::DrawPrimitiveUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, void const* pVertexStreamZeroData, UINT vertexStreamZeroStride)
{
   //GetVertexShaderBinding();
   return mDevice->DrawPrimitiveUP(primitiveType, primitiveCount, pVertexStreamZeroData, vertexStreamZeroStride);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT primCount)
{
   //GetVertexShaderBinding();
   return mDevice->DrawIndexedPrimitive(type, baseVertexIndex, minVertexIndex, numVertices, startIndex, primCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetPixelShader(IDirect3DPixelShader9* pShader)
{
   mpPixelShader = pShader;
   return mDevice->SetPixelShader(pShader);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetVertexShader(IDirect3DVertexShader9* pShader)
{
   mpVertexShader = pShader;
   return mDevice->SetVertexShader(pShader);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
   mpVertexDecl = pDecl;
   return mDevice->SetVertexDeclaration(pDecl);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetPixelShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount)
{
   return mDevice->SetPixelShaderConstantF(startRegister, pConstantData, vector4fCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetPixelShaderConstantI( UINT registerIndex, CONST INT *pConstantData, UINT registerCount )
{
   return mDevice->SetPixelShaderConstantI(registerIndex, pConstantData, registerCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetPixelShaderConstantB( UINT registerIndex, CONST BOOL *pConstantData, UINT registerCount )
{
   return mDevice->SetPixelShaderConstantB(registerIndex, pConstantData, registerCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetVertexShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount)
{
   return mDevice->SetVertexShaderConstantF(startRegister, pConstantData, vector4fCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetVertexShaderConstantI( UINT registerIndex, CONST INT *pConstantData, UINT registerCount)
{
   return mDevice->SetVertexShaderConstantI(registerIndex, pConstantData, registerCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetVertexShaderConstantB( UINT registerIndex, CONST BOOL *pConstantData, UINT registerCount )
{
   return mDevice->SetVertexShaderConstantB(registerIndex, pConstantData, registerCount);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
   return mDevice->SetIndices(pIndexData);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9* pStreamData, UINT offsetInBytes, UINT stride)
{
   mStreamStrides[streamNumber] = stride;
   return mDevice->SetStreamSource(streamNumber, pStreamData, offsetInBytes, stride);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetFVF(DWORD fvf)
{
   mpVertexDecl = NULL;
   return mDevice->SetFVF(fvf);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
   return mDevice->SetRenderTarget(renderTargetIndex, pRenderTarget);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetViewport( D3DVIEWPORT9 const *pViewport )
{
   return mDevice->SetViewport(pViewport);
}

//----------------------------------------------------------------------------

inline HRESULT CD3DCachedDevice::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
   return mDevice->SetDepthStencilSurface(pNewZStencil);
}




