/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ゲットエマ手繋ぎ状況
extern command ゲットエマ状態
extern command ゲットエマライフ
extern command ゲットエマ座標
extern command VecLen


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_エマ＿デフォルト無線セット

//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)


proc rp_w31d_デフォルト無線セット {
	if(!$f:rfp_w31_プレイヤーが大統領部屋にいる８９０){
		@rp_大佐デフォルト無線セット
		@rp_プリスキン＿デフォルト無線セット
		@rp_ローズデフォルト無線セット
	}
	if( ( (!$f:rfp_w31_プレイヤーが大統領部屋にいる８９０) && (!$f:rfp_w31_エマが大統領部屋にいる９６２) ) || \
			( ($f:rfp_w31_プレイヤーが大統領部屋にいる８９０) && ($f:rfp_w31_エマが大統領部屋にいる９６２) ) ){
			@rp_エマ＿デフォルト無線セット
	}
}

trap r_prez_room d:PLAYER \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w31_プレイヤーが大統領部屋にいる８９０ = 1)
			command 無線設定 -allclear
			@rp_w31d_デフォルト無線セット
		}
		if($3 == 出る){
			eval($f:rfp_w31_プレイヤーが大統領部屋にいる８９０ = 0)
			command 無線設定 -allclear
			@rp_w31d_デフォルト無線セット
		}
	}

trap r_prez_room 手繋ぎエマ \
	-mask ? \
	-exec {
		if($3 == 入る){
			eval($f:rfp_w31_エマが大統領部屋にいる９６２ = 1)
print 'エマたんぶにょーん' $f:rfp_w31_エマが大統領部屋にいる９６２
			command 無線設定 -allclear
			@rp_w31d_デフォルト無線セット
		}
		if($3 == 出る){
			eval($f:rfp_w31_エマが大統領部屋にいる９６２ = 0)
print 'エマたんもへーん' $f:rfp_w31_エマが大統領部屋にいる９６２
			command 無線設定 -allclear
			@rp_w31d_デフォルト無線セット
		}
	}




/*
proc rp_エマ監視 {
	command ゲットエマ座標 $i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数
	command VecLen	$i:エマＸ座標変数 $i:エマＹ座標変数 $i:エマＺ座標変数 \
					$i:プレイヤー位置Ｘ $i:プレイヤー位置Ｙ $i:プレイヤー位置Ｚ
	
//	command ゲットエマ状態 $b:rfp_ゲットされたエマ状態
	print 'エマにうしし:' $status
}


chara プロック連続実行  エマ監視くん \
	-proc rp_エマ監視 \
	-time -1
*/

