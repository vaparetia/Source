//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	smkreact.c
	足元用反発煙エフェクトサーバー

	2001/07/03 K.Takabe
	$Id: smkreact.c,v 1.1.1.3 2002/11/19 11:51:27 Yoshizawa1 Exp $

*/
/*



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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
/* エフェクトパラメータ */


extern float BP_AdjustTick4(float);
#define FALL_SPEED	(BP_AdjustTick4(-5.0f))


#define START_RANGE		(250)
#define START_HEIGHT	(100)

/* ---------------------------------------------------------------- */
#define MAX_EFFECT		(32)	/* 一度に出せる最大エフェクト数 */
#define USE_PACKET		(1)		/* １つのエフェクトに使用するパケット数 */
/* ---------------------------------------------------------------- */
/* プリミティブパラメータ */
#define PRIM_FLAG	(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_VERTS		(8)
#define N_PRIMS		(MAX_EFFECT*USE_PACKET)
//#define TEXTURE		(4625891)		/* "bombgas1_alp" */
//#define TEXTURE		(9868771)		/* "bombgas6_alp" */
#define TEXTURE		(7361654)		/* "w00_sky_add_alp" */

/* ---------------------------------------------------------------- */
/* スクラッチパッド使用状況 */
typedef struct _scrpad{/* 合計１６ＫＢ以下であること！ */
	FVECTOR				pos[ 256 ] ;
	FVECTOR				vvec[ 256 ] ;
	DG_PRIM2_UVRGBWH	uvrgbwh[ 256 ] ;
} ScrpadWork ;

/* ---------------------------------------------------------------- */
typedef struct {
	int		count ;
	int		height ;
	int		speed ;
	int		range ;
	FVECTOR	sincos ;
	//FVECTOR	pos ;
	FMATRIX	mat ;
} EffectControl ;
/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			name ;

	DG_PRIM2	*prim ;
	int			u0, v0 ;
	int			u1, v1 ;

	EffectControl	effect_ctrl[ MAX_EFFECT ] ;
	FVECTOR		base_pos[ N_VERTS * USE_PACKET ];

	FVECTOR		old_player_pos ;

} Work ;

static Work *work_ptr = NULL;

/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

#if 0
/* メインメモリからスクラッチパッドへ転送 */
static void SprLoad( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartMemToSpr( scrpad, mem, num );
	DG_EndMemToSpr() ;
}
#endif
/* スクラッチパッドからメインメモリへ転送 */
static void SprStore( void *scrpad, void *mem, int num )
{
	/* ＤＭＡを使用しているので使用には細心の注意を払うこと！！ */
	//FlushCache( 0 );
	DG_StartSprToMem( mem, scrpad, num );
	DG_EndSprToMem() ;
}

/* ---------------------------------------------------------------- */
static EffectControl *GetEffectEntry( Work *work )
{
	EffectControl	*e_ctrl ;
	int		i ;
	e_ctrl = work->effect_ctrl ;
	for ( i = 0 ; i < MAX_EFFECT ; i++, e_ctrl++ ){
		if ( e_ctrl->count == 0 ){
			//printf("%s: get entry %d\n", __FILE__, i );
			return ( e_ctrl );
		}
	}
	return ( NULL );
}

static void SetEffect( FMATRIX *world, float len )
{
	Work				*work = work_ptr ;
	EffectControl		*e_ctrl ;

	if ( work_ptr == NULL ) return ;
	e_ctrl = GetEffectEntry( work );
	if ( e_ctrl != NULL ){
		e_ctrl->mat = *world ;
		e_ctrl->range = START_RANGE ;
		e_ctrl->count = 32 ;
		e_ctrl->height = START_HEIGHT ;
		e_ctrl->speed = len / 2 ;
		if ( e_ctrl->speed > 50 ) e_ctrl->speed = 50 ;
		GTE_SinCos( &e_ctrl->sincos, GTE_PS2RAD( RND( 4096 ) ) );
	}
}
/* ---------------------------------------------------------------- */
/* １エフェクト処理 */
static void SetPacket( Work *work, EffectControl *e_ctrl, FVECTOR *pos, DG_PRIM2_UVRGBWH *uvrgbwh )
{
	FMATRIX	mat ;
	float	r, s ;
	int		i, a ;

	r = e_ctrl->range * GTE_PI * 2 / (float)( N_VERTS * USE_PACKET ) ;
	r *= 2 ;
	s = e_ctrl->height * 2 ;
	mat = e_ctrl->mat ;
	GTE_ScaleVector( (FVECTOR*)mat.m[0], (FVECTOR*)mat.m[0], e_ctrl->range );
	GTE_ScaleVector( (FVECTOR*)mat.m[1], (FVECTOR*)mat.m[1], e_ctrl->range );
	GTE_ScaleVector( (FVECTOR*)mat.m[2], (FVECTOR*)mat.m[2], e_ctrl->range );
	mat.m[3][1] += s * 0.5f ;
	mat.m[3][3] = 1.0f ;

	a = e_ctrl->count * 32 / 32 ;
	a = a * a / 32 ;

	GTE_LoadMatrix( &mat );
	GTE_PutVector( pos, work->base_pos, N_VERTS * USE_PACKET );
	uvrgbwh->u0 = work->u0 ;
	uvrgbwh->v0 = work->v0 ;
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->u1 = work->u1 ;
	uvrgbwh->v1 = work->v1 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f1 = 0x0fff ;
	uvrgbwh->r = 48 ;
	uvrgbwh->g = 48 ;
	uvrgbwh->b = 48 ;
	uvrgbwh->a = a ;
	uvrgbwh->w = e_ctrl->sincos.vx * s ;
	uvrgbwh->h = e_ctrl->sincos.vy * s ;
	for ( i = 0 ; i < N_VERTS * USE_PACKET ; i++ ){
		uvrgbwh[ i ] = uvrgbwh[ 0 ];
	}

	e_ctrl->range += 30.0f ;
	e_ctrl->count-- ;
	e_ctrl->height += e_ctrl->speed ;
	e_ctrl->speed += (int)FALL_SPEED ;
	if ( e_ctrl->height < 10 ) e_ctrl->height = 10 ;
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_PACKET	*packet ;
	FVECTOR		*pos, *old_pos, *spr_pos ;
	DG_PRIM2_UVRGBWH		*uvrgbwh, *old_uvrgbwh, *spr_uvrgbwh ;
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	int			i, j ;
	EffectControl	*e_ctrl ;

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
	packet = prim->packet[ prim->buffer_clock ] ;

	/* スクラッチパッド上でのデータアドレス取得 */
	spr_pos = scrpad->pos ;
	spr_uvrgbwh = scrpad->uvrgbwh ;

	e_ctrl = work->effect_ctrl ;
	for ( i = 0 ; i < MAX_EFFECT ; i++ ){
		/* エフェクト単位での処理 */
		if ( e_ctrl->count ){

			/*  */
			SetPacket( work, e_ctrl, spr_pos, spr_uvrgbwh );
			
			/* 表示をＯＮ */
			for ( j = 0 ; j < USE_PACKET ; j++, packet++ ){
				packet->flag &= ~DG_PRIM2_INVISIBLE ;
			}
		} else {
			/* 表示をＯＦＦ */
			for ( j = 0 ; j < USE_PACKET ; j++, packet++ ){
				packet->flag |= DG_PRIM2_INVISIBLE ;
			}
		}
		e_ctrl++ ;
		spr_pos += N_VERTS * USE_PACKET ;
		spr_uvrgbwh += N_VERTS * USE_PACKET ;
	}
	FlushCache( 0 );/* ＤＭＡ転送をする前には必ず必要 */
	SprStore( scrpad->pos    , pos       , SIZEOF_QWORD(FVECTOR) * ( N_VERTS * N_PRIMS ) );
	SprStore( scrpad->uvrgbwh, uvrgbwh   , SIZEOF_QWORD(DG_PRIM2_UVRGBWH) * ( N_VERTS * N_PRIMS ) );


#if 1
	{/* プレイヤーの位置に煙を出現させる */
		EffectControl	*e_ctrl ;
		FVECTOR			tmp_vec ;
		float			len ;

		GTE_SubVector( &tmp_vec, &GM_PlayerPosition, &work->old_player_pos );
		tmp_vec.vy = 0 ;
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		if ( len > 4.0f ){
			e_ctrl = GetEffectEntry( work );
			if ( e_ctrl != NULL ){
				//e_ctrl->pos = GM_PlayerPosition ;
				//e_ctrl->pos.vy = 4000.0f ;
				GTE_MakeRotate( &e_ctrl->mat, 0, 1, 0, GTE_PS2RAD( RND( 4096 ) ) );
				*(FVECTOR*)e_ctrl->mat.m[3] = GM_PlayerPosition ;
				e_ctrl->mat.m[3][1] = 4000.0f ;
				e_ctrl->range = START_RANGE ;
				e_ctrl->count = 32 ;
				e_ctrl->height = START_HEIGHT ;
				//e_ctrl->speed = 20.0f ;
				e_ctrl->speed = len / 2 ;
				if ( e_ctrl->speed > 50 ) e_ctrl->speed = 50 ;
				GTE_SinCos( &e_ctrl->sincos, GTE_PS2RAD( RND( 4096 ) ) );
			}
		}
		work->old_player_pos = GM_PlayerPosition ;
	}
#else
#if 0
	{/* プレイヤーの位置に煙を出現させる */
		FMATRIX			mat ;
		FVECTOR			tmp_vec ;
		float			len ;

		GTE_SubVector( &tmp_vec, &GM_PlayerPosition, &work->old_player_pos );
		tmp_vec.vy = 0 ;
		len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
		if ( len > 4.0f ){
			GTE_MakeRotate( &mat, 0, 1, 0, GTE_PS2RAD( RND( 4096 ) ) );
			*(FVECTOR*)mat.m[3] = GM_PlayerPosition ;
			mat.m[3][1] = 4000.0f ;
			mat.m[3][3] = 1.0f ;
//			SetEffect( &mat, len );
		}
		work->old_player_pos = GM_PlayerPosition ;
	}
#endif
#endif

}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	GM_FreePrim2( work->prim );
	work_ptr = NULL ;
}
/* ---------------------------------------------------------------- */
#if 0
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
#endif
/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	DG_TEX		*tex ;
	FVECTOR		*pos ;
	int			i, j, k ;
	//ScrpadWork	*scrpad = SCRPAD_ADDR ;

	work->map = GM_CurrentMap ;

	/* テクスチャの取得 */
	tex = DG_GetTexture( TEXTURE );
	//if ( tex == NULL ) printf("%s:no texture(%s)\n", __FILE__, TEXTURE );

	/* プリミティブの確保 */
	work->prim = prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	if ( prim == NULL ) return ( -1 );
	DG_ConfigPrim2Tex( prim, tex );
	prim->flag |= DG_PRIM2_INVISIBLE ;

	for ( k = 0 ; k < 2 ; k++ ){
		pos = prim->pos[ k ] ;
		/* 頂点データの初期化 */
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j++, pos++ ){
				/* 頂点の設定 */
				pos->vx = 0.0f ;
				pos->vy = 0.0f ;
				pos->vz = 0.0f ;
				pos->vw = 1.0f ;
			}
		}
	}

	work->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
	work->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
	work->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
	work->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;
	for ( k = 0 ; k < 2 ; k++ ){
		uvrgbwh = prim->uvrgb[ k ] ;
		{/* 頂点データの初期化 */
			int		u0, v0, u1, v1 ;
			u0 = work->u0 ; v0 = work->v0 ;
			u1 = work->u1 ; v1 = work->v1 ;
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
					uvrgbwh->r = 0 ;
					uvrgbwh->g = 0 ;
					uvrgbwh->b = 0 ;
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
	}

	/* 全パケットを非表示に */
	for ( k = 0 ; k < 2 ; k++ ){
		DG_PRIM2_PACKET	*packet ;
		packet = prim->packet[ k ] ;
		for ( i = 0 ;  i < N_PRIMS ; i++ ){
			packet->flag |= DG_PRIM2_INVISIBLE ;
			packet++ ;
		}
	}

	{/* エフェクト制御構造体の初期化 */
		EffectControl	*e_ctrl ;
		e_ctrl = work->effect_ctrl ;
		for ( i = 0 ; i < MAX_EFFECT ; i++ ){
			e_ctrl->count = 0 ;
			e_ctrl++ ;
		}
	}

	/* ベース座標を生成 */
	for ( i = 0 ; i < N_VERTS * USE_PACKET ; i++ ){
		FVECTOR		tmp_vec ;
		GTE_SinCos( &tmp_vec, GTE_PS2RAD( i * 4096 / ( N_VERTS * USE_PACKET ) ) );
		work->base_pos[i].vx = tmp_vec.vy ;
		work->base_pos[i].vy = 0.0f ;
		work->base_pos[i].vz = tmp_vec.vx ;
		work->base_pos[i].vw = 1.0f ;
	}

	work->old_player_pos = GM_PlayerPosition ;

	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *NewSmokeReactionServer( void )
{
	Work		*work ;

	if ( work_ptr != NULL ) return ( NULL );

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		//GV_SetActorSignalFunc( work, ReceiveSignal );
		work_ptr = work ;
	}
	return (void *)work ;
}






/* ---------------------------------------------------------------- */
extern int DM_FrameSkip ;
/* ---------------------------------------------------------------- */
	/*
		デモ関連
	*/

typedef struct {
	GV_ACT_EX	actor ;
	FVECTOR		old_pos ;
	FMATRIX		*world ;
	FVECTOR		*now_pos ;
} DemoWork ;


static void DemoAct( DemoWork *work )
{
	int		i, n_loop ;
	FVECTOR	pos, old_pos, now_pos ;

	n_loop = DM_FrameSkip + 1 ;
	old_pos = work->old_pos ;
	now_pos = *work->now_pos ;
	for ( i = 0 ; i < n_loop ; i++ ){
		GTE_InterVector( &pos, &old_pos, &now_pos, (float)( i + 1 ) / (float)n_loop );
		{/* １フレーム分の処理 */
			FVECTOR			tmp_vec ;
			FMATRIX			mat ;
			float			len ;

			GTE_SubVector( &tmp_vec, &pos, &work->old_pos );
			tmp_vec.vy = 0 ;
			len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
			if ( len > 4.0f ){
				GTE_MakeRotate( &mat, 0, 1, 0, GTE_PS2RAD( RND( 4096 ) ) );
				*(FVECTOR*)mat.m[3] = pos ;
				mat.m[3][1] = 4000.0f ;
				mat.m[3][3] = 1.0f ;
				SetEffect( &mat, len );
			}
			work->old_pos = pos ;
		}
	}
}

static void DemoDie( DemoWork *work )
{
}

static int DemoGetResources( DemoWork *work, FMATRIX *root )
{
	work->world = root ;
	work->now_pos = (FVECTOR*)root->m[3] ;
	work->old_pos = *( work->now_pos );
	return ( 0 );
}

void *NewSmokeReactionClient( FMATRIX *root )
{
	DemoWork		*work ;

	OPERATOR() ;
	work = (DemoWork *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( DemoWork ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), DemoAct, DemoDie ) ;
		GV_ActorEX( &work->actor );
		if ( DemoGetResources( work, root ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

