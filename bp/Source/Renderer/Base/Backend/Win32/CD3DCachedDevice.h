//------------------------------------------------------------------------------------------
// CD3DCachedDevice.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include <d3d9.h>

#include "Engine/Math/CVector4.h"
#include "Engine/Mechanics/TComPtr.h"

//----------------------------------------------------------------------------

struct SCounters
{
   SCounters()
   {
      memset( this, 0, sizeof(SCounters) );
   }
   
   int   mSetRenderState;
   int   mSetTextureStageState;
   int   mSetSamplerState;
   int   mSetPixelConstant;
   int   mSetVertexConstant;
   int   mSetIndex;
   int   mSetVertexStream;
   int   mDrawPrimitive;
   int   mDrawPrimitiveUP;
   int   mDrawIndexedPrimitive;
   int   mSetPixelShader;
   int   mSetVertexShader;
   int   mSetVertexDecl;
   int   mSetTexture;
   int   mSetRenderTarget;
   int   mSetDepthBuffer;
};

struct SStreamSourceInfo
{
   SStreamSourceInfo(IDirect3DVertexBuffer9* pStreamData, UINT offsetInBytes, UINT stride)
   :  mpStreamData(pStreamData)
   ,  mOffsetInBytes(offsetInBytes)
   ,  mStride(stride)
   {
   }
   
   bool const operator == (SStreamSourceInfo const & rhs)
   {
      return memcmp(this, &rhs, sizeof(SStreamSourceInfo)) == 0;
   }

   IDirect3DVertexBuffer9* mpStreamData;
   UINT                    mOffsetInBytes;
   UINT                    mStride;
};

//----------------------------------------------------------------------------

class CD3DCachedDevice
{
public:
   CD3DCachedDevice(TComPtr<IDirect3DDevice9> const & device);

   RENDERER_API void    InvalidateCachedState();     // Call this after using external library calls (such as CD3DFont)
   RENDERER_API HRESULT BeginScene();
   RENDERER_API HRESULT EndScene();

   RENDERER_API HRESULT SetRenderState(D3DRENDERSTATETYPE state, DWORD value);
   RENDERER_API HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value);
   RENDERER_API HRESULT SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

   RENDERER_API HRESULT SetPixelShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount);

   RENDERER_API HRESULT SetPixelShaderConstantI(UINT registerIndex,
                                                CONST INT *pConstantData,
                                                UINT registerCount);

   RENDERER_API HRESULT SetPixelShaderConstantB(UINT registerIndex,
                                                CONST BOOL *pConstantData,
                                                UINT registerCount);
   
   RENDERER_API HRESULT SetVertexShaderConstantF(UINT startRegister, float const * pConstantData, UINT vector4fCount);

   RENDERER_API HRESULT SetVertexShaderConstantI(UINT registerIndex,
                                                 CONST INT *pConstantData,
                                                 UINT registerCount);

   RENDERER_API HRESULT SetVertexShaderConstantB(UINT registerIndex,
                                                 CONST BOOL *pConstantData,
                                                 UINT registerCount);

   RENDERER_API HRESULT SetIndices(IDirect3DIndexBuffer9* pIndexData);
   RENDERER_API HRESULT SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9* pStreamData, UINT offsetInBytes, UINT stride);

   RENDERER_API HRESULT DrawPrimitive(D3DPRIMITIVETYPE primitiveType, UINT startVertex, UINT primitiveCount);
   RENDERER_API HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, void const* pVertexStreamZeroData, UINT vertexStreamZeroStride);
   RENDERER_API HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT primCount);

   RENDERER_API HRESULT SetPixelShader(IDirect3DPixelShader9* pShader);

   RENDERER_API HRESULT SetVertexShader(IDirect3DVertexShader9* pShader);
   RENDERER_API HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl);

   RENDERER_API HRESULT SetFVF(DWORD fvf);
   RENDERER_API HRESULT SetTexture(DWORD stage, IDirect3DBaseTexture9* pTexture);

   RENDERER_API HRESULT SetRenderTarget(DWORD renderTargetIndex, IDirect3DSurface9* pRenderTarget);

   RENDERER_API HRESULT SetViewport( D3DVIEWPORT9 const *pViewport );

   RENDERER_API HRESULT SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil);

   RENDERER_API HRESULT SetTransform(D3DTRANSFORMSTATETYPE type, CONST D3DMATRIX * matrix);

   RENDERER_API HRESULT SetMaterial(CONST D3DMATERIAL9 *pMaterial);
   RENDERER_API HRESULT SetLight(DWORD index, CONST D3DLIGHT9 *pLight);

   RENDERER_API HRESULT LightEnable(DWORD index, BOOL enable);
   RENDERER_API HRESULT SetNPatchMode(FLOAT numSegments);
   
public:
   SCounters   mCalls;
   SCounters   mUncachedCalls;
   
private:
   typedef std::vector< std::pair<bool, DWORD> > TRenderStateMap;

   typedef std::pair<DWORD, D3DTEXTURESTAGESTATETYPE> TTextureStageState; 
   typedef std::map<TTextureStageState, DWORD> TTextureStageStateMap;

   typedef std::pair<DWORD, D3DSAMPLERSTATETYPE> TSamplerState; 
   typedef std::map<TSamplerState, DWORD> TSamplerStateMap;

   TComPtr<IDirect3DDevice9>  mDevice;

   std::vector<IDirect3DBaseTexture9*> mBoundTextureState;
   TRenderStateMap                     mRenderState;
   TTextureStageStateMap               mTextureStageState;
   TSamplerStateMap                    mSamplerState;

   IDirect3DIndexBuffer9*              mpIndexBuffer;
   std::vector<SStreamSourceInfo>      mStreamSources;

   std::vector<CVector4>               mPixelShaderConstants;
   std::vector<CVector4>               mVertexShaderConstants;

   IDirect3DPixelShader9*              mpPixelShader;
   IDirect3DVertexShader9*             mpVertexShader;
   IDirect3DVertexDeclaration9*        mpVertexDecl;
};


