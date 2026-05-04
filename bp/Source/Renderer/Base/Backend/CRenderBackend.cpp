//----------------------------------------------------------------------------
// CRenderBackend.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRenderBackend.h"

//----------------------------------------------------------------------------

#include "Engine/Resource/CResourceCache.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/System/CTaskQueue.h"
#include "Engine/System/COsContext.h"
#include "Engine/Streams/CMemoryInputStream.h"

#include "Renderer/Base/Material/CCompiledShaderCache.h"
#include "Renderer/Base/Frontend/RenderObject/CDrawableRenderEntity.h"
#include "Renderer/Base/Frontend/RenderObject/CFakeRenderEntity.h"
#include "Renderer/Base/CMTXFont.h"
#include "Renderer/Base/ShaderObjects/CGameObjectComponentPropertiesShader.h"
#include "Renderer/Base/ShaderObjects/CShaderObjectFactory.h"
#include "Renderer/Base/Primitive/ProgShader/PSCMesh.h"

#include "Renderer/Base/Primitive/CVertexBuffer.h"
#include "Renderer/Base/Primitive/CIndexBuffer.h"

#include <boost/weak_ptr.hpp>

uint8 const *   CBaseRenderBackend::mFontCFONBuffer = NULL;
uint32          CBaseRenderBackend::mFontCFONBufferSize = -1;
uint8 const *   CBaseRenderBackend::mFontTXTRBuffer = NULL;
uint32          CBaseRenderBackend::mFontTXTRBufferSize = -1;

//----------------------------------------------------------------------------

CBaseRenderBackend::SRenderInitialization::SRenderInitialization()
:  mpFrameBufferSize(NULL)
,  mFrameBufferSizeCount(0)
,  mCreationFlags(kCF_WaitForVSync)
,  mWindowHandle(NULL)

,  mIndexBufferPoolSize(512 * 1024)
,  mIndexBufferPoolChunkCount(8192)
,  mVertexBufferPoolSize(4 * 1024 * 1024)
,  mVertexBufferPoolChunkCount(8192)

{
}

//----------------------------------------------------------------------------

SScreenOutputControl::SScreenOutputControl()
{
   ResetToDefaultState();
}

//----------------------------------------------------------------------------

void SScreenOutputControl::ResetToDefaultState()
{
   SetOutputMode(kOM_Unprocessed);
   mPostProcessShader.reset();
   mOutputModulateColor = CColorf::White();
}

//----------------------------------------------------------------------------

void SScreenOutputControl::SetOutputMode(EScreenOutputMode const mode)
{
   mOutputMode = mode;

   switch(mOutputMode)
   {
   case kOM_Unprocessed:
      mShaderColorSpace = kSCS_Gamma;
      mFrameBufferFormat = kSFF_Gamma;
      break;

   case kOM_BlastFactor:
   case kOM_BlastFactor_NoToneMapping:
      mShaderColorSpace = kSCS_Linear;
      mFrameBufferFormat = kSFF_BlastfactorHdr;
      break;

   case kOM_JKE:
      mShaderColorSpace = kSCS_Gamma;
      mFrameBufferFormat = kSFF_JKE;
      break;
   }
}

//----------------------------------------------------------------------------

void SScreenOutputControl::SetPostProcessShader(boost::shared_ptr<CShader> const & shader)
{
   mPostProcessShader = shader;
}

//----------------------------------------------------------------------------

boost::shared_ptr<CShader> const & SScreenOutputControl::GetPostProcessShader() const
{
   return mPostProcessShader;
}

//----------------------------------------------------------------------------

CRenderBackend * gpRenderBackend = NULL;

//----------------------------------------------------------------------------

CBaseRenderBackend::CBaseRenderBackend(IResourcePool & resourcePool,
                                       SRenderInitialization const & initFlags)
:  mpResourcePool(&resourcePool)
,  mInitFlags(initFlags)

,  mUniverseTime(0.0)
,  mFrameCount(0)

,  mRenderTargetWidth(-1)
,  mRenderTargetHeight(-1)
,  mInterlaced(false)
,  mRefreshRate(60.0f)

,  mProjectionMatrix(CMatrix4::Identity())

,  mViewportJitterX(0.0f)
,  mViewportJitterY(0.0f)

,  mpShaderObjectFactory( new CShaderObjectFactory() )

,  mWhiteMap(NULL)
,  mGreyMap(NULL)
,  mBlackMap(NULL)
,  mFlatNormalMap(NULL)

,  mMetrics()

