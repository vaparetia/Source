/*
	a24d_tale_enemy.h
		シェル１中央棟電算室(スネークテイルズ用敵兵設定ファイル)

	2002/07/17 H.Yoshiike
	$Id: a24d_tale_enemy.h,v 1.1 2002/07/17 16:17:51 usr03682 Exp $                      
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	エレベータ前待機Ｐ = 0,
	電算室前待機Ｐ,
	中央廊下巡回Ｒ,
	電算室前巡回Ｒ,
	ノード前巡回Ｒ,
	電算室の周り巡回Ｒ,
	電算室内追加Ｒ
}

// 敵兵ディフェンスポイント
#define	D_POS_A24D_01	 -7250	 -16000	 -64000	 電算室	//	左の階段前
#define	D_POS_A24D_02	 7250	 -16000	 -64000	 電算室	//	右の階段前
#define	D_POS_A24D_03	 2250	 -16000	 -64000	 電算室	//	ノード前
#define	D_POS_A24D_04	 -3000	 -12000	 -72000	 電算室	//	エレベータ前

//---------------
// 警備兵のセット
//---------------
proc スネークテイルズＡ用警備兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:中央廊下巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24D_01
	@A_警備兵 敵兵:02 d:T_ER:電算室前巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24D_02
	@A_警備兵 敵兵:03 d:T_ER:ノード前巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24D_03
	@A_警備兵 敵兵:04 d:T_ER:電算室の周り巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24D_04
}





//---------------
// 攻撃兵のセット
//---------------
proc スネークテイルズＡ用攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 攻撃兵:21 d:T_ER:電算室前待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHIELD d:T_ER:電算室内追加Ｒ
	@A_攻撃兵 攻撃兵:22 d:T_ER:電算室前待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 攻撃兵:23 d:T_ER:電算室前待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHOTGUN
}
