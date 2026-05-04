/*
	a16a_tale_enemy.h
		Ｃ脚食堂(スネークテイルズ用敵兵設定ファイル)

	2002/06/07 H.Yoshiike
	$Id: a16a_tale_enemy.h,v 1.10 2002/06/28 06:10:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＢＣ待機Ｐ = 0,
	ＣＤ待機Ｐ,
	廊下巡回Ｒ,
	食堂巡回Ｒ,
	廊下上巡回Ｒ,
	食堂のみ巡回Ｒ,

	//	敵全滅イベント用
	食堂西待機Ｐ = 10,
	食堂南待機Ｐ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_ＢＣ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下巡回Ｒ 36 d:ENE_STATUS_NORMAL -44000 0 -95500 廊下
	@A_警備兵 敵兵:02 d:T_ER:食堂巡回Ｒ 6 d:ENE_STATUS_NORMAL -53250 0 -95500 廊下
}

proc 警備兵セット_ＢＣ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下上巡回Ｒ 0 d:ENE_STATUS_NORMAL -44000 0 -95500 廊下
	@A_警備兵 敵兵:02 d:T_ER:食堂のみ巡回Ｒ 0 d:ENE_STATUS_NORMAL -53250 0 -95500 廊下
}

proc 警備兵セット_ＣＤ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下巡回Ｒ 32 d:ENE_STATUS_NORMAL -44000 0 -95500 廊下
	@A_警備兵 敵兵:02 d:T_ER:食堂巡回Ｒ 0 d:ENE_STATUS_NORMAL -53250 0 -95500 廊下
}

//	テイルズＢエマ遭遇前警備兵セット
proc 警備兵セット_ＣＤ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下巡回Ｒ 32 d:ENE_STATUS_NORMAL -53250 0 -95500 廊下
}

proc 警備兵セット_ダンボールから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:廊下巡回Ｒ 0 d:ENE_STATUS_NORMAL -44000 0 -95500 廊下
	@A_警備兵 敵兵:02 d:T_ER:食堂巡回Ｒ 5 d:ENE_STATUS_NORMAL -53250 0 -95500 廊下
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＢＣ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_サポート攻撃兵 敵兵:22 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:23 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＣＤ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_サポート攻撃兵 敵兵:22 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:02
	@A_攻撃兵 敵兵:23 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}
//	テイルズＢエマ遭遇前攻撃兵セット
proc 攻撃兵セット_ＣＤ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_サポート攻撃兵 敵兵:21 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL 敵兵:01
	@A_攻撃兵 敵兵:22 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
	@A_攻撃兵 敵兵:23 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}

//	テイルズＢ敵全滅イベント用敵兵（気絶、居眠りでも再発生させるためにd:ENE_STST_FAINT_EXITを立てる）
proc 攻撃兵セット_敵全滅イベント {
	@A_拡張攻撃兵 敵兵:21 d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_拡張攻撃兵 敵兵:22 d:T_ER:食堂南待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL
	@A_テイルズＢ用寄り道拡張攻撃兵 敵兵:23 \
		d:T_ER:ＢＣ待機Ｐ d:ENE_STATUS_NORMAL d:ENE_STST_FAINT_EXIT d:ENE_EQUIP_TYPE_NORMAL -55750 0 -96000 食堂
}


