//=============================================================================
// w16ab_common.h
// Ｃ脚・食堂(共通)
// 2001/05/06 S.Yamashita
// $Id: w16ab_break.h,v 1.15 2001/09/26 15:12:29 usr04098 Exp $
//

//=============================================================================
// 壊れ物
//=============================================================================

#define BRK_TOILET_MEN   1
#define BRK_TOILET_WOMEN 1
#define BRK_DINNING      1
#define BRK_KITCHEN      1

//-----------------------------------------------------------------------------

#include "../module/inc_se/se_defin.h"

proc 壊れ物女子トイレ {
#if d:BRK_TOILET_WOMEN

	//---------------------------------------------------------
	@w16a_壊れ物チェック (6)
	chara プットガラスオブジェ ガラス \
		-model   w16a2_mirror_bro btl_piece \
		-texture w16a2_mirror_alp w16a2_mirror_bro_alp \
                         w16a_mirror_ura_alp \
		-pos     -48375,2000,-87490 \
		-flag	 0 \
		-hazard  d:BRK_HZD_W16A_TOILET_WOMEN \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_6

	//---------------------------------------------------------
	@w16a_壊れ物チェック (7)
	chara プットガラスオブジェ ガラス \
		-model   w16a2_mirror_bro btl_piece \
		-texture w16a2_mirror_alp w16a2_mirror_bro_alp \
                         w16a_mirror_ura_alp \
		-pos     -49625,2000,-87490 \
		-flag	 0 \
		-hazard  d:BRK_HZD_W16A_TOILET_WOMEN \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_7
#endif
}

//-----------------------------------------------------------------------------

proc 壊れ物男子トイレ {
#if d:BRK_TOILET_MEN

	//---------------------------------------------------------
	@w16a_壊れ物チェック (8)
	chara プットガラスオブジェ ガラス \
		-model   w16a2_mirror_bro btl_piece \
		-texture w16a2_mirror_alp w16a2_mirror_bro_alp \
                         w16a_mirror_ura_alp \
		-pos     -56875,2000,-87490 \
		-flag	 0 \
		-hazard  d:BRK_HZD_W16A_TOILET_MEN \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_8

	//---------------------------------------------------------
	@w16a_壊れ物チェック (9)
	chara プットガラスオブジェ ガラス \
		-model   w16a2_mirror_bro btl_piece \
		-texture w16a2_mirror_alp w16a2_mirror_bro_alp \
                         w16a_mirror_ura_alp \
		-pos     -58125,2000,-87490 \
		-flag	 0 \
		-hazard  d:BRK_HZD_W16A_TOILET_MEN \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_9
#endif
}

//-----------------------------------------------------------------------------

proc 壊れ物食堂 {
#if d:BRK_DINNING

	//---------------------------------------------------------
	@w16a_壊れ物チェック (10)
	chara プットガラスオブジェ ガラス \
		-model   w16a0_glass_bro btl_piece \
		-texture w16a0_glass_alp w16a0_glass_bro_alp \
		-pos     -60000,2075,-102825 \
		-flag	 0 \
		-hazard  d:BRK_HZD_W16A_LRG_GLASS \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_10

	@食堂調味料セット１ -62225,800,-94175 d:BRK_HZD_W16A_DINING_TBL0
	@食堂調味料セット２ -58175,800,-94175 d:BRK_HZD_W16A_DINING_TBL1
	@食堂調味料セット３ -60370,800,-99500 d:BRK_HZD_W16A_DINING_TBL2
#endif

#if d:BRK_KITCHEN
//	@キッチン調理台
	@キッチン流し

	@キッチンコロコロ野菜セット
	@キッチン揺れ物

	@キッチン野菜箱セット -53080,0,-105327,  924  d:BRK_HZD_W16A_LOCKER
#endif

}

//-----------------------------------------------------------------------------

proc 食堂調味料セット１
{

	//---------------------------------------------------------
	@w16a_壊れ物チェック (11)
	chara プット壊れオブジェ 壊れソース \
		-model w16a0_source w16a0_source_bro w16a0_source_broS \
		-param  -55 0 -57  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 4 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_11

	//---------------------------------------------------------
	@w16a_壊れ物チェック (12)
	chara プット壊れオブジェ 壊れケチャップ \
		-model w16a0_ketchup w16a0_ketchup_bro w16a0_ketchup_broS \
		-param  -50 0 131  0, 0, 0 \
			 50 0 -59  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 3 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_12

	//---------------------------------------------------------
	@w16a_壊れ物チェック (13)
	chara プット壊れオブジェ 壊れ塩 \
		-model w16a0_solt w16a0_solt_bro w16a0_solt_broS \
		-param  -55 0 202  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 2 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_13
}

proc 食堂調味料セット２
{

	//---------------------------------------------------------
	@w16a_壊れ物チェック (14)
	chara プット壊れオブジェ 壊れソース \
		-model w16a0_source w16a0_source_bro w16a0_source_broS \
		-param  -55 0 -57  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 4 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_14

	//---------------------------------------------------------
	@w16a_壊れ物チェック (15)
	chara プット壊れオブジェ 壊れケチャップ \
		-model w16a0_ketchup w16a0_ketchup_bro w16a0_ketchup_broS \
		-param  -50 0 131  0, 0, 0 \
			 50 0 -59  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 3 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_15

	//---------------------------------------------------------
	@w16a_壊れ物チェック (16)
	chara プット壊れオブジェ 壊れ塩 \
		-model w16a0_solt w16a0_solt_bro w16a0_solt_broS \
		-param  -55 0 202  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 2 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_16
}

proc 食堂調味料セット３
{

	//---------------------------------------------------------
	@w16a_壊れ物チェック (17)
	chara プット壊れオブジェ 壊れソース \
		-model w16a0_source w16a0_source_bro w16a0_source_broS \
		-param  -55 0 -57  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 4 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_17

	//---------------------------------------------------------
	@w16a_壊れ物チェック (18)
	chara プット壊れオブジェ 壊れケチャップ \
		-model w16a0_ketchup w16a0_ketchup_bro w16a0_ketchup_broS \
		-param  -50 0 131  0, 0, 0 \
			 50 0 -59  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 3 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_18

	//---------------------------------------------------------
	@w16a_壊れ物チェック (19)
	chara プット壊れオブジェ 壊れ塩 \
		-model w16a0_solt w16a0_solt_bro w16a0_solt_broS \
		-param  -55 0 202  0, 0, 0 \
			 50 0  89  0, 0, 0 \
		-number 2 \
		-stable 50 \
		-explode 8 10\ // 最初は10 30
		-box w16a0_source \
		-geometry $1 $2 $3 0 0 0 \ // 箱のpos
		-brk_se d:se_code:SD_A_BOTTLE01 \
//		-drp_se d:se_code:SD_A_BOTTLE02 \
		-dust 2 \
		-hazard $4 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_STABLE_X | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG |d:BRK_STRICT_FLR| d:BRK_BRK_SPREAD) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_19
}

proc キッチン調理台 {
	chara メリケン粉 メリケン１Ａ \
		-m w01d_flour_B \
		-p -49160,1000,-99822 \
		-r 0,300,0 \
		-e null
	chara メリケン粉 メリケン１Ｂ \
		-m w01d_flour_A \
		-p -49160,1140,-99822 \
		-r 0,300,0 \
		-e null
}

proc null{
}

proc キッチン流し
{

	//---------------------------------------------------------
	@w16a_壊れ物チェック (20)
	chara プット壊れオブジェ 逆さグラス \
		-model gls_01 gls_01brk btl_piece \
		-param	(-56462- 50+  0),1150,(-101020-100-  0) 0,0,0 \
			(-56462-100+  0),1150,(-101020-200-  0) 0,0,0 \
			(-56462- 50+150),1150,(-101020-100- 75) 0,0,0 \
			(-56462-100+150),1150,(-101020-200- 75) 0,0,0 \
			(-56462- 50+300),1150,(-101020-100-150) 0,0,0 \
			(-56462-100+300),1150,(-101020-200-150) 0,0,0 \
			(-56462- 50+450),1150,(-101020-100-225) 0,0,0 \
			(-56462-100+450),1150,(-101020-200-225) 0,0,0 \
			(-56462- 50+600),1150,(-101020-100-300) 0,0,0 \
			(-56442-100+600),1150,(-101020-200-300) 0,0,0 \
			(-56332+ 50+  0),1425,(-100520-100-  0) 0,0,0 \
			(-55862+100+  0),1425,(-100520-200-  0) 0,0,0 \
			(-56462+ 50+100),1425,(-100520-100- 50) 0,0,0 \
			(-56363+100+100),1425,(-100920-200- 50) 0,0,0 \
			(-56436+ 50+200),1425,(-100920-100-100) 0,0,0 \
			(-56762+100+200),1425,(-100540-200-100) 0,0,0 \
			(-56465+ 50+300),1425,(-100524-100-150) 0,0,0 \
			(-56482+100+300),1420,(-100550-200-150) 0,0,0 \
			(-56742+ 50+400),1425,(-100928-100-200) 0,0,0 \
			(-56642+100+400),1425,(-100942-200-200) 0,0,0 \
		-brk_se d:se_code:SD_A_BOTTLE02 \
//		-drp_se d:se_code:SD_A_BOTTLE01 \
		-hazard d:BRK_HZD_W16A_SINK \
		-number 20 \
		-explode 10 10 \ // 最初は10 30
		-flag  (d:BRK_NO_GRAVITY | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG | d:BRK_STRICT_FLR | d:BRK_NO_KICK) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_20

	//---------------------------------------------------------
	@w16a_壊れ物チェック (21)
	chara プット皿オブジェ plate \
		-model dish5 dish1 dishBroL_cm dishBroS_cm \
		-param	1, 0,-1024,0, -56360,1526,-101083 \
			2, 0,-1024,0, -55989,1526,-100679 \
			1, 0, 1024,0, -55792,1526,-100740 \
			2, 0, 1024,0, -55792,1526,-101235 \
		-light d:LT2_NAME \
		-hazard d:BRK_HZD_W16A_SINK \
		-number (1+2+1+2) \
		-flag 0 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_21
}

proc キッチンコロコロ野菜セット
{

	//---------------------------------------------------------
	@w16a_壊れ物チェック (22)
	chara プットポテトオブジェ りんご \
	        -m apple_org apple_bro_l \
		-l d:LT2_NAME \
		-n 10 \
		-b pataHakoKuro \
		-r (2072) (2072+46) (2072-56) \
		-type 1 \
		-p -50578 1000 -97400 \
		-hazard d:BRK_HZD_W16A_CABINET \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_22

	//---------------------------------------------------------
	@w16a_壊れ物チェック (23)
	chara プットポテトオブジェ じゃがいも \
	        -m potato02 potato03 \
		-l d:LT2_NAME \
		-n 10 \
		-b pataHakoKuro \
		-r (1352) (1352+46) (1352-56) \
		-type 1 \
		-p -49853 1000 -97700 \
		-hazard d:BRK_HZD_W16A_CABINET \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_23
}

proc キッチン揺れ物 {

	chara プット揺れオブジェ 揺れオタマなど \
		-flag 0 \

		-model w01d0Otama \
		-rot   (-3*4096/360) (85*4096/360) (2*4096/360) \
		-pos -45747 2082 -101250 \
		-sound d:se_code:SD_A_OTAMA_E \
		-end \

		-model w01d0Otama \
		-rot   (3*4096/360) (90*4096/360) (-6*4096/360) \
		-pos -45747 2082 -101250 \
		-sound d:se_code:SD_A_OTAMA_CS \
		-end \

		-model flypan_si \
		-rot   (0*4096/360) (-108*4096/360) (-1*4096/360) \
		-scale 81 81 81 \
		-sound d:se_code:SD_A_FLYPAN_B \
		-pos -45747 2082 -101637 \
	    -end \

		-model flypan_co \
		-rot   (0*4096/360) (-108*4096/360) (-1*4096/360) \
		-sound d:se_code:SD_A_FLYPAN_G \
		-pos -45747 2082 -102000 \
		-end \

		-model flypan_bk \
		-rot   (0*4096/360) (-110*4096/360) (-1*4096/360) \
		-pos -45747 2082 -102350 \
		-sound d:se_code:SD_A_FLYPAN_F \
		-end \
}


