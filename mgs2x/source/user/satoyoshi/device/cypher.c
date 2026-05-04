//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cypher.c,v 1.1.1.3 2002/11/19 11:48:15 Yoshizawa1 Exp $		*/
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
#include "../../korekado/enemy/enemy.x"
#include "eyecheck.h"
#include "cypher.h"
#include "layer_check.h"
#include "eye_check.c"

#include "../util/sato_util.h"

#include "cyp_trace.c"
#include "cyp_funcs.c"
#include "cyp_target.c"

#include "cyp_alert.c"
#include "cyp_call.c"
#include "cyp_wbk.c"
#include "cyp_act.c"


#if 0
#define	SATO_DEBUG
#define	PRINT_INFO
#endif



/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern CONTROL *EMA_CommandGetControl() ;

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetObjs							*/
/*	引数:	Work *work   						       	*/
/*	説明:	オブジェの表示				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetObjs(Work *work){
	FMATRIX	world;

	/*ボディをゆっくり,羽を早く回転*/
	work->rot_fin  += ROT_SPEED;
	work->rot_fin  &= 4095 ;

	/*胴体*/

	{	
	    FVECTOR	tmpvec= {0.0f,0.0f,0.0f,1.0f};
	    SVECTOR	tmp2 = {work->rot_body.vx/5,
				work->rot_body.vy/5,
				work->rot_body.vz/5,
			    0};
	    SVECTOR	dummy_rot = {0,0,0,0};
			    

	    work->kyodo_timer += (TIME_BASE+RAND(20));

	    // **上下のゆらゆら
	    tmpvec.vy += sinf(M_PI*work->kyodo_timer/1200.0f)*100.0f;

	    if (work->kyodo_timer > 2400.0f){
		work->kyodo_timer -= 2400.0f;
	    }


	    DG_SetPos2( &work->control.mov, &dummy_rot);

	    // *****銃発射の発射振動
	    if (work->fire_flag){	
		SVECTOR	tmprot = {0,0,0,0};
		FVECTOR	tmpsft= {0.0f,0.0f,-50.0f,1.0f};
		FMATRIX tmpmat;

		_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[1].world);
		_sceVu0CopyVector(&tmpmat.m[3], &DG_ZeroVector);

		DG_SetPos( &tmpmat );
		tmprot.vy = RAND(81)-40;
		DG_RotatePos(&tmprot);
		DG_MovePos(&tmpsft);
		DG_GetPos( &tmpmat);
		_sceVu0CopyVector(&tmpsft, tmpmat.m[3]);

		DG_SetPos2( &work->control.mov, &dummy_rot);
		DG_MovePos(&tmpsft);
		DG_GetPos( &world ) ;
		_sceVu0CopyVector(&work->control.mov, world.m[3]);
	    }

	    DG_MovePos(&tmpvec);
	    DG_GetPos( &world ) ;
	    DG_RotatePos( &tmp2 );

	}

	/*胴体設置*/
	DG_PutObjs( work->body.objs );
	GM_GroupObjs( work->body.objs, work->control.map ) ;
	DG_GetPos( &work->body.objs->objs[0].world  ) ;

	/*羽*/
	DG_SetPos( &world ) ;
	DG_MovePos( &work->body.objs->objs[2].trans ) ;
	{	
	    SVECTOR tmp = {0,work->rot_fin,0,0};
	    DG_RotatePos( &tmp );
	}
	DG_GetPos( &work->body.objs->objs[2].world  ) ;


	/*頭*/
	{
	    FMATRIX	tmpmat;
	    DG_SetPos( &work->body.objs->objs[0].world  ) ;
	    DG_MovePos( &work->body.objs->objs[1].trans ) ;
	    DG_GetPos( &tmpmat ) ;

	    *((FVECTOR*)world.m[3]) = *((FVECTOR*)tmpmat.m[3]);
	}
	DG_SetPos( &world ) ;
	//	DG_MovePos( &work->body.objs->objs[1].trans ) ;
	DG_RotatePos( &work->eye.rot ) ;

	if (work->fire_flag){	// *****発射振動
	    SVECTOR tmprot = {40,0,0,0};
	    DG_RotatePos( &tmprot ) ;
	}

	DG_GetPos( &work->body.objs->objs[1].world  ) ;

	work->rctrl.dir = work->eye.rot.vy ;	//レーダ

#ifdef DEBUG_MODE
    DG_SetPos2( work->eye.eyepos, &work->eye.rot ) ;
    DG_GetPos(&work->eyeview) ;
#endif

}


#ifdef SATO_DEBUG
extern void RouteView(ROUTENAVI *);


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Debug_Print						*/
/*	引数:	Work *work   						       	*/
/*	説明:	デバッグ用のプリント			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void DebugPrint(Work *work)
{
#if 1
    ROUTENAVI	*rnavi;
    rnavi = &work->rnavi ;

    DEV_DebugMode(&work->eye);		//視界表示

    MENU_Locate( 30, 420, 0 ) ;
    MENU_SetColor( 150, 0, 0 ) ;
    MENU_Printf( "STEP :%d   discv: %d  alert: %d   al_mod %d",
		 G1_STEP,
		 work->eye.discv_time,
		 work->eye.alert_time,
		 GM_AlertMode==ALERT_MODE_ALERT
		 );

#endif
}

#endif

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void 							*/
/*	引数:	Work *work   						       	*/
/*	説明:					       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ck_zidou_mv(Work *work){

    
    if (work->zidouset == OFF){
	return;
    }

    if (work->ACTION_LAYER < EYE_LAYER_Alert0){
	return;
    }
    
    if ((work->mokuhyo_flg==GO1)&&(work->noreturn == OFF)){
	if ((GM_AlertMode == ALERT_MODE_SNEAK)||(GM_AlertMode == ALERT_MODE_SEARCH)){
	    work->route =  work->zido_root[0];
	    work->mokuhyo_flg = BK2;
	    work->mode = MOVE_MODE;
printf ("							++GO1 > BK2\n");

	    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
	    cet_tmp(work);
	    work->trg.pos = work->rnavi.nodes[0] ;
	    work->trg.map = work->rnavi.mapbit[0] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    CYP_InitTracePoint( work );
	}
    }

    if ((work->mokuhyo_flg==GO2)&&(work->noreturn == OFF)){
	if ((GM_AlertMode == ALERT_MODE_SNEAK)||(GM_AlertMode == ALERT_MODE_SEARCH)){
	    work->route =  work->zido_root[1];
	    work->mokuhyo_flg = BK1;
	    work->mode = MOVE_MODE;
printf ("							++GO2 > BK1\n");

	    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
	    cet_tmp(work);
	    work->trg.pos = work->rnavi.nodes[0] ;
	    work->trg.map = work->rnavi.mapbit[0] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    CYP_InitTracePoint( work );
	}
    }

    if (work->mokuhyo_flg==BK1){
	if ((GM_AlertMode == ALERT_MODE_ALERT)||(GM_AlertMode == ALERT_MODE_AVOID)
	    ||(work->cyp_call_flag == ON)){
	    work->route =  work->zido_root[2];
	    work->mokuhyo_flg = GO2;
	    work->mode = MOVE_MODE;
printf ("							++BK1 > GO2\n");

	    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
	    cet_tmp(work);
	    work->trg.pos = work->rnavi.nodes[0] ;
	    work->trg.map = work->rnavi.mapbit[0] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    CYP_InitTracePoint( work );
	}

    }

    if (work->mokuhyo_flg==BK2){
	if ((GM_AlertMode == ALERT_MODE_ALERT)||(GM_AlertMode == ALERT_MODE_AVOID)
	    ||(work->cyp_call_flag == ON)){
	    work->route =  work->zido_root[1];
	    work->mokuhyo_flg = GO1;
	    work->mode = MOVE_MODE;
printf ("							++BK2 > GO1\n");

	    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
	    cet_tmp(work);
	    work->trg.pos = work->rnavi.nodes[0] ;
	    work->trg.map = work->rnavi.mapbit[0] ;
	    work->trg.addr = HZX_GetAddress( work->trg.map,
					     &work->trg.pos, -1 ) ;
	    CYP_InitTracePoint( work );
	}
    }

    if (GM_AlertMode == ALERT_MODE_ALERT){
	Dir_from_2Vec(&work->trg.pos, &work->control.mov, &work->control.turn);
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:	レーダーシステムのイニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void InitRadarParam(Work *work){
    RADAR_CTRL *rctrl ;
    rctrl = &work->rctrl;
    rctrl->angle = (work->eye.range.vy)*2 ;/* 視野 */
    //    rctrl->col = RADAR_COLOR_YELOW ;	/* 視野描画色 */
    rctrl->col = RADAR_COLOR_BLUE ;	/* 視野描画色 */
    rctrl->range = work->eye.length;	/* 視力 */
    GM_RadarSetVRange( rctrl, 5000 , -3000 );
}

extern void *NewAutoSplush( FVECTOR *, float );
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work *work   						       	*/
/*		int  name							*/
/*		int  where							*/	
/*	説明:	ワークの確保をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Work *work, int name, int where)
{
    int model;
    SVECTOR	range ;
    float	length ;
    FVECTOR pos = {0.0f, 0.0f, 0.0f, 0.0f};
    SVECTOR rot = {0, 0, 0, 0}; 
    short	loop;
    work->name = name ;
    work->kyodo_timer = RAND(590);

    work->ACTION_LAYER = EYE_LAYER_Normal;

    //  ********ＳＥの設定など
    COM_SetCypher();
    work->com_uniq_id = COM_GetUniqID();
    work->uniq_id = CYPHER_UNIQ_ID;
    CYPHER_UNIQ_ID++;

printf ("  +++++++++++++++		Init cypher %d\n", work->uniq_id);
    switch ((work->uniq_id)%4) {
    case 0 :
	work->move_se_num = SD_E_PHYFAR01 ;
	work->still_se_num = SD_E_PHYFAR05 ;
	work->yota_se_num = SD_E_PHYFAR09 ;
	work->tugi_se_num = SD_E_PHYFAR0D ;
	break;
    case 1 :
	work->move_se_num = SD_E_PHYFAR02 ;
	work->still_se_num = SD_E_PHYFAR06 ;
	work->yota_se_num = SD_E_PHYFAR0A ;
	work->tugi_se_num = SD_E_PHYFAR0E ;
	break;
    case 2 :
	work->move_se_num = SD_E_PHYFAR03 ;
	work->still_se_num = SD_E_PHYFAR07 ;
	work->yota_se_num = SD_E_PHYFAR0B ;
	work->tugi_se_num = SD_E_PHYFAR0F ;
	break;
    case 3 :
	work->move_se_num = SD_E_PHYFAR04 ;
	work->still_se_num = SD_E_PHYFAR08 ;
	work->yota_se_num = SD_E_PHYFAR0C ;
	work->tugi_se_num = SD_E_PHYFAR10 ;
	break;
    default :
	work->move_se_num = 0 ;
	work->still_se_num = 0 ;
    }

    switch ((work->uniq_id)%4) {
    case 0 :
	work->turn_se_num = SD_E_CAMMOV01;
	work->chaf_se_num = SD_E_CAMCHA01;
	break;
    case 1 :
	work->turn_se_num = SD_E_CAMMOV02;
	work->chaf_se_num = SD_E_CAMCHA02;
	break;
    case 2 :
	work->turn_se_num = SD_E_CAMMOV03;
	work->chaf_se_num = SD_E_CAMCHA03;
	break;
    case 3 :
	work->turn_se_num = SD_E_CAMMOV04;
	work->chaf_se_num = SD_E_CAMCHA04;
	break;
    }



    // ********コントロールのイニシャライズ********
    GM_InitControl( &work->control, name, where );	//コントロールシステムに登録
    GM_ConfigControlMessageCheck( &work->control );	//メッセージ受信処理を行なう 	
    GM_ConfigControlMapCheck( &work->control );		//マップ変更チェックを行なう
    GM_ConfigControlTrapCheck( &work->control );	//トラップ変更チェックを行なう

    GM_ConfigControlPosition( &work->control, &pos, &rot);


    work->control.interp = 16 ;
    
    //CONTROLフラグ
	
    if (GCL_GetOptionValue('w', 0) == 1){
	//トラップイベント処理をしない-アドレス変更処理をしない
	work->control.skip_flag = (CTRL_SKIP_GET_ADDRESS);
    }
    else {
	work->control.skip_flag = (CTRL_SKIP_GET_ADDRESS|CTRL_SKIP_HZX);
    }

    work->control.seg_flag = HZX_SEG_NO_PLAYER|HZX_SEG_NO_ENEMY|HZX_SEG_NO_BULLET|HZX_SEG_NO_MISSILE;
    work->control.hzx_check_type = HZX_CHK_ALL|HZX_CHK_CYPHER;	/* サイファー当たりをチェック */


    /*胴体の方向とゆれを乱数セット*/

    /*最高速度*/
    work->max_speed_norm = (float) GCL_GetOptionValue( 's', DEF_MAX_SPEED );
    work->max_speed_warn = (float) GCL_GetOptionValue( 'k', work->max_speed_norm );
    /*加速力*/
    if ( GCL_GetOption( 'f' ) != NULL ){
	work->accel_norm = 0.01f*GCL_GetNextInt() ;
	work->accel_alert = 0.01f*GCL_GetNextInt() ;
    }
    else {
	work->accel_norm = 2.0f;
	work->accel_alert = 2.0f; ;
    }

    /*装備タイプ*/
    work->type = GCL_GetOptionValue( 't', NORMAL_TYPE );

    /*探査目標*/
    work->eye.trgpos = &GM_PlayerFindPos;	//プレイヤをセット
    /*顔の位置*/
    work->eye.eyepos = &work->camera;		//視線の原点をセット
    /*顔の向き*/
    work->eye.rot  = DG_ZeroSVector;
    work->face_y_buf = (int) work->eye.rot.vy;	

    /*視野角*/
    range.vx = GCL_GetOptionValue( 'x', 512 ) ; 
    range.vy = GCL_GetOptionValue( 'y', 512 ) ;
    range.vz = 0;

    /*視力*/
    length = GCL_GetOptionValue( 'i', CYP_EYE_S_DEF  );
    DEV_InitEyeParam(&work->eye,&range,length);	//視界パラメータ設定
    //    work->alert = 0;
    work->mode = MOVE_MODE;

    /*待機*/
    work->status = GCL_GetOptionValue( 'a', 5 );

    if (work->status == 1){
	work->status = CYP_WAIT;
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
	work->hom.status |= HOMING_SKIP;
	set_alltrg_skip(work);
	work->r_trg.class &= ~TARGET_LOCKON;
	work->rctrl.flag &= ~RADAR_VISIBLE;
    }
    else if (work->status == 2){
	work->unreal = ON;
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
	work->hom.status |= HOMING_SKIP;
	set_alltrg_skip(work);
	work->r_trg.class &= ~TARGET_LOCKON;
	work->rctrl.flag &= ~RADAR_VISIBLE;
    }
    else {
	work->status = CYP_ACTIVE;
    }

    /*破壊プロック*/
    work->brake_proc_id = GCL_GetOptionValue( 'p', 0 );


    /*復活回数*/
    work->life = GCL_GetOptionValue( 'l', 0 );
    work->local_life = work->life;

    /*復活回数*/
    work->max_life = GCL_GetOptionValue( 'm', 0 );


    /*エマを攻撃*/
    if ( GCL_GetOption( 'e' ) != NULL ){
	work->emma_flag = ON;
    }



    /*到達プロック*/
    if( GCL_GetOption( 'C' ) != NULL ){
	work->reach_proc_id = GCL_GetInt( GCL_NextStr() );
	work->reach_proc_route = GCL_GetInt( GCL_NextStr() );
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		work->reach_proc_route += COM_GetRootOfset( ) ;
	}
	work->reach_proc_node = GCL_GetInt( GCL_NextStr() );
    }


    /*帰宅場所*/
    if( GCL_GetOption( 'h' ) != NULL ){
	int	loop;
	for (loop=0; loop<1; loop++){
	    work->home_pos[loop].vx = (float)GCL_GetNextInt();
	    work->home_pos[loop].vy = (float)GCL_GetNextInt();
	    work->home_pos[loop].vz = (float)GCL_GetNextInt();
	}
	printf ("				Set home pos %f %f %f\n",
		work->home_pos[0].vx, work->home_pos[0].vy, work->home_pos[0].vz );
    }
    else {
	printf ("				Can't get home pos\n");
    }

    /*スキップフラグ*/
    work->skip_flag = GCL_GetOptionValue( 'F', 0 );

    /* モデル */
    if (work->type == NORMAL_TYPE){
	model = GV_StrCode("cyp") ;
    }
    else {
	model = GV_StrCode("gcyp") ;
    }


    // ****視認敵死体＆眠り兵記憶領域初期化
    for (loop=0; loop<UNIQ_LIST_MAX; loop++){
	work->uniq_id_list[loop] = -1;	//発見した死体や眠り兵を覚えておく
    }

    GM_InitObject(&(work->body),model,(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION) );

    /*ctrlからobjectを参照*/
    GM_ConfigControlObject( &work->control, &work->body ) ;
    GM_ConfigObjectLight(&(work->body),work->lights) ;
    GM_ConfigControlMapID( &work->control ) ;
    DG_SetPos2( &work->control.mov, &work->control.rot ) ;
    DG_PutObjs( work->body.objs );

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		extern int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot ) ;

		C4MAN_Regist( name, &work->control.map, work->body.objs, &work->body.objs->objs[1].world, &DG_ZeroVector, &DG_ZeroSVector ) ;
	}

    // ****自動設定
    if ( GCL_GetOption( 'z' ) != NULL ){
	work->zidouset = ON;
	work->zido_root[0] = GCL_GetNextInt() ;
	work->zido_root[1] = GCL_GetNextInt() ;
	work->zido_root[2] = GCL_GetNextInt() ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		work->zido_root[0] += COM_GetRootOfset( ) ;
		work->zido_root[1] += COM_GetRootOfset( ) ;
		work->zido_root[2] += COM_GetRootOfset( ) ;
	}

	work->mokuhyo_flg = BK2;
    }

	/* zidouset で追跡ルートに来た時にもどらない設定 */
    if ( GCL_GetOption( 'n' ) != NULL ){
	work->noreturn = ON;
    }

    /*高高度指定*/
    work->high_root = GCL_GetOptionValue( 'b', 0 );
    printf("Set high root %d\n", work->high_root);


    // **ルート設定*/
    if ( GCL_GetOption( 'r' ) != NULL ){
	short	now_node;

	work->route = GCL_GetNextInt() ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		work->route += COM_GetRootOfset( ) ;
	}
	now_node = GCL_GetNextInt();

	ENE_InitRouteNavi( &work->rnavi, work->route, now_node );	//ルートナビ初期化
	CYP_InitTracePoint( work );
	//	printf("g\n");
	CYPSetPointAction( work );

    /*ルートポイントに配置*/
    work->trg.pos.vx = work->control.mov.vx = work->rnavi.nodes[ now_node ].vx ;
    work->trg.pos.vy = work->control.mov.vy = work->rnavi.nodes[ now_node ].vy ;
    work->trg.pos.vz = work->control.mov.vz = work->rnavi.nodes[ now_node ].vz ;
    work->nowpos.vx = work->control.mov.vx ;
    work->nowpos.vy = work->control.mov.vy ;
    work->nowpos.vz = work->control.mov.vz ;

    }else {
	return 0 ;
    }

    /* 水落ちエフェクト */
    //    GV_SetActorChild( work,
    //		      NewAutoSplush ( &work->control.mov, 700.0f));

    /*ターゲット設定*/
    SetTarget(work);

    //ヘッドマーク表示用の子アクターを呼ぶ

    work->hmk_work_p = NewControl_Headmark3( &work->body.objs->objs[CAMERA_PARTS].world,
					     NULL, &work->control);
    GV_SetActorChild( work, work->hmk_work_p);


    switch (work->type){	// **サイファのタイプで分岐**
    case NORMAL_TYPE :	//ノーマルサイファ
    {
	FVECTOR	shift = {80, 0, 165, 1.0f};
	SET_COLOR_CYP(0, 255, 0);
	GV_SetActorChild( work,
			  NewCypherLight( &work->body.objs->objs[CAMERA_PARTS].world,
					  &shift,
					  &work->bonbori_color)
	    );
    }
    break ;
    
    case GUN_TYPE :	//GUNサイファ
    {
	FVECTOR	shift = {-80, -200, 545, 1.0f};
	SET_COLOR_CYP(0, 255, 0);
	GV_SetActorChild( work,
			  NewCypherLight( &work->body.objs->objs[CAMERA_PARTS].world,
					  &shift,
					  &work->bonbori_color)
	    );
    }
    break;
    }
    
    
    //レーダーのイニシャライズ
    GM_InitRadarControl(&work->rctrl,&work->camera,
			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->control.map ) ;
    InitRadarParam(work);

#ifdef DEBUG_MODE	//デバッグ用視界表示
    GV_SetActorChild( work,NewEyeView( &work->eyeview,
	(int)length,(int)(range.vy/2),(int)(-range.vx/2),(int)(range.vx/2),COM_GetCommander(),NULL));
#endif
#ifdef SATO_DEBUG
	//当たり判定を表示
    NewTargetView( &work->h_trg, 0, 0, 255 ) ;
    {
	short loop;
	for (loop=0; loop<6; loop++){
	    NewTargetView( &work->b_trg[loop], 255, 0, 0 ) ;
	}
    }
    NewTargetView( &work->r_trg, 0, 255, 0 ) ;
#endif

    //  ********ホーミング設定
    GM_SetHomingTrg( &work->hom, &work->body.objs->world, &work->body, &work->control.hzx_id 
		     ,&work->control, HOMING_MECA ) ;

    GM_PutHomingTrg( &work->hom );

    //コントロールシステムで移動
    GM_ActControl(&work->control) ;

    //ライトマトリックスの取得
    DG_GetLightMatrix( &work->control.mov, work->lights );
	
    SetObjs(work);	//本体位置設定
    SetFaceDir(work);	//カメラ方向設定
    MoveTargets(work);	//ターゲットの移動


    return 1;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ワークの解放をする			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work)
{
    short loop;

    HZX_FlashTrap( work->control.hzx_id, &work->control.evt);	// トラップ抜け

    GM_FreeObject(&(work->body));
    GM_FreeControl( &work->control);
    GM_FreeRadarControl( &work->rctrl);
    GM_FreeTarget( &work->h_trg ) ;
    GM_FreeTarget( &work->r_trg ) ;
    for(loop=0; loop<6; loop++){
	GM_FreeTarget( &work->b_trg[loop] );
    }
    GM_FreeHomingTrg( &work->hom );
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void cyp_act_go_home						*/
/*	引数:	Work *work   						       	*/
/*	説明:	帰宅を実行				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cyp_act_go_home(Work* work)
{
    int num = 0;
    static FVECTOR speed;

    if (work->home_pos[num].vy>work->control.mov.vy+100.0f){
	speed.vy += 3.0f;
    }
    else {
	if (work->home_pos[num].vx > work->control.mov.vx+100.0f){
	    speed.vx += 3.0f;
	}
	else if (work->home_pos[num].vx < work->control.mov.vx-100.0f){
	    speed.vx -= 3.0f;
	}
	if (work->home_pos[num].vz > work->control.mov.vz+100.0f){
	    speed.vz += 3.0f;
	}
	if (work->home_pos[num].vz < work->control.mov.vz-100.0f){
	    speed.vz -= 3.0f;
	}
    }

    if (work->home_pos[num].vy < work->control.mov.vy-100.0f){
	speed.vy -= 3.0f;
    }

    speed.vx *= 0.96f;
    speed.vy *= 0.96f;
    speed.vz *= 0.96f;

    work->control.mov.vx += speed.vx;
    work->control.mov.vy += speed.vy;
    work->control.mov.vz += speed.vz;
}

static int die_check(Work* work){
    // ****死へのカウントダウン
    if (work->act_end_time > 0){
	work->act_end_time -= TIME_BASE;

	if ((0 < work->act_end_time)&&(work->act_end_time <  10)){
	    CYP_Destroy(work) ;
	    COM_CypherDestroy();
	    return 1;
	}
    }
    return 0;
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int CYP_Gohome_Check		** EYE_LAYER_CHAFF **		*/
/*	引数:	Work	*work							*/
/*	説明:								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int CYP_Gohome_Check(Work *work){

    if ( ! ((work->home_pos[0].vx == 0)&&
	    (work->home_pos[0].vy == 0)&&
	    (work->home_pos[0].vz == 0)) ){

	if ( (COM_AlertStatus()&COM_ALERT_ATK_TOTAL_DEFEAT)&&
	     (COM_AlertStatus()&COM_ALERT_WTC_TOTAL_DEFEAT)&&
	     (COM_AlertStatus()&COM_ALERT_SUP_TOTAL_DEFEAT) ){

	    work->go_home_time += TIME_BASE;
	    //	    printf ("+++++ time %d\n", work->go_home_time);
	}
	else {
	    work->go_home_time = 0;
	}

	if ((work->go_home_time > 3000)||(work->go_home_flag == ON)){
	    if (work->ACTION_LAYER > EYE_LAYER_GOHOME){
		G1_STEP = CYP_GOHOME;
		work->ACTION_LAYER = EYE_LAYER_GOHOME;
		work->go_home_flag = ON;
		printf ("#############################Set Go Home !!!!\n");
		return 1;
	    }
	}

    }
    return 0;
}






static int boot_check(Work* work){
    if (work->status == CYP_WAIT){
	// 待機状態を解除して進む
	if ((GM_AlertMode == ALERT_MODE_ALERT)||(GM_AlertMode == ALERT_MODE_AVOID)){
	    if (work->zidouset == ON){
		work->status = CYP_ACTIVE ;
		work->body.objs->flag &= ~DG_FLAG_INVISIBLE;
		work->hom.status &= ~HOMING_SKIP;
		unset_alltrg_skip(work);
		work->r_trg.class |= TARGET_LOCKON;
		work->rctrl.flag |= RADAR_VISIBLE;
	    }
	}
	work->bonbori_color.vw = 0;
	return 1;
    }
    return 0;
}

static SV_FUNCLIST Cyp_Check_Funk[CYP_CHECK_NUM]={
    CYP_Chaff_Check,
    CYP_Gohome_Check,
    CYP_Alert_Check,
    CYP_Emma_Alert_Check,
    CYP_Evasion_Check,
    CYP_Call_Check,
    CYP_Wbk_Check,
};

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Do_Act							*/
/*	引数:	Work *work   						       	*/
/*	説明:	実行関数				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Do_Act(Work *work)
{
    OBJECT 	*body ;
    EYEPARAM	*eye = &work->eye;
    int		loop;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( GM_VRStatus & (GM_VR_IDLE|GM_VR_CLEAR) ) return ;
	}

#ifdef DEBUG_MODE
    //    printf ("M:%d  %d\n", G1_STEP, work->ACTION_LAYER);
    if (COM_GetCommander()->status & CMST_ENEMY_SIGHT_VIEW){
	FVECTOR	verts[2];
	_sceVu0CopyVector(&verts[0], eye->trgpos);
	_sceVu0CopyVector(&verts[1], eye->eyepos);
	NewLineView(verts, 1, 100, 255, 0);
    }

    //    MENU_Locate( 400, 50, 0 ) ;
    //    MENU_SetColor( 200, 0, 0 ) ;
    //    MENU_Printf( "MOD %d %d %d", G1_STEP, G2_STEP, work->ACTION_LAYER);

#endif   

    work->rctrl.flag &= ~(RADAR_RADIO);

    body = 	&work->body;
    work->fire_flag = 0;
    GM_ActControl(&work->control);
    CheckMessage(work);	//メッセージのチェック

    if (work->unreal == ON){
	return;
    }

    // メカアイチェック
    Meca_Eye_Check( &work->eye );


#if 0
    if (COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)
	printf ("Pc detect\n");
    if (COM_AlertStatus()&COM_ALERT_NPC_DETECT)
	printf ("Emma detect\n");
#endif

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
	if ( !(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)){// &&
	     //	     (COM_AlertStatus()&COM_ALERT_NPC_DETECT) ){
	    COM_SetPlayerLastPos(EMA_CommandGetPosition(),GM_GetHzxGroupID(EMA_CommandGetControl()->map));
	}
    }

    if (die_check(work)){	// 死へのカウントダウン
	return;
    }

    if ((GM_AlertMode == ALERT_MODE_ALERT)||(GM_AlertMode == ALERT_MODE_AVOID)){
	work->max_speed = work->max_speed_warn; 
	work->accel = work->accel_alert;
    }
    else {
	work->max_speed = work->max_speed_norm; 
	work->accel = work->accel_norm;
    }


    if (boot_check(work)){	// 待機を解除して突き進む
	return;
    }

    
    if ( work->ACTION_LAYER >= EYE_LAYER_Alert0 ){
	ck_zidou_mv(work);
    }

    // ===============レイヤーチェックとモード切替え
    for (loop=0; loop<CYP_CHECK_NUM; loop++){
	if (Cyp_Check_Funk[loop](work)){
	    break;
	}
    }

    switch(G1_STEP){// ****************ステップ分岐****************

    case CYP_WAIT4RESURECT:	// *****復活待ち*****
	if (GM_AlertMode == ALERT_MODE_ALERT){
	    CYP_Destroy(work);
	}
	break;

    case CYP_NORMAL:	// *****ノーマルモード*****
	cyp_norm_act(work);
	SET_COLOR_CYP_A(64, 180, 112, 156);
	work->rctrl.col = RADAR_COLOR_BLUE;
	break;

    case CYP_ALERT:	// *****警戒モード*****
	SET_COLOR_CYP_A(255, 80, 48, 156);
	cyp_alert_control(work);
	break;

    case CYP_EVASION:	// *****警戒モード*****
	SET_COLOR_CYP_A(180, 148, 64, 156);
	cyp_evasion_control(work);
	break;

    case CYP_EMMA_ALERT:	// *****警戒モード*****
	SET_COLOR_CYP_A(255, 80, 48, 156);
	cyp_emma_alert_control(work);
	break;

    case CYP_CALL:	// *****呼びモード*****
	SET_COLOR_CYP_A(180, 148, 64, 156);
	cyp_call_control(work);
	break;

    case CYP_WBK:	// *****白びっくり*****
	SET_COLOR_CYP_A(180, 148, 64, 156);
	cyp_wbk_control(work);
	break;

    case CYP_CHAFF:	// *****チャフモード*****
    {
	int temp = 128;
	int r, g, b;
	int alpha = RAND(240)+10;

	work->rctrl.col = RADAR_COLOR_BLUE;

	r = temp+RAND(201)-100;
	if (r>255){
	    r = 255;
	}
	if (r<0){
	    r = 0;
	}
	g = temp+RAND(201)-100;
	if (g>255){
	    g = 255;
	}
	if (g<0){
	    g = 0;
	}
	b = temp+RAND(201)-100;
	if (b>255){
	    b = 255;
	}
	if (b<0){
	    b = 0;
	}
	SET_COLOR_CYP_A(r, g, b, alpha);
	cyp_act_chaff(work);	
    }
	break;

    case CYP_DAMAGE:	// *****墜落モード*****
	work->rctrl.col = RADAR_COLOR_YELOW;
	SET_COLOR_CYP(255, 0, 0);
	cyp_act_stall(work);
	break;

    case CYP_GOHOME:	// *****帰宅モード*****
	work->rctrl.col = RADAR_COLOR_BLUE;
	SET_COLOR_CYP_A(64, 180, 112, 156);
	cyp_act_go_home(work);
	break;

    case CYP_DEMO:	// *****DEMOモード*****
	cyp_norm_act(work);
	SET_COLOR_CYP_A(255, 80, 48, 156);
	work->rctrl.col = RADAR_COLOR_BLUE;
	break;

    }

    
    {	//ダメージのふらつき

	if ( (work->damage_force.vx != 0.0f)||(work->damage_force.vz != 0.0f) ){
	    if (HZX_NearHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
				 &work->control.mov, 1200.0f, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,
				     (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE), 0.0f)){
		work->damage_force.vx = 0.0f;
		work->damage_force.vy = 0.0f;
		work->damage_force.vz = 0.0f;
		printf("CYP Hit Wall Check !! \n");
	    }
	}
	
	work->control.step.vx += work->damage_force.vx*0.1f;
	work->control.step.vy += work->damage_force.vy*0.1f;
	work->control.step.vz += work->damage_force.vz*0.1f;
	work->damage_force.vx = work->damage_force.vx * 0.6f;
	work->damage_force.vy = work->damage_force.vy * 0.6f;
	work->damage_force.vz = work->damage_force.vz * 0.6f;
	
	
	work->rot_body.vx = work->rot_body.vx * 0.92;
	work->rot_body.vz = work->rot_body.vz * 0.92;
	
	work->rot_body.vx += work->damage_rot.vx;
	work->rot_body.vz += work->damage_rot.vz;
	work->damage_rot.vx = 0;
	work->damage_rot.vz = 0;
    }


    work->control.interp = 8 ;

    //ライトマトリックスの取得
    DG_GetLightMatrix( &work->control.mov, work->lights );
	
    SetObjs(work);	//本体位置設定

    //    if (G1_STEP != CYP_CHAFF){
	SetFaceDir(work);	//カメラ方向設定
	//    }

    MoveTargets(work);	//ターゲットの移動

    cyp_sound_cnt(work);


#ifdef SATO_DEBUG
    DebugPrint(work);	//いろいろな情報表示
#endif

}

/*******************************<Global function>********************************/
/*	名前:	void *NewCypher							*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewCypher( name , where )
int	name ;
int	where ;
{
    Work *work ;
    int	mem;

    mem = GV_GetMaxFreeMemory(GV_NORMAL_MEMORY);

    printf ("All: %d\n",GV_GetFreeMemorySize(GV_NORMAL_MEMORY));
    printf ("Max: %d\n",GV_GetMaxFreeMemory(GV_NORMAL_MEMORY));

    if (mem < 100000){
	printf ("########### CYPHER Can't Get Enough memory\n");
	return NULL ;
    }

    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Act,Die) ;
	GV_ActorEX( &work->actor ) ;
	if(!GetResources( work,name,where )){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    printf ("End All: %d\n\n",GV_GetFreeMemorySize(GV_NORMAL_MEMORY));
    return (void *)work ;
}




