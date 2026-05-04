/*
	fmt_sar.h
	SAR(sequence data archiver) フォーマット

	1999/12/06 K.Takabe
	$Id: fmt_sar.h,v 1.1.1.3 2002/11/19 11:41:59 Yoshizawa1 Exp $
*/


#ifndef __FMT_SAR_H__
#define __FMT_SAR_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif

/* ---------------------------------------------------------------- */
	/*
		SAR形式SEVファイルフォーマット
	 */
typedef struct {
	unsigned short	time_le ;			/* シーケンスタイム（１／３００秒単位） */
	unsigned char	next_offset ;	/* 次のシーケンスデータまでのオフセット（バイト単位） */
	unsigned char	id ;			/* 効果音ID */
} SEV_ELEMENT ;

typedef struct {
	unsigned short	se_code_le ;
	unsigned char	mode ;
	unsigned char	joint ;
	unsigned short	length_le ;
	unsigned short	flags_le ;
} SEV_PARAM ;

typedef struct {
	int			n_data_le ;
	SEV_PARAM	params[ NULL_ARRAY ] ;
} SEV_LOCAL_HEADER ;


/* ---------------------------------------------------------------- */
	/*
		SARデータフォーマット
	 */
typedef struct {
	unsigned short	time_le ;	/* _le means little endian */ 		/* シーケンスタイム（１／３００秒単位） */
	unsigned char	next_offset ;	/* 次のシーケンスデータまでのオフセット（バイト単位） */
	unsigned char	pad ;
} SAR_ELEMENT ;

typedef struct {
	int		id ;				/* 元ファイルID（０ならエントリが存在しない） */
	int		offset ;			/* データ部先頭からのオフセット（読み込み時にアドレスへ変換） */
	int		size ;				/* データサイズ */
	int		local_header_size ;	/* ローカルヘッダサイズ（バイト単位） */
} SAR_LIST ;

typedef struct {
	int			format_id ;	/* データフォーマットID */
	int			n_datas ;	/* 格納データ数 */
	int			data_size ;	/* データサイズ（ヘッダ及びリスト情報除く） */
	int			pad[5] ;
	SAR_LIST	list[ NULL_ARRAY ] ;
} SAR_HEADER ;


#define SAR_FORMATID_SEV	(87286)/* "SEV" */



/*
	データフォーマット解説

	ＳＡＲデータの並びは以下のようになっている

	[SAR_HEADER]
	[SAR_LISTINFO(0)]
	[SAR_LISTINFO(1)]
	[SAR_LISTINFO(2)]
	.
	.
	.
	[SAR_LISTINFO(n_datas-1)]
	[データ部分（任意サイズ）]


	＜データ部分構造＞
	データ部分はSAR_LISTINFOで指定されたサイズのローカルヘッダの下に
	各シーケンスデータ（可変サイズ）が連続する形になる
	offset + 0                                      : ローカルヘッダ（local_header_sizeバイト）
	offset + local_header_size                      : シーケンスデータ（size(0)バイト）
	offset + local_header_size + size(0) + size(1)  : シーケンスデータ（size(1)バイト）
	.
	.
	.
	

*/

/* ---------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif


