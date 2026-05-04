/*
    fatexternal.c
    ファットマン戦:外部通信用
    2001/04/12 Masafumi Okuta
    $Id: fatextern.c,v 1.1.1.3 2002/11/19 11:47:58 Yoshizawa1 Exp $
*/
#define KMNG_MESS_COMEON 		(32)
#define KMNG_MESS_COMEON_END		(34)
#define CALL_BOX_XZ			(8000.f)

// カモメを呼び寄せる
static void FAT_CallKamome( Work* work, int nNode, int x, int y, int z, int nDir)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { 
	GV_MSG msg ;
	u_int buffer[  ] = { KMNG_MESS_COMEON, (int)(npc->ctrl->mov.vx - CALL_BOX_XZ),(int)(npc->ctrl->mov.vy - 5000.f),(int)(npc->ctrl->mov.vz - CALL_BOX_XZ),
			      (int)(npc->ctrl->mov.vx + CALL_BOX_XZ),(int)(npc->ctrl->mov.vy + 45000.f),(int)(npc->ctrl->mov.vz + CALL_BOX_XZ),
			     work->name, nNode, nDir, 
			     x, y, z, COUNT_VMODE(30)};
	
	msg.address = work->codeKmmngName;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
}
// カモメを飛び立たせる
static void FAT_TakeOffKamome( Work* work, int nDelay)
{
    NPCWORK*	npc;

    npc = &work->npc;
    { // 
	GV_MSG msg ;
	u_int buffer[  ] = { KMNG_MESS_COMEON_END, work->name, nDelay };
	
	msg.address = work->codeKmmngName;
	msg.message = buffer;
	msg.message_len = sizeof(buffer)/sizeof(u_int) ;
	GV_SendMessage( &msg ) ;
    }
}
