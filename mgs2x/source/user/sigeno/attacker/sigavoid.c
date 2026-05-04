//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sigavoid.c
	攻撃兵の回避モード
	
	2000/02/04 K.Sigeno
	$Id: sigavoid.c,v 1.1.1.3 2002/11/19 11:49:04 Yoshizawa1 Exp $
*/
#if 1
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
#include "at_thk.h"
#include "sig_conv.x"



extern void ENE_Attacker_Think1_Avoid( ENETHINK * );
extern void ENE_AttackerStartModeAvoid( ENETHINK * );


extern int COM_GetUnitBuff( ENETHINK *, int ) ;

extern float SIG_VecLen3F2D( FVECTOR * , FVECTOR * ) ;


void	SIG_AttackerStartModeAvoid( ENETHINK * ) ;

/*----- 思考状態 --------------------------------------------------*/

enum {
	TH2_MOVE,TH2_CHASE,TH2_SEARCH,TH2_NOISE,TH2_TOUCH,TH2_DAMAGE, /*5*/
	TH2_DISCOVERY,TH2_DEFENSE_ZONE,TH2_NEAR_ZONE, TH2_BOX, TH2_CORPS, /*10*/
	TH2_TRACE,TH2_INDISTINCT,TH2_ENE_DAMAGE,TH2_FOUND,TH2_ACCIDENT,
	TH2_HOLDUP,TH2_UNIFORM,
} ;
enum {
	TH3_ZONE_CHASE, TH3_DIRECT_CHASE, TH3_SEARCH_STAND, /*2*/
	TH3_MOVE_SAFEAREA,TH3_WAIT,TH3_LOOK_CHECK_ZONES,	/*5*/
	TH3_LOOKAROUND, TH3_LOOKAROUND_CHECK_ZONE, TH3_MOVE_DEFENSE, /*8*/
	TH3_RELOAD,TH3_READY, TH3_BUDDYSEARCH_STAND,TH3_LOOK_NEARZONE, /*12*/
	TH3_INTR_WAIT
} ;

#define AT_AVOID_WAIT_LOOK 120
#define AT_AVOID_WAIT_STILL (AT_AVOID_WAIT_LOOK+120)
#define AT_AVOID_WAIT_LIMIT (AT_AVOID_WAIT_STILL+180)

/*-----	定数定義	-----*/

	/*アクションパッド */
enum {
	SP_NONE,
	SP_DOWNBACK,
	SP_DISCOVERY,	/* 発見ポーズ */
//	SP_READYGUN,	/* 銃構える */
	SP_CAUT_STAND,	/* 警戒立ち */
	SP_MGUN_SHOT,	/* マシンガン撃つ */
	SP_RELOAD,		/* リロード */
	SP_CLE_GOSIGN,	/* 突撃合図 */
	SP_CLE_PEEP_L,	/* 左覗き込み */
	SP_CLE_PEEP_R,	/* 右覗き込み */
	SP_CLE_PEEP_D,	/* 下覗き込み */

	/* 注！！移動変更アクションパッドはこれ以上の値にすること */
	SP_MV_WALK,		/* 歩き */
	SP_MV_RUN,		/* 走り */
	SP_MV_CAUT_WALK,/* 警戒歩き */
	SP_MV_CAUT_RUN,	/* 警戒走り */
	SP_MV_SIDE_L,	/* 左横移動 */
	SP_MV_SIDE_R,	/* 右横移動 */
	SP_MV_BACK		/* バック */
} ;

#include	"sigavoact.c"

/*-----------------------------------------------------------------------------------*/


enum {
	RDATA_CON_NONE,
	RDATA_CON_START_SIGN,
	RDATA_CON_DE_END,
	RDATA_CON_CLE_SIGN,
	RDATA_CON_END,
	RDATA_CON_END_SIGN
} ;

#endif

/*-----------------------------------------------------------------------------------*/

extern int GetNewSerch( int,int ) ;

#ifdef DEBUG_MODE
extern void SigZoneView(int,SVECTOR *,float) ; 
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

static int MapAddrSet(int addr,int map){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(map)<<16));
	return addr;
}

void SIG_AttackerReturnModeAvoid( ENETHINK *);

