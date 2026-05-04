/*
	w11c_boss.gcl                     
	    フォーチュン戦

	2001/01/18 H.Yoshiike         
	$Id: e11c_boss.h,v 1.8 2001/05/02 09:22:19 usr03682 Exp $                      

	
*/


#include "../source/user/morita/fort/include/fort.mh"

//
// ここからボス戦 常駐キャラ
//
//
proc ボスフォーチュン  
{
	#if d:DEBUG_PRINT
		print 'w11c_boss'
	#endif
	chara ボスフォーチュン フォーチュン \
		-model for_def_sh_mt for_coat_mh_mt \
		-pos   0,-43000,-8200 \
		-dir   0,2048,0 \
		-proc  弾避けのカット \
		    グレネード避けのカット \
		    フォークリフトのカット \
		    昇降機スイッチのカット \
		    昇降機降下 \
		-voice t:vc060101 t:vc060102 t:vc060103 t:vc060104 t:vc060105 t:vc060106
}

proc フォーチュン戦オイル燃え
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_fire'
	#endif

	chara フォーチュン戦オイル燃え 初期燃え \
		-life (30*60) \
		-time 1 \
		-lift 6 \
		-pos  -3000,-44990, 3200
}

proc 昇降機
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_elevator'
	#endif

	chara フォーチュン戦昇降機 昇降機 \
		-e_model  elv_1 \
		-e_top    0, 45000, 0 \
		-e_bottom 0,     0, 0 \
		-e_rot    0,     0, 0 \
		-kms_model sna_def        \
		-evm_model vmp_coat_mh_mt \
		-motion    snake         \
		-rotation  0,1024,0 \
		-position  0,-45000,-12000
	mesg フォーチュン戦昇降機 昇降機 モーション停止
}


proc 共通シネマスクリーンセット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_cinemascreen'
	#endif
	chara シネマスクリーン シネマ \
		-top d:CINEMA_BELT_UPPER \
		-bottom d:CINEMA_BELT_LOWER \
		-ftime 30	//デフォルト値（ゼロは駄目）
}


//
//  立ち上がりプロック
//
//
proc 昇降機到着
{
	//シャッターを開けるモーション
	// chara 昇降機 用のプロック
	mesg フォーチュン戦プットモーション 昇降機シャッター 再生開始 0 0
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 緑に点灯
	command セットサウンドコード -c d:se_code:SD_A_EVDMOV01
}

proc 昇降機出発
{
	//シャッターを開けるモーション
	// chara 昇降機 用のプロック
	mesg フォーチュン戦プットモーション 昇降機シャッター 再生開始 2 0
	mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 赤に点灯
	command セットサウンドコード -c d:se_code:SD_A_EVDMOV01
}

proc 昇降機扉開き終り
{
	//シャッターを開けきった時の音
	command セットサウンドコード -c d:se_code:SD_A_EVDOPN01
}


proc 弾避けのカット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_avoiding_firecut'
	#endif

	@カットインデモ開始
	command ゲットフォーチュン座標 $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数
	command ゲットフォーチュン弾座標 $i:Ｘ座標 $i:Ｙ座標 $i:Ｚ座標
	chara カメラ設定 デモカメラ  \
		-c 1 \	
		-p $i:Ｘ座標 $i:Ｙ座標 $i:Ｚ座標 \
		-t $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数 \
		-f 1490 \
		-a 158 \
		-i 0 0 0 0 \
		-s 1

	chara delay ディレイ -time (60*2) -exec {
		@カットインデモ終了
		chara カメラ設定 デモカメラ -s -1
	}
}

proc グレネード避けのカット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_avoiding_bombcut'
	#endif

	@カットインデモ開始
	command ゲットフォーチュングレネード座標 $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数
	chara カメラ設定 デモカメラ  \
		-c 1 \	
		-p $i:Ｘ座標変数 ($i:Ｙ座標変数+800) ($i:Ｚ座標変数+200) \
		-t $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数 \
		-f 1490 \
		-a 158 \
		-i 0 0 0 0 \
		-s 1

	chara delay ディレイ -time (60*3) -exec {
		@カットインデモ終了
		chara カメラ設定 デモカメラ -s -1
	}
}

proc フォークリフトのカット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_forklift_cut'
	#endif
    
}


proc 昇降機スイッチのカット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_elevator_cut'
	#endif
/*
	@カットインデモ開始
	chara カメラ設定 デモカメラ \
		-c 1 \	
		-p 3199,-42465,-5915 -t 1826,-43206,-8493 \
		-r 162,2367,0 -f 3013 \
		-a 200 \
		-s 1

	chara delay ディレイ -time (60*1) -exec {
		mesg フォーチュン戦昇降機スイッチ 昇降機スイッチ 下ボタンを押す
	}

	chara delay ディレイ -time (60*4) -exec {
		@カットインデモ終了
		chara カメラ設定  デモカメラ -s -0
	}
*/
}


proc 昇降機降下
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_elevdown'
	#endif
/*
	mesg フォーチュン戦昇降機 昇降機 下がる
	mesg フォーチュン戦昇降機 昇降機 モーション再生
*/
}



//
//  デモ用プロック
//
//
proc カットインデモ開始
{
	//メニュー＆レーダーを消す
	command メニュー設定 \
		-menu off \ // 武器・アイテムメニューのオフ
		-gage off \ // ゲージのオフ
		-radar off \ // レーダーのオフ
		-subwin off  // 子画面のオフ

	mesg ボスフォーチュン フォーチュン デモ開始
	command パッド操作 -release
	command プレイヤー無敵セット
	mesg シネマスクリーン シネマ フェードアウト 0
}

proc カットインデモ終了
{
	//メニュー＆レーダーを表示
	command メニュー設定 \
		-menu on \ //武器・アイテムメニューのオン
		-gage on \ //ゲージのオン
		-radar on \ //レーダーのオン
		-subwin on  //子画面のオン

	mesg ボスフォーチュン フォーチュン デモ終了
	command パッド操作 -cancel
	command プレイヤー無敵解除
	mesg シネマスクリーン シネマ フェードイン 0
}
