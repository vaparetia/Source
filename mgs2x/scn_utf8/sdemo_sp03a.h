/*
	sdemo_sp03a.h
	ザコサバイバルで使用するシナリオデモ

	2002/08/06 H.Yoshiike
	$Id: sdemo_sp03a.h,v 1.27 2002/10/02 06:30:29 usr03682 Exp $


*/

// 各デモのシーン番号
// ゴルルゴン登場デモ用
enum GUR_DEMO {
	地震発生_弱 = 1,
	地震発生_中,
	地震発生_強,
	ゴルルゴン登場,
	ゴルルゴン見上げるカット,
	ゴルルゴン背後カット,
	ゴルルゴン雄叫びカット,
	ライデンびっくり,
	ライデンびっくりカット,
	ステージフェードアウト
}

// メカゲノラ登場デモ用
enum MECH_DEMO {
	フォグ発生 = 1,
	振動開始,
	プレイヤー移動,
	左右きょろきょろ,
	稲光01,
	空見る,
	メカゲノラ生成,
	稲光11,
	メカゲノラ生成カット,
	稲光21,
	稲光22,
	稲光23,
	稲光24,
	メカゲノラ引きカット,
	稲光31,
	稲光32,
	メカゲノラ落下,
	メカゲノラ落下中カット,
	メカゲノラ着地カット,
	メカゲノラ着地戻りカット,
	メカゲノラ雄叫びカット右下,
	メカゲノラ雄叫びカット左下,
	メカゲノラ雄叫びカット左上,
	メカゲノラ雄叫びカット上,
	メカゲノラ雄叫びカット右上,
	メカゲノラ雄叫びカット右下２,
	メカゲノラ雄叫びカット中央,
	スネーク背後カット,
	ステージフェードアウト
}

// ゲノラ登場デモ用
enum GENO_DEMO {
	地震発生_弱 = 1,
	地震発生_中,
	地震発生_強,
	外壁崩れる,
	箱一列目壊れる,
	箱二列目壊れる,
	箱三列目壊れる１,
	箱三列目壊れる２,
	箱四列目壊れる,
	箱五列目壊れる,
	床崩れる,
	ゲノラ登場,
	ゲノラ登場カメラ,
	ゲノラ雄叫びカメラ正面,
	ゲノラ雄叫びカメラ左下,
	ゲノラ雄叫びカメラ左上,
	ゲノラ雄叫びカメラ上,
	ゲノラ雄叫びカメラ右上,
	ゲノラ雄叫びカメラ右下,
	ゲノラ雄叫びカメラ中央,
	カメラ引く,
	ステージフェードアウト
}


// ライデン変装時デモ用
enum RAI_DEMO {
	ライデン発見カット = 1,
	ゴルルゴンドキドキ振動開始,
	ゴルルゴン背後カメラ,
	ゴルルゴンチェンジ,
	ゴルルゴンかがむ,
	ゴルルゴンちょっと起き上がる,
	ゴルルゴン再びかがむ１,
	ゴルルゴン再びかがむ２,
	ライデン捕まる,
	ライデン振動開始,
	ゴルルゴン起き上がってくる１,
	ゴルルゴン起き上がってくる２,
	ゴルルゴンにっこり,
	ゴルルゴン立ち去る,
	カメラ引く,
	ストリームフェードアウト,
	ゲームオーバー
}

// ゴルルゴン雑誌発見時
enum GUR_BOOK_DEMO {
	雑誌発見カット = 1,
	ステージモデルチェンジ,
	ゴルルゴン背後カメラ,
	ゴルルゴン見上げカメラ
}

// メカゲノラ、チャフ放り込まれたとき
enum MECH_CHAFF_DEMO {
	チャフ食らったカット = 1,
	メカゲノラ背後カメラ,
	メカゲノラ見上げカメラ
}


// ゲノラたらい落ち
enum GENO_TARAI_DEMO {
	たらい落下カメラ = 1
}


// ゲノラvsゴルルゴン
enum GENO_VS_GUR_DEMO {
	地震_弱 = 1,
	地震_中,
	地震_強,
	ゴルルゴン登場,
	ゴルルゴンアップ,
	ゴルルゴンきょろきょろ１,
	ゴルルゴンきょろきょろ２,
	ゴルルゴンがゲノラ見つける,
	ゴルルゴン雄叫び,
	ゴルルゴン雄叫び振動開始,
	ゴルルゴンジャンプ,
	ゴルルゴンジャンプ振動開始,
	ゴルルゴンジャンプ前カット,
	ゴルルゴンジャンプカット１,
	ゴルルゴンジャンプカット２,
	ゴルルゴンジャンプカット３,
	ゴルルゴン飛び立つ,
	ゴルルゴン着地開始,
	ゴルルゴン着地振動開始,
	ゴルルゴン着地カメラ,
	ゴルルゴン着地,
	ゴルルゴン着地戻り,
	二人そろって雄叫び,
	雄叫び振動開始,
	戦闘開始,
	戦闘振動開始,
	ゲノラパンチ,
	ゲノラパンチヒット,
	ゴルルゴンタックル,
	ゴルルゴンタックルヒット,
	元のカメラ,
	組み合う,
	二匹が落下,
	カメラ前に水しぶき,
	ゲームクリア
}

// ゲノラvsメカゲノラ
enum GENO_VS_MECH_DEMO {
	暗くなる = 1,
	稲光01,
	稲光02,
	空を見る,
	メカゲノラ登場,
	稲光11,
	メカゲノラ登場カメラ,
	稲光21,
	稲光22,
	稲光23,
	稲光24,
	ゲノラ捕捉１,
	ゲノラ捕捉２,
	メカゲノラ落下,
	メカゲノラ落下カメラ,
	メカゲノラ着地,
	メカゲノラ着地戻り,
	二人そろって雄叫び,
	雄叫び振動開始,
	戦闘開始,
	戦闘振動開始,
	ゲノラパンチ,
	ゲノラパンチヒット,
	メカゲノラタックル,
	メカゲノラタックルヒット,
	元のカメラ,
	組み合う,
	二匹が落下,
	カメラ前に水しぶき,
	ゲームクリア
}


// 登場デモで使用する共通プロック
proc プレイヤーびっくりモーション発動 {
	mesg プレイヤー d:PLAYER motion \
		d:モーションリスト:rai_non_odoroki 2048 2048 d:FA_END_STAND 0

	command	強制モーション終了プロック -proc プレイヤーびっくりモーションループ
}

proc プレイヤーびっくりモーションループ {
	mesg プレイヤー d:PLAYER motion \
		d:モーションリスト:rai_non_odoroki_idle 2048 2048 d:FA_END_STAND 0

	command 強制モーションループ回数設定 -loop 100
}

proc 巨大生物テロップ表示 $:p_ファイル名 $:p_フェードフレーム $:p_表示フレーム {
	chara ２Ｄスプライト表示 説明文 \
		-tri sp03a -x $:p_ファイル名 \
		-pos (512/2) (384-(d:CINEMA_BELT_LOWER/2)-24-5) \
		-count $:p_フェードフレーム,$:p_表示フレーム,$:p_フェードフレーム \
		-alp 128 \
		-flag ( d:SPRT2D_FLAG_CENTER | d:SPRT2D_FLAG_BOTTOM | d:SPRT2D_FLAG_ADD )

}

proc スネーク張り付きっぱなし_ゴルルゴン用 {
	chara delay シナリオデモディレイ -time 60 \
		-exec {
			mesg パッドデモ 張り付きパッドデモ１ start
		}
}

proc スネーク張り付きっぱなし_メカゲノラ用 {
	chara delay シナリオデモディレイ -time 110 \
		-exec {
			mesg パッドデモ 張り付きパッドデモ２ start
		}
}

proc ストリーミングフェードアウト $:p_ストリームＩＤ {
	$$b:ストリームボリューム = 0x3F

	command StreamSetPan $:p_ストリームＩＤ -p 0x20 -v $$b:ストリームボリューム

	chara delay ストリーミングフェードアウトディレイ -time 1 -arg $:p_ストリームＩＤ \
		-exec {
			$$b:ストリームボリューム = $$b:ストリームボリューム - 1;
			if ( $$b:ストリームボリューム > 0 ) {
				#if d:DEBUG_PRINT 
					print '$$b:ストリームボリューム='$$b:ストリームボリューム
				#endif
				command StreamSetPan $1 -p 0x20 -v $$b:ストリームボリューム
				return 1;
			} else {
				#if d:DEBUG_PRINT 
					print 'stream stopです!!!!!'
				#endif
				// 64フレーム目で停止する
				command StreamStop $1
				return 0;
			}
		}
}

proc プレイヤーアイテム追加解除 {
	if ( $w:アイテム == d:アイテム:暗視ゴーグル || $w:アイテム == d:アイテム:サーマルゴーグル ) {
		eval($w:アイテム = d:アイテム:素手);
	}
}


