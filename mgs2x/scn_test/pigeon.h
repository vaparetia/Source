/*
    pigeon.c
    鳩
    2001/07/18 Masafumi Okuta
*/


proc フェデラルホールの鳩 {
#if d:DEBUG_PRINT
	print '*** Pigeon ***'
#endif
    chara かもめマネージャ かもめ管理 \
        // ハトモード
	-qpigeon 1 \
	// モデルデータ名
	-datamdl pgn_def, pgn_def, pgn_def, pgn_def \ 
	// 平均速度
	-zspeed 60 \
	// 基本ルーチン
	-base d:KMNG_BASE_ESCAPE \	// 基本状態:敏感
	// 鳩の名前
	-name \ 
	Pigeon00, Pigeon01, Pigeon02, Pigeon03, Pigeon04, \		// 
	Pigeon05, Pigeon06, Pigeon07, Pigeon08, Pigeon09, \		// 
	Pigeon10, Pigeon11, Pigeon12, Pigeon13, Pigeon14, \		// 
	Pigeon15, Pigeon16, Pigeon17, Pigeon18, Pigeon19, \		// 
	Pigeon20, Pigeon21, Pigeon22, Pigeon23, Pigeon24 \		// 

	// 初期Y方向 ( 0 〜 4096 : -1でランダム)
	-yrot  \
	0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0, \
	0, 0, 0, 0, 0  \

	// 初期位置指定フラグ (-fareaの何番目を初期位置として与えるか[0x00の部分が何番目に相当]）
	-unitpos \	
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \

	// エリア指定型初期位置  : 始点に当たる ([最小X, 最小Y, 最小Z] [最大X, 最大Y, 最大Z]の順番は守る)
	-farea \   
	 4000, 18000, 8000, 7000, 18000, 11000 \		

	// 飛行エリアインデックス(-areadataの何番目を飛行エリアにするか [0x00の部分が何番目に相当])
	-indexarea \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \

	// 飛行エリア : 終点にあたる ([最小X, 最小Y, 最小Z] [最大X, 最大Y, 最大Z]の順番は守る)
	-areadata \
	 3000, 19000, 24000, 8000, 25000, 35000 \		

	// 初期モーション
	-mot \
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  

	// 初期思考
	-routine \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY   

	// 飛び立ちからはじめる
//	chara delay てす -time 600 -exec { mesg かもめマネージャ かもめ管理 全かもめ緊急離陸 0 }                   
}


// 50 匹版
proc フェデラルホールの鳩_50 {
#if d:DEBUG_PRINT
	print '*** Pigeon ***'
#endif
    chara かもめマネージャ かもめ管理 \
        // ハトモード
	-qpigeon 1 \
	// モデルデータ名
	-datamdl pgn_def, pgn_def, pgn_def, pgn_def \ 
	// 平均速度
	-zspeed 60 \
	// 基本ルーチン
	-base d:KMNG_BASE_ESCAPE \	// 基本状態:敏感
	// 鳩の名前
	-name \ 
	Pigeon00, Pigeon01, Pigeon02, Pigeon03, Pigeon04, \		// 
	Pigeon05, Pigeon06, Pigeon07, Pigeon08, Pigeon09, \		// 
	Pigeon10, Pigeon11, Pigeon12, Pigeon13, Pigeon14, \		// 
	Pigeon15, Pigeon16, Pigeon17, Pigeon18, Pigeon19, \		// 
	Pigeon20, Pigeon21, Pigeon22, Pigeon23, Pigeon24 \		// 
	Pigeon25, Pigeon26, Pigeon27, Pigeon28, Pigeon29 \		// 
	Pigeon30, Pigeon31, Pigeon32, Pigeon33, Pigeon34 \		// 
	Pigeon35, Pigeon36, Pigeon37, Pigeon38, Pigeon39 \		// 
	Pigeon40, Pigeon41, Pigeon42, Pigeon43, Pigeon44 \		// 
	Pigeon45, Pigeon46, Pigeon47, Pigeon48, Pigeon49 \		// 
	Pigeon50, Pigeon51, Pigeon52, Pigeon53, Pigeon54 \		// 

	// 初期位置指定フラグ
	-unitpos \	/* 位置指定用番号 0〜32*/
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \
	(d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), (d:KMM_IPOS_AREA | 0x00), \

	// エリア指定型初期位置
	-farea \   
	 4000, 18000, 8000, 7000, 18000, 11000 \		

	// 飛行エリアインデックス
	-indexarea \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \
	(d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), (d:KMM_FLYA_BOUND | 0x00), \

	// 飛行エリア : 終点にあたる ([最小X, 最小Y, 最小Z] [最大X, 最大Y, 最大Z]の順番は守る)
	-areadata \
	 3000, 19000, 24000, 8000, 25000, 35000 \		

	// 初期モーション
	-mot \
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  
	d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, d:KMM_MOT_FLYAWAY_QUICK, \  

	// 初期思考 : いきなり飛び立ちに
	-routine \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \ 
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, \
	d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY, d:KMM_INITTHK_FLYAWAY 

//	chara delay てす -time 600 -exec { mesg かもめマネージャ かもめ管理 全かもめ緊急離陸 10 }                   
}

