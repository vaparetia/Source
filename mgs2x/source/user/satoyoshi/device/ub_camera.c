//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Ubsv_camera.c								*/
/*	監視カメラ  *NewUbSvCamera						*/
/*	2001/03/23 H.Satoyoshi							*/
/*	$Id: ub_camera.c,v 1.1.1.3 2002/11/19 11:48:17 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <float.h>

#include "gameheader.h"
#include "libutl.h"
#include "../../korekado/enemy/enemy.h"
//里吉追加
#include "../util/sato_util.h"
#include "eyecheck.h"
#include "ub_camera.h"

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern void *NewCypherLight( FMATRIX *mat, FVECTOR *offset, FVECTOR *color );

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

         /******シナリオ指定ステータス******/
#define SVC_SCN_BROKEN1 (0x01)

         /*****work.status*****/
#define	SV_BLIND	(0x01)	/*視界閉じ レーダーそのまま*/

#define	SV_WAIT_TIME	(SVC_COUNT*10) /*首振り終端で停止時間*/
#define	SE_START	(SVC_COUNT*30)	/*破壊後エフェクト用カウンタ*/

#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

#define B_SIZE			(80)		//動作ランプのサイズ
#define B_RISE			(40.0F)		//動作ランプの表示優先
#define UBALERT_TIME		(240)


enum {
MSG_SIGHT_OFF = 1,
MSG_SIGHT_ON
};


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work   						       	*/
/*	説明:	メッセージを受信する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->n_msg ;
    msg = work->msg ;
    
    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ];


printf("****-UB CAMERA-**** Get Mess No.%d\n", code);

	if (code < 10){
	    work->mode = code;
	    work->timer = 0;
	}
	else {
	    if (code == 10){
		work->timer = 0;
		GM_SeSetMode (SD_S_START01, &work->ctrl.mov, GM_SEMODE_BOMB);	//決定音（タイトル、ブリーフィング
	    }
	    if (code == 11){
		work->timer = 0;
		//		GM_SeSetMode (SD_E_BIKKRI01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カードキーＮＧブザー
		COM_DetectVibration( );
		work->alert_time = UBALERT_TIME*5;
	    }
	    if (code == 12){
		work->timer = 0;
		GM_SeSetMode (SD_S_SIREN06 , &work->ctrl.mov, GM_SEMODE_BOMB);	//警報
	    }
	    if (code == 13){
		work->timer = 0;
		GM_SeSetMode (SD_S_TYPING01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//パスコード通常タイプ音
	    }
	    if (code == 14){
		work->timer = 0;
		GM_SeSetMode (SD_S_TYPING02 , &work->ctrl.mov, GM_SEMODE_BOMB);	//パスコードラストタイプ音
	    }
	    if (code == 15){
		work->timer = 0;
		GM_SeSetMode (SD_S_V_CANS02 , &work->ctrl.mov, GM_SEMODE_BOMB);	//SPモードキャンセル音
	    }
	    if (code == 16){
		work->timer = 0;
		GM_SeSetMode (SD_S_CAM_OK01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カメラ撮影ＯＫ
	    }
	    if (code == 17){
		work->timer = 0;
		GM_SeSetMode (SD_S_CAM_NG01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カメラ撮影ＮＧ
	    }
	    printf ("receve mesage %d\n", code);
 	}
	msg++;
    }
}


/********************************************************************************/
/*	名前:	void SetObjs							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/********************************************************************************/
static void SetObjs(Work *work){
    FVECTOR	eye_shift;
    FVECTOR	shift = {0.0f, 240.0f, 320.0f, 1.0f};

//	レンズ部分
    eye_shift.vx = 0.0F;
    eye_shift.vy = 50.0F;
    eye_shift.vz = 150.0F;
    
    
    GM_GroupObjs( work->body.objs, work->map );	//マップグループに登録

    /*****土台部分*****/
    DG_SetPos2( &work->ctrl.mov, &work->rot ) ;
    DG_PutObjs( work->body.objs );	//現在位置にオブジェを配置
    DG_GetPos( &work->body.objs->world  ) ;
    DG_GetPos( &work->body.objs->objs[0].world  ) ;	//オブジェクトの位置を設定

    /*****カメラ部分*****/

    DG_MovePos(&shift);
    DG_RotatePos(&work->hrot);
    DG_GetPos( &work->body.objs->objs[1].world  ) ;

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
    GM_FreeTarget( &work->h_trg);
}

