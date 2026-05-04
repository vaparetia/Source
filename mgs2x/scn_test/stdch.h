/*
	stdch.h
	    標準キャラ用ヘッダファイル

	1999/09/03 Y.Matsuhana
	$Id: stdch.h,v 1.1 2002/02/01 06:08:32 usr01475 Exp $


*/

// ファイルが二重呼びされたときの対処
#ifndef d:STDCH_H
#define STDCH_H	1

command 解析封じ変数移動
command メモリオフセット初期化

// 変数のdefineと初期化してよい変数の初期化
//------------------------------------------------------
#include "vardef.h"


// 共通化procライブラリのインクルード
//------------------------------------------------------
#include "commdef.h"		// inf ファイルで用いるキャラ名の StrCode 定義ヘッダ
#include "stdproc.h"		// 各ファイル共通して使用するプロック
#include "demo_stdproc.h"	// ポリデモ再生ステージで共通して使用するプロック
#include "loadproc.h"		// ロード関係を扱ったプロック
#include "camproc.h"		// カメラワーク関係を扱ったプロック
#include "gameover.h"		// ゲームオーバー処理関係を扱ったプロック
#include "paddemo.h"		// パッドデモ関係を扱ったプロック
#include "sd_set.h"			// サウンド設定用プロック
#include "dogtag_data.h"	// ドッグタグのリソースが入っている
#ifdef d:STAGE_TANKER
	#include "dcam_tanker.h"
#endif


// 無線セット（無線はincludeされた時点で先呼び）
//------------------------------------------------------
#ifdef d:STAGE_PLANT
	chara 無線システム Codec -player rai_def
#else
	chara 無線システム Codec -player sna_def
#endif
	chara 2Dレイアウトドライバ ２ＤＬＤ
// 効果音の設定内容を最初に初期化する
	command 壁床効果音設定 -clear


// デモかそうでないかの値を初期化
#ifdef d:DEMO_STAGE
eval ( $s:d_num = demo) ;
#else
eval ( $s:d_num = not_demo) ;
#endif


