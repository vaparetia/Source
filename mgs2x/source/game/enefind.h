/*
	eenfind.h
	敵兵発見物ヘッダ
	
	2000/06/09 Y.Korekado
	$Id: enefind.h,v 1.1.1.3 2002/11/19 11:41:48 Yoshizawa1 Exp $
	
*/

#ifndef __ENEFIND___
#define	__ENEFIND___ 1

typedef	struct _ENEFIND {
	FVECTOR		pos ;			/* 場所		*/
	int			zoneaddr ;		/* ゾーンアドレス	*/
	int			type ;			/* タイプ	*/

    struct _ENEFIND	*next ;
} ENEFIND ;

/* type */
#define	EF_TYPE_NO_FIND		0x00000001	/* 敵が発見しない */
#define	EF_TYPE_LV1			0x00000002	/* 発見Lv1 ちらっと見やる */
#define	EF_TYPE_LV2			0x00000004	/* 発見Lv2 調べに行って「？」 */
#define	EF_TYPE_LV3			0x00000008	/* 発見Lv3 調べに行って探索モード */
#define	EF_TYPE_ITEM		0x00000010	/* アイテム、見つけたら拾う */
#define	EF_TYPE_ADULT		0x00000020	/* 大人のアイテム */

#endif
