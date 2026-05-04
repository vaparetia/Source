//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	screen.c
	チャンネル処理ユニット／モデルスクリーン座標計算ルーチン

	1999/07/07 K.Takabe
	$Id: xscreen.c,v 1.1.1.3 2002/11/19 11:42:37 Yoshizawa1 Exp $

*/
/*
	チャンネル処理ユニット／モデルスクリーン座標計算ルーチン
	＆バウンディングチェック

	void		DG_ScreenChanl( cp, which )
	DG_CHANL	*cp ;		チャンネル構造体
	int		which ;		ダブルバッファ選択

	キューされた各物体のワールド座標と関節回転データをもとに
	各ユニットのワールド座標／スクリーン座標を計算する
*/


#if 1//BP_ASM - use Xbox C version instead of PS2 ASM - #ifdef KP_XBOX //BP


#ifdef KP_XBOX
#include <xtl.h>
#else
//BP_ASM -  #include <windows.h>
//BP_ASM -  #include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"
#include "dgconf.h"
#include "private.h"

#include "def_dma.h"
#include "sse_inline.h"

#ifdef DEBUG_MODE
//int		DG_OnePieceSkip = 0 ;
#endif

//BP_ASM - extra defines, data and functions that are in commented out PS2 files
FMATRIX	DG_SSE_CurrentMatrix ;
void DG_StartBoundingCheckSupport( void )
{
}
//BP_ASM - extra defines, data and functions that are in commented out PS2 files


#define _CopyVector( a, b ) { *(u_long128*)a = *(u_long128*)b ; }
#if 1
/* SSEレジスタ破壊するので注意！！ */
static inline void _CopyMatrix( FMATRIX *m0, FMATRIX *m1 )
{
	__asm {
		mov ebx, m1 ;
		movups xmm0, [ebx+00h] ;
		movups xmm1, [ebx+10h] ;
		movups xmm2, [ebx+20h] ;
		movups xmm3, [ebx+30h] ;
		mov ebx, m0 ;
		movups [ebx+00h], xmm0 ;
		movups [ebx+10h], xmm1 ;
		movups [ebx+20h], xmm2 ;
		movups [ebx+30h], xmm3 ;
	}
}
#else
#define _CopyMatrix( a, b ) { 0[(u_long128*)a] = 0[(u_long128*)b] ;\
								1[(u_long128*)a] = 1[(u_long128*)b] ;\
								2[(u_long128*)a] = 2[(u_long128*)b] ;\
								3[(u_long128*)a] = 3[(u_long128*)b] ; }
#endif


	/*
		スクラッチパッド割り当て
	*/
typedef	struct	{
	FMATRIX		eye_pers ;
	FMATRIX		eye_pers2 ;
	FMATRIX		eye_inv ;

	FVECTOR		bound[8] ;
	FVECTOR		scale ;
	FVECTOR		tmp_vec ;
	float		screen ;		/* Ｚ値算出用補正定数 */
	int			sort_z ;		/* Ｗより求めたＺ値 */
	int			group_bound ;
	int			invisible_flag ;
	float		fog_param1, fog_param2 ;
	int			store_clock ;
	int			pad[1] ;
	char		bound_cache[ DG_MAX_JOINTS ];

	FMATRIX		j_root ;
	FMATRIX		joints[ DG_MAX_JOINTS ] ;
	FMATRIX		j_root2 ;
	FMATRIX		tmp[4];

	u_long128	local_work[0] ;

} ScrPad ;

#define	SCRPAD		((ScrPad*)SCRPAD_ADDR)
#define	EYE_PERS	(&(SCRPAD->eye_pers))
#define	EYE_PERS2	(&(SCRPAD->eye_pers2))
#define	J_ROOT		(&(SCRPAD->j_root))
#define	J_ROOT2		(&(SCRPAD->j_root2))
#define	JOINTS		(SCRPAD->joints)
#define BOUNDS		(SCRPAD->bound)
#define SCALE		(&SCRPAD->scale)
#define SCREEN		(SCRPAD->screen)
#define SORT_Z		(SCRPAD->sort_z)
#define GBOUND		(SCRPAD->group_bound)
#define	WORK_MAT	(&SCRPAD->tmp[0])
#define	WORK_MAT2	(&SCRPAD->tmp[1])

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
static	void	OnePieceObjs( DG_OBJS *, int ) ;
static	void	ScreenObjs( DG_OBJS *, int ) ;
static	void	SlideFrameObjs( DG_OBJS *, int ) ;
static	void	JointFrameObjs( DG_OBJS *, int ) ;
#endif


/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
#define Vu0CopyMatrix( _a, _b ) { *(_a) = *(_b) ; }


/*----------------------------------------------------------------*/
static inline void SetLightMatrix( FMATRIX *res_light, FMATRIX *light, FMATRIX *world )
{
	FMATRIX		tmp_mat ;
	tmp_mat = *world ;
	tmp_mat.m[3][0] = 0 ;
	tmp_mat.m[3][1] = 0 ;
	tmp_mat.m[3][2] = 0 ;
	_sceVu0MulMatrix( res_light, light, &tmp_mat );
}

/* ---------------------------------------------------------------- */
static int BoundCheck( FMATRIX *mat, FVECTOR *bound, int mode )
{
	int		and_flag, or_flag, prim_over_flag, flag ;
	FVECTOR		verts, tmp_v, bound_verts[8] ;
	float		w, total_w ;
	int			i ;

	and_flag = 0xffffffff ;
	or_flag = 0 ;
	prim_over_flag = 0 ;
	total_w = 0 ;

	/* バウンディング用頂点の生成 */
	bound_verts[0].vx = bound_verts[2].vx = bound_verts[4].vx = bound_verts[6].vx = bound[0].vx ;
	bound_verts[1].vx = bound_verts[3].vx = bound_verts[5].vx = bound_verts[7].vx = bound[1].vx ;
	bound_verts[0].vy = bound_verts[1].vy = bound_verts[4].vy = bound_verts[5].vy = bound[0].vy ;
	bound_verts[2].vy = bound_verts[3].vy = bound_verts[6].vy = bound_verts[7].vy = bound[1].vy ;
	bound_verts[0].vz = bound_verts[1].vz = bound_verts[2].vz = bound_verts[3].vz = bound[0].vz ;
	bound_verts[4].vz = bound_verts[5].vz = bound_verts[6].vz = bound_verts[7].vz = bound[1].vz ;

	_SetMatrix( mat );
	for ( i = 0 ; i < 8 ; i++ ){
		_RotTrans( &tmp_v, &bound_verts[i] );	/* wは1.0と見なす */
		total_w += tmp_v.vw ;					/* 射影マトリクスを掛けているのでvwにはカメラからの距離 */
#if 1 //BP - Enabled the non-assembler version of the bounding check.
		w = DG_FABS( tmp_v.vw );

		/* クリップチェック */
		flag = 0 ;
		if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
		if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
		if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
		if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
		if ( tmp_v.vz >  w ) flag |= CLIP_Z0_FLAG;
		if ( tmp_v.vz < -w ) flag |= CLIP_Z1_FLAG;
		//if ( tmp_v.vz < 0.0f ) flag |= CLIP_Z1_FLAG;	/* XBOXはPS2とは違うので注意 */
#else
		{/* 条件分岐を減らすためにＳＳＥを使ってチェック */
			static IVECTOR		bitmask_abs = {0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff};
			static IVECTOR		bitmask_mins = {0x80000000,0x80000000,0x80000000,0x80000000};
			IVECTOR			res_cmp1, res_cmp2 ;
			__asm {
				movups		xmm0, tmp_v				; 演算結果のロード
				movups		xmm1, bitmask_mins		; 符号ビットロード
				movaps		xmm2, xmm0				; 成分コピー
				shufps		xmm0, xmm0, 11111111b	; xmm0 をWのみに展開
				andps		xmm0, bitmask_abs		; ビットマスクで絶対値をとる
				orps		xmm1, xmm0				; xmm1に-Wを展開
				cmpltps		xmm0, xmm2				; 比較 W<X|Y|Z なら真
				cmpnleps	xmm1, xmm2				; 比較 -W>X|Y|Z なら真
				movups		res_cmp1, xmm0
				movups		res_cmp2, xmm1
			}
			flag = res_cmp1.vx & CLIP_X0_FLAG ;
			flag |= res_cmp1.vy & CLIP_Y0_FLAG ;
			flag |= res_cmp1.vz & CLIP_Z0_FLAG ;
			flag |= res_cmp2.vx & CLIP_X1_FLAG ;
			flag |= res_cmp2.vy & CLIP_Y1_FLAG ;
			flag |= res_cmp2.vz & CLIP_Z1_FLAG ;
		}
#endif
		and_flag &= flag ;
		or_flag |= flag ;

#if 0
		if ( mode == 0 ){
			/* ＸＹ軸において余裕を持った大きさでチェック */
			tmp_v.vx *= (SCALE)->vx / 640.0f ;
			tmp_v.vy *= (SCALE)->vy / 512.0f ;
			flag = 0 ;
			if ( tmp_v.vx >  w ) flag |= CLIP_X0_FLAG;
			if ( tmp_v.vx < -w ) flag |= CLIP_X1_FLAG;
			if ( tmp_v.vy >  w ) flag |= CLIP_Y0_FLAG;
			if ( tmp_v.vy < -w ) flag |= CLIP_Y1_FLAG;
			prim_over_flag |= flag ;
		}
#endif

	}
	/* バウンディング中央値のＺ座標取得 */
	SORT_Z = total_w * ( 1.0f / 8.0f ) ;

#if 0
	/* バウンディング判定 */
	if ( mode == 0 ){
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x30 ) return ( 1 );	/* クリップ必要 */
		if ( prim_over_flag ) return ( 1 );	/* クリップ必要 */
	} else {
		if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
		if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
	}
