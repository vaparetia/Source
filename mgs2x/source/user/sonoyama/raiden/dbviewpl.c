//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   dbview.c
   デバッグ情報
   
   2000/02/03 Y.Korekado
   $Id: dbviewpl.c,v 1.1.1.3 2002/11/19 11:50:55 Yoshizawa1 Exp $
   */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"camera.h"
#include	"libutl.h"
#include	"debugmenu.h"
#include	"locker.h"
#include	"raiden.h"
#include	"rand.h"

extern PlayerWork	*GM_PlayerWork ;
#ifdef PAD_PRESSURE_CHECK
extern GM_GageSet 	PressureGage[ 12 ] ;
#endif
extern	inline	void	GM_VisibleGage( GM_GageSet * ) ;
extern	inline	void	GM_InvisibleGage( GM_GageSet * ) ;

#define	Work	PlayerWork
#define	ACTION	PL_ACTION

#include	"rai_arm.h"

#include	"pl_inline.h"

extern	SVECTOR		ArmCamRotateShift ;

#ifdef DEBUG_MODE

static char *player_view_items[] = 
{ "ON", "OFF" } ;
static int  player_view_values[] = 
{ PDMS_STATUSVIEW, PDMS_NONE } ;

static GM_DEBUG_MENU player_view = {
	NULL, /* next */
	"PLAYER", /* class */
	"STATUS VIEW", /* menu */
	player_view_items,/*items*/
	player_view_values, /* values */
&PlayerDebugMenuStatus,
PDMS_STATUSVIEW,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static char *gstatus_view_items[] = 
{ "ON", "OFF" } ;
static int  gstatus_view_values[] = 
{ PDMS_GSTATUSVIEW, PDMS_NONE } ;
static GM_DEBUG_MENU gstatus_view = {
	NULL, /* next */
	"GAME", /* class */
	"STATUS VIEW", /* menu */
	gstatus_view_items,/*items*/
	gstatus_view_values, /* values */
&PlayerDebugMenuStatus,
PDMS_GSTATUSVIEW,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

static	int	GameLevel ;

static char *glevel_items[] = 
{ "NORMAL", "VERYEASY", "EASY", "HARD", "EXTREME", "E-EXTREME" } ;
static int  glevel_values[] = 
{ GM_LEVEL_NORMAL, GM_LEVEL_VERYEASY, GM_LEVEL_EASY,
			GM_LEVEL_HARD, GM_LEVEL_EXTREME, GM_LEVEL_E_EXTREME } ;
static GM_DEBUG_MENU glevel = {
	NULL, /* next */
	"GAME", /* class */
	"GAME LEVEL", /* menu */
	glevel_items,/*items*/
	glevel_values, /* values */
&GameLevel,
0,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	6, /*max*/
	0, /*current*/
	0  /*padding */
} ;

#ifdef PAL
static	int	Lang ;

static char *glang_items[] = 
{ "ENGLISH", "FRENCH", "GERMANY", "ITALY", "SPANISH" } ;
static int  glang_values[] = 
{ GM_LANG_ENGLISH, GM_LANG_FRENCH, GM_LANG_GERMANY,
			GM_LANG_ITALY, GM_LANG_SPANISH } ;

static GM_DEBUG_MENU glang = {
	NULL, /* next */
	"GAME", /* class */
	"LANGUAGE", /* menu */
	glang_items,/*items*/
	glang_values, /* values */
&Lang,
0,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	5, /*max*/
	0, /*current*/
	0  /*padding */
} ;

#endif

static char *gresult_items[] = 
{ "ON", "OFF" } ;
static int  gresult_values[] = 
{ PDMS_RESULTVIEW, 0 } ;
static GM_DEBUG_MENU gresult = {
	NULL, /* next */
	"GAME", /* class */
	"RESULT", /* menu */
	gresult_items,/*items*/
	gresult_values, /* values */
&PlayerDebugMenuStatus,
PDMS_RESULTVIEW,
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/*----------------------------------------------------------------*/

typedef	struct	{
    GV_ACT		actor ;
    Work		*work ;
    int			visible ;
} DBVIEW ;

/*----------------------------------------------------------------*/

static	void	ViewPlayerStatus( void )
{
	long64		s ;

	s = _GM_PlayerStatus ;
	DEBUG_Printf( "STATUS:" ) ;
	if ( s & PLAYER_WATCH ) DEBUG_Printf( "W" ) ;
	if ( s & PLAYER_INTRUDE ) DEBUG_Printf( "I" ) ;
	if ( s & PLAYER_SQUAT ) DEBUG_Printf( "S" ) ;	
	if ( s & PLAYER_GROUND ) DEBUG_Printf( "G" ) ;
	if ( s & PLAYER_CAUTION ) DEBUG_Printf( "C" ) ;		
	if ( s & PLAYER_LOCKER ) DEBUG_Printf( "L" ) ;
	if ( s & PLAYER_ATTACK ) DEBUG_Printf( "A" ) ;
	if ( s & PLAYER_LOCKER ) DEBUG_Printf( "L" ) ;
	if ( s & PLAYER_DAMAGED ) DEBUG_Printf( "Dg" ) ;
	if ( s & PLAYER_DOWNED ) DEBUG_Printf( "Dn" ) ;
	if ( s & PLAYER_HOLD ) DEBUG_Printf( "H" ) ;
	
	if ( s & PLAYER_BEYOND ) DEBUG_Printf( "E" ) ;
	if ( s & PLAYER_FORCE ) DEBUG_Printf( "F" ) ;
	if ( s & PLAYER_CB_BOX ) DEBUG_Printf( "Bx" ) ;
	if ( s & PLAYER_DEAD ) DEBUG_Printf( "Dd" ) ;
	if ( s & PLAYER_LADDER ) DEBUG_Printf( "Ld" ) ;
	if ( s & PLAYER_ENEMY_PULL ) DEBUG_Printf( "P" ) ;
	if ( s & PLAYER_CB_BOX_STAND ) DEBUG_Printf( "Bs" ) ;
	if ( s & PLAYER_BLOOD_DROP ) DEBUG_Printf( "Bl" ) ;
	if ( s & PLAYER_WEAPON_DISABLE ) DEBUG_Printf( "Wp" ) ;
	if ( s & PLAYER_ITEM_DISABLE ) DEBUG_Printf( "It" ) ;
	if ( s & PLAYER_MENU_OPEN ) DEBUG_Printf( "Mo" ) ;

	DEBUG_Printf( "\n" ) ;
	DEBUG_Printf( "STATUS:" ) ;

	if ( s & PLAYER_STOP ) DEBUG_Printf( "St" ) ;
	if ( s & PLAYER_BEHIND_CAMERA_ENABLE ) DEBUG_Printf( "Bc" ) ;
	if ( s & PLAYER_WEAPON_INVISIBLE ) DEBUG_Printf( "Wv" ) ;
	if ( s & PLAYER_DEBUG ) DEBUG_Printf( "D" ) ;
	if ( s & PLAYER_WEAPON_QUICK_ONLY ) DEBUG_Printf( "Wq" ) ;
	if ( s & PLAYER_ITEM_QUICK_ONLY ) DEBUG_Printf( "Iq" ) ;
	if ( s & PLAYER_NEED_NEW_PRESS ) DEBUG_Printf( "Np" ) ;
	if ( s & PLAYER_ENEMY_HANG ) DEBUG_Printf( "H" ) ;
	if ( s & PLAYER_SIGHT_LOCKON ) DEBUG_Printf( "Lk" ) ;
	if ( s & PLAYER_SNAKE ) DEBUG_Printf( "Sn" ) ;
	if ( s & PLAYER_DARK_AREA ) DEBUG_Printf( "Dk" ) ;
	if ( s & PLAYER_ENEMY_HIDDEN ) DEBUG_Printf( "Eh" ) ;

	if ( s & PLAYER_INVINCIBLE ) DEBUG_Printf( "Iv" ) ;
	if ( s & PLAYER_PAD_OFF ) DEBUG_Printf( "Po" ) ;	
	if ( s & PLAYER_BEHIND ) DEBUG_Printf( "Bh" ) ;
	if ( s & PLAYER_MOVE ) DEBUG_Printf( "M" ) ;
	if ( s & PLAYER_EVENT_ENABLE ) DEBUG_Printf( "Ev" ) ;
	if ( s & PLAYER_IN_THE_WATER ) DEBUG_Printf( "Wr" ) ;
	if ( s & PLAYER_ON_CORPSE ) DEBUG_Printf( "Oc" ) ;
	if ( s & PLAYER_CROSS ) DEBUG_Printf( "Cr" ) ;
	if ( s & PLAYER_INVINCIBLE_SCN ) DEBUG_Printf( "Si" ) ;
	if ( s & PLAYER_INVINCIBLE_PRG ) DEBUG_Printf( "Pi" ) ;
	if ( s & PLAYER_BEHIND_ATTACK ) DEBUG_Printf( "Ba" ) ;
	if ( s & PLAYER_CB_BOX_CANCELED ) DEBUG_Printf( "Bc" ) ;
	if ( s & PLAYER_ROLLING ) DEBUG_Printf( "R" ) ;
	if ( s & PLAYER_NARROW ) DEBUG_Printf( "N" ) ;
	if ( s & PLAYER_CB_BOX_HIDDEN ) DEBUG_Printf( "Bn" ) ;
	if ( s & PLAYER_NORECOVER ) DEBUG_Printf( "Nr" ) ;

	if ( s & PLAYER_WALK ) DEBUG_Printf( "Wk" ) ;
	if ( s & PLAYER_DASH ) DEBUG_Printf( "Ds" ) ;
	if ( s & PLAYER_CBBOX_RUN ) DEBUG_Printf( "Br" ) ;
	if ( s & PLAYER_WATER_SURFACE ) DEBUG_Printf( "Ws" ) ;

	DEBUG_Printf( "\n" ) ;	
}

static	void	ViewPlayerFlag( long64 f )
{
	DEBUG_Printf( "FLAG:" ) ;	
	if ( f & FLAG_CAMERA_ON_WAIST ) DEBUG_Printf( "Cw" ) ;
	if ( f & FLAG_DONOT_CHECK_WATCH ) DEBUG_Printf( "Nw" ) ;
	if ( f & FLAG_PEEPING ) DEBUG_Printf( "Pp" ) ;
	if ( f & FLAG_SUBJECT_HORIZON_LIMIT ) DEBUG_Printf( "Hl" ) ;
	if ( f & FLAG_NO_STEP ) DEBUG_Printf( "Ns" ) ;
	if ( f & FLAG_NO_IK ) DEBUG_Printf( "Ni" ) ;
	if ( f & FLAG_FORCE ) DEBUG_Printf( "F" ) ;
	if ( f & FLAG_FORCE_END ) DEBUG_Printf( "Fe" ) ;
	if ( f & FLAG_CANNOT_CHANGE_WEAPON ) DEBUG_Printf( "Wp" ) ;
	if ( f & FLAG_CANNOT_CHANGE_ITEM ) DEBUG_Printf( "It" ) ;
	if ( f & FLAG_DONOT_EXEC_MESSAGE ) DEBUG_Printf( "Em" ) ;
	if ( f & FLAG_DONOT_RECEIVE_MESSAGE ) DEBUG_Printf( "Rm" ) ;
	if ( f & FLAG_BEHIND_PEEP ) DEBUG_Printf( "Bp" ) ;
	if ( f & FLAG_CANNOT_PEEP ) DEBUG_Printf( "Np" ) ;
	if ( f & FLAG_NO_HEADTOENEMY ) DEBUG_Printf( "He" ) ;
	if ( f & FLAG_CANNOT_PEEP_LR ) DEBUG_Printf( "Plr" ) ;
	if ( f & FLAG_CANNOT_USE_RATION ) DEBUG_Printf( "Ur" ) ;
	if ( f & FLAG_BEHIND_ATTACK ) DEBUG_Printf( "Ba" ) ;
	if ( f & FLAG_NO_MOTION_STEP_Y ) DEBUG_Printf( "My" ) ;
	if ( f & FLAG_CTRL_HEIGHT_NOT_UPDATE ) DEBUG_Printf( "Hu" ) ;
	if ( f & FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) DEBUG_Printf( "Ho" ) ;
	if ( f & FLAG_NO_GRAVITY ) DEBUG_Printf( "Ng" ) ;
	if ( f & FLAG_FINDPOS_IS_WAIST ) DEBUG_Printf( "Fw" ) ;
	if ( f & FLAG_CANNOT_ATTACK ) DEBUG_Printf( "Ca" ) ;
	if ( f & FLAG_NO_TOUCH_DAMAGE ) DEBUG_Printf( "Td" ) ;
	if ( f & FLAG_NO_MOTION_STEP_XZ ) DEBUG_Printf( "Mxz" ) ;
	if ( f & FLAG_NO_CAUTION ) DEBUG_Printf( "Nc" ) ;
	if ( f & FLAG_SUBJECT_ARM_ADJUST ) DEBUG_Printf( "Aa" ) ;
	if ( f & FLAG_RECOVER_ENABLE ) DEBUG_Printf( "Re" ) ;
	if ( f & FLAG_PEEPING_UP ) DEBUG_Printf( "Pu" ) ;
	if ( f & FLAG_CANNOT_SUBJECT_LR ) DEBUG_Printf( "Slr" ) ;

	DEBUG_Printf( "\n" ) ;
	DEBUG_Printf( "FLAG:" ) ;	

	if ( f & FLAG_CANNOT_SUBJECT_U ) DEBUG_Printf( "Su" ) ;
	if ( f & FLAG_CANNOT_SUBJECT_D ) DEBUG_Printf( "Sd" ) ;
	if ( f & FLAG_BEHIND_PEEP_R_MAX ) DEBUG_Printf( "Prm" ) ;
	if ( f & FLAG_BEHIND_PEEP_L_MAX ) DEBUG_Printf( "Plm" ) ;

	if ( f & FLAG_KNOCK_WALL ) DEBUG_Printf( "Kw" ) ;

	if ( f & FLAG_NO_WAIST_INTERP ) DEBUG_Printf( "Wi" ) ;
	if ( f & FLAG_RESET_MOTION2_AT_END ) DEBUG_Printf( "M2" ) ;
	if ( f & FLAG_MSG_SHADOW_OFF ) DEBUG_Printf( "So" ) ;
	if ( f & FLAG_RELOADING ) DEBUG_Printf( "Rl" ) ;
	if ( f & FLAG_HOMING ) DEBUG_Printf( "Hm" ) ;
	if ( f & FLAG_WEAPON_CAMERA_ON ) DEBUG_Printf( "Wc" ) ;

	DEBUG_Printf( "\n" ) ;	
}

static void DebugViewer( dbview, work )
DBVIEW	*dbview ;
Work	*work ;
{
//    if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE ) {
	if ( PlayerDebugMenuStatus & PDMS_STATUSVIEW ) {
		FVECTOR		root ;
	
		if ( GV_PadData[ 1 ].press & PAD_R1 ) {
			dbview->visible = 1 - dbview->visible ;
		}
		if ( dbview->visible == 0 ) {
#ifdef PAD_PRESSURE_CHECK
			{
				GM_GageSet	  	*pgs ;
				int			i ;
	    
				pgs = PressureGage ;
	    
				for ( i = 0; i < 12; i ++ ) {
					GM_InvisibleGage( pgs ) ;
					pgs ++ ;
				}
			}
#endif
			return ;
		}

		DEBUG_Locate( 260, 32, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "MOV : %.0f %.0f %.0f\n", work->control.mov.vx,
					work->control.mov.vy, work->control.mov.vz ) ;
		DEBUG_Printf( "ROT : %d %d %d\n", work->control.rot.vx,
					work->control.rot.vy, work->control.rot.vz ) ;
		_sceVu0AddVector( &root, &work->control.mov, &work->root_diff ) ;
		DEBUG_Printf( "ROOT : %.0f %.0f %.0f\n", root.vx,
					root.vy, root.vz ) ;
		DEBUG_Printf( "FIND : %.0f %.0f %.0f %.2f\n", GM_PlayerFindPos.vx,
					GM_PlayerFindPos.vy, GM_PlayerFindPos.vz, GM_PlayerFindPos.vw ) ;
		DEBUG_Printf( "FIND_OBJ : %x\n", GM_PlayerFindObj ) ;
//		DEBUG_Printf( "G_STATUS : %x\n", GM_GameStatus ) ;

		ViewPlayerStatus() ;
		ViewPlayerFlag( work->flag ) ;

//		DEBUG_Printf( "STATUS : %lx\n", GM_PlayerStatus ) ;
//		DEBUG_Printf( "FLAG : %lx\n", work->flag ) ;
		DEBUG_Printf( "HZX : %.1f %.0f %.1f %d\n", work->control.hzx_base,
					  work->control.hzx_height, work->control.height, work->control.r_sphere ) ;
		DEBUG_Printf( "BODY : %.1f\n", work->body.height ) ;
		DEBUG_Printf( "CTRL_FLAG : %x\n", work->control.skip_flag ) ;
		DEBUG_Printf( "LEVEL : %.2f %.2f\n", work->control.levels[ 0 ],	
					work->control.levels[ 1 ] ) ;
		DEBUG_Printf( "ADDR : %x FLAG %x\n", GM_PlayerAddress, HZX_GetZoneFromAdd( GM_PlayerAddress )->flag ) ;
		DEBUG_Printf( "MAP : %x\n", work->control.map ) ;
		DEBUG_Printf( "MOTION : %d %d %d GROT %d\n", work->motion1, work->motion2,
					work->arm_motion,
					work->g_rot ) ;
		DEBUG_Printf( "TIME : %d %d\n", PL_MotionTime( work ), PL_MotionTime2( work ) ) ;
		//	DEBUG_Printf( "CAUTION_DIR : %d\n", GM_PlayerCautionDir ) ;
#if 0
		//	DEBUG_Color( 0, 0, 255, 128 ) ;
		DEBUG_Printf( "N_TOUCHES : %d\n", work->control.n_touches ) ;
		if ( work->control.n_touches > 0 ) {
			DEBUG_Printf( "SEGS %.0f %.0f %.0f %.0f\n",
						work->control.segs[ 0 ]->p1.x,
						work->control.segs[ 0 ]->p1.y,
						work->control.segs[ 0 ]->p1.z,
						work->control.segs[ 0 ]->p1.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.segs[ 0 ]->p2.x,
						work->control.segs[ 0 ]->p2.y,
						work->control.segs[ 0 ]->p2.z,
						work->control.segs[ 0 ]->p2.h ) ;
			DEBUG_Printf( "\t%.2f %.2f\n", 
						work->control.vecs[ 0 ].vx,
						work->control.vecs[ 0 ].vz ) ;
			if ( work->control.n_touches == 2 ) {
				DEBUG_Printf( "SEGS %.0f %.0f %.0f %.0f\n",
							work->control.segs[ 1 ]->p1.x,
							work->control.segs[ 1 ]->p1.y,
							work->control.segs[ 1 ]->p1.z,
							work->control.segs[ 1 ]->p1.h ) ;
				DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
							work->control.segs[ 1 ]->p2.x,
							work->control.segs[ 1 ]->p2.y,
							work->control.segs[ 1 ]->p2.z,
							work->control.segs[ 1 ]->p2.h ) ;
			}
		}
#endif
#if 0
		if ( work->control.level_found & 1 ) {
			DEBUG_Printf( "FLR %.0f %.0f %.0f %.0f\n",
						work->control.level[ 0 ]->p1.x,
						work->control.level[ 0 ]->p1.y,
						work->control.level[ 0 ]->p1.z,
						work->control.level[ 0 ]->p1.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 0 ]->p2.x,
						work->control.level[ 0 ]->p2.y,
						work->control.level[ 0 ]->p2.z,
						work->control.level[ 0 ]->p2.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 0 ]->p3.x,
						work->control.level[ 0 ]->p3.y,
						work->control.level[ 0 ]->p3.z,
						work->control.level[ 0 ]->p3.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 0 ]->p4.x,
						work->control.level[ 0 ]->p4.y,
						work->control.level[ 0 ]->p4.z,
						work->control.level[ 0 ]->p4.h ) ;
			DEBUG_Printf( "atr %x\n", work->control.flr_atrs[ 0 ] ) ;
		}
		if ( work->control.level_found & 2 ) {
			DEBUG_Printf( "CEIL %.0f %.0f %.0f %.0f\n",
						work->control.level[ 1 ]->p1.x,
						work->control.level[ 1 ]->p1.y,
						work->control.level[ 1 ]->p1.z,
						work->control.level[ 1 ]->p1.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 1 ]->p2.x,
						work->control.level[ 1 ]->p2.y,
						work->control.level[ 1 ]->p2.z,
						work->control.level[ 1 ]->p2.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 1 ]->p3.x,
						work->control.level[ 1 ]->p3.y,
						work->control.level[ 1 ]->p3.z,
						work->control.level[ 1 ]->p3.h ) ;
			DEBUG_Printf( "\t%.0f %.0f %.0f %.0f\n",
						work->control.level[ 1 ]->p4.x,
						work->control.level[ 1 ]->p4.y,
						work->control.level[ 1 ]->p4.z,
						work->control.level[ 1 ]->p4.h ) ;
			DEBUG_Printf( "atr %x\n", work->control.flr_atrs[ 1 ] ) ;
		}
