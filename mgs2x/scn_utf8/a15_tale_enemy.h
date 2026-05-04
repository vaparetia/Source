/*
	a15_tale_enemy.h
	    ＢＣ連絡橋(スネークテイルズ用敵兵設定ファイル)

	2002/06/13 S.Mukaide
	$Id: a15_tale_enemy.h,v 1.7 2002/07/01 13:02:00 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ｃ脚待機Ｐ = 0,
	Ｂ脚待機Ｐ,

	連絡橋南北Ｒ = 10,
	連絡橋南北連絡Ｒ,

	//	サイファ用
	連絡橋西側Ｒ = 20,
	連絡橋東側Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ｂ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋南北連絡Ｒ 1 d:ENE_STATUS_REPORT -52000,0,-73000 ＢＣ連絡橋
}

proc 警備兵セット_Ｃ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
		@A_警備兵 敵兵:01 d:T_ER:連絡橋南北連絡Ｒ 1 d:ENE_STATUS_REPORT -52000,0,-47000 ＢＣ連絡橋
}

proc 警備兵セット_Ｂ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋南北Ｒ 2 d:ENE_STATUS_REPORT -52000,0,-73000 ＢＣ連絡橋
}

proc 警備兵セット_Ｃ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
		@A_警備兵 敵兵:01 d:T_ER:連絡橋南北Ｒ 0 d:ENE_STATUS_REPORT -52000,0,-47000 ＢＣ連絡橋
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_Ｂ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｃ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｃ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｂ脚待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}





//---------------
// サイファのセット
//---------------
proc サイファセット_Ｂ脚から {
	#if d:DEBUG_PRINT
		print 'cypher_set-snake_tales'
	#endif

	@サイファー再セットチェック $b:a15_サイファー破壊ロード回数[0]
	if ($f:再セットフラグ) {
		//	A_サイファ $:名前 $:ルート $:ノード $:視力 $:帰宅Ｘ１ $:帰宅１Ｙ１ $:帰宅Ｚ１ $:帰宅Ｘ２ $:帰宅Ｙ２ $:帰宅Ｚ２ $:速度
		@A_サイファ サイファー:01 d:T_ER:連絡橋西側Ｒ 0 6000 52000 10000 80000 52000 10000 200000 25
	}
}

proc サイファセット_Ｃ脚から {
	#if d:DEBUG_PRINT
		print 'cypher_set-snake_tales'
	#endif

	@サイファー再セットチェック $b:a15_サイファー破壊ロード回数[0]
	if ($f:再セットフラグ) {
		//	A_サイファ $:名前 $:ルート $:ノード $:視力 $:帰宅Ｘ１ $:帰宅１Ｙ１ $:帰宅Ｚ１ $:帰宅Ｘ２ $:帰宅Ｙ２ $:帰宅Ｚ２ $:速度
		@A_サイファ サイファー:01 d:T_ER:連絡橋東側Ｒ 0 6000 52000 10000 80000 52000 10000 200000 15
	}
}


