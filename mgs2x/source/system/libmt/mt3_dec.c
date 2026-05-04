/*
	mt3_dec.c
	ｍｔ３ファイルモーションデータ再生ルーチン（デバッグ用のため現在未使用）

	1999/07/07 K.Takabe
	$Id: mt3_dec.c,v 1.1.1.3 2002/11/19 11:42:52 Yoshizawa1 Exp $

*/
/*
	《ＰＳＹ ＭＥＴＡＬ ＧＥＡＲ ＳＯＬＩＤ》
	ｍｔ３ファイルモーションデータ再生ルーチン

 ＭＴ３モーション再生ワークの初期化 
extern void MT_InitMT3Work( MT3_CONTROL *mt3_ctrl,
					MT3_MOVE_SEGMENT *move,
					MT3_FIX_SEGMENT *fix,
					MT3_JOINT_SEGMENT *joints,
					MT3_TURN_SEGMENT *turn,
					FVECOTR *rots );

 ｍｔ３モーションデータを再生ワークにセットする 
extern void MT_SetMT3Motion( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *motion_data );

 モーション再生処理 
extern void MT_PlayMT3Motion( MT3_CONTROL *mt3_ctrl );

*/

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

#include	"libgv.h"
#include	"libmt.h"

#define TIME_BASE	(5)


/* データ位置補正マクロ */
#define ADJUST_POINTER(a,b,c) { b += c ; a += b / 16 ; b &= 0x0f ; }

/* ========= スタティック関数用プロトタイプ宣言 ========= */
/* 移動データ展開ワーク初期化 */
static void InitMoveControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 移動データ展開ワーク初期化 */
static void InitJointControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header );
/* 次フレームの移動量データを設定 */
static void PlayMoveData( MT3_CONTROL *mt3_ctrl, int end_flag );
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


/* ========= ＭＴ３モーション再生 ========= */
/* ＭＴ３モーション再生ワークの初期化 */
void MT_InitMT3Work( MT3_CONTROL *mt3_ctrl,
					MT3_MOVE_SEGMENT *move,
					MT3_FIX_SEGMENT *fix,
					MT3_JOINT_SEGMENT *joints,
					MT3_TURN_SEGMENT *turn,
					FVECTOR *rots )
{
	mt3_ctrl->move = move ;
	mt3_ctrl->fix = fix ;
	mt3_ctrl->joints = joints ;
	mt3_ctrl->turn = turn ;
	mt3_ctrl->rots = rots ;
	mt3_ctrl->play_tick_base = TIME_BASE ;
	mt3_ctrl->play_time_scale = 0 ;
}

/* ｍｔ３モーションデータを再生ワークにセットする */
void MT_SetMT3Motion( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *motion_data )
{
	mt3_ctrl->file_header = motion_data ;
	//mt3_ctrl->data_ptr = motion_data->archives_data ;
	mt3_ctrl->data_ptr = (unsigned short*)( (int)motion_data + (int)motion_data->archives_data );
	mt3_ctrl->flag = 0 ;
	mt3_ctrl->time = 0 ;
	mt3_ctrl->loop = 0 ;
	mt3_ctrl->play_tick = 0 ;
	/* 移動データ処理ワークの初期化 */
	if ( mt3_ctrl->move != NULL && motion_data->flag & MT3_FLAG_MOVE_PART ){
		InitMoveControlWork( mt3_ctrl, motion_data );
	}
	/* 関節回転データ処理ワークの初期化 */
	if ( mt3_ctrl->joints != NULL && motion_data->flag & MT3_FLAG_ROT_PART ){
		InitJointControlWork( mt3_ctrl, motion_data );
	}
}

