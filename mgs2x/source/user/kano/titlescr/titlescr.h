/*
	titlescr.h
		タイトル画面の表示マネージャー

	2001/06/12 K.Kano
	$Id: titlescr.h,v 1.1.1.3 2002/11/19 11:43:42 Yoshizawa1 Exp $
*/

#ifndef _titlescr_h_
#define _titlescr_h_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"

////#ifdef PSX2
#include "../mcman/mcman.h"
////#endif

#include "../resman/resman.h"

#include "mode/menu/xtextscn.h"

#define LANG_MAX_NUM	7 /* サブスタンス用決めうち */

extern int BP_BASE_TICK(void);
extern int BP_FRAMES_PER_SEC(void);
#define DIV_TICK_VALUE		(BP_BASE_TICK())
#define FRAMES_PER_SEC		(BP_FRAMES_PER_SEC())


/* text_scn.c */

/* プロトタイプ宣言 */
/* 説明分テキストを初期化する */
void MENU_ClearTextTexture( void *work );
void MENU_ClearPartTextTexture( void *work, int start_line, int height );
/* 説明文を表示する */
void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );

/* テキスト用テクスチャをVRAMへ転送 */
void MENU_TransTextTexture(void *work_ptr);
/* 説明文を表示する */
void MENU_PutTextScreenNoTR( void *work_ptr, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );

/* テキストテクスチャ管理デーモン */
void *NewTextScreenControl( void );
void *NewTextScreenControlForTitle( void );
void *NewTextScreenControlForCodec( void );



enum {
	PROC_COM_OK=0,
	PROC_COM_CANCEL,
};

enum {
	PROC_COM_GAMESEL_CANCEL=-1,

	PROC_COM_GAMESEL=0,
	PROC_COM_NEWGAME,
	PROC_COM_OPTION,
};

enum {
	TITLE_NONE=0,

	TITLE_GAMESEL,
	TITLE_NEWGAME,
	TITLE_LOADGAME,
	TITLE_OPTION,
	TITLE_SPECIAL,
	TITLE_MISSIONS,
	TITLE_SNAKETALES,
	TITLE_TRANSFARRING,
   TITLE_BACKTOGAMESELECT,
};

enum {
	TITLE_MSG_NONE=0,
	TITLE_MSG_MODEEND,
	TITLE_MSG_MODECANCEL,
};

enum {
	TITLE_CHANL=4,
};

enum {
	TITLE_PRI_FRAME=0,
	TITLE_PRI_NORMAL=0,
};

#define N_PROC_ARGS		6


#define SEL_ALPHA		128
#define UNSEL_ALPHA		52


#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */
#define CODE_SELCURSNULL		0x006719f0		/* selCursNull */
#define CODE_SELCURS			0x00e38043		/* selCurs */


/* タイトルロゴ */
#define CODE_SHOWLOGO			0x009b7c34		/* showLogo */
#define CODE_HIDELOGO			0x007b245d		/* hideLogo */
#define CODE_SHOWLOGO2			0x006f86c5		/* showLogo2 */
#define CODE_HIDELOGO2			0x00648be1		/* hideLogo2 */
#define CODE_BLINKSTART			0x001af71c		/* blinkStart */

#define CODE_SETLOGO			0x0067e6e0		/* setLogo */
#define CODE_SETLOGO2			0x00fcdc3e		/* setLogo2 */


/* メインメニュー表示/消去 */
#define CODE_SHOWMAIN			0x00dbdf20		/* openMainSlant1 */
#define CODE_BACKMAIN			0x00dbdf21		/* openMainSlant2 */
#define CODE_HIDEMAIN			0x0095d615		/* closeMainSlant1 */
#define CODE_TONEWGAME			0x0035facb		/* closeMainToTop */
#define CODE_TOLOADGAME			0x0095d615		/* closeMainSlant1 */
#define CODE_TOOPTIONS			0x00c8164b		/* closeMainToRight */
#define CODE_TOSPECIAL			0x0095d615		/* closeMainSlant1 */
#define CODE_TOMISSIONS			0x0095d615		/* closeMainSlant1 */
#define CODE_TOSNAKETALES		0x0095d615		/* closeMainSlant1 */
#define CODE_TOTRANSFARRING	0x00c8164b		/* closeMainToRight */
#define CODE_TOGAMESELECT		0x0095d615		/* closeMainSlant1 */

#define CODE_MENUNEW			0x001be128		/* menuNew */
#define CODE_MENULOAD			0x007b4aa7		/* menuLoad */
#define CODE_MENUOPT			0x001be685		/* menuOpt */
#define CODE_MENUSP				0x0088df90		/* menuSP */
#define CODE_MENUMIS			0x0069cd60		/* missions */
#define CODE_MENUSNA			0x0096b0f2		/* snake_tales */
#define CODE_MENUXFAR      0x001bf5d3     /* menuXfar */     // todo: update this strcode to 8126b5
#define CODE_MENUSKB			0x001bf5d3		/* menuSkb */

