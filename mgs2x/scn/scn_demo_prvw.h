/*
	scn_demo_prvw.h
	    シナリオデモプレビューヘッダ

	2000/07/06 S.Nojiri
	$Id: scn_demo_prvw.h,v 1.9 2001/05/22 06:39:53 usr01098 Exp $

	
*/

#if d:シナリオデモカメラアングル作成

// 実験
if($s:エリア == w00a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 甲板)
}

// ステージ毎のマップ設定緒元代入
/*w01b-居住区２Ｆ*/
if($s:エリア == w01b){
print 'シナリオデモプレビューヘッダ_000'
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ２Ｆ廻廊部)

// 回廊部右側
	eval( $i:プレイヤー初期Ｘ位置 = 3500 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 3000 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = -9000 ) ;
	eval( $i:プレイヤー初期方向 = 1024 ) ;
	eval( $f:ロードチェックＯＮフラグ = 0);
	eval( $s:登場ポイント = 居住区一階中央階段右からロード ) ;

print 'シナリオデモプレビューヘッダ_001'

/*w02a-エンジンルーム*/
} else if($s:エリア == w02a){
	eval($i:global_シナリオデモ作成_マップ数 = 6)
	eval($s:global_シナリオデモ作成_マップ名１ = 右舷小部屋)
	eval($s:global_シナリオデモ作成_マップ名２ = エンジンルーム)
	eval($s:global_シナリオデモ作成_マップ名３ = エンジンルーム底)
	eval($s:global_シナリオデモ作成_マップ名４ = 左舷小部屋)
	eval($s:global_シナリオデモ作成_マップ名５ = 右舷居住区Ｂ１)
	eval($s:global_シナリオデモ作成_マップ名６ = 左舷居住区Ｂ１)

// 回廊部右側
	eval( $i:プレイヤー初期Ｘ位置 = 12153 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = -4035 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = -23679 ) ;
	eval( $i:プレイヤー初期方向 = 1024 ) ;
//	eval( $f:ロードチェックＯＮフラグ = 0);
//	eval( $s:登場ポイント = 居住区一階中央階段右からロード ) ;

/*w00c-航海甲板*/
} else if($s:エリア == w00c){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ブリッジ屋上部)

// 

/*w04a-第一船倉*/
} else if($s:エリア == w04a){
	eval($i:global_シナリオデモ作成_マップ数 = 3)
	eval($s:global_シナリオデモ作成_マップ名１ = 船倉１)
	eval($s:global_シナリオデモ作成_マップ名２ = 通路１)
	eval($s:global_シナリオデモ作成_マップ名３ = 長廊下小部屋)

/*w04b-第二船倉*/
} else if($s:エリア == w04b){
	eval($i:global_シナリオデモ作成_マップ数 = 3)
	eval($s:global_シナリオデモ作成_マップ名１ = 船倉２)
	eval($s:global_シナリオデモ作成_マップ名２ = 通路１)
	eval($s:global_シナリオデモ作成_マップ名３ = 通路２)

/*w04c-第三船倉*/
} else if($s:エリア == w04c){
	eval($i:global_シナリオデモ作成_マップ数 = 2)
	eval($s:global_シナリオデモ作成_マップ名１ = 通路２)
	eval($s:global_シナリオデモ作成_マップ名１ = 船倉３)

/*w11a-海底ドック*/
} else if($s:エリア == w11a){
	eval($i:global_シナリオデモ作成_マップ数 = 3)
	eval($s:global_シナリオデモ作成_マップ名１ = 潜入プール)
	eval($s:global_シナリオデモ作成_マップ名２ = 連結ハッチ)
	eval($s:global_シナリオデモ作成_マップ名３ = 昇降機ホール)

/*w11c-海底ドック（フォーチュン戦）*/
} else if($s:エリア == w11c){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 昇降機ホール)

/*w14a-Ｂ脚変電室*/
} else if($s:エリア == w14a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 変電室)

/*w15a-ＢＣ連絡橋*/
} else if($s:エリア == w15a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ＢＣ連絡橋)

/*w16a-Ｃ脚食堂*/
} else if($s:エリア == w16a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 食堂＆トイレ)

/*w19a-ＤＥ連絡橋*/
} else if($s:エリア == w19a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ＤＥ連絡橋)

/*w20c-Ｅ脚ヘリポート（ファットマン戦）*/
} else if($s:エリア == w20c){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ヘリポート)

/*w21a-ＥＦ連絡橋*/
} else if($s:エリア == w21a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ＥＦ連絡橋)

/*w23a-ＦＡ連絡橋*/
} else if($s:エリア == w23a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ＦＡ連絡橋)

/*w24a-シェル１中央棟１Ｆ*/
} else if($s:エリア == w24a){
	eval($i:global_シナリオデモ作成_マップ数 = 2)
	eval($s:global_シナリオデモ作成_マップ名１ = シェル１中央棟１Ｆ)
	eval($s:global_シナリオデモ作成_マップ名２ = シェル１中央棟小部屋)

	eval( $i:プレイヤー初期Ｘ位置 = 2134 ) ;
	eval( $i:プレイヤー初期Ｙ位置 = 926 ) ;
	eval( $i:プレイヤー初期Ｚ位置 = -72116 ) ;
	eval( $i:プレイヤー初期方向 = 0 ) ;
	eval( $f:ロードチェックＯＮフラグ = 0);
	eval( $s:登場ポイント = sp_w15a_w24a_0 ) ;

/*w24b-シェル１中央棟Ｂ１*/
} else if($s:エリア == w24b){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = シェル１中央棟Ｂ１)

/*w24c-シェル１中央棟Ｂ１ホール（人質部屋）*/
} else if($s:エリア == w24c){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 人質部屋)

/*w24d-シェル１中央棟Ｂ２電算室*/
} else if($s:エリア == w24d){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = シェル１中央棟Ｂ３電算室)

/*w25a-ＤＧ連絡橋＆ハリアー戦*/
} else if($s:エリア == w25a){
	eval($i:global_シナリオデモ作成_マップ数 = 2)
	eval($s:global_シナリオデモ作成_マップ名１ = ＤＧ連絡橋1F)
	eval($s:global_シナリオデモ作成_マップ名２ = ＤＧ連絡橋B1)

/*w25b-タイトロープ前半*/
} else if($s:エリア == w25b){
	eval($i:global_シナリオデモ作成_マップ数 = 2)
	eval($s:global_シナリオデモ作成_マップ名１ = タイトロープ１)
	eval($s:global_シナリオデモ作成_マップ名２ = タイトロープ２)

/*w25c-タイトロープ後半*/
} else if($s:エリア == w25c){
	eval($i:global_シナリオデモ作成_マップ数 = 2)
	eval($s:global_シナリオデモ作成_マップ名１ = タイトロープ１)
	eval($s:global_シナリオデモ作成_マップ名２ = タイトロープ２)

/*w31a-シェル２中央棟１Ｆ*/
} else if($s:エリア == w31a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = シェル２中央棟１Ｆ)

/*w32a-海上（狙撃イベント）*/
} else if($s:エリア == w32a){
	eval($i:global_シナリオデモ作成_マップ数 = 4)
	eval($s:global_シナリオデモ作成_マップ名１ = 海上１)
	eval($s:global_シナリオデモ作成_マップ名２ = 海上２)
	eval($s:global_シナリオデモ作成_マップ名３ = 海上３)
	eval($s:global_シナリオデモ作成_マップ名４ = 海上４)

/*w20a-Ｅ脚Ｂ１（狙撃イベント終了後）*/
} else if($s:エリア == w20a){
	eval($i:global_シナリオデモ作成_マップ数 = 5)
	eval($s:global_シナリオデモ作成_マップ名１ = Ｅ脚１Ｆ)
	eval($s:global_シナリオデモ作成_マップ名２ = Ｅ脚２Ｆ)
	eval($s:global_シナリオデモ作成_マップ名３ = Ｅ脚Ｂ１)
	eval($s:global_シナリオデモ作成_マップ名４ = Ｅ脚Ｂ１倉庫)
	eval($s:global_シナリオデモ作成_マップ名５ = Ｅ脚Ｂ１梯子部屋)

/*w44a-第二メタルハンガー*/
} else if($s:エリア == w44a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = 第ニメタルハンガー)

/*w45a-昇降機縦穴*/
} else if($s:エリア == w45a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = アーセナルギア小部屋Ｂ)

/*w50a-フェデラルホール（ソリダス戦）*/
} else if($s:エリア == w50a){
	eval($i:global_シナリオデモ作成_マップ数 = 1)
	eval($s:global_シナリオデモ作成_マップ名１ = ？)

}

proc null2 {}

proc pre_pro {
print 'シナリオデモプレビューヘッダ_002'
	chara シネマスクリーン シネマ \
		-top d:CINEMA_BELT_UPPER \
		-bottom d:CINEMA_BELT_LOWER \
		-ftime 60	//デフォルト値（ゼロは駄目）
	mesg シネマスクリーン シネマ フェードアウト 1
print 'シナリオデモプレビューヘッダ_003'
}

#include "p_u.h"
/*
proc play_unit {
	chara demo demo_play \
		-i t:data_dummy2 \
		-p null2 \
		-next play	\
		-begin pre_pro
}
*/

proc play {
	if($i:global_シナリオデモ作成_マップ数 == 1){
print 'シナリオデモプレビューヘッダ_004'
		command マップ設定 $s:global_シナリオデモ作成_マップ名１ -set {
			@play_unit
print 'シナリオデモプレビューヘッダ_005'
		}
	} else if($i:global_シナリオデモ作成_マップ数 == 2){
		command マップ設定	$s:global_シナリオデモ作成_マップ名１ \
							$s:global_シナリオデモ作成_マップ名２ -set {
			@play_unit
		}
	} else if($i:global_シナリオデモ作成_マップ数 == 3){
		command マップ設定	$s:global_シナリオデモ作成_マップ名１ \
							$s:global_シナリオデモ作成_マップ名２ \
							$s:global_シナリオデモ作成_マップ名３ -set {
			@play_unit
		}
	} else if($i:global_シナリオデモ作成_マップ数 == 4){
		command マップ設定	$s:global_シナリオデモ作成_マップ名１ \
							$s:global_シナリオデモ作成_マップ名２ \
							$s:global_シナリオデモ作成_マップ名３ \
							$s:global_シナリオデモ作成_マップ名４ -set {
			@play_unit
		}
	} else if($i:global_シナリオデモ作成_マップ数 == 4){
		command マップ設定	$s:global_シナリオデモ作成_マップ名１ \
							$s:global_シナリオデモ作成_マップ名２ \
							$s:global_シナリオデモ作成_マップ名３ \
							$s:global_シナリオデモ作成_マップ名４ \
							$s:global_シナリオデモ作成_マップ名５ -set {
			@play_unit
		}
	} else {
		command マップ設定	$s:global_シナリオデモ作成_マップ名１ \
							$s:global_シナリオデモ作成_マップ名２ \
							$s:global_シナリオデモ作成_マップ名３ \
							$s:global_シナリオデモ作成_マップ名４ \
							$s:global_シナリオデモ作成_マップ名５ \
							$s:global_シナリオデモ作成_マップ名６ -set {
			@play_unit
		}
	}
}

// ステージ開始時からまわす。
@play
/*
chara プロック連続実行 ボタン監視 \
	-exec {
			command パッドチェック -which 0 -press
			if ( $status & d:PAD_A ) {
				@play
				// パッドデモの再生開始処理。出来上がったら入れる。
			} else if ( $status & d:PAD_B ) {
				command StreamStopAll
				restart
			}

		} \
	-time -1
*/
#endif
