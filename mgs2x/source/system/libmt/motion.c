//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	motion.c
	モーション管理ルーチン

	1999/07/07 K.Takabe
	$Id: motion.c,v 1.4 2002/11/23 11:49:43 Yoshizawa1 Exp $

*/
/*

	void *MT_InitMotion( DG_OBJS *objs, int n_layer, int motion, int flag )
	DG_OBJS *objs ;	モーションを割り当てるオブジェクト
	int n_layer ;	同時に使用するモーション数
	int motion ;	モーションアーカイバ名ＩＤ
	int flag ;		各種フラグ
	返値：初期化したモーション構造体へのポインタ

	モーション用ワークの初期化


	void MT_ActMotion( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, DG_EVMOBJ *evmobj )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体
	DG_OBJS	*objs ;				モーションを割り当てるオブジェクト

	モーション再生処理を行う


	void MT_FreeMotion( MOTION_CONTROL *m_ctrl )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体

	モーション用ワークを開放する


	void MT_SetMotionData( MOTION_CONTROL *m_ctrl, int layer, int motion, int time, u_long64 mask )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体
	int layer ;					モーション再生レイヤー
	int motion ;				モーション番号
	int time ;					モーション再生初期時間（1/300単位）
	u_long64 mask ;				モーション有効関節指定

	モーションの設定


	void MT_ResetMotionData( MOTION_CONTROL *m_ctrl, int layer )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体
	int layer ;					モーション再生レイヤー

	モーションの停止


	void MT_SetMotionInterp( MOTION_CONTROL *m_ctrl, int time, u_long64 flag )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体
	int time ;					補間有効時間（1/300単位）
	u_long64 flag ;				補間有効関節指定

	モーション補間の設定


	void MT_SetMotionSeTable( MOTION_CONTROL *m_ctrl, int map_name, int table_id, int hazard_type, int segment_type )
	MOTION_CONTROL *m_ctrl ;	モーション制御構造体
	int map_name ;				キャラクタの位置するマップ名
	int table_id ;				ＳＥ変換テーブルＩＤ
	int hazard_type ;			ハザードの属性
	int segment_type ;			セグメントの属性

	モーションに同期して再生される効果音の変換テーブルの情報をセットする


	void MT_SetMotionSpeed( MOTION_CONTROL *m_ctrl, float time )
	MOTION_CONTROL	*m_ctrl ;	モーション制御構造体
	float			time ;		モーション再生間隔時間（１／３００秒単位）

	モーションの再生スピードを変更する（負ならデフォルト値に戻す）


	float MT_GetMotionStartHeight( MOTION_CONTROL *m_ctrl, int motion )
	MOTION_CONTROL	*m_ctrl ;	モーション制御構造体
	int				motion ;	モーション番号

	モーションの初期高さ取得

	--------------------------------

	マクロの解説

	#define MT_CHECK_END(mt,a) ( ( (mt)->mt3_ctrl[a].flag & MT3_PLAYEND ) != 0 )
	MOTION_CONTROL	*mt ;		モーション制御構造体
	int				a ;			レイヤ番号

		モーションの最終フレームを再生した後に１になる


	#define MT_CHECK_LAST1(mt,a) ( ( (mt)->mt3_ctrl[a].flag & MT3_PLAYLAST1 ) != 0 )
	MOTION_CONTROL	*mt ;		モーション制御構造体
	int				a ;			レイヤ番号

		モーションの最終フレームの１フレーム前を再生した後に１になる


	#define MT_CHECK_MOTION_TIME(mt,ly,tm)	( MT_CheckMotionTime( &(mt)->mt3_ctrl[(ly)], (tm) ) )
	MOTION_CONTROL	*mt ;		モーション制御構造体
	int				ly ;		レイヤ番号
	int				time ;		チェックする時間（１／３００単位）

		指定した時間が経過したかをチェック
		複数のチェックを行なう場合には小さい値から順番にチェックする事

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

#ifdef KP_XBOX
#include <xtl.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"

#ifdef PSX2
#include	"utl_dma.h"
#include	"def_dma.h"
#endif

#include "bp_quaternion.h"
#include "bp_vector.h"

#ifdef KP_XBOX
#ifdef BPE_TEMP_UNUSED
#define acosf( f )	((float)acos( f ))
#define cosf( f )	((float)cos( f ))
#define sqrtf( f )	((float)sqrt( f ))
#define sinf( f )	((float)sin( f ))
#define atanf( f )	((float)atan( f ))
#endif
#endif



static void MT_QuatMulFast( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
static float MFtoF( unsigned short mf );

/*----------------------------------------------------------------*/

extern int BP_BASE_TICK();
#define TIME_BASE	(BP_BASE_TICK())

/* データ位置補正マクロ */
#define ADJUST_POINTER(a,b,c) { b += c ; a += b / 16 ; b &= 0x0f ; }

typedef struct {
	FVECTOR			prev, c_prev ;	/* 補間元クォータニオン＆補正用クォータニオン */
	FVECTOR			next, c_next ;	/* 補間先クォータニオン＆補正用クォータニオン */
	FVECTOR			t ;
} SQUAD_WORK ;

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[128] ;
	FVECTOR	trans[128] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[128] ;
	SQUAD_WORK	squad_work ;
} ScrPadWork ;

#define SCRPADWORK	((ScrPadWork*)SCRPAD_ADDR)
#define QUAT_TMP	(((ScrPadWork*)SCRPAD_ADDR)->quat_buffer)
#define QUAT_ROTS	(((ScrPadWork*)SCRPAD_ADDR)->joints)
#define TRANS_VEC	(((ScrPadWork*)SCRPAD_ADDR)->trans)
#define TMP_VEC		(((ScrPadWork*)SCRPAD_ADDR)->vec)

static FVECTOR	ZeroFQuat = {0.0F,0.0F,0.0F,1.0F} ;

#ifdef PSX2
extern qword MotionDecodeVu0 ;
extern qword MotionDecodeVu0_Func ;
#endif

/*----------------------------------------------------------------*/

