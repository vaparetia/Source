//----------------------------------------------------------------------------
// BP_RenderGS.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "BP_RenderGS.h"

//----------------------------------------------------------------------------

#include "Renderer/Base/Backend/CRenderBackend.h"

//----------------------------------------------------------------------------

int gGS_DebugEnableBlendModeEmulation = 1;
int gGS_DebugEnableAlphaTestFail = 1;

SBlendAttributes gGSBlend;

int gGS_AlphaTestFailStat_Success = 0;
int gGS_AlphaTestFailStat_Fail = 0;
uint64 gGS_LastSetBlendMode = -1;
int gGS_Vita_CurrentSpecialBlend = 0;
TFnSetAlphaFunc gCurrentAlphaFunc = BP_GS_SetAlpha_Default;

//----------------------------------------------------------------------------

//#define BP_UNSUPPORTED_BLENDMODE_IS_BLACK 1

//----------------------------------------------------------------------------

namespace
{
   static const CRenderBackend::EAlphaFunc kAlphaFunctionMap[] =
   {
      CRenderBackend::kAF_Never,
      CRenderBackend::kAF_Always,
      CRenderBackend::kAF_Less,
      CRenderBackend::kAF_LEqual,
      CRenderBackend::kAF_Equal,
      CRenderBackend::kAF_GEqual,
      CRenderBackend::kAF_Greater,
      CRenderBackend::kAF_NotEqual
   };

   static const CRenderBackend::EAlphaFunc kAlphaTestInvMap[] =
   {
      CRenderBackend::kAF_Always,
      CRenderBackend::kAF_Never,
      CRenderBackend::kAF_GEqual,
      CRenderBackend::kAF_Greater,
      CRenderBackend::kAF_NotEqual,
      CRenderBackend::kAF_Less,
      CRenderBackend::kAF_LEqual,
      CRenderBackend::kAF_Equal
   };

   static const CRenderBackend::EDepthFunc kDepthTestMap[] =
   {
      CRenderBackend::kDF_Never,
      CRenderBackend::kDF_Always,
      CRenderBackend::kDF_GEqual,
      CRenderBackend::kDF_Greater,
   };
}

//----------------------------------------------------------------------------

#define GS_ALPHA_MAKE_ABCD(a, b, c, d) (a | b << 2 | c << 4 | d << 6)

union BP_GS_ALPHA_DATA
{
   uint64 data;

#if BPE_IS_ENDIAN_BIG()
   struct  
   {
      uint32 pad1 : 24;
      uint32 fix  : 8;

      union
      {
         uint32 abcd;

         struct
         {
            uint32 pad0 : 24;
            uint32 d : 2;
            uint32 c : 2;
            uint32 b : 2;
            uint32 a : 2;
         };
      };
   };

#else
   struct  
   {
      union
      {
         uint32 abcd;

         struct
         {
            uint32 a : 2;
            uint32 b : 2;
            uint32 c : 2;
            uint32 d : 2;
            uint32 pad0 : 24;
         };
      };

      uint32 fix  : 8;
      uint32 pad1 : 24;
   };
#endif

};

static CVector4 sVTAPS2Regs( 0.f, 0.f, 0.f, 0.f );
static const int skVTAPS2RegIndex = 12;
static const int skVTAPS2AlphaRefBufferIndex = 12;
static const int skVTAPS2AlphaFixBufferIndex = 13;

namespace NVitaRenderGSAlphaTest
{
   bool sAlphaTestEnabled = false;
   CRenderBackend::EAlphaFunc sAlphaTestLastSetFunc = CRenderBackend::kAF_Always;
   bool sIsGreaterZeroATest = false;
}

#if BP_VITA
CRenderHWAllocator::SHandle const * spVTAPS2Uniforms;
static float* spVTAPS2RefUniforms;
static float* spVTAPS2FixUniforms;
void BP_InitDefaultAlphaBlend();
void BP_InitOffscreenAlphaBlend();

union SBlendCache
{
   uint32 data;
   
   // must match SBlendAttributes
   struct
   {
      uint32 attrib        : 6;
      uint32 special       : 1;
      uint32 pad           : 1;
      uint32 blend         : 24;
   };
};

void BP_InitGS()
{
   gGSBlend.data = 0;

   // allocate and fill uniform buffers for alpha ref and fixed alpha
   spVTAPS2Uniforms = gpRenderBackend->AllocFixed(sizeof(float) * 512, 4, kRM_System);
   spVTAPS2RefUniforms = (float*)spVTAPS2Uniforms->mpAddress;
   spVTAPS2FixUniforms = ((float*)spVTAPS2Uniforms->mpAddress) + 256;
   for (int i = 0; i < 256; i++)
   {
      spVTAPS2RefUniforms[i] = (float)i;
      spVTAPS2FixUniforms[i] = (float)i * (1.0f / 128.0f);
   }

   // allocate and fill the precalculated blend infos for default and offscreen
   BP_InitDefaultAlphaBlend();
   BP_InitOffscreenAlphaBlend();
}
#endif

#if defined(BP_VITA)
int BP_GS_GetCurrentVitaAlphaFunc()
{
   if ( NVitaRenderGSAlphaTest::sAlphaTestEnabled )
   {
      if ( NVitaRenderGSAlphaTest::sIsGreaterZeroATest && gGS_IsStandardAlphaBlend && !gpRenderBackend->GetDepthWriteEnabled() )
      {
         return CRenderBackend::kAF_Always;
      }
      else
      {
         return NVitaRenderGSAlphaTest::sAlphaTestLastSetFunc;
      }
   }
   else
   {
      return CRenderBackend::kAF_Always;
   }
}
#endif
int lastref = -1;

