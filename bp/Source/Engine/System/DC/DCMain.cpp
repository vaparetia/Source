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
#include "Renderer/Base/Backend/CRenderBackend.h"
#include "Renderer/Base/Frontend/CRenderer.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char* argv[])
{
   (void)argc; (void)argv;

   // --- OS context ---
   COsContext osContext;

   // --- Resource pool (no asset factory yet; asset loading is Phase 6) ---
   CResourcePool resourcePool(NULL);

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

   // --- Game loop ---
   while (!osContext.mShouldTerminateApplication) {
      renderer.FrameBegin();
      // Draw calls go here once assets and game logic are wired up (Phase 6).
      renderer.FrameEnd();
      renderer.FrameFlip();
   }

   return 0;
}
