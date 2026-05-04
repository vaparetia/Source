/*
	beyond_high.h                     
	    高い場所ビヨンドモード用proc

	1999/12/16 H.Yoshiike         
	$Id: beyond_high.h,v 1.1 2002/02/01 06:08:31 usr01475 Exp $                      

	
*/

// ファイルが二重呼びされたときの対処
#ifndef d:BEYOND_HIGH_H
#define BEYOND_HIGH_H	1

/*
※このファイルをincludeする際は、list以下でステージ固有のmlsファイルを作成した後に次の例のようなモーションリストをgclに記述してください。

enum モーションリスト {
	none_drop_start = 0,		// ビヨンド開始
	sna_non_drop_start_behind_t,// ビヨンド開始(ビハインドから)
	none_drop,					// ビヨンド静止中
	none_drop_r,				// ビヨンド右移動
	none_drop_l,				// ビヨンド左移動
	none_drop_end_fence,		// ビヨンド終了(壁なし)
	none_drop_end_wall,			// ビヨンド終了(壁あり)
	none_drop_idle_tired,		// ビヨンド疲れ
	non_elude_fall_start		// 落下モーション(落ちはじめ)
	non_elude_fall				// 落下中モーション
	non_elude_fall_damage		// 落下してダメージ
	non_elude_fall_out			// 落下して死亡
	non_elude_fall_normal		// 落下しても無事着地
	non_drop_dam				// エルード中のダメージ
	non_drop_pullup_start		//懸垂開始
	non_drop_pullup_idle		//懸垂静止
	non_drop_pullup_end			//懸垂終了
};
*/

// ビヨンド関係

// 同じでファインを別のヘッダファイルで行っている。
// 値の同一性が薄れるので、修正すること。とりあえずマスクした。
// (2001.3.12 松花）

//#define		ADJ_DIS		450		// トラップの端に入った場合にずらす距離
// スネーク
//#define		SNA_ADJ_DIS_NOMAL	550		// 通常で入ったときの手すりとの距離
//#define		SNA_ADJ_DIS_BEHIND	300		// ビハインドで入ったときの手すりとの距離
// ライデン
//#define		RAI_ADJ_DIS_NOMAL	442		// 通常で入ったときの手すりとの距離
//#define		RAI_ADJ_DIS_BEHIND	422		// ビハインドで入ったときの手すりとの距離

//#define		ADJ_BEYOND_X_DELTA	50	// ビヨンドになるモーション発動前と発動後のＸ軸座標のずれ


// ビヨンド用proc
//-----------------------------------------------------
// 手すりが垂直か平行方向にのびている場合
// $:強制移動軸：軸方向に強制移動 ： 0/1/2 ： Ｘ軸/Ｙ軸/Ｚ軸
// $:強制移動座標１：通常用の移動座標：	("手すりの座標"+"$i:ADJ_DIS_NOMAL")
//										例) (-1000+($i:ADJ_DIS_NOMAL))
// $:強制移動座標２：ビハインド用の移動座標

	proc 北向きビヨンドハイ発動 $:トラップ名 $:手すり位置 $:プレイヤーＸ座標 $:トラップ左端 $:トラップ右端 $:フラグ {
		@ビヨンドハイ補正座標初期設定
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );

		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (2048-256)) && (($w:BY_プレイヤー方向) < (2048+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置+$i:ADJ_DIS_NOMAL)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					2048 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					0 \											// 開始時向き
					0 \											// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了



				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			if ($w:BY_プレイヤー方向 <= 2048 ) {
				eval( $w:BY_プレイヤー方向 = $w:BY_プレイヤー方向 + 4096 );
			}
			
			if ((($w:BY_プレイヤー方向) > (4096-256)) && (($w:BY_プレイヤー方向) < (4096+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置+$i:ADJ_DIS_BEHIND)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					2048 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					0 \											// 開始時向き
					0 \											// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}


	proc 南向きビヨンドハイ発動 $:トラップ名 $:手すり位置 $:プレイヤーＸ座標 $:トラップ左端 $:トラップ右端 $:フラグ {
		@ビヨンドハイ補正座標初期設定
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );

		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ($w:BY_プレイヤー方向 >= 3072 ) {
				eval( $w:BY_プレイヤー方向 = $w:BY_プレイヤー方向 - 4096 );
			}

			if ((($w:BY_プレイヤー方向) > (0-256)) && (($w:BY_プレイヤー方向) < (0+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置-$i:ADJ_DIS_NOMAL)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					0 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					2048 \										// 開始時向き
					2048 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (2048-256)) && (($w:BY_プレイヤー方向) < (2048+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置-$i:ADJ_DIS_BEHIND)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					0 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					2048 \										// 開始時向き
					2048 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}


	proc 西向きビヨンドハイ発動 $:トラップ名 $:手すり位置 $:プレイヤーＺ座標 $:トラップ上端 $:トラップ下端 $:フラグ {
		@ビヨンドハイ補正座標初期設定
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );

		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (3072-256)) && (($w:BY_プレイヤー方向) < (3072+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 0 ($:手すり位置+$i:ADJ_DIS_NOMAL)
				if ($:プレイヤーＺ座標 < $:トラップ上端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ上端+d:ADJ_DIS)
				} else if ($:プレイヤーＺ座標 > $:トラップ下端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ下端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					3072 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					1024 \										// 開始時向き
					1024 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (1024-256)) && (($w:BY_プレイヤー方向) < (1024+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 0 ($:手すり位置+$i:ADJ_DIS_BEHIND)
				if ($:プレイヤーＺ座標 < $:トラップ上端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ上端+d:ADJ_DIS)
				} else if ($:プレイヤーＺ座標 > $:トラップ下端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ下端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					3072 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					1024 \										// 開始時向き
					1024 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}


	proc 東向きビヨンドハイ発動 $:トラップ名 $:手すり位置 $:プレイヤーＺ座標 $:トラップ上端 $:トラップ下端 $:フラグ {
		@ビヨンドハイ補正座標初期設定
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );

		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (1024-256)) && (($w:BY_プレイヤー方向) < (1024+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 0 ($:手すり位置-$i:ADJ_DIS_NOMAL)
				if ($:プレイヤーＺ座標 < $:トラップ上端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ上端+d:ADJ_DIS)
				} else if ($:プレイヤーＺ座標 > $:トラップ下端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ下端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					1024 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					3072 \										// 開始時向き
					3072 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (3072-256)) && (($w:BY_プレイヤー方向) < (3072+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 0 ($:手すり位置-$i:ADJ_DIS_BEHIND)
				if ($:プレイヤーＺ座標 < $:トラップ上端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ上端+d:ADJ_DIS)
				} else if ($:プレイヤーＺ座標 > $:トラップ下端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 2 ($:トラップ下端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					1024 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					3072 \										// 開始時向き
					3072 \										// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}




// 斜めビハインドPROC
// ----------------------------------------------------
	proc 斜めビヨンドハイ発動 $:トラップ名 $:プレイヤーＸ位置 $:プレイヤーＺ位置 $:手すりＡ点Ｘ座標 $:手すりＡ点Ｚ座標 $:手すりＢ点Ｘ座標 $:手すりＢ点Ｚ座標 $:発動距離 $:フラグ {
		@ビヨンドハイ補正座標初期設定
		if ( $:手すりＡ点Ｘ座標  < $:手すりＢ点Ｘ座標) {
			eval( $i:EX_手すりＡ点Ｘ座標 = $:手すりＡ点Ｘ座標 );
			eval( $i:EX_手すりＡ点Ｚ座標 = $:手すりＡ点Ｚ座標 );
			eval( $i:EX_手すりＢ点Ｘ座標 = $:手すりＢ点Ｘ座標 );
			eval( $i:EX_手すりＢ点Ｚ座標 = $:手すりＢ点Ｚ座標 );
		} else {
			eval( $i:EX_手すりＡ点Ｘ座標 = $:手すりＢ点Ｘ座標 );
			eval( $i:EX_手すりＡ点Ｚ座標 = $:手すりＢ点Ｚ座標 );
			eval( $i:EX_手すりＢ点Ｘ座標 = $:手すりＡ点Ｘ座標 );
			eval( $i:EX_手すりＢ点Ｚ座標 = $:手すりＡ点Ｚ座標 );
		}
		if ( $:手すりＡ点Ｘ座標  < $:手すりＢ点Ｘ座標) {
			eval( $i:可動範囲最小Ｘ座標 = $:手すりＡ点Ｘ座標 );
			eval( $i:可動範囲最大Ｘ座標 = $:手すりＢ点Ｘ座標 );
		} else {
			eval( $i:可動範囲最小Ｘ座標 = $:手すりＢ点Ｘ座標 );
			eval( $i:可動範囲最大Ｘ座標 = $:手すりＡ点Ｘ座標 );
		}
		if ( $:手すりＡ点Ｚ座標  < $:手すりＢ点Ｚ座標) {
			eval( $i:可動範囲最小Ｚ座標 = $:手すりＡ点Ｚ座標 );
			eval( $i:可動範囲最大Ｚ座標 = $:手すりＢ点Ｚ座標 );
		} else {
			eval( $i:可動範囲最小Ｚ座標 = $:手すりＢ点Ｚ座標 );
			eval( $i:可動範囲最大Ｚ座標 = $:手すりＡ点Ｚ座標 );
		}

		eval( $w:デルタＸ座標 = $i:EX_手すりＢ点Ｘ座標 - $i:EX_手すりＡ点Ｘ座標 ) ;
		eval( $w:デルタＺ座標 = $i:EX_手すりＢ点Ｚ座標 - $i:EX_手すりＡ点Ｚ座標 ) ;
		eval( $i:手すり傾き１ = ($w:デルタＺ座標 * 10000 ) / ($w:デルタＸ座標)) ;
		eval( $i:手すり傾き２ = (-(($w:デルタＸ座標 * 10000 ) / ($w:デルタＺ座標)))) ;
		eval( $i:手すり加算値１ = $i:EX_手すりＡ点Ｚ座標 - (($i:EX_手すりＡ点Ｘ座標 * $i:手すり傾き１) / 10000)) ;
		eval( $i:手すり加算値２ = $:プレイヤーＺ位置 - (($:プレイヤーＸ位置 * $i:手すり傾き２) / 10000)) ;
		eval( $i:手すり交点Ｘ = (((( $i:手すり加算値２ - $i:手すり加算値１ ) * 10000 ) / ( $i:手すり傾き１ - $i:手すり傾き２ )))) ;
		eval( $i:手すり交点Ｚ = (( $i:手すり傾き１ * $i:手すり交点Ｘ ) / 10000 ) + $i:手すり加算値１) ;

		eval( $w:デルタＸ座標１ = $i:EX_手すりＢ点Ｘ座標 - $i:EX_手すりＡ点Ｘ座標 ) ;
		eval( $w:デルタＺ座標１ = $i:EX_手すりＢ点Ｚ座標 - $i:EX_手すりＡ点Ｚ座標 ) ;
		eval( $w:手すり角度 = `command atan $w:デルタＺ座標１ $w:デルタＸ座標１`) ;
		eval( $i:発動誤差Ｘ座標 = `command cos $w:手すり角度` * d:ADJ_BEYOND_X_DELTA / 4096 );
		eval( $i:発動誤差Ｘ座標 = `command abs $i:発動誤差Ｘ座標` );
		eval( $i:発動範囲最小Ｘ座標 = $i:可動範囲最小Ｘ座標 + $i:発動誤差Ｘ座標 );
		eval( $i:発動範囲最大Ｘ座標 = $i:可動範囲最大Ｘ座標 - $i:発動誤差Ｘ座標 );
		eval( $i:発動範囲最小Ｚ座標 = (( $i:手すり傾き１ * $i:発動範囲最小Ｘ座標 ) / 10000 ) + $i:手すり加算値１);
		eval( $i:発動範囲最大Ｚ座標 = (( $i:手すり傾き１ * $i:発動範囲最大Ｘ座標 ) / 10000 ) + $i:手すり加算値１);
		if ( $i:発動範囲最小Ｘ座標 > $i:発動範囲最大Ｘ座標 ) {
			eval( $i:EX_発動範囲Ｘ座標  = $i:発動範囲最小Ｘ座標 ) ;
			eval( $i:発動範囲最小Ｘ座標 = $i:発動範囲最大Ｘ座標 ) ;
			eval( $i:発動範囲最大Ｘ座標 = $i:EX_発動範囲Ｘ座標  ) ;
		}
		if ( $i:発動範囲最小Ｚ座標 > $i:発動範囲最大Ｚ座標 ) {
			eval( $i:EX_発動範囲Ｚ座標  = $i:発動範囲最小Ｚ座標 ) ;
			eval( $i:発動範囲最小Ｚ座標 = $i:発動範囲最大Ｚ座標 ) ;
			eval( $i:発動範囲最大Ｚ座標 = $i:EX_発動範囲Ｚ座標  ) ;
		}

		if (( $i:手すり交点Ｘ < $i:発動範囲最小Ｘ座標 ) || ( $i:手すり交点Ｘ > $i:発動範囲最大Ｘ座標 )) { 
			print 'beyond error : Out of range(X)'
			print $i:発動範囲最小Ｘ座標 $i:発動範囲最大Ｘ座標 $i:手すり交点Ｘ
			print $i:手すり加算値１ $i:手すり加算値２ $i:手すり傾き１ $i:手すり傾き２
			return 0 ;
		 }
		if (( $i:手すり交点Ｚ < $i:発動範囲最小Ｚ座標 ) || ( $i:手すり交点Ｚ > $i:発動範囲最大Ｚ座標 )) { 
			print 'beyond error : Out of range(Z)'
			print $i:発動範囲最小Ｚ座標 $i:発動範囲最大Ｚ座標 $i:手すり交点Ｚ
			return 0 ;
		 }
		eval( $w:デルタＸ座標２ = $i:手すり交点Ｘ - $:プレイヤーＸ位置  ) ;
		eval( $w:デルタＺ座標２ = $i:手すり交点Ｚ - $:プレイヤーＺ位置  ) ;
		eval( $w:垂線角度 = `command atan $w:デルタＺ座標２ $w:デルタＸ座標２`) ;
		eval( $i:線分距離 = (( $w:デルタＺ座標２ * 4096 ) / (`command cos $w:垂線角度`))) ;

		if (($i:線分距離 > $:発動距離) || ($i:線分距離 < (-$:発動距離))) { 
			print 'beyond error : long distance'
			return 0 ;
		}

        //可動範囲が狭すぎるときは、５００まで広げる */
        eval ( $i:可動範囲幅 = $i:可動範囲最大Ｘ座標 - $i:可動範囲最小Ｘ座標)
        if ( $i:可動範囲幅 < 500 ) {
			eval ( $i:可動範囲幅 = 500 - $i:可動範囲幅 ) 
			eval ( $i:可動範囲最小Ｘ座標 = $i:可動範囲最小Ｘ座標 - $i:可動範囲幅 / 2 )
			eval ( $i:可動範囲最大Ｘ座標 = $i:可動範囲最大Ｘ座標 + $i:可動範囲幅 / 2 )
		}
        eval ( $i:可動範囲幅 = $i:可動範囲最大Ｚ座標 - $i:可動範囲最小Ｚ座標)
        if ( $i:可動範囲幅 < 500 ) {
			eval ( $i:可動範囲幅 = 500 - $i:可動範囲幅 ) 
			eval ( $i:可動範囲最小Ｚ座標 = $i:可動範囲最小Ｚ座標 - $i:可動範囲幅 / 2 )
			eval ( $i:可動範囲最大Ｚ座標 = $i:可動範囲最大Ｚ座標 + $i:可動範囲幅 / 2 )
		}

		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );
		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ( `command 絶対角度差 $w:BY_プレイヤー方向 $w:垂線角度` < 384 ) {


				//ビヨンドモーション開始座標に移動
				eval( $w:EX_発動角度 = ( $w:垂線角度 + 2048 ) & 4095 );
				eval( $i:モーション発動Ｘ座標 = `command sin $w:EX_発動角度` * $i:ADJ_DIS_NOMAL / 4096 + $i:手すり交点Ｘ ) ;
				eval( $i:モーション発動Ｚ座標 = `command cos $w:EX_発動角度` * $i:ADJ_DIS_NOMAL / 4096 + $i:手すり交点Ｚ ) ;
				mesg プレイヤー d:PLAYER position2 0 ($i:モーション発動Ｘ座標)
				mesg プレイヤー d:PLAYER position2 2 ($i:モーション発動Ｚ座標)
				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					$w:垂線角度 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					(($w:垂線角度+2048) & 4095) \				// 開始時向き
					(($w:垂線角度+2048) & 4095) \				// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				command エルード移動範囲設定 \
					-x_range $i:可動範囲最小Ｘ座標 $i:可動範囲最大Ｘ座標 \
					-z_range $i:可動範囲最小Ｚ座標 $i:可動範囲最大Ｚ座標

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
				return $:トラップ名
			} else {
				print 'beyond error : out of angle'
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			eval( $w:逆垂線角度 = ( $w:垂線角度 + 2048 ) & 4095 ) ;
			if ( `command 絶対角度差 $w:BY_プレイヤー方向 $w:逆垂線角度` < 512 ) {


				//ビヨンドモーション開始座標に移動
				eval( $w:EX_発動角度 = ( $w:垂線角度 + 2048 ) & 4095 );
				eval( $i:モーション発動Ｘ座標 = `command sin $w:EX_発動角度` * $i:ADJ_DIS_BEHIND / 4096 + $i:手すり交点Ｘ ) ;
				eval( $i:モーション発動Ｚ座標 = `command cos $w:EX_発動角度` * $i:ADJ_DIS_BEHIND / 4096 + $i:手すり交点Ｚ ) ;
				mesg プレイヤー d:PLAYER position2 0 $i:モーション発動Ｘ座標
				mesg プレイヤー d:PLAYER position2 2 $i:モーション発動Ｚ座標
				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					$w:垂線角度 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					(($w:垂線角度+2048) & 4095) \				// 開始時向き
					(($w:垂線角度+2048) & 4095) \				// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				command エルード移動範囲設定 \
					-x_range $i:可動範囲最小Ｘ座標 $i:可動範囲最大Ｘ座標 \
					-z_range $i:可動範囲最小Ｚ座標 $i:可動範囲最大Ｚ座標

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
				
				return $:トラップ名
			} else {
				print 'beyond error : out of angle'
				return 0
			}
		} else {
			print 'beyond error : Invalid Mode'
			return 0
		}
	}

	proc ビヨンドハイ補正座標初期設定 {
		command プレイヤー状態取得 -d ;
		if ( $status & d:PFLAG_SNAKE ) {
			print 'snake beyond'
			eval( $i:ADJ_DIS_NOMAL  = d:SNA_ADJ_DIS_NOMAL  ) ;
			eval( $i:ADJ_DIS_BEHIND = d:SNA_ADJ_DIS_BEHIND ) ;
		} else {
			print 'raiden beyond'
			eval( $i:ADJ_DIS_NOMAL  = d:RAI_ADJ_DIS_NOMAL  ) ;
			eval( $i:ADJ_DIS_BEHIND = d:RAI_ADJ_DIS_BEHIND ) ;
		}
	}



// 以下旧バージョン
	proc ビヨンドモードハイ発動 $:トラップ名 $:初期方向 $:強制移動軸 $:強制移動座標１ $:強制移動座標２ $:発動軸 $:発動範囲対象座標 $:発動範囲小 $:発動範囲大 $:フラグ {
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );

		if (`command PlayerStatus -f 0`== 1) {
			if ( $:初期方向 < 256 && $w:BY_プレイヤー方向 >= 3072 ) {
				eval( $w:BY_プレイヤー方向 = $w:BY_プレイヤー方向 - 4096 );
			}

			if ((($w:BY_プレイヤー方向) > (($:初期方向)-256)) && (($w:BY_プレイヤー方向) < (($:初期方向)+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 $:強制移動軸 $:強制移動座標１
				if ($:発動範囲対象座標 < $:発動範囲小) {
					mesg プレイヤー d:PLAYER position2 $:発動軸 $:発動範囲小
				} else if ($:発動範囲対象座標 > $:発動範囲大) {
					mesg プレイヤー d:PLAYER position2 $:発動軸 $:発動範囲大
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					$:初期方向 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					($:初期方向+2048) \							// 開始時向き
					($:初期方向+2048) \							// 終了時向き
					$:フラグ \								// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			if ($:初期方向 > (3840-2048) && $w:BY_プレイヤー方向 <= 2048 ) {
				eval( $w:BY_プレイヤー方向 = $w:BY_プレイヤー方向 + 4096 );
			}
			
			if ((($w:BY_プレイヤー方向) > (($:初期方向)-256+2048)) && \
				(($w:BY_プレイヤー方向) < (($:初期方向)+256+2048))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 $:強制移動軸 $:強制移動座標２
				if ($:発動範囲対象座標 < $:発動範囲小) {
					mesg プレイヤー d:PLAYER position2 $:発動軸 $:発動範囲小
				} else if ($:発動範囲対象座標 > $:発動範囲大) {
					mesg プレイヤー d:PLAYER position2 $:発動軸 $:発動範囲大
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					$:初期方向 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					($:初期方向+2048) \							// 開始時向き
					($:初期方向+2048) \							// 終了時向き
					$:フラグ \									// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam \			// エルード中のダメージ
					d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
					d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
					d:モーションリスト:non_drop_pullup_end		// 懸垂終了

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}

// 落下寸前強制ビヨンド用proc
//-----------------------------------------------------
// 落下寸前エッジが垂直か平行方向にのびている場合
// $:強制移動軸：軸方向に強制移動 ： 0/1/2 ： Ｘ軸/Ｙ軸/Ｚ軸
// $:強制移動座標１：通常用の移動座標：	("手すりの座標"+"$i:ADJ_DIS_NOMAL")
//										例) (-1000+($i:ADJ_DIS_NOMAL))
// $:強制移動座標２：ビハインド用の移動座標
	proc 北向き強制ビヨンドハイ発動 $:トラップ名 $:手すり位置 $:プレイヤーＸ座標 $:トラップ左端 $:トラップ右端 $:フラグ {
		eval( $w:BY_プレイヤー方向 = $w:プレイヤー方向 & 4095 );
		if (`command PlayerStatus -f 0`== 1) {
			// 通常状態からのビヨンド
			if ((($w:BY_プレイヤー方向) > (2048-256)) && (($w:BY_プレイヤー方向) < (2048+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置+$i:ADJ_DIS_NOMAL)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:none_drop_start \	//モーション番号
					2048 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					0 \											// 開始時向き
					0 \											// 終了時向き
					$:フラグ \								// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam				// エルード中のダメージ



				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else if (`command PlayerStatus -f 4` == 1) {
			// ビハインド状態からのビヨンド
			if ($w:BY_プレイヤー方向 <= 2048 ) {
				eval( $w:BY_プレイヤー方向 = $w:BY_プレイヤー方向 + 4096 );
			}
			
			if ((($w:BY_プレイヤー方向) > (4096-256)) && (($w:BY_プレイヤー方向) < (4096+256))) {
				//ビヨンドモーション開始座標に移動
				mesg プレイヤー d:PLAYER position2 2 ($:手すり位置+$i:ADJ_DIS_BEHIND)
				if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ左端+d:ADJ_DIS)
				} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
					mesg プレイヤー d:PLAYER position2 0 ($:トラップ右端-d:ADJ_DIS)
				}

				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:sna_non_drop_start_behind_t \	//モーション番号
					2048 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003	//フラグ（壁床みない）

				//ビヨンド設定 
				command エルード設定 -values \
					d:モーションリスト:none_drop \				// 静止モーション
					d:モーションリスト:none_drop_r \			// 移動モーション右
					d:モーションリスト:none_drop_l \			// 移動モーション左
					d:モーションリスト:none_drop_end_fence \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					0 \											// 開始時向き
					0 \											// 終了時向き
					$:フラグ \								// フラグ
					d:モーションリスト:none_drop_end_wall \		// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）

//					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
//					d:モーションリスト:non_elude_fall \			// 落下中モーション

					d:モーションリスト:non_elude_fall_high \	// 落下モーション(高いところ)
					d:モーションリスト:non_elude_fall_high \	// 落下中モーション(高いところ)

					d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
					d:モーションリスト:non_elude_fall_out \		// 落下して死亡
					d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
					d:モーションリスト:non_drop_dam				// エルード中のダメージ

				mesg プレイヤー d:PLAYER beyond_trap $:トラップ名

				return $:トラップ名
			} else {
				return 0
			}
		} else {
			return 0
		}
	}




#else

print 'beyond_high.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif

