/*
	a13_tale_enemy.h
	    ＡＢ連絡橋(スネークテイルズ用敵兵設定ファイル)

	2002/06/13 S.Mukaide
	$Id: a13_tale_enemy.h,v 1.8 2002/07/26 11:46:37 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ｂ脚待機Ｐ = 0,
	Ａ脚待機Ｐ,

	連絡橋南左右Ｒ = 10,
	連絡橋北左右Ｒ,
	連絡橋南西巡回Ｒ,
	連絡橋北西巡回Ｒ,
	連絡橋南東巡回Ｒ,
	連絡橋北東巡回Ｒ,
	連絡橋西南東巡回Ｒ,
	連絡橋西北東巡回Ｒ,

	//	テイルズＥ敵全滅イベント用
	Ｂ脚側登場デモ待機Ｐ = 20,
	Ｂ脚側登場デモ２待機Ｐ,
	Ａ脚側登場デモ待機Ｐ,
	Ａ脚側登場デモ２待機Ｐ,

	Ｂ脚側登場デモＲ = 30,
	Ｂ脚側登場デモ２Ｒ,			//	ルート長い
	Ａ脚側登場デモＲ,
	Ａ脚側登場デモ２Ｒ,			//	ルート長い

	Ｂ脚側接続部待機Ｐ = 40,
	Ｂ脚側南連絡橋待機Ｐ,
	Ａ脚側接続部待機Ｐ,
	Ａ脚側南連絡橋待機Ｐ,

	//	テイルズＥオルガ登場デモ用
	オルガ待機Ｐ = 50,
	オルガ移動Ｒ,
	オルガ移動２Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ａ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋南左右Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＡＢ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋北左右Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＡＢ連絡橋
}

proc 警備兵セット_Ｂ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋南左右Ｒ 1 d:ENE_STATUS_NORMAL 13000 0 0 ＡＢ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋北左右Ｒ 1 d:ENE_STATUS_NORMAL -13000 0 0 ＡＢ連絡橋
}

proc 警備兵セット_Ａ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋北西巡回Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＡＢ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋南東巡回Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＡＢ連絡橋
}

proc 警備兵セット_Ｂ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋南西巡回Ｒ 0 d:ENE_STATUS_NORMAL -13000 0 0 ＡＢ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋北東巡回Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＡＢ連絡橋
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_Ａ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ａ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ａ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｂ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ａ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

//	テイルズＥ敵兵全滅イベント用敵兵（気絶、居眠りでも再発生させるためにd:ENE_STST_FAINT_EXITを立てる）
proc 攻撃兵セット_敵兵全滅イベント {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_拡張攻撃兵 敵兵:61 d:T_ER:Ａ脚側接続部待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_拡張攻撃兵 敵兵:62 d:T_ER:Ａ脚側南連絡橋待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_拡張攻撃兵 敵兵:63 d:T_ER:Ｂ脚側接続部待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_拡張攻撃兵 敵兵:64 d:T_ER:Ｂ脚側南連絡橋待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
}




