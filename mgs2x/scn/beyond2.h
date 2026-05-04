/*
	beyond2.h                     
	    片手ぶらさがりビヨンドモード用proc

	2001/04/02 Y.Matsuhana         
	$Id: beyond2.h,v 1.13 2001/08/27 12:28:34 usr00725 Exp $                      

	
*/

// ファイルが二重呼びされたときの対処
#ifndef d:BEYOND2_H
#define BEYOND2_H	1

/*
※このファイルをincludeする際は、list以下でステージ固有のmlsファイルを
作成した後、make mhにて次の例のようなモーションリストを作成してください。

enum モーションリスト {
	non_drop1hand_start,		// 片手エルード開始
	non_drop1hand_idle,			// 片手エルード静止
	non_drop1hand_end			// 片手エルード戻り
};
*/

// ビヨンド関係
#define		ADJ_DIS		450		// トラップの端に入った場合にずらす距離
// スネーク用
#define		SNA_ADJ_DIS_NOMAL	550		// 通常で入ったときの手すりとの距離
#define		SNA_ADJ_DIS_BEHIND	300		// ビハインドで入ったときの手すりとの距離
// ライデン用
#define		RAI_ADJ_DIS_NOMAL	512		// 通常で入ったときの手すりとの距離
#define		RAI_ADJ_DIS_BEHIND	492		// ビハインドで入ったときの手すりとの距離

#define		ADJ_BEYOND_X_DELTA	50	// ビヨンドになるモーション発動前と発動後のＸ軸座標のずれ
#define		ONEHAND_ADJ_DIS		110	// 片手エルード開始のエッジとモーション発動位置の平面補正
#define		ONEHAND_ADJ_HIGH	995	// 片手エルード開始の床と腰位置の高さ補正

