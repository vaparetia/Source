//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  	vr_gmover.c
	VR用ゲームオーバー画面
	2002/04/24 Y.Yano
	
	$Id: vr_gmover.c,v 1.4 2002/11/23 12:36:06 Yoshizawa1 Exp $
*/

/* TODO:::::
   コンティニューのないゲームオーバー画面
command ＶＲコンティニュー禁止
で
GM_VRStatus |= GM_VR_NO_CONTINUE ;
となるので、
if(GM_VRStatus & GM_VR_NO_CONTINUE)
ってなかんじでよろしこー（≧ｗ≦）ﾉ
   


ゲームオーバーEXITの行き先が特別指定のフラグをつくりました
scn/command.def
source/game
source/user/sigeno
をupdateしてください。

シナリオから
command セットＶＲステータス d:VR_EXIT_TO_SPECIAL	
でフラグセットすると
プログラマからは
if(GM_VRStatus & GM_VR_EXIT_TO_SPECIAL)
で判定できます。

 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

extern void ShowExitGameWarning(int *pResult);

/*************************************************************************************************/
/* 特殊ゲームオーバー用呼び出しプロックid記憶 */
static int g_proc_id = 0;
void COMVRGameOverProc( int mode ){
	if( GCL_GetOption( 'p' ) != NULL ){
		if( g_proc_id != 0 ){
			printf("Warning:特殊ＶＲゲームオーバーを２回呼んでいます\n");
			//ASSERT( 0 );
		}
		g_proc_id = GCL_GetNextInt();	
		printf("特殊ＶＲゲームオーバー proc[%d]\n",g_proc_id);
	}
	return;
}


/**************************************************************************************************/
#include 	"../include/dmatags.h"
#include 	"../../shibata/util/dma_set.h"

/* L2D アクション、オブジェクト名strcode #defineヘッダ */
#include	"vr_geme_over.h"

//powoeroff phaseで使用 
#define 	B_VERTS 4
#define		NOISE_CLAMP	(SCE_GS_SET_CLAMP( 2, 2, 0, DRAW_WIDTH, 0, DRAW_HEIGHT ))
#define		BG_CLAMP	(SCE_GS_SET_CLAMP( 2, 2, 0, DRAW_WIDTH, 0, DRAW_HEIGHT ))
#define 	TIME_NOISE 40
#define 	TIME_SHAKE 10
#define 	TIME_CLOSE 10
#define 	TIME_LAST  100
#define 	LAST_SLIT  1.0f
#define 	LAST_FLEN  100.0f
//#ifdef KP_XBOX //単色ノイズで使用
#define NOISE2_TEX_WIDTH 	384
#define NOISE2_TEX_HEIGHT 	384
#define NOISE2_WIDTH	256
#define NOISE2_HEIGHT	256
#define NOISE2_RANDU	(NOISE2_TEX_WIDTH-NOISE2_WIDTH)
#define NOISE2_RANDV	(NOISE2_TEX_HEIGHT-NOISE2_HEIGHT)
//#endif


//l2d phaseで使用
#define 	POSITION_CONTI 0
#define 	POSITION_EXIT  1
#define		TO_RESTART 0
#define		TO_RELOAD  1

extern int BP_FRAMES_PER_SEC();
#define SEC_FRAME (BP_FRAMES_PER_SEC())

#define		SO_NUM 		(6+1)
#define		SO_TEX_NUM 	18
#define		SO_HEIGHT 	65.0f
#define 	SO_TEX_STRCODE0 GV_StrCode("moji_001_alp_ovl")
#define 	SO_TEX_STRCODE1 GV_StrCode("moji_002_alp_ovl")
#define 	SO_TEX_STRCODE2 GV_StrCode("moji_003_alp_ovl")
#define 	SO_TEX_STRCODE3 GV_StrCode("moji_004_alp_ovl")
#define 	SO_TEX_STRCODE4 GV_StrCode("moji_005_alp_ovl")
#define 	SO_TEX_STRCODE5 GV_StrCode("moji_006_alp_ovl")
#define 	SO_TEX_STRCODE6 GV_StrCode("moji_007_alp_ovl")
#define 	SO_TEX_STRCODE7 GV_StrCode("moji_008_alp_ovl")
#define 	SO_TEX_STRCODE8 GV_StrCode("moji_009_alp_ovl")
#define 	SO_TEX_STRCODE9 GV_StrCode("moji_010_alp_ovl")
#define 	SO_TEX_STRCODE10 GV_StrCode("moji_011_alp_ovl")
#define 	SO_TEX_STRCODE11 GV_StrCode("moji_012_alp_ovl")
#define 	SO_TEX_STRCODE12 GV_StrCode("moji_013_alp_ovl")
#define 	SO_TEX_STRCODE13 GV_StrCode("moji_014_alp_ovl")
#define 	SO_TEX_STRCODE14 GV_StrCode("moji_015_alp_ovl")
#define 	SO_TEX_STRCODE15 GV_StrCode("moji_016_alp_ovl")
#define 	SO_TEX_STRCODE16 GV_StrCode("moji_017_alp_ovl")
#define 	SO_TEX_STRCODE17 GV_StrCode("moji_018_alp_ovl")


#ifdef BP_PS2
extern char _program_top[];
#else
static char _program_top[16];
#endif

#define SAND_ADDR  ((void *)( (int)(_program_top + 0x00008000) & 0x0fffff00 )) /* 128バイト境界 */
#define SAND_PSM   SCE_GS_PSMCT16

#define FLAG_SHOW_MOJI   0x01
#define FLAG_PRINT_START 0x02
#define FLAG_SELECT_OK   0x04

enum {
	PHASE_POWEROFF = 0,
	PHASE_SELECTL2D,
};
enum {
	PO_PHASE_NOISE = 0,
	PO_PHASE_SHAKE,
	PO_PHASE_CLOSE,
	PO_PHASE_LAST,
};
enum {
	L_PHASE_START = 0,
	L_PHASE_START_WAIT,
	L_PHASE_CHARA_DISP,
	L_PHASE_CHARA_DISP_WAIT,
	L_PHASE_SELECT,
	L_PHASE_OUT,
	L_PHASE_OUT_WAIT,
};
enum {
	SCRL_INIT = 0,
	SCRL_MOVE_START,
	SCRL_MOVE_NEW,
	SCRL_MOVE,
};

typedef struct vr_gmover_POWEROFF_PACK_XBOX{
	/* RGBノイズ */
	DG_DMAPACK_TEX 		rgb_noise_tex; //DG_SetDmapackTexLin();
	DG_DMAPACK_ALPHA 	rgb_noise_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_SPRT 	rgb_noise_sprt; //DG_SetDmapackSprt();
	/* 画面変化 */
	DG_DMAPACK_PARAM	frame_backup; //DG_SetDmapackBackupFrame();
	DG_DMAPACK_PARAM	frame_use; //DG_SetDmapackUseFrameTex();
	DG_DMAPACK_ALPHA 	frame_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_TRIANGLESTRIP frame_triangle; //DG_SetDmapackTriangleStrip();
	DG_DMAPACK_VERTEX	frame_vertex[ B_VERTS ]; //DG_SetDmapackVertex();
	/* 白フェード */
	DG_DMAPACK_ALPHA 	white_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_BOX 		white_sprt; //DG_SetDmapackBox();
	/* 黒帯 */
	DG_DMAPACK_ALPHA 	black_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_BOX 		black_sprt[2]; //DG_SetDmapackBox();
	/* 黒フェード */
	DG_DMAPACK_ALPHA 	fade_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_TRIANGLESTRIP fade_triangle0; //DG_SetDmapackTriangleStrip();
	DG_DMAPACK_VERTEX	fade_vertex0[6]; //DG_SetDmapackVertex();
	DG_DMAPACK_TRIANGLESTRIP fade_triangle1; //DG_SetDmapackTriangleStrip();
	DG_DMAPACK_VERTEX	fade_vertex1[6]; //DG_SetDmapackVertex();
	/* 単色ノイズ */
	DG_DMAPACK_TEX 		mono_noise_tex; //DG_SetDmapackTexLin();
	DG_DMAPACK_ALPHA 	mono_noise_alpha; //DG_SetDmapackAlpha();
	DG_DMAPACK_SPRT 	mono_noise_sprt; //DG_SetDmapackSprt();	

	DG_DMAPACK_TAG		end; //DG_SetDmapackEnd();
} POWEROFF_PACK_XBOX;


