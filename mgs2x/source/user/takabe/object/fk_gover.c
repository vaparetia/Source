//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fk_gover.c
	偽ゲームオーバー画面表示キャラ

	2001/06/24 K.Takabe
	$Id: fk_gover.c,v 1.1.1.3 2002/11/19 11:51:14 Yoshizawa1 Exp $

*/
/*

chara 偽ゲームオーバー画面[NewFakeGameOverSet] $s:name \
	-time $w:消滅までの時間（１／３００秒単位） \
	-x $s:TRI_ID $s:FAILD_TEX_ID $s:CONTINUE_TEX_ID $s:EXIT_TEX_ID \
	-proc $p:終了Proc \
	-vox $t:ジングル音声 \
	-flag $w:フラグ
// 表示に必要なテクスチャを指定する必要があります。とりあえず
// pack_all fake_gameover.tri \
//		2D/game_over/over_failed_alp_ovl.bmp \
//		2D/game_over/over_exit_alp_ovl.bmp \
//		2D/game_over/over_cont_alp_ovl.bmp 
// と.srcに記述し、オプションに以下を指定すれば出ます
// -x fake_gameover over_failed_alp_ovl over_cont_alp_ovl over_exit_alp_ovl


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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"

#include "../other/vec_util.h"

/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
extern	int	GM_GameOverVox ;

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/


/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/

#define START_SCN_TIME	(100)
#define START_SCN_X1	(64)
#define START_SCN_Y1	(64)
#define START_SCN_X2	(400+64)
#define START_SCN_Y2	(300+64)
#define END_SCN_TIME	(600)
#define END_SCN_X1		(33)
#define END_SCN_Y1		(29)
#define END_SCN_X2		(236)
#define END_SCN_Y2		(181)

#define LEFT_BAR_X1		(27)
#define LEFT_BAR_Y1		(29)
#define LEFT_BAR_Y2		(344)
#define CURSOR_X1		(20)
#define CURSOR_Y1		(193)
#define CURSOR_X2		(20+8)
#define CURSOR_Y2		(193+8)

#define START_FADE_TIME		(200)
#define END_FADE_TIME		(800)

#define SPRITE_FAILD_X		(246)
#define SPRITE_FAILD_Y		(29)
#define SPRITE_FAILD_W		(246)
#define SPRITE_FAILD_H		(16)
#define SPRITE_FAILD_X2		(SPRITE_FAILD_X+SPRITE_FAILD_W)
#define SPRITE_FAILD_Y2		(SPRITE_FAILD_Y+SPRITE_FAILD_H)
#define SPRITE_FAILD_COL	(0x805a5a5a)
#define SPRITE_FAILD_COL2	(0x802d2d2d)
#define SPRITE_CONTINUE_X	(35)
#define SPRITE_CONTINUE_Y	(191)
#define SPRITE_CONTINUE_W	(132)
#define SPRITE_CONTINUE_H	(12)
#define SPRITE_CONTINUE_COL	(0x805a5a5a)
#define SPRITE_EXIT_X		(35)
#define SPRITE_EXIT_Y		(211)
#define SPRITE_EXIT_W		(60)
#define SPRITE_EXIT_H		(12)
#define SPRITE_EXIT_COL		(0x805a5a5a)

//#define BACK_COLOR	(0x80300800)
//#define BACK_COLOR	(0x802a3426)
#define BACK_COLOR	(0x80303226)
//#define BACK_COLOR	(0x80083000)
#define LINE_COLOR	(0x80c0c0c0)

#define PHASE_TIME0	(600)
//#define PHASE_TIME1 (1200)
//#define PHASE_TIME2	(1500)
#define PHASE_TIME1 (900)
#define PHASE_TIME2	(1400)

#define TRI_ID	(9586795)	/*(GV_StrCode("fake_gameover"))*/
#define TEX_ID1	(9816882)	/*(GV_StrCode("over_failed_alp_ovl"))*/
#define TEX_ID2	(9630303)	/*(GV_StrCode("over_cont_alp_ovl"))*/
#define TEX_ID3	(15922050)	/*(GV_StrCode("over_exit_alp_ovl"))*/

/* ---------------------------------------------------------------- */
extern void *NewReduceScreenEffectEx( int phase, int prio );
extern void TAKABE_SetReduceScreenSize( void *work_ptr, int x1, int y1, int x2, int y2, int color );
extern void TAKABE_SetReduceScreenSizeF( void *work_ptr, float x1, float y1, float x2, float y2, int color );

extern void *NewMenuPrimUtil( int dmapack_phase, int prio, int tri_id, int buffer_size );
extern int MENU_PrimUtil_GetUseBufferSize( void *work_ptr );
extern void MENU_PrimUtil_PutSprite( void *work_ptr, int x, int y, int w, int h, int col, DG_TEX *tex );
extern void MENU_PrimUtil_PutBox( void *work_ptr, int x1, int y1, int x2, int y2, int col );
extern void MENU_PrimUtil_PutLine( void *work_ptr, int x1, int y1, int x2, int y2, int col );

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name ;
	int			map ;
	/* 基本ワーク */
	int			first_flag ;		/* 起動時間 */
	int			time ;				/* 起動時間 */
	int			phase ;				/* 処理フェーズ */
	int			stream_handler ;	/* ジングルストリーム用 */
	int			stream_flag ;		/* 再生フラグ */
	int			vox_id ;			/* 再生ＶＯＸ番号 */
	FVECTOR		win_pos ;			/* 縮小ウィンドウの座標位置 */
	int			tri_id ;			/* TRI_ID */
	int			tex_id_list[3] ;	/*  */
	DG_TEX		*tex_list[3] ;		/* 各種テクスチャ */
	/* コンソール表示用 */
	int			text_flag ;
	int			text_pos[2] ;
	int			text_count ;
	int			text_alpha ;
	int			disp_line[2] ;
	int			last_num ;
	char		text_buffer[512] ;
	int			param_raidenchin_up ;
	int			param_savecount ;
	int			param_shootcount ;
	int			param_alertcount ;
	int			param_killcount ;
	int			param_damagecount ;
	int			param_camera_x ;
	int			param_camera_y ;
	int			param_camera_z ;
	int			param_global_load ;
	char		*param_area ;
	/* シナリオ起動ワーク */
	int			kill_time ;			/* 消滅までの時間 */
	int			flag ;				/* 各種フラグ */
	int			proc_func ;
	/* 他キャラ連動用ワーク */
	void		*mini_screen_work_ptr ;
	void		*primd_work_ptr ;
	void		*menuprint_work_ptr ;

} Work ;

/* ---------------------------------------------------------------- */
static FVECTOR	start_pos = {START_SCN_X1,START_SCN_Y1,START_SCN_X2,START_SCN_Y2} ;
static FVECTOR	end_pos = {END_SCN_X1,END_SCN_Y1,END_SCN_X2,END_SCN_Y2} ;
static int text_mask_list[] = {
	245,245,269,275,296,317,323,345,367,400,422,428,450,472,494
};
/* ---------------------------------------------------------------- */
static float CalcBezier( float q0, float q1, float q2, float t )
{
	float	ti ;
	ti = 1.0f - t ;
	return ( ti * ti * q0 + 2.0f * ti * t * q1 + t * t * q2 );
}
static void InterVector( FVECTOR *v0, FVECTOR *v1, FVECTOR *v2, float s )
{
	float	is ;
	is = 1.0f - s ;
	v0->vx = v1->vx * is + v2->vx * s ;
	v0->vy = v1->vy * is + v2->vy * s ;
	v0->vz = v1->vz * is + v2->vz * s ;
	v0->vw = v1->vw * is + v2->vw * s ;
}
/* ---------------------------------------------------------------- */
static void ActBackgraund( Work *work )
{
	FVECTOR		scn_pos ;
	float		t ;
	u_int		alpha ;

	/* 縮小スクリーンアニメーション処理 */
	if ( work->time < START_SCN_TIME ){
		t = 0.0f ;
	} else if ( work->time < END_SCN_TIME ){
		t = (float)( work->time - START_SCN_TIME ) / (float)( END_SCN_TIME - START_SCN_TIME ) ;
		//GTE_InterVector( &work->win_pos, &work->win_pos, &end_pos, 1.0f/20.0f );
		GTE_InterVector( &work->win_pos, &work->win_pos, &end_pos, CalcBezier( 1.0f/20.0f, 1.0f/20.0f, 1.0f/5.0f, t ) );
		//InterVector( &work->win_pos, &work->win_pos, &end_pos, CalcBezier( 1.0f/20.0f, 1.0f/20.0f, 1.0f/5.0f, t ) );
	} else {
		t = 1.0f ;
	}
	//t = MT_HermiteLerp( 0.0f, 1.0f, 1.0f, 0.0f, t );
	//GTE_InterVector( &scn_pos, &start_pos, &end_pos, 1.0f - ( 1.0f - t ) * ( 1.0f - t ) );
	//t = CalcBezier( 0.0f, 0.9f, 1.0f, t );
	//GTE_InterVector( &scn_pos, &start_pos, &end_pos, t );
	scn_pos = work->win_pos ; 


	/* 画面縮小処理 */
	TAKABE_SetReduceScreenSizeF( work->mini_screen_work_ptr,
								scn_pos.vx, scn_pos.vy, scn_pos.vz, scn_pos.vw, 0x80808080 );
	/* 背景色描画 */
	MENU_PrimUtil_PutBox( work->primd_work_ptr,
						 0, 0, 512, scn_pos.vy, BACK_COLOR );
	MENU_PrimUtil_PutBox( work->primd_work_ptr,
						 0, scn_pos.vw, 512, 384, BACK_COLOR );
	MENU_PrimUtil_PutBox( work->primd_work_ptr,
						 0, scn_pos.vy, scn_pos.vx, scn_pos.vw, BACK_COLOR );
	MENU_PrimUtil_PutBox( work->primd_work_ptr,
						 scn_pos.vz, scn_pos.vy, 512, scn_pos.vw, BACK_COLOR );
	/* 画面縁取りライン描画 */
	MENU_PrimUtil_PutLine( work->primd_work_ptr,
						  scn_pos.vx, scn_pos.vy, scn_pos.vz, scn_pos.vy, LINE_COLOR );
	MENU_PrimUtil_PutLine( work->primd_work_ptr,
						  scn_pos.vz, scn_pos.vy, scn_pos.vz, scn_pos.vw, LINE_COLOR );
	MENU_PrimUtil_PutLine( work->primd_work_ptr,
						  scn_pos.vx, scn_pos.vy, scn_pos.vx, scn_pos.vw, LINE_COLOR );
	MENU_PrimUtil_PutLine( work->primd_work_ptr,
						  scn_pos.vx, scn_pos.vw, scn_pos.vz, scn_pos.vw, LINE_COLOR );

	/* 左バーの表示 */
	MENU_PrimUtil_PutLine( work->primd_work_ptr,
						  LEFT_BAR_X1, LEFT_BAR_Y1, LEFT_BAR_X1, LEFT_BAR_Y2, LINE_COLOR );

	/* その他表示 */
	if ( work->phase > 0 ){
		MENU_PrimUtil_PutSprite( work->primd_work_ptr,
								SPRITE_FAILD_X, SPRITE_FAILD_Y, SPRITE_FAILD_W, SPRITE_FAILD_H,
								SPRITE_FAILD_COL, work->tex_list[0] );
		if ( work->time > PHASE_TIME0 ){
			int		index ;
			index = ( work->time - PHASE_TIME0 ) / 20 ;
			if ( index > 14 ) index = 14 ;
			MENU_PrimUtil_PutBox( work->primd_work_ptr,
								 text_mask_list[index], SPRITE_FAILD_Y, SPRITE_FAILD_X2, SPRITE_FAILD_Y2, BACK_COLOR );
		}
	}
	if ( work->phase > 1 ){
		/* 選択肢表示 */
		MENU_PrimUtil_PutSprite( work->primd_work_ptr,
								SPRITE_CONTINUE_X, SPRITE_CONTINUE_Y, SPRITE_CONTINUE_W, SPRITE_CONTINUE_H,
								SPRITE_FAILD_COL2, work->tex_list[1] );
		MENU_PrimUtil_PutSprite( work->primd_work_ptr,
								SPRITE_EXIT_X, SPRITE_EXIT_Y, SPRITE_EXIT_W, SPRITE_EXIT_H,
								SPRITE_FAILD_COL2, work->tex_list[2] );
	}
	if ( work->phase > 2 ){
		/* 選択肢表示 */
		MENU_PrimUtil_PutSprite( work->primd_work_ptr,
								SPRITE_CONTINUE_X, SPRITE_CONTINUE_Y, SPRITE_CONTINUE_W, SPRITE_CONTINUE_H,
								SPRITE_FAILD_COL, work->tex_list[1] );
		MENU_PrimUtil_PutSprite( work->primd_work_ptr,
								SPRITE_EXIT_X, SPRITE_EXIT_Y, SPRITE_EXIT_W, SPRITE_EXIT_H,
								SPRITE_FAILD_COL2, work->tex_list[2] );

		/* カーソルの表示 */
		MENU_PrimUtil_PutBox( work->primd_work_ptr,
							 CURSOR_X1, CURSOR_Y1, CURSOR_X2, CURSOR_Y2, LINE_COLOR );

	}

	/* フェード描画処理 */
	t = (float)( work->time - START_FADE_TIME ) / ( END_FADE_TIME - START_FADE_TIME );
	t = DG_MIN( t, 1.0f );
	t = DG_MAX( t, 0.0f );
	alpha =DG_FTOI(  ( 1.0f - t ) * 128 ) ;
	MENU_PrimUtil_PutBox( work->primd_work_ptr, 0, 0, 512, 384, 0x00ffffff | ( alpha << 24 ) );

}
/* ---------------------------------------------------------------- */
#if 0
static void ActConsole( Work *work )
{
	int		len ;
	if ( work->phase < 1 ) return ;
	/* コンソールテキスト内容の生成 */
	sprintf( work->text_buffer,
			"Mission Faild report...\n"
			"\n"
			"Area : %s\n"
			"X : %08x\n"
			"Y : %08x\n"
			"Z : %08x\n"
			"Status : 0x%08x\n"
			"Enamey Kill : %d\n"
			"Enemey Find : %d\n"
			"Bullet Fire : %d",
			/* 各種表示用パラメータ */
			"test", 0x101010101, 0x1020203, 0x20afbbd8,
			0x7345fd90, 2957, 762, 5041
			);
	/* コンソールを１文字づつ表示しているかのように見せる処理 */
	if ( strlen( work->text_buffer ) > work->text_pos ){
		work->text_count -= 5 ;
		if ( work->text_count < 0 ){
			char	c ;
			c = work->text_buffer[ work->text_pos ];
			work->text_pos++ ;
			/* 新たに表示された文字の種類によって効果音などをつける */
			if ( c != ' ' && c != '\n' ){
				/* dummy */
			}
			/* 次の文字が出るまでの時間を設定 */
			work->text_count += 1*5 ;
		}
		work->text_buffer[ work->text_pos ] = '\0' ;
	}

	/* コンソールの内容を画面に出力 */
#ifdef PSX2
	___MENU_Locate( work->menuprint_work_ptr, 1, 35, 240*DRAW_HEIGHT/384, 0 );
#else
	___MENU_Locate( work->menuprint_work_ptr, 1, 35*DRAW_WIDTH/512, 240*DRAW_HEIGHT/384, 0 );
#endif
	___MENU_Color( work->menuprint_work_ptr, 1, 192, 192, 192, 128 );
	___MENU_PrintMini( work->menuprint_work_ptr, 1, work->text_buffer );
}
#else
static void MiniMiniFontPhase1Update( Work *pWork )
{
	static short line_count[] = { 1 , 1 , 1 };
	static short line_frame[] = { 2 , 2 , 2 };
	int      count;

	if ( pWork->text_alpha < 1 ){
		return;
	}
	// コンソールテキスト内容の生成
	sprintf( pWork->text_buffer,
			"YUMIKUROTAS\n"
			"61301891\n"
			"ONIKAMOTOYK\n"
			/* 各種表示用パラメータ */
	);
	/* コンソールを１文字づつ表示しているかのように見せる処理 */
	if ( strlen( pWork->text_buffer ) > pWork->text_pos[ 0 ] ){
		pWork->text_count--;
		if ( pWork->text_count < 0 ){
			count = 0;
			while ( count < line_count[ pWork->disp_line[ 0 ] ] ){
				/* 新たに表示された文字の種類によって効果音などをつける */
				GM_SeSet( GM_PAN_CENTER , 255 , SD_S_GO_TYPE1 );
				while ( pWork->text_buffer[ pWork->text_pos[ 0 ] ] != '\n' ){
					pWork->text_pos[ 0 ]++;
				}
				pWork->text_pos[ 0 ]++;
				count++;
			}
			/* 次の文字が出るまでの時間を設定 */
			pWork->text_count = DIRECT_TICK( line_frame[ pWork->disp_line[ 0 ] ] );
			pWork->disp_line[ 0 ]++;
		}
		pWork->text_buffer[ pWork->text_pos[ 0 ] ] = '\0' ;
	} else {
		//pWork->flag |= SK_MINI_FONT_PHASE_1_OK;
		pWork->text_flag |= 0x0001 ;
	}
	/* コンソールの内容を画面に出力 */
#ifdef PSX2
#ifndef PAL
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 492 , 60 * ( DRAW_HEIGHT / 384 ) , 1 );
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 492 , 60 * DRAW_HEIGHT / 448 , 1 );
#endif
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 492 * DRAW_WIDTH / 512 , 60 * DRAW_HEIGHT / 448 , 1 );
#endif
	___MENU_Color( pWork->menuprint_work_ptr, 1, 255, 255, 255, ( u_char )pWork->text_alpha );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 1, pWork->text_buffer );
}

static void MiniMiniFontPhase2Update( Work *pWork )
{
	static short line_count[] = { 1 , 1 , 1 , 1 , 2 , 2 , 6 , 1 }; 
	static short line_frame[] = { 12 , 12 , 6 , 15 , 3 , 6 , 18 , 6 }; 
	int      count;

	//if ( pWork->text_alpha < 1.0f || !( pWork->flag & SK_MINI_FONT_PHASE_1_OK ) ){
	//	return;
	//}
	if ( pWork->text_alpha < 1.0f || !( pWork->text_flag & 0x0001 ) ){
		return;
	}
	// コンソールテキスト内容の生成
	sprintf( pWork->text_buffer,
			"ARMSTREN GTH%d\n"
			"PRUD ENCE%d\n"
			"AMMOUS ED%d\n"
			"ALRTNUM%d\n\n"
			"NUET%d\n"
			"DMGA MMOUNT%d\n\n"
			"CAMPOSX%d\n"
			"CAMPO SY%d\n"
			"CAM POSZ%d\n"
			"AREAMO VE%d\n\n\n"
#ifdef PSX2
			"VANISHI NGPOINT%s                      dsjw%6d\n" ,
#else
			/* MENU_Printf系は横幅が実際の画面ピクセルに依存する為 */
			"VANISHI NGPOINT%s                                  dsjw%6d\n" ,
#endif
			/* 各種表示用パラメータ */
			//GM_RaidenChin_Up , GM_SaveCount , GM_ShootCount , GM_AlertCount ,
			//GM_KillCount , GM_DamageCount , GM_CameraX , GM_CameraY , GM_CameraZ , GM_GlobalLoadCount , GM_GetArea() , pWork->last_num

			pWork->param_raidenchin_up, pWork->param_savecount, pWork->param_shootcount, pWork->param_alertcount,
			pWork->param_killcount, pWork->param_damagecount,
			pWork->param_camera_x, pWork->param_camera_y, pWork->param_camera_z,
			pWork->param_global_load, pWork->param_area, pWork->last_num
			);
	/* コンソールを１文字づつ表示しているかのように見せる処理 */
	if ( strlen( pWork->text_buffer ) > pWork->text_pos[ 1 ] ){
		pWork->text_count--;
		if ( pWork->text_count < 0 ){
			count = 0;
			while ( count < line_count[ pWork->disp_line[ 1 ] ] ){
				/* 新たに表示された文字の種類によって効果音などをつける */
				GM_SeSet( GM_PAN_CENTER , 255 , SD_S_GO_TYPE1 );
				while ( pWork->text_buffer[ pWork->text_pos[ 1 ] ] != '\n' ){
					pWork->text_pos[ 1 ]++;
				}
				pWork->text_pos[ 1 ]++;
				count++;
			}
			/* 次の文字が出るまでの時間を設定 */
			pWork->text_count = DIRECT_TICK( line_frame[ pWork->disp_line[ 1 ] ] );
			pWork->disp_line[ 1 ]++;
		}
		pWork->text_buffer[ pWork->text_pos[ 1 ] ] = '\0' ;
	} else {
		//pWork->flag |= SK_MINI_FONT_PHASE_2_OK;
		pWork->text_flag |= 0x0002 ;
	}


	/* コンソールの内容を画面に出力 */
#ifdef PSX2
#ifndef PAL
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 35, 280 * ( DRAW_HEIGHT / 384 ) , 0 );
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 35, 280 * DRAW_HEIGHT / 448 + 16, 0 );
#endif
#else
	___MENU_Locate( pWork->menuprint_work_ptr, 1, 35 * DRAW_WIDTH / 512, 280 * DRAW_HEIGHT / 448 + 16, 0 );
#endif
	___MENU_Color( pWork->menuprint_work_ptr, 1, 255, 255, 255, ( u_char )pWork->text_alpha );
	___MENU_PrintMini( pWork->menuprint_work_ptr, 1, pWork->text_buffer );
}

static void TextAlphaControl( Work *work )
{
	//work->text_alpha = ( work->time - PHASE_TIME1 ) * 128 / 600 ;
	//if ( work->text_alpha > 128 ) work->text_alpha = 128 ;
	work->text_alpha = 128 ;
}
#endif
/* ---------------------------------------------------------------- */
/* proc呼び出し */
static void ExecEndProc( Work *work )
{
	GCL_ARGS	arg ;
	int			data[4] ;

	if ( work->proc_func == 0 ) return ;
	arg.argc = 0 ;
	arg.argv = data ;
	//data[0] = mode ;
	GCL_ExecProc( work->proc_func, &arg );
}
/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	if ( work->first_flag ){
		work->first_flag-- ;
		return ;
	}
	switch ( work->phase ){
	  case 0:/* 起動～文字表示 */
		if ( work->time > PHASE_TIME0 ) work->phase++ ;
		break ;
	  case 1:
		if ( work->time > PHASE_TIME1 ) work->phase++ ;
		break ;
	  case 2:
		if ( work->time > PHASE_TIME2 ) work->phase++ ;
		break ;
	  case 3:
		break ;
	}


#if 1
	/* ストリーム関連 */
	if ( work->time > (TIME_BASE*16)  && !(work->stream_flag) ) {
		/* 音声再生 */
		/* いいタイミングでスタートしてください */
		/* 多重呼びしないように注意してください */
		if ( work->vox_id >= 0 && GM_StreamStatus( work->stream_handler ) == GM_STREAM_STATE_WAIT ) {
			GM_StreamStart( work->stream_handler );
			work->stream_flag = 1;
		}
	}
#endif

	/* 背景の描画 */
	ActBackgraund( work );
	/* レポート内容の描画 */
	//ActConsole( work );
	if ( work->phase > 1 ){
		MiniMiniFontPhase1Update( work );
		MiniMiniFontPhase2Update( work );
	}
	TextAlphaControl( work );

	/* アニメーション後はポーズ中の時間進行を止める */
	if ( work->phase < 3 || GV_PauseLevel == 0 ){
		work->time += TIME_BASE ;
	}

	if ( work->time > work->kill_time ){
		ExecEndProc( work );
		GV_DestroyActor( work );
	}
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	GM_SdSet( SNG_FIN_S );
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;
	work->win_pos = start_pos ;
	work->tri_id = TRI_ID ;
	work->first_flag = 2 ;

	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'p' ) != NULL ){
		work->proc_func = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 'v' ) != NULL ){
		work->vox_id = GCL_GetNextInt() ;
	}

	if ( GCL_GetOption( 't' ) != NULL ){
		work->kill_time = GCL_GetNextInt();
	} else {
		work->kill_time = 0x7fffffff ;
	}

	if ( GCL_GetOption( 'x' ) != NULL ){
		work->tri_id = GCL_GetNextInt();
		work->tex_id_list[0] = GCL_GetNextInt();
		work->tex_id_list[1] = GCL_GetNextInt();
		work->tex_id_list[2] = GCL_GetNextInt();
	} else {
		work->tri_id = TRI_ID ;
		work->tex_id_list[0] = TEX_ID1 ;
		work->tex_id_list[1] = TEX_ID2 ;
		work->tex_id_list[2] = TEX_ID3 ;
	}
	work->tex_list[0] = DG_GetTexture2( work->tri_id, work->tex_id_list[0] ) ;
	work->tex_list[1] = DG_GetTexture2( work->tri_id, work->tex_id_list[1] ) ;
	work->tex_list[2] = DG_GetTexture2( work->tri_id, work->tex_id_list[2] ) ;

