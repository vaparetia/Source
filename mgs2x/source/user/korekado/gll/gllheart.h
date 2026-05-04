/*
	gllheart.h
	ごるるごん心臓ヘッダ

	2002/07/09 Y.Korekado
	$Id: gllheart.h,v 1.1.1.3 2002/11/19 11:44:13 Yoshizawa1 Exp $
*/
/* ハート側 */
#define GLL_HEART_DESTROY	0x00000001	/* 破壊された */
#define GLL_HEART_DAMAGE	0x00000002	/* ダメージを受けた */

/* 本体側 */
#define GLL_BODY_DAMAGE		0x00010000	/* ダメージを受けた */
#define GLL_BODY_BREAK		0x00020000	/* 壊わされた */
#define GLL_BODY_SPCLEAR	0x00040000	/* 特殊クリア */