/* ステージ選択表示/消去 */
#define CODE_SHOWSELGAME		0x000ab6c2		/* openNewSlant3 */
#define CODE_NEXTFROMSELGAME	0x00d5e0fc		/* closeNewToLeft */
#define CODE_HIDESELGAME		0x004688eb		/* closeNewToBtm */

#define CODE_MENU1ST			0x001b6ee5		/* menu1st */
#define CODE_MENUTANKER			0x0051470e		/* menuTanker */
#define CODE_MENUPLANT			0x00a7d6a3		/* menuPlant */
#define CODE_MENUT_P			0x001bf841		/* menuT_P */

#define CODE_SHOWSELGAME_AFTERCLEAR			0x000ab6c0		/* openNewSlant1 */
#define CODE_SHOWSELGAME_FROMDIF_AFTERCLEAR	0x000ab6c1		/* openNewSlant2 */
#define CODE_BACKLOADGAME_AFTERCLEAR		0x00b08677		/* closeNewSlant1 */
#define CODE_HIDESELGAME_AFTERCLEAR			0x001e228f		/* closeNewToRight */
#define CODE_SHOWDIFALL_AFTERCLEAR			0x00c7c9a5		/* openDifAllSlant3 */
// #define CODE_HIDEDIFALL_AFTERCLEAR			0x00a6a237		/* closeDifSlant1 */
#define CODE_HIDEDIFALL_AFTERCLEAR			0x00cbfcbc		/* closeDifToLeft */
#define CODE_HIDEDIFALLTOSEL_AFTERCLEAR		0x00cbfcbc		/* closeDifToLeft */
#define CODE_SHOWRADTYPE_AFTERCLEAR			0x0010a554		/* openRadTypeSlant2 */
#define CODE_HIDERADTYPE_AFTERCLEAR			0x008b024e		/* closeRadTypeSlant1 */
#define CODE_HIDERADTYPE_TO_GAMEOVER_AFTERCLEAR	0x00fda42c		/* closeRadTypeToTop */
#define CODE_BACKFROMRADTYPE_AFTERCLEAR		0x006d9d6a		/* closeRadTypeToRight */
#define CODE_SHOWGAMEOVER_AFTERCLEAR		0x006e2742		/* openGameOverSlant3 */
#define CODE_HIDEGAMEOVER_AFTERCLEAR		0x00b9c6b0		/* closeGameOverSlant2 */
#define CODE_BACKFROMGAMEOVER_AFTERCLEAR	0x0006d2ed		/* closeGameOverToBtm */


/* 井上喜久子アンケート */
#define CODE_SHOWQST			0x000e2601		/* openQstSlant2 */
#define CODE_NEXTFROMQST		0x0046ebc8		/* closeQstToTop */
#define CODE_HIDEQST			0x008c0a8f		/* closeQstToRight */

/* 難易度設定表示/消去 */
#define CODE_SHOWDIF			0x0000d281		/* openDifSlant2 */
#define CODE_HIDEDIF			0x00a6a238		/* closeDifSlant2 */

#define CODE_SHOWDIFALL			0x00c7c9a4		/* openDifAllSlant2 */
#define CODE_SHOWDIFALLFROMQST	0x00c7c9a5		/* openDifAllSlant3 */
#if 0
#define CODE_HIDEDIFALLTONEXT	0x00a6a238		/* closeDifSlant2 */
#define CODE_HIDEDIFALLTONEXT1	0x00a6a237		/* closeDifSlant1 */
#else
#define CODE_HIDEDIFALLTONEXT	0x00cbfcbc		/* closeDifToLeft */
#define CODE_HIDEDIFALLTONEXT1	0x0046812c		/* closeDifToTop */
#endif
#define CODE_HIDEDIFALLTOSEL	0x00e19a8d		/* closeDifToRight */
// #define CODE_HIDEDIFALLTOQST	0x0046812c		/* closeDifToTop */
#define CODE_HIDEDIFALLTOQST	0x004639c9		/* closeDifToBtm */
#define CODE_SHOWDIFEASY		0x0017e65a		/* openDifEasySlant2 */
#if 0
#define CODE_HIDEDIFEASYTONEXT	0x00a6a238		/* closeDifSlant2 */
#define CODE_HIDEDIFEASYTONEXT1	0x00a6a237		/* closeDifSlant1 */
#else
#define CODE_HIDEDIFEASYTONEXT	0x00cbfcbc		/* closeDifToLeft */
#define CODE_HIDEDIFEASYTONEXT1	0x0046812c		/* closeDifToTop */
#endif
#define CODE_HIDEDIFEASYTOSEL	0x00e19a8d		/* closeDifToRight */
// #define CODE_HIDEDIFEASYTOQST	0x0046812c		/* closeDifToTop */
#define CODE_HIDEDIFEASYTOQST	0x004639c9		/* closeDifToBtm */
#define CODE_SHOWDIFHARD		0x0077d91a		/* openDifHardSlant2 */
#if 0
#define CODE_HIDEDIFHARDTONEXT	0x00f23615		/* closeDifHardSlant2 */
#define CODE_HIDEDIFHARDTONEXT1	0x00f23614		/* closeDifHardSlant1 */
#else
#define CODE_HIDEDIFHARDTONEXT	0x0017909a		/* closeDifHardToLeft */
#define CODE_HIDEDIFHARDTONEXT1	0x0030ddcb		/* closeDifHardToTop */
#endif
#define CODE_HIDEDIFHARDTOSEL	0x00541637		/* closeDifHardToRight */
// #define CODE_HIDEDIFHARDTOQST	0x0030ddcb		/* closeDifHardToTop */
// #define CODE_HIDEDIFHARDTOQST	0x004639c9		/* closeDifToBtm */
#define CODE_HIDEDIFHARDTOQST	0x00309668		/* closeDifHardToBtm */

#define CODE_SHOWDIFALLFROMMAIN	0x00c7c9a5		/* openDifAllSlant3 */
#define CODE_HIDEDIFALLTOMAIN	0x004639c9		/* closeDifToBtm */	


#define CODE_MENUVEASY			0x002ca0d9		/* menuV_easy */
#define CODE_MENUEASY			0x007794fc		/* menuEasy */
#define CODE_MENUNORMAL			0x00334e7d		/* menuNormal */
#define CODE_MENUHARD			0x007914c7		/* menuHard */
#define CODE_MENUVHARD			0x002e20a4		/* menuV_hard */
#define CODE_MENUEEX			0x001bb909		/* menuEEX */

#define CODE_SWEEX				0x002b1be2		/* EEX_null */


/* 見つかりモード表示/消去 */
#define CODE_SHOWFIND			0x000bfc45		/* openFound */
#define CODE_BACKFROMFIND		0x0068f7bf		/* closeFound */
#define CODE_HIDEFIND			0x00a6a238		/* closeDifSlant2 */

#define CODE_SHOWYESNO			0x0036f051		/* openYesNo */
#define CODE_HIDEYESNO			0x0093ebcb		/* closeYesNo */

#if 0
#define CODE_MENUON				0x0088df2e		/* menuOn */
#define CODE_MENUOFF			0x001be537		/* menuOff */
#else
#define CODE_MENUON				0x001c0d24		/* menuYes */
#define CODE_MENUOFF			0x0088df0f		/* menuNo */
#endif


/* 追加  2001/9/7  Ken Kano
   「レーダータイプ」「見つかるとゲームオーバー」のアクションを追加 */

#define CODE_SHOWGAMEOVER				0x006e2742		/* openGameOverSlant3 */
#define CODE_HIDEGAMEOVER				0x00b9c6b0		/* closeGameOverSlant2 */
#define CODE_BACKFROMGAMEOVER			0x0006d2ed		/* closeGameOverToBtm */

#define CODE_SHOWGAMEOVER1				0x006e2741		/* openGameOverSlant2 */
#define CODE_HIDEGAMEOVER1				0x00b9c6af		/* closeGameOverSlant1 */
#define CODE_BACKFROMGAMEOVER1			0x00462990		/* closeGameOverToRight */
#define CODE_HIDEGAMEOVER1TODOGTAGS		0x00071a50		/* closeGameOverToTop */

#define CODE_SHOWRADTYPE				0x0010a554		/* openRadTypeSlant2 */
#define CODE_HIDERADTYPE				0x008b024e		/* closeRadTypeSlant1 */
#define CODE_HIDERADTYPE_TO_GAMEOVER	0x00fda42c		/* closeRadTypeToTop */
#define CODE_BACKFROMRADTYPE			0x006d9d6a		/* closeRadTypeToRight */

#define CODE_SHOWRADTYPE1				0x0010a555		/* openRadTypeSlant3 */
#define CODE_HIDERADTYPE1				0x008b024f		/* closeRadTypeSlant2 */
#define CODE_HIDERADTYPE1_TO_GAMEOVER	0x00b05cd3		/* closeRadTypeToLeft */
#define CODE_BACKFROMRADTYPE1			0x00fd5cc9		/* closeRadTypeToBtm */

/* 追加  2002/9/26  Yano
   「ドックタグ」のアクションを追加 */
#define CODE_SHOWDOGTAGS1				0x00fa707b		/* openDogTagSlant1 */
#define CODE_SHOWDOGTAGS2				0x00fa707c		/* openDogTagSlant2 */
#define CODE_SHOWDOGTAGS3				0x00fa707d		/* openDogTagSlant3 */
#define CODE_BACK_TO_GAMEOVER			0x00a7b6d2		/* closeDogTagToBtm */
#define CODE_BACK_TO_RADAR				0x00d5c013		/* closeDogTagToRight */
#define CODE_HIDEDOGTAGS1				0x00d64363		/* closeDogTagSlant1 */
#define CODE_HIDEDOGTAGS2				0x00d64364		/* closeDogTagSlant2 */

