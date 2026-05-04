/**************************************************
		無線でぽいいん
**************************************************/

#include "cdc_common_p.h"

extern command ゲット敵兵状態
extern command ゲット敵兵状態拡張版

chara 携帯端末 くそメール -skin htl -file t:mobile

//@rp_大佐デフォルト無線セット
@rpc_大佐＿通常無線セット＿連絡橋
@rp_プリスキン＿デフォルト無線セット
@rp_ローズデフォルト無線セット



//うんこ
trap r_unko d:PLAYER \
	-mask ? \
	-exec {
		print 'r_unko'
		command 無線設定 -over $3 14085 t:RPC_うんこ
	}



//ＤＧ連絡橋のＤ側。亀裂超える前
trap r_DG_D d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_DG_D in'
			eval($s:rfp_w25b_いまいるところ = "DG_D")
		}

trap r_DG_G d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_DG_G in'
			eval($s:rfp_w25b_いまいるところ = "DG_G")
		}

trap r_DG_G_pipe d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_DG_G_pipe'
			eval($s:rfp_w25b_いまいるところ = "DG_G_pipe")
		}


trap r_G d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_G in'
			eval($s:rfp_w25b_いまいるところ = "G")
		}


trap r_G_LG d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_G_LG in'
			eval($s:rfp_w25b_いまいるところ = "G_LG")
		}

trap r_LG d:PLAYER \
	-mask 入る \
	-exec {
			print 'r_LG in'
			eval($s:rfp_w25b_いまいるところ = "LG")
		}



proc rp_炎無線セット $:出る入るその他 $:そこの炎発生した $:無線シンボル {
	if( ($:出る入るその他 == いる ) || ($:出る入るその他 == 入る ) ){
		eval($f:rfp_そこの炎発生した = $:そこの炎発生した)
	}
	if( ($:出る入るその他 == 入る) || ($:出る入るその他 == 出る) ){
		print 'r_fire'
		command 無線設定 -over $:出る入るその他 14085 $:無線シンボル
	}
}


trap r_fire678 d:PLAYER \
	-mask * \
	-exec {
		if( ($f:ステージ置き炎燃焼６) || ($f:ステージ置き炎燃焼７) ||  ($f:ステージ置き炎燃焼８) ){
			@rp_炎無線セット $3 1 t:RPC_炎近く
		}else {
			@rp_炎無線セット $3 0 t:RPC_炎近く
		}
	}


trap r_fire123 d:PLAYER \
	-mask * \
	-exec {
		if( ($f:ステージ置き炎燃焼１) || ($f:ステージ置き炎燃焼２) ||  ($f:ステージ置き炎燃焼３) ){
			@rp_炎無線セット $3 1 t:RPC_炎近く
		}else {
			@rp_炎無線セット $3 0 t:RPC_炎近く
		}
	}

proc PHONE_消火終了 {
	print 'PHONE_消火終了'
	eval($f:rfp_PHONE_消火聞いた = 1)
	command varsave $f:rfp_PHONE_消火聞いた
}

trap r_fire678 d:PLAYER \
	-mask ? \
	-exec {
		if( ( ($f:ステージ置き炎燃焼６) && ($f:ステージ置き炎燃焼７) &&  ($f:ステージ置き炎燃焼８) ) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				($3 == 入る) && (!$f:rfp_PHONE_消火ＣＡＬＬした) && (!$f:rfp_PHONE_消火聞いた) ){
			eval($f:rfp_PHONE_消火ＣＡＬＬした = 1)
			command 携帯呼出 -call t:PHONE_消火 -p PHONE_消火終了
		}
		if($3 == 出る) {
			if($f:rfp_PHONE_消火ＣＡＬＬした){
				command 携帯呼出 -reset
				eval($f:rfp_PHONE_消火ＣＡＬＬした = 0)
			}
		}
	}

