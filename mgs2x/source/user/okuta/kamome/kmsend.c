/*
    kmsend.c
    カモメメッセージ送信処理
    2001/06/12 Masafumi Okuta
    $Id: kmsend.c,v 1.1.1.3 2002/11/19 11:48:04 Yoshizawa1 Exp $
*/

// プレイヤー攻撃命令
static void KMMNG_SendMessageNumAttackPlayer( KAMOME_MNG* kmmng )
{
    if ( GM_PlayerControl == NULL ) return; 
    {
	GV_MSG msg ;
	u_int buffer[  ] = { KMNG_MESS_NUM_ATTACK, KMM_GetDemoRandom( kmmng, 2, 4), 
			   GM_PlayerControl->name, COUNT_VMODE(180)};
	
	msg.address = kmmng->name;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
}
