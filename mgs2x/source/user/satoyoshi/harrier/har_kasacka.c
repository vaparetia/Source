//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/

#define _MAIN_KASACKA_

#include "harrier.h"
#include "harrier.def"

extern void kas_call_event_proc(Kas_Work*, int);

#include "har_inline.c"

#include "har_snkgrn.c"
#include "har_kastgt.c"
#include "har_snake.c"
#include "har_kasmove.c"
#include "har_kakitem.c"
#include "har_kastart.c"

#ifdef DEBUG_MODE
#include "har_dbg.c"
#endif


#if 0
//カサッカ被攻撃時のオタコン
vc104504	//何てことすんだよ		00
vc104505	//どこ狙ってるんだよ		01
vc104506	//僕らは味方だ			02
vc104507	//ひどいじゃないか		03
vc104508	//この人でなし			04
//アイテム音
vc104518	//弾だ,受けとれ			05
vc104519	//ライデン,受けとれ		06
vc104520	//こいつを使え			07
#endif



/********************************************************************************/
/*	Program									*/
/********************************************************************************/


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void call_event_proc						*/
/*	引数:	Kas_Work	*work						*/
/*	説明:	イベント関連プロック						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void kas_call_event_proc(Kas_Work *work, int num){

    if ( work->event_proc_id != 0 ){
	GCL_ARGS	args;
	int		event_num = num;

	args.argc = 1;
	args.argv = &event_num;
	printf ("						Kas Event proc call %d \n", num);	
	GCL_ExecProc(work->event_proc_id, &args);
    }
    else {
	printf ("						Kas Event proc is NULL!!!!\n", num);	
    }

}


/*******************************<Global function>********************************/
/*	名前:	void har_kassetgage						*/
/*	引数:	Kas_Work	*work						*/
/*	説明:									*/
/********************************************************************************/
void har_kassetgage(Kas_Work *work)
{
    /* ゲージの初期化 */
    GM_GageSet	*gs ;
    int		max, cur ;

    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	cur = 200;
	max = 200;
	break;
    case ST_LEV_EASY:
	cur = 150;
	max = 150;
	break;
    case ST_LEV_NORMAL:
	cur = 100;
	max = 100;
	break;

    case ST_LEV_UPNORM:
	cur = 85;
	max = 85;
	break;

    case GM_LEVEL_HARD:
	cur = 70;
	max = 70;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	cur = 50;
	max = 50;
	break;
    default:
	cur = 100;
	max = 100;
	break;
    }
    cur = cur*2;
    max = max*2;

    gs = &work->gageset ;
    
    GM_InitGageSet( gs, "KASATKA", 16, 190, GM_DEFAULT_GAGE_HEIGHT,
		    cur, max, 0, 30, GM_GAGE_LEVEL_NPC_LIFE) ;
    GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_NPC_LIFE ) ;
    GM_AppendGageSet( gs ) ;
    GM_VisibleGage( gs ) ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Kas_Work *work){
    short	loop;

#ifdef KP_XBOX
	HAR_ReleaseSngTrackTable();
#endif

    GM_FreeObject(&work->body);
    GM_FreeObject(&work->mrot);
    GM_FreeObject(&work->trot);
    GM_FreeObject(&work->m4gl);
    GM_FreeObject(&work->chair);
    GM_FreeObject(&work->zabuton);
    GM_FreeObject(&work->hako);
    GM_FreeObject(&work->snake);

    GM_RemoveGageSet( &work->gageset );

    for (loop=0; loop<SNAKE_GRND_NUM; loop++){
	GM_FreeObject(&work->grn_sel[loop]);
    }
    GM_FreeControl( &work->control);



    for (loop=0; loop<8; loop++){
	GM_FreeTarget( &work->tgt_body[loop]);
    }
    for (loop=0; loop<20; loop++){
	GM_FreeTarget( &work->kak_body[loop]);
    }

    for (loop=0; loop<30; loop++){
	DG_DequeueComdlObjs( work->kak_itembox[loop] );
	DG_FreeComdl( work->kak_itembox[loop] );
    }
}


#define	EFFECT_FLAGS_ON_TMP	(0x08)
#define MT3_PLAYEND		(0x00000001)		/* 最終フレームセット完了フラグ */

#define BASEX 60

#define IS_SEKKIN(_far) ((_far) < 0)
#define SEKKIN(_far) (-1*(_far))
#if 0
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void debug_print						*/
/*	引数:	Work	*work							*/
/*	説明:	デバッグプリント						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void kck_debug_print(Kas_Work *work, int now_kyori){
    FVECTOR	verts[2];

    _sceVu0CopyVector(&verts[0],&work->goal_pos);
    _sceVu0CopyVector(&verts[1],&work->control.mov);

    NewLineView(verts, 1, 120,0,0);

    MENU_SetColor( 10, 250, 10 );
    MENU_Locate  ( 20, BASEX, 0 ) ;
    MENU_Printf  ( "Now  %5.0f, %5.0f, %5.0f",
		   work->control.mov.vx,
		   work->control.mov.vy,
		   work->control.mov.vz);

    MENU_Locate  ( 20, BASEX+17, 0 ) ;
    MENU_Printf  ( "Targ    %d  %d         %d", work->root_num, work->node_num, now_kyori);
    MENU_Locate  ( 20, BASEX+34, 0 ) ;
    MENU_Printf  ( "        %5.0f, %5.0f, %5.0f",
		   work->point[work->root_num][work->node_num].goal_pos.vx,
		   work->point[work->root_num][work->node_num].goal_pos.vy,
		   work->point[work->root_num][work->node_num].goal_pos.vz);

    MENU_Locate  ( 20, BASEX+51, 0 ) ;
    if (work->KAK_MODE & KMODE_ZYUNKAI_STOP){
	MENU_Printf  ( "STOP--          %d",work->mode_timer/TIME_BASE );
    }
    else if (work->KAK_MODE == KMODE_ZYUNKAI_NORM){
	MENU_Printf  ( "NORMAL          %d",work->mode_timer/TIME_BASE );
    }
    else if (work->KAK_MODE == KMODE_ZYUNKAI_AIM){
	MENU_Printf  ( "AIM---          %d",work->mode_timer/TIME_BASE );
    }
    else if (work->KAK_MODE == KMODE_DROP_ITEM){
	MENU_Printf  ( "ITM---          %d",work->mode_timer/TIME_BASE );
    }
    else {
	MENU_Printf  ( "err  %d  ---          %d",
		       work->KAK_MODE,
		       work->mode_timer/TIME_BASE );
    }
}
#endif


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void azituke_move						*/
/*	引数:	Work	*work							*/
/*	説明:	味付けの動き							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void azituke_move(Kas_Work *work){
    SVECTOR	tmprot ={0,0,0,0};
    FVECTOR	tmpvec;
    float	step;
    short	loop;


    for (loop=0; loop<3; loop++){
	work->azi_r_time[loop] += TIME_BASE;
	if (work->azi_r_time[loop] > work->azi_r_time_end[loop]){
	    work->azi_r_time[loop] = 0;
	    work->azi_r_time_end[loop] = (RAND(50)+100)*TIME_BASE;
	    work->azi_rot_old[loop] = work->azi_rot_now[loop];
	    work->azi_rot_now[loop] = RAND(171)-85;
	}
	
	step = (1.0f-sinf( (M_PI*work->azi_r_time[loop]/work->azi_r_time_end[loop])-M_PI/2.0f))/2.0f;
	if (loop == 0){
	    tmprot.vx = work->azi_rot_now[loop]*(1.0-step) + work->azi_rot_old[loop]*step;
	}
	if (loop == 1){
	    tmprot.vy = work->azi_rot_now[loop]*(1.0-step) + work->azi_rot_old[loop]*step;
	}
	if (loop == 2){
	    tmprot.vz = work->azi_rot_now[loop]*(1.0-step) + work->azi_rot_old[loop]*step;
	}
    }

    for (loop=0; loop<3; loop++){
	work->azi_m_time[loop] += TIME_BASE;
	if (work->azi_m_time[loop] > work->azi_m_time_end[loop]){
	    work->azi_m_time[loop] = 0;
	    work->azi_m_time_end[loop] = (RAND(50)+130)*TIME_BASE;
	    work->azi_mov_old[loop] = work->azi_mov_now[loop];

	    if (loop == 1){
		//		work->azi_mov_now[loop] = (RAND(59)-29)*40;
		work->azi_mov_now[loop] = RAND(59)*40;
	    }
	    else {
		work->azi_mov_now[loop] = (RAND(41)-20)*40;
	    }
	}
	
	step = (1.0f-sinf( (M_PI*work->azi_m_time[loop]/work->azi_m_time_end[loop])-M_PI/2.0f))/2.0f;
	if (loop == 0){
	    tmpvec.vx = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
	if (loop == 1){
	    tmpvec.vy = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
	if (loop == 2){
	    tmpvec.vz = work->azi_mov_now[loop]*(1.0f-step) + work->azi_mov_old[loop]*step;
	}
    }

#if 0
    int			azi_r_time[3];
    int			azi_m_time[3];
    char		azi_rot_now[3];
    char		azi_mov_now[3];
#endif

    DG_RotatePos( &tmprot );
    DG_MovePos( &tmpvec );
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Kak_ActPlayStream						*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ ストリーム再生管理					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void Kak_ActPlayStream( Kas_Work *work ){
    int  vol, pan ;
    float bp_angle;

    if ( !(har_gameover_check_ok()) ){
	return;
    }


    if (work->str_hdl_snk){	//スネーク
	GM_SeGetVolPanFromVolCurves(&work->control.mov, &vol, &pan, &kac_se_curves, &bp_angle);
	//	GM_VoxStreamSetPan( work->str_hdl_snk, vol, pan ) ;
//	GM_VoxStreamSetPan( work->str_hdl_snk, 0x3f, 0x20 ) ;
	GM_VoxStreamSetParam( work->str_hdl_snk, &work->control.mov, GM_INVALID_ADDR, 0x3f, 0x20, 0.f ) ;
	/* 再生が終っていれば ハンドラを使っていない状態に戻す*/
	if ( GM_StreamStatus( work->str_hdl_snk ) == GM_STREAM_STATE_END ){
	    work->str_hdl_snk = 0;
	}
    }

    if (work->str_hdl_ota){	//オタコン
	GM_SeGetVolPanFromVolCurves(&work->control.mov, &vol, &pan, &kac_se_curves, &bp_angle);
//	GM_VoxStreamSetPan( work->str_hdl_ota, vol, pan ) ;
	GM_VoxStreamSetParam( work->str_hdl_ota, &work->control.mov, GM_INVALID_ADDR, vol, pan, bp_angle ) ;
	/* 再生が終っていれば ハンドラを使っていない状態に戻す*/
	if ( GM_StreamStatus( work->str_hdl_ota ) == GM_STREAM_STATE_END ){
	    work->str_hdl_ota = 0;
	}
    }

    if (work->ot_speek_time > 0){
	work->ot_speek_time -= TIME_BASE;
	if (work->ot_speek_time <= 0){
	    work->ot_speek_time  = 0;

#if 0		//オタコン苦情
vc104504	//何てことすんだよ		00
vc104505	//どこ狙ってるんだよ		01
vc104506	//僕らは味方だ			02
vc104507	//ひどいじゃないか		03
vc104508	//この人でなし			04
#endif
		if (work->str_hdl_ota == 0){
//		    work->str_hdl_ota = GM_VoxStream(work->str_id[RAND(5)], 0);
		    work->str_hdl_ota = GM_VoxStream(work->str_id[RAND(5)], GM_STREAM_FLAG_3D);
		}
	}
    }
}


VOLUMECURVE kac_se_def = {
    2,
    SE_COS_60,
    SE_COS_90,
    {30*KTAR_DIS, 200*KTAR_DIS, -1, -1},
    {100*TAR_VOL, 100*TAR_VOL, 35*TAR_VOL, 20*TAR_VOL, 20*TAR_VOL}, 
    {24*KTAR_DIS, 180*KTAR_DIS, -1, -1},
    { 85*TAR_VOL,  85*TAR_VOL, 28*TAR_VOL, 10*TAR_VOL, 10*TAR_VOL}, 
    1.0f
};

VOLCURVES kac_se_curves = {
    &kac_se_def,
    &kac_se_def,
    &kac_se_def,
    &kac_se_def,
};


static VOLUMECURVE ktarbin_ownview = {
    2,
    SE_COS_60,
    SE_COS_90,
    {10*KTAR_DIS, 40*KTAR_DIS, -1, -1},
    {100*TAR_VOL, 100*TAR_VOL, 0, 0, 0}, 
    {8*KTAR_DIS, 30*KTAR_DIS, -1, -1},
    {85*TAR_VOL, 85*TAR_VOL, 0, 0, 0}, 
    1.0f
};

#if 0
static VOLUMECURVE kwing_ownview = {
    3,
    SE_COS_60,
    SE_COS_90,
    {10*KTAR_DIS, 25*KTAR_DIS, 41*KTAR_DIS, -1},
    {100*TAR_VOL, 100*TAR_VOL, 31*TAR_VOL, 0, 0}, 
    {10*KTAR_DIS, 25*KTAR_DIS, 41*KTAR_DIS, -1},
    {85*TAR_VOL, 85*TAR_VOL, 28*TAR_VOL, 0, 0}, 
    1.0f
};
#endif

static VOLUMECURVE kwing_ownview = {
    3,
    SE_COS_60,
    SE_COS_90,
    {10*KTAR_DIS, 30*KTAR_DIS, 50*KTAR_DIS, -1},
    {100*TAR_VOL, 100*TAR_VOL,   31*TAR_VOL, 0, 0}, 
    {10*KTAR_DIS, 30*KTAR_DIS, 50*KTAR_DIS, -1},
    {90*TAR_VOL,   90*TAR_VOL,   28*TAR_VOL, 0, 0}, 
    1.0f
};

static VOLCURVES kak_tarbin_curves = {
    &ktarbin_ownview,
    &ktarbin_ownview,
    &ktarbin_ownview,
    &ktarbin_ownview
};

static VOLCURVES kak_wing_curves = {
    &kwing_ownview,
    &kwing_ownview,
    &kwing_ownview,
    &kwing_ownview
};

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void sound_control						*/
/*	引数:	Work	*work							*/
/*	説明:	サウンドコントロール						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef PSX2
static void KacarkaBGMFader( Kas_Work *work )
{
    FVECTOR	shift;
    int		distance;
    int		volume = 0x2f;
    int		pan = 0x20;
    float   bp_angle = 0.f;

    //	距離の計算
    SAT_Minus_FVECTOR(&shift, &GM_PlayerFindPos, &work->control.mov);
    distance = (int)_FVecLen3( &shift);

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &kak_tarbin_curves, &bp_angle);
    GM_MixConvFrequencyFader(KACATKA_TARBIN, 0x40, pan, volume);

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &kak_wing_curves, &bp_angle);
    GM_MixConvFrequencyFader(KACATKA_WING, 0x40, pan, volume);

}
#else	// PSX2
static void KacarkaBGMFader( Kas_Work *work )
{
    int		volume = 0x2f;
    int		pan = 0x20;

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &kak_tarbin_curves);
    BP_SOUND_TODO_BREAK;
#if 0 //BP
	sd_3dsrc_setvol( work->hTarbin , GM_ConvertVol( volume ) );
	sd_3dsrc_setpos( work->hTarbin , &work->control.mov );

    GM_SeGetVolPanFromVolCurves(&work->control.mov, &volume, &pan, &kak_wing_curves);
	sd_3dsrc_setvol( work->hRoter , GM_ConvertVol( volume ) );
	sd_3dsrc_setpos( work->hRoter , &work->control.mov );
#endif
}
#endif	// PSX2




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void kac_zahyou							*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ の座標計算						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void kac_zahyou(Kas_Work *work){

    FMATRIX	tempmat;
    FVECTOR	tempvec = {0.0f, 0.0f, -3000.0f, 1.0f};

    DG_SetPos(&work->snake.objs->world);
    DG_MovePos(&tempvec);
    DG_GetPos(&tempmat);

    work->snk_back = *(FVECTOR*)tempmat.m[3];

    tempvec.vz = 0.0f;
    tempvec.vy = 3000.0f;

    DG_SetPos(&work->snake.objs->world);
    DG_MovePos(&tempvec);
    DG_GetPos(&tempmat);

    work->snk_ue = *(FVECTOR*)tempmat.m[3];

#if 0
    {
	FVECTOR verts[2];
	FVECTOR verts1[2];

	verts[0] = verts1[0] = *(FVECTOR*)work->snake.objs->world.m[3];
	verts[1] = work->snk_back;
	verts1[1] = work->snk_ue;
	NewLineView (verts, 1, 255, 0, 0);
	NewLineView (verts1, 1, 0, 255, 0);
    }
#endif


}







/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Do_Kas_Act							*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ アクション						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Do_Kas_Act(Kas_Work *work){

#ifdef DEBUG_MODE
    Kas_DbgDispSnkStat( work );
#endif


    if (work->makasero_count > 0){
	work->makasero_count -= TIME_BASE;
    }

    Kak_ActPlayStream(work);


    KacarkaBGMFader(work);

    GM_ActControl(&work->control) ;


    if (work->start_move_flg){
	move_kasaka(work);	// 	新移動
    }
    else {			// 	最初の移動
	if (control_kas_start(work)==ON){
	    work->start_move_flg = ON;
	}
    }

    snake_act(work);
    fly_grn(work);		//グレネード飛ぶ！

    // **********ダメージ処理 
    if (work->damage != 0){
	work->gageset.value = work->gageset.value - work->damage;
	work->damage = 0;
	work->damage_act_time = 150*5;

	if (work->gageset.value < 0){
	    work->gageset.value = 0;

	    if ( work->die_proc_id != 0 ){
		GCL_ARGS	args;
		args.argc = 1;
		args.argv = &work->name;
		GCL_ExecProc(work->die_proc_id, &args);
		printf ("	Kas Die proc Call. ID = %d\n", work->name);
	    }

	    // メカKillカウント
	    GM_MecaKillCount ++;
	    if ( GM_MecaKillCount > GM_MAX_RESULT_COUNT ) GM_MecaKillCount = GM_MAX_RESULT_COUNT ;
	    // Killカウント
	    GM_KillCount = GM_KillCount+2 ;
	    if ( GM_KillCount > GM_MAX_RESULT_COUNT ) GM_KillCount = GM_MAX_RESULT_COUNT ;

	    GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_OTAOUT11);
	    GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_SNAOUT11);
	    GM_GameOverProcStart(NULL);
	    GM_GameOverProcEnd(NULL);
	} 
    }
    if (work->snk_mtk_time > 0){	// ****スネークの無敵時間
	work->snk_mtk_time -= TIME_BASE;
	if (work->snk_mtk_time < 0){
	    work->snk_mtk_time = 0;
	}
    }

    if (work->damage_act_time > 0){
	work->damage_act_time -= TIME_BASE;
	if (work->damage_act_time < 0){
	    work->damage_act_time = 0;
	}
    }

    // *********カサッカの位置
    {
	DG_SetPos2( &work->control.mov, &work->control.rot);
	DG_GetPos( &work->body.objs->world ) ;

	// ****味付けの動き
#ifdef DEBUG_MODE
	if (HAR_ItemDbg==OFF)
#endif
	{
	    azituke_move(work);
	}


	DG_GetPos( &work->body.objs->objs[0].world ) ;

	DG_MovePos( &work->body.objs->objs[1].trans ) ;
	DG_RotatePos( &work->rots[0] );
	DG_GetPos( &work->body.objs->objs[1].world ) ;

	DG_SetPos( &work->body.objs->objs[0].world ) ;
	DG_MovePos( &work->body.objs->objs[2].trans ) ;
	DG_RotatePos( &work->rots[1] );
	DG_GetPos( &work->body.objs->objs[2].world ) ;

	DG_SetPos( &work->body.objs->objs[0].world );
	DG_MovePos( &work->body.objs->objs[3].trans );
	DG_GetPos( &work->body.objs->objs[3].world );

	work->rots[0].vy += 660;
	work->rots[1].vx += 722;

	work->rots[0].vy %= 4096;
	work->rots[1].vx %= 4096;

	DG_SetPos( &work->body.objs->objs[1].world  ) ;
	GM_ActObject(&work->mrot);

	DG_SetPos( &work->body.objs->objs[2].world  ) ;
	GM_ActObject(&work->trot);

    }

    // ********椅子やザブトンの位置
    {
	FVECTOR	shift = {0.0f, -1993.0f, 800.0f, 1.0f};
	DG_SetPos( &work->body.objs->objs[0].world  ) ;
	DG_GetPos( &work->zabuton.objs->world ) ;
	DG_GetPos( &work->zabuton.objs->objs[0].world ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->chair.objs->world ) ;
	DG_GetPos( &work->chair.objs->objs[0].world ) ;

    }

    // ********スネークの位置
    {
	FVECTOR	shift = {600.0f, -1570.0f, 300.0f, 1.0f};
	SVECTOR	rot = {0, 1024, 0, 0};	
	DG_SetPos( &work->body.objs->objs[0].world  ) ;
	DG_MovePos( &shift );
	DG_RotatePos ( &rot );
    }

    // ********スネークモーションの設定
    GM_ActMotion (&work->snake);
    GM_ActObject2 (&work->snake);



    {
	FVECTOR tmpvec;
	FVECTOR tmpvec2;
	float temp;
	int dist;
	Work* har_work = (Work*)GET_HAR_WORK;	
	
	_sceVu0SubVector(&tmpvec,
			 (FVECTOR*)har_work->body.objs->objs[0].world.m[3],
			 (FVECTOR*)work->snake.objs->objs[0].world.m[3]);

	dist = _FVecLen3(&tmpvec);

	_sceVu0Normalize(&tmpvec, &tmpvec);
	tmpvec2 = *(FVECTOR*)work->body.objs->objs[0].world.m[0];
	_sceVu0Normalize(&tmpvec2, &tmpvec2);
	temp = _sceVu0InnerProduct(&tmpvec, &tmpvec2);

	
	//	printf ("%f   %d %d \n", temp, dist, work->shoottime);





	if (work->snake_sizume_time > 0){	// *****プレイヤを撃つ
	    work->shoottime = 400*5;
	    if (work->snake_ikari!=0){
		work->aimtarget = GM_PlayerFindPos;	// ***プレイヤを狙ってみたり
		if (work->snake_ikari < 4 ){
		    if ((work->set_snake == SET_SNK_ENABLE)&&(work->m4time==0)){
			work->set_snake = SET_SNK_M4FIRE;
			work->m4time = 60*5;
		    }
		    work->homing_p = &work->aimtarget;
		}
		else {
		    if (work->set_snake == SET_SNK_ENABLE){
			work->set_snake = SET_SNK_GLFIRE;

			//気合い 現行鳴らすべき気合いはない
			//GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_H_SWINGL);

		    }
		    get_grn_aim(work);	//グレネードの狙いを定める
		}
	    }
	    else {
		work->snake_sizume_time -= TIME_BASE;
	    }

	    // ********カサッカを撃つために止まる
	    if (work->ks_speed > 0.02f){
		work->ks_speed -= 0.005f;
	    }
	}
	else {
	    //	    static int IF_SHT_HAR = OFF;
	    //	    int IF_SHT_HAR = ON;
	    int IF_SHT_HAR = ON;
	    {				//  とどめは指さない
		Work	*hwork;
		if ( GET_HAR_WORK != NULL ){
		    hwork = (Work*)GET_HAR_WORK;
		    if (hwork->gageset.value <= 175){
			IF_SHT_HAR = OFF;
		    }
		}
	    }
	    {
		int shoot_wait;	// **撃ち待ち

		switch( GM_GameLevel ){
		case GM_LEVEL_VERYEASY:
		    shoot_wait = 170*5;
		    break;
		case ST_LEV_EASY:
		    shoot_wait = 350*5;
		    break;
		case ST_LEV_NORMAL:
		    shoot_wait = 750*5;
		    break;

		case ST_LEV_UPNORM:
		    shoot_wait = 875*5;
		    break;

		case GM_LEVEL_HARD:
		    shoot_wait = 1000*5;
		    break;
		case GM_LEVEL_E_EXTREME:
		case GM_LEVEL_EXTREME:
		    shoot_wait = 20000*5;
		    break;
		default:
		    shoot_wait = 500*5;
		    break;
		}
		
		// *******スネークの向き設定
		if (temp < 0.6f){				// 前方にハリアがいなければ
		    work->shoottime = shoot_wait;		// 撃たない
		    work->aimtarget = GM_PlayerFindPos;	// ***プレイヤを狙ってみたり
		}
		else {
		    work->aimtarget = *(FVECTOR*)(((Work*)GET_HAR_WORK)->body.objs->world.m[3]);
		}
		if ((dist < 100000)&&(IF_SHT_HAR == ON)){




		    // ==============================================================
		    if (work->shoottime < 200){
			if (work->str_hdl_snk==0){

			    if (work->makasero_count <= 0){

			    if (work->snk_atk_voice_num == 0){
//				work->str_hdl_snk = GM_VoxStream(work->str_id[8], 0);//ライデン,任せろ
				work->str_hdl_snk = GM_VoxStream(work->str_id[8], GM_STREAM_FLAG_3D);//ライデン,任せろ
				work->snk_atk_voice_num = 1;
			    }
			    else {
				work->str_hdl_snk = GM_VoxStream(work->str_id[9], GM_STREAM_FLAG_3D);//任せろ
			    }
			    work->makasero_count = 15*60*5;

			    }
			}
		    }


		    if (work->shoottime < 0){
			work->shoottime = shoot_wait;
			work->set_snake = SET_SNK_GLFIRE;
		    }
		    get_grn_aim(work);	//グレネードの狙いを定める
		    work->shoottime -= TIME_BASE;
		}
		else {
		    work->shoottime = shoot_wait;
		    work->homing_p = &work->aimtarget;
		}

	    }

	    {	    // *******スピード増減
		float	target_speed;

		target_speed = sinf((work->timer%(1200*5)*M_PI*2/(1200*5)))*0.8f+1.0f;
		
		if (work->ks_speed < target_speed){
		    work->ks_speed += 0.01f;
		}
		if (work->ks_speed > target_speed){
		    work->ks_speed -= 0.01f;
		}
	    }
	}
    }
    work->timer += TIME_BASE;

#if 0
    {
	FVECTOR verts[2];
	verts[0] = *(FVECTOR*)work->body.objs->objs[0].world.m[3],
	verts[1] = work->aimtarget;
	NewLineView(verts, 1, 250, 10, 10);
    }
#endif


    move_itembox(work); //アイテムボックスの追従
    
    if ( (work->snake_motion != SET_SNK_ITEM ) &&
	 (work->snake_motion != SET_SNK_DAM ) ){
	CalcAdjustLR(work,1);
	CalcAdjustUD(work,1);
    }
    else {
	CalcAdjustLR(work,0);
	CalcAdjustUD(work,0);
    }


    drop_cnt_itembox2(work);



    // ********Ｍ４の位置
    DG_SetPos( &work->snake.objs->objs[HUMAN21_MIGI_TE].world  ) ;
    GM_ActObject(&work->m4gl);


    move_Kas_target(work);


    // ********ライティングの設定
    DG_GetLightMatrixFix ( &work->control.mov, work->lights );    
    DG_GetLightMatrix( (FVECTOR*)work->snake.objs->world.m[3], work->snk_lights );    

    kac_zahyou(work);	// 全て終った後に座標計算

}

extern void *NewEvmEquip(CONTROL *ctrl, OBJECT *body,
			 int model, int mirror, int unit);

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HarInitControlParam					*/
/*	引数:	Work	*work							*/
/*	説明:	オブジェクトのイニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void KasInitObjectlParam(Kas_Work *work){
    short	loop;

    //カサッカ
    GM_InitObject(&work->body,GV_StrCode( "kck_plant_mt" ),
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);

    GM_InitObject(&work->mrot,GV_StrCode( "kck_plant_mrot" ),
		  DG_FLAG_SHADE);
		  //		  DG_FLAG_SHADE|DG_FLAG_FOGPARAM);

    GM_InitObject(&work->trot,GV_StrCode( "kck_plant_trot" ),
		  DG_FLAG_SHADE|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);

    //    DG_SetFogParamObjs (work->body.objs, FOG_NEAR, FOG_FAR);
    //    DG_SetFogParamObjs (work->mrot.objs, FOG_NEAR, FOG_FAR);
    //    DG_SetFogParamObjs (work->trot.objs, FOG_NEAR, FOG_FAR);


    //椅子
    GM_InitObject(&work->chair,GV_StrCode( "kck_chair" ),	
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);
    GM_InitObject(&work->zabuton,GV_StrCode( "kck_cushion_iro" ),	
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);
    GM_InitObject(&work->hako,GV_StrCode( "kck_plant_trot" ),
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_INVISIBLE);


    //    DG_SetFogParamObjs (work->chair.objs, FOG_NEAR, FOG_FAR);
    //    DG_SetFogParamObjs (work->zabuton.objs, FOG_NEAR, FOG_FAR);


    //モーション設定
    GM_ConfigObjectMotion(&work->hako, 0, GV_StrCode("snknglhako"), MT_FLAG_HUMAN2);
    GM_ConfigObjectStep(&work->hako, &work->hako_step);
    //    GM_ConfigObjectAction(&work->hako, 0, 1, 		  0, 0xfffff, 0);



    //スネーク
    GM_InitObject(&work->snake,GV_StrCode( "iro_def_mt" ),
		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);
		  //    DG_SetFogParamObjs (work->snake.objs, FOG_NEAR, FOG_FAR);



    if (GM_ClearFlag & GM_CLEARED_SUNGLASSES_PLAYING){
	GV_SetActorChild(work, NewEvmEquip(NULL, &work->snake,
			 MDL_PLISKIN_GLASSES_BLACK, 0, 
			 HUMAN21_ATAMA) );
    }


    //Ｍ４
    GM_InitObject(&work->m4gl,GV_StrCode( "m4b_gl" ),
		  DG_FLAG_SHADE|DG_FLAG_IRREACTION);
		  //		  DG_FLAG_SHADE|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);
    //    DG_SetFogParamObjs (work->m4gl.objs, FOG_NEAR, FOG_FAR);

    //グレネード弾
    for (loop=0; loop<SNAKE_GRND_NUM; loop++){
	GM_InitObject(&work->grn_sel[loop],GV_StrCode( "demo_m4_grn_bul" ),	//グレネード弾
		      DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_INVISIBLE|DG_FLAG_FOGPARAM);
	//		      		      DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION);
	DG_SetFogParamObjs (work->grn_sel[loop].objs, FOG_NEAR, FOG_FAR);
    }

    //モーション設定
    GM_ConfigObjectMotion(&work->snake, 0, GV_StrCode("snkinkak"), MT_FLAG_HUMAN2);
    GM_ConfigObjectAction(&work->snake, 0, work->mode_snake, 
			  0, 0xfffff, 0);
    
    //ライトマトリクスの設定  毎フレーム lightmatrixを設定すれば自動反映
    GM_ConfigObjectLight(&(work->body),work->lights);
    GM_ConfigObjectLight(&(work->snake),work->snk_lights);
    GM_ConfigObjectLight(&(work->m4gl),work->snk_lights);
    GM_ConfigObjectLight(&(work->chair),work->lights);
    
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void HarInitControlParam					*/
/*	引数:	Work	*work							*/
/*	説明:	コントロールのイニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void KasInitControlParam(Kas_Work *work, int name, int where){

    GM_InitControl( &work->control, name, where );	//コントロールシステムに登録
    GM_ConfigControlMessageCheck( &work->control );	//メッセージ受信処理を行なう 	
    GM_ConfigControlMapCheck( &work->control );		//マップ変更チェックを行なう

    GM_ConfigControlNoSegmentArea( &work->control );	//当たりチェックしないからハザード格納領域はいらない

    work->control.skip_flag =
	(CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK
	 |CTRL_SKIP_GET_ADDRESS
	 |CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_ONLINE_CHECK
	 |CTRL_SKIP_TRAP
	 );

    GM_ConfigControlObject( &work->control, &work->body);//controlにbodyを連結
    GM_ConfigControlMapID( &work->control ) ;

}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InitRadarParam						*/
/*	引数:	Work	*work							*/
/*	説明:	レーダーシステムのイニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void kacInitRadarParam(Kas_Work *work, int where){
    RADAR_CTRL *rctrl ;

    GM_InitRadarControl(&work->rctrl,&work->control.mov,
			RADAR_VISIBLE|RADAR_NOFIX_SIGHT, where ) ;
    //RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT, where ) ;
    rctrl = &work->rctrl;
    rctrl->angle = 0;
    rctrl->col = RADAR_COLOR_BLUE ;	/* 視野描画色 */
    rctrl->range = 0;
    //    GM_RadarSetVRange( rctrl, 0 , 0 );
}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResourcesKas						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	カサッカワークの確保＆イニシャライズ				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResourcesKas(Kas_Work *work, int name, int where){
    int		buf[3];

#ifdef DEBUG_MODE
    Har_InitDbgConfiguration();
#endif
    {	//乱数を回す
	int tmp = RAND(1);
	tmp = 0;
    }

    GET_KAK_WORK = work;	// ポインタの登録

    //		*****オブジェクトのイニシャライズ*****
    KasInitObjectlParam(work);

    //		********コントロールのイニシャライズ********
    KasInitControlParam(work, name, where);
    kacInitRadarParam(work, where);

    //  ******名前保存
    work->name = name;
    har_kassetgage(work);

    work->item_reserve_flag = 0; //アイテム投下命令受けとり準備完了

    {
	short	loop;
	/* ストリームの登録 */
	if ( !GCL_GetOption( 'o' ) )
	    PERROR( "There is no -o option found in GCL : Newharria\n" ) ;
	for( loop=0 ; loop<KAC_N_STREAM ; loop++ ){
	    work->str_id[loop] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;
	}
    }

    work->event_proc_id = GCL_GetOptionValue( 'e', 0 );
    work->die_proc_id = GCL_GetOptionValue( 'd', 0 );

    //		******配置座標******
    if ( GCL_GetOption( 'p' ) ){	//pos
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->control.mov ) ;
    }else {
	_sceVu0CopyVector(&work->control.mov, &DG_ZeroVector);	//位置の初期化
    }
    _sceVu0CopyVector(&work->control.rot, &DG_ZeroVector);	//方向の初期化

    kak_settarget(work);	//ターゲット

    //オブジェ表示
    DG_SetPos2( &work->control.mov, &work->control.rot ) ;
    DG_GetPos( &work->body.objs->world  ) ;

    //アイテムボックスのイニシャライズ
    init_itembox(work, where);

#ifdef KP_XBOX	// Sound
	{
		// ３Ｄ音バッファテーブルの取得
      BP_SOUND_TODO_BREAK;
#if 0//BP
		work->pSngTrack = HAR_GetSngTrackTable();
		
		// ハンドルの取得
		work->hRoter = work->pSngTrack[ TR_KASACKA_WING ].handle;
		work->hTarbin = work->pSngTrack[ TR_KASACKA_TARBIN ].handle;
#endif
	}
#endif
    return 1;
}


/*******************************<Global function>********************************/
/*	名前:	void *NewKasacka						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewHarKasacka( int name, int where, void *oya_work )
{
    Kas_Work *work ;
    work = (Kas_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Kas_Work ), 0 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Do_Kas_Act,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResourcesKas( work,name,where)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}