/*
  user/shibata/util/dma_set.h に記述
  //フレーム退避用
  typedef struct _tr_buffer {
	DG_GIFTAG		giftag ;
	struct _tr_buffer_data{
		DG_GSREG	texflush0;
		DG_GSREG	bitbltbuf;
		DG_GSREG	trxpos;
		DG_GSREG	trxreg;
		DG_GSREG	trxdir;
		DG_GSREG	texflush1;
	} data ;
} TR_BUFFER;
*/



/*
 * Work
 */
typedef struct vr_gmover_POWEROFF_WORK{
	DG_DMAPACK		*dmapack;
	POWEROFF_PACK_XBOX	*draw_packet;
	void *prim_x;
	int  noise_alpha;
	int  noise_alpha2;
	int  white_alpha;
	DG_TEX_LIN *noise_tex;
	DG_TEX_LIN *noise_tex2;
	//void *prim;
	int phase;
	int cnt;

	int n_cnt;
	int s_cnt;
	int c_cnt;
	int l_cnt;

	float u_pos[2];
	int p_m[2];

	float f_alpha;

	////TODO: シナリオ渡し-->最終的にはdefineに
/*
	int tmp;//test汎用変数
*/
	////デバック用関数

} POWEROFF_WORK;
typedef struct vr_gmover_SELECTL2D_WORK{
	int handle_2d;
	SPR_OBJ *root;
	SPR_OBJ *conti;
	SPR_OBJ *exit;
	SPR_OBJ *m_failed;
	SPR_OBJ *cursor;
	SPR_OBJ *cursor2;
	SPR_OBJ *cursor3;
	SPR_OBJ *cursor4;
	SPR_OBJ *cursor5;
	SPR_OBJ *nul_moji;
	SPR_OBJ *nul_moji2;
	SPR_OBJ *nul_conti;
	SPR_OBJ *nul_exit;
	SPR_OBJ *nul_source;
	SPR_OBJ *source[SO_NUM];
	SPR_OBJ *fade;
	

	int phase;	
	int action_num;
	int action;
	int cnt;
	int position;
	int todo;

	void *menuprint_work_ptr;
	char text_buffer[ 256 ];
	char text_buffer2[ 128 ];
	int  text_len;
	int  text_len2;
	int  text_cnt;
	int  text_alpha;

	int  c_alpha;
	int  c_alpha_add;
	int  fade_alpha;

	int   	scrl_phase;
	float  	scrl_pos;
	float  	scrl_cnt;
	int		scrl_yasumi;
	float	scrl_alpha;
	char	narabi;
	int 	so_tex[SO_TEX_NUM];
	int		pic_handle;
	
	//////test
	int test_cnt;
} SELECTL2D_WORK;
typedef struct vr_gmover_Work{
	GV_ACT_EX		actor ;
	int				timer;
	int 			phase;
	POWEROFF_WORK	powork;
	SELECTL2D_WORK  l2dwork;

	int 			flag;
	int				stream_handler;
	int				stream_count;
#ifdef KP_XBOX
	int 			confirm;	/* スネークテイルズの時EXIT確認画面を出すときに使う変数 */
#endif

#ifdef DEBUG_MODE
	int mode;
#endif
} Work;

extern int VR_TIME ;
extern int VR_TIME_Original;
extern int VR_TARGET_NUM ;
extern int VR_TARGET_MAX ;
extern int VR_ENEMY_NUM ;
extern int VR_ENEMY_MAX ;
extern int VR_BOMBS_NUM ;
extern int VR_BOMBS_MAX ;
//vr.h
//extern int VR_RetryCount ;		//そのステージをリトライした回数
//extern int VR_ContinueCount ;	//そのステージをコンティニューした回数
#include "../../yamashita/2D/msn.h"


static void *InitWHITESprtData( void *addr, int n_verts );
static void *InitBLACKSprtData( void *addr, int n_verts );
static void *InitFADESprtData( void *addr, int n_verts );
static void InitPowerOffPacket_XBOX( Work *work, POWEROFF_PACK_XBOX *packet );


static void VoxStreamPlay( Work *work )
{
	if ( GM_GameOverVox >= 0 ){
		work->stream_handler = GM_VoxStream( GM_GameOverVox , GM_STREAM_NO_PAUSE );
		if ( work->stream_handler < 0 ){
			printf("Stream Retry---------- %d\n" , work->stream_handler );
			if ( work->stream_count >= DIRECT_TICK( 60 ) ){
				printf("Stream Retry Time Over---------- %d\n" , work->stream_handler );
				work->stream_handler = -1 ;
			} else {
				work->stream_count++;
			}
		} else {
			printf("Stream Play---------- %d\n" , work->stream_handler );
			//work->act = ( void * )SK_StreamControl; // 復帰
		}
	} else {
		printf("Stream No Play---------- Err Call Koba4\n");
		work->stream_handler = -1 ;
	}
	
	
}

static void LCharacterDispInit( Work *work, SELECTL2D_WORK *lwork ){
	char name[20];
	char time[20];
	char break_num[50];
	char retry[15];
	char conti[20];
	char play_time[30];
	char other[60];
	int frame_cnt, sec, sec2, min, min2, hour, hour2, tmp;

	if(GM_VRStatus & GM_VR_SNAKETALES) {
		/* スネークテイルズの時は、文字表示をしない */
		return ;
	}

	/* プレイヤーの名前 */
	switch( GM_AnotherPlayer ){
	  case GM_ANOTHER_PLAYER_SNAKE:
		sprintf( name, "SNAKE\n" );
		break;
	  case GM_ANOTHER_PLAYER_RAIDEN:
		sprintf( name, "RAIDEN\n" );
		break;
	  case GM_ANOTHER_PLAYER_OLD_SNAKE:
		sprintf( name, "MGS1-SNAKE\n" );
		break;
	  case GM_ANOTHER_PLAYER_PLISKIN:
		sprintf( name, "PLISKIN\n" );
		break;
	  case GM_ANOTHER_PLAYER_TUXEDO:
		sprintf( name, "TUXEDO-SNAKE\n" );
		break;
	  case GM_ANOTHER_PLAYER_BLADE:
		sprintf( name, "BLADE-RAIDEN\n" );
		break;
	}		
	/* mode, level はとりあえず保留 */

	/* 死んだ時のタイム */
	frame_cnt = VR_TIME;
	tmp = frame_cnt * 100 / SEC_FRAME;/* 切り捨て */
	sec = tmp % 100;
	sec2 = sec % 10;
	sec  = sec / 10;
	tmp = tmp / 100; 
	min = tmp % 60;
	min2 = min % 10;
	min = min / 10;
	tmp = tmp / 60;
	hour = tmp % 100;
	tmp = tmp / 100;
	if( tmp > 0 ){
		hour = 99;
	}
	hour2 = hour % 10;
	hour = hour / 10;
	sprintf( time, "TIME %d%d:%d%d:%d%d\n", hour, hour2, min, min2, sec, sec2 );/* 表示しているのは「分/秒/100分の1秒」 */
	
	/* 破壊したものの数 */
	sprintf( break_num, "TARGETS:%d / ENEMIES:%d / BOMBS:%d\n", VR_TARGET_NUM, VR_ENEMY_NUM, VR_BOMBS_NUM );

	/* リトライ回数 */
	sprintf( retry, "RETRY    %d\n", MSN_RETRY_COUNT );
	
	/* コンティニュー回数 */
	// AS(JM) - Removed \x82 to fix continue string
   sprintf( conti, "CONTINUE %d\n", MSN_CONTINUE_COUNT );
//   sprintf( conti, "CONTINUE \x82%d\n", MSN_CONTINUE_COUNT );

   /* プレイタイム */    
   frame_cnt = GM_PlayTime;
   tmp = frame_cnt / SEC_FRAME;/* 切り捨て */
   sec = tmp % 60;
   tmp = tmp / 60; 
   min = tmp % 60;
   tmp = tmp / 60;
   hour = tmp % 100;
   tmp = tmp / 100;
   if( tmp > 0 ){
      hour = 99;
      min = 59;
      sec = 59;
   }
   sec2 = sec % 10;
   sec  = sec / 10;
   min2 = min % 10;
   min = min / 10;
   hour2 = hour % 10;
   hour = hour / 10;
   sprintf( play_time, "PLAY TIME %d%d:%d%d:%d%d\n\n", hour, hour2, min, min2, sec, sec2 );
	
	/* その他 */
	sprintf( other, "X=%6d Y=%6d Z=%6d\nX=%6d Y=%6d Z=%6d\n", 
			 GM_PlayerPosX, GM_PlayerPosY, GM_PlayerPosZ, 
			 GM_CameraX, GM_CameraY, GM_CameraZ );

	sprintf( lwork->text_buffer, "%s%s%s%s%s%s", name, time, break_num, retry, conti, play_time /*, other*/ );
	strcpy( lwork->text_buffer2,  other );

	{	/* 文字数検出 */
		int len;
		len = strlen( lwork->text_buffer );
		if( len > 256 ){
			ASSERT( 0 );
		}
		printf("len = [%d]\n", len );
		lwork->text_len = len;
		len = strlen( lwork->text_buffer2 );
		if( len > 128 ){
			ASSERT( 0 );
		}
   		printf("len2 = [%d]\n", len );
		lwork->text_len2 = len;
		//printf("%sBBB", lwork->text_buffer );
	}
	lwork->text_alpha = 128;
	
	
}
static void LCharacterDisp( Work *work, SELECTL2D_WORK *lwork ){
	SPR_OBJ *chara;
	int x, y, x2, y2;
	char disp[256], disp2[128];

	if(GM_VRStatus & GM_VR_SNAKETALES) {
		/* スネークテイルズの時は、文字表示をしない */
		return ;
	}

	chara = lwork->nul_moji;
	x = (int)(SPR_SCALE_X( DG_CHANL_MENU, chara->empty.pos.x ));/* 仮想座標->実座標 */
	y = (int)(SPR_SCALE_Y( DG_CHANL_MENU, chara->empty.pos.y ));
	chara = lwork->nul_moji2;
	x2 = (int)(SPR_SCALE_X( DG_CHANL_MENU, chara->empty.pos.x )); /* 仮想座標->実座標 */
	y2 = (int)(SPR_SCALE_Y( DG_CHANL_MENU, chara->empty.pos.y ));
	//printf("x[%d]y[%d]\n", x, y );

	lwork->text_cnt += 1;
	if( lwork->text_cnt <= lwork->text_len ){ /* 1文字づつ表示するためよう */
		strncpy( disp, lwork->text_buffer, lwork->text_cnt );
		disp[ lwork->text_cnt ] = '\0';
		disp2[ 0 ] = '\0';
		//strcat( disp, "\0" );

		GM_SdSet( SD_S_GO_TYPE1 );

	}else if( lwork->text_cnt <= (lwork->text_len + lwork->text_len2) ){ /* 下のほうの文字 */
		int len;
		len = lwork->text_cnt - lwork->text_len;
		strcpy( disp, lwork->text_buffer );
		strncpy( disp2, lwork->text_buffer2, len );
		disp2[ len ] = '\0';		

		GM_SdSet( SD_S_GO_TYPE1 );

	}else{
		strcpy( disp, lwork->text_buffer );
		strcpy( disp2, lwork->text_buffer2 );
	}
	
	//___MENU_Color( lwork->menuprint_work_ptr, 1, 100, 100, 100, lwork->text_alpha );
	___MENU_Color( lwork->menuprint_work_ptr, 1, 50, 50, 50, lwork->text_alpha );
	
	___MENU_Locate( lwork->menuprint_work_ptr, 1, x, y, 0 /* 左詰め */ );//上
  	___MENU_Print( lwork->menuprint_work_ptr, 1, disp );
	___MENU_Locate( lwork->menuprint_work_ptr, 1, x2, y2, 0	/* 左詰め */ );//下
  	___MENU_Print( lwork->menuprint_work_ptr, 1, disp2 );
	
	return;
}

static void LSelect( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	SELECTL2D_WORK *lwork = &work->l2dwork;	
	SPR_OBJ *conti = lwork->conti;
	SPR_OBJ *exit = lwork->exit;
	SPR_OBJ *cursor = lwork->cursor;
	//SPR_OBJ *cursor2 = lwork->cursor2;
	SPR_OBJ *cursor3 = lwork->cursor3;
	//SPR_OBJ *cursor4 = lwork->cursor4;
	SPR_OBJ *cursor5 = lwork->cursor5;
	SPR_OBJ *c_conti = lwork->nul_conti;
	SPR_OBJ *c_exit = lwork->nul_exit;
	int stat;

	GM_GameOverClear();/* パッドリリース等解除 */

	stat = L2D_ActionStatus( lwork->handle_2d );
	if( stat < 0 ){
		return ;
	}

	/* カーソル点滅処理 */
	lwork->c_alpha += lwork->c_alpha_add;
	if( lwork->c_alpha > 128 ){
		lwork->c_alpha = 128;
		lwork->c_alpha_add = (-lwork->c_alpha_add);
	}
	if( lwork->c_alpha < 0 ){
		lwork->c_alpha = 0;
		lwork->c_alpha_add = (-lwork->c_alpha_add);
	}

	if( lwork->position == POSITION_CONTI ){/* continueにカーソルがある */
	    // AS - Commented out since the layout was fixed in data. (e.g. all animations are constructed in data)
		//SPR_SetPosSprite( cursor, &c_conti->empty.pos );
		SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, lwork->c_alpha );
		SPR_SetColorSprite( cursor3, cursor3->sprite.col.r, cursor3->sprite.col.g, cursor3->sprite.col.b, lwork->c_alpha);
		SPR_SHOW( cursor3 );		  
		if( GV_PadDataDirect[ 0 ].press & PAD_OK ){
			lwork->todo = TO_RESTART ;/* Restart */
			lwork->phase = L_PHASE_OUT;
			SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, 128 );
			SPR_SetColorSprite( cursor3, cursor3->sprite.col.r, cursor3->sprite.col.g, cursor3->sprite.col.b, 128);

			powork->f_alpha = 0.0f;/* ノイズが残っているといけないので */

			GM_SdSet( SD_S_START001 );
			MSN_CONTINUE_COUNT ++;
		}
		if( GV_PadDataDirect[ 0 ].press & PAD_R ){
			lwork->position = POSITION_EXIT;/* exit に */
			L2D_EvokeAction( lwork->handle_2d, 9875614 /* move_icon */);
			lwork->c_alpha = 128;/* 一番明るく */

			lwork->scrl_pos += 30.0f;
			
			GM_SdSet( SD_S_CUR01 );

		}
	} else {/* exitにカーソルがある */
   	    // AS - Commented out since the layout was fixed in data. (e.g. all animations are constructed in data)
		//SPR_SetPosSprite( cursor, &c_exit->empty.pos );
		SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, lwork->c_alpha );
		SPR_SetColorSprite( cursor5, cursor5->sprite.col.r, cursor5->sprite.col.g, cursor5->sprite.col.b, lwork->c_alpha);
		SPR_HIDE( cursor3 );
		SPR_SHOW( cursor5 );

		if( GV_PadDataDirect[ 0 ].press & PAD_OK ){
#ifdef KP_XBOX
			if( !(GM_VRStatus & GM_VR_EXIT_TO_SPECIAL) && (GM_VRStatus & GM_VR_SNAKETALES) ){
				/* スネークテイルズの時のみEXIT確認画面を出す */
				/* confirm::: 0:起動前 -1:起動中 1:YES選択 2:NO選択フェード始め 3:NO選択のち確認画面死んだ */
				extern void *NewGameOverExitConfirm( int *confirm  );
				void *res;
				res = NewGameOverExitConfirm( &work->confirm );
				if( res != NULL ){
					GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
					return;
				}
			}
#endif
         //BP - added system dialog on X360 to prevent destructive action
         //without confirmation.
         int bp_doFinish = 1;
         int confirmRet;
         ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
         }
         else
         {
            //Pretend the user did not ever hit the button.
            bp_doFinish = 0;
         }

         if( bp_doFinish )
         {
			   lwork->todo = TO_RELOAD ;/* Reload */
			   lwork->phase = L_PHASE_OUT;
			   SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, 128 );
			   SPR_SetColorSprite( cursor5, cursor5->sprite.col.r, cursor5->sprite.col.g, cursor5->sprite.col.b, 128 );

			   powork->f_alpha = 0.0f;/* ノイズが残っているといけないので */

			   GM_SdSet( SD_S_START01 );
         }
		}
		if( GV_PadDataDirect[ 0 ].press & PAD_L ){
			lwork->position = POSITION_CONTI;/* continue に */
			lwork->c_alpha = 128;/* 一番明るく */
			L2D_EvokeAction( lwork->handle_2d, 5295498 /* move_icon_01 */);
			SPR_HIDE( cursor5 );

			lwork->scrl_pos += 30.0f;

			GM_SdSet( SD_S_CUR01 );

		}
	}
	
}
static void LSelect2( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	SELECTL2D_WORK *lwork = &work->l2dwork;	
	//SPR_OBJ *conti = lwork->conti;
	SPR_OBJ *exit = lwork->exit;
	SPR_OBJ *cursor = lwork->cursor;
	//SPR_OBJ *cursor2 = lwork->cursor2;
	SPR_OBJ *cursor3 = lwork->cursor3;
	//SPR_OBJ *cursor4 = lwork->cursor4;
	SPR_OBJ *cursor5 = lwork->cursor5;
	//SPR_OBJ *c_conti = lwork->nul_conti;
	SPR_OBJ *c_exit = lwork->nul_exit;
	int stat;

	GM_GameOverClear();/* パッドリリース等解除 */

	stat = L2D_ActionStatus( lwork->handle_2d );
	if( stat < 0 ){
		return ;
	}

	/* カーソル点滅処理 */
	lwork->c_alpha += lwork->c_alpha_add;
	if( lwork->c_alpha > 128 ){
		lwork->c_alpha = 128;
		lwork->c_alpha_add = (-lwork->c_alpha_add);
	}
	if( lwork->c_alpha < 0 ){
		lwork->c_alpha = 0;
		lwork->c_alpha_add = (-lwork->c_alpha_add);
	}

	{/* exitにカーソルがある */
		SPR_SetPosSprite( cursor, &c_exit->empty.pos );
		SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, lwork->c_alpha );
		SPR_SetColorSprite( cursor5, cursor5->sprite.col.r, cursor5->sprite.col.g, cursor5->sprite.col.b, lwork->c_alpha);
		SPR_HIDE( cursor3 );
		SPR_SHOW( cursor5 );

		if( GV_PadDataDirect[ 0 ].press & PAD_OK ){
#ifdef KP_XBOX
			if( !(GM_VRStatus & GM_VR_EXIT_TO_SPECIAL) && (GM_VRStatus & GM_VR_SNAKETALES) ){
				/* スネークテイルズの時のみEXIT確認画面を出す */
				/* confirm::: 0:起動前 -1:起動中 1:YES選択 2:NO選択フェード始め 3:NO選択のち確認画面死んだ */
				extern void *NewGameOverExitConfirm( int *confirm  );
				void *res;
				res = NewGameOverExitConfirm( &work->confirm );
				if( res != NULL ){
					GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01);
					return;
				}
			}
#endif
         //BP - added system dialog on X360 to prevent destructive action
         //without confirmation.
         //NOTE: this case is kind of ridiculous-- it only comes up in Snake Tales, for example if
         //you start "A Wrongdoing" and immediately go back down the elevator.  The only option is Exit.
         //On the original release of Substance on XBox they dutifully added a confirmation dialog to this
         //screen even though it's pointless-- all you can do is change your mind about exiting the screen
         //that only has the option to exit.  We'll do the same thing here to match in any case.

         // SteveM
         // In this case the confirmation dialog is plain WRONG (see explanation kindly typed up above)
         // So I removed it.

         int confirmRet = 0;
         // ShowExitGameWarning( &confirmRet );
         if( confirmRet == 0 )   //yes
         {
            lwork->todo = TO_RELOAD ;/* Reload */
            lwork->phase = L_PHASE_OUT;
            SPR_SetColorSprite( cursor, cursor->sprite.col.r, cursor->sprite.col.g, cursor->sprite.col.b, 128 );
            SPR_SetColorSprite( cursor5, cursor5->sprite.col.r, cursor5->sprite.col.g, cursor5->sprite.col.b, 128 );

            powork->f_alpha = 0.0f;/* ノイズが残っているといけないので */

            GM_SdSet( SD_S_START01 );

         }
         else
         {
            //Pretend the user did not ever hit the button.
         }
		}
	}	
}
	 
