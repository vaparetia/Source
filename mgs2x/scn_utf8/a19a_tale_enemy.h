/*
	a19a_tale_enemy.h
	    ＤＥ連絡橋(スネークテイルズ用敵兵設定ファイル)

	2002/06/13 S.Mukaide
	$Id: a19a_tale_enemy.h,v 1.6 2002/07/02 04:40:23 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ｅ脚待機Ｐ = 0,
	Ｄ脚１Ｆ待機Ｐ,
	Ｄ脚Ｂ１待機Ｐ,

	連絡橋西１ＦＢ１Ｒ = 10,
	連絡橋東１ＦＢ１Ｒ,
	連絡橋西Ｂ１Ｒ,
	連絡橋東Ｂ１Ｒ,

	Ｅ脚監視Ｒ,
	Ｅ脚屋上Ｒ,

	//	立ち話盗み聞きイベント
	立ち話南Ｐ = 20,
	立ち話北Ｐ

}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ｄ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:Ｅ脚監視Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ連絡橋	//	双眼鏡兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋

}

proc 警備兵セット_Ｅ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:Ｅ脚監視Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ連絡橋	//	双眼鏡兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋
}

proc 警備兵セット_Ｄ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西１ＦＢ１Ｒ 8 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋

}

proc 警備兵セット_Ｅ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東１ＦＢ１Ｒ 6 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋
}

proc 警備兵セット_Ｄ脚から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:Ｅ脚監視Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ連絡橋	//	双眼鏡兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西Ｂ１Ｒ 3 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋
}

proc 警備兵セット_Ｅ脚から_３ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:Ｅ脚監視Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ連絡橋	//	双眼鏡兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西Ｂ１Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:連絡橋東Ｂ１Ｒ 2 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋
}

//	スネークテイルズＢ立ち話盗み聞きイベント用
proc 警備兵セット_立ち話盗み聞き後から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:Ｅ脚監視Ｒ 0 d:ENE_STATUS_ACCDNT_REPO 34000 5500 8500 ＤＥ連絡橋	//	双眼鏡兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:立ち話南Ｐ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＤＥ連絡橋
	@A_警備兵 敵兵:03 d:T_ER:立ち話北Ｐ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＤＥ連絡橋

	//	持ち場に戻らせる
	mesg 警備兵 敵兵:02 ルート変更 d:T_ER:連絡橋西１ＦＢ１Ｒ 13000 0 0 ＤＥ連絡橋
	mesg 警備兵 敵兵:03 ルート変更 d:T_ER:連絡橋東Ｂ１Ｒ -13000 0 0 ＤＥ連絡橋
}





//---------------
// 攻撃兵のセット
//---------------
//	敵兵:01は双眼鏡兵なのでサポート兵をくっつけてはダメ！
proc 攻撃兵セット_Ｄ脚１Ｆから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｄ脚Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｅ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｄ脚１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｄ脚Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｅ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｄ脚Ｂ１待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}