// 各シナリオデモ
proc ゴルルゴン登場デモ {
	#if d:DEBUG_PRINT 
		print 'gll_demo_start!!!!!'
	#endif

	#if d:DEMO_DEBUG
		command パッド操作 -cancel
		command メニュー設定 -pause on
	#endif

	// 変数を初期化
	$$b:シナリオデモシーン番号 = 0;

	// 登場音のストリームを読み込み、スタート開始待ち状態にする
	$$i:シナリオデモストリーム返り値 = `command Stream t:vc032011 -w`;

	// アイテムボックスは消去
	mesg アイテム Ｍ９本体 kill
	mesg アイテム ソコム本体 kill
	mesg アイテム ＡＫＳ本体 kill
	mesg アイテム ＰＳＧ１－Ｔ本体 kill
	mesg アイテム ＰＳＧ１本体 kill
	mesg アイテム ＲＧＢ６本体 kill
	mesg アイテム Ｃ４ kill
	mesg アイテム クレイモア kill
	mesg アイテム レーション kill
	mesg アイテム ジアゼパム kill
	mesg アイテム Ｍ９弾 kill
	mesg アイテム ソコム弾 kill

	chara delay シナリオデモディレイ -time d:C_FADE_TIME \
		-exec {
			$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

			switch ( $$b:シナリオデモシーン番号 ) {
				case ( d:GUR_DEMO:地震発生_弱 ) {
					// ストリーミングがちゃんと読み込まれているかチェック
					command StreamStatus $$i:シナリオデモストリーム返り値
					if ( $status == d:STREAM:WAIT ) {
						#if d:DEBUG_PRINT 
							print 'StreamStatus start! $status='$status
						#endif

						command StreamStart $$i:シナリオデモストリーム返り値

						chara シナリオカメラ振動 地震:01 -xlevel 10 -ylevel 10 -time 180 -mode 0

						chara パッド振動スクリプト 地震振動:01 -mode 2 \
							-data 60 60 50 60 90 50 150 10

						// 画面暗くなる
						chara フォグ 霧 -c 10, 15, 17 -n -500 -f 48000 -t 360

						mesg ＶＲ空 ＶＲ空 ベース用環境光変化 30 40 35 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 0 28 32 30 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 1 33 53 67 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 3 37 67 85 300
						mesg ＶＲ空 ＶＲ空 モデル用環境光変化 0 17 26 25 300
						mesg ＶＲ空 ＶＲ空 モデル用環境光変化 1 10 45 45 300

						mesg 太陽 ソル 非表示

						return 180;

					} else {
						#if d:DEBUG_PRINT 
							print 'StreamStatus wait待ち $status='$status
						#endif

						// スタート準備が完了していない場合は再度チェック
						$$b:シナリオデモシーン番号 = 0;
						return 1;
					}

				}
				case ( d:GUR_DEMO:地震発生_中 ) {
					mesg シナリオカメラ振動 地震:01 kill
					chara シナリオカメラ振動 地震:02 -xlevel 100 -ylevel 100 -time 180 -mode 0

					chara パッド振動スクリプト 地震振動:02 -mode 1 \
						-data 127 63 0 63 127 54
					chara パッド振動スクリプト 地震振動:02 -mode 2 \
						-data 200 63 80 63 220 54

					return 180;
				}
				case ( d:GUR_DEMO:地震発生_強 ) {
					mesg シナリオカメラ振動 地震:02 kill
					chara シナリオカメラ振動 地震:03 -xlevel 150 -ylevel 150 -time 60 -mode 0

					chara パッド振動スクリプト 地震振動:03 -mode 1 \
						-data 127 50
					chara パッド振動スクリプト 地震振動:03 -mode 2 \
						-data 160 50 80 10

					return 60;
				}
				case ( d:GUR_DEMO:ゴルルゴン登場 ) {
					mesg シナリオカメラ振動 地震:03 kill
					chara シナリオカメラ振動 地震:04 -xlevel 200 -ylevel 200 -time 180 -mode 1
					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 190
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 255 60 100 60 230 30 80 60 60 60 40 30 20 14

					mesg プットモーションモデル デモゴルルゴン 水平移動量無効
					mesg プットモーションモデル デモゴルルゴン 表示
					mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_idle
					mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble
					mesg ＶＲオブジェ ゴルルゴンを隠す壁の影 crumble
					mesg ＶＲオブジェ 外壁 on
					mesg ＶＲオブジェ 外壁の影 on
					mesg ＶＲオブジェ 箱03への影 on
					mesg ＶＲオブジェ 箱09への影 on
					mesg ＶＲオブジェ 箱 off
					mesg ＶＲオブジェ デモ箱 on
					mesg ＶＲオブジェ 箱影 off
					mesg ＶＲオブジェ デモ箱影 on

					mesg ステージアウトライン ステージ全体ライン 非表示
					mesg ステージアウトライン ゴルルゴンステージライン 表示

					mesg フェードオブジェ ステージトンボ:02 フェードアウト

					return 90;
				}
				case ( d:GUR_DEMO:ゴルルゴン見上げるカット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -2682,641,-33254 -t -2678,644,-33354 \
						-r 4078,2023,0 -f 100 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -2682,641,-33254 -t -2680,729,-33301 \
						-r 3390,2023,0 -f 100 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 120\ // 補完時間を指定
						-s 1
	
					return 120;
				}
				case ( d:GUR_DEMO:ゴルルゴン背後カット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p 3081,18798,-44151 -t 1615,9991,-28277 \
						-r 329,4036,0 -f 18213 \
						-a 318 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1798,18722,-44264 -t 280,9991,-28421 \
						-r 326,85,0 -f 18209 \
						-a 318 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 300 \ // 補完時間を指定
						-s 1
	
					return 300;
				}
				case ( d:GUR_DEMO:ゴルルゴン雄叫びカット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -10150,43,-26591 -t 1754,9935,-36544 \
						-r 3726,1478,0 -f 18402 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p 3800,43,-21871 -t -840,10500,-36284 \
						-r 3702,2251,0 -f 18401 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 300 \ // 補完時間を指定
						-s 1
	
					mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_appear_wall
					mesg プレイヤー d:PLAYER position 550 1000 -3250
					mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0

					switch ( $w:言語 ) {
						case ( d:LANG_JAPANESE ) {
							@巨大生物テロップ表示 gur_alp_ovl 30 450
						}
						default {
							@巨大生物テロップ表示 gur_e_alp_ovl 30 450
						}
					}

					return 300;
				}
				case ( d:GUR_DEMO:ライデンびっくり ) {
					@プレイヤーびっくりモーション発動

					return 2;
				}
				case ( d:GUR_DEMO:ライデンびっくりカット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p 1553,5,550 -t 1205,658,-2068 \
						-r 3938,2134,0 -f 2721 \
						-a 318 \
						-i 0 -1 0 0 \
						-s 1

					return 180;
				}
				case ( d:GUR_DEMO:ステージフェードアウト ) {
					chara フェードインアウト フェードアウト \
						-c 0,0,0 128 \
						-t d:FADEOUT_TIME \
						-p ゴルルゴンステージスタート
				}

			}
		}
}

proc メカゲノラ登場デモ {
	#if d:DEMO_DEBUG
		command パッド操作 -cancel
		command メニュー設定 -pause on
	#endif

	// 変数を初期化
	$$b:シナリオデモシーン番号 = 0;

	// 登場音のストリームを読み込み、スタート開始待ち状態にする
	$$i:シナリオデモストリーム返り値 = `command Stream t:vc032012 -w`;

	// アイテムボックスは消去
	mesg アイテム Ｍ９本体 kill
	mesg アイテム ＵＳＰ本体 kill
	mesg アイテム Ｍ４本体 kill
	mesg アイテム ＰＳＧ１－Ｔ本体 kill
	mesg アイテム ＰＳＧ１本体 kill
	mesg アイテム ＲＧＢ６本体 kill
	mesg アイテム Ｃ４ kill
	mesg アイテム クレイモア kill
	mesg アイテム レーション kill
	mesg アイテム ジアゼパム kill
	mesg アイテム Ｍ９弾 kill
	mesg アイテム ＵＳＰ弾 kill

	chara delay シナリオデモディレイ -time 15 \
		-exec {
			$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

			switch ( $$b:シナリオデモシーン番号 ) {
				case ( d:MECH_DEMO:フォグ発生 ) {
					// ストリーミングがちゃんと読み込まれているかチェック
					command StreamStatus $$i:シナリオデモストリーム返り値
					if ( $status == d:STREAM:WAIT ) {
						#if d:DEBUG_PRINT 
							print 'StreamStatus start! $status='$status
						#endif

						command StreamStart $$i:シナリオデモストリーム返り値

						// 画面暗くなる
						chara フォグ 霧 -c 10, 15, 17 -n -500 -f 48000 -t 300

						mesg ＶＲ空 ＶＲ空 ベース用環境光変化 30 40 35 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 0 28 32 30 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 1 33 53 67 300
						mesg ＶＲ空 ＶＲ空 雲用環境光変化 3 37 67 85 300
						mesg ＶＲ空 ＶＲ空 モデル用環境光変化 0 17 26 25 300
						mesg ＶＲ空 ＶＲ空 モデル用環境光変化 1 10 45 45 300

						mesg 太陽 ソル 非表示

						return 14;

					} else {
						#if d:DEBUG_PRINT 
							print 'StreamStatus wait待ち $status='$status
						#endif

						// スタート準備が完了していない場合は再度チェック
						$$b:シナリオデモシーン番号 = 0;
						return 1;
					}
				}
				case ( d:MECH_DEMO:振動開始 ) {
					chara パッド振動 登場振動 -vibfile mgn_appear_sdemo
					return 286;
				}
				case ( d:MECH_DEMO:プレイヤー移動 ) {
					mesg プレイヤー d:PLAYER position 0 1000 2000

					return 2;
				}
				case ( d:MECH_DEMO:左右きょろきょろ ) {
					// 左右を見る
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p 476,1771,-4532 -t 315,1970,-5063 \
						-r 3872,2239,0 -f 590 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p 476,1771,-4531 -t 770,1939,-5014 \
						-r 3908,1691,0 -f 590 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 120\ // 補完時間を指定
						-s 1

					chara 任意稲光 稲妻君 \
						-x 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v -3600   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v -3500   // -4096(UPPER) ～ 4096(LOWER)

					return 64;
				}
				case ( d:MECH_DEMO:稲光01 ) {
					chara 任意稲光 稲妻君 \
						-x 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v -3500   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v -3500   // -4096(UPPER) ～ 4096(LOWER)

					return 56;
				}
				case ( d:MECH_DEMO:空見る ) {
					// 空を見る
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p 476,1771,-4531 -t 475,2302,-4789 \
						-r 3366,2051,0 -f 590 \
						-a 465 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 40\ // 補完時間を指定
						-s 1

					return 40;
				}
				case ( d:MECH_DEMO:メカゲノラ生成 ) {
					// メカゲノラ生成開始
					mesg オブジェクト分解 オブジェクト分解君 変形開始
					mesg プットモーションモデル デモメカゲノラ 水平移動量無効

					mesg シナリオカメラ振動 地震振動:01 kill

					chara 任意稲光 稲妻君 \
						-x (256/5) \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y (-3584/5) \ // -4096(UPPER) ～ 4096(LOWER)
						-u (4096/5) \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v (1536/5)   // -4096(UPPER) ～ 4096(LOWER)

					return 18;
				}
				case ( d:MECH_DEMO:稲光11 ) {
					chara 任意稲光 稲妻君 \
						-x (-256/5) \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y (-3584/5) \ // -4096(UPPER) ～ 4096(LOWER)
						-u (-3072/5) \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v (3072/5)   // -4096(UPPER) ～ 4096(LOWER)
					
					return 42;
				}
				case ( d:MECH_DEMO:メカゲノラ生成カット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -2832,31677,-12910 -t -2798,31672,-13001 \
						-r 38,1816,0 -f 97 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -2832,31676,-12909 -t -2812,31754,-12964 \
						-r 3488,1816,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 130 \ // 補完時間を指定
						-s 1

					mesg シナリオカメラ振動 地震振動:02 kill

					mesg プレイヤー d:PLAYER position -500 1000 -6500
					mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0

					return 56;
				}
				case ( d:MECH_DEMO:稲光21 ) {
					chara 任意稲光 稲妻君 \
						-x -2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -3584 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 2048   // -4096(UPPER) ～ 4096(LOWER)
					
					return 30;
				}
				case ( d:MECH_DEMO:稲光22 ) {
					chara 任意稲光 稲妻君 \
						-x 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 1024   // -4096(UPPER) ～ 4096(LOWER)
					
					return 60;
				}
				case ( d:MECH_DEMO:稲光23 ) {
					chara 任意稲光 稲妻君 \
						-x -512 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 1024 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 4096   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x 1536 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 3584 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 4096   // -4096(UPPER) ～ 4096(LOWER)
					
					return 10;
				}
				case ( d:MECH_DEMO:稲光24 ) {
					chara 任意稲光 稲妻君 \
						-x -1024 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4000 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 4096   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 256   // -4096(UPPER) ～ 4096(LOWER)
					
					return 34;
				}
				case ( d:MECH_DEMO:メカゲノラ引きカット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -1872,210,-3866 -t -1416,3455,-5307 \
						-r 3356,1848,0 -f 3580 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1872,210,-3866 -t -1564,3455,-5345 \
						-r 3356,1914,0 -f 3580 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 120 \ // 補完時間を指定
						-s 1

					mesg シナリオカメラ振動 地震振動:03 kill

					return 30;
				}
				case ( d:MECH_DEMO:稲光31 ) {
					chara 任意稲光 稲妻君 \
						-x -256 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 3900 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 1024   // -4096(UPPER) ～ 4096(LOWER)
					
					return 60;
				}
				case ( d:MECH_DEMO:稲光32 ) {
					chara 任意稲光 稲妻君 \
						-x -512 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -2048 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 2048   // -4096(UPPER) ～ 4096(LOWER)
					
					return 60;
				}
				case ( d:MECH_DEMO:メカゲノラ落下 ) {
					// メカゲノラ落下スタート
					mesg プットモーションモデル デモメカゲノラ モーション再生 d:G_MTN:gno_meca_toujyou
					mesg プットモーションモデル デモメカゲノラ 基本モーション変更 d:G_MTN:gol_idle

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1872,210,-3866 -t -1459,3162,-5849 \
						-r 3464,1914,0 -f 3580 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 25 \ // 補完時間を指定
						-s 1
	
					return 25;
				}
				case ( d:MECH_DEMO:メカゲノラ落下中カット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1872,210,-3866 -t -1198,1590,-7100 \
						-r 3838,1914,0 -f 3580 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 80 \ // 補完時間を指定
						-s 1
	
					return 45;
				}
				case ( d:MECH_DEMO:メカゲノラ着地カット ) {
					// メカゲノラ着地の振動
					chara シナリオカメラ振動 地震:01 -xlevel 200 -ylevel 200 -time 180 -mode 1
					@プレイヤーびっくりモーション発動

					mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble
					mesg ＶＲオブジェ ゴルルゴンを隠す壁の影 crumble
					mesg ＶＲオブジェ 外壁 on
					mesg ＶＲオブジェ 外壁の影 on
					mesg ＶＲオブジェ 箱03への影 on
					mesg ＶＲオブジェ 箱09への影 on

					mesg ステージアウトライン ステージ全体ライン 非表示
					mesg ステージアウトライン メカゲノラステージライン 表示

					mesg フェードオブジェ ステージトンボ:02 フェードアウト

					return 40;
				}
				case ( d:MECH_DEMO:メカゲノラ着地戻りカット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1872,210,-3866 -t -1238,1989,-6908 \
						-r 3757,1914,0 -f 3580 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 70 \ // 補完時間を指定
						-s 1

					return 100;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット右下 ) {
					// メカゲノラ雄叫び
					mesg プットモーションモデル デモメカゲノラ モーション再生 d:G_MTN:gol_howl
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -1732,902,-9768 -t -1717,970,-9835 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,902,-9768 -t -1704,970,-9832 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 25 \ // 補完時間を指定
						-s 1

					return 25;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット左下 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,902,-9767 -t -1737,971,-9838 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 60 \ // 補完時間を指定
						-s 1

					return 60;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット左上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,902,-9767 -t -1742,973,-9836 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 65 \ // 補完時間を指定
						-s 1

					switch ( $w:言語 ) {
						case ( d:LANG_JAPANESE ) {
							@巨大生物テロップ表示 mecgeno1_alp_ovl 30 (543-60-85)
						}
						default {
							@巨大生物テロップ表示 mecgeno1_e_alp_ovl 30 (543-60-85)
						}
					}

					chara 任意稲光 稲妻君 \
						-x -512 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -3072 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 2048   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x -256 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 3072   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 3586 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 3586   // -4096(UPPER) ～ 4096(LOWER)

					return 65;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,902,-9766 -t -1719,977,-9826 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 101 \ // 補完時間を指定
						-s 1

					chara 任意稲光 稲妻君 \
						-x -1024 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 3072 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 2048   // -4096(UPPER) ～ 4096(LOWER)

					chara 任意稲光 稲妻君 \
						-x 1024 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
						-u 0 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 3072   // -4096(UPPER) ～ 4096(LOWER)

					return 101;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット右上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,901,-9765 -t -1703,974,-9825 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 96 \ // 補完時間を指定
						-s 1

					chara 任意稲光 稲妻君 \
						-x 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
						-u -2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
						-v 3072   // -4096(UPPER) ～ 4096(LOWER)

					return 96;
				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット右下２ ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,901,-9764 -t -1698,969,-9828 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 90 \ // 補完時間を指定
						-s 1

					return 90;

				}
				case ( d:MECH_DEMO:メカゲノラ雄叫びカット中央 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -1732,901,-9764 -t -1717,970,-9832 \
						-r -509,1906,0 -f 97 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 40 \ // 補完時間を指定
						-s 1

					return 110;

				}
				case ( d:MECH_DEMO:スネーク背後カット ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p 910,-9,-4343 -t -25,1837,-7115 \
						-r -367,2260,0 -f 3459 \
						-a 197 \
						-i 0 -1 0 0 \
						-s 1

					return 120;
				}
				case ( d:MECH_DEMO:ステージフェードアウト ) {
					chara フェードインアウト フェードアウト \
						-c 0,0,0 128 \
						-t d:FADEOUT_TIME \
						-p メカゲノラステージスタート
				}

			}
		}
}

