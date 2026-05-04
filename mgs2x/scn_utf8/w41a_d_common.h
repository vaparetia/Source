/*
	w41a_d_common.h
	    拷問部屋　デモ／ゲーム共通プロック

	2000/11/28 H.Yoshiike         
	$Id: w41a_d_common.h,v 1.2 2001/08/02 05:16:49 usr03379 Exp $                      

	
*/



//###################################################################################
//									オブジェクト
//###################################################################################

	proc ドア設置 {
		#if d:DEBUG_PRINT
			print 'slide_door_set'
		#endif

		chara ドア 廊下東ドア \
			-m d:DOOR_NAME_LOAD1A \
			-d 0,3072,0 \
			-p 10625,0,-8000 \
			-slide d:SD_SLIDE_ARSENAL_LEFT \
			-time d:SD_OPEN_TIME \
			-between 拷問部屋 拷問部屋 \
			-w_door d:DOOR_NAME_LOAD1B d:SD_SLIDE_ARSENAL_RIGHT \
			-h 1500 \
			-exec {
				if ( !($:開閉フラグ) ){
					mesg プットオブジェ 拷問部屋ロードエリア off
				} else {
					mesg プットオブジェ 拷問部屋ロードエリア on
				}
			}
		trap dr001 ？ \
			-mask ？ \	
			-exec {
				if ($3==入る) {
					mesg ドア 廊下東ドア open $2
				} else {
					mesg ドア 廊下東ドア close $2
				}
			}

		chara ドア 廊下西ドア \
			-m d:DOOR_NAME2 \
			-d 0,1024,0 \
			-p 4125,0,-4500 \
			-slide (0-d:SD_SLIDE_OUTSIDE) \
			-time d:SD_OPEN_TIME \
			-between 拷問部屋 拷問部屋 \
			-exec {
				if ( !($:開閉フラグ) ){
					mesg プットオブジェ 廊下西ドアランプ青 off
					mesg プットオブジェ 廊下西ドアランプ赤 on
				} else {
					mesg プットオブジェ 廊下西ドアランプ青 on
					mesg プットオブジェ 廊下西ドアランプ赤 off
				}

			}
		trap dr002 ？ \
			-mask ？ \	
			-exec {
				if ($3==入る) {
					mesg ドア 廊下西ドア open $2
				} else {
					mesg ドア 廊下西ドア close $2
				}
			}

/*
		//	必要なし
		chara ドア トイレドア \
			-m d:DOOR_NAME \
			-d 0,2048,0 \
			-p 6000,0,-125 \
			-slide d:SD_SLIDE_OUTSIDE \
			-time d:SD_OPEN_TIME \
			-between 拷問部屋 拷問部屋 \
			-exec {
			}
*/
		chara ドア 独房ドア \
			-m d:DOOR_NAME \
			-d 0,0,0 \
			-p -5500,0,1875 \
			-slide (0-d:SD_SLIDE_OUTSIDE) \
			-time d:SD_OPEN_TIME \
			-between 拷問部屋 拷問部屋 \
			-card_level 9999 \			//	無理矢理開かなくする
			-exec {
			}
		trap dr003 ？ \
			-mask ？ \	
			-exec {
				if ($3 == 入る) {
					mesg ドア 独房ドア open $2
					@ドアランプ点滅処理
				} else {
					mesg ドア 独房ドア close $2
				}
			}
	}

	proc ドアランプ点滅処理 {
		chara プロック連続実行 ドア点滅管理人 \
			-t 90 \
			-p ドアランプ点滅 \
			-end_exec {
				mesg プットオブジェ 独房ドアランプ赤 on
			}
	}

	proc ドアランプ点滅 {
		chara delay ドアランプ点滅ディレイ \
			-t 30 \
			-e {
				if ($f:w41a_ドアランプフラグ == d:FALSE) {
					eval($f:w41a_ドアランプフラグ = d:TRUE )
					mesg プットオブジェ 独房ドアランプ赤 off
				} else {
					eval($f:w41a_ドアランプフラグ = d:FALSE )
					mesg プットオブジェ 独房ドアランプ赤 on
				}
			}
	}

	proc オブジェクト設定 {
		//	股間隠し系
		chara プットオブジェ コーラ -m d:CAN \
			-r 0,0,0 \
			-p -1500,1000,1950  -l d:LT2_NAME \
			-s 100,100,100
		chara プットオブジェ ストロー -m d:STRAW \
			-r 0,512,0 \
//			-p (-1500-25),(1000+160),1950  -l d:LT2_NAME \
			-p (-1490),1120,1970  -l d:LT2_NAME \
			-s 100,100,100

		//	ロードエリア
		chara プットオブジェ 拷問部屋ロードエリア \
			-m 	d:KMS_NAME2 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
		mesg プットオブジェ 拷問部屋ロードエリア off

		//	ドアランプ
		//	廊下西ドアランプ
		chara プットオブジェ 廊下西ドアランプ赤 \
			-m 	d:KMS_NAME3 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME

		chara プットオブジェ 廊下西ドアランプ青 \
			-m 	d:KMS_NAME4 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME
			mesg プットオブジェ 廊下西ドアランプ青 off

		//	独房ドアランプ
		chara プットオブジェ 独房ドアランプ赤 \
			-m 	d:KMS_NAME5 -r 0 0 0 -p 0 0 0  -l d:LT2_NAME

	}

	proc 天井設定 {
		#if d:DEBUG_PRINT
			print 'w41a_ceil_set'
		#endif

		chara 天井君 拷問部屋天井 -m d:CEIL_NAME0 -h 4000 -p 0,0,0 -l d:LT2_NAME
	}

//	監視カメラ
	proc 監視カメラ設定 {

	/*
		chara プラントカメラ  監視カメラ:01 \
		-pos -5000,1500,5500 \
		-rot 300 512 0 \ 	// ＸＹ値のみ有効
		-dir 0 \ /*片側角度*/
		-center 0 0 \ /*初期方向は符号付き相対指定*/
		-i 1
		

		mesg プラントカメラ 監視カメラ:01 視界オフ
	*/
		chara プットオブジェ ダミー監視カメラ \
			-m svc_plant \
			-p -4950,3250,-5430 \
			-r 450 384 0 \
			-s 100 100 100 \
			-l d:LT2_NAME

	}



//###################################################################################
//									 エフェクト
//###################################################################################

	proc フォグ設定 {
		#if d:DEBUG_PRINT
			print 'fog_set'
		#endif

		chara フォグ 霧\
			-c 8, 35, 30 \
			-n 1000 \
			-f 18000 
	}


