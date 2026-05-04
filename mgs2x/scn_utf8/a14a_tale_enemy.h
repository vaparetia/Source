/*
	a14a_tale_enemy.h
		Ｂ脚変電室(スネークテイルズ用敵兵設定ファイル)

	2002/06/05 H.Yoshiike
	$Id: a14a_tale_enemy.h,v 1.3 2002/06/21 12:09:01 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＢＣ待機Ｐ = 0,
	ＡＢ待機Ｐ,
	変電室一周巡回Ｒ,
	変電室中央上下巡回Ｒ,
	廊下上下巡回Ｒ,

	//	テイルズＢ用
	変電室北扉待機Ｐ = 10,
	変電室一周巡回逆Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_ＡＢ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下上下巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 廊下
	@A_警備兵 敵兵:02 d:T_ER:変電室一周巡回Ｒ 3 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 廊下
}

proc 警備兵セット_ＢＣ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下上下巡回Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 廊下
	@A_警備兵 敵兵:02 d:T_ER:変電室一周巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 廊下
}

proc 警備兵セット_ダンボールから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下上下巡回Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 廊下
	@A_警備兵 敵兵:02 d:T_ER:変電室一周巡回Ｒ 1 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 廊下
}

//	スネークテイルズＢノードアクセスイベント用
proc 警備兵セット_Ｂ脚ノードアクセス後から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:廊下上下巡回Ｒ 2 d:ENE_STATUS_GUNLIGHT -55000 0 -41000 廊下
	@A_警備兵 敵兵:02 d:T_ER:変電室北扉待機Ｐ 0 d:ENE_STATUS_GUNLIGHT -55000 0 -29000 廊下

	//	巡回ルートへ
	mesg 警備兵 敵兵:02 ルート変更 d:T_ER:変電室一周巡回逆Ｒ -55000 0 -29000 廊下 4
}




//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＡＢ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_探索攻撃兵 敵兵:22 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:変電室中央上下巡回Ｒ
	@A_攻撃兵 敵兵:23 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＢＣ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_探索攻撃兵 敵兵:22 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:変電室中央上下巡回Ｒ
	@A_攻撃兵 敵兵:23 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