/* ========= スタティック関数用プロトタイプ宣言 ========= */
/* ｍｔ３モーションデータを再生ワークにセットする */
static void SetMotion( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *motion_data, int time );
/* モーション再生処理 */
static void PlayMotion( MT3_CONTROL *mt3_ctrl );
/* 移動データ展開ワーク初期化 */
static void InitMoveControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 移動データ展開ワーク初期化 */
static void InitMoveControlWork2( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 移動軌跡データ展開ワーク初期化 */
static void InitRootControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 関節固定情報管理ワーク初期化 */
static void InitFixControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 移動データ展開ワーク初期化 */
static void InitJointControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 関節移動量データ展開ワーク初期化 */
static void InitTransControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 次フレームの移動量データを設定 */
static void PlayMoveData( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 次フレームの移動量データを設定 */
static void PlayMoveData2( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 次フレームの移動軌跡データを設定 */
static void PlayRootData( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 関節固定データを更新 */
static void PlayFixData( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 次フレームの関節移動量データを設定 */
static void PlayTransData( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 関節回転データの書き出し */
static void WriteJointData( MT3_CONTROL *mt3_ctrl, long64 mask );
/* 関節移動量データの書き出し */
static void WriteTransData( MT3_CONTROL *mt3_ctrl, long64 mask );
/* 次フレームの関節回転データを設定 */
static void PlayJointData( MT3_CONTROL *mt3_ctrl, int end_flag );
/* 新たな関節用データを読み込んで設定する */
static void SetNextQuatData( MT3_JOINT_SEGMENT *joints );
/* クォータニオンデータをバッファから読み込む */
static void GetQuatDataFromBuffer( unsigned short *data, int bit_pos, FVECTOR *q );


/* ========= インライン関数群 ========= */
/* マスク指定型 */
static inline int PopDataMask( u_short *pos, char b_pos, u_int mask )
{
	u_int work;
	work = pos[0] + (pos[1] << 16);
	work >>= b_pos;
	return ( work & mask );
}
/* ビット数指定型 */
static inline int PopDataBit( u_short *pos, char b_pos, int bit )
{
	u_int work,mask;
	mask = ( 1 << bit ) - 1 ;
	work = pos[0] + (pos[1] << 16);
	work >>= b_pos;
	return ( work & mask );
}
/* マスク指定符号拡張型 */
static inline int PopDataMaskExt( u_short *pos, char b_pos, int bit, u_int mask )
{
	u_int work;
	work = pos[0] + (pos[1] << 16);
	work >>= b_pos;
	work &= mask;
	if ( work & (1 << (bit-1))){
		work |= ~mask;
	}
	return ( work );
}
/* マスク指定符号拡張型 */
static inline int PopDataBitExt( u_short *pos, char b_pos, int bit )
{
	u_int work,mask;
	mask = ( 1 << bit ) - 1 ;
	work = pos[0] + (pos[1] << 16);
	work >>= b_pos;
	work &= mask;
	if ( work & (1 << (bit-1))){
		work |= ~mask;
	}
	return ( work );
}




/* ---------------------------------------------------------------- */
	/*
		クォータニオン補間計算マイクロプログラム読み込み
	*/
void MT_StartMotionDecodeSupport( void )
{
#if 0 //BP_PS2
//#ifdef PSX2
	DG_DmaStartCheck( 0 );
	DPUT_D0_MADR( (int)((DG_DMATAG*)MotionDecodeVu0)->addr ) ;
	DPUT_D0_QWC( ( ((DG_DMATAG*)MotionDecodeVu0)->qwc ) & 0x7fff ) ;
	//DPUT_D0_CHCR( 0x0141 ) ; /* STR:1 TIE:0 TTE:1 ASP:0 MOD:0 DIR:1 */
	asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D0_CHCR),"r"(0x141) );
#endif	
}
void MT_WaitMotionDecodeSupport( void )
{
#ifdef PSX2	
	DG_WaitDma( 0 );
#endif	
}

#if 0 //BP_PS2
//#ifdef PSX2
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
#endif //PSX2

/*----------------------------------------------------------------*/

	/*
		モーション用ワークの初期化
	*/
void *MT_InitMotion( DG_OBJS *objs, int n_layer, int motion, int flag )
{
	MOTION_CONTROL	*m_ctrl ;
	int		n_joints, size, i ;
	void	*mem ;
	MAR_HEADER		*mar_header ;

#if 0
	if ( objs == NULL ){
		printf("motion init faild!! -- no object\n");
		return ( NULL ) ;
	}
#endif

	/* モーションデータの取得 */
	mar_header = MT_GetMotionArchives( motion );
	if ( mar_header == NULL ){
		printf("motion init faild!! -- no motions file\n");
		return ( NULL ) ;
	}

	n_joints = mar_header->joints ;

	/* メモリの確保 */
	size = ( sizeof(MOTION_CONTROL)
			+ sizeof(MT3_CONTROL) * n_layer
			+ sizeof(MT3_MOVE_SEGMENT) * n_layer
			+ sizeof(MT3_MOVE_SEGMENT2) * n_layer
			+ sizeof(MT3_ROOT_SEGMENT) * n_layer
			+ sizeof(MT3_FIX_SEGMENT) * n_layer
			+ sizeof(MT3_TURN_SEGMENT) * n_layer
			+ sizeof(MT3_JOINT_SEGMENT) * n_joints * n_layer
			+ sizeof(FVECTOR) * n_joints * 4 /* rots + adjust + abs_rots + old_abs_rots */
			);
	/* 移動量データの展開フラグがついているときのみメモリを確保 */
	if ( flag & MT_FLAG_TRANS ){
		size += sizeof(MT3_TRANS_SEGMENT) * n_joints * n_layer ;
		size += sizeof(FVECTOR) * n_joints * 2 ;/* trans + old_trans */
	}

	mem = GV_Malloc( size );
	if ( mem == NULL ) return ( NULL );
	GV_ZeroMemory( mem, size );
	size = 0 ;
	m_ctrl = mem ;
	size += sizeof(MOTION_CONTROL) ;
	m_ctrl->mt3_ctrl = (MT3_CONTROL*)((char*)mem + size);
	size += sizeof(MT3_CONTROL) * n_layer ;
	m_ctrl->rots = (FVECTOR*)((char*)mem + size) ;
	size += sizeof(FVECTOR) * n_joints ;
	m_ctrl->adjust = (FVECTOR*)((char*)mem + size) ;
	size += sizeof(FVECTOR) * n_joints ;
	m_ctrl->abs_rots = (FVECTOR*)((char*)mem + size) ;
	size += sizeof(FVECTOR) * n_joints ;
	m_ctrl->old_abs_rots = (FVECTOR*)((char*)mem + size);
	size += sizeof(FVECTOR) * n_joints ;
	/* 移動量データに関するメモリ割り当て */
	if ( flag & MT_FLAG_TRANS ){
		m_ctrl->trans = (FVECTOR*)((char*)mem + size) ;
		size += sizeof(FVECTOR) * n_joints ;
		m_ctrl->old_trans = (FVECTOR*)((char*)mem + size );
		size += sizeof(FVECTOR) * n_joints ;
	}

	/* ワークの初期化 */
	m_ctrl->motion_arc = mar_header ;
	m_ctrl->n_layer = n_layer ;
	m_ctrl->n_joints = n_joints ;
	m_ctrl->flag = flag ;
	for ( i = 0 ; i < n_layer ; i++ ){
		if ( i == 0 || ( flag & MT_FLAG_MOVE_EX ) ){
			/* 第１レイヤのモーションもしくはMT_FLAG_MOVE_EXフラグがついているときのみ有効 */
			m_ctrl->mt3_ctrl[i].move = (MT3_MOVE_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_MOVE_SEGMENT);
			m_ctrl->mt3_ctrl[i].move2 = (MT3_MOVE_SEGMENT2*)((char*)mem + size) ; size += sizeof(MT3_MOVE_SEGMENT2);
			m_ctrl->mt3_ctrl[i].root = (MT3_ROOT_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_ROOT_SEGMENT);
			m_ctrl->mt3_ctrl[i].fix = (MT3_FIX_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_FIX_SEGMENT);
			m_ctrl->mt3_ctrl[i].turn = (MT3_TURN_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_TURN_SEGMENT);
		}
		m_ctrl->mt3_ctrl[i].motion_arc = m_ctrl->motion_arc ;
		m_ctrl->mt3_ctrl[i].joints = (MT3_JOINT_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_JOINT_SEGMENT) * n_joints ;
		m_ctrl->mt3_ctrl[i].rots = m_ctrl->rots ;
		m_ctrl->mt3_ctrl[i].motion_num = -1 ;
		m_ctrl->mt3_ctrl[i].play_time_base = (float)TIME_BASE ;
		if ( flag & MT_FLAG_TRANS ){
			/* 移動量展開ワークの設定 */
			m_ctrl->mt3_ctrl[i].trans = (MT3_TRANS_SEGMENT*)((char*)mem + size) ; size += sizeof(MT3_TRANS_SEGMENT) * n_joints ;
		}
	}

	if ( objs != NULL ){/* ＳＥシーケンス設定 */
		m_ctrl->sar_ctrl = MT_InitSequence( n_layer, motion, 0 );
		m_ctrl->se_table_id = -1 ;
		if ( m_ctrl->sar_ctrl != NULL ){
			m_ctrl->sar_ctrl->param1 = (int)objs ;
		}
	}

	return (m_ctrl);
}

	/*
		モーション再生処理を行う
	*/
void MT_ActMotion( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, DG_EVMOBJ *evmobj )
{
	int			i, move_layer ;
	MT3_CONTROL	*mt3_ctrl ;
	long64		mask, mask2, merge_flag ;
	long64		interp_flag ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
#ifdef PSX2	/// いらない
	int			time ;
	
	GV_SET_PRFC_CLOCK();
#endif	///

	if ( m_ctrl == NULL ) return ;

	move_layer = 0 ;
	if ( m_ctrl->flag & MT_FLAG_MOVE_EX ){
		move_layer = m_ctrl->use_move_layer ;
	}

	/* 先に回転情報を取得する */
	MT_StartMotionDecodeSupport();
	mask = I64(0xffffffffffffffff) ;
	merge_flag = I64(0x0000000000000000) ;
	mt3_ctrl = &m_ctrl->mt3_ctrl[ m_ctrl->n_layer - 1 ] ;
	MT_WaitMotionDecodeSupport();
	for ( i = m_ctrl->n_layer ; i > 0 ; mt3_ctrl--, i-- ){
		if ( !( mt3_ctrl->flag & MT3_ACTIVE ) ) continue ;
		WriteJointData( mt3_ctrl, mask & mt3_ctrl->mask );
		merge_flag |= ( mask & mt3_ctrl->mask ) & mt3_ctrl->merge_flag ;
		mask &= ~(mt3_ctrl->mask) ;
	}

	/* 移動量の初期化（vwには高さが入っているのでクリアしていない） */
	m_ctrl->mt3_ctrl[move_layer].move->step.vx = 0.0f ;
	m_ctrl->mt3_ctrl[move_layer].move->step.vy = 0.0f ;
	m_ctrl->mt3_ctrl[move_layer].move->step.vz = 0.0f ;

	/* モーションを進ませる */
	mt3_ctrl = m_ctrl->mt3_ctrl ;
	for ( i = m_ctrl->n_layer ; i > 0 ; i-- ){
		PlayMotion( mt3_ctrl );
		mt3_ctrl++ ;
	}

	if ( m_ctrl->flag & MT_FLAG_TRANS ){
		/* 関節移動量を取り出す */
		mask = I64(0xffffffffffffffff) ;
		merge_flag = I64(0x0000000000000000) ;
		mt3_ctrl = &m_ctrl->mt3_ctrl[ m_ctrl->n_layer - 1 ] ;
		for ( i = m_ctrl->n_layer ; i > 0 ; mt3_ctrl--, i-- ){
			if ( !( mt3_ctrl->flag & MT3_ACTIVE ) ) continue ;
			WriteTransData( mt3_ctrl, mask & mt3_ctrl->mask );
			merge_flag |= ( mask & mt3_ctrl->mask ) & mt3_ctrl->merge_flag ;
			mask &= ~(mt3_ctrl->mask) ;
		}
	}

	/* 絶対回転クォータニオンへ変換 */
	if ( evmobj != NULL )
   {
		FVECTOR	*joints, *adjust ;
		EVM_SKEL		*skel ;
     
      //BP_ANIM - fix incorrect loop amount (it was going too far and reading off array).
      // This deals with memory access issues when the skeleton and animation joint count don't quite match up.
      // This fixes KDEUS-336.
      int processJointCount = (evmobj->n_skeleton < m_ctrl->n_joints) ? evmobj->n_skeleton : m_ctrl->n_joints;

		joints = scrpad->joints ;
		skel = evmobj->def->skeleton ;
		scrpad->root = ZeroFQuat ;
		adjust = m_ctrl->adjust ;
		mask = merge_flag ;
		mask2 = m_ctrl->adjust_flag ;
		/* モデル情報から親子関係を取得して求める */

      for ( i = processJointCount ; i > 0 ; i-- )
      {
			/* 相対絶対変換 */
			if ( !( mask & 0x01 ) ){
				MT_QuatMulFast( joints, &scrpad->joints[ skel->parent ], joints );
			}
			/* 関節アジャスト */
			if ( mask2 & 0x01 ){
				if ( !( mask & 0x01 ) ) MT_QuatNormalize( joints, joints );
				MT_QuatMulFast( joints, adjust, joints );
			}
			skel++ ;
			joints++ ;
			adjust++ ;
			mask >>= 1 ;
			mask2 >>= 1 ;
		}
	} 
   else if ( objs != NULL )
   {
		FVECTOR	*joints, *adjust ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj ;

		joints = scrpad->joints ;
		def = objs->def ;
		obj = objs->objs ;
		scrpad->root = ZeroFQuat ;
		adjust = m_ctrl->adjust ;
		mask = merge_flag ;
		mask2 = m_ctrl->adjust_flag ;
		/* モデル情報から親子関係を取得して求める */
		for ( i = def->n_models ; i > 0 ; i-- )
      {
			mdl = obj->model ;
			/* 相対絶対変換 */
			if ( !( mask & 0x01 ) ){
				MT_QuatMulFast( joints, &scrpad->joints[ mdl->parent ], joints );
			}
			/* 関節アジャスト */
			if ( mask2 & 0x01 ){
				if ( !( mask & 0x01 ) ) MT_QuatNormalize( joints, joints );
				MT_QuatMulFast( joints, adjust, joints );
			}
			obj++ ;
			joints++ ;
			adjust++ ;
			mask >>= 1 ;
			mask2 >>= 1 ;
		}
	}

	/* モーションの左右対称化処理 */
	MT_ReversalMotion( scrpad->joints, m_ctrl->flag );
	/* 平行移動量の反転処理 */
	if ( m_ctrl->flag & MT_FLAG_REVERSAL2 ){
		m_ctrl->mt3_ctrl[move_layer].move->step.vx = -m_ctrl->mt3_ctrl[move_layer].move->step.vx ;
	}


#ifndef KP_WINDOWS
	{/* 補間処理 */
		FVECTOR	*old_abs, *abs ;

		mask = m_ctrl->interp_flag ;
		abs = scrpad->joints ;
		old_abs = m_ctrl->old_abs_rots ;
		for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
			if ( mask & 0x01 ){
				MT_QuatNormalize( abs, abs );
#if 1 //BP_PS2
//#ifndef PSX2
				MT_QuatSlerp( abs, old_abs, abs, m_ctrl->interp_time );
#else
				QuatSlerp( abs, old_abs, abs, m_ctrl->interp_time );
#endif
				//MT_QuatNormalize( abs, abs );
			}
			mask >>= 1 ;
			abs++ ;
			old_abs++ ;
		}
	}
	/* 関節移動量の補間処理 */
	if ( m_ctrl->flag & MT_FLAG_TRANS ){
		FVECTOR	*old_trans, *trans ;

		mask = m_ctrl->interp_flag ;
		trans = scrpad->trans ;
		old_trans = m_ctrl->old_trans ;
		for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
			if ( mask & 0x01 ){
				//_sceVu0InterVector( trans, old_trans, trans, m_ctrl->interp_time );
				_sceVu0InterVector( trans, trans, old_trans, m_ctrl->interp_time );
			}
			mask >>= 1 ;
			trans++ ;
			old_trans++ ;
		}
	}
#else
	interp_flag = m_ctrl->interp_flag ;
	if( interp_flag )
	{
		/* 補間処理 */
		FVECTOR	*old_abs, *abs ;

		mask = interp_flag ;
		abs = scrpad->joints ;
		old_abs = m_ctrl->old_abs_rots ;
		for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
			if ( mask & 0x01 ){
				MT_QuatNormalize( abs, abs );
#if 1 //BP_PS2
//#ifndef PSX2
				MT_QuatSlerp( abs, old_abs, abs, m_ctrl->interp_time );
#else
				QuatSlerp( abs, old_abs, abs, m_ctrl->interp_time );
#endif
				//MT_QuatNormalize( abs, abs );
			}
			mask >>= 1 ;
			abs++ ;
			old_abs++ ;
		}

		/* 関節移動量の補間処理 */
		if ( m_ctrl->flag & MT_FLAG_TRANS ){
			FVECTOR	*old_trans, *trans ;

			mask = interp_flag ;
			trans = scrpad->trans ;
			old_trans = m_ctrl->old_trans ;
			for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
				if ( mask & 0x01 ){
					//_sceVu0InterVector( trans, old_trans, trans, m_ctrl->interp_time );
					_sceVu0InterVector( trans, trans, old_trans, m_ctrl->interp_time );
				}
				mask >>= 1 ;
				trans++ ;
				old_trans++ ;
			}
		}
	}
#endif

	/* ステップの計算処理 */
	if ( objs != NULL ){
		if ( objs->root != NULL )	scrpad->root_mat = *( objs->root );
		else						scrpad->root_mat = objs->world ;
	} else {
		scrpad->root_mat = evmobj->world ;
	}
	scrpad->vec.vx = m_ctrl->mt3_ctrl[move_layer].move->step.vx ;
	scrpad->vec.vy = 0.0f ;
	scrpad->vec.vz = m_ctrl->mt3_ctrl[move_layer].move->step.vz ;
	scrpad->vec.vw = 0.0F ;
	_sceVu0ApplyMatrix( &m_ctrl->step, &scrpad->root_mat, &scrpad->vec );
	m_ctrl->step.vw = m_ctrl->mt3_ctrl[move_layer].move->step.vy ;
	scrpad->vec.vx = m_ctrl->mt3_ctrl[move_layer].root->step.vx ;
	scrpad->vec.vy = 0.0f ;
	scrpad->vec.vz = m_ctrl->mt3_ctrl[move_layer].root->step.vz ;
	scrpad->vec.vw = 0.0F ;
	_sceVu0ApplyMatrix( &m_ctrl->root_step, &scrpad->root_mat, &scrpad->vec );
	m_ctrl->root_step.vw = m_ctrl->mt3_ctrl[move_layer].root->step.vy ;
	/* 高さの取得 */
	if ( !( m_ctrl->interp_flag & 0x01 ) ){
		m_ctrl->height = m_ctrl->mt3_ctrl[move_layer].move->step.vw ;
		m_ctrl->root_height = m_ctrl->mt3_ctrl[move_layer].root->step.vw ;
	} else {
		m_ctrl->height = 
		  ( m_ctrl->mt3_ctrl[move_layer].move->step.vw - m_ctrl->old_height ) * m_ctrl->interp_time
			+ m_ctrl->old_height ;
		m_ctrl->root_height = 
		  ( m_ctrl->mt3_ctrl[move_layer].root->step.vw - m_ctrl->root_old_height ) * m_ctrl->interp_time
			+ m_ctrl->root_old_height ;
	}

	/* 補間処理 */
	if ( m_ctrl->interp_count ){
		m_ctrl->interp_time += m_ctrl->interp_inc ;
		if ( ( m_ctrl->interp_count -= TIME_BASE ) <= 0 ){
			m_ctrl->interp_count = 0 ;
			m_ctrl->interp_flag = 0 ;
		}
	}

	{/* クォータニオンのコピー */
		FVECTOR	*rots ;
		rots = scrpad->joints ;
		for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
			MT_QuatNormalize( rots, rots );
			rots++ ;
		}
		/* ワークに書き戻す */
		MT_StartSprToMem( m_ctrl->abs_rots, scrpad->joints, m_ctrl->n_joints );
		MT_WaitSprToMem();
	}
	if ( m_ctrl->flag & MT_FLAG_TRANS ){
		/* ワークに書き戻す */
		MT_StartSprToMem( m_ctrl->trans, scrpad->trans, m_ctrl->n_joints );
		MT_WaitSprToMem();
	}

	/* 関節固定補正値計算 */
	if ( m_ctrl->mt3_ctrl->fix != NULL ){
		MT3_FIX_SEGMENT	*fix ;
		int		child, parent ;
		fix =  m_ctrl->mt3_ctrl->fix ;
		if ( ( child = fix->fix_joint ) != -1 ){
			DG_MDL	*mdl ;
			FMATRIX	mat ;
			FVECTOR	vec, total = DG_ZeroVector ;

			total = DG_ZeroVector ;
			mdl = objs->def->models ;
			while ( child != 0 ){
				parent = mdl[child].parent ;
				MT_QuatToMat( &mat, &scrpad->joints[parent] );
				vec.vx = mdl[child].tx ;
				vec.vy = mdl[child].ty ;
				vec.vz = mdl[child].tz ;
            vec.vw = 1.0f;
				_sceVu0ApplyMatrix( &vec, &mat, &vec );
				_sceVu0AddVector( &total, &total, &vec );
				child = parent ;
			}
			_sceVu0ApplyMatrix( &total, &scrpad->root_mat, &total );
			total.vx += m_ctrl->step.vx;
			total.vz += m_ctrl->step.vz;
			if ( fix->flag ){
				fix->fix_pos = total ;
				fix->flag = 0 ;
			}
			_sceVu0SubVector( &vec, &fix->fix_pos, &total );
			m_ctrl->step.vx = vec.vx ;
			m_ctrl->step.vz = vec.vz ;
		}
	}

