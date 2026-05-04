/*
	a18a_tale_enemy.h
		Ｄ涤(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/08 H.Yoshiike
	$Id: a18a_tale_enemy.h,v 1.3 2002/09/17 12:23:35 usr03682 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	ＣＤ略怠Ｐ = 0,
	ＤＥ１Ｆ略怠Ｐ,
	ＤＥＢ１略怠Ｐ,
	１Ｆ颂娄焊宝戒搀Ｒ,
	１Ｆ面丙办件戒搀Ｒ,
	Ｂ１颂娄年箕息晚焊宝戒搀Ｒ,
	１Ｆ链挛戒搀Ｒ,
	１Ｆ面丙焊宝戒搀Ｒ,
	Ｂ１颂娄年箕息晚超檬戒搀Ｒ
}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_ＣＤ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales' 
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚焊宝戒搀Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ颂娄焊宝戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ面丙办件戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
}

proc 焚洒始セット_ＣＤ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚超檬戒搀Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ面丙焊宝戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ链挛戒搀Ｒ 9 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
}

proc 焚洒始セット_ＤＥ息晚抖１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚焊宝戒搀Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ颂娄焊宝戒搀Ｒ 5 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ面丙办件戒搀Ｒ 4 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
}

proc 焚洒始セット_ＤＥ息晚抖１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚超檬戒搀Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ面丙焊宝戒搀Ｒ 2 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ链挛戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
}

proc 焚洒始セット_ＤＥ息晚抖Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚焊宝戒搀Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ颂娄焊宝戒搀Ｒ 5 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ面丙办件戒搀Ｒ 4 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
}

proc 焚洒始セット_ＤＥ息晚抖Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ１颂娄年箕息晚超檬戒搀Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ涤
	@A_焚洒始 浓始:02 d:T_ER:１Ｆ面丙焊宝戒搀Ｒ 2 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ涤
	@A_焚洒始 浓始:03 d:T_ER:１Ｆ链挛戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ涤
}





//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_ＣＤ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_苟封始 浓始:21 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:23 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_苟封始 浓始:24 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＤＥ息晚抖１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_苟封始 浓始:21 d:T_ER:ＤＥ１Ｆ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:ＤＥ１Ｆ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:23 d:T_ER:ＤＥ１Ｆ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_苟封始 浓始:24 d:T_ER:ＤＥ１Ｆ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＤＥ息晚抖Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_苟封始 浓始:21 d:T_ER:ＤＥＢ１略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:ＤＥＢ１略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:23 d:T_ER:ＤＥＢ１略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_苟封始 浓始:24 d:T_ER:ＤＥＢ１略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
