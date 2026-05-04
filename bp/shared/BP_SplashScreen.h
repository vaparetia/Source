//----------------------------------------------------------------------------
// BP_SplashScreen.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

void BP_Splash_Start();
void BP_Splash_UpdateAndRender();
int BP_Splash_IsDone();
void BP_Splash_Finish();

int BP_Splash_IsStatic();
   
//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