proc キッチン野菜箱セット
{
	chara 蓋付き段ボール スイカ箱 \
		-p $1 $2 $3 \
		-r 0 $4 0 \
		-a 110 100 106 90

	//---------------------------------------------------------
	@w16a_壊れ物チェック (24)
	chara プット壊れオブジェ 壊れスイカ群 \
#if 0
		-model watermelon_olg watermelon_bro_l watermelon_bro_s \
		-param  -125  300  375 (-100*4096/360), (-1000*4096/360-42*4096/360),	(90*4096/360) \
			-100  250  175 (-100*4096/360), (-1000*4096/360-60*4096/360),(90*4096/360)	 \
			-135  300  -75 (-25*4096/360), (-1000*4096/360-60*4096/360),0	 \
			-120  250 -350 (-12*4096/360), (-1000*4096/360-90*4096/360),(-40*4096/360)	 \
			 125  265 -360	(60*4096/360), (-1000*4096/360-55*4096/360),(-30*4096/360)	 \
			 120  250 -135 (110*4096/360), (-1000*4096/360-60*4096/360),(-90*4096/360)	 \
			 150  325   90	(88*4096/360), (-1000*4096/360-58*4096/360),(-70*4096/360)	 \
			 130  235  350 (-50*4096/360), (-1000*4096/360-75*4096/360),(90*4096/360)	 \
		-number 8 \
#else
		-model watermelon_olg watermelon_bro_l watermelon_bro_s \
		-param  -75  300  295 (-100*4096/360), (-1000*4096/360-42*4096/360),	(90*4096/360) \
			 70  250    5  (-100*4096/360), (-1000*4096/360-60*4096/360),(90*4096/360)	 \
			-75  300 -295 (-25*4096/360), (-1000*4096/360-60*4096/360),0	 \
		-number 3 \
#endif
		-stable 50 \
		-explode 10 10 \ // 最初は10 30
		-box hakoSub_2 \
		-geometry $1 $2 $3 0 $4 0 \ // 箱のpos
		-hazard $5 \
		-dust 6 \
		-flag  (d:BRK_NO_GRAVITY | d:BRK_NO_BOXSHOW | d:BRK_STABLE_Z | d:BRK_STRICT_FLR | d:BRK_BRK_SPREAD | d:BRK_NO_BOXTRG) \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_24
}

//-----------------------------------------------------------------------------

proc 壊れ物広場 {

	//---------------------------------------------------------
	@w16a_壊れ物チェック (25)
	chara プット植物オブジェ 揺れポプラＡ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 1934 0 \
		-p -55900 45 -78838 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_25

	//---------------------------------------------------------
	@w16a_壊れ物チェック (26)
	chara プット植物オブジェ 揺れポプラＢ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 3868 0 \
		-p -55150 45 -78900 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_26

	//---------------------------------------------------------
	@w16a_壊れ物チェック (27)
	chara プット植物オブジェ 揺れポプラＣ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 3755 0 \
		-p -54550 70 -78850 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_27

	//---------------------------------------------------------
	@w16a_壊れ物チェック (28)
	chara プット植物オブジェ 揺れポプラＤ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 3413 0 \
		-p -54000 -5 -78838 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_28

	//---------------------------------------------------------
	@w16a_壊れ物チェック (29)
	chara プット植物オブジェ 揺れポプラＥ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 1820 0 \
		-p -53450 -30 -78875 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_29



	//---------------------------------------------------------
	@w16a_壊れ物チェック (30)
	chara プット植物オブジェ 揺れポプラＦ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 3868 0 \
		-p -52800 -5 -78850 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_30

	//---------------------------------------------------------
	@w16a_壊れ物チェック (31)
	chara プット植物オブジェ 揺れポプラＧ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 3231 0 \
		-p -52300 -5 -78825 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_31

	//---------------------------------------------------------
	@w16a_壊れ物チェック (32)
	chara プット植物オブジェ 揺れポプラＨ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 1252 0 \
		-p -51650 20 -78937 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_32

	//---------------------------------------------------------
	@w16a_壊れ物チェック (33)
	chara プット植物オブジェ 揺れポプラＩ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 1934 0 \
		-p -51150 45 -78900 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_33

	//---------------------------------------------------------
	@w16a_壊れ物チェック (34)
	chara プット植物オブジェ 揺れポプラＪ \
		-m tree_00 -b tree_01 -l d:LT2_NAME -s 100 100 100 \
		-r 0 967 0 \
		-p -50600 145 -78900 \
		-broken  $f:再セットフラグ \
        -proc    セット壊れフラグ_34
}

//-----------------------------------------------------------------------------
proc w16a_壊れ物チェック $:番号 {
#if d:DEBUG_PRINT
	print '*** Broken Check ***'
#endif

	@壊れ物再セットチェック $b:w16a_破壊ロード回数[$:番号]
	if($f:再セットフラグ) {
		eval($f:再セットフラグ = d:FALSE)
	} else {
		eval($f:再セットフラグ = d:TRUE)
	}
}
proc セット壊れフラグ_6 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 6 ***'
#endif

	eval($b:w16a_破壊ロード回数[6] = $w:グローバルロード回数);
}
proc セット壊れフラグ_7 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 7 ***'
#endif

	eval($b:w16a_破壊ロード回数[7] = $w:グローバルロード回数);
}
proc セット壊れフラグ_8 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 8 ***'
#endif

	eval($b:w16a_破壊ロード回数[8] = $w:グローバルロード回数);
}
proc セット壊れフラグ_9 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 9 ***'
#endif

	eval($b:w16a_破壊ロード回数[9] = $w:グローバルロード回数);
}
proc セット壊れフラグ_10 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 10 ***'
#endif

	eval($b:w16a_破壊ロード回数[10] = $w:グローバルロード回数);
}
proc セット壊れフラグ_11 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 11 ***'
#endif

	eval($b:w16a_破壊ロード回数[11] = $w:グローバルロード回数);
}
proc セット壊れフラグ_12 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 12 ***'
#endif

	eval($b:w16a_破壊ロード回数[12] = $w:グローバルロード回数);
}
proc セット壊れフラグ_13 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 13 ***'
#endif

	eval($b:w16a_破壊ロード回数[13] = $w:グローバルロード回数);
}
proc セット壊れフラグ_14 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 14 ***'
#endif

	eval($b:w16a_破壊ロード回数[14] = $w:グローバルロード回数);
}
proc セット壊れフラグ_15 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 15 ***'
#endif

	eval($b:w16a_破壊ロード回数[15] = $w:グローバルロード回数);
}
proc セット壊れフラグ_16 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 16 ***'
#endif

	eval($b:w16a_破壊ロード回数[16] = $w:グローバルロード回数);
}
proc セット壊れフラグ_17 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 17 ***'
#endif

	eval($b:w16a_破壊ロード回数[17] = $w:グローバルロード回数);
}
proc セット壊れフラグ_18 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 18 ***'
#endif

	eval($b:w16a_破壊ロード回数[18] = $w:グローバルロード回数);
}
proc セット壊れフラグ_19 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 19 ***'
#endif

	eval($b:w16a_破壊ロード回数[19] = $w:グローバルロード回数);
}
proc セット壊れフラグ_20 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 20 ***'
#endif

	eval($b:w16a_破壊ロード回数[20] = $w:グローバルロード回数);
}
proc セット壊れフラグ_21 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 21 ***'
#endif

	eval($b:w16a_破壊ロード回数[21] = $w:グローバルロード回数);
}
proc セット壊れフラグ_22 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 22 ***'
#endif

	eval($b:w16a_破壊ロード回数[22] = $w:グローバルロード回数);
}
proc セット壊れフラグ_23 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 23 ***'
#endif

	eval($b:w16a_破壊ロード回数[23] = $w:グローバルロード回数);
}
proc セット壊れフラグ_24 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 24 ***'
#endif

	eval($b:w16a_破壊ロード回数[24] = $w:グローバルロード回数);
}
proc セット壊れフラグ_25 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 25 ***'
#endif

	eval($b:w16a_破壊ロード回数[25] = $w:グローバルロード回数);
}
proc セット壊れフラグ_26 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 26 ***'
#endif

	eval($b:w16a_破壊ロード回数[26] = $w:グローバルロード回数);
}
proc セット壊れフラグ_27 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 27 ***'
#endif

	eval($b:w16a_破壊ロード回数[27] = $w:グローバルロード回数);
}
proc セット壊れフラグ_28 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 28 ***'
#endif

	eval($b:w16a_破壊ロード回数[28] = $w:グローバルロード回数);
}
proc セット壊れフラグ_29 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 29 ***'
#endif

	eval($b:w16a_破壊ロード回数[29] = $w:グローバルロード回数);
}
proc セット壊れフラグ_30 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 30 ***'
#endif

	eval($b:w16a_破壊ロード回数[30] = $w:グローバルロード回数);
}
proc セット壊れフラグ_31 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 31 ***'
#endif

	eval($b:w16a_破壊ロード回数[31] = $w:グローバルロード回数);
}
proc セット壊れフラグ_32 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 32 ***'
#endif

	eval($b:w16a_破壊ロード回数[32] = $w:グローバルロード回数);
}
proc セット壊れフラグ_33 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 33 ***'
#endif

	eval($b:w16a_破壊ロード回数[33] = $w:グローバルロード回数);
}
proc セット壊れフラグ_34 {
#if d:DEBUG_PRINT
	print '*** Set Broken Flag 34 ***'
#endif

	eval($b:w16a_破壊ロード回数[34] = $w:グローバルロード回数);
}