/*安地使用権 登録*/
static inline void SetUseZone(ENETHINK *entk,int addr)
{
	entk->at_com->usezones[entk->u_id][entk->id] = addr;
}
/*安地使用権 破棄*/
static inline void ResetUseZone(ENETHINK *entk)
{
	entk->at_com->usezones[entk->u_id][entk->id] = HZX_NO_ZONE;
}
/*他の攻撃兵が調べようとしてたら他を当たる*/
static int SearchZone( ENETHINK *entk ,int tmpaddr )
{
	int i;
	/*同じチーム内でのみぶつかり判定*/
	for(i=0;i < ATENEMY_MAX; i++){
		if(i==entk->id) continue; /*自分用はスキップ*/
		if((entk->at_com->usezones[entk->u_id][i] == tmpaddr) 
		||(tmpaddr == HZX_NO_ZONE))
			return (0);
	}
	return (1);
}


#if 0
static int COM_GetUnitBuff( entk, n )
ENETHINK	*entk ;
int			n ;
{
	E_UNIT	*unit ;
	
	unit = COM_GetUnit( entk->g_id, entk->u_id ) ;
	return unit->u_buff[ n ] ;
}
#endif

/*-----------------------------------------------------------------------------------*/

/****/
//AT_COM_WATCH_SEARCH
static void StartSearchMode(entk)
ENETHINK	*entk ;
{
	int safe_num ,i ,normaddr;
	HZX_ZON *nowzone;

//	entk->at_com->watch_status |= AT_COM_WATCH_LAST ;
	entk->at_com->watch_status |= AT_COM_WATCH_SEARCH ;
	/* 今いるゾーンの安全地帯数を調べて、確認するゾーン数とする */
	safe_num = ENE_GetSafeZoneNum( entk ) ;
//	safe_num = ENE_GetSafeZoneNumIntrpt( entk ) ;
	/*安地あり*/
	if(safe_num > 0){
		nowzone = ENE_HZX_GetZone(entk->ctrl->addr);
		for(i=0;i<safe_num;i++){
			if(SearchZone( entk ,nowzone->safes[i])) {
/*プレイヤのいる安地が引っかかったら*/
				normaddr = 
					MapAddrSet( nowzone->safes[i],entk->ctrl->addr);
				if(normaddr == GM_PlayerAddress){
					if(CheckNoSearch(normaddr)){
						continue ;
					}
				}
				/*目標設定*/
				SetUseZone(entk,nowzone->safes[i]);
				ENE_SetTrgpZone( &(entk->trgpoint), 
				HZX_Zone1(nowzone->safes[i]),entk->ctrl->hzx_id ) ;
printf("GO SAFE AREA %d\n",HZX_Zone1(nowzone->safes[i]));
				entk->think2 = TH2_SEARCH ; 
				entk->think3 = TH3_MOVE_SAFEAREA ; 
				entk->count3 = 0 ;
				return ;
			}
		}
	}
	/*調べる場所がないのでキョロキョロして終了*/
printf("GO KYORO KYOROd\n");
	entk->think2 = TH2_SEARCH ;
	entk->think3 = TH3_LOOKAROUND ;
	entk->count3 = 0 ;
	entk->scene = CLE_ENE_WAIT_START ;
	return ;
}

#if 0
static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	return addr;
}
#endif
static void Think3_LookCheckZones( entk )
ENETHINK	*entk ;
{
	int		near, diff_dir ;


	entk->act->pad = SP_CAUT_STAND ;
	/* ルートのつながっている方向を見る 最大４回 */
#if 1
	if ( !(entk->count3% (AT_THK_RATE*40)) ) {
		near = ENE_GetNearZone( entk->ctrl->addr&255, entk->count3/(AT_THK_RATE*20), entk->ctrl->hzx_id ) ;
		if ( near != HZX_NO_ZONE ) {
			entk->act->aim_dir = ENE_ZoneDir( &entk->ctrl->mov, near, entk->ctrl->hzx_id ) ;
		}
	}
#endif

#if 1
	if((entk->count3%(AT_THK_RATE*40) ) < (AT_THK_RATE*20)){
		if ( (entk->count3%(AT_THK_RATE*20) ) > 0 && (entk->count3%(AT_THK_RATE*20) ) < (AT_THK_RATE*10)  ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
		} else if ( entk->count3%(AT_THK_RATE*20) > (AT_THK_RATE*10) && entk->count3%(AT_THK_RATE*20)  < (AT_THK_RATE*20)  ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
		}
	}
#endif

	diff_dir = GV_DiffDirAbs( entk->ctrl->rot.vy, entk->act->aim_dir ) ;
	if ( diff_dir > 1024 ) 	entk->act->dir = entk->act->aim_dir ;
	entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;

	entk->count3 ++ ;
}


#define		AVOID_RANK_DIS	(2000)

