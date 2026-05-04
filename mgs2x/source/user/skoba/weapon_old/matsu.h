/*
	matsu.h
	

	1999/11/15 M.Matsuzaki


*/

#ifndef ___MATSUZAKI___
#define ___MATSUZAKI___

#ifdef MATSUZAKI_SOURCE

/* ------------------------------------------------------- */
/* include */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <libpkt.h>
#endif


#include "libdg.h"
#include "libdg.cnf"
#include "libmt.h"
#include "gameheader.h"
#include "camera.h"
#include "libutl.h"

#include "utl_dma.h"
//#include "def_dma.h"


/* ------------------------------------------------------- */
/* debug switch */

/* ------------------------------------------------------- */
/* define */

#define ALIGNSIZE1(_n) (_n)
#define ALIGNSIZE16(_n) (((_n)+15)&0xfffffff0)
#define ALIGNSIZE64(_n) (((_n)+63)&0xffffffc0)
#define ALIGNSIZE128(_n) (((_n)+127)&0xffffff80)

/* ボタンマクロの再定義 */
#define PADLup     (1<<12)
#define PADLdown   (1<<14)
#define PADLleft   (1<<15)
#define PADLright  (1<<13)
#define PADRup     (1<< 4)
#define PADRdown   (1<< 6)
#define PADRleft   (1<< 7)
#define PADRright  (1<< 5)
#define PADL3      (1<< 9)
#define PADR3      (1<<10)
#define PADselect  (1<< 8)
#define PADR1      (1<< 3)
#define PADR2      (1<< 1)
#define PADL1      (1<< 2)
#define PADL2      (1<< 0)
#define PADstart   (1<<11)




/* ------------------------------------------------------- */
/* ファイル別 */

/* --- goggle.c --- */
enum {			/* ゴーグルエフェクトID */
	GOGGLE_ID_NOMAL,	/* 暗視 */
	GOGGLE_ID_IR,		/* 赤外線 */
	GOGGLE_ID_X,		/* Ｘ線 */
	GOGGLE_WITHOUT,	/* デバッグ用 */
	CLUTMODE_MAX
};

#ifdef XBOX

#if 0

/* ゴーグル用ワークテンプレート */
typedef	struct	{
	GV_ACT		actor ;
	
	DG_PRIM2	*primScrn ;			/* プリミティブ */
	DG_PRIM2	*primLine ;			/* プリミティブ */
	
	int 		mode ;		 	 	/* モード */
	
	int			timer ;				/* タイマー */
	
	void		*tmp_clut_addr ;	/* 第二CLUTバッファアドレス */
	int 		use_block;			/* バッファ使用ブロック数 */
	
} GOGGLE_Work ;

#else

/* ゴーグル用ワークテンプレート  */
typedef	struct	{
	GV_ACT		actor ;
	
	DG_PRIM		*primScrn ;			/* プリミティブ  */
	DG_PRIM		*primLine ;			/* プリミティブ  */
	
	int 		mode ;		 	 	/* モード  */
	
	int			timer ;				/* タイマー  */
	
	void		*tmp_clut_addr ;	/* 第二CLUTバッファアドレス  */
	int 		use_block;			/* バッファ使用ブロック数  */
	
} GOGGLE_Work ;

#endif

#endif /* XBOX */

#endif




/* ------------------------------------------------------- */
/* 武器系エフェクト用 */

/* ---武器エフェクト管理構造体--- */
typedef struct _WEAPON_EF_CTRL {
	int 		motion;						/* モーションタイマー */
	int			reload_wait;				/* リロード中のウェイト用(usp) */
	int 		kind;						/* 武器種類 */
	OBJECT		*root;						/* rootオブジェクト */
	DG_OBJS		*pobjs;						/* 武器モデル */
	u_int 		flag;						/* フラグ */
	u_int 	   	flag2;						/* 内部フラグ */
	u_int 	   	flag3;						/* 特殊フラグ */

	float		ftemp;
	int			count;
} WEAPON_EF_CTRL ;

/* ---フラグ--- */
#define WPEF_FLG_START		(0x00000001)	/* モーション再生を開始します */
#define WPEF_ENEMY_UNREAL   (0x00000800)

/* ---関数プロトタイプ--- */

/* 武器オブジェクトを初期化 ＆ 武器エフェクトの使用準備 */
/* (戻り値) 0:初期化成功	-1:初期化成功 */
int WeaponEfInitObject	(
		 WEAPON_EF_CTRL *,		/* 呼ぶ側のworkにとった管理構造体のポインタ */
		 OBJECT *,				/* 初期化するオブジェクトのポインタ */
		 OBJECT *,				/* rootオブジェクトポインタ */
		 int ,					/* くっつきユニット番号 */
		 int ,					/* 武器種類(g_define.h参照) */
		 int					/* その武器のモデルコード */
		 );

/* 解説:武器エフェクトのメイン */
/* (戻り値)なし */
void WeaponEfAct	(
		 WEAPON_EF_CTRL *		/* 呼ぶ側のworkにとった管理構造体のポインタ */
		 );

/* ------------------------------------------------------- */
/* ------------------------------------------------------- */
/* ------------------------------------------------------- */
/* ------------------------------------------------------- */
/* ------------------------------------------------------- */


#endif
