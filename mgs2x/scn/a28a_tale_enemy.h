/*
	a28a_tale_enemy.h
		LµÓ(¥¹¥Í¡¼¥¯¥Æ¥¤¥ë¥ºÍÑÅ¨Ê¼ÀßÄê¥Õ¥¡¥¤¥ë)

	2002/06/12 H.Yoshiike
	$Id: a28a_tale_enemy.h,v 1.2 2002/07/02 10:31:44 usr03379 Exp $                      

	
*/

//------------------------------------------------------------
// Å¨Ê¼¤Ë´Ø¤¹¤ëÀßÄê
//------------------------------------------------------------
//---------------
// Å¨Ê¼¥ë¡¼¥È
//---------------
enum T_ER {
	£Ë£ÌÂÔµ¡£Ð = 0,
	£ÌµÓÆî½ä²ó£Ò,
	£ÌµÓËÌ½ä²ó£Ò
}


//---------------
// ·ÙÈ÷Ê¼¤Î¥»¥Ã¥È
//---------------
proc ·ÙÈ÷Ê¼¥»¥Ã¥È_¤Ï¤·¤´¤«¤é {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_·ÙÈ÷Ê¼ Å¨Ê¼:01 d:T_ER:£ÌµÓÆî½ä²ó£Ò 0 d:ENE_STATUS_URBAN 52500,0,-207000 £ÌµÓ
	@A_·ÙÈ÷Ê¼ Å¨Ê¼:02 d:T_ER:£ÌµÓËÌ½ä²ó£Ò 0 d:ENE_STATUS_URBAN 52000,0,-212250 £ÌµÓ
}

proc ·ÙÈ÷Ê¼¥»¥Ã¥È_£Ë£ÌÏ¢Íí¶¶¤«¤é {
	#if d:DEBUG_PRINT 
		print 'watcher_set-snake_tales'
	#endif

	@A_·ÙÈ÷Ê¼ Å¨Ê¼:01 d:T_ER:£ÌµÓÆî½ä²ó£Ò 0 d:ENE_STATUS_URBAN 52500,0,-207000 £ÌµÓ
	@A_·ÙÈ÷Ê¼ Å¨Ê¼:02 d:T_ER:£ÌµÓËÌ½ä²ó£Ò 0 d:ENE_STATUS_URBAN 52000,0,-212250 £ÌµÓ
}





//---------------
// ¹¶·âÊ¼¤Î¥»¥Ã¥È
//---------------
proc ¹¶·âÊ¼¥»¥Ã¥È_£Ë£ÌÏ¢Íí¶¶¤«¤é {
	#if d:DEBUG_PRINT 
		print 'attacker_set-snake_tales'
	#endif

	@A_¥µ¥Ý¡¼¥È¹¶·âÊ¼ Å¨Ê¼:21 d:T_ER:£Ë£ÌÂÔµ¡£Ð d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_SHOTGUN Å¨Ê¼:01
//	@A_¹¶·âÊ¼ Å¨Ê¼:22 d:T_ER:£Ë£ÌÂÔµ¡£Ð d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_HITECH_1
	@A_¹¶·âÊ¼ Å¨Ê¼:22 d:T_ER:£Ë£ÌÂÔµ¡£Ð d:ENE_STATUS_NORMAL d:ENE_EQUIP_TYPE_NORMAL
}
