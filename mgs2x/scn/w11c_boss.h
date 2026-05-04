/*
	w11c_boss.gcl                     
	    フォーチュン戦

	2001/01/18 H.Yoshiike         
	$Id: w11c_boss.h,v 1.61 2002/08/09 10:50:10 usr04098 Exp $                      

	
*/


#include "../source/user/morita/fort/include/fort.mh"
#include "cap_resource.h"

// フォーチュン戦壊れ物専用アタリ
proc 壊れ物アタリ設定 {
	#if d:DEBUG_PRINT
		print 'w11c_bk_objhzd_set'
	#endif

	// フォークリフトデフォルト位置
	command 追加ＨＺＸグループ登録 -h 0 -a 1
	command 追加ＨＺＸグループ登録 -h 0 -a 2
	command 追加ＨＺＸグループ登録 -h 0 -a 29

	// 中央右ドラム缶
	command 追加ＨＺＸグループ登録 -h 0 -a 5
	// 下右ドラム缶
	command 追加ＨＺＸグループ登録 -h 0 -a 6

	// 下段前列一番左の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 7
	// 下段前列左から２番目の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 8
	// 下段前列中央の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 9
	// 下段前列右から２番目の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 10
	// 下段前列一番右の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 11
	// 下段後列一番右の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 12
	// 下段後列一番左の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 13

	// 上段前列一番左の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 14
	// 上段前列中央の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 15
	// 上段前列一番右の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 16
	// 上段後列一番右の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 17
	// 上段後列一番左の箱
	command 追加ＨＺＸグループ登録 -h 0 -a 18

	// 2m箱
	command 追加ＨＺＸグループ登録 -h 0 -a 19

}

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
//		-pos   0,-43000,-8200 \
//		-dir   0,2048,0 \
		-pos   -782,-43000,-6491 \
		-dir   0,4000,0 \
		-proc  弾避けのカット \
		    グレネード避けのカット \
		    フォークリフトのカット \
		    昇降機スイッチのカット \
		    昇降機降下 \
			フォーチュン戦終了デモスタート \
		-voice t:vc060101 t:vc060102 t:vc060103 t:vc060104 t:vc060105 t:vc060106

	

	// 欧州版では言語選択があるためにこのコマンドを使う(:01とかで定義プロックを変えられる)
	command 字幕言語設定
	@ＳＥ字幕リソース
}

proc フォーチュン戦終了デモスタート 
{
	#if d:DEBUG_PRINT
		print 'fort_demo_start'
	#endif

	command 無線設定 -reset
	command パッド操作 -release
	command プレイヤー無敵セット
	@rt_P030_01_R01フォーチュン戦中無線無線デモ１
}

proc フォーチュン戦オイル燃え
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_fire'
	#endif

	chara フォーチュン戦オイル燃え 初期燃え \
		-life (30*60) \
		-time 120 \
		-lift 6 \
		-pos  -3000,-44990, 3200
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
	chara カメラ設定 シナリオデモカメラ  \
		-c 1 \	
		-p $i:Ｘ座標 $i:Ｙ座標 $i:Ｚ座標 \
		-t $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数 \
		-f 1490 \
		-a 158 \
		-i 0 0 0 0 \
		-s 1

	chara delay ディレイ -time (60*2) -exec {
		@カットインデモ終了
		chara カメラ設定 シナリオデモカメラ -s -1
	}

	eval( $f:rfp_w11c_フォーチュン攻撃それそれデモ見た = 1 );
}

proc グレネード避けのカット
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_avoiding_bombcut'
	#endif

	@カットインデモ開始
	command ゲットフォーチュングレネード座標 $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数
	chara カメラ設定 シナリオデモカメラ  \
		-c 1 \	
		-p $i:Ｘ座標変数 ($i:Ｙ座標変数+800) ($i:Ｚ座標変数+200) \
		-t $i:Ｘ座標変数 $i:Ｙ座標変数 $i:Ｚ座標変数 \
		-f 1490 \
		-a 158 \
		-i 0 0 0 0 \
		-s 1

	chara delay ディレイ -time (60*3) -exec {
		@カットインデモ終了
		chara カメラ設定 シナリオデモカメラ -s -1
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

	@カットインデモ開始
	chara カメラ設定 シナリオデモカメラ \
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
		chara カメラ設定  シナリオデモカメラ -s -0
	}
}


proc 昇降機降下
{
	#if d:DEBUG_PRINT
	        print 'w11c_boss_elevdown'
	#endif

	mesg フォーチュン戦昇降機 昇降機 下がる
	mesg フォーチュン戦昇降機 昇降機 モーション再生
}


//
//  デモ用プロック
//
//
proc デモ用基本設定開始 {
	command 無線設定 -reset
	command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off -disable_menu off

	@特殊装備解除
	mesg シネマスクリーン シネマ フェードアウト 0
}

proc デモ用基本設定終了 {
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on -disable_menu on
	mesg シネマスクリーン シネマ フェードイン 0
}

proc カットインデモ開始
{
	command シナリオデモ開始
	@デモ用基本設定開始
	command パッド操作 -release
	mesg ボスフォーチュン フォーチュン デモ開始
	command プレイヤー無敵セット
}

proc カットインデモ終了
{
	command シナリオデモ終了
	@デモ用基本設定終了
	command パッド操作 -cancel
	mesg ボスフォーチュン フォーチュン デモ終了
	command プレイヤー無敵解除
}
