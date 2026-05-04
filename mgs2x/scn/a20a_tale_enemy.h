/*
	a20a_tale_enemy.h
		Ｅ涤礁芹眷(スネ〖クテイルズ脱浓始肋年ファイル)

	2002/06/15 H.Yoshiike
	$Id: a20a_tale_enemy.h,v 1.8 2002/08/28 11:15:55 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 浓始に簇する肋年
//------------------------------------------------------------
//---------------
// 浓始ル〖ト(0-3はダンボ〖ル脱)
//---------------
enum T_ER {
	ＤＥ略怠Ｐ = 4,
	ヘリポ〖ト略怠Ｐ,
	ＥＦ略怠Ｐ,

	Ａ脱澎捏绩息晚戒搀Ｒ,
	Ａ脱面丙戒搀Ｒ,
	Ａ脱颂焊宝戒搀Ｒ,
	Ａ脱面丙惧布纳裁Ｒ,
	Ａ脱面丙纳裁Ｒ,

	Ｂ脱ノ〖ド涟戒搀Ｒ,
	Ｂ脱颂捏绩息晚戒搀Ｒ,
	Ｂ脱谰戒搀Ｒ,
	Ｂ脱颂戒搀Ｒ,
	Ｂ脱面丙纳裁Ｒ,

	Ｄ脱谰惧布戒搀Ｒ,
	Ｄ脱颂捏绩息晚戒搀Ｒ,
	Ｄ脱面丙惧布戒搀Ｒ,
	Ｄ脱澎惧布戒搀Ｒ,
	Ｄ脱祁纳裁Ｒ,

	//	テイルズＢＥ涤忙叫イベント脱
	颂谰略怠Ｐ = 30,
	颂略怠Ｐ,
	澎略怠Ｐ,
	面丙略怠Ｐ,
	谰略怠Ｐ
}

//---------------
// 浓始ステ〖タス
//---------------
//	d:ENE_STATUS_NPCは焚洒始のみ铜跟。苟封始はエマしかいないときはエマを、それ笆嘲はスネ〖クを苟封する
#define		ENE_STATUS_A20A_N	d:ENE_STATUS_GUNLIGHT
#define		ENE_STATUS_A20A_R	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT)
#define		ENE_STATUS_A20A_E	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_NPC)


//---------------
// 焚洒始のセット
//---------------
proc 焚洒始セット_ＤＥ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱澎捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱面丙戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱颂焊宝戒搀Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ涤１Ｆ
}

proc 焚洒始セット_ＥＦ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱澎捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱面丙戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱颂焊宝戒搀Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ涤１Ｆ
}

proc 焚洒始セット_ヘリポ〖トから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ａ脱澎捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ａ脱面丙戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ａ脱颂焊宝戒搀Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ涤１Ｆ
}


proc 焚洒始セット_ＤＥ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱ノ〖ド涟戒搀Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱谰戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱颂戒搀Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}

proc 焚洒始セット_ＥＦ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱ノ〖ド涟戒搀Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱ノ〖ド涟戒搀Ｒ 2 d:ENE_STATUS_A20A_N 58000 0 -79500 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱谰戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱颂戒搀Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}

proc 焚洒始セット_ヘリポ〖トから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｂ脱ノ〖ド涟戒搀Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｂ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｂ脱谰戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｂ脱颂戒搀Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}


proc 焚洒始セット_ＤＥ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱谰惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱面丙惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｄ脱澎惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}

proc 焚洒始セット_ＥＦ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱谰惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱面丙惧布戒搀Ｒ 1 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｄ脱澎惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}

proc 焚洒始セット_ヘリポ〖トから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_焚洒始 浓始:01 d:T_ER:Ｄ脱谰惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:02 d:T_ER:Ｄ脱颂捏绩息晚戒搀Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ涤１Ｆ
	@A_焚洒始 浓始:03 d:T_ER:Ｄ脱面丙惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ涤１Ｆ
//	@A_焚洒始 浓始:04 d:T_ER:Ｄ脱澎惧布戒搀Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ涤１Ｆ
}





//---------------
// 苟封始のセット
//---------------
proc 苟封始セット_ＤＥ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙惧布纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ＥＦ息晚抖から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙惧布纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 苟封始セット_ヘリポ〖トから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙纳裁Ｒ
	@A_玫瑚苟封始 浓始:62 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ脱面丙惧布纳裁Ｒ
	@A_苟封始 浓始:63 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 苟封始セット_ＤＥ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱面丙纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＥＦ息晚抖から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱面丙纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ヘリポ〖トから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ脱面丙纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}


proc 苟封始セット_ＤＥ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ脱祁纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ＤＥ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ＥＦ息晚抖から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ脱祁纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ＥＦ略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 苟封始セット_ヘリポ〖トから_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_玫瑚苟封始 浓始:61 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ脱祁纳裁Ｒ
	@A_苟封始 浓始:62 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_苟封始 浓始:63 d:T_ER:ヘリポ〖ト略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

//	Ｅ涤忙叫イベント脱浓始∈丹冷、碉滩りでも浩券栏させるためにd:ENE_STST_FAINT_EXITを惟てる∷
proc 苟封始セット_Ｅ涤忙叫イベント {
	@A_橙磨苟封始 浓始:61 d:T_ER:颂略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_橙磨苟封始 浓始:62 d:T_ER:澎略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_テイルズＢ脱大り苹橙磨苟封始 浓始:63 \
		d:T_ER:面丙略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL 45000 0 -98000 Ｅ涤１Ｆ
	@A_テイルズＢ脱大り苹橙磨苟封始 浓始:64 \
		d:T_ER:谰略怠Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL 45000 0 -99000 Ｅ涤１Ｆ
}
