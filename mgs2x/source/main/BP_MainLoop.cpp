//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "Renderer/Base/Backend/CRenderBackend.h"

#include "MGS_Common.h"

#if BP_VITA
#  include "gesture_vta.h"
#endif

extern "C" { char *GM_GetArea(); }

extern "C" void BP_MainLoopBegin()
{
#if defined(BP_VITA)
   GestureUpdate();
#endif
   RenderBackend()->SetCurrentAreaDebugName(GM_GetArea());
}
