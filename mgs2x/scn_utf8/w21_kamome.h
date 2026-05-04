/*
	w21_kamome.h
	    ＥＦ連絡橋かもめ

	2001/05/17 S.Mukaide
	$Id: w21_kamome.h,v 1.7 2002/05/09 13:07:50 usr01475 Exp $                      

	
*/



	proc かもめ設定 {
		chara かもめマネージャ かもめ管理人 \ 
			-datamdl \
				kmo_noalp, kmo_low, kmo_low, kmo_low \
//				kmo_low kmo_low kmo_obj_stop kmo_obj_flying \
			-name \

			かもめ１	かもめ２	かもめ３	かもめ４ \
			かもめ５	かもめ６	かもめ７	かもめ８ \
			かもめ９	かもめ１０	かもめ１１	かもめ１２ \
			かもめ１３	かもめ１４	かもめ１５	かもめ１６ \

			-unitpos \	/* 位置指定用番号 0～32*/
			0, 0, 0, 0 \
			0, 0, 0, 0 \
			0, 0, 0, 0 \
			0, 0, 0, 0 \

			-farea \
			25000,-1500,-60500	42000,-1500,-59500 \

			-traparea \
			ka001 \

			-indexarea \
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \

//	インテグラル以外
#ifndef d:ANOTHER
			-base \
			d:KMNG_BASE_ESCAPE \
			-wdmgproc rp_かもめ殺しやがった

//	インテグラル
#else
			-base \
			d:KMNG_BASE_ESCAPE
#endif

		//	落し穴がどれか落ちている場合は飛んでいる
#if 0
		if ( ($f:w21a_落し穴１Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴１Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴２Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴２Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴３Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴３Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴４Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴４Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴５Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴５Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴６Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴６Ｂ開いたフラグ == d:TRUE) || \
			($f:w21a_落し穴７Ａ開いたフラグ == d:TRUE) || ($f:w21a_落し穴７Ｂ開いたフラグ == d:TRUE) ) {
				mesg かもめマネージャ かもめ管理人 全かもめ飛行 0
		}
#else
		if ($f:w21a_落し穴１Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴１Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴２Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴２Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴３Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴３Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴４Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴４Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴５Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴５Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴６Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴６Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴７Ａ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0

		} else if ($f:w21a_落し穴７Ｂ開いたフラグ == d:TRUE) {
			mesg かもめマネージャ かもめ管理人 全かもめ飛行 0
		}
#endif

	}

