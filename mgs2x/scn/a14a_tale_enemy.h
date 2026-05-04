/*
	a14a_tale_enemy.h
		Ｂ涤恃排技(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/05 H.Yoshiike
	$Id: a14a_tale_enemy.h,v 1.3 2002/06/21 12:09:01 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	ＢＣ略怠Ｐ = 0,
	ＡＢ略怠Ｐ,
	恃排技办件戒搀Ｒ,
	恃排技面丙惧布戒搀Ｒ,
	檄布惧布戒搀Ｒ,

	//	テイルズＢ脱
	恃排技颂肉略怠Ｐ = 10,
	恃排技办件戒搀嫡Ｒ
}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_ＡＢ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布惧布戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 檄布
	@A_焚洒始 浓始:02 d:T_ER:恃排技办件戒搀Ｒ 3 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 檄布
}

proc 焚洒始セット_ＢＣ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布惧布戒搀Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 檄布
	@A_焚洒始 浓始:02 d:T_ER:恃排技办件戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 檄布
}

proc 焚洒始セット_ダンボ〖ルから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:檄布惧布戒搀Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 檄布
	@A_焚洒始 浓始:02 d:T_ER:恃排技办件戒搀Ｒ 1 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 檄布
}

//	スネ〖クテイルズＢノ〖ドアクセスイベント脱
proc 焚洒始セット_Ｂ涤ノ〖ドアクセス稿から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_焚洒始 $:叹涟 $:ル〖ト $:ノ〖ド $:ステ〖タス $:奸洒Ｘ $:奸洒Ｙ $:奸洒Ｚ $:奸洒マップ
	@A_焚洒始 浓始:01 d:T_ER:檄布惧布戒搀Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 檄布
	@A_焚洒始 浓始:02 d:T_ER:恃排技颂肉略怠Ｐ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 檄布

	//	戒搀ル〖トへ
	mesg 焚洒始 浓始:02 ル〖ト恃构 d:T_ER:恃排技办件戒搀嫡Ｒ -55000 0 -29000 檄布 4
}




//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_ＡＢ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_玫瑚苟封始 浓始:22 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:恃排技面丙惧布戒搀Ｒ
	@A_苟封始 浓始:23 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＢＣ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:01
	@A_玫瑚苟封始 浓始:22 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:恃排技面丙惧布戒搀Ｒ
	@A_苟封始 浓始:23 d:T_ER:ＢＣ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
