/*
	a24d_tale_enemy.h
	    人質部屋（スネークテイルズ用敵兵設定ファイル）

	2002/06/14 S.Mukaide
	$Id: a24c_tale_enemy.h,v 1.2 2002/06/17 05:45:26 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	中央棟Ｂ１廊下待機Ｐ = 0,

	内部見回り１Ｒ = 10,
	内部見回り２Ｒ

}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	//	A_警備兵 $:名前 $:ルート $:ノード $:ステータス $:守備Ｘ $:守備Ｙ $:守備Ｚ $:守備マップ
	@A_警備兵 敵兵:01 d:T_ER:内部見回り２Ｒ 0 \
	(d:ENE_STATUS_ENDLES_CHASE | d:ENE_STATUS_BIG_SNORE | d:ENE_STATUS_REPORT|d:ENE_STATUS_TALK_SLEEP) \
	10250,-8500,-65000 人質部屋

}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_共通 {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	//	A_攻撃兵 $:名前 $:ルート $:ステータス $:装備
	//	A_サポート攻撃兵 $:名前 $:ルート $:ステータス $:装備 $:サポートする敵兵名
	@A_サポート攻撃兵 敵兵:61 d:T_ER:中央棟Ｂ１廊下待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:62 d:T_ER:中央棟Ｂ１廊下待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL

}


