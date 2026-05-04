//
//	p_face.h
//	プラント編の顔設定がいっぱい書いてあるようだ。
//	2001/05/21	T.Fukushima
//	$Id: p_face.h,v 1.73 2001/08/28 07:51:58 usr03005 Exp $

enum PC_大佐顔状態 {
	大佐顔設定１ = 1,
	大佐顔設定＿正面,
	大佐顔設定＿左,
	大佐顔設定＿右,
	大佐顔設定＿後ろ,
	大佐顔設定＿アオリ,
	大佐顔設定＿アップ,
	バグ大佐顔設定１,
	バグ大佐顔設定＿正面,
	バグ大佐顔設定＿左,
	バグ大佐顔設定＿右,
	バグ大佐顔設定＿後ろ,
	バグ大佐顔設定＿アオリ,
	バグ大佐顔設定＿アップ,
	バグバグ処理,
	終了時,
}


/*
	// オタコン設定
	face       0 otc_radio_mh_mt オタコン
	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
	facelimit 0 40000 512 512 113 170

	facedelay 0 75 75
	facedelay 1 75 75
*/





proc 右側顔設定 {
/*
	lightvec   1 -30  -35  -40
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
*/
//	facelimit 1 40000 512 512 113 170
	facedelay 1 75 75
}


proc ライデン＿カメラ設定 {
/*
	facecamera 1 \
	  994500   \   // far
	  11100    \   // zoom
	  59       \   // heading
	  -3750    \   // pan
	  86       \   // pitch
	  0            // gain
*/
//facecamera 1 994500 9234 90 1150 -105 -4000
//facecamera 1 994500 9863 241 -10750 66 -10999
//facecamera 1 994500 9862 147 -10750 101 -10998
	facecamera 1 994500 9862 114 -10750 100 -10998

//	facelimit 1 40000 512 512 113 170
	facelimit 1 40000 80 30 55 20
}


proc 潜水ライデン＿カメラ設定 {
//	@ライデン＿カメラ設定
//	facecamera 1 994500 8316 308 -15049 106 -14997
facecamera 1 994500 9010 294 -15648 54 -8696

//	facelimit 1 40000 512 512 113 170
//	facelimit 1 40000 125 125 50 30
	facelimit 1 40000 110 30 40 15
}

proc 裸ライデン＿カメラ設定 {
	@ライデン＿カメラ設定
}

proc 大佐＿カメラ設定＿普通 {
//	facecamera 0 1000000 10114 -624 22325 435 27399
//		facecamera 0 1000000 9778 -415 23424 466 19498
//		facecamera 0 1000000 9778 -415 23424 466 19498
//facecamera 0 1000000 9777 -636 23423 597 19497
//facecamera 0 1000000 10142 -636 23423 714 25697
//facecamera 0 1000000 8951 -580 22122 484 25697
//facecamera 0 1000000 8951 -803 22121 292 25697
//facecamera 0 1000000 9777 -636 23423 610 19497
//facecamera 0 1000000 9777 -415 23423 375 19497
facecamera 0 1000000 9776 -278 16522 296 15997
//facecamera 0 1000000 9776 -426 16521 454 7496
//facecamera 0 1000000 9776 -426 16521 454 15996

//facelimit 0 40000 512 512 113 170
	facelimit 0 40000 60 20 60 85
/*
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
*/
}

proc 大佐＿カメラ設定＿右 {
	facecamera 0 1000000 9620 -811 115621 73 28096
	facelimit 0 40000 20 50 150 150

}

proc 大佐＿カメラ設定＿左 {
//	facecamera 0 1000000 8115 734 -159478 610 56896
	facecamera 0 1000000 10119 780 -76477 185 22495
	facelimit 0 40000 50 20 150 180
}

proc 大佐＿カメラ設定＿後ろ {
	facecamera 0 1000000 8255 2049 -4777 15 -11203
	facelimit 0 40000 50 50 50 50
}

proc 大佐＿カメラ設定＿正面 {
	facecamera 0 1000000 10285 -37 4021 -27 -3903
	facelimit 0 40000 50 50 50 50
}

proc 大佐＿カメラ設定＿アオリ {
	facecamera 0 1000000 9455 72 -13977 703 77096
	facelimit 0 40000 100 20 50 20
}

proc 大佐＿カメラ設定＿アップ {
	facecamera 0 1000000 23098 -84 14121 185 32496
	facelimit 0 40000 20 40 50 80
}


proc ローズ＿カメラ設定 {
//	facecamera 0 1000000 11594 -418 33424 -174 -3899
//facecamera 0 1000000 9468 -480 12923 135 -2999
//		facecamera 0 1000000 10123 -371 7123 186 3100
//facecamera 0 1000000 9793 -306 25222 186 3100
//facecamera 0 1000000 9793 -239 7721 141 3100
//facecamera 0 1000000 10037 -189 4921 128 3100
//facecamera 0 1000000 10036 -109 -1979 125 1899
//facecamera 0 1000000 10036 -80 7221 113 1899
facecamera 0 1000000 10036 -79 7221 79 1899

//facelimit 0 40000 512 512 113 170
	facelimit 0 40000 60 70 50 80

}

proc プリスキン＿カメラ設定 {
//	facecamera 0 1000000 11099 -346 23625 -263 -29699
//	facecamera 0 1000000 9673 -225 2824 227 -98
//facecamera 0 1000000 9432 -396 36323 -66 -42397
//facecamera 0 1000000 10046 -240 8622 197 -15696
//	facecamera 0 1000000 10046 -313 17621 161 -15696
//facecamera 0 1000000 10046 -289 10821 246 -15696
	facecamera 0 1000000 10046 -203 12820 155 -9395

//facelimit 0 40000 512 512 113 170
	facelimit 0 40000 40 60 60 60
}

proc スネーク＿カメラ設定 {
//	facecamera 0 1000000 11099 -123 5025 -140 0
//	facecamera 0 1000000 10883 -138 2324 182 -14399
	facecamera 0 1000000 10737 -100 5223 71 -24098

//facelimit 0 40000 512 512 113 170
//	facelimit 0 40000 48 85 50 40
	facelimit 0 40000 48 85 50 10
}

proc rt_タンカースネークカメラ設定 {
/*
	facecamera 1 \
	  994500   \   // far
	  11100    \   // zoom
	  59       \   // heading
	  -3750    \   // pan
	  86       \   // pitch
	  0            // gain
*/
//facecamera 1 994500 11099 21 -1150 153 -22499
//facecamera 1 994500 11098 23 -1149 97 -22498
facecamera 1 994500 10642 5 -1149 53 -22497

//	facelimit 1 40000 512 512 113 170
	facelimit 1 40000 65 65 50 45
}


proc ピーター＿カメラ設定 {
//	facecamera 0 1000000 9294 -213 20224 181 13799
//facecamera 0 1000000 8983 -137 9723 430 19498
//	facecamera 0 1000000 9817 -137 9723 429 19497
	facecamera 0 1000000 9817 -137 9723 297 20096

//	facelimit 0 40000 512 512 113 170
//	facelimit 0 40000 60 105 60 100
	facelimit 0 40000 60 95 60 90
}

proc ピーター＿カメラ設定＿寄り {
	facecamera 0 1000000 13231 -3 1322 119 20095
	facelimit 0 40000 60 95 60 90
}

proc オタコン＿カメラ設定 {
//	facecamera 0 1000000 10414 -47 27724 232 0
//facecamera 0 1000000 10414 -50 27724 173 0
//facecamera 0 1000000 10414 11 5124 238 0
//facecamera 0 1000000 10078 -69 8423 310 0
facecamera 0 1000000 10078 -51 3222 317 9899

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 40 40 50 50
}

proc rt_タンカーオタコンカメラ設定 {
/*
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
*/
//facecamera 0 1000000 10489 -183 23625 176 -12099
//facecamera 0 1000000 9973 -181 17825 269 -12098
//facecamera 0 1000000 10472 -85 12824 126 -12098
facecamera 0 1000000 10472 -106 12323 64 -20897

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 5 50 45 20
}

proc 忍者＿カメラ設定 {
//	facecamera 0 1000000 8819 -362 16725 -401 -23399
facecamera 0 1000000 7708 -18 -775 -210 -52298

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 70 60 80 20
}

