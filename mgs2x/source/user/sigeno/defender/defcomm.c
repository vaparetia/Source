//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	defcomm.c
	イベント兵用コマンダー
	2000/03/02 K.Sigeno
	$Id: defcomm.c,v 1.1.1.3 2002/11/19 11:49:08 Yoshizawa1 Exp $
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
#include	"korekado/enemy/enemy.h"

#include	"../attacker/at_thk.h"
#include	"eve_a.h"

/*---------------------------------------------------------------*/

static int	DefKillFlag ;

typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	COMMANDER	*com ;
	
	AT_COM		at_com;
	DEF_COM 	def_com;
	int			pos_num ;
	ENE_ARGS	end_proc ;		/* 全滅時プロック */
	int		type;			/* 右舷か左舷か */
	int			msg_delay[2];
	int		status ;
} Work ;
extern  HZX_ZON *ENE_HZX_GetZone(int addr);
extern int ENE_GetRouteDis( FVECTOR *,FVECTOR *,int,int,u_int);

extern void Sig_DefenseModeStart( ENETHINK *entk); 

//extern void DisRankCheck( Work * );
//extern void FormDirCheck( Work * );
//extern void CheckPlayerHide( Work * );

extern void AT_ComRenew( AT_COM * ) ;



extern int Sig_GetRoute( HZX_GROUP_ID,int ,int);


#define LIGHT_DELAY ((AT_THK_RATE*2))



#ifdef DEBUG_MODE
//#define COM_DEBUG_PRIM
#endif

#ifdef COM_DEBUG_PRIM

#endif

/*---------------------------------------------------------------*/

static void DEF_ALL_OBSERVE(Work *work){
	int j ;
	ENETHINK *entk ;
	ENTK_TYPE_A *eve_a ;
	for(j=0;j < ATENEMY_MAX;j++){
		if(j < work->at_com.unit[0].enemy_num){
			entk = work->at_com.unit[0].entk[j];
			eve_a = (ENTK_TYPE_A *) entk->eve_a;

			entk->alert = ALERT_LEVEL_MAX ;
			eve_a->status |= EVE_A_OBSERVE;
		}
	}
	COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
}

#define		MSG_LIGHT_BREAK		(1)
#define		MSG_CHANGE_RES_POS	(2)
#define		MSG_ALL_OBSERVE		(3)

static	void	DEF_COM_CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code , num;
//printf("DEF MESG CHECK \n");
	n_msg = GV_ReceiveMessage( work->name, &msg );

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case MSG_LIGHT_BREAK :
			/*破壊されたライトのフラグを寝かせる*/
				num = msg->message[ 1 ] ;
				work->msg_delay[num] = LIGHT_DELAY ;
				break;
			case MSG_CHANGE_RES_POS :
				work->def_com.res_pos_num = msg->message[ 1 ] ;
				break ;

			case MSG_ALL_OBSERVE :
				DEF_ALL_OBSERVE(work) ;
				break ;
		}
		msg++ ;
	}
}
static void DEF_COM_DefenseMode( Work *work ){

	int i ;
	for (i=0;i<2;i++){
		if(work->msg_delay[i] >0) {
			work->msg_delay[i]--;
		}else if(work->msg_delay[i]==0){
			work->def_com.trgmode &= ~(1<<i) ;
			work->msg_delay[i]= -1;
		}
	}
	return ;
}


static void DEF_ComInit(Work *work){
	int i,j,z;
	int addr1,addr2;
	ENETHINK *entk;
	HZX_GROUP_ID	id1,id2;
	ENTK_TYPE_A *eve_a ;
	AT_THK	*at_thk;
	AT_COM	*at_com ;
	DEF_COM	*def_com ;
	CONTROL	ctrl ;

	work->def_com.defense_num = 0;
	def_com = &work->def_com ;
	at_com = &work->at_com ;

	for(i=0;i<ATUNIT_MAX;i++){
		for(j=0;j < ATENEMY_MAX;j++){
			if((i<at_com->group.unit_num)
			&&(j < at_com->unit[i].enemy_num)
			&&(j < EVENT_DEF_POINT_MAX)
			){
				/****/
				entk = at_com->unit[i].entk[j];
				at_thk = (AT_THK *) entk->character;
				eve_a = (ENTK_TYPE_A *) entk->eve_a;
				at_com->teamaddr[i][j] = &entk->ctrl->addr;
				entk->at_com = at_com;
//				entk->def_com = &work->def_com;
	((ENTK_TYPE_A *) entk->eve_a )->def_com = &work->def_com;
				at_thk->dis_rank = entk->id;
				if(work->type ==1){
					/*守備配置*/
					entk->def_pos = def_com->def_pos[j] ;
					entk->def_mapbit = def_com->def_mapbit[j] ;
					def_com->def_use[j] = entk->id ;
					eve_a->def_pos_num = j; /*守備位置番号*/
					eve_a->def_pos_req = j;
					ctrl.mov = entk->def_pos ; /*この座標が正しくないと危険*/
					GM_ConfigControlMapID( &ctrl ) ;
					entk->trgpoint.pos = entk->def_pos ;
					entk->trgpoint.map = ctrl.map ;
					entk->trgpoint.addr = ctrl.addr ;

//					ENE_SetTrgpDefense( &(entk->trgpoint), 
//					&entk->def_pos, entk->def_mapbit ) ;
//					eve_a->def_pos_num = j; /*守備位置番号*/
//					eve_a->def_pos_req = j;
					 /*次に向かう守備位置のリクエスト*/
					work->def_com.defense_num++;
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
	if( work->type ==0 ){
		work->at_com.chasedis 
			= (work->at_com.unit[0].entk[0]->sense.eye_s) ;
	}else {
		/*接近*/
		work->at_com.chasedis 
			= 3000 ;
	}
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
	work->at_com.player_lastaddr = GM_PlayerAddress ;


	work->def_com.def_mode = AT_COM_DEF_NORMAL ;
	work->def_com.pl_fi_dis = 5000000;
	work->def_com.res_pos_num = 0 ;
	work->msg_delay[0] = -1; /*ライト破壊認識遅れ*/
	work->msg_delay[1] = -1;

	work->def_com.mode_cnt = 0 ;
	work->def_com.grd_cnt = 0 ;
	work->def_com.wait_num = 0 ;

	for(i=0;i< EVENT_DEF_POINT_MAX ; i++){
		work->def_com.def_use[i] = -1;
	}
#if 1
	/* 右舷の守備位置のゾーン距離 */
	if( work->type ==0 ){
		id1 = HZX_GetHzxIDbyZone( 0, &work->def_com.final_pos, &z ) ;
		addr1 = 
			HZX_GetAddress( id1,&work->def_com.final_pos, -1 );
		work->def_com.final_addr = addr1;
		for(i=0; i<work->def_com.def_pos_num; i++){
			id2 = HZX_GetHzxIDbyZone( 0, &work->def_com.def_pos[i], &z ) ;
			addr2 = HZX_GetAddress( id2,&work->def_com.def_pos[i], -1 );
			work->def_com.def_zonedis[i] = 
			ENE_GetRouteDis(&work->def_com.final_pos,
			&work->def_com.def_pos[i],addr1,addr2, 9000000 );
printf("POINT %d DIS %d\n",i,work->def_com.def_zonedis[i]);
		}
	}
#endif
}


extern int ENE_ReadOnlinInfo(int ,int);

static void DEF_COM_LEFT(Work *work)
{
	int i,j;
	ENETHINK *entk;
	ENTK_TYPE_A *eve_a ;

	/*現在proc以外の処理がないためproc実行後なら終了*/
	if (work->end_proc.proc == 0) return ;

	for(i=0;i<work->at_com.group.unit_num;i++){
		for(j=0;j < work->at_com.unit[i].enemy_num;j++){
			entk = work->at_com.unit[i].entk[j];
			eve_a = (ENTK_TYPE_A *) entk->eve_a;
			if(
			( entk->think1 == ENE_TH1_DEFENSE)&&
			( eve_a->det_time >EVENT_SUPPORT_TIME)
			){
				if(work->end_proc.proc != 0){
					if(!(GM_GameStatus & STATE_GAMEOVER)){
						ENE_DeathProc( &work->end_proc ) ;
					}
					work->end_proc.proc = 0;
					break;
				}
			}
		}
	}
}


static void Def_ComRenew(Work *work){
//	int list[ATENEMY_MAX] ;
//	ENETHINK *entk;
//	ENTK_TYPE_A *eve_a ;

	DEF_COM_CheckMessage( work ) ;
	if(GM_AlertMode == ALERT_MODE_ALERT){
		work->at_com.player_lastaddr = GM_PlayerAddress ;
		COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
	}
	if(work->def_com.seq_cont>0){
		work->def_com.seq_cont--;
	}else{
	/*残り人数で動きの激しさを変えよう*/
		if(work->def_com.def_mode < AT_COM_DEF_WAIT){
			work->def_com.seq_cont = DEF_ATTACK_CYCLE;
		}else {
			work->def_com.seq_cont = DEF_ATTACK_CYCLE_LAST;
		}
	}
}

#if 0
	short		live_num ;			/* 生存人数 */
	short		enemy_count ;		/* 出現人数 */
	short		enemy_appear_max ;	/* 最大出現人数 */
	short		res_count_in_alert ;
	short		max_res_in_alert ;

printf("live_num %d\n",work->com->live_num);
printf("res_count_in_alert %d\n",work->com->res_count_in_alert);
printf("max_res_in_alert %d\n",work->com->max_res_in_alert);

#endif



#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern void SIG_NumPrint(FVECTOR *pos ,int disp) ;

#endif

static void Act( Work *work )
{
	int j;
	ENETHINK *entk ;
	




#ifdef DEBUG_MODE
	if(0){
		DEBUG_Locate(20,300,0) ;
		DEBUG_Printf("max[%d]res[%d]destroy[%d]retire[%d]defmode[%d]\n",
			work->com->max_res_in_alert,
			work->com->res_count_in_alert,
			work->def_com.destroy,
			work->def_com.retire_num ,
			work->def_com.def_mode 
		) ;
		if(work->end_proc.proc == 0){
			DEBUG_Locate(20,320,0) ;
			DEBUG_Printf("proc call!!!!!!!\n");
		}
	}


	if(0){
		int i ;
//		for(i=0; i<(EVENT_DEF_POINT_MAX); i++){
		for(i=0; i<(work->def_com.def_pos_num); i++){
			PosBox( &work->def_com.def_pos[i] ,250.0F ,NULL );
//			SIG_NumPrint(&work->def_com.def_pos[i] , i) ;
		}
	}

#endif


	if(work->def_com.count == 0){
		work->def_com.mode_cnt++;
		if(work->def_com.mode_cnt > DEF_MODE_CNT ){
			work->def_com.mode_cnt = 0 ;
		}
	}
	if(work->def_com.count == 0){
		if(work->def_com.grd_cnt < DEF_GRD_CNT ){
			work->def_com.grd_cnt++ ;
		}
	}

#if 1
	/*右舷の全滅前は常に危険モード*/
	if(
	(work->type == DEF_COM_TYPE_B)
	){
		GM_AlertMode = ALERT_MODE_ALERT ;
		GM_AlertLevel = ALERT_LEVEL_MAX ;
		if(
		(work->at_com.com->res_count_in_alert > (work->at_com.com->max_res_in_alert-work->at_com.unit[0].enemy_num))
		&&(work->at_com.com->res_count_in_alert != work->at_com.com->max_res_in_alert)
		){
			work->def_com.count = 0 ;
			if(work->def_com.def_mode != AT_COM_DEF_WAIT ){
				work->def_com.def_mode = AT_COM_DEF_WAIT ;
			}
		}
		
		if(
		((work->at_com.com->res_count_in_alert >= work->at_com.com->max_res_in_alert)
		||(work->def_com.wait_num >= work->at_com.unit[0].enemy_num))
		&&(work->def_com.def_mode != AT_COM_DEF_DYNAMIC )
		){
			work->def_com.def_mode = AT_COM_DEF_DYNAMIC ;
			for(j=0;j < ATENEMY_MAX;j++){
//				if((i<work->at_com.group.unit_num)
//				&&(j < work->at_com.unit[0].enemy_num)){
				if(j < work->at_com.unit[0].enemy_num){
				/****/
				entk = work->at_com.unit[0].entk[j];
				entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_DYNAMIC_SIGHT ;
				}
			}
		}
	}
#endif
	/*左右共通処理*/
	Def_ComRenew(work) ;

	switch(work->type){
		/*左舷処理*/
		case DEF_COM_TYPE_A :
			DEF_COM_LEFT(work);
		break;
		case DEF_COM_TYPE_B :
		/*右舷処理*/	
#if 0			
			switch (work->def_com.count){
				case (25*60) :
					COM_CallRouteVoice( -1 , 0, EV_TALK_ONESELF, work->at_com.unit[0].entk[0]) ;
					break ;
				case (20*60) :
					COM_CallRouteVoice( -1 , 1, EV_TALK_ONESELF, work->at_com.unit[0].entk[0]) ;
					break ;
				case (15*60) :
					COM_CallRouteVoice( -1 , 2, EV_TALK_ONESELF, work->at_com.unit[0].entk[0]) ;
					break ;
				case (10*60) :
					COM_CallRouteVoice( -1 , 3, EV_TALK_ONESELF, work->at_com.unit[0].entk[0]) ;
					break ;
			}
#else
			if((work->def_com.count == DIRECT_TICK(60*1))&&(work->def_com.trgmode)){
				COM_CallRouteVoice( -1 , 0, EV_TALK_ONESELF, work->at_com.unit[0].entk[2]) ;
			}
#endif
			if(work->def_com.count>0){
				work->def_com.count--;
			}
			/*守備モード中の処理*/
			DEF_COM_DefenseMode(work);
		break;
	}

#if 1
	if(work->type == 0) {
		work->def_com.pl_fi_dis 
			= ENE_GetRouteDis(&work->def_com.final_pos,
			&GM_PlayerPosition,work->def_com.final_addr,
			GM_PlayerAddress,120000);
	}
#endif
	if(GM_GameStatus & STATE_VR_ONLY){
		HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
	}
	AT_ComRenew( &work->at_com );
	if(GM_GameStatus & STATE_VR_ONLY){
		HZX_ClearRouteCourse( 0 ) ;
	}



	if(work->type == 1) {
		if(
			(work->com->max_res_in_alert == work->com->res_count_in_alert)
			&&
			((work->com->max_res_in_alert+work->at_com.unit[0].enemy_num )
			==(work->def_com.destroy + work->def_com.retire_num))
		){
			work->def_com.destroy = 0 ;
/*全体警戒解除*/
			if(work->end_proc.proc != 0){
				if(!(GM_GameStatus & STATE_GAMEOVER)){
					ENE_DeathProc( &work->end_proc ) ;
				}
				work->end_proc.proc = 0;
			}
		}
	}


/*AFTER PROCESS*/
	work->def_com.retire_num = 0 ;
	work->def_com.wait_num = 0 ;
//	printf("RETIRE COUNT CLEAR!\n") ;

}
/*---------------------------------------------------------------*/
static void Die( Work *work )
{

}
/*---------------------------------------------------------------*/

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
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
		GCL_SetNextStr( p );
		work->at_com.clearing[i].unit = &work->at_com.unit[i] ;
		work->at_com.clearing[i].hzx = work->at_com.com->hzx ;
	}

	return i;
}

static int GetResources( Work *work, int where ,int name)
{
	char	*opt ;
	int i,buf[3] ;
	AT_COM		*at_com ;

	at_com = &work->at_com ;
	work->name = name ;
	at_com->com = COM_GetCommander( ) ;
//	at_com->com->at_com = at_com ;
	work->com = COM_GetCommander( ) ;

	at_com->think1 = 0 ;

	if ( COM_SetGroup( &at_com->com->enemys, &at_com->group ) < 0 ) return (-1) ;

	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		if ( AttackerUnitCall(work, opt ) < 0 ) return (-1) ;
	}

	/*守備地点設定*/
	work->def_com.def_pos_num = EVENT_DEF_POINT_MAX;
	if ( ( opt = GCL_GetOption( 'd' ) ) != NULL ){
		for(i=0; i<(EVENT_DEF_POINT_MAX); i++){
			if( GCL_NextStr() != NULL ){
				work->def_com.def_pos[i].vx = (float) GCL_GetNextInt() ;
				work->def_com.def_pos[i].vy = (float) GCL_GetNextInt() ;
				work->def_com.def_pos[i].vz = (float) GCL_GetNextInt() ;
				work->def_com.def_act_num[i] = GCL_GetNextInt() ;
#if 0
/*2000.12.20 mapset 実験*/
				ctrl.mov = work->def_com.def_pos[i] ;
				GM_ConfigControlMapID( &ctrl ) ;
				work->def_com.def_mapbit[i] = ctrl.map;
				work->def_com.def_mapbit[i] = GV_GetBit( ctrl.map ) ;
#endif
			}else {
				work->def_com.def_pos_num = i;
				break;
			}

		}
	}else {
		return 0;
//		for(i=0; i<(EVENT_DEF_POINT_MAX); i++){
//			work->def_com.def_mapbit[i] = GM_CurrentMap ;
//			work->def_com.def_pos[i] = DG_ZeroVector ;
//		}
	}

	/*READ TEST*/
#if 0
	for(i=0;i<EVENT_DEF_POINT_MAX;i++){
		printf("X %f\n",work->def_com.def_pos[i].vx);
		printf("Y %f\n",work->def_com.def_pos[i].vy);
		printf("Z %f\n",work->def_com.def_pos[i].vz);
		printf("ACT %f\n",work->def_act_num[i]);
	}
#endif

//	work->def_com.def_zonedis = work->def_zonedis ;
	/*全滅時proc*/
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		work->end_proc.argc = ENE_GclGetProc( &(work->end_proc.proc),
			&work->end_proc.argv[0] ) ;
		if ( work->end_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		work->end_proc.proc = 0 ;
	}

	/*右舷用と左舷用の設定*/
	work->type = GCL_GetOptionValue( 't', 0 ) ;

	/*ライトの座標*/
	work->def_com.trgmode = 0 ;
	if ( GCL_GetOption( 'l' ) ){
		for(i=0;i<2;i++){
			if( GCL_NextStr() != NULL ){
				GCL_GetIV( GCL_NextStr(), buf ) ;
				vu0_IV0toFV( (IVECTOR *)buf, &work->def_com.trgpos2[i] ) ;
				work->def_com.trgmode |= (1<<i) ;
printf("LIGHT SET!!\n");
			}
		}
printf("LIGHT MODE %d\n",work->def_com.trgmode);

	}else {
		work->def_com.trgpos2[0] = DG_ZeroVector;
		work->def_com.trgmode = 0 ;
	}
	/*通路終端座標*/
	if ( GCL_GetOption( 'f' ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->def_com.final_pos ) ;
	}else {
		printf("DEFENDER FINAL POS NOT DEFINED!!\n");
		return (-1) ;
//		work->def_com.final_pos = DG_ZeroVector;
	}
	/*ライト破壊開始時間 count*/
	if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){
		work->def_com.count = DIRECT_TICK(GCL_GetNextInt()) ;
	}else {
		work->def_com.count = 0;
	}
	if ( GCL_GetOption( 's' ) != NULL ){
		work->status = GCL_GetNextInt() ;
	}else {
		work->status = 0 ;
	}

	if(work->status & DEF_COM_ST_RNDPOS){
		/*守備位置シャッフル*/
      extern void init_rnd( int ) ;
		FVECTOR		buf ;
		int rnd_num ,act_buf ;

		init_rnd(GV_Time);

		for(i=0; i<work->def_com.def_pos_num; i++){
			buf = work->def_com.def_pos[i] ;
			act_buf = work->def_com.def_act_num[i] ;

			rnd_num = (irnd()>>8)%work->def_com.def_pos_num ;

			work->def_com.def_pos[i] = work->def_com.def_pos[rnd_num];
			work->def_com.def_act_num[i] = work->def_com.def_act_num[rnd_num] ;

			work->def_com.def_pos[rnd_num] = buf;
			work->def_com.def_act_num[rnd_num] = act_buf ;
		}
	}

	if ( ( opt = GCL_GetOption( 'k' ) ) != NULL ){
		work->def_com.max_res_num = GCL_GetNextInt() ;
	}else {
		work->def_com.max_res_num = -1 ;
	}
	work->def_com.res_num = 0 ;


	DEF_ComInit(work);
	work->def_com.seq_cont = 0 ; 
	work->def_com.order = 0 ;
	work->def_com.use_num = 0 ;

	at_com->cov_num = 0;

	/*全滅判定*/
	work->def_com.destroy = 0 ;
	work->def_com.retire_num = 0 ;

	at_com->wait_cnt = 0 ;
	at_com->cov_num = 0;
	DefKillFlag = 0 ;
	return 0;
}

/*---------------------------------------------------------------*/
void *NewDefCommander( name, where )
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

/**
	group = com->enemys.group[ 0 ] ;
	for ( i=0; i<group->unit_num; i++ ) {
		unit = group->unit[ i ] ;
		for ( j=0; j<unit->enemy_num; j++ ) {
			entk = unit->entk[ j ] ;
			if ( !(entk->act->status & ACT_STATUS_DAMAGE) ) {
				dis = _FVecTrgDis( &entk->ctrl->mov, &GM_NoisePosition 
printf("GROUP %d\n,work->com->enemys.group_num");
printf("UNIT %d\n",work->com->enemys.group[0]->unit_num);

**/

void SetDefKillFlag(void) {
	DefKillFlag = 1;
}
int GetDefKillFlag(void) {
	return DefKillFlag ;
}
