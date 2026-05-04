/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"


@rp_大佐デフォルト無線セット
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット


	//ろかお
	// 左側上から１
	trap lk001 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:01 t:RPC_ロッカー共通１
	}

	// 左側上から２
	trap lk002 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:02 t:RPC_ロッカー共通２
	}

	// 左側上から３
	trap lk003 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:03 t:RPC_ロッカー共通１
	}

	// 左側上から４
	trap lk004 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:04 t:RPC_ロッカー共通２
	}

	// 右側上
	trap lk005 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:05 t:RPC_ロッカー共通１
	}

	// 右側下
	trap lk006 d:PLAYER \
	-mask * \
	-exec {
		@rp_ロッカー無線セット $3 ロッカー:06 t:RPC_ロッカー共通２
	}



//オウム＆エマ席
trap r_oumu d:PLAYER \
	-mask ? \
	-exec {
			print 'r_oumu'
			command 無線設定 -over $3 14085 t:RPC_w24d_エマ席付近
	}

//おたく席
trap r_otaku d:PLAYER \
	-mask ? \
	-exec {
			print 'r_otaku'
			command 無線設定 -over $3 14085 t:RPC_w24d_オタク席付近
	}

//ＡＧ扉前
trap r_AGdoor d:PLAYER \
	-mask ? \
	-exec {
			print 'r_AGdoor'
			command 無線設定 -over $3 14085 t:RPC_w24d_ＡＧ扉前
	}



