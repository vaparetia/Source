/*
	a12a_tale_enemy.h
		Ａ脚屋上(スネークテイルズ用敵兵設定ファイル)

	2002/06/05 H.Yoshiike
	$Id: a12a_tale_enemy.h,v 1.3 2002/06/06 13:45:44 usr03682 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	左待機Ｐ = 0,
	右待機Ｐ,
	一周居眠り付巡回Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc スネークテイルズＡ用警備兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:一周居眠り付巡回Ｒ 0 d:ENE_STATUS_NO_FINGER 0 5000 -3000 屋上
}





//---------------
// 攻撃兵のセット
//---------------
proc スネークテイルズＡ用攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif
	@A_サポート攻撃兵 敵兵:21 d:T_ER:左待機Ｐ d:ENE_STATUS_NO_FINGER d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:22 d:T_ER:右待機Ｐ d:ENE_STATUS_NO_FINGER d:ENE_EQUIP_TYPE_NORMAL
}