trap r_fire123 d:PLAYER \
	-mask ? \
	-exec {
		if( ( ($f:ステージ置き炎燃焼１) && ($f:ステージ置き炎燃焼２) &&  ($f:ステージ置き炎燃焼３) ) && \
				( ($f:ステージ置き炎燃焼６) && ($f:ステージ置き炎燃焼７) &&  ($f:ステージ置き炎燃焼８) ) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				($3 == 入る) && (!$f:rfp_PHONE_消火ＣＡＬＬした) && (!$f:rfp_PHONE_消火聞いた) ){
			eval($f:rfp_PHONE_消火ＣＡＬＬした = 1)
			command 携帯呼出 -call t:PHONE_消火 -p PHONE_消火終了
		}
		if($3 == 出る) {
			if($f:rfp_PHONE_消火ＣＡＬＬした){
				command 携帯呼出 -reset
				eval($f:rfp_PHONE_消火ＣＡＬＬした = 0)
			}
		}
	}



trap r_fire_later d:PLAYER \
	-mask * \
	-exec {
			@rp_炎無線セット $3 $f:侵入禁止炎設置済み t:RPC_炎近く
	}




if($f:ステージ置き炎燃焼４){}
if($f:ステージ置き炎燃焼５){}
if($f:ステージ置き炎燃焼９){}

/*
trap r_fire_later d:PLAYER \
	-mask ? \
	-exec {
		if( ( ($f:ステージ置き炎燃焼１) && ($f:ステージ置き炎燃焼２) &&  ($f:ステージ置き炎燃焼３) ) && \
				( ($f:ステージ置き炎燃焼６) && ($f:ステージ置き炎燃焼７) &&  ($f:ステージ置き炎燃焼８) ) && \
				(`@rp_強制ＣＡＬＬしてもいいよ`) && \
				($3 == 入る) && (!$f:rfp_PHONE_消火ＣＡＬＬした) && (!$f:rfp_PHONE_消火聞いた) ){
			eval($f:rfp_PHONE_消火ＣＡＬＬした = 1)
			command 携帯呼出 -call t:PHONE_消火 -p PHONE_消火終了
		}
		if($3 == 出る) {
			if($f:rfp_PHONE_消火ＣＡＬＬした){
				command 携帯呼出 -reset
				eval($f:rfp_PHONE_消火ＣＡＬＬした = 0)
			}
		}
	}
*/

//ズバリヒント計算
if(!$f:rfp_w25b_ズバリヒントカウントスタートした){
	eval($f:rfp_w25b_ズバリヒントカウントスタートした = 1)
	eval($i:イベント開始時刻 = $i:プレイタイム)
	command varsave $i:イベント開始時刻
}

trap r_rolling d:PLAYER \
	-mask ? \
	-exec {
			//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
			print 'r_rolling'
			command 無線設定 -over $3 14085 t:RPC_ＤＧ連絡橋＿渡り方飛びズバリ
	}

trap r_elude d:PLAYER \
	-mask ? \
	-exec {
		//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
			print 'r_elude'
			command 無線設定 -over $3 14085 t:RPC_ＤＧ連絡橋＿渡り方エルードズバリ
	}


trap r_pit_fa001_s d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床手前２ t:RPC_落し穴共通１
	}

trap r_pit_fa001_n d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床手前１ t:RPC_落し穴共通２
	}

trap r_pit_fa002_s d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床中央２ t:RPC_落し穴共通１
	}

trap r_pit_fa002_n d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床中央１ t:RPC_落し穴共通２
	}

trap r_pit_fa003_s d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床奥２ t:RPC_落し穴共通１
	}

trap r_pit_fa003_n d:PLAYER \
	-mask * \
	-exec {
		@rp_落とし穴無線セット $3 崩落床奥１ t:RPC_落し穴共通２
	}


trap r_hashigo d:PLAYER \
	-mask ? \
	-exec {
		//ズバリヒント出す処理。時間？ＸＸＸＸＸＸＸＸＸＸｘｘ
			print 'r_hashigo'
			command 無線設定 -over $3 14085 t:RPC_ＬＧ連絡橋＿ハシゴ
	}


