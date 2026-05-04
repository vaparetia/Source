//----------------------------------------------------------------------------
// BP_DebugPad.h
//----------------------------------------------------------------------------

#include "BP_BuildDefines.h"

//----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// DEFINES
//----------------------------------------------------------------------------

#if MGS_VERSION == 2

#define BP_PAD_LEFT_ANALOG_CLICK    PAD_AL
#define BP_PAD_A                    PAD_A
#define BP_PAD_B                    PAD_B
#define BP_PAD_L                    PAD_L
#define BP_PAD_R                    PAD_R
#define BP_PAD_U                    PAD_U
#define BP_PAD_D                    PAD_D
#define BP_PAD_R1                   PAD_R1
#define BP_PAD_R2                   PAD_R2
#define BP_PAD_R3                   PAD_R3
#define BP_PAD_L1                   PAD_L1
#define BP_PAD_L2                   PAD_L2
#define BP_PAD_L3                   PAD_L3
#define BP_PAD_START                PAD_STA
#define BP_PAD_SELECT               PAD_SEL
#define BP_PAD_CROSS                PAD_B
#define BP_PAD_CIRCLE               PAD_A
#define BP_PAD_SQUARE               PAD_Y
#define BP_PAD_TRIANGLE             PAD_X

#else // MGS_VERSION == 3

#define BP_PAD_LEFT_ANALOG_CLICK    _PAD_L3
#define BP_PAD_A                    _PAD_A
#define BP_PAD_B                    _PAD_B
#define BP_PAD_L                    _PAD_L
#define BP_PAD_R                    _PAD_R
#define BP_PAD_U                    _PAD_U
#define BP_PAD_D                    _PAD_D
#define BP_PAD_R1                   _PAD_R1
#define BP_PAD_R2                   _PAD_R2
#define BP_PAD_R3                   _PAD_R3
#define BP_PAD_L1                   _PAD_L1
#define BP_PAD_L2                   _PAD_L2
#define BP_PAD_L3                   _PAD_L3
#define BP_PAD_START                _PAD_START
#define BP_PAD_SELECT               _PAD_SELECT
#define BP_PAD_CROSS                _PAD_B
#define BP_PAD_CIRCLE               _PAD_A
#define BP_PAD_SQUARE               _PAD_Y
#define BP_PAD_TRIANGLE             _PAD_X

#endif

//----------------------------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------------------------

int BP_DebugPad_Press( int port, int button );
int BP_DebugPad_Status( int port, int button );
int BP_DebugPad_Repeat( int port, int button );

//----------------------------------------------------------------------------

#ifdef __cplusplus
};
#endif

//----------------------------------------------------------------------------
