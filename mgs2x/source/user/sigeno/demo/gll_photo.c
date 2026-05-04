//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gll_photo.c
	chara ゴルルゴン写真撮影
	2002/07/03 K.Sigeno
	$Id: gll_photo.c,v 1.1.1.3 2002/11/19 11:49:16 Yoshizawa1 Exp $
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

/* ワーク */
#define MAX_POS	(32)
typedef	struct _Work {
	GV_ACT_EX	actor ;
	CONTROL		control;
	OBJECT		body ;
	OBJECT		item ; /*持ち物*/
	FMATRIX		lights[2] ;
//	VAR_CONTROL	vctrl ;	/*振動制御*/
	FVECTOR		pos[MAX_POS];	/*移動目標*/
	FVECTOR		last_pos ;	/*前回座標*/
	FVECTOR		start_pos ;	/*初期座標*/
	FVECTOR		appear_pos ;	/*	初顔出し座標*/
	FVECTOR		speed ;
	FVECTOR		accel ;
	FVECTOR		ef_pos ;
	int			now_pos ;
	int			rand_rate ;
	int			max_pos ;
	float		shift ;
	int			name ;
	int			bas_mot; /*基本モーション*/
	short		status ; /*シナリオ指定*/
	short		in_flag ;	/*内部フラグ*/

	int			proc_id	;	/*出現時プロック*/
	LIT_DEF		*lit_def ;
    RADAR_CTRL	rctrl ;
	/*******************/
	int cnt ;
	int	model;
//	int dive_wait ;
	int	test_flag ;
	int mot_cnt ;
	int sp_sw_head ;
	int sp_sw_koshi ;
	int sp_sw_hands ;
	int sp_sw_legs ;
	int sp_sw_sin ;
	int	ef_cansel_cnt ;
} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)

#define	ITEM_FLAG (DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)
//DG_FLAG_IRREACTION 赤外線ゴーグル
#define JUMP_SPEED	(200.0f)
/*シナリオ ステータス*/
#define	NO_XZ_MOVE			(0x0001) /*モーションの水平移動を無視*/
#define	PAUSE_MOT			(0x0008) /*静止*/
#define	PUT_MOT_THERMAL		(0x0100)	//赤外線ゴーグル反応
#define	PUT_MOT_BOUND		(0x0200)	//デバッグ用表示
#define	RAD_POINT			(0x0800)	//ストリーム口パク

static int	GLL_Photo_Check ;
/*内部フラグ*/
#define SEQ_MODE		(0x01) /*シーケンスモード*/
#define ITEM_SET		(0x02) /*アイテム付随*/
#define PROC_CHECK		(0x08) /*proc判定あり*/
#define FIRST_APPEAR	(0x10) /*最初のポイントでは必ず顔を出す*/
#define DEMO_END		(0x20) /*登場のデモ済み*/

extern int BP_FRAMES_PER_SEC();
#define INTERP   (6*(300/BP_FRAMES_PER_SEC()))
#define RISE_INTERP   20*(300/BP_FRAMES_PER_SEC())

//#define SWIM_MOT_SPEED	(0.6f)
#define SWIM_MOT_SPEED	(1.0f)
#define DEF_MOT_SPEED	(1.0f)


#define 	ACCEL	(5.0f)
#define 	SPEED_LIMIT (500.0f)
#define		DIVE_DEPTH			(-900.0f)
#define		SHIFT_DEPTH_MIN		(-2000.0f)
#define		HEIGHT_SCALE		(10.0f)
#define		FLOAT_SPEED			(50.0f)
#define		EF_SIN_LEN			(2000.0f)
#define		RISE_FORCE			(200.0f)
#define		SWIM_FORCE			(200.0f)

#define		APPEAR_STR		(14084262)
#define		DIVE_STR		(432759)

enum{
	MSG_KILL = 12,
	MOT_WAKE,
	MSG_DEMO_CANSEL
};
enum {
	MOT_RISE = 0 ,
	MOT_DIVE ,
	MOT_SWIM ,
};
#define GLL_PHOTO_OK	(0x00000001)
extern void *NewGllSplush( FVECTOR *pos, float radius ,int *sw) ;
extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag ) ;
extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos ) ;
extern void *NewShakeCamera( int chanl, int intense, int time ) ;

//extern void *NewDiveSplash( FVECTOR *center, int pattern ) ;

extern void	*NewPadVibrationRadio( int file ) ;

extern float GM_WaterLevel;


/* 0 エフェ無し １飛沫のみ ２飛沫＋水幕(重い)*/
static void SetSpModeSwim(Work* work){
	work->sp_sw_head = 1;
	work->sp_sw_koshi = 2;
	work->sp_sw_hands = 0;
	work->sp_sw_legs = 1;
	work->sp_sw_sin = 0;
}

static void SetSpModeDive(Work* work){
	work->sp_sw_head = 1 ;
	work->sp_sw_koshi = 2 ;
	work->sp_sw_hands = 0 ;
	work->sp_sw_legs = 1 ;
	work->sp_sw_sin = 1;
}

static void SetSpModeRise(Work* work){
	work->sp_sw_head = 2;
	work->sp_sw_koshi = 2;
	work->sp_sw_hands = 1;
	work->sp_sw_legs = 0;
	work->sp_sw_sin = 2;
}
static void SetSpModeAllOff(Work* work){
	work->sp_sw_head = 0;
	work->sp_sw_koshi = 0;
	work->sp_sw_hands = 0;
	work->sp_sw_legs = 0;
}


static void CamShake(int inte ,int time ){
	NewShakeCamera( 0, inte, time ) ;
}
static void ResetPhotoCheck(void){
	GLL_Photo_Check &= ~GLL_PHOTO_OK ;
}
static void SetPhotoCheck(void){
	GLL_Photo_Check |= GLL_PHOTO_OK ;
}
int GLLPhotoCheck_Scn(void){
	return (GLL_Photo_Check & GLL_PHOTO_OK) ;
}
#if 1
static float isin( d )
int	d ;
{
	float f, t ;

	f = ( float )M_PI * d / 2048.0F ;
	t = sinf( f ) ;
	return t ;
}
#endif
static void MotSleep(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
	work->status |= PAUSE_MOT ;

}
static void MotWakeUp(Work *work){
	work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;
	work->status &= ~PAUSE_MOT ;
}

static void ChangeMot(Work *work,int mot,int interp){
//	int interp = INTERP;

	if( work->body.m_ctrl->mt3_ctrl->motion_num != mot){
		GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	}
	work->mot_cnt = 0 ;

}
static void ForceChangeMot(Work *work,int mot,int interp){
//	int interp = INTERP;

	GM_ConfigObjectAction( &work->body, 0, mot, 0,0xfffff,interp);
	work->mot_cnt = 0 ;
}

static void SetDemoCansel(Work *work){
	work->in_flag &= ~FIRST_APPEAR ;
	SetSpModeAllOff(work) ;
	work->ef_cansel_cnt = 3 ;
	work->speed = DG_ZeroVector ;
	work->control.mov = work->appear_pos ;
	work->control.mov.vy -= 6000.0f ;
	work->bas_mot = MOT_SWIM ;
	ForceChangeMot(work,work->bas_mot,INTERP) ;
	GM_ActMotion( &work->body ) ;
	work->now_pos = 0 ;
}

#if 1
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
			case MSG_KILL :
printf("MSG_KILL RECIEVED!!!!!!\n");
				GV_DestroyActor(work) ;
				break;
			case MOT_WAKE :
				MotWakeUp(work);
				break;
			case MSG_DEMO_CANSEL :
				SetDemoCansel(work);
				work->in_flag |= DEMO_END ;
				break;
		}
		msg++ ;
	}
}
#endif

