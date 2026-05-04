//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_pad.c
   プレイヤー共通PAD定義

   2002/03/26	M.Sonoyama
   $Id: pl_pad.c,v 1.7 2002/12/17 04:09:48 takaki Exp $
*/

#include <stdio.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include <libgv.h>
#include "gameheader.h"
#include "BP_Misc.h"

int gBP_UseVitaControlOverrides = BP_VITA_CONTROL_SCHEME_V2; // Can be switched from the controls debug menu.
const char* gBP_VitaControlsEnum[] = 
{
   "Original",
   "Version1",
   "Version2",
};


/* default is PATTERNA */
int PL_PAD_ACTION        = (PAD_X)           ;
int PL_PAD_SUBJECT       = 0;
int PL_PAD_STOP_AIM      = 0;
int PL_PAD_SQUAT         = (PAD_B)           ;
int PL_PAD_WEAPON        = 0;
int PL_PAD_PRESS_WEAPON  = (PAD_PRESS_Y)     ;
int PL_PAD_PUNCH         = (PAD_A)           ;
int PL_PAD_CHANGE_WEAPON = (PAD_R2)          ;
int PL_PAD_PEEP          = (PAD_R2 | PAD_L2) ;
int PL_PAD_PEEP_R        = (PAD_R2)          ;
int PL_PAD_PEEP_L        = (PAD_L2)          ;
int PL_PAD_PRESS_PEEP_R  = (PAD_PRESS_R2)    ;
int PL_PAD_PRESS_PEEP_L  = (PAD_PRESS_L2)    ; 
int PL_PAD_CAPUTRE       = (PAD_Y)           ;
int PL_PAD_LOCKON        = (PAD_L1);
int PL_PAD_KNOCK         = (PAD_A)           ;
int PL_PAD_HANG          = (PAD_Y)           ;
int PL_PAD_BEYOND        = (PAD_X)           ;
int PL_PAD_READY_RUN     = (PAD_B | PAD_L1)  ;
int PL_PAD_BEYOND_DOWN   = (PAD_B)           ;
int PL_PAD_LOCKER_ZOOM   = (PAD_R1)  ;
int PL_PAD_PRESS_LOCKER  = (PAD_PRESS_R1)  ;

int PL_PAD_ZOOMIN        = (PAD_B)           ;
int PL_PAD_PRESS_ZOOMIN  = (PAD_PRESS_B)     ;
int PL_PAD_ZOOMOUT       = (PAD_A)           ;
int PL_PAD_PRESS_ZOOMOUT = (PAD_PRESS_A)     ;

//BP - Modified to 0, to gurrantee it gets set up correctly in the SetPattern functions below (if they don't get called for some reason the button won't work and it will be more obvious!)
int PL_PAD_ITEMUSE       = (0); 

extern int PL_SubjectToggle ;


#ifdef KP_WINDOWS
int PL_PAD_SUBJECT_TOGGLE = (PAD_EX1)        ;
#endif

