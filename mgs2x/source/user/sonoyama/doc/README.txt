/*
	README.txt	
	sonoyamaディレクトリのファイル構成

	2001/10/22	M.Sonoyama
	$Id: README.txt,v 1.1.1.3 2002/11/19 11:50:40 Yoshizawa1 Exp $
*/

bugs/		予定、バグ、仕様書、メモ書き保存ディレクトリ
	2dtool.txt  			２Ｄツールの仕様案メモ
	bugs.txt    			バグや仕様変更や予定メモ（2000／10／02）
	kobayashi.txt  			小林（聖）作業メモ
	schedule.txt			スケジュールメモ（2001／01／26）
	memo.txt				作業上注意することなどのメモ
	tsukurumono.txt			つくるものリストと覚え書き（2001／05／23）

debug/		デバッグキャラ
	dbgobj.c				動的壁弾痕テスト

doc/		ドキュメント
	README.txt				このファイル
	player.txt				プレイヤープログラム解説書
	solidus.txt				ソリダス戦プログラム解説書

etc/		いろいろ
	belt.c					ベルトコンベアベルト部分
	beltconv.c				ベルトコンベア移動処理
	beltobj.c				ベルトコンベア上荷物
	boss_survival.c			ボスサバイバルモードセットコマンド
	breath.c				白息
	bul_fall.c				落ちる弾。グレネードピン、フォーチュンバリアにあたったミサイルなど
	ch_camera.c				キャラ追従カメラ（デバッグ用）
	demo_snakearm.c			主観デモ用ソリダス蛇手
	dsegment.c				動的壁床、シナリオ呼び出し
	dtrap.c					動的トラップ（デバッグ用）
	dv_goggles.c			潜水ゴーグル主観
	etc.c					プロック連続実行
	evt_wall.c				イベント壁（未使用）
	fallflr.c				崩落床
	gaged.c					ゲージ管理デーモン
	hide.c					隠し君（未使用）
	lodctrl.c				ＬｏＤ管理
	multi.c					マルチウェイトモデル上乗せ
	o2gage.c				Ｏ２ゲージ
	objhzx.c				オブジェクトにハザードを張る
	paddemo.c				パッドデモ再生
	padrec.c				パッドデモ録画
	pl_vib.c				プレイヤーモーション振動（未使用）
	psg_sight.c				ＰＳＧ１仮サイト（未使用）
	scncamvib.c				シナリオカメラ振動
	shavedsnake.c			髭剃りスネークテクスチャ入れ替え
	sunglasses.c			サングラス管理
	t_sight.c				双眼鏡仮サイト（未使用）
	targettrap.c			トラップにターゲットを張る
	zoomcam.c				ズームカメラシステム

plugin/		プレイヤープラグイン各種
	beltply.c				ベルトコンベアモード
	bladeply.c				ブレード攻撃
	ee_swim.c				水中エマ
	eeply.c					エマ手つなぎモード
	eetest.c				未使用
	elevator.c				エレベータ
	fall.c					落下死
	jump.c					１ｍ段差登り降り
	kageshibari.c			バンプ影縛り
	ladder.c				はしごモード
	lckcnct.c				つながりロッカー（未使用）
	locker2.c				ロッカーモード（新型）
	lockerd.c				ロッカーモード（旧型）
	run.c					強制走り
	shadowhzx.c				影発見位置計算（未使用）
	sneeze.c				くしゃみ発動
	spothzx.c				影発見位置計算
	stance.c				強制姿勢制御
	superblow.c				ハリアークラスター専用プレイヤー吹っ飛ばし
	water.c					水中モード
	waterB.c				水中モード実験（未使用）
	water_old.c				水中モード旧型（未使用）
	wt_door.c				水密ドア（旧型）
	wt_door2.c				水密ドア（新型）

raiden/		プレイヤー
	adjust.c				位置角度調整ルーチン		raiden.cにinclude
	arm_motion.h			主観腕モーション定義ヘッダ（未使用）
	command.c				プレイヤーシナリオコマンド群１
	command2.c				２
	command3.c				３
	command4.c				４
	command5.c				５
	dbg_menu.c				デバッグメニュールーチン	raiden.cにinclude
	dbviewpl.c				デバッグステータス表示		
	equip.c					特殊装備品ルーチン			raiden.cにinclude
	event.c					イベント処理ルーチン		raiden.cにinclude
	force.c					強制、エルード処理ルーチン	raiden.cにinclude
	item.h					装備品処理定義ヘッダ		raiden.cにinclude
	motion.h				モーション定義ヘッダ		
	normal.c				基本モードルーチン			raiden.cにinclude
	peep.c					覗き込み関連ルーチン		raiden.cにinclude
	pl_arm.c				主観腕
	pl_arm.h				主観腕ヘッダ	
	pl_arm_mot.h			主観腕モーション定義ヘッダ
	pl_bul_splash.c			弾水飛沫設定
	pl_deadmode.c			特殊ゲームオーバー設定
	pl_define.h				プレイヤー各種定義ヘッダ
	pl_dummy.c				ダミープレイヤー
	pl_global.c				プレイヤー関連グローバル関数
	pl_inline.c				プレイヤーインライン関数
	pl_inline.h				インライン関数のヘッダ
	pl_plugin.c				プラグイン設定
	pl_subcam_mng.c			主観カメラＯＦＦ制御
	pl_subject_demo.c		主観デモプレイヤー
	pl_work.h				プレイヤーワーク定義
	pl_work.x				プレイヤー外部関数宣言
	rai_action_blur.c		モーションブラー
	rai_arm.c				主観腕（旧型）
	rai_arm.h				主観腕ヘッダ（旧型）
	rai_equip.c				装備品管理
	rai_init.c				変数初期化
	rai_nude_ik.c			はだか時腕ＩＫ
	rai_shadow_hair.c		影用髪の毛
	raiden.c				プレイヤー起動
	raiden.h				プレイヤー通常時各種定義
	raiden.x				プレイヤープロトタイプ宣言
	routine.c				各種便利関数				raiden.cにinclude
	special.c				ダメージなど				raiden.cにinclude
	weapon.h				武器処理定義ヘッダ			raiden.cにinclude

solidus/	ソリダス戦
	attack_sol.c			攻撃関連					solidus.cにinclude
	between_cam.c			ソリダス戦専用カメラ
	body_flame.c			体につく炎（本体は別）
	careful_sol.c			間合いとりフェーズ			solidus.cにinclude
	damage_sol.c			ダメージ処理				solidus.cにinclude
	dbg_menu_sol.c			デバッグメニュー			solidus.cにinclude
	event_sol.c				イベント処理				solidus.cにinclude
	flame.c					未使用
	init_sol.c				初期化						solidus.cにinclude
	kwt_mst.c				刀
	normal_sol.c			基本モード					solidus.cにinclude
	pl_liftup.c				プレイヤーつかみ上げ
	sa_missile.c			蛇手ミサイル
	sam_slashed.c			斬られた蛇手ミサイル
	snakearm.c				蛇手
	snakearm_mot.h			蛇手モーション定義
	sol_common.h			ソリダス共通ヘッダ
	sol_global.c			ソリダスグローバル関数
	solidus.c				ソリダス起動、メイン関数
	solidus.h				ソリダス各種定義
	solidus.x				ソリダスプロトタイプ宣言
	subjectarm_chain.c		プレイヤーの主観腕につく手錠
	think_sol.c				ソリダス思考				solidus.cにinclude
	util_sol.c				ソリダス各種便利関数		solidus.cにinclude

	
	
	
	

