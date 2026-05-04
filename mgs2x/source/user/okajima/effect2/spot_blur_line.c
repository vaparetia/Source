//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	spot_blur_line.c
	３Ｄ空間に前画面のテクスチャを使用したブラーを置く：ライン状に配置

	2001/01/26 S.Okajima
	$Id: spot_blur_line.c,v 1.1.1.3 2002/11/19 11:47:26 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define RND(_a)		( ( (BP_PS2_rand()>>16) * (_a) ) >> 15  )


/* 時間基準設定 */
#define CLOCK_COUNT	(BP_BASE_TICK())

/* プリミティブ設定パラメータ */
//#define N_PRIMS	(32)				/* プリミティブ数 */
#define N_PRIMS	(4)				/* プリミティブ数 */
#define N_VERTS	(8*2+1)				/* １プリミティブの頂点数 */

/* 同次座標＞テクスチャアドレス変換定数 */
#define SHIFT_SCALE			(0.98f)
#define ADDRESS_SCALE_U		(0.5f*SHIFT_SCALE)
#define ADDRESS_OFFSET_U	(0.5f)
//#define ADDRESS_SCALE_V		((float)DRAW_HEIGHT/2.0f/512.0f*SHIFT_SCALE)
//#define ADDRESS_OFFSET_V	((float)DRAW_HEIGHT/2.0f/512.0f)
#define ADDRESS_SCALE_V		((float)DRAW_HEIGHT/2.0f/DRAW_WIDTH*SHIFT_SCALE)
#define ADDRESS_OFFSET_V	((float)DRAW_HEIGHT/2.0f/DRAW_WIDTH)

#define COL_R (129)
#define COL_G (129)
#define COL_B (129)
#define COL_A (48.0f)

#define UPPER_RATIO (0.05f)
#define ADD_RATIO (0.20f)

#define LIFE (120)

/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR		pos ;		/* 座標（ｗは生命力１～０までを表す） */
	FVECTOR		size ;		/* サイズ */
} PARTICLE_WORK ;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	DG_PRIM2		*prim[2] ;
	DG_TEX_TRANS	frame_tex[2] ;

	int				life;
	float			upper_flow;
	float			add_size;

	int				buffer_switch ;

	int				n_prims ;		/* 出現パーティクル数 */
	PARTICLE_WORK	particles[ 0 ];
} Work ;

/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR		base_verts[ N_VERTS ] ;
	FVECTOR		trans_verts[ N_VERTS ] ;
	FVECTOR		pers_verts[ N_VERTS ] ;
	FMATRIX		poly_mat ;
	FMATRIX		pers_mat ;
	FVECTOR		sin_data[ ( N_VERTS + 3 ) / 4 ] ; 
	FVECTOR		cos_data[ ( N_VERTS + 3 ) / 4 ] ; 
	FVECTOR		tmp_vert[ N_VERTS + 3 ] ;
	FVECTOR		tex_scale ;
	FVECTOR		tex_offset ;
	FVECTOR		uv ;
	PARTICLE_WORK	particles[ 0 ] ;
} ScrpadWork ;
/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	ScrpadWork	*scrpad = SCRPAD_ADDR ;
	FVECTOR		*dst_pos ;
	PARTICLE_WORK	*particle, *scrpad_particle ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int			i, j ;
	float		flife;
	float		fratio;

	flife = 1.0f / (float)(work->life);

	if ( GV_PauseLevel == 0 ){ /* パーティクルアニメーション処理 */
		if( work->life-- < 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
		particle = work->particles ;
		scrpad_particle = scrpad->particles ;
		for ( i = N_PRIMS ; i > 0 ; particle++, scrpad_particle++, i-- ){
			fratio = particle->pos.vw;
			scrpad_particle->pos.vx = particle->pos.vx ;
			scrpad_particle->pos.vy = particle->pos.vy + work->upper_flow*fratio;
			scrpad_particle->pos.vz = particle->pos.vz ;
			scrpad_particle->pos.vw = fratio - flife;

			fratio*= work->add_size;
			scrpad_particle->size.vx = particle->size.vx + fratio;
			scrpad_particle->size.vy = particle->size.vy + fratio;
			scrpad_particle->size.vz = particle->size.vz + fratio;

			/* メモリへ書き戻す */
			particle->pos = scrpad_particle->pos ;
			particle->size = scrpad_particle->size ;
		}
	} else { /* ポーズ時の処理 */
		/* パーティクル情報をスクラッチパッドへ */
		particle = work->particles ;
		scrpad_particle = scrpad->particles ;
		for ( i = N_PRIMS ; i > 0 ; particle++, scrpad_particle++, i-- ){
			*scrpad_particle = *particle ;
		}
	}


   if (!DG_Arm_SkipThisFrame())
   {
   	work->buffer_switch = 1 - work->buffer_switch ;
   }
	work->prim[     work->buffer_switch ]->flag |= DG_PRIM2_INVISIBLE ;
	work->prim[     work->buffer_switch ]->flag &=~DG_PRIM2_INVISIBLE0 ;
	work->prim[ 1 - work->buffer_switch ]->flag |= DG_PRIM2_INVISIBLE ;
	prim = work->prim[ work->buffer_switch ] ;

	scrpad->poly_mat = DG_Chanl(0)->eye ;
	scrpad->pers_mat = DG_Chanl(0)->eye_pers ;
	{/* 基準頂点の作成 */
		float	*s_dat, *c_dat, *t_dat ;

		DG_SetPos( &scrpad->poly_mat );
		/* ＳｉｎＣｏｓ計算 */
		t_dat = (float*)scrpad->tmp_vert ;
		for ( i = 0 ; i < N_VERTS ; i++ ){
			int		iangle ;
			/* ±πになるように角度データを生成 */
			if ( i & 1 ) continue ;
			iangle = i * 65536 / ( N_VERTS - 1 ) ;
			iangle &= 0xffff ;
			if ( iangle >= 32768 ) iangle -= 65536 ;
			*t_dat++ = iangle * 3.14159265f / 32768.0f ;
		}
		for ( i = 0 ; i < ( ( N_VERTS + 1 ) / 2 + 3 ) / 4 ; i++ ){
			MT_SinX4( &scrpad->sin_data[ i ], &scrpad->tmp_vert[ i ] );
			MT_CosX4( &scrpad->cos_data[ i ], &scrpad->tmp_vert[ i ] );
		}
		s_dat = (float*)scrpad->sin_data ;
		c_dat = (float*)scrpad->cos_data ;

		dst_pos = scrpad->base_verts ;
		/* 基準円ポリゴン座標を計算 */
		for ( i = 0 ; i < N_VERTS ; dst_pos++, i++ ){
			if ( i & 1 ){
				/* 奇数頂点は原点に */
				*dst_pos = DG_ZeroVector ;
			} else {
				dst_pos->vx = ( *c_dat++ ) ;
				dst_pos->vy = ( *s_dat++ ) ;
				dst_pos->vz = 0.0f ;
				dst_pos->vw = 1.0f ;
				
			}
		}
		/* 基準円ポリゴンをカメラに対して垂直になるように回転させる */
		DG_RotVector( scrpad->base_verts, scrpad->base_verts, N_VERTS );
	}

	/* プリミティブ座標及びテクスチャ座標の生成 */
	DG_SetPos( &scrpad->pers_mat );
	scrpad->tex_scale.vx = ADDRESS_SCALE_U ;
	scrpad->tex_scale.vy = ADDRESS_SCALE_V ;
	scrpad->tex_offset.vx = ADDRESS_OFFSET_U + ( 1.0f / 8192.0f );
	scrpad->tex_offset.vy = ADDRESS_OFFSET_V + ( 1.0f / 8192.0f );

	particle = scrpad->particles ;
	dst_pos = prim->pos[ 0 ] ;
	uvrgb = prim->uvrgb[ 0 ] ;
	for ( j = 0 ; j < N_PRIMS ; particle++, j++ ){
#ifndef BP_PSX2_ASM
		float	div ;
#endif
		if ( particle->pos.vw <= 0.0f ){
			prim->packet[0][j].flag |= DG_PRIM2_INVISIBLE0 ;
			dst_pos += N_VERTS ;
			uvrgb += N_VERTS ;
			continue ;
		}
		prim->packet[0][j].flag &= ~DG_PRIM2_INVISIBLE0 ;

#ifndef BP_PSX2_ASM
		/* プリミティブ座標生成 */
		for ( i = 0 ; i < N_VERTS ; i++ ){
			_sceVu0MulVector( &scrpad->trans_verts[ i ], &scrpad->base_verts[ i ], &particle->size );
			_sceVu0AddVector( &scrpad->trans_verts[ i ], &scrpad->trans_verts[ i ], &particle->pos );
			*dst_pos++ = scrpad->trans_verts[ i ] ;
		}

		/* 透視座標計算 */
		DG_PutVector( scrpad->trans_verts, scrpad->pers_verts, N_VERTS );
		div = 1.0f / scrpad->pers_verts->vw ;
		for ( i = 0 ; i < N_VERTS ; i++ ){
			/* 透視変換 */
			scrpad->pers_verts[ i ].vx *= div ;
			scrpad->pers_verts[ i ].vy *= div ;
			/* テクスチャ座標を求める */
			_sceVu0MulVector( &scrpad->uv, &scrpad->pers_verts[ i ], &scrpad->tex_scale );
			_sceVu0AddVector( &scrpad->uv, &scrpad->uv, &scrpad->tex_offset );

			uvrgb->u = FTOI12( scrpad->uv.vx );
			uvrgb->v = FTOI12( scrpad->uv.vy );
			if ( i & 1 ){
				/* アルファ設定 */
				uvrgb->a = (int)(particle->pos.vw * COL_A) ;
			}
			uvrgb++ ;
		}
#else
		/* データのセット */
		asm volatile ("
			lqc2			vf04,0x00(%0)	# マトリクス読み込み
			lqc2			vf05,0x10(%0)	# マトリクス読み込み
			lqc2			vf06,0x20(%0)	# マトリクス読み込み
			lqc2			vf07,0x30(%0)	# マトリクス読み込み
			lqc2			vf08,0x00(%1)	# パーティクルサイズ
			lqc2			vf09,0x00(%2)	# パーティクル座標
			lqc2			vf10,0x00(%3)	# ＵＶスケール
			lqc2			vf11,0x00(%4)	# ＵＶオフセット
			vmulax.xyzw		ACC ,vf04,vf09	# 透視変換
			vmadday.xyzw	ACC ,vf05,vf09	# 透視変換
			vmaddaz.xyzw	ACC ,vf06,vf09	# 透視変換
			vmaddw.xyzw		vf31,vf07,vf00	# 透視変換
		"::"r"(&scrpad->pers_mat),"r"(&particle->size),"r"(&particle->pos),
		"r"(&scrpad->tex_scale),"r"(&scrpad->tex_offset));
		/* 変換＆ＵＶ計算 */
		asm volatile ("
			move			$8,%0			# カウンタ初期化
			move			$9,%1			# カウンタ初期化
			move			$10,%2			# カウンタ初期化
			move			$11,%3			# カウンタ初期化
			vdiv			Q,vf00w,vf31w	# 除数算出
			lqc2			vf12,0x00($11)	# 元座標読み込み（先行読み込み）
0:
			addi			$8,$8,-1		# カウンタ減算
			vmula.xyz		ACC ,vf12,vf08	# サイズ変更
			vmaddw.xyz		vf13,vf09,vf00	# 座標変更
			addiu			$11,$11,0x10	# アドレス増加
			lqc2			vf12,0x00($11)	# 元座標読み込み（先行読み込み）
			vwaitq							# 
			vmulq.xy		vf15,vf10,Q		# ＵＶスケール＆除数合成
			vmulax.xyzw		ACC ,vf04,vf13	# 透視変換
			vmadday.xyzw	ACC ,vf05,vf13	# 透視変換
			vmaddaz.xyzw	ACC ,vf06,vf13	# 透視変換
			vmaddw.xyzw		vf14,vf07,vf00	# 透視変換
			sqc2			vf13,0x00($9)	# 頂点座標書き戻し
			addiu			$9,$9,0x10	# アドレス増加
			addiu			$10,$10,0x10	# アドレス増加
			vmula.xy		ACC,vf15,vf14	# 同次座標＊除数＊ＵＶスケール計算
			vmaddw.xy		vf16,vf11,vf00	# ＵＶ座標算出
			vftoi12.xy		vf17,vf16		# ＵＶ整数化
			sqc2			vf17,-16($10)	# ＵＶ書き出し
			bgtz			$8,0b			# ループ
			nop								# 
		"::"r"(N_VERTS),"r"(scrpad->trans_verts),"r"(scrpad->pers_verts),"r"(scrpad->base_verts)
		:"$8","$9","$10","$11");

		{
			FVECTOR		*src_pos ;
			src_pos = scrpad->trans_verts ;
			for ( i = N_VERTS ; i > 0 ; i-- ){
				*dst_pos++ = *src_pos++ ;
			}
		}
		{
			IVECTOR		*src_vec ;
			src_vec = (IVECTOR*)scrpad->pers_verts ;
			for ( i = 0 ; i < N_VERTS ; i++ ){
				uvrgb->u = src_vec->vx ;
				uvrgb->v = src_vec->vy ;
				if ( i & 1 ){/* アルファ設定 */
					uvrgb->a = (int)(particle->pos.vw * COL_A) ;
				}
				if ( ( (int)uvrgb & 0x3f ) == 0 ) GV_PREFECH( uvrgb + 4 );
				src_vec++ ;
				uvrgb++ ;
			}
		}
#endif
	}

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	if ( work->prim[0] != 0 ) GM_FreePrim2( work->prim[0] );
	if ( work->prim[1] != 0 ) GM_FreePrim2( work->prim[1] );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FVECTOR *top_pos, FVECTOR *before_pos )
{
	DG_PRIM2	*prim ;
#ifdef PSX2
	DG_TEX		*tex ;
#endif
	FVECTOR		*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	PARTICLE_WORK	*particle ;
	int			i, j, k, prim_flag ;
	FVECTOR		diff;
	FVECTOR		fvtemp;
	float		len;


	work->life = LIFE;

#ifdef PSX2
	//--------------------------------------
	/* ダミーのテクスチャを取得 */
	tex = DG_GetTexture( 0 );
	/* フレームバッファテクスチャを設定 */
	work->frame_tex[0] = tex->tex_trans ;
	work->frame_tex[0].clamp.data = SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) ;
	work->frame_tex[0].alpha.data = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 64 ) ;
	work->frame_tex[1] = work->frame_tex[0] ;
	work->frame_tex[0].tex0.data =
	  SCE_GS_SET_TEX0( BUFFER_PAGE(1) / 64 , BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,9,0,0,0,0,0,0,0) ;
	work->frame_tex[1].tex0.data =
	  SCE_GS_SET_TEX0( BUFFER_PAGE(0) / 64 , BUFFER_WIDTH/64,FRAME_BUFFER_COLOR_MODE(),
					  9,9,0,0,0,0,0,0,0) ;
#endif

	/*
		同じ内容のプリミティブをシングルバッファフラグを付けて２つ用意する
		（奇数フレーム偶数フレーム表示用をそれぞれ別に確保）
	*/

	prim_flag = DG_PRIM2_POLY|DG_PRIM2_SINGLEBUFF
	  |DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA
		|(DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3) ;
#ifndef PSX2
	prim_flag |= DG_PRIM2_FRAMETEX ;
#endif
//	prim_flag &= ~DG_PRIM2_TEX ;


	//--------------------------------------
	_sceVu0SubVector( &diff, top_pos, before_pos ) ;
	_sceVu0ScaleVector( &diff, &diff, 1.0f/(float)(N_PRIMS) );


	len = GV_VecLen3F( &diff )*(0.75f+rnd()*0.25f);
	work->upper_flow = len * UPPER_RATIO;
	work->add_size   = len * ADD_RATIO;



	DG_COPY_VEC( &fvtemp, &diff );

	for ( k = 0 ; k < 2 ; k++ ){
		/* スプライトプリミティブ初期化 */
		prim = work->prim[k] = 
		  GM_MakePrim2( prim_flag, N_PRIMS, N_VERTS );
#if 0 //BP_PS2 def PSX2
		prim->tex_trans = work->frame_tex[k] ;
#else
		DG_ConfigPrim2Tex( prim, &DG_SystemTexture[ k ] );
#endif

		/* 座標初期化 */
		pos = prim->pos[ 0 ] ;/* シングルバッファなので片方だけ初期化すればよい */
		for ( i = N_PRIMS * N_VERTS ; i > 0 ; i-- ){
			_sceVu0AddVector( pos, before_pos, &fvtemp );
			pos->vy += work->upper_flow * (float)(N_PRIMS - i) / (float)(N_PRIMS);
			_sceVu0AddVector( &fvtemp, &fvtemp, &diff ) ;
			pos++ ;
		}

		/* プリミティブ状態初期化 */
		uvrgb = (DG_PRIM2_UVRGB*)prim->uvrgb[ 0 ] ;/* シングルバッファなので片方だけ初期化すればよい */
		for ( j = 0 ; j < N_PRIMS ; j++ ){
			prim->packet[0][j].flag |= DG_PRIM2_INVISIBLE ;	/* デフォルトは非表示に */
			for ( i = 0 ; i < N_VERTS ; uvrgb++, i++ ){
				uvrgb->u = FTOI12( 0.0f );
				uvrgb->v = FTOI12( 0.0f );
				uvrgb->q = FTOI12( 1.0f );
				uvrgb->f = 0x0fff ;
				if ( i & 1 || i < 2 ) uvrgb->f = 0x8fff ;
				uvrgb->r = COL_R ;
				uvrgb->g = COL_G ;
				uvrgb->b = COL_B ;
				uvrgb->a = 0 ;
			}
		}

	}

	/* 仮想パーティクル座標初期化 */
	DG_COPY_VEC( &fvtemp, &diff );
	particle = work->particles ;
	for ( i = N_PRIMS ; i > 0 ; i-- ){
		_sceVu0AddVector( &particle->pos, before_pos, &fvtemp );
		particle->pos.vx = before_pos->vx + fvtemp.vx;
		particle->pos.vy = before_pos->vy + fvtemp.vy
		          +  work->upper_flow * (float)(N_PRIMS - i) / (float)(N_PRIMS);
		particle->pos.vz = before_pos->vz + fvtemp.vz;
		particle->pos.vw = rnd()*0.8f + 0.2f;

		particle->size.vx = len;
		particle->size.vy = len;
		particle->size.vz = len;

		_sceVu0AddVector( &fvtemp, &fvtemp, &diff ) ;
		particle++;
	}

	return (0);
}

/*
pos から before_pos までの間に スポットブラーを 置く

*/
/* ---------------------------------------------------------------- */
void *NewSpotBlurLine( FVECTOR *pos, FVECTOR *before_pos )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) + sizeof( PARTICLE_WORK ) * N_PRIMS ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if ( GetResources( work, pos, before_pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

