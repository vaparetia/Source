//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
攻撃兵専用は別に移動予定
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
#include	"libutl.h"

#include	"attacker.h"
#include	"sig_conv.x"

//#include	"../tng/tng.h"


extern float _RsinF( int ) ;
extern float _RcosF( int ) ;
extern float GV_VecLen3F2( FVECTOR *,FVECTOR * ) ;

extern CONTROL	*EMA_CommandGetControl(void) ;


int	SIG_CheckDirSub(int ,int ) ;
int SIG_GetFarZone3(int ,int ,int ,int) ;
int SIG_EyeInsideCheck( SENSEPARAM *,ENETHINK * ,FVECTOR *) ;

/* ステルス無効になる条件*/
#define PL_NO_STEALTH_STATUS (PLAYER_BLOOD_DROP|PLAYER_ENEMY_PULL|PLAYER_ENEMY_HANG)

SNC_EXTERN_INLINE int SIG_CheckStealthStatus(ENETHINK *entk){
#if 1
	return ((!(entk->status &ENE_STATUS_NO_STEALTH))&&(GM_PlayerStatus & PLAYER_STEALTH)
//	&&(!(GM_PlayerStatus & PL_NO_STEALTH_STATUS))
	) ;
#else 
	return 0 ;
#endif
}

static int MapAddrSet(int addr,int map){
	addr = HZX_Zone1(addr);
	addr = (addr|(addr<<8)|(HZX_ZoneMapNo(map)<<16));
	return addr;
}


/*同じゾーンなら０ 隣なら１*/
int Sig_GetRoute(hzx_id,addr1,addr2)
HZX_GROUP_ID hzx_id;
int addr1;
int addr2;
{
	int route;
	if(addr1 == addr2 ){
		route = 0;
	}else {
		if(HZX_ZoneMapNo(addr1) == HZX_ZoneMapNo(addr2)){
			/*同じマップ内*/
			route = HZX_GetRoute( hzx_id ,
			HZX_Zone1(addr1),HZX_Zone2(addr2) ) ;
		}else {
			route = HZX_GetRouteCrossGroup( addr1,addr2 ) ;
		} 
	}
	return route;
}


/*ゾーンが画面内に入ってるかチェック*/
int SIG_ZoneBoundCheck(HZX_ZON *zone,float h){
    FMATRIX	m ;
    FVECTOR	b_min, b_max ,pos;
	pos.vx = zone->x ;
	pos.vy = zone->y ;
	pos.vz = zone->z ;

	m = DG_UnitMatrix ;
	m.m[3][0] = pos.vx ;
	m.m[3][1] = pos.vy ;
	m.m[3][2] = pos.vz ;

	b_min.vx = - (float)zone->w ;
	b_min.vy = 0.0f ;
	b_min.vz = - (float)zone->h ;

	b_max.vx = (float)zone->w ;
	b_max.vy = h ;
	b_max.vz = (float)zone->h ;

	return DG_BoundCheck( &m, &b_max, &b_min ) ;
}
int SIG_BoundCheck(FVECTOR *base, float w,float h){
    FMATRIX	m ;
    FVECTOR	b_min, b_max ,pos;
	pos.vx = base->vx ;
	pos.vy = base->vy ;
	pos.vz = base->vz ;

	m = DG_UnitMatrix ;
	m.m[3][0] = pos.vx ;
	m.m[3][1] = pos.vy ;
	m.m[3][2] = pos.vz ;

	b_min.vx = - w ;
	b_min.vy = 0.0f ;
	b_min.vz = - w ;

	b_max.vx = w ;
	b_max.vy = h ;
	b_max.vz = w ;

	return DG_BoundCheck( &m, &b_max, &b_min ) ;
}

/*任意のゾーンの安全度*/
int SIG_CheckSafeType(ENETHINK *entk,int addr)
{
	int i,safeaddr,addr1;
	HZX_ZON *chk_zone ;

	if(HZX_ZoneMapNo(entk->ctrl->addr) != HZX_ZoneMapNo(addr)){
		return 0;
	}
	chk_zone = ENE_HZX_GetZone(entk->ctrl->addr);
	addr1 = HZX_Zone1(addr);

	for(i=0;i<HZX_MAX_SAFEZONE_NUM;i++){
		safeaddr = chk_zone->safes[i];
		if(safeaddr == HZX_NO_ZONE) {
			return 0;
		}
		if(safeaddr != addr1){
			continue ;
		}
		return chk_zone->safe_types[i];
//		if(chk_zone->safe_types[i]&SAFE_LOW)
	}
	return 0;
}


/*
座標がゾーンの範囲内か調べる
許容範囲 wide
*/
int Sig_InsideZone( zone, pos ,wide )
HZX_ZON		*zone ;		/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
float		wide ;		/* 追加チェック幅*/
{
	float		value, center, width ;
	
	value = pos->vx ;
	center = zone->x ;	width = zone->w + wide ;
	if ( value < center - width || value > center + width ) return 0 ;
	value = pos->vz ;
	center = zone->z ;	width = zone->h  + wide;
	if ( value < center - width || value > center + width ) return 0 ;
	return 1 ;
}
#if 1
void Sig_SetTrgpInsideZone(entk)
ENETHINK	*entk ;		/* ゾーン構造体		*/
{
	float		value, center, width ;
	FVECTOR		trg_in_zone ;
	HZX_ZON		*zone ;		/* ゾーン構造体		*/

	zone = ENE_HZX_GetZone(entk->ctrl->addr);
	trg_in_zone = entk->trgpoint.pos ;

	value = entk->trgpoint.pos.vx ;
	center = zone->x ;	width = zone->w  ;
	if ( value < (center - width) ) {
		trg_in_zone.vx = (center - width) ;
	}else if ( value > (center + width) ) {
		trg_in_zone.vx = (center + width) ;
	}
	value = entk->trgpoint.pos.vz ;
	center = zone->z ;	width = zone->h  ;
	if ( value < (center - width) ){
		trg_in_zone.vz = (center - width) ;
	}else if( value > (center + width) ){
		trg_in_zone.vz = (center + width) ;
	}
	trg_in_zone.vy = entk->ctrl->mov.vy ;
	entk->trgpoint.pos = trg_in_zone ;
}
#endif
/*座標がゾーンの範囲内か*/
int Sig_InsideZoneWide( zone, pos ,wide )
HZX_ZON		*zone ;		/* ゾーン構造体		*/
FVECTOR		*pos ;		/* 座標			*/
float		wide ;		/* 追加チェック幅*/
{
	float		value, center, width ;
	
	value = pos->vx ;
	center = zone->x ;	width = zone->w + wide ;
	if ( (value >= (center - width)) && (value <= (center + width)) ) return 1 ;
	value = pos->vz ;
	center = zone->z ;	width = zone->h  + wide;
	if ( (value >= (center - width)) && (value <= (center + width)) ) return 1 ;
	return 0 ;
}
int Sig_InsideZoneWide2( ENETHINK *entk , float wide )
{
	float		value, center, width ;
	HZX_ZON *hzx_zone ;
	hzx_zone = ENE_HZX_GetZone(entk->ctrl->addr) ;
	
	value = entk->ctrl->mov.vx ;
	center = hzx_zone->x ;	
	width = hzx_zone->w + wide ;
	if ( (value >= (center - width)) && (value <= (center + width)) ){
	}else {
		return 0 ;
	}
	value = entk->ctrl->mov.vz ;
	center = hzx_zone->z ;	width = hzx_zone->h  + wide;
	if ( (value >= (center - width)) && (value <= (center + width)) ) {
	}else {
		return 0 ;
	}
	return 1 ;
}