#define CODE_DOGTAGS2001			0x0019c631		/* 2001 */
#define CODE_DOGTAGS2002			0x0019c632		/* 2002 */



#define CODE_RADARTYPE1			0x00228609		/* radarType1 */
#define CODE_RADARTYPE2			0x0022860a		/* radarType2 */
#define CODE_RADARTYPE3			0x0022860b		/* radarType3 */

#define CODE_NOTGAMEOVER		0x00cfb7f8		/* notGameOver */
#define CODE_GAMEOVER			0x007cb632		/* gameOver */


/* オプション表示/消去 */
#if 1
#define CODE_SHOWOPT			0x000c0e03		/* openOptSlant4 */
#define CODE_HIDEOPT			0x00d7383c		/* closeOptToLeft */
#define CODE_BACKOPT			0x000c0e02		/* openOptSlant3 */
#else
#define CODE_SHOWOPT			0x000c0e00		/* "openOptSlant1" */
#define CODE_HIDEOPT			0x00b1ddb7		/* "closeOptSlant1" */
#endif


#if 0

#define CODE_MENUVIB			0x001c0193		/* menuVib */
#define CODE_MENURAD			0x001bf095		/* menuRad */
#define CODE_MENUBLD			0x001bb1f5		/* menuBld */
#define CODE_MENUSND			0x001bf635		/* menuSnd */
#define CODE_MENUCAP			0x001bb4a1		/* menuCap */
#define CODE_MENUSCRN			0x007e9cd1		/* menuScrn */
#define CODE_MENUCOL			0x001bb65d		/* menuCol */

#define CODE_MENUEXIT			0x0077efb7		/* menuExit */

#else

#define CODE_MENUVIB			0x003b6667		/* optVib */
#define CODE_MENURAD			0x003b5569		/* optRad */
#define CODE_MENUBLD			0x003b16c9		/* optBld */
#define CODE_MENUSND			0x003b5b09		/* optSnd */
#define CODE_MENU5_1CH			0x0083e193		/* opt5_1CH */
#define CODE_MENUCAP			0x003b1975		/* optCap */

#define CODE_MENUOWN			0x003b4813		/* optOWN */
#define CODE_MENUITEM			0x0065f594		/* optITEM */
#define CODE_MENUQUICK			0x003f4198		/* optQUICK */

#define CODE_MENUSCRN			0x006b3755		/* optScrn */
#define CODE_MENUCOL			0x003b1b31		/* optCol */

#define CODE_MENUEXIT			0x0077efb7		/* menuExit */

#endif

#if 0

#define CODE_MENUVIB_COL		0x003057cc		/* menuColon1 */
#define CODE_MENURAD_COL		0x003057cd		/* menuColon2 */
#define CODE_MENUBLD_COL		0x003057ce		/* menuColon3 */
#define CODE_MENUSND_COL		0x003057cf		/* menuColon4 */
#define CODE_MENUCAP_COL		0x003057d0		/* menuColon5 */

#else

#define CODE_MENUVIB_COL		0x002b9601		/* colonVIB */
#define CODE_MENURAD_COL		0x002b8503		/* colonRAD */
#define CODE_MENUBLD_COL		0x002b4663		/* colonBLD */
#define CODE_MENUSND_COL		0x002b8aa3		/* colonSND */
#define CODE_MENU5_1CH_COL		0x0052c954		/* colon5_1CH */
#define CODE_MENUCAP_COL		0x002b490f		/* colonCAP */

#define CODE_MENUOWN_COL		0x002b7bcd		/* colonOWN */
#define CODE_MENUITEM_COL		0x006c6cd2		/* colonITEM */
#define CODE_MENUQUICK_COL		0x000e2959		/* colonQUICK */

#endif

#if 0

#define CODE_MENUVIB_VAL		0x00cae902		/* menuVibSub */
#define CODE_MENURAD_VAL		0x004be8fa		/* menuRadSub */
#define CODE_MENUBLD_VAL		0x00fbe8da		/* menuBldSub */
#define CODE_MENUSND_VAL		0x001be8fd		/* menuSndSub */
#define CODE_MENUCAP_VAL		0x0051e8dc		/* menuCapSub */

#else

#define CODE_MENUVIB_VAL		0x006ce589		/* swVib */
#define CODE_MENURAD_VAL		0x006cd06b		/* swRAD */
#define CODE_MENUBLD_VAL		0x006c91cb		/* swBLD */
#define CODE_MENUSND_VAL		0x006cd60b		/* swSND */
#define CODE_MENU5_1CH_VAL		0x00806a59		/* sw5_1CH */
#define CODE_MENUCAP_VAL		0x006c9477		/* swCAP */

#define CODE_MENUOWN_VAL		0x006cc735		/* swOWN */
#define CODE_MENUITEM_VAL		0x0095d9da		/* swITEM */
#define CODE_MENUQUICK_VAL		0x003bca5e		/* swQUICK */

#endif


/* オプション項目オブジェクト名称 */
#define OBJ_optVIB	0x3b6247	/* "optVIB" */
#define OBJ_optRAD	0x3b5149	/* "optRAD" */
#define OBJ_optBLD	0x3b12a9	/* "optBLD" */
#define OBJ_optSND	0x3b56e9	/* "optSND" */
#define OBJ_opt5_1CH	0x83e193	/* "opt5_1CH" */
#define OBJ_optCAP	0x3b1555	/* "optCAP" */
#define OBJ_optOWN	0x3b4813	/* "optOWN" */
#define OBJ_optITEM	0x65f594	/* "optITEM" */
#define OBJ_optQUICK	0x3f4198	/* "optQUICK" */
#define OBJ_optSCRN	0x6ab335	/* "optSCRN" */
#define OBJ_optCOL	0x3b1711	/* "optCOL" */
/* OBJ_optEXIT		:optEXIT */
#define OBJ_optEXIT	0x77efb7	/* "menuExit" */


/* 選択状態キー名称 */
#define KEY_selVIB	0x87624e	/* "selVIB" */
#define KEY_selRAD	0x875150	/* "selRAD" */
#define KEY_selBLD	0x8712b0	/* "selBLD" */
#define KEY_selSND	0x8756f0	/* "selSND" */
#define KEY_sel5_1CH	0x83fec3	/* "sel5_1CH" */
#define KEY_selCAP	0x87155c	/* "selCAP" */
#define KEY_selOWN	0x87481a	/* "selOWN" */
#define KEY_selITEM	0xe5f67d	/* "selITEM" */
#define KEY_selQUICK	0x3f5ec8	/* "selQUICK" */
#define KEY_selSCRN	0xeab41e	/* "selSCRN" */
#define KEY_selCOL	0x871718	/* "selCOL" */
#define KEY_selEXIT	0xe40704	/* "selEXIT" */

/* 非選択状態キー名称 */
#define KEY_unVIB	0x886169	/* "unVIB" */
#define KEY_unRAD	0x88506b	/* "unRAD" */
#define KEY_unBLD	0x8811cb	/* "unBLD" */
#define KEY_unSND	0x88560b	/* "unSND" */
#define KEY_un5_1CH	0x806ac7	/* "un5_1CH" */
#define KEY_unCAP	0x881477	/* "unCAP" */
#define KEY_unOWN	0x884735	/* "unOWN" */
#define KEY_unITEM	0x5d9de	/* "unITEM" */
#define KEY_unQUICK	0x3bcacc	/* "unQUICK" */
#define KEY_unSCRN	0xa977f	/* "unSCRN" */
#define KEY_unCOL	0x881633	/* "unCOL" */
#define KEY_unEXIT	0x3ea65	/* "unEXIT" */



/* オプションスイッチオブジェクト名称 */
#define OBJ_swVIB	0x6ce169	/* "swVIB" */
#define OBJ_swRAD	0x6cd06b	/* "swRAD" */
#define OBJ_swBLD	0x6c91cb	/* "swBLD" */
#define OBJ_swSND	0x6cd60b	/* "swSND" */
#define OBJ_sw5_1CH	0x806a59	/* "sw5_1CH" */
#define OBJ_swCAP	0x6c9477	/* "swCAP" */
#define OBJ_swOWN	0x6cc735	/* "swOWN" */
#define OBJ_swITEM	0x95d9da	/* "swITEM" */
#define OBJ_swQUICK	0x3bca5e	/* "swQUICK" */


/* オプションスイッチ状態キー名称 */
#define SW_vibON	0x961235	/* "vibON" */
#define SW_vibOFF	0xc245f8	/* "vibOFF" */
#define SW_radON	0x521a35	/* "radON" */
#define SW_radTYPE1	0x7171de	/* "radTYPE1" */
#define SW_radTYPE2	0x7171df	/* "radTYPE2" */
#define SW_radOFF	0x4345f0	/* "radOFF" */
#define SW_radOFF1	0x68be39	/* "radOFF1" */
#define SW_radOFF2	0x68be3a	/* "radOFF2" */
#define SW_bldON	0x579a34	/* "bldON" */
#define SW_bldOFF	0xf345d0	/* "bldOFF" */
#define SW_sndSTEREO	0xda5a9a	/* "sndSTEREO" */
#define SW_sndMONO	0x67e3b1	/* "sndMONO" */
#define SW_5_1chON	0x43382f	/* "5_1chON" */
#define SW_5_1chOFF	0x67052e	/* "5_1chOFF" */
#define SW_capON	0x624a34	/* "capON" */
#define SW_capOFF	0x4945d2	/* "capOFF" */
#define SW_capJPN	0x0049331a	/* capJPN */ 
#define SW_capITA	0x00492f8d	/* capITA */ 
#define SW_capSPA	0x0049570d	/* capSPA */ 
#define SW_capKRA	0x00491ed3	/* capENG */ ///-->仮!!! yano
#define SW_capENG	0x00491ed3	/* capENG */
#define SW_capFRE	0x00492351	/* capFRE */
#define SW_capGER	0x004925be	/* capGER */
#define SW_ownNORM	0x86342	/* "ownNORM" */
#define SW_ownREV	0xa851db	/* "ownREV" */
#define SW_itemLINEAR	0x4f605c	/* "itemLINEAR" */
#define SW_itemGROUP	0x7010f	/* "itemGROUP" */
#define SW_quickPREV	0x85b2	/* "quickUNEQUIP" */
#define SW_quickUNEQUIP	0x4e1403	/* "quickPREV" */


/* カーソル位置指定キー名称 */
#define POS_curVIB	0x24178b	/* "curO_VIB" */
#define POS_curRAD	0x24068d	/* "curO_RAD" */
#define POS_curBLD	0x23c7ed	/* "curO_BLD" */
#define POS_curSND	0x240c2d	/* "curO_SND" */
#define POS_cur5_1CH	0x58f136	/* "curO_5_1CH" */
#define POS_curCAP	0x23ca99	/* "curO_CAP" */
#define POS_curOWN	0x23fd57	/* "curO_OWN" */
#define POS_curITEM	0x7c9e11	/* "curO_ITEM" */
#define POS_curQUICK	0x14513b	/* "curO_QUICK" */
#define POS_curSCRN	0x815bb2	/* "curO_SCRN" */
#define POS_curCOL	0x23cc55	/* "curO_COL" */
#define POS_curEXIT	0x7aae98	/* "curO_EXIT" */

/* カーソルオブジェクト名称 */
#define CUR_curs	0xe38043	/* "selCurs" */



#define CODE_NEXTTOBTMCFG		0x004693a5		/* closeOptToBtm */
#define CODE_NEXTTOSCRADJ		0x00b1ddb8		/* closeOptSlant2 */
#define CODE_NEXTTOCOLADJ		0x00b1ddb8		/* closeOptSlant2 */

#define CODE_SHOWBTMCFG			0x00ffbac0		/* openCfgSlant2 */
#define CODE_HIDEBTMCFG			0x0046786e		/* closeCfgToTop */

/* スクリーンアジャスト */
#define CODE_SHOWSCRADJ			0x00ca26ea		/* openAdjust */
#define CODE_HIDESCRADJ			0x00699636		/* closeAdjust */

#define CODE_NUM_X_10			0x00311070		/* num_x_10 */
#define CODE_NUM_X_01			0x00311051		/* num_x_01 */
#define CODE_NUM_Y_10			0x00319070		/* num_y_10 */
#define CODE_NUM_Y_01			0x00319051		/* num_y_01 */
#define CODE_X_MINUS			0x000665d8		/* x_minus */
#define CODE_Y_MINUS			0x00066618		/* y_minus */

#define CODE_NUM_FADEIN			0x00841d9d		/* num_fade_in */
#define CODE_NUM_FADEOUT		0x0083cd04		/* num_fade_out */


/* カラーアジャスト */
#define CODE_SHOWCLRADJ			0x00dbd872		/* openColor */
#define CODE_HIDECLRADJ			0x0038d3ed		/* closeColor */


/* ロードゲーム */
#if 0
#define CODE_OPENSAVE			0x000ea7f6		/* openSave */
#define CODE_OPENLOAD			0x000b5d55		/* openLoad */
#else
#define CODE_OPENSAVE			0x000d2c26		/* openFrame */
#define CODE_OPENLOAD			0x000d2c26		/* openFrame */
#endif
#define CODE_MCCHECK			0x00d83aeb		/* openCheck */
#define CODE_MCCHECKEND			0x00353666		/* closeCheck */
#define CODE_OPENSELSLOT1		0x0071dd2c		/* openMemory1 */
#define CODE_OPENSELSLOT2		0x0071dd2d		/* openMemory2 */
#define CODE_OPENSELSLOT12		0x0071dd2e		/* openMemory3 */
#define CODE_OPENSELFILE1		0x0068f4d2		/* openList1 */
#define CODE_OPENSELFILE2		0x0068f4d3		/* openList2 */
#define CODE_OPENSELFILE12_1	0x00a50090		/* openList3_slot1 */
#define CODE_OPENSELFILE12_2	0x00a50091		/* openList3_slot2 */
#define CODE_OPENCONFIRM		0x006132fa		/* openOK_Cancel */
#define CODE_CLOSECONFIRM		0x000702b2		/* closeOK_Cancel */
#if 0
#define CODE_CLOSEALL			0x0066c5d1		/* closeAll */
#else
#define CODE_CLOSEALL			0x006a27a0		/* closeFrame */
#endif
#if 0
#define CODE_TODOGTAG			0x009b2216		/* setDogTag */
#define CODE_TOALBUM			0x004b4939		/* setAlbum */
#else
#define CODE_TODOGTAG			0x00de2f80		/* closeList */
#define CODE_TOALBUM			0x00353666		/* closeCheck */
#endif

