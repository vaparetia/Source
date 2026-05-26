//----------------------------------------------------------------------------
// DCMain.cpp
// KallistiOS entry point for the MGS2 Dreamcast port.
//
// KOS calls main() after hardware initialization (video, timers, IRQs).
// KOS_INIT_FLAGS must appear exactly once in the final binary; it lives here.
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include <kos.h>

#include "Engine/System/COsContext.h"
#include "Engine/Resource/CResourcePool.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Resource/CResourceManager.h"
#include "Engine/Mechanics/IObject.h"
#include "Engine/Basics/NEndian.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

//----------------------------------------------------------------------------
// Minimal test asset — IObject subclass so the resource system can delete it.

class CTestAsset : public IObject
{
public:
   explicit CTestAsset(bool valid) : mValid(valid) {}
   virtual ~CTestAsset() {}
   bool mValid;
};

static void TestBinFactory(SFactoryResourceBuildData & buildData, SFactoryReturnResource & returnResource)
{
   bool valid = false;
   if (buildData.mSize >= 4)
   {
      uint8 const * p = reinterpret_cast<uint8 const *>(buildData.mpMemory);
      valid = (p[0] == 'M') && (p[1] == 'G') && (p[2] == 'S') && (p[3] == 'D');
   }
   returnResource.mpResource = new CTestAsset(valid);
}

//----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   // --- OS context ---
   COsContext osContext;

   // --- Resource factory: register .bin loader ---
   CResourceFactory factory;
   {
      uint32 bin4CC;
      char const ext[4] = {'.','b','i','n'};
      memcpy(&bin4CC, ext, 4);
      NEndian::Swap4Bytes(&bin4CC);
      CResourceFactory::SFactoryEntry entry(CResourceFactory::SFactoryEntry::kFF_None, TestBinFactory);
      factory.AddFactory(bin4CC, entry);
   }

   // --- Resource pool ---
   CResourcePool resourcePool(&factory);

   // --- Render backend ---
   CRenderBackend::SRenderFrameBufferSize fbSize(
      CRenderBackend::SRenderFrameBufferSize::kDA_Fullscreen,
      640, 480,   // display
      640, 480,   // framebuffer
      640, 480,   // alt framebuffer
      4.0f / 3.0f);

   CRenderBackend::SRenderInitialization initParams;
   initParams.mpFrameBufferSize    = &fbSize;
   initParams.mFrameBufferSizeCount = 1;

   CRenderBackend renderBackend(resourcePool, initParams);
   renderBackend.InitializeDisplay(CRenderBackend::kSM_Monoscopic);

   // --- Renderer front-end ---
   CRenderer renderer;

   // --- Phase 6 Step 2: load test.bin through CResourcePool/DCCResourceFactoryLoadItem ---
   // Green bg = asset loaded with correct magic; red = failed or wrong magic.
   {
      CResource res = resourcePool.GetResource(CResId("$/test.bin"));
      res.Lock();
      void const * pRaw = res.GetResource_Untyped();
      bool const assetOk = pRaw && static_cast<CTestAsset const *>(pRaw)->mValid;
      pvr_set_bg_color(0.0f, assetOk ? 0.5f : 0.0f, assetOk ? 0.0f : 0.5f);
   }

   // --- Game loop ---
   while (!osContext.mShouldTerminateApplication) {
      renderer.FrameBegin();
      renderer.FrameEnd();
      renderer.FrameFlip();
   }

   return 0;
}