extern void	NewSpark( FMATRIX * );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CallSpark							*/
/*	引数:	FVECTOR *pos							*/
/*	引数:	FVECTOR *force							*/
/*	説明:	火花を呼ぶ	       						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CallSpark(FVECTOR *pos ,FVECTOR *force) {
    SVECTOR	rot ;
    FMATRIX	w ;

    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}

/**************************<-------local function------>*************************/
/*	名前:	void	ChildTargCallBack_Lens					*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void*	*ptr							*/
/*	説明:			      						*/
/********************************************************************************/
static	void	ChildTargCallBack_UB( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
	    CallSpark(&def->hit ,&off->power->force);
	    GM_SeSetMode( SD_W_RICOCH01, &work->camera_pos, GM_SEMODE_BOMB ) ;
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
    FVECTOR	h_size = {90.0f, 50.0f, 200.0f, 0.0F} ;
    FVECTOR	offset = {0.0F, 20.0f, 180.0f, 0.0F} ;

    FVECTOR	lens_size = {90.0f, 120.0f, 270.0f, 0.0F} ;
    FVECTOR	lens_offset = {0.0F, 90.0f, 85.0F, 0.0F};


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
    GM_SetTargetCallBack( &work->h_trg, ChildTargCallBack_UB, work ) ;
    GM_SetTargetCallBack( &work->lens_trg, ChildTargCallBack_UB, work ) ;


    GM_SetTargetWeaponType(&work->h_trg,0);
    GM_SetTargetWeaponType(&work->lens_trg,0);

    GM_PutTarget( &work->h_trg );
    GM_PutTarget( &work->lens_trg );

}


