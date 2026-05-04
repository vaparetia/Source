/*
   item.h
   アイテム関係定義ファイル

   1999/10/20 M.Sonoyama
   $Id: item.h,v 1.1.1.3 2002/11/19 11:50:56 Yoshizawa1 Exp $
*/

extern	ITEM	NewC_BoxA, NewC_BoxB, NewC_BoxC, NewC_BoxD, NewC_BoxE ;
extern	ITEM	NewScope, NewCigar, NewTnkDigitalCamera , NewNormalDigitalCamera ;
extern	ITEM	NewBombSenserB, NewUniform, NewMugenBandana ;
extern	ITEM	NewIRGoggles, NewNVGoggles, NewStealth ;
extern	ITEM	NewMugenWig, NewMugenO2Wig, NewMugenGripWig ;

/* タイプの詳細は player.h */

PL_ItemSet		PL_ItemSets[] = {
    { NULL,   	NULL, 	0, 	NULL, 0, NULL },	/* 素手 */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* レーション */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* ダミー双眼鏡 */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* 風邪薬 */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },    	/* 止血剤 */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },    	/* シアゼパム */
    { NewUniform,NULL,  UNIFORM_TYPE, 	NULL, 0, NULL },    	/* ゴル兵制服 */
    { NULL		,NULL, 	JACKET_TYPE, 	NULL, 0, NULL },    	/* 防弾チョッキ */
    { NewStealth,  	NULL, 	0, 	NULL, 0, NULL },	/* ステルス迷彩 */
    { NULL,  	NULL, 	MINEDT_TYPE, NULL, 0, NULL },	/* 地雷探知器 */
    { NULL,  	NULL, 	BSENSA_TYPE, NULL, 0, NULL },	/* 爆弾解体センサーＡ */
    { NewBombSenserB,  	NULL, 	BSENSB_TYPE, NULL, 0, NULL },	/* 爆弾解体センサーＢ */
    { NewNVGoggles,  	NULL, 	GOGGLES_TYPE, 	NULL, HUMAN21_ATAMA, NULL },	/* 暗視ゴーグル */
    { NewIRGoggles,  	NULL, 	GOGGLES_TYPE, 	NULL, HUMAN21_ATAMA, NULL },	/* 熱源ゴーグル */
    { NewScope,	SetScope, 	SCOPE_TYPE, 	NULL, 0, NULL },/* 双眼鏡 */
    { NewNormalDigitalCamera, SetScope, 	DGCAMERA_TYPE, 	NULL, 0, NULL },		/* デジカメ */
    { NewC_BoxA, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* ダンボール */
    { NewCigar, NULL, 		CIGAR_TYPE, 	NULL, 12, NULL },/* たばこ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* ＩＤカード */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* 髭剃り */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },	/* 携帯電話 */
	{ NewTnkDigitalCamera, SetScope, 	DGCAMERA_TYPE, 	NULL, 0, NULL },	/* タンカー編カメラ */
    { NewC_BoxB, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* ダンボールＢ */
    { NewC_BoxC, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* ダンボールＣ */
    { NewC_BoxA, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* 濡れダンボール */
    { NULL,   	NULL, 	0, 	NULL, 0, NULL },		/* 振動センサー */
    { NewC_BoxD, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* ダンボールＤ */
    { NewC_BoxE, CB_BoxStop, 	C_BOX_TYPE, 	NULL, 0, NULL },/* ダンボールＥ */
    { NULL,   	NULL, 	0, 	NULL, 0, NULL },		/* 無駄毛処理器 */
	{ NULL,   	NULL, 	0, 	NULL, 0, NULL },		/* ソコムサプレッサ */
    { NULL,   	NULL, 	0, 	NULL, 0, NULL },		/* ＡＫサプレッサ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },		/* ダミータンカーカメラ */
    { NewMugenBandana, 	NULL, 	BANDANA_TYPE, 	NULL, 0, NULL },		/* 無限バンダナ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },		/* ドッグタグ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },		/* ＭＯディスク */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },		/* ＵＳＰサプレッサ */
    { NewMugenWig,  	NULL, 	MUGENWIG_TYPE, 	NULL, 0, NULL },		/* 無限カツラ */
    { NewMugenO2Wig,  	NULL, 	WIG_TYPE, 	NULL, 0, NULL },		/* カツラＡ */
    { NewMugenGripWig,  NULL, 	WIG_TYPE, 	NULL, 0, NULL },		/* カツラＢ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL },		/* カツラＣ */
    { NULL,  	NULL, 	0, 	NULL, 0, NULL }			/* カツラＤ */
} ;