#ifdef PSX2	/// いらない
	time = GV_GET_PRFC_CLOCK();
	//printf("motion setup time: %d\n", time );
#endif	
}

	/*
		モーション再生処理を行う
	*/
void MT_ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;

	if ( m_ctrl == NULL ) return ;

	if ( objs->root != NULL )	scrpad->root_mat = *( objs->root );
	else						scrpad->root_mat = objs->world ;

	MT_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	MT_WaitMemToSpr();

	if ( !( m_ctrl->flag & MT_FLAG_TRANS ) ){/* オブジェクトにマトリクスを設定する */
		/* 関節移動量なし */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				//scrpad->vec.vx = mdl->tx ;
				//scrpad->vec.vy = mdl->ty ;
				//scrpad->vec.vz = mdl->tz ;
				scrpad->vec = obj->trans ;
				parent = &scrpad->mats[ mdl->parent ] ;
				_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
			}
		}
	} else {
		/* 関節移動量付き */
		FVECTOR			*joints = scrpad->joints ;
		FVECTOR			*trans = scrpad->trans ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

#ifdef PSX2
		UTL_StartMemToSpr( scrpad->trans, m_ctrl->trans, m_ctrl->n_joints );
		UTL_EndMemToSpr();
#else		
		MT_StartMemToSpr( scrpad->trans, m_ctrl->trans, m_ctrl->n_joints );
		MT_WaitMemToSpr();
#endif		

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				//scrpad->vec.vx = mdl->tx + trans->vx ;
				//scrpad->vec.vy = mdl->ty + trans->vy ;
				//scrpad->vec.vz = mdl->tz + trans->vz ;
				_sceVu0AddVector( &scrpad->vec, &obj->trans, trans );
				scrpad->vec.vw = 1.0f ;
				parent = &scrpad->mats[ mdl->parent ] ;
				_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
				trans++ ;
			}
		}
	}

	/* ＳＥシーケンス設定 */
	//MT_ActSequence( m_ctrl->sar_ctrl );
	MT_ActSequenceSEV( m_ctrl->sar_ctrl, m_ctrl, objs, NULL );

}

