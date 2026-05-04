//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	faceanim.c
	フェイスモーション管理ルーチン

	2000/06/12 K.Takabe
	$Id: faceanim.c,v 1.1.1.3 2002/11/19 11:42:50 Yoshizawa1 Exp $

*/
/*


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

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"def_dma.h"


//static void MT_QuatMulFast( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
//static float MFtoF( unsigned short mf );

/*----------------------------------------------------------------*/

extern int BP_BASE_TICK();
#define TIME_BASE	(BP_BASE_TICK())

#define FACE_JOINT_NUM	(21)

#ifdef PSX2
extern qword MotionDecodeVu0 ;
extern qword MotionDecodeVu0_Func ;
#endif

/*----------------------------------------------------------------*/

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;

/* ========= スタティック関数用プロトタイプ宣言 ========= */
/* ２つのモーションデータの中間を計算する */
static void InterpFaceData( MT_FACE_DATAS *res, MT_FACE_DATAS *fm1, MT_FACE_DATAS *fm2, float t );


/* ========= インライン関数群 ========= */



/* ---------------------------------------------------------------- */
	/*
		クォータニオン補間計算マイクロプログラム読み込み
	*/
#if 0
void MT_StartMotionDecodeSupport( void )
{
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( (int)((DG_DMATAG*)MotionDecodeVu0)->addr ) ;
	DPUT_D0_QWC( ( ((DG_DMATAG*)MotionDecodeVu0)->qwc ) & 0x7fff ) ;
	DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
}
void MT_WaitMotionDecodeSupport( void )
{
	DG_WaitDma( 0 );
}
#endif

#if 0
static void QuatSlerp( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, float t )
{
	FVECTOR		tvec ;
	tvec.vx = 1.0f - t ;
	tvec.vy = t ;
	asm volatile("
		lqc2		vf01,0x00(%0)
		lqc2		vf02,0x00(%1)
		lqc2		vf03,0x00(%2)
		vcallms		0x00
	"::"r"(q0),"r"(q1),"r"(&tvec));
	asm volatile("
		qmfc2.i		$12,vf01
		sq			$12,0(%0)
	"::"r"(res): "$12","memory" );
}
#endif

#if 0 //BP_PS2
//#ifdef PSX2
static void SetQuatSlerp( FVECTOR *q0, FVECTOR *q1, float t )
{
	FVECTOR		tvec ;
	tvec.vx = 1.0f - t ;
	tvec.vy = t ;

	asm volatile("
		lqc2		vf01,0x00(%0)
		lqc2		vf02,0x00(%1)
		lqc2		vf03,0x00(%2)
		vcallms		0x00
	"::"r"(q0),"r"(q1),"r"(&tvec));
}
static void GetQuatSlerp( FVECTOR *res )
{
	asm volatile("
		qmfc2.i		$12,vf01
		sq			$12,0(%0)
	"::"r"(res): "$12","memory" );
}
#endif

static void LerpVecFpu( FVECTOR *res, FVECTOR *v1, FVECTOR *v2, float t )
{
#ifndef PSX2
	res->vx = ( v2->vx - v1->vx ) * t + v1->vx ;
	res->vy = ( v2->vy - v1->vy ) * t + v1->vy ;
	res->vz = ( v2->vz - v1->vz ) * t + v1->vz ;
#else
	/* X = x1 - ( x1 * t ) + ( x2 * t ) = ( x2 - x1 ) * t + x1 */
	DG_ADDA( v1->vx, 0.0f );
	DG_MSUBA( v1->vx, t );
	res->vx = DG_MADD( v2->vx, t );
	/* Y */
	DG_ADDA( v1->vy, 0.0f );
	DG_MSUBA( v1->vy, t );
	res->vy = DG_MADD( v2->vy, t );
	/* Z */
	DG_ADDA( v1->vz, 0.0f );
	DG_MSUBA( v1->vz, t );
	res->vz = DG_MADD( v2->vz, t );
#endif
}

static void LoadNextKeyMotion( MT_FACE_CONTROL *f_ctrl, int pattern )
{
	int		i ;
	/* キーモーションパターンをコピー */
	FAR_JOINT_DATA	*motion_pattern ;
	FVECTOR	*dst_quat, *dst_trans ;

	motion_pattern = &f_ctrl->header->pattern_table[ pattern * f_ctrl->header->n_joints ] ;
	//f_ctrl->org_face = f_ctrl->trg_face ;
	dst_quat = f_ctrl->trg_face.quat ;
	dst_trans = f_ctrl->trg_face.trans ;
	for ( i = FACE_JOINT_MAX ; i > 0 ; i-- ){
		DG_COPY128( dst_quat, &motion_pattern->qrot );
		DG_COPY128( dst_trans, &motion_pattern->trans );
		dst_quat++ ;
		dst_trans++ ;
		motion_pattern++ ;
	}
}
/*----------------------------------------------------------------*/
	/*
		フェイスモーション制御構造体の初期化
	*/
void MT_InitFaceControl( MT_FACE_CONTROL *f_ctrl, int name_id, int flag )
{
	FAR_HEADER	*header ;
	int			i ;

	f_ctrl->header = NULL ;
	f_ctrl->motion_num = -1 ;
	f_ctrl->flag = 0 ;

	header = GV_GetCache( GV_CacheID( name_id, 'f' ) );
	if ( header == NULL ){
		printf("no face motion file !!\n");
		return ;
	}
	f_ctrl->header = header ;

	for ( i = 0 ; i < FACE_JOINT_MAX ; i++ ){
		f_ctrl->trg_face.trans[ i ] = DG_ZeroVector ;
		f_ctrl->trg_face.quat[ i ] = DG_ZeroVector ;
		f_ctrl->org_face.trans[ i ] = DG_ZeroVector ;
		f_ctrl->org_face.quat[ i ] = DG_ZeroVector ;
		f_ctrl->fix_face.trans[ i ] = DG_ZeroVector ;
		f_ctrl->fix_face.quat[ i ] = DG_ZeroVector ;
	}
}

	/*
		フェイスモーション設定
	*/
void MT_SetFaceMotion( MT_FACE_CONTROL *f_ctrl, int motion_num )
{
	FAR_MOTION_INFO	*motion ;
	FAR_MOTION_DATA	*data ;

	if( ( motion_num < 0 ) || ( motion_num >= f_ctrl->header->max_motion ) ){
		motion = NULL ;
		f_ctrl->motion_num = -1 ;
	} else {
		motion = &f_ctrl->header->motion_table[ motion_num ] ;
		f_ctrl->motion_num = motion_num ;
	}
	f_ctrl->time = 0.0f ;
	if ( motion != NULL && motion->data->time != 0xffff ){
		data = motion->data ;
		f_ctrl->total_time = motion->max_frame ;
		f_ctrl->next_time = 0.0f ;

		f_ctrl->prev_time = f_ctrl->next_time ;
		f_ctrl->next_time = data->time ;
		if ( f_ctrl->prev_time != f_ctrl->next_time ){
			f_ctrl->inv_time = 1.0f / ( f_ctrl->next_time - f_ctrl->prev_time ) ;
		} else {
			f_ctrl->inv_time = 1.0f ;
		}
		memcpy( &f_ctrl->org_face, &f_ctrl->fix_face, sizeof(MT_FACE_DATAS) );
		LoadNextKeyMotion(f_ctrl, data->pattern );
		f_ctrl->current_data = data ;
		f_ctrl->flag = MT_FACE_ACTIVE ;
	} else {
		f_ctrl->total_time = 0.0f ;
		f_ctrl->next_time = 0.0f ;
	}
}

	/*
		フェイスモーション実行
	*/
void MT_ActFaceMotion( MT_FACE_CONTROL *f_ctrl )
{
	float	t ;
	FAR_MOTION_DATA	*data ;

	if ( !( f_ctrl->flag & MT_FACE_ACTIVE ) ) return ;

	/* ＶＵ０サポートルーチン読み込み */
	MT_StartMotionDecodeSupport();
	//MT_WaitMotionDecodeSupport();

	f_ctrl->time += TIME_BASE ;
	if ( f_ctrl->time > f_ctrl->next_time ){
		data = f_ctrl->current_data ;
		data++ ;
		if ( data->time != 0xffff ){
			f_ctrl->prev_time = f_ctrl->next_time ;
			f_ctrl->next_time = data->time ;
			//MT_WaitMotionDecodeSupport();/* なぜこんなところに入っているのだろう？ */
			memcpy( &f_ctrl->org_face, &f_ctrl->trg_face, sizeof(MT_FACE_DATAS) );
			LoadNextKeyMotion( f_ctrl, data->pattern );
			if ( f_ctrl->prev_time != f_ctrl->next_time ){
				f_ctrl->inv_time = 1.0f / ( f_ctrl->next_time - f_ctrl->prev_time ) ;
			} else {
				f_ctrl->inv_time = 1.0f ;
			}
			f_ctrl->current_data = data ;
		}
	}
	MT_WaitMotionDecodeSupport();

	t = ( f_ctrl->time - f_ctrl->prev_time ) * f_ctrl->inv_time ;
	t = DG_MIN( t, 1.0f );
	InterpFaceData( &f_ctrl->fix_face, &f_ctrl->org_face, &f_ctrl->trg_face, t );

	if ( f_ctrl->time >= f_ctrl->total_time ){
		f_ctrl->flag &= ~MT_FACE_ACTIVE ;
		f_ctrl->motion_num = -1 ;
	}

}

	/*
		フェイスモーション割り当て
	*/
void MT_PutFaceMotion( DG_EVMOBJ *evmobj, MT_FACE_CONTROL *f_ctrl )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FVECTOR		*skel_rot, *skel_trans ;
	FMATRIX		*skel_mats ;
	FMATRIX		*mats ;
	EVM_SKEL	*skel ;

	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;
	skel_rot = f_ctrl->fix_face.quat ;
	skel_trans = f_ctrl->fix_face.trans ;

	/* オブジェクトにマトリクスを設定する */
	skel = &evmobj->def->skeleton[ FACE_JOINT_NUM ] ;
	scrpad->vec.vw = 1.0F ;
	mats = &scrpad->mats[FACE_JOINT_NUM] ;
	/* モデル情報から親子関係を取得してマトリクスを求める */
	for ( i = FACE_JOINT_NUM ; i < ( FACE_JOINT_NUM + FACE_JOINT_MAX ) ; i++ ){
		FMATRIX	*parent ;
		MT_QuatToMat( mats, skel_rot );
		mats->m[3][0] = skel->rt_tx + skel_trans->vx ;
		mats->m[3][1] = skel->rt_ty + skel_trans->vy ;
		mats->m[3][2] = skel->rt_tz + skel_trans->vz ;
		if ( skel->parent < FACE_JOINT_NUM ){
			parent = &skel_mats[ skel->parent ] ;	/* 顔関節以前の番号であればメモリから取得 */
		} else {
			parent = &scrpad->mats[ skel->parent ] ;/* それ以外ならスクラッチパッドから取得 */
		}
		_sceVu0MulMatrix( mats, parent, mats ) ;
		scrpad->vec.vx = -skel->rt_tx ;
		scrpad->vec.vy = -skel->rt_ty ;
		scrpad->vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
		skel_mats[i] = *mats ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
		mats++ ;
	}
}

	/*
		フェイスモーション割り当て（マスク合成）
	*/
void MT_PutFaceMotionEx( DG_EVMOBJ *evmobj, MT_FACE_CONTROL *f_ctrl1, MT_FACE_CONTROL *f_ctrl2, int mask_num )
{
}


/*----------------------------------------------------------------*/
/* ２つのモーションデータの中間を計算する */
static void InterpFaceData( MT_FACE_DATAS *res, MT_FACE_DATAS *fm1, MT_FACE_DATAS *fm2, float t )
{
	int		i ;
	FVECTOR	*quat_res, *quat1, *quat2 ;
	FVECTOR	*trans_res, *trans1, *trans2 ;

	quat_res = res->quat ;
	trans_res = res->trans ;
	quat1 = fm1->quat ;
	trans1 = fm1->trans ;
	quat2 = fm2->quat ;
	trans2 = fm2->trans ;
	for ( i = 0 ; i < FACE_JOINT_MAX ; i++ ){

#if 1 //BP_PS2
//#ifndef PSX2
		MT_QuatSlerp( quat_res, quat1, quat2, t );
		LerpVecFpu( trans_res, trans1, trans2, t );
		MT_QuatNormalize( quat_res, quat_res );
#else
		SetQuatSlerp( quat1, quat2, t );		/* ＶＵ０のマイクロプログラムでバックグラウンド実行 */
		LerpVecFpu( trans_res, trans1, trans2, t );/* ＦＰＵのみによる演算でＶＵ０と並列に実行する */
		GetQuatSlerp( quat_res );
		MT_QuatNormalize( quat_res, quat_res );
#endif
		quat_res++ ;
		quat1++ ;
		quat2++ ;
		trans_res++ ;
		trans1++ ;
		trans2++ ;
	}
}


/*----------------------------------------------------------------*/
#if 0

	/*
		クォータニオンの乗算（正規化なし）
	*/
static void MT_QuatMulFast( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
#ifdef PSX2
	asm ( "
		lqc2			vf04,0(%1)
		lqc2			vf05,0(%2)
		vaddw.xyz		vf01,vf00,vf00
		vmulx.xyzw		vf06,vf05,vf04
		vmuly.xyzw		vf07,vf05,vf04
		vmulz.xyzw		vf08,vf05,vf04
		vmulaw.xyzw		ACC ,vf05,vf04
		vmaddaw.x		ACC ,vf01,vf06
		vmaddaz.x		ACC ,vf01,vf07
		vmsuby.x		vf04,vf01,vf08
		vmaddaw.y		ACC ,vf01,vf07
		vmaddax.y		ACC ,vf01,vf08
		vmsubz.y		vf04,vf01,vf06
		vmaddaw.z		ACC ,vf01,vf08
		vmadday.z		ACC ,vf01,vf06
		vmsubx.z		vf04,vf01,vf07
		vmsubax.w		ACC ,vf00,vf06
		vmsubay.w		ACC ,vf00,vf07
		vmsubz.w		vf04,vf00,vf08
		sqc2			vf04,0(%0)
	"::"r"(res),"r"(q1),"r"(q2):"memory");
#else
#endif
}


/* ---------------------------------------------------------------- */
/* ================================================================ */
/*

	１６ビット浮動小数点フォーマット

	seeeeeffffffffff
	||    |
	||    +- 10bit 仮数部
	||
	|+- 5bit 指数（バイアス値 N の下駄履き指数）
	|
	+- 1bit 符合

*/

/* 指数のバイアス値（2^0となる指数値） */
#define E_BIAS	(8)		/* メタルギアでは大きい値を重視する */

/* 単精度浮動小数点から１６ビット浮動小数点へ変換 */
static unsigned short FtoMF( float f )
{
	unsigned int	data, s_dat, e_dat, f_dat ;
	unsigned short	mf ;

	data = *(unsigned int*)&f ;
	/* 符合取りだし */
	s_dat = ( data & 0x80000000 ) >> 16 ;
	/* 指数取りだし */
	e_dat = ( data & 0x7f800000 ) ;
	if ( e_dat != 0 ){
		if ( e_dat <= ( 127 - E_BIAS ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x0000 | s_dat );
		}
		if ( e_dat > ( 127 - E_BIAS + 31 ) << 23 ){
			//printf("convert error!!\n");
			return ( 0x7fff | s_dat );
		}
		e_dat -= ( 127 - E_BIAS ) << 23 ;
		e_dat >>= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( data & 0x007fffff ) >> 13 ;

	/* ビット合成 */
	mf = s_dat | e_dat | f_dat ;

	return ( mf );
}

/* １６ビット浮動小数点から単精度浮動小数点へ変換 */
static float MFtoF( unsigned short mf )
{
	unsigned int	s_dat, e_dat, f_dat ;
	float			f ;

	/* 符合取りだし */
	s_dat = ( mf & 0x8000 ) << 16 ;
	/* 指数取りだし */
	e_dat = ( mf & 0x7c00 ) ;
	if ( e_dat != 0 ){
		e_dat += ( 127 - E_BIAS ) << 10 ;
		e_dat <<= 13 ;
	}
	/* 仮数部取りだし */
	f_dat = ( mf & 0x03ff ) << 13 ;
	/* ビット合成 */
	*(unsigned int*)&f = s_dat | e_dat | f_dat ;

	return ( f );
}


#endif
