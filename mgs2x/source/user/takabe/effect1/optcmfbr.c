//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	optcmfbr.c
	デモ用光学迷彩壊れエフェクト

	2000/04/03 K.Takabe
	$Id: optcmfbr.c,v 1.1.1.3 2002/11/19 11:51:12 Yoshizawa1 Exp $

*/
/*
	void *NewOpticalCamouflageBreakDemo( DG_OBJS *objs, int type, int start_time, int end_time, int color )
	DG_OBJS	*objs ;			光学迷彩を設定するオブジェクト
	int		type ;			エフェクトタイプ（0:横歪み 1:縦縮小）
	int		start_time ;	壊れエフェクト開始時間（1/300sec）
	int		end_time ;		壊れエフェクト終了時間（1/300sec）
	int		color ;			光学迷彩カラー（0x00BBGGRR）

	壊れエフェクト終了時間に達するとこのキャラは自動消滅します


	光学迷彩パラメータ変更メッセージ
	name = 9983938（"デモ用光学迷彩壊れ"）
	message[0] = 0(128で補間時間が1/1000秒単位に)
	message[1] = Ｘ方向ずらし量（ピクセル単位default=16）
	message[2] = Ｙ方向ずらし量（ピクセル単位default=16）
	message[3] = 補間時間（１／３００秒単位。最低でも６以上）

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
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
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../mode/demo/libdemo.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */

#undef PIXEL_ASPECT

#ifndef HIRESO_FFI
#define PIXEL_ASPECT	(2)
#else
#define PIXEL_ASPECT	(1)
#endif

#define CHARA_NAME	(9983938)/* デモ用光学迷彩壊れ */

/* ---------------------------------------------------------------- */
/* ワーク構造体定義 */
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX		optcmf_param ;
	FMATRIX		optcmf_rotmat ;
	DG_OBJS		*objs ;
	int			old_flag ;
	int			count ;
	int			start_time ;
	int			end_time ;
	int			time_length ;
	int			phase ;
	int			type ;
	int			time_list[4] ;
	float		diff_x, diff_y ;
	int			color ;
	int			interp_time ;
	float		trg_diff_x, trg_diff_y ;

	int			name ; /* T.Morita added  Thu Oct  5 17:58:36 JST 2000 */
} Work ;

/* ---------------------------------------------------------------- */
/* ローカル関数群 */
static void SetOpticalCamouflageParam( Work *work, float diff_x, float diff_y, int color )
{
	/* 光学迷彩パラメータの設定 */
	work->optcmf_param.m[0][0] = DRAW_WIDTH / 2.0f - diff_x ;
	work->optcmf_param.m[0][1] = DRAW_HEIGHT / 2.0f - diff_y / PIXEL_ASPECT ;
	work->optcmf_param.m[1][0] = DRAW_WIDTH / 2.0f ;
	work->optcmf_param.m[1][1] = DRAW_HEIGHT / 2.0f ;
	work->optcmf_param.m[2][0] = diff_x ;
	work->optcmf_param.m[2][1] = diff_y / PIXEL_ASPECT ;

	/* 無色光学迷彩 */
	work->optcmf_param.m[3][0] = ( color ) & 0xff ;
	work->optcmf_param.m[3][1] = ( color >> 8 ) & 0xff ;
	work->optcmf_param.m[3][2] = ( color >> 16 ) & 0xff ;
	work->optcmf_param.m[3][3] = 96.0f ;
}

/* ---------------------------------------------------------------- */
/* キャラアクター関数 */
static void Act( Work *work )
{
	float	tm_scale, s, ss, sss, ssssss ;

	{
		GV_MSG	*msg ;
		int		n_msg ;
		if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) != 0 ){/*T.Morita chane CHARA_NAME to work->name */
			switch ( msg->message[0] ){
			  case 0:/* パラメータ変更 */
			  case 128:
				work->trg_diff_x = msg->message[1] ;
				work->trg_diff_y = msg->message[2] ;
				work->interp_time = msg->message[3] ;
				if ( msg->message[0] >= 128 ){
					work->interp_time = msg->message[3] * 300 / 1000 ;
				}
				break ;
			}
		}
	}

	switch ( work->phase ){

	  case 0:/* 壊れ開始前 */
		if ( work->interp_time >= TIME_BASE ){
			float	t ;
			t = (float)TIME_BASE / work->interp_time ;
			work->diff_x = ( work->trg_diff_x - work->diff_x ) * t + work->diff_x ;
			work->diff_y = ( work->trg_diff_y - work->diff_y ) * t + work->diff_y ;
			work->interp_time -= TIME_BASE ;
		}
		SetOpticalCamouflageParam( work, work->diff_x, work->diff_y, work->color );
		break ;

	  case 1:/* 壊れフェーズ１ */
		tm_scale = (float)work->count / work->time_list[1] ;
		s = ( 1.0f - tm_scale ) ;
		if ( s < 0 ) s = 0 ;
		ss = s * s ;	/* ２乗の変化量変数作成 */
		sss = ss * s ;	/* ３乗の変化量変数作成 */
		ssssss = sss * sss * ss * ss ;	/* １０乗 */

		/* 徐々に薄くなるように設定 */
		work->optcmf_param.m[3][3] = 160.0f * ss ;
		if ( work->optcmf_param.m[3][3] > 128.0f ) work->optcmf_param.m[3][3] = 128.0f ;
		/* 法線投影マトリクス補正用のマトリクスをＹ軸に回転させる */
		_sceVu0RotMatrixY( &work->optcmf_rotmat, &work->optcmf_rotmat, 3.14159265f * ( 30 + 200 * ssssss ) / 2048.0f );
		if ( work->type == 0 ){
			/* マッピング領域を狭い状態から通常の状態へ戻していく */
			work->optcmf_param.m[0][0] = DRAW_WIDTH / 2.0f - 16 - ssssss * ( DRAW_WIDTH/3 - 32 ) ;
			work->optcmf_param.m[2][0] = 16 ;
			/* Ｙ軸方向の変化をなくす */
			work->optcmf_param.m[0][1] = DRAW_HEIGHT / 2.0f  ;
			work->optcmf_param.m[2][1] = 0 ;
		} else {
			work->optcmf_param.m[0][1] = DRAW_HEIGHT / 2.0f - ssssss * ( DRAW_HEIGHT/2 - 16 ) ;
			work->optcmf_param.m[1][1] = DRAW_HEIGHT / 2.0f + ssssss * ( DRAW_HEIGHT/2 - 16 ) ;
			work->optcmf_param.m[2][1] = 0 ;
		}
		/* 通常本体の表示ＯＮ（光学迷彩ＯＮ） */
		work->objs->flag &= ~DG_FLAG_PLUGINDRAW ;
		break ;

	  default:
	  case 2:/* 壊れフェーズ終了 */
		work->objs->flag = work->old_flag ;
		GV_DestroyActor( work );
		break ;
	}

	/* 各フェーズの移行判定 */
	work->count += TIME_BASE + DM_FrameSkip * TIME_BASE ;
	if ( work->count > work->time_list[ work->phase ] ){
		work->count -= work->time_list[ work->phase ] ;
		work->phase++ ;
	}

}

/* ---------------------------------------------------------------- */
/* キャラアクター終了関数 */
static void Die( Work *work )
{
	if ( work->objs == NULL ) return ;
	/* 光学迷彩ＯＦＦ */
	work->objs->flag = work->old_flag ;
	if ( work->objs->extend_data == &work->optcmf_param ) work->objs->extend_data = NULL ;
}

/* ---------------------------------------------------------------- */
/* キャラクタ初期化関数 */
static int GetResources( Work *work, DG_OBJS *objs, int type, int start_time, int end_time, int color )
{
	if ( objs->extend_data != NULL ) return ( -1 );

	work->objs = objs ;
	work->type = type ;
	work->start_time = start_time ;
	work->end_time = end_time ;
	work->time_length = end_time - start_time ;

	work->time_list[0] = start_time ;
	work->time_list[1] = end_time - start_time ;

	/* 光学迷彩ＯＮ */
	work->old_flag = objs->flag ;
	objs->flag |= DG_FLAG_OPTCMF | DG_FLAG_DELAYED ;
	/* パラメータ設定マトリクス */
	objs->extend_data = &work->optcmf_param ;
	/* パラメータ設定 */
	work->diff_x = 16 ;
	work->diff_y = 16 ;
	work->color = color ;
	SetOpticalCamouflageParam( work, work->diff_x, work->diff_y, work->color );
	_sceVu0UnitMatrix( &work->optcmf_rotmat );

	return ( 0 );
}

/* ---------------------------------------------------------------- */
/* デモ用インターフェイス */
void *NewOpticalCamouflageBreakDemo( DG_OBJS *objs, int type, int start_time, int end_time, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = CHARA_NAME ;
		if ( GetResources( work, objs, type, start_time, end_time, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* T.Morita added  Thu Oct  5 17:58:36 JST 2000 */
void *NewOpticalCamouflageBreakDemo2( int name, DG_OBJS *objs, int type, int start_time, int end_time, int color )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name ;
		if ( GetResources( work, objs, type, start_time, end_time, color ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
