/*
	w45a_d_common.h
	    アーセナルギア小部屋Ｂ　デモ／ゲーム共通プロック

	2000/12/06 H.Yoshiike         
	$Id: w45a_d_common.h,v 1.9 2002/05/09 13:07:50 usr01475 Exp $                      

	
*/



//###################################################################################
//									 ドア＆マップ切り替え
//###################################################################################

	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		chara ドア 南側ドア \
			-m d:DOOR_NAME_LOAD1A \
			-d 0,0,0 \
			-p 1250,0, -133125 \
			-slide d:SD_SLIDE_ARSENAL_LEFT \
			-time (d:SD_OPEN_TIME + 10)\
			-between アーセナルギア小部屋Ｂ アーセナルギア小部屋Ｂ \
			-w_door d:DOOR_NAME_LOAD1B d:SD_SLIDE_ARSENAL_RIGHT \
			-h 1500 \
			-exec {
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ ロードエリア off
				} else {
					mesg プットオブジェ ロードエリア on
				}
			}

		//	ドアは開かない
		if ($f:w45a_天狗兵登場フラグ == d:TRUE) {
			mesg ドア 南側ドア locked
		}

		trap dr001 ？ \
			-mask ？ \	
			-exec {
				if ($3 == 入る) {
					mesg ドア 南側ドア open $2
				} else {
					mesg ドア 南側ドア close $2
				}
			}

	}




