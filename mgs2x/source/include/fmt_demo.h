/*
	fmt_demo.h
	デモストリーミングデータフォーマット

	2000/07/24
	$Id: fmt_demo.h,v 1.1.1.3 2002/11/19 11:41:58 Yoshizawa1 Exp $

*/

/*

	デモストリーミングの構造について

	デモデータはストリーミングデータ内のデモ認識タグが記述された
	パケットに１フレーム分のパケットが全て格納された状態で入っており、
	その１フレーム分のパケットは以下のようなデモパケットの集合
	構造になっている。

	デモデータはフレーム単位に以下のようにDEMO_PACKETをヘッダとした
	可変長データの集まりで構成されている。デモパケットはそれぞれ
	パケットタイプが設定され、必要なデータがパックされた状態に
	なっている。フレームデータの終了はDEMO_PACKET_TERMINATE_FRAMEが
	来たところで終了する。

	+-----------+---------------------+
	|DEMO_PACKET|                     |
	+-----------+-------+-------------+
	|DEMO_PACKET|       |
	+-----------+-------+-----+
	|DEMO_PACKET|             |
	+-----------+-------------+-------+
	|DEMO_PACKET|                     |
	+-----------+-------------+-------+
	|DEMO_PACKET|             |
	+-----------+-------------+
	|DEMO_PACKET|
	+-----------+
	.
	.
	.
	.
	+-----------+
	|DEMO_PACKET|(=DEMO_PACKET_TERMINATE_FRAME)
	+-----------+

*/

#ifndef __FMT_DEMO_H__
#define __FMT_DEMO_H__


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif


/* ---------------------------------------------------------------- */
/* デモパケットヘッダ */
typedef struct _demo_packet {
	int		_packet_type ;			/* パケットタイプ */
	int		_size ;					/* パケットサイズ（１６バイト境界に揃えること） */
	int		_id ;					/* ユニークＩＤ（各オブジェクトの識別などに使用） */
	int		_reserved ;				/* 予約 */
} DEMO_PACKET ;

#define DEMO_PACKET_PACKET_TYPE(p) ( BP_LE_SwapSInt( (p)._packet_type ) )
#define DEMO_PACKET_SIZE(p) ( BP_LE_SwapSInt( (p)._size ) )
#define DEMO_PACKET_SIZE_SET(p,s) (p)._size = BP_LE_SwapSInt( (s) )
#define DEMO_PACKET_ID(p) ( BP_LE_SwapSInt( (p)._id ) )

enum {
	/* パケットパターン定義開始 */
	DEMO_PACKET_TERMINATE_SEQUENCE,		/* デモシーケンスの終了パケット（拡張情報なし） */
	DEMO_PACKET_TERMINATE_FRAME,		/* フレームの終了パケット（拡張情報なし） */
	DEMO_PACKET_COMMAND,				/* 汎用コマンドパケット */
	DEMO_PACKET_FRAME,					/* フレームパケット */
	DEMO_PACKET_OBJECT,					/* オブジェクトパケット */
	DEMO_PACKET_MOTION,					/* モーションパケット */
	DEMO_PACKET_EFFECT,					/* エフェクトパケット */
	DEMO_PACKET_MOTION_PACK,			/* モーションパックパケット */
	/* パケットパターン定義終端 */
	DEMO_PACKET_END,					/* == 終端コード == */
	/* 新方式モーションストリーム（互換性のためにここで定義しておく） */
	DEMO_PACKET_MST = 0x1000,			/* MST */
	/* その他拡張フラグ関連 */
	DEMO_PACKET_NOSKIP = 0x80000000,	/* スキップ禁止フラグ */

   DEMO_PACKET_ENDIAN_SWAPPED = 0x40000000, // BP - Has the packet already been swapped?

   DEMO_PACKET_TYPE_MASK = 0x0000FFFF // Mask for querying type

} ;

#define DEMO_PACKET_PACKET_TYPE_MASKED(p) ( DEMO_PACKET_PACKET_TYPE(p) & DEMO_PACKET_TYPE_MASK )
#define DEMO_PACKET_IS_ENDIAN_SWAP(p) ( DEMO_PACKET_PACKET_TYPE(p) & DEMO_PACKET_ENDIAN_SWAPPED )
#define DEMO_PACKET_SET_ENDIAN_SWAP(p) (p)._packet_type |= BP_LE_SwapSInt( DEMO_PACKET_ENDIAN_SWAPPED )

