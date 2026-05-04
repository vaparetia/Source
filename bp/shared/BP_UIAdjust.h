//----------------------------------------------------------------------------
// BP_UIAdjust.h
//----------------------------------------------------------------------------
// IMPORTANT:
// This file will be included by C code, as such it MUST stay free of C++ of any sort.
//----------------------------------------------------------------------------

#pragma once
#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

void BP_UI_AdjustSprite(int const originalViewWidth, int const originalViewHeight, 
                        int const newViewWidth, int const newViewHeight,
                        unsigned int bp_tex,
                        int* spriteX, int* spriteY, int* spriteWidth, int *spriteHeight,
                        int* texWidth, int* texHeight);

//----------------------------------------------------------------------------

int BP_UI_IsPixelPerfect(unsigned int bp_tex);
void BP_UI_GetTextureSize(unsigned int bp_tex, int * width, int * height);

//----------------------------------------------------------------------------

float BP_AdjustWidescreenCenterX(float x, float width);

//----------------------------------------------------------------------------

void BP_FitToVirtualPS2Screen( int* outVWidth, int* outVHeight, int realPelWidth,
                               int realPelHeight );

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif
