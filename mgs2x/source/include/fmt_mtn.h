/*
	fmt_mtn.h
	モーション関連ファイルフォーマット

	2000/06/09 K.Takabe
	$Id: fmt_mtn.h,v 1.1.1.3 2002/11/19 11:41:59 Yoshizawa1 Exp $
*/


#ifndef __FMT_MTN_H__
#define __FMT_MTN_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif


/*----------------------------------------------------------------*/

	/*
		ｍｔ３ファイルヘッダ構造体
	*/
typedef struct {
	int		name_id ;			/* ファイル名ＩＤ */
	int		flag ;				/* 各種フラグ情報 */
	int		motion_base_tick ;	/* モーションのフレームベースカウント（１／３００秒単位） */
	int		motion_length ;		/* モーションフレーム数 */
	short	*archives_data ;	/* アーカイブデータへのポイント */
	int		archives_size ;		/* アーカイブデータのサイズ（ハーフワード単位） */
	int		rot_units ;			/* 可動関節ユニット数 */
	int		root_offset ;		/* 軌跡トレース情報のオフセット */
	int		move_offset ;		/* 移動量データのオフセット */
	int		turn_offset ;		/* 方向転換情報のオフセット */
	int		fix_offset ;		/* 関節固定情報のオフセット */
	int		rots_offset[ 0 ] ;	/* 関節回転情報のオフセット */
} MT3_FILE_HEADER ;

#define MT3_FLAG_MOVE_PART	(0x00000001)
#define MT3_FLAG_ROT_PART	(0x00000002)
#define MT3_FLAG_FIX_PART	(0x00000004)
#define MT3_FLAG_TURN_PART	(0x00000008)
#define MT3_FLAG_TURN_FLAG	(0x00000008)
#define MT3_FLAG_ROOT_PART	(0x00000010)
#define MT3_FLAG_MOVE2_PART	(0x00000020)	/* 新方式圧縮型移動量 */
#define MT3_FLAG_TRANS_PART	(0x00000040)	/* 関節移動量込み */
#define MT3_FLAG_TYPE_MASK	(0x00ff0000)	/* タイプのマスク */
#define MT3_FLAG_TYPE_01	(0x00010000)	/* 上半身下半身分離型モーションデータ */
#define MT3_FLAG_INITDATA	(0x80000000)

/*----------------------------------------------------------------*/

	/*
		ｍａｒファイルヘッダ構造体
	*/
typedef struct {
	MT3_FILE_HEADER	*addr ;		/* ＭＴ３へのポインタ */
	int				name_id ;	/* 旧式StrCodeのＩＤ */
} FILE_TABLE ;
typedef struct {
	int		mar_id ;			/* ファイル判別ＩＤ用 */
	int		joints ;			/* モーションで定義されている関節数 */
	int		n_motion ;			/* 書庫に入っているモーションの数 */
	int		header_size ;		/* アーカイブデータまでのオフセット */
	FILE_TABLE	data_table[ 0 ] ;
} MAR_HEADER;


/* ---------------------------------------------------------------- */

#define FAR_FORMAT_TYPE	(11658451)
#define FACE_JOINT_MAX	(32)		/* モーションの最大関節数 */

typedef struct _face_joint_data{
	FVECTOR	trans ;
	FVECTOR	qrot ;
} FAR_JOINT_DATA;

typedef struct _face_motion_data {
	u_short	time ;				/* キーフレーム */
	u_char	pattern ;			/* パターン番号 */
	u_char	pad ;				/* リザーブ */
} FAR_MOTION_DATA ;

/* モーションシーケンスデータヘッダ */
typedef struct _face_motion_info{
	int				id ;			/* データＩＤ */
	int				max_frame ;		/* 最大フレーム数 */
	FAR_MOTION_DATA	*data ;			/* シーケンスデータへのオフセット */
	int		pad ;					/*  */
} FAR_MOTION_INFO ;

/* フェイスモーションファイルヘッダ */
typedef struct _fmd_header {
	int		format_type ;		/* フォーマットタイプ（11658451 == "face motion"） */
	int		id ;				/* データＩＤ */
	int		n_joints ;			/* モーションの関節数（＝３２） */
	int		pad0[1] ;			/**/
	int		max_pattern ;		/* 定義パターン数 */
	int		max_mask ;			/* 定義マスク数（未使用） */
	int		max_motion ;		/* シーケンスデータ数 */
	int		pad1[1] ;			/**/
	FAR_JOINT_DATA	*pattern_table ;	/* 定義パターンへのオフセット */
	int				mask_offset ;		/* 定義マスクへのオフセット（未使用） */
	FAR_MOTION_INFO	*motion_table ;		/* シーケンスデータへのオフセット */
	int				pad2[1] ;			/**/
} FAR_HEADER ;


/* ---------------------------------------------------------------- */

	/*
		圧縮型顔モーションアーカイバ形式
	*/