// 片手ぶら下りビヨンド用proc
//-----------------------------------------------------
// 手すりが垂直か平行方向にのびている場合
// $:強制移動軸：軸方向に強制移動 ： 0/1/2 ： Ｘ軸/Ｙ軸/Ｚ軸
// $:強制移動座標１：通常用の移動座標：	("手すりの座標"+"$i:ADJ_DIS_NOMAL")
//										例) (-1000+($i:ADJ_DIS_NOMAL))
// $:強制移動座標２：ビハインド用の移動座標

	proc 北向き片手ビヨンド発動 $:トラップ名 $:手すり位置 $:プレイヤーＸ座標 $:トラップ左端 $:トラップ右端 $:床の高さ $:フラグ {
		// 通常状態からのビヨンド
		//ビヨンドモーション開始座標に移動
		if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				2048 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:トラップ左端+d:ADJ_DIS) \		// 開始Ｘ座標
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				2048 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:トラップ右端-d:ADJ_DIS) \		// 開始Ｘ座標
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				2048 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				$i:プレイヤー位置Ｘ \					// 開始Ｘ座標
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		}

		//ビヨンド設定 
		command エルード設定 -values \
			d:モーションリスト:non_drop1hand_idle \	// 静止モーション
			0 \											// 移動モーション右
			0 \											// 移動モーション左
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション
			6 \											// 床チェックする関節（ここでは右腕）
			0 \											// 開始時向き
			0 \											// 終了時向き
			($:フラグ | d:BY_NO_MOVE_LR | d:BY_DEC_GRIP_FAST | d:BY_NO_RETURNSTAMP) \	// フラグ 左右移動なし戻り時踏み攻撃なし
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション（壁有り
			14 \										// 戻り時に壁チェックする関節番号
			d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
			d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
			d:モーションリスト:non_elude_fall \			// 落下中モーション
			d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
			d:モーションリスト:non_elude_fall_out \		// 落下して死亡
			d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
			d:モーションリスト:non_drop_dam \			// エルード中のダメージ
			d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
			d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
			d:モーションリスト:non_drop_pullup_end		// 懸垂終了
		mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
		return $:トラップ名
	}

	proc 南向き片手ビヨンド発動 $:トラップ名 $:手すり位置 $:プレイヤーＸ座標 $:トラップ左端 $:トラップ右端 $:床の高さ $:フラグ {
		// 通常状態からのビヨンド
		//ビヨンドモーション開始座標に移動
		if ($:プレイヤーＸ座標 < $:トラップ左端+d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				0 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:トラップ左端+d:ADJ_DIS) \		// 開始Ｘ座標
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else if ($:プレイヤーＸ座標 > $:トラップ右端-d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				0 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:トラップ右端-d:ADJ_DIS) \		// 開始Ｘ座標
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				0 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				$i:プレイヤー位置Ｘ \					// 開始Ｘ座標
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		}

		//ビヨンド設定 
		command エルード設定 -values \
			d:モーションリスト:non_drop1hand_idle \	// 静止モーション
			0 \											// 移動モーション右
			0 \											// 移動モーション左
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション
			6 \											// 床チェックする関節（ここでは右腕）
			2048 \											// 開始時向き
			2048 \											// 終了時向き
			($:フラグ | d:BY_NO_MOVE_LR | d:BY_DEC_GRIP_FAST | d:BY_NO_RETURNSTAMP) \	// フラグ 左右移動なし戻り時踏み攻撃なし
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション（壁有り
			14 \										// 戻り時に壁チェックする関節番号
			d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
			d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
			d:モーションリスト:non_elude_fall \			// 落下中モーション
			d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
			d:モーションリスト:non_elude_fall_out \		// 落下して死亡
			d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
			d:モーションリスト:non_drop_dam \			// エルード中のダメージ
			d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
			d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
			d:モーションリスト:non_drop_pullup_end		// 懸垂終了
		mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
		return $:トラップ名
	}

	proc 東向き片手ビヨンド発動 $:トラップ名 $:手すり位置 $:プレイヤーＺ座標 $:トラップ左端 $:トラップ右端 $:床の高さ $:フラグ {
		// 通常状態からのビヨンド
		//ビヨンドモーション開始座標に移動
		if ($:プレイヤーＺ座標 < $:トラップ左端+d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				1024 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				($:トラップ左端+d:ADJ_DIS) \		// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else if ($:プレイヤーＺ座標 > $:トラップ右端-d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				1024 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				($:トラップ右端-d:ADJ_DIS) \		// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				1024 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置-d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				$i:プレイヤー位置Ｚ \					// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		}

		//ビヨンド設定 
		command エルード設定 -values \
			d:モーションリスト:non_drop1hand_idle \	// 静止モーション
			0 \											// 移動モーション右
			0 \											// 移動モーション左
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション
			6 \											// 床チェックする関節（ここでは右腕）
			3072 \											// 開始時向き
			3072 \											// 終了時向き
			($:フラグ | d:BY_NO_MOVE_LR | d:BY_DEC_GRIP_FAST | d:BY_NO_RETURNSTAMP) \	// フラグ 左右移動なし戻り時踏み攻撃なし
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション（壁有り
			14 \										// 戻り時に壁チェックする関節番号
			d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
			d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
			d:モーションリスト:non_elude_fall \			// 落下中モーション
			d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
			d:モーションリスト:non_elude_fall_out \		// 落下して死亡
			d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
			d:モーションリスト:non_drop_dam \			// エルード中のダメージ
			d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
			d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
			d:モーションリスト:non_drop_pullup_end		// 懸垂終了
		mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
		return $:トラップ名
	}

	proc 西向き片手ビヨンド発動 $:トラップ名 $:手すり位置 $:プレイヤーＺ座標 $:トラップ左端 $:トラップ右端 $:床の高さ $:フラグ {
		// 通常状態からのビヨンド
		//ビヨンドモーション開始座標に移動
		if ($:プレイヤーＺ座標 < $:トラップ左端+d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				3072 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				($:トラップ左端+d:ADJ_DIS) \		// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else if ($:プレイヤーＺ座標 > $:トラップ右端-d:ADJ_DIS) {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				3072 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				($:トラップ右端-d:ADJ_DIS) \		// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		} else {
			mesg プレイヤー d:PLAYER motion \
				d:モーションリスト:non_drop1hand_start \	//モーション番号
				3072 \	//初期方向
				-1 \	//終了方向
				4 \	//終了姿勢（４はビヨンド）
				0x00000403 \	//フラグ（壁床みない）
				($:手すり位置+d:ONEHAND_ADJ_DIS) \	// 開始Ｘ座標
				$i:プレイヤー位置Ｚ \					// 開始Ｚ座標
				($:床の高さ+d:ONEHAND_ADJ_HIGH)		// 開始Ｙ座標
		}

		//ビヨンド設定 
		command エルード設定 -values \
			d:モーションリスト:non_drop1hand_idle \	// 静止モーション
			0 \											// 移動モーション右
			0 \											// 移動モーション左
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション
			6 \											// 床チェックする関節（ここでは右腕）
			1024 \											// 開始時向き
			1024 \											// 終了時向き
			($:フラグ | d:BY_NO_MOVE_LR | d:BY_DEC_GRIP_FAST | d:BY_NO_RETURNSTAMP) \	// フラグ 左右移動なし戻り時踏み攻撃なし
			d:モーションリスト:non_drop1hand_end \	// 戻りモーション（壁有り
			14 \										// 戻り時に壁チェックする関節番号
			d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
			d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
			d:モーションリスト:non_elude_fall \			// 落下中モーション
			d:モーションリスト:non_elude_fall_damage \	// 落下してダメージ
			d:モーションリスト:non_elude_fall_out \		// 落下して死亡
			d:モーションリスト:non_elude_fall_normal \	// 落下しても無事着地
			d:モーションリスト:non_drop_dam \			// エルード中のダメージ
			d:モーションリスト:non_drop_pullup_start \	// 懸垂開始
			d:モーションリスト:non_drop_pullup_idle \	// 懸垂静止
			d:モーションリスト:non_drop_pullup_end		// 懸垂終了
		mesg プレイヤー d:PLAYER beyond_trap $:トラップ名
		return $:トラップ名
	}

// 斜めビハインドPROC
// ----------------------------------------------------
	proc 斜め片手ビヨンド発動 $:トラップ名 $:プレイヤーＸ位置 $:プレイヤーＺ位置 $:手すりＡ点Ｘ座標 $:手すりＡ点Ｚ座標 $:手すりＢ点Ｘ座標 $:手すりＢ点Ｚ座標 $:フラグ {
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
		eval( $i:手すり傾き差分 = $i:手すり傾き１ - $i:手すり傾き２ )
		if ( $i:手すり傾き差分 == 0 ) {
			// ゼロ除算対策
			print 'Warning : Anti ZERO divid proc (1)' $i:手すり傾き１ $i:手すり傾き２
			eval( $i:手すり傾き差分 = 1 ) ;
		}
		eval( $i:手すり交点Ｘ = (((( $i:手すり加算値２ - $i:手すり加算値１ ) * 10000 ) / ( $i:手すり傾き差分 )))) ;
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
			print 'beyond error : 0 Out of range(X)'
			print $i:発動範囲最小Ｘ座標 $i:発動範囲最大Ｘ座標 $i:手すり交点Ｘ
			print $i:手すり加算値１ $i:手すり加算値２ $i:手すり傾き１ $i:手すり傾き２
			return 0 ;
		 }
		if (( $i:手すり交点Ｚ < $i:発動範囲最小Ｚ座標 ) || ( $i:手すり交点Ｚ > $i:発動範囲最大Ｚ座標 )) {
			print 'beyond error : 1 Out of range(Z)'
			print $i:発動範囲最小Ｚ座標 $i:発動範囲最大Ｚ座標 $i:手すり交点Ｚ
			return 0 ;
		 }
		eval( $w:デルタＸ座標２ = $i:手すり交点Ｘ - $:プレイヤーＸ位置  ) ;
		eval( $w:デルタＺ座標２ = $i:手すり交点Ｚ - $:プレイヤーＺ位置  ) ;
		eval( $w:垂線角度 = `command atan $w:デルタＺ座標２ $w:デルタＸ座標２`) ;
		eval( $w:垂線角度コサイン = `command cos $w:垂線角度`) ;
		if ( $w:垂線角度コサイン == 0 ) {
			// ゼロ除算対策
			print 'Warning : Anti ZERO divid proc (2)' $w:垂線角度
			eval( $w:垂線角度コサイン = 1 ) ;
		}
		eval( $i:線分距離 = (( $w:デルタＺ座標２ * 4096 ) / $w:垂線角度コサイン )) ;

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
			if ( `command 絶対角度差 $w:BY_プレイヤー方向 $w:垂線角度` < 512 ) {


				//ビヨンドモーション開始座標に移動
				eval( $w:EX_発動角度 = ( $w:垂線角度 + 2048 ) & 4095 );
				eval( $i:モーション発動Ｘ座標 = `command sin $w:EX_発動角度` * d:ONEHAND_ADJ_DIS / 4096 + $i:手すり交点Ｘ ) ;
				eval( $i:モーション発動Ｚ座標 = `command cos $w:EX_発動角度` * d:ONEHAND_ADJ_DIS / 4096 + $i:手すり交点Ｚ ) ;
				mesg プレイヤー d:PLAYER motion \
					d:モーションリスト:non_drop1hand_start \	//モーション番号
					$w:垂線角度 \	//初期方向
					-1 \	//終了方向
					4 \	//終了姿勢（４はビヨンド）
					0x00000003 \	//フラグ（壁床みない）
					($i:モーション発動Ｘ座標) \
					($i:モーション発動Ｚ座標)

				//ビヨンド設定
				command エルード設定 -values \
					d:モーションリスト:non_drop1hand_idle \	// 静止モーション
					0 \											// 移動モーション右
					0 \											// 移動モーション左
					d:モーションリスト:non_drop1hand_end \	// 戻りモーション
					6 \											// 床チェックする関節（ここでは右腕）
					(($w:垂線角度+2048) & 4095) \				// 開始時向き
					(($w:垂線角度+2048) & 4095) \				// 終了時向き
					($:フラグ | d:BY_NO_MOVE_LR | d:BY_DEC_GRIP_FAST | d:BY_NO_RETURNSTAMP) \	// フラグ 左右移動なし戻り時踏み攻撃なし
					d:モーションリスト:non_drop1hand_end \	// 戻りモーション（壁有り
					14 \										// 戻り時に壁チェックする関節番号
					d:モーションリスト:none_drop_idle_tired \	// 静止モーション（疲れ）
					d:モーションリスト:non_elude_fall_start \	// 落下モーション(落ちはじめ)
					d:モーションリスト:non_elude_fall \			// 落下中モーション
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
				eval ( $s:エルード落下トラップ = $:トラップ名 )
				return $:トラップ名
			} else {
				print 'beyond error : 3 out of angle'
				return 0
			}
		} else {
			print 'beyond error : 5 Invalid Mode'
			return 0
		}
	}



#else

print 'beyond2.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif

