/*
	a17a_tale_enemy.h
	    ＣＤ息晚抖(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/13 S.Mukaide
	$Id: a17a_tale_enemy.h,v 1.4 2002/07/02 02:39:17 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	Ｄ涤略怠Ｐ = 0,
	Ｃ涤略怠Ｐ,

	息晚抖澎谰Ｒ = 10,
	息晚抖澎谰１ＦＢ１Ｒ,
	息晚抖谰１ＦＢ１Ｒ,
	息晚抖澎１ＦＢ１Ｒ,
	息晚抖澎谰布橇きＲ,		//	澎谰超檬烧夺で布橇き
	息晚抖澎谰布橇き２Ｒ,	//	息晚抖靠ん面で布橇き

}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_Ｃ涤から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:息晚抖谰１ＦＢ１Ｒ 9 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ息晚抖
	@A_焚洒始 浓始:02 d:T_ER:息晚抖澎１ＦＢ１Ｒ 5 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ息晚抖

}

proc 焚洒始セット_Ｄ涤から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:息晚抖谰１ＦＢ１Ｒ 5 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ息晚抖
	@A_焚洒始 浓始:02 d:T_ER:息晚抖澎１ＦＢ１Ｒ 9 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ息晚抖
}

proc 焚洒始セット_Ｃ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:息晚抖澎谰布橇き２Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ息晚抖
}

proc 焚洒始セット_Ｄ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:息晚抖澎谰布橇き２Ｒ 4 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ息晚抖
}


//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_Ｃ涤から {
	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｄ涤から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_鼎奶 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｃ涤から_２ {
	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	
}

proc 苟封始セット_Ｄ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_鼎奶_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒
	//	A_サポ〖ト苟封始 $:叹涟 $:ル〖ト $:ステ〖タス $:刘洒 $:サポ〖トする浓始叹
	@A_サポ〖ト苟封始 浓始:61 d:T_ER:Ｃ涤略怠Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:62 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:63 d:T_ER:Ｄ涤略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}




