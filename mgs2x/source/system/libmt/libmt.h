/*
	libmt.h
	モーション再生ライブラリヘッダ

	1999/07/07 K.Takabe
	$Id: libmt.h,v 1.1.1.3 2002/11/19 11:42:51 Yoshizawa1 Exp $

*/

#ifndef __LIBMT__H__
#define __LIBMT__H__

#include	"mgs_type.h"
#include	"libdg.h"

#include	"fmt_sar.h"
#include	"fmt_mtn.h"

#ifdef __cplusplus
extern "C" {
#endif

#define long32 int

/* ---------------------------------------------------------------- */
	/*
		壁床用効果音変換テーブルリスト管理
	*/
#define MAX_EXCHG_LIST	(256)

typedef struct _mt_exchg_se{
	u_short		se_code ;
	u_char		noise_vol ;
	u_char		flag ;
} MT_EXCHG_SE ;

typedef struct _mt_exchg_se_list{
	u_short		target_se_code ;	/* 変換対象音コード */
	char		id ;				/* ＩＤ */
	u_char		type ;				/* 属性 */
	MT_EXCHG_SE	*data_ptr ;			/* データへのポインタ */
} MT_EXCHG_SE_LIST ;

typedef ALIGN16_DECL(struct) _mt_se_exchange{
	MT_EXCHG_SE_LIST	se_list[ MAX_EXCHG_LIST ] ;
	MT_EXCHG_SE			exchg_data[ MAX_EXCHG_LIST ][ 8 ] ;
} MT_SE_EXCHANGE  ;

extern MT_SE_EXCHANGE		MT_SeExchange ;

// RMT
extern void RMT_VerifyEndianSwap( RMT_FILE_HEADER *pData );

/*----------------------------------------------------------------*/
	/*
		SAR再生関連
	*/

typedef struct _sar_inst {
	float		time ;			/* 再生基準時間（１／３００秒単位） */
	float		loop_time ;		/* ループ再生時間 */
	float		base_tick ;		/* 進行時間（１／３００秒単位） */
	int			play_flag ;		/* 再生フラグ */
	int			play_num ;		/* 再生しているデータの番号 */
	void		*local_header ;	/* 再生しているデータのローカルヘッダ */
	void		*data_top ;		/* 再生しているデータの先頭アドレス */
	int			pad ;
} SAR_INST ;

typedef struct _sar_control {
	int			flag ;			/* 各種フラグ */
	int			n_instance ;	/* 再生ワーク数 */
	int			type ;			/* 再生データタイプ */
	SAR_HEADER	*header ;		/* 関連付けされたSARファイル */
	int			param1 ;		/* コールバック用パラメータ */
	int			(*callback)( SAR_INST*, int, int ) ;
	int			pad[2] ;
	SAR_INST	instance[0] ;	/* 再生用ワーク */
} SAR_CONTROL ;

	/*
		SEVファイル関連
	*/
typedef struct _sev_work {
	u_long64		mask ;
	DG_OBJS		*objs ;
	int			flag ;
} SEV_WORK ;


#define SAR_FLAG_PLAY	(0x0001)
#define SAR_FLAG_MASK	(0x0002)
#define SAR_FLAG_LOOP	(0x0004)
#define SAR_FLAG_PAUSE	(0x0008)


/*----------------------------------------------------------------*/

	/*
		ｍｔ３データ再生ワーク構造体
	*/

/* 移動データ展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			step ;			/* 取得移動量＆中心高さ情報 */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
	float			ave_height ;	/* Ｙの平均値 */
	char			e_bit_x;		/* Ｘ有効ビット桁数 */
	char			e_bit_y;		/* Ｙ有効ビット桁数 */
	char			e_bit_z;		/* Ｚ有効ビット桁数 */
	char			pad ;
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			total_time ;	/* 総経過時間（１／３００秒単位） */
	float			base_tick ;		/* モーションファイルの１フレームの時間（１／３００秒単位） */
} MT3_MOVE_SEGMENT  ;

/* 新・移動データ展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			step ;			/* 取得移動量＆中心高さ情報 */
	FVECTOR			prev, d_prev ;	/* 補間先座標＆補間先変化量 */
	FVECTOR			next, d_next ;	/* 補間元座標＆補間元変化量 */
	FVECTOR			old_pos ;		/* 前回の算出座標（移動量計算用） */
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			inv_time ;		/* 補間区間時間の逆数 */
	float			base_tick ;		/* モーションファイルの１フレームの時間（１／３００秒単位） */
	float			total_time ;	/* キー位置の累計時間 */
	float			scale ;			/* 補間区間の変化量の補正用 */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
} MT3_MOVE_SEGMENT2  ;

/* 移動データ展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			step ;			/* 取得移動量＆中心高さ情報 */
	FVECTOR			prev ;			/* 補間元座標 */
	FVECTOR			next ;			/* 補間先座標 */
	FVECTOR			old_pos ;		/* 旧座標 */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			inv_time ;		/* 補間区間時間の逆数 */
	float			base_tick ;		/* モーションの１フレームの時間（１／３００秒単位） */
	float			total_time ;	/* キー位置の累計時間 */
	int				pad[1] ;
} MT3_ROOT_SEGMENT  ;

/* 関節固定処理ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			fix_pos ;		/* 固定座標 */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
	int				fix_joint ;		/* 固定関節ユニット番号 */
	int				flag ;			/* 更新フラグ（１で固定座標更新要求） */
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			base_tick ;		/* モーションの１フレームの時間（１／３００秒単位） */
	float			total_time ;	/* キー位置の累計時間 */
} MT3_FIX_SEGMENT  ;

/* 関節角度展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			prev, c_prev ;	/* 補間元クォータニオン＆補正用クォータニオン */
	FVECTOR			next, c_next ;	/* 補間先クォータニオン＆補正用クォータニオン */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			inv_time ;		/* 補間区間時間の逆数 */
	float			base_tick ;		/* モーションの１フレームの時間（１／３００秒単位） */
	float			total_time ;	/* キー位置の累計時間 */
	int				pad[1] ;
} MT3_JOINT_SEGMENT  ;

/* 方向転換角度展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			prev, c_prev ;	/* 補間元クォータニオン＆補正用クォータニオン */
	FVECTOR			next, c_next ;	/* 補間先クォータニオン＆補正用クォータニオン */
	float			old_rot ;
	float			new_rot ;
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
	int				count ;			/* 残りカウンタ（初期値は補間フレーム数、０で次のキーに移行） */
	float			interp ;		/* 補間パラメータ（0<t<=1） */
	float			interp_base ;	/* 補間パラメータ増加値 */
	int				pad ;
} MT3_TURN_SEGMENT  ;

/* 関節移動量データ展開ワーク */
typedef ALIGN16_DECL(struct) {
	FVECTOR			pos ;			/* 展開座標 */
	FVECTOR			prev, d_prev ;	/* 補間先座標＆補間先変化量 */
	FVECTOR			next, d_next ;	/* 補間元座標＆補間元変化量 */
	float			interp_time ;	/* 補間区間時間（１／３００秒単位） */
	float			time ;			/* １／３００秒単位カウンタ（初期値は前補間区間のあまり） */
	float			inv_time ;		/* 補間区間時間の逆数 */
	float			base_tick ;		/* モーションファイルの１フレームの時間（１／３００秒単位） */
	float			total_time ;	/* キー位置の累計時間 */
	float			scale ;			/* 補間区間の変化量の補正用 */
	short			*data ;			/* データ読み取り位置 */
	int				bit_pos ;		/* データ読み取りビット位置 */
} MT3_TRANS_SEGMENT  ;

	/*
		ｍｔ３モーション再生コントロールワーク
	*/
typedef ALIGN16_DECL(struct) {
	int					motion_num ;		/* 再生モーション番号（拡張用） */
	MAR_HEADER			*motion_arc ;		/* モーションアーカイバへのポインタ */
	MT3_FILE_HEADER		*file_header ;		/* 再生中モーションファイルヘッダ */
	unsigned short		*data_ptr ;			/* アーカイブデータの先頭アドレス */
	long64				mask ;				/* 再生マスク（関節毎にビットが１であれば有効） */
	long64				merge_flag ;		/* 角度合成フラグ（ビットが１で親関節の角度無視） */
	int					flag ;				/* 各種フラグ */
	int					time ;				/* 再生経過フレーム数（今後は使わなくなるかも） */
	int					loop ;				/* 再生ループ数 */
	int					last_check_time ;	/* 最後にチェックした経過時間（高速化の為整数でチェック） */
	float				motion_total_time ;	/* モーションの長さ（１／３００秒単位） */
	float				motion_time_base ;	/* モーション本来の再生間隔（１／３００秒単位） */
	float				play_time ;			/* モーション再生時間を実数で表現（１／３００秒単位） */
	float				play_time_base ;	/* モーション再生時間に対する進行時間（１／３００秒単位） */
	float				motion_total_time_inv ;	/* モーションの再生時間の逆数（必要無いか？） */
	MT3_MOVE_SEGMENT	*move ;
	MT3_MOVE_SEGMENT2	*move2 ;
	MT3_ROOT_SEGMENT	*root ;
	MT3_FIX_SEGMENT		*fix ;
	MT3_JOINT_SEGMENT	*joints ;
	MT3_TURN_SEGMENT	*turn ;
	MT3_TRANS_SEGMENT	*trans ;
	FVECTOR				*rots ;				/* モーション展開先バッファ */
	int					pad[3] ;
} MT3_CONTROL  ;