void MT_EvmActMotion( MOTION_CONTROL *m_ctrl, DG_EVMOBJ *evmobj )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;

	if ( m_ctrl == NULL ) return ;

	if ( evmobj->root != NULL )	scrpad->root_mat = *( evmobj->root );
	else						scrpad->root_mat = evmobj->world ;
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

#ifdef PSX2
	UTL_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	UTL_EndMemToSpr();
#else
	MT_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	MT_WaitMemToSpr();
#endif	

	if ( !( m_ctrl->flag & MT_FLAG_TRANS ) ){/* オブジェクトにマトリクスを設定する */
		/* 関節移動量なし */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
				FMATRIX	*parent ;
				if ( i < m_ctrl->n_joints ){
					MT_QuatToMat( mats, joints );
					_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
					scrpad->vec.vx = skel->rt_tx + 0.0f ;
					scrpad->vec.vy = skel->rt_ty + 0.0f ;
					scrpad->vec.vz = skel->rt_tz + 0.0f ;
					parent = &scrpad->mats[ skel->parent ] ;
					_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
					scrpad->vec.vx = -skel->rt_tx ;
					scrpad->vec.vy = -skel->rt_ty ;
					scrpad->vec.vz = -skel->rt_tz ;
					_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
					*skel_mats = *mats ;
				} else {
					*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
				}
				skel++ ;
				mats++ ;
				skel_mats++ ;
				joints++ ;
			}
		}
	} else {
		/* 関節移動量付き */
		FVECTOR			*joints = scrpad->joints ;
		FVECTOR			*trans = scrpad->trans ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

#ifdef PSX2		
		UTL_StartMemToSpr( scrpad->trans, m_ctrl->trans, m_ctrl->n_joints );
		UTL_EndMemToSpr();
#else
		MT_StartMemToSpr( scrpad->trans, m_ctrl->trans, m_ctrl->n_joints );
		MT_WaitMemToSpr();
#endif		
		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
				FMATRIX	*parent ;
				if ( i < m_ctrl->n_joints ){
					MT_QuatToMat( mats, joints );
					_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
					scrpad->vec.vx = skel->rt_tx + trans->vx ;
					scrpad->vec.vy = skel->rt_ty + trans->vy ;
					scrpad->vec.vz = skel->rt_tz + trans->vz ;
					parent = &scrpad->mats[ skel->parent ] ;
					_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
					scrpad->vec.vx = -skel->rt_tx ;
					scrpad->vec.vy = -skel->rt_ty ;
					scrpad->vec.vz = -skel->rt_tz ;
					_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
					*skel_mats = *mats ;
				} else {
					*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
				}
				skel++ ;
				mats++ ;
				skel_mats++ ;
				joints++ ;
				trans++ ;
			}
		}
	}

}

	/*
		モーション用ワークを開放する
	*/
void MT_FreeMotion( MOTION_CONTROL *m_ctrl )
{
	if ( m_ctrl == NULL ) return ;

	/* ＳＥシーケンス設定 */
	MT_FreeSequence( m_ctrl->sar_ctrl );

	GV_Free( m_ctrl );
} 

/*----------------------------------------------------------------*/

	/*
		モーションの設定
	*/
void MT_SetMotionData( MOTION_CONTROL *m_ctrl, int layer, int motion, int time, u_long64 mask )
{
	MT3_FILE_HEADER	*mt3, *old_mt3 ;
	MT3_CONTROL		*mt3_ctrl ;

	if ( m_ctrl == NULL ) return ;

	mt3_ctrl = &m_ctrl->mt3_ctrl[ layer ] ;
	if ( m_ctrl->motion_arc == NULL ){
		mt3 = GV_GetCache( GV_CacheID( motion, 'm' ) );
	} else {
		if ( motion >= m_ctrl->motion_arc->n_motion ) motion = 0 ;
		mt3 = m_ctrl->motion_arc->data_table[ motion ].addr ;
	}
	old_mt3 = mt3_ctrl->file_header ;

	/* 次モーションセット */
	SetMotion( mt3_ctrl, mt3, time );
	/* モーション切り替え前処理 */
	/* 本当ならそのとき腰関節が有効になっているレイヤかどうかを調べるべきであるが、
	   基本的に腰関節＝０番レイヤになるようにしているためこのチェックを簡略化している */
	if ( layer == 0 && old_mt3 != NULL ){
		m_ctrl->rot_correct = 0 ;
		if ( old_mt3->flag & MT3_FLAG_TURN_FLAG ){
			/* 前回のモーションがターンフラグ付きなら現在のモーションから回転補正値を計算する */
			FVECTOR		x, y, res, quat ;
			int		i ;
			FMATRIX		mat ;
			float		angle ;
#define _ABSf(x) ((x>=0.0f)?(x):(-(x)))

			/* 前モーションの最終角度を求める */
			MT_QuatToMat( &mat, m_ctrl->abs_rots );
			if ( _ABSf(mat.m[0][1]) > _ABSf(mat.m[2][1]) ){
				y.vx = mat.m[2][0] ;
				x.vx = mat.m[2][2] ;
			} else {
				x.vx = mat.m[0][0] ;
				y.vx = -mat.m[0][2] ;
			}
			/* 次モーションの開始角度を求める */
			MT_QuatToMat( &mat, &mt3_ctrl->joints->next );
			if ( _ABSf(mat.m[0][1]) > _ABSf(mat.m[2][1]) ){
				y.vy = mat.m[2][0] ;
				x.vy = mat.m[2][2] ;
			} else {
				x.vy = mat.m[0][0] ;
				y.vy = -mat.m[0][2] ;
			}
			/* 同時に角度を求め、合計しておく */
#if 0 //BP_PS2
//#ifdef PSX2			
			MT_Atan2X4( &res, &y, &x );
#else
			res.vx = atan2f(y.x, x.x);
			res.vy = atan2f(y.y, x.y);
#endif			
			angle = res.vx - res.vy ;
			/* 補正計算用クォータニオンを求める */
			quat.vw = cosf( angle / 2 );
			quat.vx = 0 ; 
			quat.vz = 0 ; 
			if ( angle > 0 ) quat.vy = -bp_sqrtf( 1.0f - quat.vw * quat.vw ) ;   //BP_MATH - emulate PS2 sqrtf
			else quat.vy = bp_sqrtf( 1.0f - quat.vw * quat.vw ) ;    //BP_MATH - emulate PS2 sqrtf
			/* モーション補正及び補正値の格納 */
			for ( i = 0 ; i < m_ctrl->n_joints ; i++ ) MT_QuatMul( &m_ctrl->abs_rots[i], &quat, &m_ctrl->abs_rots[i] );
			m_ctrl->rot_correct = (int)(angle * 2048 / (float)M_PI) ;
		}
	}

	if ( layer == 0 ) mask = I64(0xffffffffffffffff) ;
	mt3_ctrl->motion_num = motion ;
	mt3_ctrl->mask = mask ;
	mt3_ctrl->flag |= MT3_ACTIVE ;
	if ( mask & 1 ){
		m_ctrl->old_height = m_ctrl->height ;
		m_ctrl->root_old_height = m_ctrl->root_height ;
	}

	/* 特殊モーションフラグ処理 */
	switch ( mt3->flag & MT3_FLAG_TYPE_MASK ){
	  case 0:
		mt3_ctrl->merge_flag = 0 ;
		break ;
	  case MT3_FLAG_TYPE_01:
		/* 上半身下半身分離型モーションデータ */
		mt3_ctrl->merge_flag = 0x00000002 ;
		break ;
	}

	/* ＳＥシーケンス設定 */
	MT_PlaySequence( m_ctrl->sar_ctrl, layer, motion, SAR_FLAG_PLAY|SAR_FLAG_LOOP,
					mt3->motion_base_tick * mt3->motion_length );
}

	/*
		モーションの停止
	*/
void MT_ResetMotionData( MOTION_CONTROL *m_ctrl, int layer )
{
	u_long64	old_mask ;

	if ( m_ctrl == NULL ) return ;

	old_mask = m_ctrl->mt3_ctrl[ layer ].mask ;
	m_ctrl->mt3_ctrl[ layer ].motion_num = -1 ;
	m_ctrl->mt3_ctrl[ layer ].mask = 0 ;
	m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;

	/* ＳＥシーケンス設定 */
	MT_StopSequence( m_ctrl->sar_ctrl, layer );

}

	/*
		モーション補間の設定
	*/
void MT_SetMotionInterp( MOTION_CONTROL *m_ctrl, int time, u_long64 flag )
{
	int		i ;
	u_long128	*dst, *src ;

	if ( m_ctrl == NULL ) return ;

	/* 現在のクォータニオンをコピー */
	src = (u_long128*)m_ctrl->abs_rots ;
	dst = (u_long128*)m_ctrl->old_abs_rots ;
	for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
		*dst++ = *src++ ;
	}
	if ( m_ctrl->flag & MT_FLAG_TRANS ){
		/* 現在の移動量をコピー */
		src = (u_long128*)m_ctrl->trans ;
		dst = (u_long128*)m_ctrl->old_trans ;
		for ( i = m_ctrl->n_joints ; i > 0 ; i-- ){
			*dst++ = *src++ ;
		}
	}
	/* 補間用パラメータを設定 */
	m_ctrl->interp_count = time ;
	m_ctrl->interp_time = 0.0F ;
	m_ctrl->interp_inc = (float)TIME_BASE / (float)time ;

	m_ctrl->interp_flag |= flag ;
}

	/*
		モーションに同期して再生される効果音の変換テーブルの情報をセットする
	*/
void MT_SetMotionSeTable( MOTION_CONTROL *m_ctrl, int map_name, int table_id, int hazard_type, int segment_type )
{
	if ( m_ctrl == NULL ) return ;

	m_ctrl->map_name = map_name ;
	m_ctrl->se_table_id = table_id ;
	m_ctrl->se_hazard_type = hazard_type ;
	m_ctrl->se_segment_type = segment_type ;
}

	/*
		モーションの再生スピードを変更する（負ならデフォルト値に戻す）
	*/
void MT_SetMotionSpeed( MOTION_CONTROL *m_ctrl, float time )
{
	int		i ;
	if ( m_ctrl == NULL ) return ;
	if ( time < 0.0f ) time = (float)TIME_BASE ;
	for ( i = 0 ; i < m_ctrl->n_layer ; i++ ){
		m_ctrl->mt3_ctrl[ i ].play_time_base = time ;
	}
	
	/* ＳＥシーケンスの再生スピード変更 */
	MT_SetSequenceSpeed( m_ctrl->sar_ctrl, time );
}


	/*
		モーションの初期高さ取得
	*/
float MT_GetMotionStartHeight( MOTION_CONTROL *m_ctrl, int motion )
{
	MT3_FILE_HEADER	*mt3 ;
	unsigned short		*data ;
	float				height ;

	if ( m_ctrl == NULL ) return 0.f;
	if ( motion >= m_ctrl->motion_arc->n_motion ) motion = 0 ;
	mt3 = m_ctrl->motion_arc->data_table[ motion ].addr ;

	data = (unsigned short*)mt3->archives_data + mt3->move_offset ;

	height = MFtoF( data[1] );

	return ( height );
}

/*----------------------------------------------------------------*/

/* ========= モーション再生 ========= */
/* ｍｔ３モーションデータを再生ワークにセットする */
static void SetMotion( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *motion_data, int time )
{
	mt3_ctrl->file_header = motion_data ;
	mt3_ctrl->data_ptr = motion_data->archives_data ;
	//mt3_ctrl->data_ptr = (unsigned short*)( (int)motion_data + (int)motion_data->archives_data );
	mt3_ctrl->motion_time_base = (float)motion_data->motion_base_tick ;
	mt3_ctrl->motion_total_time = motion_data->motion_length * mt3_ctrl->motion_time_base ;
	mt3_ctrl->flag = 0 ;
	mt3_ctrl->time = (int)(time / mt3_ctrl->motion_time_base) ;
	mt3_ctrl->loop = 0 ;
	mt3_ctrl->last_check_time = time ;
	/* モーション再生時間基準のパラメータ設定 */
	mt3_ctrl->motion_total_time_inv = 1.0f / mt3_ctrl->motion_total_time ;
	mt3_ctrl->play_time = mt3_ctrl->motion_time_base + time ;
	/* 移動データ処理ワークの初期化 */
	if ( mt3_ctrl->move != NULL && motion_data->flag & MT3_FLAG_MOVE_PART ){
		InitMoveControlWork( mt3_ctrl, motion_data );
	}
	if ( mt3_ctrl->move2 != NULL && motion_data->flag & MT3_FLAG_MOVE2_PART ){
		InitMoveControlWork2( mt3_ctrl, motion_data );
	}
	/* 移動軌跡データ処理ワークの初期化 */
	if ( mt3_ctrl->root != NULL && motion_data->flag & MT3_FLAG_ROOT_PART ){
		InitRootControlWork( mt3_ctrl, motion_data );
	}
	/* 軸固定データ処理ワークの初期化 */
	if ( mt3_ctrl->fix != NULL ){
		if ( motion_data->flag & MT3_FLAG_FIX_PART ){
			InitFixControlWork( mt3_ctrl, motion_data );
		} else {
			mt3_ctrl->fix->fix_joint = -1 ;
		}
	}
	/* 関節回転データ処理ワークの初期化 */
	if ( mt3_ctrl->joints != NULL && motion_data->flag & MT3_FLAG_ROT_PART ){
		InitJointControlWork( mt3_ctrl, motion_data );
	}
	/* 関節移動量データ処理ワークの初期化 */
	if ( mt3_ctrl->trans != NULL ){
		if ( motion_data->flag & MT3_FLAG_TRANS_PART ){
			InitTransControlWork( mt3_ctrl, motion_data );
		} else {
			/* モーション自体に移動量データがない場合 */
			InitTransControlWork( mt3_ctrl, NULL );
		}
	}

	/* 指定再生時間までモーションを進める */
	if ( time != 0 ){
		int		flag ;
		float	base_backup ;
		flag = mt3_ctrl->file_header->flag ;
		base_backup = mt3_ctrl->play_time_base ;
		mt3_ctrl->play_time_base = (float)time ;
		/* 移動データ処理 */
		if ( mt3_ctrl->move != NULL && flag & MT3_FLAG_MOVE_PART ){
			PlayMoveData( mt3_ctrl, 0 );
		}
		/* 移動データ処理 */
		if ( mt3_ctrl->move2 != NULL && flag & MT3_FLAG_MOVE2_PART ){
			if ( mt3_ctrl->play_time_base > base_backup ){
				/* 差分が必要なので最終フレームから１フレーム分戻して改めて１フレームのみ再生処理を行う */
				mt3_ctrl->play_time_base -= base_backup ;
				PlayMoveData2( mt3_ctrl, 0 );
				mt3_ctrl->play_time_base = base_backup ;
				PlayMoveData2( mt3_ctrl, 0 );
				mt3_ctrl->play_time_base = (float)time ;
			} else {
				PlayMoveData2( mt3_ctrl, 0 );
			}
		}
		/* 移動軌跡データ処理 */
		if ( mt3_ctrl->root != NULL && flag & MT3_FLAG_ROOT_PART ){
			if ( mt3_ctrl->play_time_base > base_backup ){
				/* 差分が必要なので最終フレームから１フレーム分戻して改めて１フレームのみ再生処理を行う */
				mt3_ctrl->play_time_base -= base_backup ;
				PlayRootData( mt3_ctrl, 0 );
				mt3_ctrl->play_time_base = base_backup ;
				PlayRootData( mt3_ctrl, 0 );
				mt3_ctrl->play_time_base = (float)time ;
			} else {
				PlayRootData( mt3_ctrl, 0 );
			}
		}
		/* 軸固定データ処理 */
		if ( mt3_ctrl->fix != NULL && flag & MT3_FLAG_FIX_PART ){
			PlayFixData( mt3_ctrl, 0 );
		}
		/* 関節回転データ処理 */
		if ( mt3_ctrl->joints != NULL && flag & MT3_FLAG_ROT_PART ){
			PlayJointData( mt3_ctrl, 0 );
		}
		/* 関節移動量データ処理 */
		if ( mt3_ctrl->trans != NULL /* && flag & MT3_FLAG_TRANS_PART */ ){
			PlayTransData( mt3_ctrl, 0 );
		}
		mt3_ctrl->play_time_base = base_backup ;
	}
}

