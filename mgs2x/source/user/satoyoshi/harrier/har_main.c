//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Har_main.c								*/
/*	ハリアメイン *NewHarrier						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_main.c,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include "harrier.h"
#include "harrier.def"

#include "fly_data.c"

#include "har_inline.c"
#include "har_message.c"

#include "har_target.c"
#include "har_debug.c"

#include "har_vibration.c"

#include "har_parts.c"
#include "har_move.c"
#include "har_flare.c"

#include "harp_norm.c"
#include "har_pmove.c"
#include "harp_amram.c"

#include "har_vulc.c"
#include "har_seeffect.c"
#include "har_pirots.c"
#include "har_missile.c"
#include "har_claster.c"
#include "har_mpod.c"
#include "har_acrobat.c"
#include "har_bakudan.c"
//#include "har_mpatack.c"
#include "har_burn.c"
#include "har_hover.c"
#include "har_mphov.c"
#include "har_mpmv.c"
#include "har_start.c"

#include "har_damage.c"
#include "har_sound.c"
#include "har_think.c"

//yano 2002.02.22
//一部 har_global.c に移行
ACRO_POINT* LAST_ACROPOINT;
ACRO_POINT* pointdata[30];
ACRO_POINT_EX* pointexdat[30];
int HAR_Canp_Break;


/********************************************************************************/
/*	Program									*/
/********************************************************************************/
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Work *work){

#ifdef KP_XBOX
	HAR_ReleaseSngTrackTable();
#endif	
	
    GM_FreeObject(&(work->body));
    GM_FreeControl( &work->control);
    RDR_FinishOutrangeTarget(&work->rctrl);
    GM_RemoveGageSet( &work->gageset );

    Clean_damage_tex_set(work);			//壊れテクスチャセット

    GM_FreeTarget( &work->target_b_nzl[0]);
    GM_FreeTarget( &work->target_b_nzl[1]);
    GM_FreeTarget( &work->target_body_center);		//胴体の芯
    GM_FreeTarget( &work->target_body_side[0]);		//胴体下
    GM_FreeTarget( &work->target_body_side[1]);		//胴体上
    GM_FreeTarget( &work->target_body_side[2]);		//胴体左
    GM_FreeTarget( &work->target_body_side[3]);		//胴体右
    GM_FreeTarget( &work->target_body_tale);		//胴体しっぽ
    GM_FreeTarget( &work->target_body_pod[0]);		//胴体下面ポッド右
    GM_FreeTarget( &work->target_body_pod[1]);		//胴体下面ポッド右
    GM_FreeTarget( &work->target_body_pod[2]);		//胴体下面ポッド
    GM_FreeTarget( &work->target_wepon_mpod[0]);	//ミサイルポッド右
    GM_FreeTarget( &work->target_wepon_mpod[1]);	//ミサイルポッド左
    GM_FreeTarget( &work->target_rwing[0]);		//右翼0
    GM_FreeTarget( &work->target_rwing[1]);		//右翼1
    GM_FreeTarget( &work->target_lwing[0]);		//左翼0
    GM_FreeTarget( &work->target_lwing[1]);		//左翼1
    GM_FreeTarget( &work->target_tale_wing[2]);		//垂直尾翼
    GM_FreeTarget( &work->target_tale_wing[0]);		//垂平尾翼0
    GM_FreeTarget( &work->target_tale_wing[1]);		//垂平尾翼1
    GM_FreeTarget( &work->target_body_canopy);		//キャノピー
    GM_FreeTarget( &work->target_body_nose);		//ノーズ
    GM_FreeTarget( &work->target_body_centertop);	//胴体の上
    GM_FreeTarget( &work->target_wheel_bar[0]);		//車輪右
    GM_FreeTarget( &work->target_wheel_bar[1]);		//車輪左
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void ClearPadData						*/
/*	引数:	PAD_DATA *pad_d						       	*/
/*	説明:	パッドワークをクリアする			 		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ClearPadData(PAD_DATA *pad_d){
    pad_d->right_x = 128;
    pad_d->right_y = 128;
    pad_d->left_x = 128;
    pad_d->left_y = 128;
    pad_d->buton = 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void copy_action_data						*/
/*	引数:	PAD_DATA *pad_d		コピー先				*/
/*		PAD_DATA *act_d		コピー元				*/
/*	説明:	アクションデータからパッドデータへのコピー	    		*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void copy_action_data(PAD_DATA *pad_d, PAD_DATA *act_d){
    pad_d->right_x = act_d->right_x;
    pad_d->right_y = act_d->right_y;
    pad_d->left_x = act_d->left_x;
    pad_d->left_y = act_d->left_y;
    pad_d->buton = act_d->buton;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void har_set_move_object						*/
/*	引数:	Work *work   						       	*/
/*	説明:	再生時に狙いをのっとる			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void har_set_move_object(Work* work){

    DG_SetPos2( &work->control.mov, &work->control.rot ) ;

    harDamageaction(work);

    DG_GetPos( &work->body.objs->world  ) ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void directAiming						*/
/*	引数:	Work *work   						       	*/
/*	説明:	再生時に狙いをのっとる			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void directAiming(Work *work){

    // ***********機銃ついび計算
    {
	FMATRIX	tmat;
	static FVECTOR	verts[2];
	static FVECTOR	testverts[2];
	static FVECTOR	xverts[4];
	FVECTOR	line = {0.0f, 0.0f, 80000.0f, 1.0f};	
	
	_sceVu0CopyMatrix(&tmat, &work->body.objs->objs[0].world );
	
	tmat.m[3][0] = work->gun_pos.vx;
	tmat.m[3][1] = work->gun_pos.vy;
	tmat.m[3][2] = work->gun_pos.vz;
	
	DG_SetPos(&tmat);
	DG_MovePos(&line);
	DG_GetPos(&tmat);
	Fvec_from_Matrix( &line, &tmat );
	
	_sceVu0CopyVector(&verts[0], &line);
	
	if ( HZX_OnlineHazardCheck(0, &verts[1], &verts[0], HZX_CHK_F_FLOOR|HZX_CHK_RECOIL_TYPE_ONLY,	//ハザードに接触
				   (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE), (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE)) ){
	    FVECTOR	point, direction;
	    HZX_HZD 	seg;
	    int		tmp;
	    HZX_GetOnlineHazard( &seg, &tmp );	//ハザードを取得
	    HZX_GetOnlinePoint( &point );	//接触点を取得
	    
	    _sceVu0CopyVector(&verts[0], &point);
	    
	    HZX_GetOnlineVector( &direction );	//弾の方向ベクトルを取得
	}
	
	if (work->aiming_flg)
	{
	    FVECTOR	aim_vec;
	    SVECTOR	tmprot;
	    
	    // *****************:狙い位置確定
	    aim_vec.vx = work->p_position.vx;
	    aim_vec.vz = work->p_position.vz-W25A_STAGE_Z_SHIFT;
	    aim_vec.vy = work->p_position.vy;

	    Dir_from_2Vec(&work->gun_pos, &aim_vec, &tmprot);
	    {
		FVECTOR	tmpline = {0.0f, 0.0f, 80000.0f, 1.0f};	
		FMATRIX	pmat;
		
		DG_SetPos2( &work->control.mov, &tmprot);
		
		DG_MovePos( &tmpline ) ;
		DG_GetPos( &pmat ) ;
		
		work->control.turn.vx = tmprot.vx;
		work->control.turn.vy = tmprot.vy;
		
		Fvec_from_Matrix( &tmpline, &pmat );
		_sceVu0CopyVector(&testverts[0], &tmpline);
		_sceVu0CopyVector(&testverts[1], &work->gun_pos);
		if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
		    //		    NewLineView(testverts, 1, 50,250,50);
		}
	    }
	}
	
	_sceVu0CopyVector(&verts[1], &work->gun_pos);
	
	_sceVu0CopyVector(&xverts[0], &verts[0]);
	_sceVu0CopyVector(&xverts[1], &verts[0]);
	_sceVu0CopyVector(&xverts[2], &verts[0]);
	_sceVu0CopyVector(&xverts[3], &verts[0]);
	
	xverts[0].vx = verts[0].vx + 500.0f;
	xverts[1].vx = verts[0].vx - 500.0f;
	xverts[2].vx = verts[0].vx + 500.0f;
	xverts[3].vx = verts[0].vx - 500.0f;
	xverts[0].vz = verts[0].vz + 500.0f;
	xverts[1].vz = verts[0].vz - 500.0f;
	xverts[2].vz = verts[0].vz - 500.0f;
	xverts[3].vz = verts[0].vz + 500.0f;
	
	if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
	    //	    NewLineView(verts, 1, 50,50,250);
	    //	    NewLineView(xverts, 2, 250,50,50);
	}
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetObjs							*/
/*	引数:	Work *work   						       	*/
/*	説明:	オブジェの表示				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetObjs(Work *work){
    
    work->l_flap_rot =-work->r_flap_rot;
    work->noz_bk_rot = work->noz_fr_rot;
    
    if (work->noz_bk_rot < -0.8f ){	//後ろノズルには限界角度設定
	work->noz_bk_rot = -0.8f;
    }

    if (work->control.rot.vy > 4095){
	work->control.rot.vy = work->control.rot.vy - 4096;
    }
    if (work->control.rot.vy < -4095){
	work->control.rot.vy = work->control.rot.vy + 4096;
    }
    
    DG_SetPos( &work->body.objs->objs[0].world  ) ;
    DG_GetPos( &work->body.objs->world  ) ;
    
    DG_GetLightMatrix( &work->control.mov, work->lights );
    // *****レーダ表示
    //    work->rctrl.dir = work->control.rot.vy ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work   						       	*/
/*	説明:	メッセージを受信する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->control.n_msg ;
    msg = work->control.msg ;
    
    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ] ;
	switch( code ) {
	case HAR_MES_SET_POSITION:
	    work->control.mov.vx = msg->message[1] * 1.0f ;
	    work->control.mov.vy = msg->message[2] * 1.0f ;
	    work->control.mov.vz = msg->message[3] * 1.0f ;
	    printf ("Harrier Message Get [Move Position] %d %d %d \n",
		    msg->message[1],msg->message[2],msg->message[3]);
	    break;
	    
	case HAR_MES_SET_DIRECTION:
	    work->control.rot.vx = msg->message[1];
	    work->control.rot.vy = msg->message[2];
	    work->control.rot.vz = msg->message[3];
	    work->control.turn.vx = msg->message[1];
	    work->control.turn.vy = msg->message[2];
	    work->control.turn.vz = msg->message[3];
	    HAR_MODE_L1 = H_MOD_L1_PROGRAM_MOVE;
	    printf ("Harrier Message Get [Set Direction] %d %d %d \n",
		    msg->message[1],msg->message[2],msg->message[3]);
	    break;
	    
	case HAR_MES_SET_GOAL:
	    work->goal_pos.vx = msg->message[1] * 1.0f ;
	    work->goal_pos.vy = msg->message[2] * 1.0f ;
	    work->goal_pos.vz = msg->message[3] * 1.0f ;
	    HAR_MODE_L1 = H_MOD_L1_PROGRAM_MOVE;
	    printf ("Harrier Message Get [Set Goal] %d %d %d \n",
		    msg->message[1],msg->message[2],msg->message[3]);
	    break ;
	    
	case HAR_MES_SET_SPEED:
	    work->max_speed = msg->message[1] * 1.0f ;
	    printf ("Harrier Message Get [Set Speed] %d  \n",
		    msg->message[1]);
	    break ;
	    
	case HAR_MES_SET_ROTATE:
	    work->z_turn_speed = msg->message[1];
	    printf ("Harrier Message Get [Set Rotate] %d  \n",
		    msg->message[1]);
	    break ;
	    
	case HAR_MES_MP_FIRE:	//ミサイルポッド発射	
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_MPOD|HAR_SIGNAL_FIRE, 0);
	    printf ("Harrier Message Get [MisPod fire]\n");
	    break;
	    
	case HAR_MES_GUN_FIRE:	//機銃発射
	{
	    printf ("Harrier Message Get [Gun fire] %d  \n",
		    msg->message[1]);
	    
	    if (msg->message[1]){
		SET_GUN_MODE(GUN_FIRE_FLG_NORM, GUN_FIRE_FLG_HIT);
	    }
	    else  {
		SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
	    }
	    work->gun_time = 0;
	    work->gun_time_first = 0;
	}
	break;
	case HAR_MES_ACCEL:	//加速減速
	    printf ("Harrier Message Get [Accel & Brake] %d %d \n",
		    msg->message[1],msg->message[2]);
	    work->target_speed = msg->message[1] * 1.0f ;
	    work->accel_speed = msg->message[2];
	    break;
	    
	case HAR_MES_AMRAM_FIRE:	//アムラーム発射
	    printf ("Harrier Message Get [Amram fire]\n");
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_FIRE, 0);
	    break;
	    
	case HAR_MES_RELOAD:		//リロード
	    printf ("Harrier Message Get [Reload]\n");
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_AMRAM|HAR_SIGNAL_EQUIP, 0);
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_EQUIP, 0);
	    break;
	    
	case HAR_MES_CLASTER:		//クラスター爆弾投下
	    printf ("Harrier Message Get [Claster fire]\n");
	    GV_CallChildSignalFunc(work, HAR_SIGNAL_CLASTER|HAR_SIGNAL_FIRE, 0);
	    break;

	case HAR_MES_MIS_FOLLOW:	//ミサイル追尾 ON/OFF
	    printf ("Harrier Message Get [Mis Follow] %d \n",
		    msg->message[1]);
	    if (msg->message[1] == 0){
		work->har_flag &= ~HAR_DOUSA_FLAG_FOLLOW;
	    }
	    else if (msg->message[1] == 1){
		work->har_flag |= HAR_DOUSA_FLAG_FOLLOW;
	    }
	    break;
	}
	
	msg++ ;
    }
}

#define		POS_UP		(0x0100)
#define		POS_DOWN	(0x0200)
#define		POS_STARE	(0x0010)
#define		POS_CENTER	(0x0020)
#define		POS_RIGHT	(0x0040)
#define		POS_LEFT	(0x0080)
#define		POS_TOP_R	(0x0001)
#define		POS_TOP_L	(0x0002)
#define		POS_BOTOM_R	(0x0004)
#define		POS_BOTOM_L	(0x0008)

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void get_player_pos						*/
/*	引数:									*/
/*	説明:	プレイヤの位置を得る						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void get_player_pos(Work * work)
{
    FVECTOR	pc_pos;
    
    MENU_SetColor( 50, 250, 50 );
    MENU_Locate( 50, 300, 0 );
    
    _sceVu0CopyVector(&pc_pos, &GM_PlayerFindPos);
    
    pc_pos.vz += W25A_STAGE_Z_SHIFT;
    
    _sceVu0CopyVector(&work->p_position, &pc_pos);

}




/* プレイヤー消火特殊処理メッセージ受信関数 */
static void ExtraRecieveMessage( Work *work )
{

    GV_MSG*	msg;
    int 	n_msg;


    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( GM_PLAYER_CHAR_BODY_FLAME, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){
	switch ( msg->message[ 0 ] ) {
	case 0:			/* fade_in, fade_out */
	    work->fire_damage_point = 0;
	    printf("harrier_fire-------------extra_extinguish!!\n");
	    break;
	}
	msg++;
    }
}


extern int gBP_KillCurrentBoss;
extern int gBP_DamageCurrentBoss;

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Do_Har_Act							*/
/*	引数:	Work	*work							*/
/*	説明:	ハリアの挙動メイン:						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Do_Har_Act(Work *work)
{

#if 0
    if (!har_gameover_check_ok()){
	printf ("##The END\n");
    }
#endif

    // these work the same for the harrier - not sure how to kill it in one stroke
    if (gBP_KillCurrentBoss || gBP_DamageCurrentBoss)
    {
       gBP_KillCurrentBoss = FALSE;
       gBP_DamageCurrentBoss = FALSE;
       work->har_damage = 10000;
    }

    if (KAS_ITEM_FLAG == 2){

	har_call_item_proc(work);
	KAS_ITEM_FLAG = 0;
    }

    _sceVu0SubVector(&work->Speed, &work->control.mov, &work->Oldpos);
    work->Oldpos = work->control.mov;

    //    printf ("S: %f, %f, %f\n", work->Speed.vx, work->Speed.vy, work->Speed.vz);


#if 0
    MENU_Locate( 400, 50, 0 ) ;
    MENU_SetColor( 200, 0, 0 ) ;
    MENU_Printf( "MOD %d %d %d", HAR_MODE_L1, HAR_MODE_L2, HAR_MODE_L3);
#endif

    if (work->GAME_OVER_FLAG == OFF){
	Har_ActPlayStream(work);	//ストリーミングの管理
	check_damages(work);
    }
    
    GM_ActControl(&work->control) ;

    CheckMessage(work);	//メッセージのチェック

    get_player_pos(work);	//プレイヤの位置取得表示

    work->zenkei_rot = 45;


#ifdef DEBUG_MODE
    {
	static int old_dbgmode;
	if ((HAR_DbgMode != 0)&&(old_dbgmode != HAR_DbgMode)){
	    next_move(work);
	}
	old_dbgmode = HAR_DbgMode;
    }
#endif


    switch (HAR_MODE_L1){
    case H_MOD_L1_START:	//ハリア戦開始
	if (control_start(work)){  //  ******終了
	    next_move(work);
	}
	break;

    case H_MOD_L1_AMRAM:	//アムラーム
	Har_Move2Goal(work);
	if (atack_amram(work)){  //  ******終了
	    next_move(work);
	}
	break;
	
    case H_MOD_L1_PROGRAM_MOVE:    // *******機銃攻撃
	Har_Move2Goal(work);
	if (Har_ControlPmove(work)){  //  ******終了
	    next_move(work);
	}
	break;

#if 0	
    case H_MOD_L1_MISSILEPOD:    // *******ミサイルポッド
	if (control_mpod(work)){
	    next_move(work);
	    //	    SET_STEP_L1(H_MOD_L1_PROGRAM_MOVE);
	}
	break;
#endif

    case H_MOD_L1_BURNING:    // *******排気炎焼き
	work->zenkei_rot = 0;
	//	apply_move(work);
	if (control_burning(work)){
	    next_move(work);
	    //	    SET_STEP_L1(H_MOD_L1_PROGRAM_MOVE);
	}
	break;


    case H_MOD_L1_ACROBAT_MOVE:	// ***	アクロバット飛行
	if (acrobat_move(work)){
	    next_move(work);
	    //	    SET_STEP_L1(H_MOD_L1_PROGRAM_MOVE);
	}
	break;

    case H_MOD_L1_CLASTER:
	if (bakudan_move(work)){
	    next_move(work);
	    //	    SET_STEP_L1(H_MOD_L1_PROGRAM_MOVE);
	}
	break;

    case H_MOD_L1_HOVER_GUN:	// ***	気中くるくる
	if (control_hover(work)){
	    next_move(work);
	    //	    SET_STEP_L1(H_MOD_L1_ACROBAT_MOVE);
	    //	    set_acro(work);
	}
	break;

    case H_MOD_L1_HOVER_MP:	// ***	新ミサイルポッド
	if (control_mphov(work)){
	    next_move(work);
	}
	break;

    case H_MOD_L1_STOP:	// ***	デバッグ用停止
	next_move(work);
	break;
    }



    har_set_move_object(work); //ダメージ挙動

    if (work->hokan_timer>0){	// *******補間移動しますよ
	    FMATRIX tmpmat;
	    DG_SetPos2 (&work->act_move->begin_pos, &work->act_move->begin_rot);
	    DG_GetPos (&tmpmat);
	    
	    Har_HokanMove( &work->body.objs->world, work, &tmpmat, work->hokan_timer);
	    work->hokan_timer--;
    }
    else {
	directAiming(work);
    }

    Har_MoveParts(work);
    Har_SeEffect(work);
    Har_MoveTarget(work);


    SetObjs(work);
    
    GM_SubjectVMaxTmp[1] = -900;	//プレイヤ上向き限界角度

#ifdef SATO_DEBUG
    Har_DBMoveCamera(work);
    Har_DBPrintMenu(work);
#endif

    HarrierBGMFader( work ) ;



    if (work->fire_damage_point>0){
	ExtraRecieveMessage(work);
	if (work->fire_damage_point%16==0){
	    printf("Burn Damage\n");
	    GM_VitalityAdjust = GM_VitalityAdjust-1;
	}
	work->fire_damage_point--;	//何も考えずにTIME_BASEにすると危険
    }

}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void SetParts							*/
/*	引数:	Work	*work							*/
/*	説明:	パーツの設定 GetResource内で呼びだし				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void SetParts(Work *work, int where)
{
    {	//  *****パイロット*****
	GV_SetActorChild( work,NewHarPilots(work) );//子アクターを呼ぶ
    }
    {	//  *****ミサイル*****
	FVECTOR	tmp ={	3950.0f, -340.0f, 700.0f};
	GV_SetActorChild( work,NewHarMissile(work, &tmp, where) );//子アクターを呼ぶ
	tmp.vx = -3950.0f;
	GV_SetActorChild( work,NewHarMissile(work, &tmp, where) );//子アクターを呼ぶ
    }
    {	//  *****クラスター*****
	FVECTOR	tmp ={	3270.0f, -300.0f, 0.0f};
	GV_SetActorChild( work,NewHarClaster(work, &tmp, where) );//子アクターを呼ぶ
	tmp.vx = -3270.0f;
	GV_SetActorChild( work,NewHarClaster(work, &tmp, where) );//子アクターを呼ぶ
    }
    printf ("***********************   Here come Missile Pod\n");
    {	//  *****ミサイルポッド弾*****
	GV_SetActorChild( work,NewHarMpod(work, where, MPOD_LEFT) );//子アクターを呼ぶ
	GV_SetActorChild( work,NewHarMpod(work, where, MPOD_RIGHT) );//子アクターを呼ぶ
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:	レーダーシステムのイニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void HarInitRadarParam(Work *work){
    RADAR_CTRL *rctrl ;
    
    //int RDR_InitOutrangeTarget(OUTRANGE_CTRL * orange,
    //			   FVECTOR * mov, int flag, int map)

    RDR_InitOutrangeTarget(&work->rctrl,&work->control.mov,
			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->control.map ) ;

//    GM_InitRadarControl(&work->rctrl,&work->control.mov,
//			RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, work->control.map ) ;

    rctrl = (RADAR_CTRL*)&work->rctrl;
    rctrl->angle = 1;			/* 視野 */
    rctrl->col = RADAR_COLOR_YELOW ;	/* 視野描画色 */
    rctrl->range = 1000000.0f;	/* 視力 */
    GM_RadarSetVRange( rctrl, 50000 , -15000 );	//上下視野の限界
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HarInitControlParam					*/
/*	引数:	Work	*work							*/
/*	説明:	コントロールのイニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void HarInitControlParam(Work *work, int name, int where){

    GM_InitControl( &work->control, name, where );	//コントロールシステムに登録
    GM_ConfigControlMessageCheck( &work->control );	//メッセージ受信処理を行なう 	
    GM_ConfigControlMapCheck( &work->control );		//マップ変更チェックを行なう

    GM_ConfigControlNoSegmentArea( &work->control );	//当たりチェックしないからハザード格納領域はいらない

    work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	 |CTRL_SKIP_GET_ADDRESS
	 |CTRL_SKIP_NEAR_CHECK); 

    GM_ConfigControlObject( &work->control, &work->body);//controlにbodyを連結
    GM_ConfigControlMapID( &work->control ) ;
    GM_ConfigControlTrapCheck( &work->control ) ;	//トラップチェックを行なう

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HarInitControlParam					*/
/*	引数:	Work	*work							*/
/*	説明:	コントロールのイニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void HarInitObjectlParam(Work *work){

    //リアルタイムシェード
    GM_InitObject(&work->body,GV_StrCode( "hri_def" ),
		  //    GM_InitObject(&work->body,GV_StrCode( "hri_brk1" ),
		  //		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_NOFOG|);
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|
		  DG_FLAG_FOGPARAM);
    DG_SetFogParamObjs (work->body.objs, -40000.0f, 500000.0f);

    //初期化した回転ベクトルをオブジェクトの関節に結び付ける
    //以降自動的に反映される
    //    GM_ConfigObjectJoint(&work->body, work->rots);
    
    //ライトマトリクスの設定  毎フレーム lightmatrixを設定すれば自動反映
    GM_ConfigObjectLight(&(work->body),work->lights);
}

void *Burn_CallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;
    int		time = 150;
    work = (Work *)ptr ;    

    if (GM_PlayerFindPos.vy < -1000.0f){
	return 0;
    }

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	time = 250;
	break;
    case ST_LEV_EASY:
	time = 180;
	break;
    case ST_LEV_NORMAL:
	time = 120;
	break;

    case ST_LEV_UPNORM:
	time = 95;
	break;

    case GM_LEVEL_HARD:
	time = 70;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	time = 30;
	break;
    }

    if (!GM_CheckPlayerStatus( PLAYER_INVINCIBLE_ALL )){
	if (work->burn_dm_time > time){	// 継続ダメージ
	    GM_VitalityAdjust = GM_VitalityAdjust-1;
	    work->burn_dm_time -= time;
	    GM_SeSetMode (SD_V_PDMG02, &work->control.mov, GM_SEMODE_BOMB);
	}
    }

    work->burn_dm_time += TIME_BASE;
    call_pad_vibration(VIB_BURNING);	//パッド振動
    return 0;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HarInitControlParam					*/
/*	引数:	Work	*work							*/
/*	説明:	コントロールのイニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Har_SetEffectColors(Work* work){
    int	loop;

    if ( GCL_GetOption( 'c' ) ){	//color
	printf ("Get color\n");
	for (loop=0; loop<4; loop++){	//	ノズルブラー	
	    work->nozl_blur_col[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->nozl_blur_col[loop]);
	}
	printf ("\n");
	for (loop=0; loop<4; loop++){	//	ボンボリ色
	    work->bonbori_col[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->bonbori_col[loop]);
	}
	printf ("\n");
	for (loop=0; loop<4; loop++){	//	Tempライトカラー
	    work->temp_light_col[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->temp_light_col[loop]);
	}
	printf ("\n");
	for (loop=0; loop<8; loop++){	//	ノズル攻撃色
	    work->nozl_atack_col[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->nozl_atack_col[loop]);
	}
	printf ("\n");
	for (loop=0; loop<8; loop++){	//	ロケットバーニア
	    work->missile_burn_col[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->missile_burn_col[loop]);
	}
	printf ("\n");
	for (loop=0; loop<8; loop++){	//	ミサイルバーニア
	    work->missile_burn_col2[loop] = (u_char)GCL_GetNextInt();
	    printf ("%d ", work->missile_burn_col2[loop]);
	}
	printf ("\n");
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Work *work, int name, int where, int flag){
    int		buf[4];

    buf[3] = RAND(1);

    //  *****ミサイルポッド煙フラグ初期化
    work->dam_mpsmk_flg[0] = work->dam_mpsmk_flg[1] = ON;

    GET_HAR_WORK = work;	// ポインタの登録

    //		*****オブジェクトのイニシャライズ*****
    HarInitObjectlParam(work);

    //		********コントロールのイニシャライズ********
    HarInitControlParam(work, name, where);
    

    //		*******ホーミング設定
    GM_SetHomingTrg( &work->hom, &work->body.objs->world, &work->body, &work->control.hzx_id 
		     , &work->control, 0 );
    GM_PutHomingTrg( &work->hom );


    //		*******フレア設定
    Har_InitFlare(work);

    //  ******名前保存
    work->name = name;

    {
	short	loop;
	/* ストリームの登録 */
	if ( !GCL_GetOption( 'o' ) )
 	    PERROR( "There is no -o option found in GCL : Newharria\n" ) ;
	for( loop=0 ; loop<HAR_N_STREAM ; loop++ ){
	    work->str_id[loop] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;
	}
    }

    
    //		******配置座標******
    if ( GCL_GetOption( 'p' ) ){	//pos
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->control.mov ) ;
    }else {
	_sceVu0CopyVector(&work->control.mov, &DG_ZeroVector);	//位置の初期化
    }
    _sceVu0CopyVector(&work->control.rot, &DG_ZeroVector);	//方向の初期化

    //デバッグモード
    if ( GCL_GetOption( 'v' ) ){	//view target
	if (GCL_GetNextInt() == 1){
	    work->har_flag |= HAR_DOUSA_FLAG_DEBUG;
	}
    }
    //ボスモード
    if (flag == ON){
	work->har_flag |= HAR_DOUSA_FLAG_BOSSMD;
    }

    //追尾フラグ
    if ( GCL_GetOption( 'f' ) ){	//follow
	if (GCL_GetNextInt() == 1){
	    work->har_flag |= HAR_DOUSA_FLAG_FOLLOW;
	}
    }

    // ****プロック
    work->brake_proc_id = GCL_GetOptionValue( 's', 0 );
    work->die_proc_id = GCL_GetOptionValue( 'd', 0 );
    work->event_proc_id = GCL_GetOptionValue( 'e', 0 );
    work->item_proc_id = GCL_GetOptionValue( 'i', 0 );


    {
	short loop;
	FVECTOR	shift = {0.0f, 0.0f, 2800.0f, 1.0f};
	FVECTOR	size = {700.0f, 700.0f, 3500.0f, 1.0f};

	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    break;
	case ST_LEV_EASY:
	    break;
	case ST_LEV_NORMAL:
	    break;

	case ST_LEV_UPNORM:
	    size.vx = size.vy = 800.0f;
	    size.vz = 3700.0f;
	    shift.vz = 2900.0f;
	    break;

	case GM_LEVEL_HARD:
	    size.vx = size.vy = 900.0f;
	    size.vz = 4000.0f;
	    shift.vz = 3100.0f;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    size.vx = size.vy = 1200.0f;
	    size.vz = 4000.0f;
	    shift.vz = 3500.0f;
	    break;
	}


	/*攻撃ターゲット設定*/
	for(loop=0; loop<2; loop++){
	    GM_SetTarget( &work->noz_of_tgt[loop],
			  TARGET_OFFENSE|TARGET_ROTATE|TARGET_THROUGH|TARGET_POWER,
			  where, PLAYER_SIDE,
			  &size, &shift ) ;
	    
	    GM_SetTargetCallBack( &work->noz_of_tgt[loop],
				  (void*)Burn_CallBack,
				  work);
	}
    }


    
    //最初から操作可能
    //    work->menu_flag |= HAR_DEB_CONTROL_FLAG;

    // *****エフェクとの色類設定
    Har_SetEffectColors(work);
    
    //オブジェ表示
    DG_SetPos2( &work->control.mov, &work->control.rot ) ;
    DG_GetPos( &work->body.objs->world  ) ;
    
    // *****各種パーツの設定
    SetParts(work, where);
    // *****各種エフェクトの設定
    Har_SetupEffect(work);
    // *****ゲージやＨＰ/ターゲットの設定
    Har_SetGageTarget(work);

    //レーダーのイニシャライズ
    HarInitRadarParam(work);    



    // *****びっくりマーク
    work->headmark = 0;
    work->headmark_timer = 999;
    GV_SetActorChild( work,
		NewControl_Headmark2( &work->headmark_pos,
		&work->headmark, &work->target_body_canopy, NULL )) ;


#ifdef DEBUG_MODE    
    {	// *************************カメラ設定
	int nFlag;
	work->camera = NewProgramCamera( name, 0, GM_CAMERA_PROG1, 128 );
	
	// * カメラフラグ設定 */
	nFlag = CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP;
	
	GM_SetCameraType( work->camera, GM_CAM_TYPE_CAMERA_AND_TARGET, nFlag);
	GM_SetCameraAngle( work->camera, 2.0F ) ;
	GM_SetCameraInterpMode( work->camera, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK, 0, 0);
    }
    //    harclearpaddata(&work->act_work); //パッドワークくりあ
#endif

    MACRO_HARRIER_STOP;

    SET_STEP_L1(H_MOD_L1_START);

#ifdef KP_XBOX	// Sound
	{
		// ３Ｄ音バッファテーブルの取得
      BP_SOUND_TODO_BREAK;
#if 0//BP
		work->pSngTrack = HAR_GetSngTrackTable();
		
		// ハンドルの取得
		work->hTarbin = work->pSngTrack[ TR_HARRIER_TARBIN ].handle;
		work->hEngine = work->pSngTrack[ TR_HARRIER_HOBARING ].handle;
		work->hJet = work->pSngTrack[ TR_HARRIER_DOPPLER ].handle;
#endif
	}
#endif
    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarrier						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewHarrier( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Har_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,OFF)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


/*******************************<Global function>********************************/
/*	名前:	void *NewBossHarrier						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewBossHarrier( int name, int where )
{
    Work *work ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Har_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work,name,where,ON)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}

/*******************************<Global function>********************************/
/*	名前:	void Har_PlayAction						*/
/*	引数:	Work *work   						       	*/
/*	説明:	アクションの再生			       			*/
/********************************************************************************/
void Har_PlayAction(Work *work, short num)
{
    HAR_MOVE* actmove;
    actmove = work->act_move;




    
    copy_action_data(&work->act_work, &actmove->p_data[work->play_timer]);
    
    //		開始位置の再生
    if (work->play_timer == 0){
	_sceVu0CopyVector(&(work->control.mov), &(actmove->begin_pos));
	work->control.turn.vx = work->control.rot.vx = actmove->begin_rot.vx;
	work->control.turn.vy = work->control.rot.vy = actmove->begin_rot.vy;
	work->control.turn.vz = work->control.rot.vz = actmove->begin_rot.vz;
	_sceVu0CopyVector(&work->control.step, &DG_ZeroVector);	//位置の初期化
    }
    
    //終了コード
    if (actmove->p_data[work->play_timer].buton == HAR_ACTION_END){
	work->play_timer = HAR_MOVE_FRAMES;
    }
    
    //タイマー
    if (work->play_timer<HAR_MOVE_FRAMES){
	work->play_timer++;
    }
    else {
	MENU_Locate( 400, 50, 0 ) ;
	MENU_SetColor( 200, 0, 0 ) ;
	MENU_Printf( "          - END"); 
	work->play_flag = 0;
	ClearPadData(&work->act_work);
    }
}/*******************************<Global function>********************************/
/*	名前:	void Har_PlayAction						*/
/*	引数:	Work *work   						       	*/
/*	説明:	アクションの再生			       			*/
/********************************************************************************/
void Har_PlayAction2(Work *work, short num)
{
    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){
	MENU_Locate( 400, 50, 0 ) ;
	MENU_SetColor( 200, 0, 0 ) ;
	MENU_Printf( "PLAY"); 
    }
    
    copy_action_data(&work->act_work, &work->har_move.p_data[work->play_timer]);
    
    //		開始位置の再生
    if (work->play_timer == 0){
	_sceVu0CopyVector(&work->control.mov, &work->har_move.begin_pos);
	work->control.turn.vx = work->control.rot.vx = work->har_move.begin_rot.vx;
	work->control.turn.vy = work->control.rot.vy = work->har_move.begin_rot.vy;
	work->control.turn.vz = work->control.rot.vz = work->har_move.begin_rot.vz;
	_sceVu0CopyVector(&work->control.step, &DG_ZeroVector);	//位置の初期化
	
    }
    
    //終了コード
    if (work->har_move.p_data[work->play_timer].buton == HAR_ACTION_END){
	work->play_timer = HAR_MOVE_FRAMES;
    }
    
    //タイマー
    if (work->play_timer<HAR_MOVE_FRAMES){
	work->play_timer++;
    }
    else {
	MENU_Locate( 400, 50, 0 ) ;
	MENU_SetColor( 200, 0, 0 ) ;
	MENU_Printf( "          - END"); 
	work->play_flag = 0;
	ClearPadData(&work->act_work);
    }
}