static void SetNextPoint(Work *work){
	work->now_pos++ ;
	if(work->now_pos>=work->max_pos){
		work->now_pos = 0 ;
	}
}
static void SetNextPointRnd(Work *work){
	int rnd_pos ;

	rnd_pos = ((irnd()>>8)%work->max_pos) ;
	if(work->now_pos == rnd_pos){
		work->now_pos += 1 ;
		work->now_pos %= work->max_pos ;
	}else {
		work->now_pos = rnd_pos ;
	}
}

#ifdef DEBUG_MODE
static void SetPosCheck(Work *work){
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
	int i ;
	for (i=0;i<work->max_pos;i++){
		PosBox(&work->pos[i],1000.0f,NULL) ;
	}
}
#endif

static void HamonEfCall(Work *work){
	if(work->bas_mot != MOT_SWIM){
//	if(work->dive_wait == 0) {
		if(((irnd()>>8)&63)==0){
			NewWaterSurfaceMountain( &work->control.mov,3500.0f,1.0f, 0x01 ) ; 
		}
	}
}









static void SwimGll(Work *work )
{
	/*潜水モード*/
	float dis ;
	short dir,dir_sub;
	FVECTOR trg_pos ;
	SVECTOR	rot ;
	int rnd_res ;
	if(!(work->status & PAUSE_MOT )){


   //BP JG - Changed switch logic to ifelse so we can use DIRECT_TICK as a function.
   if ( work->mot_cnt == 1 )
      GM_SeSetMode(SD_A_GSWIM_02,&work->control.mov,GM_SEMODE_BOMB) ;
   else if ( work->mot_cnt == DIRECT_TICK(28) ) /*17/0.6*/
       GM_SeSetMode(SD_A_GSWIM_01,&work->control.mov,GM_SEMODE_BOMB) ;
   else if ( work->mot_cnt == DIRECT_TICK(56) ) /*34*0.6*/
       GM_SeSetMode(SD_A_GSWIM_02,&work->control.mov,GM_SEMODE_BOMB) ;
   else if ( work->mot_cnt == DIRECT_TICK(90) ) /*54*0.6*/
       GM_SeSetMode(SD_A_GSWIM_01,&work->control.mov,GM_SEMODE_BOMB) ;

#if 0
      switch(work->mot_cnt)
      {
			case 1 :
				GM_SeSetMode(SD_A_GSWIM_02,&work->control.mov,GM_SEMODE_BOMB) ;
				break ;
			case DIRECT_TICK(28) : /*17/0.6*/
				GM_SeSetMode(SD_A_GSWIM_01,&work->control.mov,GM_SEMODE_BOMB) ;
				break ;
			case DIRECT_TICK(56) : /*34*0.6*/
				GM_SeSetMode(SD_A_GSWIM_02,&work->control.mov,GM_SEMODE_BOMB) ;
				break ;
			case DIRECT_TICK(90) :/*54*0.6*/
				GM_SeSetMode(SD_A_GSWIM_01,&work->control.mov,GM_SEMODE_BOMB) ;
				break ;
			default :
				;
		}
#endif

		/*目標に向けて泳ぐ*/
		if(work->in_flag & FIRST_APPEAR){
			dis = GV_VecLen3F2( &work->control.mov,&work->appear_pos ) ;
		}else {
			dis = GV_VecLen3F2( &work->control.mov,&work->pos[work->now_pos] ) ;
		}
		/*ゴールが近ければ減速*/
		if(dis< 12000.0f){
			work->speed.vx *= 0.95f ;
			work->speed.vz *= 0.95f ;
		}
		if(GV_VecLen3F( &work->speed )> SPEED_LIMIT){
			work->speed.vx *= 0.80f ;
			work->speed.vz *= 0.80f ;
		}

		if(dis< 7000.0f){
//			work->speed.vx *= 0.95f ;
//			work->speed.vz *= 0.95f ;
		}
		if(dis< 3000.0f){
			/*到達*/
			rnd_res = ((irnd()>>8)%work->rand_rate);

//printf("rnd_res[%d] rand_rate[%d]\n",rnd_res,work->rand_rate);

			if((rnd_res==0)||(work->in_flag & FIRST_APPEAR)){
				/*浮上*/
				work->in_flag &= ~FIRST_APPEAR ;
				if(work->proc_id != 0){
					GM_ExecProc( work->proc_id, NULL );
					work->proc_id = 0 ;
				}
				/*浮上の瞬間のエフェクトを少し遅らせる*/
				work->ef_cansel_cnt = 10 ;
//				work->dive_wait = 0 ;
				work->bas_mot = MOT_RISE ;
//				GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
				MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE*DEF_MOT_SPEED ) ;
				ForceChangeMot(work,work->bas_mot,INTERP) ;
				if(work->in_flag & DEMO_END){
					NewPadVibrationRadio(APPEAR_STR);
				}
			}else {				/*フェイント*/
//				SetNextPoint(work) ;
				SetNextPointRnd(work) ;
			}
		}

		if(work->in_flag & FIRST_APPEAR){
			trg_pos = work->appear_pos ;
		}else {
			trg_pos = work->pos[work->now_pos] ;
		}

		dir_sub = GV_DiffDirS( work->control.turn.vy, 
			GV_VecDir2FromTo( &work->control.mov , &trg_pos ) ) ;
		work->control.turn.vy += dir_sub/20 ;
		dir = GV_VecDir2FromTo(&work->control.mov,&trg_pos ) ;
		rot.vx = 0 ;	rot.vy = dir ;	rot.vz = 0 ;
		GV_DirVec3( &rot, ACCEL, &work->accel ) ;
		work->speed.vx += work->accel.vx ;
		work->speed.vz += work->accel.vz ;
	}
}

