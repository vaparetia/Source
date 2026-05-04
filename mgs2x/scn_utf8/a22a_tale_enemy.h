/*
	a22a_tale_enemy.h
		Ｆ脚倉庫(スネークテイルズ用敵兵設定ファイル)

	2002/06/17 H.Yoshiike
	$Id: a22a_tale_enemy.h,v 1.5 2002/09/17 12:35:28 usr03682 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＦＡ１Ｆ待機Ｐ = 0,
	ＦＡＢ１待機Ｐ,
	ＥＦ待機Ｐ,
	Ａ用１ＦＥ倉庫巡回Ｒ,
	Ａ用１Ｆ南提示連絡巡回Ｒ,
	Ａ用Ｂ１中央巡回Ｒ,
	Ａ用Ｂ１Ｇ倉庫巡回Ｒ,
	Ｂ用１Ｆ中央巡回Ｒ,
	Ｂ用１Ｆ北廊下巡回Ｒ,
	Ｂ用Ｂ１中央提示連絡巡回Ｒ,
	Ｂ用Ｂ１Ｇ倉庫巡回Ｒ,
	Ｂ用１Ｆ東追加Ｒ,
	Ｂ用Ｂ１南東追加Ｒ,
	Ｄ用１Ｆ中央巡回Ｒ,
	Ｄ用Ｂ１中央提示連絡巡回Ｒ,
	Ｄ用Ｂ１南西巡回Ｒ,

	//	テイルズＢ用追加分
	Ｂ用１Ｆ東半分巡回Ｒ = 20,
	Ｂ用１ＦＢ１階段巡回Ｒ,			//	長め
	Ｂ用１Ｆ西半分定時連絡巡回Ｒ,
	Ｂ用１ＦＢ１階段巡回２Ｒ		//	短め
}

//---------------
// 敵兵ステータス
//---------------
//	d:ENE_STATUS_NPCは警備兵のみ有効。攻撃兵はエマしかいないときはエマを、それ以外はスネークを攻撃する
#define		ENE_STATUS_A22A_N	d:ENE_STATUS_GUNLIGHT
#define		ENE_STATUS_A22A_R	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT)
#define		ENE_STATUS_A22A_E	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_NPC)


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_ＦＡ連絡橋１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用１ＦＥ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ａ用１Ｆ南提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ａ用Ｂ１中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ａ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＦＡ連絡橋Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用１ＦＥ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ａ用１Ｆ南提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ａ用Ｂ１中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ａ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＥＦ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用１ＦＥ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ａ用１Ｆ南提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ａ用Ｂ１中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ａ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ダンボールから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用１ＦＥ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ａ用１Ｆ南提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ａ用Ｂ１中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ａ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}


proc 警備兵セット_ＦＡ連絡橋１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１Ｆ北廊下巡回Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＦＡ連絡橋Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１Ｆ北廊下巡回Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＥＦ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１Ｆ北廊下巡回Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ダンボールから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１Ｆ北廊下巡回Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}


proc 警備兵セット_ＦＡ連絡橋１Ｆから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif
#if 0

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１Ｆ北廊下巡回Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場
#else
	@A_警備兵 敵兵:01 d:T_ER:Ｄ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用Ｂ１南西巡回Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１中央広場
#endif

}

proc 警備兵セット_ＦＡ連絡橋Ｂ１から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用Ｂ１南西巡回Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１中央広場
}

proc 警備兵セット_ＥＦ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用Ｂ１南西巡回Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１中央広場
}

proc 警備兵セット_ダンボールから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用Ｂ１南西巡回Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１中央広場
}



//	以下テイルズＢ用追加
proc 警備兵セット_ＦＡ連絡橋１Ｆから_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＦＡ連絡橋Ｂ１から_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＥＦ連絡橋から_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ダンボールから_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}


proc 警備兵セット_ＦＡ連絡橋１Ｆから_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＦＡ連絡橋Ｂ１から_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ＥＦ連絡橋から_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}

proc 警備兵セット_ダンボールから_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ東半分巡回Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ中央広場
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用１Ｆ中央巡回Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ中央広場

//	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１中央広場
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用１ＦＢ１階段巡回２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ中央広場

//	@A_警備兵 敵兵:03 d:T_ER:Ｂ用１Ｆ西半分定時連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ中央広場
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用Ｂ１中央提示連絡巡回Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１中央広場

	@A_警備兵 敵兵:04 d:T_ER:Ｂ用Ｂ１Ｇ倉庫巡回Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１中央広場
}


//	以下テイルズＤ用追加
proc 警備兵セット_テイルズＤ開始直後 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用Ｂ１中央提示連絡巡回Ｒ 6 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１中央広場
}

//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＦＡ連絡橋１Ｆから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:62 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＦＡ連絡橋Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:62 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＥＦ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 攻撃兵セット_ＦＡ連絡橋１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_探索攻撃兵 敵兵:61 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用１Ｆ東追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用Ｂ１南東追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＦＡ連絡橋Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_探索攻撃兵 敵兵:61 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用１Ｆ東追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用Ｂ１南東追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＥＦ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_探索攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用１Ｆ東追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用Ｂ１南東追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 攻撃兵セット_ＦＡ連絡橋１Ｆから_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡ１Ｆ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＦＡ連絡橋Ｂ１から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＦＡＢ１待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＥＦ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポート攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_攻撃兵 敵兵:64 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