proc オルガ＿カメラ設定 {
//	facecamera 0 1000000 8819 -362 16725 -401 -23399
//facecamera 0 1000000 10288 -356 10825 373 11901
//facecamera 0 1000000 10457 -188 5825 -164 5000
facecamera 0 1000000 10456 -131 5825 -116 5000


//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 45 60 60 110
}

proc エイムズ＿カメラ設定 {
//	facecamera 0 1000000 9469 -157 3925 103 -22399
facecamera 0 1000000 9773 169 9024 109 -17598

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 20 80 50 90
}

proc 大統領＿カメラ設定 {
//facecamera 0 1000000 9468 -125 16223 22 4099
//	facecamera 0 1000000 9469 -278 16224 -118 4100
//facecamera 0 1000000 9467 -191 16222 33 -8100
facecamera 0 1000000 9467 -191 14121 32 -8099

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 35 30 55 80
}

proc エマ＿カメラ設定 {
//	facecamera 0 1000000 10409 -200 24524 -38 0
//facecamera 0 1000000 10328 -444 11623 -170 16700
//facecamera 0 1000000 10707 -239 8322 42 12899
facecamera 0 1000000 10706 -126 1322 -49 18798

//	facelimit 0 40000 512 512 113 170
	facelimit 0 40000 30 50 30 80
}

proc エマプレイヤー側＿カメラ設定 {
//facecamera 1 1000000 10374 175 -6374 78 19700
//	facecamera 1 994500 9862 114 -10750 100 -10998
facecamera 1 994500 10171 34 -5249 46 10901

//	facelimit 1 40000 512 512 113 170
	facelimit 1 40000 45 45 60 70

/*
	facecamera 1 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
*/
}



proc 左側顔設定 {
/*
	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
*/
//	facelimit 0 40000 512 512 113 170

	facedelay 0 75 75
}

proc rt_タンカー左側設定 {
//	facelimit 0 40000 512 512 113 170
	facedelay 0 75 75
}

proc rt_タンカー右側設定 {
//	facelimit 1 40000 512 512 113 170
	facedelay 1 75 75
}


proc rpd_大佐＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  447 -568 -690
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_バグ大佐＿光設定 {
	@rpd_大佐＿光設定
/*
	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
*/
}