void BP_GS_SetAlphaTestFunc( int const func, int const refValue )
{
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaFunc( CRenderBackend::EAlphaFunc( func ), refValue );
#endif

#if defined(BP_VITA)
   // Set the "alpha test reference value" register
   if (lastref != (refValue & 0xFF))
   {
      lastref = refValue & 0xFF;
      gpRenderBackend->SetStaticFragmentUniformBuffer( skVTAPS2AlphaRefBufferIndex, &spVTAPS2RefUniforms[lastref] );
   }

   // Set the function that this function set, and set the global one
   // if alpha test is enabled
   NVitaRenderGSAlphaTest::sAlphaTestLastSetFunc = CRenderBackend::EAlphaFunc( func );
   NVitaRenderGSAlphaTest::sIsGreaterZeroATest = ( func == CRenderBackend::kAF_Greater && refValue == 0 );
#endif
}

void BP_GS_SetAlphaTestEnabled( bool const enabled )
{
#if RENDERBACKEND_SUPPORTS_ALPHA_TEST()
   gpRenderBackend->SetAlphaTestEnable( enabled );
#endif

#if defined(BP_VITA)
   NVitaRenderGSAlphaTest::sAlphaTestEnabled = enabled;
#endif
}

#if defined(BP_VITA)
int lastfix = -1;

static void BP_GS_VTA_SetAlpha( uint64 data )
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;

//   uint32 const fixColor = (fixAlpha << 24) | (fixAlpha << 16) | (fixAlpha << 8) | fixAlpha;

   if (alpha.fix != lastfix)
   {
      lastfix = alpha.fix;
      gpRenderBackend->SetStaticFragmentUniformBuffer( skVTAPS2AlphaFixBufferIndex, &spVTAPS2FixUniforms[alpha.fix] );
   }

   if (alpha.c == 2)
      gGS_Vita_CurrentSpecialBlend = 1;
   else
      gGS_Vita_CurrentSpecialBlend = 0;
}
#endif

void BP_GS_SetAlpha_UseLastBlendMode(int iBlendPass)
{
   BP_GS_SetAlpha(gGS_LastSetBlendMode, iBlendPass);
}

uint64 BP_GS_OptimizeAlpha(uint64 data, int AlwaysOne)
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
   
   if (alpha.abcd == GS_ALPHA_MAKE_ABCD(0, 1, 0, 1))
   {
      if (AlwaysOne)
      {
         alpha.abcd = 0;
      }
   }
   else if (alpha.abcd == GS_ALPHA_MAKE_ABCD(0, 1, 2, 1))
   {
      if (alpha.fix == 0x80)
         alpha.abcd = 0;
   }
   return alpha.abcd;
}

void BP_GS_SetAlpha_NormalBlend(uint64 data)
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;

#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   gGS_AlphaBlendEnabled = true;
   gGS_ForceRGBWhite = false;
   gGS_BlendModeDrawCount = 1;
   gGS_IsStandardAlphaBlend = true;
   gGS_SrcAlphaBlend = true;

   if (gCurrentAlphaFunc == BP_GS_SetAlpha_Default)
      gpRenderBackend->SetBlendMode( true, CRenderBackend::kBF_One, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero );
   else
      gpRenderBackend->SetBlendMode( true, CRenderBackend::kBF_One, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_InvSrcAlpha );
}

void BP_GS_SetAlpha_NoBlend(uint64 data)
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
 
#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   gGS_AlphaBlendEnabled = false;
   gGS_ForceRGBWhite = false;
   gGS_BlendModeDrawCount = 1;
   gGS_IsStandardAlphaBlend = true;
   gGS_SrcAlphaBlend = true;

   if (gCurrentAlphaFunc == BP_GS_SetAlpha_Default)
      gpRenderBackend->SetBlendMode( false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero );
   else
      gpRenderBackend->SetBlendMode( true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero );
}

#if BP_VITA
void BP_GS_SetAlpha_Default_Setup(uint64 data, int iBlendPass);

SBlendCache sDefaultBlendCache[256];

void BP_InitDefaultAlphaBlend()
{
   int i;
   SBlendCache b;
   BP_GS_ALPHA_DATA alpha;

   for (i = 0; i < 256; i++)
   {
      alpha.data = i;
      alpha.fix = 0x80;

      BP_GS_SetAlpha_Default_Setup(alpha.data, 0);

      // attrib must be set after data, GetBlend drops the color mask into that byte
      b.data = gpRenderBackend->GetBlend();
      b.attrib = gGSBlend.data;
      b.special = 0;
      if (i == GS_ALPHA_MAKE_ABCD(1, 0, 2, 1))
         b.special = 1;
      b.pad = 0;

      sDefaultBlendCache[i] = b;
   }
}

void BP_GS_SetAlpha_Default(uint64 data, int iBlendPass)
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
   gGS_LastSetBlendMode = data;
   SBlendCache b;

#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   if ( iBlendPass == 0)
   {
      b = sDefaultBlendCache[alpha.data & 0xFF];

      if (b.special)
      {
         BP_GS_SetAlpha_Default_Setup(data, iBlendPass);
      }
      else
      {
         gGSBlend.data = b.attrib;

         if (gGS_AlphaBlendEnabled)
         {
            gpRenderBackend->SetBlend(b.data);
         }
         else
         {
            gpRenderBackend->DisableBlend();
         }
      }
   }
   else
   {
      gGS_ForceRGBWhite = 0;
      gGS_AlphaBlendEnabled = 1;
      gGS_BlendModeDrawCount = 1;

      switch(alpha.abcd)
      {
      case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
         gGS_BlendModeDrawCount = 2;
         break;
      }
   }
}

void BP_GS_SetAlpha_Default_Setup(uint64 data, int iBlendPass)
#else
void BP_GS_SetAlpha_Default(uint64 data, int iBlendPass)
#endif
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
   gGS_LastSetBlendMode = data;

#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   // NOTE: No over brightning support!
   uint32 fixAlpha = alpha.fix << 1;
   if( fixAlpha > 0xFF )
      fixAlpha = 0xFF;

   uint32 const fixColor = (fixAlpha << 24) | (fixAlpha << 16) | (fixAlpha << 8) | fixAlpha;

   gGS_ForceRGBWhite = 0;
   gGS_AlphaBlendEnabled = 1;
   gGS_BlendModeDrawCount = 1;

   if( iBlendPass == 0 )
   {
         //(Cd - Cs)*As>>7 + Cd > Cd*As - Cs*As + Cd
         //First pass = Cd + Cd*As
         //Second pass = Cd - Cs*As


   // NOTE: No over brightning support!
   gGS_IsStandardAlphaBlend = false;
   gGS_SrcAlphaBlend = 0;

   switch(alpha.abcd)
   {
   case GS_ALPHA_MAKE_ABCD(0, 0, 0, 0):
      //(Cs - Cs)*As>>7 + Cs -> Src*One + Dst*Zero
      gGS_AlphaBlendEnabled = 0;
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 0, 2, 1):
      //(Cs - Cs)*Fix>>7 + Cd -> Src*Zero + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [7627] Cv = ((Cs - Cd)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(0, 1, 0, 1):
      //(Cs - Cd)*As>>7 + Cd -> Src*SrcAlpha + Dst*InvSrcAlpha
      gGS_IsStandardAlphaBlend = true;
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 0, 2):
      //(Cs - Cd)*As>>7 + 0 -> Src*SrcAlpha - Dst*SrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Subtract, CRenderBackend::kBO_Add);
      gGS_SrcAlphaBlend = 1;
      break;

      //Stats: [2] Cv = ((Cs - Cd)*Ad)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(0, 1, 1, 1):
      //(Cs - Cd)*Ad>>7 + Cd -> Src*DstAlpha + Dst*InvDstAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstAlpha, CRenderBackend::kBF_InvDstAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 2, 1):
      //(Cs - Cd)*Fix>>7 + Cd >  Src*Fix + Dst*InvFix
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_InvFixedAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 2, 2):
      //(Cs - Cd)*Fix>>7 + 0 >  Src*Fix - Dst*Fix
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Subtract, CRenderBackend::kBO_Add);
      break;

      //Stats: [217] Cv = ((Cs - 0)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(0, 2, 0, 1):
      //(Cs - 0)*As>>7 + Cd -> Src*SrcAlpha + Dst*One
      gGS_IsStandardAlphaBlend = true;
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 0, 2):
      //(Cs - 0)*As>>7 + 0 -> Src*SrcAlpha + Dst*Zero
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [164] Cv = ((Cs - 0)*Ad)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(0, 2, 1, 1):
      //(Cs-0)*Ad>>7 + Cd > Src*AlphaDest + Dest*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 2, 1):
      //(Cs - 0)*Fix>>7 + Cd -> Src*Fix + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [5] Cv = ((Cd - Cs)*As)>>7 + Cs
   case GS_ALPHA_MAKE_ABCD(1, 0, 0, 0):
      //(Cd - Cs)*As>>7 + Cs -> Src*InvSrcAlpha + Dst*SrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [23] Cv = ((Cd - Cs)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
      //(Cd - Cs)*As>>7 + Cd > Cd*As - Cs*As + Cd
      //First pass = Cd*As - Cs*As
      //Second pass = + Cd

      //As:0.0 -> Cd
      //As:1.0 -> Cd + Cd - Cs
      //As:2.0 -> 2*(Cd-Cs)+Cd

      if( gGS_DebugEnableBlendModeEmulation )
      {
         //NOTE: There is no simple emulation for this behavior!
         // We can emulate it assuming the alpha output doesn't go above 1.0 (need to verify this by looking at actual vertex color values using these textures!) AND
         // ASSUMING that the color is perfect BLACK (which isn't the case for two textures in MGS3: 00b67dea.tga and 00fa7dea.tga), but they are very close to it making me think the different MAY not be noticable.
         // The color portion of the texture IS used but discarded FOR US in this blend mode, we use it to ensure the color is white so we can use the emulation below:
         // Our emulation is: SrcColor (1, 1, 1) * DstColor + SrcAlpha * DstColor

         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstColor, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);

         gGS_ForceRGBWhite = 1;
         gGS_BlendModeDrawCount = 2;
      }
      else
      {
#if BP_UNSUPPORTED_BLENDMODE_IS_BLACK
         //NOTE: There is no simple emulation for this behavior! Make it obvious something is wrong by making it all black!
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
#else
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
#endif
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      }

      break;

   case GS_ALPHA_MAKE_ABCD(1, 0, 0, 2):
      //TED - frame ~11306 of intro cinema
      //(Cd-Cs)*As>>7 + 0 > Cd*As - Cs*As

      BP_MARCO_BREAK;
      BP_DAK_BREAK;

      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
      break;

      //Stats: [1] Cv = ((Cd - Cs)*FIX)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(1, 0, 2, 1):
      {
         //(Cd - Cs)*Fix>>7 + Cd > Cd*Fix - Cs*Fix + Cd
         //Fix = 0: Cd
         //Fix = 1: Cd + Cd - Cs

         // Fix is always 0 according to our texture stats output. This makes this blend mode a LOT easier to emulate correctly :)
         if( fixAlpha != 0 )
         {
            gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
         }
         else
         {
            BP_MARCO_BREAK;
            gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
         }
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      }
      break;

   case GS_ALPHA_MAKE_ABCD(1, 1, 2, 1):
      //(Cd - Cd)*Fix>>7 + Cd
      // v005a_2 near end (fix = 128)
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [2] Cv = ((Cd - 0)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(1, 2, 0, 1):
      //(Cd - 0)*As>>7 + Cd -> Cd*As + Cd
      //As = 0: Cd
      //As = 1: Cd + Cd
      //As = 2: Cd*2 + Cd

#if MGS_VERSION == 3
      //NOTE: There is no simple emulation for this behavior!
      // We can emulate it assuming the alpha output doesn't go above 1.0 (need to verify this by looking at actual vertex color values using these textures!)
      // The color portion of the texture itself isn't used in this blend mode, but we abuse it to ensure the color is white so we can use the emulation below:
      // Our emulation is: SrcColor (1, 1, 1) * DstColor + SrcAlpha * DstColor
      gGS_ForceRGBWhite = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstColor, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
#else

      //TODO Need to double check if the above approach can be used to emulate this behavior or if it exists at all!
      // Happens when leaving first door in w11a

#if BP_UNSUPPORTED_BLENDMODE_IS_BLACK
      //NOTE: There is no simple emulation for this behavior! Make it obvious something is wrong by making it all black!
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
#else
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
#endif


#endif

      break;

      //Stats: [1] Cv = ((Cd - 0)*As)>>7 + 0
   case GS_ALPHA_MAKE_ABCD(1, 2, 0, 2):
      //(Cd - 0)*As>>7 + 0 -> Src*Zero + Dst*SrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 0, 0, 0):
      //(0 - Cs)*As>>7 + Cs -> Src*InvSrcAlpha + Dst*Zero
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [328] Cv = ((0 - Cs)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(2, 0, 0, 1):
      //(0-Cs)*As>>7 + Cd -> Dst - Src*SrcAlpha
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
      break;

      //Stats: [8] Cv = ((0 - Cs)*Ad)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(2, 0, 1, 1):
      //((0 - Cs)*Ad)>>7 + Cd -> Cd * One - Cs * Ad
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 1, 0, 1):
      //((0 - Cd)*As>>7 + Cd -> Src*Zero + Dest*InvSrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 0, 0):
      //(0 - 0)*As>>7 + Cs -> Src*One + Dst*Zero
      gGS_AlphaBlendEnabled = 0;
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

      //Stats: [1] Cv = ((0 - 0)*As)>>7 + Cd
   case GS_ALPHA_MAKE_ABCD(2, 2, 0, 1):
      //(0 - 0)*As>>7 + Cd -> Src*Zero + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 2, 0):
      //(0-0)*Fix>>7 + Cs > Src*One + Dst*Zero
      gGS_AlphaBlendEnabled = 0;
      gpRenderBackend->SetBlendMode(false, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 2, 1):
      //(0-0)*Fix>>7 + Cd > Src*Zero + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 2, 2):
      //codec sequence after intro cinematic
      //(0-0)*Fix>>7 + 0
      BP_MARCO_BREAK;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(1, 2, 2, 2):
      //second codec sequence in w11a (after door is opened)
      //codecrt.c dmapack
      //(Cd-0)*Fix>>7 + 0 > Src*Zero + Dst*Fix
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 2, 2):
      //(Cs-0)*Fix>>7 + 0 > Src*Zero + Dst*One
      // cinematic in v005a_0 (looking at machine through binox)
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   default:
      BP_RENDER_TODO_BREAK;
      //Make it obvious something is wrong by making it all black!
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, fixColor);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
         break;
      }
   }
   else 
   {
      switch(alpha.abcd)
      {
      case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
         gGS_BlendModeDrawCount = 2;
         break;
      }
   }
}


#if BP_VITA
void BP_GS_SetAlpha_OffscreenPrim_Setup(uint64 data, int iBlendPass);

SBlendCache sOffscreenBlendCache[256];

void BP_InitOffscreenAlphaBlend()
{
   int i;
   SBlendCache b;
   BP_GS_ALPHA_DATA alpha;

   for (i = 0; i < 256; i++)
   {
      alpha.data = i;
      alpha.fix = 0x80;

      BP_GS_SetAlpha_OffscreenPrim_Setup(alpha.data, 0);

      // attrib must be set after data, GetBlend drops the color mask into that byte
      b.data = gpRenderBackend->GetBlend();
      b.attrib = gGSBlend.data;
      b.special = 0;
      b.pad = 0;

      sOffscreenBlendCache[i] = b;
   }
}

void BP_GS_SetAlpha_OffscreenPrim(uint64 data, int iBlendPass)
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
   gGS_LastSetBlendMode = data;
   SBlendCache b;

#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   if ( iBlendPass == 0)
   {
      b = sOffscreenBlendCache[alpha.data & 0xFF];

      if (b.special)
      {
         BP_GS_SetAlpha_OffscreenPrim_Setup(data, iBlendPass);
      }
      else
      {
         gGSBlend.data = b.attrib;

         if (gGS_AlphaBlendEnabled)
         {
            gpRenderBackend->SetBlend(b.data);
         }
         else
         {
            gpRenderBackend->DisableBlend();
         }
      }
   }
   else
   {
      gGS_ForceRGBWhite = 0;
      gGS_AlphaBlendEnabled = 1;
      gGS_BlendModeDrawCount = 1;
      gGS_SrcAlphaBlend = 0;

      switch (alpha.abcd)
      {
      case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
         gGS_BlendModeDrawCount = 2;
         break;
      }
   }
}

void BP_GS_SetAlpha_OffscreenPrim_Setup(uint64 data, int iBlendPass)
#else
void BP_GS_SetAlpha_OffscreenPrim(uint64 data, int iBlendPass)
#endif
{
   BP_GS_ALPHA_DATA alpha;
   alpha.data = data;
#if defined(BP_VITA)
   BP_GS_VTA_SetAlpha( data );
#endif

   // NOTE: No over brightning support!
   uint32 fixAlpha = alpha.fix << 1;
   if( fixAlpha > 0xFF )
      fixAlpha = 0xFF;

   uint32 const fixColor = (fixAlpha << 24) | (fixAlpha << 16) | (fixAlpha << 8) | fixAlpha;

   gGS_ForceRGBWhite = 0;
   gGS_AlphaBlendEnabled = 1;
   gGS_BlendModeDrawCount = 1;
   gGS_SrcAlphaBlend = 0;
   gGS_IsStandardAlphaBlend = false;

   if ( iBlendPass == 0 )
   {

   switch(alpha.abcd)
   {
   case GS_ALPHA_MAKE_ABCD(0, 1, 0, 1):
      // src invsrc blend
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_InvSrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_InvSrcAlpha);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 0, 1):
      // additive
      gGS_IsStandardAlphaBlend = true;
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 0, 0, 0):
      //(Cs - Cs)*As>>7 + Cs -> Src*One + Dst*Zero
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 0, 2):
      //(Cs - Cs)*As>>7 + Cs -> Src*One + Dst*Zero
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 0, 2):
      //(Cs - Cd)*As>>7 + 0 -> Src*SrcAlpha - Dst*SrcAlpha
      // visibility = 0*DstAlpha - 1*SrcAlpha, color = Src * SrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Subtract, CRenderBackend::kBO_RevSubtract);
      gGS_SrcAlphaBlend = 1;
      break;

   case GS_ALPHA_MAKE_ABCD(2, 0, 0, 1):
      //(0-Cs)*As>>7 + Cd -> Dst - Src*SrcAlpha
      gGS_SrcAlphaBlend = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 2, 1):
      //(Cs - 0)*Fix>>7 + Cd -> Src*Fix + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_InvFixedAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_InvFixedAlpha, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(1, 2, 0, 1):
      // dst * srca + dst
      // handle as adding srca to visibility
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_One, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 1, 0, 1):
      //((0 - Cd)*As>>7 + Cd -> Src*Zero + Dest*InvSrcAlpha
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_InvSrcAlpha);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 0, 1):
      //(0 - 0)*As>>7 + Cd -> Src*Zero + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 2, 1):
      //(Cs - 0)*Fix>>7 + Cd -> Src*Fix + Dst*One
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 1, 2, 2):
      //(Cs - Cd)*Fix>>7 + 0 >  Src*Fix - Dst*Fix
      // Color = Src*Fix - Dst*Fix, Visibility = 0 - Visibility * fix
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_FixedAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_FixedAlpha, fixColor);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Subtract, CRenderBackend::kBO_Subtract);
      break;

   case GS_ALPHA_MAKE_ABCD(2, 2, 2, 0):
      //(0-0)*Fix>>7 + Cs > Src*One + Dst*Zero
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
      // (Dst - Src) * SrcA + Dst
      // p0: color = color + color * SrcA, vis = vis + SrcA
      // p1: color = color - Src * SrcA, vis = vis
      gGS_SrcAlphaBlend = 1;
      gGS_BlendModeDrawCount = 2;
      gGS_ForceRGBWhite = 1;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_DstColor, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add, CRenderBackend::kBO_Add);
      break;

   case GS_ALPHA_MAKE_ABCD(0, 2, 1, 1):
      // Should be Src*DstA + Dst, but we don't have a correct dsta here, just use SrcA for now
      // BP_RENDER_TODO_BREAK
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;

   default:
      BP_RENDER_TODO_BREAK;
      gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero, CRenderBackend::kBF_Zero);
      gpRenderBackend->SetBlendOp(CRenderBackend::kBO_Add);
      break;
   }
   }
   else
   {
      switch (alpha.abcd)
      {
      case GS_ALPHA_MAKE_ABCD(1, 0, 0, 1):
         gpRenderBackend->SetBlendMode(true, CRenderBackend::kBF_SrcAlpha, CRenderBackend::kBF_One, CRenderBackend::kBF_Zero, CRenderBackend::kBF_One);
         gpRenderBackend->SetBlendOp(CRenderBackend::kBO_RevSubtract, CRenderBackend::kBO_Add);
         gGS_BlendModeDrawCount = 2;
         break;
      }
   }
}

//----------------------------------------------------------------------------

struct SBP_TestState
{
   SBP_TestState()
   {
      mAlphaTestFunc = CRenderBackend::kAF_Greater;
      mAlphaTestRef = 0;
      mAlphaTestEnabled = true;
      mWriteColor = true;
      mWriteAlpha = true;
      mWriteDepth = true;
   }

   CRenderBackend::EAlphaFunc mAlphaTestFunc;   // alpha test function
   int mAlphaTestRef;                           // alpha test reference value
   bool mAlphaTestEnabled;
   bool mWriteColor;                            // should color be written
   bool mWriteAlpha;                            // should alpha be written
   bool mWriteDepth;                            // should depth be written
};

SBP_TestState gBP_AlphaTestState[kPass_Count];

int gGS_AlphaFailPassStart = kPass_DepthWrite;
int gGS_AlphaFailPassCount = 1;
int gBP_EnableAlphaTestGEqualOptimization = 0;

union BP_GS_TEST_DATA
{
   uint64 data;

   struct
   {
#if BPE_IS_ENDIAN_BIG()
      uint64 pad     : 45;
      uint64 ztst    : 2;
      uint64 zte     : 1;
      uint64 datm    : 1;
      uint64 date    : 1;
      uint64 afail   : 2;
      uint64 aref    : 8;
      uint64 atst    : 3;
      uint64 ate     : 1;
#else
      uint64 ate     : 1;
      uint64 atst    : 3;
      uint64 aref    : 8;
      uint64 afail   : 2;
      uint64 date    : 1;
      uint64 datm    : 1;
      uint64 zte     : 1;
      uint64 ztst    : 2;
      uint64 pad     : 45;
#endif
   };
};

void BP_GS_SetTest(uint64 data)
{
   BP_GS_TEST_DATA test;
   test.data = data;

   // ATE: ENABLED
   if( test.ate )
   {
      int const refValue = bpe::min_val(int(test.aref << 1), 255);
      
      gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestEnabled = true;

      gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestFunc = kAlphaFunctionMap[test.atst];
      gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestRef = refValue;

      gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestFunc = kAlphaTestInvMap[test.atst];
      gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestRef = refValue;

      int willAlwaysFail = 0;
      int willAlwaysSucceed = 0;
      {
         switch(test.atst)
         {
         case 0: //NEVER
            willAlwaysFail = 1;
            break;
         
         case 1: //ALWAYS
            willAlwaysSucceed = 1;
            break;
         
         case 2: //LESS
            willAlwaysFail = (test.aref == 0);
            break;

         case 3: //LEQUAL
            willAlwaysSucceed = (test.aref == 255);
            break;

         case 4: //EQUAL
            break;

         case 5: //GEQUAL
            willAlwaysSucceed = (test.aref == 0);
            break;

         case 6: //GREATER
            willAlwaysFail = (test.aref == 255);
            break;

         case 7: //NOTEQUAL
            break;
         }
      }

      switch(test.afail)
      {
         // AFAIL: KEEP
      case 0:
         {
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteColor = 0;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteAlpha = 0;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteDepth = 0;
         }
         break;

         // AFAIL: FB_ONLY
      case 1:
         {
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteColor = 1;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteAlpha = 1;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteDepth = 0;
         }
         break;

         // AFAIL: ZB_ONLY
      case 2:
         {
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteColor = 0;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteAlpha = 0;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteDepth = 1;
         }
         break;

         // AFAIL: RGB_ONLY
      case 3:
         {
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteColor = 1;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteAlpha = 0;
            gBP_AlphaTestState[kPass_NoDepthWrite].mWriteDepth = 0;
         }
         break;
      }

      if( willAlwaysSucceed )
      {
         gGS_AlphaFailPassStart = kPass_DepthWrite;
         gGS_AlphaFailPassCount = 1;
         gBP_AlphaTestState[ kPass_DepthWrite ].mAlphaTestEnabled = false;
      }
      else if( willAlwaysFail )
      {
         gGS_AlphaFailPassStart = kPass_NoDepthWrite;
         gGS_AlphaFailPassCount = 1;
         gBP_AlphaTestState[ kPass_NoDepthWrite ].mAlphaTestEnabled = false;

         if( gBP_EnableAlphaTestGEqualOptimization )
         {
            // Optimize special case where we're ALWAYS failing, and the fail case is GEqual->0.
            // This prevents frame buffer writes for a NULL alpha when depth writes are disabled!
            if( gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestFunc == CRenderBackend::kAF_GEqual && gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestRef == 0 )
               gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestFunc = CRenderBackend::kAF_Greater;
         }
      }
      else
      {
         if( gGS_DebugEnableAlphaTestFail )
         {
            gGS_AlphaFailPassStart = kPass_First;
            gGS_AlphaFailPassCount = kPass_Count;
            gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestEnabled = true;
            gBP_AlphaTestState[kPass_NoDepthWrite].mAlphaTestEnabled = true;
         }
         else
         {
            gGS_AlphaFailPassStart = kPass_DepthWrite;
            gGS_AlphaFailPassCount = 1;
            gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestRef = 0;
            gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestFunc = CRenderBackend::kAF_Greater;
         }
      }
   }
   else
   {
      gGS_AlphaFailPassStart = kPass_DepthWrite;
      gGS_AlphaFailPassCount = 1;

      gBP_AlphaTestState[kPass_DepthWrite].mAlphaTestEnabled = false;
   }

   gpRenderBackend->SetDepthCompareEnabled(true);

   if( test.zte )
      gpRenderBackend->SetDepthFunc(kDepthTestMap[test.ztst]);
   else
      gpRenderBackend->SetDepthFunc(CRenderBackend::kDF_Always);
}

//----------------------------------------------------------------------------

inline void BP_GS_SetupAlphaFailPass_Internal(SBP_TestState const & testState)
{

   if( testState.mAlphaTestEnabled )
   {
      BP_GS_SetAlphaTestEnabled( true );
      BP_GS_SetAlphaTestFunc( testState.mAlphaTestFunc, testState.mAlphaTestRef);
   }
   else
   {
      BP_GS_SetAlphaTestEnabled( false );
   }

   gpRenderBackend->SetColorAlphaWriteEnabled(testState.mWriteColor, testState.mWriteAlpha);
   gpRenderBackend->SetDepthWriteEnabled(testState.mWriteDepth);
}

inline void BP_GS_SetupAlphaFailPass_NoAlphaTest( SBP_TestState const &testState )
{
   BP_GS_SetAlphaTestEnabled( false );
   gpRenderBackend->SetColorAlphaWriteEnabled(testState.mWriteColor, testState.mWriteAlpha);
   gpRenderBackend->SetDepthWriteEnabled(testState.mWriteDepth);
}

void BP_GS_SetupAlphaFailPass(int currentPass)
{
   SBP_TestState const & testState = gBP_AlphaTestState[currentPass];
   BP_GS_SetupAlphaFailPass_Internal(testState);
}

//----------------------------------------------------------------------------

EAlphaTestPass BP_GS_GetPassFor_SingleAlphaValue(int alphaValue)
{
   SBP_TestState const & testStateDepthWrite = gBP_AlphaTestState[kPass_DepthWrite];
   SBP_TestState const & testStateNoDepthWrite = gBP_AlphaTestState[kPass_NoDepthWrite];

   if( alphaValue > testStateDepthWrite.mAlphaTestRef )
   {
      return kPass_DepthWrite;
   }
   else
   {
      return kPass_NoDepthWrite;
   }
}

//----------------------------------------------------------------------------

void BP_GS_SetupAlphaFailPass_SingleAlphaValue(int alphaValue)
{
   SBP_TestState const & testStateDepthWrite = gBP_AlphaTestState[kPass_DepthWrite];
   SBP_TestState const & testStateNoDepthWrite = gBP_AlphaTestState[kPass_NoDepthWrite];

   if( alphaValue > testStateDepthWrite.mAlphaTestRef )
   {
      BP_GS_SetAlphaTestEnabled(true);
      BP_GS_SetAlphaTestFunc(testStateDepthWrite.mAlphaTestFunc, testStateDepthWrite.mAlphaTestRef);

      gpRenderBackend->SetColorAlphaWriteEnabled(testStateDepthWrite.mWriteColor, testStateDepthWrite.mWriteAlpha);
      gpRenderBackend->SetDepthWriteEnabled(testStateDepthWrite.mWriteDepth);
   }
   else
   {
      BP_GS_SetAlphaTestEnabled(false);

      gpRenderBackend->SetColorAlphaWriteEnabled(testStateNoDepthWrite.mWriteColor, testStateNoDepthWrite.mWriteAlpha);
      gpRenderBackend->SetDepthWriteEnabled(testStateNoDepthWrite.mWriteDepth);
   }
}

int gGS_Punchthrough = 0;

void BP_GS_SetupAlphaFailPass_Optimize(int currentPass, SBP_TestState* pOutTestState)
{
   if( pOutTestState )
   {
      SBP_TestState const & testState = *pOutTestState;
      if (gGS_Punchthrough)
         BP_GS_SetupAlphaFailPass_Internal(testState);
      else
         BP_GS_SetupAlphaFailPass_NoAlphaTest(testState);
   }
   else
   {
      BP_GS_SetupAlphaFailPass(currentPass);
   }
}
inline bool BP_GS_PassesTest(int minAlphaValue, int maxAlphaValue, SBP_TestState & test )
{
   switch( test.mAlphaTestFunc )
   {
   case CRenderBackend::kAF_Never:
      return false;
   case CRenderBackend::kAF_Less:
      return maxAlphaValue < test.mAlphaTestRef;
   case CRenderBackend::kAF_Equal:
      return minAlphaValue == test.mAlphaTestRef && maxAlphaValue == test.mAlphaTestRef;
   case CRenderBackend::kAF_LEqual:
      return maxAlphaValue <= test.mAlphaTestRef;
   case CRenderBackend::kAF_Greater:
      return minAlphaValue > test.mAlphaTestRef;
   case CRenderBackend::kAF_NotEqual:
      return minAlphaValue == maxAlphaValue && minAlphaValue != test.mAlphaTestRef;
   case CRenderBackend::kAF_GEqual:
      return minAlphaValue >= test.mAlphaTestRef;
   case CRenderBackend::kAF_Always:
      return true;
   }
   return false;
}
int BP_GS_AlphaFailPass_Optimize(int minAlphaValue, int maxAlphaValue, int minAlphaNonZero, SBP_TestState** ppOutTestState)
{
   SBP_TestState & testStateDepthWrite = gBP_AlphaTestState[kPass_DepthWrite];
   SBP_TestState & testStateNoDepthWrite = gBP_AlphaTestState[kPass_NoDepthWrite];
   gGS_Punchthrough = 0;
   if( BP_GS_PassesTest(minAlphaValue, maxAlphaValue, testStateDepthWrite) )
   {
      *ppOutTestState = &testStateDepthWrite;
      return 1;
   }
   else if( BP_GS_PassesTest(minAlphaValue, maxAlphaValue, testStateNoDepthWrite) )
   {
      *ppOutTestState = &testStateNoDepthWrite;
      return 1;
   }
   else if ( BP_GS_PassesTest(minAlphaNonZero, maxAlphaValue, testStateDepthWrite) )
   {
      gGS_Punchthrough = 1;
      *ppOutTestState = &testStateDepthWrite;
      return 1;
   }
   return gGS_AlphaFailPassCount;
}