//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

		chara フォグ 霧\
			-c 143,186,159 \
			-n 0 \
			-f 80000
	}

	proc 空中文字設定 {
		chara 円筒テクスチャ 空中帯１−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin 0 18000		// 軸回転
			-spin 0 36000		// 軸回転

		chara 円筒テクスチャ 空中帯１−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin 683 18000		// 軸回転
			-spin 683 36000		// 軸回転

		chara 円筒テクスチャ 空中帯１−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin 1362 18000		// 軸回転
			-spin 1362 36000		// 軸回転

		chara 円筒テクスチャ 空中帯１−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin 2048 18000		// 軸回転
			-spin 2048 36000		// 軸回転

		chara 円筒テクスチャ 空中帯１−５ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin -1362 18000		// 軸回転
			-spin -1362 36000		// 軸回転

		chara 円筒テクスチャ 空中帯１−６ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8750  \
			-posy 2000  \
			-width 7200 \
			-height  500 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_02_alp_add_ovl \
//			-spin -683 18000		// 軸回転
			-spin -683 36000		// 軸回転


		chara 円筒テクスチャ 空中帯２−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin 0 -18000		// 軸回転
			-spin 0 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin 512 -18000		// 軸回転
			-spin 512 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin 1024 -18000		// 軸回転
			-spin 1024 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin 1536 -18000		// 軸回転
			-spin 1536 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−５ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin 2048 -18000		// 軸回転
			-spin 2048 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−６ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin -512 -18000		// 軸回転
			-spin -512 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−７ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin -1024 -18000		// 軸回転
			-spin -1024 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯２−８ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 8250  \
			-posy 1650  \
			-width 4800 \
			-height  300 \
			-inner 4096 \
			-tex f_window w45a_efct_rbn_01_alp_add_ovl \
//			-spin -1536 -18000		// 軸回転
			-spin -1536 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯３−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 1200  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  1024 18000		// 軸回転
			-spin  1024 0		// 軸回転

		chara 円筒テクスチャ 空中帯３−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 1200  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  3072 18000		// 軸回転
			-spin  3072 0		// 軸回転

		chara 円筒テクスチャ 空中帯３−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 1200  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
//			-spin  0 18000		// 軸回転
			-spin  0 0		// 軸回転

		chara 円筒テクスチャ 空中帯３−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 1200  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
//			-spin  2048 18000		// 軸回転
			-spin  2048 0		// 軸回転

		chara 円筒テクスチャ 空中帯４−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 3900  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  1024 -18000		// 軸回転
			-spin  1024 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯４−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 3900  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  3072 -18000		// 軸回転
			-spin  3072 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯４−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 3900  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_3_alp_add_ovl \
//			-spin  0 -18000		// 軸回転
			-spin  0 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯４−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 3900  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_3_alp_add_ovl \
//			-spin  2048 -18000		// 軸回転
			-spin  2048 -36000		// 軸回転

		chara 円筒テクスチャ 空中帯５−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 8800  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  2048 -18000		// 軸回転
			-spin  2048 -54000		// 軸回転

		chara 円筒テクスチャ 空中帯５−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 8800  \
			-width 15572 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  4096 -18000		// 軸回転
			-spin  4096 -54000		// 軸回転

		chara 円筒テクスチャ 空中帯５−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 8800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_3_alp_add_ovl \
//			-spin  1024 -18000		// 軸回転
			-spin  1024 -54000		// 軸回転

		chara 円筒テクスチャ 空中帯５−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5275  \
			-posy 8800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_3_alp_add_ovl \
//			-spin  3072 -18000		// 軸回転
			-spin  3072 -54000		// 軸回転

#if 0
	//	原形
		chara 円筒テクスチャ 空中帯６−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 22562 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  1024 18000		// 軸回転
			-spin  1024 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 22562 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
//			-spin  3072 18000		// 軸回転
			-spin  3072 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
//			-spin  0 18000		// 軸回転
			-spin  0 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
//			-spin  2048 18000		// 軸回転
			-spin  2048 54000		// 軸回転

#else 

	//	分割
		chara 円筒テクスチャ 空中帯６−１Ａ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 11281 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
			-spin  534 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−１Ｂ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 11281 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
			-spin  1514 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−２Ａ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 11281 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
			-spin  2582 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−３Ａ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 11281 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_alp_add_ovl \
			-spin  3562 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−３ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
			-spin  0 54000		// 軸回転

		chara 円筒テクスチャ 空中帯６−４ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 7500  \
			-posy 6800  \
			-width 1000 \
			-height  200 \
			-tex f_window w45a_efct_rbn_smt01_2_alp_add_ovl \
			-spin  2048 54000		// 軸回転

#endif




		//	ＣＡＵＴＩＯＮ	右上
		chara 円筒テクスチャ 空中文字１-１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 6750  \
			-posy 2570  \
			-width 800 \
			-height 288 \
			-tex f_window w45a_efct_caution_alp_add_ovl \
			-blink \
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-spin  1650 0		// 軸回転

		//	ＣＡＵＴＩＯＮ	左上
		chara 円筒テクスチャ 空中文字１-２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000  \
			-r 6000  \
			-posy 1100  \
			-width 800 \
			-height 288 \
			-tex f_window w45a_efct_caution_alp_add_ovl \
			-blink \
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-spin  2674 0		// 軸回転



#if 0
		//	メニューっぽいやつ
		chara 円筒テクスチャ 空中文字２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
			-r 5410  \
			-posy 0  \
			-width 1000 \
			-height 2000 \
			-tex f_window w45a_efct_disp_06_alp_add_ovl \
			-blink \
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-spin  2389 0		// 軸回転

		//	スクロール系	正面
		chara 空中文字 空中文字３−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000 \
			-r 6000 \
			-posy 2600 \
			-width 1000 \
			-height 562 \
			-tex f_window w45a_efct_disp_01_alp_add_ovl w45a_efct_disp_01_scr_alp_add_ovl \
			-chr 0 295 1000 636 \
			-spin 2048 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 350 \
				0 1000 650 350

		//	スクロール系	左下
		chara 空中文字 空中文字３−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000 \
			-r 6000 \
			-posy 3000 \
			-width 1000 \
			-height 562 \
			-tex f_window w45a_efct_disp_01_alp_add_ovl w45a_efct_disp_01_scr_alp_add_ovl \
			-chr 0 295 1000 636 \
			-spin 3427 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 350 \
				0 1000 650 350


		//	スクロール系	右上
		chara 空中文字 空中文字４−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 6700 \
			-r 5700 \
			-posy 1200 \
			-width 2000 \
			-height 1250 \
			-tex f_window w45a_efct_disp_02_alp_add_ovl w45a_efct_disp_02_scr_alp_add_ovl \
			-chr 0 194 1000 722 \
			-spin 1479 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 541 \
				0 1000 459 541

		//	スクロール系	左下
		chara 空中文字 空中文字４−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 6700 \
			-r 5700 \
			-posy 1200 \
			-width 2000 \
			-height 1250 \
			-tex f_window w45a_efct_disp_02_alp_add_ovl w45a_efct_disp_02_scr_alp_add_ovl \
			-chr 0 194 1000 722 \
			-spin 3243 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 541 \
				0 1000 459 541


		//	スクロール系	右上
		chara 空中文字 空中文字５−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 6250 \
			-r 5250 \
			-posy 1600 \
			-width 1200 \
			-height 562 \
			-tex f_window w45a_efct_disp_03_alp_add_ovl w45a_efct_disp_03_scr_alp_add_ovl \
			-chr 0 90 1000 727 \
			-spin 1650 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 500 \
				0 1000 500 500

		//	スクロール系	右下
		chara 空中文字 空中文字５−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 6250 \
			-r 5250 \
			-posy 1600 \
			-width 1200 \
			-height 562 \
			-tex f_window w45a_efct_disp_03_alp_add_ovl w45a_efct_disp_03_scr_alp_add_ovl \
			-chr 0 90 1000 727 \
			-spin 398 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 500 \
				0 1000 500 500


		//	スクロール系	右下
		chara 空中文字 空中文字６−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000 \
			-r 6000 \
			-posy 1000 \
			-width 1400 \
			-height 862 \
			-tex f_window w45a_efct_disp_04_alp_add_ovl w45a_efct_disp_04_scr_alp_add_ovl \
			-chr 0 63 1000 875 \
			-spin 569 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 656 \
				0 1000 344 656

		//	スクロール系	左上
		chara 空中文字 空中文字６−２ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000 \
			-r 6000 \
			-posy 1000 \
			-width 1400 \
			-height 862 \
			-tex f_window w45a_efct_disp_04_alp_add_ovl w45a_efct_disp_04_scr_alp_add_ovl \
			-chr 0 63 1000 875 \
			-spin 2503 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 656 \
				0 1000 344 656


		//	スクロール系	左上
		chara 空中文字 空中文字７−１ \
			-alpha 64 \
			-O 0,0,-149250 \  // 起点
//			-r 7000 \
			-r 6000 \
			-posy 2500 \
			-width 1000 \
			-height 1850 \
			-tex f_window w45a_efct_disp_05_alp_add_ovl w45a_efct_disp_05_scr_alp_add_ovl \
			-chr 0 78 1000 859 \
			-spin 1536 0 \		// 軸回転
			-plate \	// 円筒ではなく、円筒に対する接面として作成
			-uvanim 3000 \
				0 1000 0 658 \
				0 1000 342 658

#else
	chara 天狗兵ステージモニター モニタ

#endif

	}

