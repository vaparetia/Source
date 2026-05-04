//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mirror2.c
	リアル鏡面処理

	2001/04/07 K.Takabe
	$Id: mirror2.c,v 1.1.1.3 2002/11/19 11:51:15 Yoshizawa1 Exp $

*/
/*

chara 真鏡面[NewTrueMirror] $s:name \
	-pos	$v:鏡面座標 \
	-axis	$b:反点軸 \		// 0:X軸反点 1:Y軸反点 2:Z軸反点
	-normal	$v:鏡面面の法線直接指定 \
	-enable // 起動時からＯＮに（デフォルトはＯＦＦで起動）
mesg 真鏡面 $s:name off[0]
mesg 真鏡面 $s:name on[1]
// 子画面との併用は不可！！
// -axis オプションと -normal オプションは排他なので注意
// -axis 0 と -normal 1,0,0 は同じ結果になる
// -axis 1 と -normal 0,1,0 は同じ結果になる
// -axis 2 と -normal 0,0,1 は同じ結果になる

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

#include "../other/vec_util.h"

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	int			name ;			/* キャラ名 */
	int			map ;			/* キャラ存在マップ */
	int			axis ;			/* 反転軸フラグ */
	FVECTOR		mirror_point ;	/* 鏡面位置 */
	FVECTOR		mirror_plane ;	/* 鏡面パラメータ */
	int			flag ;
	int			old_active_flag ;
	int			old_bg_clear_flag ;
} Work ;


/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg ;

	GTE_InitGTE();

	/* メッセージチェック */
	if ( GV_PauseLevel == 0 ){
		if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for ( ; n_msg > 0 ; n_msg--, msg++ ){
				switch ( msg->message[0] ){
				  case 0:
					work->flag = 0 ;
					break ;
				  case 1:
					work->flag = 1 ;
					break ;
				}
			}
		}
	}

	if ( GM_ChanlTargetMap[0] == work->map && work->flag ){
		
		FMATRIX		eye, eye_inv ;
		FVECTOR		*v ;
		float		len ;

		/* 現在のカメラマトリクス取得 */
		eye = DG_Chanl( 0 )->eye ;

		/* 鏡面用カメラマトリクスの生成 */
		v = (FVECTOR*)&eye ;
		len = v[3].vx * work->mirror_plane.vx + v[3].vy * work->mirror_plane.vy +
		  v[3].vz * work->mirror_plane.vz + v[3].vw * work->mirror_plane.vw ;
		v[3].vx -= len * 2.0f * work->mirror_plane.vx ;
		v[3].vy -= len * 2.0f * work->mirror_plane.vy ;
		v[3].vz -= len * 2.0f * work->mirror_plane.vz ;

		len = v[0].vx * work->mirror_plane.vx + v[0].vy * work->mirror_plane.vy + v[0].vz * work->mirror_plane.vz ;
		v[0].vx -= len * 2.0f * work->mirror_plane.vx ;
		v[0].vy -= len * 2.0f * work->mirror_plane.vy ;
		v[0].vz -= len * 2.0f * work->mirror_plane.vz ;
		len = v[1].vx * work->mirror_plane.vx + v[1].vy * work->mirror_plane.vy + v[1].vz * work->mirror_plane.vz ;
		v[1].vx -= len * 2.0f * work->mirror_plane.vx ;
		v[1].vy -= len * 2.0f * work->mirror_plane.vy ;
		v[1].vz -= len * 2.0f * work->mirror_plane.vz ;
		len = v[2].vx * work->mirror_plane.vx + v[2].vy * work->mirror_plane.vy + v[2].vz * work->mirror_plane.vz ;
		v[2].vx -= len * 2.0f * work->mirror_plane.vx ;
		v[2].vy -= len * 2.0f * work->mirror_plane.vy ;
		v[2].vz -= len * 2.0f * work->mirror_plane.vz ;

		/* チャンネル１に対して直接カメラ関連パラメータを書き込む */
		_sceVu0InversMatrix( &eye_inv, &eye );
		DG_Chanl( 1 )->eye = eye ;
		DG_Chanl( 1 )->eye_inv = eye_inv ;
		DG_Chanl( 1 )->pers = DG_Chanl( 0 )->pers ;
		DG_Chanl( 1 )->raise_pers = DG_Chanl( 0 )->raise_pers ;
		DG_Chanl( 1 )->screen = DG_Chanl( 0 )->screen ;
		GTE_MulMatrix( &(DG_Chanl( 1 )->eye_pers), &(DG_Chanl( 1 )->pers), &eye_inv );
		GTE_MulMatrix( &(DG_Chanl( 1 )->raise_eye_pers), &(DG_Chanl( 1 )->raise_pers), &eye_inv );
#ifdef KP_XBOX
		DG_Chanl( 1 )->xpers = DG_Chanl( 0 )->xpers ;
		DG_Chanl( 1 )->raise_xpers = DG_Chanl( 0 )->raise_xpers ;
		GTE_MulMatrix( &(DG_Chanl( 1 )->eye_xpers), &(DG_Chanl( 1 )->xpers), &eye_inv );
		GTE_MulMatrix( &(DG_Chanl( 1 )->raise_eye_xpers), &(DG_Chanl( 1 )->raise_xpers), &eye_inv );
#endif

		if ( work->old_active_flag == 0 ){
			/* カメラ以外の設定を行う（最初の１回目のみ） */
			DG_SetDrawEnv( DG_Chanl( 1 ),
						  DG_Chanl( 0 )->offset_x, DG_Chanl( 0 )->offset_y,
						  DG_Chanl( 0 )->width, DG_Chanl( 0 )->height );
			//DG_Chanl( 1 )->group_id = DG_Chanl( 0 )->group_id ;
			DG_Chanl( 1 )->group_id = GM_GetDGGroupID( work->map );	/* 対象を自分と同じマップのみに設定する */

			DG_Chanl( 1 )->flag = 1 ;			/* サブ画面のＯＮ */
			DG_SetReverseDrawOrderMode( 1 );	/* サブ画面の描画をメインの前に持っていく */
			work->old_bg_clear_flag = DG_Chanl( 0 )->bg_clear_flag ;
			DG_Chanl( 0 )->bg_clear_flag = 0 ;	/* メイン描画時に画面初期化を行わないように設定 */
			work->old_active_flag = 1 ;
		}
		DG_Chanl( 1 )->group_id = GM_GetDGGroupID( work->map );	/* 対象を自分と同じマップのみに設定する */
	} else {
		if ( work->old_active_flag ){
			/* チャンネルの状態を元に戻す（最初の１回目のみ） */
			DG_Chanl( 1 )->flag = 0 ;			/* サブ画面のＯＦＦ */
			DG_SetReverseDrawOrderMode( 0 );	/* 描画順を元に戻す */
			DG_Chanl( 0 )->bg_clear_flag = work->old_bg_clear_flag ;	/* 背景消去フラグを元に戻す */
			work->old_active_flag = 0 ;
		}
	}
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->map = where ;

	/* 設置座標 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		work->mirror_point.vx = (float)GCL_GetNextInt() ;
		work->mirror_point.vy = (float)GCL_GetNextInt() ;
		work->mirror_point.vz = (float)GCL_GetNextInt() ;
		work->mirror_point.vw = 1.0f ;
	}

	if ( GCL_GetOption( 'a' ) != NULL ) {
		work->axis = GCL_GetNextInt() ;
		switch ( work->axis ){
		  case 0:
			work->mirror_plane.vx = 1.0f ;
			work->mirror_plane.vy = 0.0f ;
			work->mirror_plane.vz = 0.0f ;
			break ;
		  case 1:
			work->mirror_plane.vx = 0.0f ;
			work->mirror_plane.vy = 1.0f ;
			work->mirror_plane.vz = 0.0f ;
			break ;
		  case 2:
			work->mirror_plane.vx = 0.0f ;
			work->mirror_plane.vy = 0.0f ;
			work->mirror_plane.vz = 1.0f ;
			break ;
		}
	}

	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->mirror_plane.vx = (float)GCL_GetNextInt() ;
		work->mirror_plane.vy = (float)GCL_GetNextInt() ;
		work->mirror_plane.vz = (float)GCL_GetNextInt() ;
		GTE_Normalize( &work->mirror_plane, &work->mirror_plane );
	}

	if ( GCL_GetOption( 'e' ) != NULL ) {
		work->flag = 1 ;
	}

	work->mirror_plane.vw = -GTE_InnerProduct( &work->mirror_point, &work->mirror_plane );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewTrueMirror( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
