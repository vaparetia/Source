/*
	a19a_tale_enemy.h
	    ＤＥ息晚抖(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/13 S.Mukaide
	$Id: a19a_tale_enemy.h,v 1.6 2002/07/02 04:40:23 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	Ｅ涤略怠Ｐ = 0,
	Ｄ涤１Ｆ略怠Ｐ,
	Ｄ涤Ｂ１略怠Ｐ,

	息晚抖谰１ＦＢ１Ｒ = 10,
	息晚抖澎１ＦＢ１Ｒ,
	息晚抖谰Ｂ１Ｒ,
	息晚抖澎Ｂ１Ｒ,

	Ｅ涤雌浑Ｒ,
	Ｅ涤舶惧Ｒ,

	//	惟ち厦硼み使きイベント
	惟ち厦祁Ｐ = 20,
	惟ち厦颂Ｐ

}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_Ｄ涤から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:Ｅ涤雌浑Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ息晚抖	//	列淬独始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖

}

proc 焚洒始セット_Ｅ涤から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:Ｅ涤雌浑Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ息晚抖	//	列淬独始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖
}

proc 焚洒始セット_Ｄ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰１ＦＢ１Ｒ 8 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖

}

proc 焚洒始セット_Ｅ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎１ＦＢ１Ｒ 6 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖
}

proc 焚洒始セット_Ｄ涤から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:Ｅ涤雌浑Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ息晚抖	//	列淬独始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰Ｂ１Ｒ 3 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖
}

proc 焚洒始セット_Ｅ涤から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:Ｅ涤雌浑Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ息晚抖	//	列淬独始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:息晚抖谰Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:息晚抖澎Ｂ１Ｒ 2 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖
}

//	スネ〖クテイルズＢ惟ち厦硼み使きイベント脱
proc 焚洒始セット_惟ち厦硼み使き稿から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:Ｅ涤雌浑Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ息晚抖	//	列淬独始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:惟ち厦祁Ｐ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ息晚抖
	@A_焚洒始 浓始:03 d:T_ER:惟ち厦颂Ｐ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ息晚抖

	//	积ち眷に提らせる
	mesg 焚洒始 浓始:02 ル〖ト恃构 d:T_ER:息晚抖谰１ＦＢ１Ｒ 13000 0 0 ＤＥ息晚抖
	mesg 焚洒始 浓始:03 ル〖ト恃构 d:T_ER:息晚抖澎Ｂ１Ｒ -13000 0 0 ＤＥ息晚抖
}





//---------------
// 苟封始のセット
//---------------
//	浓始:01は列淬独始なのでサポ〖ト始をくっつけてはダメ—
proc 苟封始セット_Ｄ涤１Ｆから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｄ涤Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｅ涤から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_鼎奶 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｄ涤１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｄ涤Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｅ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_鼎奶_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤１Ｆ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｄ涤Ｂ１略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}




