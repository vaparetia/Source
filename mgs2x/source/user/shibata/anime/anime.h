/*

	anime.h
	２Ｄアニメスクリプトインタプリタヘッダ
	1999/07/07 S.Okajima
	$Id: anime.h,v 1.1.1.3 2002/11/19 11:48:28 Yoshizawa1 Exp $

*/
#ifndef ANIME_H
#define ANIME_H

typedef struct	{
	FVECTOR		pos ;				/* 位置 */
	FVECTOR		speed ;				/* スピード */
	short		scr_num ;			/* スクリプト番号 */
	short		s_anim ;			/* パターン番号 */
} PRESCRIPT ;

typedef	struct	{
	int		tex;				/* テクスチャ番号 */
	short		texdev_x, texdev_y ;	/* テクスチャ分割数 */
	short		n_anime ;				/* アニメパターン数 */
	short		raise ;					/* 表示優先 */
	short		size_w, size_h ;		/* サイズ */
	short		v ;				/* 輝度値 */
	short           amb ;                           /* 半透明属性(田中) */
	short           mode ;                          /* モード(田中) （フラグ） */
	PRESCRIPT	*pre_script ;			/* スクリプト前処理 */
	char		*script ;				/* スクリプト */
} ANIMATION ;


/* in anime.c */
extern void *NewAnime( FMATRIX *world, int where, ANIMATION *anmform );
extern void *NewAnimeWithTrgt( FMATRIX *world, int where, ANIMATION *anmform, TARGET *trg, FVECTOR *pos, FVECTOR *size );

#define AN_MODE_SIZENOEM 	(0x00)	//サイズ ×1.0f
#define AN_MODE_SIZEHARF 	(0x01)	//サイズ ×0.5f
#define AN_MODE_SIZEDBL 	(0x02)	//サイズ ×2.0f
//#define AN_MODE_SIZE 	(0x03)	//
#define AN_MODE_SIZECHECK 	(0x03)	//サイズチェック
#define	AN_MODE_PWATCHINV	(0x04)	//主観時 見えない
#define AN_MODE_POSONLY 	(0x08)	//マトリクスのポジションだけ使用


#define AN_ALPHA_ADD		(0)
#define AN_ALPHA_SUB		(1)
#define AN_ALPHA_ALPHA		(2)
#define AN_ALPHA_TEX		(-1)
#define AN_PRIMTYPE_FOG		(0x0010)

#endif
