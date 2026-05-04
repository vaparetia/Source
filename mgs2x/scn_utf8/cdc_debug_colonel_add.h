proc rp_vc266901 {
	command 無線設定 -call 14112 t:RTO_vc266901 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266911 {
	command 無線設定 -call 14112 t:RTO_vc266911 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266941 {
	command 無線設定 -call 14112 t:RTO_vc266941 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266951 {
	command 無線設定 -call 14112 t:RTO_vc266951 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266961 {
	command 無線設定 -call 14112 t:RTO_vc266961 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266971 {
	command 無線設定 -call 14112 t:RTO_vc266971 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266981 {
	command 無線設定 -call 14112 t:RTO_vc266981 d:CODEC_DIRECT 終わり_p
}

proc rp_vc266991 {
	command 無線設定 -call 14112 t:RTO_vc266991 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267001 {
	command 無線設定 -call 14112 t:RTO_vc267001 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267011 {
	command 無線設定 -call 14112 t:RTO_vc267011 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267021 {
	command 無線設定 -call 14112 t:RTO_vc267021 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267031 {
	command 無線設定 -call 14112 t:RTO_vc267031 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267041 {
	command 無線設定 -call 14112 t:RTO_vc267041 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267051 {
	command 無線設定 -call 14112 t:RTO_vc267051 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267061 {
	command 無線設定 -call 14112 t:RTO_vc267061 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267071 {
	command 無線設定 -call 14112 t:RTO_vc267071 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267081 {
	command 無線設定 -call 14112 t:RTO_vc267081 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267091 {
	command 無線設定 -call 14112 t:RTO_vc267091 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267101 {
	command 無線設定 -call 14112 t:RTO_vc267101 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267111 {
	command 無線設定 -call 14112 t:RTO_vc267111 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267121 {
	command 無線設定 -call 14112 t:RTO_vc267121 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267131 {
	command 無線設定 -call 14112 t:RTO_vc267131 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267141 {
	command 無線設定 -call 14112 t:RTO_vc267141 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267151 {
	command 無線設定 -call 14112 t:RTO_vc267151 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267161 {
	command 無線設定 -call 14112 t:RTO_vc267161 d:CODEC_DIRECT 終わり_p
}

proc rp_vc267171 {
	command 無線設定 -call 14112 t:RTO_vc267171 d:CODEC_DIRECT 終わり_p
}

proc codec_set_p_vc266901_vc267001 {
	chara select vc266901_vc267001 -s {
		'vc266901' rp_vc266901
		'vc266911' rp_vc266911
		'vc266941' rp_vc266941
		'vc266951' rp_vc266951
		'vc266961' rp_vc266961
		'vc266971' rp_vc266971
		'vc266981' rp_vc266981
		'vc266991' rp_vc266991
		'vc267001' rp_vc267001
		'back' back_to_root_p_colonel_add
		'exit' exit_codec_debug_mode_p_colonel_add
	}
}

proc codec_set_p_vc267011_vc267091 {
	chara select vc267011_vc267091 -s {
		'vc267011' rp_vc267011
		'vc267021' rp_vc267021
		'vc267031' rp_vc267031
		'vc267041' rp_vc267041
		'vc267051' rp_vc267051
		'vc267061' rp_vc267061
		'vc267071' rp_vc267071
		'vc267081' rp_vc267081
		'vc267091' rp_vc267091
		'back' back_to_root_p_colonel_add
		'exit' exit_codec_debug_mode_p_colonel_add
	}
}

proc codec_set_p_vc267101_vc267171 {
	chara select vc267101_vc267171 -s {
		'vc267101' rp_vc267101
		'vc267111' rp_vc267111
		'vc267121' rp_vc267121
		'vc267131' rp_vc267131
		'vc267141' rp_vc267141
		'vc267151' rp_vc267151
		'vc267161' rp_vc267161
		'vc267171' rp_vc267171
		'back' back_to_root_p_colonel_add
		'exit' exit_codec_debug_mode_p_colonel_add
	}
}

proc codec_set_root_p_colonel_add {
	chara select root -s {
		'vc266901 vc267001' enter_vc266901_vc267001
		'vc267011 vc267091' enter_vc267011_vc267091
		'vc267101 vc267171' enter_vc267101_vc267171
		'back' exit_codec_debug_mode_p_colonel_add
	}
}

proc enter_vc266901_vc267001 {
	eval($f:rfp_enter_vc266901_vc267001 = 1)
	restart
}

proc enter_vc267011_vc267091 {
	eval($f:rfp_enter_vc267011_vc267091 = 1)
	restart
}

proc enter_vc267101_vc267171 {
	eval($f:rfp_enter_vc267101_vc267171 = 1)
	restart
}

proc codec_debug_list_p_colonel_add {
	if($f:rfp_enter_vc266901_vc267001){
		@codec_set_p_vc266901_vc267001

	}else if($f:rfp_enter_vc267011_vc267091){
		@codec_set_p_vc267011_vc267091

	}else if($f:rfp_enter_vc267101_vc267171){
		@codec_set_p_vc267101_vc267171

	}else {
		@clear_flag_p_colonel_add
		@codec_set_root_p_colonel_add
	}
}

proc clear_flag_p_colonel_add {
	eval($f:rfp_enter_vc266901_vc267001 = 0)
	eval($f:rfp_enter_vc267011_vc267091 = 0)
	eval($f:rfp_enter_vc267101_vc267171 = 0)
}

proc exit_codec_debug_mode_p_colonel_add {
	@clear_flag_p_colonel_add
	eval($f:rfp_enter_codec_debug_mode_p_colonel_add = 0)
	restart
}

proc back_to_root_p_colonel_add {
	@clear_flag_p_colonel_add
	restart
}

