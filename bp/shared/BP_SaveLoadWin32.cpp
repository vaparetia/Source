//------------------------------------------------------------------------------------------
// BP_SaveLoadWin32.cpp
// BP adapted from CP3
//
// Utility functions for saving/loading on Windows.
//------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <direct.h>

#include "assert.h"
#include "BP_SaveLoad.h"
#include "BP_SaveLoadMGS.h"

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"
#include "Engine/System/COsContext.h"

#pragma warning(disable:4996)

//----------------------------------------------------------------------------
//Note: this is currently just a place to throw stub functions for BP_SaveLoadMGS.cpp on Windows
//because that file is not compiled in due to a lack of Win32 implementations of the BP_SaveLoad API.
int MGS_SaveStatus_IsCorrupted() { return 0; }
int MGS_SaveStatus_WrongUser() { return 0; }
void MGS_SaveStatus_ClearWrongUser() {}

//----------------------------------------------------------------------------
