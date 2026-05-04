/*
	a20a_tale_enemy.h
		Ｅ脚集配場(スネークテイルズ用敵兵設定ファイル)

	2002/06/15 H.Yoshiike
	$Id: a20a_tale_enemy.h,v 1.8 2002/08/28 11:15:55 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート(0-3はダンボール用)
//---------------
enum T_ER {
	ＤＥ待機Ｐ = 4,
	ヘリポート待機Ｐ,
	ＥＦ待機Ｐ,

	Ａ用東提示連絡巡回Ｒ,
	Ａ用中央巡回Ｒ,
	Ａ用北左右巡回Ｒ,
	Ａ用中央上下追加Ｒ,
	Ａ用中央追加Ｒ,

	Ｂ用ノード前巡回Ｒ,
	Ｂ用北提示連絡巡回Ｒ,
	Ｂ用西巡回Ｒ,
	Ｂ用北巡回Ｒ,
	Ｂ用中央追加Ｒ,

	Ｄ用西上下巡回Ｒ,
	Ｄ用北提示連絡巡回Ｒ,
	Ｄ用中央上下巡回Ｒ,
	Ｄ用東上下巡回Ｒ,
	Ｄ用南追加Ｒ,

	//	テイルズＢＥ脚脱出イベント用
	北西待機Ｐ = 30,
	北待機Ｐ,
	東待機Ｐ,
	中央待機Ｐ,
	西待機Ｐ
}

//---------------
// 敵兵ステータス
//---------------
//	d:ENE_STATUS_NPCは警備兵のみ有効。攻撃兵はエマしかいないときはエマを、それ以外はスネークを攻撃する
#define		ENE_STATUS_A20A_N	d:ENE_STATUS_GUNLIGHT
#define		ENE_STATUS_A20A_R	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT)
#define		ENE_STATUS_A20A_E	(d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_NPC)


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_ＤＥ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用東提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ａ用中央巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ａ用北左右巡回Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ脚１Ｆ
}

proc 警備兵セット_ＥＦ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用東提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ａ用中央巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ａ用北左右巡回Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ脚１Ｆ
}

proc 警備兵セット_ヘリポートから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ａ用東提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 59250 0 -84750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ａ用中央巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ａ用北左右巡回Ｒ 1 d:ENE_STATUS_A20A_N 50000 0 -98250 Ｅ脚１Ｆ
}


proc 警備兵セット_ＤＥ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用ノード前巡回Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用西巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｂ用北巡回Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}

proc 警備兵セット_ＥＦ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

//	@A_警備兵 敵兵:01 d:T_ER:Ｂ用ノード前巡回Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
	@A_警備兵 敵兵:01 d:T_ER:Ｂ用ノード前巡回Ｒ 2 d:ENE_STATUS_A20A_N 58000 0 -79500 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用西巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｂ用北巡回Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}

proc 警備兵セット_ヘリポートから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ用ノード前巡回Ｒ 2 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｂ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 50000 0 -94750 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｂ用西巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｂ用北巡回Ｒ 1 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}


proc 警備兵セット_ＤＥ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用西上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用中央上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｄ用東上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}

proc 警備兵セット_ＥＦ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用西上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用中央上下巡回Ｒ 1 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｄ用東上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}

proc 警備兵セット_ヘリポートから_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｄ用西上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 45000 0 -83000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:02 d:T_ER:Ｄ用北提示連絡巡回Ｒ 0 d:ENE_STATUS_A20A_R 47000 0 -94000 Ｅ脚１Ｆ
	@A_警備兵 敵兵:03 d:T_ER:Ｄ用中央上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 60250 0 -79500 Ｅ脚１Ｆ
//	@A_警備兵 敵兵:04 d:T_ER:Ｄ用東上下巡回Ｒ 0 d:ENE_STATUS_A20A_N 63250 2250 -95750 Ｅ脚１Ｆ
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＤＥ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央上下追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ＥＦ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央上下追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}

proc 攻撃兵セット_ヘリポートから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央追加Ｒ
	@A_探索攻撃兵 敵兵:62 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ａ用中央上下追加Ｒ
	@A_攻撃兵 敵兵:63 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
}


proc 攻撃兵セット_ＤＥ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用中央追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＥＦ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用中央追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ヘリポートから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｂ用中央追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}


proc 攻撃兵セット_ＤＥ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ用南追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ＤＥ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＥＦ連絡橋から_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ用南追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ヘリポートから_３ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:61 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL d:T_ER:Ｄ用南追加Ｒ
	@A_攻撃兵 敵兵:62 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:63 d:T_ER:ヘリポート待機Ｐ d:ENE_STATUS_A20A_N d:ENE_EQUIP_TYPE_NORMAL
}

//	Ｅ脚脱出イベント用敵兵（気絶、居眠りでも再発生させるためにd:ENE_STST_FAINT_EXITを立てる）
proc 攻撃兵セット_Ｅ脚脱出イベント {
	@A_拡張攻撃兵 敵兵:61 d:T_ER:北待機Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_拡張攻撃兵 敵兵:62 d:T_ER:東待機Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_テイルズＢ用寄り道拡張攻撃兵 敵兵:63 \
		d:T_ER:中央待機Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL 45000 0 -98000 Ｅ脚１Ｆ
	@A_テイルズＢ用寄り道拡張攻撃兵 敵兵:64 \
		d:T_ER:西待機Ｐ d:ENE_STATUS_A20A_N d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL 45000 0 -99000 Ｅ脚１Ｆ
}