#define CODE_DATALOADTAG		0x00da7a07		/* dataLoadTag */
#define CODE_DATASAVETAG		0x002afbad		/* dataSaveTag */

#define CODE_SLOT1				0x0067cab8		/* slot1 */
#define CODE_SLOT2				0x0067cab9		/* slot2 */

#define CODE_MENU_OK			0x001c265c		/* menu_ok */
#define CODE_MENU_CANCEL		0x005a5b59		/* menu_cancel */

#define CODE_RARROW				0x00d535f0		/* right_arrow */
#define CODE_LARROW				0x00332429		/* left_arrow */

#define CODE_CLOSELIST			0x00de2f80		/* closeList */

#define CODE_SRASH				0x006792cf		/* slash */
#define CODE_DOGTAG				0x00ac6ef5		/* dogTagTag */
#define CODE_ALBUMTAG			0x008b0052		/* albumTag */
#define CODE_DATASAVETAG		0x002afbad		/* dataSaveTag */
#define CODE_DATALOADTAG		0x00da7a07		/* dataLoadTag */
#define CODE_UNDERBAR			0x008dd8e0		/* underBar */

#define CODE_HIDE_ALBUMLOAD		0x00e2f811		/* hideAlbumLoad */
#define CODE_SHOW_ALBUMLOAD		0x0023a7c0		/* showAlbumLoad */
#define CODE_HIDE_DOGTAGLOAD	0x0060e1ea		/* hideDogtagLoad */
#define CODE_SHOW_DOGTAGLOAD	0x0076d7b2		/* showDogtagLoad */
#define CODE_HIDE_LOAD			0x007b2392		/* hideLoad */
#define CODE_SHOW_LOAD			0x009b7b69		/* showLoad */
#define CODE_HIDE_SAVE			0x007e6e33		/* hideSave */
#define CODE_SHOW_SAVE			0x009ec60a		/* showSave */
#define CODE_HIDE_MODE			0x007ba3f3		/* hideMode */
#define CODE_SHOW_MODE			0x009bfbca		/* showMode */


/* スペシャル選択画面 */
#define CODE_SHOWSP				0x00ebb7fc		/* openSpecial */
#define CODE_HIDESP				0x00d9a170		/* closeSpecial */
#define CODE_SHOWSPMENU			0x009bd51a		/* showMenu */
#define CODE_HIDESPMENU			0x007b7d43		/* hideMenu */

#define CODE_MENUPRE			0x006f680a		/* prestory */
#define CODE_MENUPHOTO			0x00b32cfd		/* photoAlbum */
#define CODE_MENUDOG			0x0024dd56		/* dogTag */
#define CODE_MENUEXIT			0x0077efb7		/* menuExit */

#define CODE_SHOWSPPRE			0x002cec3c		/* showPre */
#define CODE_HIDESPPRE			0x0073e97d		/* hidePre */
#define CODE_SELPRE				0x00874f91		/* selPre */
#define CODE_UNSELPRE			0x002cec3c		/* showPre */
#define CODE_SHOWSPPHOTO		0x00ac27a2		/* showPhoto */
#define CODE_HIDESPPHOTO		0x00a12cbe		/* hidePhoto */
#define CODE_SELPHOTO			0x00397d0c		/* selPhoto */
#define CODE_UNSELPHOTO			0x00ac27a2		/* showPhoto */
#define CODE_SHOWSPDOG			0x002cbbde		/* showDog */
#define CODE_HIDESPDOG			0x0073b91f		/* hideDog */
#define CODE_SELDOG				0x00871f33		/* selDog */
#define CODE_UNSELDOG			0x002cbbde		/* showDog */
#define CODE_SHOWSPEXIT			0x00982079		/* showExit */
#define CODE_HIDESPEXIT			0x0077c8a2		/* hideExit */
#define CODE_SELEXIT			0x00e48b24		/* selExit */
#define CODE_UNSELEXIT			0x00982079		/* showExit */

#define CODE_SHOWCURS			0x00971598		/* showCurs */
#define CODE_HIDECURS			0x0076bdc1		/* hideCurs */
#define CODE_CURSSPPRE			0x005ae856		/* cursPre */
#define CODE_CURSSPPHOTO		0x009c905a		/* cursPhoto */
#define CODE_CURSSPDOG			0x005ab7f8		/* cursDog */
#define CODE_CURSSPEXIT			0x0057a3bf		/* cursExit */

#define CODE_MENUDEMO			0x008f8d09		/* demo_mode */
#define CODE_MENUBOSS			0x008dde91		/* boss_mode */

#define CODE_SHOWSPBOSS			0x00967db8		/* showBoss */
#define CODE_HIDESPBOSS			0x007625e1		/* hideBoss */
#define CODE_SELBOSS			0x00e2e863		/* selBoss */
#define CODE_UNSELBOSS			0x00967db8		/* showBoss */
#define CODE_SHOWSPDEMO			0x009754f4		/* showDemo */
#define CODE_HIDESPDEMO			0x0076fd1d		/* hideDemo */
#define CODE_SELDEMO			0x00e3bf9f		/* selDemo */
#define CODE_UNSELDEMO			0x009754f4		/* showDemo */

#define CODE_CURSSPDEMO			0x0056d83a		/* cursDemo */
#define CODE_CURSSPBOSS			0x005600fe		/* cursBoss */

#define CODE_MENUBASICACTION	0x0002ad9a		/* basic_actions */

#define CODE_HIDESPBASICACTION	0x00bdbb51		/* hideBasic */
#define CODE_SHOWSPBASICACTION	0x00c8b635		/* showBasic */
#define CODE_SELSPBASICACTION	0x00560b9f		/* selBasic */
#define CODE_UNSELSPBASICACTION	0x00c8b635		/* showBasic */

#define CODE_CURSSPBASICACTION	0x00b91eed		/* cursBasic */


/* フレームへの指令 */
#define FRAME_SIGNAL_UP				0x00000b10		/* Up */
#define FRAME_SIGNAL_DOWN			0x0023cb4e		/* Down */
#define FRAME_SIGNAL_LEFT			0x0027a134		/* Left */
#define FRAME_SIGNAL_RIGHT			0x00562979		/* Right */
#define FRAME_SIGNAL_RIGHTUP		0x00a5f068		/* RightUp */
#define FRAME_SIGNAL_LEFTDOWN		0x00664561		/* LeftDown */


/* SEコールタイミング */
#define CODE_SOUND					0x0069622b		/* sound */


#define SE_SEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_CUR01)
// #define SE_START()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_START01)
#define SE_START()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define SE_OK()			GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WIN01)
#define SE_CANCEL()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_V_CANS02)

#define SE_SCRMOVE()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_R_TUNE01)

// #define SE_DECIDE()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_A_PAGE10B1)
#define SE_DECIDE()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_SHAFFLE1)


/* セーブ終了 */
#define SE_SAVE_FINISH()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_SAVEOK01)

/* ウィンドウ開閉音 */
#define SE_WINOPEN()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPN01)
#define SE_WINOPENL2R()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPNL1)
#define SE_WINOPENR2L()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINOPNR1)
#define SE_WINCLOSE()		GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLS01)
#define SE_WINCLOSEL2R()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLSL1)
#define SE_WINCLOSER2L()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_WINCLSR1)


/* オプションの切替え音 */
#define SE_SWITCHOPT()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_TYPING03)

/* SAVE,LOAD,SPECIALのウィンドウ開閉音その１ */
#define SE_EXPANDLINE()	GM_SeSet(GM_PAN_CENTER,GM_MAX_VOL,SD_S_LINEMOV1)


/* gamesel.c */
void *NewGameSelScr(int parent_name,int l2d_handle,int proc,int start_mode,
					int pre_gamesel_cursor,int bgname);

/* newgame.c */
void *NewNewGameScr(int parent_name,int l2d_handle,int proc,int bgname);

/* option.c */
void *NewOptionScrForNode(int parent_name,int l2d_handle,void *strman);

/* loadgame.c */
void *NewLoadGameScrForDogtag(int name,int parent_name,int l2d_handle,void *strman,int start_mode);
//
// ドッグタグ用のロードゲーム画面です
// 終了をメッセージで知らせます
// メッセージ
//   配列要素 0  ... name
//   配列要素 1  ... OK(1) / CANCEL(2)
// 
// name        ... このアクターにつける名前です
// parent_name ... 親アクターの名前です
// l2d_handle  ... 初期化済みのL2Dのハンドルです。標準では、save_load.l2dを初期化して
//                 使うことになると思います。
// strman      ... NewTextScreenControl()が返すワークのアドレスです
// start_mode  ... 0:初回起動 / 1:同じ選択モードでの二回目移行の起動
//
// 内部でMCManの起動を行っていますので、呼び出し側でMCManを起動しておく必要はありません
//


#include "portsel.h"

/* frame.c */
void ComNodeFrameAction(int action_strcode);
int ComNodeFrameBusy(void);

/* source/user/skoba/etc/encute.c */
void *NewQuestion( int parent_name , int child_name , int mode );

/* stloadsave.c */
extern void *CODEC_NewSnakeTalesSave( void );

/* util.c */
extern float TTL_CalcVRAchievementRatio( void );

#define TTL_MAX_VARREF 6
extern GCL_VAR_REF	TTL_ReferenceVariable[ TTL_MAX_VARREF ];
#endif
