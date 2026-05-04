/*
	a18a_tale_enemy.h
		Ｄ脚(スネークテイルズ用敵兵設定ファイル)

	2002/06/08 H.Yoshiike
	$Id: a18a_tale_enemy.h,v 1.3 2002/09/17 12:23:35 usr03682 Exp $                      

	
*/

//------------------------------------------------------------
// 敵兵に関する設定
//------------------------------------------------------------
//---------------
// 敵兵ルート
//---------------
enum T_ER {
	ＣＤ待機Ｐ = 0,
	ＤＥ１Ｆ待機Ｐ,
	ＤＥＢ１待機Ｐ,
	１Ｆ北側左右巡回Ｒ,
	１Ｆ中央一周巡回Ｒ,
	Ｂ１北側定時連絡左右巡回Ｒ,
	１Ｆ全体巡回Ｒ,
	１Ｆ中央左右巡回Ｒ,
	Ｂ１北側定時連絡階段巡回Ｒ
}


//---------------
// 警備兵のセット
//---------------
proc 警備兵セット_ＣＤ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales' 
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡左右巡回Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ北側左右巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ中央一周巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
}

proc 警備兵セット_ＣＤ連絡橋から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡階段巡回Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ中央左右巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ全体巡回Ｒ 9 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
}

proc 警備兵セット_ＤＥ連絡橋１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡左右巡回Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ北側左右巡回Ｒ 5 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ中央一周巡回Ｒ 4 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
}

proc 警備兵セット_ＤＥ連絡橋１Ｆから_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡階段巡回Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ中央左右巡回Ｒ 2 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ全体巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
}

proc 警備兵セット_ＤＥ連絡橋Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡左右巡回Ｒ 6 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ北側左右巡回Ｒ 5 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ中央一周巡回Ｒ 4 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
}

proc 警備兵セット_ＤＥ連絡橋Ｂ１から_２ {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_警備兵 敵兵:01 d:T_ER:Ｂ１北側定時連絡階段巡回Ｒ 0 ( d:ENE_STATUS_GUNLIGHT | d:ENE_STATUS_REPORT ) 12250 -4000 -118625 Ｄ脚
	@A_警備兵 敵兵:02 d:T_ER:１Ｆ中央左右巡回Ｒ 2 d:ENE_STATUS_GUNLIGHT 0 0 -123250 Ｄ脚
	@A_警備兵 敵兵:03 d:T_ER:１Ｆ全体巡回Ｒ 0 d:ENE_STATUS_GUNLIGHT 9500 0 -127000 Ｄ脚
}





//---------------
// 攻撃兵のセット
//---------------
proc 攻撃兵セット_ＣＤ連絡橋から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_攻撃兵 敵兵:21 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポート攻撃兵 敵兵:22 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:23 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_攻撃兵 敵兵:24 d:T_ER:ＣＤ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＤＥ連絡橋１Ｆから {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_攻撃兵 敵兵:21 d:T_ER:ＤＥ１Ｆ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポート攻撃兵 敵兵:22 d:T_ER:ＤＥ１Ｆ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:23 d:T_ER:ＤＥ１Ｆ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_攻撃兵 敵兵:24 d:T_ER:ＤＥ１Ｆ待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}

proc 攻撃兵セット_ＤＥ連絡橋Ｂ１から {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_攻撃兵 敵兵:21 d:T_ER:ＤＥＢ１待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_LIGHT_SHIELD
	@A_サポート攻撃兵 敵兵:22 d:T_ER:ＤＥＢ１待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL 敵兵:03
	@A_攻撃兵 敵兵:23 d:T_ER:ＤＥＢ１待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
//	@A_攻撃兵 敵兵:24 d:T_ER:ＤＥＢ１待機Ｐ d:ENE_STATUS_GUNLIGHT d:ENE_EQUIP_TYPE_NORMAL
}
