/*
	a12b_tale_enemy.h
		Ａ涤ポンプ技(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/05 H.Yoshiike
	$Id: a12b_tale_enemy.h,v 1.4 2002/07/01 06:57:43 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト
//---------------
enum T_ER {
	舶惧焊略怠Ｐ = 0,
	舶惧宝略怠Ｐ,
	ＡＢ略怠Ｐ,
	ＦＡ略怠Ｐ,
	扩告技焊戒搀Ｒ,
	扩告技宝戒搀焊搀りＲ,
	扩告技宝戒搀宝搀りＲ,
	ポンプ技戒搀Ｒ,
	扩告技面丙焊宝Ｒ,
	扩告技面丙盖年Ｒ,
	扩告技モニタ〖涟焊宝Ｒ,

	//	笆布テイルズＤ脱纳裁
	扩告技ＡＢ息晚抖娄祁颂Ｒ = 20,
	扩告技ＦＡ息晚抖娄祁颂Ｒ
}


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_舶惧焊から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技焊戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技宝戒搀焊搀りＲ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
	@A_焚洒始 浓始:03 d:T_ER:ポンプ技戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ技
}

proc 焚洒始セット_舶惧宝から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技焊戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技宝戒搀宝搀りＲ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
	@A_焚洒始 浓始:03 d:T_ER:ポンプ技戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ技
}

proc 焚洒始セット_ＡＢ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙盖年Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技モニタ〖涟焊宝Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
	@A_焚洒始 浓始:03 d:T_ER:ポンプ技戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ技
}

proc 焚洒始セット_ＦＡ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙盖年Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技モニタ〖涟焊宝Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
	@A_焚洒始 浓始:03 d:T_ER:ポンプ技戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ技
}

proc 焚洒始セット_ダンボ〖ルから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙焊宝Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:03 d:T_ER:ポンプ技戒搀Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ技
}


//	笆布テイルズＤ脱纳裁尸
proc 焚洒始セット_ＡＢ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙盖年Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技モニタ〖涟焊宝Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
}

proc 焚洒始セット_ＦＡ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙盖年Ｒ 4 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
	@A_焚洒始 浓始:02 d:T_ER:扩告技モニタ〖涟焊宝Ｒ 1 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 扩告技
}

proc 焚洒始セット_ダンボ〖ルから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:扩告技面丙焊宝Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 扩告技
}





//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_舶惧焊から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポ〖ト苟封始 浓始:21 d:T_ER:舶惧焊略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:22 d:T_ER:舶惧焊略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_舶惧宝から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポ〖ト苟封始 浓始:21 d:T_ER:舶惧宝略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:22 d:T_ER:舶惧宝略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＡＢ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:22 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＦＡ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポ〖ト苟封始 浓始:21 d:T_ER:ＦＡ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 浓始:03
	@A_苟封始 浓始:22 d:T_ER:ＦＡ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}


//	笆布テイルズＤ脱纳裁尸
proc 苟封始セット_ＡＢ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	//	A_玫瑚苟封始 $:p_叹涟 $:p_ル〖ト $:p_ステ〖タス $:p_刘洒 $:p_玫瑚ル〖ト
	@A_玫瑚苟封始 浓始:21 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:扩告技ＡＢ息晚抖娄祁颂Ｒ
	@A_苟封始 浓始:22 d:T_ER:ＡＢ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＦＡ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_玫瑚苟封始 浓始:21 d:T_ER:ＦＡ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL  d:T_ER:扩告技ＦＡ息晚抖娄祁颂Ｒ
	@A_苟封始 浓始:22 d:T_ER:ＦＡ略怠Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