/* ---------------------------------------------------------------- */
/* 汎用コマンドパケット */
// BP - THIS GETS ENDIAN SWAPPED
typedef struct _demo_command {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				command ;		/* コマンド */
	int				id ;			/* コマンド対象ＩＤ */
	int				param1 ;		/* パラメータ１ */
	int				param2 ;		/* パラメータ２ */
} DEMO_COMMAND ;
enum {	/* 仮コマンド */
	DEMO_COMMAND_OBJECT_VISIBLE,	/* オブジェクトの表示・非表示 */
	DEMO_COMMAND_OBJECT_DESTROY,	/* オブジェクトの破棄 */
	DEMO_COMMAND_EFFECT_DESTROY,	/* エフェクトの破棄 */
	DEMO_COMMAND_CAMERA_CREATE,		/* カメラの起動 */
} ;

/* フレーム設定パケット */
// BP - THIS GETS ENDIAN SWAPPED
typedef struct _demo_frame {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				no ;			/* カメラ番号 */
	float			screen ;		/* カメラ投影面距離（default:2.0f） */
	float			z_rotate ;		/* カメラＺ軸回転量 */
	int				chanl ;			/* チャンネル番号 */
	FVECTOR			camera_pos ;	/* カメラ座標 */
	FVECTOR			camera_trg ;	/* カメラ注視点 */
} DEMO_FRAME ;

/* オブジェクト作成パケット */
// BP - THIS GETS ENDIAN SWAPPED
typedef struct _demo_object {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				cache_id ;		/* EVMのキャッシュＩＤ（＝モデルＩＤ|拡張子ＩＤ） */
	int				flag ;			/* モデル初期化フラグ */
	int				name ;			/* オブジェクト名 */
	int				kms_cache_id ;		/* KMSのキャッシュID */
} DEMO_OBJECT ;

/* モーションパケット */
// BP - THIS GETS ENDIAN SWAPPED EXCEPT unswapped_motion, which is sometimes shorts
// and sometimes floats depending on the flags in the packet
typedef struct _demo_motion {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	int				motion_type ;	/* モーションのタイプ */
	int				start_joint ;	/* 開始関節番号 */
	int				n_joints ;		/* 関節モーションデータ数 */
	FVECTOR			pos ;			/* オブジェクト中心座標 */
	SVECTOR			rot ;			/* オブジェクト中心傾き（CONTROL構造体に入れる値） */
	int				pad[2] ;		/* ダミー */
	FVECTOR  		motion[NULL_ARRAY] ;	/* 関節モーションデータ */
} DEMO_MOTION ;

enum {
	DEMO_MOTION_ROT			= 1,	/* 関節回転情報（クォータニオン指定）のみ */
	DEMO_MOTION_TRANS		= 2,	/* 関節オフセット情報のみ */
	DEMO_MOTION_ROTTRANS	= 3,	/* 関節回転情報（クォータニオン指定）＋関節オフセット情報 */
									/* (rot0+trans0),(rot1+trans1),(rot2+trans2),... */
	DEMO_MOTION_COMPRESS	= 4,	/* １６ビット化＆圧縮 */
	DEMO_MOTION_ABSROTTRANS	= 8,	/* 関節回転情報を絶対回転クォータニオンで指定 */
};
enum {
	/* モーション圧縮関連定義 */
	MOTIONDIET_ROT_X		= 0x0100,
	MOTIONDIET_ROT_Y		= 0x0200,
	MOTIONDIET_ROT_Z		= 0x0400,
	MOTIONDIET_ROT_W		= 0x0800,
	MOTIONDIET_TRANS_X		= 0x1000,
	MOTIONDIET_TRANS_Y		= 0x2000,
	MOTIONDIET_TRANS_Z		= 0x4000,
} ;

/* エフェクトパケット */
// BP - THIS GETS ENDIAN SWAPPED EXCEPT data_le
typedef struct _demo_effect {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				effect_id ;		/* エフェクトＩＤ */
	int				exec_type ;		/* 実行タイプ */
	int				pad[2] ;		/* ダミー */
	int				data[NULL_ARRAY] ;	/* パラメータ */
} DEMO_EFFECT ;

KP_CTASSERT( sizeof( DEMO_PACKET ) + sizeof( int ) * 4 );