static void FloatingGll(Work *work){
	/*浮上中*/
	short dir_sub ;
	int next_pos ;

	work->shift = work->body.m_ctrl->height * HEIGHT_SCALE ;

	next_pos = work->now_pos + 1 ;
	if(next_pos >= work->max_pos){
		next_pos = 0 ;
	}

	if(work->bas_mot == MOT_RISE){
		dir_sub = GV_DiffDirS( work->control.turn.vy, 
			GV_VecDir2FromTo( &work->control.mov , &GM_PlayerPosition  ) ) ;
	}else {
		/*(work->bas_mot == MOT_DIVE)*/
		dir_sub = GV_DiffDirS( work->control.turn.vy, 
			GV_VecDir2FromTo( &work->control.mov , &work->pos[next_pos]  ) ) ;
	}

	work->control.turn.vy += dir_sub/40 ;

	work->speed.vx *= 0.95f ;
	work->speed.vz *= 0.95f ;

}


static void SetSwimDepth(Work *work){
	/*潜水深度処理*/
	if(work->bas_mot == MOT_RISE){
		if(work->shift < 0.0f) work->shift += FLOAT_SPEED ;
		if(work->mot_cnt==DIRECT_TICK(9) ){
				CamShake(60,70);
		}
		if(work->mot_cnt==DIRECT_TICK(42) ){
//				CamShake(30,60);
		}
		if(work->mot_cnt==DIRECT_TICK(100) ){
			CamShake(60,70);
		}
		SetSpModeRise(work) ;
	}else if(work->bas_mot == MOT_DIVE){
		if(work->mot_cnt==DIRECT_TICK(70) ){
				CamShake(60,120);
		}
		SetSpModeDive(work) ;
	}else if(work->bas_mot == MOT_SWIM){
		SetSpModeSwim(work) ;
		if(!(work->status & PAUSE_MOT )){
			if(work->shift < SHIFT_DEPTH_MIN ) work->shift += FLOAT_SPEED ;
		}
		if((work->mot_cnt%DIRECT_TICK(12))==0 ){
//				CamShake(5,5);
		}
	}
}