#else
	/* ＸＢＯＸではクリップによるコストは無視しても大丈夫そうなので */
	if ( and_flag & 0x3f ) return ( 2 );	/* 画面外 */
	if ( or_flag & 0x3f ) return ( 1 );	/* クリップ必要 */
#endif

	return ( 0 );	/* 完全画面内 */

}

static int CheckSortZ( FMATRIX *mat, FVECTOR *bound )
{
	FVECTOR		verts, tmp_v ;

	verts.vx = ( bound[0].vx + bound[1].vx ) * 0.5f ;
	verts.vy = ( bound[0].vy + bound[1].vy ) * 0.5f ;
	verts.vz = ( bound[0].vz + bound[1].vz ) * 0.5f ;
	//verts.vw = 1.0F ;
	//_sceVu0ApplyMatrix( &tmp_v, mat, &verts );
	_SetMatrix( mat );
	_RotTrans( &tmp_v, &verts );

	/* バウンディング中央値のＺ座標取得 */
	SORT_Z = tmp_v.vw ;

	return ( 0 );	/* 完全画面内 */
}

/*----------------------------------------------------------------*/
static FMATRIX	tmp_screen ;
static FMATRIX	tmp_offset ;
	/*
		マトリクス生成＆エンベロープ用補正マトリクスの生成
	*/
static inline void MakeMatrixStart( FMATRIX *world, FMATRIX *parent, FVECTOR *trans )
{
#if 0
	FMATRIX		mat ;
	_sceVu0InversMatrix( &mat, world );
	_sceVu0MulMatrix( &tmp_offset, &mat, parent );
	_sceVu0MulMatrix( &tmp_screen, EYE_PERS, world );
#endif
#if 0
	tmp_offset = *parent ;
	tmp_offset.m[3][0] = world->m[3][0] ;
	tmp_offset.m[3][1] = world->m[3][1] ;
	tmp_offset.m[3][2] = world->m[3][2] ;
	tmp_offset.m[3][3] = world->m[3][3] ;
#else
	tmp_offset = *parent ;
	//_sceVu0ApplyMatrix( (FVECTOR*)&tmp_offset.m[3][0], parent, trans );
	ApplyMatrix2( (FVECTOR*)&tmp_offset.m[3][0], parent, trans );
#endif
	//_sceVu0MulMatrix( &tmp_screen, EYE_PERS, world );
	MulMatrix( &tmp_screen, EYE_PERS, world );
}
static inline void MakeMatrixEnd( FMATRIX *screen, FMATRIX *offset_mat )
{
	*screen = tmp_screen ;
	*offset_mat = tmp_offset ;
}
/*----------------------------------------------------------------*/

	/*
		関節なし一体型モデルについて処理

		各ユニットのワールドマトリクス／スクリーンマトリクスは
		全て物体そのものと同じになる
	*/
static	void	OnePieceObjs( DG_OBJS *objs, int n_objs )
{
	DG_OBJ		*obj ;
	int		i, last_objs, flag, type ;
	ScrPad		*work = SCRPAD_ADDR ;

	_sceVu0MulMatrix( JOINTS, EYE_PERS, J_ROOT ) ;
	_sceVu0MulMatrix( J_ROOT2, EYE_PERS2, J_ROOT );
	obj = objs->objs ;
	for ( i = n_objs ; i > 0 ; -- i, obj++ ) {
		obj->world = *J_ROOT ;
		//obj->inv_mat = *J_ROOT ;

		type = obj->mdl_type ;
		/* 拡張モデル処理 */
		if ( type & DG_TYPE_EXTEND ){
			DG_OBJ	*parent = &objs->objs[ obj->parent ] ;
			obj->screen = parent->screen ;
			obj->bound_mode = parent->bound_mode ;
			obj->sort_z = parent->sort_z ;
			continue ;
		}

		/* バウンドチェック */
		if ( GBOUND == 1 ){
			flag = BoundCheck( JOINTS, &obj->bound_min, 0 );
		} else {
			flag = GBOUND ;
			if ( type & DG_TYPE_TRANS || !( obj->flag & DG_FLAG_PAINT ) ) CheckSortZ( JOINTS, &obj->bound_min );
		}
#if 0
	{/* バウンディングのデバッグ表示 */
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		//static FVECTOR max = {100,100,100,1}, min = {-100,-100,-100,1} ;
		//NewBoundingBoxView_1( joints, &max, &min, 0x00ffff00 );
		switch ( flag ){
		  case 0:
			NewBoundingBoxView_1( J_ROOT, &obj->model->lx, &obj->model->ux, 0x0000ffff );
			break ;
		  case 1:
			NewBoundingBoxView_1( J_ROOT, &obj->model->lx, &obj->model->ux, 0x008080ff );
			break ;
		  default:
			break ;
		}
	}
#endif
		/* バウンディングフラグ設定（０：クリップ必要なし、１：クリップ必要、２：完全画面外） */
		obj->bound_mode = flag ;
		/* 投影マトリクス設定 */
		if ( flag != 2 ) _CopyMatrix( &obj->screen, JOINTS );
		/* ソート値設定 */
		obj->sort_z = SORT_Z ;
	}
}

/*----------------------------------------------------------------*/

	/*
		各ユニットのワールドマトリクスから
		スクリーンマトリクスを計算する
	*/