proc rpd_ローズ＿光設定 {
//	lightvec   0 30  -35  -40
//	lightvec 0  664 -439 -603
//	lightvec 0  488 -410 -769
//	lightvec 0  414 -360 -835
//	lightvec 0  -563 -251 -786
lightvec 0  -660 -110 -742

	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_プリスキン＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  566 -533 -628
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_スネーク＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  513 -502 -695
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rt_タンカースネーク＿光設定 {
//	lightvec   1 -30  -35  -40
	lightvec 1  -435 -205 -876
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
}


proc rpd_ピーター＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  874 -53 -482
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_オタコン＿光設定 {
//	lightvec   0 30  -35  -40
//	lightvec 0  357 -328 -874
	lightvec 0  338 -481 -808
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rt_タンカーオタコン＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  492 -155 -856
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}


proc rpd_忍者＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  294 -559 -774
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_オルガ＿光設定 {
//	lightvec   0 30  -35  -40
//	lightvec 0  814 -525 -246
lightvec 0  777 -301 -551

	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_エイムズ＿光設定 {
//	lightvec   0 30  -35  -40
	lightvec 0  88 -513 -853
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_大統領＿光設定 {
//	lightvec   0 30  -35  -40
//	lightvec 0  538 -538 -647
lightvec 0  -319 -344 -882

	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_エマ＿光設定 {
//	lightvec   0 30  -35  -40
//	lightvec 0  723 -618 -307
//	lightvec 0  496 -676 -544
//lightvec 0  -323 -178 -929
//	lightvec 0  -400 -407 -820 //ぼつ
lightvec 0  -463 -366 -806

	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_エマ＿光設定２ {
//	lightvec   0 30  -35  -40
//	lightvec 0  723 -618 -307
//	lightvec 0  496 -676 -544
//lightvec 0  -323 -178 -929
lightvec 0  -463 -366 -806

	lightcol   0 160  187   2
	ambientcol 0 39   57    110
}

proc rpd_エマ＿光設定プレイヤー側 {
//	lightvec   1 -30  -35  -40
	lightvec 1  -299 -603 -739
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
}

proc rpd_ライデン＿光設定 {
//	lightvec   1 -30  -35  -40
//	lightvec 1  -474 -496 -726
	lightvec 1  -492 -266 -828
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
}

proc rpd_潜水ライデン＿光設定 {
//	lightvec   1 -30  -35  -40
	lightvec 1  4 424 -905
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
}

proc rpd_裸ライデン＿光設定 {
	@rpd_ライデン＿光設定
/*
//	lightvec   1 -30  -35  -40
	lightvec 1  4 424 -905
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
*/
}

proc rpd_ゴルルコ兵マスクなしライデン＿光設定 {
	@rpd_ライデン＿光設定
/*
	lightvec   1 -30  -35  -40
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
*/
}

proc rpd_ゴルルコ兵マスクありライデン＿光設定 {
	@rpd_ライデン＿光設定
/*
	lightvec   1 -30  -35  -40
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
*/
}

/*
proc 右側顔設定 {
	lightvec   1 -30  -35  -40
	lightcol   1 160  187   2
	ambientcol 1 39   57    110
	facecamera 1 \
	  994500   \   // far
	  11100    \   // zoom
	  59       \   // heading
	  -3750    \   // pan
	  86       \   // pitch
	  0            // gain
	facelimit 1 40000 512 512 113 170
	facedelay 1 75 75
}

proc 左側顔設定 {
	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
	facelimit 0 40000 512 512 113 170

	facedelay 0 75 75
}
*/
proc rpd_ライデン＿デフォルトモーション設定１ {
	defmtn 1 cdc_rai // ライデンデフォルトモーション
}

proc rpd_大佐＿デフォルトモーション設定１ {
	defmtn 0 cdc_cam // 大佐デフォルトモーション
}

proc rpd_ローズ＿デフォルトモーション設定１ {
	defmtn 0 cdc_ros // ローズデフォルトモーション
}

proc rpd_オタコン＿デフォルトモーション設定１ {
	defmtn 0 cdc_otacon // オタコンデフォルトモーション
}

proc rt_タンカーオタコン＿デフォルトモーション設定１ {
	defmtn 0 cdc_otacon // オタコンデフォルトモーション
}

proc rpd_スネーク＿デフォルトモーション設定１ {
	defmtn 0 cdc_snake // スネークデフォルトモーション
}

proc rt_タンカースネーク＿デフォルトモーション設定１ {
	defmtn 1 cdc_snake // スネークデフォルトモーション
}



proc rpd_ピーター＿デフォルトモーション設定１ {
	defmtn 0 cdc_ptr // エマデフォルトモーション
}

proc rpd_エマ＿デフォルトモーション設定１ {
	defmtn 0 cdc_ema // エマデフォルトモーション
}

proc rpd_エマ＿デフォルトモーション設定１プレイヤー側１ {
	defmtn 1 cdc_ema // エマデフォルトモーション
}

proc rpd_エイムズ＿デフォルトモーション設定１ {
	defmtn 0 cdc_ric // エイムズ デフォルトモーション
}

proc rpd_大統領＿デフォルトモーション設定１ {
	defmtn 0 cdc_jam // 大統領 デフォルトモーション
}

proc rpd_オルガ＿デフォルトモーション設定１ {
	defmtn 0 cdc_org_plant // 大統領 デフォルトモーション
}

proc rpd_忍者＿デフォルトモーション設定１ {
	defmtn 0 cdc_org_tng // 大統領 デフォルトモーション
}

proc rpd_ライデン顔設定１ {
	// ライデン設定
	face       1 rai_radio_mh_mt ライデン
	eyeclose	1	lip_rai_nom_eye_toji
	hair	   1 -n rai_hair_mh_mt -d 1 -b rai_hair_bounding  // ライデン髪の毛設定

	@rpd_ライデン＿デフォルトモーション設定１
	@rpd_ライデン＿光設定
	@右側顔設定
	@ライデン＿カメラ設定
}

proc rpd_潜水ライデン顔設定１ {
	// ライデン設定
	face       1 rai_radio_diver_mh_mt ライデン
	eyeclose	1	lip_rai_nom_eye_toji
	hair	   1 -n rai_hair_diver_mh_mt -d 15 -b rai_diver_bounding 
	@rpd_ライデン＿デフォルトモーション設定１
	@rpd_潜水ライデン＿光設定
	@右側顔設定
	@潜水ライデン＿カメラ設定
}

proc rpd_裸ライデン顔設定１ {
	// 裸ライデン設定
	face       1 rai_radio_naked_mh_mt ライデン
	eyeclose	1	lip_rai_nom_eye_toji
	hair	   1 -n rai_hair_mh_mt -d 1  -b rai_hair_bounding 
	hair	   1 -n rai_dogtag_naked_mh -d 25 -b rai_dogtag_bounding
	@rpd_ライデン＿デフォルトモーション設定１
	@rpd_裸ライデン＿光設定
	@右側顔設定
	@裸ライデン＿カメラ設定
}

proc rpd_ゴルルコ兵マスクなしライデン顔設定１ {
	face       1 rai_radio_gbsbody_mh_mt ライデン
	eyeclose	1	lip_rai_nom_eye_toji
	hair	   1 -n rai_hair_mh_mt -d 1 -b rai_hair_bounding  // ライデン髪の毛設定
	@rpd_ライデン＿デフォルトモーション設定１
	@rpd_ゴルルコ兵マスクなしライデン＿光設定
	@右側顔設定
	@ライデン＿カメラ設定
}

proc rpd_ゴルルコ兵マスクありライデン顔設定１ {
	face       1 rai_radio_gbshead_mh ライデン
	eyeclose	1	lip_rai_nom_eye_toji
	@rpd_ライデン＿デフォルトモーション設定１
	@rpd_ゴルルコ兵マスクありライデン＿光設定
	@右側顔設定
	@ライデン＿カメラ設定
}

proc rpd_大佐共通設定 {
	face       0 cam_radio_mh_mt 大佐
	eyeclose	0	lip_cam_nom_eye_toji

	@rpd_大佐＿デフォルトモーション設定１
	@rpd_大佐＿光設定
	@左側顔設定
}

proc rpd_バグ大佐共通設定 {
	// バグ大佐設定
	face       0 cam_skull_mh_mt 大佐
	eyeclose	0	lip_cam_nom_eye_toji

	@rpd_大佐＿デフォルトモーション設定１
	@rpd_バグ大佐＿光設定
	@左側顔設定
}


proc rpd_大佐顔設定１ {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定１)
	@rpd_大佐共通設定
	@大佐＿カメラ設定＿普通
}

proc rpd_大佐顔設定＿右 {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿右)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿右
	}
}

proc rpd_大佐顔設定＿左 {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿左)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿左
	}
}

proc rpd_大佐顔設定＿後ろ {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿後ろ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿後ろ
	}
}

proc rpd_大佐顔設定＿正面 {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿正面)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿正面
	}
}

proc rpd_大佐顔設定＿アオリ {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿アオリ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿アオリ
	}
}

proc rpd_大佐顔設定＿アップ {
	// 大佐設定
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:大佐顔設定＿アップ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_大佐共通設定
		@大佐＿カメラ設定＿アップ
	}
}


proc rpd_バグ大佐顔設定１ {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定１)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿普通
	}
}

proc rpd_バグ大佐顔設定＿右 {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿右)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿右
	}
}

proc rpd_バグ大佐顔設定＿左 {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿左)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿左
	}
}

proc rpd_バグ大佐顔設定＿後ろ {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿後ろ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿後ろ
	}
}

proc rpd_バグ大佐顔設定＿正面 {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿正面)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿正面
	}
}

proc rpd_バグ大佐顔設定＿アオリ {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿アオリ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿アオリ
	}
}

proc rpd_バグ大佐顔設定＿アップ {
	eval($b:rfp_PC_大佐顔状態 = d:PC_大佐顔状態:バグ大佐顔設定＿アップ)
	if($b:rfp_PC_一つ前の大佐顔状態 != $b:rfp_PC_大佐顔状態){
		@rpd_バグ大佐共通設定
		@大佐＿カメラ設定＿アップ
	}
}


proc rpd_ローズ顔設定１ {
	// ローズ設定
	face       0 ros_radio_mh_mt ローズ
	hair	   0 -n ros_hair_f_mh_mt -d 16 -b ros_bounding -nowind // ローズ髪の毛設定
	eyeclose	0	lip_ros_nom_eye_toji
	@rpd_ローズ＿デフォルトモーション設定１ // ローズ デフォルトモーション
	@rpd_ローズ＿光設定
	@左側顔設定
	@ローズ＿カメラ設定
}


proc rpd_プリスキン顔設定１ {
	// プリスキン設定 
	face       0 iro_radio_mh_mt プリスキン
	eyeclose	0	lip_sna_nom_eye_toji
	@rpd_スネーク＿デフォルトモーション設定１// スネークデフォルトモーション

	@rpd_プリスキン＿光設定
	@左側顔設定
	@プリスキン＿カメラ設定
}

proc rpd_スネーク顔設定１ {
	// スネーク設定 
	if ( $w:プレイヤーフラグ & d:PL_GAVE_SHAVER_TO_SNAKE ) {
		face       0 sna_radio_shaved_mh_mt プリスキン	//ひげそりっす。
	}else {
		face       0 sna_radio_mh_mt プリスキン
	}
	eyeclose	0	lip_sna_nom_eye_toji
	@rpd_スネーク＿デフォルトモーション設定１// スネークデフォルトモーション
	@rpd_スネーク＿光設定
	@左側顔設定
	@スネーク＿カメラ設定
}

proc rpd_ひげそりスネーク顔設定１ {
	// スネーク設定 
	face       0 sna_radio_shaved_mh_mt プリスキン
	eyeclose	0	lip_sna_nom_eye_toji
	@rpd_スネーク＿デフォルトモーション設定１// スネークデフォルトモーション
	@rpd_スネーク＿光設定
	@左側顔設定
	@スネーク＿カメラ設定
}

proc rpd_ピーター共通 {
	face       0 ptr_radio_mh_mt ピーター
	eyeclose	0	lip_ptr_nom_eye_toji
	hair 0 -n ptr_food_mh_mt -d 12 -b ptr_dummy -nowind
	@rpd_ピーター＿デフォルトモーション設定１// ピーターデフォルトモーション
	@rpd_ピーター＿光設定
	@左側顔設定
}

proc rpd_ピーター顔設定１ {
	// ピーター設定 
	@rpd_ピーター共通
	@ピーター＿カメラ設定
}

proc rpd_ピーター顔設定＿寄り {
	// ピーター設定 
	@rpd_ピーター共通
	@ピーター＿カメラ設定＿寄り
}


proc rpd_オタコン＿眼鏡設定 {
	object 	0 \
			otc_glasses_mt \
//			otc_glass_sh_50hlf_mt \
			オタコン \
			0 \
			12
}

proc rpd_オタコン顔設定１ {
	// オタコン設定
	face       0 otc_radio_mh_mt オタコン
	eyeclose	0	lip_otc_nom_eye_toji
	hair 0 -n otc_hair_mh -d 28 -b otc_dummy -nowind
	@rpd_オタコン＿デフォルトモーション設定１ // オタコンデフォルトモーション
	@rpd_オタコン＿眼鏡設定
	@rpd_オタコン＿光設定
	@左側顔設定
	@オタコン＿カメラ設定
}


proc rpd_忍者顔設定１ {
	// 忍者設定
	face       0 org_tng_radio_mh_mt ミスターＸ
	@rpd_忍者＿光設定
	@左側顔設定
	@rpd_忍者＿デフォルトモーション設定１
	@忍者＿カメラ設定
}

proc rpd_ノイズ顔設定１ {
	noise 0 1 
	@rpd_忍者顔設定１
}

proc rpd_オルガ顔設定１ {
	// オルガ設定
	face       0 org_plant_radio_mh_mt オルガ	//本当はオルガ。
	hair 0 -n org_plant_hair_mh -d 20 -b org_plant_dummy -nowind
	eyeclose	0	lip_org_nom_eye_toji
	@rpd_オルガ＿光設定
	@rpd_オルガ＿デフォルトモーション設定１
	@左側顔設定
	@オルガ＿カメラ設定
}


proc rpd_エイムズ顔設定１ {
	// エイムズたん設定
	face 0 ric_radio_mh エイムズ
	eyeclose	0	lip_ric_nom_eye_toji
	@rpd_エイムズ＿デフォルトモーション設定１
	@rpd_エイムズ＿光設定
	@左側顔設定
	@エイムズ＿カメラ設定
}


proc rpd_大統領顔設定１ {
	// 大統領設定
	face       0 jam_radio_mh_mt 大統領 //仮です
	eyeclose	0	lip_jam_nom_eye_toji
	@rpd_大統領＿光設定
	@左側顔設定
	@rpd_大統領＿デフォルトモーション設定１
	@大統領＿カメラ設定
}


proc rpd_エマ＿眼鏡設定 {
	object 	0 \
			ema_glasses_mt \
			エマ \
			0 \
			12

}

proc rpd_エマ＿眼鏡設定プレイヤー側 {
	object 	1 \
			ema_glasses_mt \
			エマ \
			0 \
			12
}

proc rpd_エマ顔設定１ {
	// エマ設定
	face       0 ema_radio_mh_mt エマ //仮です
	eyeclose	0	lip_ema_nom_eye_toji
	hair 0 -n ema_hair_mh -d 17 -b ema_bounding
	if($w:p_story < d:ST:P063_01_P01カード五１ポリゴンデモ１開始){
		hair 0 -n ema_idrope_mh_mt -d 18 -b ema_bounding
	}
	if(! ( ($s:エマ存在ステージ == "w31f") || \
			($s:エマ存在ステージ == "w31b")  || \
			(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P062_08_P04エマとＡＩ８ポリゴンデモ４終了) ) ){
		@rpd_エマ＿眼鏡設定
	}
	@rpd_エマ＿デフォルトモーション設定１ 
	@rpd_エマ＿光設定
	@左側顔設定
	@エマ＿カメラ設定
}


proc rpd_エマ顔設定１プレイヤー側１ {
	// エマ設定
	face       1 ema_radio_mh_mt エマ //仮です
	hair 1 -n ema_hair_mh -d 17 -b ema_bounding
	eyeclose	1	lip_ema_nom_eye_toji
	if($w:p_story < d:ST:P063_01_P01カード五１ポリゴンデモ１開始){
		hair 1 -n ema_idrope_mh_mt -d 18 -b ema_bounding
	}
	if(! ( ($s:エマ存在ステージ == "w31f") || \
			($s:エマ存在ステージ == "w31b")  || \
			(d:ST:P058_08_P04エマ救出８ポリゴンデモ４終了 <= $w:p_story && \
			$w:p_story < d:ST:P062_08_P04エマとＡＩ８ポリゴンデモ４終了) ) ){
		@rpd_エマ＿眼鏡設定プレイヤー側
	}
	@rpd_エマ＿デフォルトモーション設定１プレイヤー側１ 
	@rpd_エマ＿光設定プレイヤー側
	@右側顔設定
	@エマプレイヤー側＿カメラ設定
}

proc rt_タンカースネーク顔設定１ {
	face       1 sna_radio_mh_mt スネーク
	eyeclose	1	lip_sna_nom_eye_toji
	@rt_タンカースネークカメラ設定
	@rt_タンカースネーク＿デフォルトモーション設定１
	@rt_タンカースネーク＿光設定
	@rt_タンカー右側設定
}

proc rt_タンカーオタコン顔設定１ {
	face       0 otc_radio_mh_mt オタコン
	eyeclose	0	lip_otc_nom_eye_toji
	hair 0 -n otc_hair_mh -d 28  -b otc_dummy -nowind
	@rt_タンカーオタコン＿デフォルトモーション設定１
	@rt_タンカーオタコンカメラ設定
	@rpd_オタコン＿眼鏡設定
	@rt_タンカーオタコン＿光設定
	@rt_タンカー左側設定
}



/*
//これはどーでしょう？
proc rpd_モーション設定デフォルト {
	defmtn 0 cdc_otacon // オタコンデフォルトモーション
	defmtn 1 cdc_snake // スネークデフォルトモーション
}
*/


proc rpd_ライデン顔設定デフォルト {
	if( ($w:p_story < d:ST:P005_01_P01ライデン昇降機上昇１ポリゴンデモ１終了) && \
		($s:エリア == "w11a")  ){
		@rpd_潜水ライデン顔設定１
	
	}else if(d:ST:P070_11_M02ＡＧ起動１１ムービーデモ２終了 <= $w:p_story && \
				$w:p_story < d:ST:P073_02_P01通路Ａ２ポリゴンデモ１開始 ) {
		@rpd_裸ライデン顔設定１

	}else {
		if($w:アイテム == 6){
			//マスクついてるよ
			if( d:ST:P036_13_R04忍者登場１３無線デモ４終了 <= $w:p_story && \
					$w:p_story < d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了) {
				@rpd_ゴルルコ兵マスクありライデン顔設定１	

			//マスクとられちゃいました。
			}else {
				@rpd_ゴルルコ兵マスクなしライデン顔設定１	
			}

		}else {
			@rpd_ライデン顔設定１
		}
	}
}

