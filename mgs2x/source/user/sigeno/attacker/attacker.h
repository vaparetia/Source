/*
	attacker.h
	attacker用 各種宣言

	1999/07/06 K.Sigeno
	$Id: attacker.h,v 1.1.1.3 2002/11/19 11:49:01 Yoshizawa1 Exp $
*/
/***TEST***/
#ifndef __WATCHER___
#define	__WATCHER___ 1
#define PRIM_NUM 24

/*-----	外部変数	-----*/
#if 0
extern	int		GM_PlayerAddress ;
extern	FVECTOR	GM_PlayerPosition ;
extern	CONTROL	*GM_PlayerControl ;
#endif

/*------*/

#include "at_enum.h"
#include "at_thk.h"




/*-----	定数定義	-----*/


/*----- ワーク定義 -----*/
typedef	struct	{
	GV_ACT_EX		actor ;
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
#if 1	//kore
	/* クリアリングルートナビ */
	ROUTENAVI	cl_route ;
#endif
	/* ゾーンナビ */
	ZONENAVI	zonenavi ;
	/* 敵兵思考 */
	ENETHINK	enethink ;
	/*攻撃兵固有思考*/
	AT_THK		at_thk;
	/*実験用Prim*/
#ifdef DEBUG_PRIM
	DG_PRIM		*prim;
	FVECTOR		primpos[PRIM_NUM][4]; /*頂点数*/
	FVECTOR		checkpos ;
#endif
	int			sw; /*ハンドサイン実験*/
	/*雪洞*/
//	int			blur_sw ;
} Work ;

#endif


//#define AT_WARP_DIS			(12000/250)	/*ワープ距離*/
//#define AT_WARP_DIS			(16)
#define AT_WARP_DIS			(30)

#define	AT_V_NIGETAZO	(300)
#define	AT_V_KURAE		(120)
#define	AT_V_SOKONI		(300)
#define	AT_V_NIGETA		(300)
#define	AT_V_UTUNA		(480)
#define	AT_V_TOMARE		(480)