#define FPK_FORMAT_TYPE	(9230807)
typedef struct {
	int			id ;			/* 識別ID */
	//int			offset ;		/* データ部におけるオフセット（バイト単位） */
	short		*data ;			/* データ部におけるオフセット（バイト単位） */
	int			data_size ;		/* データサイズ */
	int			n_joints ;		/* モーション関節数 */
} FPK_DATALIST ;
typedef struct {
	int				format_type ;	/* データ識別ID(9230807)"facepack" */
	int				n_datas ;		/* 格納データ数 */
	int				size ;			/* データサイズ（ヘッダ及びリスト情報除く） */
	int				data_offset ;	/* データ部へのオフセッ（バイト単位） */
	FPK_DATALIST	list[0] ;		/**/
} FPK_HEADER ;

//----------------------------------------------------------------------------



/* セーブロード用ヘッダ */
typedef struct {
   int			flags_le ;			/* 各種フラグ */
   int			motion_length ;	/* モーションの有効フレーム数 */
   int			motion_joints ;	/* モーションの関節数 */
   int			motion_tick ;	/* ベースクロック値（１／３００単位） */
   int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
   int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
   int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
   int			unused_minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
   int			unused_jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
   int			unused_sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
   int			unused_anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
   int			pad[4] ;
   int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} RMT_FILE_HEADER ;

#define RMT_FLAG_ENDIAN_SWAPPED 0x80000000

/* ---------------------------------------------------------------- */
/*
	モーションデータファイル生成手順

	*.mdl : ベースモデルデータ（ＶＲＳ用）
	*.mot : ベースモーションデータ（ＶＲＳ用）
	*.mtn : ＭＧＳ２ゲーム用無圧縮モーションデータ（データ管理、調整用）
	*.mt3 : ＭＧＳ２ゲーム用圧縮モーションデータ（間接的に実機で使用）
	*.mar : モーションアーカイバ（mt3データアーカイバ）

	ＭＧＳ２ゲーム用モーションデータをＶＲＳ用データから生成する
	mot + mdl > mtn

	ＭＧＳ２ゲーム用モーションデータはActGuyを使用して圧縮品質を
	調整し、圧縮する
	mtn > mt3

	圧縮されたモーションデータは管理しやすいように使用キャラ毎に
	アーカイブし、実機で使用出来るデータにする
	*.mt3 > mar

*/
/*
	ＭＴ３ファイルフォーマットについて

	ＭＴ３フォーマットは可変スピード再生などにも耐えられるように
	可変位置キーと、２次もしくは３次の補間によってデータを間引く事で
	データの圧縮を実現している。また、各データの要素は先頭オフセット
	からのシーケンシャルデータになっているためランダムアクセスには
	不向きになっている。

	＜モーションデータの要素＞
	・オブジェクト中心の移動軌跡データ（絶対座標）
	・各関節の回転量データ（親オブジェクトからの相対回転角度）
	・オブジェクト中心の代表移動軌跡データ（絶対座標。カメラや当たりチェック用）
	・関節固定フラグ（強制的に座標を固定させる関節を番号で指定）

	＜移動軌跡データ＞
	各キーには以下の情報が含まれている。
	・次のキーまでのフレーム数（８ビット整数）
	・絶対座標ＸＹＺ（１６ビット浮動小数点×３）
	・変化量ＸＹＺ（１フレームでの変化量。１６ビット浮動小数点×３）
	各キー間の補間にはエルミート補間関数を使用する。

	＜回転量データ＞
	各キーには以下の情報が含まれている。
	・次のキーまでのフレーム数（８ビット。最上位ビットは補助クォータニオンなしフラグ）
	・キーの回転クォータニオンＸＹＺＷ（±２０４７の１２ビット整数×４）
	・補間用補助クォータニオンＸＹＺＷ（±２０４７の１２ビット整数×４）
	各キー間の補間には球四角形補間を使用する。
	また、クォータニオンは１／２０４７を乗算して±１の範囲の値にする。


	＜モーションの長さとキーの関係について＞
	移動量等を求める必要があるためＭＴＮファイルの状態では実質的には
	モーション長＋１のデータがあるが、ＭＴ３以降ではその一番最初の
	フレームを除いた分のデータが記録される。また、モーションのループ
	再生のような場合には以下のようになる
	TOTAL:        (1)-(2)-(3)-(4)-(5)-(6)-...-(length)-(1)-(2)-(3)-...
	1ST  :(dummy)-(1)-(2)-(3)-(4)-(5)-(6)-...-(length)
	2ND  :                                     (dummy)-(1)-(2)-(3)-...

	＜キー補間の規則について＞
	フレームＡのキーとフレームＢのキーとの補間で計算される領域は
	フレームＡ＜フレームｎ＜＝フレームＢとなる。（つまり補間定数
	のとる範囲は０＜ｔ＜＝１）
	また、モーション再生時の初期時間は０ではなく、必ずオリジナル
	モーションデータでの第１フレームでの時間からスタートする
	再生間隔 +------+------+------+------+------+--
	　　　　(1)--(2)--(3)--(4)--(5)--(6)--(7)--(8)-：フレームレートの高いモーションデータの場合
	　　　　(1)------(2)------(3)------(4)------(5)：フレームレートの低いモーションデータの場合

*/

/* ---------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif


