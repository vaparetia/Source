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
#include "Engine/Math/CMatrix34.h"
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"
#include "Renderer/Base/Primitive/CMesh.h"
#include "Renderer/Base/Primitive/ProgShader/PSCMeshBuffers.h"
#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

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

   // --- Resource factory: register .bin and .psc loaders ---
   CResourceFactory factory;
   {
      uint32 bin4CC;
      char const binExt[4] = {'.','b','i','n'};
      memcpy(&bin4CC, binExt, 4);
      NEndian::Swap4Bytes(&bin4CC);
      CResourceFactory::SFactoryEntry binEntry(CResourceFactory::SFactoryEntry::kFF_None, TestBinFactory);
      factory.AddFactory(bin4CC, binEntry);
   }
   {
      uint32 psc4CC;
      char const pscExt[4] = {'.','p','s','c'};
      memcpy(&psc4CC, pscExt, 4);
      NEndian::Swap4Bytes(&psc4CC);
      CResourceFactory::SFactoryEntry pscEntry(CResourceFactory::SFactoryEntry::kFF_None, CMesh::FMeshFactory);
      factory.AddFactory(psc4CC, pscEntry);
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

   // --- Phase 7 Step 1: load test.psc mesh ---
   // Green bg = mesh loaded with 1 chunk; red = failed.
   CMesh const * pMesh = NULL;
   CResource meshRes = resourcePool.GetResource(CResId("$/test.psc"));
   meshRes.Lock();
   pMesh = static_cast<CMesh const *>(meshRes.GetResource_Untyped());
   bool const meshOk = pMesh && pMesh->GetMeshChunks().size() == 1;
   pvr_set_bg_color(0.0f, meshOk ? 0.5f : 0.0f, meshOk ? 0.0f : 0.5f);

   // --- Phase 7 Step 2: set identity camera ---
   renderBackend.SetCameraMatrix(CMatrix34::Identity());

   // Persistent index buffer for the loaded mesh triangle.
   static uint16 s_triIdx[] = { 0, 1, 2 };
   CIndexBufferChunk const idxChunk(NULL, s_triIdx, 3);

   // --- Game loop ---
   while (!osContext.mShouldTerminateApplication) {
      renderer.FrameBegin();

      if (pMesh) {
         CShaderVertexDataBinding binding;
         pMesh->GetMeshBuffers().SetVertexData(binding);
         renderBackend.SetIndexData(idxChunk);
         CMeshChunk const & chunk = pMesh->GetMeshChunks()[0];
         renderBackend.RenderPrimitives(chunk.mPrimitiveType, chunk.mIndexBufferOffset, chunk.mIndicesCount);
      }

      renderer.FrameEnd();
      renderer.FrameFlip();
   }

   return 0;
}
