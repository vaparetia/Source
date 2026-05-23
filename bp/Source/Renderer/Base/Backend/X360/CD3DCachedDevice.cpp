//----------------------------------------------------------------------------
// CD3DCachedDevice.cpp
// Bluepoint
// Copyright 2011
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CD3DCachedDevice.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/X360/CRenderBackendPrivate.h"

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
   mpIndexBuffer = NULL;

   m_boundVertexShaders.clear();
   m_boundVertexShaders.reserve(100);
   mLastBoundMatch = -1;
      
   mpPixelShader = NULL;
   mpVertexShader = NULL;
   mpVertexDecl = NULL;

   XMemSet(mRenderState,-1,sizeof(mRenderState));
}

void CD3DCachedDevice::InvalidateCachedState2()
{
   memset(mRenderState,-1,sizeof(mRenderState));
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::BeginScene()
{   
   return D3DDeviceUncached()->BeginScene();
}

//----------------------------------------------------------------------------

HRESULT CD3DCachedDevice::EndScene()
{
   return D3DDeviceUncached()->EndScene();
}

//----------------------------------------------------------------------------
int gOkCount = 0;
void CD3DCachedDevice::GetVertexShaderBinding()
{
   return;
   ++gOkCount;

   if( mpVertexDecl == NULL )
   {
      return;
   }
   SVertexShaderBinding* pVSBinding = NULL;
   if( mLastBoundMatch != -1 )
   {
      pVSBinding = &m_boundVertexShaders[mLastBoundMatch];
      if ( pVSBinding->pVertexDecl == mpVertexDecl &&
         pVSBinding->pPixelShader == mpPixelShader &&
         pVSBinding->pVertexShader == mpVertexShader )
      {
          D3DDeviceUncached()->SetVertexShader(pVSBinding->pBoundVertexShader);
          return;
      }
   }
   for( int ii=0; ii < m_boundVertexShaders.size(); ++ii )
   {
      pVSBinding = &m_boundVertexShaders[ii];
      if ( pVSBinding->pVertexDecl == mpVertexDecl &&
         pVSBinding->pPixelShader == mpPixelShader &&
         pVSBinding->pVertexShader == mpVertexShader )
      {
         D3DDeviceUncached()->SetVertexShader(pVSBinding->pBoundVertexShader);
         mLastBoundMatch = ii;
         return;
      }
   }

   SVertexShaderBinding newVSB;
   newVSB.pVertexDecl = mpVertexDecl;
   newVSB.pPixelShader = mpPixelShader;
   newVSB.pVertexShader = mpVertexShader;

   UINT sizeOfShader;
   mpVertexShader->GetFunction(NULL, &sizeOfShader);
   char* pNewShader = new char[sizeOfShader];
   mpVertexShader->GetFunction(pNewShader, &sizeOfShader);
   IDirect3DVertexShader9* pNewVertexShader;
   D3DDeviceUncached()->CreateVertexShader((DWORD*)pNewShader, &pNewVertexShader);

   newVSB.pNewMemory = pNewShader;
   newVSB.pBoundVertexShader = pNewVertexShader;
   m_boundVertexShaders.push_back(newVSB);

   pNewVertexShader->Bind(0, mpVertexDecl, mStreamStrides, mpPixelShader);
   D3DDeviceUncached()->SetVertexShader(pNewVertexShader);
   mLastBoundMatch = m_boundVertexShaders.size()-1;
   return;
}

