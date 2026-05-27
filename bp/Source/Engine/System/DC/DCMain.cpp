//----------------------------------------------------------------------------
// DCMain.cpp
// KallistiOS entry point for the MGS2 Dreamcast port.
//
// KOS calls main() after hardware initialization (video, timers, IRQs).
// KOS_INIT_FLAGS must appear exactly once in the final binary; it lives here.
//----------------------------------------------------------------------------

#include "Engine/StdAfx.h"

#include <kos.h>
#include <dc/maple/controller.h>

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

   // --- Phase 10: load real game mesh (crate) + perspective camera ---
   // Green bg = mesh loaded with 1 chunk; red = failed.
   CMesh const * pMesh = NULL;
   CResource meshRes = resourcePool.GetResource(CResId("$/crate.psc"));
   meshRes.Lock();
   pMesh = static_cast<CMesh const *>(meshRes.GetResource_Untyped());
   bool const meshOk = pMesh && pMesh->GetMeshChunks().size() == 1;
   pvr_set_bg_color(0.0f, meshOk ? 0.3f : 0.0f, meshOk ? 0.0f : 0.3f);

   // Identity camera at origin — cube is at world (0, 0, -3) in view frustum.
   renderBackend.SetCameraMatrix(CMatrix34::Identity());
   // Perspective: 60° FOV, 4:3 aspect, near=0.1, far=100.
   renderBackend.SetPerspectiveProjection(CAngle::FromDegrees(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);

   // --- Load cube.tex from GD-ROM and upload to PVR VRAM ---
   {
      FILE* f = fopen("/cd/crate.tex", "rb");
      if (f) {
         fseek(f, 0, SEEK_END);
         int const texBytes = (int)ftell(f);
         fseek(f, 0, SEEK_SET);
         void* buf = malloc((size_t)texBytes);
         if (buf) {
            fread(buf, 1, (size_t)texBytes, f);
            pvr_ptr_t vramTex = pvr_mem_malloc((size_t)texBytes);
            pvr_txr_load_ex(buf, vramTex, 256, 256, PVR_TXRLOAD_16BPP);
            free(buf);
            renderBackend.SetDCTexture((void*)(uintptr_t)vramTex, 256, 256);
         }
         fclose(f);
      }
   }

   // Initial rotation angles; controller input accumulates into these each frame.
   CAngle rotX = CAngle::FromDegrees(30.0f);
   CAngle rotY = CAngle::FromDegrees(45.0f);
   float const rotSpeed = 90.0f;   // degrees/second
   float const dt       = 1.0f / 30.0f;

   // --- Game loop ---
   while (!osContext.mShouldTerminateApplication) {
      // --- Controller input ---
      maple_device_t* dev = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
      if (dev) {
         cont_state_t* st = (cont_state_t*)maple_dev_status(dev);
         if (st) {
            if (st->buttons & CONT_DPAD_LEFT)  rotY -= CAngle::FromDegrees(rotSpeed * dt);
            if (st->buttons & CONT_DPAD_RIGHT) rotY += CAngle::FromDegrees(rotSpeed * dt);
            if (st->buttons & CONT_DPAD_UP)    rotX -= CAngle::FromDegrees(rotSpeed * dt);
            if (st->buttons & CONT_DPAD_DOWN)  rotX += CAngle::FromDegrees(rotSpeed * dt);
            // Analog stick (dead zone ±32)
            if (st->joyx >  32) rotY += CAngle::FromDegrees(rotSpeed * dt * (st->joyx / 128.0f));
            if (st->joyx < -32) rotY -= CAngle::FromDegrees(rotSpeed * dt * (-st->joyx / 128.0f));
            if (st->joyy >  32) rotX += CAngle::FromDegrees(rotSpeed * dt * (st->joyy / 128.0f));
            if (st->joyy < -32) rotX -= CAngle::FromDegrees(rotSpeed * dt * (-st->joyy / 128.0f));
            if (st->buttons & CONT_START) osContext.mShouldTerminateApplication = true;
         }
      }

      CMatrix4 const cubeModel =
         CMatrix4::Translation(CVector3(0.0f, 0.0f, -3.0f)) *
         CMatrix4::RotateY(rotY) *
         CMatrix4::RotateX(rotX);

      renderer.FrameBegin();

      if (pMesh) {
         renderBackend.SetModelMatrix(cubeModel);
         CShaderVertexDataBinding binding;
         pMesh->GetMeshBuffers().SetVertexData(binding);
         renderBackend.SetIndexData(pMesh->GetMeshBuffers().GetIndexBuffer());
         CMeshChunk const & chunk = pMesh->GetMeshChunks()[0];
         renderBackend.RenderPrimitives(chunk.mPrimitiveType, chunk.mIndexBufferOffset, chunk.mIndicesCount);
      }

      renderer.FrameEnd();
      renderer.FrameFlip();
   }

   return 0;
}
