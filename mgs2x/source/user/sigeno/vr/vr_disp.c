//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vr_disp.c
	VR中の表示
	2002/02/12 K.Sigeno
	$Id: vr_disp.c,v 1.1.1.3 2002/11/19 11:49:57 Yoshizawa1 Exp $
*/

#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>

#endif

#include	"gameheader.h"
#include	"vr.h"
/**/
#define ST_STOP			(0x01)
#define ST_INVISIBLE	(0x02)
#define ST_DESTOROY		(0x04)
#define ST_DISP_ZERO	(0x08)

/*内部ステータス*/
#define INT_ST_INC	(0x01)
#define INT_ST_BOM	(0x02)
#define INC_SPEED (19)
#define WAR_LEFT	DIRECT_TICK(60*10)

extern int BP_FRAMES_PER_SEC();
#define SEC_FRAME (BP_FRAMES_PER_SEC())

#define MIN_FRAME (SEC_FRAME*60)


#define COUNT_MAX (MIN_FRAME*99)


typedef struct {
	GV_ACT_EX	actor ;
	int			name ;
	int			proc_id ;
	int inter_s ; /*内部ステータス*/
	int disp_time ; /*表示上の数値*/
	int	mode;
} Work;
enum {
	MSG_STOP = 0 ,
	MSG_START,
	MSG_INC,
	MSG_VISIBLE
};
enum {
	MODE_MIN = 0,
	MODE_SEC ,
	MODE_BOMBSEC
};

extern void SIG_PadPos(int pad,int mode,float *x,float *y,float def_sp,float rate) ;


#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN)
//#define MENU_MODE_OFF	(MENU_RADIO_ON|MENU_WEAPON_OPEN|MENU_ITEM_OPEN|MENU_WEAPON_OFF|MENU_ITEM_OFF|MENU_RADAR_OFF|MENU_GAGE_OFF|MENU_CAPTION_OFF|MENU_SUBWIN_OFF)


#if 0
static void TimeUVSet(SPR_OBJ **spr,float width,int time){
	SPR_OBJ **sprite ;
	float w ;
	sprite = spr;
	w = (float) (time/10) ;
	SetUVSpr(*sprite,w*width);
	sprite++;
	w = (float) (time%10) ;
	SetUVSpr(*sprite,w*width);
//	w = (float) (time/10) ;
//	SetUVSpr(work->min[1],w*FONT_WIDTH,0.0F);
}
#endif
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))


#if 0
static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case MSG_STOP :
					TimerStatus |= ST_STOP ;
					break ;
				case MSG_START :
					TimerStatus &= ~ST_STOP ;
					break ;
				case MSG_INC :
//					work->time += DIRECT_TICK(msg->message[ 1 ]) ;
					work->inter_s |= INT_ST_INC ;
					SigTimerLeft += DIRECT_TICK(msg->message[ 1 ]) ;
					break ;
				case MSG_VISIBLE :
					if(msg->message[ 1 ] == 0){
						/*消す*/
						TimerStatus |= ST_INVISIBLE ;
					}else if(msg->message[ 1 ] == 1){
						/*出す*/
						TimerStatus &= ~ST_INVISIBLE ;
					}
					break ;
			}
			msg++;
		}
	}
}
#endif
#define SC_X	500
#define SC_Y	160
#define	SC_DIF_Y 15
static void Act( Work *work )
{
	int	disp ,min[2],sec[2],m_sec[2];
	MENU_Color( 180,180,180,0x80 );
	if(VR_TARGET_MAX !=0){
		MENU_Locate( SC_X, SC_Y, MENU_MODE_RIGHT );
		MENU_Printf( "TARGET");
//	MENU_Locate( SC_X, (SC_Y+10), MENU_MODE_RIGHT );
		MENU_Locate( SC_X, (SC_Y+15), MENU_MODE_RIGHT );
//	MENU_Printf( "%d",VR_SCORE );
		MENU_Printf( "%d/%d",VR_TARGET_NUM ,VR_TARGET_MAX );
		MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *2), MENU_MODE_RIGHT );
		MENU_Printf( "SCORE");
		MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *3), MENU_MODE_RIGHT );
		MENU_Printf( "%d",VR_SCORE);
	}
	if(VR_ENEMY_MAX !=0){
		MENU_Locate( SC_X, SC_Y, MENU_MODE_RIGHT );
		MENU_Printf( "ENEMY");
		MENU_Locate( SC_X, (SC_Y+15), MENU_MODE_RIGHT );
		MENU_Printf( "%d/%d",VR_ENEMY_NUM ,VR_ENEMY_MAX );
//		MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *2), MENU_MODE_RIGHT );
//		MENU_Printf( "SCORE");
//		MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *3), MENU_MODE_RIGHT );
//		MENU_Printf( "%d",VR_SCORE);
	}

	MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *4), MENU_MODE_RIGHT );
	MENU_Printf( "TIME");
	MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *5), MENU_MODE_RIGHT );

	disp = (VR_TIME/MIN_FRAME) ;
	min[0] = disp/10 ;
	min[1] = disp%10 ;
	disp = (VR_TIME/SEC_FRAME) ;
	disp %= 60 ;
	sec[0] = disp/10 ;
	sec[1] = disp%10 ;
	disp = (VR_TIME%SEC_FRAME) ;
	disp = (disp*100)/SEC_FRAME ;
	m_sec[0] = disp/10 ;
	m_sec[1] = disp%10 ;

	MENU_Printf( "%d%d '%d%d ''%d%d",min[0],min[1],sec[0],sec[1],m_sec[0],m_sec[1] );


	MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *6), MENU_MODE_RIGHT );
	MENU_Printf( "COMBO TIME %d",VR_COMBO_CNT);
	MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *7), MENU_MODE_RIGHT );
	MENU_Printf( "COMBO_CHAIN %d",VR_COMBO_CHAIN_MAX);
	MENU_Locate( SC_X, (SC_Y+SC_DIF_Y *8), MENU_MODE_RIGHT );
	MENU_Printf( "RATE %d",VR_COMBO_RATE);



}


static void Die( Work *work )
{
}
static int GetResources( Work *work ,int name,int where )
{
//	VR_SCORE = 0 ;
#if 0
	if ( GCL_GetOption( 'k' ) != NULL ) {
	}else {
	}
#endif
	return 0;
}

/*プログラム呼び出し版*/
void *NewVRprint( int name, int where )
{
	Work		*work;
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