#define MT3_ACTIVE		(0x80000000)		/* 有効フラグ */
#define MT3_PLAYEND		(0x00000001)		/* 最終フレームセット完了フラグ */
#define MT3_SLEEP		(0x00000002)		/* 再生処理停止 */
#define MT3_PLAYLAST1	(0x00000004)		/* 最終フレームセット完了フラグ */


	/*
		モーションコントロール構造体
	*/
typedef ALIGN16_DECL(struct) _motion_control{
	MAR_HEADER		*motion_arc ;			/* モーションアーカイバへのポインタ */
	MT3_CONTROL		*mt3_ctrl ;				/* モーション管理構造体 */
	int				n_layer ;				/* 同時管理モーション数 */
	int				n_joints ;				/* 関節数 */
	int				flag ;					/* 各種フラグ */
	int				interp_count ;			/* 補間カウンタ */
	float			interp_time ;			/* 補間時間 */
	float			interp_inc ;			/* 補間時間増加値 */
	long64			merge_flag ;			/* 角度合成フラグ（ビットが１で親関節の角度無視） */
	long64			interp_flag ;			/* 補間有効フラグ（ビットが１で有効） */
	long64			adjust_flag ;			/* アジャスト有効フラグ（下位ビットから関節０、関節１・・・） */
	FVECTOR			*rots ;					/* モーションから取得した関節角度（クォータニオン） */
	FVECTOR			*adjust ;				/* 関節アジャスト用クォータニオン */
	FVECTOR			*abs_rots ;				/* 絶対回転クォータニオン */
	FVECTOR			*old_abs_rots ;			/* モーション補間用保存値 */
	FVECTOR			*trans ;				/* 関節移動量 */
	FVECTOR			*old_trans ;			/* モーション補間用保存値 */
	float			old_height ;			/* 前回のモデル高さ */
	float			height ;				/* モデル高さ */
	float			root_old_height ;		/* 当たり、トラップ判定座標用前回の高さ */
	float			root_height ;			/* 当たり、トラップ判定座標用高さ */
	SAR_CONTROL		*sar_ctrl ;				/* 効果音シーケンス用ワーク */
	char			se_table_id ;			/* 効果音シーケンス用音コード変換テーブルＩＤ */
	char			se_hazard_type ;		/* 効果音シーケンス用床属性番号 */
	char			se_segment_type ;		/* 効果音シーケンス用壁属性番号 */
	char			pad0 ;
	int				rot_correct ;			/* 回転補正値（MT_SetMotionData()直後のみ有効） */
	int				map_name ;				/* 効果音シーケンス用マップ名（ノイズ発生用） */
	int				use_move_layer ;		/* 参照する移動量のレイヤ番号（MT_FLAG_MOVE_EXが必要） */
	int				pad1[3] ;
	FVECTOR			step ;					/* モーションステップ値格納 */
	FVECTOR			root_step ;				/* 当たり、トラップ判定座標用ステップ値格納 */
} MOTION_CONTROL  ;

enum {
	MT_FLAG_MOVE_EX		= 0x00000001,	/* ０番以外のレイヤの移動量の有効化（初期化時に必要） */
	MT_FLAG_TRANS		= 0x00000002,	/* 関節移動量データの再生を許可する（初期化時に必要） */
	MT_FLAG_REVERSAL1	= 0x00000100,	/* 上半身左右反転 */
	MT_FLAG_REVERSAL2	= 0x00000200,	/* 下半身左右反転 */
	/* モデル構造定義関連 */
	MT_FLAG_TYPE_MASK	= 0x000F0000,	/* 構造モデル構造マスク */
	MT_FLAG_HUMAN1		= 0x00010000,	/* 人型構造１（１６関節＝ＭＧＳ１と同じ） */
	MT_FLAG_HUMAN2		= 0x00020000,	/* 人型構造２（２１関節） */
	MT_FLAG_HUMAN3		= 0x00030000,	/* 人型構造３ */
	MT_FLAG_OBJECT		= 0x00040000,	/* 人型構造３ */
	MT_FLAG_PDRAY		= 0x000f0000,	/* 量産型レイ */
};

/*----------------------------------------------------------------*/
	/*
		モーション再生位置チェックマクロ
	*/
#define MT_CHECK_END(mt,a) ( ( (mt)->mt3_ctrl[a].flag & MT3_PLAYEND ) != 0 )
#define MT_CHECK_LAST1(mt,a) ( ( (mt)->mt3_ctrl[a].flag & MT3_PLAYLAST1 ) != 0 )
#define MT_CHECK_MOTION_TIME(mt,ly,tm)	( MT_CheckMotionTime( &(mt)->mt3_ctrl[(ly)], (tm) ) )
/*----------------------------------------------------------------*/
	/*
		モーションが指定時間を通過したかどうかをチェック
		（複数チェックする場合には小さい値から順番にチェックすること）
	*/
static inline int MT_CheckMotionTime( MT3_CONTROL *mt3_ctrl, int time )
{
	if ( mt3_ctrl->last_check_time >= time ) return ( 0 );
	if ( DG_FTOI( mt3_ctrl->play_time ) >= time ){
		mt3_ctrl->last_check_time = time ;
		return ( 1 );
	}
	return ( 0 );
}
/*----------------------------------------------------------------*/
	/*
		フェイスモーション管理構造体
	*/
typedef struct _mt_face_datas {
	FVECTOR		trans[FACE_JOINT_MAX] ;
	FVECTOR		quat[FACE_JOINT_MAX] ;
} MT_FACE_DATAS ;

typedef struct _mt_face_control {
	FAR_HEADER		*header ;		/* モーションデータヘッダ */
	int				motion_num ;	/* プレイデータナンバー */
	int				flag ;			/* 再生フラグ */
	FAR_MOTION_DATA	*current_data ;	/* 現在読み取り中のポインタ */
	float			time ;			/* 経過時間 */
	float			total_time ;	/* モーションデータの再生時間 */
	float			prev_time ;		/* 前のキーの時間 */
	float			next_time ;		/* 次のキーの時間 */
	float			inv_time ;		/* 次のキーまでの相対時間の逆数 */
	int				pad[3] ;		/*  */
	MT_FACE_DATAS	org_face ;		/* 補間元モーション */
	MT_FACE_DATAS	trg_face ;		/* 補間先モーション */
	MT_FACE_DATAS	fix_face ;		/* 出力モーション */
} MT_FACE_CONTROL ;

enum {
	MT_FACE_ACTIVE		= 0x0001,	/* アクティブフラグ */
	MT_FACE_SLEEP		= 0x0002,	/*  */
	MT_FACE_INTERP		= 0x0004,	/*  */
};


/*----------------------------------------------------------------*/
	/*
		ＩＫ関連
	*/
typedef struct  {/* 未使用 */
	int	pad ;
} MT_JOINT_INFO ;


typedef struct _mt_ik_joint {
	FVECTOR		rotate ;				/* 回転量（親からの相対回転クォータニオン） */
	//FVECTOR		abs_rot ;				/* 回転量（計算結果） */
	FVECTOR		max ;					/* 最大角度（オイラー角ＸＹＺ） */
	FVECTOR		min ;					/* 最小角度（オイラー角ＸＹＺ） */
	FVECTOR		trans ;					/* 関節先端までのオフセット */
	int			joint_num ;				/* モデルオブジェクトでの対応関節番号 */
	float		weight ;				/* ＩＫ変化量 */
	int			pad[2] ;
} MT_IK_JOINT ;

typedef struct _mt_ik_control {
	int				flag ;				/* 制御フラグ */
	int				n_joints ;			/* ＩＫ関節数 */
	int				n_loop ;			/* ＩＫ処理ループ回数（デフォルト＝１） */
	DG_OBJS			*objs ;				/* ＩＫ対象モデルオブジェクト */
	FVECTOR			target_pos ;		/* ＩＫ制御座標 */
	FMATRIX			base_world ;		/* ＩＫ接続元モデルオブジェクトマトリクス */
	MT_IK_JOINT		joints[ 0 ];			/* ＩＫ関節情報 */
} MT_IK_CONTROL ;

