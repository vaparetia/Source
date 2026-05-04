/*
	checkene.h
	確認兵（船倉用）

	2000/03/28 Y.Korekado
	$Id: checkene.h,v 1.1.1.3 2002/11/19 11:44:16 Yoshizawa1 Exp $
	
*/

#include	"chkmot.h"

typedef	struct	{
	int			*flag ;			/* フラグ */
	CNCTOBJ		*cnct ;
} CHKENEWORK ;

typedef	struct	{
	GV_ACT_EX		actor ;

	/* 暫定モーションデータ */
	int			motion_data[ MOTION_MAX ];

	/* アクション */
	ACTION		action ;

	/* ゾーンナビ */
	ZONENAVI	zonenavi ;

	/* 敵兵思考 */
	ENETHINK	enethink ;

	/* 固有ワーク */
	CHKENEWORK	chkwork ;

	FVECTOR		return_pos ;	/* 戻る場所 */
	int			return_dir ;	/* 戻る向き */
	int			*flag ;			/* フラグ */
	FVECTOR		*abs_rots ;		/* 立ち状態の時のabs_rots。受け渡し時の補完に利用 */
	HOLD_ENE	*hold ;
	int			gameover_delay ;
} Work ;

extern SVECTOR HOLD_WP_Rot[] ;
extern FVECTOR HOLD_WP_Shift[] ;




#define TH3_ALLEND	0x100000
