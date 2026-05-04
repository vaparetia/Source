//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    title_trial.c
    タイトル
	2000/08/28 T.Shibata

	$Id: title_trial.c,v 1.1.1.3 2002/11/19 11:48:45 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"
#include	"rankenc.h"
#include 	"font.h"
#include	"jimaku.h"

#include "mode/menu/xtextscn.h"

#define	ACTOR_PRIO		(254)

#define		START_Y		(218.0f + 8.0f)
#define		OPTION_Y	(238.0f + 8.0f)
#define		MLOG_Y		(258.0f + 8.0f)
#define		SPECIAL_Y	(278.0f + 8.0f)

//-------------------------------------------------
//		extern
extern void *NewGameOption_Trial( int mode );
//extern void *NewDebugCamera( int chanl );
//extern void *NewFarFocusEffect( int name, int max_plane, int near, int far );
extern void *NewRainCamera_Demo_AddesMessageName( int name, int life );
extern void *NewThunder_Demo( float sx, float sy );

/* 説明分テキストを初期化する */
extern void MENU_ClearTextTexture( void *work );
/* 説明文を表示する */
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2,
								int u1, int v1, int u2, int v2, int col );
/* テキストテクスチャ管理デーモン */
extern void *NewTextScreenControl( void );


//#define TTL_FLAGS_SLEEP	(0x02)
//#define TTL_FLAGS_PRESLEEP	(0x01)
	
#define TTL_FLAGS_ACTIVE	(0x01)
#define TTL_FLAGS_CAMSTOP	(0x02)
#define TTL_FLAGS_RAIN		(0x04)
#define TTL_FLAGS_CHANGECAM	(0x08)

#define TTL_FLAGS_ENGLISH	(0x10)
#define TTL_FLAGS_ENG_PAL	(0x20)

#define TTL_WAIT_TO_OPTN	(0)

#define TEX_CODE_STR		(9167091)
#define TEX_CODE_KONAMI		(7545956)
#define TEX_CODE_KONAMI_ENG	(9543198)

#define TEX_CODE_JPN		(12886304)
#define TEX_CODE_WARNING		(5820592)
#define TEX_CODE_WARNING_ENG	(4838430)

#define TEX_CODE_LOGO		(10842417)
#define TEX_CODE_SOL		(6247691)
#define TEX_CODE_TRIAL		(203059)
#define TEX_CODE_TRIAL_ENG	(12425761)
#define TEX_CODE_KCEJ		(233616)

#define TEX_CODE_MLOG_BG	(12222804)
#define TEX_CODE_MLOG_TITLE	(11330107)
#define TEX_CODE_MLOG_LOG1	(1940408)
#define TEX_CODE_MLOG_LOG2	(5556552)
#define TEX_CODE_MLOG_LOG3	(5622088)
#define TEX_CODE_MLOG_LOG4	(14654391)
#define TEX_CODE_MLOG_TRI	(3862202)
#define TEX_CODE_MLOG_FIG	(10486004)
#define TEX_CODE_MLOG_MFIG	(14348664)

//#define TEX_CODE_STR		(GV_StrCode("menu_fnt_alp_ovl"))
//#define TEX_CODE_KONAMI	(GV_StrCode("logo_konami"))
//#define TEX_CODE_JPN		(GV_StrCode("logo_japan"))
//#define TEX_CODE_WARNING	(GV_StrCode("warning"))

//#define TEX_CODE_LOGO		(GV_StrCode("title_mgs2_alp_ovl"))
//#define TEX_CODE_SOL		(GV_StrCode("title_sol_alp_ovl"))
//#define TEX_CODE_TRIAL	(GV_StrCode("title_hatsutaiken_alp_ovl"))
//#define TEX_CODE_KCEJ		(GV_StrCode("mgs2_pd_allrights_alp_ovl"))

#define	FAR_FORCAS	1
#define MOVE_TYPE	1

#if MOVE_TYPE
#define MOVE_TIME		(728)
#define MOVE_T			(1.0f/729.0f)

#define MOVE_T2			(1.0f/1330.0f)
#else
#define MOVE_TIME		(600)
#define MOVE_T			(1.0f/600.0f)
#define MOVE_T2			(1.0f/1523.0f)
#endif


//#define	FORCAS_NAME		(GV_StrCode("far_blur"))
//#define CAMRAIN_NAME	(GV_StrCode("cam_rain"))

#define	FORCAS_NAME		(2492476)
#define CAMRAIN_NAME	(2998958)

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))
#define SET_COLOR_2DPRIM3(_prim,_a,_col) SET_COLOR_2DPRIM((_prim),(((_a)<<24) | (_col)))

enum {
	TTL_MENU_START = 0,
	TTL_MENU_OPTION,
	TTL_MENU_MLOG,
	TTL_MENU_SPECIAL,

	TTL_MENU_VEASY = 0,
	TTL_MENU_EASY,
	TTL_MENU_NORMAL,
	TTL_MENU_HARD,
	TTL_MENU_VHARD,

};

enum {

	TTL_STEP_CAUTION = -1,		//注意
	TTL_STEP_TOUT = 0,			//タイムアウト処理
	TTL_STEP_KONAMI,			//コナミロゴ						pad -> TTL_STEP_WAIT0
	TTL_STEP_KCEJ,				//ＫＣＥＪロゴ						pad -> TTL_STEP_WAIT0
	TTL_STEP_TITLE,				//タイトルフェードイン カメラ移動	pad -> TTL_STEP_WAIT0

	TTL_STEP_PRE0,				//文字フェードイン
	TTL_STEP_WAIT0,				//選択待ち	<<-->>	オプション
	TTL_STEP_AFTER0,			//文字フェードアウト

	TTL_STEP_PRE1,				//文字フェードイン
	TTL_STEP_WAIT1,				//選択待ち	難易度
	TTL_STEP_AFTER1,			//文字フェードアウト

	TTL_STEP_PRE2,				//文字フェードイン
	TTL_STEP_WAIT2,				//選択待ち  レーダー
	TTL_STEP_AFTER2,			//文字フェードアウト

	TTL_STEP_ZOOM,				//橋ズーム スタート（雨起動）
	TTL_STEP_PREKILL,			//橋ズーム スタート フェードアウト
	TTL_STEP_KILL,				//終了処理	proc呼び出し

	//オプションへ
	TTL_STEP_PRESLEEP,			//オプション前処理
	TTL_STEP_SLEEP,				//オプション中
	TTL_STEP_WAKEUP,			//オプション終了処理

	//ミッションログ
	TTL_STEP_PREMLOG,
	TTL_STEP_MLOG_NEXT,
	TTL_STEP_MLOG_BACK,
	TTL_STEP_MLOG_WAIT,
	TTL_STEP_AFTMLOG,

	//スペシャルへ
	TTL_STEP_SPECIAL0,
	TTL_STEP_SPECIAL1,

	//スペシャルから
	TTL_STEP_FROM_SP,
};

static FVECTOR TrtPosList[] ={
    { 34800.0f, 164800.0f, 32300.0f, 0.0f },
	{ -302200.0f, 105800.0f, 53000.0f, 0.0f },
};

static FVECTOR TrtVecList[] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f },
//	{ -0.983322f, 0.172136f, 0.060394f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
};

static FVECTOR CmrPosList[] =
{
	{ 123000.0f, 35000.0f, 30600.0f, 0.0f },
	{ 141400.0f, 35000.0f, -10400.0f, 0.0f },
	{ 143400.0f, 35000.0f, -15000.0f, 0.0f },
};

static FVECTOR CmrVecList[] =
{
	{ 0.0f, 0.0f, 0.0f, 0.0f },
	{ 0.409439f, 0.0f, -0.912337f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 0.0f },
};

#define	SELECT_COLOR_R	(160)
#define	SELECT_COLOR_G	( 40)
#define	SELECT_COLOR_B	( 30)

#define	TRIAL_COLOR_ENG	(0x00303090)
#define	TRIAL_COLOR_JPN	(0x00808080)

#define	NOT_SELECT_COLOR_R	(64)
#define	NOT_SELECT_COLOR_G	(90)
#define	NOT_SELECT_COLOR_B	(80)

typedef	struct {
	GV_ACT_EX	actor;

	FVECTOR		fvec;

	int			name;
	int			where;
	int			count;
	int			which;

	int			flags;
	int			end_proc_id;
	int			rain_proc_id;
	int			stop_proc_id;
	int			special_proc_id;

	int			step;
	int			timer;
	int			u_off;
	int			v_off;

	int			rot_x;
	int			rot_y;
	SVECTOR		init_rot;

	GM_CameraSet	*camera;
	GM_CameraSet	*rot_cam;
	void			*son;
	void			*mlog_work;
	void			*cl_code_work;

	SPR_OBJ   		*title_logo;    //タイトルロゴ
	SPR_OBJ   		*title_kcej;    //タイトルallright	
	SPR_OBJ   		*title_sol;   	//タイトルson of ....
	SPR_OBJ   		*title_trial;   //タイトル初体験

	SPR_OBJ   		*title_start;   //タイトルstart		(タイトルnormal)
	SPR_OBJ   		*title_option;  //タイトルoption	(タイトルdeiffcult)
	SPR_OBJ   		*title_easy;   	//タイトルeasy
	SPR_OBJ   		*title_veasy;   //タイトルvery easy
	SPR_OBJ   		*title_vhard;   //タイトルvery hard
	
	SPR_OBJ   		*title_fade;   	//fade
	SPR_OBJ   		*title_konami;  //
	SPR_OBJ   		*title_japan;   //
	SPR_OBJ   		*title_warning; //

	//SPR_OBJ   		*title_base;   	//下地

	int				game_level;
	int				font_wide;

	char			*str_data[3];
	char			*mlog_data[12];

	char			clear_code[32];

	char			*vhard_exp;

	SPR_POS			logo_pos;
	
} Work;

#if 0
static void DebugWinView( int which )
{
	if ( which == 0 ) {
		MENU_Locate( 256 + 180, 128, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 128, 128 ) ;
		MENU_Printf( "START" ) ;
		MENU_Locate( 256 + 180, 144, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 255, 255 ) ;
		MENU_Printf( "OPTION" ) ;
    } else {
		MENU_Locate( 256 + 180, 128, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 255, 255 ) ;
		MENU_Printf( "START" ) ;
		MENU_Locate( 256 + 180, 144, MENU_MODE_CENTER ) ;
		MENU_SetColor( 255, 128, 128 ) ;
		MENU_Printf( "OPTION" ) ;
	}
	MENU_SetColor( 255, 255, 255 ) ;
}
#endif

static void ClearCodePrint( Work *work )
{
	if( work->clear_code[0] != '\0'){
		MENU_PutTextScreen( work->mlog_work,
							512 - 24 - work->font_wide*2/3, 24,//384 - 60 - 1,
							512 - 24, 44,//384 - 40 - 1,

							0, 0, work->font_wide, FONT_BUFFER_HEIGHT(1,0),
							0x80707070 );

#if 0
		MENU_Locate( 512 - 16, 448 - 80, MENU_MODE_RIGHT );
		MENU_SetColor( 192, 192, 192 ) ;
		MENU_Printf( "%s", work->clear_code ) ;
		MENU_ResetColor();
#endif
	}
}

#ifdef PSX2
static void SendMsg_FarFocusEffect( int time, int near, int far )
{
	GV_MSG 	msg;
	int msg_data[5];
	msg.address = FORCAS_NAME;
	msg.message = msg_data;
	msg.message_len = 5;
	msg_data[0] = 2;
	msg_data[1] = time;
	msg_data[2] = 0;
	msg_data[3] = near;
	msg_data[4] = far;
	GV_SendMessage( &msg );
}
#else
static void SendMsg_FarFocusEffect( int time, int ___near, int ___far )
{
	GV_MSG 	msg;
	int msg_data[5];
	msg.address = FORCAS_NAME;
	msg.message = msg_data;
	msg.message_len = 5;
	msg_data[0] = 2;
	msg_data[1] = time;
	msg_data[2] = 0;
	msg_data[3] = ___near;
	msg_data[4] = ___far;
	GV_SendMessage( &msg );
}
#endif

static void SendMsg_RainCam( int on_off )
{
	GV_MSG 	msg;
	int msg_data;

	msg.address = CAMRAIN_NAME;
	msg.message = &msg_data;
	msg.message_len = 1;
	msg_data = on_off;
	GV_SendMessage( &msg );
}

static void SetSpriteUV( SPR_SPRITE *prim, int u, int v, int w, int h  )
{
	SPR_TEX		*tex = &prim->head.tex;
	int			tw,th,off_u,off_v;

	DG_GetTexelInfo( &tw, &th, &off_u, &off_v, tex->dgtex );
//	printf("off_u %d:off_v %d:size_w %d:size_h %d\n",off_u,off_v,tw,th);
//	printf("u %d:v %d:w %d:h %d\n",u,v,w,h);
#ifdef PSX2
	if( u < u+w ){
		tex->u = ((off_u + u)<<4) + (1<<3);
	}else{
		tex->u = ((off_u + u)<<4) - (1<<3);
	}

	if( v < v+h ){
		tex->v = ((off_v + v)<<4) + (1<<3);
	}else{
		tex->v = ((off_v + v)<<4) - (1<<3);
	}

	tex->w = (w<<4) - (1<<4);
	tex->h = (h<<4) - (1<<4);
#else
	if( u < u+w ){
		tex->u = SPR_FIXED(off_u + u + 0.5f);
	}else{
		tex->u = SPR_FIXED(off_u + u - 0.5f);
	}

	if( v < v+h ){
		tex->v = SPR_FIXED(off_v + v + 0.5f);
	}else{
		tex->v = SPR_FIXED(off_v + v - 0.5f);
	}
	tex->w = SPR_FIXED(w - 1);
	tex->h = SPR_FIXED(h - 1);
#endif
	
}

static void ChangeCameraRot( Work *work )
{	
	SVECTOR svtemp;

	SendMsg_FarFocusEffect( 0, 50000,1000000 );

	GM_CopyCamera( work->rot_cam, work->camera );
	work->rot_cam->on = 1;
	GM_ChangeCamera( 0 );
	work->init_rot = work->rot_cam->rotate;
	work->timer = 0;
	work->flags |= TTL_FLAGS_CHANGECAM;
	svtemp.vx = work->init_rot.vx + work->rot_x;
	svtemp.vy = work->init_rot.vy - work->rot_y;
	svtemp.vz = work->init_rot.vz;
	GM_SetCameraRotate(work->rot_cam,&svtemp);
	
}
//決定　１
//キャンセル -1
//通常　０
static int PadCheck( Work *work, int max, int min, int cam_mode )
{
	GV_PAD *pad = GV_PadDataDirect ;
	int	   check = work->which;
	
	if ( pad->press & PAD_U ) work->which--;
	else if ( pad->press & PAD_D ) work->which++;
	
	if(work->which > max) work->which = min;
	else if(work->which < min) work->which = max;
	
	if( pad->press & (PAD_U|PAD_D) && (work->which^check) ){
		work->timer = 0;
		work->flags|=TTL_FLAGS_CAMSTOP;
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01);
	}

    if( pad->press & (PAD_OK | PAD_STA) ){
		work->flags|=TTL_FLAGS_CAMSTOP;
		return 1;
	}else if( pad->press & (PAD_CANCEL) ){
		return -1;
	}

	//カメラいじりん
	if( cam_mode && (pad->analog_input & GV_PAD_ANALOG_R_USE) ){
		SVECTOR svtemp;
		int		x,y;

		if(!(work->flags & TTL_FLAGS_CHANGECAM)) ChangeCameraRot( work );

		x = (pad->right_dx-128)/32;
		y = (pad->right_dy-128)/32;

		if(pad->right_dx == 127) x = 4;
		if(pad->right_dy == 127) y = 4;

		if(work->rot_y > 260 && x >= 2) x = 1;
		if(work->rot_y > 200 && x >= 3) x = 2;
		if(work->rot_y > 160 && x >= 4) x = 3;

		if(work->rot_y < -1536 && x <= -2) x = -1;
		if(work->rot_y < -1476 && x <= -3) x = -2;
		if(work->rot_y < -1716 && x <= -4) x = -3;
		
		if(work->rot_x > -60 && y >= 2) y = 1;
		if(work->rot_x > -120 && y >= 3) y = 2;
		if(work->rot_x > -180 && y >= 4) y = 3;

		if(work->rot_x < -540 && y <= -2) y = -1;
		if(work->rot_x < -480 && y <= -3) y = -2;
		if(work->rot_x < -420 && y <= -4) y = -3;

		work->rot_y += x;
		work->rot_x += y;

		//printf("dx %d\n",(pad->right_dx-128)/32);
		//printf("dy %d\n",(pad->right_dy-128)/32);

		if(work->rot_y > 320) work->rot_y = 320;
		if(work->rot_y < -1536) work->rot_y = -1536;
		if(work->rot_x > 0) work->rot_x = 0;
		if(work->rot_x < -600) work->rot_x = -600;
	       
		svtemp.vx = work->init_rot.vx + work->rot_x;
		svtemp.vy = work->init_rot.vy - work->rot_y;
		svtemp.vz = work->init_rot.vz;
		//printf("rot x %d: y %d timer %d\n",svtemp.vx,svtemp.vy,work->timer);
		//printf("rot x %d: y %d\n",work->rot_x,work->rot_y);
		if(GV_Clock) GM_SetCameraRotate(work->rot_cam,&svtemp);


		work->timer = 0;

	}

	return 0;
}

static void TTL_StepCaution_Act( Work *work )
{

	if( work->timer <= 64 )
		SET_COLOR_2DPRIM2(work->title_warning,128,128,128,work->timer*2);
	else if( work->timer > 128 + 128 ){
		SET_COLOR_2DPRIM2(work->title_warning,128,128,128,128 - (work->timer-128+128)*2);
	}

	if(work->timer >= 192 + 128 ){
		work->step = TTL_STEP_KONAMI;
		work->timer = 0;

		SPR_HIDE(work->title_warning);
		SPR_SHOW(work->title_konami);

		//GM_SdSet(SNG_PLAY_01);
	}
}

//タイムアウト
static void TTL_StepTimeOut_Act( Work *work )
{
	SET_COLOR_2DPRIM2(work->title_fade,0,0,0,work->timer);

	if(!(work->flags&TTL_FLAGS_CAMSTOP)){
		FVECTOR		pos;//,trgt;
		MT_HermiteLerpVec( &pos, &CmrPosList[1], &CmrPosList[2],
		&CmrVecList[1], &CmrVecList[2], MOVE_T2 * (float)(work->timer+1200) );
		DG_COPY_VEC(&work->camera->position, &pos);	
	}

	if(work->timer >= 128 ){

		SPR_HIDE(work->title_logo);
		SPR_HIDE(work->title_sol);
		SPR_HIDE(work->title_kcej);
		SPR_HIDE(work->title_trial);
		SPR_HIDE(work->title_option);
		SPR_HIDE(work->title_start);
		SPR_HIDE(work->title_veasy);
		SPR_HIDE(work->title_vhard);

		SPR_SHOW(work->title_konami);
		//SPR_SHOW(work->title_japan);
		SET_COLOR_2DPRIM(work->title_konami,0x00000000);
		SET_COLOR_2DPRIM(work->title_japan,0x00000000);

		SPR_SetPriority( work->title_konami, 6);
		SPR_SetPriority( work->title_japan, 6);

		work->timer = 0;
		work->flags = (work->flags&(TTL_FLAGS_ENGLISH|TTL_FLAGS_ENG_PAL));
		work->rot_cam->on = 0;
		work->rot_x = 0;
		work->rot_y = 0;
		GM_ChangeCamera( 0 );
		GM_SetCameraQuick( 0 );

		GCL_ExecProc( work->stop_proc_id, NULL );

		work->step = TTL_STEP_KONAMI;
		
		//GM_SdSet(SNG_PLAY_01);

		SendMsg_FarFocusEffect( 0, 100000, 500000 );
		SendMsg_RainCam( 0 );
	}
}

static void TTL_StepKonami_Act( Work *work )
{

	if( work->timer <= 64 )
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,work->timer*2);
	else if( work->timer > 128 ){
		SET_COLOR_2DPRIM2(work->title_konami,128,128,128,128 - (work->timer-128)*2);
	}

	if( PadCheck( work, 0, 0, 0 )){
		if(work->timer <= 70) return;
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
		printf("title skip\n");

		//タイトル強制表示

		//SPR_HIDE(work->title_fade);

		SPR_HIDE(work->title_konami);
		SPR_HIDE(work->title_japan);

		SET_COLOR_2DPRIM(work->title_fade,0x00000000);
		SET_COLOR_2DPRIM(work->title_logo,0x80808080);
		SET_COLOR_2DPRIM(work->title_sol,0x80808080);
		if(work->flags & TTL_FLAGS_ENGLISH){
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_ENG);
		}else{
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_JPN);
		}
		SET_COLOR_2DPRIM(work->title_kcej,0x80606060);

		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_option);
		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);

		SPR_SHOW(work->title_logo);
		SPR_SHOW(work->title_sol);
		SPR_SHOW(work->title_trial);
		SPR_SHOW(work->title_kcej);
		SPR_SHOW(work->title_fade);
		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);

		DG_COPY_VEC(&work->camera->position,&CmrPosList[1] );
		DG_COPY_VEC(&work->camera->target,&TrtPosList[1] );

		GM_SetCameraQuick( 0 );
		GM_SdSet(SNG_PLAY_01);
	}else if(work->timer >= 192 ){
		work->step = TTL_STEP_KCEJ;
		work->timer = 0;
		SPR_HIDE(work->title_konami);
		SPR_SHOW(work->title_japan);
	}
}

static void TTL_StepKCEJ_Act( Work *work )
{
	if( work->timer <= 64 )
		SET_COLOR_2DPRIM2(work->title_japan,128,128,128,work->timer*2);
	else if( work->timer > 128 ){
		SET_COLOR_2DPRIM2(work->title_japan,128,128,128,128 - (work->timer-128)*2);
	}

	if( PadCheck( work, 0, 0, 0 )){
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
		printf("title skip\n");

		//タイトル強制表示

		//SPR_HIDE(work->title_fade);

		SPR_HIDE(work->title_konami);
		SPR_HIDE(work->title_japan);

		SET_COLOR_2DPRIM(work->title_fade,0x00000000);
		SET_COLOR_2DPRIM(work->title_logo,0x80808080);
		SET_COLOR_2DPRIM(work->title_sol,0x80808080);
		if(work->flags & TTL_FLAGS_ENGLISH){
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_ENG);
		}else{
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_JPN);
		}

		SET_COLOR_2DPRIM(work->title_kcej,0x80606060);

		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_option);

		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);

		SPR_SHOW(work->title_logo);
		SPR_SHOW(work->title_sol);
		SPR_SHOW(work->title_trial);
		SPR_SHOW(work->title_kcej);
		SPR_SHOW(work->title_fade);
		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);

		DG_COPY_VEC(&work->camera->position,&CmrPosList[1] );
		DG_COPY_VEC(&work->camera->target,&TrtPosList[1] );

		GM_SetCameraQuick( 0 );
		GM_SdSet(SNG_PLAY_01);
	}else if(work->timer >= 192){
		work->timer = 0;
		work->step = TTL_STEP_TITLE;
		SPR_HIDE(work->title_japan);
		SET_COLOR_2DPRIM(work->title_japan,0x00000000);

		SET_COLOR_2DPRIM(work->title_fade,0x80000000);
		SET_COLOR_2DPRIM(work->title_logo,0x00808080);
		SET_COLOR_2DPRIM(work->title_sol,0x00808080);

		if(work->flags & TTL_FLAGS_ENGLISH){
			//SET_COLOR_2DPRIM(work->title_trial,TRIAL_COLOR_ENG);
		}else{
			//SET_COLOR_2DPRIM(work->title_trial,TRIAL_COLOR_JPN);
		}

		SET_COLOR_2DPRIM(work->title_kcej,0x00606060);
		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_option);
		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);

		SPR_SHOW(work->title_logo);
		SPR_SHOW(work->title_sol);
		SPR_SHOW(work->title_trial);
		SPR_SHOW(work->title_kcej);
		SPR_SHOW(work->title_fade);
		SPR_SHOW(work->title_option);
		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);

		DG_COPY_VEC(&work->camera->position,&CmrPosList[0] );
		DG_COPY_VEC(&work->camera->target,&TrtPosList[0] );

		GM_SetCameraQuick( 0 );

		GM_SdSet(SNG_PLAY_01);
	}
}

static void TTL_StepFadeIn_Act( Work *work )
{
	FVECTOR pos,trgt;

	MT_HermiteLerpVec( &pos, &CmrPosList[0], &CmrPosList[1],
	                         &CmrVecList[0], &CmrVecList[1], MOVE_T * (float)work->timer );
	MT_HermiteLerpVec( &trgt, &TrtPosList[0], &TrtPosList[1],
	                          &TrtVecList[0], &TrtVecList[1], MOVE_T * (float)work->timer );

	DG_COPY_VEC(&work->camera->position, &pos);
	DG_COPY_VEC(&work->camera->target, &trgt);


	if( work->timer == 60 ) SendMsg_FarFocusEffect( 300*4, 50000, 1000000 );

	//フェードイン
	if( work->timer <= 256 )SET_COLOR_2DPRIM2(work->title_fade,0,0,0,128 - (work->timer>>1));

	//タイトル
	if(work->timer >= 568 && work->timer <= 664 ){
		if(work->timer <= 632 ){
			SET_COLOR_2DPRIM2(work->title_logo,0x80,0x80,0x80,(work->timer-568)*2);
		}
		if(work->timer >= 584 && work->timer <= 648 ){
			SET_COLOR_2DPRIM2(work->title_sol,0x80,0x80,0x80,(work->timer-584)*2);

			if(work->flags & TTL_FLAGS_ENGLISH){
				//SET_COLOR_2DPRIM3(work->title_trial,(work->timer-584)*2,TRIAL_COLOR_ENG);
			}else{
				//SET_COLOR_2DPRIM3(work->title_trial,(work->timer-584)*2,TRIAL_COLOR_JPN);
			}

		}
		if(work->timer >= 600 ){
			SET_COLOR_2DPRIM2(work->title_kcej,96,96,96,(work->timer-600)*2);
		}
#if 0
		if(work->timer <= 632 ){
			SET_COLOR_2DPRIM2(work->title_logo,0x80,0x80,0x80,(work->timer-568)*2);
			SET_COLOR_2DPRIM2(work->title_sol,0x80,0x80,0x80,(work->timer-568)*2);
		}
		SET_COLOR_2DPRIM2(work->title_kcej,96,96,96,(work->timer-568)*2);
#endif

	}

	if(work->timer == 664){
		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_TENDER_M);
		NewThunder_Demo( 0.5f, -0.5f );
	}

	//文字フェードイン
	if(work->timer >= 664){
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,(work->timer-664)*2);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,(work->timer-664)*2);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,(work->timer-664)*2);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,(work->timer-664)*2);


		if( work->clear_code[0] != '\0'){
			MENU_PutTextScreen( work->mlog_work,
								512 - 24 - work->font_wide*2/3, 24,//384 - 60 - 1,
								512 - 24, 44,//384 - 40 - 1,
								0, 0, work->font_wide, FONT_BUFFER_HEIGHT(1,0),
								(((work->timer-664)*2)<<24)|0x00707070 );
		}
	}

	if( PadCheck( work, 0, 0, 0 )){
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
		printf("title skip\n");

		//タイトル強制表示

		//SPR_HIDE(work->title_fade);

		SET_COLOR_2DPRIM(work->title_fade,0x00000000);
		SET_COLOR_2DPRIM(work->title_logo,0x80808080);
		SET_COLOR_2DPRIM(work->title_sol,0x80808080);
		if(work->flags & TTL_FLAGS_ENGLISH){
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_ENG);
		}else{
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_JPN);
		}

		SET_COLOR_2DPRIM(work->title_kcej,0x80606060);

		//SPR_SHOW(work->title_start);
		//SPR_SHOW(work->title_option);

		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);

		DG_COPY_VEC(&work->camera->position,&CmrPosList[1] );
		DG_COPY_VEC(&work->camera->target,&TrtPosList[1] );

		GM_SetCameraQuick( 0 );

	}else if( work->timer > MOVE_TIME ){
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
		//SPR_SHOW(work->title_start);
		//SPR_SHOW(work->title_option);
		printf("title fade in\n");

	}
}

static void TTL_StepStrFin0_Act( Work *work )
{
	work->timer = 0;
	work->step = TTL_STEP_WAIT0;
}

static void TTL_StepWait0_Act( Work *work )
{
	int ret;

	ret = PadCheck( work, TTL_MENU_SPECIAL, TTL_MENU_START, 1 );
//printf("[%d]\n",work->which);
	if(work->which == 0){
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else if(work->which == 1){
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else if(work->which == 2){
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else{
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
	}

	if(!(work->flags&TTL_FLAGS_CAMSTOP)){
		FVECTOR		pos;//,trgt;
		MT_HermiteLerpVec( &pos, &CmrPosList[1], &CmrPosList[2],
		                         &CmrVecList[1], &CmrVecList[2], MOVE_T2 * (float)(work->timer) );

		DG_COPY_VEC(&work->camera->position, &pos);		
	}

	if( ret == 1 ){

		if( work->which == TTL_MENU_START ){
			work->step = TTL_STEP_AFTER0;
			if( !(work->flags & TTL_FLAGS_RAIN) ){
				work->flags |= TTL_FLAGS_RAIN;

				GCL_ExecProc( work->rain_proc_id, NULL );
				
				GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_HELIMIXM);
				GM_SdSet(SD_AUTO_FADER2);

				SendMsg_RainCam( 1 );
			}
		}else if( work->which == TTL_MENU_OPTION ){
			work->step = TTL_STEP_PRESLEEP;
		}else if( work->which == TTL_MENU_MLOG ){
			work->step = TTL_STEP_PREMLOG;
			GM_SdSet(SE_EXP_STOP);
			GM_SdSet(SNG_FOUTS_M);
		}else{
			work->step = TTL_STEP_SPECIAL0;
			GM_SdSet(SE_EXP_STOP);
			GM_SdSet(SNG_FOUTS_M);
		}

		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);

		work->timer = 0;
	}else if( work->timer > 1200 ){
		//タイムアウト
		work->timer = 0;
		work->step = TTL_STEP_TOUT;
		GM_SdSet(SNG_FOUTS_M);
	}
}

static void TTL_StepStrFout0_Act( Work *work )
{

	SET_COLOR_2DPRIM2(work->title_start,
					  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,64-work->timer*2);
	SET_COLOR_2DPRIM2(work->title_option,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,64-work->timer*2);
	SET_COLOR_2DPRIM2(work->title_veasy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,64-work->timer*2);
	SET_COLOR_2DPRIM2(work->title_vhard,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,64-work->timer*2);

	if(work->timer > 32){
		work->timer = 0;
		work->step = TTL_STEP_PRE1;
		work->which = TTL_MENU_NORMAL;

		printf("to pre_kill\n");

		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 16*12, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 48.0f * 0.7, 256.0f});
#else
		{
			SPR_POS	tmp = {256.0f - 48.0f * 0.7f, 256.0f};
			SPR_SetPosSprite(work->title_start, &tmp);
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 16*13, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 32.0f * 0.7f, 276.0f});
#else
		{
			SPR_POS	tmp = {256.0f - 32.0f * 0.7f, 276.0f};
			SPR_SetPosSprite(work->title_option, &tmp);
		}
#endif

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 72.0f * 0.7f, 216.0f});
#else
		{
			SPR_POS	tmp = {256.0f - 72.0f * 0.7f, 216.0f};
			SPR_SetPosSprite(work->title_veasy, &tmp);
		}
#endif
		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 16*19, 168, 16  );
		SPR_SetSizeSprite(work->title_veasy, 160.0f * 0.7f, 16.0f * 0.7f);

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_vhard, &(SPR_POS){256.0f - 72.0f * 0.7f, 296.0f});
#else
		{
			SPR_POS	tmp = {256.0f - 72.0f * 0.7f, 296.0f};
			SPR_SetPosSprite(work->title_vhard, &tmp);
		}
#endif
		SetSpriteUV( (SPR_SPRITE*)work->title_vhard, 0, 16*18, 168, 16  );

		SPR_SHOW(work->title_easy);
		//SPR_SHOW(work->title_veasy);
		//SPR_SHOW(work->title_vhard);
		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_easy,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);

	}

}

static void TTL_StepStrFin1_Act( Work *work )
{

	SET_COLOR_2DPRIM2(work->title_veasy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_vhard,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_easy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_start,
					  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_option,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);

	if( work->timer > 32 ){
		work->timer = 0;
		work->step = TTL_STEP_WAIT1;
	}
}

static void TTL_StepWait1_Act( Work *work )
{
	int ret,check = work->which;

	ret = PadCheck( work, TTL_MENU_VHARD, TTL_MENU_VEASY, 1 );

	if(work->which == 0){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else if(work->which == 1){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_easy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);

	}else if(work->which == 2){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else if(work->which == 3){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else{
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
	}

	if( check ^ work->which ){
#if 0
		extern void *NewJimakuDirect( char *mes );
		extern void JimakuSetPos( void *w, int posx, int posy );

		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}

		if(work->vhard_exp && work->which == 4){
			work->son = NewJimakuDirect(work->vhard_exp);
			JimakuSetPos(work->son,-1,DIRECT_SCREEN_Y(256+100));

			//if(work->son)GV_SetActorChild(work,work->son);
		}
#else
		GM_JimakuHide();

		if(work->vhard_exp && work->which == 4){
			GM_JimakuShow( 0, work->vhard_exp );
			GM_JimakuSetPosY( DIRECT_SCREEN_Y(256+100) );
		}
#endif
	}

	if( ret == -1 ){
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
		work->which = 0;

		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 0, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 64.0f * 0.7f, START_Y});
#else
		{
			SPR_POS tmp = {256.0f - 64.0f * 0.7f, START_Y} ;
			SPR_SetPosSprite(work->title_start, &tmp );
		}
#endif
		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 16, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 44.0f * 0.7f, OPTION_Y});
#else
		{
			SPR_POS tmp = {256.0f - 44.0f * 0.7f, OPTION_Y} ;
			SPR_SetPosSprite(work->title_option, &tmp );
		}
#endif


		//SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 70.0f * 0.7f, MLOG_Y});
		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 16*20, 256, 16  );
		//SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 82.0f * 0.7f, MLOG_Y});
		//SPR_SetSizeSprite(work->title_veasy, 192.0f * 0.7f, 16.0f * 0.7f);

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 112.0f * 0.7f, MLOG_Y});
#else
		{
			SPR_POS tmp = {256.0f - 112.0f * 0.7f, MLOG_Y} ;
			SPR_SetPosSprite(work->title_veasy, &tmp);
		}
#endif
		SPR_SetSizeSprite(work->title_veasy, 256.0f * 0.7f, 16.0f * 0.7f);

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_vhard, &(SPR_POS){256.0f - 50.0f * 0.7f, SPECIAL_Y});
#else
		{
			SPR_POS tmp = {256.0f - 50.0f * 0.7f, SPECIAL_Y} ;
			SPR_SetPosSprite(work->title_vhard, &tmp);
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_vhard, 0, 16*21, 168, 16  );

		SPR_HIDE(work->title_easy);
		//SPR_HIDE(work->title_veasy);
		//SPR_HIDE(work->title_vhard);
		SET_COLOR_2DPRIM(work->title_start,0x00202020);
		SET_COLOR_2DPRIM(work->title_option,0x00202020);
		SET_COLOR_2DPRIM(work->title_easy,0x00202020);
		SET_COLOR_2DPRIM(work->title_veasy,0x00202020);
		SET_COLOR_2DPRIM(work->title_vhard,0x00202020);
		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}
	}else if( ret == 1 ){
		//return work->which;
		work->timer = 0;
		work->step = TTL_STEP_AFTER1;
		if(work->which==4)
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START001);
		else
			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
		printf("string fade out \n");
		//GCL_ExecProc( work->rain_proc_id, NULL );
		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}
	}
}

static void TTL_StepStrFout1_Act( Work *work )
{

	if(work->which == 0){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
	}else if(work->which == 1){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_easy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);

	}else if(work->which == 2){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
	}else if(work->which == 3){
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
	}else{
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_easy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80-work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80-work->timer*4);
	}

	if( work->timer >= 32 ){
		work->timer = 0;
		work->step = TTL_STEP_PRE2;
		
		work->game_level = work->which;
		work->which = 0;

		//SPR_HIDE(work->title_start);
		//SPR_HIDE(work->title_option);

		//SPR_HIDE(work->title_veasy);
		SPR_HIDE(work->title_vhard);
#if 0
		SetSpriteUV( (SPR_SPRITE*)work->title_easy, 0, 48, 168, 16 );
		SPR_SetPosSprite(work->title_easy, &(SPR_POS){256.0f - 44.0f * 0.7f, START_Y - 24.0f});

		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 22*16, 168, 16 );
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 44.0f * 0.7f, START_Y});

		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 23*16, 168, 16 );
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 44.0f * 0.7f, OPTION_Y});

		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 144, 168, 16  );
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 24.0f * 0.7f, MLOG_Y});
#else
		SetSpriteUV( (SPR_SPRITE*)work->title_easy, 0, 48, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_easy, &(SPR_POS){256.0f - 104.0f * 0.7f, START_Y});
#else
		{
			SPR_POS tmp = {256.0f - 104.0f * 0.7f, START_Y} ;
			SPR_SetPosSprite(work->title_easy, &tmp );
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 22*16, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f+8.0f, START_Y});
#else
		{
			SPR_POS tmp = {256.0f+8.0f, START_Y} ;
			SPR_SetPosSprite(work->title_start, &tmp );
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 23*16, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f+8.0f, OPTION_Y});
#else
		{
			SPR_POS tmp = {256.0f+8.0f, OPTION_Y} ;
			SPR_SetPosSprite(work->title_option, &tmp );
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 144, 168, 16  );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f+8.0f, MLOG_Y});
#else
		{
			SPR_POS tmp = {256.0f+8.0f, MLOG_Y} ;
			SPR_SetPosSprite(work->title_veasy, &tmp);
		}
#endif
#endif

		SET_COLOR_2DPRIM(work->title_veasy,0);
		SET_COLOR_2DPRIM(work->title_easy,0);
		SET_COLOR_2DPRIM(work->title_start,0);
		SET_COLOR_2DPRIM(work->title_option,0);

		if(work->game_level == 4){
			GM_SdSet(SE_EXP_STOP);			
			GM_SdSet(SNG_FOUTS_M);
			work->which = 2;
			work->timer = 0;
			work->step = TTL_STEP_ZOOM;
			_sceVu0Normalize( &work->fvec,(FVECTOR*)DG_Chanls[0].eye.m[2]);
			_sceVu0ScaleVector(&work->fvec,&work->fvec,100.0f);
		}
#if 0
		_sceVu0Normalize( &work->fvec,(FVECTOR*)DG_Chanls[0].eye.m[2]);
		_sceVu0ScaleVector(&work->fvec,&work->fvec,100.0f);

		
		//printf("fvec vx %f:vy %f:vz %f\n",work->fvec.vx,work->fvec.vy,work->fvec.vz);

		{
			//ＢＧＭフェードのためここで終了ＰＲＯＣ呼び出し
			GCL_ARGS	gcl_args;
			int			data[2];

			data[0] = work->game_level;
			data[0] = work->whitch;

			gcl_args.argc = 2;
			gcl_args.argv = data;
			GCL_ExecProc( work->end_proc_id, &gcl_args );
        }
#endif
	}
}

static void TTL_StepStrFin2_Act( Work *work )
{
	SET_COLOR_2DPRIM2(work->title_easy,
					  196,196,196,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_start,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_option,
					  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_veasy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);

	if( work->timer > 32 ){
		work->which = 1;
		work->timer = 0;
		work->step = TTL_STEP_WAIT2;
	}

}

static void TTL_StepWait2_Act( Work *work )
{

	int ret;
	int check = work->which;

	ret = PadCheck( work, 2, 0, 1 );

	if(work->which == 0){
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else if(work->which == 1){
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
	}else{
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80);
	}

	if( check ^ work->which || work->timer==1){
#if 0
		extern void *NewJimakuDirect( char *mes );

		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}

		if(work->str_data[work->which]){
			work->son = NewJimakuDirect(work->str_data[work->which]);
			//if(work->son)GV_SetActorChild(work,work->son);
		}
#else
		GM_JimakuHide();
		if(work->str_data[work->which]){
			GM_JimakuShow( 0, work->str_data[work->which] );
		}

#endif
	}

	if( ret == -1 ){
		work->timer = 0;
		work->step = TTL_STEP_WAIT1;
		work->which = work->game_level;
 
		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 16*12, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 48.0f * 0.7f, 256.0f});
#else
		{
			SPR_POS tmp = {256.0f - 48.0f * 0.7f, 256.0f} ;
			SPR_SetPosSprite(work->title_start, &tmp);
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 16*13, 168, 16 );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 32.0f * 0.7f, 276.0f});
#else
		{
			SPR_POS tmp = {256.0f - 32.0f * 0.7f, 276.0f} ;
			SPR_SetPosSprite(work->title_option, &tmp);
		}
#endif

		SetSpriteUV( (SPR_SPRITE*)work->title_easy, 0, 16*14, 168, 16  );
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_easy, &(SPR_POS){256.0f - 32.0f * 0.7f, 236.0f});
#else
		{
			SPR_POS tmp = {256.0f - 32.0f * 0.7f, 236.0f} ;
			SPR_SetPosSprite(work->title_easy, &tmp );
		}
#endif

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 72.0f * 0.7f, 216.0f});
#else
		{
			SPR_POS tmp = {256.0f - 72.0f * 0.7f, 216.0f} ;
			SPR_SetPosSprite(work->title_veasy, &tmp );
		}
#endif
		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 16*19, 168, 16  );

		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);
		
		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}

		//SPR_HIDE(work->title_easy);
	}else if( ret == 1 ){
		if(work->son){
			GV_DestroyOtherActor(work->son);
			work->son = NULL;
		}
		work->timer = 0;
		work->step = TTL_STEP_AFTER2;

		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START001);
		//GCL_ExecProc( work->rain_proc_id, NULL );
	}
}

static void TTL_StepStrFout2_Act( Work *work )
{

	SET_COLOR_2DPRIM2(work->title_easy,
					  196,196,196,0x80 - work->timer*4);	
	if(work->which == 0){
		SET_COLOR_2DPRIM2(work->title_start,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
	}else if(work->which == 1){
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
	}else{
		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,0x80 - work->timer*4);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,0x80 - work->timer*4);
	}

	if( work->timer >= 32 ){
		work->timer = 0;
		work->step = TTL_STEP_ZOOM;
		_sceVu0Normalize( &work->fvec,(FVECTOR*)DG_Chanls[0].eye.m[2]);
		_sceVu0ScaleVector(&work->fvec,&work->fvec,100.0f);

		
		//printf("fvec vx %f:vy %f:vz %f\n",work->fvec.vx,work->fvec.vy,work->fvec.vz);
		GM_SdSet(SE_EXP_STOP);
		GM_SdSet(SNG_FOUTS_M);
#if 0
		{
			//ＢＧＭフェードのためここで終了ＰＲＯＣ呼び出し
			GCL_ARGS	gcl_args;
			int			data[2];

			data[0] = work->game_level;
			data[1] = work->which;

			gcl_args.argc = 2;
			gcl_args.argv = data;
			GCL_ExecProc( work->end_proc_id, &gcl_args );
        }
#endif
	}
}

static void TTL_StepZoom_Act( Work *work )
{
#if 0
	if(work->timer <= 32){
		if(work->which == 0){
			SET_COLOR_2DPRIM2(work->title_easy,
							  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,32-work->timer);
			SPR_HIDE(work->title_start);
			SPR_HIDE(work->title_option);
		}else if(work->which == 1){
			SET_COLOR_2DPRIM2(work->title_start,
							  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,32-work->timer);

			SPR_HIDE(work->title_easy);
			SPR_HIDE(work->title_option);
		}else{
			SET_COLOR_2DPRIM2(work->title_option,
							  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,32-work->timer);
			SPR_HIDE(work->title_start);
			SPR_HIDE(work->title_easy);
		}
	}
#endif
	if(work->timer <= 64){
		SET_COLOR_2DPRIM2(work->title_logo,0x80,0x80,0x80,128-work->timer*2);
		SET_COLOR_2DPRIM2(work->title_sol,0x80,0x80,0x80,128-work->timer*2);

		if(work->flags & TTL_FLAGS_ENGLISH){
			SET_COLOR_2DPRIM3(work->title_trial,128-work->timer*2,TRIAL_COLOR_ENG);
		}else{
			SET_COLOR_2DPRIM3(work->title_trial,128-work->timer*2,TRIAL_COLOR_JPN);
		}

		SET_COLOR_2DPRIM2(work->title_kcej,0x80,0x80,0x80,128-work->timer*2);
	}
	if( work->timer > 52 )
		SET_COLOR_2DPRIM2(work->title_fade,0,0,0,work->timer-52);

	if(work->rot_cam->on){
		_sceVu0AddVector(&work->rot_cam->position,&work->rot_cam->position,&work->fvec );
	}else{
		_sceVu0AddVector(&work->camera->position,&work->camera->position,&work->fvec );
	}
	//_sceVu0ScaleVector(&work->fvec,&work->fvec,2.0f);
		  
	if( work->timer > 180 ){
		work->timer = 0;
		work->step = TTL_STEP_PREKILL;
	}
}

static void TTL_StepPreKill_Act( Work *work )
{
	GCL_ARGS	gcl_args;
	int			data[2];

	data[0] = work->game_level;
	data[1] = work->which;

	gcl_args.argc = 2;
	gcl_args.argv = data;
    GM_DeleteCamera( work->camera ) ;
    GM_DeleteCamera( work->rot_cam ) ;
	GCL_ExecProc( work->end_proc_id, &gcl_args );

	work->timer = 0;
	work->step = TTL_STEP_KILL;
	if(work->son){
		GV_DestroyOtherActor(work->son);
		work->son = NULL;
	}

}

static void TTL_StepKill_Act( Work *work )
{
	GV_DestroyActor( work );
}

static void TTL_StepPreOption_Act( Work *work )
{
	SET_COLOR_2DPRIM2(work->title_start,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,128 - work->timer*4);
	SET_COLOR_2DPRIM2(work->title_option,
					  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,128 - work->timer*4);

	SET_COLOR_2DPRIM2(work->title_veasy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,128 - work->timer*4);
	SET_COLOR_2DPRIM2(work->title_vhard,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,128 - work->timer*4);

	if( work->timer > 32 ){
		work->timer = 0;
		work->step = TTL_STEP_SLEEP;
		printf("Title Sleep\n");
		work->son = NewGameOption_Trial(0);
		GV_SetActorChild(work,work->son);
		SPR_HIDE(work->title_start);
		SPR_HIDE(work->title_option);
		SPR_HIDE(work->title_veasy);
		SPR_HIDE(work->title_vhard);
	}
}

static void TTL_StepWaitOption_Act( Work *work )
{
	if( work->flags & TTL_FLAGS_ACTIVE ){
		work->timer = 0;
		work->step = TTL_STEP_WAKEUP;
		SET_COLOR_2DPRIM(work->title_start,0);
		SET_COLOR_2DPRIM(work->title_option,0);
		SET_COLOR_2DPRIM(work->title_vhard,0);
		SET_COLOR_2DPRIM(work->title_veasy,0);
		//表示開始
		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_option);
		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);
	}
}

static void TTL_StepAfterOption_Act( Work *work )
{
	SET_COLOR_2DPRIM2(work->title_start,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_option,
					  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_veasy,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	SET_COLOR_2DPRIM2(work->title_vhard,
					  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
	if( work->timer > 32 ){
		work->timer = 0;
		work->step = TTL_STEP_WAIT0;
		work->son = NULL;
		work->flags &= ~(TTL_FLAGS_ACTIVE);
	}
}

static void TTL_StepPreMlog_Act( Work *work )
{
	if(work->timer < 32){
		SET_COLOR_2DPRIM3( work->title_fade,work->timer*4,0);
	}else if(work->timer == 32){
		//初期化
		SPR_SetPriority( work->title_fade, 2);
		SPR_SetPriority( work->title_warning, 6);

		SPR_SetPriority( work->title_konami, 3);
		SPR_SetPriority( work->title_japan, 4);
		SPR_SetPriority( work->title_warning, 4);

		SPR_SHOW(work->title_konami);
		//SPR_SHOW(work->title_japan);
		SPR_SHOW(work->title_easy);
		SET_COLOR_2DPRIM(work->title_konami,0);
		//SET_COLOR_2DPRIM(work->title_japan,0);
		SET_COLOR_2DPRIM(work->title_start,0);
		SET_COLOR_2DPRIM(work->title_option,0);
		SET_COLOR_2DPRIM(work->title_easy,0);
		SET_COLOR_2DPRIM(work->title_veasy,0);
		SET_COLOR_2DPRIM(work->title_vhard,0);

		//BG
		SPR_ObjSetTexture(work->title_konami, TEX_CODE_MLOG_BG, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_konami, &(SPR_POS){ 0.0f, 48.0f*384.0f/448.0f});
#else
		{
			SPR_POS tmp = {0.0f, 48.0f*384.0f/448.0f} ;
			SPR_SetPosSprite(work->title_konami, &tmp);
		}
#endif

		SPR_SetSizeSprite(work->title_konami, 512.0f, 350.0f*384.0f/448.0f );
		SET_COLOR_2DPRIM(work->title_konami,0);

		//ログタイトル
		SPR_ObjSetTexture(work->title_start, TEX_CODE_MLOG_TITLE, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){ 0.0f, 14.0f*384.0f/448.0f});
#else
		{
			SPR_POS tmp = {0.0f, 14.0f*384.0f/448.0f} ;
			SPR_SetPosSprite(work->title_start, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_start, 512.0f, 32.0f*384.0f/448.0f );
		SET_COLOR_2DPRIM(work->title_start,0);
		//SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 16*13, 168, 16 );
//写真
//座標76,51
//サイズ354*214
		if(work->flags & TTL_FLAGS_ENG_PAL){
			SPR_ObjSetTexture(work->title_japan, 5349683, 0);
//		}else if(work->flags & TTL_FLAGS_ENGLISH){
//			SPR_ObjSetTexture(work->title_japan, 14284416, 0);
		}else{
			SPR_ObjSetTexture(work->title_japan, 14284416, 0);
		}
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_japan, &(SPR_POS){ 256.0f - 316.0f*0.5f, 16.0f*384.0f/448.0f});
#else
		{
			SPR_POS tmp = {256.0f - 316.0f*0.5f, 16.0f*384.0f/448.0f } ;
			SPR_SetPosSprite(work->title_japan, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_japan, 316.0f, 240.0f*384.0f/448.0f );
		SET_COLOR_2DPRIM(work->title_japan,0);
//発売中
//座標140,268
//サイズ232*132
		if(work->flags & TTL_FLAGS_ENG_PAL){
			SPR_ObjSetTexture(work->title_warning, 5353779, 0);
//		}else if(work->flags & TTL_FLAGS_ENGLISH){
//			SPR_ObjSetTexture(work->title_warning, 14349952, 0);
		}else{
			SPR_ObjSetTexture(work->title_warning, 14349952, 0);
		}

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_warning, &(SPR_POS){ 140.0f, 256.0f*384.0f/448.0f});
#else
		{
			SPR_POS tmp = {140.0f, 256.0f*384.0f/448.0f } ;
			SPR_SetPosSprite(work->title_warning, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_warning, 232.0f, 132.0f*384.0f/448.0f );
		SET_COLOR_2DPRIM(work->title_warning,0);


		//現在のページ数
		SPR_ObjSetTexture(work->title_option, TEX_CODE_MLOG_FIG, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){ 256.0f-24.0f, 348.0f});
#else
		{
			SPR_POS tmp = {256.0f-24.0f, 348.0f } ;
			SPR_SetPosSprite(work->title_option, &tmp );
		}
#endif
		SPR_SetSizeSprite(work->title_option, 16.0f, 16.0f );
		SET_COLOR_2DPRIM(work->title_option,0);
		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 0, 16, 16 );

		//最大ページ数
		SPR_ObjSetTexture(work->title_easy, TEX_CODE_MLOG_MFIG, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_easy, &(SPR_POS){ 256.0f-8.0f, 348.0f});
#else
		{
			SPR_POS tmp = {256.0f-8.0f, 348.0f } ;
			SPR_SetPosSprite(work->title_easy, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_easy, 32.0f, 16.0f );
		SET_COLOR_2DPRIM(work->title_easy,0);

		//←
		SPR_ObjSetTexture(work->title_vhard, TEX_CODE_MLOG_TRI, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_vhard, &(SPR_POS){ 256.0f-40.0f, 348.0f});
#else
		{
			SPR_POS tmp = {256.0f-40.0f, 348.0f } ;
			SPR_SetPosSprite(work->title_vhard, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_vhard, 16.0f, 16.0f );
		SET_COLOR_2DPRIM(work->title_vhard,0);
		SetSpriteUV( (SPR_SPRITE*)work->title_vhard, 0, 0, 16, 16 );

		//→
		SPR_ObjSetTexture(work->title_veasy, TEX_CODE_MLOG_TRI, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){ 256.0f+24.0f, 348.0f});
#else
		{
			SPR_POS tmp = { 256.0f+24.0f, 348.0f } ;
			SPR_SetPosSprite(work->title_veasy, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_veasy, 16.0f, 16.0f );
		SET_COLOR_2DPRIM(work->title_veasy,0);
		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 16, 0, -16, 16 );

		// ろぐ初期化
		MENU_ClearTextTexture( work->mlog_work );
        // 説明分テキストを展開する

		MENU_CreateTextTexture( work->mlog_work, 32, 16,
								640-64+24, 384-32,
								0, 12, 0, work->mlog_data[0] );


	}else{
		//BG表示
		SET_COLOR_2DPRIM3(work->title_konami,(work->timer-32)*4,0x00808080);
		//SET_COLOR_2DPRIM3(work->title_japan,(work->timer-32)*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_start,(work->timer-32)*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_option,(work->timer-32)*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_easy,(work->timer-32)*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_vhard,(work->timer-32)*4,0x00202020);
		SET_COLOR_2DPRIM3(work->title_veasy,(work->timer-32)*4,0x00808080);

		MENU_PutTextScreen( work->mlog_work,
							0, (int)(48.0f*384.0f/448.0f),
							512, (int)(48.0f+350.0f*384.0f/448.0f),
							0, 0, 640, 384, (((work->timer-32)*4)<<24)|0x00808080 );
	}

	if( work->timer > 64 ){
		work->timer = 0;
		work->step = TTL_STEP_MLOG_WAIT;
		work->which = 0;
		
		SPR_HIDE(work->title_logo);
		SPR_HIDE(work->title_sol);
		SPR_HIDE(work->title_kcej);
		SPR_HIDE(work->title_trial);
	}
}

static void TTL_StepMlogNext_Act( Work *work )
{
	int n_mlog_max = (work->flags & TTL_FLAGS_ENGLISH)?10:6;

	if(work->timer < 16){
		MENU_PutTextScreen( work->mlog_work,
							0, (int)(48.0f*384.0f/448.0f),
							512, (int)(48.0f+350.0f*384.0f/448.0f),
							0, 0, 640, 384, ((128-work->timer*8)<<24)|0x00808080 );
	}else if(work->timer == 16){
		SetSpriteUV( (SPR_SPRITE*)work->title_option, 16*work->which, 0, 16, 16 );
		if(work->which != n_mlog_max){
			// ろぐ初期化
			MENU_ClearTextTexture( work->mlog_work );
			// 説明分テキストを展開する
			MENU_CreateTextTexture( work->mlog_work, 32, 16,
									640-64+24, 384-32,
									0, 12, 0, work->mlog_data[work->which] );
		}else{
			SPR_SHOW(work->title_japan);
			SPR_SHOW(work->title_warning);

			SET_COLOR_2DPRIM(work->title_japan,0);
			SET_COLOR_2DPRIM(work->title_warning,0);
		}

	}else{
		if(work->which != n_mlog_max){
			MENU_PutTextScreen( work->mlog_work,
								0, (int)(48.0f*384.0f/448.0f),
								512, (int)(48.0f+350.0f*384.0f/448.0f),
								0, 0, 640, 384, (((work->timer-16)*8)<<24)|0x00808080 );
		}else{
			SET_COLOR_2DPRIM3(work->title_japan,((work->timer-16)*8),0x00808080 );
			SET_COLOR_2DPRIM3(work->title_warning,((work->timer-16)*8),0x00808080 );
			SET_COLOR_2DPRIM3(work->title_konami,128-(work->timer-16)*8,0x00808080);
			SET_COLOR_2DPRIM3(work->title_start,128-(work->timer-16)*8,0x00808080);
		}
	}
	
	if(work->timer >= 32){
		work->timer = 0;
		work->step = TTL_STEP_MLOG_WAIT;
	}
}

static void TTL_StepMlogBack_Act( Work *work )
{
	int n_mlog_max = (work->flags & TTL_FLAGS_ENGLISH)?10:6;

	if(work->timer < 16){
		if(work->which != n_mlog_max-1){
			MENU_PutTextScreen( work->mlog_work,
								0, (int)(48.0f*384.0f/448.0f),
								512, (int)(48.0f+350.0f*384.0f/448.0f),
								0, 0, 640, 384, ((128-work->timer*8)<<24)|0x00808080 );
		}else{
			SET_COLOR_2DPRIM3(work->title_japan,((128-work->timer*8)),0x00808080 );
			SET_COLOR_2DPRIM3(work->title_warning,((128-work->timer*8)),0x00808080 );
			SET_COLOR_2DPRIM3(work->title_konami,((work->timer*8)),0x00808080 );
			SET_COLOR_2DPRIM3(work->title_start,((work->timer*8)),0x00808080 );
		}
	}else if(work->timer == 16){
		SetSpriteUV( (SPR_SPRITE*)work->title_option, 16*work->which, 0, 16, 16 );
		// ろぐ初期化
		MENU_ClearTextTexture( work->mlog_work );
        // 説明分テキストを展開する
		MENU_CreateTextTexture( work->mlog_work, 32, 16,
								640-64+24, 384-32,
								0, 12, 0, work->mlog_data[work->which] );
		SPR_HIDE(work->title_japan);
		SPR_HIDE(work->title_warning);

	}else{
		MENU_PutTextScreen( work->mlog_work,
							0, (int)(48.0f*384.0f/448.0f),
							512, (int)(48.0f+350.0f*384.0f/448.0f),
							0, 0, 640, 384, (((work->timer-16)*8)<<24)|0x00808080 );
	}

	if(work->timer > 32){
		work->timer = 0;
		work->step = TTL_STEP_MLOG_WAIT;
	}
}

static void TTL_StepMlogWait_Act( Work *work )
{
	GV_PAD *pad = GV_PadDataDirect ;
	int n_mlog_max = (work->flags & TTL_FLAGS_ENGLISH)?10:6;

//	int	   check = work->which;
//	int	   rgb = abs(work->timer%193-96) + 32;

	if(work->which!=n_mlog_max){
		MENU_PutTextScreen( work->mlog_work,
							0, (int)(48.0f*384.0f/448.0f),
							512, (int)(48.0f+350.0f*384.0f/448.0f),
							0, 0, 640, 384, 0x80808080 );
	}
    if( pad->press & (PAD_OK | PAD_R | PAD_R1)){

		if(work->which < n_mlog_max){
			work->which++;
			work->timer = 0;
			work->step = TTL_STEP_MLOG_NEXT;

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAPER01);
		}
	}else if( pad->press & (PAD_L | PAD_L1) ){

		if(work->which > 0){
			work->which--;
			work->timer = 0;
			work->step = TTL_STEP_MLOG_BACK;

			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAPER01);
		}

	}else if( pad->press & (PAD_CANCEL) ){
		
		work->timer = 0;
		work->step = TTL_STEP_AFTMLOG;

		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01);
	}	

	if( work->which ){
		//SET_COLOR_2DPRIM2(work->title_vhard,rgb,rgb,rgb,128);
		SET_COLOR_2DPRIM(work->title_vhard,0x80808080);
	}else{
		SET_COLOR_2DPRIM(work->title_vhard,0x80202020);
	}
	if( work->which!=n_mlog_max){
		SET_COLOR_2DPRIM(work->title_veasy,0x80808080);
		//SET_COLOR_2DPRIM2(work->title_veasy,rgb,rgb,rgb,128);
	}else{
		SET_COLOR_2DPRIM(work->title_veasy,0x80202020);
	}

}

static void TTL_StepAfterMlog_Act( Work *work )
{
	int n_mlog_max = (work->flags & TTL_FLAGS_ENGLISH)?10:6;

	if(work->timer < 32){
		SET_COLOR_2DPRIM3(work->title_option,128 - work->timer*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_easy,128 - work->timer*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_vhard,128 - work->timer*4,0x00808080);
		SET_COLOR_2DPRIM3(work->title_veasy,128 - work->timer*4,0x00808080);

		if(work->which == n_mlog_max){
			SET_COLOR_2DPRIM3(work->title_japan,128 - work->timer*4,0x00808080);
			SET_COLOR_2DPRIM3(work->title_warning,128 - work->timer*4,0x00808080);
		}else{
			MENU_PutTextScreen( work->mlog_work,
								0, (int)(48.0f*384.0f/448.0f),
								512, (int)(48.0f+350.0f*384.0f/448.0f),
								0, 0, 640, 384, ((128-work->timer*4)<<24)|0x00808080 );
			SET_COLOR_2DPRIM3(work->title_konami,128 - work->timer*4,0x00808080);
			SET_COLOR_2DPRIM3(work->title_start,128 - work->timer*4,0x00808080);
		}
	}else if(work->timer == 32){
		//元に戻す
		SPR_SetPriority( work->title_fade, 5);

		SPR_SetPriority( work->title_konami, 6);
		SPR_SetPriority( work->title_japan, 7);
		SPR_SetPriority( work->title_warning, 6);
		//
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 64.0f * 0.7f, START_Y});
#else
		{
			SPR_POS tmp = { 256.0f - 64.0f * 0.7f, START_Y } ;
			SPR_SetPosSprite(work->title_start, &tmp );
		}
#endif

		SPR_ObjSetTexture(work->title_start, TEX_CODE_STR, 0);
		SPR_SetSizeSprite(work->title_start, 160.0f * 0.7f, 16.0f * 0.7f);
		SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 0, 168, 16  );

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 44.0f * 0.7f, OPTION_Y });
#else
		{
			SPR_POS tmp = { 256.0f - 44.0f * 0.7f, OPTION_Y } ;
			SPR_SetPosSprite(work->title_option, &tmp );
		}
#endif
		SPR_ObjSetTexture(work->title_option, TEX_CODE_STR, 0);
		SPR_SetSizeSprite(work->title_option, 160.0f * 0.7f, 16.0f * 0.7f);
		SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 16, 168, 16  );

		SPR_ObjSetTexture(work->title_easy, TEX_CODE_STR, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_easy, &(SPR_POS){256.0f - 32.0f * 0.7f, 236.0f});
#else
		{
			SPR_POS tmp = { 256.0f - 32.0f * 0.7f, 236.0f } ;
			SPR_SetPosSprite(work->title_easy, &tmp );
		}
#endif
		SPR_SetSizeSprite(work->title_easy, 160.0f * 0.7f, 16.0f * 0.7f);
		SetSpriteUV( (SPR_SPRITE*)work->title_easy, 0, 16*14, 168, 16  );

		SPR_ObjSetTexture(work->title_veasy, TEX_CODE_STR, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 70.0f * 0.7f, MLOG_Y});
#else
		{
			SPR_POS tmp = { 256.0f - 70.0f * 0.7f, MLOG_Y } ;
			SPR_SetPosSprite(work->title_veasy, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_veasy, 160.0f * 0.7f, 16.0f * 0.7f);
		SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 16*20, 256, 16  );
		//SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 82.0f * 0.7f, MLOG_Y});
		//SPR_SetSizeSprite(work->title_veasy, 192.0f * 0.7f, 16.0f * 0.7f);

#ifdef BP_PS2
		SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 112.0f * 0.7f, MLOG_Y});
#else
		{
			SPR_POS tmp = { 256.0f - 112.0f * 0.7f, MLOG_Y } ;
			SPR_SetPosSprite(work->title_veasy, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_veasy, 256.0f * 0.7f, 16.0f * 0.7f);

		SPR_ObjSetTexture(work->title_vhard, TEX_CODE_STR, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_vhard, &(SPR_POS){256.0f - 50.0f * 0.7f, SPECIAL_Y});
#else
		{
			SPR_POS tmp = { 256.0f - 50.0f * 0.7f, SPECIAL_Y } ;
			SPR_SetPosSprite(work->title_vhard, &tmp );
		}
#endif
		SPR_SetSizeSprite(work->title_vhard, 160.0f * 0.7f, 16.0f * 0.7f);
		SetSpriteUV( (SPR_SPRITE*)work->title_vhard, 0, 16*21, 168, 16  );


		SET_COLOR_2DPRIM2(work->title_start,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
		SET_COLOR_2DPRIM2(work->title_option,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
		SET_COLOR_2DPRIM2(work->title_veasy,
						  SELECT_COLOR_R,SELECT_COLOR_G,SELECT_COLOR_B,work->timer*4);
		SET_COLOR_2DPRIM2(work->title_vhard,
						  NOT_SELECT_COLOR_R,NOT_SELECT_COLOR_G,NOT_SELECT_COLOR_B,work->timer*4);
		
		SPR_HIDE(work->title_japan);
//		SPR_HIDE(work->title_konami);
		SPR_HIDE(work->title_warning);
		SPR_SHOW(work->title_logo);
		SPR_SHOW(work->title_sol);
		SPR_SHOW(work->title_kcej);
		SPR_SHOW(work->title_trial);

	}else{
		SET_COLOR_2DPRIM3(work->title_fade,128 - (work->timer-32)*4,0);
	}

	if( work->timer > 64 ){
		work->timer = 0;
		work->step = TTL_STEP_WAIT0;
		work->which = 2;

		SET_COLOR_2DPRIM(work->title_fade,0);
		if(work->flags & TTL_FLAGS_ENGLISH){
			SPR_ObjSetTexture(work->title_konami, TEX_CODE_KONAMI_ENG, 0);
		}else{
			SPR_ObjSetTexture(work->title_konami, TEX_CODE_KONAMI, 0);
		}
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_konami, &(SPR_POS){ 0.0f, 0.0f});
#else
		{
			SPR_POS tmp = { 0.0f, 0.0f } ;
			SPR_SetPosSprite(work->title_konami, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_konami, 512.0f, 384.0f );
		SET_COLOR_2DPRIM(work->title_konami,0);
		work->title_konami->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);

		SPR_ObjSetTexture(work->title_japan, TEX_CODE_JPN, 0);
#ifdef BP_PS2
		SPR_SetPosSprite(work->title_japan, &(SPR_POS){ 0.0f, 0.0f});
#else
		{
			SPR_POS tmp = { 0.0f, 0.0f } ;
			SPR_SetPosSprite(work->title_japan, &tmp );
		}
#endif

		SPR_SetSizeSprite(work->title_japan, 512.0f, 384.0f );
		SET_COLOR_2DPRIM(work->title_japan,0);

		// ろぐ初期化
		MENU_ClearTextTexture( work->mlog_work );
        // 説明分テキストを展開する
		work->font_wide = MENU_CreateTextTexture( work->mlog_work, 0, 0,
								FONT_BUFFER_WIDTH(14,0), FONT_BUFFER_HEIGHT(1,0),
								0, 0, 0, work->clear_code );

		GM_SdSet(SNG_PLAY_01);

		if( work->flags & TTL_FLAGS_RAIN ){
			GM_SdSet(SD_AUTO_FADER2);
		}
	}

}

static void TTL_StepSpecial0_Act( Work *work )
{
	SET_COLOR_2DPRIM2(work->title_fade,0,0,0,work->timer*4);

	if(work->timer > 32+2 ){
		work->step = TTL_STEP_SPECIAL1;
		work->timer = 0;

		SPR_HIDE(work->title_logo);
		SPR_HIDE(work->title_sol);
		SPR_HIDE(work->title_kcej);
		SPR_HIDE(work->title_trial);
		SPR_HIDE(work->title_option);
		SPR_HIDE(work->title_start);
		SPR_HIDE(work->title_veasy);
		SPR_HIDE(work->title_vhard);
	}
}

static void TTL_StepSpecial1_Act( Work *work )
{
    GM_DeleteCamera( work->camera ) ;
    GM_DeleteCamera( work->rot_cam ) ;

	GCL_ExecProc( work->special_proc_id, NULL);

	work->timer = 0;
	work->step = TTL_STEP_KILL;
	if(work->son){
		GV_DestroyOtherActor(work->son);
		work->son = NULL;
	}
}

static void TTL_StepFromSp_Act( Work *work )
{

	SET_COLOR_2DPRIM2(work->title_fade,0,0,0,128 - (work->timer<<2));

	if(work->timer >= 32){
		work->timer = 0;
		work->step = TTL_STEP_PRE0;
	}
}
/*----------------------------------------------------------------*/

static void Act( Work *work )
{
	
	//if((irnd()%16) == 0){
	//	NewThunder_Demo( 0.5f, -1.0f );
	//	printf("KAMINARI SAMA\n");
	//}
	//printf("time %d\n",work->timer);

	switch(work->step){

	case TTL_STEP_CAUTION:			//注意
		TTL_StepCaution_Act( work );
		break;


	case TTL_STEP_TOUT:				//タイムアウト処理
		TTL_StepTimeOut_Act( work );
		break;
	case TTL_STEP_KONAMI:			//コナミロゴ
		TTL_StepKonami_Act( work );
		break;
	case TTL_STEP_KCEJ:				//ＫＣＥＪロゴ
		TTL_StepKCEJ_Act( work );
		break;
	case TTL_STEP_TITLE:			//タイトルフェードイン カメラ移動
		TTL_StepFadeIn_Act( work );
		//ClearCodePrint(work);
		break;


	case TTL_STEP_PRE0:				//文字フェードイン
		TTL_StepStrFin0_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_WAIT0:	   		//選択待ち	<<-->>	オプション
		TTL_StepWait0_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_AFTER0:			//文字フェードアウト
		TTL_StepStrFout0_Act( work );
		ClearCodePrint(work);
		break;


	case TTL_STEP_PRE1:				//文字フェードイン
		TTL_StepStrFin1_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_WAIT1:		   	//選択待ち	難易度
		TTL_StepWait1_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_AFTER1:			//文字フェードアウト
		TTL_StepStrFout1_Act( work );
		ClearCodePrint(work);
		break;


	case TTL_STEP_PRE2:				//文字フェードイン
		TTL_StepStrFin2_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_WAIT2:		   	//選択待ち  レーダー
		TTL_StepWait2_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_AFTER2:			//文字フェードアウト
		TTL_StepStrFout2_Act( work );
		ClearCodePrint(work);
		break;


	case TTL_STEP_ZOOM:				//橋ズーム スタート（雨起動）
		TTL_StepZoom_Act( work );
		break;
	case TTL_STEP_PREKILL:			//橋ズーム スタート フェードアウト
		TTL_StepPreKill_Act( work );
		break;
	case TTL_STEP_KILL:				//終了処理	proc呼び出し
		TTL_StepKill_Act( work );
		break;

   
	case TTL_STEP_PRESLEEP:			//オプション前処理
		TTL_StepPreOption_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_SLEEP:		   	//オプション中
		TTL_StepWaitOption_Act( work );
		ClearCodePrint(work);
		break;
	case TTL_STEP_WAKEUP:			//オプション終了処理
		TTL_StepAfterOption_Act( work );
		ClearCodePrint(work);
		break;

	case TTL_STEP_PREMLOG:
		TTL_StepPreMlog_Act( work );
		break;
	case TTL_STEP_MLOG_NEXT:
		TTL_StepMlogNext_Act( work );
		break;
	case TTL_STEP_MLOG_BACK:
		TTL_StepMlogBack_Act( work );
		break;
	case TTL_STEP_MLOG_WAIT:
		TTL_StepMlogWait_Act( work );
		break;
	case TTL_STEP_AFTMLOG:
		TTL_StepAfterMlog_Act( work );
		break;

	case TTL_STEP_SPECIAL0:
		TTL_StepSpecial0_Act( work );
		break;
	case TTL_STEP_SPECIAL1:
		TTL_StepSpecial1_Act( work );
		break;
	case TTL_STEP_FROM_SP:
		TTL_StepFromSp_Act( work );
		break;
	default:
		printf("title step err!!\n");
		break;
	}
	//printf("time[%4d]\n",work->timer);
	work->timer++;
}

