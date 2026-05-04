//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	atcomm.c
	攻撃兵用コマンダー
	
	1999/08/24 Y.Korekado
	$Id: atcomm.c,v 1.1.1.3 2002/11/19 11:49:00 Yoshizawa1 Exp $

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


#include	"at_enum.h"
#include	"at_thk.h"
#include	"../tng/tng.h"

#include	"sig_conv.x"

/*NPC EMMA */
#include	"../../morita/emma/include/emma_com.h"

/*---------------------------------------------------------------*/

#define DEF_POINT_MAX	(6)

//FVECTOR	AT_BOUND[2];
typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	AT_COM		at_com;
} Work ;






#if 0
SD_V_C01MAKI,	//敵兵１「グレネード！」//gbs_a004 292
SD_V_C04MAKI,	//敵兵１「くらえ」//gbs_a026 296
SD_V_C07MAKI,	//敵兵１「停まれ!」//gbs_a02e 300
SD_V_C08MAKI,	//敵兵１「クリア」//gbs_a02f 304
SD_V_ATKO01,	//攻撃兵１対離れ「そこにいるぞ！」//gbs_a001 308
SD_V_ATKO03,	//攻撃兵１対逃げ「逃げたぞ！」//gbs_a002 312
SD_V_ATKO05,	//攻撃兵１非発砲「撃つな！」//gbs_a003 316
SD_V_MAKI20,	//敵兵１足跡モード「ん？」//gbs_a006 320
SD_V_MAKI21,	//敵兵１足跡モード「うん？」//gbs_a007 324
SD_V_MAKI24,	//敵兵１プレイヤー発見モード「あっ」//gbs_a00a 328
SD_V_MAKI27,	//敵兵１味方ダメージモード「んっ！」//gbs_a00d 332
SD_V_MAKI35,	//敵兵１ホールドアップモード「ひぃ」//gbs_a015 336
SD_V_GBS_A016,	//敵兵１ホールドアップ２「お助け」//gbs_a016 340
SD_V_MAKI36,	//敵兵１エロ本発見「ん！」//gbs_a018 344
SD_V_MAKI37,	//敵兵１「ゴッ（Ｇｏ)」//gbs_a019 348
SD_V_MAKI39,	//敵兵１「待てっ」//gbs_a021 352
#endif














//static Work *atcom ;

extern  HZX_ZON *ENE_HZX_GetZone(int addr);
extern int ENE_GetRouteDis( FVECTOR *,FVECTOR *,int,int,u_int);

extern void Sig_DefenseModeStart( ENETHINK * ); 
extern void SIG_CheckCorner( ENETHINK * ); 


extern void COM_AttackerStartModeAvoid( CLEARING * );
extern void COM_AttackerAvoid( CLEARING * );

extern void PosBox(FVECTOR * ,float ,SVECTOR * );
extern ENETHINK *GetAtRanking(ENETHINK *,int );
extern int CheckNoSearch(int);
extern int GetNewSerch( int ,int ) ;

extern int CheckAnesAT(ENETHINK *);


#define AT_COM_MSG_FLAGSET	(1) /*scn_statusセット*/
#define	PL_HIDE_STATUS	(PLAYER_BEYOND|PLAYER_INTRUDE|PLAYER_LOCKER)


//int AT_COM_GetRetreatAddr(COMMANDER * , ENETHINK *) ;
int AT_COM_GetRetreatAddr(ENETHINK *) ;

static void ATCOM_CheckHostage(AT_COM * ) ;

static void ResetAT_Bound(AT_COM * ,int *) ;
static void MakeAT_Bound(AT_COM * ,ENETHINK * ,int * );



#ifdef COM_DEBUG_PRIM
static int AddrSet(int addr){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(GM_PlayerAddress)<<16));
	return addr;
}
static void ComActPrimPos( Work *work,ENETHINK *entk)
{

	int i,zonenum,near[6],mode;
	int safezone;
	HZX_ZON *pl_zone;

	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);

	mode = 1 ;

	switch (mode){
		case 0: break;
		case 1:
			/*安全地帯表示*/
			for(i=0;i<(HZX_MAX_SAFEZONE_NUM);i++){
				safezone = pl_zone->safes[i];
				safezone = AddrSet(safezone);
				NewZoneViewer( safezone,30,0);
			}
			break;
		case 2 :
			/*プレイヤアドレス*/
			NewZoneViewer( GM_PlayerAddress,15,0);
			break;
		case 3 :
			/** 隣接ゾーン **/
			for(i=0;i<6;i++) near[i] = 255; 
			zonenum = HZX_NearZones( GM_PlayerControl->hzx_id,
			HZX_Zone1(GM_PlayerAddress), near );
			for(i=0;i<zonenum;i++) {
				NewZoneViewer( AddrSet(near[i]),30,0);
//				SetZonePrim(work,i,near[i],0);
			}
			break;
		case 4:
			/*低い安全地帯のみ表示*/
			for(i=0;i<(HZX_MAX_SAFEZONE_NUM);i++){
				if(pl_zone->safe_types[i] & SAFE_LOW){
					safezone = pl_zone->safes[i];
					safezone = AddrSet(safezone);
//					SetZonePrim(work,i,safezone,0);
					NewZoneViewer( safezone,1,0);
				}else {
//					SetZonePrim(work,i,255,0);
//					NewZoneViewer( safezone,1,0);
				}
			}
			break;
		case 5:
			/*ゾーン間視界情報チェック*/
			NewZoneViewer( entk->ctrl->addr,30,0);
			NewZoneViewer( GM_PlayerAddress,30,0);
			if(ENE_ReadOnlinInfo(entk->ctrl->addr,
			GM_PlayerAddress)){
				printf("DAME !!\n");
			} else {
				printf("MIERU !!\n");
			}
			break;
	}
}

#endif

/*---------------------------------------------------------------*/


#define AT_BOUND_WIDE (500.0F)
/*攻撃兵の戦闘エリア*/
/*プレイヤ座標を基準とする*/
//static void ResetAT_Bound(Work *work ,int *flag){
static void ResetAT_Bound(AT_COM *at_com ,int *flag){
//	FVECTOR pos0,pos1;
//	AT_BOUND[2];
//	at_com->at_bound[0] = at_com->at_bound[1] = GM_PlayerPosition ;
	at_com->at_bound[0] = at_com->at_bound[1] = DG_ZeroVector ;
	*flag = 0;

//	AT_BOUND[0].vx -= AT_BOUND_WIDE ;
//	AT_BOUND[0].vy -= AT_BOUND_WIDE ;
//	AT_BOUND[0].vz -= AT_BOUND_WIDE ;

//	AT_BOUND[1].vx += AT_BOUND_WIDE ;
//	AT_BOUND[1].vy += AT_BOUND_WIDE ;
//	AT_BOUND[1].vz += AT_BOUND_WIDE ;
}
/*戦闘中の兵士の位置をセット*/
//static void MakeAT_Bound(Work *work,ENETHINK *entk ,int *flag ){
static void MakeAT_Bound(AT_COM	*at_com,ENETHINK *entk ,int *flag ){
//	int i;
	FVECTOR epos ;
	AT_THK *at_thk ;

	at_thk = (AT_THK *) entk->character ;

	at_thk->at_status &= ~AT_ST_IN_BOUND ;
	if( entk->pl_eyei.dis >= (entk->sense.eye_s+1000)){
		return ;
	} 
//	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT ) {
	if(entk->pl_eyei.sight != EYE_INFO_SIGHT_IN ) {
		return ;
	}
	if(
	( entk->act->status & AT_NO_ACTIVE ) 
	||( entk->act->bodyp.life <= 0  )
	){
		return ;
	}
	at_thk->at_status |= AT_ST_IN_BOUND ;

	/*はじめて有効兵を検出*/
	epos = entk->ctrl->mov ;
	if(*flag == 0) {
		*flag = 1;
		at_com->at_bound[0] = at_com->at_bound[1] = epos ;

		at_com->at_bound[0].vx -= AT_BOUND_WIDE ;
		at_com->at_bound[0].vy -= AT_BOUND_WIDE ;
		at_com->at_bound[0].vz -= AT_BOUND_WIDE ;

		at_com->at_bound[1].vx += AT_BOUND_WIDE ;
		at_com->at_bound[1].vy += AT_BOUND_WIDE ;
		at_com->at_bound[1].vz += AT_BOUND_WIDE ;

		return ;
	}

	if((epos.vx - AT_BOUND_WIDE) < at_com->at_bound[0].vx ) 
		at_com->at_bound[0].vx = epos.vx - AT_BOUND_WIDE ;
	if((epos.vy - AT_BOUND_WIDE) < at_com->at_bound[0].vy ) 
		at_com->at_bound[0].vy = epos.vy - AT_BOUND_WIDE ;
	if((epos.vz - AT_BOUND_WIDE) < at_com->at_bound[0].vz ) 
		at_com->at_bound[0].vz = epos.vz - AT_BOUND_WIDE ;

	if((epos.vx + AT_BOUND_WIDE) > at_com->at_bound[1].vx ) 
		at_com->at_bound[1].vx = epos.vx + AT_BOUND_WIDE ;
	if((epos.vy + AT_BOUND_WIDE) > at_com->at_bound[1].vy ) 
		at_com->at_bound[1].vy = epos.vy + AT_BOUND_WIDE ;
	if((epos.vz + AT_BOUND_WIDE) > at_com->at_bound[1].vz ) 
		at_com->at_bound[1].vz = epos.vz + AT_BOUND_WIDE;
}
//static void SetAvoidMode( Work *work )
static void SetAvoidMode( AT_COM *at_com )
{
	int i ;
	FVECTOR	l_pos ;
	int		l_map ;
	ENETHINK	*topene;

//printf("SET AVOID!!\n");
//printf("group unit num %d\n",at_com->group.unit_num);
	for ( i=0; i<at_com->group.unit_num; i++ ) {
		COM_AttackerStartModeAvoid( &at_com->clearing[ i ] ) ;
	}
	/*回避モード開始時点での目標アドレス待避*/
#if 0
	at_com->player_lastaddr = GM_PlayerAddress ;
#else
	COM_GetPlayerLastPos( &l_pos, &l_map ) ;
//	at_com->player_lastaddr = GM_PlayerAddress ;
//printf("ATCOM LAST MAP 0x%x\n",l_map);
	at_com->player_lastaddr = 
		HZX_GetAddress( GV_GetBit( l_map), &l_pos, -1);

	if(CheckNoSearch(at_com->player_lastaddr)){
		/*先頭兵のentk*/
		topene = GetAtRanking(at_com->unit[0].entk[0] ,0 ) ;
		at_com->player_lastaddr =
			GetNewSerch(at_com->player_lastaddr,
				topene->ctrl->addr);
	}
//printf("PL MAP 0x%x\n",HZX_ZoneMapNo(GM_PlayerAddress));
//printf("PL HZX_ID 0x%x\n",GM_PlayerControl->hzx_id);
//printf("No2Bit 0x%x\n",GV_GetBit(HZX_ZoneMapNo(GM_PlayerAddress)));
//	ENE_SetTrgpPoint( &(entk->trgpoint), &l_pos, l_map ) ;
#endif
	at_com->berserk = 0 ;
}

static	void	AT_COM_CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ;
	n_msg = GV_ReceiveMessage( work->name, &msg );

	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case AT_COM_MSG_FLAGSET :
				/*scn_statusセット*/
//printf("MESG STATUS SET %d\n",msg->message[ 1 ]);
				work->at_com.scn_status = msg->message[ 1 ] ;
			break;
		}
		msg++ ;
	}
}


//static void Avoid( Work *work )
static void Avoid( AT_COM *at_com )
{
	int i,k ;
	ENETHINK *entk;
	for ( i=0; i<at_com->group.unit_num; i++ ) {
		COM_AttackerAvoid( &at_com->clearing[ i ] ) ;
	}
	/*サーチ開始時に全員の回避値OFF*/
	if(at_com->watch_status & AT_COM_WATCH_SEARCH)
	{
		for(k=0;k<at_com->group.unit_num;k++){
			for(i=0;i < at_com->unit[k].enemy_num;i++){
				entk = at_com->unit[k].entk[i];
				entk->avoid = 0 ; 
			}
		}
	}
}



//static inline void SET_STATUS_AT_ACTION(Work *work){
static inline void SET_STATUS_AT_ACTION(AT_COM *at_com){

//	ENE_AlertGameLevel ;
//	at_com->esctime = 180; /* 逃げ開始時間*/
//	at_com->esctime = 120; /* 逃げ開始時間*/
	at_com->esctime = (AT_THK_RATE*10); /* 逃げ開始時間*/
	at_com->chasedis 
		= (at_com->unit[0].entk[0]->sense.eye_s)*3/4 ;
	 /* 追跡距離可変  視力を標準値にとる*/
	if(
	(GM_PlayerStatus & PLAYER_DARK_AREA)
//	&&(entk->sw.n_sight != 2)
	){
		at_com->chasedis /= 4;
	}
	if(GM_PlayerStatus & PLAYER_STEALTH){
		if(at_com->chasedis < 2000){
			at_com->chasedis = 2000 ;
		}
	}

	at_com->attack_num = 4;	/*同時攻撃人数*/
	at_com->shoot_delay = (AT_THK_RATE*10);	/*発砲ためらい時間*/
	at_com->siege_num = 3;
}


//static inline void AT_DEFAULT(Work *work){
static inline void AT_DEFAULT(AT_COM	*at_com){
	at_com->esctime = (AT_THK_RATE*3); /* 逃げ開始時間*/
	at_com->chasedis 
	= (at_com->unit[0].entk[0]->sense.eye_s) ;
	 /* 追跡距離可変  視力を標準値にとる*/
//	at_com->minlen = 2000;		/* 最接近距離 */
	at_com->attack_num = 4;	/*同時攻撃人数*/
	at_com->shoot_delay = 0;	/*発砲ためらい時間*/
	at_com->siege_num = 2;
}



//static void AT_LevelSet(Work *work){
static void AT_LevelSet(AT_COM *at_com){

	ENETHINK *entk;
	int i,k,ene_num,unit_num,all_die=0;
	at_com->alert_time++;


	unit_num = at_com->group.unit_num;
	for(i=0;i<unit_num;i++){
		all_die += at_com->unit[i].die_num;
	}

	/*レベルによって攻撃人数増加*/
//	at_com->attack_num = ENE_AlertGameLevel ;

#if 0	//korekore
	/*ビデオ撮り用じゃなければ*/
	if(at_com->video==0){
		/*むかで状態は最初だけ*/
		if((at_com->level==0)&&(at_com->com_sight)){
			at_com->level = 1;
			for(k=0;k<at_com->group.unit_num;k++){
				ene_num = at_com->unit[k].enemy_num;
				for(i=0;i < ene_num;i++){
					entk = at_com->unit[k].entk[i];
					if(entk->arm_ik != NULL ){
						ExitArmIK(entk->arm_ik) ;
						entk->arm_ik = NULL ; 
					}
				}
			}
		}
	}
#else
	/*むかで状態は最初だけ*/
	if((at_com->level==0)&&(at_com->com_sight)){
		at_com->level = 1;
		/*腕のＩＫ解放*/
		for(k=0;k<at_com->group.unit_num;k++){
			ene_num = at_com->unit[k].enemy_num;
			for(i=0;i < ene_num;i++){
				entk = at_com->unit[k].entk[i];
				if(entk->arm_ik != NULL ){
					ExitArmIK(entk->arm_ik) ;
					entk->arm_ik = NULL ; 
				}
			}
		}
	}
#endif

#ifdef DEBUG_MODE
	/*ビデオ撮り*/
	if(at_com->video){
		switch(at_com->video){
			case 1 : at_com->level=0;
			break;
			case 2 : at_com->level=1;
			break;
			case 3 : at_com->level=2;
			break;
			case 4 : at_com->level=1;
			break;
		}
	}
#endif

	switch(at_com->level){
		case 0:
			/* ムカデ状態 */
			AT_DEFAULT(at_com);
			break;
		case 1:
			/*普通*/
			SET_STATUS_AT_ACTION(at_com);
			break;
		default :
			SET_STATUS_AT_ACTION(at_com);
			break;
	}
}


void DisRankCheck( AT_COM *at_com )
{

//	FVECTOR	l_pos ;
//	int		l_map ;

//pl
	ENETHINK *entk;
	COMMANDER	*com ;
	int i,j,k,dislist[ATENEMY_MAX],idlist[ATENEMY_MAX];
//	int reachlist[ATENEMY_MAX];
	int tmpdis,tmplen,nearid;
	int	addr1,addr2,trgaddr;
	int ene_num;
	FVECTOR	*pos1,*pos2 ;
	FVECTOR	trgpos;
	AT_THK *at_thk;
	long64 front_st[ATENEMY_MAX] ;
//	SVECTOR	rgb ;

	com = at_com->com ;
	entk = at_com->unit[0].entk[0];


#if 0
printf("GM_GetHzxGroupID(GM_NoiseMap)[%x]\n", GM_GetHzxGroupID(GM_NoiseMap));
printf("GM_NoisePosition[%x]\n", GM_NoisePosition);
printf("GM_PlayerHzxID[%x]\n",GM_PlayerHzxID);
printf("GM_PlayerMap[%x]\n",GM_PlayerMap);
printf("com->player_lastmap[%x]\n",com->player_lastmap);
printf("com->player_lastpos x[%f] y[%f] z[%f] \n",com->player_lastpos.vx,com->player_lastpos.vy,com->player_lastpos.vz);
printf("HZX_ID [%x] \n",GM_PlayerControl->hzx_id);
printf("GM_PlayerAddress[%x]\n",GM_PlayerAddress);
#endif


#if 1
	if(com->player_lastmap == 0 ){
		/*イベント等で正規の手順を踏まず危険モードになっている*/
		/*この場合NPCまたはプレイヤを直接認識でOK*/
		if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
			/*対エマ*/
			trgpos = at_com->npc_ctrl->mov ;
			trgaddr = at_com->npc_ctrl->addr ;
		}else {
//printf("com->player_lastmap == 0\n");
			trgpos = GM_PlayerPosition ;
			trgaddr = GM_PlayerAddress ;
		}
	}else {
//printf("com->player_lastmap == [%x]\n",com->player_lastmap);
/*For Japan*/
/*nosear*/
		if( (CheckNoSearch(GM_PlayerAddress))&&( GM_AlertMode == ALERT_MODE_AVOID )&&(HZX_Zone1(at_com->player_lastaddr)!=HZX_NO_ZONE ) ){
			HZX_ZON *last_zone;
			trgaddr = at_com->player_lastaddr ;
			last_zone = ENE_HZX_GetZone(at_com->player_lastaddr);
			SIG_SetZonePos2Fvec(last_zone ,&trgpos) ;
		}else if(at_com->watch_status &(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER)){
			trgpos = GM_PlayerPosition ;
			trgaddr = GM_PlayerAddress ;
		}else {
			trgaddr = HZX_GetAddress( com->player_lastmap, &com->player_lastpos, -1 ) ;
			trgpos = com->player_lastpos ;
		}
//printf("trgaddr = [%x]\n",trgaddr);
		if(HZX_Zone1(trgaddr) == HZX_NO_ZONE){
//printf("SET GM_PlayerAddress\n");
			trgaddr = GM_PlayerAddress ;
			trgpos = GM_PlayerPosition ;
		}
	}
#else

	if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
		if ( GM_AlertMode == ALERT_MODE_AVOID ) {
			HZX_ZON *last_zone;
			if(HZX_Zone1(at_com->player_lastaddr) != HZX_NO_ZONE){
				last_zone = ENE_HZX_GetZone(at_com->player_lastaddr);
			}else {
				last_zone = ENE_HZX_GetZone(GM_PlayerAddress);
			}
			SIG_SetZonePos2Fvec(last_zone ,&trgpos) ;
			trgaddr = at_com->player_lastaddr;
		}else {
			trgpos = GM_PlayerPosition ;
			if(at_com->watch_status 
			&(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER) 
			){
				/*隠れた瞬間を認識している場合*/
				trgaddr = HZX_Zone1(com->plzone_in_zone[0]);
				trgaddr |= (HZX_Zone1(com->plzone_in_zone[0])<<8);
				trgaddr |= (HZX_ZoneMapNo(GM_PlayerAddress) << 16 );
			}else {
				/*通常*/
				trgaddr = com->plzone_in_zone[0] ;
			}
		}
	}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
		/*対エマ*/
		trgpos = at_com->npc_ctrl->mov ;
		trgaddr = at_com->npc_ctrl->addr ;
	}
#endif

	at_com->at_trg_addr = trgaddr ;

//		SIG_SetTrgpPlayerLastpos( &(entk->trgpoint), (entk->com) ) ;
//com->enemys.group[].unit[].entk[]
//	entk = at_com->unit[0].entk[0];
	for(k=0;k<at_com->group.unit_num;k++){
		ene_num = at_com->unit[k].enemy_num;
		for(i=0;i < ene_num;i++){
			entk = at_com->unit[k].entk[i];
			at_thk = (AT_THK *) entk->character ;
			/*最後尾フラグクリア*/
ASSERT(entk != NULL);
ASSERT(at_thk != NULL);
ASSERT(entk->at_com != NULL);
			at_thk->at_status &= ~AT_ST_LAST ;
			at_thk->zone_dis 
				= ENE_GetRouteDis(&entk->znavi->flore_pos,
				&trgpos,entk->ctrl->addr,trgaddr,200000);
			if((at_com->level==0)
			&&(at_thk->zone_dis < entk->at_com->chasedis+1000 )
			){
				at_com->level = 1;
			}else {
			}
		}
	}

	for(k=0;k<at_com->group.unit_num;k++){
		/*ソート準備*/
		ene_num = at_com->unit[k].enemy_num;
		for(i=0;i < ene_num;i++){
			entk = at_com->unit[k].entk[i];
			/*手前兵の状態チェック用にstセット*/
			front_st[i] = entk->act->status;
			at_thk = (AT_THK *) entk->character ;
			/*目標までの距離*/
			dislist[i] =at_thk->zone_dis;
			idlist[i] =i;
/*行動不能者は順位外*/
/*死亡者は別扱い*/
/*相手が隠れてるときは追い越しなし*/
#if 0
/*当時と気絶 睡眠が変わってるため排除*/
			if(!(at_com->watch_status 
			&(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE
			|AT_COM_WATCH_LOCKER))
			){
#else
			if(1){
#endif
/*注 この時点ではfront_stには自分のstatusがセットされている*/
				if(
/*守備モード兵 も除外*/
				((!(entk->act->bodyp.type & ENE_TYPE_EVENT_A))&&( entk->think3 == TH3_DEFENCE))
				/*守備天狗*/
				||((entk->act->bodyp.type & ENE_TYPE_EVENT_A)&&( entk->think1 == ENE_TH1_DEFENSE))
				/*逃げ中*/
				||(!(at_com->watch_status &(AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER))
				&&(!(entk->act->bodyp.type & ENE_TYPE_EVENT_A))&&( entk->think3 == TH3_ZONE_POS))
/*負傷中*/
				||( entk->act->status & ACT_STATUS_MEDICATION) 
				){
//printf("DIS RANK SHIFT %d id %d\n",entk->think3,entk->id);
					dislist[i] += 8000 ;
				}

/*
2001.09.07 いままでみんな同じ値だけ行動優先値を下げていたが　動かない理由の重要度により
優先幅を変える
*/
				/*動かない兵は大きく下げる*/
				if(
				((entk->act->bodyp.anesthesia == -1)&&(entk->act->status & ACT_STATUS_FAINT))
				||(( front_st[i] & AT_NO_ACTIVE ) &&( entk->act->bodyp.life > 0  ))
				){
					if(entk->act->bodyp.type & ENE_TYPE_EVENT_A){
						dislist[i] += 20000 ;
					}else {
						dislist[i] += 8000 ;
					}
				}

/*天狗兵w44 守備と追跡混合のため判定*/
				if(entk->act->bodyp.type & ENE_TYPE_TNG_A){
					ENTK_TENG_A *tng_a ;
					tng_a = (ENTK_TENG_A *) entk->eve_a ;
					if(!(tng_a->status & EVE_TNG_NO_DEF )){
//printf("TNG DIS RANK SHIFT %d id %d\n",entk->think3,entk->id);
						dislist[i] += 5000 ;
					}
#if 0
					if( entk->act->bodyp.life <= 0  ){
printf("TNG DEATH RANK SHIFT %d id %d\n",entk->think3,entk->id);
						dislist[i] += 30000 ;
					}
#endif
				}
			}
		}
		/*プレイヤに近い順 挿入ソート*/
		for(i=1;i<ene_num;i++){
			j = i;
			while ( j >= 1 && dislist[j-1] > dislist[j]) {
				tmpdis = dislist[j]; dislist[j] =dislist[j-1];
				dislist[j-1] = tmpdis;
//				tmpdis = reachlist[j]; reachlist[j] =reachlist[j-1];
//				reachlist[j-1] = tmpdis;
				tmpdis = idlist[j]; idlist[j] = idlist[j-1];
				idlist[j-1] = tmpdis;
				j--;
			}
		}
#if 0
		/*むかで状態なら順位チェック最初だけ*/
		if((at_com->level==0)&&(at_com->alert_time>10))
#else
		/*ムカデ状態ならＩＤ順で連結*/
//		if(at_com->level==0)
		/* 重なり状態から開始なのでほぐすために最初はＩＤ順*/
		if(at_com->alert_time < DIRECT_TICK(60) )
#endif
		{
			for(i=0;i<ene_num;i++){
				at_thk = (AT_THK *) at_com->unit[k].entk[i]->character;
				idlist[i]= 
				at_thk->dis_rank =
				at_com->unit[k].entk[i]->id ;
			}
		}else {
			/*順位更新*/
			for(i=0;i<ene_num;i++){
				at_thk = 
				(AT_THK *) at_com->unit[k].entk[idlist[i]]->character;
				at_thk->dis_rank = i;
			}
		}
		/*味方との距離*/
		for(i=0;i<ene_num;i++){
			if(i==0) {
				/*先頭兵士*/
				at_thk = (AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
				at_thk->dis_dif = at_thk->zone_dis ;
				at_thk->front_st = 0 ;
				addr1 = at_com->unit[k].entk[idlist[i]]->ctrl->addr ;
			}else {
				/*自分より近い兵全てと距離を計算し一番近い物を採用*/
				/*初期値には目標距離を設定*/
				/*プレイヤのほうが近ければそれを採用*/
				at_thk = 
				(AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
				at_thk->dis_dif = at_thk->zone_dis ;
				/*自分*/
				pos1 = &at_com->unit[k].entk[idlist[i]]->znavi->flore_pos ;

				addr1 = at_com->unit[k].entk[idlist[i]]->ctrl->addr ;
				nearid = 255;
				for(j=(i-1);j>=0;j--){
					/*相手*/
					pos2 = &at_com->unit[k].entk[idlist[j]]->znavi->flore_pos ;
					addr2 = at_com->unit[k].entk[idlist[j]]->ctrl->addr ;
					tmplen = ENE_GetRouteDis( pos1, pos2 ,
					addr1,addr2,10000 ) ;

					at_thk = 
					(AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
			/*最近兵 更新*/
					if( at_thk->dis_dif > tmplen){
						at_thk->dis_dif = tmplen; 
						nearid = j;
					}
				}
				if(nearid==255) {
					nearid = 0;
				}
				/*手前兵の状態セット*/
				at_thk = 
				(AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
				at_thk->front_st = front_st[idlist[nearid]] ;

				if(
				(front_st[idlist[nearid]] & AT_NO_ACTIVE)
				&&( entk->act->bodyp.life > 0  )
				)
				{
					at_thk->dis_dif = 25000; 
				}else {
					/*手前との距離判定*/
					addr1 = at_com->unit[k].entk[idlist[i]]->ctrl->addr ;
					addr2 = at_com->unit[k].entk[idlist[nearid]]->ctrl->addr ;
				}
			}
			/*行動可能な最後尾兵を調べる*/
			{
				int x ;
				ENETHINK	*last ;
				AT_THK		*last_at ;
				for(x=(ene_num-1);x>=0;x--){
					last = at_com->unit[k].entk[idlist[x]] ;
					if(last->act->status & AT_NO_ACTIVE ) {
						continue ;
					}
					/*寝てない？*/
					if(CheckAnesAT(last)){
						continue ;
					}
					/*行動可能ラスト兵*/
					last_at  = (AT_THK *) last->character ;
					last_at->at_status |= AT_ST_LAST ;
					break ;
				}
			}
			/*バウンド内の最後尾兵*/
			{
				AT_THK *at_thk ;
				at_thk = (AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
				if(at_thk->at_status & AT_ST_IN_BOUND){
					at_com->bound_last = at_com->unit[k].entk[idlist[i]]->id ;
				}
			}
			/*MUKADE MODE*/
			if(at_com->level==0){

				if(i==0) {
					at_com->unit[k].entk[idlist[0]]->buddy
					= at_com->unit[k].entk[idlist[ene_num-1]];
				}else {	
					at_com->unit[k].entk[idlist[i]]->buddy
					= at_com->unit[k].entk[idlist[i-1]];
					/*自分*/
					pos1 = &at_com->unit[k].entk[idlist[i]]->znavi->flore_pos ;
					addr1 = at_com->unit[k].entk[idlist[i]]->ctrl->addr;
					/*相手*/
					pos2 = &at_com->unit[k].entk[idlist[i-1]]->znavi->flore_pos ;
					addr2 = at_com->unit[k].entk[idlist[i-1]]->ctrl->addr ;
					tmplen = ENE_GetRouteDis( pos1, pos2 ,
					addr1,addr2,10000 ) ;
					at_thk = 
					(AT_THK *) at_com->unit[k].entk[idlist[i]]->character ;
					at_thk->dis_dif = tmplen;
//at_com->unit[k].entk[idlist[i]]->dis_dif = tmplen;
				}
			}
		}
	}
}


/*敵兵配置陣形*/
/*********************************
09.16 位置順位を考慮するように変更
必ずのDisRankCheck後に実行すること
*********************************/

//static void FormDirCheck( Work *work )
static void FormDirCheck( AT_COM	*at_com )
{

	ENETHINK *entk;
	int i,j,k,lastnum;
	short dirlist[ATENEMY_MAX],idlist[ATENEMY_MAX],tmpdir;
	int id1,id2,shiftdir,formnum=0;	/*包囲参加人数*/
	AT_THK *at_thk;

	for(k=0;k<at_com->group.unit_num;k++){
		/*盾兵参加時の陣形変更*/
      XASSERT( k < ATUNIT_MAX, "Too many at units" );
#if 1
		if(
	((!at_com->gosign)&&((at_com->unit[k].enemy_num-at_com->com_sight) <= 1))
		||(at_com->com_sight >= at_com->siege_num)
		||(at_com->shield_num)
		||(1)
		){
			/*包囲停止*/
			at_com->siege[ k ] = 0;
			for(i=0;i<at_com->unit[k].enemy_num;i++){
				at_thk =(AT_THK *) at_com->unit[k].entk[i]->character ;
				at_thk->form_dir = 
				at_com->unit[k].entk[i]->pl_eyei.dir;
			}
			continue;
		}
#endif
		at_com->siege[ k ] = 1;
		/*ソート準備*/
		/*最初に攻撃中のみのリスト*/

		/*攻撃中兵士リスト*/
		for(i=0;i < at_com->unit[k].enemy_num;i++){
			entk = at_com->unit[k].entk[i];
			if(
			(entk->think2==TH2_ATTACK)
//			||(entk->think3==TH3_WAIT_CHASE)
			){
				idlist[formnum] = i; /*参加者のＩＤ*/
				dirlist[formnum] = entk->pl_eyei.dir;
				formnum++;		/*参加人数*/
			}
		}

		/*次に一番近くで待機してる兵士の検索*/
		lastnum = formnum; /*攻撃中人数を待避*/
		if(formnum==1){
			for(i=0;i < at_com->unit[k].enemy_num;i++){
				entk = at_com->unit[k].entk[i];
					/*待機中兵士だ*/
	//			if((entk->think2==TH2_CHASE)&&(entk->think3==TH3_WAIT_CHASE)){
				if((entk->think2==TH2_CHASE)
				&&((entk->think3==TH3_WAIT_CHASE)||(entk->think3==TH3_ATTACK_SIEGE))){
					/*最初に発見された待機兵じゃなければ*/
					if(lastnum != formnum){
						/*dis_rankを比較して*/
//if( at_com->unit[k].entk[idlist[lastnum]]->dis_rank
//<= at_com->unit[k].entk[i]->dis_rank )
if(
((AT_THK *) at_com->unit[k].entk[idlist[lastnum]]->character)->dis_rank
<= 
((AT_THK *) at_com->unit[k].entk[i]->character)->dis_rank 
)
						{
							/*遠いので対象外*/
							/*現在地でのままでＯＫ*/
	((AT_THK *) at_com->unit[k].entk[i]->character)->form_dir =
//							at_com->unit[k].entk[i]->form_dir = 
							at_com->unit[k].entk[i]->pl_eyei.dir;
							continue;
						}
					}
					/*参加登録*/
					idlist[formnum] = i; /*参加者のＩＤ*/
					dirlist[formnum] = entk->pl_eyei.dir;
					formnum++;		/*参加人数*/
				}
			}
		}
		for(i=1;i<formnum;i++){
			j = i;
			/*角度順にソート*/
			while ( j >= 1 && dirlist[j-1] > dirlist[j]) {
				tmpdir = dirlist[j]; dirlist[j] =dirlist[j-1];
				dirlist[j-1] = tmpdir;
				tmpdir = idlist[j]; idlist[j] = idlist[j-1];
				idlist[j-1] = tmpdir;
				j--;
			}
		}
		/*参加者リストの最下位兵を位置基準にする*/
		if(formnum>1) {
			for(i=0;i<formnum;i++){
				id1 = i-1;
				id2 = i+1;
				shiftdir = 4096;
				if(i==0) {
					id1 = (formnum-1);
				}else if(i == (formnum-1) ) {
					id2 = 0;
				}else {
					shiftdir = 0;
				}
				/*両隣の中間位置*/
				tmpdir = 
					(at_com->unit[k].entk[idlist[id1]]->pl_eyei.dir +
					at_com->unit[k].entk[idlist[id2]]->pl_eyei.dir +
					shiftdir)>>1;
				tmpdir &= 4095;
((AT_THK *) at_com->unit[k].entk[idlist[i]]->character)->form_dir = tmpdir ;
//				at_com->unit[k].entk[idlist[i]]->form_dir = tmpdir;
			}
		}else{
			/*参加者が1名以下なら*/
			for(i=0;i<at_com->unit[k].enemy_num;i++){
		((AT_THK *) at_com->unit[k].entk[i]->character)->form_dir = 
//				at_com->unit[k].entk[i]->form_dir = 
				at_com->unit[k].entk[i]->pl_eyei.dir;
			}
		}
	}
}

/*プレイヤの隠れ状態をチェック*/
//(GM_PlayerStatus & PLAYER_FORCE) 強制モーション中
//PLAYER_LOCKER
/*ビヨンドに移行するときは*/
//void CheckPlayerHide(Work *work){
void CheckPlayerHide(AT_COM *at_com){
	int i,j,dis=0;
	long64	check_st,check_st_old ;
	HZX_ZON *pl_zone;
	ENETHINK *entk ;
	AT_THK	*at_thk ;
	check_st = (GM_PlayerStatus & PL_HIDE_STATUS ) ;
	if(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE){
		/*攻撃兵にはイントルードフラグゾーンとトラップが等価なので判定バッファにフラグを立てる*/
		check_st |= PLAYER_INTRUDE;
	} 

	check_st_old = (at_com->pl_status_old) & PL_HIDE_STATUS ;
	if(check_st != check_st_old ){
#if 0
		dis = 0;
#else
		if(
		(!(at_com->com_sight))
		&&(!(at_com->feel_flag))
//		&&((ENE_CLBoundCheck( &GM_PlayerPosition ) )<0)
		){
			for(i=0;i< at_com->group.unit_num ;i++){
				for(j=0;j < at_com->unit[i].enemy_num;j++){
					entk = at_com->unit[i].entk[j];
					at_thk = (AT_THK *) entk->character ;
					if(SIG_CheckStealthStatus(entk)){
						continue ;
					}
//zone_dis
//					if(entk->pl_eyei.dis < entk->sense.eye_s ){
					if(at_thk->zone_dis  < (entk->sense.eye_s) ){
						dis = 1;
						break ;
					}
				}
			}
		}
#endif
//detect
		/*feel フラグをチェック*/
		if(
		(at_com->com_sight) 
		||(at_com->feel_flag)
		||(dis)
		){
//printf("AT_COM sight [%d] feel [%d] dis [%d]\n" ,at_com->com_sight ,at_com->feel_flag,dis);

			if(GM_PlayerStatus & PLAYER_BEYOND) 
				at_com->watch_status |= AT_COM_WATCH_BEYOND ;
			if((GM_PlayerStatus & PLAYER_INTRUDE)||(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE) )
				at_com->watch_status |= AT_COM_WATCH_INTRUDE ;
			if(GM_PlayerStatus & PLAYER_LOCKER) 
				at_com->watch_status |= AT_COM_WATCH_LOCKER ;
			if(GM_PlayerStatus & PL_HIDE_STATUS){ 
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			}
		}
	}


	/*追い詰めゾーンチェック*/
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
	if(pl_zone->flag & HZX_ZONE_NO_AVOID){
		if((at_com->com_sight)||(at_com->feel_flag)){
			at_com->watch_status |= AT_COM_WATCH_NO_AVOID ;
		}
	}else {
		at_com->watch_status &= ~AT_COM_WATCH_NO_AVOID ;
	}
	/*ステルス中は認識フラグ全クリア*/
	/*攻撃兵はステルス無効モードなしなのでプレイヤステータスのみで判断でOK*/
	if(GM_PlayerStatus & PLAYER_STEALTH ){
		at_com->watch_status &= 
		~(AT_COM_WATCH_NO_AVOID|AT_COM_WATCH_BEYOND|AT_COM_WATCH_INTRUDE|AT_COM_WATCH_LOCKER) ;
	}
	if((COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)&&(GM_AlertLevel==ALERT_LEVEL_MAX)){
		/*攻撃兵以外が認識中*/
		if(GM_PlayerStatus & PLAYER_BEYOND){
			at_com->watch_status |= AT_COM_WATCH_BEYOND ;
		}
	}
	/*ぶら下がり終了*/
	if(!(GM_PlayerStatus & PLAYER_BEYOND) ){
		at_com->watch_status &= ~AT_COM_WATCH_BEYOND ;
	}
	if(!(GM_PlayerStatus & PLAYER_INTRUDE)&&(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)) ){
		at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
	}
	if(!(GM_PlayerStatus & PLAYER_LOCKER) ){
		at_com->watch_status &= ~AT_COM_WATCH_LOCKER ;
	}
}
static void CheckIknowIntrude(AT_COM *at_com){
	int i,j;
	ENETHINK *entk ;
	AT_THK	*at_thk ;
	if(
	(!(GM_PlayerStatus & PLAYER_INTRUDE) )
	&&(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE))){
		at_com->watch_status &= ~AT_COM_WATCH_INTRUDE ;
		return;
	}
	if(at_com->watch_status & AT_COM_WATCH_INTRUDE ){
		/*すでに知られてるのでチェック不要*/
		return;
	}
	for(i=0;i< at_com->group.unit_num ;i++){
		for(j=0;j < at_com->unit[i].enemy_num;j++){
			entk = at_com->unit[i].entk[j];
			at_thk = (AT_THK *) entk->character ;
//zone_dis
			if(SIG_CheckStealthStatus(entk)) continue ;
			if(
//			(entk->iknow_flag & IKNOW_HIDDEN_PLAYER )
			((entk->iknow_flag & IKNOW_HIDDEN_PLAYER )||(entk->iknow_flag & IKNOW_HIDDEN_PLAYER_CLEARING_INTRUDE ))
//			&&(at_thk->at_status & (AT_ST_FEEL|AT_ST_NEXT) )
			&&(at_thk->zone_dis  < (entk->sense.eye_s) )
			){
				/*知ってる人がいた*/
				at_com->watch_status |= AT_COM_WATCH_INTRUDE ;
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
				return ;
			}
		}
	}
}

//GM_PlayerStatus & PLAYER_ENEMY_HANG
//AT_COM_WATCH_HOSTAGE
/*味方が捕まってびっくり判定*/
/*pl_status_old更新前に実行*/
//static void ATCOM_CheckHostage(Work *work){
static void ATCOM_CheckHostage(AT_COM	*at_com){

	/*１フレームだけ有効*/
	if(at_com->watch_status & AT_COM_WATCH_HOSTAGE )
	{
		at_com->watch_status &= ~AT_COM_WATCH_HOSTAGE ;
		return ;
	}


	if(
	(!(at_com->pl_status_old & PLAYER_ENEMY_HANG))
	&&(GM_PlayerStatus & PLAYER_ENEMY_HANG)
	){
		/*つかんだ瞬間*/
		if(at_com->surprised == 0) {
			at_com->watch_status |= AT_COM_WATCH_HOSTAGE ;
//		entk->at_com->surprised = AT_SURP_TIME ;

		}
	}
	if(!(GM_PlayerStatus & PLAYER_ENEMY_HANG)){
		/*解放された*/
		at_com->watch_status &= ~AT_COM_WATCH_HOSTAGE ;
	}
	if(at_com->surprised > 0) {
		at_com->surprised -- ;
	}
}
/*仲間の死に際にビックリ*/
//static void CheckDeathBedFlag(Work *work)
static void CheckDeathBedFlag(AT_COM	*at_com)
{
	if(at_com->watch_status & AT_COM_WATCH_DEATHBED ){
		at_com->watch_status &= ~AT_COM_WATCH_DEATHBED ;
	}
	if(at_com->watch_status_req & AT_COM_WATCH_DEATHBED ){
		at_com->watch_status |= AT_COM_WATCH_DEATHBED ;
		at_com->watch_status_req &= ~AT_COM_WATCH_DEATHBED ;
	}
}
/**/

/* 毎フレーム更新される情報取得 */
//void AT_ComRenew( Work *work )
void AT_ComRenew( AT_COM *at_com )
{
	int i,k,sight,flag = 0 ,se_flag; 
	ENETHINK *entk;
	AT_THK	*at_thk ;
	HZX_ZON *pl_zone;

	ResetAT_Bound( at_com ,&flag) ;
	at_com->bound_last = -1 ;
	/*毎フレーム再チェックすべきフラグはここで寝かせる*/
	at_com->watch_status &= ~AT_COM_WATCH_NO_SHOT ;


	if(at_com->rollout > 0) at_com->rollout-- ;
	if(at_com->OldPlAddr != GM_PlayerAddress) {
		at_com->Pl_StayTime = 0; 
		/*プレイヤが移動したら待機フラグリセット*/
		at_com->watch_status &= ~AT_COM_WATCH_APPROACH ;
		/*プレイヤのいるゾーンの属性をチェック*/
		pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
/********/
	}else {
		if(at_com->Pl_StayTime < 0xffffffff)  
			at_com->Pl_StayTime++;
	}
	at_com->OldPlAddr = GM_PlayerAddress ;

	sight = 0;
	at_com->shield_num = 0;
	at_com->feel_flag = 0;

	if(at_com->gosign>0) at_com->gosign--;
	if(at_com->rollout>0) at_com->rollout--;
	for(k=0;k<at_com->group.unit_num;k++){
		for(i=0;i < at_com->unit[k].enemy_num;i++){
			entk = at_com->unit[k].entk[i];
			at_thk = (AT_THK *) entk->character ;
			/*チームの装備状況 編成*/
			/*盾兵の人数をカウント*/
			if((entk->act->bodyp.type & ENE_TYPE_SHIELD)
			&&(at_thk->dis_rank<2))
			{
				at_com->shield_num++;
			}
			/* 発砲禁止チェック */
			if(at_thk->at_status &  AT_ST_NO_SHOT ){
				at_com->watch_status |= AT_COM_WATCH_NO_SHOT ;
			}

			/* 共有視界情報 */

			if(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT){
				if( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN )
				{
					sight++;
					at_thk->in_sight++;
				}else {
					at_thk->in_sight = 0;
				}
			}else if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
				if( entk->npc_eyei.sight == EYE_INFO_SIGHT_IN )
				{
					sight++;
					at_thk->in_sight++;
				}else {
					at_thk->in_sight = 0;
				}
			}
			if(
			(at_thk->at_status & AT_ST_FEEL )
			||(at_thk->at_status & AT_ST_NEXT )
			){
				at_com->feel_flag++ ;
			}
			/*バウンディング作成 DisRankCheckの前にやること*/
			MakeAT_Bound(at_com,entk,&flag );
/*********************
気配察知処理 10/29 実験
待機中兵はぼやけ距離内でも察知する
2000.05 気配察知モード作成につき不要になる
**************************/
		}
	}
	if((at_com->com_sight)&&(sight==0)&&(GM_AlertLevel!=ALERT_LEVEL_MAX)){
		se_flag = 1;
	}else {
		se_flag = 0;
	}
	at_com->com_sight = sight;
	if(GM_PlayerStatus & PLAYER_HOLD){
		at_com->pl_weapon = PL_GetPlayerWeapon( ) ;
	}else{
		at_com->pl_weapon = 0;
	}

	DisRankCheck( at_com );

	FormDirCheck( at_com );
	/*首絞めびっくり廃止*/
//	ATCOM_CheckHostage(at_com) ;
	CheckPlayerHide( at_com);
	/*見てないけど知ってる状態*/
	CheckIknowIntrude( at_com);
	CheckDeathBedFlag( at_com ) ;
	AT_LevelSet(at_com);
	if(at_com->berserk > 0 ){
		at_com->berserk-- ;
	}
	if(se_flag){
		ENETHINK	*top_entk ;		
		top_entk= GetAtRanking(at_com->unit[0].entk[0],0) ;
		if((top_entk->act->status & AT_NO_ACTIVE ) 
		||( top_entk->act->bodyp.life <= 0  )){
		}else {
//			SIG_AT_VoiceCall(top_entk,SD_V_ATKO03,AT_V_NIGETAZO) ;
//			SIG_AT_VoiceCall(top_entk,SD_V_ATKO03,180) ;
		}
	}
}


static void AT_ComStartAlert(AT_COM *at_com){
	at_com->think1 = ENE_TH1_ALERT ;
	at_com->berserk = 0 ;
	at_com->rollout = 0 ;
}

static void AT_ComInit(AT_COM *at_com){
	int i,j;
	ENETHINK *entk;
	static	FVECTOR	shift = {80.0F,   0.0F,-120.0F, 0.0F};


	AT_THK	*at_thk ;

	for(i=0;i<ATUNIT_MAX;i++){
		for(j=0;j < ATENEMY_MAX;j++){
			if((i<at_com->group.unit_num)
			&&(j < at_com->unit[i].enemy_num)){
				/****/
				entk = at_com->unit[i].entk[j];
				at_thk = (AT_THK *) entk->character ;
				at_com->teamaddr[i][j] = &entk->ctrl->addr;
				entk->at_com = at_com;
ASSERT(entk->at_com != NULL) ;
				at_thk->dis_rank = entk->id;
/*ムカデ用ＩＫセット*/
				if(j==0){
					entk->buddy 
						= at_com->unit[i].entk[at_com->unit[i].enemy_num-1];
				}else {
					entk->buddy 
						= at_com->unit[i].entk[j-1];

#if 1
//printf("IK MOVE SET\n");
					entk->arm_ik = NULL ; 
					entk->arm_ik = 
					InitArmIK(entk->act->body,HUMAN21_HIDARI_TE,
					entk->buddy->act->body,HUMAN21_HIDARI_KATA,
					&shift);
#endif
				}
/*危険モードで開始なのでワープ*/
			}else {
				at_com->teamaddr[i][j] = NULL;
			}
		}
		at_com->siege[ i ] = 1;
	}
	/* 連絡用 */
	at_com->rollout = 0;	/* 転がりタイミング*/
	at_com->gosign = 0;		/* 号令情報 */

	/* 可変 */
	at_com->esctime = (AT_THK_RATE*3); /* 逃げ開始時間*/
	if(at_com->unit[0].enemy_num > 0 ){
		at_com->chasedis 
		= (at_com->unit[0].entk[0]->sense.eye_s) ;
	}
	 /* 追跡距離可変  視力を標準値にとる*/
	at_com->attack_num = 4;	/*同時攻撃人数*/
	at_com->alert_time	= 0 ; /* 危険モード経過時間 */
	at_com->shoot_delay = 0;	/*発砲ためらい時間*/
	at_com->siege_num = 2;

	at_com->pl_status_old = 0; /*プレイヤ状態バッファ*/
	at_com->watch_status = 0;
	at_com->watch_status_req = 0;
	at_com->Pl_StayTime = 0; /**/
	at_com->OldPlAddr = 0;
#ifdef DEBUG_MODE
	at_com->debug_count = 0;
#endif
	at_com->esc_dis = 6000;
	at_com->player_lastaddr = GM_PlayerAddress ;
	at_com->berserk = 0;
	at_com->surprised = 0;
	at_com->feel_flag = 0 ;
	at_com->now_cov = 0;
	at_com->bound_last = -1 ;
	at_com->called_se_code = -1;
	at_com->called_se_count = 0;
	at_com->called_se_pl_addr =-1 ;
/*危険モードでスタートするため特別処理*/
#if 1
	/*ムカデ廃止？*/
	at_com->level = 1;			/* 攻撃モード */
#else
//	if(GM_StartAlertMode == ALERT_MODE_ALERT){
	if(GM_AlertMode == ALERT_MODE_ALERT){
		at_com->level = 1;			/* 攻撃モード */
	}else {
		at_com->level = 0;			/* 攻撃モード */
	}
#endif
	at_com->at_trg_addr = GM_PlayerAddress ;
}



extern int ENE_ReadOnlinInfo(int ,int);
//static inline int FBoundCheck 
/*座標がpos0<=pos1である事*/
static int	FvecBoundingCheck(FVECTOR *pos0 ,FVECTOR *pos1 ,FVECTOR *trg){
	if(
	(pos0->vx <= trg->vx )&&(trg->vx <= pos1->vx )
	&&(pos0->vy <= trg->vy )&&(trg->vy <= pos1->vy )
	&&(pos0->vz <= trg->vz )&&(trg->vz <= pos1->vz )
	){
		return 1;
	}
	return 0;
}
static void FvecSymmet(FVECTOR *trg,FVECTOR *cent,FVECTOR *res){
	res->vx = cent->vx - (trg->vx - cent->vx) ;
	res->vy = cent->vy - (trg->vy - cent->vy) ;
	res->vz = cent->vz - (trg->vz - cent->vz) ;
}
/*指定点の両サイド座標を取得*/
/****
trg 角 中心座標
pos センター座標
res res[2] であること
****/

static void MakeSidePos(FVECTOR *trg,FVECTOR *pos,FVECTOR *res ,float slide){
	FVECTOR trg0[2],trg1[2],sub;
	SVECTOR	rot;
	int dir  ;

	_sceVu0SubVector( &sub, trg, pos ) ;
	dir = GV_VecDir2( &sub );

	rot.vx = 0;
	rot.vy = dir;
	rot.vz = 0;

	trg0[0] = trg0[1] = DG_ZeroVector ;
	trg0[0].vx =  slide ;
	trg0[1].vx = -slide ;
//	DG_SetPos2( trg, &rot ) ;
	DG_SetPos2( pos, &rot ) ;
	DG_PutVector( trg0, trg1, 2 );
	res[0] =  trg1[0];
	res[1] =  trg1[1];
}
/*SYMMETRY MODE*/
//static void AT_CheckCover(Work *work ){
static void AT_CheckCover(AT_COM *at_com ){
	int i,sym_addr;
//	short	min_size ;
	float	min_size ;

	HZX_ZON		*testzone; /*ゾーン*/
	FVECTOR		testpos ;
//	,sidepos[2];

	AT_COV	*at_cov ;

	at_cov = at_com->at_cov ;

/*イントルード優先*/
	if(GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_BEYOND|PLAYER_CB_BOX)) 
	{
		at_com->watch_status &= ~AT_COM_WATCH_COVER ;
		at_com->cov_addr = -1 ;
		return ;
	}
	if(!(at_com->watch_status & AT_COM_WATCH_COVER)){
		/*フラグが立ってない時は条件に視界を追加*/
		if(at_com->com_sight == 0){
			return ;
		}
	}

	for(i=0;i<at_com->cov_num;i++){
		if(at_cov[i].cov_map != GM_PlayerMap ){
			continue ;
		}
		if(FvecBoundingCheck(&at_cov[i].cov_box[0],
			&at_cov[i].cov_box[1],&GM_PlayerPosition))
		{
			u_char	r,g,b;
			r = 0; g = 255; b = 0;
			GM_CurrentMap = GM_PlayerMap;
			FvecSymmet(&GM_PlayerPosition,&at_cov[i].cov_cent_pos,
			&at_com->cov_pos) ;
			at_com->cov_pos.vy = at_cov[i].cov_box[0].vy ;
#if 0
	/*対称ゾーンを更新*/
			at_com->cov_addr = 
				HZX_GetAddress( GM_PlayerControl->hzx_id,
				&at_com->cov_pos, at_com->cov_addr ) ;
#else
	/*前の配置から隣接してないとこまで動いたら更新に変更*/
			sym_addr = 
				HZX_GetAddress( GM_PlayerControl->hzx_id,
				&at_com->cov_pos, at_com->cov_addr ) ;
			if(HZX_ReachTo(sym_addr,at_com->cov_addr)>= HZX_UNREACH){
				at_com->cov_addr = sym_addr ;
			}
#endif
			if(HZX_Zone1(at_com->cov_addr) == HZX_NO_ZONE){
				/*目標がおかしいので解除*/
				if(at_com->watch_status & AT_COM_WATCH_COVER ){
					at_com->side_pos[0] = GM_PlayerPosition ;
					at_com->side_pos[1] = GM_PlayerPosition ;
				}
				at_com->watch_status &= ~AT_COM_WATCH_COVER ;
				at_com->cov_addr = -1 ;
				return ;
			}

			testzone = ENE_HZX_GetZone(at_com->cov_addr);
			/*目標が進入禁止だ*/
			if(testzone->flag & (HZX_ZONE_ZINTRPT|HZX_ZONE_INTRUDE)){
				/*ここに到達した時はカバー解除*/
				if(at_com->watch_status & AT_COM_WATCH_COVER ){
					at_com->side_pos[0] = GM_PlayerPosition ;
					at_com->side_pos[1] = GM_PlayerPosition ;
				}
				at_com->watch_status &= ~AT_COM_WATCH_COVER ;
				at_com->cov_addr = -1 ;
				return ;
			}
			testpos.vx = testzone->x ;
			testpos.vy = testzone->y ;
			testpos.vz = testzone->z ;
			min_size = testzone->w ;
			if(min_size > testzone->h ) min_size = testzone->h ;
			min_size -= 250.0f ;
		//MakeSidePos(&GM_PlayerPosition,&testpos,at_com->side_pos,500.0F);
			MakeSidePos(&GM_PlayerPosition,&testpos,at_com->side_pos,min_size );
			at_com->watch_status |= AT_COM_WATCH_COVER ;
			at_com->now_cov = i;
			return ;
		}
	}
	/*ここに到達した時はカバー解除*/
	if(at_com->watch_status & AT_COM_WATCH_COVER ){
		at_com->side_pos[0] = GM_PlayerPosition ;
		at_com->side_pos[1] = GM_PlayerPosition ;
	}
	at_com->watch_status &= ~AT_COM_WATCH_COVER ;
	at_com->cov_addr = -1 ;
}
static int MapAddrSet(int addr,int map){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(map)<<16));
	return addr;
}
//static void Act( Work *work )
static void AT_COM_Act( Work *work )
{
	int clear;
//	int l_map;
//	FVECTOR	l_pos;
//	SVECTOR	col;
	AT_COM	*at_com ;
	at_com = &work->at_com ;






	if(at_com->called_se_count>0)	at_com->called_se_count--;

	/*メッセージ処理*/
	AT_COM_CheckMessage( work );

//printf("PL HZX_ID 0x%x\n",GM_PlayerControl->hzx_id);
//printf("PL CTRL_MAP 0x%x\n",GM_PlayerControl->map);
//printf("PL MAP 0x%x\n",GM_PlayerMap);
//printf("PL LAST_MAP 0x%x\n",at_com->com->player_lastmap );

//printf("PL MAP 0x%x\n",HZX_ZoneMapNo(GM_PlayerAddress));
//printf("PL HZX_ID 0x%x\n",GM_PlayerControl->hzx_id);
//printf("No2Bit 0x%x\n",GV_GetBit(HZX_ZoneMapNo(GM_PlayerAddress)));

/*debug*/
/****/
#if 1
	/*カバーエリアチェック*/
	if(work->at_com.cov_num) AT_CheckCover( at_com ) ;
#endif

	switch( at_com->think1 ){
		case	ENE_TH1_SNEAK:
			if ( GM_AlertMode == ALERT_MODE_ALERT ) {
				AT_ComStartAlert(at_com) ;
			}
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				at_com->think1 = ENE_TH1_AVOID ;
				SetAvoidMode( at_com ) ;
			}
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
		break ;
		case	ENE_TH1_ALERT:
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ChangeRouteCourse( HZX_ROOT_COURSE1 ) ;
			}
			AT_ComRenew( at_com );
			if(GM_GameStatus & STATE_VR_ONLY){
				HZX_ClearRouteCourse( 0 ) ;
			}
			if ( GM_AlertMode == ALERT_MODE_AVOID ) {
				at_com->think1 = ENE_TH1_AVOID ;
				SetAvoidMode( at_com ) ;
			}
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
		break ;
		case	ENE_TH1_AVOID:
/*回避モード中 兵士間距離チェックのみ行う*/
			DisRankCheck( at_com ) ;
			AT_LevelSet(at_com);
			Avoid( at_com ) ;
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
			if ( GM_AlertMode == ALERT_MODE_ALERT ) {
				at_com->think1 = ENE_TH1_ALERT ;
				AT_ComStartAlert(at_com) ;
				/*トイレ隠れを発見*/
				if(
				(ENE_GameStatus & ENE_GMSTATUS_TOILET)
				&&(GM_PlayerStatus & PLAYER_LOCKER)
				&&(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)
				){
					at_com->watch_status |= AT_COM_WATCH_LOCKER ;
				}
			}
			if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
				at_com->think1 = ENE_TH1_SNEAK ;
			}
		break ;
	}

/*after proc*/
	if(at_com->wait_cnt > 0) at_com->wait_cnt--;
	if(at_com->think1 != ENE_TH1_AVOID){ 
		at_com->avoid_wait_cnt = 0 ;
	}else {
		if(at_com->avoid_wait_cnt>0) at_com->avoid_wait_cnt-- ;
	}
	at_com->pl_status_old = GM_PlayerStatus;
	if(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE){
		/*攻撃兵にはイントルードフラグゾーンとトラップが等価なので判定バッファにフラグを立てる*/
		at_com->pl_status_old |= PLAYER_INTRUDE;
	} 

	/*回避モード以外で使わないフラグを寝かせる*/
	clear =(AT_COM_WATCH_LAST|AT_COM_WATCH_SEARCH);
	if(at_com->think1 != ENE_TH1_AVOID){ 
		at_com->watch_status &= ~clear ;
	}
	if(at_com->think1 != ENE_TH1_ALERT){ 
		at_com->bound_last = -1 ;
	}


#if 0
	{
//		extern void SigZoneView( int ,SVECTOR * ,float size);
//		SVECTOR rgb ; 
//		rgb.vx = 255 ; 
//		rgb.vy = 0 ; 
//		rgb.vz = 0 ; 
		DEBUG_Locate(20,340,0) ;
		DEBUG_Printf("watch_status[%x] \n",at_com->watch_status) ;
//		SigZoneView(at_com->player_lastaddr,&rgb,100.0F) ; 
	}
#endif





}
int AT_COM_GetRetreatAddr(ENETHINK *entk){
	COMMANDER *com ;
	AT_COM *at_com ;

	com = entk->com ;
//	at_com = com->at_com ;
	at_com = entk->at_com ;
	ASSERT(at_com != NULL) ;
	if(at_com->bound_last == -1) return -1 ;
	if(FvecBoundingCheck(&at_com->at_bound[0] ,&at_com->at_bound[1] ,&entk->ctrl->mov)){
		ENETHINK	*at_entk;
		int rout=0 ,addr;
		at_entk = at_com->unit[0].entk[at_com->bound_last] ;

		addr = HZX_FarZoneNavigate( entk->ctrl->hzx_id,HZX_Zone1(at_entk->ctrl->addr),
			HZX_Zone1(GM_PlayerAddress),&rout);
		return MapAddrSet(addr,at_entk->ctrl->addr) ;
	}
	return -1 ;
}
/*---------------------------------------------------------------*/
static void Die( Work *work )
{
//	HZX_FreeOnlineInfo( HZX_CurrentHzx->def ) ;

	if(work->at_com.at_cov != NULL) GV_Free(work->at_com.at_cov) ;
}
/*---------------------------------------------------------------*/
static int AttackerUnitCall( Work *work, char *top )
{
	int i, id;
	char *p ;
	AT_COM *at_com ;
	at_com = &work->at_com ;
	for( i = 0; GCL_NextStr() != NULL; i++ ){
		if ( i >= ATUNIT_MAX ) {
			printf("ATUNIT_MAX OVER!!\n");
			return (-1) ;
		}
		if ( COM_SetUnit( &at_com->group, &at_com->unit[i] ) < 0 ) {
			printf("MAX_UNIT OVER!!\n");
			return (-1) ;
		}
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
		GCL_SetNextStr( p );
		at_com->clearing[i].unit = &at_com->unit[i] ;
		at_com->clearing[i].hzx = at_com->com->hzx ;
	}

	return i;
}
static void SetCentFvec(FVECTOR *pos0,FVECTOR *pos1,FVECTOR *res){
	res->vx = (pos0->vx + pos1->vx) / 2.0F ;
	res->vy = (pos0->vy + pos1->vy) / 2.0F ;
	res->vz = (pos0->vz + pos1->vz) / 2.0F ;
}
int AT_COM_GetResources( Work *work, int where ,int name)
{
	char	*opt ;
	int i,j,buf[3] ;
	CONTROL		ctrl;
	AT_COV *at_cov = NULL;
	AT_COM *at_com ;

extern 	void	HZX_MakeOnlineInfo( HZX_DEF * );

	work->at_com.at_cov = NULL;
	at_com = &work->at_com ;
	work->name = name ;
	at_com->com = COM_GetCommander( ) ;
//	at_com->com->at_com = at_com ;

	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		at_com->npc_ctrl = (CONTROL *) EMA_CommandGetPosition() ;
	}

	/*盾兵士人数*/
	at_com->shield_max = GCL_GetOptionValue( 's', 0 );
	at_com->shield_num = 0;
	if ( COM_SetGroup( &at_com->com->enemys, &at_com->group ) < 0 ) {
		return (-1) ;
	}
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		if ( AttackerUnitCall( work, opt ) < 0 ) return (-1) ;
	}

	switch(GM_AlertMode){
		case	ALERT_MODE_SNEAK : /* 潜入モード */
			at_com->think1 = ENE_TH1_SNEAK ;
			break;
		case	ALERT_MODE_ALERT : /* 危険モード */
			at_com->think1 = ENE_TH1_ALERT ;
			break;
		case	ALERT_MODE_AVOID : /* 回避モード */
			SetAvoidMode( at_com ) ;
			at_com->think1 = ENE_TH1_AVOID ;
			break;
		case	ALERT_MODE_SEARCH : /* 探索モード */
			at_com->think1 = ENE_TH1_SNEAK ;
			break;
		default :
			at_com->think1 = ENE_TH1_SNEAK ;
			break;
	}

	/*待機時間設定*/
	if ( ( opt = GCL_GetOption( 'w' ) ) != NULL ){
		at_com->wait_cnt = DIRECT_TICK(GCL_GetNextInt()) ;
	}else{
		at_com->wait_cnt = AT_WAIT_COUNT ;
	}
	at_com->avoid_wait_cnt = 0 ;
	if(GM_AlertMode == ALERT_MODE_AVOID){
		if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){
			at_com->avoid_wait_cnt = DIRECT_TICK(GCL_GetNextInt()) ;
		}
	}
	/*ステータスセット*/
	at_com->scn_status = GCL_GetOptionValue( 's', 0 );

/*カバーモード*/
	work->at_com.cov_num = 0;
	work->at_com.cov_pos = DG_ZeroVector ;
	work->at_com.side_pos[0] = DG_ZeroVector ;
	work->at_com.side_pos[1] = DG_ZeroVector ;

	work->at_com.cov_addr = -1 ;

	if ( GCL_GetOption( 'c' ) ){
		if( GCL_NextStr() != NULL ){
			/*エリア数*/
			work->at_com.cov_num = GCL_GetNextInt( ) ;
			at_cov = GV_Malloc( sizeof(AT_COV) * work->at_com.cov_num );
			if(at_cov == NULL){
				printf("AT_COM NO MEMORY!!\n");
				return -1 ;
			}
			for(j=0;j<work->at_com.cov_num;j++){
				if( GCL_NextStr() != NULL ){
					for(i=0;i<2;i++){
						if( GCL_NextStr() != NULL ){
							GCL_GetIV( GCL_NextStr(), buf ) ;
							vu0_IV0toFV( (IVECTOR *)buf, 
							&at_cov[j].cov_box[i] ) ;
						}else {
							return -1 ;
						}
					}
					SetCentFvec(&at_cov[j].cov_box[0],
						&at_cov[j].cov_box[1],
						&at_cov[j].cov_cent_pos );
					ctrl.mov = at_cov[j].cov_cent_pos ;
					GM_ConfigControlMapID( &ctrl ) ;
					at_cov[j].cov_map = ctrl.map ;
//					work->at_com.cov_num++;
				}else {
					printf("COVER AREA TOO FEW\n");
					return -1 ;
				}
			}
		}
		work->at_com.at_cov = at_cov ;
	}
#ifdef DEBUG_MODE
	/*ビデオ録り用 嘘モードスィッチ*/
	work->at_com.video = GCL_GetOptionValue( 'v', 0 );
#endif

	AT_ComInit(at_com);
	/*TEST*/

	/* ゾーンアドレス */

	/*視界情報上書き*/
//	HZX_MakeOnlineInfo( HZX_CurrentHzx->def ) ;

	return 0 ;
}

/*---------------------------------------------------------------*/
void *NewAtCommander( name, where )
int	name ;
int	where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), SUBCOMM_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), AT_COM_Act, Die ) ;
		if ( AT_COM_GetResources( work, where ,name) < 0 ) {
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