,  mFakeRenderEntity(new CFakeRenderEntity())
#if BPE_TARGET==BPE_TARGET_VITA
,  mFrameBufferSizeMono(SRenderFrameBufferSize::kDA_Widescreen, 960, 544, 960, 544, 960, 544, 16.0f / 9.0f)
,  mFrameBufferSizeStereo(SRenderFrameBufferSize::kDA_Widescreen, 960, 544, 960, 544, 960, 544, 16.0f / 9.0f)
#else
,  mFrameBufferSizeMono(SRenderFrameBufferSize::kDA_Widescreen, 1280, 720, 1280, 720, 1280, 720, 16.0f / 9.0f)
,  mFrameBufferSizeStereo(SRenderFrameBufferSize::kDA_Widescreen, 1280, 720, 1280, 720, 1280, 720, 16.0f / 9.0f)

#endif
,  mRenderTargetFrameBufferSizeType(SRenderFrameBufferSize::kST_AltFrameBuffer)

,  mIsStereo3D(false)
{
   // Can't keep these values around cached, they might be ptrs to the stack.
   mInitFlags.mFrameBufferSizeCount = 0;
   mInitFlags.mpFrameBufferSize = NULL;

   BPE_VERIFY(gpRenderBackend == NULL, false, "only one instance of render backend supported");
   gpRenderBackend = (CRenderBackend*)this;

   IResourceCache * pIResourceCache = mpResourcePool->ResourceCache();
   
   if (pIResourceCache && (pIResourceCache->GetType() == CResourceCache::skType))
   {
      CResourceCache *pResourceCache = static_cast<CResourceCache*>(pIResourceCache);
      pResourceCache->AddResourcesChangedDelegate(fastdelegate::MakeDelegate(this, &CBaseRenderBackend::OnResourcesChanged));
   }
   mCurrentAreaDebugName[0] = '\0';
}

//----------------------------------------------------------------------------

CBaseRenderBackend::~CBaseRenderBackend()
{
   if( gpResources )
   {
      IResourceCache * pIResourceCache = gpResources->ResourceCache();

      if (pIResourceCache && (pIResourceCache->GetType() == CResourceCache::skType))
      {
         CResourceCache *pResourceCache = static_cast<CResourceCache*>(pIResourceCache);
         pResourceCache->RemoveResourcesChangedDelegate(fastdelegate::MakeDelegate(this, &CBaseRenderBackend::OnResourcesChanged));
      }
   }
   delete mWhiteMap;
   delete mGreyMap;
   delete mBlackMap;
   delete mFlatNormalMap;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::SetDefaultFontFromMemory(uint8 const * CFONBuffer, uint32 const CFONBufferSize, uint8 const * TXTRBuffer, uint32 const TXTRBufferSize)
{
   mFontCFONBuffer = CFONBuffer;
   mFontCFONBufferSize = CFONBufferSize;
   mFontTXTRBuffer = TXTRBuffer;
   mFontTXTRBufferSize = TXTRBufferSize;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::InitDefaultObjects()
{
   mShaderCache.reset(new CCompiledShaderCache());

   // Default textures
   mWhiteMap = CBaseTexture::Create(CBaseTexture::kGTT_White);
   mGreyMap = CBaseTexture::Create(CBaseTexture::kGTT_Grey);
   mBlackMap = CBaseTexture::Create(CBaseTexture::kGTT_Black);
   mFlatNormalMap = CBaseTexture::Create(CBaseTexture::kGTT_FlatNormal);

   CResId const fontResId("$/enginesupport/fonts/console.cfon");

   if (mFontCFONBufferSize == -1)
   {
      //Load the hard-coded font resource off of the disk
      TLockedResource<CMTXFont> fontResource(CResource(gpResources->GetResource(fontResId)));
      {
         fontResource.GetPtr();
         mFonts.push_back(fontResource);
      }
   }
   else
   {
      TResource<CBaseTexture> texture(CBaseTexture::Create((void*)mFontTXTRBuffer, mFontTXTRBufferSize));

      CMemoryInputStream memStream((void *)mFontCFONBuffer, mFontCFONBufferSize, CMemoryInputStream::kOwner_App);
   {
         uint32 const magicNumber = memStream.ReadUint32();
         BPE_VERIFY(magicNumber == 'FONT', false, "Invalid header for FONT resource.");
         uint32 const version = memStream.ReadUint32();
         BPE_VERIFY(version == 5, false, "Invalid version number for FONT resource, recook fonts with latest tools.");
      }
      CMTXFont * nFont = new CMTXFont(memStream, texture);

      CResource cRes(new CResourceReference(nFont, gpResources, fontResId));
      TLockedResource<CMTXFont> fontResource(cRes);

      gpResources->ManuallyAddToResourceMap(fontResId, cRes);
      if (gpResources->ResourceCache() != NULL)
         gpResources->ResourceCache()->CacheResource(cRes, fontResId);
         mFonts.push_back(fontResource);
      }
}

//----------------------------------------------------------------------------

IGameObjectFactory & CBaseRenderBackend::ShaderObjectFactory()
{
   return *mpShaderObjectFactory;
}

//----------------------------------------------------------------------------

real32 const CBaseRenderBackend::GetScreenAspectRatio() const 
{
   return GetFrameBufferSize().GetAspect();
}

//----------------------------------------------------------------------------

real32 const CBaseRenderBackend::GetScreenRefreshRate() const 
{
   return mRefreshRate;
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::Is43Aspect() const
{
   return (GetScreenAspectRatio() < 1.77f);
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::IsLowResolution() const
{
   return (GetBackBufferWidth() < 960) && (GetBackBufferHeight() < 720);
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::IsInterlaced() const
{
   return mInterlaced;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::AddMaterialPropertiesMapping(boost::shared_ptr<CShader> const & material)
{
   if( material->GetProperties() )
   {
      mComponentEditorIdToComponentMap.insert(std::make_pair(material->GetProperties()->mEditorId, material));
   }
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::RemoveMaterialPropertiesMapping(CShader const & material)
{
   // Remove material references.
   // We're going to have to look at this again when we consider duplicated materials/properties.
   if( material.GetProperties() )
   {
      TComponentEditorIdToComponentMapIterPair componentsIter = mComponentEditorIdToComponentMap.equal_range(material.GetProperties()->mEditorId);
      if (componentsIter.first == componentsIter.second)
      {
         return;
      }
      for (TComponentEditorIdToComponentMap::iterator it = componentsIter.first; it != componentsIter.second;)
      {
         boost::weak_ptr<CShader const> const & mapMaterial = (*it).second;
         if (mapMaterial.use_count() > 0)
         {
            // If it's still in use, don't erase.
            ++it;
         }
         else
         {
            it = mComponentEditorIdToComponentMap.erase(it);
         }
      }
   }
}

//----------------------------------------------------------------------------

bool CBaseRenderBackend::ModifyComponentProperties(CGuid const &componentEditorId, void* pPropertiesData, uint32 const propertiesDataSize, IEvaluatorFactory const &factory)
{
   TComponentEditorIdToComponentMapConstIterPair const componentsIter = mComponentEditorIdToComponentMap.equal_range(componentEditorId);
   
   for( TComponentEditorIdToComponentMap::const_iterator it = componentsIter.first; it != componentsIter.second; ++it )
   {
      CMemoryInputStream propertiesStream(pPropertiesData, propertiesDataSize, CMemoryInputStream::kOwner_App);

      // Update properties
      boost::shared_ptr<CShader> material(it->second);
      // const cast! (special case, we're modifying const resource data shared between components)
      CGameObjectComponentProperties &componentProperties = *const_cast<CGameObjectComponentProperties*>(static_cast<CGameObjectComponentProperties const * const>(material->GetProperties()));
      CGameObjectApplyPropertiesData applyProperties(propertiesStream, factory, CGameObjectApplyPropertiesData::kUpdateOnly);
      componentProperties.ApplyProperties(applyProperties, componentProperties.mEvaluatorAllocator);
      componentProperties.PostLoadUpdate();  // Reload resources if required

      material->BuildConstantParameterBuffer();
   }

   return true;
}


//----------------------------------------------------------------------------
   
CBaseTexture const & CBaseRenderBackend::GetWhiteMap() const
{
   return *mWhiteMap;
}

//----------------------------------------------------------------------------

CBaseTexture const & CBaseRenderBackend::GetGreyMap() const
{
   return *mGreyMap;
}

//----------------------------------------------------------------------------
   
CBaseTexture const & CBaseRenderBackend::GetBlackMap() const
{
   return *mBlackMap;
}

//----------------------------------------------------------------------------

CBaseTexture const & CBaseRenderBackend::GetFlatNormalMap() const
{
   return *mFlatNormalMap;
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseRenderBackend::CreateRenderTarget(int width, int height, CBaseTexture::EAntiAliasType const aaFlags, CBaseTexture::EFormat format, ERenderMemory const memory)
{
   return CBaseTexture::Create(width, height, 1, format, CBaseTexture::kUsage_RenderTarget, aaFlags, memory );
}

//----------------------------------------------------------------------------

CBaseTexture * CBaseRenderBackend::CreateDepthBuffer(int width, int height, CBaseTexture::EAntiAliasType const aaFlags, ERenderMemory const memory)
{
   return CBaseTexture::Create(width, height, 1, CBaseTexture::kFormat_D24X8, CBaseTexture::kUsage_DepthBuffer, aaFlags, memory);
}

//----------------------------------------------------------------------------
   
void CBaseRenderBackend::RenderText(const char * const text, 
                                    real32 const x, 
                                    real32 const y,
                                    ERenderFonts const font,
                                    CColor const & color,
                                    int const flags)
{
   mFonts[font]->RenderText(x, y, color, text, flags);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::RenderFullscreenQuad(real32 const z)
{
#if 0
   static float xscale = 0.f;
   static float yscale = 0.f;

   xscale += 0.05f;
   yscale += 0.05f;

   if ( xscale > 1.f )
   {
      xscale = -1.f;
   }

   if ( yscale > 1.f )
   {
      yscale = -1.f;
   }
#else
   static float const xscale = 1.0f;
   static float const yscale = 1.0f;
#endif
   RenderQuad(-1.0f, xscale, 
              -1.0f, yscale, 
              z,
              0.0f, 1.0f,
              0.0f, 1.0f);
}

//----------------------------------------------------------------------------

real32 const CBaseRenderBackend::GetFontHeight(ERenderFonts const font) const
{
   return static_cast<real32>( mFonts[font]->GetFontHeight() );
}

//----------------------------------------------------------------------------

TLockedResource<CMTXFont> CBaseRenderBackend::GetFont(ERenderFonts const font) const
{
   return mFonts[font];
}

//----------------------------------------------------------------------------

CVector2 const CBaseRenderBackend::GetTextExtents( std::string const & text, ERenderFonts const font ) const
{
   CVector2 size(CVector2::kConstructUninitialized);
   CMTXFont const * pFont = &(*(mFonts[font]));
   SMTXFontControl fontControl;
   CMTXFont::GetTextExtent(text.c_str(), size, &pFont, fontControl);
   return size;
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::InternalBeginScene()
{
   mMetrics = SRenderBackendMetrics();
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::InternalPresent()
{
   // Reset output control to default state to prevent state from 
   // output inactive components being persistent.
   // We can't do this inside BeginScene because that function is called after the "render" of the components is invoked.
   ScreenOutputControl().ResetToDefaultState();
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::OnResourcesChanged()
{
   for( TComponentEditorIdToComponentMap::const_iterator it = mComponentEditorIdToComponentMap.begin(); it != mComponentEditorIdToComponentMap.end(); ++it )
   {
      boost::weak_ptr<CShader> const & mapMaterial = (*it).second;
      if (mapMaterial.use_count() > 0)
      {
         boost::shared_ptr<CShader> material = mapMaterial.lock();
         material->BuildConstantParameterBuffer();
      }
   }
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::DeleteRenderResourceFrameDelayed(IObject* pObject)
{
   CSyncCriticalSectionLocker locker( mGeneralCriticalSection );

   mArrayToRelease_Object.push_back(pObject);
}

//----------------------------------------------------------------------------

void CBaseRenderBackend::InternalBeginFrameResourceTick()
{
   CSyncCriticalSectionLocker locker( mGeneralCriticalSection );

   ++mFrameCount;

   mArrayToRelease_Object_0.swap(mArrayToRelease_Object);
   for( int i = 0; i < mArrayToRelease_Object_2.size(); ++i )
   {
      IObject * pResource = mArrayToRelease_Object_2[i];
      delete pResource;
   }
   mArrayToRelease_Object_2.clear();
   mArrayToRelease_Object_2.swap(mArrayToRelease_Object_1);
   mArrayToRelease_Object_1.swap(mArrayToRelease_Object_0);
  
   for(int ii=0;ii<mArrayDynamicVertexBuffer.size();++ii)
   {
      CDynamicVertexBuffer * pDynamicVertexBuffer = mArrayDynamicVertexBuffer[ii];
      pDynamicVertexBuffer->FrameReset();
   }

   for(int ii=0;ii<mArrayDynamicIndexBuffer.size();++ii)
   {
      CDynamicIndexBuffer * pDynamicIndexBuffer = mArrayDynamicIndexBuffer[ii];
      pDynamicIndexBuffer->FrameReset();
   }
}

void CBaseRenderBackend::RegisterDynamicVertexBuffer(CDynamicVertexBuffer* pObj)
{
   RegisterWithArray(pObj, &mArrayDynamicVertexBuffer);
}

void CBaseRenderBackend::RemoveDynamicVertexBuffer(CDynamicVertexBuffer* pObj)
{
   RemoveFromArray(pObj, &mArrayDynamicVertexBuffer);
}

void CBaseRenderBackend::RegisterDynamicIndexBuffer(CDynamicIndexBuffer* pObj)
{
   RegisterWithArray(pObj, &mArrayDynamicIndexBuffer);
}

void CBaseRenderBackend::RemoveDynamicIndexBuffer(CDynamicIndexBuffer* pObj)
{
   RemoveFromArray(pObj, &mArrayDynamicIndexBuffer);
}
