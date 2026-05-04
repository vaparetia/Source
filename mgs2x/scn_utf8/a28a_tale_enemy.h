/*
	a28a_tale_enemy.h
		L脚(スネークテイルズ用敵兵設定ファイル)

	2002/06/12 H.Yoshiike
	$Id: a28a_tale_enemy.h,v 1.2 2002/07/02 10:31:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＫＬ待機Ｐ = 0,
	Ｌ脚南巡回Ｒ,
	Ｌ脚北巡回Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_はしごから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｌ脚南巡回Ｒ 0 d:ENE_STATUS_URBAN 52500,0,-207000 Ｌ脚
	@A_警備兵 敵兵:02 d:T_ER:Ｌ脚北巡回Ｒ 0 d:ENE_STATUS_URBAN 52000,0,-212250 Ｌ脚
}

proc 警備兵セット_ＫＬ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｌ脚南巡回Ｒ 0 d:ENE_STATUS_URBAN 52500,0,-207000 Ｌ脚
	@A_警備兵 敵兵:02 d:T_ER:Ｌ脚北巡回Ｒ 0 d:ENE_STATUS_URBAN 52000,0,-212250 Ｌ脚
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＫＬ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＫＬ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHOTGUN 敵兵:01
//	@A_攻撃兵 敵兵:22 d:T_ER:ＫＬ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_HITECH_1
	@A_攻撃兵 敵兵:22 d:T_ER:ＫＬ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}
