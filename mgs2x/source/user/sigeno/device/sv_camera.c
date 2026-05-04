/********************************************************************************/
/*	Sv_camera.c								*/
/*	監視カメラ  *NewSvCamera						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: sv_camera.c,v 1.1.1.3 2002/11/19 11:49:29 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "korekado/enemy/enemy.h"
//里吉追加
#include "camera.h"
#include "satoyoshi.h"

//#include "enemy.x"
#include "eyecheck.h"

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern int DEV_EnemyCheck(EYEPARAM * , HZX_GROUP_ID, Work * ) ;

#ifdef DEBUG_MODE 
extern void *NewEyeView(FMATRIX *,int,int,int,int,COMMANDER *);
#endif

extern void *NewFlyingSmoke2( FVECTOR * , FVECTOR * , int );

//extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;
extern void *NewSpark( FMATRIX   *world ) ;
extern void *NewGeneralSprite(int,FVECTOR *,float,int *,int,int,int,int * );
extern void *NewCrushWithForce( FVECTOR *pos, FVECTOR *force );
extern float _RsinF( int ) ;
extern float _RcosF( int ) ;

extern void GSprite_TexChange( void *p, int tex_code );
extern void PosBox(FVECTOR *,float ,SVECTOR *) ;


/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define GSPRT_MODE_TEX		(0x00000000)   	// 描画モード　テクスチャーそのまま　起動時のみ
#define GSPRT_MODE_ADD		(0x00010000)	// 描画モード　加算　起動時のみ
#define GSPRT_MODE_SUB		(0x00020000)	// 描画モード　減算　起動時のみ
#define GSPRT_MODE_ALPHA	(0x00030000)	// 描画モード　アルファ　起動時のみ

#define	MAX_VEC		(150.0f)

#define SVC_COUNT	(BP_BASE_TICK())

         /******シナリオ指定ステータス******/
#define SVC_SCN_BROKEN1 (0x01)

         /*****work.status*****/
#define	SV_BLIND	(0x01)	/*視界閉じ レーダーそのまま*/

#define	SV_WAIT_TIME	(SVC_COUNT*10) /*首振り終端で停止時間*/
#define	SE_START	(SVC_COUNT*30)	/*破壊後エフェクト用カウンタ*/

         /*ターゲット設定*/
#define HEAD_SIZE_XY (50.0F)
#define HEAD_SIZE_Z (120.0F)
#define LENS_SIZE_Z (40.0F)

#define HEAD_SHIFT_Y (70.0F)
#define HEAD_SHIFT_Z (40.0F)

#if 1
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#else
#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#endif

#define	TURN_SPEED (BP_AdjustTick5(5))

//#define	TURN_SPEED (100)

#define	SVC_EYE_S_DEF		(4000)	//デフォルトの視力
#define	DIR_X_MAX		(512)	//限界値
#define	DIR_Y_MAX		(2000)	//限界値

#define	CAMERA_PARTS		(1)	//カメラ部分のモデルナンバ

#define SVC_BROKEN_MDL		(2526173)	//監視カメラモデルID
#define SVC_NORMAL_MDL		(121635)	//破壊監視カメラモデルID

#define B_SIZE			(80)		//動作ランプのサイズ
#define B_RISE			(40.0F)		//動作ランプの表示優先


#define CHF_CYC	(BP_FRAMES_PER_SEC())	/*チャフ時揺れ周期*/


/*駆動制御*/
enum{
	RIGHT_TURN ,
	LEFT_TURN ,
};

enum {
MSG_SIGHT_OFF = 1,
MSG_SIGHT_ON
};


/**************************<-------local function------>*************************/
/*	名前:	void SV_ActChaff						*/
/*	引数:	Work *work   						       	*/
/*	説明:	チャフ時の振舞いを制御する		       			*/
/********************************************************************************/
static void SV_ActChaff(Work *work){

    work->ctrl.turn.vx &=4095 ;
    work->ctrl.turn.vy &=4095 ;

    if((work->chf_cnt % CHF_CYC) == 0){	//チャフサイクル(CHF_CYC)毎に振舞い変化
	work->chf_cnt = 0;
	work->rnd_speed.vx = (irnd()&15)-7;
	work->rnd_speed.vy = (irnd()&15)-7;
	if((irnd()&1)) work->rnd_speed.vx *= -1 ;
	if((irnd()&1)) work->rnd_speed.vy *= -1 ;
    }

    /*LIMIT*/
    work->ctrl.turn.vx += work->rnd_speed.vx ;
    work->ctrl.turn.vy += work->rnd_speed.vy ;

    work->ctrl.turn.vz = 0 ;
    work->chf_cnt++;		 
}


/**************************<-------local function------>*************************/
/*	名前:	void SetRGBA							*/
/*	引数:	int r,g,b	RGBの輝度				       	*/
/*	    :	int a   	αチャンネル輝度			       	*/
/*	説明:	RGBAをセットする			       			*/
/********************************************************************************/
static void SetRGBA(int r ,int g ,int b ,int a,int *rgba){
    *rgba = ((a<<24)|(b<<16)|(g<<8)|(r)) ;
} 


/**************************<-------local function------>*************************/
/*	名前:	void AlertColSet						*/
/*	引数:	Work *work							*/
/*	説明:	警戒色をセットする						*/
/********************************************************************************/
static void AlertColSet(Work *work){
    if(work->alert == GM_AlertMode) return ;
    switch (GM_AlertMode){
    case ALERT_MODE_ALERT :
	SetRGBA(127 ,30 ,30 ,127, &work->rgba);
	break ;
    case ALERT_MODE_AVOID :
	SetRGBA(127 ,127 ,30 ,127, &work->rgba);
	break ;
    case ALERT_MODE_SEARCH :
	SetRGBA( 0 ,100 ,30 ,127, &work->rgba);
	break ;
    case ALERT_MODE_SNEAK :
	SetRGBA( 0 ,100 ,30 ,127, &work->rgba);
	break ;
    }
    work->alert = GM_AlertMode ;
}


/**************************<-------local function------>*************************/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work							*/
/*	説明:	メッセージをチェックする       					*/
/********************************************************************************/
static	void	CheckMessage( work )
Work	*work ;
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->ctrl.n_msg ;
    msg = work->ctrl.msg ;
    
    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ] ;
	switch( code ) {
	case MSG_SIGHT_OFF :
	    /*****視界ＯＦＦ*****/
	    work->sight_stat |= SV_BLIND ;
	    break ;
	case MSG_SIGHT_ON :
	    /*****視界ＯＮ*****/
	    work->sight_stat &= SV_BLIND ;
	    break ;
	}
	msg++ ;
    }
}


#ifdef DEBUG_MODE
/**************************<-------local function------>*************************/
/*	名前:	void FindPosChecker						*/
/*	引数:	Work *work							*/
/*	説明:									*/
/********************************************************************************/
static void FindPosChecker(Work *work)
{
    SVECTOR	rgb ;
    FVECTOR	line[2] ;

    line[0] = work->eye_pos ;
    line[1] = *work->eye.trgpos ;
	
    rgb.vx = 255; rgb.vy = 0; rgb.vz = 0;
    PosBox(work->eye.trgpos,25.0F ,&rgb) ;
    NewLineView( &line[0] ,1,0,0,255) ;
}
#endif


#if 0
/**************************<-------local function------>*************************/
/*	名前:	void CamSeCheck							*/
/*	引数:	Work *work							*/
/*	説明:									*/
/********************************************************************************/
static void CamSeCheck(Work *work){
    int dirsub ;
    if((DG_TickCount - work->se_tick) <= CAM_SE_INT){
	work->y_buf = work->ctrl.rot.vy ;
	return ;
    }
    dirsub = DEV_CheckDirSub(work->y_buf, work->ctrl.rot.vy) ;
    if(abs(dirsub)> 1){
	GM_SeSetMode( SD_E_CAMMOV01, &work->ctrl.mov,GM_SEMODE_NORMAL ) ;
	work->se_tick = DG_TickCount ;
    }
}
#endif

/**************************<-------local function------>*************************/
/*	名前:	void CallSpark							*/
/*	引数:	FVECTOR *pos							*/
/*	引数:	FVECTOR *force							*/
/*	説明:	火花を呼ぶ	       						*/
/********************************************************************************/
static void CallSpark(FVECTOR *pos ,FVECTOR *force) {
    SVECTOR	rot ;
    FMATRIX	w ;

    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}

/**************************<-------local function------>*************************/
/*	名前:	void Call_spark							*/
/*	引数:	Work	worl							*/
/*	説明:									*/
/********************************************************************************/
static void Call_spark(Work *work){

    FVECTOR pos,force ;
    int dir ;
    if(work->se_cnt ==0) return ;
    if(work->se_cnt == (SE_START-3)){		/* 本体破壊音 */
	GM_SeSetMode( SD_E_ECODBR01, &work->camera_pos, GM_SEMODE_BOMB );
    }
    if(work->se_cnt < (SE_START - SVC_COUNT*5) ){	/* 火花破壊音 */
	if(!((irnd()>>8)&15)){		//ＰＡＬ版もこの確率で良いのでしょうか- - - 
	    GM_SeSetMode( SD_E_HIBANA02, &work->camera_pos, GM_SEMODE_BOMB );
	    dir = ((irnd()>>8)&4095) ;
	    force.vx = 100.0F * _RcosF( dir ) ;
	    force.vz = 100.0F * _RsinF( dir ) ;
	    force.vy = +1.0F * (float)((irnd()>>8)&127) ;
	    pos = work->eye_pos ;
	    pos.vx -= (force.vx*3/4) ;
	    pos.vz -= (force.vz*3/4) ;
	    CallSpark( &pos , &force) ;
	}
    }

    work->se_cnt--;
}

