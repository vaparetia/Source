/*
	rootene.h
	確認兵（船倉用）

	2000/12/20 Y.Korekado
	$Id: rootene.h,v 1.1.1.2 2002/11/03 17:07:58 Yoshizawa1 Exp $
*/

#include	"chkmot.h"

typedef	struct	{
	int			*flag ;			/* フラグ */
	CNCTOBJ		*cnct ;
} ROOTENEWORK ;

typedef	struct	{
	GV_ACT_EX		actor ;

	/* 暫定モーションデータ */
	int			motion_data[ MOTION_MAX ];

	/* アクション */
	ACTION		action ;

	/* ルートナビ */
	ROUTENAVI	routenavi ;

	/* ゾーンナビ */
	ZONENAVI	zonenavi ;

	/* 敵兵思考 */
	ENETHINK	enethink ;

	/* 固有ワーク */
	ROOTENEWORK	rtwork ;

	FVECTOR		return_pos ;	/* 戻る場所 */
	int			return_dir ;	/* 戻る向き */
	int			*flag ;			/* フラグ */
	FVECTOR		*abs_rots ;		/* 立ち状態の時のabs_rots。受け渡し時の補完に利用 */
	HOLD_ENE 	*hold ;
	int			gameover_delay ;
} Work ;



#define TH3_ALLEND	0x100000
