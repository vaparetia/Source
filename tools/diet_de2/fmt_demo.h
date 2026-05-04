/*
	fmt_demo.h
	デモストリーミングデータフォーマット

	2000/07/24
	$Id: fmt_demo.h,v 1.3 2001/08/07 10:52:18 usr02774 Exp $

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
	int		packet_type ;			/* パケットタイプ */
	int		size ;					/* パケットサイズ（１６バイト境界に揃えること） */
	int		id ;					/* ユニークＩＤ（各オブジェクトの識別などに使用） */
	int		reserved ;				/* 予約 */
} DEMO_PACKET ;

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
	DEMO_PACKET_MOTION2,					/* モーションパケット */
	/* その他拡張フラグ関連 */
	DEMO_PACKET_NOSKIP = 0x80000000,	/* スキップ禁止フラグ */
} ;

/* ---------------------------------------------------------------- */
/* 汎用コマンドパケット */
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
typedef struct _demo_object {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				cache_id ;		/* EVMのキャッシュＩＤ（＝モデルＩＤ|拡張子ＩＤ） */
	int				flag ;			/* モデル初期化フラグ */
	int				name ;			/* オブジェクト名 */
	int				kms_cache_id ;		/* KMSのキャッシュID */
} DEMO_OBJECT ;

/* モーションパケット */
typedef struct _demo_motion {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	int				motion_type ;	/* モーションのタイプ */
	int				start_joint ;	/* 開始関節番号 */
	int				n_joints ;		/* 関節モーションデータ数 */
	FVECTOR			pos ;			/* オブジェクト中心座標 */
	SVECTOR			rot ;			/* オブジェクト中心傾き（CONTROL構造体に入れる値） */
	int				pad[2] ;		/* ダミー */
	FVECTOR			motion[NULL_ARRAY] ;	/* 関節モーションデータ */
} DEMO_MOTION ;
enum {
	DEMO_MOTION_ROT			= 1,	/* 関節回転情報（クォータニオン指定）のみ */
	DEMO_MOTION_TRANS		= 2,	/* 関節オフセット情報のみ */
	DEMO_MOTION_ROTTRANS	= 3,	/* 関節回転情報（クォータニオン指定）＋関節オフセット情報 */
									/* (rot0+trans0),(rot1+trans1),(rot2+trans2),... */
	DEMO_MOTION_COMPRESS	= 4,	/* １６ビット化＆圧縮 */
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
typedef struct _demo_effect {
	DEMO_PACKET		header ;		/* パケットヘッダ */
	int				effect_id ;		/* エフェクトＩＤ */
	int				exec_type ;		/* 実行タイプ */
	int				pad[2] ;		/* ダミー */
	int				data[NULL_ARRAY] ;	/* パラメータ */
} DEMO_EFFECT ;


/* モーションパックパケット */
typedef struct _demo_motion_info {
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	unsigned short	flag ;			/* データフラグ */
	unsigned char	start_joint ;	/* 開始関節番号 */
	unsigned char	n_joints ;		/* 関節モーションデータ数 */
	unsigned short	offset ;		/* データの先頭オフセット */
	unsigned short	frame ;			/* フレーム補間時間 */
} DEMO_MOTION_INFO ;
typedef struct _demo_motion2 {
	DEMO_PACKET			header ;		/* パケットヘッダ */
	int					n_lists ;		/* パケットに含まれるモーションの総数 */
	int					data_offset ;	/* パケット先頭からデータ領域までのオフセット */
	int					pad[2] ;		/*  */
	DEMO_MOTION_INFO	lists[NULL_ARRAY] ;
} DEMO_MOTION_PACK ;
enum {
	DEMO_MOTION2_MOV			= 0x0001,	/* 中心位置移動量 */
	DEMO_MOTION2_ROT			= 0x0002,	/* 中心位置回転量 */
	DEMO_MOTION2_ROTS			= 0x0004,	/* 各関節の回転量（親関節からの相対回転） */
	DEMO_MOTION2_ABSROTS		= 0x0008,	/* 各関節の回転量（中心からの絶対回転） */
	DEMO_MOTION2_TRANS			= 0x0010,	/* 各関節の移動量 */
};
#if 0
/* 新モーションパケット */
typedef struct _demo_motion_info {
	int				object_id ;		/* 割り当てるオブジェクトのＩＤ */
	unsigned short	flag ;			/* データフラグ */
	unsigned char	start_joint ;	/* 開始関節番号 */
	unsigned char	n_joints ;		/* 関節モーションデータ数 */
	unsigned short	offset ;		/* データの先頭オフセット */
	unsigned short	frame ;			/* フレーム補間時間 */
} DEMO_MOTION_INFO ;
typedef struct _demo_motion2 {
	DEMO_PACKET			header ;		/* パケットヘッダ */
	int					n_lists ;		/* パケットに含まれるモーションの総数 */
	int					data_offset ;	/* パケット先頭からデータ領域までのオフセット */
	int					pad[2] ;		/*  */
	DEMO_MOTION_INFO	lists[NULL_ARRAY] ;
} DEMO_MOTION2 ;
enum {
	DEMO_MOTION2_MOV			= 0x0001,	/* 中心位置移動量 */
	DEMO_MOTION2_ROT			= 0x0002,	/* 中心位置回転量 */
	DEMO_MOTION2_ROTS			= 0x0004,	/* 各関節の回転量（親関節からの相対回転） */
	DEMO_MOTION2_ABSROTS		= 0x0008,	/* 各関節の回転量（中心からの絶対回転） */
	DEMO_MOTION2_TRANS			= 0x0010,	/* 各関節の移動量 */
};
#endif

/* ---------------------------------------------------------------- */


#ifdef __cplusplus
};
#endif

#endif	/* __FMT_DEMO_H__ */
