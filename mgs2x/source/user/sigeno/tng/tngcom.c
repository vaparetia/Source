//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tngcomm.c
	イベント兵用コマンダー
	2000/03/02 K.Sigeno
	$Id: tngcom.c,v 1.1.1.3 2002/11/19 11:49:54 Yoshizawa1 Exp $
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
#include "korekado/enemy/enemy.h"
#include	"../attacker/at_thk.h"
#include	"tng.h"

/*---------------------------------------------------------------*/

typedef	struct	{
	GV_ACT		actor ;
	int			name ;
//	E_GROUP		group ;
//	E_UNIT		unit[ ATUNIT_MAX ] ;
//	CLEARING	clearing[ ATUNIT_MAX ] ;
	COMMANDER	*com ;
	
//	int			think1 ;
//	int			*teamaddr[ATUNIT_MAX][ATENEMY_MAX];
	AT_COM		at_com;
	TNG_COM 	tng_com;
	int			pos_num ;
	ENE_ARGS	end_proc ;		/* 全滅時プロック */

	int		type;			/* 右舷か左舷か */

	int			msg_delay[2];
//	FVECTOR		final_pos;
//	int			final_addr;
} Work ;


static int TNG_Destroy_num ;
static int TngKillFlag ;

extern  HZX_ZON *ENE_HZX_GetZone(int addr);
extern int ENE_GetRouteDis( FVECTOR *,FVECTOR *,int,int,u_int);

extern void Sig_DefenseModeStart( ENETHINK *entk); 

extern void DisRankCheck( AT_COM * );
extern void AT_ComRenew( AT_COM * ) ;
extern ENETHINK *GetAtRanking(ENETHINK *,int );
extern int CheckNoSearch(int);

extern void COM_AttackerStartModeAvoid( CLEARING * );
extern void COM_AttackerAvoid( CLEARING * );

extern int GetNewSerch( int ,int ) ;

extern void TngReadNode( ENETHINK *) ;



#if 0 
extern void SigZoneView( int , SVECTOR * .float ) ;
#endif


enum {
	DEF_MODE_WAIT = 0,
	DEF_MODE_ALERT ,
	DEF_MODE_GUARD 
};


#define TNG_SHOT_DELAY	DIRECT_TICK(12)		/*飛び出してから撃つまで*/
#define TNG_SHOT_TIME	DIRECT_TICK(48)		/*射撃時間*/
#define TNG_WAIT_TIME	DIRECT_TICK(90)		/*射撃終了後の隙*/

/*---------------------------------------------------------------*/

void	TNG_COM_GetResPos( tng_com ,pos, map ,num )
TNG_COM *tng_com ;
FVECTOR	*pos ;
int		*map ;
int		num ;
{
//	ASSERT(num < Comm.res_num) ;
	*pos = tng_com->tng_respos[num] ;
	*map = tng_com->tng_resmap[num] ;
	pos->vy += 1000.0f ;
}

#define		MSG_LV_CHANGE	(1)
#define		MSG_ALERT_START	(2)
#define		MSG_HANG_LV_CHANGE	(3)
#define		MSG_STOP	(4)

static void AT_ComStartAlert(Work *work){
	work->at_com.think1 = ENE_TH1_ALERT ;
	work->at_com.berserk = 0 ;
	work->at_com.rollout = 0 ;
}

static	void	TNG_COM_CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ,num;
	n_msg = GV_ReceiveMessage( work->name, &msg );


	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_LV_CHANGE :
				/*攻撃開始*/
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
				num = msg->message[ 1 ] ;
				if(work->tng_com.now_level < num){
					work->tng_com.now_level = num ;
				}
				printf("NOW TNG_COM LEVEL = %d\n",work->tng_com.now_level );
				break;
			case MSG_ALERT_START :
				/*危険開始*/
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
printf("MESG MSG_ALERT_START RECIEVED\n" );
				if(work->tng_com.phase < TNG_PH_ALART){
					work->tng_com.phase = TNG_PH_ALART ;
					work->at_com.think1 = ENE_TH1_ALERT ;
					AT_ComStartAlert(work) ;
printf("MSG_ALERT_START SET!!!!\n" );
				}else {
//printf("MSG_ALERT_START UNSET!!!!\n" );
				}
//ASSERT(0) ;
				break;
			case MSG_HANG_LV_CHANGE :
				/*首絞め担当の行動許可*/
				num = msg->message[ 1 ] ;
				work->tng_com.hang_num = num ;
				printf("NOW TNG_COM HANG_NUM = %d\n",work->tng_com.hang_num );
				break;
			case MSG_STOP :
				work->tng_com.phase = TNG_PH_END ;
				break;
		}
		msg++ ;
	}
}

#if 0
static void TNG_COM_DefenseMode( Work *work ){

	int i,j,new_num=0,live_flag=0,all_num = 0;
	E_UNIT		*unit;
	ENETHINK		*entk;
//	ENTK_TYPE_A *eve_a ;

	for (i=0;i<2;i++){
		if(work->msg_delay[i] >0) {
			work->msg_delay[i]--;
		}else if(work->msg_delay[i]==0){
			work->msg_delay[i]= -1;
		}
	}

return ;
	/*死亡判定*/
	unit = work->at_com.group.unit[ 0 ] ;
	for ( j=0; j<unit->enemy_num; j++ ) {
		entk = unit->entk[ j ] ;
	}
}


#endif


static void TNG_ComInit(Work *work){
	int i,j;
	ENETHINK *entk;
	ENTK_TENG_A *eve_a ;
	AT_THK	*at_thk;



	for(i=0;i<ATUNIT_MAX;i++){
		for(j=0;j < ATENEMY_MAX;j++){
			if((i<work->at_com.group.unit_num)
			&&(j < work->at_com.unit[i].enemy_num)){
				/****/
				entk = work->at_com.unit[i].entk[j];
				at_thk = (AT_THK *) entk->character;
				work->at_com.teamaddr[i][j] = &entk->ctrl->addr;
printf("entk->at_com = &work->at_com SET!!!!!!!\n");
				entk->at_com = &work->at_com;
printf("entk->eve_a SET!!!!!!!\n");
				eve_a = (ENTK_TENG_A *) (entk->eve_a) ;
				eve_a->tng_com = &work->tng_com;
ASSERT(entk->at_com != NULL);
				at_thk->dis_rank = entk->id;
				if(j == 0){
					entk->buddy = entk ;
				}else {
					entk->buddy = work->at_com.unit[i].entk[j-1] ;
				}
				if(work->type == 1) {
#if 0
					TngGetDefPos4(entk);
#else
					TngReadNode(entk) ;
#endif
				}

			}else {
				work->at_com.teamaddr[i][j] = NULL;
			}
		}
		work->at_com.siege[ i ] = 1;
	}
	/* 連絡用 */
	work->at_com.rollout = 0;	/* 転がりタイミング*/
	work->at_com.gosign = 0;		/* 号令情報 */

	/* 可変 */
	work->at_com.esctime = (AT_THK_RATE*3); /* 逃げ開始時間*/
	 /* 追跡距離可変  視力を標準値にとる*/
	work->at_com.chasedis 
		= 1000 ;
	work->at_com.attack_num = 4;	/*同時攻撃人数*/
/*むかで状態にしない*/
	work->at_com.level = 1;			/* 攻撃モード */
	work->at_com.alert_time	= 0 ; /* 危険モード経過時間 */
	work->at_com.shoot_delay = 0;	/*発砲ためらい時間*/
	work->at_com.siege_num = 4;

	work->at_com.pl_status_old = 0; /*プレイヤ状態バッファ*/
//	work->at_com.find_hide = 0;
	work->at_com.watch_status = 0;
//AT_COM_WATCH_LAST
	work->at_com.Pl_StayTime = 0; /**/
	work->at_com.OldPlAddr = 0;
#ifdef DEBUG_MODE
	work->at_com.debug_count = 0;
#endif
	work->at_com.esc_dis = 6000;

	if(work->tng_com.mode & TNG_COM_W41){
		CONTROL ctrl ;
		ctrl.mov = work->tng_com.avoid_pos ;
		GM_ConfigControlMapID( &ctrl ) ;
		work->at_com.player_lastaddr = HZX_GetAddress( ctrl.hzx_id, &ctrl.mov, -1 ) ;
		COM_SetPlayerLastPos( &ctrl.mov, ctrl.map ) ;
printf("GET AVOID POS!!!!!!\n") ;
	}else {
		work->at_com.player_lastaddr = GM_PlayerAddress ;
	}

	work->msg_delay[0] = -1; /*ライト破壊認識遅れ*/
	work->msg_delay[1] = -1;
	work->tng_com.now_level = 0 ;
	work->tng_com.hang_num = -1 ;
	work->tng_com.phase = TNG_PH_WAIT ;
	work->tng_com.now_hang_num = 0 ;
	work->tng_com.jump_attack = 0 ;
	work->tng_com.quiet_time = 0 ;
	work->tng_com.quiet_time2 = 0 ;
}



static void SetAvoidMode( Work *work )
{
	int i ;
	FVECTOR	l_pos ;
	int		l_map ;
	ENETHINK	*topene;

printf("SET AVOID!!\n");
printf("group unit num %d\n",work->at_com.group.unit_num);
	for ( i=0; i<work->at_com.group.unit_num; i++ ) {
printf("COM_AttackerStartModeAvoid\n");
		COM_AttackerStartModeAvoid( &work->at_com.clearing[ i ] ) ;
	}


	if(work->tng_com.mode & TNG_COM_W41){
		CONTROL ctrl ;
		ctrl.mov = work->tng_com.avoid_pos ;
		GM_ConfigControlMapID( &ctrl ) ;
		work->at_com.player_lastaddr = HZX_GetAddress( ctrl.hzx_id, &ctrl.mov, -1 ) ;
	}else {
		/*回避モード開始時点での目標アドレス待避*/
		COM_GetPlayerLastPos( &l_pos, &l_map ) ;
		work->at_com.player_lastaddr = 
			HZX_GetAddress( l_map, &l_pos, -1);
	}


	if(CheckNoSearch(work->at_com.player_lastaddr)){
		/*先頭兵のentk*/
		topene = GetAtRanking(work->at_com.unit[0].entk[0] ,0 ) ;
		work->at_com.player_lastaddr =
			GetNewSerch(work->at_com.player_lastaddr,
				topene->ctrl->addr);
	}
	work->at_com.berserk = 0 ;
}

static void Avoid( Work *work )
{
	int i,k ;
	ENETHINK *entk;
	for ( i=0; i<work->at_com.group.unit_num; i++ ) {
		COM_AttackerAvoid( &work->at_com.clearing[ i ] ) ;
	}
	/*サーチ開始時に全員の回避値OFF*/
	if(work->at_com.watch_status & AT_COM_WATCH_SEARCH)
	{
		for(k=0;k<work->at_com.group.unit_num;k++){
			for(i=0;i < work->at_com.unit[k].enemy_num;i++){
				entk = work->at_com.unit[k].entk[i];
				entk->avoid = 0 ; 
			}
		}
	}
}


static void Act( Work *work )
{
//extern void SigNearZone( int  ,int) ;
	int i,k;
	ENETHINK *entk ;
//	int test ;
	ENTK_TENG_A *tng_a ;

/*prepro*/
	work->tng_com.now_hang_num = 0 ;

/********/

//SigNearZone( GM_PlayerAddress ,1) ;



#if 0
	{
		SVECTOR	rgb;
		rgb.vx = 125 ;
		SigZoneView( work->com->player_lastaddr , &rgb ,120.0f ) ;
	}
#endif



#ifdef DEBUG_MODE
	if(work->tng_com.mode & TNG_COM_DEBUG){
		DEBUG_Locate(20,300,0) ;
		DEBUG_Printf("max[%d]res[%d]destroy[%d]retire[%d]hang[%d]deflevel[%d]quiet[%d]\n",
			work->com->max_res_in_alert,
			work->com->res_count_in_alert,
			work->tng_com.destroy,
			work->tng_com.retire_num,
//			work->tng_com.hang_num,
			work->tng_com.now_hang_num ,
			work->tng_com.now_level,
			work->tng_com.quiet_time
		) ;
		if(work->end_proc.proc == 0){
			DEBUG_Locate(20,320,0) ;
			DEBUG_Printf("proc call!!!!!!!\n");
		}
	}
#endif

	TNG_COM_CheckMessage( work ) ;

	switch( work->at_com.think1 ){
		case	ENE_TH1_SNEAK:
			if ( GM_AlertMode == ALERT_MODE_ALERT ) {
				AT_ComStartAlert(work) ;
			}
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				work->at_com.think1 = ENE_TH1_AVOID ;
				SetAvoidMode( work ) ;
			}
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
		break ;
		case	ENE_TH1_ALERT:
			/*2001.06.07 外からやってきた*/
			/*左右共通処理*/
//			TNG_COM_CheckMessage( work ) ;
			if(GM_AlertMode == ALERT_MODE_ALERT){
				work->at_com.player_lastaddr = GM_PlayerAddress ;
			}
			work->tng_com.pl_fi_dis 
				= ENE_GetRouteDis(&work->tng_com.final_pos,
			&GM_PlayerPosition,work->tng_com.final_addr,
			GM_PlayerAddress,5000000);
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ChangeRouteCourse( HZX_ROOT_COURSE2 ) ;
			}
			AT_ComRenew( &work->at_com );
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ClearRouteCourse( 0 ) ;
			}
//		tng_a->status |= EVE_TNG_DAM_WAIT ;

			if(work->com->max_res_in_alert <= work->com->res_count_in_alert){
				work->tng_com.quiet_time++ ;
			}
			for(k=0;k<work->at_com.group.unit_num;k++){
				for(i=0;i < work->at_com.unit[k].enemy_num;i++){
					entk = work->at_com.unit[k].entk[i];
					/*全員分ループする処理はここ*/
					tng_a = (ENTK_TENG_A *) entk->eve_a ;
					if(tng_a->status & EVE_TNG_HANG){
						/*首絞めモード中*/
						work->tng_com.now_hang_num++;
					}
					if(tng_a->status & EVE_TNG_ACTIVE){
						work->tng_com.quiet_time = 0 ;
					}
					/*第二の保険*/
					/*ダメージ制御部異常状態で60秒経過したらクリアプロックコール*/
					if(tng_a->status & EVE_TNG_DAM_WAIT ){
						work->tng_com.quiet_time2++ ;
					}
					/*冷却スプレーはめ対策*/
					if((PL_GetPlayerWeapon()== WP_ColdSpray)){
						work->tng_com.quiet_time2 = 0 ;
					}
				}
			}
			if(
			(work->tng_com.phase >= TNG_PH_ALART)
			&&(work->tng_com.phase < TNG_PH_DESTROY)
			){
				extern void COM_SetSecurityAlert( FVECTOR * , int );
				COM_SetSecurityAlert( &GM_PlayerPosition , GM_PlayerMap);
			}
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				work->at_com.think1 = ENE_TH1_AVOID ;
				SetAvoidMode( work ) ;
			}
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
			if(work->at_com.wait_cnt > 0) work->at_com.wait_cnt--;
		break ;
		case	ENE_TH1_AVOID:
/*回避モード中 兵士間距離チェックのみ行う*/
			DisRankCheck( &work->at_com ) ;
//			AT_LevelSet(work);
			Avoid( work ) ;
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_ALERT ) {
				work->at_com.think1 = ENE_TH1_ALERT ;
				AT_ComStartAlert(work) ;
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				work->at_com.think1 = ENE_TH1_SNEAK ;
			}
		break ;
	}

	/*回避モード以外で使わないフラグを寝かせる*/
	if(work->at_com.think1 != ENE_TH1_AVOID){ 
		work->at_com.watch_status &= ~(AT_COM_WATCH_LAST|AT_COM_WATCH_SEARCH) ;
	}

/*それ以上再発生無しで死にと行動不能の合計*/
	if(work->type == 1) {
		if(
			((work->com->max_res_in_alert <= work->com->res_count_in_alert)
			&&
			(((work->com->max_res_in_alert+work->at_com.unit[0].enemy_num )
			<=(work->tng_com.destroy ))&&(work->tng_com.quiet_time >10)))
			/*保険*/
			||(work->tng_com.quiet_time > DIRECT_TICK(RETIRE_CHECK_TIME_ABS+10))
#if 1
			||(work->tng_com.quiet_time2 > DIRECT_TICK(60*10))
#else
			||(work->tng_com.quiet_time2 > 0 )
#endif
		){

printf("enemy num[%d]\n",work->at_com.unit[0].enemy_num) ;
		printf("max[%d]res[%d]destroy[%d]retire[%d]hang[%d]deflevel[%d]\n",
			work->com->max_res_in_alert,
			work->com->res_count_in_alert,
			work->tng_com.destroy,
			work->tng_com.retire_num,
//			work->tng_com.hang_num,
			work->tng_com.now_hang_num ,
			work->tng_com.now_level
		) ;



			work->tng_com.destroy = 0 ;
/*全体警戒解除*/
			work->tng_com.phase = TNG_PH_DESTROY ;
			if(work->end_proc.proc != 0){
				if(!(GM_GameStatus & STATE_GAMEOVER)){
					ENE_DeathProc( &work->end_proc ) ;
				}
				work->end_proc.proc = 0;
			}
		}
	}


/*AFTER PROCESS*/
	work->tng_com.retire_num = 0 ;
	if(work->at_com.think1 != ENE_TH1_AVOID){ 
		work->at_com.avoid_wait_cnt = 0 ;
	}else {
		if(work->at_com.avoid_wait_cnt>0) work->at_com.avoid_wait_cnt-- ;
	}
	TNG_Destroy_num = work->tng_com.destroy + work->tng_com.retire_num ;
	if(work->tng_com.jump_attack>0) work->tng_com.jump_attack-- ;
}
/*---------------------------------------------------------------*/
static void Die( Work *work )
{

}
/*---------------------------------------------------------------*/

static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}

static int AttackerUnitCall( Work *work, char *top )
{
	int i, id;
	char *p ;

//	for( i = 0; (p=GCL_NextStr()) != NULL; i++ ){
	for( i = 0; GCL_NextStr() != NULL; i++ ){
		if ( i >= ATUNIT_MAX ) {
			printf("ATUNIT_MAX OVER!!\n");
			return (-1) ;
		}
		if ( COM_SetUnit( &work->at_com.group, &work->at_com.unit[i] ) < 0 ) {
			printf("MAX_UNIT OVER!!\n");
			return (-1) ;
		}
//		id = GCL_GetInt(p) ;
//		id = GCL_GetNextInt() ;
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
#if 0
		GCL_SetArgTop( p );
#else
		GCL_SetNextStr( p );
#endif
		work->at_com.clearing[i].unit = &work->at_com.unit[i] ;
		work->at_com.clearing[i].hzx = work->com->hzx ;
	}

	return i;
}

static int GetResources( Work *work, int where ,int name)
{
	char	*opt ;
	int i,buf[3] , t_name;
	CONTROL		ctrl ;

	work->name = name ;
	work->com = COM_GetCommander( ) ;
	work->tng_com.mode = TNG_COM_NORMAL ;
	work->at_com.com = COM_GetCommander( ) ;
//	work->at_com.com->at_com = &work->at_com ;

// TNG_SHOT_DELAY	DIRECT_TICK(12)		/*飛び出してから撃つまで*/

	if ( ( opt = GCL_GetOption( 'b' ) ) != NULL ){
		work->tng_com.shot_delay = DIRECT_TICK(GCL_GetNextInt()); 
		work->tng_com.shot_time = DIRECT_TICK(GCL_GetNextInt()); 
		work->tng_com.wait_time = DIRECT_TICK(GCL_GetNextInt()); 
	}else {
		work->tng_com.shot_delay = TNG_SHOT_DELAY ; /*登場から発砲まで*/
		work->tng_com.shot_time = TNG_SHOT_TIME ; /*発砲時間*/
		work->tng_com.wait_time = TNG_WAIT_TIME ; /*発砲後の隙*/
	}


	if ( ( opt = GCL_GetOption( 'm' ) ) != NULL ){
		work->tng_com.mode = (GCL_GetNextInt()); 
	}

	if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){
		CONTROL ctrl ;
		work->tng_com.avoid_pos.vx = (float)(GCL_GetNextInt()); 
		work->tng_com.avoid_pos.vy = (float)(GCL_GetNextInt()); 
		work->tng_com.avoid_pos.vz = (float)(GCL_GetNextInt()); 
		work->tng_com.mode |= TNG_COM_W41 ;

		ctrl.mov = work->tng_com.avoid_pos ;
		GM_ConfigControlMapID( &ctrl ) ;
		work->at_com.player_lastaddr = HZX_GetAddress( ctrl.hzx_id, &ctrl.mov, -1 ) ;
		COM_SetPlayerLastPos( &ctrl.mov, ctrl.map ) ;
		work->at_com.avoid_wait_cnt = DIRECT_TICK(GCL_GetNextInt()) ;
	}

	if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){
		t_name = GCL_GetNextInt() ;
		work->tng_com.subtrg = SearchControl( t_name ) ; 
//		ASSERT(work->tng_com.subtrg != NULL) ;
	} else {
		work->tng_com.subtrg = NULL ;
	}


	/*強制再配置トラップ*/
	if ( ( opt = GCL_GetOption( 'w' ) ) != NULL ){
		work->tng_com.res_trap = GCL_GetNextInt() ;
	} else {
		work->tng_com.res_trap = 0 ;
	}


	if ( COM_SetGroup( &work->com->enemys, &work->at_com.group ) < 0 ) return (-1) ;

	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
			if ( AttackerUnitCall( work, opt ) < 0 ) return (-1) ;
	}


//	switch(GM_StartAlertMode){
	switch(GM_AlertMode){
		case	ALERT_MODE_SNEAK : /* 潜入モード */
printf("TNG_COM LINE[%d]\n",__LINE__);
			work->at_com.think1 = ENE_TH1_SNEAK ;
			break;
		case	ALERT_MODE_ALERT : /* 危険モード */
printf("TNG_COM LINE[%d]\n",__LINE__);
			COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			work->at_com.think1 = ENE_TH1_ALERT ;
			break;
		case	ALERT_MODE_AVOID : /* 回避モード */
printf("TNG_COM LINE[%d]\n",__LINE__);
			COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			SetAvoidMode( work ) ;
			work->at_com.think1 = ENE_TH1_AVOID ;
			break;
		case	ALERT_MODE_SEARCH : /* 探索モード */
printf("TNG_COM LINE[%d]\n",__LINE__);
			work->at_com.think1 = ENE_TH1_SNEAK ;
			break;
		default :
printf("TNG_COM LINE[%d]\n",__LINE__);
			work->at_com.think1 = ENE_TH1_SNEAK ;
			break;
	}

	/*守備地点設定*/
//	work->tng_com.def_pos_num = (EVENT_DEF_POINT_MAX);
	work->tng_com.def_pos_num = 0 ;
	work->tng_com.max_level = 0 ;
	if ( ( opt = GCL_GetOption( 'd' ) ) != NULL ){
		for(i=0; i<(EVENT_DEF_POINT_MAX); i++){
			if( GCL_NextStr() != NULL ){
				work->tng_com.def_pos_num++ ;
				work->tng_com.def_route[i] = GCL_GetNextInt() ;
				if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
					work->tng_com.def_route[i] += GM_RouteOffset ; 
				}
				work->tng_com.tng_respos_num[i]  = GCL_GetNextInt() ;
				work->tng_com.active_lv[i] = (u_char) GCL_GetNextInt() ;
				if(work->tng_com.max_level < work->tng_com.active_lv[i])
					work->tng_com.max_level = work->tng_com.active_lv[i] ;
printf("res pos num %d\n",work->tng_com.tng_respos_num[i]);

#if 0
/*2000.12.20 mapset 実験*/
				ctrl.mov = work->tng_com.def_pos[i] ;
				GM_ConfigControlMapID( &ctrl ) ;
				work->tng_com.def_mapbit[i] = ctrl.map;
				work->tng_com.def_mapbit[i] = GV_GetBit( ctrl.map ) ;
#endif
			}else {
//				work->tng_com.def_pos_num = i+1;
				break;
			}

		}
	}

	/*READ TEST*/
#if 0
	for(i=0;i<EVENT_DEF_POINT_MAX;i++){
		printf("X %f\n",work->tng_com.def_pos[i].vx);
		printf("Y %f\n",work->tng_com.def_pos[i].vy);
		printf("Z %f\n",work->tng_com.def_pos[i].vz);
		printf("ACT %f\n",work->tng_com.def_pos[i].vw);
	}
#endif

//	FVECTOR		tng_respos[MAX_TNG_RES_POS] ;
	/*再発生場所リスト*/
//	work->tng_com.def_pos_num = MAX_TNG_RES_POS;
	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		for(i=0; i<(MAX_TNG_RES_POS); i++){
			if( GCL_NextStr() != NULL ){
				GCL_GetIV( GCL_NextStr(), buf ) ;
				vu0_IV0toFV( (IVECTOR *)buf, &work->tng_com.tng_respos[i] ) ;
				ctrl.mov = work->tng_com.tng_respos[i] ;
				GM_ConfigControlMapID( &ctrl ) ;
				work->tng_com.tng_resmap[i] = ctrl.map;
//				work->tng_com.tng_resmap[i] = GV_GetBit( ctrl.map ) ;
printf("TENG RES POS MAP SET %x\n",work->tng_com.tng_resmap[i]);
			}else {
//				work->tng_com.def_pos_num = i;
				break;
			}

		}
	}

	work->tng_com.guard_max 
		= GCL_GetOptionValue( 'g', KATANA_GUARD_DEF ) ;

	if ( GCL_GetOption( 'f' ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->tng_com.final_pos ) ;
	}else {
		work->tng_com.final_pos = DG_ZeroVector;
	}
	if ( GCL_GetOption( 'e' ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->tng_com.esc_pos ) ;
	}else {
		work->tng_com.esc_pos = DG_ZeroVector;
	}
	/*標準床高さ*/
	if ( GCL_GetOption( 'c' ) != NULL ){
		work->tng_com.base_floor = (float) GCL_GetNextInt() ;
	}else {
		work->tng_com.base_floor = -100000.0f ;
	}

	/*全滅時proc*/
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		work->end_proc.argc = ENE_GclGetProc( &(work->end_proc.proc),
			&work->end_proc.argv[0] ) ;
		if ( work->end_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		work->end_proc.proc = 0 ;
	}

	if ( ( opt = GCL_GetOption( 'k' ) ) != NULL ){
		work->tng_com.max_res_num = GCL_GetNextInt() ;
	}else {
		work->tng_com.max_res_num = -1 ;
	}
	work->tng_com.res_num = 0 ;


	/*右舷用と左舷用の設定*/
	work->type = GCL_GetOptionValue( 't', 0 ) ;

	TNG_ComInit(work);
	work->at_com.cov_num = 0;

	/*全滅判定*/
	work->tng_com.destroy = 0 ;
	work->tng_com.retire_num = 0 ;
	work->at_com.wait_cnt = 0 ;
	work->at_com.cov_num = 0;
	TNG_Destroy_num = 0 ;
	TngKillFlag = 0 ;
	return 0;
}
int TNG_GetDestroyNum(void){
	return TNG_Destroy_num ;
}
/*---------------------------------------------------------------*/
void *NewTngCommander( name, where )
int	name ;
int	where ;
{
	Work		*work ;
	
	OPERATOR() ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), SUBCOMM_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where ,name) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


void SetTngKillFlag(void){
	TngKillFlag = 1 ;
};
int GetTngKillFlag(void){
	return TngKillFlag ;
}
