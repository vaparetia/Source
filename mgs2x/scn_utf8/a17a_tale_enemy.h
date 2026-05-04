/*
	a17a_tale_enemy.h
	    ＣＤ連絡橋(スネークテイルズ用敵兵設定ファイル)

	2002/06/13 S.Mukaide
	$Id: a17a_tale_enemy.h,v 1.4 2002/07/02 02:39:17 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ｄ脚待機Ｐ = 0,
	Ｃ脚待機Ｐ,

	連絡橋東西Ｒ = 10,
	連絡橋東西１ＦＢ１Ｒ,
	連絡橋西１ＦＢ１Ｒ,
	連絡橋東１ＦＢ１Ｒ,
	連絡橋東西下覗きＲ,		//	東西階段付近で下覗き
	連絡橋東西下覗き２Ｒ,	//	連絡橋真ん中で下覗き

}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ｃ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋西１ＦＢ１Ｒ 9 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋東１ＦＢ１Ｒ 5 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ連絡橋

}

proc 警備兵セット_Ｄ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋西１ＦＢ１Ｒ 5 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋東１ＦＢ１Ｒ 9 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ連絡橋
}

proc 警備兵セット_Ｃ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋東西下覗き２Ｒ 0 d:ENE_STATUS_NORMAL 13000 0 0 ＣＤ連絡橋
}

proc 警備兵セット_Ｄ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋東西下覗き２Ｒ 4 d:ENE_STATUS_NORMAL -13000 0 0 ＣＤ連絡橋
}


//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_Ｃ脚から {
	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｄ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｃ脚から_２ {
	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	
}

proc 攻撃兵セット_Ｄ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:63 d:T_ER:Ｄ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}




