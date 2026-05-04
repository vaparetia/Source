/*
	utl_anmtex.h
	アニメーションテクスチャ展開処理

	2000/06/02 K.Takabe
	$Id: utl_anmtex.h,v 1.2 2002/12/18 05:27:43 takaki Exp $

*/
/*

	.rat形式にまとめられているアニメーションテクスチャを展開して指定した
	テクスチャと差し替えます。

	使い方はmgs2/source/takabe/test/anmtex.cを参照してください。

	.rat形式はtools/anmtex_pack以下のソースを参照してください。

	現在の展開ルーチンはまだ高速化の余地あり。余裕があればどうにかしたいが・・・

*/


/* ---------------------------------------------------------------- */
/* フレーム情報 */
typedef struct {
	int			compress_flag ;		/* 圧縮方式（ bit0~3:compress_type, bit4~7:setup_type ） */
	int			data_offset ;		/* データ格納位置オフセット */
	int			compress_size ;		/* 圧縮データサイズ */
	int			palette_offset ;	/* パレットデータ格納位置オフセット */
} ANMTEX_INFO ;

/* アニメーションテクスチャファイルヘッダ */
typedef struct {
	int			format_id ;	/* フォーマットID */
	int			flag_le ;		/* データフラグ(0:16color 1:256color) */
	int			width ;		/* 画像幅 */
	int			height ;	/* 画像高さ */
	int			frames ;	/* 格納フレーム数 */
	int			pad[3] ;
	ANMTEX_INFO	info[ NULL_ARRAY ];
} ANMTEX_HEADER ;

#define ANMTEX_HEADER_FLAG_ENDIANSWAPPED 0x10

/* 展開用ワーク */
typedef ALIGN16_DECL(struct) {
	ANMTEX_HEADER		*anmtex_header ;		/* アニメテクスチャヘッダー */
	unsigned char		*data_ptr ;				/* データ先頭ポインタ */
	int					frame ;					/* 現在のフレーム数 */
	int					flag ;					/* 再生モード */
	int					buffer_clock ;			/* バッファ切り替えクロック */
	int					tri_id ;				/* 対象ＴＲＩＩＤ */
	int					tex_id ;				/* 対象テクスチャＩＤ */
	int					anmtex_id ;				/* アニメテクスチャＩＤ */
	DG_TEX				*tex ;					/* 対象テクスチャ */
	int					width ;					/* 対象テクスチャ幅 */
	int					height ;				/* 対象テクスチャ高さ */
	int					image_size ;			/* 展開イメージサイズ */
	int					clut_size ;				/* 展開イメージＣＬＵＴサイズ */
	int					last_set_buffer ;		/* 最後に繋げたテクスチャ入れ替えパケット番号 */
//#ifdef PSX2
	DG_TEX_LOADREPLACE	*tex_load[2] ;			/* テクスチャ入れ替えパケット */
//#endif
	unsigned char		*tex_image[2] ;			/* テクセル展開バッファ */
	unsigned char		*tex_clut[2] ;			/* ＣＬＵＴ展開バッファ */

#ifdef KP_WINDOWS
	DWORD				last_frame_pcntr ;		/* 最後に処理した切替カウンタ値 */
#endif
} ANMTEX_WORK ;

enum {
	/* 動作方法指定用 */
	ANMTEX_FLAG_LOOP		= 0x00000001,		/* ループ再生 */
	/* 動作制御用 */
	ANMTEX_FLAG_SLEEP		= 0x00000100,		/* 再生の停止 */
};


extern int UTL_InitAnmtexWork( ANMTEX_WORK *anmtex_work, int tri_id, int tex_id, int anmtex_id, int flag );
extern void UTL_FreeAnmtexWork( ANMTEX_WORK *anmtex_work );
extern int UTL_ActAnmtex( ANMTEX_WORK *anmtex_work );
