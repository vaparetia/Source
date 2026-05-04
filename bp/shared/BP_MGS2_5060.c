//----------------------------------------------------------------------------
// BP_50_60.cpp
//----------------------------------------------------------------------------

//#include "Engine/Stdafx.h"
//#include "Engine/System/COsContext.h"

//#include "MGS_Common.h"

#if MGS_VERSION == 3
#include "libgv.h"
#endif

#if BP_PS3
#include <cell/sysmodule.h>
#include <cell/rtc.h>
#else
#include <time.h>
#endif

#include "BP_Misc.h"

#include "libutl.h"

float BP_P_GRAVITY;

int BP_BASE_TICK(void)
{
   if ( BP_IsPAL() == TRUE )
      return 300 / 50; //6
   else 
      return 300 / 60; //5
}

int BP_AdjustTick(int _tick)
{
   if ( BP_IsPAL() == TRUE )
      return (((_tick)*5+1)/6);
   else
      return _tick;
}

int BP_AdjustTick2(int _tick)
{
   if ( BP_IsPAL() == TRUE )
      return (((_tick)*6)/5);
   else
      return _tick;
}

float BP_AdjustTick3(float _tick)
{
   if ( BP_IsPAL() == TRUE )
      return (((_tick)*6.f)/5.f);
   else
      return _tick;
}

float BP_AdjustTick4(float _tick)
{
   if ( BP_IsPAL() == TRUE )
      return (_tick*1.44f);
   else
      return _tick;
}

float BP_AdjustTick5(float _tick)
{
   if ( BP_IsPAL() == TRUE )
      return _tick;
   else
      return (((_tick)*5)/6);
}


int BP_HSYNC_PER_VSYNC(void)
{
   if ( BP_IsPAL() == TRUE )
      return 312;
   else
      return 262;   // 1frame == 262 HSync == 16.66msec
}

int BP_FRAMES_PER_SEC(void)
{
   if ( BP_IsPAL() == TRUE )
      return 50;
   else
      return 60;
}

void BP_SetTimeBased_PAL_NTSC_Globals()
{
   if ( BP_IsPAL() == TRUE )
      BP_P_GRAVITY =	(-CVM2N(9.8f)/(50.0f*50.0f));
   else
      BP_P_GRAVITY =	(-CVM2N(9.8f)/(60.0f*60.0f));
}

int BP_FramesSysToFrames60( int frames )
{
   if( BP_IsPAL() )
   {
      int frames60 = (int)((u_int64)(frames) * 6 / 5);
      return frames60;
   }
   else
   {
      return frames;
   }
}

int BP_Frames60ToFramesSys( int frames60 )
{
   if( BP_IsPAL() )
   {
      int frames = (int)((u_int64)(frames60) * 5 / 6);
      return frames;
   }
   else
   {
      return frames60;
   }
}

