/*
	a23_tale_enemy.h
	    ＦＡ連絡橋（スネークテイルズ用敵兵設定ファイル）

	2002/06/14 S.Mukaide
	$Id: a23_tale_enemy.h,v 1.3 2002/07/01 05:19:48 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ａ脚待機Ｐ = 0,
	Ｆ脚１Ｆ待機Ｐ,
	Ｆ脚Ｂ１待機Ｐ,

	連絡橋西１ＦＢ１Ｒ = 10,
	連絡橋東１ＦＢ１Ｒ,
	連絡橋１ＦＢ１Ｒ,

	//	サイファ
	連絡橋西側Ｒ = 20,
	連絡橋南側Ｒ,
	連絡橋東側Ｒ,
	連絡橋北側Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ａ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋東１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL 13000,0,0 ＦＡ連絡橋
}

proc 警備兵セット_Ｆ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋西１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000,0,0 ＦＡ連絡橋
}

proc 警備兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000,0,0 ＦＡ連絡橋
}

proc 警備兵セット_共通_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:連絡橋東１ＦＢ１Ｒ 4 d:ENE_STATUS_NORMAL 13000,0,0 ＦＡ連絡橋
	@A_警備兵 敵兵:02 d:T_ER:連絡橋西１ＦＢ１Ｒ 0 d:ENE_STATUS_NORMAL -13000,0,0 ＦＡ連絡橋
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

proc 攻撃兵セット_Ｆ脚１Ｆから {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｆ脚１Ｆ待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｆ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｆ脚Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ｆ脚Ｂ１待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｆ脚Ｂ１待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:Ａ脚待機Ｐ d:ENE_STATUS_CONVERT2 d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:Ｆ脚１Ｆ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}





//---------------
// サイファのセット
//---------------
proc サイファセット_Ａ脚から {
	#if d:DEBUG_PRINT
		print 'cypher_set-snake_tales'
	#endif

	//	A_サイファ $:名前 $:ルート $:ノード $:視力 $:帰宅Ｘ１ $:帰宅１Ｙ１ $:帰宅Ｚ１ $:帰宅Ｘ２ $:帰宅Ｙ２ $:帰宅Ｚ２ $:速度

	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[0]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:01 d:T_ER:連絡橋東側Ｒ 0 6000 140000 10000 0 -140000 10000 0 25
	}
	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[1]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:02 d:T_ER:連絡橋北側Ｒ 3 6000 140000 10000 0 -140000 10000 0 25
	}
}

proc サイファセット_Ｆ脚１Ｆから {
	#if d:DEBUG_PRINT
		print 'cypher_set-snake_tales'
	#endif

	//	A_サイファ $:名前 $:ルート $:ノード $:視力 $:帰宅Ｘ１ $:帰宅１Ｙ１ $:帰宅Ｚ１ $:帰宅Ｘ２ $:帰宅Ｙ２ $:帰宅Ｚ２ $:速度

	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[0]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:01 d:T_ER:連絡橋北側Ｒ 0 6000 140000 10000 0 -140000 10000 0 25
	}
	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[1]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:02 d:T_ER:連絡橋西側Ｒ 0 6000 140000 10000 0 -140000 10000 0 25
	}
}

proc サイファセット_Ｆ脚Ｂ１から {
	#if d:DEBUG_PRINT
		print 'cypher_set-snake_tales'
	#endif

	//	A_サイファ $:名前 $:ルート $:ノード $:視力 $:帰宅Ｘ１ $:帰宅１Ｙ１ $:帰宅Ｚ１ $:帰宅Ｘ２ $:帰宅Ｙ２ $:帰宅Ｚ２ $:速度

	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[0]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:01 d:T_ER:連絡橋南側Ｒ 0 6000 140000 10000 0 -140000 10000 0 25
	}
	@サイファー再セットチェック $b:a23_サイファー破壊ロード回数[1]
	if ($f:再セットフラグ) {
		@A_サイファ サイファー:02 d:T_ER:連絡橋西側Ｒ 0 6000 140000 10000 0 -140000 10000 0 25
	}
}


