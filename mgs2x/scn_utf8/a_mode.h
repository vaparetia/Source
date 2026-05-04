/*
	a_mode.h
		アナザーミッション用モード設定ファイル

	2002/02/01 H.Yoshiike
	$Id: a_mode.h,v 1.61 2002/08/16 01:38:55 usr03682 Exp $

	※ 各モードごとに設定ファイルを用意し、変数で呼び分ける。
	※ このファイルは皆で共有する。
	※ stdch.hの下でincludeする
*/

//------------------------------------------------
// includeする共通ファイル
//------------------------------------------------
#include "a_vardef.h"		// アナザーミッション用defineファイル
#include "a_stdproc.h"		// アナザーミッション用共通プロック

#ifndef d:STAGE_TANKER
	#include "dcam_tanker.h"	// タンカー編の場合はすでにインクルードされている
#endif

//------------------------------------------------
// includeする各ステージ別のファイル
//------------------------------------------------
#ifdef d:STAGE_W00A
	#include "a00a_bomb.h"
	#include "a00a_photo.h"
	#include "a00a_tale.h"

#elseifdef d:STAGE_W00B
	#include "a00b_tale.h"

#elseifdef d:STAGE_W01A
	#include "a01a_bomb.h"
	#include "a01a_elim.h"
	#include "a01a_photo.h"
	#include "a01a_tale.h"

#elseifdef d:STAGE_W01F
	#include "a01f_bomb.h"
	#include "a01f_photo.h"
	#include "a01f_tale.h"

#elseifdef d:STAGE_W01B
	#include "a01b_bomb.h"
	#include "a01b_photo.h"
	#include "a01b_hold.h"
	#include "a01b_tale.h"

#elseifdef d:STAGE_W01C
	#include "a01c_bomb.h"
	#include "a01c_tale.h"


#elseifdef d:STAGE_W01D
	#include "a01d_bomb.h"
	#include "a01d_photo.h"	//多分使用しない
	#include "a01d_tale.h"

#elseifdef d:STAGE_W01E
	#include "a01e_bomb.h"
	#include "a01e_tale.h"

#elseifdef d:STAGE_W00C
	#include "a00c_bomb.h"
	#include "a00c_photo.h"
	#include "a00c_tale.h"

#elseifdef d:STAGE_W02A
	#include "a02a_bomb.h"
	#include "a02a_elim.h"
	#include "a02a_photo.h"
	#include "a02a_tale.h"

#elseifdef d:STAGE_W03A
	#include "a03a_bomb.h"
	#include "a03a_photo.h"
	#include "a03a_tale.h"

#elseifdef d:STAGE_W03B
	#include "a03b_bomb.h"
	#include "a03b_tale.h"

#elseifdef d:STAGE_W04A
	#include "a04a_bomb.h"

#elseifdef d:STAGE_W04B
	#include "a04b_bomb.h"

#elseifdef d:STAGE_W04C
	#include "a04c_bomb.h"

#elseifdef d:STAGE_W12B
#include "a12b_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a12b_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W12C
#include "a12a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a12a_hold.h"		// 武装解除ミッションLv2用設定ファイル
#include "a12a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W13A || d:STAGE_W13B
#include "a13_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a13_hold.h"		// 武装解除ミッション用設定ファイル
#include "a13_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W14A
#include "a14a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a14a_hold.h"		// 武装解除ミッションLv5 用設定ファイル
#include "a14a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W15A || d:STAGE_W15B
#include "a15_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a15_hold.h"		// 武装解除ミッション用Lv3設定ファイル
#include "a15_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W16A
#include "a16a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a16a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W17A
#include "a17a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a17a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W18A
#include "a18a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a18a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W19A
//#include "a19a_bomb.h"		// 爆弾解体ミッション用設定ファイル( 爆弾解体は無いはず )
#include "a19a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a19a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W20A
#include "a20a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a20a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a20a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W20B
#include "a20b_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a20b_hold.h"		// 武装解除ミッション用設定ファイル
#include "a20b_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W20C
#include "a20c_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W21A || d:STAGE_W21B
#include "a21_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a21_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W22A
#include "a22a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a22a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a22a_hold.h"		// 武装解除ミッション用設定ファイル
#include "a22a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W23A || d:STAGE_W23B
#include "a23_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a23_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a23_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W24A
#include "a24a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a24a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a24a_photo.h"		// 写真撮影ミッション用設定ファイル
#include "a24a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W24B
#include "a24b_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a24b_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W24C
#include "a24c_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a24c_photo.h"		// 写真撮影ミッション用設定ファイル
#include "a24c_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W24D
#include "a24d_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a24d_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a24d_hold.h"		// 武装解除ミッション用設定ファイル
#include "a24d_tale.h"		// スネークテイルズ用設定ファイル
#include "a24d_photo.h"		// 写真撮影ミッション用

#elseifdef d:STAGE_W25A
#include "a25a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W25D
#include "a25d_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a25d_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W28A
#include "a28a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a28a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W31A || d:STAGE_W31D
#include "a31a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a31a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a31a_hold.h"		// 武装解除ミッション用設定ファイル
#include "a31a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W31C
#include "a31c_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W41A
#include "a41a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a41a_photo.h"		// 写真撮影ミッション用設定ファイル

#elseifdef d:STAGE_W42A
#include "a42a_bomb.h"		// 爆弾解体ミッション用設定ファイル
#include "a42a_elim.h"		// 敵兵排除ミッション用設定ファイル
#include "a42a_hold.h"		// 武装解除ミッション用設定ファイル

#elseifdef d:STAGE_W43A
#include "a43a_bomb.h"		// 爆弾解体ミッション用設定ファイル

#elseifdef d:STAGE_W44A
#include "a44a_bomb.h"		// 爆弾解体ミッション用設定ファイル

#elseifdef d:STAGE_W45A
#include "a45a_bomb.h"		// 爆弾解体ミッション用設定ファイル

#elseifdef d:STAGE_W46A
#include "a46a_tale.h"		// スネークテイルズ用設定ファイル

#elseifdef d:STAGE_W61A
#include "a61a_tale.h"		// スネークテイルズ用設定ファイル

#endif

//------------------------------------------------



//------------------------------------------------
// chara マップの前に呼ばれるproc
//------------------------------------------------
proc アナザー用マップ定義前設定 {
	#if d:DEBUG_PRINT
		print 'another set1 アナザー用マップ定義前設定'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用マップ定義前設定
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用マップ定義前設定
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用マップ定義前設定
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				#ifndef d:STAGE_TANKER
					@タンカーデジカメ写真判定初期化	// タンカー編の場合はすでに呼ばれている
				#endif
				@写真撮影ミッション用マップ定義前設定
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用マップ定義前設定
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) アナザー用マップ定義前設定'
		}
	}
}



//---------------------------------------------------
// chara マップ設定、常駐キャラ設定の後に呼ばれるproc
// ※ 各プロック内でcommand マップ設定をしないと全マップに登録されます。
//---------------------------------------------------
proc アナザー用マップ定義後設定 {
	#if d:DEBUG_PRINT
		print 'another set2 アナザー用マップ定義後設定'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用マップ定義後設定
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用マップ定義後設定
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用マップ定義後設定
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用マップ定義後設定
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用マップ定義後設定
			#endif
		}
		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) アナザー用マップ定義後設定'
		}
	}
}

proc アナザー用最終設定 {
	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用最終設定
			#endif
		}
	}
}


//------------------------------------------------------------
// クリア時に呼ばれるプロック
//------------------------------------------------------------
proc クリア時処理 {
	#if d:DEBUG_PRINT
		print 'clear'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用クリア時処理
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用クリア時処理
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用クリア時処理
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用クリア時処理
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用クリア時処理
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) クリア時処理'
		}
	}
}


//------------------------------------------------
// 監視カメラ用のプロック(プロック名固定のためここに記述)
//------------------------------------------------
proc 監視カメラ破壊時処理 $:名前 $:フラグ {
	#if d:DEBUG_PRINT
		print 'svc_camera_broken'
		print 'name='$:名前
		print 'flag='$:フラグ
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用監視カメラ破壊時処理 $:名前 $:フラグ
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用監視カメラ破壊時処理 $:名前 $:フラグ
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用監視カメラ破壊時処理 $:名前 $:フラグ
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用監視カメラ破壊時処理 $:名前 $:フラグ
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用監視カメラ破壊時処理 $:名前 $:フラグ
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) 監視カメラ破壊時処理'
		}
	}
}



//------------------------------------------------
// サイファー用のプロック(プロック名固定のためここに記述)
//------------------------------------------------
proc サイファー破壊 $:サイファー名 {

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用サイファー破壊 $:サイファー名
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用サイファー破壊 $:サイファー名
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用サイファー破壊 $:サイファー名
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用サイファー破壊 $:サイファー名
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用サイファー破壊 $:サイファー名
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) サイファー破壊'
		}
	}

	#ifdef d:DEBUG_PRINT
		print $:サイファー名
		print $w:グローバルロード回数
	#endif
}



//------------------------------------------------
// アイテム用のプロック(プロック名固定のためここに記述)
//------------------------------------------------
proc アイテム取得時番号設定 $:ボックス名 {
	#if d:DEBUG_PRINT
		print 'item_no_set'
	#endif

	switch ( $b:ミッション番号 ) {
		case ( d:MISSION:爆弾解体 ) {
			#ifdef d:MISSION_BOMB
				@爆弾解体ミッション用アイテム取得時番号設定 $:ボックス名
			#endif
		}
		case ( d:MISSION:敵兵排除 ) {
			#ifdef d:MISSION_ELIM
				@敵兵排除ミッション用アイテム取得時番号設定 $:ボックス名
			#endif
		}
		case ( d:MISSION:武装解除 ) {
			#ifdef d:MISSION_HOLD
				@武装解除ミッション用アイテム取得時番号設定 $:ボックス名
			#endif
		}
		case ( d:MISSION:写真撮影 ) {
			#ifdef d:MISSION_PHOTO
				@写真撮影ミッション用アイテム取得時番号設定 $:ボックス名
			#endif
		}
		case ( d:MISSION:スネークテイルズ ) {
			#ifdef d:MISSION_TALE
				@スネークテイルズ用アイテム取得時番号設定 $:ボックス名
			#endif
		}

		default {
			#if d:DEBUG_PRINT
				print 'WARNING!!!!!不正な値が代入されています。$b:ミッション番号='$b:ミッション番号
			#endif

			command assert ( d:FALSE ) 'mission no error  (scenario err) アイテム取得時番号設定'
		}
	}

	#ifdef d:DEBUG_PRINT
		print $:ボックス名
		print $w:グローバルロード回数
	#endif
}