/* 見失った地点まで移動 */
static void Think3_ZoneMoveLastPoint( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	int trg ,intr;
//	SVECTOR	rgb;
//	float trg_dis ;

	
	at_thk = (AT_THK *)entk->character ;

//printf("ZONE MOVE\n");
//printf("I AM ID [%d] RANK [%d] DIS [%d]\n",entk->id,at_thk->dis_rank,at_thk->dis_dif);
//	if(((entk->count3%DIRECT_TICK(60))==0)&&( at_thk->dis_dif < AVOID_RANK_DIS  )&&(at_thk->dis_rank > 0)){
	if(( at_thk->dis_dif < AVOID_RANK_DIS  )&&(at_thk->dis_rank > 0)){
		entk->act->move_s = MoveCautionWalk ;
	}else {
		entk->act->move_s = MoveRun ;
	}

	if(( at_thk->dis_dif < AVOID_RANK_DIS  )&&(at_thk->dis_rank > 0)){
		entk->act->pad = SP_CAUT_STAND ;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_INTR_WAIT ;
		entk->count3 = 0 ;
		return ;
	}

	entk->act->pad = SP_MV_RUN ;
	entk->act->tmp_time = 0 ;

	if((entk->count3&63) == 0){
		if((entk->trgpoint.addr==GM_PlayerAddress)&&(CheckNoSearch(entk->trgpoint.addr))){
			/*目標が非探査ゾーン*/
			trg = GetNewSerch(entk->trgpoint.addr,entk->ctrl->addr);
			ENE_SetTrgpZone(&entk->trgpoint, 
			HZX_Zone1(trg), GV_GetBit(HZX_ZoneMapNo(trg)) );
//			ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
printf("TRG NO SEARCH ZONE!!!\n");
		}
	}

//	SigZoneView(entk->trgpoint.addr, &rgb,100.0F) ;

	/*2000.09.12
	遮られていたらそこで終わり
	*/
//Zone
//	ENE_ZoneTrace( entk->znavi, &(entk->trgpoint),
	intr = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;

	if(intr ==1){
		if(CheckOnlyRIntrpt(entk)){
			intr = -2 ;
		}else {
		}
	}
	if(
	(intr == -2)||(entk->at_com->watch_status & AT_COM_WATCH_SEARCH)
	){
		ENE_SetTrgpZone( &(entk->trgpoint), 
			HZX_Zone1(entk->ctrl->addr),entk->ctrl->hzx_id ) ;
		entk->act->dir = entk->ctrl->turn.vy;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	/*正常到達*/
	if(intr == -1){
		entk->act->dir = entk->ctrl->turn.vy;
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}

//printf("AVOID INTR CHECK %d\n",intr);
#if 0
	if(intr == 1){
		/*遮っているもの判定*/
		if( SIG_CheckRIntrpt(entk) ){
			ENE_SetTrgpZone( &(entk->trgpoint), 
				HZX_Zone1(entk->ctrl->addr),entk->ctrl->hzx_id ) ;
			entk->act->dir = entk->ctrl->turn.vy;
			entk->think3 = TH3_DIRECT_CHASE ;
			entk->count3 = 0 ;
//("ZONE 2 DIRECT LINE [%d]\n",__LINE__);
			return ;
		}
	}
	/*侵入禁止ゾーン*/
	if(intr == -2){
		/*現在地を目標に修正*/
		ENE_SetTrgpZone( &(entk->trgpoint), 
			HZX_Zone1(entk->ctrl->addr),entk->ctrl->hzx_id ) ;
		entk->act->dir = entk->ctrl->turn.vy;
printf("ZONE 2 DIRECT FOUND ZINTR  [%d]\n",__LINE__);
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
	if(entk->at_com->watch_status & AT_COM_WATCH_SEARCH){
		ENE_SetTrgpZone( &(entk->trgpoint), 
			HZX_Zone1(entk->ctrl->addr),entk->ctrl->hzx_id ) ;
		entk->act->dir = entk->ctrl->turn.vy;
printf("ZONE 2 DIRECT LINE [%d]\n",__LINE__);
		entk->think3 = TH3_DIRECT_CHASE ;
		entk->count3 = 0 ;
		return ;
	}
#endif

#if 0
	trg_dis = SIG_VecLen3F2D( &entk->ctrl->mov, &entk->trgpoint.pos ) ;
	if ( SIG_VecLen3F2D( &entk->ctrl->mov, &entk->trgpoint.pos ) < (float)(350 + (1500*at_thk->dis_rank)) ) {
#else 
	if ( at_thk->zone_dis < (500 + 3000*at_thk->dis_rank) ) {
#endif
		entk->act->dir = entk->ctrl->turn.vy;
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0;
		return ;
	}
//	if(at_thk->dis_dif<250) return ;

	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir = entk->trgpoint.dir ;

	entk->count3 ++ ;
}
static void Think3_DirectMoveLastPoint( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

/**********
	DirectTraceは範囲以下だと
	角度セットを行わずに－１を返す
***********/

	Sig_SetTrgpInsideZone(entk) ;

	entk->act->pad = SP_MV_RUN ;
	entk->act->move_s = MoveCautionWalk ;
	entk->act->tmp_time = 0 ;

//PosBox( &(entk->trgpoint.pos) ,250.0f ,NULL );
//printf("ZONE MOVE\n");
//printf("I AM ID [%d] RANK [%d] DIS [%d]\n",entk->id,at_thk->dis_rank,at_thk->dis_dif);

#if 0
	{
		float trg_dis ;
//("DIRET CHASE \n");
		trg_dis = SIG_VecLen3F2D( &entk->ctrl->mov, &entk->trgpoint.pos ) ;
//		printf("ENE X[%f] Z[%f]\n",entk->ctrl->mov.vx,entk->ctrl->mov.vz);
//		printf("TRG X[%f] Z[%f]\n",entk->trgpoint.pos.vx,entk->trgpoint.pos.vz);
	}
#endif
//	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
	if(1){
		if(
//		(ENE_DirectTrace( &(entk->trgpoint),&(entk->ctrl->mov),(500 + 1500*entk->id) ) < 0)
//		(ENE_DirectTrace( &(entk->trgpoint),&(entk->ctrl->mov),(500 + 1500*at_thk->dis_rank) ) < 0)
		(ENE_DirectTrace( &(entk->trgpoint),&(entk->ctrl->mov),(1000 + 1500*at_thk->dis_rank) ) < 0)
		||(entk->at_com->watch_status & AT_COM_WATCH_SEARCH)
		||(GM_PlayerStatus & PLAYER_NARROW)
//		||(GM_CheckPlayerStatusEX(0,PLAYER2_NARROW_HIDDEN))
		||(( at_thk->dis_dif < AVOID_RANK_DIS  )&&(at_thk->dis_rank > 0))
		){
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_WAIT ;
			entk->count3 = 0;
			return ;
		}
	}

	

#if 0
	if(
	(entk->ctrl->addr == entk->at_com->at_trg_addr)
	){
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0;
		return ;
	}
#endif
	/* 体に移動命令 */
	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}
//自分の所属チームの人数
//entk->com->enemys.group[entk->g_id]->unit[entk->u_id]->enemy_num;;
/* メンバーが配置に着くまで待機 */

static void Think3_MoveLastPointWait( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;


	if(entk->count3 < AT_AVOID_WAIT_LOOK  ) {
		/*キョロキョロ*/
		entk->act->pad = SP_CAUT_STAND ;
		if ( (entk->count3%(AT_THK_RATE*20) ) > 0 && (entk->count3%(AT_THK_RATE*20) ) < (AT_THK_RATE*10)  ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
		} else if ( entk->count3%(AT_THK_RATE*20) > (AT_THK_RATE*10) && entk->count3%(AT_THK_RATE*20)  < (AT_THK_RATE*20) ) {
			entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
			entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
		}
	}else if(entk->count3 < AT_AVOID_WAIT_STILL  ) {
		/*待機*/
		entk->act->pad = SP_CAUT_STAND ;
	}else {
		if(at_thk->dis_rank!=1) entk->act->pad = SP_CAUT_STAND ;
		if(at_thk->dis_rank==1) entk->act->pad = SP_CLE_GOSIGN ;
		/*ゴーサイン終了で探索開始*/
		/*チームが一人なら即探索*/
		if((entk->act->act_end)
		||(entk->com->enemys.group[entk->g_id]->unit[entk->u_id]->enemy_num <=1)
		||(entk->count3 > AT_AVOID_WAIT_LIMIT)
		){
			entk->at_com->watch_status |= AT_COM_WATCH_SEARCH ;
		}
		if(entk->at_com->watch_status & AT_COM_WATCH_SEARCH)
		{
			StartSearchMode(entk) ;
			return ;
		}
	}
	entk->count3 ++ ;
}
#if 1
static void Think3_AvoidWait( entk )
ENETHINK	*entk ;
{
	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;

	entk->act->pad = SP_CAUT_STAND ;

	if(( at_thk->dis_dif < AVOID_RANK_DIS  )&&(at_thk->dis_rank > 0)){
	}else {
		if(entk->count3> DIRECT_TICK(60)){
			entk->think2 = TH2_CHASE ;
			entk->think3 = TH3_ZONE_CHASE ;
			entk->count3 = 0 ;
			return ;
		}
	}
	if((entk->count3 > DIRECT_TICK(180))||(entk->at_com->watch_status & AT_COM_WATCH_SEARCH)){
		entk->think2 = TH2_CHASE ;
		entk->think3 = TH3_WAIT ;
		entk->count3 = 0;
		return ;
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}
#endif
static void Think3_SearchReady( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( entk->count3 > (AT_THK_RATE*25) ) {
		entk->scene = CLE_ENE_WAIT_COMPLETE ;
		entk->avoid = 0 ;
	}

	entk->count3 ++ ;
}

static void Think3_DefenseLookAround( entk )
ENETHINK	*entk ;
{
	entk->act->pad = SP_CAUT_STAND ;

	if ( (entk->count3 ) > 0 && (entk->count3 ) < (AT_THK_RATE*10)  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy + 512 ;
	} else if ( (entk->count3 > (AT_THK_RATE*10)) && (entk->count3  < (AT_THK_RATE*20))  ) {
		entk->status2 |= ENE_STATUS2_AIM_DIR_FACE ;
		entk->act->aim_dir = entk->ctrl->turn.vy - 512 ;
	}
	/*全員がこの状態でavoid終了*/
	/* 各コマンダーとの状況連絡用 */
	entk->scene = CLE_ENE_WAIT_COMPLETE ;
	/* 回避値 */
//	entk->avoid = 0 ; 

	if ( entk->count3 > (AT_THK_RATE*30) ) {
printf("AVOID LOOK AROUND\n");
		entk->think3 = TH3_LOOK_NEARZONE ;
		/*全員がこの状態でavoid終了*/
		/* 各コマンダーとの状況連絡用 */
		entk->scene = CLE_ENE_WAIT_COMPLETE ;
		/* 回避値 */
		entk->avoid = 0 ; 
		entk->count3 = 0 ;
		return ;
	}
	entk->count3 ++ ;
}
static	void	Think3_MoveSafeArea( entk )
ENETHINK	*entk ;
{
	int	route,intr ;

	entk->act->pad = SP_MV_RUN ;
	entk->act->tmp_time = 0;
	
	route = ENE_GetTrgRoute( entk ) ;
	
	if ( route <= 1 ){
		entk->status2 |= ENE_STATUS2_AIM_GUN ;
		entk->act->aim_pos = entk->trgpoint.pos ;
		if ( route < 1 ){
			entk->act->aim_pos.vy += 1000 ;	/* ゾーンなので高さ＋ */
		}
		entk->act->move_s = MoveCautionWalk ;
	} else {
//		entk->act->move_s = MoveWalk ;
		entk->act->move_s = MoveRun ;
	}
	
	if( SIG_CheckRIntrpt(entk) ){
		entk->think2 = TH2_SEARCH ;
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		entk->scene = CLE_ENE_WAIT_START ;
		return ;
	}

	
	/*
	-1 到達
	-2 侵入禁止
	1  何かで遮断、種類判別必要
	0  探索続行
	*/
	intr = ENE_ZoneTraceIntrpt( entk->znavi, &(entk->trgpoint),
		entk->ctrl, entk->count3 ) ;

	if (
	( intr < 0 )||( SIG_CheckRIntrpt(entk) )
	){

#if 1
		entk->think2 = TH2_SEARCH ;
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		entk->scene = CLE_ENE_WAIT_START ;
		return ;
#else
		entk->act->move_s = MoveWalk ;
		entk->think3 = TH3_LOOKAROUND_CHECK_ZONE ; 
		entk->count3 = 0 ;
		return ;
#endif
	}

	entk->act->dir = entk->trgpoint.dir ;
	entk->act->tmp_dir = entk->trgpoint.dir ;
	entk->count3 ++ ;
}

/*----- 中レベル思考モード --------------------------------------------*/
#if 0
static	void	PlayerCheck( entk )
ENETHINK	*entk ;
{
	if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
	}
}
#endif
/*----------------------------------------------------------------------------*/
/**
case TH2_CHASE
//TH3_LOOKAROUND_CHECK_ZONE, TH3_MOVE_DEFENSE,
***/
static	void	Think2_LastPlayerPoint( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
	    case TH3_ZONE_CHASE :
	    	Think3_ZoneMoveLastPoint( entk ) ;
		break ;
	    case TH3_DIRECT_CHASE :
	    	Think3_DirectMoveLastPoint( entk ) ;
		break ;
		case TH3_WAIT :
	    	Think3_MoveLastPointWait( entk ) ;
		break ;
		case TH3_INTR_WAIT :
			Think3_AvoidWait( entk ) ;
		break ;
		default :
			printf("TH3 CHASE ERR %d\n",entk->think3);
		break;
	}
//	PlayerCheck( entk ) ;

}
/********
TH2_SEARCH
********/
static	void	Think2_Search( entk )
ENETHINK	*entk ;
{
	switch ( entk->think3 ) {
		case TH3_SEARCH_STAND :
			Think3_SearchReady( entk ) ;
			break ;
		case TH3_LOOKAROUND :
			Think3_DefenseLookAround( entk ) ;
			break ;
		case TH3_LOOK_NEARZONE :
			Think3_LookCheckZones( entk ) ;
			break;
//		case TH3_WAIT :
//		break ;
		case TH3_MOVE_SAFEAREA :
			Think3_MoveSafeArea( entk ) ;
			break ;
		default :
			printf("TH3 SEARCH ERR %d\n",entk->think3);
		break;
	}
//	PlayerCheck( entk ) ;
}

#if 1
static	void	Think2_Discovery( entk )
ENETHINK	*entk ;
{
//	switch ( entk->think3 ) {
//	    case TH3_DISCOVERY_POSE :
//	    	Think3_DiscoveryPose( entk ) ;
//		break ;
//	}
	THK_Discovery( entk ) ;
	/*長廊下兵がAVOID中に発見した場合*/
	if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
//		if(entk->alert == );
		if( GM_AlertMode == ENE_TH1_ALERT){
//			entk->think_mode = ENE_TH1_ALERT ;
		}
	}
}
#endif
/*----- モードチェック --------------------------------------------*/
#include "../../korekado/attacker/modechng.c"

static	void	ChaseModeCheck( entk )
ENETHINK	*entk ;
{
	/* 他のモードへ */
	TravelModeChange( entk ) ;
}

static	void	TouchModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TOUCH) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	TouchModeChange( entk ) ;
}

static	void	NoiseModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_NOISE) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	NoiseModeChange( entk ) ;
}

static	void	TraceModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_TRACE) ) {
		ENE_TraceEnd( entk ) ;
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	TraceModeChange( entk ) ;
}

static	void	IndistinctModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_INDISTINCT) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	IndistinctModeChange( entk ) ;
}

static	void	UniformModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_UNIFORM) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	UniformModeChange( entk ) ;
}

static	void	CorpsModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	CorpsModeChange( entk ) ;
}

static	void	EneDamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_ENE_DAMAGE) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	EneDamageModeChange( entk ) ;
}

static	void	HoldUpModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_HOLDUP) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	HoldUpModeChange( entk ) ;
}

static	void	BoxModeCheck( entk )
ENETHINK	*entk ;
{
	/* 巡回に戻る */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	BoxModeChange( entk ) ;
}

static	void	DamageModeCheck( entk )
ENETHINK	*entk ;
{
	/* サーチに戻る */
	if ( !(entk->c_notice & ENE_NOTICE_DAMAGE) ) {
		SIG_AttackerReturnModeAvoid( entk ) ;
	}

	/* 他のモードへ */
	DamageModeChange( entk ) ;
}

static	void	DiscoveryModeCheck( entk )
ENETHINK	*entk ;
{
}

