/*
	a24a_tale_enemy.h
		シェル１中央棟１Ｆ(スネークテイルズ用敵兵設定ファイル)

	2002/07/17 H.Yoshiike
	$Id: a24a_tale_enemy.h,v 1.1 2002/07/17 16:17:46 usr03682 Exp $                      
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＥＦ待機Ｐ = 0,
	南東部屋前巡回Ｒ,
	西廊下巡回Ｒ,
	東廊下追加Ｒ,
	エレベータ前追加Ｒ
}

// 敵兵ディフェンスポイント
#define	D_POS_A24A_01	 11000	 0	 -53000 シェル１中央棟１Ｆ	//	ＥＦ連絡橋への廊下前
#define	D_POS_A24A_02	 -11000	 0	 -70000 シェル１中央棟１Ｆ	//	ステージ北西

//---------------
// 警備兵のセット
//---------------
proc スネークテイルズＡ用警備兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:南東部屋前巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24A_01
	@A_警備兵 敵兵:02 d:T_ER:西廊下巡回Ｒ 0 d:ENE_STATUS_NORMAL d:D_POS_A24A_02
}





//---------------
// 攻撃兵のセット
//---------------
proc スネークテイルズＡ用攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_探索攻撃兵 敵兵:21 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHIELD d:T_ER:東廊下追加Ｒ
	@A_探索攻撃兵 敵兵:22 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHIELD d:T_ER:エレベータ前追加Ｒ
	@A_攻撃兵 敵兵:23 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHOTGUN
	@A_攻撃兵 敵兵:24 d:T_ER:ＥＦ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHOTGUN
}