int SIG_GetRIntrptZ2Z( HZX_GROUP_ID id, int addr1, int addr2 ){
	int map,nowaddr,trgaddr ;
	map = GV_GetNo( id ) ;
	nowaddr = (addr1|(addr1<<8)|(map<<16));
	trgaddr = (addr2|(addr2<<8)|(map<<16));
	if(GM_GetRIntrptZ2Z(trgaddr,nowaddr) != NULL) return 1 ;
	return 0 ;
}

/*ＦＶＥＣにゾーンの中心座標をセット*/
void SIG_SetZonePos2Fvec(HZX_ZON *hzx_zone ,FVECTOR *fvec){
	fvec->vx = (float) hzx_zone->x ;
	fvec->vy = (float) hzx_zone->y ;
	fvec->vz = (float) hzx_zone->z ;
}

void SIG_SetChasePos(ENETHINK *entk ,FVECTOR *pos)
{
	CONTROL	ctrl;
	/*目標地点セット*/
#ifdef BP_PS2
	(FVECTOR) entk->trgpoint.pos = *pos ;
	(FVECTOR) ctrl.mov =  *pos ;
#else
	entk->trgpoint.pos = *pos ;
	ctrl.mov =  *pos ;
#endif
	GM_ConfigControlMapID( &ctrl ) ;

	entk->trgpoint.addr = ctrl.addr ;
	entk->trgpoint.map = ctrl.map ;
//	entk->znavi->going_addr = ctrl.addr ;	/* 最終目標アドレス	*/
//	entk->znavi->going_map = ctrl.map ;	/* 最終目標マップ */

}

/*麻酔寝？*/
int CheckAnesAT(ENETHINK *entk){
	AT_THK		*at_thk ;
	at_thk = (AT_THK *) entk->character ;

//	return 0 ;
//
//	if(entk->act->bodyp.anesthesia != entk->act->bodyp.m_anesthesia){
	if(
	(entk->act->bodyp.anesthesia == -1)
	&&(entk->act->status & ACT_STATUS_FAINT)
	&&(!(entk->act->status & (ACT_STATUS_LOCKER|ACT_STATUS_CAPTURE)))
	&&(!(entk->status & ENE_STATUS_EVER_ZZZ))
	){
		return 1 ;
	}
	return 0 ;
}
/*麻酔寝 あるいは気絶*/
int CheckNoActive(ENETHINK *entk){
	if(
	(entk->act->bodyp.anesthesia < 0)
	||(entk->act->bodyp.faint <0)
	){
		return 1 ;
	}
	return 0 ;
}

int CheckLastAT(ENETHINK *entk){
	AT_THK		*at_thk ;
	at_thk = (AT_THK *) entk->character ;
	if(at_thk->at_status & AT_ST_LAST){
		return 1 ;
	}
	return 0 ;
}
#if 1
/*自分の隊に救助必要な人がいるか？*/
int CheckRescueAll(ENETHINK *entk ){
	AT_THK		*at_thk,*trg_atthk ;
	E_UNIT		*e_unit ;
	ENETHINK	*trgentk;
	int i ;
	SVECTOR		rgb;

	rgb.vx = 0;
	rgb.vy = 127;
	rgb.vz = 0;
	
	at_thk = (AT_THK *) entk->character ;
	e_unit = entk->com->enemys.group[entk->g_id]->unit[entk->u_id] ;
	
	for(i=0;i<e_unit->enemy_num;i++){
		if(i==entk->id) {
			/*自分*/
			continue ;
		}
		trgentk = e_unit->entk[i];
		trg_atthk = (AT_THK *) trgentk->character ;
		if(trg_atthk->at_status & AT_ST_WAKE ){
			/*救助派遣済み*/
			continue ;
		}
		if(CheckAnesAT(trgentk)){
			if(!(SIG_EyeInsideCheck( &entk->sense ,entk ,&trgentk->ctrl->mov))){
				/*視界範囲外*/
				continue ;
			}
			if(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &entk->ctrl->mov,&trgentk->ctrl->mov ) ){
				/*壁がある*/
				continue ;
			}
			/*要救助*/
			trg_atthk->at_status |= AT_ST_WAKE ;
			entk->think2 = TH2_RESCUE ; 
			entk->think3 = TH3_ZONE_POS ; 
			entk->count3 = 0 ; 
			SIG_SetChasePos(entk ,&trgentk->ctrl->mov);
printf("RESCUE BUDDY SET!!!!\n");
			entk->buddy = trgentk ;
			/*相互連絡*/
			entk->buddy->buddy = entk ;

//			PosBox(&entk->ctrl->mov ,250.0F , NULL );
//			PosBox(&trgentk->ctrl->mov ,250.0F , &rgb );
			return 1;
		}
	}
	return 0;
}
#endif

#if 0
static	int	SIG_ZoneIntrptCheck( HZX_ZONE_ADD this )
{
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( this ) ;
	if(z == NULL)  return 1;
	if ( z->flag & (HZX_ZONE_ZINTRPT|HZX_ZONE_INTRUDE) ) return 1;
	return 0 ;
}
#endif