/* モーション再生処理 */
static void PlayMotion( MT3_CONTROL *mt3_ctrl )
{
	int		end_flag = 0, flag ;
	float	motion_length ;

	if ( !( mt3_ctrl->flag & MT3_ACTIVE ) ) return ;

	/* モーション停止チェック */
	if ( mt3_ctrl->flag & MT3_SLEEP ) return ;

	/* 再生終了チェック */
#if 0
	mt3_ctrl->time++ ;
	mt3_ctrl->flag &= ~( MT3_PLAYEND | MT3_PLAYLAST1 ) ;
	if ( mt3_ctrl->time == mt3_ctrl->file_header->motion_length ){
		/* 終了フレームならフラグを立てる */
		end_flag = 1 ;
		mt3_ctrl->flag |= MT3_PLAYEND ;
		mt3_ctrl->time = 0 ;
		mt3_ctrl->play_tick = 0 ;
		mt3_ctrl->loop++ ;
		/* １フレームモーションチェック */
		if ( mt3_ctrl->file_header->motion_length == 1 ) mt3_ctrl->flag |= MT3_SLEEP ;
	} else if ( mt3_ctrl->time == ( mt3_ctrl->file_header->motion_length - 1 ) ){
		/* 終了１フレーム前 */
		mt3_ctrl->flag |= MT3_PLAYLAST1 ;
	}
#else
	mt3_ctrl->time++ ;	/* old */
	mt3_ctrl->flag &= ~( MT3_PLAYEND | MT3_PLAYLAST1 ) ;
	mt3_ctrl->play_time += mt3_ctrl->play_time_base ;
	motion_length = (float)mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick ;
	if ( mt3_ctrl->play_time > motion_length ){
		/* 終了フレームならフラグを立てる */
		end_flag = 1 ;
		mt3_ctrl->flag |= MT3_PLAYEND ;
		mt3_ctrl->time = 0 ;	/* old */
		mt3_ctrl->last_check_time = 0 ;
		mt3_ctrl->play_time -= motion_length ;
		mt3_ctrl->loop++ ;
		/* １フレームモーションチェック */
		if ( mt3_ctrl->file_header->motion_length == 1 ) mt3_ctrl->flag |= MT3_SLEEP ;
	} else if ( mt3_ctrl->play_time > ( motion_length - mt3_ctrl->play_time_base ) ){
		/* 終了１フレーム前 */
		mt3_ctrl->flag |= MT3_PLAYLAST1 ;
	}
#endif

	flag = mt3_ctrl->file_header->flag ;
	/* 移動データ処理 */
	if ( mt3_ctrl->move != NULL && flag & MT3_FLAG_MOVE_PART ){
		PlayMoveData( mt3_ctrl, end_flag );
	}
	/* 移動データ処理 */
	if ( mt3_ctrl->move2 != NULL && flag & MT3_FLAG_MOVE2_PART ){
		PlayMoveData2( mt3_ctrl, end_flag );
	}
	/* 移動軌跡データ処理 */
	if ( mt3_ctrl->root != NULL && flag & MT3_FLAG_ROOT_PART ){
		PlayRootData( mt3_ctrl, end_flag );
	} else {
		if ( mt3_ctrl->root != NULL ){
			mt3_ctrl->root->step = mt3_ctrl->move->step ;
		}
	}
	/* 軸固定データ処理 */
	if ( mt3_ctrl->fix != NULL && flag & MT3_FLAG_FIX_PART ){
		PlayFixData( mt3_ctrl, end_flag );
	} else {
		if ( mt3_ctrl->fix != NULL ){
			mt3_ctrl->fix->fix_joint = -1 ;
		}
	}
	/* 関節回転データ処理 */
	if ( mt3_ctrl->joints != NULL && flag & MT3_FLAG_ROT_PART ){
		PlayJointData( mt3_ctrl, end_flag );
	}
	/* 関節移動量データ処理 */
	if ( mt3_ctrl->trans != NULL && flag & MT3_FLAG_TRANS_PART ){
		PlayTransData( mt3_ctrl, 0 );
	}
}

/* ========= 内部使用関数群 ========= */
/* 移動データ展開ワーク初期化 */
static void InitMoveControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_MOVE_SEGMENT	*move ;
	unsigned short		*data, tmp ;
	int					bit_pos ;

	move = mt3_ctrl->move ;
	move->data = data = mt3_ctrl->data_ptr + file_header->move_offset ;
	move->bit_pos = bit_pos = 0 ;

	/* Ｙの平均値保存 */
	move->ave_height = (float)PopDataBitExt( data, bit_pos, 16 );
	ADJUST_POINTER( data, bit_pos, 16 );
	/* 有効ビット数の読み込み */
	tmp = PopDataBit( data, bit_pos, 16 );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->e_bit_x = (char)( tmp & 0x0f ) ; tmp >>= 4 ;
	move->e_bit_y = (char)( tmp & 0x0f ) ; tmp >>= 4 ;
	move->e_bit_z = (char)( tmp & 0x0f ) ;

	move->data = data ;
	move->bit_pos = bit_pos ;

	move->base_tick = (float)file_header->motion_base_tick ;
	move->interp_time = move->base_tick ;
	move->time = move->interp_time ;
	move->total_time = 0.0f ;

}

/* 移動軌跡データ展開ワーク初期化 */
static void InitMoveControlWork2( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_MOVE_SEGMENT2	*move ;
	unsigned short		*data ;
	int					len, bit_pos ;

	move = mt3_ctrl->move2 ;
	move->data = data = mt3_ctrl->data_ptr + file_header->move_offset ;
	move->bit_pos = bit_pos = 0 ;

	move->old_pos.vx = MFtoF( data[0] );
	move->old_pos.vy = MFtoF( data[1] );
	move->old_pos.vz = MFtoF( data[2] );
	data += 3 ;
	move->prev = move->old_pos ;

	move->base_tick = (float)file_header->motion_base_tick ;
	move->total_time = 0.0f ;
	move->time = move->base_tick ;
	move->scale = 1.0f / move->base_tick ;

	/* キーのフレーム数を取得 */
	len = PopDataBit( data, bit_pos, 8 );
	ADJUST_POINTER( data, bit_pos, 8 );
	move->interp_time = len * move->base_tick ;

	/* 補間先ベクトルを取得 */
	move->next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );
	/* 補間先ベクトルの傾きを取得 */
	move->d_next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->d_next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->d_next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
	ADJUST_POINTER( data, bit_pos, 16 );

	move->inv_time = 1.0f / move->interp_time ;

	move->data = data ;
	move->bit_pos = bit_pos ;

}


/* 移動軌跡データ展開ワーク初期化 */
static void InitRootControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_ROOT_SEGMENT	*root ;
	unsigned short		*data ;

	root = mt3_ctrl->root ;
	root->data = data = mt3_ctrl->data_ptr + file_header->root_offset ;
	root->bit_pos = 0 ;

	root->base_tick = (float)file_header->motion_base_tick ;
	root->total_time = 0.0f ;
	root->time = root->base_tick ;
	root->interp_time = (float)data[0] * root->base_tick ;
	root->next.vx = ((short*)data)[1] ;
	root->next.vy = ((short*)data)[2] ;
	root->next.vz = ((short*)data)[3] ;
	root->old_pos.vx = 0.0f ;
	root->old_pos.vy = 0.0f ;
	root->old_pos.vz = 0.0f ;
	root->inv_time = 1.0f / root->interp_time ;

	root->data = data + 4 ;
}

/* 関節固定情報管理ワーク初期化 */
static void InitFixControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_FIX_SEGMENT		*fix ;
	unsigned short		*data ;
	int					bit_pos ;

	fix = mt3_ctrl->fix ;
	fix->data = data = mt3_ctrl->data_ptr + file_header->fix_offset ;
	fix->bit_pos = bit_pos = 0 ;

	fix->base_tick = (float)file_header->motion_base_tick ;
	fix->interp_time = (float)data[0] * fix->base_tick ;
	fix->time = fix->base_tick ;
	fix->total_time = 0.0f ;
	fix->fix_joint = ((short*)data)[1] ;
	fix->flag = 1 ;
	fix->data = data + 2 ;
}

/* 関節回転データ展開ワーク初期化 */
static void InitJointControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_JOINT_SEGMENT	*joints ;
	int					i ;

	joints = mt3_ctrl->joints ;
	for ( i = 0 ; i < file_header->rot_units ; i++ ){
		joints->data = mt3_ctrl->data_ptr + file_header->rots_offset[i] ;
		joints->bit_pos = 0 ;
		joints->base_tick = (float)file_header->motion_base_tick ;
		/* 補間パラメータセット */
		SetNextQuatData( joints );
		/* 先頭のキーは用意されていないので一番最初のみ
		   モーション再生スピードに関わらず必ず補間完了ということにしてしまう */
		joints->total_time = 0.0f ;
		joints->time = joints->base_tick ;

		joints++ ;
	}

}

/* 関節移動量データ展開ワーク初期化 */
static void InitTransControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_TRANS_SEGMENT	*trans_seg ;
	unsigned short		*data ;
	int					len, bit_pos ;
	int					i ;

	trans_seg = mt3_ctrl->trans ;
	/* モーションにデータない場合 */
	if ( file_header == NULL ){
		for ( i = 0 ; i < file_header->rot_units ; i++ ){
			trans_seg->data = NULL ;
			trans_seg++ ;
		}
		return ;
	}
	for ( i = 0 ; i < file_header->rot_units ; i++ ){
		trans_seg->data = data = mt3_ctrl->data_ptr + file_header->rots_offset[ i + file_header->rot_units ] ;
		trans_seg->bit_pos = bit_pos = 0 ;
		trans_seg->base_tick = (float)file_header->motion_base_tick ;
		/* 先頭のキーは用意されていないので一番最初のみ
		   モーション再生スピードに関わらず必ず補間完了ということにしてしまう */
		trans_seg->total_time = 0.0f ;
		trans_seg->time = trans_seg->base_tick ;
		trans_seg->scale = 1.0f / trans_seg->base_tick ;

		/* キーのフレーム数を取得 */
		len = PopDataBit( data, bit_pos, 8 );
		ADJUST_POINTER( data, bit_pos, 8 );
		trans_seg->interp_time = len * trans_seg->base_tick ;

		/* 補間先ベクトルを取得 */
		trans_seg->next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		trans_seg->next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		trans_seg->next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		/* 補間先ベクトルの傾きを取得 */
		trans_seg->d_next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		trans_seg->d_next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		trans_seg->d_next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );

		trans_seg->inv_time = 1.0f / trans_seg->interp_time ;

		trans_seg->data = data ;
		trans_seg->bit_pos = bit_pos ;

		trans_seg++ ;

	}

}

