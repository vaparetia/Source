/*
	a22a_tale_enemy.h
		Ｆ涤烈杆(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/17 H.Yoshiike
	$Id: a22a_tale_enemy.h,v 1.5 2002/09/17 12:35:28 usr03682 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	ＦＡ１Ｆ略怠Ｐ = 0,
	ＦＡＢ１略怠Ｐ,
	ＥＦ略怠Ｐ,
	Ａ脱１ＦＥ烈杆戒搀Ｒ,
	Ａ脱１Ｆ祁捏绩息晚戒搀Ｒ,
	Ａ脱Ｂ１面丙戒搀Ｒ,
	Ａ脱Ｂ１Ｇ烈杆戒搀Ｒ,
	Ｂ脱１Ｆ面丙戒搀Ｒ,
	Ｂ脱１Ｆ颂檄布戒搀Ｒ,
	Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ,
	Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ,
	Ｂ脱１Ｆ澎纳裁Ｒ,
	Ｂ脱Ｂ１祁澎纳裁Ｒ,
	Ｄ脱１Ｆ面丙戒搀Ｒ,
	Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ,
	Ｄ脱Ｂ１祁谰戒搀Ｒ,

	//	テイルズＢ脱纳裁尸
	Ｂ脱１Ｆ澎染尸戒搀Ｒ = 20,
	Ｂ脱１ＦＢ１超檬戒搀Ｒ,			//	墓め
	Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ,
	Ｂ脱１ＦＢ１超檬戒搀２Ｒ		//	没め
}

//---------------
// 浓始ステ〖タス
//---------------
//	d:ENE_STATUS_NPCは焚洒始のみ铜跟。苟封始はエマしかいないときはエマを、それ笆嘲はスネ〖クを苟封する
#define		ENE_STATUS_A22A_N	d:ENE_STATUS_GUNLIGHT
#define		ENE_STATUS_A22A_R	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT)
#define		ENE_STATUS_A22A_E	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_NPC)


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_ＦＡ息晚抖１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱１ＦＥ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱１Ｆ祁捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱Ｂ１面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ａ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＦＡ息晚抖Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱１ＦＥ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱１Ｆ祁捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱Ｂ１面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ａ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＥＦ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱１ＦＥ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱１Ｆ祁捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱Ｂ１面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ａ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ダンボ〖ルから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱１ＦＥ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱１Ｆ祁捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱Ｂ１面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ａ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}


proc 焚洒始セット_ＦＡ息晚抖１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１Ｆ颂檄布戒搀Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＦＡ息晚抖Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１Ｆ颂檄布戒搀Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＥＦ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１Ｆ颂檄布戒搀Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ダンボ〖ルから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１Ｆ颂檄布戒搀Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}


proc 焚洒始セット_ＦＡ息晚抖１Ｆから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif
#if 0

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１Ｆ颂檄布戒搀Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷
#else
	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱Ｂ１祁谰戒搀Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１面丙弓眷
#endif

}

proc 焚洒始セット_ＦＡ息晚抖Ｂ１から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱Ｂ１祁谰戒搀Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＥＦ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱Ｂ１祁谰戒搀Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ダンボ〖ルから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱Ｂ１祁谰戒搀Ｒ 0 d:ENE_STATUS_A22A_N 50500,-5000,-26000 Ｂ１面丙弓眷
}



//	笆布テイルズＢ脱纳裁
proc 焚洒始セット_ＦＡ息晚抖１Ｆから_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＦＡ息晚抖Ｂ１から_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＥＦ息晚抖から_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ダンボ〖ルから_４ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}


proc 焚洒始セット_ＦＡ息晚抖１Ｆから_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＦＡ息晚抖Ｂ１から_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ＥＦ息晚抖から_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}

proc 焚洒始セット_ダンボ〖ルから_エマ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ澎染尸戒搀Ｒ 0 d:ENE_STATUS_A22A_N 51250,0,-26000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱１Ｆ面丙戒搀Ｒ 0 d:ENE_STATUS_A22A_E 51250,0,-26000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀Ｒ 0 d:ENE_STATUS_A22A_N 54500,-5000,-30750 Ｂ１面丙弓眷
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱１ＦＢ１超檬戒搀２Ｒ 0 d:ENE_STATUS_A22A_N 52000,0,-42000 １Ｆ面丙弓眷

//	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱１Ｆ谰染尸年箕息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 52000,0,-42000 １Ｆ面丙弓眷
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱Ｂ１面丙捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A22A_R 54500,-5000,-30750 Ｂ１面丙弓眷

	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱Ｂ１Ｇ烈杆戒搀Ｒ 0 d:ENE_STATUS_A22A_N 59250,-5000,-32000 Ｂ１面丙弓眷
}


//	笆布テイルズＤ脱纳裁
proc 焚洒始セット_テイルズＤ倡幌木稿 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱Ｂ１面丙捏绩息晚戒搀Ｒ 6 d:ENE_STATUS_A22A_R 59250,-5000,-32000 Ｂ１面丙弓眷
}

//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_ＦＡ息晚抖１Ｆから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:62 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＦＡ息晚抖Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:62 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＥＦ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 苟封始セット_ＦＡ息晚抖１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_玫瑚苟封始 浓始:61 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱１Ｆ澎纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱Ｂ１祁澎纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＦＡ息晚抖Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_玫瑚苟封始 浓始:61 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱１Ｆ澎纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱Ｂ１祁澎纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＥＦ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_玫瑚苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱１Ｆ澎纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱Ｂ１祁澎纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 苟封始セット_ＦＡ息晚抖１Ｆから_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡ１Ｆ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＦＡ息晚抖Ｂ１から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＦＡＢ１略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＥＦ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_苟封始 浓始:64 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A22A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


