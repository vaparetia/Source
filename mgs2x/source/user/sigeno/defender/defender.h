/*
	defender.h
	長廊下兵用 各種宣言

	1999/07/06 K.Sigeno
	$Id: defender.h,v 1.1.1.3 2002/11/19 11:49:09 Yoshizawa1 Exp $
*/

//#define FACE_TEST 
//#define DEBUG_PRIM 
#define PRIM_NUM 1

#define ALERT_SET



/*-----	外部変数	-----*/
extern	int		GM_PlayerAddress ;
extern	FVECTOR	GM_PlayerPosition ;
extern	CONTROL	*GM_PlayerControl ;

//#include "../attacker/at_enum.h"
#include "def_enum.h"

/*思考切り替える時間定義*/
#define LEVEL_COUNT	((AT_THK_RATE*10)*10) /*10秒*/
/*標準視力*/
//#define DEF_DEF_EYE_SIGHT	(10000)
#define DEF_DEF_EYE_SIGHT	(14000)

/*-----	定数定義	-----*/

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
	ENTK_TYPE_A		entk_a ;
	/*実験用Prim*/
#ifdef DEBUG_PRIM
	DG_PRIM		*prim;
	FVECTOR		primpos[PRIM_NUM][4]; /*頂点数*/
#endif
} Work ;
