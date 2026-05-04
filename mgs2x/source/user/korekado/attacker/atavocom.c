//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atavocom.c
	敵兵用、汎用思考ルーチン

	1999/07/07 Y.Korekado
	$Id: atavocom.c,v 1.1.1.3 2002/11/19 11:43:58 Yoshizawa1 Exp $
	
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
#include	"../enemy/enemy.h"

#include	"libutl.h"

/* 仮 */
CLEARING	*Cle ;

/*-------------------------------------------------------------------------*/
int	COM_ClearingCancel( void )
{
	SET_FLAG( Cle->iknow_flag, CLE_IKNOW_CANCEL ) ;

	return 1 ;
}
int	COM_ClearingGoNormal( void )
{
printf(" claerig commander set go normal \n");
	SET_FLAG( Cle->iknow_flag, CLE_IKNOW_GO_NORMAL ) ;
	COM_SetAvoidTime( AVOID_TIME/2 ) ;

	return 1 ;
}
/*-------------------------------------------------------------------------*/
static int ENE_EnemySceneCheck( unit ,scene )
E_UNIT	*unit ;
int scene;
{
	int	 i ;
	
	for( i=0; i<unit->enemy_num; i++){
		if ( unit->entk[i]->scene != scene ) {
			return 0 ;
		}
	}
	return 1 ;
}

/*-------------------------------------------------------------------------*/
static void ClearingAssemble( cle )
CLEARING	*cle ;
{
	if ( cle->unit->u_buff[ 2 ] >= PA_TIME_CLE_START ) {
		cle->unit->u_buff[ 2 ] = PA_TIME_CLE_START - 1 ;	/* GO_TIME分待つ */
		cle->cle_count3 ++ ;
	}
	if ( cle->cle_count3 > cle->clear_gotime ) {
		cle->cle_think3 = CLE_TH3_SCENE_MANAGE ;
		cle->cle_count3 = 0 ;

		return ;
	}
}

static void CleSceneManage( cle )
CLEARING	*cle ;
{
	if ( cle->unit->u_buff[ 2 ] >= PA_TIME_AVO_END ) {
		KR_ClearingCount() ;
		cle->cle_think3 = CLE_TH3_COMP ;
		cle->cle_count3 = 0 ;
		COM_CleProc( cle->clear_area, -2, -2 ) ;	/* エリア別終了プロック */
		COM_CleProc( cle->clear_area, -3, -3 ) ;	/* クリアリング正常終了プロック */

		return ;
	}
	cle->cle_count3 ++ ;
}

static void ClearingGoSign( cle )
CLEARING	*cle ;
{
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_GOSIGN ) ) {
		cle->cle_think3 = CLE_TH3_DYNAMICENTRY ;
		cle->cle_count3 = 0 ;

		return ;
	}
	cle->cle_count3 ++ ;
}

static void ClearingDynamicEntry( cle )
CLEARING	*cle ;
{
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_DENTRY ) ) {
		cle->cle_think3 = CLE_TH3_D_TO_S ;
		cle->cle_count3 = 0 ;

		return ;
	}
	cle->cle_count3 ++ ;
}

static void ClearingD2S( cle )
CLEARING	*cle ;
{
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_D2S ) ) {
		cle->cle_think3 = CLE_TH3_2CLEARING ;
		cle->cle_count3 = 0 ;

		return ;
	}
#if 0
	if ( 物音がした ){
		GM_ClearingMode = CLEARING_MODE_NOISE ;
	}
	if ( 血痕を発見 ){
		GM_ClearingMode = CLEARING_MODE_BLOOD ;
	}
#endif
	cle->cle_count3 ++ ;
}

static void SecondClearing( cle )
CLEARING	*cle ;
{
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_CLEARING ) ) {
		cle->cle_think3 = CLE_TH3_COMP ;
		cle->cle_count3 = 0 ;
		COM_CleProc( cle->clear_area, -2, -2 ) ;	/* エリア別終了プロック */

		return ;
	}
#if 0
	if ( 物音がした ){
		GM_ClearingMode = CLEARING_MODE_NOISE ;
	}
	if ( 血痕を発見 ){
		GM_ClearingMode = CLEARING_MODE_BLOOD ;
	}
#endif
	cle->cle_count3 ++ ;
}

static void ClearingComplete( cle )
CLEARING	*cle ;
{
/*
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_COMPLETE ) ) {
		cle->cle_think1 = CLE_TH1_NONE ;
		cle->cle_count3 = 0 ;

		return ;
	}
*/
#if 0
	if ( 物音がした ){
		GM_ClearingMode = CLEARING_MODE_NOISE ;
	}
	if ( 血痕を発見 ){
		GM_ClearingMode = CLEARING_MODE_BLOOD ;
	}
#endif
	cle->cle_count3 ++ ;
}

static void LastPointAssemble( cle )
CLEARING	*cle ;
{
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_START ) ) {
		cle->cle_think3 = CLE_TH3_GOSIGN ;
		cle->cle_count3 = 0 ;

		return ;
	}
	cle->cle_count3 ++ ;
}

static void SearchGoSign( cle )
CLEARING	*cle ;
{
/*
	if ( ENE_EnemySceneCheck(  cle->unit, CLE_ENE_WAIT_COMPLETE ) ) {
		cle->cle_think1 = CLE_TH1_NONE ;
		cle->cle_count3 = 0 ;

		return ;
	}
*/
	cle->cle_count3 ++ ;
}




/*-------------------------------------------------------------------------*/
static void CleTh2Clearing( cle )
CLEARING	*cle ;
{
	switch( cle->cle_think3 ){
		case	CLE_TH3_ASSEMBLE :		/* 集合モード */
			ClearingAssemble( cle ) ;
		break ;
		case	CLE_TH3_SCENE_MANAGE :		/* 進行管理 */
			CleSceneManage( cle ) ;
		break ;


		case	CLE_TH3_GOSIGN :		/* 突撃合図 */
			ClearingGoSign( cle ) ;
		break ;
		case	CLE_TH3_DYNAMICENTRY :	/* ダイナミックエントリーモード */
			ClearingDynamicEntry( cle ) ;
		break ;
		case	CLE_TH3_D_TO_S :		/* ダイナミック～ステルス */
			ClearingD2S( cle ) ;
		break ;
		case	CLE_TH3_2CLEARING :		/* セカンドクリアリング */
			SecondClearing( cle ) ;
		break ;
		case	CLE_TH3_COMP :			/* クリアリング完了 */
			ClearingComplete( cle ) ;
		break ;
#if 0
		case	CLEARING_MODE_NOISE :			/* 物音モード */
		break ;
		case	CLEARING_MODE_DAMAGE :			/* ダメージモード */
		break ;
		case	CLEARING_MODE_BLOOD :			/* 血発見モード */
		break ;
		case	CLEARING_MODE_WITHDROW :		/* 撤収 */
		break ;
#endif
	}

}

static void CleTh2Search( cle )
CLEARING	*cle ;
{
	switch( cle->cle_think3 ){
		case	CLE_TH3_ASSEMBLE :		/* 集合モード */
			LastPointAssemble( cle ) ;
		break ;
		case	CLE_TH3_GOSIGN :		/* 探索開始 */
			SearchGoSign( cle ) ;
		break ;
	}
}

/*-------------------------------------------------------------------------*/
static void ClearingModeCheck( cle )
CLEARING	*cle ;
{
	if ( cle->iknow_flag & CLE_IKNOW_GO_NORMAL ) {
		extern void TS_SubWindowSleep( void );

		cle->cle_think2 = CLE_TH2_SEARCH ;
		cle->cle_think3 = CLE_TH3_ASSEMBLE ;
		cle->cle_count3 = 0 ;
		cle->clear_gotime = 0 ;
		UNSET_FLAG( GM_GameStatus, STATE_CLEARING ) ;
		TS_SubWindowSleep();	/* 子画面出ていたら消去 */

printf(" clearing -> normal avoid !!\n");

		COM_CleProc( cle->clear_area, -2, -2 ) ;	/* エリア別終了プロック */
	}
}
/*-------------------------------------------------------------------------*/
static int GetClearingScene( unit )
E_UNIT	*unit ;
{
	int	 i, min ;
	
	min = 1000000;
	for( i=0; i<unit->enemy_num; i++){
		if ( min > unit->entk[i]->scene  ) {
			min = unit->entk[i]->scene ;
		}
	}
	
	return min ;
}

static void ClearingThinkAvoid( cle )
CLEARING	*cle ;
{
	switch( cle->cle_think2 ){
		case	CLE_TH2_CLEARING :
			CleTh2Clearing( cle ) ;
			ClearingModeCheck( cle ) ;
		break ;
		case	CLE_TH2_SEARCH :
			CleTh2Search( cle ) ;
		break ;
	}
}

static	void StatusCheck( cle )
CLEARING	*cle ;
{
	E_UNIT	*unit ;
	int	 i, notice, ns, old_notice ;

	ns = notice = 0 ;
	unit = cle->unit ;
	old_notice = cle->c_notice ;
	for( i=0; i< unit->enemy_num; i++){
		if ( unit->entk[i]->c_notice & CLE_NOTICE_SUPPORT ) {
			if ( !(cle->c_notice) ) {
				cle->ene_num = i ;
				cle->c_notice = unit->entk[i]->c_notice ;
				cle->trg_pos = unit->entk[i]->trgpoint.pos ;
				if ( cle->c_notice & ENE_NOTICE_NOISE ) cle->noise_num ++ ;
			}
			notice = 1;
		} 
	}
	if ( !notice ) {
		cle->c_notice = 0 ;
	}
//printf( " Cle Notice [%x]\n ", cle->c_notice ) ;
}

void COM_AttackerAvoid( cle )
CLEARING	*cle ;
{
	cle->unit->u_buff[ 1 ] = cle->cle_think2 ;
	cle->unit->u_buff[ 2 ] = GetClearingScene( cle->unit ) ;
	StatusCheck( cle ) ;
	ClearingThinkAvoid( cle ) ;

	cle->flam_flag = 0 ;
}

/*-------------------------------------------------------------------------*/
static int AnyoneNotStatus_U( E_UNIT *unit, int status )
{
	int j ;

	for ( j=0; j<unit->enemy_num; j++ ) {
		if ( unit->entk[ j ] == NULL ) continue ;
		if ( !(unit->entk[ j ]->act->status & status) ) {
			return 1 ;
		}
	}
printf("Clearing Member all faint \n");
	return 0 ;
}

void	COM_AttackerStartModeAvoid( cle )
CLEARING	*cle ;
{
	int		area, i ;
	E_UNIT *unit ;

	/* 仮 */
	Cle = cle ;

	cle->cle_think1 = CLE_TH1_AVOID ;
	cle->cle_count3 = 0 ;
	cle->clear_area = -1 ;	/* カレントクリアリングエリア */
	cle->clear_gotime = 0 ;	/* 突入カウント */
	COM_InitTmpCleProc( ) ;
	CLEAR_FLAG ( cle->iknow_flag ) ;

	cle->c_notice = 0 ;
	cle->scene_flag = CLE_ENE_MOVE_START ;
	cle->noise_num = 0 ;

	COM_CleProc( -1, -1, -1 ) ;	/* クリアリングスタートプロック */

printf("atavocom: cle->iknow_flag[%x]\n",cle->iknow_flag);

	if ( ((area = ENE_CLBoundCheck( &COM_GetCommander()->player_lastpos )) >= 0) &&
			(AnyoneNotStatus_U( cle->unit, ACT_STATUS_FAINT)) &&
			!( cle->iknow_flag & CLE_IKNOW_GO_NORMAL ) ) {
		HZX_CLE_AREA *cle_area ;
printf("atavocom: Clearung\n");

		COM_CleProc( area, -1, -1 ) ;	/* エリア別スタートプロック */

		cle->clear_area = area ;
		cle->cle_think2 = CLE_TH2_CLEARING ;
		cle->cle_think3 = CLE_TH3_ASSEMBLE ;
		cle_area = ENE_GetClearArea( area ) ;
		cle->clear_gotime = cle_area->go_time ;
		cle->cle_count3 = 0 ;

		unit = cle->unit ;
		for( i=0; i< unit->enemy_num; i++){
			unit->entk[i]->scene =  0;
		}
		SET_FLAG( GM_GameStatus, STATE_CLEARING ) ;

	} else {
printf("atavocom: Normal avoid\n");
		cle->cle_think2 = CLE_TH2_SEARCH ;
		cle->cle_think3 = CLE_TH3_ASSEMBLE ;
		cle->cle_count3 = 0 ;
		cle->clear_gotime = 0 ;
	}
	cle->unit->u_buff[ 0 ] = cle->clear_area ;
	cle->unit->u_buff[ 1 ] = cle->cle_think2 ;
//	cle->unit->u_buff[ 2 ] = cle->cle_think3 ;

	cle->unit->u_buff[ 2 ] = 0 ;

}


