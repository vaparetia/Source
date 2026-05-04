//----------------------------------------------------------------------------
// PlatformIncludes.h
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Cg/cg.h"

#ifdef BPE_DEBUG
// NOTE: This must be commented in if linking the GCM debug libs.
// Otherwise JTS RSX synchronization will hang.

//   #define CELL_GCM_DEBUG

#endif

#include "cell/gcm.h"