int SIG_CheckRIntrpt(ENETHINK *entk){
	FVECTOR		zone_pos;
	R_INTRPT	*intrpt ;
	if( ENE_ZoneIntrptCheck( entk->znavi->next_addr ) ) return 1 ;
	intrpt = GM_GetRIntrpt( entk->ctrl->addr,entk->znavi->next_addr ) ;
	if(intrpt != NULL ){
#if 1
		HZX_ZON		*hzx_zone ;
		if((intrpt->status & ROOT_INTRPT_OPEN)&&(intrpt->status & ROOT_INTRPT_ELV) ) return 0 ;
		if( intrpt->status & ROOT_INTRPT_JUMP ) return 0 ;
		if((intrpt->status & ROOT_INTRPT_AUTO)&&!(intrpt->status & ROOT_INTRPT_NOKEY) ) return 0 ;
		if((intrpt->status & ROOT_INTRPT_HINGED)&&!(intrpt->status&ROOT_INTRPT_CLOSE) ) return 0 ;
		hzx_zone = ENE_HZX_GetZone(entk->ctrl->addr) ;
		SIG_SetZonePos2Fvec(hzx_zone,&zone_pos) ;
		SIG_SetChasePos(entk ,&zone_pos) ;
		return 1 ;
#else
		if(
		(!(intrpt->status & ROOT_INTRPT_AUTO))
		&&(!(intrpt->status & ROOT_INTRPT_JUMP))
		){
			HZX_ZON		*hzx_zone ;
			hzx_zone = ENE_HZX_GetZone(entk->ctrl->addr) ;
			SIG_SetZonePos2Fvec(hzx_zone,&zone_pos) ;
			SIG_SetChasePos(entk ,&zone_pos) ;
			return 1 ;
		}
#endif
	}
	return 0 ;
}
/*進入禁止または敵兵突破不可能なゾーン間フラグを見つけた*/
int CheckOnlyRIntrpt(ENETHINK *entk){
//	FVECTOR		zone_pos;
	R_INTRPT	*intrpt ;
	if( ENE_ZoneIntrptCheck( entk->znavi->next_addr ) ) return 1 ;
	intrpt = GM_GetRIntrpt( entk->ctrl->addr,entk->znavi->next_addr ) ;
	if(intrpt != NULL ){
#if 1
		if((intrpt->status & ROOT_INTRPT_OPEN)&&(intrpt->status & ROOT_INTRPT_ELV) ) return 0 ;
		if( intrpt->status & ROOT_INTRPT_JUMP ) return 0 ;
		if((intrpt->status & ROOT_INTRPT_AUTO)&&!(intrpt->status & ROOT_INTRPT_NOKEY) ) return 0 ;
		if((intrpt->status & ROOT_INTRPT_HINGED)&&!(intrpt->status&ROOT_INTRPT_CLOSE) ) return 0 ;
		return 1 ;
#else
		if((!(intrpt->status & ROOT_INTRPT_AUTO))
		&&(!(intrpt->status & ROOT_INTRPT_JUMP))
		){
			return 1 ;
		}
#endif
	}
	return 0 ;
}

/*一時目標がジャンプ先なら情報取得
FVECTOR *res に踏み切り座標をセット
res->vwを方向として使用
res2->vwを水平距離として使用
*/
//#define JUMP_POS_DECAY (500.0F) /*少し目標手前に着地*/
#if 0
static int CheckJumpPos( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr,FVECTOR *res1 ,FVECTOR *res2){
	HZX_ZON		*hzx_zone ;
	FVECTOR		tmp ;
	R_INTRPT	*r_intrpt ;
	*res1 = DG_ZeroVector ;
/*
	if(from_addr == to_addr ) {
printf("SIG_CONV LINE %d\n",__LINE__);
		return 0;
	}
*/
	/*別マップ*/
	if( HZX_ZoneMapNo( from_addr ) != HZX_ZoneMapNo( to_addr ) ){
//		return 1 ;
		return 0;
	} 
//	r_intrpt = GM_GetRIntrptZ2Z( from_addr,  to_addr ) ;
	r_intrpt = GM_GetRIntrpt( from_addr,  to_addr ) ;
	if(r_intrpt == NULL) return 0 ;
	if(!(r_intrpt->status & ROOT_INTRPT_JUMP)) {
		return 0 ;
	}
	if(r_intrpt->zone1 == from_addr){
		*res1 = r_intrpt->pos ;
		res1->vw = (float) r_intrpt->dir ;
		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone2) ;

		res2->vx = (float)(hzx_zone->x) ;
		res2->vy = (float)(hzx_zone->y) ;
		res2->vz = (float)(hzx_zone->z) ;

		_sceVu0SubVector( &tmp ,res1 , res2 ) ;
		tmp.vy = 0.0F ;
		res2->vw = GV_VecLen3F(&tmp) ;
		return 1;
	}else {
		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone2) ;
		res1->vx = (float)(hzx_zone->x) ;
		res1->vy = (float)(hzx_zone->y) ;
		res1->vz = (float)(hzx_zone->z) ;
		res1->vw = (float) ((r_intrpt->dir+2048)&4095) ;

		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone1) ;

		res2->vx = (float)(hzx_zone->x) ;
		res2->vy = (float)(hzx_zone->y) ;
		res2->vz = (float)(hzx_zone->z) ;

		_sceVu0SubVector( &tmp ,res1 , res2 ) ;
		tmp.vy = 0.0F ;
		res2->vw = GV_VecLen3F(&tmp) ;
		return 1;
	}
}
#endif
#if 0
/*現在地と最終目標からジャンプ場所判定*/
static int CheckJumpPosOne( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD going_addr,FVECTOR *res1 ,FVECTOR *res2){
	HZX_ZON		*hzx_zone ;
	FVECTOR		tmp ;
	R_INTRPT	*r_intrpt ;
	*res1 = DG_ZeroVector ;

	r_intrpt = GM_GetRIntrptOne( from_addr ) ;

	if(r_intrpt == NULL) return 0 ;
	if(!(r_intrpt->status & ROOT_INTRPT_JUMP)) {
		return 0 ;
	}
	if(r_intrpt->zone1 == from_addr){
		*res1 = r_intrpt->pos ;
		res1->vw = (float) r_intrpt->dir ;
		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone2) ;

		res2->vx = (float)(hzx_zone->x) ;
		res2->vy = (float)(hzx_zone->y) ;
		res2->vz = (float)(hzx_zone->z) ;

		_sceVu0SubVector( &tmp ,res1 , res2 ) ;
		tmp.vy = 0.0F ;
		res2->vw = GV_VecLen3F(&tmp) ;
		return 1;
	}else {
		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone2) ;
		res1->vx = (float)(hzx_zone->x) ;
		res1->vy = (float)(hzx_zone->y) ;
		res1->vz = (float)(hzx_zone->z) ;
		res1->vw = (float) ((r_intrpt->dir+2048)&4095) ;

		hzx_zone = ENE_HZX_GetZone(r_intrpt->zone1) ;

		res2->vx = (float)(hzx_zone->x) ;
		res2->vy = (float)(hzx_zone->y) ;
		res2->vz = (float)(hzx_zone->z) ;

		_sceVu0SubVector( &tmp ,res1 , res2 ) ;
		tmp.vy = 0.0F ;
		res2->vw = GV_VecLen3F(&tmp) ;
		return 1;
	}
}
#endif


static int CheckJumpPos3( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr ){
	HZX_ZON		*zone ;
	int i ;

	if( HZX_ZoneMapNo( from_addr ) != HZX_ZoneMapNo( to_addr ) ){
		return 0;
	} 
	zone = ENE_HZX_GetZone(from_addr) ;
	for(i=0;i<6;i++){
		if(zone->nears[i] != HZX_Zone1(to_addr) ){
			continue ;
		}
		if(zone->near_flag[i] & HZX_ROOT_JUMP){
			return 1;
		}
	}
	return 0;
}

int CheckJumpPosENE( ENETHINK *entk ){
	HZX_ZON		*zone ;
	FVECTOR		tmp ;
//		PosBox(&entk->act->target_pos,250.0F ,NULL) ;
#if 0
	return CheckJumpPos( entk->ctrl->addr, entk->znavi->next_addr, &entk->tmp_pos,&entk->act->target_pos) ;
#else
	if(CheckJumpPos3( entk->ctrl->addr, entk->znavi->next_addr )){
//printf("CheckJumpPos!!!!!\n");
		entk->tmp_pos = entk->znavi->flore_pos ;
		zone = ENE_HZX_GetZone(entk->znavi->next_addr) ;
		entk->act->target_pos.vx = (float)zone->x ;
		entk->act->target_pos.vy = (float)zone->y ;
		entk->act->target_pos.vz = (float)zone->z ;

		entk->tmp_pos.vw = 
		(float) GV_VecDir2FromTo( &entk->tmp_pos,&entk->act->target_pos ) ;
//tmp
		_sceVu0SubVector( &tmp ,&entk->tmp_pos ,&entk->act->target_pos) ;
		tmp.vy = 0.0F ;
		entk->act->target_pos.vw = GV_VecLen3F(&tmp) ;
		return 1 ;
	}
	return 0 ;
#endif
}


#define	SIDE_DIR	512
//#define LIM_DIR (64)
//#define LIM_DIR (128)
#define LIM_DIR (64)

//static void SetMoveMode(entk)
/**
entk->pl_eyei.dir	のほうを向きながら
entk->trgpoint.dirへ向かうのに適したモーションモードをセットする
entk->act->aim_pos = GM_PlayerFindPos で固定
if(act->tmp_time > 0)　なら静止　自動インクリ
**/
void SetMoveMode(entk)
ENETHINK *entk;
{
	int subdir,move_s;

	move_s = entk->act->move_s ;
	entk->act->tmp_dir = entk->trgpoint.dir ;

	if(SIG_CheckStealthStatus(entk)){
		/*ステルス有効*/
		entk->act->move_s = MoveAttackRun;
		return ;
	}

//	entk->status2 |= ENE_STATUS2_AIM_GUNSHOOT ;
	entk->status2 |= ENE_STATUS2_AIM_GUN ;

//	shotpos = GM_PlayerPosition;
	entk->act->aim_pos = GM_PlayerFindPos ;

	/*最終目標と一時目標の角度差を求める*/
	/*境界線際では前フレームの結果優先*/
	subdir = SIG_CheckDirSub(entk->pl_eyei.dir,entk->trgpoint.dir);
	if(((1024-SIDE_DIR)<subdir)&&(subdir<(1024+SIDE_DIR))){
		/*ＬＥＦＴ*/
		if(
		(((1024-SIDE_DIR+LIM_DIR)>subdir)
		&&(move_s == MoveCautionWalk))
		||
		(((1024+SIDE_DIR-LIM_DIR)<subdir)
		&&(move_s == MoveBack)))
		{
			return ;
		}
		move_s = MoveSideL ;
	}else if(((-1024+SIDE_DIR)>subdir)&&(subdir>(-1024-SIDE_DIR))){
		/*ＲＩＧＨＴ*/
		if(
		(((-1024+SIDE_DIR-LIM_DIR)<subdir)
		&&(move_s == MoveCautionWalk))
		||
		(((-1024-SIDE_DIR+LIM_DIR)>subdir)
		&&(move_s == MoveBack)))
		return ;
		move_s = MoveSideR ;
	}else if(((-1024-SIDE_DIR) >= subdir)
	|| ( (1024+SIDE_DIR) <= subdir )){
		/*BACK*/

		if(
		(((1024+SIDE_DIR+LIM_DIR)>subdir)&&(subdir>0)
		&&(move_s == MoveSideL))
		||
		(((-1024-SIDE_DIR-LIM_DIR)<subdir)&&(subdir<0)
		&&(move_s == MoveSideR))
		)
			return ;
		move_s = MoveBack ;
	}else {
		/*FORWARD*/
		if(
		(((1024-SIDE_DIR-LIM_DIR)<subdir)
		&&(move_s == MoveSideL))
		||
		(((-1024+SIDE_DIR+LIM_DIR)>subdir)
		&&(move_s == MoveSideR)))
		return ;
		move_s = MoveCautionWalk;
	}
	entk->act->move_s = move_s;
}

/*射撃予測 発砲前にオンラインで着弾距離を求める*/
float	CheckGunPlOnline(ENETHINK *entk,FVECTOR *trg_pos ){
	FVECTOR	start_pos,hit_pos;
	FMATRIX	*hand ;
	FVECTOR	Ak_shift = { 20.0F, -712.0F, 90.0F };
	float	len;
	/* 銃を持つ手のマトリックス */
	if( entk->sw_gun & SW_FLAG_SWITCH1 ){
//	if ( flag & ENE_BULLET_LEFT ) {
		hand = &BODYWORLD( entk->act->body, HUMAN21_HIDARI_TE ) ;
	} else {
		hand = &BODYWORLD( entk->act->body, HUMAN21_MIGI_TE ) ;
	}
    DG_SetPos( hand ) ;
	/* 銃口までシフト */
	DG_PutVector( &Ak_shift, &start_pos, 1 ) ;
	if(HZX_OnlineHazardCheck(entk->ctrl->hzx_id,&start_pos,
	trg_pos,HZX_CHK_ALL,HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) )
	{
		/*遮蔽物有り*/
		HZX_GetOnlinePoint(&hit_pos);
		len = GV_VecLen3F2( &start_pos, &hit_pos );
	}else {
		len = 100000.0F;
	}
//printf("HZD DIS %f\n",len);
	return  len;
}

/*dir1からみた角度差*/
//static int	SIG_CheckDirSub(int dir1,int dir2){
int	SIG_CheckDirSub(int dir1,int dir2){
	int sub;
	
	sub = dir2 - dir1;
	sub	&= 4095;
	if(sub >= 2048) sub= -(4096-sub);
//	else if(sub <= -2048) sub= 4096+sub;
	
	return sub;
}

/*目標のゾーンに対して角距離差の大きいゾーンを返す*/
int	SIG_GetFarDirZone( HZX_GROUP_ID id, int nowaddr, int trgaddr )
//int		nowaddr ;		/* 現在のゾーン		*/
//int		trgaddr ;		/* 目標のゾーン		*/
{
	u_char		*nears ;
	int		i, near ;
	int		tmpdir ,nowdir ,trgdir ,maxdir,maxaddr;
	int map;
//	HZX_GRP	*grp ;
	FVECTOR	from,to;
	HZX_ZON *nowzone ,*trgzone ;

	nowaddr = HZX_Zone1(nowaddr) ;
	trgaddr = HZX_Zone1(trgaddr) ;

	map = GV_GetNo( id ) ;
	if ( nowaddr == trgaddr ) {
		return (nowaddr)|(nowaddr<<8)|(map<<16) ;
	}
	maxaddr = nowaddr ;

	nowzone = HZX_GetZone(id,nowaddr);
	trgzone = HZX_GetZone(id,trgaddr);

	SIG_SetZonePos2Fvec(trgzone ,&from) ;
	SIG_SetZonePos2Fvec(nowzone ,&to ) ;
	maxdir = 0 ;
	nowdir = GV_VecDir2FromTo(&from,&to) ;
	nears = nowzone->nears ;
//	n_zones = grp->n_zones ;
	for ( i = 6 ; i > 0 ; -- i ) {
		if ( ( near = *( nears ++ ) ) == HZX_NO_ZONE ) break ;
		if(near == trgaddr) break ;
		if(SIG_GetRIntrptZ2Z( id, nowaddr, near )) break ;
		nowzone = HZX_GetZone(id,near);
		SIG_SetZonePos2Fvec(nowzone ,&to ) ;
		trgdir = GV_VecDir2FromTo(&from,&to) ;
		if((tmpdir = GV_DiffDirAbs( nowdir, trgdir )) > maxdir){
			maxdir = tmpdir ;
			maxaddr = near;
		}
	}
	return (maxaddr)|(maxaddr<<8)|(map<<16) ;

}

/*アドレスが非探査フラグか？*/
int CheckNoSearch(int addr){
	HZX_ZON *zone;

	if(HZX_Zone1(addr) != HZX_NO_ZONE){
		zone = ENE_HZX_GetZone(addr);
	}else {
		zone = ENE_HZX_GetZone(GM_PlayerAddress);
	}
	if(zone->flag & HZX_ZONE_SAFE){
		return 1 ;
	}
	return 0 ;
}
void SIG_CheckCorner(ENETHINK *entk){
	int i,zonenum,near[6] ,addr,route,test_r;
	int	trgaddr ;
	HZX_ZON		*zone ;
	AT_THK	*at_thk ;
	float	py,ey;
//	int testaddr;

	at_thk = (AT_THK *)entk->character ;

	at_thk->at_status &= ~AT_ST_NEXT ;
	at_thk->at_status &= ~AT_ST_FEEL ;
	at_thk->at_status &= ~AT_ST_FEEL_LOW ;



#if 0
	trgaddr = GM_PlayerAddress ;
#else 
	trgaddr = entk->at_com->at_trg_addr ;
#endif

	if(SIG_CheckStealthStatus(entk)){
		return  ;
	}


/*視界ＯＦＦ中は気配もＯＦＦ*/
#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		/* ステルスチェック */
		if ( entk->com->status & CMST_ENEMY_SIGHT_OFF )
		{
			return ;
		}
	}
#endif
	
	/*相手が暗闇内なら不可*/
	/*ゾーン外無効*/
	/*特別隠れゾーン内なら無効*/
	if(
	(GM_PlayerStatus & PLAYER_DARK_AREA)
	||(GM_PlayerStatus & PLAYER_ENEMY_HIDDEN)
	||(HZX_Zone2(trgaddr)==HZX_NO_ZONE)
//	||(CheckNoSearch(trgaddr)) /*非探査ゾーンより気配察知優先2001.07.09*/
	||(GM_PlayerStatus &PLAYER_CB_BOX)
	||(!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT))
	){
		return ;
	}
	/*気配無効モード*/
	if(entk->at_com->berserk > 0) {
		return ;
	}
	
	/*近すぎる 2000.09.04*/
#if 1
	if(entk->pl_eyei.dis < 2000.0F){
		return ;
	}
#endif
	/*高さが違いすぎる場合*/
	zone = HZX_GetZone(entk->ctrl->hzx_id,
		HZX_Zone1(entk->ctrl->addr) );
	ey = (float)zone->y;
	zone = HZX_GetZone(GM_PlayerControl->hzx_id,
			HZX_Zone1(trgaddr) );
	py = (float)zone->y;
	if(abs(ey-py)> 500.0F){
		return ;
	}

	/*自分が物影に隠れてるとき用*/
	if(
	(entk->pl_eyei.dis < entk->sense.eye_s  )&&
	((!(GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_BEYOND|PLAYER_LOCKER)))
	&&(!(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE))
	)
	){

		if(ENE_ReadOnlinInfo(entk->ctrl->addr,trgaddr)){
			/*現在からは見えない*/
			zonenum = HZX_NearZones( entk->ctrl->hzx_id,
			HZX_Zone1(entk->ctrl->addr), near );
		/*次の目標ゾーンから調べる*/
			if(!ENE_ReadOnlinInfo(entk->znavi->next_addr,trgaddr)){
				zone = HZX_GetZone(entk->ctrl->hzx_id,
					HZX_Zone1(entk->znavi->next_addr) );
				if(Sig_InsideZoneWide( zone, &GM_PlayerPosition ,0.0F )){
					if(Sig_InsideZone( zone, &entk->ctrl->mov ,500.0F )){
//printf("AT_ST_NEXT\n");
						at_thk->at_status |= AT_ST_NEXT ;
					}
				}
			}
		}
	}

	if( entk->act->bodyp.type & ENE_TYPE_HITECH){
/*ハイテク兵は気配感じない*/
//		return ;
	}
	if(
	(entk->pl_eyei.dis > entk->sense.eye_s  )
	||(GM_PlayerStatus & (PLAYER_INTRUDE|PLAYER_BEYOND) )
	||(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)
	){
		/*視界外なので無条件終了*/
		/*隠れモード中は例外*/
		return ;
	}
	/*プレイヤ隣接から敵兵チェック*/
	/*プレイヤが隠れてる時用*/

#if 1
	if(
//	(!ENE_ReadOnlinInfo(trgaddr,entk->ctrl->addr))
	( entk->pl_eyei.sight != EYE_INFO_SIGHT_OUT_HZD)
	){
	/*現在地が見える*/
//EYE_INFO_SIGHT_OUT_HZD == 1
		return ;
	}
#else
	if(at_thk->in_sight >= (AT_THK_RATE*2) ) {
	/*現在地が見える*/
		return ;
	}
#endif
/*テストロッカールーム内での威嚇禁止*/
#if 0
	testaddr = HZX_Zone1(trgaddr) ;
	
	if(
	((testaddr >= 22)&&(testaddr <= 25))
	||(testaddr == 0)||(testaddr == 1)
	||(testaddr == 11)||(testaddr == 12)
	){
		return ;
	}
#endif

	/* 立ってれば見えるハズなら気配状態*/
	if(!ENE_ReadOnlinInfo(trgaddr,entk->ctrl->addr)){
#if 0
		zone = HZX_GetZone(entk->ctrl->hzx_id,
			HZX_Zone1(trgaddr) );
		if(Sig_InsideZoneWide( zone, &entk->ctrl->mov,0.0F )){
#else
		if(SIG_CheckSafeType(entk,trgaddr)& SAFE_LOW ){
#endif
			/*低安地*/
//printf("AT_ST_FEEL_LOW\n");
			at_thk->at_status |= AT_ST_FEEL ;
			at_thk->at_status |= AT_ST_FEEL_LOW ;
			return ;
		}
	}


/**/
#if 1
	/*
	2001.08.24 この時期にしては大きい変更かも
	現在ゾーンの内側にしっかり入ってないと気配モードにしない
	影響多そうなのでなんかあったら削除
	*/
	if(!(Sig_InsideZoneWide2(entk , -350.0f))){
		/*ゾーン状態不安定なので却下*/
		return  ;
	}
#endif
/**/

	zonenum = HZX_NearZones( GM_PlayerControl->hzx_id,
	HZX_Zone1(trgaddr), near );

	/*プレイヤと敵の距離を求め、手前判定の基準にする*/
	route = Sig_GetRoute( GM_PlayerControl->hzx_id,
		entk->ctrl->addr,trgaddr);
	for(i=0;i<zonenum;i++) {
		addr = HZX_Zone1(near[i]);
		addr = (addr|(addr<<8)|(HZX_ZoneMapNo(trgaddr)<<16));
		test_r = Sig_GetRoute( entk->ctrl->hzx_id,
			entk->ctrl->addr,addr) ;
		if( route <= test_r ) {
			/*手前ゾーンじゃないので除外*/
			continue ;
		}
		if(
		(!ENE_ReadOnlinInfo(addr,entk->ctrl->addr))
		){
		/*近接ゾーンが敵から見えている*/
/*2000.12.05 近接ゾーンの正面に敵がいるときのみ有効とする*/
			/*オンラインinfo済みなので削除*/
//			if(!SIG_CheckSafeType(entk,addr)){
			if(1){
				zone = HZX_GetZone(GM_PlayerControl->hzx_id,
					HZX_Zone1(near[i]) );
				if(Sig_InsideZoneWide( zone, &entk->ctrl->mov ,0.0F )){
					if(Sig_InsideZone( zone, &GM_PlayerPosition ,1000.0F )){
						/*となりのゾーンとの距離が近い*/
						if(!(SIG_CheckSafeType(entk,trgaddr)& SAFE_LOW ))
						{
							/*低安地ではない*/
							at_thk->at_status |= AT_ST_FEEL ;
							{
				/*実験 直接見えてなくても探知*/
								if(GM_PlayerStatus & PLAYER_BEYOND) {
									entk->at_com->watch_status |= AT_COM_WATCH_BEYOND;
								}
								if((GM_PlayerStatus & PLAYER_INTRUDE) 
								||(ENE_HZX_GetZone(GM_PlayerAddress)->flag & HZX_ZONE_INTRUDE)){
									entk->at_com->watch_status |= AT_COM_WATCH_INTRUDE ;
								}
							}
							return ;
						}
					}
				}
			}
		}
	}
}

/*自分のチームのｎ番目の兵士のENTKを取得*/
ENETHINK *GetAtRanking(entk,rank)
ENETHINK	*entk ; /*自分のentk*/
int			rank ;	/*要求順位*/
{
	int i ;
	E_UNIT		*e_unit ;
	ENETHINK	*trgentk;
	AT_THK		*at_thk ;

//	at_thk = (AT_THK *)entk->character ;

	e_unit = entk->com->enemys.group[entk->g_id]->unit[entk->u_id] ;
	for(i=0;i<e_unit->enemy_num;i++){
		trgentk = e_unit->entk[i];
		at_thk = (AT_THK *) trgentk->character ;
		if(at_thk->dis_rank == rank){
			return trgentk ;
		}
	}
	return NULL ;
}

/*非探査ゾーンの近くの仮目標を決める*/
int GetNewSerch( int addr1,int addr2){
	int now ;
	HZX_ZON *zone;
	SVECTOR	rgb ;
	rgb.vx = 127 ;
	rgb.vz = 0 ;
	zone = ENE_HZX_GetZone(addr1);
	now = zone->nears[0] ;
	now = MapAddrSet(now,addr1) ;
printf("GetNewSerch Zone %d\n",HZX_Zone1(addr1)) ;
printf("GetNewSerch MapNo %d\n",HZX_ZoneMapNo(addr1)) ;

printf("GetNewSerch Zone %d\n",HZX_Zone1(now)) ;
printf("GetNewSerch MapNo %d\n",HZX_ZoneMapNo(now)) ;

#if 0
	/*yellow*/
	SigZoneView(now,NULL,200.0F) ; 
	rgb.vx = 127 ;
	rgb.vy = 0 ;
	rgb.vz = 0 ;
	/*red*/
	SigZoneView(addr1,&rgb,200.0F) ; 
	rgb.vx = 0 ;
	rgb.vy = 0 ;
	rgb.vz = 127 ;
	/*blue*/
	SigZoneView(addr2,&rgb,200.0F) ; 
#endif
//	now = GetFarZone2(addr,now ,1) ;
	now = SIG_GetFarZone3(addr1,addr2,now ,2) ;
	rgb.vx = 0 ;
	rgb.vy = 127 ;
	rgb.vz = 127 ;
	/*green*/
//	SigZoneView(now,&rgb,200.0F) ; 

	return now ;
}

/*離れるゾーンを二つ指定可能*/
int SIG_GetFarZone3(int trgaddr1,int trgaddr2,int nowaddr ,int reach){
	int rout=0,tmpaddr,i;
	int hzx_id,trg1,trg2 ;
	
	hzx_id = GV_GetBit( HZX_ZoneMapNo(trgaddr1) );

	trg1 = HZX_Zone1(trgaddr1);
	trg2 = HZX_Zone1(trgaddr2);
	if((trg1 == HZX_Zone1(nowaddr) )
	||( trg1 ==HZX_NO_ZONE)
	||(HZX_Zone1(nowaddr)==HZX_NO_ZONE)
	||(GM_GetRIntrptOne( trg1 ) != NULL)

	){
//printf("GetFarZone3 %d \n",__LINE__);
		return nowaddr ;
	}
	if((trg2 == HZX_Zone1(nowaddr) )
	||( trg2 ==HZX_NO_ZONE)
	||(HZX_Zone1(nowaddr)==HZX_NO_ZONE)
	||(GM_GetRIntrptOne( trg2 ) != NULL)

	){
//printf("GetFarZone3 %d \n",__LINE__);
		return nowaddr ;
	}

	nowaddr = tmpaddr = HZX_Zone1(nowaddr);
	for(i=0;i<reach;i++){
#if 0
		tmpaddr = HZX_FarZoneNavigate( hzx_id,
		HZX_Zone1(nowaddr),trg1,&rout);
#else
		tmpaddr = HZX_FarZoneNavigate2( hzx_id,
		HZX_Zone1(nowaddr) , trg1,trg2, &rout ) ;
#endif
printf("GetFarZone3 %d \n",__LINE__);
		if(GM_GetRIntrptOne( tmpaddr ) != NULL) {
printf("GetFarZone3 %d \n",__LINE__);
			break;
		}
		if(tmpaddr == trg1) {
printf("GetFarZone3 %d \n",__LINE__);
			break;
		}
		if(tmpaddr == nowaddr) {
printf("GetFarZone3 %d \n",__LINE__);
			break;
		}
		nowaddr = tmpaddr;
	}
printf("GetFarZone3 %d LOOP %d\n",__LINE__,i);
	ASSERT(tmpaddr != HZX_NO_ZONE);
	return MapAddrSet(tmpaddr,GM_PlayerAddress);
}


/*FVECに指定範囲内で乱数増減させる*/
void SIG_SetRandFvec(FVECTOR *pos,int wide){
	float rand;

	pos->vx -= (float) wide ;
	pos->vy -= (float) wide ;
	pos->vz -= (float) wide ;

	rand = (float) ((irnd()>>8) % (wide*2) ) ;
	pos->vx += rand;
	rand = (float) ((irnd()>>8) % (wide*2) ) ;
	pos->vy += rand;
	rand = (float) ((irnd()>>8) % (wide*2) ) ;
	pos->vz += rand;

}

#if 0
/*ランダム間隔射撃*/
#define AK_INTER 3
/*マカロフ*/
#define MKR_INTER 8
#else 
/*ランダム間隔射撃*/
#define AK_INTER 2
/*マカロフ*/
#define MKR_INTER 4
#endif

/*ＳＰＳ*/
#define SPS_INTER 1
void SIG_BasicShoot( ENETHINK *entk ,FVECTOR *trg,int flag)
{
	int rnd_rate;

#ifdef DEBUG_NO_SHOT
	return ;
#endif
	if(GM_PlayerStatus &PLAYER_DEAD) return ;

	/*乱数で発砲*/
	if(entk->act->bodyp.type & ENE_TYPE_SHIELD){
		/*盾兵 マカロフ*/
		rnd_rate = MKR_INTER;
	}else if(entk->act->bodyp.type & ENE_TYPE_SHOTGUN){
		/*ショットガン ＳＰＳ*/
//		entk->bullet = 0;
		rnd_rate = SPS_INTER;
	}else {
		/*ＡＫ*/
		rnd_rate = AK_INTER;
	}
	if ( !(DG_TickCount % rnd_rate) ) {
		if ( (irnd()>>8)%(4)  ) {
			return;
		}
		ENE_ShootBullet( entk->act,(ENE_BULLET_NORMAL|flag) ,trg ) ;
		entk->bullet++;
	}
}

/*移動しながらなので狙いがぶれる*/
void SIG_RandShoot( ENETHINK	*entk )
{
	int subdir;
	FVECTOR	trgpos ;
	float	len;

	if( entk->bullet >= entk->max_bullet ) {
		return ; 
	}
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT ){ 
//	if( entk->pl_eyei.dis > entk->sense.eye_s ){
		return;
	}

//	subdir = SIG_CheckDirSub(entk->sense.facedir,entk->pl_eyei.dir);
	subdir = SIG_CheckDirSub(entk->ctrl->turn.vy,entk->pl_eyei.dir);
	if((subdir < -90 )||(90 < subdir )) {
		return;
	}
	/*狙いを揺らす*/

	trgpos = GM_PlayerFindPos ;
//	trgpos = *entk->pl_eyei.pos ;
	len = GV_VecLen3F2( &trgpos, &entk->ctrl->mov );

	if(len > 6000.0F){
		/*乱数で発砲*/
		/*距離の5分の１*/
		SIG_SetRandFvec( &trgpos,(len/5 ));
		SIG_BasicShoot( entk ,&trgpos ,ENE_BULLET_NOATTACK);
	}else if(len > 2000.0F){
		/*乱数で発砲*/
		/*距離の20分の１*/
		SIG_SetRandFvec( &trgpos,(len/20 ));
		SIG_BasicShoot( entk ,&trgpos ,0);
	}else {
		/*近すぎる時は銃身マトリクス*/
		SIG_BasicShoot( entk ,NULL ,0);
	}
}
/*移動しながらなので狙いがぶれる*/
void SIG_RandShoot_TNG( ENETHINK	*entk )
{
	int subdir;
	FVECTOR	trgpos ;
	float	len;

	if( entk->bullet >= entk->max_bullet ) {
		return ; 
	}
	if(entk->pl_eyei.sight == EYE_INFO_SIGHT_OUT ){ 
//	if( entk->pl_eyei.dis > entk->sense.eye_s ){
		return;
	}

//	subdir = SIG_CheckDirSub(entk->sense.facedir,entk->pl_eyei.dir);
	subdir = SIG_CheckDirSub(entk->ctrl->turn.vy,entk->pl_eyei.dir);
	if((subdir < -90 )||(90 < subdir )) {
		return;
	}
	/*狙いを揺らす*/

//	trgpos = GM_PlayerFindPos ;
	trgpos = *entk->pl_eyei.pos ;
	len = GV_VecLen3F2( &trgpos, &entk->ctrl->mov );

	if(len > 6000.0F){
		/*乱数で発砲*/
		/*距離の5分の１*/
		SIG_SetRandFvec( &trgpos,(len/5 ));
		SIG_BasicShoot( entk ,&trgpos ,ENE_BULLET_NOATTACK);
	}else if(len > 2000.0F){
		/*乱数で発砲*/
		/*距離の20分の１*/
		SIG_SetRandFvec( &trgpos,(len/20 ));
		SIG_BasicShoot( entk ,&trgpos ,0);
	}else {
		/*近すぎる時は銃身マトリクス*/
		SIG_BasicShoot( entk ,NULL ,0);
	}
}

int	SIG_CheckTrap( CONTROL *ctrl ,u_int trapname ) 
{
//	u_int	Traps[] = { tng_com->res_trap  } ;
	u_int	Traps[] = { trapname  } ;
    return (GM_CheckInsideTrap( ctrl, Traps, 1, 0 )) ;
}

#if 0

int SIG_NktCheck(void){
typedef	struct{
	RADAR_CTRL	rctrl ;

	short	facedir ;	/* 顔の方向 */
	short	facedir_x ;	/* 顔の上下方向 */
	short 	eye_r ; 	/* 顔の向きを中心とした片側視野角度 */
	short 	padddddd ; 	/* */
	int 	eye_s ;		/* 視力 */
	int		hearing ;	/* 聴力 */
	int		smell ;		/* 嗅覚 */
	int		status ;	/* ステータス（レーダーの色） */

	int 	eye_s_s[4] ;		/* 視力 */
} SENSEPARAM ;

	if(ENE_EyeOnlineCheck( hzx_id, eye->eyepos, eye->trgpos ) ){
#endif
int SIG_EyeInsideCheck( SENSEPARAM *eye,ENETHINK *entk ,FVECTOR *trgpos)
{
	FVECTOR vec ,eyepos;
	float dis ;
	int dir ,diff;

	KR_FMatToFvec( &BODYWORLD(entk->act->body, HUMAN21_ATAMA), &eyepos ) ;
	_sceVu0SubVector( &vec, trgpos , &eyepos ) ;
	dis = GV_VecLen3F( &vec ) ;

	if ( dis >  (float) eye->eye_s ) return 0 ;

	dir = GV_VecDir2( &vec ) ;
	if ( GV_DiffDirAbs( eye->facedir, dir ) > eye->eye_r ) return 0 ;
	dir = GV_VecDir2X( &vec )  ;
	diff = GV_DiffDirS( eye->facedir_x, dir ) ;
	if ( diff > (512)|| diff <  (-512) ) return 0 ;
	return 1 ;
}

int SIG_NktCheck(ENETHINK *entk){
	FVECTOR	diff ;
	if(GM_NikitaAlive[0] == NKT_NORMAL){
		_sceVu0SubVector( &diff, &entk->ctrl->mov, &GM_NikitaPosition[ 0 ] ) ;
		if( GV_VecLen3F( &diff ) < 1500.0F ){
			return 1 ;
		}
		if(SIG_EyeInsideCheck( &entk->sense ,entk ,&GM_NikitaPosition[ 0 ]) ){
			return 1 ;
		}
	}
	return 0 ;
}

void SIG_MultiTexOn(OBJECT *body){
	body->flag |= DG_FLAG_MULTITEX ;
	body->objs->flag |= DG_FLAG_MULTITEX ;
	KR_SetAllObjsFlag( body->objs, DG_FLAG_MULTITEX ) ;
}

void SIG_MultiTexOff(OBJECT *body){
	body->flag &= ~DG_FLAG_MULTITEX ;
	body->objs->flag &= ~DG_FLAG_MULTITEX ;
	KR_UnsetAllObjsFlag( body->objs, DG_FLAG_MULTITEX ) ;
}
void SIG_AllObjectInvisible(OBJECT *body){
	body->flag |= DG_FLAG_INVISIBLE ;
	body->objs->flag |= DG_FLAG_INVISIBLE ;
	KR_SetAllObjsFlag( body->objs, DG_FLAG_INVISIBLE ) ;
}
void SIG_AllObjectVisible(OBJECT *body){
	body->flag &= ~DG_FLAG_INVISIBLE ;
	body->objs->flag &= ~DG_FLAG_INVISIBLE ;
	KR_UnsetAllObjsFlag( body->objs, DG_FLAG_INVISIBLE ) ;
}



void SIG_SetTrgpPlayerLastpos( trgp, com )
TRGPOINT	*trgp ;
COMMANDER	*com ;
{
	int addr  ;

	if(
	(!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT))
	&&(COM_AlertStatus()&COM_ALERT_NPC_DETECT)
	){
		/*NPC目標*/
		CONTROL *ema_ctrl ;
		ema_ctrl = EMA_CommandGetControl() ;

		trgp->addr = ema_ctrl->addr ;
		trgp->pos  = ema_ctrl->mov ;
//		trgp->map  = ema_ctrl->map ;
		trgp->map  = ema_ctrl->hzx_id ;
//map に　hzxidをいれろ
//printf("SET TARGET EMA \n");
	}else {
		if(com->player_lastmap == 0 ){
//printf("com->player_lastmap == 0\n");
			/*イベント等で正規の手順を踏まず危険モードになっている*/
			/*天狗兵攻撃開始メッセージなど*/
			if(COM_AlertStatus()&COM_ALERT_NPC_DETECT){
				/*対エマ*/
				CONTROL *ema_ctrl ;
				ema_ctrl = EMA_CommandGetControl() ;
				trgp->addr = ema_ctrl->addr ;
				trgp->pos  = ema_ctrl->mov ;
//				trgp->map  = ema_ctrl->map ;
				trgp->map  = ema_ctrl->hzx_id ;

//printf("SET TARGET EMA \n");
			}else {
				trgp->addr = GM_PlayerAddress ;
				trgp->pos  = GM_PlayerPosition ;
//				trgp->map  = GM_PlayerMap ;
				trgp->map  = GM_PlayerControl->hzx_id ;

//printf("SET TARGET PLAYER \n");
			}
		}else {
//extern void SigZoneView(int,SVECTOR *,float) ; 

//printf("com->player_lastmap == [%x]\n",com->player_lastmap);
			addr = HZX_GetAddress( com->player_lastmap, &com->player_lastpos, -1 ) ;

			if(HZX_Zone1(addr) == HZX_NO_ZONE){
//printf("SET GM_PlayerAddress\n");
				trgp->addr = GM_PlayerAddress ;
				trgp->pos  = GM_PlayerPosition ;
//				trgp->map  = GM_PlayerMap ;
				trgp->map  = GM_PlayerControl->hzx_id ;
			}else {
				trgp->addr = addr ;
				trgp->pos  = com->player_lastpos ;
				trgp->map  = com->player_lastmap ; /* ここはOK */
			}
//printf("TRG ADDR [%x]\n",trgp->addr);
//SigZoneView(trgp->addr,NULL,200.0F) ; 

		}
	}
}

/*2点の水平距離*/
float SIG_VecLen3F2D( FVECTOR *v1 , FVECTOR *v2 ){

    FVECTOR	diff ;

    _sceVu0SubVector( &diff, v1, v2 ) ;
	diff.vy = 0.0f ;
    return GV_VecLen3F( &diff ) ;
}

int SIG_CheckFrontSeg(CONTROL *ctrl,float len){
	extern void PosBox( FVECTOR * ,float ,SVECTOR *) ;

	FVECTOR		check ,trg;
	SVECTOR rgb;

	check = DG_ZeroVector ;
	check.vz = len;
	rgb.vx = 127 ;
	
	DG_SetPos2( &ctrl->mov, &ctrl->rot ) ;
	DG_PutVector( &check, &trg, 1 ) ;
//	GM_CurrentMap = GM_CurrentStageMap ;
//	PosBox(&trg,250.0F , &rgb );
	return (HZX_OnlineHazardCheck(ctrl->hzx_id,&ctrl->mov,&trg,HZX_CHK_ALL,
		HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY ) ) ;
}

/*
攻撃兵用音声管理 コマンダが連続呼びを防止する
SD_V_ATKO03,	//攻撃兵１対逃げ「逃げたぞ！」//gbs_a002 312
SD_V_ATMO03,	//攻撃兵２対逃げ「追跡しろ！」//gbs_b002 313
SD_V_ATKO04,	//攻撃兵３対逃げ「追え！」//gbs_c002 314
SD_V_ATMO04,	//攻撃兵４対逃げ「待て！」//gbs_d002 315
*/
void SIG_AT_VoiceCall(ENETHINK *entk,int code,int count){
	int se_code ;

	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ; 

//	if((entk->at_com->called_se_code == code)&&(entk->at_com->called_se_count>0)){
	if((entk->at_com->called_se_count>0)){
		return ;
	}
	if((code ==SD_V_ATKO01)
	&&(entk->at_com->called_se_pl_addr == GM_PlayerAddress )){
		/*「そこにいるぞ」は移動時のみ*/
		return ;
	}

	se_code = code+(entk->act->name_id->voice%4) ;
	/*暫定処理 似合わない音声を差し替え*/
#if 1
	switch(se_code){
		case SD_V_ATKO04 : /*「追え！」*/
			se_code = SD_V_ATMO03 ; /*「追跡しろ！」*/
//			se_code = SD_V_ATKO03 ; /*「逃げたぞ！」*/
			break ;
		case SD_V_ATKO03 : /*「逃げたぞ！」*/
//		case SD_V_ATMO04 : /*「待て！」*/
//			se_code = SD_V_ATMO04 ; /*「待て！」*/
			se_code = SD_V_ATMO03 ; /*「追跡しろ！」*/
			break ;
	}
#endif	
	GM_SeSetMode( se_code, &entk->ctrl->mov, GM_SEMODE_NORMAL ) ;
	entk->at_com->called_se_code = code ;
//	entk->at_com->called_se_count = DIRECT_TICK(count) ;
	entk->at_com->called_se_count = DIRECT_TICK(60*20) ;
	if(code ==SD_V_ATKO01){
		entk->at_com->called_se_pl_addr = GM_PlayerAddress ;
	}
}


void	SIG_SetSpeak( v, act )
int		v ;
ACTION	*act ;
{
	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ; 
	GM_SeSetMode( v+(act->name_id->voice%4), &act->ctrl->mov, GM_SEMODE_NORMAL ) ;
}

