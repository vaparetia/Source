/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"
extern command ＮＰＣスネークのライフ
extern command ＮＰＣスネークの状態
extern command コントロール座標取得

@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_オタコン＿デフォルト無線セット

//eval($w:武器弾数Ｒ[6] = -1)
//eval($f:rfp_PS_リモコンミサイルの場所聞いた = 1)


trap r_south_door d:PLAYER \
	-mask ? \
	-exec {
		print 'r_south_door'
		command 無線設定 -over $3 14112 t:RPO_w44a_南開かない扉
	}

/*
trap r_south_door d:PLAYER \
	-mask いる \
	-exec {
		command コントロール座標取得 ＮＰＣスネーク $i:ＮＰＣスネークＸ座標 $i:ＮＰＣスネークＹ座標 $i:ＮＰＣスネークＺ座標
		print 'すねすねＺ:' $i:ＮＰＣスネークＺ座標
	}
*/

trap r_north_door d:PLAYER \
	-mask ? \
	-exec {
		print 'r_north_door'
		command 無線設定 -over $3 14112 t:RPO_w44a_天狗通路ドア扉
	}



/*
proc rp_スネーク状態監視太郎 {
	command ＮＰＣスネークの状態
	print 'どらばっとぼんＮＰＣスネークの状態:' $status
}

chara プロック連続実行  スネーク状態監視太郎 \
	-proc rp_スネーク状態監視太郎 \
	-time -1
*/