static void ActSelectL2D( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	SELECTL2D_WORK *lwork = &work->l2dwork;

#if 1
	lwork->cnt ++ ;
	if( lwork->cnt == 23 ){
		VoxStreamPlay( work );/* ゲームオーバーBGM再生 */
	}
#endif

	switch( lwork->phase ){

	  case L_PHASE_START:

		lwork->phase = L_PHASE_START_WAIT;

		break;
	  case L_PHASE_START_WAIT:
		if( work->flag & FLAG_SHOW_MOJI ){
			powork->f_alpha = (float)powork->noise_alpha2;		
			lwork->phase = L_PHASE_CHARA_DISP;
		}
		break;
	  case L_PHASE_CHARA_DISP:/* 文字表示 */
		LCharacterDispInit( work, lwork );
		LCharacterDisp( work, lwork );

		lwork->phase = L_PHASE_CHARA_DISP_WAIT;
		break;
	  case L_PHASE_CHARA_DISP_WAIT:
		LCharacterDisp( work, lwork );
		if( work->flag & FLAG_SELECT_OK ){
			lwork->c_alpha = 128;/* カーソルの点滅の初期化 */
			lwork->c_alpha_add = 2;
			SPR_SHOW( lwork->cursor );
			SPR_SHOW( lwork->cursor2 );
			SPR_SHOW( lwork->cursor3 );
			//SPR_HIDE( lwork->cursor4 );
			SPR_HIDE( lwork->cursor5 );
			
			lwork->phase  = L_PHASE_SELECT;
		}
		break;		
	  case L_PHASE_SELECT:
		LCharacterDisp( work, lwork );
		if( GM_VRStatus & GM_VR_NO_CONTINUE ){
			/* EXITしかない */
			LSelect2( work );
		} else {
			/* EXITとCONTINUEを選ぶ */
			LSelect( work );
		}
		break;
	  case L_PHASE_OUT:
		/*
		  //再生終了後にRestart呼びだし
		  スプライトでフェード
		 */
		lwork->text_alpha -= 2;/* L2Dのフェードアウト時間に合わせる必要がある(L2d:300-->1秒) */
		lwork->fade = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, lwork->root );
		SPR_SET_FLAGS( lwork->fade, (SPR_FLAG_PRIV | SPR_FLAG_ALPHA) );
		SPR_ALPHA( lwork->fade, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		lwork->fade_alpha = 0;
		{ 
			SPR_POS pos = { 0.0f, 0.0f };
			SPR_SetPosSprite( lwork->fade, &pos );
		}
		SPR_SetSizeSprite( lwork->fade, (float)DRAW_WIDTH, (float)DRAW_HEIGHT );
		SPR_SetColorSprite( lwork->fade, 0, 0, 0, lwork->fade_alpha );
		SPR_SetPriority( lwork->fade, 7 );
		SPR_SHOW( lwork->fade );
		lwork->phase = L_PHASE_OUT_WAIT;
		LCharacterDisp( work, lwork );
		/* ストリームフェードアウト */
		if ( GM_StreamGetChannelHandler( 0 ) == work->stream_handler ) {
			GM_SdSet( STR1_FADE_OUT );
		} else if ( GM_StreamGetChannelHandler( 1 ) == work->stream_handler ) {
			GM_SdSet( STR2_FADE_OUT ) ;
		}
	  case L_PHASE_OUT_WAIT:
		lwork->text_alpha -= 2;/* L2Dのフェードアウト時間に合わせる必要がある(L2d:300-->1秒) */
		if( lwork->text_alpha < 1 ){/* alpha = 0 になると表示されてしまう仕様なので */
			lwork->text_alpha = 1;
		} else {
			LCharacterDisp( work, lwork );
		}
		lwork->fade_alpha += 2;
		SPR_SetColorSprite( lwork->fade, 0, 0, 0, lwork->fade_alpha );/* 黒スプライト */		

		lwork->scrl_pos += (int)(BP_PS2_rand()%10);
			
		if( lwork->fade_alpha > 128 ){
			/* ストリーム停止 */
			GM_StreamStopAll();

			if( (GM_VRStatus & GM_VR_EXIT_TO_SPECIAL) && (lwork->todo == TO_RELOAD) ){
				/* プロック呼び出し */
				if( g_proc_id != 0 ){
					/* プロック呼び出しゲームオーバー */
					GV_PauseOffActorSystem( GV_PAUSE_PAUSE | GV_PAUSE_MENU | GV_PAUSE_READERROR ) ;/* ポーズ解除 */
					GM_SdSet( SNG_PAUSEOFF ) ;
					GM_PrevArea = GM_SaveArea ;
					GM_GameOverFlag |= GM_OVERFLAG_LOADOK;
					
					GCL_ExecProc( g_proc_id, NULL );

				} else {
					printf("特殊ＶＲゲームオーバー を呼んでくれないと困るんです\n");
					GM_GameOverRestart( lwork->todo );
				}
				GV_DestroyActor( work );
			} else {
				/* 通常のゲームオーバーorコンティニュー */
				GM_GameOverRestart( lwork->todo );
				GV_DestroyActor( work );
			}
		}
		break;
	  default:
		printf("vr_gmover.c L_PHASE ERR!!\n");
		break;
	}
		

	/* にぎやかしテクスチャスクロール */
#define SCRL_STEP 1.0f
	if( work->flag & FLAG_PRINT_START ){
		//SPR_OBJ **so;
		SPR_POS pos;
		int k;
		int r;
		int handle;
		
		//so = lwork->source;
		pos = lwork->nul_source->empty.pos;/* 座標代入 */
		switch( lwork->scrl_phase ){
		  case SCRL_INIT:
			//printf("pos[%f][%f]\n",pos.x, pos.y);
			//SPR_SHOW( lwork->nul_source );

			handle = lwork->pic_handle = SPR_LoadTexture( GV_StrCode("game_over") );
			r = BP_PS2_rand();
			for( k = 0; k < SO_NUM; k ++ ){
				int res;
				res = r % SO_TEX_NUM;
				//lwork->narabi[k] = res;
				printf("narabi[%d]= %d\n",k,res);
				r /= SO_TEX_NUM;

				SPR_ObjSetTexture( lwork->source[k], lwork->so_tex[res], handle );
				SPR_SHOW( lwork->source[k] );
			}
			SPR_HIDE( lwork->source[0] );
			SPR_HIDE( lwork->source[1] );
			SPR_HIDE( lwork->source[2] );
			lwork->scrl_pos = 30.0f;

			lwork->narabi = 0;

			lwork->scrl_phase = SCRL_MOVE;
			break;
		  case SCRL_MOVE_NEW: /* 新しいものをランダムで決める */
			r = BP_PS2_rand();
			r = r % SO_TEX_NUM;
printf("tex[%d][%d]\n", r, lwork->so_tex[r]);
			handle = lwork->pic_handle;
			SPR_ObjSetTexture( lwork->source[lwork->narabi], lwork->so_tex[r], handle );
			SPR_SHOW( lwork->source[lwork->narabi] );
			lwork->narabi ++;
			if( lwork->narabi >= SO_NUM ){
				lwork->narabi = 0;
			}
			//printf("最後= %d\n",r);

			//lwork->scrl_pos = 0.0f;

			lwork->scrl_phase = SCRL_MOVE;
			//break;
		  case SCRL_MOVE:
#define SCRL_MAX_ALPHA 15
			SPR_SHOW( lwork->nul_source );
			for( k = 0; k < SO_NUM; k ++ ){
				SPR_OBJ *so;
				int n;

				n = lwork->narabi;
				n = n + k;
				if( n >= SO_NUM ){
					n = n - SO_NUM;
				}
				so = lwork->source[n];
				
				pos.y = -lwork->scrl_pos + SO_HEIGHT * k;
				SPR_SetPosSprite( so, &pos );
				SPR_SetColorSprite( so, 128, 128, 128, SCRL_MAX_ALPHA );				
				//SPR_SHOW( so );
			}
			if( lwork->scrl_pos >= SO_HEIGHT ){
				lwork->scrl_phase = SCRL_MOVE_NEW;
				lwork->scrl_pos -= SO_HEIGHT;
			}
			
			if( lwork->scrl_cnt >= lwork->scrl_yasumi  ){
				lwork->scrl_pos += SCRL_STEP * (BP_PS2_rand() % 15);
				lwork->scrl_yasumi = BP_PS2_rand() % 50;
				lwork->scrl_cnt = 0;
			}
			lwork->scrl_cnt ++;

			break;
		}

		if( lwork->scrl_alpha <= (float)SCRL_MAX_ALPHA ){
			int alp = (int)lwork->scrl_alpha;
			for( k = 0; k < SO_NUM; k ++ ){				
				SPR_SetColorSprite( lwork->source[k], 128, 128, 128, alp );				
			}
			lwork->scrl_alpha += 0.1f;
		}			


	}
		
		
		
	if( powork->noise_alpha2 > 0 ){ /* 単色ノイズ */
		// TODO: パレットは変更できないので、データをランダムに動かす
		void *prim = powork->prim_x;
		float u[2], v[2], ftemp;
		int col, alpha, r;
	
		r = BP_PS2_rand() % NOISE2_RANDU;
		u[0] = ( 0.5f + (float)r )/NOISE2_TEX_WIDTH;
		u[1] = ( 0.5f + (float)r + (float)NOISE2_WIDTH )/NOISE2_TEX_WIDTH;
		r = BP_PS2_rand() % NOISE2_RANDV;
		v[0] = ( 0.5f + (float)r )/NOISE2_TEX_HEIGHT;
		v[1] = ( 0.5f + (float)r + (float)NOISE2_HEIGHT )/NOISE2_TEX_HEIGHT;
		r = (BP_PS2_rand()) % 3;
		if( r == 0 ){		/* 左右反転 */
			ftemp = u[1];
			u[1] = u[0];
			u[0] = ftemp;
		} else if( r == 1 ){/* 上下反転 */
			ftemp = v[1];
			v[1] = v[0];
			v[0] = ftemp;
		}

		alpha = powork->noise_alpha2;
		if( lwork->phase >= L_PHASE_CHARA_DISP ){
			powork->f_alpha -= 0.05f;
			alpha = (int)powork->f_alpha;
			if( alpha < 0 ){
				alpha = 0;
				powork->f_alpha = 0.0f;
			}
		}
		powork->noise_alpha2 = alpha;
		col = 0x00808080 | ( alpha << 24 );

		prim = DG_SetDmapackTexLin( prim, powork->noise_tex2 );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, u[0], v[0],
								 (float)DRAW_WIDTH, (float)DRAW_HEIGHT, u[1], v[1], DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}
}