/* モーション再生処理 */
void MT_PlayMT3Motion( MT3_CONTROL *mt3_ctrl )
{
	int		end_flag = 0, flag ;

	/* 経過時間計算 */
	mt3_ctrl->play_tick += mt3_ctrl->play_tick_base ;

	/* モーション停止チェック */
	if ( mt3_ctrl->flag & MT3_SLEEP ) return ;

	/* 再生終了チェック */
	mt3_ctrl->time++ ;
	if ( mt3_ctrl->time == mt3_ctrl->file_header->motion_length ){
		/* 終了フレームならフラグを立てる */
		end_flag = 1 ;
		mt3_ctrl->flag |= MT3_PLAYEND ;
		mt3_ctrl->time = 0 ;
		mt3_ctrl->play_tick = 0 ;
		mt3_ctrl->loop++ ;
		/* １フレームモーションチェック */
		if ( mt3_ctrl->file_header->motion_length == 1 ) mt3_ctrl->flag |= MT3_SLEEP ;
	}

	flag = mt3_ctrl->file_header->flag ;
	/* 移動データ処理 */
	if ( mt3_ctrl->move != NULL && flag & MT3_FLAG_MOVE_PART ){
		PlayMoveData( mt3_ctrl, end_flag );
	}
	/* 関節回転データ処理 */
	if ( mt3_ctrl->joints != NULL && flag & MT3_FLAG_ROT_PART ){
		PlayJointData( mt3_ctrl, end_flag );
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
	move->ave_height = (float)PopDataBit( data, bit_pos, 16 );
	ADJUST_POINTER( data, bit_pos, 16 );
	/* 有効ビット数の読み込み */
	tmp = PopDataBit( data, bit_pos, 16 );
	ADJUST_POINTER( data, bit_pos, 16 );
	move->e_bit_x = (char)( tmp & 0x0f ) ; tmp >>= 4 ;
	move->e_bit_y = (char)( tmp & 0x0f ) ; tmp >>= 4 ;
	move->e_bit_z = (char)( tmp & 0x0f ) ;

	move->data = data ;
	move->bit_pos = bit_pos ;
}

/* 移動データ展開ワーク初期化 */
static void InitJointControlWork( MT3_CONTROL *mt3_ctrl, MT3_FILE_HEADER *file_header )
{
	MT3_JOINT_SEGMENT	*joints ;
	int					i ;

	joints = mt3_ctrl->joints ;
	for ( i = 0 ; i < file_header->rot_units ; i++ ){
		unsigned short		*data ;
		int					bit_pos, len ;
		joints->data = data = mt3_ctrl->data_ptr + file_header->rots_offset[i] ;
		joints->bit_pos = bit_pos = 0 ;

		/* 初期角度の読み込み */
		len = PopDataMask( data, bit_pos, 0xff );
		ADJUST_POINTER( data, bit_pos, 8 );
		GetQuatDataFromBuffer( data, bit_pos, &joints->next );
		ADJUST_POINTER( data, bit_pos, (14*4) );
		if ( len & 0x80 ){
			joints->c_next = joints->next ;
		} else {
			GetQuatDataFromBuffer( data, bit_pos, &joints->c_next );
			ADJUST_POINTER( data, bit_pos, (14*4) );
		}
		joints->data = data ;
		joints->bit_pos = bit_pos ;

		/* 補間パラメータセット */
		SetNextQuatData( joints );

		joints++ ;
	}

}

/* 次フレームの移動量データを設定 */
static void PlayMoveData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_MOVE_SEGMENT	*move ;
	unsigned short		*data ;
	int					bit_pos, e_bit ;

	move = mt3_ctrl->move ;
	data = move->data ;
	bit_pos = move->bit_pos ;

	/* Ｘ軸移動量取得 */
	e_bit = move->e_bit_x ;
	move->step.vx = (float)PopDataBitExt( data, bit_pos, e_bit );
	ADJUST_POINTER( data, bit_pos, e_bit );

	/* 中心Ｙ座標取得 */
	e_bit = move->e_bit_y ;
	move->step.vy = (float)PopDataBitExt( data, bit_pos, e_bit ) + move->ave_height ;
	ADJUST_POINTER( data, bit_pos, e_bit );

	/* Ｚ軸移動量取得 */
	e_bit = move->e_bit_z ;
	move->step.vz = (float)PopDataBitExt( data, bit_pos, e_bit );
	ADJUST_POINTER( data, bit_pos, e_bit );

	/* 再生終了チェック（データ取得位置の初期化） */
	if ( end_flag ){
		data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->move_offset ;
		data += 2 ;		/* Ｙ平均値、各軸有効ビット数のデータは必要ないため */
		bit_pos = 0 ;
	}

	move->data = data ;
	move->bit_pos = bit_pos ;
}

/* 次フレームの関節回転データを設定 */
static void PlayJointData( MT3_CONTROL *mt3_ctrl, int end_flag )
{
	MT3_JOINT_SEGMENT	*joints ;
	int					i, n_joint ;
	FVECTOR				*q, *rots ;
	FVECTOR				buffer[4] ;

	/* 角度補間処理 */
	joints = mt3_ctrl->joints ;
	rots = mt3_ctrl->rots ;
	n_joint = mt3_ctrl->file_header->rot_units ;
	q = buffer ;
	for ( i = 0 ; i < n_joint ; i++ ){
		float		t ;
		t = joints->interp ;
		MT_QuatSlerp( q + 0, &joints->prev, &joints->next, t );
		MT_QuatSlerp( q + 1, &joints->c_prev, &joints->c_next, t );
		//MT_QuatNormalize( &q[0], &q[0] );
		//MT_QuatNormalize( &q[1], &q[1] );
		t = 2.0F * t * ( 1.0F - t ) ;
		MT_QuatSlerp( rots, q + 0, q + 1, t );
		MT_QuatNormalize( rots, rots );
		joints++ ;
		rots++ ;
	}

	/* 補間終了処理 */
	joints = mt3_ctrl->joints ;
	rots = mt3_ctrl->rots ;
	for ( i = 0 ; i < n_joint ; i++ ){
		joints->count-- ;
		joints->interp += joints->interp_base ;
		if ( joints->count == 0 ){
			if ( end_flag == 0 ){
				/* 次の補間パラメータの準備 */
				SetNextQuatData( joints );
			} else {
				/* 先頭のデータをセット */
				unsigned short	*data ;
				int		bit_pos, len ;
				data = mt3_ctrl->data_ptr + mt3_ctrl->file_header->rots_offset[i] ;
				bit_pos = 0 ;
				/* ループモーションを想定している為、読み取り位置のみを設定し直す */
				len = PopDataMask( data, bit_pos, 0xff );
				if ( len & 0x80 ){
					ADJUST_POINTER( data, bit_pos, (8+14*4) );
				} else {
					ADJUST_POINTER( data, bit_pos, (8+14*4*2) );
				}
				joints->data = data ;
				joints->bit_pos = bit_pos ;
				/* 補間パラメータセット */
				SetNextQuatData( joints );
			}
		}

		joints++ ;
		rots++ ;
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
	ADJUST_POINTER( data, bit_pos, (14*4) );
	if ( len & 0x80 ){
		joints->c_next = joints->next ;
		len &= 0x7f ;
	} else {
		GetQuatDataFromBuffer( data, bit_pos, &joints->c_next );
		ADJUST_POINTER( data, bit_pos, (14*4) );
	}
	joints->count = len ;
	joints->interp = 0.0F ;
	joints->interp_base = 1.0F / (float)len ;

	joints->data = data ;
	joints->bit_pos = bit_pos ;
}

/* クォータニオンデータをバッファから読み込む */
static void GetQuatDataFromBuffer( unsigned short *data, int bit_pos, FVECTOR *q )
{
#if 1
	q->vx = (float)PopDataMaskExt( data, bit_pos, 14, 0x3fff ) / 4096.0F;
	ADJUST_POINTER( data, bit_pos, 14 );
	q->vy = (float)PopDataMaskExt( data, bit_pos, 14, 0x3fff ) / 4096.0F;
	ADJUST_POINTER( data, bit_pos, 14 );
	q->vz = (float)PopDataMaskExt( data, bit_pos, 14, 0x3fff ) / 4096.0F;
	ADJUST_POINTER( data, bit_pos, 14 );
	q->vw = (float)PopDataMaskExt( data, bit_pos, 14, 0x3fff ) / 4096.0F;
	ADJUST_POINTER( data, bit_pos, 14 );
#else
	int	tmp ;
	tmp = PopDataMaskExt( data, bit_pos, 14, 0x3fff );
	asm( "mtc1	%0,$f4;itof12.s $f4,$f4;swc1 $f4,0(%1)"::"r"(tmp),"r"(q):"$f4");
	tmp = PopDataMaskExt( data, bit_pos, 14, 0x3fff );
	asm( "mtc1	%0,$f4;itof12.s $f4,$f4;swc1 $f4,4(%1)"::"r"(tmp),"r"(q):"$f4");
	tmp = PopDataMaskExt( data, bit_pos, 14, 0x3fff );
	asm( "mtc1	%0,$f4;itof12.s $f4,$f4;swc1 $f4,8(%1)"::"r"(tmp),"r"(q):"$f4");
	tmp = PopDataMaskExt( data, bit_pos, 14, 0x3fff );
	asm( "mtc1	%0,$f4;itof12.s $f4,$f4;swc1 $f4,12(%1)"::"r"(tmp),"r"(q):"$f4");
#endif
}