static	void	ScreenObjs( DG_OBJS *objs, int n_objs )
{
	DG_OBJ		*obj ;
	int		i, flag, type ;
	ScrPad		*work = SCRPAD_ADDR ;

	obj = objs->objs ;
	for ( i = 0 ; i < n_objs ; i++ ) {

		type = obj->mdl_type ;
		/* 拡張モデル処理 */
		if ( type & DG_TYPE_EXTEND ){
			DG_OBJ	*parent = &objs->objs[ obj->parent ] ;
			obj->inv_mat = parent->inv_mat ;
			obj->world = parent->world ;
			obj->screen = parent->screen ;
			obj->bound_mode = parent->bound_mode ;
			obj->sort_z = parent->sort_z ;
			obj++ ;
			continue ;
		}

		/* 大幅な計算手順変更バージョン */

		/* シングルウェイトエンベロープ用補正マトリクスを計算 */
		//MakeMatrixStart( &obj->world, &objs->objs[ obj->parent ].world, &obj->trans );
		//MakeMatrixEnd( &work->tmp[0], &obj->inv_mat );
		{
			FMATRIX		*parent ;
			parent = &objs->objs[ obj->parent ].world ;
			obj->inv_mat = *parent ;
			ApplyMatrix2( (FVECTOR*)&obj->inv_mat.m[3][0], parent, &obj->trans );
			MulMatrix( &work->tmp[0], EYE_PERS, &obj->world );
		}

		/* バウンドチェック */
		if ( GBOUND == 1 ){
			/* バウンディング計算＆代表点Ｚ値計算 */
			flag = BoundCheck( &work->tmp[0], &obj->bound_min, 0 );
		} else {
			flag = GBOUND ;
#if 1
			if ( type & DG_TYPE_TRANS || !( obj->flag & DG_FLAG_PAINT ) ){
				/* 半透明及び光源計算モデルの場合のみＺ座標代表値を求める（フォグ＆ソート用） */
				CheckSortZ( &work->tmp[0], &obj->bound_min );
			}
#endif
		}

		/* バウンディングフラグ設定（０：クリップ必要なし、１：クリップ必要、２：完全画面外） */
#if 0
		//obj->bound_mode = flag ;
		work->bound_cache[ i ] = flag ;	/* 関節単体のバウンディング結果を保存 */
		if ( obj->parent != -1 &&
			work->bound_cache[ i ] != work->bound_cache[ obj->parent ] ){
			obj->bound_mode = 1 ;	/* 親のバウンディング結果と食い違っている場合には念のためクリップありを設定 */
		} else {
			obj->bound_mode = flag ;/* そうでなければ単体でのバウンディング結果を設定 */
		}
#else
		obj->bound_mode = flag ;/* クリップはハードウェアでやってくれるので */
#endif
		/* 投影マトリクス設定 */
		if ( obj->bound_mode != 2 ) _CopyMatrix( &obj->screen, &work->tmp[0] );
		/* ソート値設定 */
		obj->sort_z = SORT_Z ;

		obj ++ ;
	}
}


/*----------------------------------------------------------------*/

	/*
		横すべり関節型モデルについて処理

		objs->movs は、親ユニットからの相対位置
	*/
static	void	SlideFrameObjs( DG_OBJS *objs, int n_objs )
{
	FMATRIX		*joints, *parent ;
	FVECTOR		*movs, tmp ;
	DG_OBJ		*obj ;
	int		i ;

	movs = objs->movs ;
	joints = JOINTS ;
	obj = objs->objs ;
	for ( i = n_objs ; i > 0 ; -- i ) {
		parent = JOINTS + obj->parent ;
		_sceVu0ApplyMatrix( &tmp, parent, movs );
		*joints = *parent ;
		joints->m[3][0] = tmp.vx ;
		joints->m[3][1] = tmp.vy ;
		joints->m[3][2] = tmp.vz ;
		obj->world = *joints ;
		movs ++ ;
		obj ++ ;
		joints ++ ;
	}
}

/*----------------------------------------------------------------*/
	/*
		回転関節型モデルについて処理

		objs->rots は、ルートユニットからの相対回転
	*/
static	void	JointFrameObjs( DG_OBJS *objs, int n_objs )
{
	int			i ;
	DG_OBJ		*obj ;
	FMATRIX		*joints ;
	FVECTOR		*rots ;

	_sceVu0UnitMatrix( WORK_MAT );

	obj = objs->objs ;
	rots = objs->rots ;
	joints = JOINTS ;
	for ( i = 0 ; i < objs->def->n_models ; i++, obj++, rots++ ){
		if ( !( objs->flag & DG_FLAG_QUATROT ) ){
			_sceVu0UnitMatrix( WORK_MAT );
			if ( rots->vx != 0.0f ) _sceVu0RotMatrixX( WORK_MAT, WORK_MAT, rots->vx );
			if ( rots->vy != 0.0f ) _sceVu0RotMatrixY( WORK_MAT, WORK_MAT, rots->vy );
			if ( rots->vz != 0.0f ) _sceVu0RotMatrixZ( WORK_MAT, WORK_MAT, rots->vz );
		} else {
			extern void MT_QuatToMat( FMATRIX*, FVECTOR* );
			MT_QuatToMat( WORK_MAT, rots );
		}
		*(FVECTOR*)WORK_MAT->m[3] = obj->trans ;
		_sceVu0MulMatrix( (void*)&joints[i], (void*)&joints[ obj->parent ], (void*)WORK_MAT );
		obj->world = joints[i] ;
	}
}

/*----------------------------------------------------------------*/
	/*
		オブジェクトのマトリクスをセットアップする
	*/
void		DG_SetObjsMatrix( DG_OBJS *objs )
{
	int		n_objs, n_joints ;
	ScrPad	*scrpad = (ScrPad*)SCRPAD_ADDR ;

	n_objs = objs->n_models ;
	n_joints = objs->def->n_models ;
	/*
		物体自体に、親物体が指定されていた場合
		物体のワールド座標をコピーする
	*/
	if ( objs->root != NULL ) objs->world = *( objs->root ) ;
	*J_ROOT = objs->world ;
	if ( objs->flag & DG_FLAG_ONEPIECE ){
		/* 原点オフセット付加 */
		J_ROOT->m[3][0] += objs->trans.vx ;
		J_ROOT->m[3][1] += objs->trans.vy ;
		J_ROOT->m[3][2] += objs->trans.vz ;
	}
	/*
		全体バウンディングを行う
	*/
	_sceVu0MulMatrix( WORK_MAT, EYE_PERS, J_ROOT ) ;
	objs->bound_mode = GBOUND = BoundCheck( WORK_MAT, &objs->bound_min, ( objs->flag & DG_FLAG_ONEPIECE ) == 0 );
#if 0
	{/* ルートバウンディングのデバッグ表示 */
		extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		//static FVECTOR max = {100,100,100,1}, min = {-100,-100,-100,1} ;
		//NewBoundingBoxView_1( J_ROOT, &max, &min, 0x00ff0000 );
		if ( objs->bound_mode != 2 )
		NewBoundingBoxView_1( J_ROOT, &objs->def->lx, &objs->def->ux, 0x000000ff );
	}
#endif
	/*
		処理フラグにしたがって、マトリクスのセットアップ
	*/
	if ( DG_FLAG_ONEPIECE & objs->flag ) {
		OnePieceObjs( objs, n_objs ) ;
	} else if ( DG_FLAG_FINISHCALC & objs->flag ){
		if ( ( objs->flag & scrpad->invisible_flag ) && !( objs->flag & DG_FLAG_SHADOWMAKE ) ) return ;
		ScreenObjs( objs, n_objs ) ;
	} else {
		if ( objs->rots != NULL ) {
			JointFrameObjs( objs, n_joints ) ;
		} else if ( objs->movs != NULL ) {
			SlideFrameObjs( objs, n_joints ) ;
		} else {
			OnePieceObjs( objs, n_objs ) ;
			return ;
		}
		ScreenObjs( objs, n_objs ) ;
	}
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static void SetupLowObjs( DG_OBJS *objs )
{
	DG_OBJS		*low_objs ;
	DG_OBJ		*obj, *low_obj ;
	int		i ;
	obj = objs->objs ;
	low_objs = objs->low ;
	low_obj = low_objs->objs ;
	/* objsの内容を複製する */
	low_objs->world = objs->world ;
	low_objs->group_id = objs->group_id ;
	low_objs->bound_mode = objs->bound_mode ;

	for ( i = 0 ; i < low_objs->n_models ; i++ ){
		low_obj->world = obj->world ;
		low_obj->screen = obj->screen ;
		low_obj->inv_mat = obj->inv_mat ;
		low_obj->sort_z = obj->sort_z ;
		low_obj->fog = obj->fog ;
		low_obj->matrix_addr = obj->matrix_addr ;

		obj++ ;
		low_obj++ ;
	}
}
/*----------------------------------------------------------------*/
	/*
		カメラに応じたマトリクスのセットアップを行う
	*/
void		DG_ScreenChanl( DG_CHANL *cp, int which )
{
#if 0 //BP_XBOX - NOTE: This code is about to not be used anymore, the rendering code checked in doesn't require this data.
	DG_OBJS		**oque, *objs ;
	DG_OBJ_QUEUE	*que ;
	DG_OBJ_BUFFER	*obj_buff ;
	ScrPad	*scrpad = (ScrPad*)SCRPAD_ADDR ;
	int		i, time ;

	que = cp->obj_queue ;
	MARK( "screen.c" );
	if ( que == NULL ) return ;
	obj_buff = &que->objs_buffer ;
	if ( obj_buff->n_queue == 0 ) return ;
	GV_SET_PRFC_CLOCK();

	/*
		カメラ逆行列を、スクラッチパッドに複製しておく
		その際、ピクセルアスペクト比率を調整しておく
	*/
	*EYE_PERS = cp->eye_pers ;
	// *EYE_PERS2 = cp->eye_pers2 ;
	scrpad->eye_inv = cp->eye_inv ;
	SCALE->vx = cp->width / 2 ;
	SCALE->vy = cp->height / 2 ;
	SCREEN = 0.5F ;
	scrpad->invisible_flag = DG_FLAG_INVISIBLE0 << cp->chanl_num ;
	scrpad->fog_param1 = DG_FogParam1 ;
	scrpad->fog_param2 = DG_FogParam2 ;
	scrpad->store_clock = 0 ;

	/* マトリクスのセットアップ */
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; -- i ) {
		DG_SetObjsMatrix( *oque );
		oque ++;
	}

	if ( DG_UnDrawFrameCount != 0 ) return ;

	/* マトリクス情報のメモリへの書き出し */
	oque = (DG_OBJS**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i--, oque++ ) {
		objs = *oque ;
		if ( ( objs->flag & scrpad->invisible_flag ) || ( objs->bound_mode == 2 ) ){
			/* ローポリ用モデルが関連付けされている場合にはセットアップ内容を複製する */
			/* このときは親が非表示になっているためセットアップパケットアドレスは意味をなさない */
			if ( objs->low != NULL ){
				SetupLowObjs( objs );
			}
			continue ;
		}
		/* オブジェクト固有フォグパラメータ設定 */
		if ( objs->flag & DG_FLAG_FOGPARAM ){
			scrpad->fog_param1 = objs->fog_param[0] ;
			scrpad->fog_param2 = objs->fog_param[1] ;
		}
		//StoreMatrixObjs( *oque );
		/* オブジェクト固有フォグパラメータ設定を元に戻す */
		if ( objs->flag & DG_FLAG_FOGPARAM ){
			scrpad->fog_param1 = DG_FogParam1 ;
			scrpad->fog_param2 = DG_FogParam2 ;
		}
		/* ローポリ用モデルが関連付けされている場合にはセットアップ内容を複製する */
		if ( objs->low != NULL ){
			SetupLowObjs( objs );
		}
	}

	time = GV_GET_PRFC_CLOCK();
	if ( GV_PadData[1].press & PAD_A ){
		//printf("screen.c: %d\n", time );
	}
#endif
}

#endif
