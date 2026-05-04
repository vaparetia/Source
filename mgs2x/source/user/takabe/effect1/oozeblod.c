//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	oozeblod.c
	にじみ血エフェクト

	1999/10/15 K.Takabe
	$Id: oozeblod.c,v 1.4 2002/11/23 12:28:38 Yoshizawa1 Exp $

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


デモ用インターフェイス
	void *NewOozeBloodDemo( int name, DG_OBJS *objs, int model_id );
	int		name ;		キャラクタ名（メッセージ受け取り用）
	DG_OBJS	*objs ;		適応オブジェクト
	int		model_id ;	モデルファイルＩＤ（要ＣＶ２データ）

	指定したオブジェクトに滲み血を発生させることができるように
	キャラクタを起動。滲み血エフェクトをかけるオブジェクトは
	マルチテクスチャシングルウェイトモデルであり、同名の
	ＣＶ２データファイルが必要になるので注意


	滲み血発生メッセージ
	message[0] = 0
	message[1] = 基準ジョイント番号
	message[2] = 滲み発生座標Ｘ
	message[3] = 滲み発生座標Ｙ
	message[4] = 滲み発生座標Ｚ
	message[5] = 滲み有効半径

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
#include	"def_dma.h"

#include	"../other/vec_util.h"

#include "bp_matrix.h"

#if 1
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
	FVECTOR		color_vector ;
	FVECTOR		one_vector ;
	FVECTOR		zero_vector ;

	ColorBuffer	color_buffer[0] ;
} Work ;


/* ---------------------------------------------------------------- */
	/*
		FVECTORをSVECTORに変換し、スクラッチパッドへ転送する
	*/
