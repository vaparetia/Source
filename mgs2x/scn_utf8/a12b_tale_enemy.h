/*
	a12b_tale_enemy.h
		Ａ脚ポンプ室(スネークテイルズ用敵兵設定ファイル)

	2002/06/05 H.Yoshiike
	$Id: a12b_tale_enemy.h,v 1.4 2002/07/01 06:57:43 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	屋上左待機Ｐ = 0,
	屋上右待機Ｐ,
	ＡＢ待機Ｐ,
	ＦＡ待機Ｐ,
	制御室左巡回Ｒ,
	制御室右巡回左回りＲ,
	制御室右巡回右回りＲ,
	ポンプ室巡回Ｒ,
	制御室中央左右Ｒ,
	制御室中央固定Ｒ,
	制御室モニター前左右Ｒ,

	//	以下テイルズＤ用追加
	制御室ＡＢ連絡橋側南北Ｒ = 20,
	制御室ＦＡ連絡橋側南北Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_屋上左から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室左巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室右巡回左回りＲ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
	@A_警備兵 敵兵:03 d:T_ER:ポンプ室巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ室
}

proc 警備兵セット_屋上右から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室左巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室右巡回右回りＲ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
	@A_警備兵 敵兵:03 d:T_ER:ポンプ室巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ室
}

proc 警備兵セット_ＡＢ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央固定Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室モニター前左右Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
	@A_警備兵 敵兵:03 d:T_ER:ポンプ室巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ室
}

proc 警備兵セット_ＦＡ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央固定Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室モニター前左右Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
	@A_警備兵 敵兵:03 d:T_ER:ポンプ室巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ室
}

proc 警備兵セット_ダンボールから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央左右Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:03 d:T_ER:ポンプ室巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 6000 0 1500 ポンプ室
}


//	以下テイルズＤ用追加分
proc 警備兵セット_ＡＢ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央固定Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室モニター前左右Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
}

proc 警備兵セット_ＦＡ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央固定Ｒ 4 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
	@A_警備兵 敵兵:02 d:T_ER:制御室モニター前左右Ｒ 1 d:ENE_STATUS_GUNLIGHT 9500 0 -1250 制御室
}

proc 警備兵セット_ダンボールから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:制御室中央左右Ｒ 0 d:ENE_STATUS_GUNLIGHT -9500 0 -1250 制御室
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_屋上左から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポート攻撃兵 敵兵:21 d:T_ER:屋上左待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:22 d:T_ER:屋上左待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_屋上右から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポート攻撃兵 敵兵:21 d:T_ER:屋上右待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:22 d:T_ER:屋上右待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＡＢ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:22 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＦＡ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＦＡ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:22 d:T_ER:ＦＡ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}


//	以下テイルズＤ用追加分
proc 攻撃兵セット_ＡＢ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	//	A_探索攻撃兵 $:p_名前 $:p_ルート $:p_ステータス $:p_装備 $:p_探索ルート
	@A_探索攻撃兵 敵兵:21 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL d:T_ER:制御室ＡＢ連絡橋側南北Ｒ
	@A_攻撃兵 敵兵:22 d:T_ER:ＡＢ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＦＡ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_探索攻撃兵 敵兵:21 d:T_ER:ＦＡ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL  d:T_ER:制御室ＦＡ連絡橋側南北Ｒ
	@A_攻撃兵 敵兵:22 d:T_ER:ＦＡ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