static void PONoise( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	{/* 単色ノイズのアルファ計算  */
		float ftemp;
		ftemp = (float)powork->n_cnt * (10.0f/*40.0f*/ / (float)(TIME_NOISE - 1));
		powork->noise_alpha2 = (int)ftemp;
	}
	{/* rgbノイズ */
		int r, alpha, col;
		float u[2], v[2];
		float ftemp;
		POWEROFF_PACK_XBOX *packet = powork->draw_packet;
		void *prim;
		//チラチラさせる
		r = (BP_PS2_rand()) % 3;
		if( r == 0 ){
			u[0] = 0.5f / DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f / DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 1 ){/* 左右反転 */
			u[1] = 0.5f / DRAW_WIDTH;
			u[0] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f /DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 2 ){/* 上下反転 */
			u[0] = 0.5f /DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[1] = 0.5f /DRAW_HEIGHT;
			v[0] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		}
		ftemp = 20.0f / (float)TIME_NOISE;
		ftemp = ftemp * (powork->n_cnt+1);
		alpha = (int)ftemp;
		col = 0x00808080 | ( alpha << 24 );
		powork->noise_alpha = alpha;
		prim = powork->prim_x;
		prim = DG_SetDmapackTexLin( prim, powork->noise_tex );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 16 ) );
		prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, u[0], v[0],
								 (float)DRAW_WIDTH, (float)DRAW_HEIGHT, u[1], v[1],
								 DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}
	//printf("NO !![%d:%d]\n",powork->n_cnt,powork->cnt);

   powork->n_cnt ++;
   powork->cnt ++ ;
#if BP_VITA
	powork->n_cnt ++;
	powork->cnt ++ ;
#endif
	if( powork->cnt >= TIME_NOISE ){
		powork->phase = PO_PHASE_SHAKE;
	}

}
static void POShake( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	{/* RGBノイズ */
		//チラチラさせる
		float u[2], v[2];
		int r;
		int alpha;
		void *prim;
		int col;

		r = (BP_PS2_rand()) % 3;
		if( r == 0 ){
			u[0] = 0.5f / DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f / DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 1 ){	/* 左右反転 */
			u[1] = 0.5f / DRAW_WIDTH;
			u[0] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f /DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 2 ){	/* 上下反転 */
			u[0] = 0.5f /DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[1] = 0.5f /DRAW_HEIGHT;
			v[0] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		}
		alpha = powork->noise_alpha;
		col = 0x00808080 | ( alpha << 24 );
		prim = powork->prim_x;
		prim = DG_SetDmapackTexLin( prim, powork->noise_tex );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 16 ) );
		prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, u[0], v[0],
								 (float)DRAW_WIDTH, (float)DRAW_HEIGHT, u[1], v[1], DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}
	{/* 画面変化 */
		//画面を横に伸ばしたり、揺らしたり(uv使用)
		float fr;
		float width, time;
		int s_cnt = powork->s_cnt;
		float pos0 = powork->u_pos[0], pos1 = powork->u_pos[1];
		void 	*prim;
		int 	i;
		float 	u[4], v[4];
		float 	h;
		
		time = (float)powork->s_cnt / (float)(TIME_SHAKE-11);
		if( powork->s_cnt >= (TIME_SHAKE - 11) ){
			time = 1.0f;
		}
		width = (1.0f-time)*(1.0f-time)*200.0f + 2.0f*time*(1.0f-time)*60.0f + time*time*20.0f;
		fr = 170.0f + (BP_PS2_rand())%30;
		v[0] = ((float)(DRAW_HEIGHT/2.0f - fr)) / DRAW_HEIGHT;
		v[1] = ((float)(DRAW_HEIGHT/2.0f - fr)) / DRAW_HEIGHT;
		v[2] = ((float)(DRAW_HEIGHT/2.0f + fr)) / DRAW_HEIGHT;
		v[3] = ((float)(DRAW_HEIGHT/2.0f + fr)) / DRAW_HEIGHT;
		if( powork->s_cnt < (TIME_SHAKE - 10) ){
			pos0 += 10.0f*powork->p_m[0];
			if( pos0 > (float)DRAW_WIDTH - width ){
				pos0 = (float)DRAW_WIDTH - width;
				powork->p_m[0] *= -1;
			}else if( pos0 < 0.0f ){
				pos0 = 0.0f;
				powork->p_m[0] *= -1;
			}
			//pos1 += 100.0f*powork->p_m[1];
			pos1 += 10.0f*powork->p_m[0]*(-1);
			if( pos1 > (float)DRAW_WIDTH ){
				pos1 = (float)DRAW_WIDTH;
				powork->p_m[1] *= -1;
			}else if( pos1 < width ){
				pos1 = width;
				powork->p_m[1] *= -1;
			}
			powork->u_pos[0] = pos0;
			powork->u_pos[1] = pos1;
		}
		fr = (float)(4-(BP_PS2_rand())%8);
		u[0] = ((float)( pos0 + fr )) / DRAW_WIDTH;
		u[1] = ((float)( pos0 + width + fr )) / DRAW_WIDTH;
		u[2] = ((float)( pos1 - width + fr )) / DRAW_WIDTH;
		u[3] = ((float)( pos1 + fr )) / DRAW_WIDTH;

		prim = powork->prim_x;
		prim = DG_SetDmapackBackupFrame( prim, 0 );/* 表示中のものを待避バッファへ */
		prim = DG_SetDmapackUseFrameTex( prim, 2 );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		prim = DG_SetDmapackTriangleStrip( prim, B_VERTS );
		h = (float)( DRAW_HEIGHT / (( B_VERTS / 2 ) - 1) );/* 縦の間隔 */
		for( i = 0; i < B_VERTS; i++ ){
			float x, y;
			
			if( i % 2 ){		/* 奇数 */
				x = (float)DRAW_WIDTH;
			} else {			/* 偶数 */
				x = 0.0f;
			}
			y = h*(float)((int)(i/2));
			prim = DG_SetDmapackVertex( prim, x, y, u[i], v[i], DG_MakeDmaPackColorFromInt(0x80808080) );
		}
		powork->prim_x = prim;
	}

#if BP_VITA
   powork->s_cnt ++;
   powork->cnt ++ ;
#endif
	powork->s_cnt ++;
	powork->cnt ++ ;
	if( powork->cnt >= TIME_NOISE+TIME_SHAKE ){
		powork->phase = PO_PHASE_CLOSE;

	}
}
static void POClose( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;
	int num, i;

	/* ノイズをのせる */
	{/* RGBノイズ */
		//チラチラさせる
		float u[2], v[2];
		int r;
		int alpha;
		void *prim;
		int col;

		r = (BP_PS2_rand()) % 3;
		if( r == 0 ){
			u[0] = 0.5f / DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f / DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 1 ){	/* 左右反転 */
			u[1] = 0.5f / DRAW_WIDTH;
			u[0] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[0] = 0.5f /DRAW_HEIGHT;
			v[1] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		} else if( r == 2 ){	/* 上下反転 */
			u[0] = 0.5f /DRAW_WIDTH;
			u[1] = (DRAW_WIDTH - 0.5f)/DRAW_WIDTH ;
			v[1] = 0.5f /DRAW_HEIGHT;
			v[0] = (DRAW_HEIGHT - 0.5f)/DRAW_HEIGHT ;
		}
		alpha = powork->noise_alpha;
		col = 0x00808080 | ( alpha << 24 );
		prim = powork->prim_x;
		prim = DG_SetDmapackTexLin( prim, powork->noise_tex );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 16 ) );
		prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, u[0], v[0],
								 (float)DRAW_WIDTH, (float)DRAW_HEIGHT, u[1], v[1], DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}

	{/* 画面変化 */
		//上の帯に合わせて上下を縮小して表示
		int r;
		float u[4], v[4];
		float fr;
		float pos, h, y[B_VERTS], time;
		void *prim;
		fr = 170.0f + (BP_PS2_rand())%30;
		v[0] = ((float)(DRAW_HEIGHT/2.0f - fr)) / DRAW_HEIGHT;
		v[1] = ((float)(DRAW_HEIGHT/2.0f - fr)) / DRAW_HEIGHT;
		v[2] = ((float)(DRAW_HEIGHT/2.0f + fr)) / DRAW_HEIGHT;
		v[3] = ((float)(DRAW_HEIGHT/2.0f + fr)) / DRAW_HEIGHT;
		num = B_VERTS;
		for( i = 0; i < num; i++ ){
			if( !(i%2) ){
				r = (BP_PS2_rand()) % (int)(20);
				u[i] = (float)r / DRAW_WIDTH;
			}else{
				u[i] = ( DRAW_WIDTH - (float)r ) / DRAW_WIDTH;
			}			
		}
		/* ノイズ画面を上下に圧縮 */
		time = (float)powork->c_cnt / (float)(TIME_CLOSE-1);
		if( powork->c_cnt == (TIME_CLOSE - 1) ){
			time = 1.0f;
		}
		pos = (1.0f-time)*(1.0f-time)*0.0f + 2.0f*time*(1.0f-time)*200.0f + time*time*(DRAW_HEIGHT/2.0f-LAST_SLIT);
		h = (DRAW_HEIGHT-pos*2) / (( B_VERTS / 2 ) - 1);/* 縦の間隔 */
		for( i = 0; i < B_VERTS; i ++ ){
			y[i] = pos + h*(float)((int)(i/2)) ;
		}

		prim = powork->prim_x;
		prim = DG_SetDmapackBackupFrame( prim, 0 ); /* 表示中のものを待避バッファへ */
		prim = DG_SetDmapackUseFrameTex( prim, 2 );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		prim = DG_SetDmapackTriangleStrip( prim, B_VERTS );
		h = (float)( DRAW_HEIGHT / (( B_VERTS / 2 ) - 1) ); /* 縦の間隔 */
		for( i = 0; i < B_VERTS; i++ ){
			float x;			
			if( i % 2 ){		/* 奇数 */
				x = (float)DRAW_WIDTH;
			} else {			/* 偶数 */
				x = 0.0f;
			}
			prim = DG_SetDmapackVertex( prim, x, y[i], u[i], v[i], DG_MakeDmaPackColorFromInt(0x80808080) );
		}
		powork->prim_x = prim;
	}	
	{/* 白くフェードアウト */
		int alpha, col;
		void *prim;
		alpha = (int)(powork->c_cnt * ( 80.0f / ((float)TIME_CLOSE - 1.0f)));
		powork->white_alpha = alpha;
		col = 0x00ffffff | ( alpha << 24 );
		prim = powork->prim_x ;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		prim = DG_SetDmapackBox( prim, 0.0f, 0.0f, (float)DRAW_WIDTH, (float)DRAW_HEIGHT, DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}		
	{ /* 黒い帯が閉じる */
		float time, pos;
		float x[2], y[2];
		int col;
		void *prim;
		
		time = (float)powork->c_cnt / (float)(TIME_CLOSE-1);
		if( powork->c_cnt == (TIME_CLOSE - 1) ){
			time = 1.0f;
		}
		pos = (1.0f-time)*(1.0f-time)*0.0f + 2.0f*time*(1.0f-time)*200.0f + time*time*(DRAW_HEIGHT/2.0f-LAST_SLIT);

		/* 黒い帯閉じ */
		prim = powork->prim_x;
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		x[0] = 0.0f;
		y[0] = 0.0f;
		x[1] = (float)DRAW_WIDTH;
		y[1] = (float)pos ;
		col = 0x80000000;
		prim = DG_SetDmapackBox( prim, x[0], y[0], x[1], y[1], DG_MakeDmaPackColorFromInt(col) );
		x[0] = 0.0f;
		y[0] = (float)DRAW_HEIGHT - pos;
		x[1] = (float)DRAW_WIDTH;
		y[1] = (float)DRAW_HEIGHT;
		col = 0x80000000;
		prim = DG_SetDmapackBox( prim, x[0], y[0], x[1], y[1], DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}
#if BP_VITA
   powork->c_cnt ++;
   powork->cnt ++ ;
#endif
	powork->c_cnt ++;
	powork->cnt ++ ;

	if( powork->cnt >= TIME_NOISE+TIME_SHAKE+TIME_CLOSE-1 ){
		/* アクション再生 */
		/* PS2で1フレ背後が見えてしまうのでここで再生 */
		if( GM_VRStatus & GM_VR_NO_CONTINUE ){
			/* EXITのないゲームオーバーのアクション */
			L2D_EvokeAction( work->l2dwork.handle_2d, 
							 L2D_vr_geme_over_ACT_show_mission_failed_exit );			
		} else {
			/* 通常のゲームオーバーのアクション */
			L2D_EvokeAction( work->l2dwork.handle_2d, 
							 GV_StrCode("show_mission_failed") );
		}
		SPR_SHOW( work->l2dwork.root );
	}
	
	if( powork->cnt >= TIME_NOISE+TIME_SHAKE+TIME_CLOSE ){

		powork->phase = PO_PHASE_NOISE;/* 最初に一応戻しておく */
		work->phase = PHASE_SELECTL2D;//l2d フェーズに移行
	}
}


static void ActPowerOff( Work *work )
{
	POWEROFF_WORK *powork = &work->powork;


//	 GM_SdSet( SD_A_V_NOIZE1 );


	switch( powork->phase ){
	  case PO_PHASE_NOISE:
		//printf("Noise\n");
		PONoise( work );
		break;
	  case PO_PHASE_SHAKE:
		//printf("Shake\n");
		POShake( work );
		break;
	  case PO_PHASE_CLOSE:
		//printf("Close\n");
		POClose( work );
		break;

	}

	/* 単色ノイズは一様にずっとかかっているので、ここで設定 */
	{ /* 単色ノイズ */
		// TODO: パレットは変更できないので、データをランダムに動かす
		void *prim = powork->prim_x;
		float u[2], v[2], ftemp;
		int col, alpha;
		int r;
	
		r = BP_PS2_rand() % NOISE2_RANDU;
		u[0] = ( 0.5f + (float)r )/NOISE2_TEX_WIDTH;
		u[1] = ( 0.5f + (float)r + (float)NOISE2_WIDTH )/NOISE2_TEX_WIDTH;
		r = BP_PS2_rand() % NOISE2_RANDV;
		v[0] = ( 0.5f + (float)r )/NOISE2_TEX_HEIGHT;
		v[1] = ( 0.5f + (float)r + (float)NOISE2_HEIGHT )/NOISE2_TEX_HEIGHT;
		r = (BP_PS2_rand()) % 3;
		if( r == 0 ){		/* 左右反転 */
			ftemp = u[1];
			u[1] = u[0];
			u[0] = ftemp;
		} else if( r == 1 ){/* 上下反転 */
			ftemp = v[1];
			v[1] = v[0];
			v[0] = ftemp;
		}

		alpha = powork->noise_alpha2;
//printf("noise2 alpha[%d]\n",alpha);
		col = 0x00808080 | ( alpha << 24 );
		//r = BP_PS2_rand() % 16384;
		prim = DG_SetDmapackTexLin( prim, powork->noise_tex2 );
		prim = DG_SetDmapackAlpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
		prim = DG_SetDmapackSprt( prim, 0.0f, 0.0f, u[0], v[0],
         (float)DRAW_WIDTH, (float)DRAW_HEIGHT, u[1], v[1], DG_MakeDmaPackColorFromInt(col) );
		powork->prim_x = prim;
	}
	
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
   /* XBOX用dmapackの毎フレーム処理 */
   POWEROFF_WORK *powork = &work->powork;
   powork->prim_x = powork->dmapack->autopacket;

#ifdef KP_XBOX
	/* スネークテイルズの時だけEXIT確認画面が出る */
	/* 確認画面中はパッドなどの情報をみる部分前にreturnさせる */
	if( work->confirm != 0 ){
		if( work->confirm == -1 ){
			/* 起動中 */
			return;
		} else if( work->confirm == 3 ){
			/* NO選択、確認画面から制御戻る */
			work->confirm = 0;
			return;
		}
		return;
	}
#endif
	switch( work->phase )
   {
	  case PHASE_POWEROFF:
		ActPowerOff( work );
		break;
	  case PHASE_SELECTL2D:
		ActSelectL2D( work );
		break;
	  default:
		printf("VRGameOver ERR\n");
		ASSERT(0);
		break;
	}

	work->powork.dmapack->flag &= ~DG_DMAPACK_INVISIBLEMENU ;
	DG_SetDmapackEnd( powork->prim_x );

	return ;
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	g_proc_id = 0;

	if(work->powork.dmapack){
		// ＤＭＡパケットオブジェクト開放
		DG_DequeueDmapack( work->powork.dmapack );
		DG_FreeDmapack( work->powork.dmapack );
	}
	if( work->powork.draw_packet ) {
		GV_Free( work->powork.draw_packet );
	}
	if( work->powork.noise_tex ){
		DG_FreeLinerTexture( work->powork.noise_tex );
	}

   if( work->powork.noise_tex2 )
   {
      DG_FreeLinerTexture( work->powork.noise_tex2 );
      work->powork.noise_tex2 = NULL;
   }
	
	/* L2Dデータの開放 */
	if( work->l2dwork.handle_2d >= 0 ){
		printf("vr_gmover.c L2d開放\n");
	    L2D_ReleaseLayout( work->l2dwork.handle_2d );
	}

	printf("vr_gmover.c DIE\n");

	//DG_SetPrivilegeMode( 0 );/* 特権モードOFF */

}
/* ---------------------------------------------------------------- */

#define SHOW_MOJI   10397277
#define PRINT_START 1581640
#define SELECT_OK   14798363
static void L2dSignal( void *pwork , int sign , int value )
{
	Work *work;

	work = ( Work * )pwork;

	switch ( sign ){
	case SHOW_MOJI:
		work->flag |= FLAG_SHOW_MOJI;
		break;
	case PRINT_START:
		work->flag |= FLAG_PRINT_START;
		break;
	case SELECT_OK:
		//work->dot.cursor->sprite.col.a = 128;
		work->flag |= FLAG_SELECT_OK;
		break;
		
	}
}


/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	/*
	   TODO:::::
	   
	   dmapack作り、設定
	   l2d素材ゲット
	   
	   */     
	
	//DG_SetPrivilegeMode( 1 );/* 特権モードON */		
	
	/*
	   Workの初期化
	   */
	work->timer = 0;
	work->phase = PHASE_POWEROFF;
	work->stream_handler = -1;
	
	
	{	/*
		   PowerOffWork の初期化
		   */
		POWEROFF_WORK 	*powork = &work->powork;
		
		powork->phase = PO_PHASE_NOISE;
		

		
		powork->n_cnt = 0;
		powork->s_cnt = 0;
		powork->c_cnt = 0;
		powork->l_cnt = 0;
		
		powork->u_pos[0] = 4.0f;
		powork->u_pos[1] = (float)DRAW_WIDTH-4.0f;
		
		/////debug
		  if( (BP_PS2_rand()>>16)%2  ){
			  powork->p_m[0] = 1;
			  powork->p_m[1] = -1;
		  }else {
			  powork->p_m[0] = -1;
			  powork->p_m[1] = 1;
		  }
		
		{
			DG_DMAPACK		*dmapack;
			POWEROFF_PACK_XBOX	*packet;
			unsigned int psize ;
			
			psize = sizeof( POWEROFF_PACK_XBOX );
			printf("size [%d]\n",psize);  
			powork->draw_packet = (POWEROFF_PACK_XBOX*)GV_Malloc( psize );
			//powork->draw_packet = (POWEROFF_PACK_XBOX*)GV_Malloc( 1024 );
			packet = (POWEROFF_PACK_XBOX*)powork->draw_packet;
			if( packet == NULL ) return -1;
			
			//ＤＭＡパケット作成
			powork->dmapack= dmapack = DG_MakeDmapack2( DG_DMAPACK_MENU, DG_DMAPACK_PHASE_LAST, 254 );
			if( dmapack == NULL ){
				printf("ERR!! MAKE DMAPACK!!\n");
				return -1;
			}
			DG_QueueDmapack( dmapack );
			dmapack->flag |= DG_DMAPACK_INVISIBLEMENU;
			dmapack->flag |= DG_DMAPACK_PRIVILEGE;/* 特権モード(他のメニューなどをみえなくするため)*/

			powork->dmapack->autopacket = packet ;
			//InitPowerOffPacket_XBOX( work, powork->dmapack->autopacket );
			DG_SetDmapackEnd( packet );
			powork->prim_x = packet;

			{
            extern void* GV_NoiseBlock;
				//線形テクスチャでノイズテクスチャ作り
            powork->noise_tex = DG_MakeLinerTexture( DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_R5G6B5 );
				memcpy( powork->noise_tex->image, GV_NoiseBlock, DRAW_WIDTH*DRAW_HEIGHT*2 );				

				powork->noise_tex2 = DG_MakeLinerTexture( DRAW_WIDTH, DRAW_HEIGHT, DG_TEXLIN_FORMAT_P8 );
            memcpy(powork->noise_tex2->image, GV_NoiseBlock, DRAW_WIDTH*DRAW_HEIGHT);
			}
		}
	}

	{	/* 
		   L2Dフェーズ用初期化 
		*/
		SELECTL2D_WORK *lwork = &work->l2dwork;
		int handle, status;
		SPR_OBJ *spr;
		
		handle = L2D_LoadLayout2( 7595510/* vr_geme_over.l2d */ ,
								  DG_CHANL_MENU , 1, SPR_FLAG_PRIV, GV_PAUSE_STOP ) ;
		if ( handle < 0 ){
			return -1 ;
		}
		lwork->handle_2d = handle ;
		lwork->action_num = L2D_GetActionNumber( lwork->handle_2d );
		lwork->action = 0;
		L2D_SetSignalHandle( lwork->handle_2d , work , L2dSignal );

		spr = L2D_GetObject( lwork->handle_2d, 2770484/* root */ );
		if ( spr == NULL ){
			printf("root strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->root = spr;
		spr = L2D_GetObject( lwork->handle_2d, 7961512/* continue */ );
		if ( spr == NULL ){
			printf("contine strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->conti = spr;
		spr = L2D_GetObject( lwork->handle_2d, 3435924/* exit */ );
		if ( spr == NULL ){
			printf("exit strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->exit = spr;

		spr = L2D_GetObject( lwork->handle_2d, 3239/* bg */ );
		if ( spr == NULL ){
			printf("cursor strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->cursor = spr;
		spr = L2D_GetObject( lwork->handle_2d, 12554118/* continue_line_02 */ );
		if ( spr == NULL ){
			printf("cursor strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->cursor2 = spr;
		spr = L2D_GetObject( lwork->handle_2d, 12554117/* continue_line_01 */ );
		if ( spr == NULL ){
			printf("cursor strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->cursor3 = spr;
		spr = L2D_GetObject( lwork->handle_2d, 11225720/* exit_line_02 */ );
		if ( spr == NULL ){
			printf("cursor strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->cursor4 = spr;
		spr = L2D_GetObject( lwork->handle_2d, 11225719/* exit_line_01 */ );
		if ( spr == NULL ){
			printf("cursor strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->cursor5 = spr;


		spr = L2D_GetObject( lwork->handle_2d, 12161264/* mission_failed */ );
		if ( spr == NULL ){
			printf("m_failed strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->m_failed = spr;
		spr = L2D_GetObject( lwork->handle_2d, 12930871/* NULL_moji_01 */ );
		if ( spr == NULL ){
			printf("nul_moji strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->nul_moji = spr;
		spr = L2D_GetObject( lwork->handle_2d, 12930872/* NULL_moji_02 */ );
		if ( spr == NULL ){
			printf("nul_moji strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->nul_moji2 = spr;
		spr = L2D_GetObject( lwork->handle_2d, 7678258/* NULL_exit */ );
		if ( spr == NULL ){
			printf("strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->nul_exit = spr;
		spr = L2D_GetObject( lwork->handle_2d, 6129506/* NULL_continue */ );
		if ( spr == NULL ){
			printf("strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->nul_conti = spr;

		spr = L2D_GetObject( lwork->handle_2d, 6457658/* NULL_move_source */ );
		if ( spr == NULL ){
			printf("move_source_01 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->nul_source = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086182/* move_source_mini_06 */ );
		if ( spr == NULL ){
			printf("move_source_01 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[0] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086181/* move_source_mini_05 */ );
		if ( spr == NULL ){
			printf("move_source_02 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[1] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086180/* move_source_mini_04 */ );
		if ( spr == NULL ){
			printf("move_source_03 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[2] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086179/* move_source_mini_03 */ );
		if ( spr == NULL ){
			printf("move_source_04 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[3] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086178/* move_source_mini_02 */ );
		if ( spr == NULL ){
			printf("move_source_05 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[4] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086177/* move_source_mini_01 */ );
		if ( spr == NULL ){
			printf("move_source_06 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[5] = spr;
		spr = L2D_GetObject( lwork->handle_2d, 5086183/* move_source_mini_07 */ );
		if ( spr == NULL ){
			printf("move_source_07 strcode違うよ!!!!\n");
			return -1 ;
		}
		lwork->source[6] = spr;

		status = L2D_EvokeAction( lwork->handle_2d, 566267/* DefaultAction */ );
		if( status < 0 ){
		    printf("DefaultAction 再生失敗\n");
		    return -1;
		}

		lwork->menuprint_work_ptr = NewMenuPrintManager( 18 * 1024, DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE, 
														 DG_DMAPACK_PHASE_LAST, 254 );
		if ( lwork->menuprint_work_ptr == NULL ){
			printf("menu print null -> non memory\n");
			return (-1);
		}
		GV_SetActorChild( work, lwork->menuprint_work_ptr );

		/* にぎやかしテクスチャ */
		lwork->so_tex[0] = SO_TEX_STRCODE0;
		lwork->so_tex[1] = SO_TEX_STRCODE1;
		lwork->so_tex[2] = SO_TEX_STRCODE2;
		lwork->so_tex[3] = SO_TEX_STRCODE3;
		lwork->so_tex[4] = SO_TEX_STRCODE4;
		lwork->so_tex[5] = SO_TEX_STRCODE5;
		lwork->so_tex[6] = SO_TEX_STRCODE6;
		lwork->so_tex[7] = SO_TEX_STRCODE7;
		lwork->so_tex[8] = SO_TEX_STRCODE8;
		lwork->so_tex[9] = SO_TEX_STRCODE9;
		lwork->so_tex[10] = SO_TEX_STRCODE10;
		lwork->so_tex[11] = SO_TEX_STRCODE11;
		lwork->so_tex[12] = SO_TEX_STRCODE12;
		lwork->so_tex[13] = SO_TEX_STRCODE13;
		lwork->so_tex[14] = SO_TEX_STRCODE14;
		lwork->so_tex[15] = SO_TEX_STRCODE15;
		lwork->so_tex[16] = SO_TEX_STRCODE16;
		lwork->so_tex[17] = SO_TEX_STRCODE17;
	}

/*
	 {
		 FVECTOR pos;
		 pos.x=0.0f;pos.y=0.0f;pos.z=0.0f; 
		 GM_SeSetMode( SD_A_V_NOIZE1, &pos, GM_SEMODE_NORMAL );
	 }
*/
	 GM_SdSet( SD_A_V_NOIZE1 );

	return 0;
}


void *NewVRGameOver( int mode  )/* modeの意味はない */
{
	Work		*work ;

	printf("VR GAMEOVER?\n");

	/* ジアゼパムの効力を初期化 */
	GM_DiazepamCount = 0;


	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return (void *)work ;
}