/**************************<-------local function------>*************************/
/*	名前:	void SetFaceDir							*/
/*	引数:	Work	*woek							*/
/*	説明:	カメラの向きをセット   						*/
/********************************************************************************/
static void SetFaceDir(Work *work){
    work->eye.rot = work->ctrl.rot ;
}

/**************************<-------local function------>*************************/
/*	名前:	CheckDirSub							*/
/*	引数:	int	dir1							*/
/*	引数:	int	dir2							*/
/*	説明:			       						*/
/********************************************************************************/
int CheckDirSub(int dir1,int dir2){
    int sub;
    sub = dir2 - dir1;
    sub	&= 4095;
    if(sub >= 2048) sub= -(4096-sub);
    return sub;
}	

/**************************<-------local function------>*************************/
/*	名前:	void SetDir							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void SetDir(Work *work){
    int dirsub ;
    dirsub = CheckDirSub(work->now_dir , work->max_dir) ;
    switch(work->mode){

    case RIGHT_TURN :	//*****右回転	      
	dirsub = work->max_dir - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt-- ;
	}else if ( (dirsub > TURN_SPEED) ){
	    work->now_dir += TURN_SPEED ;
        } else {
	    work->wait_cnt = SV_WAIT_TIME ;
	    work->mode = LEFT_TURN ;
	}
	break;

    case LEFT_TURN :	//*****左回転
        dirsub = (-work->max_dir) - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt-- ;
        }else if( (abs(dirsub) > TURN_SPEED ) ){
	    work->now_dir -= TURN_SPEED ;
	}else {
	    work->wait_cnt = SV_WAIT_TIME ;
	    work->mode = RIGHT_TURN ;
	}
        break;
    }
    work->ctrl.turn.vx = work->rot.vx ;
    work->ctrl.turn.vy = work->center+work->now_dir ;
    work->ctrl.turn.vz = 0 ;
}


/********************************************************************************/
/*	名前:	void SetObjs							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void SetObjs(Work *work){
    SVECTOR base_rot;
    FVECTOR	b_shift,eye_shift ;

//	動作ランプ    
    b_shift.vx = 30.0F ;
    b_shift.vy = 100.0F ;
    b_shift.vz = 160.0F ;

//	レンズ部分
    eye_shift.vx = 0.0F;
    eye_shift.vy = 50.0F;
    eye_shift.vz = 150.0F;
    
    base_rot = work->rot;
    base_rot.vx = 0 ;
    
    GM_GroupObjs( work->body.objs, work->map );	//マップグループに登録

    /*****土台部分*****/
    DG_SetPos2( &work->ctrl.mov, &base_rot ) ;
    DG_PutObjs( work->body.objs );	//現在位置にオブジェを配置
    DG_GetPos( &work->body.objs->objs[0].world  ) ;	//オブジェクトの位置を設定

    /*****カメラ部分*****/
    work->rctrl.dir = work->ctrl.rot.vy ;	//レーダ
    DG_SetPos2( &work->camera_pos, &work->ctrl.rot ) ;
    DG_GetPos( &work->body.objs->objs[1].world  ) ;

    /*****発光位置をシフト*****/
    work->b_pos.vw = 1.0F ;
    DG_PutVector( &b_shift, &work->b_pos, 1 ) ;	//光源位置

    /****視点の原点****/
//    DG_SetPos2( &work->camera_pos, &work->ctrl.rot ) ;
    DG_PutVector( &eye_shift, &work->eye_pos, 1 );	//視点を設置
    work->b_pos.vw = 1.0F ;

#ifdef DEBUG_MODE 
    DG_SetPos2( &work->camera_pos, &work->ctrl.rot ) ;
    DG_MovePos( &eye_shift ) ;
    DG_GetPos(&work->eyeview) ;
#endif

}


/**************************<-------local function------>*************************/
/*	名前:	void SV_ActNormal						*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void SV_ActNormal(Work *work){
    short	tmp_uniq_id;
    short	loop;

    if(GM_CheckGameStatus(STATE_CHAFF)){	//*チャフチェック
	SET_G1_STEP(work, MOD_CHAFF);
	return;
    }    


    //この記述だとまずい 消えて現れた時とか
    else if(work->sight_stat &  SV_BLIND ){
	/*視界off*/
	DEV_No_Sight( &work->eye );	//敵が見えない
    }




    switch (G2_STEP)//****************ステップ分岐****************
    {

    case SUB_NORMAL:	//*****ノーマルモード*****

	//視界内の敵チェック
	if (DEV_EyeInfoCheck( &work->eye ,work->ctrl.hzx_id )){
	    SET_G2_STEP(work, SUB_WARN);
	    return;
	}

	//視界内の死体＆ダメージ兵チェック
	if( (GM_AlertMode == ALERT_MODE_SEARCH)||(GM_AlertMode == ALERT_MODE_SNEAK) ){
	    tmp_uniq_id = DEV_EnemyCheck( &work->eye ,work->ctrl.hzx_id,work );   

	    if(tmp_uniq_id){    
		//******通報済みリストに追加
		loop=0;
		while (work->uniq_id_list[loop] != -1){
		    loop++;
		}
		work->uniq_id_list[loop] = tmp_uniq_id;	//ユニークIDを保存
		loop++;
		if (loop >= UNIQ_LIST_MAX){
		    loop=0;
		}
		work->uniq_id_list[loop] = -1;		//リストを１つあける


		//	    DEV_In_SightSub( &work->eye, &work->eye.trgpos, tmp_uniq_id-1) ;
		SET_G2_STEP(work, SUB_WARN2);
		return;
	    }
	}

	//*探査中の首振り
	SetDir(work);
	break;


    case SUB_WARN2:	//*****白びっくり発動待ち
    {
	EYEPARAM *eye = &work->eye ;

	if( ((eye->discv2 ==0 )&&(eye->alert2 == SIGHT_ALERT2)) ){
	    COM_SetAccident( eye->accident_uniq_id, &eye->accident_pos,	//チェックの通報
			     eye->accident_hzx_id, DEV_ACCIDENT_DELAY_TIME ) ;
	    SET_G2_STEP(work, SUB_ALERT2);		//白びっくり後処理

	}

	//時間になったらびっくりマークを表示
	DEV_HeadMarkCheck(&work->eye,
			  &work->body.objs->objs[CAMERA_PARTS].world,&work->headmark) ;

	if( DEV_EnemyStatus_Id(eye) == 0){	//途中で起きたらキャンセル
	    SET_G2_STEP(work, SUB_NORMAL);
	    eye->alert2 = eye->discv2 = 0;
	}

	//目標物へ向く
	SIG_VecDir(work->eye.eyepos,&work->eye.accident_pos,
		   &work->ctrl.turn);
	break;
    }

    case SUB_ALERT2:	//*****警戒モード*****

       if(work->eye.alert2 > 0 ){
	    //*ダメージ兵のほうを向く*/
	    SIG_VecDir(work->eye.eyepos,&work->eye.accident_pos,
		       &work->ctrl.turn);
	    work->eye.alert2--;
	}

	if( (work->eye.alert2 >0)&&(work->eye.alert2 < RADAR_RADIO_TIME) ){
	    /*レーダーに無線エフェクト*/
	    work->rctrl.flag |= (RADAR_RADIO);
	}
	
	if( (GM_AlertMode != ALERT_MODE_ALERT) && (GM_AlertMode != ALERT_MODE_AVOID) &&
	    (work->eye.alert2 == 0) ){
	    SET_G2_STEP(work, SUB_NORMAL);
	}

	if (DEV_EyeInfoCheck( &work->eye ,work->ctrl.hzx_id)){	//赤びっくり優先
	    SET_G2_STEP(work, SUB_WARN);
	}
	break;	

    case SUB_WARN:	//*****警戒モード*****
    {
	EYEPARAM *eye = &work->eye ;
	
	//赤びっくり通報
	if( ((eye->discv_time ==0 )&&(eye->alert_time == SIGHT_ALERT)) ||
	    (( GM_AlertMode == ALERT_MODE_ALERT )&&(eye->alert_time == SIGHT_ALERT)) ||
	    (eye->discv_time == 1 ) ){
	    COM_SetSecurityAlert(eye->trgpos,GM_PlayerMap);	//通報
	    SET_G2_STEP(work, SUB_ALERT);
	}
	if(eye->discv_time > 0 ) eye->discv_time --;
		
	/*びっくりセット*/
	DEV_HeadMarkCheck(&work->eye,
		      &work->body.objs->objs[CAMERA_PARTS].world,&work->headmark) ;

	/*目標物へ向く*/
	SIG_VecDir(work->eye.eyepos,work->eye.trgpos,
		   &work->ctrl.turn);
	break;
    }

    case SUB_ALERT:		//*****警戒モード*****
	
	DEV_EyeInfoCheck( &work->eye ,work->ctrl.hzx_id ) ;

	if(work->eye.alert_time>0 )
	{
	    //*目標物へ向く
	    SIG_VecDir(work->eye.eyepos,work->eye.trgpos,
		       &work->ctrl.turn);
	}
	else if( (GM_AlertMode==(ALERT_MODE_AVOID|ALERT_MODE_ALERT) )&&
		 (!(GM_CheckGameStatus(STATE_CHAFF))) )
	{
	    //回避中キョロキョロ
	    //printf("SV AVOID ACT!!!!\n");
	    SV_ActChaff(work) ;
	}
	else {
	    SET_G2_STEP(work, SUB_NORMAL);
	}
	break;	

    }