/* 主観移動用パッド定義 */
void PL_PadSetPatternSubjectMoveA( void )
{
	GV_PadSetDefaultKeyConf() ;
	GV_InitPressureChangeFlag() ;

    PL_PAD_ACTION        = (PAD_X)           ;
    PL_PAD_SUBJECT       = (0);
#ifdef KP_WINDOWS
	PL_PAD_SUBJECT_TOGGLE= (0)               ;
#endif
    PL_PAD_STOP_AIM      = BP_Pad_GetSubJctCameraAssignment();
    PL_PAD_SQUAT         = (PAD_B)           ;

    // AS MCampbell - Controls have been decided on for shoot and since we cannot pass the control scheme just put in 
    // the controls here.
    // Bug: MGSTWO-3269
    PL_PAD_WEAPON        = PAD_R1; //BP_Pad_GetShootAssignment();
    PL_PAD_PRESS_WEAPON  = PAD_PRESS_R1; //BP_Pad_GetShootPressAssignment();

    PL_PAD_PUNCH         = (PAD_A)           ;
    PL_PAD_CHANGE_WEAPON = (PAD_R2)          ;
    PL_PAD_PEEP          = (PAD_R2 | PAD_L2) ;
    PL_PAD_PEEP_R        = (PAD_R2)          ;
    PL_PAD_PEEP_L        = (PAD_L2)          ;
    PL_PAD_PRESS_PEEP_R  = (PAD_PRESS_R2)    ;
    PL_PAD_PRESS_PEEP_L  = (PAD_PRESS_L2)    ;
    PL_PAD_CAPUTRE       = PL_PAD_WEAPON     ;
    PL_PAD_LOCKON        = (BP_Pad_GetLockOnAssignment());
    PL_PAD_KNOCK         = (PAD_A)           ;
    PL_PAD_HANG          = PL_PAD_WEAPON     ;
    PL_PAD_BEYOND        = (PAD_X)           ;
    PL_PAD_READY_RUN     = (PAD_B | PAD_L1)  ;
    PL_PAD_BEYOND_DOWN   = (PAD_B)           ;             
    PL_PAD_ITEMUSE       = BP_Pad_GetOkAssignment();
    PL_PAD_LOCKER_ZOOM   = (PL_PAD_SUBJECT)  ;
    PL_PAD_PRESS_LOCKER  = (PAD_PRESS_L1)  ;

    PL_PAD_ZOOMIN        = (PAD_B)           ;
    PL_PAD_PRESS_ZOOMIN  = (PAD_PRESS_B)     ;
    PL_PAD_ZOOMOUT       = (PAD_A)           ;
    PL_PAD_PRESS_ZOOMOUT = (PAD_PRESS_A)     ;

    PL_SubjectToggle = 0 ;
    GV_SetPressureChangeFlag( GV_PAD_ANALOG_L1_CHANGE|
       GV_PAD_ANALOG_R1_CHANGE ) ;
    GV_PadSeparateOnScn( 0 ) ;/* アナログとデジタル方向キーの分離*/


#ifdef KP_XBOX
	GV_SetChangePressure( PAD_PRESS_L1, 2 ) ;
	GV_SetChangePressure( PAD_PRESS_R1, 2 ) ;
#endif
}

void PL_PadSetPatternSubjectMoveB( void )
{
	GV_PadSetDefaultKeyConf() ;
	GV_InitPressureChangeFlag() ;

    PL_PAD_ACTION        = (PAD_X)           ;
    PL_PAD_SUBJECT       = 0               ;
#ifdef KP_WINDOWS
	PL_PAD_SUBJECT_TOGGLE= (0)               ;
#endif
    PL_PAD_STOP_AIM      = BP_Pad_GetSubJctCameraAssignment();
    PL_PAD_SQUAT         = (PAD_B)           ;

    // AS MCampbell - Controls have been decided on for shoot and since we cannot pass the control scheme just put in 
    // the controls here.
    // Bug: MGSTWO-3269
    PL_PAD_WEAPON        = PAD_Y; //BP_Pad_GetShootAssignment();
    PL_PAD_PRESS_WEAPON  = PAD_PRESS_Y; //BP_Pad_GetShootPressAssignment();
    
    PL_PAD_PUNCH         = (PAD_A)           ;
    PL_PAD_CHANGE_WEAPON = (PAD_R2)          ;
    PL_PAD_PEEP          = (PAD_R2 | PAD_L2) ;
    PL_PAD_PEEP_R        = (PAD_R2)          ;
    PL_PAD_PEEP_L        = (PAD_L2)          ;
    PL_PAD_PRESS_PEEP_R  = (PAD_PRESS_R2)    ;
    PL_PAD_PRESS_PEEP_L  = (PAD_PRESS_L2)    ;
    PL_PAD_CAPUTRE       = PL_PAD_WEAPON     ;
    PL_PAD_LOCKON        = (BP_Pad_GetLockOnAssignment());
    PL_PAD_KNOCK         = (PAD_A)           ;
    PL_PAD_HANG          = PL_PAD_WEAPON     ;
    PL_PAD_BEYOND        = (PAD_X)           ;
    PL_PAD_READY_RUN     = (PAD_B | PAD_L1)  ;
    PL_PAD_BEYOND_DOWN   = (PAD_B)           ;             
    PL_PAD_ITEMUSE       = BP_Pad_GetOkAssignment();
    PL_PAD_LOCKER_ZOOM   = (PL_PAD_SUBJECT)  ;
    PL_PAD_PRESS_LOCKER  = (PAD_PRESS_L1)  ;

    PL_PAD_ZOOMIN        = (PAD_B)           ;
    PL_PAD_PRESS_ZOOMIN  = (PAD_PRESS_B)     ;
    PL_PAD_ZOOMOUT       = (PAD_A)           ;
    PL_PAD_PRESS_ZOOMOUT = (PAD_PRESS_A)     ;

    PL_SubjectToggle = 0 ;
    GV_SetPressureChangeFlag( GV_PAD_ANALOG_Y_CHANGE |
       GV_PAD_ANALOG_L1_CHANGE|
       GV_PAD_ANALOG_R1_CHANGE ) ;
    GV_PadSeparateOnScn( 0 ) ;/* アナログとデジタル方向キーの分離*/
#ifdef KP_XBOX
    GV_SetChangePressure( PAD_PRESS_L1, 2 ) ;
    GV_SetChangePressure( PAD_PRESS_R1, 2 ) ;
#endif
}


