proc rp_vc270241 {
	command 無線設定 -call 14112 t:RTO_vc270241 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270441 {
	command 無線設定 -call 14112 t:RTO_vc270441 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270461 {
	command 無線設定 -call 14112 t:RTO_vc270461 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270471 {
	command 無線設定 -call 14112 t:RTO_vc270471 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270541 {
	command 無線設定 -call 14112 t:RTO_vc270541 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270551 {
	command 無線設定 -call 14112 t:RTO_vc270551 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270631 {
	command 無線設定 -call 14112 t:RTO_vc270631 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270641 {
	command 無線設定 -call 14112 t:RTO_vc270641 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270651 {
	command 無線設定 -call 14112 t:RTO_vc270651 d:CODEC_DIRECT 終わり_p
}

proc rp_vc270711 {
	command 無線設定 -call 14112 t:RTO_vc270711 d:CODEC_DIRECT 終わり_p
}

proc rp_vc272491 {
	command 無線設定 -call 14112 t:RTO_vc272491 d:CODEC_DIRECT 終わり_p
}

proc rp_vc274681 {
	command 無線設定 -call 14112 t:RTO_vc274681 d:CODEC_DIRECT 終わり_p
}

proc rp_vc275131 {
	command 無線設定 -call 14112 t:RTO_vc275131 d:CODEC_DIRECT 終わり_p
}

proc rp_vc275141 {
	command 無線設定 -call 14112 t:RTO_vc275141 d:CODEC_DIRECT 終わり_p
}

proc rp_vc275181 {
	command 無線設定 -call 14112 t:RTO_vc275181 d:CODEC_DIRECT 終わり_p
}

proc codec_set_p_vc270241_vc270651 {
	chara select vc270241_vc270651 -s {
		'vc270241' rp_vc270241
		'vc270441' rp_vc270441
		'vc270461' rp_vc270461
		'vc270471' rp_vc270471
		'vc270541' rp_vc270541
		'vc270551' rp_vc270551
		'vc270631' rp_vc270631
		'vc270641' rp_vc270641
		'vc270651' rp_vc270651
		'back' back_to_root_p_colonel_diver
		'exit' exit_codec_debug_mode_p_colonel_diver
	}
}

proc codec_set_p_vc270711_vc275181 {
	chara select vc270711_vc275181 -s {
		'vc270711' rp_vc270711
		'vc272491' rp_vc272491
		'vc274681' rp_vc274681
		'vc275131' rp_vc275131
		'vc275141' rp_vc275141
		'vc275181' rp_vc275181
		'back' back_to_root_p_colonel_diver
		'exit' exit_codec_debug_mode_p_colonel_diver
	}
}

proc codec_set_root_p_colonel_diver {
	chara select root -s {
		'vc270241 vc270651' enter_vc270241_vc270651
		'vc270711 vc275181' enter_vc270711_vc275181
		'back' exit_codec_debug_mode_p_colonel_diver
	}
}

proc enter_vc270241_vc270651 {
	eval($f:rfp_enter_vc270241_vc270651 = 1)
	restart
}

proc enter_vc270711_vc275181 {
	eval($f:rfp_enter_vc270711_vc275181 = 1)
	restart
}

proc codec_debug_list_p_colonel_diver {
	if($f:rfp_enter_vc270241_vc270651){
		@codec_set_p_vc270241_vc270651

	}else if($f:rfp_enter_vc270711_vc275181){
		@codec_set_p_vc270711_vc275181

	}else {
		@clear_flag_p_colonel_diver
		@codec_set_root_p_colonel_diver
	}
}

proc clear_flag_p_colonel_diver {
	eval($f:rfp_enter_vc270241_vc270651 = 0)
	eval($f:rfp_enter_vc270711_vc275181 = 0)
}

proc exit_codec_debug_mode_p_colonel_diver {
	@clear_flag_p_colonel_diver
	eval($f:rfp_enter_codec_debug_mode_p_colonel_diver = 0)
	restart
}

proc back_to_root_p_colonel_diver {
	@clear_flag_p_colonel_diver
	restart
}

