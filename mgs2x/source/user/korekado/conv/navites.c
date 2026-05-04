//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	navites.c
	
	2000/5/16 Y.Korekado
	$Id: navites.c,v 1.1.1.3 2002/11/19 11:44:04 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"define.h"
#include	"debugmenu.h"

/*----------------------------------------------------------------*/
#define	BODY_NAME	MDL_M92
//#define	BODY_NAME	MDL_ABAKAN
//#define	BODY_NAME	MDL_SPS

static	int	debug_flag = 0 ;

#define DEB_SE_ON	0x00000001
#define DEB_SE_BOMB	0x00000002

/* 音モード */
#ifdef BP_PSX2_GCC
static GM_DEBUG_MENU se_mode = {
 class:		"SE_DEB",
 menu:		"SE MODE",
 max:		3,
 items:		( char *[] ){ "OFF","NORMAL", "BOMB" },
 values:	( int [] ){ 0, DEB_SE_ON, DEB_SE_ON|DEB_SE_BOMB },
 target: 	NULL,
 mask:		DEB_SE_ON|DEB_SE_BOMB ,
} ;
#else
static char *se_mode_item[] = { "OFF", "NORMAL", "BOMB" } ;
static int se_mode_value[] = { 0, DEB_SE_ON, DEB_SE_ON|DEB_SE_BOMB } ;

static GM_DEBUG_MENU se_mode = {
	NULL, /* next */
	"SE_DEB", /* class */
	"SE MODE", /* menu */
	se_mode_item,/*items*/
	se_mode_value, /* values */
	NULL,/*target*/
	DEB_SE_ON|DEB_SE_BOMB,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	3, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif


/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	
	NAVIGATE 	navi ;
	NAVITARGET	n_trg ;

	int			se ;
} Work ;


/*----------------------------------------------------------------*/
#define PRIO	0x40
/*----------------------------------------------------------------*/
static	void	Move( work )
Work		*work ;
{
	float		speed ;
	NAVIGATE 	*navi ;
	CONTROL		*ctrl ;

	ctrl = &work->control ;
	navi = &work->navi ;
	speed = 0.0F ;

#if 1
	/* 目標の位置がよく変わる場合 */
	if( GV_PadData[ 1 ].status & PAD_X ){
		GM_SetNaviTargetFromCtrl( &work->n_trg, GM_PlayerControl ) ;
		if ( !(GM_Navi( navi, &work->n_trg, 350 )) ) {
			speed = 150.0F ;
		}
	}
#else
	/* 目標の位置があんまり変わらない場合 */
	if( GV_PadData[ 1 ].press & PAD_Y ){
		GM_SetNaviTargetFromPos( &work->n_trg, &GM_PlayerPosition, ctrl->hzx_id ) ;
	}
	if( GV_PadData[ 1 ].status & PAD_X ){
		if ( !(GM_Navi( navi, &work->n_trg, 350 )) ) {
			speed = 150.0F ;
		}
	}
#endif

	ctrl->step.vx = speed * _RsinF( (int)navi->next_dir ) ;
	ctrl->step.vz = speed * _RcosF( (int)navi->next_dir ) ;

	if ( ctrl->grounded & 1 ) ctrl->step.vy = 0.0F ;
	ctrl->step.vy -= 16.0F ;

}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	if ( !(debug_flag & DEB_SE_ON) ) {
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		return ;
	} else {
		work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
	}

	ctrl = &work->control ;
	body = &work->body ;

	Move( work ) ;
	GM_ActControl( ctrl ) ;
	DG_PutObjs( body->objs );

	/* マップ変更処理 */
	if ( body->map_name != GM_CurrentMap ) {
	    body->map_name = GM_CurrentMap ;
	    GM_GroupObjs( body->objs, GM_CurrentMap ) ;
	}

	if( GV_PadData[ 1 ].status & PAD_A ){
		if ( !(GM_PlayTime%12) ){
			if ( debug_flag & DEB_SE_BOMB ) {
				GM_SeSetMode( work->se, &ctrl->mov, GM_SEMODE_BOMB ) ;
			} else {
				GM_SeSetMode( work->se, &ctrl->mov, GM_SEMODE_NORMAL ) ;
			}
		}
	}
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeControl( &work->control ) ;
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, pos, se )
Work	*work ;
FVECTOR	*pos ;
int		se ;
{
	CONTROL	*ctrl ;
	OBJECT	*body ;
	
	ctrl = &work->control ;
	body = &work->body ;
	
	/* ctrl 初期化 */
	GM_InitControl( ctrl, GV_StrCode("tes") , 0 );
	GM_ConfigControlHazard( ctrl, 500, 100, 110 ) ;
	ctrl->mov = *pos ;
    GM_ConfigControlHzxHeight( ctrl, 1000.0F, ctrl->mov.vy + 100.0F ) ;
	GM_ConfigControlMapCheck( ctrl ) ;		/* ゾーンを利用したマップ変更ON */
	ctrl->skip_flag |= CTRL_SKIP_SEG_CHECK ; /* 壁チェックなし */
//	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ; /* 壁チェックは水平方向チェック */

	/* body 初期化 */
//	GM_InitObject( body, BODY_NAME, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	GM_InitObject( body, BODY_NAME, DG_FLAG_SHADE );



	/* navi 初期化 */
	GM_SetNavi( &work->navi, ctrl ) ;

	work->se = se ;


	debug_flag = 0 ;
	se_mode.target = &debug_flag ;
//	GM_AddDebugMenu( &se_mode ) ; 	

//	DBG_ViewAllZone( ctrl->hzx_id, 200, 100, 100 ) ;

//printf("[%f][%f][%f]\n",ctrl->mov.vx, ctrl->mov.vy,ctrl->mov.vz ) ;
	return 0 ;
}

void	*NewNavites( FVECTOR	*pos, int se )
{
	Work		*work ;

	OPERATOR() ;

#ifndef DEBUG_MODE
return NULL ;
#endif

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, pos, se ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
