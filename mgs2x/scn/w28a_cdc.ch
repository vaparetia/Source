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



	trap r_no_open_door d:PLAYER \
		-mask ? \
		-exec {
			print 'r_no_open_door'
				command 無線設定 -over $3 14085 t:RPC_w28a_開かないドア
		}