/*----- 高レベル思考モード --------------------------------------------*/
	/*
		回避（クリアリング以外）
	*/
void	SIG_Attacker_Think1_Avoid( entk )
ENETHINK		*entk ;
{

	entk->status2 = 0 ;
	entk->avoid = 0 ;
	/*回避値クリア*/


//printf("SIGAVOID\n");
//printf("ID [%d] TH1[%d]TH2[%d]TH3[%d]\n",entk->id,entk->think1,entk->think2,entk->think3);
	switch ( entk->think2 ) {
	    case TH2_CHASE :	/* プレイヤーを見失った時点まで移動 */
			Think2_LastPlayerPoint( entk ) ;
			ChaseModeCheck( entk ) ;
		break ;
	    case TH2_SEARCH :	/* プレイヤーを見失った時点で探索 */
			Think2_Search( entk ) ;
			ChaseModeCheck( entk ) ;
		break ;

	    case TH2_TOUCH :			/* タッチモード */
			THK_TouchMode( entk ) ;
			TouchModeCheck( entk ) ;
		break ;
	    case TH2_NOISE :			/* 物音モード */
			THK_NoiseMode( entk ) ;
			NoiseModeCheck( entk ) ;
		break ;

			/* 不審物モード無し */

	    case TH2_TRACE :			/* 跡、追跡モード */
			THK_TraceMode( entk ) ;
			TraceModeCheck( entk ) ;
		break ;
	    case TH2_INDISTINCT :		/* 朧モード */
			THK_IndistinctMode( entk ) ;
			IndistinctModeCheck( entk ) ;
		break ;
	    case TH2_UNIFORM :			/* 変装モード */
			THK_UniformMode( entk ) ;
			UniformModeCheck( entk ) ;
		break ;
	    case TH2_CORPS :			/* 死体モード */
			THK_CorpsMode( entk ) ;
			CorpsModeCheck( entk ) ;
		break ;
	    case TH2_ENE_DAMAGE :		/* 味方ダメージ発見 */
			THK_EneDamage( entk ) ;
			EneDamageModeCheck( entk ) ;
		break ;
	    case TH2_HOLDUP :			/* ホールドアップモード */
			THK_HoldUpMode( entk ) ;
			HoldUpModeCheck( entk ) ;
		break ;
	    case TH2_BOX :				/* ダンボールモード */
			THK_BoxMode( entk ) ;
			BoxModeCheck( entk ) ;
		break ;

	    case TH2_DAMAGE :			/* ダメージモード */
			THK_Damage( entk ) ;
			DamageModeCheck( entk ) ;
		break ;
	    case TH2_DISCOVERY :		/* 発見 */
			Think2_Discovery(entk) ;
			DiscoveryModeCheck( entk ) ;
		break ;
	}
//	ENE_ZoneIntrptCheck( entk->znavi->next_addr ) ;
//printf("ENE_ZoneIntrptCheck( 16320 ) \n") ;
//ENE_ZoneIntrptCheck( 16320 ) ;

//PosBox( &(entk->trgpoint.pos) ,250.0f ,NULL );
}

void	Check_Attacker_Think1_Avoid( entk )
ENETHINK	*entk ;
{
//printf("CLEARING OR NORMAL AVOID? \n");
	if ( COM_GetUnitBuff(entk,CLE_BUFF_KIND) == CLE_TH2_CLEARING ||
		( entk->iknow_flag & IKNOW_AVOID_CLEARING ) ) {
		ENE_Attacker_Think1_Avoid( entk ) ;
	} else {
		SIG_Attacker_Think1_Avoid( entk );
	}
}
/*
物音モードなどから回避モードに戻る
*/
void	SIG_AttackerReturnModeAvoid( entk )
ENETHINK	*entk ;
{
//	int count ;
	
printf("RETURN AVOID");
	entk->c_notice = 0 ;

	entk->act->CheckPad = AttackerAvoidCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;

#if 1
	/*サーチ開始後ならこのフラグが立ってる*/
	if(entk->at_com->watch_status & AT_COM_WATCH_SEARCH){
		/*キョロキョロして終わり*/
		entk->think1 = ENE_TH1_AVOID ; 
		entk->think2 = TH2_SEARCH ;
		entk->think3 = TH3_LOOKAROUND ;
		entk->count3 = 0 ;
		entk->scene = CLE_ENE_WAIT_START ;
		return ;
	}else {
		/*サーチに戻る*/
		SIG_AttackerStartModeAvoid( entk ) ;
		return ;
	}

#else

	if ( entk->avoinfo.check_num > 0 ) {
		entk->think2 = TH2_SEARCH ; 
		
		count = entk->avoinfo.check_count ;
		if ( entk->avoinfo.check_num > count ) {
			ENE_SetTrgpZone( &(entk->trgpoint), entk->avoinfo.check_zone[count], entk->avoinfo.check_map ) ;
			entk->think3 = TH3_MOVE_SAFEAREA ; 
		} else {
//			ENE_SetTrgpDefense( &(entk->trgpoint), 
//			&entk->def_pos, entk->def_mapbit ) ;
			ENE_SetTrgpDefense( &(entk->trgpoint),
			&entk->znavi->flore_pos,
			HZX_ZoneMapNo(entk->ctrl->addr) ) ;
			entk->think3 = TH3_MOVE_DEFENSE ; 
		}
	} else {
//		ENE_SetTrgpDefense( &(entk->trgpoint), 
//		&entk->def_pos, entk->def_mapbit) ;
		ENE_SetTrgpDefense( &(entk->trgpoint),
		&entk->znavi->flore_pos,
		HZX_ZoneMapNo(entk->ctrl->addr) ) ;
		entk->think2 = TH2_MOVE ; 
		entk->think3 = TH3_ZONE_CHASE ; 
	}
	entk->count3 = 0 ;
#endif
}

/* スタートモード、攻撃兵回避 -------------------*/
void	SIG_AttackerStartModeAvoid( entk )
ENETHINK	*entk ;
{
//	FVECTOR	l_pos ;
//	int		l_map ;

printf("AT AVOID START\n");

//	AT_SetMode( entk->act, ENE_ActStandStill ) ;
	ResetUseZone(entk);
	entk->act->CheckPad = AttackerAvoidCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->dir = -1 ;
	entk->act->pad = 0 ;
	entk->avoid = 0 ;

	entk->think1 = ENE_TH1_AVOID ;

	entk->think2 = TH2_CHASE ;
	entk->think3 = TH3_ZONE_CHASE ;
	entk->count3 = 0 ;
		
	/*2001.06.022*/
	SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), entk->com ) ;
	entk->scene = CLE_ENE_MOVE_START ;
	entk->act->move_s = MoveRun ;


	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}


}

/* スタートモード、攻撃兵回避 -------------------*/
#if 1
void	SIG_AttackerStartModeAvoidPosSet( entk ,pos,hzx_id)
ENETHINK	*entk ;
FVECTOR	*pos ;
int hzx_id ;
{
//	FVECTOR	l_pos ;
//	int		l_map ;
	ResetUseZone(entk);
	entk->act->CheckPad = AttackerAvoidCheckPad ;
	entk->act->keep_pad = -1 ;
	entk->act->dir = -1 ;
	entk->avoid = 0 ;
	entk->act->pad = 0 ;

	entk->think1 = ENE_TH1_AVOID ;

	entk->think2 = TH2_CHASE ;
	entk->think3 = TH3_ZONE_CHASE ;
	entk->count3 = 0 ;
		
	ENE_SetTrgpPoint( &(entk->trgpoint), pos, hzx_id ) ;
	entk->scene = CLE_ENE_MOVE_START ;
	entk->act->move_s = MoveRun ;



	if ( entk->act->bodyp.pbreak & PBREAK_ARMLEG ) {
		entk->think2 = TH2_DAMAGE ; 
		entk->c_notice = 0 ;
		THK_DamageMedicalModeStart( entk ) ;
		return ;
	}


}
#endif

void	SIG_AttackerStartModeAvoidDamage( entk )
ENETHINK	*entk ;
{
//	FVECTOR	l_pos ;
//	int		l_map ;

	entk->think1 = ENE_TH1_AVOID ;

	if ( entk->iknow_flag & IKNOW_DETECT ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}

	if ( ENE_PlayerHoldCheck( entk ) ) {
		entk->think2 = TH2_DISCOVERY ; 
		THK_DiscoveryModeStartAvoid( entk ) ;
		return ;
	}

	if ( entk->mess_notice == MES_NOTICE_ZZZ ||
		entk->mess_notice == MES_NOTICE_GOOFY ||
		entk->mess_notice == MES_NOTICE_SMOKE ) {

		entk->think2 = TH2_TOUCH ;
		THK_TouchModeStart( entk ) ;
		return ;
	}

	entk->think2 = TH2_DAMAGE ; 
	entk->c_notice = 0 ;
	THK_DamageModeStart( entk ) ;
}


void	Check_AttackerStartModeAvoid( entk )
ENETHINK	*entk ;
{

	AT_THK	*at_thk ;
	at_thk = (AT_THK *)entk->character ;
	at_thk->blur_sw = -60;

	if ( COM_GetUnitBuff(entk,CLE_BUFF_KIND) == CLE_TH2_CLEARING ||
		( entk->iknow_flag & IKNOW_AVOID_CLEARING ) ) {
printf("CLEARING START \n") ;
		ENE_AttackerStartModeAvoid( entk ) ;
	} else {
printf("SEARCH START\n") ;
		SIG_AttackerStartModeAvoid( entk ) ;
	}
	
#if 0
	if ( entk->act->bodyp.pbreak != 0 ) {
		entk->act->bodyp.pbreak = 0 ; /* 超暫定！！アクトでやり */
	}
#endif

}

void	Check_AttackerStartModeAvoidDamage( entk )
ENETHINK	*entk ;
{

	if ( COM_GetUnitBuff(entk,CLE_BUFF_KIND) == CLE_TH2_CLEARING ||
		( entk->iknow_flag & IKNOW_AVOID_CLEARING ) ) {
printf("CLEARING START DAMAGE\n") ;
		ENE_AttackerClearingModeDamage( entk ) ;
	} else {
printf("SEARCH START DAMAGE[%x]\n",entk->notice) ;
		SIG_AttackerStartModeAvoidDamage( entk ) ;
	}

#if 0	
	if ( entk->act->bodyp.pbreak != 0 ) {
		entk->act->bodyp.pbreak = 0 ; /* 超暫定！！アクトでやり */
	}
#endif
}
