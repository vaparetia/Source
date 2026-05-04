//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ply_ap.c
	2002/05/08 K.Sigeno
	$Id: play_ap.c,v 1.1.1.3 2002/11/19 11:49:23 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#endif

#include "gameheader.h"
#include "libutl.h"
#include "camera.h"

#include "../effect/sig_vanim.h"


#define	WARP_NEGA	(1)


#define	TXD_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	int			name ;
	int			pl_name ;
	int			mode ;
	int			cnt ;
//	FVECTOR		pos ;
	FVECTOR		*pos ;
	FVECTOR		fl_pos ;
	int			proc_id ;
	int			first_se_tick ;
	int			last_se_tick ;
	DG_OBJS		*txd ;
	FMATRIX		lights[2] ;
} Work ;

#define	DEFAULT_MODE	(5)
#define	NO_XZ_MOVE			(0x0001) /*モーションの水平移動を無視*/
#define	NO_GRAVITY			(0x0002) /*重力接地無し*/
#define	NO_HAZARD			(0x0004) /*壁アタリ無し*/
#define	PAUSE_MOT			(0x0008) /*静止*/
#define	INIT_POS			(0x0010) /*再生開始時に座標リセット*/
#define	NO_INTERP			(0x0020) /*モーション補完無し*/
#define	PRESHADE			(0x0040) /*プリシェード使用*/
#define	INVISIBLE_START		(0x0080) /*非表示で起動*/
#define	PUT_MOT_THERMAL		(0x0100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x0200)	//デバッグ用表示
#define	MT_ANIM				(0x0400)	//ストリーム口パク
#define	RAD_POINT			(0x0800)	//ストリーム口パク


//#define HANKEI (750.0f)
#define HANKEI (500.0f)
#define BREAK_RANGE	(300.0f)

#define INTERP   6*(300/BP_FRAMES_PER_SEC())


extern int NewKillInvisibleChara(void) ;
extern int NewInvisibleChara(void) ;

static float GetPlObjsCenter(FVECTOR *pos){
	FVECTOR min , max ,tmp ,res;
	float len1,len2 ;
	int	i;
	min = max = GM_PlayerPosition ;
	for(i=0;i<21;i++){
		GV_MatToVec( &(GM_PlayerBody->objs->objs[i].world), &tmp ) ;
		if(min.vx > tmp.vx) min.vx = tmp.vx ;
		if(min.vy > tmp.vy) min.vy = tmp.vy ;
		if(min.vz > tmp.vz) min.vz = tmp.vz ;

		if(max.vx < tmp.vx) max.vx = tmp.vx ;
		if(max.vy < tmp.vy) max.vy = tmp.vy ;
		if(max.vz < tmp.vz) max.vz = tmp.vz ;
	}
	res.vx = (max.vx + min.vx)/2.0f ;
	res.vy = (max.vy + min.vy)/2.0f ;
	res.vz = (max.vz + min.vz)/2.0f ;
	*pos = res ;

	max.vy = min.vy = res.vy ;

	len1 = GV_VecLen3F2( &min,&res ) ;
	len2 = GV_VecLen3F2( &max,&res ) ;

	if(len1>len2) return len1 ;
	return len2 ;
}

#if 0

static void visiblemdl(work)
Work	*work ;
{
//printf("putmodel visible set!!\n");

	if(work->body.evmobj != NULL){
		work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	}else {
		DG_VisibleObjs( work->body.objs ) ;
	}
	if(work->in_flag & ITEM_SET){
		DG_VisibleObjs( work->item.objs ) ;
	}
	if(work->status & RAD_POINT){
		work->rctrl.flag |= RADAR_VISIBLE ;
	}
}
static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
	int n_msg, code ,n_motion ;
    n_msg = work->control.n_msg ;
	msg = work->control.msg ;

	n_motion = work->body.m_ctrl->motion_arc->n_motion ;
	while ( n_msg-- > 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
			case SET_MOTION :
				work->msg_mot = msg->message[ 1 ] ;
				break;
			case NEXT_MOTION :
				break;
		}
		msg++ ;
	}
}
#endif
#define TIME_A	(30)
//#define TIME_B	(90)
//#define TIME_B	(70)
#define TIME_B	(50)

//#define HEAD_DELAY	(150)
//#define BODY_DELAY	(60)
#define BODY_DELAY	(30)

//#define HEAD_DELAY	(TIME_A+TIME_B+BODY_DELAY)-(TIME_A)
#define HEAD_DELAY	(TIME_B+BODY_DELAY)
#define HAND_DELAY	(90)

#define HAND_DELAY	(90)

#define TXD_DELAY	(60)



#define NORMAL_MODE			(0x00000000)
#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)	/*足から変形開始*/
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOTION_TR			(0x00000200) /*モーション追随*/
#define BUILD_RND			(0x00000400)	/*ノイジー*/
#define NO_MAKE_OBJ			(0x00002000)

static void Act(Work *work)
{
	int mode=0,cnt=0 ,delay,se_tick;

	GM_CurrentMap = GM_CurrentStageMap ;

	work->fl_pos = GM_PlayerPosition ;
	work->fl_pos.vy = GM_PlayerControl->levels[ 0 ] ;


#if 1
	if(work->cnt==1){
extern void NewVR_Lens(FVECTOR *pos ,int mode,int time ,float size,float len) ;
extern void *NewShuChu_Line( int flag,FVECTOR *pos,int hankei ,float height,int time) ;
extern void NewGravEf(FVECTOR *pos ,int mode,int time ,float size,int len) ;
			
		NewVR_Lens(work->pos ,0,TIME_A+TIME_B ,2500.0f,150.0f) ;
		NewShuChu_Line( 24,work->pos ,2000,1000.0f,TIME_A+TIME_B+BODY_DELAY) ;
		NewGravEf(work->pos ,0,TIME_A+TIME_B ,50.0f,2000) ;
	}
#if 1
	if(work->cnt== DIRECT_TICK(TIME_A)){
		GM_SdSet( SD_S_N_START1 ) ;
	}
#endif
//	if((work->cnt<(TIME_B-15))&&(work->cnt&7)==1){
//	if((work->cnt<(TIME_B))&&(work->cnt&3)==1){
//	if((work->cnt<(TIME_B))&&(work->cnt&7)==1){
//	if((work->cnt<DIRECT_TICK(TIME_A+TIME_B))&&(work->cnt&DIRECT_TICK(7))==1){
	if((work->cnt<DIRECT_TICK(TIME_A+TIME_B))&&(work->cnt&7)==1){
extern void NewGravEf(FVECTOR *pos ,int mode,int time ,float size,int len) ;
//		NewGravEf((FVECTOR *)&objs->world.m[3] ,0,(work->time_a+work->time_b-work->cnt) ,25.0f,1250) ;
//		NewGravEf((FVECTOR *)&objs->world.m[3] ,0,(work->time_b) ,25.0f,1250) ;
//		NewGravEf(work->pos,0,TIME_B ,(50.0f)*(work->cnt&3),2000) ;
		NewGravEf(work->pos,0,(15+TIME_A+TIME_B-work->cnt ) ,(50.0f)*(work->cnt&3),2000) ;
	}
#endif
#if 0
	if(work->cnt==DIRECT_TICK(BODY_DELAY)){
extern void *NewVR_Fall_Line( int flag,FVECTOR *pos,float hankei ,float height,int time) ;
//		if(work->cnt<TIME_B >time_b){
		if(1){
			NewVR_Fall_Line( 0,&work->fl_pos,HANKEI-150.0f,2100.0f,TIME_A+TIME_B) ;
//			NewVR_Fall_Line( 0,&work->fl_pos,HANKEI-150.0f,2100.0f,TIME_A+TIME_B) ;
		}
	}
#endif

	if(work->cnt >= DIRECT_TICK(BODY_DELAY+TIME_A+TIME_B)){
		NewKillInvisibleChara() ;
		if(work->proc_id != 0){
			GM_ExecProc( work->proc_id, NULL );
		}
		GV_DestroyActor(work) ;
	}


/*SE CALL*/
	/*VR以外ではプレイヤエフェクトで音を鳴らす*/
	if(!(GM_GameStatus & STATE_VR_ONLY)){
		se_tick = DG_TickCount - work->first_se_tick ;
	//printf("TICK[%d] ACT[%d] \n",se_tick,work->cnt) ;
	//	if(work->cnt<BODY_DELAY+0){
	//		work->first_se_tick = DG_TickCount ;
		if(work->cnt == 0){
			GM_SdSet(SD_A_V_START1);
			work->first_se_tick = DG_TickCount ;
		}else if((DIRECT_TICK(36) <=se_tick)&&(work->cnt<(BODY_DELAY+TIME_A+TIME_B))){
			if( (DG_TickCount - work->last_se_tick) >DIRECT_TICK(12) ){
				GM_SdSet(SD_A_V_START2);
				work->last_se_tick = DG_TickCount;
			}else {
			}
		}else if(work->cnt == (BODY_DELAY+TIME_A+TIME_B)) {
			GM_SdSet(SD_A_V_START3 );
		}
	}

	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_TUXEDO) {
		if(work->cnt > DIRECT_TICK(TXD_DELAY)){
			DG_VisibleObjs( work->txd ) ;
		}
		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_KOSHI].world ) ;
		DG_PutObjs( work->txd );
		DG_GetLightMatrix( &GM_PlayerPosition , work->lights );

		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_KOSHI].world ) ;
		DG_GetPos( &work->txd->objs[0].world  ) ;
		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_ONAKA].world ) ;
		DG_GetPos( &work->txd->objs[1].world  ) ;
		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_MUNE].world ) ;
		DG_GetPos( &work->txd->objs[2].world  ) ;
		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_MIGI_ASHI1].world ) ;
		DG_GetPos( &work->txd->objs[3].world  ) ;
		DG_SetPos( &GM_PlayerBody->objs->objs[HUMAN21_HIDARI_ASHI1].world ) ;
		DG_GetPos( &work->txd->objs[4].world  ) ;

//		work->txd->objs[0].world = GM_PlayerBody->objs->objs[HUMAN21_KOSHI].world ;
//		work->txd->objs[1].world = GM_PlayerBody->objs->objs[HUMAN21_ONAKA].world ;
//		work->txd->objs[2].world = GM_PlayerBody->objs->objs[HUMAN21_MUNE].world ;
//		work->txd->objs[3].world = GM_PlayerBody->objs->objs[HUMAN21_MIGI_ASHI1].world ;
//		work->txd->objs[4].world = GM_PlayerBody->objs->objs[HUMAN21_HIDARI_ASHI1].world ;
	}

	work->cnt++ ;
}
static void Die(Work *work)
{
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_TUXEDO) {
		DG_DequeueObjs( work->txd ) ;
		DG_FreeObjs( work->txd );
	}
}
static int GetResources(Work *work, int name, int where)
{
#ifdef WARP_NEGA
extern void	*NewVR_WarpTag( int flag,FVECTOR *pos,float hankei ,float height,int time_a,int time_b) ;
#endif
extern void *NewSigBreakBody(DG_OBJS *,float ,CVECTOR *,int,int,int,int,void *) ;
extern void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
		int delay,int *sw,FVECTOR *shift,SVECTOR *rot) ;
extern void *NewSigBreakObj5(DG_OBJS *objs,int mode ,float len,int time_a ,int delay,int *sw) ;

	float hankei ;
	FVECTOR pl_center ;
	int mode ,delay;

	GM_CurrentMap = GM_CurrentStageMap ;

	if ( GCL_GetOption( 'p' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}else {
			work->proc_id  = 0;
		}
	}


//	mode = BUILD_Y|BUILD_REV;
	work->name = name ;

	work->pos = &GM_PlayerPosition ;

	work->fl_pos = GM_PlayerPosition ;
	work->fl_pos.vy = GM_PlayerControl->levels[ 0 ] ;

	/*ステータス*/
//GM_AnotherPlayer の中身
#if 0
enum {
	GM_ANOTHER_PLAYER_SNAKE = 0,	//スネーク
	GM_ANOTHER_PLAYER_RAIDEN,		//ライデン
	GM_ANOTHER_PLAYER_OLD_SNAKE,	//前作スネーク
	GM_ANOTHER_PLAYER_PLISKIN,		//プリスキン
	GM_ANOTHER_PLAYER_TUXEDO,		//タキシードスネーク
	GM_ANOTHER_PLAYER_BLADE,		//刀ライデン
};
#endif
	work->mode = GCL_GetOptionValue( 'm', 0 ) ;
	work->mode = DEFAULT_MODE ;

printf("PL AP MODE [%d]\n",work->mode) ;
	switch (work->mode){
//下降
		case 1 :
			mode = BUILD_UP|BUILD_Y|BUILD_REV|DISAPPEAR_MODE ;
//			mode = BUILD_UP|DISAPPEAR_MODE|BUILD_RND|MOTION_TR ;

			break;
		case 2 :
//上昇下降
			mode = BUILD_Y|DISAPPEAR_MODE;
			break;
		case 3 :
//下降分散
			mode = BUILD_UP|BUILD_REV|DISAPPEAR_MODE;
			break;
			case 4 :
//上昇螺旋
			mode = DISAPPEAR_MODE|BUILD_ROLL ;
			break;
//以下出現
		case 5 :
//下降
			mode = BUILD_UP|BUILD_Y|MOTION_TR ;;
			break;
		case 6 :
			mode = BUILD_UP|BUILD_Y|MOTION_TR ;;
			break;
		case 7 :
			mode = BUILD_UP|BUILD_Y|MOTION_TR ;;
			break;
		case 8 :
			mode = BUILD_ROLL|BUILD_REV ;
			break;
		case 9 :
			mode = BUILD_RND|MOTION_TR ;
			break;
		case 10 :
			mode = BUILD_UP|BUILD_RND|DISAPPEAR_MODE|MOTION_TR ;
			break;
		default :
			mode = BUILD_UP|DISAPPEAR_MODE|BUILD_RND|MOTION_TR ;
	}

	if( mode & BUILD_UP){
		delay = HEAD_DELAY ;
	}else {
		delay = 0 ;
	}

//	NewSigBreakBody(GM_PlayerBody->objs,200.0f, NULL,TIME_A,TIME_B,0,mode,NULL) ;
	NewSigBreakBody(GM_PlayerBody->objs,BREAK_RANGE, NULL,TIME_A,TIME_B,BODY_DELAY,mode,NULL) ;
//	NewSigBreakBody(GM_PlayerBody->objs,300.0f, NULL,30,0,0,mode,NULL) ;

	if(GM_PlayerWeaponBody != NULL) {
		if(GM_PlayerWeaponBody->objs != NULL) {

extern void *NewSigBreakObj4(DG_OBJS *objs,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot ) ;


		NewSigBreakObj4(GM_PlayerWeaponBody->objs,
			&(GM_PlayerBody->objs->objs[HUMAN21_MIGI_TE].world),
			mode|NO_CHECK_SW ,BREAK_RANGE,TIME_A ,delay,NULL,&DG_ZeroVector, 
			&DG_ZeroSVector) ;

		}
	}
	/*キャラごと個別装備*/
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_RAIDEN ) {
		/*ライデン*/
		/*頭にズラ*/
		NewSigBreakObj3(GV_StrCode("rai_hair_shadow_mt"),
			&(GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world),
			mode|NO_CHECK_SW ,BREAK_RANGE,TIME_A ,delay,NULL,&DG_ZeroVector, 
			&DG_ZeroSVector) ;

		/*右腿にホルスター*/

		if ( !GM_CheckPlayerStatusEX( PLAYER_STEALTH, PLAYER2_NUDE ) &&
		( GM_WeaponNum( WP_m92 ) >= 0 || GM_WeaponNum( WP_Socom ) >= 0 ) ) {
			NewSigBreakObj3(GV_StrCode("rai_hlst"),
				&(GM_PlayerBody->objs->objs[13].world),
				mode|NO_CHECK_SW ,BREAK_RANGE,TIME_A ,delay,NULL,&DG_ZeroVector, 
				&DG_ZeroSVector) ;
		}
	}
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_PLISKIN) {
		/*プリスキン*/
		/*腰になんか装備してる*/
		NewSigBreakObj3(GV_StrCode("iro_mag"),
			&(GM_PlayerBody->objs->objs[HUMAN21_KOSHI].world),
			mode|NO_CHECK_SW ,BREAK_RANGE,TIME_A ,delay,NULL,&DG_ZeroVector, 
			&DG_ZeroSVector) ;
	}
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_BLADE) {
		/*ニンジャライデン フェイスオープンモデル*/
		/*7862789 rai_nin_face_open */
		NewSigBreakObj3(7862789,&(GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world),
			mode|NO_CHECK_SW ,BREAK_RANGE,TIME_A ,delay,NULL,&DG_ZeroVector, 
			&DG_ZeroSVector) ;
	}
	if(GM_AnotherPlayer == GM_ANOTHER_PLAYER_TUXEDO) {
		/*タキシードスネーク*/
		/*裾*/
		DG_DEF	*def ;
		def = (DG_DEF*) GV_GetCache( GV_CacheID( 16565060, 'k' ) ) ;
		ASSERT(def != NULL) ;
		work->txd = DG_MakeObjs( def, TXD_FLAG, DG_CHANL_MAIN ) ;
		DG_QueueObjs( work->txd ) ;
//GM_PlayerBody->objs
//		DG_SetLightMatrix( work->txd, GM_PlayerBody->objs->light );
		DG_SetLightMatrix( work->txd, work->lights );
//		void *NewSigBreakObj5(DG_OBJS *objs,int mode ,float len,int time_a ,int delay,int *sw)
		NewSigBreakObj5(work->txd,mode|NO_CHECK_SW|NO_MAKE_OBJ ,BREAK_RANGE,TIME_A ,TXD_DELAY,NULL) ;
		DG_InvisibleObjs( work->txd ) ;
	}

#if 0
	if(1){
extern void *NewVR_Fall_Line( int flag,FVECTOR *pos,float hankei ,float height,int time) ;
		NewVR_Fall_Line( 0,&work->fl_pos,HANKEI-150.0f,2100.0f,TIME_A+TIME_B) ;
	}
#endif
#ifdef WARP_NEGA

	hankei = GetPlObjsCenter( &pl_center ) ;
	pl_center.vy = GM_PlayerControl->levels[ 0 ] ;
//	hankei = 1.4142135623730950488016887242097f ;
//	hankei *= 1.414213f ;

	hankei += 80.0f ;

printf("WARP TAG HANKEI [%f]\n",hankei);
	switch (work->mode){
		case 5:
			NewVR_WarpTag( 0,&pl_center ,hankei ,1900.0f,BODY_DELAY,TIME_B) ;
			break ;
		case 6:
			break ;
		case 7:
			NewVR_WarpTag( 1,&pl_center ,hankei ,1900.0f,BODY_DELAY,TIME_B) ;
			break ;
		default :
			NewVR_WarpTag( 1,&pl_center ,hankei ,1900.0f,BODY_DELAY,TIME_B) ;
			break ;
	}
#endif
	work->first_se_tick = 0;
	work->last_se_tick = 0;

	return 1;
}


/* 初期化部メイン */
void *NewPlayerAp( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

