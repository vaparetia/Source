//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	sv_camera.c								*/
/*	監視カメラ系統メイン関数群						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: sv_camera.c,v 1.1.1.3 2002/11/19 11:48:17 Yoshizawa1 Exp $		*/
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
#include "../../korekado/enemy/enemy.h"

#include "../util/sato_util.h"

#include "eyecheck.h"
#include "camera.h"
#include "layer_check.h"

#include "cam_target.c"
#include "cam_act.c"
#include "cam_alert.c"
#include "cam_call.c"
#include "cam_wbk.c"


#include "eye_check.c"
#include "cam_check.c"

/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern void *NewCypherLight( FMATRIX *mat, FVECTOR *offset, FVECTOR *color );
extern CONTROL *EMA_CommandGetControl() ;

#ifdef DEBUG_MODE 
extern void *NewEyeView(FMATRIX *,int,int,int,int,COMMANDER *, int *);
extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;
#endif

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetFaceDir							*/
/*	引数:	Work *work							*/
/*	説明:	カメラの向きの限界を設定する					*/
/*		Alert Modeの時は限界角度が大きい				*/
/*		メインループ do_Act から毎フレーム呼び出される			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetFaceDir(Work *work){
    work->eye.rot = work->ctrl.rot;

               /******限界設定******/
    DEV_DirLimitX(&work->ctrl.turn ,DIR_X_MAX,128) ;

    if ( (G1_STEP == MOD_NORMAL) || (G1_STEP == MOD_CHAFF) ){
	DEV_DirLimitY2(&work->ctrl.turn, work->center, work->max_dir, work->eye.rot.vy);
    }
    else {
	DEV_DirLimitY2(&work->ctrl.turn, work->center, work->alert_max_dir, work->eye.rot.vy);
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CheckMessage						*/
/*	引数:	Work *work							*/
/*	説明:	メッセージをチェックして取得する				*/
/*		MSG_SIGHT_ON  :視界オン						*/
/*		MSG_SIGHT_OFF :視界を切る					*/
/*		メインループ do_Act から毎フレーム呼び出される			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
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

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void AlertColSet						*/
/*	引数:	Work *work							*/
/*	説明:	各モード毎に分岐して警戒色をセットする				*/
/*		メインループ do_Act から毎フレーム呼び出される			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void AlertColSet(Work *work){

    switch (G1_STEP)	// ********ステップ分岐********
    {

    case MOD_NORMAL:	// *****ノーマルモード*****
	SET_COLOR_CAM( 64, 180, 112, 200, &work->rgba);
	break;

    case MOD_ALERT:	// *****アラーとモード
	SET_COLOR_CAM(255 ,80 ,48 ,200, &work->rgba);
	break;

    case MOD_EMMA_ALERT:// *****アラーとモード
	SET_COLOR_CAM(255 ,80 ,48 ,200, &work->rgba);
	break;

    case MOD_EVASION:	// *****回避モード
	SET_COLOR_CAM(180, 148, 64, 200, &work->rgba);
	break;

    case MOD_CHAFF:	// *****チャフモード*****
	break;

    case MOD_CALL:	// *****呼び*****
	SET_COLOR_CAM(180, 148, 64, 200, &work->rgba);
	break;

    case MOD_NIKITA_SHOOT:	// *****ニキータ撃ち*****
	SET_COLOR_CAM(255 ,80 ,48 ,200, &work->rgba);
	break;

    case MOD_WBK:	// *****白びくり*****
	SET_COLOR_CAM(180, 148, 64, 200, &work->rgba);
	break;

    case MOD_BROKEN:	// *****被破壊モード*****
	break;
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Cam_SetObjs						*/
/*	引数:	Work	*work							*/
/*	説明:	カメラのモデルの位置を更新してセットする			*/
/*		メインループ do_Act から毎フレーム呼び出される			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Cam_SetObjs(Work *work){
    SVECTOR base_rot;
    FVECTOR eye_shift ;

    //周囲の光源を取得
    DG_GetLightMatrix( (FVECTOR*)work->body.objs->objs[1].world.m[3], work->lights );

    //レンズ部分
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

    if (work->fire_flag == 1){	//銃を撃った反動
	FVECTOR tempvec = {0.0f, 0.0f, 13.0f, 1.0f};
	work->fire_flag = 0;

	DG_MovePos(&tempvec);
    }
    DG_GetPos( &work->body.objs->objs[1].world  ) ;

    /****視点の原点****/
    DG_PutVector( &eye_shift, &work->eye_pos, 1 );	//視点を設置

    work->radar_pos = work->eye_pos;
    work->radar_pos.vy = work->eye_pos.vy-1880.0f;

#ifdef DEBUG_MODE 
    DG_SetPos2( &work->camera_pos, &work->ctrl.rot ) ;
    DG_MovePos( &eye_shift ) ;
    DG_GetPos(&work->eyeview) ;
#endif

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void set_kubihuri_dir						*/
/*	引数:	Work *work							*/
/*	説明:	通常時の左右の首ふり挙動を制御する				*/
/*		RIGHT_TURN:右回転モードと LEFT_TURN:左回転モードで制御		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_kubihuri_dir(Work *work){
    int dirsub ;

    dirsub = CheckDirSub(work->now_dir , work->max_dir) ;
    switch(work->mode){

    case RIGHT_TURN :	// *****右回転
	dirsub = work->max_dir - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt -= TIME_BASE ;
	}else if ( (dirsub > TURN_SPEED) ){
	    work->now_dir += TURN_SPEED ;
        } else {
	    work->wait_cnt = SV_WAIT_TIME ;
	    work->mode = LEFT_TURN ;
	}
	break;

    case LEFT_TURN :	// *****左回転
        dirsub = (-work->max_dir) - work->now_dir ;
	if(work->wait_cnt > 0){
	    work->wait_cnt -= TIME_BASE ;
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



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SV_ActNormal						*/
/*	引数:	Work	*work							*/
/*	説明:	ノーマルモードの挙動関数					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SV_ActNormal(Work *work){

    set_kubihuri_dir(work);
    work->eye.length = work->eye_len_norm;

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void InitRadarParam(Work *work){
    RADAR_CTRL *rctrl ;
    rctrl = &work->rctrl;
    rctrl->angle = (work->eye.



range.vy)*2;	// 視野
    rctrl->col = RADAR_COLOR_BLUE ;		// 視野描画色
    rctrl->range = work->eye.length;		// 視力
    GM_RadarSetVRange( rctrl, 1800 , -1800 );
}

/**************************<-------local function------>*************************/
/*	名前:	void DEV_CameraSe						*/
/*	返値:	なし								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	首振り効果音を鳴らす						*/
/*		メインループ do_Act から毎フレーム呼び出される			*/
/********************************************************************************/
void SV_CameraSe(Work *work){
    if (G1_STEP != MOD_CHAFF){
	/*首振り効果音*/
	if((DG_TickCount - work->se_tick)*TIME_BASE > CAM_SE_INT){	//音を鳴らした瞬間を保存
	    GM_SeSetFromVolCurve( work->move_se_num ,&work->ctrl.mov, &svc_curves);
	    work->se_tick = DG_TickCount ;
	}
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Act							*/
/*	引数:	Work	*work							*/
/*	説明:	カメラの挙動メイン:全てステップ管理で進行			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void do_Act(Work *work)
{
    OBJECT	*body ;
    int		loop;

    body = &work->body ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( GM_VRStatus & (GM_VR_IDLE|GM_VR_CLEAR) ) return ;
	}

    if(work->sight_stat &  SV_BLIND ){
	return;		/*視界off*/
    }

    Meca_Eye_Check( &work->eye );	// 視界関連チェック

    AlertColSet(work);	// カメラ動作ランプ色セット

    // ===============レイヤーチェックとモード切替え
    for (loop=0; loop<CHECK_FUNC_NUM; loop++){
	if (Check_Funk[loop](work)){
	    break;
	}
    }

    if (CHECK_PC_FLG(EYECK_PC_IN_SIGHT)){
	if ( GM_AlertMode == ALERT_MODE_ALERT ){
	    COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT);
	}
	COM_SetPlayerLastPos(&GM_PlayerFindPos, GM_GetHzxGroupID(GM_PlayerMap));
    }
    if (CHECK_PC_FLG(EYECK_PC_EMMA_IN_SIGHT)){
	if ( GM_AlertMode == ALERT_MODE_ALERT ){
	    COM_SetAlertStatus(COM_ALERT_NPC_DETECT);
	}
	if ( !(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){
	    COM_SetPlayerLastPos(EMA_CommandGetPosition(),GM_GetHzxGroupID(EMA_CommandGetControl()->map));
	}
    }

    /*保持不要なフラグをリセット*/
    work->rctrl.flag &= (~RADAR_RADIO);
    switch (G1_STEP)	// ********ステップ分岐********
    {

    case MOD_NORMAL:	// *****ノーマルモード
	SV_ActNormal(work);	
	break;

    case MOD_ALERT:	// *****アラーとモード
	SV_ActAlert(work);	
	break;

    case MOD_EMMA_ALERT:// *****アラーとモード
	SV_EmmaActAlert(work);	
	break;

    case MOD_EVASION:	// *****回避モード
	SV_ActEvasion(work);
	break;

    case MOD_CHAFF:	// *****チャフモード*****
	SV_ActChaff(work);				//チャフ時の動き
	break;

    case MOD_CALL:	// *****呼び*****
	SV_ActCall(work);
	break;

    case MOD_NIKITA_SHOOT:	// *****ニキータ撃ち*****
	SV_ActNikitaShoot(work);
	break;

    case MOD_WBK:	// *****白びくり*****
	cam_wbk_control(work);
	break;

    case MOD_BROKEN:	// *****被破壊モード*****
	SV_ActBreak(work);			//火花とスパーク音のコール
	break;
    }

    SetFaceDir(work);
    GM_ActControl(&work->ctrl) ;	//移動関連
    Cam_SetObjs(work);			//Objの位置や向きをセット

    //ターゲットの移動
    {
	FMATRIX	tmpmat;
	SVECTOR	tmprot={-280, 0, 0, 0};

	DG_SetPos(&work->body.objs->objs[1].world);
	DG_RotatePos(&tmprot);
	DG_GetPos(&tmpmat);

	GM_MoveTarget2( &work->bar_trg,&tmpmat);
	
	GM_MoveTarget2( &work->pilor_trg,&work->body.objs->objs[0].world);
	GM_MoveTarget2( &work->h_trg,&work->body.objs->objs[1].world);
	GM_MoveTarget2( &work->mag_trg,&work->body.objs->objs[1].world);
	GM_MoveTarget2( &work->lens_trg,&work->body.objs->objs[1].world);
    }
    CheckMessage( work ) ;

    /*駆動音は同一マップのみ*/
    if( work->ctrl.map == GM_PlayerMap ){
	if(work->rot_vy_buf != work->ctrl.rot.vy){
	    SV_CameraSe(work);
	}
    }
    work->rot_vy_buf  = work->ctrl.rot.vy ;

#ifdef DEBUG_MODE
    DEV_DebugMode(&work->eye);
#endif
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	init_all_step							*/
/*	引数:	Work *work							*/
/*	説明:	全てのステップを初期化する					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void init_all_step(Work *p){
    p->G1_step = 0;
    p->G2_step = 0;
    p->G3_step = 0;
    p->G4_step = 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:	Die関数		      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work)
{
    GM_FreeObject(&(work->body));
    GM_FreeControl( &work->ctrl);
    GM_FreeRadarControl(&work->rctrl);
    GM_FreeTarget( &work->h_trg);
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name		キャラ名				*/
/*	    :	int	where		マップID				*/
/*	    :	int	camera_type	カメラのタイプ				*/
/*				NORMAL	:通常カメラ				*/
/*				PLANT	:プラントカメラ				*/
/*				GUN	:ガンカメラ				*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Work *work, int name, int where, int camera_type)
{
    int buf[3],model;
    FVECTOR		*trans,tmppos ;
    FMATRIX		tmpmat;
    SVECTOR		base_rot;
    short		start ; /*初期方向*/
    short		loop;

    work->chaff_time = 1;

    work->ACTION_LAYER = EYE_LAYER_Normal;

    //  ********ＳＥの設定など
    work->com_uniq_id = COM_GetUniqID();
    switch ((work->com_uniq_id)%4) {
    case 0 :
	work->move_se_num = SD_E_CAMMOV01;
	work->chaf_se_num = SD_E_CAMCHA01;
	break;
    case 1 :
	work->move_se_num = SD_E_CAMMOV02;
	work->chaf_se_num = SD_E_CAMCHA02;
	break;
    case 2 :
	work->move_se_num = SD_E_CAMMOV03;
	work->chaf_se_num = SD_E_CAMCHA03;
	break;
    case 3 :
	work->move_se_num = SD_E_CAMMOV04;
	work->chaf_se_num = SD_E_CAMCHA04;
	break;
    default :
	work->move_se_num = 0 ;
	work->chaf_se_num = 0 ;
    }



    // =============================================種類
    work->camera_type = camera_type;	

#ifdef DEBUG_MODE
    if (work->camera_type == GUN){
	printf("==  Gun  Camera Set  ==Name: %d==ID: %d============\n",name, work->com_uniq_id);
    }
    else if (work->camera_type == PLANT){
	printf("== Plant Camera Set  ==Name: %d==ID: %d============\n",name, work->com_uniq_id);
    }
    else {
	printf("== Tanker Camera Set ==Name: %d==ID: %d============\n",name, work->com_uniq_id);
    }
#endif    

    // ============================================座標
    if ( GCL_GetOption( 'p' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &tmppos ) ;
    }else {
	tmppos = DG_ZeroVector;
    }
#ifdef DEBUG_MODE
    printf("座標      vx: %8.0f\n",tmppos.vx );
    printf("          vy: %8.0f\n",tmppos.vy );
    printf("          vz: %8.0f\n",tmppos.vz );
#endif

    // ============================================方向
    if ( GCL_GetOption( 'r' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->rot.vx =  buf[0];
	work->rot.vy =  buf[1];
	work->rot.vz = 0;
    }else {
	work->rot = DG_ZeroSVector;
    }
#ifdef DEBUG_MODE
    printf("支柱方向  rx: %d\n",work->rot.vx );
    printf("          ry: %d\n",work->rot.vy );
#endif

    // ============================================可動幅
    work->max_dir = GCL_GetOptionValue( 'd', 512 ) ;
    if(work->max_dir > DIR_Y_MAX ) work->max_dir = DIR_Y_MAX ;
    // ============================================緊急可動幅
    work->alert_max_dir = GCL_GetOptionValue( 'a', work->max_dir ) ;
    if(work->alert_max_dir > DIR_Y_MAX ) work->alert_max_dir = DIR_Y_MAX ;
#ifdef DEBUG_MODE
    printf("振り幅  通常: %d\n",work->max_dir );
    printf("        緊急: %d\n",work->alert_max_dir );
#endif


    // ============================================初期方向
    start = 0;
    if ( GCL_GetOption( 'c' ) == NULL ) {
	work->center = work->rot.vy ;
    }
    else {
	work->center = GCL_GetNextInt() ;
	if( GCL_NextStr() != NULL ){
	    start = GCL_GetNextInt() ;
	}
    }
#ifdef DEBUG_MODE
    printf("振り角中心  : %d\n",work->center );
    printf("初期方向    : %d\n",work->ctrl.rot.vy);
#endif


    // ============================================視野角
    work->range.vx = GCL_GetOptionValue( 'x', 512 ) ;
    work->range.vy = GCL_GetOptionValue( 'y', 512 ) ;
    work->range.vz = 0;

    if ( GCL_GetOption( 'i' ) != NULL ) {
	loop = 0;
	while( loop <= 3){
	    if (GCL_NextStr() != NULL ){
		(&(work->eye_len_norm))[loop] = GCL_GetNextInt();
	    }
	    else {
		(&(work->eye_len_norm))[loop] = work->eye_len_norm;
	    }
	    loop++;
	}
    }
    else {
	work->eye_len_norm = work->eye_len_alert = work->eye_len_warn = 
	    work->eye_len_avoid = 4000.0f;
    }
    DEV_InitEyeParam(&work->eye,&work->range,work->eye_len_norm);
    work->mode = RIGHT_TURN ;
#ifdef DEBUG_MODE
    printf("視野角  縦  : %d\n",work->range.vx );
    printf("        横  : %d\n",work->range.vy );
    printf("視力    潜入: %7.0f\n",work->eye_len_norm );
    printf("        nn危険: %7.0f\n",work->eye_len_alert );
    printf("        警戒: %7.0f\n",work->eye_len_warn );
    printf("        回避: %7.0f\n",work->eye_len_avoid );
#endif

    // ========================================シナリオステータス
    work->scn_status = GCL_GetOptionValue( 's', 0 ) ;
#ifdef DEBUG_MODE
    printf("ステータス  : %d\n",work->scn_status);
#endif

    // ========================================首振り待ち時間
    work->wait_time = GCL_GetOptionValue( 'w', 60 ) ;
#ifdef DEBUG_MODE
    printf("首振り待ち時間: %d\n",work->wait_time);
#endif

    /******破壊時実行ブロック******/
    work->exec = GCL_GetOptionValue( 'e', 0 ) ;

    /******ニキータ射撃時間******/
    if ( GCL_GetOption( 'n' ) ){
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->nikita_deadend =  buf[0];
    }else {
	work->nikita_deadend = 600;
    }

    work->name = name ;
    work->chf_cnt = 0 ;
    work->wait_cnt = 0;

    init_all_step(work);	//ゲームステップの全てを初期化

    printf("=============================================================\n");

    work->eye.trgpos = &GM_PlayerFindPos ;	/*探査目標*/
    work->eye.eyepos = &work->eye_pos ;		/*顔の位置*/

	/* モデル */

    if(work->scn_status == 0){
	if (camera_type == PLANT){
	    model = SVC_PLANT_MDL ;
	}
	else if (camera_type == NORMAL){
	    model = SVC_NORMAL_MDL ;
	}
	else {
	    model = GV_StrCode("gcm") ;
	}
    }else {
	if (camera_type == PLANT){
	    model = SVC_PL_BROKEN_MDL ;
	}
	else if (camera_type == NORMAL){
	    model = SVC_BROKEN_MDL ;
	}
	else {
	    model = GV_StrCode("gcm") ;
	    EMA_GunCameraBroken();
	}
    }

    if (camera_type == PLANT){
	work->broken= (DG_DEF*) GV_GetCache( GV_CacheID( SVC_PL_BROKEN_MDL, 'k' ) );
    }
    else if (camera_type == NORMAL){
	work->broken= (DG_DEF*) GV_GetCache( GV_CacheID( SVC_BROKEN_MDL, 'k' ) );
    }
    else if (camera_type == GUN){
	work->broken= (DG_DEF*) GV_GetCache( GV_CacheID( GV_StrCode("gcm"), 'k' ) );
    }

    if (work->broken == NULL){
	printf ("satoyoshi ::sv_camera.c can't get broken model's data %p\n", work->broken);
	ASSERT(0);
    }
    else {
	printf ("Good: get broken model's data %p\n", work->broken);
    }

    GM_InitObject(&(work->body),model,OBJECT_FLAG );
    GM_ConfigObjectLight(&(work->body),work->lights) ;

    GM_InitControl( &work->ctrl, name, where );

	/*CONTROLフラグ*/
    GM_ConfigControlObject( &work->ctrl, &(work->body) ) ; 
    GM_ConfigControlMessageCheck( &work->ctrl ) ;
    GM_ConfigControlMapCheck( &work->ctrl ) ;
    work->ctrl.interp = 16 ;
    work->ctrl.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	 |CTRL_SKIP_GET_ADDRESS
	 |CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	 |CTRL_SKIP_TRAP
	    ); 
    work->ctrl.mov = tmppos ;
    base_rot = DG_ZeroSVector ;	//	カメラ部の原点座標を保持
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

    work->ctrl.rot = work->ctrl.turn =
	work->eye.rot  = work->rot ;
    work->now_dir = start ;
    work->ctrl.rot.vy = work->center + start;
    work->ctrl.turn.vy = work->center + start;

    GM_ActControl(&work->ctrl) ;
    GM_GroupObjs( work->body.objs, work->ctrl.map ) ;
    DG_SetPos2( &work->ctrl.mov, &work->ctrl.turn ) ;
    DG_PutObjs( work->body.objs );

    GM_InitRadarControl(&work->rctrl,&work->radar_pos,	    //レーダーのイニシャライズ
			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->map ) ;
    InitRadarParam(work);
    if(work->scn_status != 0){
	work->rctrl.flag &= (~RADAR_VISIBLE);
    }

    if ( GCL_GetOption( 'v' ) != NULL ) {
	int upper, lower;
	if (GCL_NextStr() != NULL ){
	    upper = GCL_GetNextInt();
	    if (GCL_NextStr() != NULL ){
		lower = GCL_GetNextInt();
		GM_RadarSetVRange( &work->rctrl,
				   (float)upper-tmppos.vy, (float)lower-tmppos.vy);
	    }
	}
    }

    SVC_TargetSet(work);	//ターゲットに登録

#ifdef DEBUG_MODE 
    GV_SetActorChild( work, NewEyeView( &work->eyeview,		//視界表示
	(int)work->eye_len_norm,(int)(work->range.vy/2),(int)(-work->range.vx/2),(int)work->range.vx/2,
	COM_GetCommander(), NULL));
#endif

	/*ボンボリ*/	/*abgr*/
    SET_COLOR_CAM(127 ,127 ,127 ,127, &work->rgba);
    work->b_mode = (0x00010000)|(0x80000001) ;


	/*****発光位置をシフト*****/
    if (work->camera_type == GUN){
	//	FVECTOR shift = {-30.0f, 160.0f, 560.0f, 1.0f};
	FVECTOR shift = {-30.0f, 160.0f, 545.0f, 1.0f};
	GV_SetActorChild( work,
			  NewCypherLight( &work->body.objs->objs[CAMERA_PARTS].world,
					  &shift,
					  &work->rgba)
	    );
    }
    else if (work->camera_type == PLANT){
	FVECTOR shift = {50.0f, 100.0f, 190.0f, 1.0f};
	GV_SetActorChild( work,
			  NewCypherLight( &work->body.objs->objs[CAMERA_PARTS].world,
					  &shift,
					  &work->rgba)
	    );
    }
    else {
	FVECTOR shift = {35.0f, 90.0f, 170.0f, 1.0f};
	GV_SetActorChild( work,
			  NewCypherLight( &work->body.objs->objs[CAMERA_PARTS].world,
					  &shift,
					  &work->rgba)
	    );
    }

    work->hmk_work_p = NewControl_Headmark3( &work->body.objs->objs[CAMERA_PARTS].world,
					     NULL, &work->ctrl);
    GV_SetActorChild( work, work->hmk_work_p);
    //    work->alert = -1 ;


    work->rot_vy_buf  = work->ctrl.rot.vy ;
    work->se_cnt = 0;
    
    if(work->scn_status & SVC_SCN_BROKEN1){
	SV_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行
	work->rctrl.flag &= (RADAR_VISIBLE);
	work->ACTION_LAYER = 0;
	work->ctrl.turn.vx = DIR_X_MAX;
	work->h_trg.class &= ~TARGET_LOCKON; 
    }
    else if(work->scn_status & SV_BREAK_VANISH){
	SV_G1_STEP(work, MOD_BROKEN);	//壊れモードに移行
	work->body.objs->objs[1].flag |= DG_FLAG_INVISIBLE;
	work->h_trg.class |= TARGET_SKIP; 
	work->lens_trg.class |= TARGET_SKIP; 
	work->ACTION_LAYER = 0;
	work->rctrl.flag &= (~RADAR_VISIBLE);
	work->h_trg.class &= ~TARGET_LOCKON; 
    }

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
	    Cam_SetObjs(work);			//Objの位置や向きをセット
	}

    return 1;
}

/*******************************<Global function>********************************/
/*	名前	void *NewSvCamera						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewSvCamera( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),do_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,NORMAL)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewSvPlantCamera						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewSvPlantCamera( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),do_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,PLANT)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewGunGunCamera						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewGunCamera( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),do_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,GUN)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