static void RegistCommonColor( FVECTOR *verts, int n_verts )
{
#ifdef BP_PSX2_ASM
	SVECTOR	*svec = (SVECTOR*)SCRPAD_ADDR ;
	asm("
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
	int			i ;
	SVECTOR		*svec ;

	svec = (SVECTOR*)SCRPAD_ADDR ;
	for ( i = 0 ; i < n_verts ; i++, svec++, verts++ ){
		svec->vx = DG_FTOI( verts->vx * 4096 ) ;
		svec->vy = DG_FTOI( verts->vy * 4096 ) ;
		svec->vz = DG_FTOI( verts->vz * 4096 ) ;
		svec->pad = DG_FTOI( verts->vw * 4096 ) ;
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
			if ( ( (int)index & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(index));
			if ( ( (int)rgbs & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(rgbs));
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
		滲み血の色を設定する
	*/
void TAKABE_OozeBloodSetColor( void *_work, float r, float g, float b )
{
	Work		*work = _work ;
	if ( work == NULL ) return ;

	work->color_vector.vx = r ;
	work->color_vector.vy = g ;
	work->color_vector.vz = b ;
}

	/*
		滲み血処理用インターフェイス
	*/
void TAKABE_OozeBloodAdd( void *_work, int joint, FVECTOR *pos, float range )
{
	Work		*work = _work ;
	CV2_MDL		*cv2_mdl ;
	FMATRIX		mat ;
	FVECTOR		vec, tmp, vec_abs ;
	FVECTOR		*verts, *scrpad_verts ;
	float		*blood, rangerange, drange ;
	int			i, j ;

	/* ゲーム設定の出血モード有無をみて動作を抑制する */
	if ( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return ;


	if ( work == NULL ) return ;
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
#if 0
			//scrpad_verts->vx = DG_MAX( 0.0f, 1.0f - *blood * 0.40f ) ;
			//scrpad_verts->vy = DG_MAX( 0.0f, 1.0f - *blood * 2.00f ) ;
			//scrpad_verts->vz = DG_MAX( 0.0f, 1.0f - *blood * 2.00f ) ;
			//scrpad_verts->vw = 1.0f ;
			_sceVu0ScaleVector( &tmp, &work->color_vector, *blood );
			_sceVu0SubVector( scrpad_verts, &work->one_vector, &tmp );
			scrpad_verts->vx = DG_MAX( 0.0f, scrpad_verts->vx );
			scrpad_verts->vy = DG_MAX( 0.0f, scrpad_verts->vy );
			scrpad_verts->vz = DG_MAX( 0.0f, scrpad_verts->vz );
#else
			GTE_ScaleVector( &tmp, &work->color_vector, *blood );
			GTE_SubVector( scrpad_verts, &work->one_vector, &tmp );
			GTE_MaxVector( scrpad_verts, scrpad_verts, &work->zero_vector );
#endif
			//printf("%f %f %f %f\n", scrpad_verts->vx, scrpad_verts->vy, scrpad_verts->vz, scrpad_verts->vw );
		}
		//printf("verts:%d\n", cv2_mdl->n_verts );
		/* 型変換をしてスクラッチパッドへ */
		RegistCommonColor( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
		/* インデックスを参照してデータを分配 */
		RefineStripColor( &work->objs->objs[j], work->color_buffer[j].rgbs, cv2_mdl->verts_index );
	}

#ifdef KP_WINDOWS
	/* Color Vertex Buffer適用 */
	DG_AssignDGObjCVertexBuffer(&work->objs->objs[ joint ]) ;
#endif
}

	/*
		対象オブジェクトを変更
	*/
void TAKABE_OozeBloodChangeObjs( void *_work, DG_OBJS *objs )
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

	/* ディレイド開放フラグを元に戻す */
	work->objs->flag &= ~DG_FLAG_DELAYED ;
	work->objs->flag |= work->old_objs_flag ;

	work->objs = objs ;
	/* 念のため参照オブジェクトをディレイド開放に設定する */
	work->old_objs_flag = objs->flag & DG_FLAG_DELAYED ;	/* フラグを記憶 */
	objs->flag |= DG_FLAG_DELAYED ;
}

	/*
		滲み血用インターフェイス
	*/
void TAKABE_OozeBloodClear( void *_work, float scale )
{
	Work		*work = _work ;
	CV2_MDL		*cv2_mdl ;
	FVECTOR		*scrpad_verts ;
	float		*blood ;
	int			i, j ;

	/* ゲーム設定の出血モード有無をみて動作を抑制する */
	if ( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return ;


	if ( work == NULL ) return ;
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;

	for ( j = 0 ; j < work->objs->n_models ; j++ ){
		/* 関節ローカルにおける当たり座標を求める */

		/* 座標からの距離に応じて色を加算する */
		cv2_mdl = &work->cv2_def->models[ j ] ;
		blood = work->color_buffer[j].blood ;
		scrpad_verts = SCRPAD_ADDR ;
#ifndef BP_PSX2_ASM
		for ( i = cv2_mdl->n_verts ; i > 0 ; blood++, scrpad_verts++, i-- ){
			FVECTOR		tmp_vec ;
			*blood *= scale ;
			//scrpad_verts->vx = DG_MAX( 0.0f, 1.0f - *blood * 0.40f ) ;
			//scrpad_verts->vy = DG_MAX( 0.0f, 1.0f - *blood * 2.00f ) ;
			//scrpad_verts->vz = DG_MAX( 0.0f, 1.0f - *blood * 2.00f ) ;
			GTE_ScaleVector( &tmp_vec, &work->color_vector, *blood );
			GTE_SubVector( scrpad_verts, &work->one_vector, &tmp_vec );
			GTE_MaxVector( scrpad_verts, scrpad_verts, &work->zero_vector );
		}
#else
		asm volatile ("
			lqc2		vf8,0x00(%0)
			lqc2		vf9,0x00(%1)
			lqc2		vf10,0x00(%2)
			mfc1		$4,%3
			qmtc2		$4,vf11
		"::"r"(&work->color_vector),"r"(&work->one_vector),"r"(&work->zero_vector),"f"(scale):"$4" );
		for ( i = cv2_mdl->n_verts ; i > 0 ; blood+=4, scrpad_verts+=4, i-=4 ){
			asm volatile ("
				lqc2			vf1,0x00(%1)
				pref			0,64(%1)
				vmulx.xyzw		vf1,vf1,vf11
				sqc2			vf1,0x00(%1)
				vmulx.xyzw		vf4,vf8,vf1
				vmuly.xyzw		vf5,vf8,vf1
				vmulz.xyzw		vf6,vf8,vf1
				vmulw.xyzw		vf7,vf8,vf1
				vsub.xyzw		vf4,vf9,vf4
				vsub.xyzw		vf5,vf9,vf5
				vsub.xyzw		vf6,vf9,vf6
				vsub.xyzw		vf7,vf9,vf7
				vmax.xyzw		vf4,vf4,vf10
				vmax.xyzw		vf5,vf5,vf10
				vmax.xyzw		vf6,vf6,vf10
				vmax.xyzw		vf7,vf7,vf10
				sqc2			vf4,0x00(%0)
				sqc2			vf5,0x10(%0)
				sqc2			vf6,0x20(%0)
				sqc2			vf7,0x30(%0)
			"::"r"(scrpad_verts),"r"(blood) );
		}

#endif
		/* 型変換をしてスクラッチパッドへ */
		RegistCommonColor( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
		/* インデックスを参照してデータを分配 */
		RefineStripColor( &work->objs->objs[j], work->color_buffer[j].rgbs, cv2_mdl->verts_index );
	}
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		n_msg ;
	GV_MSG	*msg ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;

	/* メッセージチェック */
	if ( work->name != 0 ){
		if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) ){
			for ( ; n_msg > 0 ; n_msg--, msg++ ){
				switch ( msg->message[0] ){
				  case 0:/* 滲み値発動 */
					{
						FVECTOR	pos ;
						pos.vx = msg->message[2] ;
						pos.vy = msg->message[3] ;
						pos.vz = msg->message[4] ;
						pos.vw = 1.0f ;
						TAKABE_OozeBloodAdd( work, msg->message[1], &pos, msg->message[5] );
												   
					}
					break ;
				}
			}
		}
	}
#if 0
	if ( GV_PadData[0].press & PAD_AL ){
		FVECTOR	pos ;
		pos.vx = work->objs->objs[ 0 ].world.m[3][0] + BP_PS2_rand() % 500 - 250 ;
		pos.vy = work->objs->objs[ 0 ].world.m[3][1] + BP_PS2_rand() % 500 - 250 ;
		pos.vz = work->objs->objs[ 0 ].world.m[3][2] + 100.0f + BP_PS2_rand() % 500 - 250 ;
		TAKABE_OozeBloodAdd( work, 0, &pos, 50.0f );
	}
#endif
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

	work->color_vector.vx = 0.40f ;
	work->color_vector.vy = 2.00f ;
	work->color_vector.vz = 2.00f ;
	work->color_vector.vw = 0.00f ;
	work->one_vector.vx = 1.0f ;
	work->one_vector.vy = 1.0f ;
	work->one_vector.vz = 1.0f ;
	work->one_vector.vw = 1.0f ;
	work->zero_vector.vx = 0.0f ;
	work->zero_vector.vy = 0.0f ;
	work->zero_vector.vz = 0.0f ;
	work->zero_vector.vw = 0.0f ;

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
			size += ( work->cv2_def->models[ i ].n_verts + 3 ) & 0xfffc ;
		}
		/* メモリを確保し、ワークに割り振る */
		ptr = GV_Malloc( sizeof( float ) * size );
		if ( ptr == NULL ) return ( -1 );
		GV_ZeroMemory( ptr, sizeof( float ) * size );
		for ( i = 0 ; i < work->cv2_def->n_models ; i++ ){
			work->color_buffer[ i ].blood = ptr ;
			ptr += ( work->cv2_def->models[ i ].n_verts + 3 ) & 0xfffc ;
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
			obj->rgbs = (SVECTOR*)ptr ;
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
			obj->rgbs = (SVECTOR*)ptr ;
			for ( j = mdl->n_verts ; j > 0 ; j-=2 ){
				ptr[0] = 0x1000100010001000 ;
				ptr[1] = 0x1000100010001000 ;
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
void *NewOozeBlood( DG_OBJS *objs, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) + sizeof(ColorBuffer)*objs->def->n_x_models ) ;
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

void *NewOozeBloodDemo( int name, DG_OBJS *objs, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) + sizeof(ColorBuffer)*objs->def->n_x_models ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name ;
		if ( GetResources( work, objs, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


































#else
/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX		lights[2] ;
	int			model_id ;
	DG_TEX		*tex ;
	DG_OBJS		*org_objs ;
	DG_OBJS		*objs ;
	CV2_DEF		*cv2_def ;
	float		*blood[21] ;
	DG_VERTS_ANIME	v_anime[21] ;
	DG_TEX_TRANS	tex_trans ;
} Work ;


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
void TAKABE_OozeBloodAdd( void *_work, int joint, FVECTOR *pos, float range )
{
	Work		*work = _work ;
	CV2_MDL		*cv2_mdl ;
	FMATRIX		mat ;
	FVECTOR		vec, tmp, vec_abs ;
	FVECTOR		*verts, *scrpad_verts ;
	float		*blood, u_scale, u_offset, v, rangerange, drange ;
	int			i, j ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	/* 定数計算 */
	rangerange = range * range ;
	drange = 1.0f / range ;
	u_scale = work->tex->u_scale ;
	u_offset = work->tex->u_offset ;
	v = 0.5f * work->tex->v_scale + work->tex->v_offset ;

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
		blood = work->blood[ j ];
		scrpad_verts = SCRPAD_ADDR ;
		verts = cv2_mdl->verts ;
		for ( i = cv2_mdl->n_verts ; i > 0 ; blood++, verts++, scrpad_verts++, i-- ){
			float	len ;
			tmp.vx = verts->vx - vec.vx ;
			tmp.vy = verts->vy - vec.vy ;
			tmp.vz = verts->vz - vec.vz ;
			len = tmp.vx * tmp.vx + tmp.vy * tmp.vy + tmp.vz * tmp.vz ;
			scrpad_verts->vx = *blood * u_scale + u_offset ;
			scrpad_verts->vy = v ;
			if ( len > ( rangerange ) ) continue ;
			len = bp_sqrtf( len );  //BP_MATH - emulate PS2 sqrtf
			len = ( range - len ) * drange ;
			*blood = DG_MIN( 1.0f, *blood + len );
			scrpad_verts->vx = *blood * u_scale + u_offset ;
		}
		/* DG_VERTS_ANIMEのバッファ切り替え */
		DG_SwitchVAnimeBuffer( &work->v_anime[ j ] );
		/* 型変換をしてスクラッチパッドへ */
		DG_RegistCommonUV( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
		/* インデックスを参照してデータを分配 */
		DG_RefineStripUV2( &work->v_anime[ j ], cv2_mdl->verts_index );
	}

#ifdef KP_WINDOWS
	/* Color Vertex Buffer適用 */
	DG_AssignDGObjCVertexBuffer(&work->objs->objs[ joint ]) ;
#endif
}

	/*
		対象オブジェクトを変更
	*/
void TAKABE_OozeBloodChangeObjs( void *_work, DG_OBJS *objs )
{
	Work	*work = _work ;
	work->org_objs = objs ;
	/* モデルの登録順が崩れる場合があるので一度キューから外し、登録し直す */
	DG_DequeueObjs( work->objs );
	DG_QueueObjs( work->objs );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int		i ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	/* オリジナルと同じポーズを取る */
	work->objs->world = work->org_objs->world ;
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		work->objs->objs[ i ].world = work->org_objs->objs[ i ].world ;
	}

	/* 可視・不可視をオリジナルに合わせる */
	if ( work->org_objs->flag & DG_FLAG_INVISIBLE ){
		work->objs->flag |= DG_FLAG_INVISIBLE ;
	} else {
		work->objs->flag &= ~DG_FLAG_INVISIBLE ;
	}
	/* マップをオリジナルに合わせる */
	work->objs->group_id = work->org_objs->group_id ;
#if 0
	if ( GV_PadData[0].press & PAD_AL ){
		FVECTOR	pos ;
		pos.vx = work->objs->objs[ 0 ].world.m[3][0] ;
		pos.vy = work->objs->objs[ 0 ].world.m[3][1] ;
		pos.vz = work->objs->objs[ 0 ].world.m[3][2] + 100.0f ;
		TAKABE_OozeBloodAdd( work, 0, &pos, 50.0f );
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	int		i ;

	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return ;
	GV_Free( work->blood[0] );
	for ( i = 0 ; i < work->objs->n_models ; i++ ){
		DG_FreeAnimVertsBuffer( &work->v_anime[i] );
	}
	DG_DequeueObjs( work->objs );
	DG_FreeObjs( work->objs );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *org_objs, int model_id )
{
	CV2_DEF		*cv2_def ;
	CV2_MDL		*cv2_mdl ;
	DG_OBJS		*objs ;
	DG_TEX			*tex ;
	int			i, j ;

	work->model_id = model_id ;
	work->org_objs = org_objs ;

	/* モデルの共有頂点データ取得 */
	work->cv2_def = cv2_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
	ASSERT( cv2_def != NULL );
	if ( work->cv2_def->id == 6754556/* "Common Vertex Data" */ ) return (0);

	/* アルファグラデーションテクスチャの取得 */
	//tex = DG_GetTexture( 21710/* gray */ );
	//work->tex = tex = DG_GetTexture( 1/* <alpha gradation texture> */ );
	//work->tex = tex = DG_GetTexture( 50771/* gradation_alp */ );
	work->tex = tex = DG_GetTexture2( org_objs->tri_id, 9507494/* gradation_alp_ovl */ );
	work->tex_trans = tex->tex_trans ;
	work->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0 ) ;/* 減算半透明 */

	{
		int		size ;
		float	*ptr ;
		/* モデルの頂点総数を計算 */
		size = 0 ;
		for ( i = 0 ; i < work->cv2_def->n_models ; i++ ){
			size += work->cv2_def->models[ i ].n_verts ;
		}
		/* メモリを確保し、ワークに割り振る */
		ptr = GV_Malloc( sizeof( float ) * size );
		GV_ZeroMemory( ptr, sizeof( float ) * size );
		for ( i = 0 ; i < work->cv2_def->n_models ; i++ ){
			work->blood[ i ] = ptr ;
			ptr += work->cv2_def->models[ i ].n_verts ;
		}
	}

	/* 同じモデルを生成する */
	work->objs = objs = DG_MakeObjs( org_objs->def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 );
	DG_QueueObjs( objs );
	DG_SetLightMatrix( objs, work->lights );
	objs->tri_id = DG_SearchTriFromTex( tex );

	/* カラーマトリクスにシアンを設定（減算半透明で赤になるように） */
	work->lights[1].m[3][0] = 20.0f ;
	work->lights[1].m[3][1] = 100.0f ;
	work->lights[1].m[3][2] = 100.0f ;

	{/*  */
		DG_OBJ			*obj ;
		DG_MDL			*mdl ;
		DG_OBJ_PACKET	*packet ;
		DG_VERTS_ANIME	*v_anime ;

		obj = objs->objs ;
		v_anime = work->v_anime ;
		cv2_mdl = cv2_def->models ;
		for ( i = objs->n_models ; i > 0 ; obj++, v_anime++, cv2_mdl++, i-- ){
			/* 頂点アニメワークの取得 */
			DG_MakeAnimVertsBuffer( v_anime, obj, DG_VANIME_UVS | DG_VANIME_SINGLE );
			/* 強制的にサブパケットを修正する（半透明属性・テクスチャの変更） */
			mdl = obj->model ;
			packet = obj->packets ;
			for ( j = mdl->n_packs ; j > 0 ; packet++, j-- ){
				packet->tex_ptr[0] = &work->tex_trans ;
			}
			{/* ＵＶ値書き換え（無色状態で初期化） */
				FVECTOR	*uvs ;
				uvs = (void*)SCRPAD_ADDR ;
				for ( j = cv2_mdl->n_verts ; j > 0 ; j-- ){
					uvs->vx = 0.0f * tex->u_scale + tex->u_offset ;
					uvs->vy = 0.5f * tex->v_scale + tex->v_offset ;
					uvs++ ;
				}
				DG_RegistCommonUV( (void*)SCRPAD_ADDR, cv2_mdl->n_verts );
				DG_RefineStripUV2( v_anime, cv2_mdl->verts_index );
			}
		}
	}

	//NewEnvmap( objs, model_id, 0 );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewOozeBlood( DG_OBJS *objs, int model_id )
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
#endif