/**************************<-------local function------>*************************/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/********************************************************************************/
static int GetResources(Work *work, int name, int where, int is_plant)
{
    int buf[3];
    FVECTOR		tmppos ;
    work->name = name ;

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

    /******方向******/
    if ( GCL_GetOption( 'h' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->hrot.vx =  buf[0];
	work->hrot.vy =  buf[1];
	work->hrot.vz = 0;
    }else {
	work->hrot = DG_ZeroSVector;
    }

    GM_InitObject(&(work->body), GV_StrCode("svc_unbreak"), OBJECT_FLAG );
    GM_ConfigObjectLight(&(work->body),work->lights) ;
    GM_InitControl (&work->ctrl, name, where);	

    work->ctrl.mov = tmppos ;
	/*カメラ部の原点座標を保持*/

    work->map = work->ctrl.map ;

    SVC_TargetSet(work);	//ターゲットに登録

	/*ボンボリ*/	/*abgr*/
    SetRGBA(255 ,80 ,48 ,200, &work->rgba);

    {
    FVECTOR	shift = {48.0f, -5.0f, 360.0f, 1.0f};
    GV_SetActorChild( work,
		      NewCypherLight( &work->body.objs->objs[1].world,
				      &shift,
				      &work->rgba));
    }


    GM_ActControl(&work->ctrl) ;
    DG_GetLightMatrix( &work->ctrl.mov, work->lights );

    return 1;
}


/**************************<-------local function------>*************************/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	カメラの挙動メイン:全てステップ管理で進行			*/
/********************************************************************************/
static void Act(Work *work)
{
    OBJECT	*body ;
    int		timer;
    //    static	int	code=10;
    
    body = &work->body ;
    
    if (work->alert_time > 0){
	COM_SetSecurityAlert(&GM_PlayerFindPos,GM_GetHzxGroupID(GM_PlayerMap));
	COM_SetFlameFlag( CMFLAG_DETECT );
	SetRGBA(128 ,0 ,0 ,127 , &work->rgba);

	if (work->alert_time >= (UBALERT_TIME-8)*5){
	    work->alert_time -= TIME_BASE;
	    if (work->alert_time <= (UBALERT_TIME-8)*5){
		GM_SeSetMode( SD_E_BIKKRI01 , &work->ctrl.mov,
			      GM_SEMODE_BOMB );
	    }
	}
	else {
	    work->alert_time -= TIME_BASE;
	}
	//	printf ("T:%d\n", work->alert_time);
    }
    
    
#if 0
    if (GV_PadDataDirect[0].press & PAD_X){    
	if (code == 10){
	    GM_SeSetMode (SD_S_START01, &work->ctrl.mov, GM_SEMODE_BOMB);	//決定音（タイトル、ブリーフィング
	}
	if (code == 11){
	    GM_SeSetMode (SD_S_BUZZER01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カードキーＮＧブザー
	}
	if (code == 12){
	    GM_SeSetMode (SD_S_SIREN06 , &work->ctrl.mov, GM_SEMODE_BOMB);	//警報
	}
	if (code == 13){
	    GM_SeSetMode (SD_S_TYPING01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//パスコード通常タイプ音
	}
	if (code == 14){
	    GM_SeSetMode (SD_S_TYPING02 , &work->ctrl.mov, GM_SEMODE_BOMB);	//パスコードラストタイプ音
	}
	if (code == 15){
	    GM_SeSetMode (SD_S_V_CANS02 , &work->ctrl.mov, GM_SEMODE_BOMB);	//SPモードキャンセル音
	}
	if (code == 16){
	    GM_SeSetMode (SD_S_CAM_OK01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カメラ撮影ＯＫ
	}
	if (code == 17){
	    GM_SeSetMode (SD_S_CAM_NG01 , &work->ctrl.mov, GM_SEMODE_BOMB);	//カメラ撮影ＮＧ
	}
	printf ("SE  %d\n", code);
	code ++;
	if (code > 16){
	    code = 10;
	}
    }
#endif
    
    
    DG_GetLightMatrix( &work->ctrl.mov, work->lights );	//周囲の光源を取得
    SetObjs(work);					//Objの位置や向きをセット
    
    work->n_msg = GV_ReceiveMessage( work->name, &( work->msg ) );
    CheckMessage( work );
    
    GM_MoveTarget2( &work->lens_trg,&work->body.objs->objs[1].world);
    GM_MoveTarget2( &work->h_trg,&work->body.objs->objs[0].world);
    
    switch (work->mode){

    case 1:
	if (work->timer == 0){
	    work->timer += TIME_BASE;
	    GM_SeSetMode (SD_A_ELTGOOD1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}

	SetRGBA(0 ,128 ,0 ,127 , &work->rgba);
	break;

    case 2:
	if (work->timer == 0){
	    work->timer += TIME_BASE;
	    GM_SeSetMode (SD_A_ELTGOOD1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}

	SetRGBA(0 ,0 ,128 ,127 , &work->rgba);
	break;

    case 3:
	work->timer += TIME_BASE;
	if (work->timer > 60){
	    GM_SeSetMode (SD_A_ELTNOT01 , &work->ctrl.mov, GM_SEMODE_BOMB);
	    work->timer -= 60;
	}
	SetRGBA(255 ,80 ,48 ,200, &work->rgba);
	break;

    case 4:
	SetRGBA(0 ,0 ,0 ,0 , &work->rgba);
	break;

    case 5:
	SetRGBA(255 ,80 ,48 ,200, &work->rgba);
	return;

	timer = work->timer;
	timer = timer%80;
	if (work->timer == 0){
	    GM_SeSetMode (SD_A_ELTSTAR1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}
	if (work->timer < 120){
	    if ((0 < timer)&&( timer < 30)){
		SetRGBA(timer*2+60 ,timer*2+60 ,0 ,127 , &work->rgba);
	    }
	    else if (timer < 60){
		SetRGBA(0 ,0 ,0 ,127 , &work->rgba);
	    }
	}
	if (work->timer < 360){
	    work->timer += TIME_BASE;
	}
	break;

    case 6:

	timer = work->timer*2/3;
	if ((0 < timer)&&( timer < 60)){
	    SetRGBA(timer*3+60 ,timer*3+60 ,0 ,127 , &work->rgba);	if (work->timer == 0){
	    GM_SeSetMode (SD_A_ELTSTAR1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}
	}
	else if (timer < 120){
	    SetRGBA( 120-timer ,120-timer ,0 ,127-timer, &work->rgba);
	}

	if (work->timer > 180){
	    work->timer -= 180;
	    GM_SeSetMode (SD_A_ELTSCAN1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}
	else if (work->timer == 0){
	    GM_SeSetMode (SD_A_ELTSCAN1 , &work->ctrl.mov, GM_SEMODE_BOMB);
	}
	work->timer += TIME_BASE;
	break;


    case 7:	//青点灯
	SetRGBA(255 ,80 ,48 ,200, &work->rgba);
	break;

    case 8:	//緑点灯
	SetRGBA(0 ,128 ,0 ,127 , &work->rgba);
	break;

    case 9:	//赤点灯
	SetRGBA(0 ,0 ,128 ,127 , &work->rgba);
	break;

    }

}


/*******************************<Global function>********************************/
/*	名前:	void *NewSvCamera						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewUbSvCamera( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,0)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}













