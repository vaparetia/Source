//----------------------------------------------------------------------------
// BP_SplashScreen.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_SplashScreen.h"

//----------------------------------------------------------------------------

#include "Engine/Resource/CResourceManager.h"
#include "Engine/Resource/CResourceCache.h"

#include "Engine/System/CStopWatch.h"

#include "Renderer/Base/Backend/CRenderBackend.h"

#include "BP_Renderer.h"
#include "BP_RendererDebug.h"
#include "BP_RenderFX.h"
#include "BP_Misc.h"

#define BP_USE_EMBEDDED_RESOURCES 0

#if BP_USE_EMBEDDED_RESOURCES
#include "../GameDataSource/Assets.h"
#endif

#include "boost/optional.hpp"
#include "Engine/System/COsContext.h"

//----------------------------------------------------------------------------

namespace
{
   struct SSplashScreen
   {
      SSplashScreen(real32 const fadeInTime, real32 const holdTime, real32 const fadeOutTime, const char * path)
      :  mFadeInTime(fadeInTime)
      ,  mHoldTime(holdTime)
      ,  mFadeOutTime(fadeOutTime)
      ,  mPath(path)
      ,  mpTexture(NULL)
      {
      }

      real32                  mFadeInTime;
      real32                  mHoldTime;
      real32                  mFadeOutTime;
      const char *            mPath;
      CBaseTexture const *    mpTexture;

      // Only used when resource is not loaded from embedded data (to keep ownership)
      boost::optional< TLockedResource<CBaseTexture> > mTextureOwner;
   };

   static SSplashScreen sAllSplashScreens[] =
   {
      SSplashScreen(1.0f, 6.0f, 1.0f, "$/misc/splashscreen/****/antipiracy_jp.ctxr"),
      SSplashScreen(1.0f, 2.0f, 1.0f, "$/misc/splashscreen/****/konami_splash.ctxr"),
      SSplashScreen(1.0f, 2.0f, 1.0f, "$/misc/splashscreen/****/kojima_splash.ctxr"),
      SSplashScreen(1.0f, 2.0f, 1.0f, "$/misc/splashscreen/****/armature_splash.ctxr"),
      SSplashScreen(1.0f, 2.0f, 1.0f, "$/misc/splashscreen/****/bluepoint_splash.ctxr")
   };

   int const kAllSplashScreenCount = BPE_ARRAY_SIZE(sAllSplashScreens);

   static SSplashScreen *sSplashScreens = NULL;
   static int sSplashScreenCount = 0;

   enum ESplashState
   {
      kSS_Wait,
      kSS_Init,
      kSS_Running,
      kSS_Done
   };

   ESplashState gState = kSS_Wait;
   CStopWatch gWatch;
   int gLastRenderedFrame = -1;

   bool GetCurrentSplashScreen(real32 const inputTime, SSplashScreen const ** pOutSplash, real32 * outSplashTime)
   {
      real32 currentTime = 0.0f;

      *pOutSplash = NULL;
      *outSplashTime = 0.0f;

      for( int i = 0; i < sSplashScreenCount; ++i )
      {
         SSplashScreen const & splash = sSplashScreens[i];
         real32 const currentSplashDuration = splash.mFadeInTime + splash.mHoldTime + splash.mFadeOutTime;
         
         if( inputTime > currentTime && inputTime < (currentTime + currentSplashDuration) )
         {
            *pOutSplash = &splash;
            *outSplashTime = inputTime - currentTime;
            return true;
         }

         currentTime += currentSplashDuration;
      }

      return false;
   }
};

//----------------------------------------------------------------------------

void BP_Splash_Init()
{
   // AS(JM) - In JPN downloadable, display antipiracy first. Otherwise, don't.
   if ( gpOsContext->IsDownloadableVersion() && gpOsContext->mBuildSKU == COsContext::kBS_Japan )
   {
      sSplashScreens = sAllSplashScreens;
      sSplashScreenCount = kAllSplashScreenCount;
   }
   else
   {
      sSplashScreens = &( sAllSplashScreens[1] );
      sSplashScreenCount = kAllSplashScreenCount - 1;
   }

   for( int i = 0; i < sSplashScreenCount; ++i )
   {
      SSplashScreen & splashScreen = sSplashScreens[i];

      CResId const resId(splashScreen.mPath);

#if BPE_TARGET == BPE_TARGET_X360 || BPE_TARGET == BPE_TARGET_PS3
      // On x360/PS3 we load the texture directly from the embedded data
      // There is no TResource<...>
      {
         unsigned long textureIdx = ast_open_file_index(resId.c_str());

         unsigned char const * pTextureDataMemory = ast_get_file_data(textureIdx);
         unsigned int textureDataSize = ast_get_file_length(textureIdx);

         splashScreen.mpTexture = CBaseTexture::Create((void*)pTextureDataMemory, textureDataSize);

         ast_close_file_index(textureIdx);
      }

#elif BPE_TARGET == BPE_TARGET_WIN32 || BPE_TARGET == BPE_TARGET_VITA
      // When loading data from the resource system we have a TResource to keep owner ship.
      splashScreen.mTextureOwner = TLockedResource<CBaseTexture>(gpResources->GetResource(resId));

      splashScreen.mpTexture = splashScreen.mTextureOwner->GetPtr();
#else

#error "Unknown platform"

#endif

   }
}

