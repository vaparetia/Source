/*
	w23_kamome.h
	    ＦＡ連絡橋かもめ

	2001/05/17 S.Mukaide
	$Id: w23_kamome.h,v 1.5 2002/05/09 13:07:50 usr01475 Exp $                      

	
*/



	proc かもめ設定 {
		//	朝のみ
		#if d:W23A
		chara かもめマネージャ かもめ管理人 \ 
			-xlod 3000 \
			-datamdl \
				kmo_low kmo_low kmo_obj_stop kmo_obj_flying \
			-name \
			かもめ１	かもめ２	かもめ３	かもめ４ \
			かもめ５	かもめ６ \

			-unitpos \	/* 位置指定用番号 0〜32*/
			//	Ａ脚屋上
			0, 0, 0, 0 \
			0, 0 \

			-farea \
			-22000,11000,-9000	-11000,15000,9000 \

			-traparea \
			ka001 \

			-indexarea \
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \
//	インテグラル以外
#ifndef d:ANOTHER
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \
			-wdmgproc rp_かもめ殺しやがった

//	インテグラル
#else
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00)
#endif

		mesg かもめマネージャ かもめ管理人 全かもめ飛行 0
		#endif
	}