#ifdef DEBUG_MODE
#if 0
			MENU_Locate( 150, 200, 0 ) ;
			MENU_SetColor( 200, 200, 200 ) ;
			MENU_Printf( "MOD: %d  DT1: %d   DT2: %d\n", 
				     G2_STEP, work->eye.discv_time, work->eye.discv2 );

			MENU_Locate( 150, 220, 0 ) ;
			MENU_SetColor( 200, 200, 200 ) ;
			MENU_Printf( "         AL1: %d   AL2: %d\n", 
				     work->eye.alert_time, work->eye.alert2);


			MENU_Locate( 150, 240, 0 ) ;
			MENU_SetColor( 200, 200, 200 ) ;
			MENU_Printf( "UNIQ ID: %d", work->eye.accident_uniq_id);

			MENU_Locate( 150, 260, 0 ) ;
			MENU_SetColor( 200, 200, 200 ) ;
			MENU_Printf( "TO: %d   NOW: %d", work->ctrl.turn.vy, work->eye.rot.vy);
#endif
#endif
}


/**************************<-------local function------>*************************/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void Die(Work *work)
{
    GM_FreeObject(&(work->body));
    GM_FreeControl( &work->ctrl);
    GM_FreeRadarControl(&work->rctrl);
    GM_FreeTarget( &work->h_trg);
}


/**************************<-------local function------>*************************/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void InitRadarParam(Work *work){
    RADAR_CTRL *rctrl ;
    rctrl = &work->rctrl;
    rctrl->angle = (work->eye.range.vy)*2 ;/* 視野 */
    rctrl->col = RADAR_COLOR_YELOW ;	/* 視野描画色 */
    rctrl->range = work->eye.length;	/* 視力 */
    GM_RadarSetVRange( rctrl, 3000 , -3000 );
}


/**************************<-------local function------>*************************/
/*	名前:	void CameraCrash						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*pos							*/
/*		FVECTOR *force							*/
/*	説明:	カメラ壊れた	      						*/
/********************************************************************************/
static void CameraCrash(Work *work, FVECTOR *pos, FVECTOR *force ){
    /* 破壊エフェクト */
    
    GM_SeSetMode( SD_A_CAMGLS01, pos, GM_SEMODE_BOMB );	/* レンズ割れ */
    
    NewCrushWithForce( pos, force );

    SET_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行


    work->eye.alert_time = 0 ;
    work->se_cnt = SE_START;
    work->rctrl.flag &= (~RADAR_VISIBLE);
#error NOTE THAT THE LINE BELOW IS DEAD CODE

    DG_ChangeModelObj( &work->body.objs->objs[1], &work->broken->models[1] ) ;

    if(work->exec != 0){
	GM_ExecBlock( ( char * )work->exec, &work->args ) ;
    }
}

/**************************<-------local function------>*************************/
/*	名前:	void CameraVanish						*/
/*	引数:	Work	*work							*/
/*		FVECTOR	*pos							*/
/*	説明:	カメラ爆散	      						*/
/********************************************************************************/
static void CameraVanish(Work *work, FVECTOR *pos ){

    FVECTOR		fvtemp;
    SVECTOR		svtemp;
    short	i;

    for(i=0; i<2; i++){
	fvtemp.vx = 0.0f;
	fvtemp.vy = rnd()*MAX_VEC*0.5f + MAX_VEC*0.5f;
	fvtemp.vz = 0.0f;
	svtemp.vx = irnd()%2048;
	svtemp.vy = irnd()%4096;
	svtemp.vz = 0;
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	NewFlyingSmoke2( pos, &fvtemp, 20 );
    }

    GM_FreeObject(&(work->body));	//本体消去

    work->eye.alert_time = 0 ;
    work->se_cnt = 0;
    work->rctrl.flag &= (~RADAR_VISIBLE);

    if(work->exec != 0){
	GM_ExecBlock( ( char * )work->exec, &work->args ) ;
    }

    SET_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行

}


/**************************<-------local function------>*************************/
/*	名前:	void	ChildTargCallBack_Head					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/********************************************************************************/
static	void	ChildTargCallBack_Head( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) )//これらの攻撃タイプで破壊
	    {
		if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force);
		    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		}
		//*****爆発物ヒット カメラ爆散
		if( ( def->weapon_type & (WP_BLAST))&&(G1_STEP != MOD_BROKEN) ){
		    CameraVanish( work, &def->hit );
		}
		//*****銃弾ヒット カメラ壊れ
		else if( ( def->weapon_type & (WP_BULLET))&&(G1_STEP != MOD_BROKEN) ){
		    CameraCrash( work, &def->hit ,&off->power->force );
		}
	    }
	}
    }
    def->weapon_type = 0 ;
}


/**************************<-------local function------>*************************/
/*	名前:	void	ChildTargCallBack_Lens					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/********************************************************************************/
static	void	ChildTargCallBack_Lens( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    if( def->weapon_type & (WP_BULLET|WP_M92|WP_BLAST) )//これらの攻撃タイプで破壊
	    {
		if(off->power != NULL ) {
		    CallSpark(&def->hit ,&off->power->force);
		    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
		}
		//*****爆発物ヒット カメラ爆散
		if( ( def->weapon_type & (WP_BLAST))&&(G1_STEP != MOD_BROKEN) ){
		    CameraVanish( work, &def->hit );
		}
		//*****銃弾ヒット カメラ壊れ
		else if(G1_STEP != MOD_BROKEN){
		    CameraCrash( work, &def->hit ,&off->power->force );
		}
	    }
	}
    }
    def->weapon_type = 0 ;
}


/**************************<-------local function------>*************************/
/*	名前:	void SVC_TargetSet						*/
/*	引数:	Work	*work							*/
/*	説明:	やられ判定をターゲットに登録					*/
/********************************************************************************/
static void SVC_TargetSet(Work *work)
{
    int flag,map;
    FVECTOR	h_size = {HEAD_SIZE_XY,HEAD_SIZE_XY,HEAD_SIZE_Z,0.0F} ;
    FVECTOR	lens_size = {HEAD_SIZE_XY,HEAD_SIZE_XY,LENS_SIZE_Z,0.0F} ;
    FVECTOR	offset = {0.0F,HEAD_SHIFT_Y,HEAD_SHIFT_Z,0.0F} ;
    FVECTOR	lens_offset = {0.0F,HEAD_SHIFT_Y,175.0F,0.0F} ;
	/*あたりサイズ 厚み 高さ 横幅 の順*/

	/*ターゲット設定*/
    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON
	   |TARGET_POWER|TARGET_ROTATE);
    map = work->ctrl.map;
    GM_SetTarget( &work->h_trg, flag,map, ENEMY_SIDE, 
		  &h_size, &offset ) ;
    GM_SetTarget( &work->lens_trg, flag,map, ENEMY_SIDE, 
		  &lens_size, &lens_offset ) ;
    
    //ターゲット(当たり判定)をセット
    GM_SetTargetCallBack( &work->h_trg, ChildTargCallBack_Head, work ) ;
    GM_SetTargetCallBack( &work->lens_trg, ChildTargCallBack_Lens, work ) ;


    GM_SetTargetWeaponType(&work->h_trg,0);
    GM_SetTargetWeaponType(&work->lens_trg,0);

    GM_SetPowerTarget( &work->h_trg, &work->h_power,
		       POWER_CONST, 127, 0, 0, &DG_ZeroVector );
    GM_SetPowerTarget( &work->lens_trg, &work->h_power,
		       POWER_CONST, 127, 0, 0, &DG_ZeroVector );

    GM_PutTarget( &work->h_trg );
    GM_PutTarget( &work->lens_trg );

#ifdef DEBUG_MODE
    //    NewTargetView( &work->h_trg, 0, 0, 255 ) ;
    //    NewTargetView( &work->lens_trg, 255, 0, 0 ) ;
#endif
}



DG_OBJS		*D_MakeObjs( DG_DEF *def, int flag, int chanl){
	DG_OBJS	*objs ;
	objs = DG_MakeObjs( def, flag, chanl );
	objs->fname = "test" ;
	return ( objs );
}