proc ゲノラ登場デモ {
	#if d:DEMO_DEBUG
		command パッド操作 -cancel
		command メニュー設定 -pause on
	#endif

	// 変数を初期化
	$$b:シナリオデモシーン番号 = 0;

	// 登場音のストリームを読み込み、スタート開始待ち状態にする
	$$i:シナリオデモストリーム返り値 = `command Stream t:vc032013 -w`;

	// アイテムボックスは消去
	mesg アイテム Ｍ９本体 kill
	mesg アイテム ＵＳＰ本体 kill
	mesg アイテム Ｍ４本体 kill
	mesg アイテム ＰＳＧ１－Ｔ本体 kill
	mesg アイテム ＰＳＧ１本体 kill
	mesg アイテム ＲＧＢ６本体 kill
	mesg アイテム Ｃ４ kill
	mesg アイテム クレイモア kill
	mesg アイテム Ｍ９弾 kill
	mesg アイテム ＵＳＰ弾 kill

	chara delay シナリオデモディレイ -time 15 \
		-exec {
			$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

			switch ( $$b:シナリオデモシーン番号 ) {
				case ( d:GENO_DEMO:地震発生_弱 ) {
					// ストリーミングがちゃんと読み込まれているかチェック
					command StreamStatus $$i:シナリオデモストリーム返り値
					if ( $status == d:STREAM:WAIT ) {
						#if d:DEBUG_PRINT 
							print 'StreamStatus start! $status='$status
						#endif

						command StreamStart $$i:シナリオデモストリーム返り値

						chara シナリオカメラ振動 地震:01 -xlevel 10 -ylevel 10 -time 180 -mode 0

						chara パッド振動スクリプト 地震振動:01 -mode 2 \
							-data 60 60 50 60 90 50 150 10

						return 180;

					} else {
						#if d:DEBUG_PRINT 
							print 'StreamStatus wait待ち $status='$status
						#endif

						// スタート準備が完了していない場合は再度チェック
						$$b:シナリオデモシーン番号 = 0;
						return 1;
					}

				}
				case ( d:GENO_DEMO:地震発生_中 ) {
					mesg シナリオカメラ振動 地震:01 kill
					chara シナリオカメラ振動 地震:02 -xlevel 100 -ylevel 100 -time 180 -mode 0
					chara パッド振動スクリプト 地震振動:02 -mode 1 \
						-data 127 180
					chara パッド振動スクリプト 地震振動:02 -mode 2 \
						-data 200 63 120 63 220 54

					return 180;
				}
				case ( d:GENO_DEMO:地震発生_強 ) {
					mesg シナリオカメラ振動 地震:02 kill
					chara シナリオカメラ振動 地震:03 -xlevel 150 -ylevel 150 -time 60 -mode 0
					chara パッド振動スクリプト 地震振動:03 -mode 1 \
						-data 127 50
					chara パッド振動スクリプト 地震振動:03 -mode 2 \
						-data 160 50

					return 60;
				}
				case ( d:GENO_DEMO:外壁崩れる ) {
					// 外壁が崩れる
					mesg シナリオカメラ振動 地震:03 kill
					chara シナリオカメラ振動 地震:04 -xlevel 200 -ylevel 200 -time 540 -mode 1
					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 90
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 255 60 100 20 50 10 20 10 10 10 5 10

					mesg プットモーションモデル デモゲノラ 水平移動量無効
					mesg プットモーションモデル デモゲノラ 表示
					// プレイヤー瞬間移動
					mesg プレイヤー d:PLAYER position -1500 1000 -2000
					mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0

					mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble
					mesg ＶＲオブジェ ゴルルゴンを隠す壁の影 crumble
					mesg ＶＲオブジェ 外壁 on
					mesg ＶＲオブジェ 外壁の影 on

					mesg ステージアウトライン ステージ全体ライン 非表示
					mesg ステージアウトライン ゴルルゴンステージライン 表示

					mesg フェードオブジェ ステージトンボ:02 フェードアウト


					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p 4951,4063,-3015 -t 3426,4636,-9141 \
						-r 4037,2207,0 -f 6338 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					return 120;
				}
				case ( d:GENO_DEMO:箱一列目壊れる ) {
					mesg シナリオカメラ振動 地震:04 kill
					chara シナリオカメラ振動 地震:05 -xlevel 200 -ylevel 200 -time 102 -mode 0
					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 1
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 2

					// プレイヤー移動開始
					mesg プレイヤー d:PLAYER run \
						-1500 0 -12500 \
						1 \ // 走り
						2048 \
						d:FA_DIRECT_MOVE

					command	強制モーション終了プロック -proc プレイヤーびっくりモーション発動

					mesg ＶＲオブジェ 箱:05 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 10
					mesg ステージアウトライン 箱ライン:05 非表示
					mesg フェードオブジェ 箱トンボ:05 フェードアウト

					mesg ＶＲオブジェ 箱:06 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 11
					mesg ステージアウトライン 箱ライン:06 非表示
					mesg フェードオブジェ 箱トンボ:06 フェードアウト

					mesg ＶＲオブジェ 箱:12 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 17
					mesg ＶＲオブジェ 箱影:12 crumble
					mesg ステージアウトライン 箱ライン:12 非表示
					mesg フェードオブジェ 箱トンボ:12 フェードアウト

					return 2;
				}
				case ( d:GENO_DEMO:箱二列目壊れる ) {
					mesg ＶＲオブジェ 箱:07 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 12
					mesg ステージアウトライン 箱ライン:07 非表示
					mesg フェードオブジェ 箱トンボ:07 フェードアウト

					mesg ＶＲオブジェ 箱:08 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 13
					mesg ステージアウトライン 箱ライン:08 非表示
					mesg フェードオブジェ 箱トンボ:08 フェードアウト

					chara カメラ設定 クリア時カメラ \
						-c 1 \	
						-p -1500,4019,11874 -t -1500,1414,-2000 \
						-r 121,2048,0 -f 14116 \
						-a 200 \
						-i 0 -1 0 0 \
						-s 1

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3541,689,-6931 -t -2424,689,-13200 \
						-r 0,1933,0 -f 6368 \
						-a 275 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 150 \ // 補完時間を指定
						-s 1

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 5
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 10

					return 10;
				}
				case ( d:GENO_DEMO:箱三列目壊れる１ ) {
					mesg ＶＲオブジェ 箱:04 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 9
					mesg ステージアウトライン 箱ライン:04 非表示

					mesg ＶＲオブジェ 箱:11 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 16
					mesg ＶＲオブジェ 箱影:11 crumble
					mesg ステージアウトライン 箱ライン:11 非表示
					mesg フェードオブジェ 箱トンボ:11 フェードアウト

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 5
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 10

					return 10;
				}
				case ( d:GENO_DEMO:箱三列目壊れる２ ) {
					mesg ＶＲオブジェ 箱:10 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 15
					mesg ステージアウトライン 箱ライン:10 非表示
					mesg フェードオブジェ 箱トンボ:10 フェードアウト

					mesg ＶＲオブジェ 箱:03 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 8
					mesg ステージアウトライン 箱ライン:03 非表示
					mesg フェードオブジェ 箱トンボ:03 フェードアウト

					mesg ＶＲオブジェ 箱:09 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 14
					mesg ＶＲオブジェ 箱09への影 crumble
					mesg ＶＲオブジェ 箱影:09 crumble
					mesg ステージアウトライン 箱ライン:09 非表示
					mesg フェードオブジェ 箱トンボ:09 フェードアウト

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 5
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 10

					return 10;
				}
				case ( d:GENO_DEMO:箱四列目壊れる ) {
					mesg ＶＲオブジェ 箱:02 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 7
					mesg ステージアウトライン 箱ライン:02 非表示
					mesg フェードオブジェ 箱トンボ:02 フェードアウト

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 5
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 10

					return 10;
				}
				case ( d:GENO_DEMO:箱五列目壊れる ) {
					mesg ＶＲオブジェ 箱:01 crumble
					command 追加ＨＺＸグループ削除 -hzx_id 0 -add_id 6
					mesg ステージアウトライン 箱ライン:01 非表示
					mesg フェードオブジェ 箱トンボ:01 フェードアウト
					mesg フェードオブジェ アウトライン箱:01 フェードアウト

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 40
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 125 40

					return 50;
				}
				case ( d:GENO_DEMO:床崩れる ) {
					mesg シナリオカメラ振動 地震:05 kill
					chara シナリオカメラ振動 地震:06 -xlevel 400 -ylevel 400 -time 120 -mode 1

					mesg ＶＲオブジェ ゲノラを隠す床 crumble
					mesg ＶＲオブジェ ゲノラ床 on

					mesg ステージアウトライン ゴルルゴンステージライン 非表示
					mesg ステージアウトライン ゲノラステージライン 表示
					mesg フェードオブジェ ステージトンボ:01 フェードアウト
					mesg フェードオブジェ アウトライン フェードアウト

					chara パッド振動スクリプト 地震振動:04 -mode 1 \
						-data 127 90
					chara パッド振動スクリプト 地震振動:04 -mode 2 \
						-data 255 90 150 15 100 15 60 10 40 10 30 10 20 2

					return 10;
				}
				case ( d:GENO_DEMO:ゲノラ登場 ) {
					mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gno_come_2
					mesg プットモーションモデル デモゲノラ 基本モーション変更 d:G_MTN:gol_idle
					return 80;
				}
				case ( d:GENO_DEMO:ゲノラ登場カメラ ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3390,-95,-7246 -t -2464,1825,-13237 \
						-r 3896,1948,0 -f 6359 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 80 \ // 補完時間を指定
						-s 1

					return 100;
				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ正面 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3277,355,-7344 -t -2292,1470,-13521 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 70 \ // 補完時間を指定
						-s 1

					return 133;
				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ左下 ) {
					// ゲノラ雄叫び
					mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gol_howl

					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7343 -t -3722,544,-13916 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 67 \ // 補完時間を指定
						-s 1

					switch ( $w:言語 ) {
						case ( d:LANG_JAPANESE ) {
							@巨大生物テロップ表示 geno_alp_ovl 30 450
						}
						default {
							@巨大生物テロップ表示 geno_e_alp_ovl 30 450
						}
					}

					return 67;
				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ左上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7343 -t -3963,1995,-13692 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 62 \ // 補完時間を指定
						-s 1

					return 62;

				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7343 -t -2322,2521,-13335 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 127 \ // 補完時間を指定
						-s 1

					return 127;

				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ右上 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7342 -t -1801,1845,-13374 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 96 \ // 補完時間を指定
						-s 1

					return 96;

				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ右下 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7342 -t -1513,1445,-13400 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 62 \ // 補完時間を指定
						-s 1

					return 62;

				}
				case ( d:GENO_DEMO:ゲノラ雄叫びカメラ中央 ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3276,355,-7342 -t -2292,1470,-13520 \
						-r 3981,1945,0 -f 6354 \
						-a 298 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 90 \ // 補完時間を指定
						-s 1

					return 90;

				}
				case ( d:GENO_DEMO:カメラ引く ) {
					chara カメラ設定 クリア時カメラ \
						-c 1 \
						-p -3064,-457,-4513 -t -2061,777,-11066 \
						-r -120,1949,0 -f 6744 \
						-a 200 \
						-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
						-interp_time 180 \ // 補完時間を指定
						-s 1

					return 120;

				}
				case ( d:GENO_DEMO:ステージフェードアウト ) {
					chara フェードインアウト フェードアウト \
						-c 0,0,0 128 \
						-t d:FADEOUT_TIME \
						-p ゲノラステージスタート
				}

			}
		}
}



//-----------------------------------------------------------
// 特殊ゲームオーバー
//-----------------------------------------------------------
proc ライデン変装時デモ発動 {
	#if d:DEBUG_PRINT 
		print 'raiden_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 ) {
		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		command ゲットゴルルゴン壊れパーツ $$i:壊れパーツフラグ
		chara ゴルルゴン部位壊れ ゴルルゴン部位壊れ君 \
			 -name デモゴルルゴン \ /*デモ人形などのシナリオ名*/
			 -bit $$i:壊れパーツフラグ

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
			print '$$i:壊れパーツフラグ'$$i:壊れパーツフラグ
		#endif

		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// 登場音のストリームを読み込み、スタート開始待ち状態にする
		$$i:シナリオデモストリーム返り値 = `command Stream t:vc032014 -w`;

		// アイテムボックスは消去
		mesg アイテム ＡＫＳ本体 kill
		mesg アイテム ＰＳＧ１本体 kill
		mesg アイテム ＲＧＢ６本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム レーション kill
		mesg アイテム 雑誌 kill
		mesg アイテム ＡＫＳ弾 kill
		mesg アイテム ＰＳＧ１弾 kill
		mesg アイテム ＲＧＢ６弾 kill
		mesg アイテム スティンガー弾 kill
		mesg アイテム ジアゼパム kill

		chara delay シナリオデモディレイ -time (d:C_FADE_TIME + 30) \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:RAI_DEMO:ライデン発見カット ) {
						// ストリーミングがちゃんと読み込まれているかチェック
						command StreamStatus $$i:シナリオデモストリーム返り値
						if ( $status == d:STREAM:WAIT ) {
							#if d:DEBUG_PRINT 
								print 'StreamStatus start! $status='$status
							#endif

							command StreamStart $$i:シナリオデモストリーム返り値

							command ゲットゴルルゴン頭座標 $$i:ゴルＸ $$i:ゴルＹ $$i:ゴルＺ

							chara カメラ設定 クリア時カメラ \
								-c 1 \	
								-p 0,2500,-6000 -t $$i:ゴルＸ ($$i:ゴルＹ+1000) $$i:ゴルＺ \
								-r 3564,2038,0 -f 7994 \
								-a 160 \
								-i 0 -1 0 0 \
								-s 1

							chara カメラ設定 クリア時カメラ \
								-c 1 \
								-p 0,2500,-6000 -t $$i:ゴルＸ ($$i:ゴルＹ+1000) $$i:ゴルＺ \
								-r 3564,2038,0 -f 7994 \
								-a 250 \
								-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
								-interp_time 25 \ // 補完時間を指定
								-s 1

								mesg ＶＲオブジェ 外壁の影 off
								mesg ＶＲオブジェ デモ用外壁の影 on
								mesg ＶＲオブジェ 箱03への影 off
								mesg ＶＲオブジェ デモ用箱03への影 on
								mesg ＶＲオブジェ 箱09への影 off
								mesg ＶＲオブジェ デモ用箱09への影 on
								mesg ＶＲオブジェ 箱 off
								mesg ＶＲオブジェ デモ箱 on
								mesg ＶＲオブジェ 箱影 off
								mesg ＶＲオブジェ デモ箱影 on

							return 110;

						} else {
							#if d:DEBUG_PRINT 
								print 'StreamStatus wait待ち $status='$status
							#endif

							// スタート準備が完了していない場合は再度チェック
							$$b:シナリオデモシーン番号 = 0;
							return 1;
						}

					}
					case ( d:RAI_DEMO:ゴルルゴンドキドキ振動開始 ) {
						chara パッド振動 ドキドキ振動 -vibfile gol_heartbeat_sdemo
						return 10;
					}
					case ( d:RAI_DEMO:ゴルルゴン背後カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 650,30277,-31091 -t 2509,5194,-11292 \
							-r 587,61,0 -f 32010 \
							-a 275 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -1547,30278,-30886 -t 311,5194,-11086 \
							-r 587,61,0 -f 32011 \
							-a 275 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 300 \ // 補完時間を指定
							-s 1

						return 420;
					}
					case ( d:RAI_DEMO:ゴルルゴンチェンジ ) {
						mesg ゴルルゴン ゴルルゴン ストップ
						mesg プットモーションモデル デモゴルルゴン 水平移動量無効
						mesg プットモーションモデル デモゴルルゴン 表示
						command セットサウンドコード -c d:INT_PAUSEON
						return 1;
					}
					case ( d:RAI_DEMO:ゴルルゴンかがむ ) {
						mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_disguise_discover
						mesg プレイヤー d:PLAYER position 0 -6000 -16000

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -1964,2321,-12358 -t 861,12932,-19649 \
							-r 3486,1807,0 -f 13181 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -2374,884,-11452 -t 1035,11885,-17854 \
							-r 3452,1729,0 -f 13177 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 142 \ // 補完時間を指定
							-s 1

						return 142;
					}
					case ( d:RAI_DEMO:ゴルルゴンちょっと起き上がる ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -2939,4078,-10392 -t 1540,13178,-18802 \
							-r 3599,1729,0 -f 13175 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 60 \ // 補完時間を指定
							-s 1

						return 90;
					}
					case ( d:RAI_DEMO:ゴルルゴン再びかがむ１ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3698,3095,-9846 -t 1213,10959,-19205 \
							-r 3679,1733,0 -f 13174 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 63 \ // 補完時間を指定
							-s 1

						return 63;
					}
					case ( d:RAI_DEMO:ゴルルゴン再びかがむ２ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3617,2756,-9958 -t 3421,8038,-19759 \
							-r 3827,1642,0 -f 13172 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 89 \ // 補完時間を指定
							-s 1

						return 89;
					}
					case ( d:RAI_DEMO:ライデン捕まる ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3517,2486,-9681 -t 3900,5234,-20212 \
							-r 3959,1648,0 -f 13170 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 47 \ // 補完時間を指定
							-s 1

						mesg プットモーションモデル デモライデン 表示
						return 20;

					}
					case ( d:RAI_DEMO:ライデン振動開始 ) {
						chara パッド振動スクリプト 地震振動:04 -mode 2 \
							-data 255 35 150 15
						mesg プットモーションモデル デモライデン モーション再生 d:G_MTN:rai_disguise_capture
						return 57;
					}
					case ( d:RAI_DEMO:ゴルルゴン起き上がってくる１ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -4022,3402,-9939 -t 2428,10746,-18749 \
							-r -386,1636,0 -f 13159 \
							-a 189 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 122 \ // 補完時間を指定
							-s 1

						return 122;
					}
					case ( d:RAI_DEMO:ゴルルゴン起き上がってくる２ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3161,6184,-12407 -t 1629,15827,-19944 \
							-r 3559,1679,0 -f 13143 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 127 \ // 補完時間を指定
							-s 1

						return 127;
					}
					case ( d:RAI_DEMO:ゴルルゴンにっこり ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3665,5108,-11638 -t 2696,14067,-18809 \
							-r 3606,1575,0 -f 13120 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 132 \ // 補完時間を指定
							-s 1

						return 453;
					}
					case ( d:RAI_DEMO:ゴルルゴン立ち去る ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,7621,2076 -t 0,8757,-21032 \
							-r 4064,2048,0 -f 23136 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						return 216;
					}
					case ( d:RAI_DEMO:カメラ引く ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 0,6472,25488 -t 0,7607,2383 \
							-r -32,2048,0 -f 23132 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 600 \ // 補完時間を指定
							-s 1

						return 417;
					}
					case ( d:RAI_DEMO:ストリームフェードアウト ) {
						@ストリーミングフェードアウト $$i:シナリオデモストリーム返り値
						return 63;
					}
					case ( d:RAI_DEMO:ゲームオーバー ) {
						command セットサウンドコード -c d:INT_PAUSEOFF
						@ミッション共通ゲームオーバー処理
					}
				}
			}
	}
}



//-----------------------------------------------------------
// 特殊ゲームクリア
//-----------------------------------------------------------
proc 雑誌反応時デモ発動 {
	#if d:DEBUG_PRINT 
		print 'raiden_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 ) {
		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
		#endif

		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// アイテムボックスは消去
		mesg アイテム ＡＫＳ本体 kill
		mesg アイテム ＰＳＧ１本体 kill
		mesg アイテム ＲＧＢ６本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム レーション kill
		mesg アイテム 雑誌 kill
		mesg アイテム ＡＫＳ弾 kill
		mesg アイテム ＰＳＧ１弾 kill
		mesg アイテム ＲＧＢ６弾 kill
		mesg アイテム スティンガー弾 kill
		mesg アイテム ジアゼパム kill
	
		chara delay シナリオデモディレイ -time (d:C_FADE_TIME + 30) \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:GUR_BOOK_DEMO:雑誌発見カット ) {
						#if d:DEBUG_PRINT 
							print 'StreamStatus start! $status='$status
						#endif

						command ゲットゴルルゴン頭座標 $$i:ゴルＸ $$i:ゴルＹ $$i:ゴルＺ

						if ( $i:プレイヤー位置Ｚ <= -2000 ) {
							mesg プレイヤー d:PLAYER position 0 1000 -6000
						} else if ( $i:プレイヤー位置Ｘ <= -4000 ) {
							mesg プレイヤー d:PLAYER position -4000 1000 -1500
						}

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,4092,9580 -t $$i:ゴルＸ,9986,$$i:ゴルＺ \
							-r 3972,2048,0 -f 31174 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 0,4092,9580 -t $$i:ゴルＸ,9986,$$i:ゴルＺ \
							-r 3972,2048,0 -f 31174 \
							-a 250 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 25 \ // 補完時間を指定
							-s 1

						return 119;
					}
					case ( d:GUR_BOOK_DEMO:ステージモデルチェンジ ) {
						mesg ＶＲオブジェ 外壁の影 off
						mesg ＶＲオブジェ デモ用外壁の影 on
						mesg ＶＲオブジェ 箱03への影 off
						mesg ＶＲオブジェ デモ用箱03への影 on
						mesg ＶＲオブジェ 箱09への影 off
						mesg ＶＲオブジェ デモ用箱09への影 on
						mesg ＶＲオブジェ 箱 off
						mesg ＶＲオブジェ デモ箱 on
						mesg ＶＲオブジェ 箱影 off
						mesg ＶＲオブジェ デモ箱影 on

						return 1;
					}
					case ( d:GUR_BOOK_DEMO:ゴルルゴン背後カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 639,31184,-32191 -t 2491,6030,-12460 \
							-r 589,61,0 -f 32023 \
							-a 300 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -1523,31154,-32027 -t 332,6030,-12258 \
							-r 588,61,0 -f 32023 \
							-a 300 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 300 \ // 補完時間を指定
							-s 1

						// プレイヤー移動開始
						mesg プレイヤー d:PLAYER run \
							0 0 2000 \
							1 \ // 走り
							2048 \
							0

						return 300;
					}
					case ( d:GUR_BOOK_DEMO:ゴルルゴン見上げカメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -1433,226,2394 -t 36,10860,-23463 \
							-r 3842,2011,0 -f 27997 \
							-a 223 \
							-i 0 -1 0 0 \
							-s 1

						return 0;
					}
				}
			}
	}
}


proc チャフ攻撃時デモ発動 {
	#if d:DEBUG_PRINT 
		print 'chaff_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 ) {
		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
		#endif

		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// アイテムボックスは消去
		mesg アイテム Ｍ４本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム レーション kill
		mesg アイテム チャフ kill
		mesg アイテム Ｍ４弾 kill
		mesg アイテム ＰＳＧ１弾 kill
		mesg アイテム スティンガー弾 kill
		mesg アイテム ジアゼパム kill

		mesg プレイヤー d:PLAYER position 0 1000 -6000

		if ( $i:プレイヤー位置Ｘ < -2000 ) {
			chara カメラ設定 クリア時カメラ \
				-c 1 \	
				-p -8738,723,-6976 -t -157,7717,-19607 \
				-r 3816,1659,0 -f 16796 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1

			chara カメラ設定 クリア時カメラ \
				-c 1 \
				-p -8738,723,-6976 -t -718,9578,-18782 \
				-r 3734,1659,0 -f 16796 \
				-a 350 \
				-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
				-interp_time d:C_FADE_TIME \ // 補完時間を指定
				-s 1

		} else if ( $i:プレイヤー位置Ｘ > 2000 ) {
			chara カメラ設定 クリア時カメラ \
				-c 1 \	
				-p 9550,1201,-5510 -t -1382,8865,-22540 \
				-r 3860,2420,0 -f 21639 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1

			chara カメラ設定 クリア時カメラ \
				-c 1 \
				-p 9550,1201,-5510 -t -750,11431,-21557 \
				-r 3775,2420,0 -f 21639 \
				-a 350 \
				-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
				-interp_time d:C_FADE_TIME \ // 補完時間を指定
				-s 1

		} else {
			chara カメラ設定 クリア時カメラ \
				-c 1 \	
				-p -0,4064,2194 -t -0,7870,-21936 \
				-r 3994,2048,0 -f 24428 \
				-a 200 \
				-i 0 -1 0 0 \
				-s 1

			chara カメラ設定 クリア時カメラ \
				-c 1 \
				-p -0,4064,2194 -t -0,10687,-21319 \
				-r 3917,2048,0 -f 24428 \
				-a 400 \
				-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
				-interp_time d:C_FADE_TIME \ // 補完時間を指定
				-s 1

		}

		chara delay シナリオデモディレイ -time (d:C_FADE_TIME + 30) \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:MECH_CHAFF_DEMO:チャフ食らったカット ) {
						#if d:DEBUG_PRINT 
							print 'StreamStatus start! $status='$status
						#endif

						return 120;
					}
					case ( d:MECH_CHAFF_DEMO:メカゲノラ背後カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -9470,25844,-40423 -t -2036,5701,-16707 \
							-r 444,198,0 -f 31992 \
							-a 300 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -6392,25843,-41387 -t 1041,5701,-17671 \
							-r 444,198,0 -f 31991 \
							-a 300 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 300 \ // 補完時間を指定
							-s 1

						// プレイヤー移動開始
						mesg プレイヤー d:PLAYER run \
							0 0 2000 \
							1 \ // 走り
							2048 \
							0

						return 360;
					}
					case ( d:MECH_CHAFF_DEMO:メカゲノラ見上げカメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -884,258,4245 -t -284,1526,589 \
							-r 3881,1942,0 -f 3916 \
							-a 260 \
							-i 0 -1 0 0 \
							-s 1

						return 0;
					}
				}
			}
	}


}





proc たらい落下時デモ発動 {
	#if d:DEBUG_PRINT 
		print 'tarai_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 && $$f:クリアデモ発動中フラグ == 0 ) {

		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		$$f:クリアデモ発動中フラグ = 1;

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
		#endif


		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// アイテムボックスは消去
		mesg アイテム Ｍ４本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム Ｍ４弾 kill

		chara カメラ設定 クリア時カメラ \
			-c 1 \	
			-p 3285,7558,-4422 -t 1875,9791,-9151 \
			-r 3819,2237,0 -f 5416 \
			-a 200 \
			-i 1 -1 0 0 \
			-s 1


		// プレイヤー移動開始
		mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0
		mesg プレイヤー d:PLAYER run 0 0 -12000 1 2048 0

		chara delay シナリオデモディレイ -time (d:C_FADE_TIME+60) \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:GENO_TARAI_DEMO:たらい落下カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 5053,-490,-6017 -t 1392,9341,-13666 \
							-r 3536,2339,0 -f 12983 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 5053,-490,-6017 -t -250,3715,-17097 \
							-r 3881,2339,0 -f 12983 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 120 \ // 補完時間を指定
							-s 1
					}
				}
			}
	}
}





proc ゲノラ対ゴルルゴンデモ発動 {
	#if d:DEBUG_PRINT 
		print 'gll_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 && $$f:クリアデモ発動中フラグ == 0 ) {

		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		$$f:クリアデモ発動中フラグ = 1;

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
		#endif


		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// 登場音のストリームを読み込み、スタート開始待ち状態にする
		$$i:シナリオデモストリーム返り値 = `command Stream t:vc032015 -w`;

		// アイテムボックスは消去
		mesg アイテム Ｍ４本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム Ｍ４弾 kill

		chara カメラ設定 クリア時カメラ \
			-c 1 \	
			-p -859,-9037,-26806 -t -2476,-3886,-32651 \
			-r 3637,2224,0 -f 7957 \
			-a 200 \
			-i 0 -1 0 0 \
			-s 1

		mesg ゴルルゴン ゲノラ ストップ
		mesg プットモーションモデル デモゲノラ 水平移動量無効
		mesg プットモーションモデル デモゲノラ 表示
		mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gol_idle

		// プレイヤー移動開始
		mesg プレイヤー d:PLAYER stance d:FA_END_STAND 2048 2048 0
		mesg プレイヤー d:PLAYER run -2000 0 -12500 1 2048 0

		chara delay シナリオデモディレイ -time d:C_FADE_TIME \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:GENO_VS_GUR_DEMO:地震_弱 ) {
						// ストリーミングがちゃんと読み込まれているかチェック
						command StreamStatus $$i:シナリオデモストリーム返り値
						if ( $status == d:STREAM:WAIT ) {
							#if d:DEBUG_PRINT 
								print 'StreamStatus start! $status='$status
							#endif

							command StreamStart $$i:シナリオデモストリーム返り値
							chara シナリオカメラ振動 地震:01 -xlevel 10 -ylevel 10 -time 120 -mode 0

							chara パッド振動 ゴルルゴン登場振動 -vibfile gol_gno_start_sdemo

							// 画面暗くなる
							chara フォグ 霧 -c 10, 15, 17 -n -500 -f 70000 -t 240

							mesg ＶＲ空 ＶＲ空 ベース用環境光変化 30 40 35 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 0 28 32 30 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 1 33 53 67 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 3 37 67 85 300
							mesg ＶＲ空 ＶＲ空 モデル用環境光変化 0 17 26 25 300
							mesg ＶＲ空 ＶＲ空 モデル用環境光変化 1 10 45 45 300

							mesg 太陽 ソル 非表示

							return 120;

						} else {
							#if d:DEBUG_PRINT 
								print 'StreamStatus wait待ち $status='$status
							#endif

							// スタート準備が完了していない場合は再度チェック
							$$b:シナリオデモシーン番号 = 0;
							return 1;
						}

					}
					case ( d:GENO_VS_GUR_DEMO:地震_中 ) {
						mesg シナリオカメラ振動 地震:01 kill
						chara シナリオカメラ振動 地震:02 -xlevel 100 -ylevel 100 -time 120 -mode 0

						return 120;
					}
					case ( d:GENO_VS_GUR_DEMO:地震_強 ) {
						mesg シナリオカメラ振動 地震:02 kill
						chara シナリオカメラ振動 地震:03 -xlevel 150 -ylevel 150 -time 60 -mode 0

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,2210,-50 -t 0,2186,-15729 \
							-r 1,2048,0 -f 15679 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						return 60;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン登場 ) {
						mesg シナリオカメラ振動 地震:03 kill
						chara シナリオカメラ振動 地震:04 -xlevel 200 -ylevel 200 -time 180 -mode 1

						mesg プットモーションモデル デモゴルルゴン 水平移動量無効
						mesg プットモーションモデル デモゴルルゴン 表示
						mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_idle
						mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble

						mesg ステージアウトライン ステージ全体ライン 非表示
						mesg ステージアウトライン ステージ全体ライン壁なし 表示

						command セットサウンドコード -c d:INT_PAUSEON


						return 60;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンアップ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 0,2210,-50 -t -1532,5175,-15370 \
							-r 3972,2113,0 -f 15679 \
							-a 563 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 25 \ // 補完時間を指定
							-s 1

						return 120;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンきょろきょろ１ ) {
						mesg シナリオカメラ振動 地震:04 kill
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -3936,13301,-40697 -t -3921,13256,-40612 \
							-r 316,110,0 -f 98 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3936,13302,-40698 -t -3932,13259,-40610 \
							-r 296,30,0 -f 98 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 60 \ // 補完時間を指定
							-s 1

						return 60;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンきょろきょろ２ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3936,13302,-40699 -t -3912,13266,-40610 \
							-r 245,170,0 -f 98 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 120 \ // 補完時間を指定
							-s 1

						return 120;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンがゲノラ見つける ) {
						// プレイヤー移動開始
						mesg プレイヤー d:PLAYER run 0 0 -11500 1 0 0
						command	強制モーション終了プロック -proc スネーク張り付きっぱなし_ゴルルゴン用

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3936,13302,-40699 -t -3922,13262,-40611 \
							-r 277,99,0 -f 98 \
							-a 620 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 25 \ // 補完時間を指定
							-s 1

						return 120;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン雄叫び ) {
						mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_appear_wall

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 386,-5250,-34263 -t -4417,11505,-44624 \
							-r 3462,2331,0 -f 20277 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -9110,-4744,-34541 -t -2630,11086,-45428 \
							-r 3512,1698,0 -f 20275 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 351 \ // 補完時間を指定
							-s 1

						return 1;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン雄叫び振動開始 ) {
						chara パッド振動 ゴルルゴン雄叫び振動 -vibfile gol_appear_wall
						return 350;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプ ) {
						mesg プットモーションモデル デモゴルルゴン 終了判定 d:G_MTN:gol_high_jump_start
						return 1;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプ振動開始 ) {
						chara パッド振動 ジャンプ開始振動 -vibfile gol_high_jump_start
						return 66;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプ前カット ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -8577,-51,-30241 -t -3140,10947,-46369 \
							-r 3722,1836,0 -f 20264 \
							-a 220 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 59 \ // 補完時間を指定
							-s 1

						return 59;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプカット１ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -8520,191,-30086 -t -3312,10097,-46976 \
							-r 3763,1853,0 -f 20261 \
							-a 220 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 20 \ // 補完時間を指定
							-s 1

						return 20;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプカット２ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -8081,640,-29664 -t -3592,11478,-46180 \
							-r 3728,1875,0 -f 20258 \
							-a 220 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 70 \ // 補完時間を指定
							-s 1

						return 100;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンジャンプカット３ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -8168,2915,-28804 -t -3090,8853,-47488 \
							-r 3902,1875,0 -f 20252 \
							-a 220 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 57 \ // 補完時間を指定
							-s 1

						return 82;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン飛び立つ ) {
						chara シナリオカメラ振動 地震:05 -xlevel 200 -ylevel 200 -time 60 -mode 1
						// ゲノラもスタンバイ
						mesg プットモーションモデル デモゲノラ 瞬間移動 6500 -10000 -21000
						mesg プットモーションモデル デモゲノラ 回転 3072

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -8168,2915,-28803 -t -4607,17941,-41906 \
							-r 3551,1875,0 -f 20252 \
							-a 220 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 15 \ // 補完時間を指定
							-s 1

						return 45;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン着地開始 ) {
						mesg プットモーションモデル デモゴルルゴン 瞬間移動 -6500 -10000 -21000
						mesg プットモーションモデル デモゴルルゴン 回転 1024
						mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_high_jump_end

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 11825,-8509,-9208 -t -2815,4135,-21298 \
							-r 3713,2622,0 -f 22811 \
							-a 250 \
							-i 0 -1 0 0 \
							-s 1

						return 1;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン着地振動開始 ) {
						chara パッド振動 ジャンプ終了振動 -vibfile gol_high_jump_end
						return 12;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン着地カメラ ) {

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 11825,-8509,-9208 -t -5447,-4196,-23471 \
							-r 3972,2622,0 -f 22811 \
							-a 250 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 31 \ // 補完時間を指定
							-s 1

						return 6;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン着地 ) {
						chara シナリオカメラ振動 地震:06 -xlevel 200 -ylevel 200 -time 120 -mode 1
						mesg ＶＲオブジェ 外壁 crumble

						mesg ステージアウトライン 対決ステージライン 表示
						mesg ステージアウトライン ステージ全体ライン壁なし 非表示

						mesg フェードオブジェ ステージトンボ:02 フェードアウト

						return 25;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴン着地戻り ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 11825,-8509,-9208 -t -4320,542,-22541 \
							-r 3830,2622,0 -f 22811 \
							-a 250 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 131 \ // 補完時間を指定
							-s 1

						return 151;
					}
					case ( d:GENO_VS_GUR_DEMO:二人そろって雄叫び ) {
						mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gol_howl
						mesg プットモーションモデル デモゴルルゴン モーション再生 d:G_MTN:gol_howl

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,6881,1828 -t 0,2842,-10721 \
							-r 203,2048,0 -f 13183 \
							-a 200 \
							-i 2 -1 0 0 \
							-s 1

						return 1;
					}
					case ( d:GENO_VS_GUR_DEMO:雄叫び振動開始 ) {
						chara パッド振動 ゴルルゴン雄叫び振動２ -vibfile gol_howl
						chara パッド振動 ゲノラ雄叫び振動 -vibfile gol_howl

						return 542;
					}
					case ( d:GENO_VS_GUR_DEMO:戦闘開始 ) {
						mesg プットモーションモデル デモゲノラ 水平移動量有効
						mesg プットモーションモデル デモゴルルゴン 水平移動量有効
						mesg プットモーションモデル デモゲノラ 終了判定 d:G_MTN:gno_fight
						mesg プットモーションモデル デモゴルルゴン 終了判定 d:G_MTN:gol_fight

						return 1;
					}
					case ( d:GENO_VS_GUR_DEMO:戦闘振動開始 ) {
						chara パッド振動 戦闘振動 -vibfile gol_fight_sdemo
						return 59;
					}
					case ( d:GENO_VS_GUR_DEMO:ゲノラパンチ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -14161,1205,-28631 -t 695,2289,-21688 \
							-r 4053,739,0 -f 16434 \
							-a 258 \
							-i 0 -1 0 0 \
							-s 1

						return 50;
					}
					case ( d:GENO_VS_GUR_DEMO:ゲノラパンチヒット ) {
						chara シナリオカメラ振動 地震:07 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 76;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンタックル ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 13023,-3314,-33154 -t -1178,-507,-20043 \
							-r 4002,3558,0 -f 19530 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 14941,-4213,-29791 -t -2000,58,-16273 \
							-r -127,3517,0 -f 21889 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 122 \ // 補完時間を指定
							-s 1

						return 122;
					}
					case ( d:GENO_VS_GUR_DEMO:ゴルルゴンタックルヒット ) {
						chara シナリオカメラ振動 地震:08 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 139;
					}
					case ( d:GENO_VS_GUR_DEMO:元のカメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,6008,1318 -t 0,2337,-11339 \
							-r 184,2048,0 -f 13178 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						return 25;
					}
					case ( d:GENO_VS_GUR_DEMO:組み合う ) {
						chara シナリオカメラ振動 地震:09 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 536;
					}
					case ( d:GENO_VS_GUR_DEMO:二匹が落下 ) {
						chara シナリオカメラ振動 地震:10 -xlevel 200 -ylevel 200 -time 180 -mode 1
						command 水位設定 -lv -13000
						chara ゴルルゴン水柱 水柱君 \
							-w	10000 \
							-h	16000 \
							-pos -1000 -13000 -37000 \
							-cycle	59 \
							-time 150

						chara ゴルルゴン水柱 水柱君 \
							-w	10000 \
							-h	16000 \
							-pos 4500 -13000 -37000 \
							-cycle	59 \
							-time 150

						chara 速度可変 スローモーション \
							-l 240 \
							-t 2

						return 120;
					}
					case ( d:GENO_VS_GUR_DEMO:カメラ前に水しぶき ) {
						//	カメラ前水飛沫
						chara カメラ雨 カメラ水飛沫 \
							-l 0 \ // ０以外で吹き込みＯＦＦする
							-i 1  // ０以外で不可視

						mesg カメラ雨 カメラ水飛沫 カメラ前水飛沫出っ放し

						mesg パッドデモ 張り付きパッドデモ stop

						mesg 速度可変 スローモーション スキップ変更 0

						@ストリーミングフェードアウト $$i:シナリオデモストリーム返り値

						return 64;
					}
					case ( d:GENO_VS_GUR_DEMO:ゲームクリア ) {
						command セットサウンドコード -c d:INT_PAUSEOFF
						command ＶＲステージクリア
					}

				}
			}
	}
}


proc ゲノラ対メカゲノラデモ発動 {
	#if d:DEBUG_PRINT 
		print 'gll_demo_start!!!!!'
	#endif

	if (`command ゲームオーバーチェック` == 0 && $$f:クリアデモ発動中フラグ == 0 ) {

		@シナリオデモ開始処理
		// ゴーグル系も解除
		@プレイヤーアイテム追加解除
		command ＶＲタイマーポーズ
		mesg ＶＲスクリーン ＶＲスクリーン君 非表示
		command セットサウンドコード -c d:SNG_FOUTS_S

		$$f:クリアデモ発動中フラグ = 1;

		#if d:DEMO_DEBUG
			command パッド操作 -cancel
			command メニュー設定 -pause on
		#endif


		// 変数を初期化
		$$b:シナリオデモシーン番号 = 0;

		// 登場音のストリームを読み込み、スタート開始待ち状態にする
		$$i:シナリオデモストリーム返り値 = `command Stream t:vc032016 -w`;

		// アイテムボックスは消去
		mesg アイテム Ｍ４本体 kill
		mesg アイテム スティンガー本体 kill
		mesg アイテム Ｍ４弾 kill

		// タライも消去
		mesg ゴルルゴン ゲノラ タライ消える

		chara カメラ設定 クリア時カメラ \
			-c 1 \	
			-p -0,1498,2581 -t -0,11514,-17470 \
			-r 3794,2048,0 -f 22414 \
			-a 200 \
			-i 0 -1 0 0 \
			-s 1

		mesg ゴルルゴン ゲノラ ストップ
		mesg プットモーションモデル デモゲノラ 水平移動量無効
		mesg プットモーションモデル デモゲノラ 表示
		mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gol_idle
		mesg プレイヤー d:PLAYER stance d:FA_END_STAND -1 2560 0

		mesg ＶＲオブジェ 箱 off
		mesg ＶＲオブジェ デモ箱 on

		chara delay シナリオデモディレイ -time (d:C_FADE_TIME+30) \
			-exec {
				$$b:シナリオデモシーン番号 = $$b:シナリオデモシーン番号 + 1;

				switch ( $$b:シナリオデモシーン番号 ) {
					case ( d:GENO_VS_MECH_DEMO:暗くなる ) {
						// ストリーミングがちゃんと読み込まれているかチェック
						command StreamStatus $$i:シナリオデモストリーム返り値
						if ( $status == d:STREAM:WAIT ) {
							#if d:DEBUG_PRINT 
								print 'StreamStatus start! $status='$status
							#endif

							command StreamStart $$i:シナリオデモストリーム返り値
							// 画面暗くなる
							chara フォグ 霧 -c 10, 15, 17 -n -500 -f 48000 -t 205

							mesg ＶＲ空 ＶＲ空 ベース用環境光変化 30 40 35 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 0 28 32 30 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 1 33 53 67 300
							mesg ＶＲ空 ＶＲ空 雲用環境光変化 3 37 67 85 300
							mesg ＶＲ空 ＶＲ空 モデル用環境光変化 0 17 26 25 300
							mesg ＶＲ空 ＶＲ空 モデル用環境光変化 1 10 45 45 300

							mesg 太陽 ソル 非表示

							chara パッド振動 登場振動 -vibfile mgn_vs_gnr_start_sdemo

							return 60;

						} else {
							#if d:DEBUG_PRINT 
								print 'StreamStatus wait待ち $status='$status
							#endif

							// スタート準備が完了していない場合は再度チェック
							$$b:シナリオデモシーン番号 = 0;
							return 1;
						}

					}
					case ( d:GENO_VS_MECH_DEMO:稲光01 ) {
						chara 任意稲光 稲妻君 \
							-x 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
							-u -2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 1024   // -4096(UPPER) ～ 4096(LOWER)

						return 68;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光02 ) {
						chara 任意稲光 稲妻君 \
							-x -2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3900 \ // -4096(UPPER) ～ 4096(LOWER)
							-u 3072 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 2048   // -4096(UPPER) ～ 4096(LOWER)

						return 52;
					}
					case ( d:GENO_VS_MECH_DEMO:空を見る ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 0,445,-6248 -t -2317,11106,-12984 \
							-r 3456,2264,0 -f 12822 \
							-a 200 \
							-i 2 -1 0 0 \
							-s 1

						return 25;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ登場 ) {
						// メカゲノラ生成開始
						mesg オブジェクト分解 オブジェクト分解君 変形開始
						mesg プットモーションモデル デモメカゲノラ 水平移動量無効

						command セットサウンドコード -c d:INT_PAUSEON


						return 29;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光11 ) {
						chara 任意稲光 稲妻君 \
							-x -512 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u -3586 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 2048   // -4096(UPPER) ～ 4096(LOWER)

						chara 任意稲光 稲妻君 \
							-x -128 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u 2048 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 3072   // -4096(UPPER) ～ 4096(LOWER)

						return 61;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ登場カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 4523,24322,-22377 -t -5584,21749,-20957 \
							-r 161,3163,0 -f 10526 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 4523,24322,-22377 -t -3714,30773,-21219 \
							-r 3666,3163,0 -f 10526 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 90 \ // 補完時間を指定
							-s 1

						return 30;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光21 ) {
						chara 任意稲光 稲妻君 \
							-x 3586 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
							-u -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 4096   // -4096(UPPER) ～ 4096(LOWER)

						return 30;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光22 ) {
						chara 任意稲光 稲妻君 \
							-x -3586 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -4096 \ // -4096(UPPER) ～ 4096(LOWER)
							-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 4096   // -4096(UPPER) ～ 4096(LOWER)

						return 68;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光23 ) {
						chara 任意稲光 稲妻君 \
							-x -256 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u -3072 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 3072   // -4096(UPPER) ～ 4096(LOWER)

						chara 任意稲光 稲妻君 \
							-x 1024 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u 4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 256   // -4096(UPPER) ～ 4096(LOWER)

						return 10;
					}
					case ( d:GENO_VS_MECH_DEMO:稲光24 ) {
						chara 任意稲光 稲妻君 \
							-x -1536 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u -4096 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 1024   // -4096(UPPER) ～ 4096(LOWER)

						chara 任意稲光 稲妻君 \
							-x 512 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-y -3710 \ // -4096(UPPER) ～ 4096(LOWER)
							-u 3072 \ // -4096(LEFT ) ～ 4096(RIGHT)
							-v 3072   // -4096(UPPER) ～ 4096(LOWER)

						return 42;
					}
					case ( d:GENO_VS_MECH_DEMO:ゲノラ捕捉１ ) {
						chara デモサイト表示 メカゲノラサイト

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p -3918,38528,-21005 -t -3912,38432,-21000 \
							-r 972,513,0 -f 97 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3918,38528,-21005 -t -3908,38432,-20996 \
							-r 936,513,0 -f 97 \
							-a 400 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 25 \ // 補完時間を指定
							-s 1

						return 35;
					}
					case ( d:GENO_VS_MECH_DEMO:ゲノラ捕捉２ ) {
						// プレイヤー移動開始
						mesg プレイヤー d:PLAYER run 0 0 -11500 1 0 0
						command	強制モーション終了プロック -proc スネーク張り付きっぱなし_メカゲノラ用

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p -3918,38527,-21005 -t -3910,38431,-20992 \
							-r 926,363,0 -f 97 \
							-a 600 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 25 \ // 補完時間を指定
							-s 1

						return 140;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ落下 ) {
						mesg プットモーションモデル デモメカゲノラ モーション再生 d:G_MTN:gno_meca_toujyou
						mesg プットモーションモデル デモメカゲノラ 基本モーション変更 d:G_MTN:gol_idle
						// ゲノラもスタンバイ
						mesg プットモーションモデル デモゲノラ 瞬間移動 6500 -10000 -21000
						mesg プットモーションモデル デモゲノラ 回転 3072

						// サイトも消去
						mesg デモサイト表示 メカゲノラサイト kill

						return 1;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ落下カメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 9516,-9413,-12213 -t 5875,-2641,-14395 \
							-r 3437,2720,0 -f 7993 \
							-a 200 \
							-i 0 -1 0 0 \
							-s 1

						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 9342,-9401,-11916 -t 3065,-6755,-16096 \
							-r 3876,2689,0 -f 7992 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 113 \ // 補完時間を指定
							-s 1


						return 70;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ着地 ) {
						chara シナリオカメラ振動 地震:06 -xlevel 200 -ylevel 200 -time 120 -mode 1
						mesg ＶＲオブジェ ゴルルゴンを隠す壁 crumble
						mesg ＶＲオブジェ 外壁 crumble

						mesg ステージアウトライン ステージ全体ライン 非表示
						mesg ステージアウトライン 対決ステージライン 表示

						mesg フェードオブジェ ステージトンボ:02 フェードアウト

						return 43;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラ着地戻り ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \
							-p 9349,-9396,-11924 -t 3444,-5705,-15844 \
							-r 3783,2690,0 -f 7991 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 44 \ // 補完時間を指定
							-s 1

						return 76;
					}
					case ( d:GENO_VS_MECH_DEMO:二人そろって雄叫び ) {
						mesg プットモーションモデル デモゲノラ モーション再生 d:G_MTN:gol_howl
						mesg プットモーションモデル デモメカゲノラ モーション再生 d:G_MTN:gol_howl

						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,5587,1582 -t 0,2426,-11206 \
							-r 158,2048,0 -f 13173 \
							-a 200 \
							-i 2 -1 0 0 \
							-s 1

						return 1;
					}
					case ( d:GENO_VS_MECH_DEMO:雄叫び振動開始 ) {
						chara パッド振動 メカゲノラ雄叫び振動 -vibfile gol_howl
						chara パッド振動 ゲノラ雄叫び振動 -vibfile gol_howl
						return 542;
					}
					case ( d:GENO_VS_MECH_DEMO:戦闘開始 ) {
						mesg プットモーションモデル デモゲノラ 水平移動量有効
						mesg プットモーションモデル デモメカゲノラ 水平移動量有効
						mesg プットモーションモデル デモゲノラ 終了判定 d:G_MTN:gno_fight
						mesg プットモーションモデル デモメカゲノラ 終了判定 d:G_MTN:gol_fight

						return 1;
					}
					case ( d:GENO_VS_MECH_DEMO:戦闘振動開始 ) {
						chara パッド振動 戦闘振動 -vibfile gol_fight_sdemo
						return 59;
					}
					case ( d:GENO_VS_MECH_DEMO:ゲノラパンチ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 437,4149,4594 -t -1323,-1030,-22697 \
							-r 122,2090,0 -f 27833 \
							-a 250 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 50 \ // 補完時間を指定
							-s 1

						return 50;
					}
					case ( d:GENO_VS_MECH_DEMO:ゲノラパンチヒット ) {
						chara シナリオカメラ振動 地震:07 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 145;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラタックル ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 361,4568,4497 -t 4927,-1363,-22307 \
							-r 140,1938,0 -f 27830 \
							-a 250 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 54 \ // 補完時間を指定
							-s 1

						return 54;
					}
					case ( d:GENO_VS_MECH_DEMO:メカゲノラタックルヒット ) {
						chara シナリオカメラ振動 地震:08 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 139;
					}
					case ( d:GENO_VS_MECH_DEMO:元のカメラ ) {
						chara カメラ設定 クリア時カメラ \
							-c 1 \	
							-p 0,5587,1582 -t 0,2426,-11206 \
							-r 158,2048,0 -f 13173 \
							-a 200 \
							-z d:CAMERA_INTERP_CAMERA \ // 補完カメラフラグを指定。
							-interp_time 60 \ // 補完時間を指定
							-s 1

						return 25;
					}
					case ( d:GENO_VS_MECH_DEMO:組み合う ) {
						chara シナリオカメラ振動 地震:09 -xlevel 200 -ylevel 200 -time 60 -mode 1
						return 536;
					}
					case ( d:GENO_VS_MECH_DEMO:二匹が落下 ) {
						chara シナリオカメラ振動 地震:10 -xlevel 200 -ylevel 200 -time 180 -mode 1
						command 水位設定 -lv -13000
						chara ゴルルゴン水柱 水柱君 \
							-w	10000 \
							-h	16000 \
							-pos -1000 -13000 -37000 \
							-cycle	59 \
							-time 150

						chara ゴルルゴン水柱 水柱君 \
							-w	10000 \
							-h	16000 \
							-pos 4500 -13000 -37000 \
							-cycle	59 \
							-time 150

						chara 速度可変 スローモーション \
							-l 240 \
							-t 2

						return 120;
					}
					case ( d:GENO_VS_MECH_DEMO:カメラ前に水しぶき ) {
						//	カメラ前水飛沫
						chara カメラ雨 カメラ水飛沫 \
							-l 0 \ // ０以外で吹き込みＯＦＦする
							-i 1  // ０以外で不可視

						mesg カメラ雨 カメラ水飛沫 カメラ前水飛沫出っ放し

						mesg 速度可変 スローモーション スキップ変更 0

						@ストリーミングフェードアウト $$i:シナリオデモストリーム返り値

						return 64;
					}
					case ( d:GENO_VS_MECH_DEMO:ゲームクリア ) {
						command セットサウンドコード -c d:INT_PAUSEOFF
						command ＶＲステージクリア
					}

				}
			}
	}
}