static void Die( Work *work )
{
	//if(work->title_logo) SPR_Destroy_2D_Object(work->title_logo);
	//if(work->title_kcej) SPR_Destroy_2D_Object(work->title_kcej);
	//if(work->title_start) SPR_Destroy_2D_Object(work->title_start);
	//if(work->title_option) SPR_Destroy_2D_Object(work->title_option);

	GM_ResetGameStatus( STATE_PAUSE_DISABLE );
	
   // GM_DeleteCamera( work->camera ) ;
   // GM_DeleteCamera( work->rot_cam ) ;
}

static int ReceiveSignal( void *workp, int signal, int value)
{
    Work *work = workp ;

	if( signal == 1 ){
	//	work->count = 0;
		work->flags |= TTL_FLAGS_ACTIVE;
		return 0;
	}

	return GV_DefaultSignalFunc( work, signal, value );
}


static void InitCamera( Work *work, FVECTOR *from, FVECTOR *to )
{
    GM_CameraSet	*cam ;

    work->camera = cam = NewProgramCamera( 0, 0, GM_CAMERA_PROG4, 0 ) ;
	if(cam == NULL){ printf("cannot make camera\n");return;}
	
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET,CAM_FLAG_FIX ) ;
    //GM_SetCameraRotate( cam, &GM_CameraDir ) ;
    //GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, 4.5f ) ;

	DG_COPY_VEC(&cam->position, from);
	DG_COPY_VEC(&cam->target, to);
	//cam->position = ;
	//cam->target = ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4,
							   GM_CAM_INTERP_EXP4, 0, 0 ) ;
    cam->on = 1 ;

    work->rot_cam = cam = NewProgramCamera( 1, 0, GM_CAMERA_PROG2, 16 ) ;
	if(cam == NULL){ printf("cannot make rot_cam\n"); return; }
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE, CAM_FLAG_FIX ) ;
    GM_SetCameraAngle( cam, 4.5f ) ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4,GM_CAM_INTERP_EXP4, 0, 0 ) ;
    cam->on = 0;

    GM_ChangeCamera( 0 );
    GM_SetCameraQuick( 0 );
}
	
static int GetResources( Work *work, int name, int where )
{
	int		mode,seed,eng;
	INPUT_DATA	base_code[2];

//    NewDebugCamera( 0 );			
	InitCamera( work, &CmrPosList[0],&TrtPosList[0] );
	
	SendMsg_FarFocusEffect( 0, 100000, 500000 );

	NewRainCamera_Demo_AddesMessageName( CAMRAIN_NAME, -1 );
	SendMsg_RainCam( 0 );

	//GM_ChangeCamera( 0 );
	//GM_UseCamera( 0 );
	
	work->name = name;
	work->where = where;
	work->count = 60;
	work->which = 0;
	work->flags = 0;
	work->end_proc_id = GCL_GetOptionValue( 'e', 0 );
	work->rain_proc_id = GCL_GetOptionValue( 'r', 0 );
	work->stop_proc_id = GCL_GetOptionValue( 's', 0 );
	work->special_proc_id = GCL_GetOptionValue( 'p', 0 );

	mode = GCL_GetOptionValue( 'm', 0 );

	eng = GCL_GetOptionValue( 'k', 0 );
	if(eng == 1) work->flags = TTL_FLAGS_ENGLISH;
	if(eng == 2) work->flags = TTL_FLAGS_ENGLISH|TTL_FLAGS_ENG_PAL;
	
	if(GCL_GetOption('d')){
		work->str_data[0] = (char*)GCL_GetNextInt();
		work->str_data[1] = (char*)GCL_GetNextInt();
		work->str_data[2] = (char*)GCL_GetNextInt();
	}else{
		work->str_data[0] = NULL;
		work->str_data[1] = NULL;
		work->str_data[2] = NULL;
	}
	
	
	if(GCL_GetOption('l')){
		work->mlog_data[0] = (char*)GCL_GetNextInt();
		work->mlog_data[1] = (char*)GCL_GetNextInt();
		work->mlog_data[2] = (char*)GCL_GetNextInt();
		work->mlog_data[3] = (char*)GCL_GetNextInt();
		work->mlog_data[4] = (char*)GCL_GetNextInt();
		work->mlog_data[5] = (char*)GCL_GetNextInt();
		if(eng){
			work->mlog_data[6] = (char*)GCL_GetNextInt();
			work->mlog_data[7] = (char*)GCL_GetNextInt();
			work->mlog_data[8] = (char*)GCL_GetNextInt();
			work->mlog_data[9] = (char*)GCL_GetNextInt();
		}

		work->vhard_exp = (char*)GCL_GetNextInt();
	}else{
		work->mlog_data[0] = NULL;
		work->mlog_data[1] = NULL;
		work->mlog_data[2] = NULL;
		work->mlog_data[3] = NULL;
		work->mlog_data[4] = NULL;
		work->mlog_data[5] = NULL;
		return -1;
	}

	if(GCL_GetOptionValue( 'f', 0 )){
		int i,j;
		char	str[32];

		seed = GCL_GetOptionValue( 't', 0 );
		printf("シード値 %d\n",seed);
		if(!GCL_GetOption('c'))return -1;
		base_code[0].data = GCL_GetNextInt();
		base_code[0].len = GCL_GetNextInt();
		base_code[1].data = GCL_GetNextInt();
		base_code[1].len = GCL_GetNextInt();
		printf("input data[0] data[%x]len[%d]\n",base_code[0].data,base_code[0].len);
		printf("input data[1] data[%x]len[%d]\n",base_code[1].data,base_code[1].len);
		encode( str, base_code, 2, seed );
		printf("encode [%s]\n",work->clear_code);
		for(i=0,j=0;i<strlen(str);i++,j++){
			if(i && i%4 == 0)work->clear_code[j++] = ' ';
			work->clear_code[j] = str[i];
		}
		printf("encode [%s]\n",work->clear_code);		
	}
/*	else{
		int i;
		for( i = 0; i < 16; i++ ){
			work->clear_code[i] = (i<12)?'a':'\0';
		}
	}
*/
	work->timer = 0;
	work->rot_x = 0;
	work->rot_y = 0;

	GV_SetActorSignalFunc( work, ReceiveSignal );
	
	//printf("end_proc_id = %d\n",work->end_ proc_id);
	
	SPR_LoadTexture(3656015);
	
	//work->logo_pos = (SPR_POS){256.0f - 384.0f * 1.1f/2.0f , 48.0f + 16.0f };
	//work->logo_pos = (SPR_POS){64.0f , 64.0f};
	
	
	work->title_konami = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_konami == NULL){ printf("ERR!! Make2DObj[work->title_konami]\n"); return -1; }
	if(eng)
		SPR_ObjSetTexture(work->title_konami, TEX_CODE_KONAMI_ENG, 0);
	else
		SPR_ObjSetTexture(work->title_konami, TEX_CODE_KONAMI, 0);
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_konami, &(SPR_POS){ 0.0f, 0.0f});
#else
	{
		SPR_POS tmp = { 0.0f, 0.0f } ;
		SPR_SetPosSprite(work->title_konami, &tmp );
	}
#endif

	SPR_SetSizeSprite(work->title_konami, 512.0f, 384.0f );
	work->title_konami->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_konami->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_konami,0x00000000);

	work->title_japan = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_japan == NULL){ printf("ERR!! Make2DObj[work->title_japan]\n"); return -1; }
	SPR_ObjSetTexture(work->title_japan, TEX_CODE_JPN, 0);
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_japan, &(SPR_POS){ 0.0f, 0.0f});
#else
	{
		SPR_POS tmp = { 0.0f, 0.0f } ;
		SPR_SetPosSprite(work->title_japan, &tmp );
	}
#endif

	SPR_SetSizeSprite(work->title_japan, 512.0f, 384.0f );
	work->title_japan->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_japan->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_japan,0x00000000);

	work->title_warning = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_warning == NULL){ printf("ERR!! Make2DObj[work->title_warning]\n"); return -1; }
	if(eng){
		SPR_ObjSetTexture(work->title_warning, TEX_CODE_WARNING_ENG, 0);
	}else{
		SPR_ObjSetTexture(work->title_warning, TEX_CODE_WARNING, 0);
	}
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_warning, &(SPR_POS){ 0.0f, 0.0f});
#else
	{
		SPR_POS tmp = { 0.0f, 0.0f } ;
		SPR_SetPosSprite(work->title_warning, &tmp );
	}
#endif

	SPR_SetSizeSprite(work->title_warning, 512.0f, 384.0f );
	work->title_warning->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_warning->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_warning,0x00000000);

	//フェード
	work->title_fade = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_fade == NULL){ printf("ERR!! Make2DObj[work->title_fade]\n"); return -1; }
//	SPR_ObjSetTexture(work->title_fade, TEX_CODE_STR, 0);
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_fade, &(SPR_POS){ 0.0f, 0.0f});
#else
	{
		SPR_POS tmp = { 0.0f, 0.0f } ;
		SPR_SetPosSprite(work->title_fade, &tmp );
	}
#endif

	SPR_SetSizeSprite(work->title_fade, 512.0f, 384.0f );
	work->title_fade->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_fade->head.flags |= SPR_FLAG_ALPHA;
	SET_COLOR_2DPRIM(work->title_fade,0x80000000);

	//work->logo_pos = (SPR_POS){256.0f - 384.0f * 1.1f/2.0f , 48.0f + 16.0f };
#ifdef BP_PS2
	work->logo_pos = (SPR_POS){256.0f - 384.0f * 1.1f * 0.5f , 48.0f + 8.0f };
#else
	{
		SPR_POS tmp = { 256.0f - 384.0f * 1.1f * 0.5f , 48.0f + 8.0f } ;
		work->logo_pos = tmp ;
	}
#endif

	//タイトルロゴ
	work->title_logo = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_logo == NULL){ printf("ERR!! Make2DObj[work->title_logo]\n"); return -1; }
	SPR_SetPosSprite(work->title_logo, &work->logo_pos );
	SPR_ObjSetTexture(work->title_logo, TEX_CODE_LOGO, 0);
	work->title_logo->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_logo->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetSizeSprite(work->title_logo, 384.0f * 1.1f, 92.8f * 1.05f * 0.75f );
	SET_COLOR_2DPRIM(work->title_logo,0x00808080);

    work->logo_pos.y += 92.8f * 1.05f * 0.75f;
    //サンズオブ
    work->title_sol = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_sol == NULL){ printf("ERR!! Make2DObj[work->title_sol]\n"); return -1; }
	SPR_SetPosSprite(work->title_sol, &work->logo_pos );
	SPR_ObjSetTexture(work->title_sol, TEX_CODE_SOL, 0);
	work->title_sol->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_sol->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetSizeSprite(work->title_sol, 384.0f * 1.1f, 92.8f * 1.05f * 0.25f );
	SET_COLOR_2DPRIM(work->title_sol,0x00808080);

	//初体験
	work->title_trial = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_trial == NULL){ printf("ERR!! Make2DObj[work->title_trial]\n"); return -1; }
	SPR_SetPosSprite(work->title_trial, &work->logo_pos );

	if(eng){
		work->logo_pos.y += 92.8f * 1.05f * 0.25f + 6.0f;
		work->logo_pos.x = 256.0f - 320.0f * 1.1f * 0.5f;
		SPR_ObjSetTexture(work->title_trial, TEX_CODE_TRIAL_ENG, 0);
		SPR_SetSizeSprite(work->title_trial, 320.0f * 1.1f, 20.0f );
		SET_COLOR_2DPRIM(work->title_trial,0x003030c0);
	}else{
		work->logo_pos.y += 92.8f * 1.05f * 0.25f + 16.0f;
		work->logo_pos.x = 256.0f - 164.0f * 1.1f * 0.5f;
		SPR_ObjSetTexture(work->title_trial, TEX_CODE_TRIAL, 0);
		SPR_SetSizeSprite(work->title_trial, 164.0f * 1.1f, 20.0f );
		SET_COLOR_2DPRIM(work->title_trial,0x00808080);
	}
	SPR_SetPosSprite(work->title_trial, &work->logo_pos );
	work->title_trial->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_trial->head.flags |= SPR_FLAG_ALPHA;


	//ALL RIGHTS
	work->title_kcej = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_kcej == NULL){ printf("ERR!! Make2DObj[work->title_kcej]\n"); return -1; }
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_kcej, &(SPR_POS){51.2f, 344.0f});
#else
	{
		SPR_POS tmp = { 51.2f, 344.0f } ;
		SPR_SetPosSprite(work->title_kcej, &tmp );
	}
#endif

	SPR_ObjSetTexture(work->title_kcej, TEX_CODE_KCEJ, 0);
	work->title_kcej->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_kcej->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetSizeSprite(work->title_kcej, 409.6f, 9.6f);
	SET_COLOR_2DPRIM(work->title_kcej,0x00808080);


	//スタート
	work->title_start = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
	if(work->title_start == NULL){ printf("ERR!! Make2DObj[work->title_start]\n"); return -1; }
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_start, &(SPR_POS){256.0f - 64.0f * 0.7f, START_Y});
#else
	{
		SPR_POS tmp = { 256.0f - 64.0f * 0.7f, START_Y } ;
		SPR_SetPosSprite(work->title_start, &tmp );
	}
#endif
	SPR_ObjSetTexture(work->title_start, TEX_CODE_STR, 0);
	work->title_start->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
	work->title_start->head.flags |= SPR_FLAG_ALPHA;	
	SPR_SetSizeSprite(work->title_start, 160.0f * 0.7f, 16.0f * 0.7f);

	//オプション
	work->title_option = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
    if(work->title_option == NULL){ printf("ERR!! Make2DObj[work->title_option]\n"); return -1; }
#ifdef BP_PS2
    SPR_SetPosSprite(work->title_option, &(SPR_POS){256.0f - 44.0f * 0.7f, OPTION_Y });
#else
	{
		SPR_POS tmp = { 256.0f - 44.0f * 0.7f, OPTION_Y } ;
		SPR_SetPosSprite(work->title_option, &tmp );
	}
#endif
    SPR_ObjSetTexture(work->title_option, TEX_CODE_STR, 0);
    work->title_option->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
    work->title_option->head.flags |= SPR_FLAG_ALPHA;
    SPR_SetSizeSprite(work->title_option, 160.0f * 0.7f, 16.0f * 0.7f);

	//イージー
	work->title_easy = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
    if(work->title_easy == NULL){ printf("ERR!! Make2DObj[work->title_easy]\n"); return -1; }
    SPR_ObjSetTexture(work->title_easy, TEX_CODE_STR, 0);
    work->title_easy->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
    work->title_easy->head.flags |= SPR_FLAG_ALPHA;
#ifdef BP_PS2
    SPR_SetPosSprite(work->title_easy, &(SPR_POS){256.0f - 32.0f * 0.7f, 236.0f});
#else
	{
		SPR_POS tmp = { 256.0f - 32.0f * 0.7f, 236.0f } ;
		SPR_SetPosSprite(work->title_easy, &tmp );
	}
#endif

    SPR_SetSizeSprite(work->title_easy, 160.0f * 0.7f, 16.0f * 0.7f);

	
	work->title_veasy = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
    if(work->title_veasy == NULL){ printf("ERR!! Make2DObj[work->title_veasy]\n"); return -1; }
    SPR_ObjSetTexture(work->title_veasy, TEX_CODE_STR, 0);
    work->title_veasy->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
    work->title_veasy->head.flags |= SPR_FLAG_ALPHA;
//	SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 70.0f * 0.7f, MLOG_Y});
//  SPR_SetSizeSprite(work->title_veasy, 160.0f * 0.7f, 16.0f * 0.7f);

//	SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 80.0f * 0.7f, MLOG_Y});

//	SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 82.0f * 0.7f, MLOG_Y});
//  SPR_SetSizeSprite(work->title_veasy, 192.0f * 0.7f, 16.0f * 0.7f);
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_veasy, &(SPR_POS){256.0f - 112.0f * 0.7f, MLOG_Y});
#else
	{
		SPR_POS tmp = { 256.0f - 112.0f * 0.7f, MLOG_Y } ;
		SPR_SetPosSprite(work->title_veasy, &tmp );
	}
#endif
	SPR_SetSizeSprite(work->title_veasy, 256.0f * 0.7f, 16.0f * 0.7f);

	work->title_vhard = SPR_Create_2D_Object(SP_SPRITE, 0, NULL);
    if(work->title_vhard == NULL){ printf("ERR!! Make2DObj[work->title_vhard]\n"); return -1; }
    SPR_ObjSetTexture(work->title_vhard, TEX_CODE_STR, 0);
    work->title_vhard->head.alpha = SCE_GS_SET_ALPHA(0, 1, 0, 1, 64);
    work->title_vhard->head.flags |= SPR_FLAG_ALPHA;
#ifdef BP_PS2
	SPR_SetPosSprite(work->title_vhard, &(SPR_POS){256.0f - 50.0f * 0.7f, SPECIAL_Y});
#else
	{
		SPR_POS tmp = { 256.0f - 50.0f * 0.7f, SPECIAL_Y } ;
		SPR_SetPosSprite(work->title_vhard, &tmp );
	}
#endif
    SPR_SetSizeSprite(work->title_vhard, 160.0f * 0.7f, 16.0f * 0.7f);


    SetSpriteUV( (SPR_SPRITE*)work->title_vhard, 0, 16*21, 168, 16  );
    SET_COLOR_2DPRIM(work->title_vhard,0x00202020);
    SetSpriteUV( (SPR_SPRITE*)work->title_veasy, 0, 16*20, 256, 16  );
    SET_COLOR_2DPRIM(work->title_veasy,0x00202020);

    SetSpriteUV( (SPR_SPRITE*)work->title_start, 0, 0, 168, 16  );
    SET_COLOR_2DPRIM(work->title_start,0x00202020);
	SetSpriteUV( (SPR_SPRITE*)work->title_option, 0, 16, 168, 16  );
	SET_COLOR_2DPRIM(work->title_option,0x00202020);
	SetSpriteUV( (SPR_SPRITE*)work->title_easy, 0, 16*14, 168, 16  );
	SET_COLOR_2DPRIM(work->title_easy,0x00202020);

//	SPR_HIDE(work->title_start);

	SPR_HIDE(work->title_japan);

	//SPR_SHOW(work->title_logo);
	//SPR_SHOW(work->title_sol);
	//SPR_SHOW(work->title_trial);
	//SPR_SHOW(work->title_kcej);
	SPR_SHOW(work->title_fade);
	SPR_HIDE(work->title_easy);
	SPR_HIDE(work->title_veasy);
	SPR_HIDE(work->title_vhard);
	//SPR_SHOW(work->title_option);
    //SPR_SHOW(work->title_start);

	SPR_SetPriority( work->title_logo, 1);
	SPR_SetPriority( work->title_kcej, 1);
	SPR_SetPriority( work->title_sol,  1);
	SPR_SetPriority( work->title_trial,  1);

	SPR_SetPriority( work->title_start, 3);
	SPR_SetPriority( work->title_option, 3);
	SPR_SetPriority( work->title_easy, 3);
	SPR_SetPriority( work->title_vhard, 3);
	SPR_SetPriority( work->title_veasy, 3);

	SPR_SetPriority( work->title_fade, 5);
	SPR_SetPriority( work->title_warning, 6);
	SPR_SetPriority( work->title_konami, 6);
	SPR_SetPriority( work->title_japan, 7);

	if(mode == 1){
		work->step = TTL_STEP_CAUTION;
		printf("タイトル体験版用（ワーニングあり）\n");
		SPR_SHOW(work->title_warning);
		SPR_HIDE(work->title_konami);
	}else if(mode == 3){

		printf("タイトル体験版用（スペシャルから）\n");
		work->step = TTL_STEP_FROM_SP;
		SET_COLOR_2DPRIM(work->title_fade,0x80000000);
		SET_COLOR_2DPRIM(work->title_logo,0x80808080);
		SET_COLOR_2DPRIM(work->title_sol,0x80808080);
		if(eng){
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_ENG);
		}else{
			//SET_COLOR_2DPRIM(work->title_trial,(128<<24)|TRIAL_COLOR_JPN);
		}
		SET_COLOR_2DPRIM(work->title_kcej,0x80606060);

		SPR_SHOW(work->title_start);
		SPR_SHOW(work->title_option);
		SPR_SHOW(work->title_veasy);
		SPR_SHOW(work->title_vhard);

		SPR_SHOW(work->title_logo);
		SPR_SHOW(work->title_sol);
		SPR_SHOW(work->title_trial);
		SPR_SHOW(work->title_kcej);
		SPR_SHOW(work->title_fade);
		SET_COLOR_2DPRIM(work->title_start,0x80505a40);
		SET_COLOR_2DPRIM(work->title_option,0x80505a40);
		SET_COLOR_2DPRIM(work->title_veasy,0x80505a40);
		SET_COLOR_2DPRIM(work->title_vhard,0x801e28a0);
		DG_COPY_VEC(&work->camera->position,&CmrPosList[1] );
		DG_COPY_VEC(&work->camera->target,&TrtPosList[1] );

		GM_SetCameraQuick( 0 );
		GM_SdSet(SNG_PLAY_01);

		work->which = 3;
		SPR_HIDE(work->title_warning);
		SPR_HIDE(work->title_konami);
	}else{
		work->step = TTL_STEP_KONAMI;
		//GM_SdSet(SNG_PLAY_01);
		printf("タイトル体験版用（ワーニングなし）\nタイトルのモードを１にすると起動\n");
		SPR_HIDE(work->title_warning);
		SPR_SHOW(work->title_konami);
	}

    GM_SetGameStatus( STATE_PAUSE_DISABLE );

	{
		extern void *NewTraffic_Demo( int name, float z_crip, int max_car, int f_speed );
		//NewTraffic_Demo( GV_StrCode("（株）Ｇ・Ｗ交通"), 0.0f, 64, 320 );
	}
	DG_COPY_VEC(&work->camera->position,&CmrPosList[1] );
	DG_COPY_VEC(&work->camera->target,&TrtPosList[1] );

	GM_SetCameraQuick( 0 );
	work->mlog_work = NewTextScreenControl();
	if(!work->mlog_work) return -1;

	if(work->clear_code[0] != '\0'){
		// ろぐ初期化
		MENU_ClearTextTexture( work->mlog_work );
        // 説明分テキストを展開する
		work->font_wide = MENU_CreateTextTexture( work->mlog_work, 1, 1,
												  FONT_BUFFER_WIDTH(14,0)+2, FONT_BUFFER_HEIGHT(1,0)+2,
												  0, 0, 0, work->clear_code );
	}

	return 0;
}

void *NewGameTitle_Trial( int name, int where )
{
	Work		*work;

	work = (Work*)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
								  sizeof( Work ), ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	printf("Title Start\n");
	return work ;
}