// 常駐キャラセット
//------------------------------------------------------
proc 常駐キャラ設定 {
	#if d:DEBUG_PRINT
		print 'stdch_set'
	#endif

	//-----------------------------------------------------------------------------
	// ゲームとデモの両方で起動しておくキャラ
	//-----------------------------------------------------------------------------
	// バウンドで消す「常駐物」の前に設定する。
	command エフェクトバウンド初期化
	// 血関係
	chara カメラ血 だらり
	chara 壁血 どっぴゅ
	chara 追跡血 トレーサー

	// ライト(敵兵のライトや、それを見るキャラ、ステージの影等が参照するデータを管理)
	chara ライト位置管理 light_man

	// デプス関係
	chara Ｚフォーカス管理 Z_manager \
	    -z 5000 20000 \
	    -t 30

	//-----------------------------------------------------------------------------
	// デモ用のキャラ(薬莢は、デモでは固定値)
	//-----------------------------------------------------------------------------
	if( $s:d_num != not_demo ) {
		chara 全薬莢コントロール 薬莢管理人 \
			-n_emb 32 \
			-ratios 1 \
					1 \
					1 \
					1 \
					1 \
					10 \
					1 \
					11 \
					1 \
					1 \
					1 \		//カメラとかの
					1 \
					1	//デモのみ
	}

	//-----------------------------------------------------------------------------
	// ゲームでのみ起動しておくキャラ
	//-----------------------------------------------------------------------------
	if( $s:d_num == not_demo ) {
		// 薬莢管理
		#ifndef d:NO_YAKKYOU
			// 一回必要なものは初期化
			eval( $i:ＵＳＰ薬莢数 = 0 );
			eval( $i:ソコム薬莢数 = 0 );
			eval( $i:Ｍ４Ａ１薬莢数 = 0 );
			eval( $i:アバカン薬莢数 = 0 );
			eval( $i:Ｍ４薬莢数 = 0 );
			eval( $i:Ｍ４グレネード薬莢数 = 0 );
			eval( $i:スパス薬莢数 = 0 );
			eval( $i:Ｐ９０薬莢数 = 0 );
			eval( $i:ＭＥＣＡ薬莢数 = 0 );
			eval( $i:グロック薬莢数 = 0 );
			eval( $i:ファマス薬莢数 = 0 );
			eval( $i:ＰＳＧ薬莢数 = 0 );
			eval( $i:表示薬莢数 = 0 );

			eval( $i:ＡＫＳ薬莢数 = 32 );			// ライデン、タンカーゴル兵

			#ifdef d:STAGE_TANKER
				eval( $i:ＵＳＰ薬莢数 = 32 );		// スネーク
			#elifdef d:NPC_SNAKE
				eval( $i:ＵＳＰ薬莢数 = 32 );		// NPCスネーク
			#elifdef d:NPC_SNAKE_RIFLE
				eval( $i:ＰＳＧ薬莢数 = 32 );		// 狙撃NPCスネーク
			#elifdef d:ENE_SHIELD
				eval( $i:ＵＳＰ薬莢数 = 32 );		// 盾兵
			#endif
			#ifdef d:STAGE_PLANT
				eval( $i:ソコム薬莢数 = 32 );		// ライデン
				eval( $i:Ｍ４Ａ１薬莢数 = 32 );		// ライデン
				eval( $i:Ｍ４薬莢数 = 32 );			// プリスキン、ハイテク兵
			#endif
			#ifdef d:ENE_P_GOL
				eval( $i:アバカン薬莢数 = 32 );		// 都市迷彩ゴル兵
			#endif

			#ifdef d:ENE_HIGH_TECH					// ハイテク兵 グレネードランチャー
				eval( $i:Ｍ４グレネード薬莢数 = 32 );
			#endif
			#ifdef d:ENE_SHOT
				eval( $i:スパス薬莢数 = 32 );		// ショットガン兵
			#endif
			#ifdef d:ENE_TNG
				eval( $i:Ｐ９０薬莢数 = 32 );		// 天狗兵
			#endif
			#ifdef d:GUN_MECA
				eval( $i:ＭＥＣＡ薬莢数 = 32 );		// ガンカメ、ガンサイファー
			#endif
			#ifdef d:STAGE_W20C
				eval( $i:グロック薬莢数 = 32 );		// ファットマン
			#endif

			eval( $i:表示薬莢数 =	$i:ＵＳＰ薬莢数 + $i:グロック薬莢数 + $i:ソコム薬莢数 + \
									$i:ファマス薬莢数 + $i:Ｐ９０薬莢数 + $i:Ｍ４Ａ１薬莢数 + \
									$i:アバカン薬莢数 + $i:ＡＫＳ薬莢数 + $i:ＰＳＧ薬莢数 + \
									$i:スパス薬莢数 + $i:ＭＥＣＡ薬莢数 + $i:Ｍ４薬莢数 + $i:Ｍ４グレネード薬莢数 );


			chara 全薬莢コントロール 薬莢管理人 \
				-n_emb $i:表示薬莢数 \
				-ratios $i:ＵＳＰ薬莢数 \
						$i:グロック薬莢数 \
						$i:ソコム薬莢数 \
						$i:ファマス薬莢数 \
						$i:Ｐ９０薬莢数 \
						$i:Ｍ４Ａ１薬莢数 \
						$i:アバカン薬莢数 \
						$i:ＡＫＳ薬莢数 \
						$i:ＰＳＧ薬莢数 \
						$i:スパス薬莢数 \
						$i:ＭＥＣＡ薬莢数 \		//カメラとかの
						$i:Ｍ４薬莢数 \
						$i:Ｍ４グレネード薬莢数	//デモのみ
		#endif

		#ifdef d:STAGE_TANKER
			//タンカーではノードに関係なくレーダーをＯＮ
			command メニュー設定 -node_access on
		#endif

		// 銃関係
		chara 弾痕 痕 \
			-u 32

		command スティンガー性能 \
			-t 64 \
			-m 400 \
			-p 14 \
			-d 70 \
			-s 0 -250 0

		command スティンガーサイト性能 \
			-l 15

		#ifdef d:STAGE_PLANT
			command ニキータタイマー設定 -time (60*23)
		#endif

		// 銃のテンプライト
		#ifdef d:MUZZLE_FLASH
			command 武器ライト -t 1				// ライトＯＮ
		#else
			command 武器ライト -t 0				// ライトＯＦＦ
		#endif

		// 装備品
		chara 装備品サーバー 装備品サーバー \
			-s 32


		// デフォルト跳弾・弾痕登録
		command 跳弾ノーマル -n 0
		command 弾痕ノーマル -n 0

		// プレイヤーのプラグイン
		command プラグイン強制移動
		command プラグイン姿勢制御

		// ゲームオーバー処理
		command システムコールバック -gameover ゲームオーバー処理 -continue コンティニュー処理
		#ifdef d:STAGE_TANKER
		    command ゲームオーバー呼びわけ -c 0
		#endif
		#ifdef d:STAGE_PLANT
		    command ゲームオーバー呼びわけ -c 1

			command ゲームオーバー背景設定 \
				-m 0

		#endif

/*
		command タンカーカメラステータス \
			-d $i:ayaya
*/

		// ノード端末
		#ifdef d:NODE
			chara ノードシステム node -skin node -p ノード接続後プロック -file t:node
		#endif

		#if d:MGS2_DEMO
		#else
			// レーダー（オンオフに関わらずアラート表示を行うため必要）
			#ifdef d:STAGE_W00B
			#elifdef d:STAGE_W13A || d:STAGE_W13B
				chara レーダー レーダー -d -341
			#elifdef d:STAGE_W17A
				chara レーダー レーダー -d 341
			#elifdef d:STAGE_W19A
				chara レーダー レーダー -d -341
			#elifdef d:STAGE_W23A || d:STAGE_W23B
				chara レーダー レーダー -d 341
			#else
				chara レーダー レーダー
			#endif
		#endif


		// ポーズ中の操作説明
/*
		#ifdef d:STAGE_TANKER
			#ifdef d:ENGLISH
				//英語版
				chara 操作説明表示 操作説明 -eng 1
			#else
				#ifdef d:JAPANESE
					//日本語版
					chara 操作説明表示 操作説明
				#endif
			#endif
		#endif
*/

		#ifdef d:STAGE_TANKER
			print 'タンカーカメラステータスセット'
//			command タンカーカメラステータス \
//				-d $i:格納場所変数

			#ifdef STAGE_W00A || STAGE_W00B || STAGE_W00C || STAGE_W01A || STAGE_W01B || STAGE_W01C || STAGE_W01D || STAGE_W01E || STAGE_W01F || STAGE_W02A || STAGE_W03A || STAGE_W03B
				@タンカーくしゃみ処理
			#endif

		#endif


		if($w:タイトルメニュー設定 & d:TITLE_MENU_START_FROM_LOAD){
			eval($w:タイトルメニュー設定 = ($w:タイトルメニュー設定 & ~ d:TITLE_MENU_START_FROM_LOAD))	//d:TITLE_MENU_START_FROM_LOADのビットを寝かせる
			command varsave $w:タイトルメニュー設定
			#ifdef d:STAGE_TANKER
//				@タンカーデジカメ写真判定初期化
			#endif
		}



		// メニューフナ虫
		#ifdef d:STAGE_PLANT
			chara メニューフナムシ メニューフナ虫 -speed (60*20)
		#endif

		// デバッグ用常駐キャラ(FIXのときは削除)
		#ifndef	d:FIX_MODE
			chara ルート表示 ルート表示
		#endif



		chara ドッグタグ名前表示マネージャ ドッグ君

		#ifdef d:GERMAN_TRIAL
			command	不可装備設定 -weapon d:武器:ＵＳＰ
		#endif
	}
}



#else

print 'stdch.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif


