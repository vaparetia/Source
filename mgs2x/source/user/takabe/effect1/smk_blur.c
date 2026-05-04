//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	smk_blur.c
	煙型ブラーエフェクト

	2000/04/12 K.Takabe
	$Id: smk_blur.c,v 1.7 2002/12/18 12:28:18 takaki Exp $

*/
/*

	void *NewSmokeBlurEffect( FMATRIX *world, int start_speed, int end_speed,
			int start_size, int end_size, int spot_size, int spot_angle,
			int n_prims, int interval, int color, int flag )
	FMATRIX	*world ;		出現位置マトリクス
	int		start_speed ;	ブラーパーティクル初期スピード
	int		end_speed ;		ブラーパーティクル最終スピード
	int		start_size ;	ブラーパーティクル初期サイズ
	int		end_size ;		ブラーパーティクル最終サイズ
	int		spot_size ;		ブラーパーティクル出現位置半径
	int		spot_angle ;	最大放射角度（ＰＳ角度0~4095）
	int		n_prims ;		最大パーティクル数
	int		interval ;		噴射間隔（フレーム単位）
	int		color ;			色指定0x00BBGGRR（推奨 0x00818181）
	int		flag ;			各種フラグ

	フラグの各種機能
	0x0001			デバッグ表示ＯＮ
	0x0002			初期指定位置で強制的に固定する

	指定マトリクスに追従してＺ軸方向に対してブラーパーティクルを連続的に生成する
	１つのパーティクルの寿命はn_preim*invervalフレームとなる

	例）
	NewSmokeBlurEffect( world, 150, 80, 200, 400, 200, 128, 64, 1, 0x00818181, 0 );

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if 0 //BP_PS2 def PSX2
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

#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#ifdef PSX2
#define RND(_a)		( ( (BP_PS2_rand()>>16) * (_a) ) >> 15  )
#else
#define RND(_a)		( ( rand() * (_a) ) >> 15  )
#endif

/* 時間基準設定 */
#define CLOCK_COUNT	(BP_BASE_TICK())

/* パーティクル関連定数 */
#define PARTICLE_VERTS	(32)	/* 仮想パーティクル数 */
#define PARTICLE_SIDES	(9)		/* 仮想パーティクル画数＋１ */

/* プリミティブ設定パラメータ */
#define N_PRIMS	(PARTICLE_VERTS)	/* プリミティブ数 */
#define N_VERTS	(8*2+1)				/* １プリミティブの頂点数 */
#define ALL_VERTS	( N_PRIMS * N_VERTS )	/* 総頂点数 */

/* 同次座標＞テクスチャアドレス変換定数 */
#if 0 //BP_XBOX ndef KP_WINDOWS
#define ADDRESS_SCALE_U	(0.5f*0.98)
#define ADDRESS_OFFSET_U	(0.5f)
//#define ADDRESS_SCALE_V	((float)DRAW_HEIGHT/2.0f/512.0f*0.98)
//#define ADDRESS_OFFSET_V	((float)DRAW_HEIGHT/2.0f/512.0f)
#define ADDRESS_SCALE_V	((float)DRAW_HEIGHT/2.0f/DRAW_WIDTH*0.98)
#define ADDRESS_OFFSET_V	((float)DRAW_HEIGHT/2.0f/DRAW_WIDTH)

#else
#define ADDRESS_SCALE_U		(0.5f*0.98)
#define ADDRESS_OFFSET_U	(0.5f)
#define ADDRESS_SCALE_V		(0.5f*0.98)
#define ADDRESS_OFFSET_V	(0.5f)

#endif

/* アニメーション設定 */
#define INTERVAL_COUNT	(3)		/* プリミティブ噴射インターバル */

/* シグナル関連 */
#define SIGNAL_SMOKEBLUREFFECT			(0xe58d1600)	/* GV_StrCode("SmokeBlurEffect")<<8 */
#define SIGNAL_SMOKEBLUREFFECT_VISIBLE	(SIGNAL_SMOKEBLUREFFECT|0x01)
/* ローカル用再定義 */
#define SIGNAL_THIS_EFFECT	(SIGNAL_SMOKEBLUREFFECT)
#define SIGNAL_VISIBLE		(SIGNAL_THIS_EFFECT|0x01)


/* ---------------------------------------------------------------- */
typedef struct smk_blur_PARTICLE_WORK {
	FVECTOR		pos ;		/* 座標（ｗは生命力１～０までを表す） */
	FVECTOR		add ;		/* 移動量 */
	FVECTOR		size ;		/* サイズ */
} PARTICLE_WORK ;

/* ---------------------------------------------------------------- */
typedef	struct smk_blur_WORK	{
	GV_ACT_EX	actor ;
	int			name ;
	int			map ;
	DG_PRIM2		*prim[2] ;
	FMATRIX			*world ;
	DG_TEX_TRANS	frame_tex[2] ;
	FMATRIX			mat ;
	int				active_count ;
	int				tick_count ;
	int				buffer_switch ;
	int				disable_flag ;
	/* 設定関連 */
	float			start_speed ;	/* パーティクル初速度 */
	float			end_speed ;		/* パーティクル最終速度 */
	float			start_size ;	/* パーティクル初期半径 */
	float			end_size ;		/* パーティクル最終半径 */
	float			spot_size ;		/* 出現位置半径 */
	int				spot_angle ;	/* 放射角度（中心からの相対角度） */
	int				n_prims ;		/* 出現パーティクル数 */
	int				interval ;		/* パーティクル噴射間隔 */
	int				color ;
	int				flag ;
	PARTICLE_WORK	particles[ 0 ];
} Work ;

/* ---------------------------------------------------------------- */
typedef struct smk_blur_ScrpadWork {
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
	FVECTOR		vec, *dst_pos ;
	FMATRIX		mat ;
	PARTICLE_WORK	*particle, *scrpad_particle ;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int			i, j ;
#ifndef BP_PSX2_ASM
	float		div ;
#endif
	float		dec_param ;

	if ( GV_PauseLevel == 0 ){
		float	vec_scale, tmp ;
		/*
			通常処理
		*/

		/* カウントチェック */
		work->tick_count += 1 ;
		if ( work->tick_count >= work->interval ){
			work->tick_count -= work->interval ;
			particle = &work->particles[ work->active_count ] ;
			/* パーティクル噴射初速度を求める（ローカル座標系） */
			vec.vx = 0.0f ;
			vec.vy = 0.0f ;
			vec.vz = work->start_speed ;
			vec.vw = 1.0f ;
			_sceVu0UnitMatrix( &mat );
			_sceVu0RotMatrixX( &mat, &mat, (float)RND(work->spot_angle) * 3.14159265f / 2048.0f );
			_sceVu0RotMatrixZ( &mat, &mat, (float)( RND(4096) - 2048 ) * 3.14159265f / 2048.0f );
			_sceVu0ApplyMatrix( &vec, &mat, &vec );
			/* 初速度をローカル座標系から絶対座標系に変換 */
			DG_SetPos( work->world );
			DG_RotVector( &vec, &particle->add, 1 );
			/* パーティクル初期位置を求める（初速度ベクトルを元にＺ軸に対して垂直な円状に配置） */
			vec.vx = 0.0f ;
			vec.vy = 0.0f ;
			vec.vz = 1.0 ;
			_sceVu0ApplyMatrix( &vec, &mat, &vec );
			tmp = 1.0f / sinf( work->spot_angle * 3.14159265f / 2048.0f );
			vec.vx = vec.vx * tmp * work->spot_size ;
			vec.vy = vec.vy * tmp * work->spot_size ;
			vec.vz = 0.0f ;
			DG_PutVector( &vec, &particle->pos, 1 );
			/* パーティクル半径を初期化 */
			particle->size.vx = work->start_size ;
			particle->size.vy = work->start_size ;
			particle->size.vz = work->start_size ;
			/* 更新プリミティブ番号変更 */
			work->active_count++ ;
			if ( work->active_count >= work->n_prims ) work->active_count = 0 ;
		}

		/* パーティクルアニメーション処理 */
		vec_scale = work->end_speed / work->start_speed ;
		particle = work->particles ;
		scrpad_particle = scrpad->particles ;
		dec_param = 1.0f / ( work->n_prims * work->interval ) ;
		for ( i = work->n_prims ; i > 0 ; particle++, scrpad_particle++, i-- ){
			float	scale ;

			scrpad_particle->pos = particle->pos ;
			scrpad_particle->add = particle->add ;
			scrpad_particle->size = particle->size ;

			scale =( 1.0f - vec_scale ) * scrpad_particle->pos.vw + vec_scale ;
			scrpad_particle->pos.vx += scrpad_particle->add.vx * scale ;
			scrpad_particle->pos.vy += scrpad_particle->add.vy * scale ;
			scrpad_particle->pos.vz += scrpad_particle->add.vz * scale ;

			scale =( work->start_size - work->end_size ) * scrpad_particle->pos.vw + work->end_size ;
			scrpad_particle->size.vx = scale ;
			scrpad_particle->size.vy = scale ;
			scrpad_particle->size.vz = scale ;

			//scrpad_particle->pos.vw -= 1.0f / ( work->n_prims * work->interval ) ;
			scrpad_particle->pos.vw -= dec_param ;
			/* メモリへ書き戻す */
			particle->pos = scrpad_particle->pos ;
			particle->size = scrpad_particle->size ;
		}

	} else {
		/*
			ポーズ時の処理
		*/

		/* パーティクル情報をスクラッチパッドへ */
		particle = work->particles ;
		scrpad_particle = scrpad->particles ;
		for ( i = work->n_prims ; i > 0 ; particle++, scrpad_particle++, i-- ){
			*scrpad_particle = *particle ;
		}

	}

	//work->buffer_switch = 1 - work->buffer_switch ;
	work->buffer_switch = DG_Clock ;
	work->prim[ work->buffer_switch ]->flag |= DG_PRIM2_INVISIBLE ;
	if ( ( GM_BlurDisableFlag == 0 ) && ( work->disable_flag == 0 ) ){
		work->prim[ work->buffer_switch ]->flag &= ~DG_PRIM2_INVISIBLE0 ;

#if 0 //BP_PS2
#ifndef PSX2
#ifndef KP_WINDOWS
		DG_ConfigPrim2Tex( work->prim[ work->buffer_switch ], &DG_SystemTexture[ 1 - DG_CurrentBackBuffer ] );
#else	// KP_WINDOWS
		DG_ConfigPrim2Tex( work->prim[ work->buffer_switch ], &DG_SystemTexture[ DG_CurrentBackBuffer ] );	// GeForce Driver 41.09が簡単には描画中バッファを直接指定できないらしい
#endif
#endif	// PSX2

#else
      DG_ConfigPrim2Tex(work->prim[work->buffer_switch], &BP_PreviousFrameTexture);
#endif
	}
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
	for ( j = 0 ; j < work->n_prims ; particle++, j++ ){
		//float	div ;

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
#ifdef KP_WINDOWS
			// Windows版は諸事情により範囲限定
			if( uvrgb->u < 0 ){  uvrgb->u = 0 ; }
			else if( uvrgb->u > 4096 ){  uvrgb->u = 4096 ; }
			if( uvrgb->v < 0 ){  uvrgb->v = 0 ; }
			else if( uvrgb->v > 4096 ){  uvrgb->v = 4096 ; }
#endif
			if ( i & 1 ){
				/* アルファ設定 */
				uvrgb->a = particle->pos.vw * 64 ;
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
					uvrgb->a = particle->pos.vw * 64 ;
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
#ifdef KP_WINDOWS
static void DmyAct( Work *work ){}
static void DmyDie( Work *work ){}
#endif

/* ---------------------------------------------------------------- */
static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *work = workp;
	switch( signal ){
	  case SIGNAL_VISIBLE:
		if ( value == 0 ){
			//work->prim[0]->flag |= DG_PRIM2_INVISIBLE0 ;
			//work->prim[1]->flag |= DG_PRIM2_INVISIBLE0 ;
			work->disable_flag = 1 ;
		} else {
			//work->prim[0]->flag &= ~DG_PRIM2_INVISIBLE0 ;
			//work->prim[1]->flag &= ~DG_PRIM2_INVISIBLE0 ;
			work->disable_flag = 0 ;
		}
		break ;
	  default:
		return GV_DefaultSignalFunc( workp, signal, value );
	}
	return 0;
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *world )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	PARTICLE_WORK	*particle ;
	int			i, j, k, prim_flag ;

	work->map = GM_CurrentMap ;

	work->world = world ;

#if 0 //BP_PS2 def PSX2
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

	if ( work->flag & 0x0001 ){
		prim_flag &= ~DG_PRIM2_TEX ;
	}
	for ( k = 0 ; k < 2 ; k++ ){
		/* スプライトプリミティブ初期化 */
#if 0 //BP_PS2 def PSX2
		prim = work->prim[k] = GM_MakePrim2( prim_flag, work->n_prims, N_VERTS );
		prim->tex_trans = work->frame_tex[k] ;
#else
		prim = work->prim[k] = GM_MakePrim2( prim_flag|DG_PRIM2_FRAMETEX, work->n_prims, N_VERTS );
		//BP_PS2 DG_ConfigPrim2Tex( prim, &DG_SystemTexture[ k ] );
      DG_ConfigPrim2Tex(prim, &BP_PreviousFrameTexture);
#endif

		/* 座標初期化 */
		pos = prim->pos[ 0 ] ;/* シングルバッファなので片方だけ初期化すればよい */
		for ( i = work->n_prims * N_VERTS ; i > 0 ; i-- ){
			*pos = *(FVECTOR*)world->m[3] ;
			pos++ ;
		}

		/* プリミティブ状態初期化 */
		uvrgb = (DG_PRIM2_UVRGB*)prim->uvrgb[ 0 ] ;/* シングルバッファなので片方だけ初期化すればよい */
		for ( j = 0 ; j < work->n_prims ; j++ ){
			prim->packet[0][j].flag |= DG_PRIM2_INVISIBLE ;	/* デフォルトは非表示に */
			for ( i = 0 ; i < N_VERTS ; uvrgb++, i++ ){
				uvrgb->u = FTOI12( 0.0f );
				uvrgb->v = FTOI12( 0.0f );
				uvrgb->q = FTOI12( 1.0f );
				uvrgb->f = 0x0fff ;
				if ( i & 1 || i < 2 ) uvrgb->f = 0x8fff ;
				uvrgb->r = work->color & 0xff ;
				uvrgb->g = ( work->color >> 8 ) & 0xff ;
				uvrgb->b = ( work->color >> 16 ) & 0xff ;
				uvrgb->a = 0 ;
			}
		}

	}

	/* 仮想パーティクル座標初期化 */
	particle = work->particles ;
	for ( i = work->n_prims ; i > 0 ; i-- ){
		particle->pos.vw = 0.0f ;
		particle->add = DG_ZeroVector ;
	}

	work->active_count = 0 ;
	work->tick_count = 0 ;

	/* 固定チェック */
	if ( work->flag & 0x0002 ){
		work->mat = *world ;
		work->world = &work->mat ;
	}

#if 0
	work->start_speed = 100.0f ;
	work->end_speed = 40.0f ;
	work->start_size = 300.0f ;
	work->end_size = 600.0f ;
	work->spot_size = 200.0f ;
	work->spot_angle = 128.0f ;
#endif

	return (0);
}

/* ---------------------------------------------------------------- */
/* プログラム呼び出しインターフェイス */
void *NewSmokeBlurEffect( FMATRIX *world, int start_speed, int end_speed, int start_size, int end_size, int spot_size, int spot_angle, int n_prims, int interval, int color, int flag )
{
	Work		*work ;

	ASSERT( ( sizeof(ScrpadWork) + sizeof(PARTICLE_WORK) * n_prims ) < 16*1024 ) ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) + sizeof( PARTICLE_WORK ) * n_prims ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		GV_SetActorSignalFunc( work, ReceiveSignal );
		work->start_speed = start_speed ;
		work->end_speed = end_speed ;
		work->start_size = start_size ;
		work->end_size = end_size ;
		work->spot_size = spot_size ;
		work->spot_angle = spot_angle ;
		work->n_prims = n_prims ;
		work->interval = interval ;
		work->color = color & 0x00ffffff ;
		work->flag = flag ;
		if ( GetResources( work, world ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if ( DG_CheckUseSmkBlur() ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor );
			GV_SetActorSignalFunc( work, ReceiveSignal );
			work->start_speed = start_speed ;
			work->end_speed = end_speed ;
			work->start_size = start_size ;
			work->end_size = end_size ;
			work->spot_size = spot_size ;
			work->spot_angle = spot_angle ;
			work->n_prims = n_prims ;
			work->interval = interval ;
			work->color = color & 0x00ffffff ;
			work->flag = flag ;
			if ( GetResources( work, world ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		} else {
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor );
		}
#endif
	}
	return (void *)work ;
}

#if 0
// デバッグ用コード
	//NewSmokeBlurEffect( &GM_PlayerBody->objs->world, 150, 80, 200, 400, 200, 128, 64, 1, 0x00818181, 0 );
	//NewSmokeBlurEffect( &GM_PlayerBody->objs->world, 230, 180, 150, 200, 200, 128, 64, 1, 0x00818181, 0 );
	//NewSmokeBlurEffect( &GM_PlayerBody->objs->world, 180, 80, 200, 200, 200, 128, 64, 1, 0x00818181, 0 );
	//NewSmokeBlurEffect( &GM_PlayerBody->objs->world, 135, 135, 80, 200, 200, 128, 64, 1, 0x00818181, 0 );
	//NewSmokeBlurEffect( &GM_PlayerBody->objs->world, 211, 120, 300, 400, 200, 128, 64, 1, 0x00818181, 0 );
#endif
