/*
	w15_d_common.h                     
		デモ・ゲーム用共通プロック

	1999/12/09 Y.Matsuhana         
	$Id: w15_d_common.h,v 1.5 2001/09/20 06:25:59 usr03379 Exp $                      

	
*/


//###################################################################################
//									オブジェクト
//###################################################################################

	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif
		chara ドア Ｂ脚ドア \
			-m d:DOOR_NAME1 \
			-d 0 0 0 \
			-p -55500 ,0,-43625 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＢＣ連絡橋 ＢＣ連絡橋 \
			-A Ｂ脚ドアランプ \
			-exec{
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｂ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｂ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｂ脚ドアランプ 0 -55500 0 -43625 1

		trap dr001 ？ -mask ？ \
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｂ脚ドア open $2
				} else if ($3 == 出る) {
					mesg ドア Ｂ脚ドア close $2
				}
			}
		chara ドア Ｃ脚ドア \
			-m d:DOOR_NAME1 \
			-d 0 0 0 \
			-p -56750,0,-76375 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between ＢＣ連絡橋 ＢＣ連絡橋 \
			-A Ｃ脚ドアランプ \
			-exec{
				if ( !($:開閉フラグ) ) {
					mesg プットオブジェ Ｃ脚ロードエリア off
				} else {
					mesg プットオブジェ Ｃ脚ロードエリア on
				}
			}
		@ドアランプ設定 Ｃ脚ドアランプ 0 -56750 0 -76375 0

		trap dr002 ？ -mask ？ \
			-exec {
				if ($3 == 入る) {
					mesg ドア Ｃ脚ドア open $2
				} else if ($3 == 出る) {
					mesg ドア Ｃ脚ドア close $2
				}
			}
	}

	proc 海面設置 {
		#if d:DEBUG_PRINT
			print 'w15a_sea_set'
		#endif

	//	朝
	#if d:W15A
		chara プラント海面 seaseasea \
			-tex oil m_oil_sea_alp_ovl_mod0222 m_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
//			-pos -52000,-40000,-60000
			-pos -62000,-40000,-60000

	//	昼
	#else
		chara プラント海面 seaseasea \
			-tex oil d_oil_sea_alp_ovl_mod0222 d_oil_ref_add_alp_ovl_mod1120 \
			-w 1,2,3 \
			-l \
//			-pos -52000,-40000,-60000
			-pos -62000,-40000,-60000
	#endif

	}

	proc 空設置 {
		#if d:DEBUG_PRINT
			print 'w15a_sky_set'
		#endif

	//	朝
	#if d:W15A

		@朝空（レンズフレア指定）	-52000 -40000 -60000 2200 -1400 4300 12

	//	昼
	#else
//		@昼空（レンズフレア指定）	-52000 -40000 -60000 -2000 -4000 2200 12
		@昼空（レンズフレア指定）	-52000 -40000 -60000 -2600 -3900 1900 12

	#endif

	}



//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif
/*
		chara フォグ fog \
			 -c 194 199 163 \
			 -n -13000 \
			 -f 105000
*/

	//	朝
	#if d:W15A

	//	明るさ可変機能
	@明るさ可変機能 194 199 163 -20000 175000 ha002 0


	//	昼
	#else
	@明るさ可変機能 220 237 198 -15000 175000 ha002 0


	#endif

	}

