//----------------------------------------------------------------------------
// CEffectStateManager.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/Win32/CRenderBackendPrivate.h"

//----------------------------------------------------------------------------

#include "d3dx9shader.h"

//----------------------------------------------------------------------------

class CEffectStateManager : public ID3DXEffectStateManager
{
public:
   CEffectStateManager()
   : mRefCount(1)
   {
   }

   // IUnknown implementation
   STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv)
   {
      if( iid == IID_IUnknown || iid == IID_ID3DXEffectStateManager )
      {
         *ppv = static_cast<ID3DXEffectStateManager*>(this);
      }
      else
      {
         *ppv = NULL;
         return E_NOINTERFACE;
      }

      reinterpret_cast<IUnknown*>(this)->AddRef();
      return S_OK;
   }

   STDMETHOD_(ULONG, AddRef)(THIS)
   {
      return (ULONG)InterlockedIncrement( (LONG*)&mRefCount );
   }

   STDMETHOD_(ULONG, Release)(THIS)
   {
      if( 0L == InterlockedDecrement( (LONG*)&mRefCount ) )
      {
         delete this;
         return 0L;
      }

      return mRefCount;
   }

   // ID3DXEffectStateManager implementaton

   STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE d3dRenderState, DWORD dwValue )
   {
      return D3DDevice()->SetRenderState(d3dRenderState, dwValue);
   }

   STDMETHOD(SetSamplerState)(THIS_ DWORD dwStage, D3DSAMPLERSTATETYPE d3dSamplerState, DWORD dwValue )
   {
      return D3DDevice()->SetSamplerState(dwStage, d3dSamplerState, dwValue);
   }

   STDMETHOD(SetTextureStageState)(THIS_ DWORD dwStage, D3DTEXTURESTAGESTATETYPE d3dTextureStageState, DWORD dwValue )
   {
      return D3DDevice()->SetTextureStageState(dwStage, d3dTextureStageState, dwValue);
   }

   STDMETHOD(SetTexture)(THIS_ DWORD dwStage, LPDIRECT3DBASETEXTURE9 pTexture )
   {
      return D3DDevice()->SetTexture(dwStage, pTexture);
   }

   STDMETHOD(SetVertexShader)(THIS_ LPDIRECT3DVERTEXSHADER9 pShader )
   {
      return D3DDevice()->SetVertexShader(pShader);
   }
   STDMETHOD(SetPixelShader)(THIS_ LPDIRECT3DPIXELSHADER9 pShader )
   {
      return D3DDevice()->SetPixelShader(pShader);
   }

   STDMETHOD(SetFVF)(THIS_ DWORD dwFVF )
   {
      return D3DDevice()->SetFVF(dwFVF);
   }

   STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE state, CONST D3DMATRIX *pMatrix )
   {
      return D3DDevice()->SetTransform(state, pMatrix);
   }

   STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL9 *pMaterial )
   {
      return D3DDevice()->SetMaterial(pMaterial);
   }

   STDMETHOD(SetLight)(THIS_ DWORD index, CONST D3DLIGHT9 *pLight )
   {
      return D3DDevice()->SetLight(index, pLight );
   }

   STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable )
   {
      return D3DDevice()->LightEnable(Index, Enable);
   }

   STDMETHOD(SetNPatchMode)(THIS_ FLOAT NumSegments )
   {
      return D3DDevice()->SetNPatchMode(NumSegments);
   }

   STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT RegisterIndex,
                                       CONST FLOAT *pConstantData,
                                       UINT RegisterCount )
   {
      //MARCO: Disabled setting of vertex shader constants, we want to explicitly set them ourselves.
#if 0 
      return D3DDevice()->SetVertexShaderConstantF(RegisterIndex,
                                                   pConstantData,
                                                   RegisterCount);
#endif
      return S_OK;
   }

   STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT RegisterIndex,
                                       CONST INT *pConstantData,
                                       UINT RegisterCount)
   {
      //MARCO: Disabled setting of vertex shader constants, we want to explicitly set them ourselves.
#if 0 
      return D3DDevice()->SetVertexShaderConstantI(RegisterIndex,
                                                   pConstantData,
                                                   RegisterCount);
#endif
      return S_OK;
   }

   STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT RegisterIndex,
                                       CONST BOOL *pConstantData,
                                       UINT RegisterCount )
   {
      //MARCO: Disabled setting of vertex shader constants, we want to explicitly set them ourselves.
#if 0 
      return D3DDevice()->SetVertexShaderConstantB(RegisterIndex,
                                                   pConstantData,
                                                   RegisterCount);
#endif
      return S_OK;
   }

   STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT RegisterIndex,
                                      CONST FLOAT *pConstantData,
                                      UINT RegisterCount )
   {
      return D3DDevice()->SetPixelShaderConstantF(RegisterIndex,
                                                  pConstantData,
                                                  RegisterCount);
   }

   STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT RegisterIndex,
                                      CONST INT *pConstantData,
                                      UINT RegisterCount )
   {
      return D3DDevice()->SetPixelShaderConstantI(RegisterIndex,
                                                  pConstantData,
                                                  RegisterCount);
   }

   STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT RegisterIndex,
                                      CONST BOOL *pConstantData,
                                      UINT RegisterCount )
   {
      return D3DDevice()->SetPixelShaderConstantB(RegisterIndex,
                                                  pConstantData,
                                                  RegisterCount);
   }

private:
   int   mRefCount;
};

//----------------------------------------------------------------------------
