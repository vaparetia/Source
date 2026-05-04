//----------------------------------------------------------------------------
// BP_DebugPad.cpp
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Engine/Math/BPETypeConversion.inl"
#include "Engine/System/CStopWatch.h"

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"

#include "MGS_Common.h"

#if MGS_VERSION == 3
#include "libgv.h"
#include "pad.h"
#endif

#include "BP_DebugPad.h"

//----------------------------------------------------------------------------

namespace
{
   CStopWatch gRepeatButtonTimer;
   real32 gRepeatButtonTimeout = 0.0f;
   real32 const kButtonInitialRepeatDelay = 0.4f;
   real32 const kButtonRepeatDelay = 0.08f;
}

//----------------------------------------------------------------------------

#if MGS_VERSION == 2
GV_PAD *BP_GV_PadData(int controllerNumber)
{
   return &GV_PadDataDirect[controllerNumber];
}
#endif

//----------------------------------------------------------------------------

#if MGS_VERSION == 3
#define BP_GV_PadData GV_PadDataDirect
#endif

//----------------------------------------------------------------------------

int BP_DebugPad_Press( int port, int button )
{
   GV_PAD* pPad = BP_GV_PadData(port);
   return pPad->press & button;
}

//----------------------------------------------------------------------------

int BP_DebugPad_Status( int port, int button )
{
   GV_PAD* pPad = BP_GV_PadData(port);
   return pPad->status & button;
}

//----------------------------------------------------------------------------

int BP_DebugPad_Repeat( int port, int button )
{
   int padPress  = BP_DebugPad_Press( port, button );
   int padStatus = BP_DebugPad_Status( port, button );

   if( padPress )
   {
      gRepeatButtonTimeout = kButtonInitialRepeatDelay;
      gRepeatButtonTimer.Reset();
      return 1;
   }
   else if(padStatus && gRepeatButtonTimer.GetElapsedTime() > gRepeatButtonTimeout)
   {
      gRepeatButtonTimeout = kButtonRepeatDelay;
      gRepeatButtonTimer.Reset();
      return 1;
   }

   return 0;
}

//----------------------------------------------------------------------------
