//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_smk3.c
	量産型ＲＡＹ用口から煙

	2001/07/19 K.Takabe
	$Id: ef_smk3.c,v 1.1.1.3 2002/11/19 11:51:21 Yoshizawa1 Exp $

*/
/*



*/
/* !NEED_PAL_READJUST! */

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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
extern int DM_FrameSkip ;
/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
/* シグナル関連 */
#define SIGNAL_RAYBREATHSMOKE			(0x78fe7700)	/* GV_StrCode("NewRAYBreathSmoke")<<8 */
#define SIGNAL_RAYBREATHSMOKE_STOP		(SIGNAL_RAYBREATHSMOKE|0x01)
/* ローカル用再定義 */
#define SIGNAL_THIS_EFFECT	(SIGNAL_RAYBREATHSMOKE)
#define SIGNAL_STOP			(SIGNAL_THIS_EFFECT|0x01)
/* ---------------------------------------------------------------- */
/* エフェクトパラメータ */
#define GEN_NUM			(2)		/* １フレームの発生数 */
#define ALIVE_FRAME		(16*2)	/* スプライトの生存フレーム数 */
#if 1
#define START_SIZE		( 1800 )
#define ZOOM_OFFSET		( 100 )	/* １フレームあたりの拡大サイズ */
#define SMOKE_SPEED		( 800 )
#define SMOKE_SCATTER	( 80 )
#define SMOKE_LOSE		( 0.80f )
#endif
#define COLOR_R			(48/3)
#define COLOR_G			(80/3)
#define COLOR_B			(108/3)

/* ---------------------------------------------------------------- */
/* プリミティブパラメータ */
#define PRIM_FLAG	(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_VERTS		(16)
#define N_PRIMS		(ALIVE_FRAME/N_VERTS*GEN_NUM)
//#define TEXTURE		(4625891)		/* "bombgas1_alp" */
//#define TEXTURE			(12296685)	/* "light10_msk" */
#define TEXTURE			(3594043)	/* "drop01_msk" */


#define UVRGBWH_WH( _e, _w, _h ) { (_e)->w = (_w) ; (_e)->h = (_h) ; }
#define UVRGBWH_RGBA( _e, _r, _g, _b, _a ) {\
											 (_e)->r = (_r) ;\
											 (_e)->g = (_g) ;\
											 (_e)->b = (_b) ;\
											 (_e)->a = (_a) ;\
									   }
#define UVRGBWH_UV( _e, _u0, _v0, _u1, _v1 ) {\
											 (_e)->u0 = (_u0) ;\
											 (_e)->v0 = (_v0) ;\
											 (_e)->q0 = 4096 ;\
											 (_e)->f0 = 0x0fff;\
											 (_e)->u1 = (_u1) ;\
											 (_e)->v1 = (_v1) ;\
											 (_e)->q1 = 4096 ;\
											 (_e)->f1 = 0x0fff;\
									   }

/* ---------------------------------------------------------------- */
/* スクラッチパッド使用状況 */
typedef struct _scrpad{/* 合計１６ＫＢ以下であること！ */
	FVECTOR				pos[ 256 ] ;
	FVECTOR				vvec[ 256 ] ;
	DG_PRIM2_UVRGBWH	uvrgbwh[ 256 ] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;

	DG_PRIM2	*prim ;
	FMATRIX		*parent ;					/* 親ミサイルのマトリクス */
	int			phase ;						/* プログラム実行フェーズ */
	int			die_count ;					/* 終了カウント */
	FMATRIX		old_world ;

	FVECTOR		vvec[ ALIVE_FRAME * GEN_NUM ] ;		/* 頂点速度 */
	FVECTOR		old_scatter ;

} Work ;


/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos, *old_pos, *spr_pos ;
	FVECTOR		*spr_vvec ;
	DG_PRIM2_UVRGBWH		*uvrgbwh, *old_uvrgbwh, *spr_uvrgbwh ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int			i, j, a ;
	FVECTOR		scatter[5] ;
	FMATRIX		world ;

	GM_CurrentMap = work->map ;

	GTE_InitGTE();

	prim = work->prim ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;

	/* 以前の頂点データを参照するためアドレスを記憶 */
	old_pos = prim->pos[ prim->buffer_clock ] ;
	old_uvrgbwh = prim->uvrgb[ prim->buffer_clock ] ;

	/* プリミティブ頂点データのダブルバッファ切り替え */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	/* 書き込み用頂点データアドレス取得 */
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgbwh = prim->uvrgb[ prim->buffer_clock ] ;

	for ( j = 0 ; j < DM_FrameSkip + 1 ; j++ ){
		world = *( work->parent );
		GTE_InterVector( (FVECTOR*)world.m[3], (FVECTOR*)work->old_world.m[3], (FVECTOR*)work->parent->m[3],
						(float)( j + 1 ) / (float)( DM_FrameSkip + 1 ) );

		/* スクラッチパッド上でのデータアドレス取得 */
		spr_pos = scrpad->pos ;
		spr_vvec = scrpad->vvec ;
		spr_uvrgbwh = scrpad->uvrgbwh ;

		/* 頂点データ記憶位置をシフトする */
		FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
		SprLoad( &scrpad->pos[0]    , &old_pos[GEN_NUM]    ,
				SIZEOF_QWORD(FVECTOR) * ( ALIVE_FRAME * GEN_NUM - GEN_NUM ) );
		SprLoad( &scrpad->vvec[0]   , &work->vvec[GEN_NUM] ,
				SIZEOF_QWORD(FVECTOR) * ( ALIVE_FRAME * GEN_NUM - GEN_NUM ) );
		SprLoad( &scrpad->uvrgbwh[0], &old_uvrgbwh[GEN_NUM],
				SIZEOF_QWORD(DG_PRIM2_UVRGBWH) * ( ALIVE_FRAME * GEN_NUM - GEN_NUM ) );

		spr_pos = scrpad->pos + ALIVE_FRAME * GEN_NUM - GEN_NUM ;
		spr_vvec = scrpad->vvec + ALIVE_FRAME * GEN_NUM - GEN_NUM ;
		spr_uvrgbwh = scrpad->uvrgbwh + ALIVE_FRAME * GEN_NUM - GEN_NUM ;

		/* 新規頂点設定 */
		switch ( work->phase ){
		  case 0:/* 煙生成フェーズ */
			{/* 頂点移動量の算出 */
				FVECTOR		base_vec ;
#if 0
				base_vec.vz = SMOKE_SPEED + RND( SMOKE_SCATTER );
				base_vec.vx = RND( SMOKE_SCATTER ) - SMOKE_SCATTER / 2 ;
				base_vec.vy = RND( SMOKE_SCATTER ) - SMOKE_SCATTER / 2 ;
				base_vec.vw = 0.0f ;
#else
				{
					FMATRIX	mat ;
					FVECTOR	vec ;
					float	len, angle_x, angle_z ;
					int		r_x ;
					r_x = RND( 256 ) ;
					angle_x = GTE_PS2RAD( r_x );
					angle_z = GTE_PS2RAD( RND( 4096 ) );
					len = RND( ( SMOKE_SPEED ) ) + SMOKE_SPEED ;
					len *= 256 / (float)( r_x + 256 ) ;
					GTE_SetVector( &vec, 0, 0, len, 1 );
					GTE_UnitMatrix( &mat );
					_sceVu0RotMatrixX( &mat, &mat, angle_x );
					_sceVu0RotMatrixZ( &mat, &mat, angle_z );
					GTE_ApplyMatrix( &base_vec, &mat, &vec );
					base_vec.vw = 0.0f ;
				}
#endif

				/* ２点間の要素をフラクタルで生成 */
				scatter[4] = work->old_scatter ;
				scatter[0] = base_vec ;
				GTE_AddVector( &scatter[2], &scatter[0], &scatter[4] );
				GTE_ScaleVector( &scatter[2], &scatter[2], 0.5f );
				scatter[2].vz += RND( SMOKE_SCATTER/2 );
				scatter[2].vx += RND( SMOKE_SCATTER/2 ) - SMOKE_SCATTER / 4 ;
				scatter[2].vy += RND( SMOKE_SCATTER/2 ) - SMOKE_SCATTER / 4 ;

				GTE_AddVector( &scatter[1], &scatter[0], &scatter[2] );
				GTE_ScaleVector( &scatter[1], &scatter[1], 0.5f );
				scatter[1].vz += RND( SMOKE_SCATTER/4 );
				scatter[1].vx += RND( SMOKE_SCATTER/4 ) - SMOKE_SCATTER / 8 ;
				scatter[1].vy += RND( SMOKE_SCATTER/4 ) - SMOKE_SCATTER / 8 ;

				GTE_AddVector( &scatter[3], &scatter[2], &scatter[4] );
				GTE_ScaleVector( &scatter[3], &scatter[3], 0.5f );
				scatter[3].vz += RND( SMOKE_SCATTER/4 );
				scatter[3].vx += RND( SMOKE_SCATTER/4 ) - SMOKE_SCATTER / 8 ;
				scatter[3].vy += RND( SMOKE_SCATTER/4 ) - SMOKE_SCATTER / 8 ;

				work->old_scatter = base_vec ;

				//GTE_ApplyMatrix( &spr_vvec[GEN_NUM-1], work->parent, &base_vec );
				//spr_vvec[GEN_NUM-1].vw = 0.0f ;
			}
			for ( i = 0 ; i < GEN_NUM ; i++ ){
				float	t ;
				t = ( i + 1 ) / (float)( GEN_NUM ) ;
				//spr_pos[0] = *(FVECTOR*)work->parent->m[3] ;
				//GTE_InterVector( &spr_pos[i], &spr_pos[-1], (FVECTOR*)work->parent->m[3], t );
				GTE_InterVector( &spr_pos[i], &spr_pos[-1], (FVECTOR*)world.m[3], t );
				spr_uvrgbwh[i] = spr_uvrgbwh[-1] ;
				spr_uvrgbwh[i].r = COLOR_R ;
				spr_uvrgbwh[i].g = COLOR_G ;
				spr_uvrgbwh[i].b = COLOR_B ;
				spr_uvrgbwh[i].a = 64 ;
				{/* 回転スプライトの大きさ及び角度を決定する */
					FVECTOR		sincos, size, offset ;
					GTE_SinCos( &sincos, GTE_PS2RAD( RND( 4096 ) ) );
					GTE_ScaleVector( &size, &sincos, START_SIZE );
					GTE_ScaleVector( &offset, &sincos, ZOOM_OFFSET );
					spr_uvrgbwh[i].w = FTOI( size.vx ) ;
					spr_uvrgbwh[i].h = FTOI( size.vy ) ;
					spr_uvrgbwh[i].pad0 = FTOI( offset.vx ) ;/* 頂点データの未使用領域にオフセット値を設定 */
					spr_uvrgbwh[i].pad1 = FTOI( offset.vy ) ;/* 頂点データの未使用領域にオフセット値を設定 */
				}
#if 0
				{/* 頂点移動量の算出 */
					FVECTOR		base_vec ;
					base_vec.vz = -SMOKE_SPEED + RND( SMOKE_SCATTER );
					base_vec.vx = RND( SMOKE_SCATTER ) - SMOKE_SCATTER / 2 ;
					base_vec.vy = RND( SMOKE_SCATTER ) - SMOKE_SCATTER / 2 ;
					base_vec.vw = 0.0f ;
					GTE_ApplyMatrix( &spr_vvec[i], work->parent, &base_vec );
					spr_vvec[i].vw = 0.0f ;
				}
#else
				//GTE_InterVector( &spr_vvec[i], &spr_vvec[-1], &spr_vvec[GEN_NUM-1], t );
				//GTE_ApplyMatrix( &spr_vvec[i], work->parent, &scatter[i] );
				GTE_ApplyMatrix( &spr_vvec[i], &world, &scatter[i] );
				spr_vvec[i].vw = 0.0f ;
#endif
			}
			break ;
		  case 1:/* フェードフェーズ */
			for ( i = 0 ; i < GEN_NUM ; i++ ){
				GTE_ZeroVector( &spr_pos[i] );	/* 移行の頂点データは原点へ */
				spr_uvrgbwh[i].a = 0 ;
				spr_uvrgbwh[i].w = 0 ;
				spr_uvrgbwh[i].h = 0 ;
			}
			work->die_count++ ;
			break ;
		}

		spr_pos = scrpad->pos ;
		spr_vvec = scrpad->vvec ;
		spr_uvrgbwh = scrpad->uvrgbwh ;

		/* 頂点データのアニメーション処理 */
		for ( i = 0 ; i < ALIVE_FRAME * GEN_NUM ; i++ ){
			if ( spr_uvrgbwh->a != 0 ){/* データがあるもののみ更新するようにする */
				/* 頂点位置アニメーション */
				GTE_AddVector( spr_pos, spr_pos, spr_vvec );
				GTE_ScaleVector( spr_vvec, spr_vvec, SMOKE_LOSE );
				/* サイズ変更 */
				spr_uvrgbwh->w += spr_uvrgbwh->pad0 ;
				spr_uvrgbwh->h += spr_uvrgbwh->pad1 ;
				/* カラーアニメーション */
				//a = spr_uvrgbwh->r - 128 / ALIVE_FRAME ;
				//if ( a < 0 ) a = 0 ;
#if 0
				a = 128 * ( i + 1 ) / ( ALIVE_FRAME * GEN_NUM ) ;
				spr_uvrgbwh->r = a ;
				spr_uvrgbwh->g = a ;
				spr_uvrgbwh->b = a ;
#endif
				//a = spr_uvrgbwh->a - 64 / ALIVE_FRAME ;
				//if ( a < 0 ) a = 0 ;
				a = 64 * ( i + 1 ) / ( ALIVE_FRAME * GEN_NUM ) ;
				spr_uvrgbwh->a = a ;
			}
			spr_uvrgbwh++ ;
			spr_pos++ ;
			spr_vvec++ ;
		}

		/* 頂点データをメモリへ書き戻す */
		FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
		SprStore( &scrpad->pos[0]    , &pos[0]       , SIZEOF_QWORD(FVECTOR) * ( ALIVE_FRAME * GEN_NUM ) );
		SprStore( &scrpad->vvec[0]   , &work->vvec[0], SIZEOF_QWORD(FVECTOR) * ( ALIVE_FRAME * GEN_NUM ) );
		SprStore( &scrpad->uvrgbwh[0], &uvrgbwh[0]   , SIZEOF_QWORD(DG_PRIM2_UVRGBWH) * ( ALIVE_FRAME * GEN_NUM ) );
	}
	work->old_world = *( work->parent );

	if ( work->die_count > 64 ) GV_DestroyActor( work );
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GM_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work	*work = workp ;
	switch( signal ){
	  case SIGNAL_STOP:
		work->phase = 1 ;	/* フェードフェーズに移行 */
		break ;
	  default:
		/* Killシグナルなどを処理しないようにする */
		return GV_DefaultSignalFunc( workp, signal, value );
	}
	return ( 0 );
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *parent )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	DG_TEX		*tex ;
	FVECTOR		*pos ;
	int			i, j ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;

	work->map = GM_CurrentMap ;
	work->parent = parent ;
	work->old_world = *parent ;

	/* テクスチャの取得 */
	tex = DG_GetTexture( TEXTURE );
	//if ( tex == NULL ) printf("%s:no texture(%s)\n", __FILE__, TEXTURE );

	/* プリミティブの確保 */
	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	if ( prim == NULL ) return ( -1 );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
	prim->flag |= DG_PRIM2_INVISIBLE ;

	/* スクラッチパッド上でのデータアドレス取得 */
	pos = scrpad->pos ;
	/* 頂点データの初期化 */
	for ( i = 0 ;  i < N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_VERTS ; j++, pos++ ){
			/* 頂点の設定 */
#if 0
			pos->vx = 0.0f ;
			pos->vy = 0.0f ;
			pos->vz = 0.0f ;
			pos->vw = 1.0f ;
#endif
			*pos = *(FVECTOR*)parent->m[3] ;
		}
	}
	/* スクラッチパッド上のデータをメモリへ書き出す */
	FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
	SprStore( scrpad->pos, prim->pos[ 0 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS );
	SprStore( scrpad->pos, prim->pos[ 1 ], SIZEOF_QWORD(FVECTOR) * N_PRIMS * N_VERTS );

	/* スクラッチパッド上でのデータアドレス取得 */
	uvrgbwh = scrpad->uvrgbwh ;
	{/* 頂点データの初期化 */
		int		u0, v0, u1, v1 ;
		u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
		v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
		u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
		v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j++, uvrgbwh++ ){
				/* ＵＶ値設定 */
				uvrgbwh->u0 = u0 ;/* 左上 */
				uvrgbwh->v0 = v0 ;/* 左上 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->u1 = u1 ;/* 右下 */
				uvrgbwh->v1 = v1 ;/* 右下 */
				uvrgbwh->q1 = 4096 ;
				/* 頂点カラー設定 */
				uvrgbwh->r = 32 ;
				uvrgbwh->g = 64 ;
				uvrgbwh->b = 128 ;
				uvrgbwh->a = 0 ;	/* 頂点有効フラグとしても使用する */
				/* サイズの設定 */
				uvrgbwh->w = 0 ;
				uvrgbwh->h = 0 ;
				uvrgbwh->pad0 = 0 ;	/* 拡大オフセットに使用しているので注意 */
				uvrgbwh->pad1 = 0 ;	/* 拡大オフセットに使用しているので注意 */
				/* フラグの設定 */
				uvrgbwh->f0 = 0x0fff ;	/* それ以降は描画キックを行う */
				uvrgbwh->f1 = 0x0fff ;	/* それ以降は描画キックを行う */
			}
		}
	}
	/* スクラッチパッド上のデータをメモリへ書き出す */
	FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
	SprStore( scrpad->uvrgbwh, prim->uvrgb[ 0 ], SIZEOF_QWORD(DG_PRIM2_UVRGBWH) * N_PRIMS * N_VERTS );
	SprStore( scrpad->uvrgbwh, prim->uvrgb[ 1 ], SIZEOF_QWORD(DG_PRIM2_UVRGBWH) * N_PRIMS * N_VERTS );



	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewRAYBreathSmoke( FMATRIX *parent )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, parent ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GV_SetActorSignalFunc( work, ReceiveSignal );
	}
	return (void *)work ;
}

