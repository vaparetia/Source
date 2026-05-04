//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dbview.c
	デバッグ情報
	
	2000/01/11 Y.Korekado
	$Id: dbview.c,v 1.1.1.3 2002/11/19 11:44:07 Yoshizawa1 Exp $
*/
 
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"enemy.h"

/*----------------------------------------------------------------*/
#include	"debugmenu.h"

static	int	debug_flag = 0 ;
static	u_int	db_count = 0 ;

#define ENE_DB_THINK_VALUE	0x00000001
#define ENE_DB_GV_TIME		0x00000002

/* 視界コントロール */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU eye_ctrl = {
 class:		"ENEMY",
 menu:		"EYE CONTROL",
 max:		2,
 items:		( char *[] ){ "OPEN", "CLOSE" },
 values:	( int [] ){ 0, CMST_ENEMY_SIGHT_OFF },
 target: 	NULL,
 mask:		CMST_ENEMY_SIGHT_OFF ,
} ;
#else
static char *eye_ctrl_item[] = { "OPEN", "CLOSE" } ;
static int eye_ctrl_value[] = { 0, CMST_ENEMY_SIGHT_OFF } ;

static GM_DEBUG_MENU eye_ctrl = {
	NULL, /* next */
	"ENEMY", /* class */
	"EYE CONTROL", /* menu */
	eye_ctrl_item,/*items*/
	eye_ctrl_value, /* values */
 	NULL,/*target*/
	CMST_ENEMY_SIGHT_OFF,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif

/* 体 光らす */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU body_light = {
 class:		"ENEMY",
 menu:		"BODY LIGHT",
 max:		2,
 items:		( char *[] ){ "OFF", "ON" },
 values:	( int [] ){ 0,CMST_ENEMY_BODY_LIGHT },
 target: 	NULL,
 mask:		CMST_ENEMY_BODY_LIGHT ,
} ;
#else
static char *body_light_item[] = { "OFF", "ON" } ;
static int body_light_value[] = { 0, CMST_ENEMY_BODY_LIGHT } ;

static GM_DEBUG_MENU body_light = {
	NULL, /* next */
	"ENEMY", /* class */
	"BODY LIGHT", /* menu */
	body_light_item,/*items*/
	body_light_value, /* values */
	NULL,/*target*/
	CMST_ENEMY_BODY_LIGHT,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif

/* 視界表示 */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU eye_view = {
 class:		"ENEMY",
 menu:		"EYE VIEW",
 max:		2,
 items:		( char *[] ){ "OFF", "ON" },
 values:	( int [] ){ 0,CMST_ENEMY_SIGHT_VIEW },
 target: 	NULL,
 mask:		CMST_ENEMY_SIGHT_VIEW ,
} ;
#else
static char *eye_view_item[] = { "OFF", "ON" } ;
static int eye_view_value[] = { 0, CMST_ENEMY_SIGHT_VIEW } ;

static GM_DEBUG_MENU eye_view = {
	NULL, /* next */
	"ENEMY", /* class */
	"EYE VIEW", /* menu */
	eye_view_item,/*items*/
	eye_view_value, /* values */
	NULL,/*target*/
	CMST_ENEMY_SIGHT_VIEW,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif

/* 視界表示 */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU think_value = {
 class:		"ENEMY",
 menu:		"THINK VALUE",
 max:		2,
 items:		( char *[] ){ "OFF", "PRINT" },
 values:	( int [] ){ 0,ENE_DB_THINK_VALUE },
 target: 	&debug_flag,
 mask:		ENE_DB_THINK_VALUE ,
} ;
#else
static char *think_value_item[] = { "OFF", "PRINT" } ;
static int think_value_value[] = { 0, ENE_DB_THINK_VALUE } ;

static GM_DEBUG_MENU think_value = {
	NULL, /* next */
	"ENEMY", /* class */
	"THINK VALUE", /* menu */
	think_value_item,/*items*/
	think_value_value, /* values */
	&debug_flag,/*target*/
	ENE_DB_THINK_VALUE,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif

/* タイマー */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU gvtime_print = {
 class:		"ENEMY",
 menu:		"DB TIME",
 max:		2,
 items:		( char *[] ){ "OFF", "PRINT" },
 values:	( int [] ){ 0,ENE_DB_GV_TIME },
 target: 	&debug_flag,
 mask:		ENE_DB_GV_TIME ,
} ;
#else
static char *gvtime_print_item[] = { "OFF", "PRINT" } ;
static int gvtime_print_value[] = { 0, ENE_DB_GV_TIME } ;

static GM_DEBUG_MENU gvtime_print = {
	NULL, /* next */
	"ENEMY", /* class */
	"DB TIME", /* menu */
	think_value_item,/*items*/
	think_value_value, /* values */
	&debug_flag,/*target*/
	ENE_DB_GV_TIME,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT		actor ;
	COMMANDER		*com ;
} Work ;

#define MODE_NUM	(11)
/*----------------------------------------------------------------*/
static void DebugViewer( com )
COMMANDER	*com ;
{
#ifdef DEBUG_MODE
	E_UNIT		*unit=NULL ;
	ENETHINK	*entk ;
	static int gp = 0 ;
	static int un = 0 ;
	static int en = 0 ;
	static int mode = 0 ;
	static int se = 438 ;
	int i, val ;
/*
	if( GV_PadData[ 1 ].press & PAD_R2 ){
		R_INTRPT	*r_intrpt ;

		r_intrpt = GM_GetRIntrpt( 0x2020, 0x2323 ) ;
		if ( r_intrpt != NULL ) {
			GM_RIntrptCallBack( r_intrpt, 100, 5, RINTRP_ORDER_OPEN ) ;
		}
	}
*/
	if ( (GM_Debug2PMode == GM_DEBUG_MODE_ENEMY_THINK) ||
		(debug_flag & ENE_DB_THINK_VALUE) ) {
/* ＳＥ */
		if( GV_PadData[ 1 ].status & PAD_R2 ){
			if( GV_PadData[ 1 ].press & PAD_U ){
				se += 4 ;
				if ( se >= 2048 ) se = 0 ;
			}
			if( GV_PadData[ 1 ].press & PAD_D ){
				se -= 4 ;
				if ( se < 0 ) se = 2047 ;
			}
			if( GV_PadData[ 1 ].press & PAD_X ){
				GM_SeSet( 0, GM_MAX_VOL, se ) ;
			}
			if( GV_PadData[ 1 ].press & PAD_Y ){
				GM_SeSet( 0, GM_MAX_VOL, se+1 ) ;
			}
			if( GV_PadData[ 1 ].press & PAD_A ){
				GM_SeSet( 0, GM_MAX_VOL, se+2 ) ;
			}
			if( GV_PadData[ 1 ].press & PAD_B ){
				GM_SeSet( 0, GM_MAX_VOL, se+3 ) ;
			}
			
			DEBUG_Locate( 40 + ( 22 ), 150, 0 );
			DEBUG_Printf( "SE = %d", se );
		}
		
/* 敵兵パラメータ */
		if ( gp < com->enemys.group_num ) {
			if( com->enemys.group_num == 0 ) return  ;
			if( com->enemys.group[ gp ]->unit_num == 0 ) return  ;
		}

		if( GV_PadData[ 1 ].press & PAD_L1 ){
			if ( gp >= com->enemys.group_num ) {
				gp = 0 ;
				un = 0 ;
				en = 0 ;
			} else {
				en += 6 ;
				if ( en >= com->enemys.group[ gp ]->unit[ un ]->enemy_num ) {
					en = 0 ;
					un ++;
					if ( un >= com->enemys.group[ gp ]->unit_num ) {
						un = 0 ;
						gp ++ ;
						if ( gp > com->enemys.group_num ) {
							gp = 0 ;
						}
					}
				}
			}
		}

		if ( gp == com->enemys.group_num ) {
			DEBUG_Locate( 40 + ( 22 ), 15, 0 );
			DEBUG_Printf( "GM_GameStatus[%x] ENE_GameStatus[%x] AlertLevel=%d AlertStatus=%x\n", 
				GM_GameStatus, ENE_GameStatus, com->alertlevel, com->alertstatus );
			DEBUG_Printf( "NoiseTime[%2d] STAGE[%x] R_OFSET[%d] C_OFSET[%d]\n", com->noise_time, com->stage_kind, GM_RouteOffset, GM_ClearingOffset );

			DEBUG_Locate( 40 + ( 72 ), 45, 0 );
			DEBUG_Printf( "ene count[%d]/[%d]\n", com->enemy_count,com->enemy_appear_max );
			DEBUG_Printf( "alert res count[%d]/[%d]\n", com->res_count_in_alert, com->max_res_in_alert );
			DEBUG_Printf( "res count[%d]/[%d]\n", com->res_count, com->max_res_num  );

DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[0] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[1] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[2] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[3] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[4] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[5] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[6] ) ;
DEBUG_Printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[7] ) ;
		} else {
			if( GV_PadData[ 1 ].press & PAD_L2 ){
				if ( ++mode > MODE_NUM ) mode = 0 ;
			}
			
			unit = com->enemys.group[ gp ]->unit[ un ] ;
			DEBUG_Locate( 40 + ( 22 ), 16, 0 );

			DEBUG_Printf( "AlertLv=%d AlertSt=%x AvoidLv=%d ComT=%d\n",
				 com->alertlevel, com->alertstatus, com->avoidlevel, com->time );
			DEBUG_Printf( "g=%2d u=%2d DIE=%2d \n", gp, un, unit->die_num );
			DEBUG_Printf( "last map=%2d pos[%f][%f][%f]\n", com->player_lastmap,
				com->player_lastpos.vx,com->player_lastpos.vy,com->player_lastpos.vz) ;
{
	extern	CLEARING	*Cle ;
	if ( Cle != NULL ) {
		DEBUG_Locate( 40 + ( 22 ), 15*23, 0 );
		DEBUG_Printf( "flag=%2d know=%2d \n", Cle->scene_flag , Cle->iknow_flag );
	}
}

			DEBUG_Locate( 40 + ( 170 ), 32, 0 );
			DEBUG_Printf( "Shoot=%3d Kill=%3d MecKill=%3d\n"
				      ,GM_ShootCount, GM_KillCount, GM_MecaKillCount );

			for ( i=0; i<unit->enemy_num; i++ ) {
				if ( (entk = unit->entk[ i ]) == NULL ) continue ;
//				if ( entk->id < 6 ) {
				if ( (entk->id >= en) && (entk->id < (en+6)) ) {
					int x ;
					
					x = entk->id%6 ;
					DEBUG_Locate( 40 + ( 72 * x), 16*4, 0 );
					DEBUG_Printf( "TH1=%2d\n", entk->think1 );
					DEBUG_Printf( "TH2=%2d\n", entk->think2 );
					DEBUG_Printf( "TH3=%2d\n", entk->think3 );
					DEBUG_Printf( "CT3=%2d\n", entk->count3 );
					switch( mode ) {
						case 0 :
							DEBUG_Printf( "LIF=%2d\n", entk->act->bodyp.life );
							DEBUG_Printf( "F=%4d\n", entk->act->bodyp.faint );
							DEBUG_Printf( "FT%4d\n", entk->act->bodyp.faint_time );
							DEBUG_Printf( "A=%4d\n", entk->act->bodyp.anesthesia );
							DEBUG_Printf( "B=%4d\n", entk->act->bodyp.blood );
							DEBUG_Printf( "!%4d\n", entk->alert );
							DEBUG_Printf( "d%2d\n", entk->act->down_s );
							DEBUG_Printf( "s%lx\n", entk->act->bodyp.stand );
							break ;
	 					case 1 :
							DEBUG_Printf( "z%4x\n", entk->ctrl->addr );
							DEBUG_Printf( "g%4x\n", entk->znavi->going_addr );
							DEBUG_Printf( "n%4x\n", entk->znavi->next_addr );
							DEBUG_Printf( "t%4x\n", entk->znavi->this_addr );
							DEBUG_Printf( "P%d\n", GV_GetNo( GM_PlayerFindObj ) );
							if ( entk->id %2 ) {
								DEBUG_Printf( "c%8x\n", entk->ctrl->hzx_id );
							}
							break ;
						case 2 :
							DEBUG_Printf( "Y %d\n", entk->ctrl->rot.vy );
							DEBUG_Printf( "T %d\n", entk->ctrl->turn.vy );
							DEBUG_Printf( "C %d\n", entk->sense.status );
							DEBUG_Printf( "e %d\n", entk->sense.eye_s );
							DEBUG_Printf( "y %d\n", entk->sense.facedir );
							DEBUG_Printf( "x %d\n", entk->sense.facedir_x );
							DEBUG_Printf( "s %d\n", entk->pl_eyei.sight );

							if ( (entk->status2 & ENE_STATUS2_AIM) ){
								DEBUG_Printf( "x%.1f\n", entk->act->aim_pos.vx );
								DEBUG_Printf( "y%.1f\n", entk->act->aim_pos.vy );
								DEBUG_Printf( "z%.1f\n", entk->act->aim_pos.vz );
							}
							break ;
						case 3 :
							DEBUG_Printf( "M%x\n", entk->ctrl->hzx_id );
							DEBUG_Printf( "g%d f%d\n", entk->ctrl->grounded, entk->ctrl->level_found );
							DEBUG_Printf( "F%4.0f\n", entk->ctrl->levels[0] );
							DEBUG_Printf( "C%4.0f\n", entk->ctrl->levels[1] );
							DEBUG_Printf( "Y%4.0f\n", entk->ctrl->mov.vy );
							DEBUG_Printf( "H%4.0f\n", entk->ctrl->height );
							DEBUG_Printf( "B%4.0f\n", entk->ctrl->hzx_base );
							DEBUG_Printf( "m%x\n", entk->ctrl->map );
							break ;
						case 4 :
							DEBUG_Printf( "UID%d\n", entk->uniq_id );
							if ( entk->rnavi != NULL ) {
								ROUTENAVI	*rnavi ;
								
								rnavi = entk->rnavi ;
								DEBUG_Printf("RT%d\n", rnavi->c_route );
								DEBUG_Printf("PT%d\n", rnavi->next_node );
								DEBUG_Printf("AC%d\n",rnavi->pa_action[(int)rnavi->next_node] );
								DEBUG_Printf("D%d\n",rnavi->p_dir );
								DEBUG_Printf("TI%d\n",rnavi->p_acttime );
								DEBUG_Printf("S%x\n",rnavi->p_actstatus );
							}
							DEBUG_Printf( "MV%d\n", entk->act->move_s );
							DEBUG_Printf( "S%d\n", entk->scene );
							break ;
						case 5 :
							DEBUG_Printf( "Defence\n");
							DEBUG_Printf( "X%5.0f\n", entk->def_pos.vx );
							DEBUG_Printf( "Y%5.0f\n", entk->def_pos.vy );
							DEBUG_Printf( "Z%5.0f\n", entk->def_pos.vz );
							DEBUG_Printf( "%x\n", entk->def_mapbit );
							DEBUG_Printf( "Target\n");
							DEBUG_Printf( "X%5.0f\n", entk->trgpoint.pos.vx );
							DEBUG_Printf( "Y%5.0f\n", entk->trgpoint.pos.vy );
							DEBUG_Printf( "Z%5.0f\n", entk->trgpoint.pos.vz );
							DEBUG_Printf( "%x\n", entk->trgpoint.map );
							break ;
						case 6 :
							DEBUG_Printf( "Pos\n");
							DEBUG_Printf( "X%5.0f\n", entk->ctrl->mov.vx );
							DEBUG_Printf( "Y%5.0f\n", entk->ctrl->mov.vy );
							DEBUG_Printf( "Z%5.0f\n", entk->ctrl->mov.vz );
							break ;
						case 7 :
							DEBUG_Printf( "Target\n");
							DEBUG_Printf( "dc%x\n", entk->act->bodyp.deftrg.class );
							DEBUG_Printf( "dd%lx\n", entk->act->bodyp.deftrg.damaged );
							DEBUG_Printf( "pc%x\n", entk->act->bodyp.pushtrg.class );
							DEBUG_Printf( "pd%lx\n", entk->act->bodyp.pushtrg.damaged );
							DEBUG_Printf( "cf%x\n", entk->act->bodyp.capture.flag );
							DEBUG_Printf( "head\n");
							DEBUG_Printf( "cl%x\n", entk->act->bodyp.def_child1[0].class );
							DEBUG_Printf( "mp%x\n", entk->act->bodyp.def_child1[0].map );
							DEBUG_Printf( "sd%x\n", entk->act->bodyp.def_child1[0].side );
							DEBUG_Printf( "dm%x\n", entk->act->bodyp.def_child1[0].damaged );
							DEBUG_Printf( "wp%x\n", entk->act->bodyp.def_child1[0].weapon_type );
							DEBUG_Printf( "cp%x\n", entk->act->bodyp.def_child1[0].capture );
							break ;
						case 8 :
							DEBUG_Printf( "status\n");
							val =(int)(entk->status) ;
							DEBUG_Printf( "%x\n", val );
							DEBUG_Printf( "notice\n");
							DEBUG_Printf( "%x\n", entk->notice );
							DEBUG_Printf( "ACID%d\n", entk->accident_id );
							DEBUG_Printf( "vc%x\n", entk->voice_chara );
							DEBUG_Printf( "iknow\n" );
							DEBUG_Printf( "%x\n", entk->iknow_flag );

							break ;
						case 9 :
							DEBUG_Printf( "thk_st\n");
							DEBUG_Printf( "%x\n",entk->thk_status);
							DEBUG_Printf( "actst\n");
							DEBUG_Printf( "%lx\n",entk->act->status);
							DEBUG_Printf( "actoldst\n");
							DEBUG_Printf( "%lx\n",entk->act->old_status);
							DEBUG_Printf( "actstst\n");
							DEBUG_Printf( "%x\n",entk->act->status_status);
							DEBUG_Printf( "rnavi2\n");
							if ( entk->rnavi2 != NULL ) {
								DEBUG_Printf("RT%d\n", entk->rnavi2->c_route );
								DEBUG_Printf("PT%d\n", entk->rnavi2->next_node );
								DEBUG_Printf("AC%d\n",entk->rnavi2->pa_action[(int)entk->rnavi2->next_node] );
								DEBUG_Printf("TI%d\n",entk->rnavi2->p_acttime);
								DEBUG_Printf("SC%d\n",entk->scene);
								DEBUG_Printf("AV%d\n",entk->avoid);
								if ( (entk->status2 & ENE_STATUS2_AIM) ){
//									DEBUG_Printf( "x%.1f\n", entk->act->aim_pos.vx );
//									DEBUG_Printf( "y%.1f\n", entk->act->aim_pos.vy );
//									DEBUG_Printf( "z%.1f\n", entk->act->aim_pos.vz );
								}
							}
							break;
						case 10:
						    if (i == 0) {
							    int	loop;
							    for ( loop=0; loop<MAX_ACCIDENT; loop++ ) {
									DEBUG_Locate( 40 + ( 68 * loop), 16*8, 0 );
									DEBUG_Printf( "-%d-\n",loop);				
									if (!(com->accident_bit & 1<<loop)){
									    DEBUG_Printf( "-\n");
									} else {
									    DEBUG_Printf( "1\n");
									}
									DEBUG_Printf( "UID= %d\n",com->accident_uniq_id[loop]);
									DEBUG_Printf( "DLY= %d\n",com->accident_delay[loop]);
									{	
									    FVECTOR tempvec;
									    DG_TransPersOne(&tempvec, &com->accident_pos[loop]);
									    DEBUG_Locate( tempvec.vx, tempvec.vy, 0 );
									    DEBUG_Printf( "[%d:%d]",loop, com->accident_uniq_id[loop]);
									}
								}
							}
							break;
						case 11 :
							DEBUG_Printf( "safe\n");
							val = ENE_HZX_GetZone( entk->ctrl->addr )->safes[0];
							DEBUG_Printf( "%x\n", val );
							val = ENE_HZX_GetZone( entk->ctrl->addr )->safes[1];
							DEBUG_Printf( "%x\n", val );
							val = ENE_HZX_GetZone( entk->ctrl->addr )->safes[2];
							DEBUG_Printf( "%x\n", val );
							val = ENE_HZX_GetZone( entk->ctrl->addr )->safes[3];
							DEBUG_Printf( "%x\n", val );

							break ;
					}
				}
			}
		}
	}

	if ( debug_flag & ENE_DB_GV_TIME ) {
		DEBUG_Locate( 40 + ( 72 * 2) , 405, 0 );
		DEBUG_Printf( "DB Count [%d]", GM_PlayTime );
	}

//	DEBUG_Locate( 40 + ( 72 * 0) , 105, 0 );
//	DEBUG_Printf( "t%4x", GM_PlayerAddress );

//	NewZoneViewer( GM_PlayerAddress&255, 2 ,0 ) ;

//	DEBUG_Locate( 40 + ( 72 * 0) , 115, 0 );
//	DEBUG_Printf( "n%4x", entk->znavi->next_addr );
#endif

}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	DebugViewer( work->com ) ;
	db_count ++ ;
}

static	void	Die( work )
Work		*work ;
{

}

/*----------------------------------------------------------------*/
static	int	GetResources( work, com )
Work	*work ;
COMMANDER	*com ;
{
	work->com = com ;

	eye_ctrl.target = &com->status ;
	body_light.target = &com->status ;
	eye_view.target = &com->status ;

	GM_AddDebugMenu( &eye_ctrl ) ; 	
	GM_AddDebugMenu( &body_light ) ; 	
	GM_AddDebugMenu( &eye_view ) ; 	

	GM_AddDebugMenu( &think_value ) ; 	
	GM_AddDebugMenu( &gvtime_print ) ; 	
	db_count = 0 ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void *NewEnemyDebugView( com )
COMMANDER	*com ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, com ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