#endif
		/* プレイヤーデバッグモードの切り替え */
		if ( GV_PadData[ 1 ].press & PAD_L1 ) {
			if ( ++ GM_PlayerDebugMode == MAX_PLAYER_DEBUG_MODE ) {
				GM_PlayerDebugMode = 0 ;
			}
		}
		DEBUG_Locate( 260, 180 * 2, 0 ) ;	    
		DEBUG_Printf( "DEBUG_MODE : %d\n", GM_PlayerDebugMode ) ;

		/* 武器カメラシフト */
//		if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
		if ( ( PlayerDebugMenuStatus & PDMS_STATUSVIEW ) &&
			 GM_PlayerDebugMode == GM_PDM_WEAPON_CAMERA ) {
			extern	FVECTOR	ArmCamShift ;
			extern	float	ArmCamRotX ;

			DEBUG_Locate( 60, 160 * 2, 0 ) ;	
			DEBUG_Printf( "WEAPON CAM %.1f %.1f %.1f\n",
						ArmCamShift.vx, ArmCamShift.vy, ArmCamShift.vz ) ;
			DEBUG_Printf( "WEAPON CAM ROT %.2f\n", ArmCamRotX ) ;	    
			DEBUG_Printf( "WEAPON CAM ROT %d %d\n", 
						   ArmCamRotateShift.vx, ArmCamRotateShift.vy ) ;
		}

#ifdef PAD_PRESSURE_CHECK
		{
			GM_GageSet	  	*pgs ;
			int			i ;
	    
			pgs = PressureGage ;
	    
			for ( i = 0; i < 12; i ++ ) {
				pgs->value = work->pad->pressure[ i ] ;
				GM_VisibleGage( pgs ) ;
				pgs ++ ;
			}
		}
#endif
	
    } else {
#ifdef PAD_PRESSURE_CHECK
		{
			GM_GageSet	  	*pgs ;
			int			i ;
	    
			pgs = PressureGage ;
	    
			for ( i = 0; i < 12; i ++ ) {
				GM_InvisibleGage( pgs ) ;
				pgs ++ ;
			}
		}
#endif
    }
}

/*----------------------------------------------------------------*/

static	void	GameStatusViewer( void )
{
	int			s ;
	GV_PAD		*pad ;
	extern	int	GV_PadMask[], GV_PadPress[] ;

//    if ( GM_Debug2PMode == GM_DEBUG_MODE_GAME_STATUS ) {
	if ( PlayerDebugMenuStatus & PDMS_GSTATUSVIEW ) {
		s = GM_GameStatus | GM_GameStatusScn ;
		DEBUG_Locate( 260, 8, 0 ) ;		
		DEBUG_Printf( "---- GameStatus\n" ) ;
		if ( s & STATE_CHAFF ) DEBUG_Printf( "CHAFF\n" ) ;
		if ( s & STATE_STUN ) DEBUG_Printf( "STUN\n" ) ;		
		if ( s & STATE_VIB_PAUSE0 ) DEBUG_Printf( "VIB_PAUSE0\n" ) ;
		if ( s & STATE_PAUSE_DISABLE ) DEBUG_Printf( "PAUSE_DISABLE\n" ) ;
		if ( s & STATE_DEMO ) DEBUG_Printf( "DEMO\n" ) ;
		if ( s & STATE_PRG_DEMO ) DEBUG_Printf( "PRG_DEMO\n" ) ;
		if ( s & STATE_PAD_DEMO ) DEBUG_Printf( "PAD_DEMO\n" ) ;
		if ( s & STATE_SCN_DEMO ) DEBUG_Printf( "SCN_DEMO\n" ) ;
		if ( s & STATE_GAMEOVER ) DEBUG_Printf( "GAMEOVER\n" ) ;
		DEBUG_Printf( "PLAYER FLAG : %x\n", GM_PlayerStateFlag ) ;

		DEBUG_Printf( "---- PadStatus\n" ) ;
		pad = GV_PadData ;
		DEBUG_Printf( "%x\n", pad->flag ) ;
		if ( pad->flag & GV_PAD_RELEASE ) DEBUG_Printf( "Pad Release\n" ) ;
		if ( pad->flag & GV_PAD_MASK ) DEBUG_Printf( "Pad Mask %x\n", 
													GV_PadMask[ 0 ] | GV_PadMask[ 1 ] ) ;
		if ( pad->flag & GV_PAD_PRESS_SCN ) DEBUG_Printf( "Pad Press %x\n",
														 GV_PadPress[ 0 ] ) ;
		if ( pad->analog_input & GV_PAD_ANALOG_L_USE ) {
			DEBUG_Printf( "STICK_L : %d %d\n", 
						 GV_PadData[ 0 ].left_dx, GV_PadData[ 0 ].left_dy ) ;
		}
		if ( pad->analog_input & GV_PAD_ANALOG_R_USE ) {
			DEBUG_Printf( "STICK_R : %d %d\n", 
						 GV_PadData[ 0 ].right_dx, GV_PadData[ 0 ].right_dy ) ;
		}
		DEBUG_Printf( "---- Configuration\n" ) ;
		s = GM_Configuration ;
		if ( s & GM_CONFIG_VIBRATION_OFF ) DEBUG_Printf( "Vibration OFF\n" ) ;
		if ( s & GM_CONFIG_CAPTION_OFF ) DEBUG_Printf( "Caption OFF\n" ) ;		
		if ( s & GM_CONFIG_RADAR_OFF ) DEBUG_Printf( "Radar OFF\n" ) ;
		if ( s & GM_CONFIG_BLOOD_OFF ) DEBUG_Printf( "Blood OFF\n" ) ;
		if ( s & GM_CONFIG_CUTSCENES_LETTERBOXED ) DEBUG_Printf( "Cutscenes Letterboxed\n" ) ;
		if ( s & GM_CONFIG_STORY_TANKER ) DEBUG_Printf( "Story Tanker\n" ) ;
		DEBUG_Printf( "---- Menu Status\n" ) ;
		s = GM_MenuStatus | GM_MenuStatusScn ;
		if ( s & MENU_WEAPON_OFF ) DEBUG_Printf( "Weapon Menu Off\n" ) ;
		if ( s & MENU_ITEM_OFF ) DEBUG_Printf( "Item Menu Off\n" ) ;
		if ( s & MENU_GAGE_OFF ) DEBUG_Printf( "Gage Off\n" ) ;
		if ( s & MENU_CAPTION_OFF ) DEBUG_Printf( "Menu Caption Off\n" ) ;
		if ( s & MENU_SUBWIN_OFF ) DEBUG_Printf( "Menu Subwin Off\n" ) ;
		if ( s & MENU_SUBWIN_ON ) DEBUG_Printf( "Menu Subwin On\n" ) ;
		if ( s & MENU_WEAPON_DISABLE ) DEBUG_Printf( "Weapon Menu Disable\n" ) ;
		if ( s & MENU_ITEM_DISABLE ) DEBUG_Printf( "Item Menu Disable\n" ) ;
		if ( s & MENU_RADIO_DISABLE ) DEBUG_Printf( "Radio Disable\n" ) ;
		if ( s & MENU_STREAM_CH_0 ) DEBUG_Printf( "Stream CH0\n" ) ;
		if ( s & MENU_STREAM_CH_1 ) DEBUG_Printf( "Stream CH1\n" ) ;
	}
	if ( PlayerDebugMenuStatus & PDMS_RESULTVIEW ) {
		int		sec, min, hour ;
		int		base ;

		DEBUG_Locate( 340, 96, 0 ) ;		
		DEBUG_Color( 255, 255, 255, 24 ) ;
		DEBUG_Printf( "CLEAR  COUNT  : %d\n", GM_GameClearCount ) ;
		DEBUG_Printf( "TANKER CLEAR  : %d\n", GM_TankerClearCount ) ;
		DEBUG_Printf( "PLANT  CLEAR  : %d\n", GM_PlantClearCount ) ;
		DEBUG_Printf( "\n" ) ;
		base = 300 / TIME_BASE ;
		hour = GM_PlayTime / 3600 / base ;
		min = ( GM_PlayTime - hour * 3600 * base ) / 60 / base  ;
		sec = ( GM_PlayTime - hour * 3600 * base - min * 60 * base ) / 60 ;
		DEBUG_Printf( "CONTINUE COUNT: %d\n", GM_ContinueCount ) ;
		DEBUG_Printf( "SHOOT COUNT   : %d\n", GM_ShootCount ) ;
		DEBUG_Printf( "ALERT COUNT   : %d\n", GM_AlertCount ) ;
		DEBUG_Printf( "KILL  COUNT   : %d\n", GM_KillCount ) ;
		DEBUG_Printf( "DAMAGECOUNT   : %d\n", GM_DamageCount ) ;
		DEBUG_Printf( "MECA KILL     : %d\n", GM_MecaKillCount ) ;
		DEBUG_Printf( "RATIONCOUNT   : %d\n", GM_RationUseCount ) ;
		DEBUG_Printf( "CLEARINGSAFE  : %d\n", GM_ClearingCount ) ;
		DEBUG_Printf( "RED EXCLAM    : %d\n", GM_RedFindCount ) ;
		DEBUG_Printf( "CLEARCODEFLAG : %x\n", GM_ClearCodeFlag ) ;
		DEBUG_Locate( 320, 400, 0 ) ;		
		DEBUG_Printf( "PLAY TIME     : %d:%02d:%02d\n", hour, min, sec ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( dbview )
DBVIEW		*dbview ;
{
#if 0
	{
		FVECTOR		res ;
		DG_TransPersOne( &res, &GM_PlayerControl->mov ) ;
		DumpVec( &res ) ;
	}
#endif
#if 0
	if ( GV_PadData->press & PAD_L1 ) {
		extern void *NewItemBox_Prog( int name, int where, int splash, int model_name,
									  int shadow_name, int label_name, FVECTOR *pos,
									  float height, int spec, int id, int num, int pattern,
									  FVECTOR *force ) ;	
		FVECTOR		force ;

		force.vx = frnd() * 32.0F ;
		force.vy = frnd() * 64.0F ;
		force.vz = frnd() * 32.0F ;
		NewItemBox_Prog( 0,	GM_CurrentStageMap,	0,	GV_StrCode( "dogtag_ibox" ),
						GV_StrCode( "dogtag_ibox_sh" ), GV_StrCode( "dogtag_ibox" ),
						&GM_PlayerPosition, 50.0f, GM_ITEM_BOX, IT_CBBoxE, 25, 2, &force ) ;
	}
#endif

#ifdef DEBUG_MODE
    if ( dbview->work != NULL ) DebugViewer( dbview, dbview->work ) ;
	GM_GameLevel = GameLevel ;
#ifdef PAL
	if ( GM_Language != Lang ) {
		GM_Language = Lang ;		
		GCL_SaveLinkVar( &GM_Language, sizeof( GM_Language ) ) ;
	}
#endif
	GameStatusViewer() ;

	/* 武器アイテム補充君 */
	if ( GV_PauseLevel & GV_PAUSE_PAUSE ) {
		if ( ( GV_PadData->status & PAD_L2 ) && 
			 ( GV_PadData->status & PAD_R2 ) && 
			 ( GV_PadData->press & PAD_R1 ) ) {
			int			i, card ;
			
			for ( i = 0; i < MAX_WEAPONS; i ++ ) {
				if ( GM_WeaponNum( i ) >= 0 ) {
					GM_SetWeaponNum( i, GM_WeaponMaxNum( i ) ) ;
				}
			}
			card = GM_ItemNum( IT_Card ) ;
			for ( i = 0; i < MAX_ITEMS; i ++ ) {
				GM_SetItemNum( i, GM_ItemMaxNum( i ) ) ;
			}
			//GM_SetItemNum( IT_Card, card ) ;
			_GM_ItemsR[ IT_Card ] = card ;
		}
	}
#endif


}

static	void	Die( dbview )
DBVIEW		*dbview ;
{
    
}

/*----------------------------------------------------------------*/
static	int	GetResources( dbview, work )
DBVIEW	*dbview ;
Work	*work ;
{
    dbview->work = work ;
    dbview->visible = 1 ;
	GM_AddDebugMenu( &player_view ) ;
	GM_AddDebugMenu( &gstatus_view ) ;
	GM_AddDebugMenu( &glevel ) ;
#ifdef PAL
	GM_AddDebugMenu( &glang ) ;
	Lang = GM_Language ;
#endif
	GM_AddDebugMenu( &gresult ) ;
	GameLevel = GM_GameLevel ;
    return 0 ;
}

/*----------------------------------------------------------------*/

void *NewPlayerDebugView( ptr )
void			*ptr ;
{
    DBVIEW		*dbview ;
    Work		*work ;

    OPERATOR() ;

	work = ( Work * )ptr ;
    dbview = (DBVIEW *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
									  sizeof( DBVIEW ), 0x10 ) ;
    if ( dbview != NULL ) {
		GV_SetActor( &( dbview->actor ), Act, Die ) ;
		if ( GetResources( dbview, work ) < 0 ) {
			GV_DestroyActor( dbview ) ;
			return NULL ;
		}
    }
    return (void *)dbview ;
}

#endif