void PL_PadSetPatternSubjectMoveC( void )
{
	GV_PadSetDefaultKeyConf() ;
	GV_InitPressureChangeFlag() ;

    PL_PAD_ACTION        = (PAD_X)           ;
    PL_PAD_SUBJECT       = (BP_Pad_GetSubJctCameraAssignment())               ;
#ifdef KP_WINDOWS
	PL_PAD_SUBJECT_TOGGLE= (0)               ;
#endif
    PL_PAD_STOP_AIM      = BP_Pad_GetSubJctCameraAssignment();
    PL_PAD_SQUAT         = (PAD_B)           ;

    // AS MCampbell - Controls have been decided on for shoot and since we cannot pass the control scheme just put in 
    // the controls here.
    // Bug: MGSTWO-3269
    PL_PAD_WEAPON        = PAD_Y; //BP_Pad_GetShootAssignment();
    PL_PAD_PRESS_WEAPON  = PAD_PRESS_Y; //BP_Pad_GetShootPressAssignment();
    
    PL_PAD_PUNCH         = (PAD_A)           ;
    PL_PAD_CHANGE_WEAPON = (PAD_R2)          ;
    PL_PAD_PEEP          = (PAD_R2 | PAD_L2) ;
    PL_PAD_PEEP_R        = (PAD_R2)          ;
    PL_PAD_PEEP_L        = (PAD_L2)          ;
    PL_PAD_PRESS_PEEP_R  = (PAD_PRESS_R2)    ;
    PL_PAD_PRESS_PEEP_L  = (PAD_PRESS_L2)    ;
    PL_PAD_CAPUTRE       = PL_PAD_WEAPON     ;
    PL_PAD_LOCKON        = (BP_Pad_GetLockOnAssignment());
    PL_PAD_KNOCK         = (PAD_A)           ;
    PL_PAD_HANG          = PL_PAD_WEAPON     ;
    PL_PAD_BEYOND        = (PAD_X)           ;
    PL_PAD_READY_RUN     = (PAD_B | PAD_L1)  ;
    PL_PAD_BEYOND_DOWN   = (PAD_B)           ;             
    PL_PAD_ITEMUSE       = BP_Pad_GetOkAssignment();
    PL_PAD_LOCKER_ZOOM   = (PL_PAD_SUBJECT)  ;
    PL_PAD_PRESS_LOCKER  = (PAD_PRESS_L1)  ;

    PL_PAD_ZOOMIN        = (PAD_B)           ;
    PL_PAD_PRESS_ZOOMIN  = (PAD_PRESS_B)     ;
    PL_PAD_ZOOMOUT       = (PAD_A)           ;
    PL_PAD_PRESS_ZOOMOUT = (PAD_PRESS_A)     ;


    GV_PadSetKeyConf( PAD_PRESS_X, PAD_PRESS_B ) ;
    GV_PadSetKeyConf( PAD_PRESS_B, PAD_PRESS_X ) ;

    GV_SetPressureChangeFlag( GV_PAD_ANALOG_Y_CHANGE |
       GV_PAD_ANALOG_L1_CHANGE|
       GV_PAD_ANALOG_R1_CHANGE ) ;
    PL_SubjectToggle = 0 ;
    GV_PadSeparateOnScn( 0 ) ;/* アナログとデジタル方向キーの分離*/
#ifdef KP_XBOX
    GV_SetChangePressure( PAD_PRESS_L1, 2 ) ;
    GV_SetChangePressure( PAD_PRESS_R1, 2 ) ;
#endif
}