/* 次フレームの移動量データを設定 */
static void PlayMoveData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_MOVE_SEGMENT	*move ;
	unsigned short		*data ;
	int					bit_pos, e_bit ;
	float				total_motion_time, play_time_base ;

	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick) ;
	play_time_base = mt3_ctrl->play_time_base ;

	move = mt3_ctrl->move ;
	data = move->data ;
	bit_pos = move->bit_pos ;

	move->step.vx = move->step.vz = 0.0f ;

	move->time += play_time_base ;
	while ( move->time > move->interp_time ){

		/* Ｘ軸移動量取得 */
		e_bit = move->e_bit_x ;
		move->step.vx += (float)PopDataBitExt( data, bit_pos, e_bit );
		ADJUST_POINTER( data, bit_pos, e_bit );

		/* 中心Ｙ座標取得 */
		e_bit = move->e_bit_y ;
		move->step.vy = (float)PopDataBitExt( data, bit_pos, e_bit ) ;
		move->step.vw = move->step.vy + move->ave_height ;
		ADJUST_POINTER( data, bit_pos, e_bit );

		/* Ｚ軸移動量取得 */
		e_bit = move->e_bit_z ;
		move->step.vz += (float)PopDataBitExt( data, bit_pos, e_bit );
		ADJUST_POINTER( data, bit_pos, e_bit );

		move->total_time += move->interp_time ;
		move->time -= move->interp_time ;
		if ( move->total_time <= total_motion_time ){
			/* モーションが最後まで達していない、もしくは変化時間が最後の補間時間よりも大きい場合 */
		} else {
			/* データ取得位置の初期化 */
			move->total_time -= total_motion_time ;
			data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->move_offset ;
			data += 2 ;		/* Ｙ平均値、各軸有効ビット数のデータは必要ないため */
			bit_pos = 0 ;
		}
	}

	move->data = data ;
	move->bit_pos = bit_pos ;
}

/* 次フレームの移動軌跡データを設定 */
static void PlayMoveData2( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_MOVE_SEGMENT2	*move ;
	unsigned short		*data ;
	int					bit_pos, len ;
	float				total_motion_time, play_time_base ;
	FVECTOR				pos ;

	move = mt3_ctrl->move2 ;
	data = move->data ;
	bit_pos = move->bit_pos ;

	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick) ;
	play_time_base = mt3_ctrl->play_time_base ;

	/* 移動量計算 */
	MT_HermiteLerpVecScale( &pos, &move->prev, &move->next, &move->d_prev, &move->d_next,
						   move->time * move->inv_time, move->interp_time * move->scale );
	move->step.vx = pos.vx - move->old_pos.vx ;
	move->step.vy = pos.vy - move->old_pos.vy ;
	move->step.vz = pos.vz - move->old_pos.vz ;
	move->step.vw = pos.vy ;
	move->old_pos = pos ;
	mt3_ctrl->move->step = move->step ;

	move->time += play_time_base ;

    while ( move->time > move->interp_time ){
		move->total_time += move->interp_time ;
		move->time -= move->interp_time ;
		if ( move->total_time  < total_motion_time ){
			/* モーションが最後まで達していない、もしくは変化時間が最後の補間時間よりも大きい場合 */
			move->d_prev = move->d_next ;
			move->prev = move->next ;
		} else {
			/* データ取得位置の初期化 */
			move->total_time -= total_motion_time ;
			data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->move_offset ;
			bit_pos = 0 ;
			/* パラメータセット */
			move->d_prev = move->d_next ;
			move->prev.vx = MFtoF( data[0] ) ;
			move->prev.vy = MFtoF( data[1] ) ;
			move->prev.vz = MFtoF( data[2] ) ;
			data += 3 ;
			move->old_pos.vx -= move->next.vx - move->prev.vx ;
			move->old_pos.vy -= move->next.vy - move->prev.vy ;
			move->old_pos.vz -= move->next.vz - move->prev.vz ;
		}

		/* キーのフレーム数を取得 */
		len = PopDataBit( data, bit_pos, 8 );
		ADJUST_POINTER( data, bit_pos, 8 );
		move->interp_time = len * move->base_tick ;
		/* 補間先ベクトルを取得 */
		move->next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		move->next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		move->next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		/* 補間先ベクトルの傾きを取得 */
		move->d_next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		move->d_next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
		move->d_next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
		ADJUST_POINTER( data, bit_pos, 16 );
#ifndef KP_WINDOWS
		move->inv_time = 1.0f / move->interp_time ;
#endif
	}
#ifdef KP_WINDOWS
	move->inv_time = 1.0f / move->interp_time ;
#endif

	move->data = data ;
	move->bit_pos = bit_pos ;
}

/* 次フレームの移動軌跡データを設定 */
static void PlayRootData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_ROOT_SEGMENT	*root ;
	unsigned short		*data ;
	float				total_motion_time, play_time_base, t ;
	FVECTOR				pos ;

	root = mt3_ctrl->root ;
	data = root->data ;

	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick) ;
	play_time_base = mt3_ctrl->play_time_base ;

	/* 移動量計算 */
	t = root->time * root->inv_time ;
	pos.vx = ( root->next.vx - root->prev.vx ) * t + root->prev.vx ;
	pos.vy = ( root->next.vy - root->prev.vy ) * t + root->prev.vy ;
	pos.vz = ( root->next.vz - root->prev.vz ) * t + root->prev.vz ;
	root->step.vx = pos.vx - root->old_pos.vx ;
	root->step.vy = pos.vy - root->old_pos.vy ;
	root->step.vz = pos.vz - root->old_pos.vz ;
	root->step.vw = pos.vy ;
	root->old_pos = pos ;

	root->time += play_time_base ;

    while ( root->time > root->interp_time ){
		root->total_time += root->interp_time ;
		root->time -= root->interp_time ;
		if ( root->total_time < total_motion_time ){
			/* モーションが最後まで達していない場合 */
			/* パラメータセット */
			root->prev = root->next ;
			root->interp_time = (float)data[0] * root->base_tick ;
			root->next.vx += ((short*)data)[1] ;
			root->next.vy = ((short*)data)[2] ;
			root->next.vz += ((short*)data)[3] ;
			data += 4 ;
		} else {
			/* データ取得位置の初期化 */
			root->total_time -= total_motion_time ;
			data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->root_offset ;
			/* パラメータセット */
			root->old_pos.vx -= root->next.vx ;
			root->old_pos.vz -= root->next.vz ;
			root->prev.vx = 0.0f ;
			root->prev.vy = root->next.vy ;
			root->prev.vz = 0.0f ;
			root->interp_time = (float)data[0] * root->base_tick ;
			root->next.vx = ((short*)data)[1] ;
			root->next.vy = ((short*)data)[2] ;
			root->next.vz = ((short*)data)[3] ;
			data += 4 ;
		}
		root->inv_time = 1.0f / root->interp_time ;
	}

	root->data = data ;
}

/* 関節固定データを更新 */
static void PlayFixData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_FIX_SEGMENT		*fix ;
	unsigned short		*data ;
	float				total_motion_time, play_time_base ;

	fix = mt3_ctrl->fix ;
	data = fix->data ;

	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick) ;
	play_time_base = mt3_ctrl->play_time_base ;

	fix->time += play_time_base ;
	while ( fix->time > fix->interp_time ){
		fix->total_time += fix->interp_time ;
		fix->time -= fix->interp_time ;
		if ( fix->total_time < total_motion_time ){
			/* 次の補間パラメータの準備 */
			fix->interp_time = (float)data[0] * fix->base_tick ;
			fix->fix_joint = ((short*)data)[1] ;
			fix->flag = 1 ;
			data += 2 ;
		} else {
			/* データ取得位置の初期化 */
			fix->total_time -= total_motion_time ;
			data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->fix_offset ;
			/* 補間パラメータセット */
			fix->interp_time = (float)data[0] * fix->base_tick ;
			fix->fix_joint = ((short*)data)[1] ;
			fix->flag = 1 ;
			data += 2 ;
		}
	}
	fix->data = data ;
}

/* 次フレームの関節移動量データを設定 */
static void PlayTransData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_TRANS_SEGMENT	*trans_seg ;
	unsigned short		*data ;
	int					bit_pos, len ;
	float				total_motion_time, play_time_base ;
	FVECTOR				pos ;
	int					i, n_joint ;

	trans_seg = mt3_ctrl->trans ;
	n_joint = mt3_ctrl->file_header->rot_units ;

	/* 移動量データがなかった場合 */
	if ( trans_seg->data == NULL ){
		for ( i = 0 ; i < n_joint ; i++, trans_seg++ ){
			trans_seg->pos.vx = 0.0f ;
			trans_seg->pos.vy = 0.0f ;
			trans_seg->pos.vz = 0.0f ;
		}
		return ;
	}

	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick );
	play_time_base = mt3_ctrl->play_time_base ;

	for ( i = 0 ; i < n_joint ; i++, trans_seg++ ){
		data = trans_seg->data ;
		bit_pos = trans_seg->bit_pos ;

		/* 移動量計算 */
		MT_HermiteLerpVecScale( &pos, &trans_seg->prev, &trans_seg->next, &trans_seg->d_prev, &trans_seg->d_next,
							   trans_seg->time * trans_seg->inv_time, trans_seg->interp_time * trans_seg->scale );
		trans_seg->pos = pos ;

		trans_seg->time += play_time_base ;

		while ( trans_seg->time > trans_seg->interp_time ){
			trans_seg->total_time += trans_seg->interp_time ;
			trans_seg->time -= trans_seg->interp_time ;
			if ( trans_seg->total_time  < total_motion_time ){
				/* モーションが最後まで達していない、もしくは変化時間が最後の補間時間よりも大きい場合 */
				trans_seg->d_prev = trans_seg->d_next ;
				trans_seg->prev = trans_seg->next ;
			} else {
				/* データ取得位置の初期化 */
				trans_seg->total_time -= total_motion_time ;
				data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->rots_offset[ i + mt3_ctrl->file_header->rot_units ] ;
				bit_pos = 0 ;
				/* パラメータセット */
				trans_seg->d_prev = trans_seg->d_next ;
				trans_seg->prev = trans_seg->next ;
			}

			/* キーのフレーム数を取得 */
			len = PopDataBit( data, bit_pos, 8 );
			ADJUST_POINTER( data, bit_pos, 8 );
			trans_seg->interp_time = len * trans_seg->base_tick ;
			/* 補間先ベクトルを取得 */
			trans_seg->next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );
			trans_seg->next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );
			trans_seg->next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );
			/* 補間先ベクトルの傾きを取得 */
			trans_seg->d_next.vx = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );
			trans_seg->d_next.vy = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );
			trans_seg->d_next.vz = MFtoF( PopDataBit( data, bit_pos, 16 ) );
			ADJUST_POINTER( data, bit_pos, 16 );

			trans_seg->inv_time = 1.0f / trans_seg->interp_time ;
		}

		trans_seg->data = data ;
		trans_seg->bit_pos = bit_pos ;
	}
}

/* 関節移動量データの書き出し */
static void WriteTransData( MT3_CONTROL *mt3_ctrl, long64 mask )
{
	MT3_TRANS_SEGMENT	*trans_seg ;
	int					i, n_joint ;
	FVECTOR				*trans ;

	if ( mask == 0 ) return ;
	/* 角度補間処理 */
	trans_seg = mt3_ctrl->trans ;
	n_joint = mt3_ctrl->file_header->rot_units ;
	trans = TRANS_VEC ;
	for ( i = 0 ; i < n_joint ; i++ ){
		if ( mask & 0x01 ){
			*trans = trans_seg->pos ;
		}
		mask >>= 1 ;
		trans_seg++ ;
		trans++ ;
	}

}

/* 関節回転データの書き出し */
static void WriteJointData( MT3_CONTROL *mt3_ctrl, long64 mask )
{
	MT3_JOINT_SEGMENT	*joints ;
	int					i, n_joint ;
	FVECTOR				*q, *rots ;
	SQUAD_WORK			*squad_work ;

	if ( mask == 0 ) return ;
	/* 角度補間処理 */
	joints = mt3_ctrl->joints ;
	n_joint = mt3_ctrl->file_header->rot_units ;
	rots = QUAT_ROTS ;
	q = QUAT_TMP ;
	squad_work = &SCRPADWORK->squad_work ;
#ifndef KP_WINDOWS
	for ( i = 0 ; i < n_joint ; i++ ){
		float		t ;
		if ( mask & 0x01 ){
			t = joints->time * joints->inv_time ;
#if 1 //BP_PS2
//#ifndef PSX2
			MT_QuatSlerp( q + 0, &joints->prev, &joints->next, t );
			MT_QuatSlerp( q + 1, &joints->c_prev, &joints->c_next, t );
			MT_QuatNormalize( &q[0], &q[0] );
			MT_QuatNormalize( &q[1], &q[1] );
			t = 2.0F * t * ( 1.0F - t ) ;
			MT_QuatSlerp( rots, q + 0, q + 1, t );
			MT_QuatNormalize( rots, rots );
#else
			*(u_long128*)&squad_work->prev = *(u_long128*)&joints->prev ;
			*(u_long128*)&squad_work->next = *(u_long128*)&joints->next ;
			SetQuatSlerp( &squad_work->prev, &squad_work->next, t );
			*(u_long128*)&squad_work->c_prev = *(u_long128*)&joints->c_prev ;
			*(u_long128*)&squad_work->c_next = *(u_long128*)&joints->c_next ;
			GetQuatSlerp( q + 0 );
			SetQuatSlerp( &squad_work->c_prev, &squad_work->c_next, t );
			MT_QuatNormalize( &q[0], &q[0] );
			t = 2.0F * t * ( 1.0F - t ) ;
			GetQuatSlerp( q + 1 );
			MT_QuatNormalize( &q[1], &q[1] );
			SetQuatSlerp( q + 0, q + 1, t );
			GV_PREFECH( &joints[1].prev );
			GetQuatSlerp( rots );
			MT_QuatNormalize( rots, rots );
#endif
		}
		mask >>= 1 ;
		joints++ ;
		rots++ ;
	}
#else
	if( mask == 0xffffffffffffffff )
	{
		/* mask判定無し */
		for ( i =  n_joint; i > 0 ; i-- ){
			float		t ;

			t = joints->time * joints->inv_time ;

			MT_QuatSlerp( q + 0, &joints->prev, &joints->next, t );
			MT_QuatSlerp( q + 1, &joints->c_prev, &joints->c_next, t );
			MT_QuatNormalize( &q[0], &q[0] );
			MT_QuatNormalize( &q[1], &q[1] );
			t = 2.0F * t * ( 1.0F - t ) ;
			MT_QuatSlerp( rots, q + 0, q + 1, t );
			MT_QuatNormalize( rots, rots );
	
			joints++ ;
			rots++ ;
		}
	}
	else
	{
		/* mask判定有り */
		for ( i = n_joint ; (i > 0) && mask ; i-- ){
			float		t ;

			if ( mask & 0x01 ){
				t = joints->time * joints->inv_time ;

				MT_QuatSlerp( q + 0, &joints->prev, &joints->next, t );
				MT_QuatSlerp( q + 1, &joints->c_prev, &joints->c_next, t );
				MT_QuatNormalize( &q[0], &q[0] );
				MT_QuatNormalize( &q[1], &q[1] );
				t = 2.0F * t * ( 1.0F - t ) ;
				MT_QuatSlerp( rots, q + 0, q + 1, t );
				MT_QuatNormalize( rots, rots );
			}
			mask >>= 1 ;
			joints++ ;
			rots++ ;
		}
	}
#endif
}

/* 次フレームの関節回転データを設定 */
static void PlayJointData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_JOINT_SEGMENT	*joints ;
	int					i, n_joint ;
	float				total_motion_time, play_time_base ;
#ifdef KP_WINDOWS
	unsigned short	*data_ptr ;		/* アーカイブデータの先頭アドレス */
	int				*rots_offset ;	/* 関節回転情報のオフセット */
#endif

	/* 補間終了処理 */
#ifdef KP_WINDOWS
	data_ptr    = mt3_ctrl->data_ptr ;
	rots_offset = mt3_ctrl->file_header->rots_offset ;
#endif
	total_motion_time = (float)(mt3_ctrl->file_header->motion_length * mt3_ctrl->file_header->motion_base_tick) ;
	play_time_base = mt3_ctrl->play_time_base ;
	n_joint = mt3_ctrl->file_header->rot_units ;
	joints = mt3_ctrl->joints ;
	for ( i = 0 ; i < n_joint ; i++ ){
		joints->time += play_time_base ;
		while ( joints->time > joints->interp_time ){
			joints->total_time += joints->interp_time ;
			joints->time -= joints->interp_time ;
			if ( joints->total_time < total_motion_time ){
				/* 次の補間パラメータの準備 */
				SetNextQuatData( joints );
			} else {
				/* 先頭のデータをセット */
				joints->total_time -= total_motion_time ;
				//joints->time = joints->total_time ;
				joints->data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->rots_offset[i] ;
				joints->bit_pos = 0 ;
				/* 補間パラメータセット */
				SetNextQuatData( joints );
			}
		}

		joints++ ;
	}

}



/* ========= その他内部使用関数群 ========= */
/* 新たな関節用データを読み込んで設定する */
static void SetNextQuatData( MT3_JOINT_SEGMENT *joints )
{
	unsigned short		*data ;
	int					bit_pos, len ;

	/* 前回の補間先クォータニオンを次の補間元として設定する */
	joints->prev = joints->next ;
	joints->c_prev = joints->c_next ;

	data = joints->data ;
	bit_pos = joints->bit_pos ;

	/* データの読み取り */
	len = PopDataMask( data, bit_pos, 0xff );
	ADJUST_POINTER( data, bit_pos, 8 );
	GetQuatDataFromBuffer( data, bit_pos, &joints->next );
	ADJUST_POINTER( data, bit_pos, (12*4) );
	if ( len & 0x80 ){
		joints->c_next = joints->next ;
		len &= 0x7f ;
	} else {
		GetQuatDataFromBuffer( data, bit_pos, &joints->c_next );
		ADJUST_POINTER( data, bit_pos, (12*4) );
	}
#if 0
	joints->count = len ;
	joints->interp_base = 1.0F / (float)len ;
	joints->interp = joints->interp_base ;
#else
	/* 1/300秒単位の総合時間を計算 */
	joints->interp_time = (float)len * joints->base_tick ;
	/* 補間パラメータｔ算出用の除算定数を計算 */
	joints->inv_time = 1.0f / joints->interp_time ;
#endif

	joints->data = data ;
	joints->bit_pos = bit_pos ;
}

/* クォータニオンデータをバッファから読み込む */
static void GetQuatDataFromBuffer( unsigned short *data, int bit_pos, FVECTOR *q )
{
	q->vx = (float)PopDataMaskExt( data, bit_pos, 12, 0x0fff ) / 2047.0F;
	ADJUST_POINTER( data, bit_pos, 12 );
	q->vy = (float)PopDataMaskExt( data, bit_pos, 12, 0x0fff ) / 2047.0F;
	ADJUST_POINTER( data, bit_pos, 12 );
	q->vz = (float)PopDataMaskExt( data, bit_pos, 12, 0x0fff ) / 2047.0F;
	ADJUST_POINTER( data, bit_pos, 12 );
	q->vw = (float)PopDataMaskExt( data, bit_pos, 12, 0x0fff ) / 2047.0F;
	ADJUST_POINTER( data, bit_pos, 12 );
}

/*----------------------------------------------------------------*/


	/*
		クォータニオンの乗算（正規化なし）
	*/
static void MT_QuatMulFast( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 )
{
#ifdef 	PSX2

#if 1//BP_ASM
   BP_Quat_MulFast( res, q1, q2 );
#else

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

#endif

#endif
#ifdef KP_XBOX
	D3DXQuaternionMultiply((D3DXQUATERNION*)res, (D3DXQUATERNION*)q2, (D3DXQUATERNION*)q1);	// 逆になるので注意
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

#if 0
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
#endif

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