//----------------------------------------------------------------------------

void BP_Splash_Destroy()
{
   for( int i = 0; i < sSplashScreenCount; ++i )
   {
      SSplashScreen & splashScreen = sSplashScreens[i];
      
      // Is texture loaded from embedded data?
      if( !splashScreen.mTextureOwner )
      {
         // In embedded load case we destroy the texture directly.
         if( splashScreen.mpTexture != NULL )
         {
            CBaseTexture::DeleteTexture((CBaseTexture*)splashScreen.mpTexture);
            splashScreen.mpTexture = NULL;
         }
      }
      else
      {
         // If texture is loaded off disk through resource system, we let the resource system thrash the resource.
         splashScreen.mTextureOwner.reset();
         splashScreen.mpTexture = NULL;
      }
   }

   sSplashScreens = NULL;
   sSplashScreenCount = 0;
}

//----------------------------------------------------------------------------

void BP_Splash_Start()
{
   switch(gState)
   {
   case kSS_Wait:
   case kSS_Done:
      gState = kSS_Init;
      break;
   }
}

//----------------------------------------------------------------------------

void BP_Splash_Finish()
{
   switch(gState)
   {
   case kSS_Wait:
      break;
   
   default:
      BP_Splash_Destroy();
      gState = kSS_Done;
      break;
   }
}


//----------------------------------------------------------------------------

void BP_Splash_UpdateAndRender()
{
   // Allow function to be called multiple times per frame.
   {
      int const currentRenderFrame = gpRenderBackend->GetFrameCount();
      
      if( currentRenderFrame == gLastRenderedFrame )
         return;

      gLastRenderedFrame = currentRenderFrame;
   }

#if BP_TGS_DEMO()
   // Wait for end of TGS demo screen to fade out
   if( BP_TGS_DEMO_IsShowingEndScreen() )
   {
      gWatch.Reset();
      return;
   }
#endif

   if( gState == kSS_Init )
   {
      BP_Splash_Init();
      gWatch.Reset();

      gState = kSS_Running;
   }

   if( gState == kSS_Running )
   {
      real32 const currentTime = gWatch.GetElapsedTime();
      
      SSplashScreen const * splashScreen;
      real32 splashTime;

      if( GetCurrentSplashScreen(currentTime, &splashScreen, &splashTime) )
      {
         real32 const fadeInAmount = bpe::min_val(1.0f, bpe::max_val(0.0f, splashTime / splashScreen->mFadeInTime));
         
         real32 const fadeOutStartTime = splashScreen->mFadeInTime + splashScreen->mHoldTime;
         real32 const fadeOutAmount = 1.0f - bpe::min_val(1.0f, bpe::max_val(0.0f, (splashTime - fadeOutStartTime) / splashScreen->mFadeInTime));

         real32 const currentAmount = fadeInAmount * fadeOutAmount;

#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
         gpRenderBackend->SetAlphaTestEnable(false);
#endif
         gpRenderBackend->SetCullMode(CRenderBackend::kCM_None);

         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
         gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);

         gpRenderBackend->SetDepthCompareEnabled(false);

         BP_DrawFullscreenTextureModulate(splashScreen->mpTexture, CVector4(currentAmount, currentAmount, currentAmount, 1.0f), 0 /*isFilter*/, 0 /*isRGBA*/);

         //BP_DebugText_Print("Splash: Fade: %d%%", (int)(currentAmount * 100));
      }
      else
      {
         BP_Splash_Destroy();
         gState = kSS_Done;
      }

      //BP_DebugText_Print("Splash: Global Time: %f Splash Time: %f", currentTime, splashTime);
   }

   //BP_DebugText_Print("Splash: State: %d", gState);
}

//----------------------------------------------------------------------------

int BP_Splash_IsDone()
{
   return (gState == kSS_Done) ? 1 : 0;
}

//----------------------------------------------------------------------------
// This function determines if there is currently no animation on the splash screen (i.e. it's stuck on a logo waiting for it to finish)
// This is used to allow certain CPU code to run during the loading of stages that causes stalls.

int BP_Splash_IsStatic()
{
   if( gState == kSS_Running )
   {
      real32 const currentTime = gWatch.GetElapsedTime();

      SSplashScreen const * splashScreen;
      real32 splashTime;

      if( GetCurrentSplashScreen(currentTime, &splashScreen, &splashTime) )
      {
         real32 const fadeOutStartTime = splashScreen->mFadeInTime + splashScreen->mHoldTime;

         if( splashTime < splashScreen->mFadeInTime || splashTime > fadeOutStartTime )
            return 0;
      }
   }

   return 1;
}
