/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

//@rp_大佐デフォルト無線セット
@rpc_大佐＿通常無線セット＿連絡橋
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット
@rp_ピーター＿デフォルト無線セット

//eval($w:アイテム数Ｒ[11] = -1)

trap by005 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap by006 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap by007 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap by008 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}

trap by009 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}


trap r_death_byond d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_奈落エルード
	}


trap by002 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap by003 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}
trap by004 d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}

trap r_live_byond d:PLAYER \
	-mask * \
	-exec {
		@rp_エルード＿無線セット $3 t:RPC_ただのエルード
	}



