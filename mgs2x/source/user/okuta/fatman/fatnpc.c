/*
    fatnpc.c
    NPCシステムからファットマン用にローカライズした関数
    2001/03/22 Masafumi Okuta
    $Id: fatnpc.c,v 1.1.1.3 2002/11/19 11:48:00 Yoshizawa1 Exp $
*/

// パッドから動作設定
static void FAT_SetModeFromPad( NPCWORK *npc, NPCACTMODE actmode, int mar, int mot, int pad )
{
    NPC_MotionLayerOverOff( npc, 1); 	// ブレンドやめる
    NPC_MotionLayerOverOff( npc, 2); 	// ブレンドやめる : 現在は顔強制修正がはいっている
    NPC_SetModeFromPad( npc, actmode, mar, mot, pad );
}

// モーション再生
static void FAT_SetActMotion( NPCWORK* npc, int base_mar, int mot)	
{
    npc->ctrl->rot.vz = 0;			// 傾きクリア
    npc->ctrl->turn.vz = 0;			// 傾きクリア
    NPC_SetActMotion( npc, base_mar, mot);	// モーション再生	

    {	// モーション再生割合をクリア::ほっとくと１フレーム遅れるので
	Work*	work;
	work = (Work*)npc->character;
	work->fFatPlayRate = 0.f;	
    }
}
// モーション再生

 /* t.morita 2002.03.11 */ 
static void FAT_SetActMotionEX( NPCWORK	*npc, int n_layer, int mar, int mot, u_long64 mask, int interp, float speed )
{
    npc->ctrl->rot.vz = 0;			// 傾きクリア
    npc->ctrl->turn.vz = 0;			// 傾きクリア
    NPC_SetActMotionEX( npc, n_layer, mar, mot, mask, interp, speed );

    {	// モーション再生割合をクリア::ほっとくと１フレーム遅れるので
	Work*	work;
	work = (Work*)npc->character;
	work->fFatPlayRate = 0.f;	
    }
}

// 体の傾き計算
#define INCLINE_ANG (64)
static void FAT_Incline( CONTROL* ctrl )
{
    int		incline ;

    incline = GV_DiffDirS( ctrl->turn.vy, ctrl->rot.vy );
    if ( incline > INCLINE_ANG ) incline = INCLINE_ANG;
    else if ( incline < -INCLINE_ANG ) incline = -INCLINE_ANG;
    ctrl->turn.vz = incline;    
}