/* パッド定義 */
void PL_PadSetPatternA( void )
{
	GV_PadSetDefaultKeyConf() ;
	GV_InitPressureChangeFlag() ;

    PL_PAD_ACTION        = (PAD_X)           ;
    PL_PAD_SUBJECT       = BP_Pad_GetSubJctCameraAssignment();
#ifdef KP_WINDOWS
	PL_PAD_SUBJECT_TOGGLE= (PAD_EX1)         ;
#endif
    PL_PAD_SQUAT         = (PAD_B)           ;

    // AS MCampbell - Controls have been decided on for shoot and since we cannot pass the control scheme just put in 
    // the controls here.
    PL_PAD_WEAPON        = PAD_Y; //BP_Pad_GetShootAssignment();
    PL_PAD_PRESS_WEAPON  = PAD_PRESS_Y; //BP_Pad_GetShootPressAssignment();
    
    PL_PAD_PUNCH         = (PAD_A)           ;
    PL_PAD_CHANGE_WEAPON = (PAD_R2)          ;
    PL_PAD_PEEP          = (PAD_R2 | PAD_L2) ;
    PL_PAD_PEEP_R        = (PAD_R2)          ;
    PL_PAD_PEEP_L        = (PAD_L2)          ;
    PL_PAD_PRESS_PEEP_R  = (PAD_PRESS_R2)    ;
    PL_PAD_PRESS_PEEP_L  = (PAD_PRESS_L2)    ;
    PL_PAD_CAPUTRE       = PL_PAD_WEAPON     ;
    PL_PAD_LOCKON        = (BP_Pad_GetLockOnAssignment());
    PL_PAD_KNOCK         = (PAD_A)           ;
    PL_PAD_HANG          = PL_PAD_WEAPON     ;
    PL_PAD_BEYOND        = (PAD_X)           ;
    PL_PAD_READY_RUN     = (PAD_B | BP_Pad_GetLockOnAssignment())  ;
    PL_PAD_BEYOND_DOWN   = PL_PAD_SQUAT      ;             
    PL_PAD_ITEMUSE       = BP_Pad_GetOkAssignment();
	PL_PAD_LOCKER_ZOOM   = (PAD_R1)        ;
	PL_PAD_PRESS_LOCKER  = (PAD_PRESS_R1)  ;

	PL_PAD_ZOOMIN        = (PAD_B)           ;
	PL_PAD_PRESS_ZOOMIN  = (PAD_PRESS_B)     ;
	PL_PAD_ZOOMOUT       = (PAD_A)           ;
	PL_PAD_PRESS_ZOOMOUT = (PAD_PRESS_A)     ;


	GV_SetPressureChangeFlag( GV_PAD_ANALOG_Y_CHANGE |
							  GV_PAD_ANALOG_L1_CHANGE|
							  GV_PAD_ANALOG_R1_CHANGE ) ;

	PL_SubjectToggle = 0 ;
#ifdef KP_XBOX
	GV_SetChangePressure( PAD_PRESS_Y , 1 ) ;
#endif
}

void PL_SetPadTypeSubjectMove( void )
{
   extern int  PL_SubjectMove ;

   printf( "raiden/pl_pad.c : -----------------PL_SubjectMove=%d--------------\n", PL_SubjectMove ) ;

   switch( PL_SubjectMove ) {
    case 1:
       PL_PadSetPatternSubjectMoveA() ;
       break ;

    case 2:
       PL_PadSetPatternSubjectMoveB() ;
       break ;

    case 3:
       PL_PadSetPatternSubjectMoveC() ;
       break ;

    default:
       ;
   }
}

void PL_SetPadType( int set ) /* set = 0 or 1*/
{
	extern int PL_PadType     ; /* パッドタイプ       */
	extern int PL_PadTypePrev ; /* パッドタイプ       */
    extern int PL_SubjectMove ;

	if ( PL_SubjectMove ) {
		PL_PadTypePrev = -1 ; 
		return ;
	}

	set = set ? PL_PadType : 3 ;
	if ( PL_PadTypePrev != set ){ /* ﾁｰ､ﾎ､ﾈｰ网ﾃ､ﾆ､､､ｿ､鯀ｷ､ｷ､ｯ･ｻ･ﾃ･ﾈ､ｹ､・*/   
		PL_PadTypePrev = set ;   
	} else {   
		return ;   
	}   
  
	printf( "raiden/pl_pad.c : Set PADTYPE -- PL_PadType%d Config%d--------------\n", set,  GM_GET_CONFIG_CONTROLS() ) ;

    /* PAD */
    PL_PadSetPatternA() ;
}



int PL_COM_SetPadType( void )
{
	extern int PL_PadType ; /* パッドタイプ       */

    /* PAD */
#ifdef KP_XBOX
	//PL_PadType = GCL_GetOptionValue( 'P', GM_GET_CONFIG_CONTROLS() ) ;

#ifndef KP_WINDOWS
	PL_PadType = GM_GET_CONFIG_CONTROLS() ;
#else
	PL_PadType = 0;
#endif

	PL_SetPadType( 1 ) ;
#else
	PL_PadType = 0 ;
#endif

    return 1 ; 
}

