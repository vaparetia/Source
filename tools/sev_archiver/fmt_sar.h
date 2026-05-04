/*

	SAR(sequence data archiver) フォーマット


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
	unsigned short	time ;			/* シーケンスタイム（１／３００秒単位） */
	unsigned char	next_offset ;	/* 次のシーケンスデータまでのオフセット（バイト単位） */
	unsigned char	id ;			/* 効果音ID */
} SEV_ELEMENT ;

typedef struct {
	unsigned short	se_code ;
	unsigned char	mode ;
	unsigned char	joint ;
	unsigned short	length ;
	unsigned short	flags ;
} SEV_PARAM ;

typedef struct {
	int			n_data ;
	SEV_PARAM	params[ NULL_ARRAY ] ;
} SEV_LOCAL_HEADER ;


/* ---------------------------------------------------------------- */
	/*
		SARデータフォーマット
	 */
typedef struct {
	unsigned short	time ;			/* シーケンスタイム（１／３００秒単位） */
	unsigned char	next_offset ;	/* 次のシーケンスデータまでのオフセット（バイト単位） */
	unsigned char	pad ;
} SAR_ELEMENT ;

typedef struct {
	int		id ;				/* 元ファイルID（０ならエントリが存在しない） */
	int		offset ;			/* データ部からのオフセット */
	int		size ;				/* データサイズ */
	int		local_header_size ;	/* ローカルヘッダサイズ（バイト単位） */
} SAR_LIST ;

typedef struct {
	int		format_id ;	/* データフォーマットID */
	int		n_datas ;	/* 格納データ数 */
	int		data_size ;	/* データサイズ（ヘッダ及びリスト情報除く） */
	int		pad[5] ;
	SAR_LIST	list[ NULL_ARRAY ] ;
} SAR_HEADER ;

#define SAR_FORMATID_SEV	(87286)/* "SEV" */

/* ---------------------------------------------------------------- */


#ifdef __cplusplus
}
#endif


#endif