/*----------------------------------------------------------------*/
/* 各種定数 */

//extern int BP_BASE_TICK();
#define TIME_BASE		(BP_BASE_TICK())		/* １／３００秒単位での１フレームの経過時間 */


/*----------------------------------------------------------------*/

#include	"libmt.x"

#ifdef PSX2
#include "eeregs.h"
#endif

#if 1
/* ＤＭＡ転送バグ回避用 */
#define TRANS_SIZE(n,s)	( ( ( n + (int)s/16 + 7 ) & 0xfff8 ) - (int)s/16 )		
#else
#define TRANS_SIZE(n,s)	(n)		
#endif
static inline void MT_StartSprToMem( void *d, void *s, int size )
{
#if 0 //BP_PS2
	u_long128	*dst = (u_long128*)d ;
	u_long128	*src = (u_long128*)s ;
#if 0
	/* ＤＭＡ使用ルーチン */
#if 0
	sceDmaChan	*fromSprDma = (sceDmaChan*)D8_CHCR ;/* fromSPR */
	fromSprDma->sadr = src ;
	sceDmaRecvN( fromSprDma, dst, TRANS_SIZE( size ) );
#else
	SyncDCache( (void*)( dst ), (void*) ( dst + size ) );
	DPUT_D8_SADR( (int)src ) ;
	DPUT_D8_MADR( (int)dst ) ;
	DPUT_D8_QWC( TRANS_SIZE( size, src ) ) ;
	DPUT_D8_CHCR( 0x0100 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:1 */
#endif
#else
	/* ＤＭＡ未使用ルーチン */
	while ( size > 0 ){
		*dst = *src ;
#ifdef PSX2
		if ( ( (int)dst & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(dst) );
#endif		
		dst++ ;
		src++ ;
		size-- ;
	}
#endif
#else
   memcpy(d, s, size * 16);
#endif
}
static inline void  MT_WaitSprToMem( void )
{
#if 0
	/* ＤＭＡ使用ルーチン */
#if 0
	sceDmaChan	*fromSprDma = (sceDmaChan*)D8_CHCR ;/* fromSPR */
	sceDmaSync( fromSprDma, 0, 10 );
#else
	while ( DGET_D8_CHCR() & 0x0100 );
#endif
#else
	/* ＤＭＡ未使用ルーチン */
#endif
}
static inline void  MT_StartMemToSpr( void *d, void *s, int size )
{
#if 0 //BP_PS2

	u_long128	*dst = (u_long128*)d ;
	u_long128	*src = (u_long128*)s ;
#if 0
	/* ＤＭＡ使用ルーチン */
#if 0
	sceDmaChan	*toSprDma = (sceDmaChan*)D9_CHCR ;/* toSPR */
	toSprDma->sadr = dst ;
	sceDmaSendN( toSprDma, src, size );
#else
	SyncDCache( (void*)( src ), (void*) ( src + size ) );
	DPUT_D9_SADR( (int)dst ) ;
	DPUT_D9_MADR( (int)src ) ;
	DPUT_D9_QWC( size ) ;
	DPUT_D9_CHCR( 0x0101 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:1 */
#endif
#else
	/* ＤＭＡ未使用ルーチン */
	while ( size > 0 ){
		*dst = *src ;
#ifdef PSX2		
		if ( ( (int)src & 63 ) == 0 ) asm( "pref 0,64(%0)"::"r"(src) );
#endif		
		dst++ ;
		src++ ;
		size-- ;
	}
#endif

#else
   memcpy(d, s, size * 16);
#endif
}
static inline void  MT_WaitMemToSpr( void )
{
#if 0
	/* ＤＭＡ使用ルーチン */
#if 0
	sceDmaChan	*toSprDma = (sceDmaChan*)D9_CHCR ;/* toSPR */
	sceDmaSync( toSprDma, 0, 10 );
#else
	while ( DGET_D9_CHCR() & 0x0100 );
#endif
#else
	/* ＤＭＡ未使用ルーチン */
#endif
}

/*----------------------------------------------------------------*/


#ifdef __cplusplus
};
#endif

#endif

