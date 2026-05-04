/*
	demo_stdproc.h
	    デモ用共通プロックヘッダファイル      

	2001/07/31 H.Yoshiike         
	$Id: demo_stdproc.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $                      

	
*/


// シネマスクリーン
// ------------------------------------
proc デモ開始時シネマスクリーンセット {
	#if d:DEBUG_PRINT
		print 'cinema_screen_set'
	#endif

	chara シネマスクリーン シネマ -top d:CINEMA_BELT_UPPER -bottom d:CINEMA_BELT_LOWER -ftime d:C_FADE_TIME
	mesg シネマスクリーン シネマ フェードアウト 1
}

// シネマスクリーン オフ
// ------------------------------------
proc デモ終了時シネマスクリーンオフ {
	#if d:DEBUG_PRINT
		print 'cinema_screen_off'
	#endif

	mesg シネマスクリーン シネマ フェードイン 1
}

// デモキャンセル
// ------------------------------------
proc ポリデモキャンセル {
	#if d:DEBUG_PRINT
		print 'polydemo_cancel'
	#endif

	command StreamStopAll
}


// ダミープロック(MAXでよびだすprocがないときに記述)
// ------------------------------------
proc ダミープロック {}


// デモステージからゲームステージをロードする際のフラグ初期化
// ------------------------------------
proc ゲームステージへ戻る {
	#if d:DEBUG_PRINT
		print 'game_stage_go!!'
	#endif

	// レーダーを殺すか生かすかフラグ
	eval( $f:global_polygon_demo = 0 );
	// デモ仕様封殺フラグ
	eval( $s:d_num = not_demo );
}

// デモステージへ入るときのフラグ立て
// ------------------------------------
proc デモではレーダー不要 {
	#if d:DEBUG_PRINT
		print 'radar_flag_on'
	#endif

	// レーダーを殺すか生かすかフラグ
	eval($f:global_polygon_demo = 1)
}

// ストーリーフラグを進める
// ------------------------------------
proc ストーリーフラグ進め先 $:ストーリーフラグ {
	#if d:DEBUG_PRINT
		print 'skip_story_flag'
	#endif

	eval ( $w:p_story = $:ストーリーフラグ );
}

// デモに、現在のアイテムを反映させる
// chara demo 直前に必ず実行してください。（園山）
// ------------------------------------

proc デモモデル差し替え処理 {
	if ( $w:コンフィグ設定 & d:CONFIG_STORY_TANKER ) {
		//タンカー編
		if ( $w:アイテム == d:アイテム:無限バンダナ ) {
			command ファイル交換処理 -k sna_bdn1 sna_mugen_bdn1
			command ファイル交換処理 -k sna_bdn2 sna_mugen_bdn2
		}
	} else {
		//プラント編
		if ( $w:アイテム == d:アイテム:ゴル兵制服 ) {
			if ( $w:p_story >= d:ST:P036_13_R04忍者登場１３無線デモ４終了 && \
				 $w:p_story <= d:ST:P040_07_P04エイムズ発見７ポリゴンデモ４終了 ) {
				//キャップあり
				command デモモデルリネーム追加 \
					-rename_evm rai_def_mh_mt rai_gbs_gbshead_mh_mt \
						        rai_def_addhand_mh_mt rai_gbshead_addhand_mh_mt
			} else if ( $w:p_story >= d:ST:P040_09_P05エイムズ発見９ポリゴンデモ５終了 ) {
				//キャップなし
				command デモモデルリネーム追加 \
					-rename_evm rai_def_mh_mt rai_gbs_raihead_mh_mt \
						        rai_def_addhand_mh_mt rai_gbs_addhand_mh_mt
			}
		} else if ( $w:アイテム == d:アイテム:無限カツラ ) {
			command ファイル交換処理 -e rai_hair_mh_mt rai_zura_mugen_mh_mt
		} else if ( $w:アイテム == d:アイテム:カツラＡ ) {
			//青カツラ
			command ファイル交換処理 -e rai_hair_mh_mt rai_zura_mugen_mh_mt
			chara カツラテクスチャ差し替え管理 カツラテクスチャ差し替え -item d:アイテム:カツラＡ
		} else if ( $w:アイテム == d:アイテム:カツラＢ ) {
			//赤カツラ
			command ファイル交換処理 -e rai_hair_mh_mt rai_zura_mugen_mh_mt
			chara カツラテクスチャ差し替え管理 カツラテクスチャ差し替え -item d:アイテム:カツラＢ
		}
		if ( $w:p_story >= d:ST:P072_01_R01大佐混乱１無線機デモ１開始 ) {
			//髭剃りスネーク
			@髭剃りスネークテクスチャ入れ替え処理
		}
	}
}