//----------------------------------------------------------------------------

void BP_GS_SetupAlphaTestSinglePass(bool forceWriteDepth)
{
   if( gGS_AlphaFailPassCount == 1 )
   {
      // If there is only a single pass then we correctly set it up
      BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart);
   }
   else
   {
      // Otherwise we have to compromise and set something up that will look as close to it as possible.
      SBP_TestState const & sucessState = gBP_AlphaTestState[kPass_DepthWrite];
      SBP_TestState const & failState = gBP_AlphaTestState[kPass_NoDepthWrite];
      bool failWriteDepth = failState.mWriteDepth || forceWriteDepth;

      if( sucessState.mAlphaTestEnabled )
      {
         switch(sucessState.mAlphaTestFunc)
         {
         case CRenderBackend::kAF_LEqual:
         case CRenderBackend::kAF_Less:
            BP_GS_SetAlphaTestEnabled(true);
            BP_GS_SetAlphaTestFunc(CRenderBackend::kAF_Less, 255);
            break;

         case CRenderBackend::kAF_Equal:
         case CRenderBackend::kAF_NotEqual:
            BP_GS_SetAlphaTestEnabled(false);
            break;

         case CRenderBackend::kAF_Greater:
         case CRenderBackend::kAF_GEqual:
            BP_GS_SetAlphaTestEnabled(true);
            BP_GS_SetAlphaTestFunc(CRenderBackend::kAF_Greater, 0);
            break;

         case CRenderBackend::kAF_Never:
         case CRenderBackend::kAF_Always:
            BP_RENDER_TODO_BREAK;
            BP_GS_SetAlphaTestEnabled(false);
            break;
         }
         
      }
      else
      {
         BP_GS_SetAlphaTestEnabled(false);
      }

      gpRenderBackend->SetColorAlphaWriteEnabled(sucessState.mWriteColor, sucessState.mWriteAlpha);
      gpRenderBackend->SetDepthWriteEnabled(failWriteDepth);
   }
}

//----------------------------------------------------------------------------

void BP_GS_SetupAlphaTestSinglePass_ForceDisableAlphaTest(bool forceWriteDepth)
{
   if( gGS_AlphaFailPassCount != 1 )
   {
      SBP_TestState const & sucessState = gBP_AlphaTestState[kPass_DepthWrite];
      SBP_TestState const & failState = gBP_AlphaTestState[kPass_NoDepthWrite];
      bool failWriteDepth = failState.mWriteDepth || forceWriteDepth;

      BP_GS_SetAlphaTestEnabled(false);

      gpRenderBackend->SetColorAlphaWriteEnabled(sucessState.mWriteColor, sucessState.mWriteAlpha);
      gpRenderBackend->SetDepthWriteEnabled(failWriteDepth);
   }
   else
   {
      // If there is only a single pass then we correctly set it up
      BP_GS_SetupAlphaFailPass(gGS_AlphaFailPassStart);
   }
}

//----------------------------------------------------------------------------

void BP_GS_SetClamp(int texUnit, uint64 data)
{
#if BP_VITA
   // this assumes little endian, which we are, and abuses xor
   // result is branchless other than the call to SetTextureAddressMode
   int st = data & 0xF;
   st = st ^ (st >> 1);

   CRenderBackend::EWrapMode const wrapU = (st & 1) ? CRenderBackend::kWM_Clamp : CRenderBackend::kWM_Wrap;
   CRenderBackend::EWrapMode const wrapV = (st & 4) ? CRenderBackend::kWM_Clamp : CRenderBackend::kWM_Wrap;
   gpRenderBackend->SetTextureAddressMode(texUnit, wrapU, wrapV);
#else
   union
   {
      uint64 data;

      struct 
      {
#if BPE_IS_ENDIAN_BIG()
         uint64 pad     : 20;
         uint64 maxV    : 10;
         uint64 minV    : 10;
         uint64 maxu    : 10;
         uint64 minu    : 10;
         uint64 wmt     : 2;
         uint64 wms     : 2;
#else
         uint64 wms     : 2;
         uint64 wmt     : 2;
         uint64 minu    : 10;
         uint64 maxu    : 10;
         uint64 minV    : 10;
         uint64 maxV    : 10;
         uint64 pad     : 20;
#endif
      };
   } clamp;

   clamp.data = data;

#if 0 && MARCO
   BPE_ASSERT_NO_MSG(clamp.wms < 2 && clamp.wmt < 2);
#endif

   CRenderBackend::EWrapMode const wrapU = (clamp.wms == 0 || clamp.wms == 3) ? CRenderBackend::kWM_Wrap : CRenderBackend::kWM_Clamp;
   CRenderBackend::EWrapMode const wrapV = (clamp.wmt == 0 || clamp.wmt == 3) ? CRenderBackend::kWM_Wrap : CRenderBackend::kWM_Clamp;

   gpRenderBackend->SetTextureAddressMode(texUnit, wrapU, wrapV);
#endif
}
