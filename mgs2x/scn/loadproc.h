/*
	loadproc.h
	    ロード関数をまとめたプロック

	2000/07/12 H.Yoshiike
	$Id: loadproc.h,v 1.93 2002/08/28 02:19:42 usr03682 Exp $


*/

// ファイルが二重呼びされたときの対処
#ifndef d:LOADPROC_H
#define LOADPROC_H	1


#include "sound.h"

//--------------------------------------------------------------------
// ステージ開始時(ロード後)に呼ぶプロック群
//--------------------------------------------------------------------
// ロード時のパッド制御とフェードイン(プレイヤーの移動付き)
// (標準仕様)
// タイプ(以下のenumを参考)
// 必要な準備
//	※$i:プレイヤー初期方向はドアの方向を指定すること
//	※sload.hに eval($f:ロードチェックＯＮフラグ = 0); を記述する
//	※ロードプロック内でロードチェックＯＮフラグが立っているときロード処理をしないようにする
//	※水密ドアはchara 水密ドアで予めドアを開ける指定をすること
//------------------------------------------------
// ロードタイプ(ステージ開始時距離で使用)
enum LOAD_TYPE {
	SL_DOOR_TYPE = 0,		// スライドドア
	WT_DOOR_TYPE,			// 水密ドア
	NO_DOOR_TYPE,			// 階段or廊下でのロード
	NO_MOVE_TYPE			// 移動なし
}

// モーションタイプのデフォルト、現在はモーションタイプを引き継ぐ
#define	DEFAULT_M_TYPE	-1

// モーションタイプ(ステージ開始時処理、ステージ終了時処理で使用)
enum M_TYPE {
	NO_CHANGE = -1,		// モーションを立ち走りにせず、そのまま引き継ぐ
	WALK_CHANGE,		// モーションを歩きに変更
	RUN_CHANGE,			// モーションを走りに変更
	DASH_CHANGE,		// モーションをダッシュに変更
	CRAWL_CHANGE,		// モーションを匍匐に変更
	HCRAWL_CHANGE,		// モーションを匍匐(速)に変更
	BOX_CHANGE			// モーションをダンボールに変更
}
// 追加モーションタイプ(ステージ終了時処理でのみ使用可)
#define NO_MOVE_M_TYPE	100 // モーションによる移動なし。立ち状態でスタートする。

