/*
	a16a_tale_enemy.h
		Ｃ涤咯撇(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/07 H.Yoshiike
	$Id: a16a_tale_enemy.h,v 1.10 2002/06/28 06:10:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	ＢＣ略怠Ｐ = 0,
	ＣＤ略怠Ｐ,
	檄布戒搀Ｒ,
	咯撇戒搀Ｒ,
	檄布惧戒搀Ｒ,
	咯撇のみ戒搀Ｒ,

	//	浓链糖イベント脱
	咯撇谰略怠Ｐ = 10,
	咯撇祁略怠Ｐ
}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_ＢＣ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布戒搀Ｒ 36 d:ENE_STATUS_NORMAL -44000 0 -95500 檄布
	@A_焚洒始 浓始:02 d:T_ER:咯撇戒搀Ｒ 6 d:ENE_STATUS_NORMAL -53250 0 -95500 檄布
}

proc 焚洒始セット_ＢＣ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布惧戒搀Ｒ 0 d:ENE_STATUS_NORMAL -44000 0 -95500 檄布
	@A_焚洒始 浓始:02 d:T_ER:咯撇のみ戒搀Ｒ 0 d:ENE_STATUS_NORMAL -53250 0 -95500 檄布
}

proc 焚洒始セット_ＣＤ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布戒搀Ｒ 32 d:ENE_STATUS_NORMAL -44000 0 -95500 檄布
	@A_焚洒始 浓始:02 d:T_ER:咯撇戒搀Ｒ 0 d:ENE_STATUS_NORMAL -53250 0 -95500 檄布
}

//	テイルズＢエマ柳而涟焚洒始セット
proc 焚洒始セット_ＣＤ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布戒搀Ｒ 32 d:ENE_STATUS_NORMAL -53250 0 -95500 檄布
}

proc 焚洒始セット_ダンボ〖ルから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布戒搀Ｒ 0 d:ENE_STATUS_NORMAL -44000 0 -95500 檄布
	@A_焚洒始 浓始:02 d:T_ER:咯撇戒搀Ｒ 5 d:ENE_STATUS_NORMAL -53250 0 -95500 檄布
}





//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_ＢＣ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:23 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＣＤ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_サポ〖ト苟封始 浓始:22 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 浓始:02
	@A_苟封始 浓始:23 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}
//	テイルズＢエマ柳而涟苟封始セット
proc 苟封始セット_ＣＤ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_苟封始 浓始:22 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_苟封始 浓始:23 d:T_ER:ＣＤ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

//	テイルズＢ浓链糖イベント脱浓始∈丹冷、碉滩りでも浩券栏させるためにd:ENE_STST_FAINT_EXITを惟てる∷
proc 苟封始セット_浓链糖イベント {
	@A_橙磨苟封始 浓始:21 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_橙磨苟封始 浓始:22 d:T_ER:咯撇祁略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_テイルズＢ脱大り苹橙磨苟封始 浓始:23 \
		d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL -55750 0 -96000 咯撇
}