/* モーションパックパケット */
// BP - THIS GETS ENDIAN SWAPPED
typedef struct _demo_motion_info {
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	unsigned short	flag ;			/* データフラグ */
	unsigned char	start_joint ;	/* 開始関節番号 */
	unsigned char	n_joints ;		/* 関節モーションデータ数 */
	unsigned short	offset ;		/* データの先頭オフセット */
	unsigned short	frame ;			/* フレーム補間時間 */
} DEMO_MOTION_INFO ;
// BP - THIS GETS ENDIAN SWAPPED
typedef struct _demo_motion2 {
	DEMO_PACKET			header ;		/* パケットヘッダ */
	int					n_lists ;		/* パケットに含まれるモーションの総数 */
	int					data_offset ;	/* パケット先頭からデータ領域までのオフセット（現在意味無し） */
	int					pad[2] ;		/*  */
	DEMO_MOTION_INFO	lists[NULL_ARRAY] ;
} DEMO_MOTION_PACK ;
enum {
	DEMO_MOTIONPACK_MOV			= 0x0001,	/* 中心位置移動量 */
	DEMO_MOTIONPACK_ROT			= 0x0002,	/* 中心位置回転量 */
	DEMO_MOTIONPACK_ROTS		= 0x0004,	/* 各関節の回転量（親関節からの相対回転） */
	DEMO_MOTIONPACK_ABSROTS		= 0x0008,	/* 各関節の回転量（中心からの絶対回転） */
	DEMO_MOTIONPACK_TRANS		= 0x0010,	/* 各関節の移動量 */
};

/* ---------------------------------------------------------------- */
	/*
		新方式モーションストリーム関連
	*/
/*
	新方式モーションストリームデータの構造について

	旧形式はデモパケットのモーションのみという形になっていたが、
	これでは圧縮率が良くなかったので、デモパケットの無駄な部分を
	省いてモーションのみに特化した形で実装したもの

	モーションストリームパケットはデモストリームと違い１つのパケットのみで
	構成されている。

	+-----------+---------------------+
	|MST_PACKET |パケットデータ実体   |
	+-----------+-------+-------------+


	パケットデータ実体の内訳
	+-----------+
	|MST_INFO   |(0)
	+-----------+
	|MST_INFO   |(1)
	+-----------+
	.
	.
	.
	+-----------+
	|MST_INFO   |(MST_PACKET->n_lists-1)
	+-----------+------------+
	|data area               |
	|                        |
	|                        |
	|                        |
	+------------------------+


	また、各モーションパケットはデータ節約のため間引かれることが
	前提になっており、MST_INFO内のframeの値をかけて線形補間が行われる
	

*/
typedef struct _mst_info {
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	unsigned short	flag ;			/* データフラグ */
	unsigned char	start_joint ;	/* 開始関節番号 */
	unsigned char	n_joints ;		/* 関節モーションデータ数 */
	unsigned short	offset ;		/* データの先頭オフセット */
	unsigned short	frame ;			/* フレーム補間時間 */
} MST_INFO ;

/* モーションパケットヘッダ */
typedef struct _mst_packet {
   // BP - Note that this starts with DEMO_PACKET, but they didn't declare
   // DEMO_PACKET header because 'id' lives where 'reserved' lives
   // in that structure.
	int				_packet_type ;	/* パケットタイプ */
	int				_size ;			/* パケットサイズ（１６バイト境界に揃えること） */
	int				_id ;			/* 現在は意味無し（＝０） */
	int				n_lists ;		/* パケットに含まれるモーション数 */
	MST_INFO		info_list[0] ;	/*  */
} MST_PACKET ;

/* MST_INFO内のフラグ内訳 */
enum {
	MST_FLAG_POS		= 0x0001,	/* 無圧縮中心移動量（現在未対応） */
	MST_FLAG_ROT		= 0x0002,	/* 無圧縮中心回転量（現在未対応） */
	MST_FLAG_ROTS		= 0x0004,	/* 無圧縮関節回転量（現在未対応） */
	MST_FLAG_TRANS		= 0x0008,	/* 無圧縮関節移動量（現在未対応） */
	MST_FLAG_COMPROTS	= 000040,	/* 圧縮回転量（現在特にフラグは見てない） */
	MST_FLAG_COMPTRANS	= 0x0080,	/* 圧縮移動量（現在特にフラグは見てない） */
};


/* ---------------------------------------------------------------- */


#ifdef __cplusplus
};
#endif

#endif	/* __FMT_DEMO_H__ */
