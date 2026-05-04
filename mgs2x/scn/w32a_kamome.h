/*
	w32a_kamome.h
	    狙撃イベントかもめ

	2001/05/11 S.Mukaide
	$Id: w32a_kamome.h,v 1.16 2001/09/06 06:33:48 usr03379 Exp $                      

	
*/
        proc かもめダメージ{ // かもめが攻撃を受けたら呼ばれる
		@rp_かもめ殺しやがった $1

#if 0
	    // かもめが20匹攻撃されたら反撃をさせる
	    eval( $i:w32a_かもめやられ数 = $i:w32a_かもめやられ数 + 1 )
	    print ' **** kamome damage **** '
	    if ( $i:w32a_かもめやられ数 == 20 ){
		eval( $i:w32a_かもめやられ数 = 0 )
		mesg かもめマネージャ かもめ管理人 指定数攻撃 PLAYER 100 
	    }
#endif
        }


	proc かもめ設定 {
//	    eval( $i:w32a_かもめやられ数 = 0 )

		chara かもめマネージャ かもめ管理人 \ 
		        -xlod 30000 \
			-datamdl \
				kmo_low kmo_low kmo_obj_stop kmo_obj_flying \
			-name \
			//	浮き橋１
			かもめ１	かもめ２	かもめ３	かもめ４ \

			//	浮き橋２
			かもめ５	かもめ６	かもめ７	かもめ８ \
			かもめ９	かもめ１０	かもめ１１	かもめ１２ \

			//	浮き橋３
			かもめ１３	かもめ１４	かもめ１５	かもめ１６ \
		    
		        -wdmgproc かもめダメージ \ 

			-unitpos \	/* 位置指定用番号 0〜32*/
			//	浮き橋１
			0, 0, 0, 0 \

			//	浮き橋２
			1, 1, 1, 1 \
			1, 1, 1, 1 \

			//	浮き橋３
			2, 2, 2, 2 \

			-linepos \	/* 初期位置線指定 */
			//	浮き橋１
			93000,-35000,-210000 93000,-35000,-172000 \

			//	浮き橋２
			100000,-35000,-170000 100000,-35000,-137500 \

			//	浮き橋３
			62500,-35000,-103750 95000,-35000,-120000 \

			-traparea \
			//	浮き橋１
		 	 ka001 \

			//	浮き橋２
		 	 ka002 \

			//	浮き橋３
		 	 ka003,\

		-indexarea \
			//	浮き橋１
			(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00),(d:KMM_FLYA_TRAP | 0x00) \

			//	浮き橋２
			(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01) \
			(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01),(d:KMM_FLYA_TRAP | 0x01) \

			//	浮き橋３
			(d:KMM_FLYA_TRAP | 0x02),(d:KMM_FLYA_TRAP | 0x02),(d:KMM_FLYA_TRAP | 0x02),(d:KMM_FLYA_TRAP | 0x02) \
		
		-eraseh		-40500 \
		-mot \
		d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, \
		d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, \
		d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, d:KMM_MOT_FLYING, \
		d:KMM_MOT_FLYING  \

		-routine \
		d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, \
		d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, \
		d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, d:KMM_INITTHK_FLYING, \
		d:KMM_INITTHK_FLYING

//		mesg かもめマネージャ かもめ管理人 全かもめ飛行 0
	}