static void Act(Work *work)
{
#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif
	OBJECT *body ;
	CONTROL	*ctrl ;
	int i;
	body = &work->body ;
	ctrl = &work->control ;

#ifdef DEBUG_MODE
	if(work->test_flag) SetPosCheck(work) ;
#endif

	ResetPhotoCheck() ;


	work->ef_pos = work->control.mov ;
	work->ef_pos.vy = work->control.mov.vy + isin((work->cnt*150)&4095) * EF_SIN_LEN ;


	if(work->bas_mot == MOT_RISE){
		/*浮上中のみ写真合格*/
		if(work->mot_cnt>=DIRECT_TICK(26)){
			SetPhotoCheck() ;
		}
	}else if(work->bas_mot == MOT_DIVE){
		if(work->mot_cnt<=DIRECT_TICK(38)){
			SetPhotoCheck() ;
		}
	}
#ifdef DEBUG_MODE 
	if(work->test_flag) {
		if(GLL_Photo_Check & GLL_PHOTO_OK){
extern void PosBox2(FVECTOR *,float ,int,int,int) ;

			PosBox2(&work->control.mov, 5000.0f,60,60,200) ;
		}
	}
#endif
	/*波紋制御*/
	HamonEfCall(work) ;

	if((work->status & NO_XZ_MOVE)||(work->status & PAUSE_MOT )){
		work->control.step.vx = 0.0F;
		work->control.step.vz = 0.0F;
	}

	/*潜水深度処理*/
//	SetSwimDepth(work) ;




	if(!(work->status & PAUSE_MOT )){
		if(work->bas_mot == MOT_SWIM){
//		if(work->dive_wait == 1){
			/*潜水モード*/
			SwimGll(work) ;
		}else {
			/*浮上モード*/
			FloatingGll(work) ;
		}
		work->speed.vx *= 0.99f ;
		work->speed.vz *= 0.99f ;
		if(work->bas_mot == MOT_SWIM){
//		if(work->dive_wait) {
			work->control.step.vx = work->speed.vx ;
			work->control.step.vz = work->speed.vz ;
		}
	}

	GM_ActControl(&work->control) ;



	if((!(work->status & PAUSE_MOT ))&&(work->ef_cansel_cnt == 0)
	&&(
	((work->bas_mot == MOT_SWIM)&&((DG_TickCount&63)==0))
	||((work->bas_mot == MOT_RISE)&&(work->mot_cnt==16))
	)
	){
		/*海面揺らし*/
extern int TAKABE_AddWaveForce( FVECTOR *, float ) ;
		FVECTOR sub ;
		float len ;
		len = GV_VecLen3F2( &work->last_pos,&work->control.mov) ;
//printf("WAVE FORCE [%f]\n",len);
//		TAKABE_AddWaveForce( &work->control.mov, len * 10.0f ) ;
		if(work->bas_mot == MOT_RISE){
			TAKABE_AddWaveForce( &work->control.mov, RISE_FORCE ) ;
		}else {
			TAKABE_AddWaveForce( &work->control.mov, SWIM_FORCE ) ;
		}
	}
	work->last_pos = work->control.mov ;

	DG_GetLightMatrix( &work->control.mov, work->lights );
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;


	if(work->bas_mot != MOT_SWIM){
		if(work->bas_mot == MOT_RISE){
			if(work->mot_cnt<3){
				work->control.mov.vy = work->pos[work->now_pos].vy 
					+ (work->body.m_ctrl->height*HEIGHT_SCALE) - 5000.0f ;
			}else {
				work->control.mov.vy = work->pos[work->now_pos].vy 
					+ (work->body.m_ctrl->height*HEIGHT_SCALE) ;
			}
		}else {
			work->control.mov.vy = work->pos[work->now_pos].vy + (work->body.m_ctrl->height*HEIGHT_SCALE) ;
		}
	}else {
		work->control.mov.vy = work->pos[work->now_pos].vy + work->shift ;
	}


	/*振動制御*/
//	GM_ActVctrl( &work->vctrl ) ;	/* ActMotionの上 */

	GM_ActObject(body) ;

    MT_SetMotionSeTable( work->body.m_ctrl, work->control.map, 1,  (0xffff) ,(0xffff) ) ;

	work->mot_cnt++ ;


	work->control.height = work->body.m_ctrl->height ;


	/*潜水深度処理*/
	SetSwimDepth(work) ;

	if(work->bas_mot == MOT_RISE){
//		if(work->mot_cnt==106 ){
		if(work->in_flag & FIRST_APPEAR){
			if(work->mot_cnt == DIRECT_TICK(64) ){
				GM_SeSetMode(SD_A_GROW_SED,&work->control.mov,GM_SEMODE_BOMB) ;
			}
		}else {
			if(work->mot_cnt== DIRECT_TICK(64) ){
				GM_SeSetMode(SD_A_GROW_SEG,&work->control.mov,GM_SEMODE_BOMB) ;
			}
		}
		if(work->mot_cnt==DIRECT_TICK(101) ){
			/*上昇から潜水へ*/
			work->bas_mot = MOT_DIVE;

			if(work->in_flag & DEMO_END){
				NewPadVibrationRadio(DIVE_STR);
			}
			ChangeMot(work,work->bas_mot,RISE_INTERP) ;
		}
	}

//	if( GM_CheckObject_PlayEnd( body, 0 ) ) {
	if( GM_GetObjectMotionEnd( body, 0 ) ) {
		{
			/*モーション終了判定*/
			if(work->bas_mot == MOT_RISE){
				/*上昇から潜水へ*/
				work->bas_mot = MOT_DIVE;
			}else if(work->bas_mot == MOT_DIVE){
				/*潜水から泳ぎへ*/
				work->in_flag |= DEMO_END ;
				MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE*SWIM_MOT_SPEED ) ;
				work->bas_mot = MOT_SWIM ;
				/*目標位置更新*/
				SetNextPointRnd(work) ;
//				GM_VctrlStopVibration( &work->vctrl ) ;
//				SetNextPoint(work) ;
//				work->dive_wait = 1 ;
			}else if(work->bas_mot == MOT_SWIM){
				/*NOP*/
			}
			/*モーション変更処理*/
			ChangeMot(work,work->bas_mot,INTERP) ;
		}
	}
	CheckMessage(work);
	/*海面へのforce決定のため前フレームでの座標保持*/
	work->last_pos = work->control.mov ;
	if(work->ef_cansel_cnt>0){
		SetSpModeAllOff(work) ;
		work->ef_cansel_cnt-- ;
	}
	work->cnt++;
}
static void Die(Work *work)
{
	HZX_FlashTrap( work->control.hzx_id, &work->control.evt ) ;

	if(work->in_flag & ITEM_SET){
		extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
		DG_DisconnectObjs( work->body.objs, work->item.objs ) ;
		GM_FreeObject(&(work->item));
	}
	GM_FreeObject(&(work->body));
	if(work->status & RAD_POINT){
		GM_FreeRadarControl(&work->rctrl);
	}
	GM_FreeControl( &work->control);
}
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model,motion,item,set_num = -1;
	int body_flag = 0 , i;

//	GM_WaterLevel += 500.0f ;


	work->name = name ;
//	work->dive_wait = 1 ;
	work->lit_def = GM_GetMap( where )->light ;
	work->speed = work->accel = DG_ZeroVector ;
	work->status = (RAD_POINT) ;
	work->mot_cnt = 0;

	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->rand_rate = GCL_GetNextInt() ;
		if(work->rand_rate<=0) {
			printf("GOLULU RAND RATE ERROR!!!!\n");
			work->rand_rate = 1 ;
		}
	}else {
		work->rand_rate = 1 ;
	}

	work->shift = SHIFT_DEPTH_MIN - 2000.0f;

	work->now_pos = 0 ;
	if ( GCL_GetOption( 't' ) != NULL ) {
		work->test_flag = GCL_GetNextInt() ;
	}else {
		work->test_flag = 0 ;
	}
	work->in_flag = FIRST_APPEAR ;

	GM_InitControl( &work->control, name, where );
	GM_ConfigControlMessageCheck( &work->control ) ;
	GM_ConfigControlTrapCheck( &work->control ) ;
	work->control.interp = 8 ;

//	GM_ConfigControlHazard( &work->control, 500, 100, 110 ) ;
//	GM_ConfigControlMapCheck( &work->control ) ;


	work->control.seg_flag |= HZX_SEG_NO_PLAYER ;
	work->control.flr_flag |= HZX_FLOOR_NO_PLAYER ;

	/*CONTROLフラグ*/
	/*壁チェックなし*/
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
	work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
	/*シナリオリード*/
	/*方向*/
	work->control.turn.vy = 0 ;
	work->control.rot.vx = 0 ;
	work->control.rot.vy = work->control.turn.vy ;
	work->control.rot.vz = 0 ;

	/* 座標 */
	if ( GCL_GetOption( 'p' ) ){
		for(i=0;i<MAX_POS;i++){
			if(GCL_NextStr() != NULL){ 
				GCL_GetIV( GCL_NextStr(), buf ) ;
				vu0_IV0toFV( (IVECTOR *)buf, &work->pos[i] ) ;
			}else {
				break ;
			}
		}
		work->max_pos = i ;
	}else {
		work->max_pos = 0 ;
		printf("SET POSITION !!!\n") ;
		ASSERT(0) ;
	}




	if( GCL_GetOption( 's' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->start_pos ) ;
	}else {
		work->start_pos = work->pos[0] ;
	}

	if( GCL_GetOption( 'b' ) ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		vu0_IV0toFV( (IVECTOR *)buf, &work->appear_pos ) ;
	}else {
		work->appear_pos = work->pos[0] ;
	}

	work->control.mov = work->start_pos ;

	GM_ConfigControlMapID( &work->control ) ;

	/* モデル */
	if ( GCL_GetOption( 'k' ) == NULL ) {
		model = GV_StrCode("sna_def") ;
	} else {
		model = GCL_GetNextInt() ;
	}
	work->model = model ;

	/* モーションファイル */
	if ( GCL_GetOption( 'm' ) == NULL ) {
		motion = GV_StrCode("locker_cp") ;
	} else {
		motion = GCL_GetNextInt() ;
	}
	/* モーション番号 */
	work->bas_mot = MOT_SWIM ;

	/* 装備品 */
	item = -1;
	if ( GCL_GetOption( 'i' ) != NULL ) {
		item = GCL_GetNextInt() ;
		set_num = GCL_GetNextInt() ;
    }

	body_flag = OBJECT_FLAG ;
	/*赤外線*/
	body_flag |= DG_FLAG_IRREACTION ;
	GM_InitObject(&(work->body),model,body_flag );
	if(item != -1 ){
		extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;

		work->in_flag |= ITEM_SET ;
		GM_InitObject(&(work->item),item,ITEM_FLAG);
		GM_ConfigObjectRoot(&work->item,&work->body,set_num );
		GM_ConfigObjectLight(&(work->item),work->lights) ;
		DG_ConnectObjs( work->body.objs, work->item.objs ) ;
	}

	/*モーション終了時プロック*/
	if ( GCL_GetOption( 'a' ) != NULL ) {
		work->proc_id = GCL_GetNextInt() ;
	}else {
		work->proc_id = 0 ;
	}


	/*ctrlからobjectを参照*/
	GM_ConfigControlObject( &work->control, &work->body ) ;
	GM_ConfigObjectStep( &work->body,&work->control.step ) ;
	GM_ConfigObjectMotion( &(work->body), 0,
		motion,MT_FLAG_HUMAN2);

ASSERT(work->body.m_ctrl != NULL) ;


	GM_ConfigObjectAction( &work->body, 0, work->bas_mot, 0,0xfffff,0);

	/*初期姿勢にする*/
	GM_ActMotion( &work->body ) ;

	work->control.mov.vy
	= work->start_pos.vy + work->body.m_ctrl->height * HEIGHT_SCALE + work->shift ;


	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	//DG_PutObjs( work->body.objs );
    //GM_GroupObjs( work->body.objs, where ) ;
	GM_ActObject2( &work->body ) ;

	GM_ConfigObjectLight(&(work->body),work->lights) ;
	if(item != -1 ){
	    GM_GroupObjs( work->item.objs, where ) ;
		GM_ConfigObjectLight(&(work->item),work->lights) ;
	}

	/*一時停止*/
	MotSleep(work) ;

	if(work->status & RAD_POINT){
		GM_InitRadarControl(&work->rctrl,&work->control.mov,
//			RADAR_VISIBLE|RADAR_SIGHT, work->control.map ) ;
			RADAR_VISIBLE, work->control.map ) ;
		work->rctrl.col = 0 ;	/* 視野描画色 */
		GM_RadarSetVRange( &work->rctrl, 50000 , -50000 );
	}

	work->sp_sw_head = 0;
	work->sp_sw_koshi = 0;
	work->sp_sw_hands = 0;
	work->sp_sw_legs = 0;
	work->sp_sw_sin = 0;
	work->ef_cansel_cnt = 0 ;

	GV_SetActorChild( work, 
//		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_KOSHI].world.m[3],3000.0f,&work->sp_sw_koshi));
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_ONAKA].world.m[3],3000.0f,&work->sp_sw_koshi));
	GV_SetActorChild( work, 
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_ATAMA].world.m[3],2000.0f,&work->sp_sw_head));
	GV_SetActorChild( work, 
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_MIGI_TE].world.m[3],1000.0f,&work->sp_sw_hands));
	GV_SetActorChild( work, 
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_HIDARI_TE].world.m[3],1000.0f,&work->sp_sw_hands));
	GV_SetActorChild( work, 
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_MIGI_KAKATO].world.m[3], 2000.0f,&work->sp_sw_legs));
	GV_SetActorChild( work, 
		NewGllSplush((FVECTOR*)&work->body.objs->objs[HUMAN21_HIDARI_KAKATO].world.m[3],2000.0f,&work->sp_sw_hands));
	GV_SetActorChild( work, 
		NewGllSplush(&work->ef_pos,2000.0f,&work->sp_sw_sin));

#if 0
	NewGllSplushVertical(&work->control.mov, 2500.0f, 1.0f, 1 ) ;
#endif
	GLL_Photo_Check = 0 ;
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE*SWIM_MOT_SPEED ) ;


#if 0
	{
	extern void *NewGolBreakPart2(DG_OBJ *obj,int *sw,float len1 ,float len2 ,int time_a1,int time_a2,
		float scale1,float scale2,int re_time) ;

		NewGolBreakPart2(&work->body.objs->objs[HUMAN21_HIDARI_ASHI2],
			&work->test_flag, 10.0f,13.0f,10,180,0.4f,0.8f,120) ;
	}
#endif

	/*振動制御*/
//	GM_InitVarControl( &work->actor, &work->vctrl, motion ) ;
//	GM_ConfigVctrlMctrl( &work->vctrl, work->body.m_ctrl ) ;
//void	GM_SetVibrationScale( void *actor, float scale )
//	GM_SetVibrationScale( &work->actor, 0.3f ) ;


	return 1;
}

void *NewGllPhoto( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
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

