/*
	a20b_tale_enemy.h
		ヘリポート(スネークテイルズ用敵兵設定ファイル)

	2002/06/15 H.Yoshiike
	$Id: a20b_tale_enemy.h,v 1.3 2002/06/27 13:36:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	Ｅ脚待機Ｐ = 0,
	ヘリポート下Ｒ,
	ヘリポート北西Ｒ,
	ヘリポート北東Ｒ,
	ヘリポート南Ｒ,
	ヘリポート北Ｒ
}

//---------------
// 敵兵ステータス
//---------------
#define		ENE_STATUS_A20B		d:ENE_STATUS_NO_FINGER


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_Ｅ脚から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:ヘリポート下Ｒ 0 d:ENE_STATUS_A20B 59750 5500 -80500 ヘリポート	//	ヘリポート下兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:ヘリポート北西Ｒ 0 d:ENE_STATUS_A20B 56500 11500 -91250 ヘリポート
	@A_警備兵 敵兵:03 d:T_ER:ヘリポート北東Ｒ 0 d:ENE_STATUS_A20B 49500 11500 -84625 ヘリポート
	@A_警備兵 敵兵:04 d:T_ER:ヘリポート南Ｒ 0 d:ENE_STATUS_A20B 57000 11500 -82000 ヘリポート
}

proc 警備兵セット_Ｅ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:ヘリポート下Ｒ 0 d:ENE_STATUS_A20B 59750 5500 -80500 ヘリポート	//	ヘリポート下兵（名前固定）
	@A_警備兵 敵兵:02 d:T_ER:ヘリポート北Ｒ 0 d:ENE_STATUS_A20B 49500 11500 -84625 ヘリポート
	@A_警備兵 敵兵:03 d:T_ER:ヘリポート南Ｒ 0 d:ENE_STATUS_A20B 57000 11500 -82000 ヘリポート
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_Ｅ脚から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_サポート攻撃兵 敵兵:22 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:23 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_Ｅ脚から_２ {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:22 d:T_ER:Ｅ脚待機Ｐ d:ENE_STATUS_A20B d:ENE_EQUIP_TYPE_NORMAL
}
