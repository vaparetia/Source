//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	cb_wet.c
	濡れ段ボール（べーやんの oozeblod.c：にじみ血エフェクトがベース）

	2000/10/16 K.Takabe
	$Id: cb_wet.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $

*/
/*
プログラム用インターフェイス
	void *NewOozeBlood( DG_OBJS *objs, int model_id )
	DG_OBJS	*objs ;		適応オブジェクト
	int		model_id ;	モデルファイルＩＤ（要ＣＶ２データ）

	指定したオブジェクトに滲み血を発生させることができるように
	キャラクタを起動。滲み血エフェクトをかけるオブジェクトは
	マルチテクスチャシングルウェイトモデルであり、同名の
	ＣＶ２データファイルが必要になるので注意


	void TAKABE_OozeBloodAdd( void *_work, int joint, FVECTOR *pos, float range )
	void	_work ;		滲み血エフェクトのワークアドレス
	int		joint ;		基準関節
	FVECTOR	*pos ;		滲み中心の絶対座標
	float	range ;		影響範囲

	起動している滲み血キャラに滲ませ処理を行なう

*/
#endif
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"


#define MIN_COL (0.1f)
#define INTENSE (0.5f)

/* ---------------------------------------------------------------- */
typedef struct {
	u_long64		*rgbs ;
	float		*blood ;
} ColorBuffer ;
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name ;
	int			model_id ;
	DG_OBJS		*objs ;
	CV2_DEF		*cv2_def ;
	int			old_objs_flag ;
	ColorBuffer	color_buffer[21] ;
	DG_TEX_TRANS	tex_trans ;
} Work ;


/* ---------------------------------------------------------------- */
	/*
		FVECTORをCVECTORに変換し、スクラッチパッドへ転送する
	*/
static void RegistCommonColor( FVECTOR *verts, int n_verts )
{
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
#ifdef BP_PSX2_ASM	
	asm volatile("
		move			$8,%0
		move			$9,%1
		move			$10,%2
0:
		lqc2			vf4,0($8)		# 読み込み
		lqc2			vf5,16($8)		# 読み込み
		lqc2			vf6,32($8)		# 読み込み
		lqc2			vf7,48($8)		# 読み込み
		pref			0,64($8)		# プリフェッチ
		vftoi12.xyzw	vf4,vf4			# 整数化
		addi			$9,$9,-1		# カウンタデクリメント
		vftoi12.xyzw	vf5,vf5			# 整数化
		addi			$8,$8,64		# アドレスインクリメント
		vftoi12.xyzw	vf6,vf6			# 整数化
		addi			$10,$10,32		# アドレスインクリメント
		vftoi12.xyzw	vf7,vf7			# 整数化
		qmfc2			$11,vf4			# ＶＵからＣＯＲＥへ
		qmfc2			$12,vf5			# ＶＵからＣＯＲＥへ
		qmfc2			$13,vf6			# ＶＵからＣＯＲＥへ
		qmfc2			$14,vf7			# ＶＵからＣＯＲＥへ
		ppach			$11,$0,$11		# IVECTORからSVECTORへ変換
		ppach			$12,$0,$12		# IVECTORからSVECTORへ変換
		ppach			$13,$0,$13		# IVECTORからSVECTORへ変換
		ppach			$14,$0,$14		# IVECTORからSVECTORへ変換
		sd				$11,0-32($10)	# メモリへ
		sd				$12,8-32($10)	# メモリへ
		sd				$13,16-32($10)	# メモリへ
		sd				$14,24-32($10)	# メモリへ
		bgtz			$9,0b
		nop
	"::"r"(verts),"r"((n_verts+3)/4),"r"(svec):"$8","$9","$10","$11","$12","$13","$14" );
#else
	int i;
	for( i = 0 ; i < n_verts ; i++ ){
		svec->vx = (int)(verts->vx * 4096.f);
		svec->vy = (int)(verts->vy * 4096.f);
		svec->vz = (int)(verts->vz * 4096.f);
		svec->pad = (int)(verts->vw * 4096.f);

		verts++;
		svec++;
	}
#endif	
}
	/*
		スクラッチパッド上のカラーデータをモデルのカラーデータに振り分ける
	*/
static void RefineStripColor( DG_OBJ *obj, u_long64 *rgbs, short *index )
{
	DG_MDLPACK	*mdl_pack ;
	DG_MDL		*mdl ;
	u_long64		*buffer ;
	int			i, j, n ;

	mdl = obj->model ;
	mdl_pack = mdl->packs ;
	buffer = (u_long64*)SCRPAD_ADDR ;

#ifndef NEW_KMX_FORMAT
	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;
		for ( j = ( n + 1 ) / 2 ; j > 0 ; j-- ){
			rgbs[0] = buffer[ index[0] ] ;
			rgbs[1] = buffer[ index[1] ] ;
			//printf("%08x%08x %08x%08x %d %d\n",1[(int*)rgbs],0[(int*)rgbs],3[(int*)rgbs],2[(int*)rgbs], index[0], index[1] );
#ifdef BP_PSX2_ASM			
			if ( ( (int)index & 63 ) == 0 ) asm volatile( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)rgbs & 63 ) == 0 ) asm volatile( "pref 0,64(%0)"::"r"(rgbs));
#endif			
			rgbs += 2 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
#else
	for ( i = mdl->n_verts ; i > 0 ; i-=2 ){
		rgbs[0] = buffer[ 0 ] ;
		rgbs[1] = buffer[ 1 ] ;
		rgbs += 2 ;
		buffer += 2 ;
	}
#endif
}
/* ---------------------------------------------------------------- */
static void GetLocalOffset( DG_OBJS *objs, int joint, FVECTOR *res )
{
	DG_DEF		*def ;
	DG_MDL		*mdl ;

	def = objs->def ;

	res->vx = 0.0f, res->vy = 0.0f, res->vz = 0.0f ;

	mdl = &def->models[ joint ];
	while ( mdl->parent != -1 ){
		res->vx += mdl->tx ;
		res->vy += mdl->ty ;
		res->vz += mdl->tz ;
		mdl = &def->models[ mdl->parent ] ;
	}
}

	/*
		滲み血処理用インターフェイス
	*/
void CB_WetEffectAdd( void *_work, int joint, FVECTOR *pos, float range )
{
	Work		*work = _work ;
	CV2_MDL		*cv2_mdl ;
	FMATRIX		mat ;
	FVECTOR		vec, tmp, vec_abs ;
	FVECTOR		*verts, *scrpad_verts ;
	float		*blood, rangerange, drange, ftemp ;
	int			i, j ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	/* 定数計算 */
	rangerange = range * range ;
	drange = 1.0f / range ;

	/* モデル原点からの相対当たり位置を計算 */
	_sceVu0InversMatrix( &mat, &work->objs->objs[ joint ].world );
	DG_SetPos( &mat );
	DG_PutVector( pos, &vec, 1 );
	GetLocalOffset( work->objs, joint, &tmp );
	_sceVu0AddVector( &vec_abs, &vec, &tmp );

	for ( j = 0 ; j < work->objs->n_models ; j++ ){
		/* 関節ローカルにおける当たり座標を求める */
		GetLocalOffset( work->objs, j, &tmp );
		_sceVu0SubVector( &vec, &vec_abs, &tmp );

		/* 座標からの距離に応じて色を加算する */
		cv2_mdl = &work->cv2_def->models[ j ] ;
		blood = work->color_buffer[j].blood ;
		scrpad_verts = SCRPAD_ADDR ;
		verts = cv2_mdl->verts ;
		for ( i = cv2_mdl->n_verts ; i > 0 ; blood++, verts++, scrpad_verts++, i-- ){
			float	len ;
			tmp.vx = verts->vx - vec.vx ;
			tmp.vy = verts->vy - vec.vy ;
			tmp.vz = verts->vz - vec.vz ;
			len = tmp.vx * tmp.vx + tmp.vy * tmp.vy + tmp.vz * tmp.vz ;
			if ( len < ( rangerange ) ){
				len = bp_sqrtf( len );  //BP_MATH - emulate PS2 sqrtf
				len = ( range - len ) * drange ;
				*blood = DG_MIN( 1.0f, *blood + len );
			}
			ftemp = 1.0f - *blood * INTENSE;
//			printf("%f\n",*blood );
			scrpad_verts->vx = DG_MAX( MIN_COL, ftemp ) ;
			scrpad_verts->vy = DG_MAX( MIN_COL, ftemp ) ;
			scrpad_verts->vz = DG_MAX( MIN_COL, ftemp ) ;
			scrpad_verts->vw = 1.0f ;
			//printf("%f %f %f %f\n", scrpad_verts->vx, scrpad_verts->vy, scrpad_verts->vz, scrpad_verts->vw );
		}
		//printf("verts:%d\n", cv2_mdl->n_verts );
		/* 型変換をしてスクラッチパッドへ */
		RegistCommonColor( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
		/* インデックスを参照してデータを分配 */
		RefineStripColor( &work->objs->objs[j], work->color_buffer[j].rgbs, cv2_mdl->verts_index );
#ifdef KP_WINDOWS
		/* Color Vertex Buffer適用 */
		DG_AssignDGObjCVertexBuffer(&work->objs->objs[j]) ;
#endif
	}
}

	/*
		対象オブジェクトを変更
	*/
void CB_WetEffectChangeObjs( void *_work, DG_OBJS *objs )
{
	DG_OBJS	*src_objs, *dst_objs ;
	DG_OBJ	*src_obj, *dst_obj ;
	DG_OBJ_PACKET	*packet ;
	Work	*work = _work ;
	int		i ;

	src_objs = work->objs ;
	dst_objs = objs ;
	src_obj = src_objs->objs ;
	dst_obj = dst_objs->objs ;
	for ( i = objs->n_models ; i > 0 ; src_obj++, dst_obj++, i-- ){
		dst_obj->rgbs = src_obj->rgbs ;
		src_obj->rgbs = NULL ;
		packet = dst_obj->packets ;
		dst_obj->packets = src_obj->packets ;
		src_obj->packets = packet ;
#ifdef KP_WINDOWS
		{
			void	*tmp ;

			tmp = dst_obj->d3d_cvbuff ;
			dst_obj->d3d_cvbuff = src_obj->d3d_cvbuff ;
			src_obj->d3d_cvbuff = tmp ;
		}

		dst_obj->cvbuff_dirty = src_obj->cvbuff_dirty ;
		src_obj->cvbuff_dirty = FALSE ;
#endif
	}
	work->objs = objs ;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i, j ;
	DG_OBJ_PACKET	*packet ;

	if ( work->cv2_def == NULL ) return ;
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	if ( work->color_buffer[0].blood != NULL ) GV_DelayedFree( work->color_buffer[0].blood );
	if ( work->color_buffer[0].rgbs != NULL ) GV_DelayedFree( work->color_buffer[0].rgbs );
	/* モデルの頂点カラーに関するワークを元に戻す */
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		work->objs->objs[ i ].rgbs = NULL ;
		/* 下はもしかしたら必要ないかも */
		packet = work->objs->objs[ i ].packets ;
		for ( j = work->objs->objs[ i ].n_packs ; j > 0 ; packet++, j-- ){
			packet->rgbs_offset = 0 ;
		}
	}
	/* ディレイド開放フラグを元に戻す */
	work->objs->flag &= ~DG_FLAG_DELAYED ;
	work->objs->flag |= work->old_objs_flag ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *objs, int model_id )
{
	CV2_DEF		*cv2_def ;
	DG_DEF		*def ;
	int			i, j, k ;

	work->model_id = model_id ;
	work->objs = objs ;

	if ( !( objs->flag & DG_FLAG_MULTITEX ) ){
		printf("oozeblod.c: error !! suport only multi texture model\n");
		return ( -1 );
	}

	/* モデルの共有頂点データ取得 */
	work->cv2_def = cv2_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );

	ASSERT( cv2_def != NULL );
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return (0);

	{/* カラーウェイト用ワークの確保 */
		int		size ;
		float	*ptr ;
		/* モデルの頂点総数を計算 */
		size = 0 ;
		for ( i = 0 ; i < work->cv2_def->n_models ; i++ ){
			size += work->cv2_def->models[ i ].n_verts ;
		}
		/* メモリを確保し、ワークに割り振る */
		ptr = GV_Malloc( sizeof( float ) * size );
		if ( ptr == NULL ) return ( -1 );
		GV_ZeroMemory( ptr, sizeof( float ) * size );
		for ( i = 0 ; i < work->cv2_def->n_models ; i++ ){
			work->color_buffer[ i ].blood = ptr ;
			ptr += work->cv2_def->models[ i ].n_verts ;
		}
	}

#ifndef NEW_KMX_FORMAT
	{/* カラーバッファワークの確保＆割り当て処理 */
		DG_OBJ			*obj ;
		DG_MDL			*mdl ;
		DG_OBJ_PACKET	*packet ;
		DG_MDLPACK		*mdl_pack ;
		u_long64	*ptr ;
		int		size ;

		def = objs->def ;
		/* モデルの頂点総数を計算 */
		size = 0 ;
		for ( i = 0 ; i < def->n_x_models ; i++ ){
			mdl_pack = def->models[ i ].packs ;
			for ( j = def->models[ i ].n_packs ; j > 0 ; mdl_pack++, j-- ){
				size += ( mdl_pack->n_verts + 1 ) & 0xfffe ;
			}
		}
		/* メモリを確保し、ワークに割り振る */
		ptr = GV_Malloc( sizeof(u_long64) * size );
		if ( ptr == NULL ) return ( -1 );
		//GV_ZeroMemory( ptr, sizeof(u_long64) * size );

		obj = objs->objs ;
		for ( i = 0 ; i < objs->n_models ; obj++, i++ ){
			mdl = obj->model ;
			packet = obj->packets ;
			mdl_pack = mdl->packs ;
			work->color_buffer[ i ].rgbs = ptr ;
			obj->rgbs = (SVECTOR *)ptr ;
			for ( j = mdl->n_packs ; j > 0 ; mdl_pack++, packet++, j-- ){
				packet->rgbs_offset = ( mdl_pack->n_verts + 1 ) / 2 ;
				/* 初期化＆ポインタインクリメント */
				for ( k = packet->rgbs_offset * 2 ; k > 0 ; k -= 2 ){
					ptr[0] = 0x1000100010001000 ;
					ptr[1] = 0x1000100010001000 ;
					ptr += 2 ;
				}
			}
#ifdef KP_WINDOWS
			/* Color Vertex Buffer適用 */
			DG_AssignDGObjCVertexBuffer(obj) ;
#endif
		}
	}
#else
	{/* カラーバッファワークの確保＆割り当て処理 */
		DG_OBJ			*obj ;
		DG_MDL			*mdl ;
		DG_OBJ_PACKET	*packet ;
		DG_MDLPACK		*mdl_pack ;
		u_long64	*ptr ;
		int		size ;

		def = objs->def ;
		/* モデルの頂点総数を計算 */
		size = 0 ;
		for ( i = 0 ; i < def->n_x_models ; i++ ){
			mdl_pack = def->models[ i ].packs ;
			size += ( def->models[ i ].n_verts + 1 ) & 0xfffe ;
		}
		/* メモリを確保し、ワークに割り振る */
		ptr = GV_Malloc( sizeof(u_long64) * size );
		if ( ptr == NULL ) return ( -1 );
		//GV_ZeroMemory( ptr, sizeof(u_long64) * size );

		obj = objs->objs ;
		for ( i = 0 ; i < objs->n_models ; obj++, i++ ){
			mdl = obj->model ;
			packet = obj->packets ;
			mdl_pack = mdl->packs ;
			work->color_buffer[ i ].rgbs = ptr ;
			obj->rgbs = (SVECTOR *)ptr ;
			for ( j = mdl->n_verts ; j > 0 ; j-=2 ){
				ptr[0] = 0x1000100010001000ULL ;
				ptr[1] = 0x1000100010001000ULL ;
				ptr += 2 ;
			}
#ifdef KP_WINDOWS
			/* Color Vertex Buffer適用 */
			DG_AssignDGObjCVertexBuffer(obj) ;
#endif
		}
	}
#endif
	/* 念のため参照オブジェクトをディレイド開放に設定する */
	work->old_objs_flag = objs->flag & DG_FLAG_DELAYED ;	/* フラグを記憶 */
	objs->flag |= DG_FLAG_DELAYED ;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewCB_WetEffect( DG_OBJS *objs, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, objs, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