proc rpd_ライデン顔表示デフォルト {
	@rpd_ライデン顔設定デフォルト
	dispen	//顔表示開始
}

proc rpd_ライデン顔表示 {
	@rpd_ライデン顔設定１
	dispen	//顔表示開始
}

proc rpd_裸ライデン顔表示 {
	@rpd_裸ライデン顔設定１
	dispen	//顔表示開始
}

proc rpd_潜水ライデン顔表示 {
	@rpd_潜水ライデン顔設定１	
	dispen	//顔表示開始
}

proc rpd_ゴルルコ兵マスクなしライデン顔表示 {
	@rpd_ゴルルコ兵マスクなしライデン顔設定１	
	dispen	//顔表示開始
}

proc rpd_ゴルルコ兵マスクありライデン顔表示 {
	@rpd_ゴルルコ兵マスクありライデン顔設定１	
	dispen	//顔表示開始
}

proc rpd_大佐顔表示 {
	@rpd_大佐顔設定１
	dispen	//顔表示開始
}

proc rpd_バグ大佐顔表示 {
	@rpd_バグ大佐顔設定１
	dispen	//顔表示開始
}

proc rpd_ローズ顔表示 {
	@rpd_ローズ顔設定１
	dispen	//顔表示開始
}

proc rpd_プリスキン顔表示 {
	print 'ぷりすきん'
	@rpd_プリスキン顔設定１
	dispen	//顔表示開始
}

proc rpd_スネーク顔表示 {
	@rpd_スネーク顔設定１
	dispen	//顔表示開始
}

proc rpd_ひげそりスネーク顔表示 {
	@rpd_ひげそりスネーク顔設定１
	dispen	//顔表示開始
}


proc rpd_プリスキン顔設定デフォルト {
	if($w:p_story >= d:ST:P073_02_P01通路Ａ２ポリゴンデモ１終了 ){
		@rpd_スネーク顔設定１
	}else {
		@rpd_プリスキン顔設定１
	}
}

proc rpd_ピーター顔表示 {
	@rpd_ピーター顔設定１ //本当はピーター
	dispen	//顔表示開始
}

proc rpd_オタコン顔表示 {
	print 'オタコン'
	@rpd_オタコン顔設定１
	dispen	//顔表示開始
}

proc rt_タンカーオタコン顔表示 {
	@rt_タンカーオタコン顔設定１
	dispen	//顔表示開始
}

proc rt_タンカーメイリンノイズ表示 {
	noise 0 1
	@rt_タンカーオタコン顔表示
}

proc rt_タンカーメイリンノイズ消去 {
	@rt_タンカーオタコン顔表示
	noise 0 0
}

proc rpd_オタコン顔表示プレイヤー側 {
	print 'オタコン ぷれいやがわん'
	//@rpd_オタコン顔設定１
	//dispen	//顔表示開始
}



proc rpd_ノイズ顔表示 {
	print '本当はノイズです'
	@rpd_ノイズ顔設定１
	dispen	//顔表示開始
}

proc rpd_忍者顔表示 {
	@rpd_忍者顔設定１
	dispen	//顔表示開始
}

proc rpd_オルガ顔表示 {
	@rpd_オルガ顔設定１
	dispen	//顔表示開始
}

proc rpd_エイムズ顔表示 {
	@rpd_エイムズ顔設定１
	dispen	//顔表示開始
}


proc rpd_大統領顔表示 {
	@rpd_大統領顔設定１
	dispen	//顔表示開始
}

proc rpd_エマ顔表示 {
	@rpd_エマ顔設定１
	dispen	//顔表示開始
}

proc rpd_エマ顔表示プレイヤー側 {
	@rpd_エマ顔設定１プレイヤー側１
	dispen	//顔表示開始
}

proc rpd_顔表示 {
	dispen	//顔表示開始
}