/**************************<-------local function------>*************************/
/*	名前:	int FRT_OBJ_InitModel						*/
/*	引数:	HIDE	*h							*/
/*	    :	FMATRIX	*mtx							*/
/*	説明:	単純なオブジェクトの表示					*/
/********************************************************************************/
int FRT_OBJ_InitModel( FMATRIX *mtx )
{
    int id ;
    DG_DEF  *def ;
    DG_OBJS *objs ;

    /* モデルをタイプから捜す */
//    id = GV_StrCode( FRT_OBJ_GetTypeModel( h ) ) ;
    id = GV_StrCode( "hri_def" ) ;
    if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "No KMS in data.cnf!! : NewFortHideObject\n" ) ;
    /* モデルを作る */
    if ( !(objs = D_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot make DG_OBJS(Maybe no memory!!) Satoyoshi Test\n" ) ;
    DG_QueueObjs( objs ) ;
    if ( mtx )
	_sceVu0CopyMatrix( &objs->world, mtx ) ;
//    D_MakePreshade( objs, h->work->lit_def ) ;

//    /* モデルがちゃんとできたら解放する */
//   FRT_OBJ_FreeObject( h, 0 ) ;
//    h->objs = objs ;

    /* 当たり関係を貼り直す */
//    FRT_OBJ_InitTargetHzxHzd( h ) ;

    return 0 ;
}



/**************************<-------local function------>*************************/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/********************************************************************************/
static int GetResources(Work *work, int name, int where)
{
    int buf[3],model,code ;
    FVECTOR		*trans,tmppos ;
    FMATRIX		tmpmat;
    SVECTOR		base_rot,range;
    float		length ;
    short		start ; /*初期方向*/
    char		loop;




    work->name = name ;
    work->chf_cnt = 0 ;
    work->wait_cnt = 0;

    INIT_ALL_STEP(work);	//ゲームステップの全てを初期化

    /******破壊時実行ブロック******/
    work->exec = GCL_GetOptionValue( 'e', 0 ) ;
    work->args.argc = 1 ;
    work->args.argv = work->buf ;
    
    /******シナリオステータス******/
    work->scn_status = GCL_GetOptionValue( 's', 0 ) ;

    /******シナリオリード******/
    //GCL_GetOptionValue( 'd', 0 ) ;
    /******座標******/
    if ( GCL_GetOption( 'p' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &tmppos ) ;
    }else {
	tmppos = DG_ZeroVector;
    }
    /******方向******/
    if ( GCL_GetOption( 'r' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->rot.vx =  buf[0];
	work->rot.vy =  buf[1];
	work->rot.vz = 0;
    }else {
	work->rot = DG_ZeroSVector;
    }
    /******初期方向******/
    start = 0; /******初期方向******/
    if ( GCL_GetOption( 'c' ) == NULL ) {
	/******支柱と同じ向き******/
	work->center = work->rot.vy ;
    } else {
	work->center = GCL_GetNextInt() ;
	if( GCL_NextStr() != NULL ){
	    start = GCL_GetNextInt() ;
	}
    }
	/*可動幅*/
    work->max_dir = GCL_GetOptionValue( 'd', 512 ) ;
    if(work->max_dir > DIR_Y_MAX ) work->max_dir = DIR_Y_MAX ;
    work->now_dir = start ;

	/*探査目標*/
    work->eye.trgpos = &GM_PlayerFindPos ;
	/*顔の位置*/
    work->eye.eyepos = &work->eye_pos ;
	/*顔の向き*/
    work->ctrl.rot = work->ctrl.turn = 
	work->eye.rot  = work->rot ;
	/*視野角*/
    range.vx = GCL_GetOptionValue( 'x', 512 ) ; 
    range.vy = GCL_GetOptionValue( 'y', 512 ) ;
    range.vz = 0;
	/*視力*/
    length = GCL_GetOptionValue( 'i', SVC_EYE_S_DEF  );

    DEV_InitEyeParam(&work->eye,&range,length);

    work->mode = RIGHT_TURN ;

	/* モデル */
    if(work->scn_status & SVC_SCN_BROKEN1){
	model = SVC_BROKEN_MDL ;
    }else {
	model = SVC_NORMAL_MDL ;
    }
    work->broken= (DG_DEF*) GV_GetCache( GV_CacheID( SVC_BROKEN_MDL, 'k' ) );

    GM_InitObject(&(work->body),model,OBJECT_FLAG );
    GM_ConfigObjectLight(&(work->body),work->lights) ;

    GM_InitControl( &work->ctrl, name, where );
    GM_ConfigControlObject( &work->ctrl, &(work->body) ) ; 
    GM_ConfigControlMessageCheck( &work->ctrl ) ;
    GM_ConfigControlMapCheck( &work->ctrl ) ;
    work->ctrl.interp = 16 ;
	/*CONTROLフラグ*/
    work->ctrl.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	 |CTRL_SKIP_GET_ADDRESS
	 |CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	 |CTRL_SKIP_TRAP
	    ); 
    work->ctrl.mov = tmppos ;
	/*カメラ部の原点座標を保持*/
    base_rot = DG_ZeroSVector ;
    base_rot.vy = work->rot.vy ;

    DG_SetPos2( &tmppos, &base_rot ) ;
    trans = &work->body.objs->objs[CAMERA_PARTS].trans ;
    DG_MovePos(trans );
    DG_GetPos( &tmpmat ) ;
    GV_MatToVec(&tmpmat,&work->camera_pos);







#if 0
    GM_ConfigControlMapID( &work->ctrl ) ;
#endif
    work->map = work->ctrl.map ;
    GM_ActControl(&work->ctrl) ;
    GM_GroupObjs( work->body.objs, work->ctrl.map ) ;
    DG_SetPos2( &work->ctrl.mov, &work->ctrl.turn ) ;
    DG_PutObjs( work->body.objs );
    //レーダーのイニシャライズ
    GM_InitRadarControl(&work->rctrl,&work->eye_pos,
			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->map ) ;
    InitRadarParam(work);

    SVC_TargetSet(work);	//ターゲットに登録

#ifdef DEBUG_MODE 
    GV_SetActorChild( work,NewEyeView( &work->eyeview,
	(int)length,(int)(range.vy)/2,(int)(-range.vx)/2,(int)range.vx,COM_GetCommander()));
#endif

	/*ボンボリ*/	/*abgr*/
    SetRGBA(127 ,127 ,127 ,127, &work->rgba);
    work->b_mode = (0x00010000)|(0x80000001) ;
    code = GV_StrCode("svc_bonbori") ;
    GV_SetActorChild( work,
	NewGeneralSprite( code,&work->b_pos,
	B_RISE,&work->rgba,B_SIZE,B_SIZE, 0, &work->b_mode )) ;

    work->headmark = 0;
    GV_SetActorChild( work,
		NewControl_Headmark2( &work->body.objs->objs[CAMERA_PARTS].world ,
		&work->headmark,NULL, NULL )) ;
    work->alert = -1 ;

    SetDir(work);
    DEV_DirLimitX(&work->ctrl.turn ,DIR_X_MAX,128);
//    DEV_DirLimitY(&work->ctrl.turn, (work->center-(work->max_dir+1)), (work->center+work->max_dir));

    DEV_DirLimitY2(&work->ctrl.turn, work->center, work->max_dir, work->eye.rot.vy);


    GM_ActControl(&work->ctrl) ;
    work->ctrl.rot = work->ctrl.turn ;
    DG_GetLightMatrix( &work->ctrl.mov, work->lights );
    SetObjs(work);
    GM_MoveTarget2( &work->h_trg,&work->body.objs->objs[1].world);
    GM_MoveTarget2( &work->lens_trg,&work->body.objs->objs[1].world);

    if(work->scn_status & SVC_SCN_BROKEN1){
	SET_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行
	work->rctrl.flag &= (RADAR_VISIBLE);
    }
    work->rot_vy_buf  = work->ctrl.rot.vy ;
    work->se_cnt = 0;

    
    //****視認敵死体＆眠り兵記憶領域初期化
    for (loop=0; loop<UNIQ_LIST_MAX; loop++){
	work->uniq_id_list[loop] = -1;	//発見した死体や眠り兵を覚えておく
    }


//    FRT_OBJ_InitModel( &GM_PlayerFindPos );


    return 1;
}




/**************************<-------local function------>*************************/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	カメラの挙動メイン:全てステップ管理で進行			*/
/********************************************************************************/
static void Act(Work *work)
{
    OBJECT *body ;
    body = &work->body ;
    
    /*保持不要なフラグをリセット*/
    work->rctrl.flag &= (~RADAR_RADIO);
    
    switch (G1_STEP)	//********ステップ分岐********
    {

    case MOD_NORMAL:	//*****ノーマルモード*****
	AlertColSet(work) ;
	SV_ActNormal(work);	
	break;


    case MOD_CHAFF:	//*****チャフモード*****
	if(GM_CheckGameStatus(STATE_CHAFF)){		//チャフでふらふら
	    SetRGBA(irnd()&127 ,irnd()&127 ,irnd()&127 ,127, &work->rgba);	//とりあえずランダム
	    DEV_No_Sight( &work->eye ) ;		//視野に敵がいない
	    SV_ActChaff(work);				//チャフ時の動き
	    work->alert = -1 ;
	}
	else {
	    SET_G1_STEP(work, MOD_NORMAL);
	}
	break;


    case MOD_BROKEN:	//*****被破壊モード*****
	SetRGBA(0 ,0 ,0 ,127, &work->rgba);	//ライトを消す
	Call_spark(work);			//火花とスパーク音のコール
	return ;
	break;


    }

    SetFaceDir(work);

               /******限界設定******/
    DEV_DirLimitX(&work->ctrl.turn ,DIR_X_MAX,128) ;
    //    DEV_DirLimitY2(&work->ctrl.turn, work->center, work->max_dir);


    
	DEV_DirLimitY2(&work->ctrl.turn, work->center, work->max_dir, work->eye.rot.vy);

    GM_ActControl(&work->ctrl) ;	//移動関連

    DG_GetLightMatrix( &work->ctrl.mov, work->lights );	//周囲の光源を取得
    SetObjs(work);					//Objの位置や向きをセット

    //ターゲットの移動
    GM_MoveTarget2( &work->h_trg,&work->body.objs->objs[1].world);
    GM_MoveTarget2( &work->lens_trg,&work->body.objs->objs[1].world);
    CheckMessage( work ) ;

    /*駆動音は同一マップのみ*/
    if( work->ctrl.map == GM_PlayerMap ){
	if(work->rot_vy_buf != work->ctrl.rot.vy){
	    DEV_CameraSe(&work->eye );
	}
    }
    work->rot_vy_buf  = work->ctrl.rot.vy ;

#ifdef DEBUG_MODE
    DEV_DebugMode(&work->eye);
#endif
}


/*******************************<Global function>********************************/
/*	名前:	void *NewSvCamera						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewSvCamera( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where )){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
