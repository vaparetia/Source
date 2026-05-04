/*  */
/* アクション名対応マクロ定義ヘッダ生成用ファイル */
/* アクション名の StrCode に対応するマクロは、mode_act.lst から、 */
/* macrobld.pl を通じて mode_act.h に変換される。 */
/*  */
/* $Id: mode_act.lst,v 1.9 2002/02/01 04:43:34 usr01475 Exp $ */
/*  */

/* ダウンロード関係は枠レイアウトにある。 */

#define SCRN_DefaultAction	0x8a3fb	/* "DefaultAction" */
#define SCRN_selectOpt	0xe18e44	/* "selectOpt" */

#define SCRN_setBaseOpen	0x8f7813	/* "setBaseOpen" */
#define SCRN_setBaseClose	0x2d2b76	/* "setBaseClose" */
#define SCRN_openDown	0x7601f	/* "openDown" */
#define SCRN_closeDown	0xda47fa	/* "closeDown" */
#define SCRN_setMapDownload	0x96907a	/* "setMapDownload" */
#define SCRN_setMapComplete	0xb662c7	/* "setMapComplete" */

/* プログレスバー関係 */
#define SCRN_progressEdge	0x9f0148	/* "progressEdge" */
#define SCRN_progress	0xae4037	/* "progress" */

#define KEY_progress0	0xc80725	/* "progress0" */
#define KEY_progress100	0x1ca150	/* "progress100" */


#define SCRN_prg_num001	0xfc96e2	/* "prg_num001" */
#define SCRN_prg_num010	0xfc9701	/* "prg_num010" */
#define SCRN_prg_num100	0xfc9ae1	/* "prg_num100" */

#define KEY_num0	0x38e1d0	/* "num0" */
#define KEY_num1	0x38e1d1	/* "num1" */
#define KEY_num2	0x38e1d2	/* "num2" */
#define KEY_num3	0x38e1d3	/* "num3" */
#define KEY_num4	0x38e1d4	/* "num4" */
#define KEY_num5	0x38e1d5	/* "num5" */
#define KEY_num6	0x38e1d6	/* "num6" */
#define KEY_num7	0x38e1d7	/* "num7" */
#define KEY_num8	0x38e1d8	/* "num8" */
#define KEY_num9	0x38e1d9	/* "num9" */


/* メニュー開閉アクション等 */
#define SCRN_openOptSlant1	0xc0e00	/* "openOptSlant1" */
#define SCRN_openOptSlant2	0xc0e01	/* "openOptSlant2" */
#define SCRN_closeOptSlant1	0xb1ddb7	/* "closeOptSlant1" */


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

/* 各項目の ":" 記号 */
#define OBJ_colonVIB	0x2b9601	/* "colonVIB" */
#define OBJ_colonRAD	0x2b8503	/* "colonRAD" */
#define OBJ_colonBLD	0x2b4663	/* "colonBLD" */
#define OBJ_colonSND	0x2b8aa3	/* "colonSND" */
#define OBJ_colon5_1CH	0x52c954	/* "colon5_1CH" */
#define OBJ_colonCAP	0x2b490f	/* "colonCAP" */
#define OBJ_colonOWN	0x2b7bcd	/* "colonOWN" */
#define OBJ_colonITEM	0x6c6cd2	/* "colonITEM" */
#define OBJ_colonQUICK	0xe2959	/* "colonQUICK" */

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

#define SW_radOFF1	0x68be39	/* "radOFF1" */
#define SW_radTYPE1	0x7171de	/* "radTYPE1" */
#define SW_radTYPE2	0x7171df	/* "radTYPE2" */
#define SW_radOFF2	0x68be3a	/* "radOFF2" */

#define SW_bldON	0x579a34	/* "bldON" */
#define SW_bldOFF	0xf345d0	/* "bldOFF" */
#define SW_sndSTEREO	0xda5a9a	/* "sndSTEREO" */
#define SW_sndMONO	0x67e3b1	/* "sndMONO" */
#define SW_5_1chON	0x43382f	/* "5_1chON" */
#define SW_5_1chOFF	0x67052e	/* "5_1chOFF" */
#define SW_capON	0x624a34	/* "capON" */
#define SW_capOFF	0x4945d2	/* "capOFF" */
#define SW_ownNORM	0x86342	/* "ownNORM" */
#define SW_ownREV	0xa851db	/* "ownREV" */
#define SW_itemLINEAR	0x4f605c	/* "itemLINEAR" */
#define SW_itemGROUP	0x7010f	/* "itemGROUP" */
#define SW_quickPREV	0x4e1403	/* "quickPREV" */
#define SW_quickUNEQUIP	0x85b2	/* "quickUNEQUIP" */


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
#define CUR_curs_parent	0x6719f0	/* "selCursNull" */

#define OBJ_ROOT	0x2a4634	/* "ROOT" */
#define OBJ_Prog	0x29d647	/* "Prog" */
#define KEY_default	0x927bc4	/* "default" */