//#ifdef PSX2
	/* 画面縮小キャラ生成 */
	work->mini_screen_work_ptr = NewReduceScreenEffectEx( DG_DMAPACK_PHASE_LAST, 250 );
	GV_SetActorChild( work, work->mini_screen_work_ptr );
	/* 低レベルプリミティブ表示キャラ生成 */
	work->primd_work_ptr = NewMenuPrimUtil( DG_DMAPACK_PHASE_LAST, 250, work->tri_id, 1024 );
	GV_SetActorChild( work, work->primd_work_ptr );
	/* 偽ゲームオーバー用MENU_Printfキャラ生成 */
	work->menuprint_work_ptr = NewMenuPrintManager( 8*1024, DG_DMAPACK_MENU, DG_DMAPACK_PHASE_LAST, 251 );
	GV_SetActorChild( work, work->menuprint_work_ptr );
//#endif

	work->param_raidenchin_up = GM_RaidenChin_Up ;
	work->param_savecount = GM_SaveCount ;
	work->param_shootcount = GM_ShootCount ;
	work->param_alertcount = GM_AlertCount ;
	work->param_killcount = GM_KillCount ;
	work->param_damagecount  = GM_DamageCount ;
	work->param_camera_x = GM_CameraX ;
	work->param_camera_y = GM_CameraY ;
	work->param_camera_z = GM_CameraZ ;
	work->param_global_load = GM_GlobalLoadCount ;
	work->param_area = GM_GetArea();
	work->last_num = BP_PS2_rand() % 10000 ;

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewFakeGameOverSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_PREV2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#if 1
		GM_StreamStopAll() ;
		if ( work->vox_id >= 0 ) {
			//work->stream_handler = GM_VoxStream( GM_GameOverVox, GM_STREAM_NO_PAUSE ) ;
			work->stream_handler = GM_VoxStream( work->vox_id, GM_STREAM_PLAY_WAIT ) ;
		} else {
			work->stream_handler = -1 ;
		}
		GM_SdSet( SNG_FOUTP_S );
#endif
	}
	return (void *)work ;
}


/* ---------------------------------------------------------------- */
