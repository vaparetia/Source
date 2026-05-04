//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  tng_stage_monitor.c
  天狗兵ステージノイズモニター制御キャラ
  2001/08/16 Yuuta Kunibe
  $Id: tng_monitor_control.c,v 1.1.1.3 2002/11/19 11:44:53 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"



#define	N_PRIMS		(1)
#define	N_VERTS		(4)

#define BASE_RGB	(24)
#define ALPHA		(64)
#define	SPEED		(1200.0f)

#define LIFE		(300)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


#define	SIZE_PARAM	(1000.0f)


#define	FLAME_TEX1		( GV_StrCode("w45a_efct_disp_01_alp_add_ovl") )
#define	FLAME_TEX2		( GV_StrCode("w45a_efct_disp_02_alp_add_ovl") )
#define	FLAME_TEX3		( GV_StrCode("w45a_efct_disp_03_alp_add_ovl") )
#define	FLAME_TEX4		( GV_StrCode("w45a_efct_disp_04_alp_add_ovl") )
#define	FLAME_TEX5		( GV_StrCode("w45a_efct_disp_05_alp_add_ovl") )
#define FLAME_TEX6		( GV_StrCode("w45a_efct_disp_06_alp_add_ovl") )

#define	MONITOR_TEX1	( GV_StrCode("w45a_efct_disp_01_scr_alp_add_ovl") )
#define	MONITOR_TEX2	( GV_StrCode("w45a_efct_disp_02_scr_alp_add_ovl") )
#define	MONITOR_TEX3	( GV_StrCode("w45a_efct_disp_03_scr_alp_add_ovl") )
#define	MONITOR_TEX4	( GV_StrCode("w45a_efct_disp_04_scr_alp_add_ovl") )
#define	MONITOR_TEX5	( GV_StrCode("w45a_efct_disp_05_scr_alp_add_ovl") )
#define MONITOR_TEX6	( GV_StrCode("w45a_efct_disp_06_noiz_alp_add_ovl") )	// dummy

#define NOISE_TEX1		( GV_StrCode("w45a_efct_disp_01_noiz_alp_add_ovl") )
#define NOISE_TEX2		( GV_StrCode("w45a_efct_disp_02_noiz_alp_add_ovl") )
#define NOISE_TEX3		( GV_StrCode("w45a_efct_disp_03_noiz_alp_add_ovl") )
#define NOISE_TEX4		( GV_StrCode("w45a_efct_disp_04_noiz_alp_add_ovl") )
#define NOISE_TEX5		( GV_StrCode("w45a_efct_disp_05_noiz_alp_add_ovl") )
#define NOISE_TEX6		( GV_StrCode("w45a_efct_disp_06_noiz_alp_add_ovl") )


// モニターステータス
enum {
	MONITOR_NOISE_OFF = 0,
	MONITOR_NOISE_ON,
	MONITOR_INVISIBLE,
	MONITOR_KILL,
};

// メッセージインデックス
enum {
	MONITOR_MSG_VISIBLE = 0,
	MONITOR_MSG_INVISIBLE,
	MONITOR_MSG_KILL,
};

typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int 	map;

	int		code[3];

    int		noise_flag[3];
	int		noise_count[3];
    
} Work ;



static int RecieveMessage( Work* work )
{
    
    GV_MSG*	msg;
	int		i;
    int 	n_msg;
	int		ret;

	ret = 0;	

    if ( work->name == 0 ) {
		return ret;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

		switch ( msg->message[ 0 ] ) {
		case 0:		// visible
			work->noise_flag[ msg->message[ 1 ] ] = MONITOR_NOISE_OFF;
			printf("recieve_message : visible\n");
			break;
		case 1:		// invisible
			work->noise_flag[ msg->message[ 1 ] ] = MONITOR_INVISIBLE;
			printf("recieve_message : invisible\n");
			break;
		case 2:		// kill
			for ( i = 0 ; i < 3 ; i++ ) {
				work->noise_flag[i] = MONITOR_KILL;
			}
			printf("recieve_message : kill\n");
			ret = 1;
			break;
		default:
			break;
		}
		msg++;

    }

	return ret;

}



static void Act( Work *work )
{

	int i;
    GV_MSG	msg;


	/*if ( GV_PadData[1].press & PAD_A ) {
		work->code[0] = MONITOR_MSG_VISIBLE;
		work->code[1] = 0;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 2;
		GV_SendMessage( &msg );

		printf("send_message : visible\n");
	}
	if ( GV_PadData[1].press & PAD_B ) {
		work->code[0] = MONITOR_MSG_INVISIBLE;
		work->code[1] = 0;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 2;
		GV_SendMessage( &msg );

		printf("send_message : invisible\n");
	}
	if ( GV_PadData[1].press & PAD_X ) {
		work->code[0] = MONITOR_MSG_KILL;
		work->code[1] = 0;

		msg.address = work->name;
		msg.message = work->code;
		msg.message_len = 2;
		GV_SendMessage( &msg );

		printf("send_message : kill\n");
	}*/


	if ( RecieveMessage( work ) ) {
		GV_DestroyActor( work );
		return;
	}

	for ( i = 0 ; i < 3 ; i++ ) {

		if ( work->noise_flag[i] != MONITOR_INVISIBLE ) {	// インビジブル時はカウンタ更新しない。

			if ( work->noise_count[i] < DIRECT_TICK( 30 ) ) {
				work->noise_flag[i] = MONITOR_NOISE_ON;
			}
			else {
				work->noise_flag[i] = MONITOR_NOISE_OFF;
			}

			if ( work->noise_count[i] == 0 ) {
				work->noise_count[i] = DIRECT_TICK( 150 + irnd()%150 );
			}
			else {
				work->noise_count[i]--;
			}

		}
		
	}

}

static void Die( Work *work )
{
}


static void CallStageMonitor( Work *work )
{

    extern void *NewTengStageMonitor( FVECTOR *pole_pos, SVECTOR *pole_rot, float radius, float offset_y,
									  float width, float height,
									  int flame_tex, int scroll_tex, int noise_tex,
									  float scroll_x, float scroll_y, float scroll_w, float scroll_h,
									  float v_start, float v_end, float v_width, float v_time, int *mode );
    FVECTOR		pos;
    SVECTOR 	rot;
    float		radius;
    float		offset_y;
    float		width,height;
    float   	scroll_x, scroll_y, scroll_w, scroll_h;
    float		v_start, v_end, v_width, v_time;


    // 共通軸座標
    pos.vx = 0.0f;
    pos.vy = 0.0f;
    pos.vz = -149250.0f;
    pos.vw = 1.0f;


	// 空中文字３－１
	rot.vx   = 0;
	rot.vy   = 2048;
	rot.vz   = 0;
	radius   = 6000.0f;
	offset_y = 2600.0f;
	width    = 1000.0f;
	height   = 562.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 636;
	v_start = 0;
	v_end   = 650;
	v_width = 350;
	v_time  = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字３－２
	rot.vx   = 0;
	rot.vy   = 3427;
	rot.vz   = 0;	
	radius   = 6000.0f;
	offset_y = 3000.0f;
	width    = 1000.0f;
	height   = 562.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 636;	
	v_start  = 0;
	v_end    = 650;
	v_width  = 350;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字４－１
	rot.vx   = 0;
	rot.vy   = 1479;
	rot.vz   = 0;	
	radius   = 5700.0f;
	offset_y = 1200.0f;
	width    = 2000.0f;
	height   = 1250.0f; 
	scroll_x = 0;
	scroll_y = 194;
	scroll_w = 1000;
	scroll_h = 722;	
	v_start  = 0;
	v_end    = 459;
	v_width  = 541;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );


	// 空中文字４－２
	rot.vx   = 0;
	rot.vy   = 3243;
	rot.vz   = 0;	
	radius   = 5700.0f;
	offset_y = 1200.0f;
	width    = 2000.0f;
	height   = 1250.0f; 
	scroll_x = 0;
	scroll_y = 194;
	scroll_w = 1000;
	scroll_h = 722;	
	v_start  = 0;
	v_end    = 459;
	v_width  = 541;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字５－１
	rot.vx   = 0;
	rot.vy   = 1650;
	rot.vz   = 0;	
	radius   = 5250.0f;
	offset_y = 1600.0f;
	width    = 1200.0f;
	height   = 562.0f; 
	scroll_x = 0;
	scroll_y = 90;
	scroll_w = 1000;
	scroll_h = 727;	
	v_start  = 0;
	v_end    = 500;
	v_width  = 500;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字５－２
	rot.vx   = 0;
	rot.vy   = 398;
	rot.vz   = 0;	
	radius   = 5250.0f;
	offset_y = 1600.0f;
	width    = 1200.0f;
	height   = 562.0f; 
	scroll_x = 0;
	scroll_y = 90;
	scroll_w = 1000;
	scroll_h = 727;	
	v_start  = 0;
	v_end    = 500;
	v_width  = 500;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字６－１
	rot.vx   = 0;
	rot.vy   = 569;
	rot.vz   = 0;	
	radius   = 6000.0f;
	offset_y = 1000.0f;
	width    = 1400.0f;
	height   = 862.0f; 
	scroll_x = 0;
	scroll_y = 63;
	scroll_w = 1000;
	scroll_h = 875;	
	v_start  = 0;
	v_end    = 344;
	v_width  = 656;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字６－２
	rot.vx   = 0;
	rot.vy   = 2503;
	rot.vz   = 0;	
	radius   = 6000.0f;
	offset_y = 1000.0f;
	width    = 1400.0f;
	height   = 862.0f; 
	scroll_x = 0;
	scroll_y = 63;
	scroll_w = 1000;
	scroll_h = 875;	
	v_start  = 0;
	v_end    = 344;
	v_width  = 656;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字７－１
	rot.vx   = 0;
	rot.vy   = 1536;
	rot.vz   = 0;	
	radius   = 6000.0f;
	offset_y = 2500.0f;
	width    = 1000.0f;
	height   = 1850.0f; 
	scroll_x = 0;
	scroll_y = 78;
	scroll_w = 1000;
	scroll_h = 859;	
	v_start  = 0;
	v_end    = 342;
	v_width  = 658;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字２
	rot.vx   = 0;
	rot.vy   = 2389;
	rot.vz   = 0;	
	radius   = 5410.0f;
	offset_y = 0.0f;
	width    = 1000.0f;
	height   = 2000.0f; 
	scroll_x = 0;
	scroll_y = 0;
	scroll_w = 0;
	scroll_h = 0;	
	v_start  = 0;
	v_end    = 0;
	v_width  = 0;
	v_time   = 1500;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX6,
						 MONITOR_TEX6,
						 NOISE_TEX6,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );


}



static int GetResources( Work *work )
{

    int 	i;
    
    for ( i = 0 ; i < 3 ; i++ ) {
		work->noise_flag[i]  = MONITOR_NOISE_OFF;
		work->noise_count[i] = 0;
    }

	/* モニター生成 */
    CallStageMonitor( work );
	
    return 0;

}



void *NewTengMonitorControl( int name, int map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;

}