proc ステージ開始時処理 $:タイプ $:ドア名 $:移動距離 $:モーションタイプ {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	// 型変換(command sin等の引数が$wのため)
	eval($w:LD_プレイヤー方向 = $i:プレイヤー初期方向)
	// 強制移動終了プロックに値を渡すのに必要
	eval($s:ロード時ドア名 = $:ドア名)

	// スライドドアなら開けるメッセージを送る
	if( $:タイプ == d:LOAD_TYPE:SL_DOOR_TYPE ){
		mesg ドア $:ドア名 open d:PLAYER
	}

	@特殊装備解除

	// パッド操作を奪う(無線やポーズも不許可にする)
	command パッド操作 -release
	command メニュー設定 -radio off -pause off

	// フェードインスタート
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME

#ifdef d:START_CAMERA
	@スタートカメラセット
#endif

	// フェードイン終了後、強制移動をし、パッド操作を戻す
	chara delay パッドディレイ \
		-time d:FADEIN_TIME \
		-arg $:タイプ $:移動距離 $:モーションタイプ \
		-exec {
			if( $1 == d:LOAD_TYPE:SL_DOOR_TYPE ){
				// スライドドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				if ( $f:エマ存在フラグ == 0 ) {
					mesg プレイヤー d:PLAYER run \
						($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
						($i:プレイヤー初期Ｙ位置) \
						($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
						$3 \
						$w:LD_プレイヤー方向 \
						#ifdef d:STAGE_W18A
							d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
						#else
							#ifdef d:STAGE_W25A
								d:FA_STAGESTART \
							#else
								(d:FA_DIRECT_MOVE | d:FA_STAGESTART) \	// フラグ
							#endif
						#endif
						スライドドアロード時強制移動終了プロック

					#ifdef d:WITH_EMMA
					// エマが置き去りにされていた場合は存在フラグを立てる
						if (  $s:エマ存在ステージ == d:EMMA_STAGE ) {
							eval( $f:エマ存在フラグ = 1 );
						}
					#endif

					#if d:DEBUG_PRINT
						print 'emma_stage = '
						print $s:エマ存在ステージ
					#endif
				} else {
					// エマがいるときは手繋ぎ状態からスタート
					#ifdef d:WITH_EMMA

						mesg プレイヤー d:PLAYER motion \
							d:モーションリスト:rai_ema_run \
							$w:LD_プレイヤー方向 \
							$w:LD_プレイヤー方向 \
							d:FA_END_STAND \
							d:FA_STAGESTART \
							$i:プレイヤー初期Ｘ位置 $i:プレイヤー初期Ｚ位置

						command 強制モーションループ回数設定 -loop 2
						command	強制モーション終了プロック \
							-motion d:モーションリスト:rai_ema_run \
							-proc スライドドアロード時強制移動終了プロック

						eval( $s:エマ存在ステージ = d:EMMA_STAGE );
						chara プロック連続実行 手繋ぎ強制 \
							-time -1 \
							-exec { \
								mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ \
							}

						#if d:DEBUG_PRINT
							print 'emma_stage = '
							print $s:エマ存在ステージ
						#endif
					#endif
				}
			} else if( $1 == d:LOAD_TYPE:WT_DOOR_TYPE ){
				// 水密ドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					水密ドアロード時強制移動終了プロック

			} else if( $1 == d:LOAD_TYPE:NO_DOOR_TYPE ){
				// 階段または廊下
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					ロード時強制移動終了プロック
			} else {
				if ( $f:デモ再生フラグ == 0 ) {
					command パッド操作 -cancel
					command メニュー設定 -radio on -pause on
				}
				eval($f:ロードチェックＯＮフラグ = 1);
				#ifdef d:START_CAMERA
					@スタートカメラオフ
				#endif

				#ifdef d:WITH_EMMA
					if ( $f:エマ存在フラグ == 1 ) {
						eval( $s:エマ存在ステージ = d:EMMA_STAGE );
					}
				#endif
				#ifdef d:CTDOWN_TIMER_P
					// プラント編爆弾タイマー
					@爆弾タイマー開始
				#endif
			}
		}
}

// スタート時に場所を表示するバージョン(defineはvardef.hでdefineされています)

proc ステージ開始時処理（ステージ名表示） $:タイプ $:ドア名 $:移動距離 $:モーションタイプ $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	// 型変換(command sin等の引数が$wのため)
	eval($w:LD_プレイヤー方向 = $i:プレイヤー初期方向)
	// 強制移動終了プロックに値を渡すのに必要
	eval($s:ロード時ドア名 = $:ドア名)

	// スライドドアなら開けるメッセージを送る
	if( $:タイプ == d:LOAD_TYPE:SL_DOOR_TYPE ){
		mesg ドア $:ドア名 open d:PLAYER
	}

	@特殊装備解除

	// パッド操作を奪う(無線やポーズも不許可にする)
	command パッド操作 -release
	command メニュー設定 -radio off -pause off

	// フェードインスタート(フェードの強さを弱める)
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME -s 10

	// 前のステージの表示に文字を重ねる
	@ステージ名表示 $:表示文字

#ifdef d:START_CAMERA
	@スタートカメラセット
#endif

	// フェードイン終了後、強制移動をし、パッド操作を戻す
	chara delay パッドディレイ \
		-time d:FADEIN_TIME \
		-arg $:タイプ $:移動距離 $:モーションタイプ \
		-exec {
			if( $1 == d:LOAD_TYPE:SL_DOOR_TYPE ){
				// スライドドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
					print 'emma_flag----------------------------------------------------------------'
					print 'emma_flag'
					print $f:エマ存在フラグ
					print 'emma_stage'
					print $s:エマ存在ステージ
					print 'elevator_flag'
					print $f:エマエレベータ内フラグ
				#endif

				if ( $f:エマ存在フラグ == 0 ) {
					mesg プレイヤー d:PLAYER run \
						($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
						($i:プレイヤー初期Ｙ位置) \
						($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
						$3 \
						$w:LD_プレイヤー方向 \
						#ifdef d:STAGE_W18A
							d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
						#else
							#ifdef d:STAGE_W25A
								d:FA_STAGESTART \
							#else
								( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
							#endif
						#endif
						スライドドアロード時強制移動終了プロック

					#ifdef d:WITH_EMMA
					// エマが置き去りにされていた場合は存在フラグを立てる
						if (  $s:エマ存在ステージ == d:EMMA_STAGE ) {
							eval( $f:エマ存在フラグ = 1 );
						}
					#endif

					#if d:DEBUG_PRINT
						print 'emma_stage = '
						print $s:エマ存在ステージ
					#endif
				} else {
					// エマがいるときは手繋ぎ状態からスタート
					#ifdef d:WITH_EMMA
						mesg プレイヤー d:PLAYER motion \
							d:モーションリスト:rai_ema_run \
							$w:LD_プレイヤー方向 \
							$w:LD_プレイヤー方向 \
							d:FA_END_STAND \
							d:FA_STAGESTART \
							$i:プレイヤー初期Ｘ位置 $i:プレイヤー初期Ｚ位置

						command 強制モーションループ回数設定 -loop 2
						command	強制モーション終了プロック \
							-motion d:モーションリスト:rai_ema_run \
							-proc スライドドアロード時強制移動終了プロック

						eval( $s:エマ存在ステージ = d:EMMA_STAGE );
						chara プロック連続実行 手繋ぎ強制 \
							-time -1 \
							-exec { \
								mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ \
							}

						#if d:DEBUG_PRINT
							print 'emma_stage = '
							print $s:エマ存在ステージ
						#endif
					#endif
				}
			} else if( $1 == d:LOAD_TYPE:WT_DOOR_TYPE ){
				// 水密ドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					水密ドアロード時強制移動終了プロック

			} else if( $1 == d:LOAD_TYPE:NO_DOOR_TYPE ){
				// 階段または廊下
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					ロード時強制移動終了プロック
			} else {
				if ( $f:デモ再生フラグ == 0 ) {
					command パッド操作 -cancel
					command メニュー設定 -radio on -pause on
				}
				eval($f:ロードチェックＯＮフラグ = 1);
				#ifdef d:START_CAMERA
					@スタートカメラオフ
				#endif

				#ifdef d:WITH_EMMA
					if ( $f:エマ存在フラグ == 1 ) {
						eval( $s:エマ存在ステージ = d:EMMA_STAGE );
					}
				#endif
				#ifdef d:CTDOWN_TIMER_P
					// プラント編爆弾タイマー
					@爆弾タイマー開始
				#endif
			}
		}
}

// スタート直後パッドデモを発動する場合のバージョン(表示文字がないときは「表示文字なし」を代入)
proc ステージ開始時処理（デモ表示） $:p_タイプ $:p_ドア名 $:p_移動距離 $:p_モーションタイプ $:p_表示文字 $:p_デモファイル名 $:p_表示フラグ $:p_終了時位置Ｘ $:p_終了時位置Ｙ $:p_終了時位置Ｚ $:p_終了時方向 $:p_終了時姿勢 {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	// 型変換(command sin等の引数が$wのため)
	eval($w:LD_プレイヤー方向 = $i:プレイヤー初期方向);
	// 強制移動終了プロックに値を渡すのに必要
	eval($s:ロード時ドア名 = $:p_ドア名);
	// デモファイル名を渡すために変数に格納
	eval($s:パッドデモファイル名 = $:p_デモファイル名);
	// 表示文字を渡すために変数に格納
	eval($s:表示文字名 = $:p_表示文字);
	// 表示フラグを渡すために変数に格納
	eval($b:表示フラグ = $:p_表示フラグ);

	// 終了時位置を渡すために変数に格納
	eval($i:終了時位置Ｘ = $:p_終了時位置Ｘ);
	eval($i:終了時位置Ｙ = $:p_終了時位置Ｙ);
	eval($i:終了時位置Ｚ = $:p_終了時位置Ｚ);
	eval($i:終了時方向 = $:p_終了時方向);
	eval($b:終了時姿勢 = $:p_終了時姿勢);

	// スライドドアなら開けるメッセージを送る
	if( $:p_タイプ == d:LOAD_TYPE:SL_DOOR_TYPE ){
		mesg ドア $:p_ドア名 open d:PLAYER
	}

	// 無線機からのコールがあった場合キャンセルをかける
	command 無線設定 -reset

	@特殊装備解除
	@ダンボール解除

	// パッド操作を奪う(無線やポーズもフラグをたてなければ不許可にする)
	command パッド操作 -release
	if (($b:表示フラグ & d:RADIO_ON) == 0) {
		command メニュー設定 -radio off
	}
	if (($b:表示フラグ & d:PAUSE_ON) == 0) {
		command メニュー設定 -pause off
	}

	eval( $f:パッドデモ再生フラグ = d:PADDEMO_PLAY );

	// フェードインスタート(フェードの強さを弱める)
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME -s 10

	// 前のステージの表示に文字を重ねる
	if ($s:表示文字名 != 表示文字なし) {
		// 登場ポイント名を表示する
		@ステージ名表示 $s:表示文字名
	}

#ifdef d:START_CAMERA
	@スタートカメラセット
#endif

	// フェードイン終了後、強制移動をし、パッド操作を戻す
	chara delay パッドディレイ \
		-time d:FADEIN_TIME \
		-arg $:p_タイプ $:p_移動距離 $:p_モーションタイプ \
		-exec {
			if( $1 == d:LOAD_TYPE:SL_DOOR_TYPE ){
				// スライドドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
					print 'emma_flag----------------------------------------------------------------'
					print 'emma_flag'
					print $f:エマ存在フラグ
					print 'emma_stage'
					print $s:エマ存在ステージ
					print 'elevator_flag'
					print $f:エマエレベータ内フラグ
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					スライドドアロード時強制移動終了プロック（デモ版）

			} else if ( $1 == d:LOAD_TYPE:WT_DOOR_TYPE ) {
				// 水密ドア
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					水密ドアロード時強制移動終了プロック（デモ版）

			} else if( $1 == d:LOAD_TYPE:NO_DOOR_TYPE ){
				// 階段または廊下
				#if d:DEBUG_PRINT
					print	'X_VAR='($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )
					print	'Z_VAR='($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )
					print	'X='($i:プレイヤー初期Ｘ位置+($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
							'Y='($i:プレイヤー初期Ｙ位置) \
							'Z='($i:プレイヤー初期Ｚ位置+($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 ))
				#endif

				mesg プレイヤー d:PLAYER run \
					($i:プレイヤー初期Ｘ位置 + ($2 * (`%sin $w:LD_プレイヤー方向`) / 4096 )) \
					($i:プレイヤー初期Ｙ位置) \
					($i:プレイヤー初期Ｚ位置 + ($2 * (`%cos $w:LD_プレイヤー方向`) / 4096 )) \
					$3 \
					$w:LD_プレイヤー方向 \
					#ifdef d:STAGE_W18A
						d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
					#else
						#ifdef d:STAGE_W25A
							d:FA_STAGESTART \
						#else
							( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
						#endif
					#endif
					ロード時強制移動終了プロック（デモ版）
			} else {
				@スタート時パッドデモ開始 $b:表示フラグ
				#ifdef d:START_CAMERA
					@スタートカメラオフ
				#endif
			}
		}
}

// ステージ開始時処理で使用するサブプロック
proc スライドドアロード時強制移動終了プロック {
	#if d:DEBUG_PRINT
		print 'slide_loadendproc_start'
	#endif

	mesg ドア $s:ロード時ドア名 close

	// スライドドアが閉まりきる時間分ディレイする(閉まるフラグだと開きっぱなしの時に問題)
	chara delay パッドディレイ \
		-time d:SD_OPEN_TIME \
		-exec {
			if ( $f:デモ再生フラグ == 0 ) {
				#ifdef d:CTDOWN_TIMER
					mesg タイマー タイマー 開始
				#endif
				#ifdef d:CTDOWN_TIMER_P
					// プラント編爆弾タイマー
					@爆弾タイマー開始
				#endif
				command パッド操作 -cancel
				command メニュー設定 -radio on -pause on
			}

			eval($f:ロードチェックＯＮフラグ = 1);

			#ifdef d:START_CAMERA
				@スタートカメラオフ
			#endif

			#ifdef d:WITH_EMMA
				if ( $f:エマ存在フラグ == 1 ) {
					mesg プロック連続実行 手繋ぎ強制 kill
					mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ解除
				}
			#endif
	}
}

proc 水密ドアロード時強制移動終了プロック {
	#if d:DEBUG_PRINT
		print 'wt_loadendproc_start'
	#endif

	#ifdef d:CTDOWN_TIMER
		mesg タイマー タイマー 開始
	#endif

	mesg 水密ドア $s:ロード時ドア名 close

	if ( $f:デモ再生フラグ == 0 ) {
		command パッド操作 -cancel
		command メニュー設定 -radio on -pause on
	}

	eval($f:ロードチェックＯＮフラグ = 1);

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif
}

proc ロード時強制移動終了プロック {
	#if d:DEBUG_PRINT
		print 'nodoor_loadendproc_start'
	#endif

	#ifdef d:CTDOWN_TIMER
		mesg タイマー タイマー 開始
	#endif

	if ( $f:デモ再生フラグ == 0 ) {
		command パッド操作 -cancel
		command メニュー設定 -radio on -pause on
	}

	eval($f:ロードチェックＯＮフラグ = 1);

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif
}

proc スライドドアロード時強制移動終了プロック（デモ版） {
	#if d:DEBUG_PRINT
		print 'slide_loadendproc_start'
	#endif

	mesg ドア $s:ロード時ドア名 close

	@スタート時パッドデモ開始 $b:表示フラグ

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif
}


proc 水密ドアロード時強制移動終了プロック（デモ版） {
	#if d:DEBUG_PRINT
		print 'wt_loadendproc_start'
	#endif

	mesg 水密ドア $s:ロード時ドア名 close

	@スタート時パッドデモ開始 $b:表示フラグ

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif
}


proc ロード時強制移動終了プロック（デモ版） {
	#if d:DEBUG_PRINT
		print 'nodoor_loadendproc_start'
	#endif

	@スタート時パッドデモ開始 $b:表示フラグ

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif
}

proc ステージ名表示 $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_name_set'
	#endif

	chara ２Ｄスプライト表示 マップネーム表示 \
		-tri 2D_tex \
		-x $:表示文字 \
		-pos 256 181 \
		-count 0,90,30 \
		-flag d:FLAG_CENTER \
		-l
}

proc ステージ名表示（終了時） $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_name_set'
	#endif

	chara ２Ｄスプライト表示 マップネーム表示 \
		-tri 2D_tex \
		-x $:表示文字 \
		-pos 256 181 \
		-count 20,600,0 \
		-flag d:FLAG_CENTER \
		-l
}

proc スタート時パッドデモ開始 {
	#if d:DEBUG_PRINT
		print 'pad_demo_end'
	#endif

	@シナリオデモ開始処理（ダンボールも解除）

	chara デモキャンセルチェック デモキャンセル \
		-p スタート時パッドデモキャンセル

	chara パッドデモ パッドデモ \
		-file $s:パッドデモファイル名 \
		-end_proc スタート時パッドデモ終了

	if (($b:表示フラグ & d:MENU_ON) == 1) {
		command メニュー設定 -menu on
	}

	if (($b:表示フラグ & d:GAGE_ON) == 1) {
		command メニュー設定 -gage on
	}

	if (($b:表示フラグ & d:RADAR_ON) == 1) {
		command メニュー設定 -radar on
	}

	if (($b:表示フラグ & d:SUBWIN_ON) == 1) {
		command メニュー設定 -subwin on
	}

	mesg パッドデモ パッドデモ start
}

proc スタート時パッドデモキャンセル {
	#if d:DEBUG_PRINT
		print 'pad_demo_cancel'
	#endif

	mesg コマンダー 敵兵セット 視界オフ

	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t d:FADEOUT_TIME \
		-p スタート時パッドデモキャンセル後
}

proc スタート時パッドデモキャンセル後 {
	#if d:DEBUG_PRINT
		print 'pad_demo_cancel_end'
	#endif

	mesg パッドデモ パッドデモ kill
	mesg デモキャンセルチェック デモキャンセル kill
	mesg シネマスクリーン シネマ フェードイン 0

	chara delay 強制移動ディレイ \
		-time 1 \
		-exec {
			mesg プレイヤー d:PLAYER position $i:終了時位置Ｘ $i:終了時位置Ｙ $i:終了時位置Ｚ
			mesg プレイヤー d:PLAYER stance $b:終了時姿勢 $i:終了時方向 $i:終了時方向 0
			// 主観系装備などは解除
			@特殊装備解除
			@ダンボール解除

			chara フェードインアウト フェードイン \
				-i 0,0,0 128 \
				-c 0,0,0 0 \
				-t d:FADEIN_TIME \
				-p スタート時パッドデモキャンセル後スタート
		}
}

proc スタート時パッドデモキャンセル後スタート {
	#if d:DEBUG_PRINT
		print 'restart'
	#endif

	@シナリオデモ終了処理
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_END );
	eval($f:ロードチェックＯＮフラグ = 1);
	#ifdef d:CTDOWN_TIMER_P
		// プラント編爆弾タイマー
		@爆弾タイマー開始
	#endif
}


proc スタート時パッドデモ終了 {
	#if d:DEBUG_PRINT
		print 'pad_demo_end'
	#endif

	@シナリオデモ終了処理
	mesg デモキャンセルチェック デモキャンセル kill
	mesg シネマスクリーン シネマ フェードイン 0
	command メニュー設定 -menu on -gage on -radar on -subwin on -radio on -pause on
	eval( $f:パッドデモ再生フラグ = d:PADDEMO_END );
	eval($f:ロードチェックＯＮフラグ = 1);
	#ifdef d:CTDOWN_TIMER_P
		// プラント編爆弾タイマー
		@爆弾タイマー開始
	#endif
}



// ロード時のパッド制御とフェードイン
//------------------------------------------------
proc ロード時フェードイン {
	#if d:DEBUG_PRINT
		print 'fadein_start'
	#endif

	// フェードインが終わるまではパッドリリース
	command パッド操作 -release

	eval($f:ロードチェックＯＮフラグ = 1);

	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME

	chara delay パッドディレイ \
		-time d:FADEIN_TIME \
		-exec {
			if ( $f:デモ再生フラグ == 0 ) {

				#ifdef d:CTDOWN_TIMER
					mesg タイマー タイマー 開始
				#endif
				#ifdef d:CTDOWN_TIMER_P
					// プラント編爆弾タイマー
					@爆弾タイマー開始
				#endif
				command パッド操作 -cancel
			}
		}
}


// ロード時のパッド制御とフェードイン(RGB値とタイムが指定可能)
//------------------------------------------------
proc ロード時フェードイン２ $:Ｒ値 $:Ｇ値 $:Ｂ値 $:タイム {
	#if d:DEBUG_PRINT
		print 'fadein2_start'
	#endif

	// フェードインが終わるまではパッドリリース
	command パッド操作 -release

	chara フェードインアウト フェードイン \
		-i $:Ｒ値,$:Ｇ値,$:Ｂ値 128 \
		-c 0,0,0 0 \
		-t $:タイム

	chara delay パッドディレイ \
		-time d:FADEIN_TIME \
		-exec {
			command パッド操作 -cancel
		}
}


// エレベーターのステージ開始時処理(特殊そうなので他とは別もち)
//------------------------------------------------
#define	NO_MESG		0		// 表示文字なし

proc ステージ開始時処理（エレベーター） $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	// エレベーターで来た時は強制的に$f:ロードチェックＯＮフラグを立てる
	eval( $f:ロードチェックＯＮフラグ = 1 )
	
	#ifdef d:STAGE_ALT
	// スネークテイルズ用にオルタナティブ系はちょっと修正
		if ( $b:ミッション番号 == d:MISSION:スネークテイルズ && \
			 $b:tales_story_no == d:TALES_NO:A && \
			 $b:tales_story_a == d:TALES_A:爆弾解体後 && \
			 $s:テイルズＡエレベータの行き先ステージ名 != なし ) {

			mesg エレベータ 中央棟エレベーター tales_stagestart
			// 紙芝居用の特殊ロードはこれで完了なので、再び初期値に戻す
			$s:テイルズＡエレベータの行き先ステージ名 = なし;
		} else {
			mesg エレベータ 中央棟エレベーター stagestart
		}
	#else
	// 本編
		mesg エレベータ 中央棟エレベーター stagestart
	#endif

	chara delay エレベータディレイ -time 3 -exec {
		command パッド操作 -release
	}

	// 前のステージの表示に文字を重ねる
	if ( $:表示文字 != d:NO_MESG ) {
		@ステージ名表示 $:表示文字
	}

	// エマが一緒にいたらステージを更新
	#ifdef d:WITH_EMMA
		if ( $f:エマ存在フラグ == 1 ) {
			eval( $s:エマ存在ステージ = d:EMMA_STAGE );
			// エレベーターフラグは次のロード時チェックまで寝かす
			eval( $f:エマエレベータ内フラグ = 0 );
		} else if (  $s:エマ存在ステージ == d:EMMA_STAGE ) {
			// エマが置き去りにされていた場合はフラグをここで立てる
			eval( $f:エマ存在フラグ = 1 );
		}

	#endif

	// フェードインスタート(フェードの強さを弱める)
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME \
		-p エレベーター起動 \
		-s 10
}

proc エレベーター起動 {
	#if d:DEBUG_PRINT
		print 'elevator_open'
	#endif

	// ドアオープン
	chara delay エレベータディレイ -time 30 -exec {
		mesg エレベータ 中央棟エレベーター open 0 NULL NULL
	}

	// パネルモードから抜ける
	chara delay エレベータディレイ -time 90 -exec {
		mesg プレイヤー d:PLAYER evpanel 出る 0
		if ( $f:デモ再生フラグ == 0 ) {
			command パッド操作 -cancel
		}
	}
}


// ダンボールワープのステージ開始時処理(特殊そうなので他とは別もち)
//------------------------------------------------
proc ステージ開始時処理（ダンボールワープ） $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	eval( $f:ロードチェックＯＮフラグ = 1 )

	// 前のステージの表示に文字を重ねる
	if ( $:表示文字 != d:NO_MESG ) {
		@ステージ名表示 $:表示文字
	}

	// フェードインスタート(フェードの強さを弱める)
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME \
		-p ダンボールでスタート \
		-s 10
}

proc ダンボールでスタート {
	if ( $f:デモ再生フラグ == 0 ) {
		command パッド操作 -cancel
		command メニュー設定 -radio on -pause on
	}
	eval($f:ロードチェックＯＮフラグ = 1);

	chara パッド振動 登場振動 -vibfile box_move
	command ＳＥセットモード -s d:se_code:SD_A_DANB_IN1 \
		-p $i:プレイヤー位置Ｘ,$i:プレイヤー位置Ｙ,$i:プレイヤー位置Ｚ -m 1

	#ifdef d:START_CAMERA
		@スタートカメラオフ
	#endif

	#ifdef d:CTDOWN_TIMER_P
		// プラント編爆弾タイマー
		@爆弾タイマー開始
	#endif
}



// 昇降機のステージ開始時処理(特殊そうなので他とは別もち)
//------------------------------------------------
proc ステージ開始時処理（昇降機） $:表示文字 {
	#if d:DEBUG_PRINT
		print 'stage_start'
	#endif

	@特殊装備解除

	// パッド操作を奪う(無線やポーズも不許可にする)
	command パッド操作 -release
	command メニュー設定 -radio off -pause off

	// 前のステージの表示に文字を重ねる
	if ( $:表示文字 != d:NO_MESG ) {
		@ステージ名表示 $:表示文字
	}

	// フェードインスタート(フェードの強さを弱める)
	chara フェードインアウト フェードイン \
		-i 0,0,0 128 \
		-c 0,0,0 0 \
		-t d:FADEIN_TIME \
		-s 10
}

proc ステージ開始時処理終了（昇降機） {
	#if d:DEBUG_PRINT
		print 'stage_start_proc_end'
	#endif

	if ( $f:デモ再生フラグ == 0 ) {
		command パッド操作 -cancel
		command メニュー設定 -radio on -pause on
	}

	eval($f:ロードチェックＯＮフラグ = 1);
}


//--------------------------------------------------------------------
// ステージ終了時(ロード前)に呼ぶプロック群
//--------------------------------------------------------------------
// ロード時のパッド制御とフェードアウト(標準仕様)
// 必要な処理
//	・LD_FADEOUTをdefineで定義する
//	・モーションタイプは上のenum M_TYPEを参照
//	・ロードプロックを各gcl内で定義する
//		例)	proc ロードプロック {
//				if ($s:呼び出しプロック == mv_w02a0_w01f_0) {
//					@mv_w02a0_w01f_0
//				} else if ($s:呼び出しプロック == mv_w02a3_w01f_1) {
//					@mv_w02a3_w01f_1
//				} else if ($s:呼び出しプロック == mv_w02a3_w03a0_0) {
//					@mv_w02a3_w03a0_0
//				}
//			}
//------------------------------------------------
#ifdef d:STAGE_END
	proc ステージ終了時処理 $:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ $:終了時方向 $:終了時プロック $:モーションタイプ {
		#if d:DEBUG_PRINT
			print 'stage_end'
		#endif

		if ( `command ゲームオーバーチェック` == 0 ) {
			// 二重呼び防止のためにフラグを寝かす
			eval($f:ロードチェックＯＮフラグ = 0);

			// 次のステージへのアラートモード引継
			if ($w:ゲーム設定 <= d:LEVEL_VERYEASY) {
				// easyより易しいレベルはロードすると潜入モード(日本語版以降は新easyのみ)
				eval( $w:スタートアラートモード = d:ALERT_MODE_SENNYU );
			} else {
				if ($w:アラートモード == d:ALERT_MODE_KIKEN && $i:アラートレベル < 1024) {
					// 危険モード時(危険値MAXは除く)は回避モードまで下げる
					eval( $w:スタートアラートモード = d:ALERT_MODE_KAIHI );
				} else {
					eval( $w:スタートアラートモード = $w:アラートモード );
				}
			}

			if ($w:アラートモード != d:ALERT_MODE_KIKEN) {
				// 敵兵の視界オフ
				mesg コマンダー 敵兵セット 視界オフ
			}

			// 無線機からのコールがあった場合キャンセルをかける
			command 無線設定 -reset

			@特殊装備解除

			// パッド操作を奪う(無線やポーズもフラグをたてなければ不許可にする)
			command パッド操作 -release
			command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off

			// 出ていた字幕を消す
			command 字幕制御 -disable

			#if d:DEBUG_PRINT
				print 'emma_flag----------------------------------------------------------------'
				print 'emma_flag'
				print $f:エマ存在フラグ
				print 'emma_stage'
				print $s:エマ存在ステージ
				print 'elevator_flag'
				print $f:エマエレベータ内フラグ
			#endif

			if ( $f:エマ存在フラグ == 0 ) {
				if ($:モーションタイプ == d:M_TYPE:WALK_CHANGE || $:モーションタイプ == d:M_TYPE:RUN_CHANGE || \
					$:モーションタイプ == d:M_TYPE:DASH_CHANGE) {
					mesg プレイヤー d:PLAYER stance 0 -1 -1 0
				}

				if ($:モーションタイプ != d:NO_MOVE_M_TYPE) {
					mesg プレイヤー d:PLAYER run\
						$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
						$:モーションタイプ \
						$:終了時方向 \
						d:FA_DIRECT_MOVE 	// フラグ
				}
			} else {
				// エマがステージ内にいるときはライフを保存
				#ifdef d:WITH_EMMA
					command ゲットエマライフ $i:エマライフ現在値
					// エマがいるときは手つなぎかどうかチェックをして
					command ゲットエマ手繋ぎ状況 $i:ローカル変数Ｉ１
					if ( $i:ローカル変数Ｉ１ > 0 ) {
						// エマと手繋ぎ状態
						mesg プレイヤー d:PLAYER motion \
							d:モーションリスト:rai_ema_run \
							$:終了時方向 \
							$:終了時方向 \
							d:FA_END_STAND \
							0 \
							$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｚ

						command 強制モーションループ回数設定 -loop 3

						chara プロック連続実行 手繋ぎ強制 \
							-time -1 \
							-exec { \
								mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ \
							}
					} else {
						// エマと手をつないでいないときは存在フラグをねかし、現在位置を保存
						if ( $f:エマエレベータ内フラグ == 0 ) {
							eval( $f:エマ存在フラグ = 0 );
							command ゲットエマ座標 $i:エマ初期位置Ｘ,$i:エマ初期位置Ｙ,$i:エマ初期位置Ｚ
						}

						if ($:モーションタイプ == d:M_TYPE:WALK_CHANGE || $:モーションタイプ == d:M_TYPE:RUN_CHANGE || \
							$:モーションタイプ == d:M_TYPE:DASH_CHANGE) {
							mesg プレイヤー d:PLAYER stance 0 -1 -1 0
						}

						if ($:モーションタイプ != d:NO_MOVE_M_TYPE) {
							mesg プレイヤー d:PLAYER run\
								$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
								$:モーションタイプ \
								$:終了時方向 \
									d:FA_DIRECT_MOVE 	// フラグ
						}
					}
				#endif
			}

			eval($s:呼び出しプロック = $:終了時プロック);

			chara フェードインアウト フェードアウト \
				-c 0,0,0 128 \
				-t d:FADEOUT_TIME \
				-p フェード後ロードへ
		}
	}

	proc ステージ終了時処理（ステージ名表示） $:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ $:終了時方向 $:終了時プロック $:モーションタイプ $:表示文字 {
		#if d:DEBUG_PRINT
			print 'stage_end'
		#endif

		if ( `command ゲームオーバーチェック` == 0 ) {
			// 二重呼び防止のためにフラグを寝かす
			eval($f:ロードチェックＯＮフラグ = 0);

			// 次のステージへのアラートモード引継
			if ($w:ゲーム設定 <= d:LEVEL_VERYEASY) {
				// easyより易しいレベルはロードすると潜入モード(日本語版以降は新easyのみ)
				eval( $w:スタートアラートモード = d:ALERT_MODE_SENNYU );
			} else {
				if ($w:アラートモード == d:ALERT_MODE_KIKEN && $i:アラートレベル < 1024) {
					// 危険モード時(危険値MAXは除く)は回避モードまで下げる
					eval( $w:スタートアラートモード = d:ALERT_MODE_KAIHI );
				} else {
					eval( $w:スタートアラートモード = $w:アラートモード );
				}
			}

			if ($w:アラートモード != d:ALERT_MODE_KIKEN) {
				// 敵兵の視界オフ
				mesg コマンダー 敵兵セット 視界オフ
			}

			// 無線機からのコールがあった場合キャンセルをかける
			command 無線設定 -reset

			@特殊装備解除

			// パッド操作を奪う(無線やポーズもフラグをたてなければ不許可にする)
			command パッド操作 -release
			command メニュー設定 -menu off -gage off -radar off -subwin off -radio off -pause off

			// 出ていた字幕を消す
			command 字幕制御 -disable

			#if d:DEBUG_PRINT
				print 'emma_flag----------------------------------------------------------------'
				print 'emma_flag'
				print $f:エマ存在フラグ
				print 'emma_stage'
				print $s:エマ存在ステージ
				print 'elevator_flag'
				print $f:エマエレベータ内フラグ
			#endif

			if ( $f:エマ存在フラグ == 0 ) {
				if ($:モーションタイプ == d:M_TYPE:WALK_CHANGE || $:モーションタイプ == d:M_TYPE:RUN_CHANGE || \
					$:モーションタイプ == d:M_TYPE:DASH_CHANGE) {
					mesg プレイヤー d:PLAYER stance 0 -1 -1 0
				}

				if ($:モーションタイプ != d:NO_MOVE_M_TYPE) {
					mesg プレイヤー d:PLAYER run\
						$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
						$:モーションタイプ \
						$:終了時方向 \
							d:FA_DIRECT_MOVE 	// フラグ
				}
			} else {
				// エマがステージ内にいるときはライフを保存
				#ifdef d:WITH_EMMA
					command ゲットエマライフ $i:エマライフ現在値
					// エマがいるときは手つなぎかどうかチェックをして
					command ゲットエマ手繋ぎ状況 $i:ローカル変数Ｉ１
					if ( $i:ローカル変数Ｉ１ > 0 ) {
						// エマと手繋ぎ状態
						mesg プレイヤー d:PLAYER motion \
							d:モーションリスト:rai_ema_run \
							$:終了時方向 \
							$:終了時方向 \
							d:FA_END_STAND \
							0 \
							$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｚ

						command 強制モーションループ回数設定 -loop 3

						chara プロック連続実行 手繋ぎ強制 \
							-time -1 \
							-exec { \
								mesg エマ・エメリッヒ 手繋ぎエマ 強制手繋ぎ \
							}

					} else {
						// エマと手をつないでいないときは存在フラグをねかし、現在位置を保存
						if ( $f:エマエレベータ内フラグ == 0 ) {
							eval( $f:エマ存在フラグ = 0 );
							command ゲットエマ座標 $i:エマ初期位置Ｘ,$i:エマ初期位置Ｙ,$i:エマ初期位置Ｚ
							#if d:DEBUG_PRINT
								print 'emma_position'
								print $i:エマ初期位置Ｘ
								print $i:エマ初期位置Ｙ
								print $i:エマ初期位置Ｚ
							#endif
						}

						if ($:モーションタイプ == d:M_TYPE:WALK_CHANGE || $:モーションタイプ == d:M_TYPE:RUN_CHANGE || \
							$:モーションタイプ == d:M_TYPE:DASH_CHANGE) {
							mesg プレイヤー d:PLAYER stance 0 -1 -1 0
						}

						if ($:モーションタイプ != d:NO_MOVE_M_TYPE) {
							mesg プレイヤー d:PLAYER run\
								$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
								$:モーションタイプ \
								$:終了時方向 \
									d:FA_DIRECT_MOVE 	// フラグ
						}
					}
				#endif
			}

			eval($s:呼び出しプロック = $:終了時プロック);

			chara delay ステージ名表示ディレイ \
				-time 40 \
				-arg $:表示文字 \
				-exec {
					@ステージ名表示（終了時） $1
				}

			chara フェードインアウト フェードアウト \
				-c 0,0,0 128 \
				-t d:FADEOUT_TIME \
				-p フェード後ロードへ -s 10
		}
	}


proc フェード後ロードへ {
	#if d:DEBUG_PRINT
		print 'stage_end'
	#endif

	// 次のステージで潜入モードになるならBGMをフェードアウト
#if 0
	if ($w:アラートモード != d:ALERT_MODE_SENNYU && $w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
	}
#else	//01.9.9
	if ( `command 再生ＢＧＭ取得` != d:SNG_PLAY_05 && $w:スタートアラートモード == d:ALERT_MODE_SENNYU) {
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
	} else if ( `command 再生ＢＧＭ取得` != d:SNG_PLAY_04 && $w:スタートアラートモード == d:ALERT_MODE_TANSAKU) {
		command セットサウンドコード -c d:SNG_FOUTS_S		//ＢＧＭフェードアウト
	}
#endif

	if ( `command ゲームオーバーチェック` == 0 ) {
		@ロードプロック
	} else {
		// フェードアウト後ゲームオーバーになった場合はフェードインして終了
		chara フェードインアウト フェードイン \
			-i 0,0,0 128 \
			-c 0,0,0 0 \
			-t 1
	}
}




#endif


// ロード時のパッド制御とフェードアウト(モーション指定なし)
// 必要な処理
//	・LD_FADEOUTをdefineで定義する
//	・ロードプロックを各gcl内で定義する
//		例)	proc ロードプロック {
//				if ($s:呼び出しプロック == mv_w02a0_w01f_0) {
//					@mv_w02a0_w01f_0
//				} else if ($s:呼び出しプロック == mv_w02a3_w01f_1) {
//					@mv_w02a3_w01f_1
//				} else if ($s:呼び出しプロック == mv_w02a3_w03a0_0) {
//					@mv_w02a3_w03a0_0
//				}
//			}
//------------------------------------------------
#ifdef d:LD_FADEOUT
	proc 新ロード時フェードアウト $:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ $:終了時方向 $:終了時プロック {
		#if d:DEBUG_PRINT
			print 'new_fadeout_start'
		#endif

		if ( `command ゲームオーバーチェック` == 0 ) {
			command パッド操作 -release

			mesg プレイヤー d:PLAYER run\
				$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
				-1 \
				$:終了時方向 \
						#ifdef d:STAGE_W18A
							d:FA_STAGESTART \	// w18a はゾーンを見ながら侵入
						#else
							#ifdef d:STAGE_W25A
								d:FA_STAGESTART \
							#else
								( d:FA_DIRECT_MOVE | d:FA_STAGESTART ) \	// フラグ
							#endif
						#endif

			eval($s:呼び出しプロック = $:終了時プロック);

			chara フェードインアウト フェードアウト \
				-c 0,0,0 128 \
				-t d:FADEOUT_TIME \
				-p ロードプロック
		}
	}
#endif

































// 以下は将来的に削除予定のものです


// ロード時のパッド制御とフェードアウト
// ※使用時には必ず$:終了時プロックに登録するプロック名を前で呼んでおくこと
// ※例）if (0) { @mv_w02a3_w03a0_0 }
// あまり美しくないので早いうちにロード前処理へ移行してください
//------------------------------------------------
proc ロード時フェードアウト $:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ $:終了時方向 $:終了時プロック {
	#if d:DEBUG_PRINT
		print 'new_fadeout_start'
	#endif

	command パッド操作 -release

	mesg プレイヤー d:PLAYER run\
		$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
		-1 \
		$:終了時方向 \
		d:FA_NO_WEAPON

	chara フェードインアウト フェードアウト \
		-c 0,0,0 128 \
		-t d:FADEOUT_TIME \
		-p $:終了時プロック
}


// ロード時のパッド制御とフェードアウト(RGB値、動作、タイムが指定可能)
// ※使用時には必ず$:終了時プロックに登録するプロック名を前で呼んでおくこと
// ※例）if (0) { @mv_w02a3_w03a0_0 }
// あまり美しくないので早いうちにロード前処理へ移行してください
//------------------------------------------------
proc ロード時フェードアウト２ $:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ $:終了時方向 $:終了時プロック $:動作番号 $:Ｒ値 $:Ｇ値 $:Ｂ値 $:タイム {
	command パッド操作 -release

	mesg プレイヤー d:PLAYER run\
		$:目標位置Ｘ $:目標位置Ｙ $:目標位置Ｚ \
		$:動作番号 \
		$:終了時方向 \
		d:FA_NO_WEAPON

	chara フェードインアウト フェードアウト \
		-c $:Ｒ値,$:Ｇ値,$:Ｂ値 128 \
		-t $:タイム \
		-p $:終了時プロック
}


#else

print 'loadproc.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif

