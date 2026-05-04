/*
	teng_a.h
	天狗兵Ａ 各種宣言

	2001/02/02 K.Sigeno
	$Id: tng_a.h,v 1.1.1.3 2002/11/19 11:49:53 Yoshizawa1 Exp $
*/


#define ALERT_SET

/*-----	外部変数	-----*/
extern	int		GM_PlayerAddress ;
extern	FVECTOR	GM_PlayerPosition ;
extern	CONTROL	*GM_PlayerControl ;


/*思考切り替える時間定義*/
#define LEVEL_COUNT	((AT_THK_RATE*10)*10) /*10秒*/
/*標準視力*/
//#define DEF_DEF_EYE_SIGHT	(10000)
#define DEF_DEF_EYE_SIGHT	(14000)

#include "../../kira/effect/floor/floor.h"
/*----- ワーク定義 -----*/
typedef	struct	{
	GV_ACT_EX	actor ;
	CONTROL		control ;
	OBJECT		body ;
	OBJECT		weapon ;
	OBJECT		sub_weapon ;
	FMATRIX		lights[2] ;
	int			name;
	int			id ;
	/* 暫定モーションデータ */
//	int			motion_data[ MOTION_MAX ];
	/* アクション */
	ACTION		action ;
	/* ルートナビ */
	ROUTENAVI	routenavi ;
	/* クリアリングルートナビ */
	ROUTENAVI	cl_route ;
	/* ゾーンナビ */
	ZONENAVI	zonenavi ;
	/* 敵兵思考 */
	ENETHINK		enethink ;
	AT_THK			at_thk ;
	ENTK_TENG_A		tng_a ;
	HEX_FLOOR_INFO   hex_info; /*ヘックス影ワーク*/
	CAPTURE_TARGET	capture ;
//	int			blur_sw ;
} Work ;
