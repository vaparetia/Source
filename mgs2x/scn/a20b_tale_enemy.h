/*
	a20b_tale_enemy.h
		ヘリポ〖ト(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/15 H.Yoshiike
	$Id: a20b_tale_enemy.h,v 1.3 2002/06/27 13:36:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	Ｅ涤略怠Ｐ = 0,
	ヘリポ〖ト布Ｒ,
	ヘリポ〖ト颂谰Ｒ,
	ヘリポ〖ト颂澎Ｒ,
	ヘリポ〖ト祁Ｒ,
	ヘリポ〖ト颂Ｒ
}

//---------------
// 浓始ステ〖タス
//---------------
#define		ENE_STATUS_A20B		d:ENE_STATUS_NO_FINGER


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_Ｅ涤から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:ヘリポ〖ト布Ｒ 0 d:ENE_STATUS_A20B 59750 5500 -80500 ヘリポ〖ト	//	ヘリポ〖ト布始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:ヘリポ〖ト颂谰Ｒ 0 d:ENE_STATUS_A20B 56500 11500 -91250 ヘリポ〖ト
	@A_焚洒始 浓始:03 d:T_ER:ヘリポ〖ト颂澎Ｒ 0 d:ENE_STATUS_A20B 49500 11500 -84625 ヘリポ〖ト
	@A_焚洒始 浓始:04 d:T_ER:ヘリポ〖ト祁Ｒ 0 d:ENE_STATUS_A20B 57000 11500 -82000 ヘリポ〖ト
}

proc 焚洒始セット_Ｅ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:ヘリポ〖ト布Ｒ 0 d:ENE_STATUS_A20B 59750 5500 -80500 ヘリポ〖ト	//	ヘリポ〖ト布始∈叹涟盖年∷
	@A_焚洒始 浓始:02 d:T_ER:ヘリポ〖ト颂Ｒ 0 d:ENE_STATUS_A20B 49500 11500 -84625 ヘリポ〖ト
	@A_焚洒始 浓始:03 d:T_ER:ヘリポ〖ト祁Ｒ 0 d:ENE_STATUS_A20B 57000 11500 -82000 ヘリポ〖ト
}





//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_Ｅ涤から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:23 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_Ｅ涤から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:22 d:T_ER:Ｅ涤略怠Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL
}
