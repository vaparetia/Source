//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  tng_stage_monitor_42a.c
  天狗兵ステージノイズモニター制御キャラ( w42a )
  2001/08/16 Yuuta Kunibe
  $Id: tng_monitor_control_42a.c,v 1.1.1.3 2002/11/19 11:44:53 Yoshizawa1 Exp $
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


#define	FLAME_TEX1		( GV_StrCode("w42a_efct_disp_01_alp_add_ovl") )
#define	FLAME_TEX2		( GV_StrCode("w42a_efct_disp_02_alp_add_ovl") )
#define	FLAME_TEX3		( GV_StrCode("w42a_efct_disp_03_alp_add_ovl") )
#define	FLAME_TEX4		( GV_StrCode("w42a_efct_disp_04_alp_add_ovl") )
#define	FLAME_TEX5		( GV_StrCode("w42a_efct_disp_05_alp_add_ovl") )

#define	MONITOR_TEX1	( GV_StrCode("w42a_efct_disp_01_scr_alp_add_ovl") )
#define	MONITOR_TEX2	( GV_StrCode("w42a_efct_disp_02_scr_alp_add_ovl") )
#define	MONITOR_TEX3	( GV_StrCode("w42a_efct_disp_03_scr_alp_add_ovl") )
#define	MONITOR_TEX4	( GV_StrCode("w42a_efct_disp_04_scr_alp_add_ovl") )
#define	MONITOR_TEX5	( GV_StrCode("w42a_efct_disp_05_scr_alp_add_ovl") )

#define NOISE_TEX1		( GV_StrCode("w42a_efct_disp_01_noiz_alp_add_ovl") )
#define NOISE_TEX2		( GV_StrCode("w42a_efct_disp_02_noiz_alp_add_ovl") )
#define NOISE_TEX3		( GV_StrCode("w42a_efct_disp_03_noiz_alp_add_ovl") )
#define NOISE_TEX4		( GV_StrCode("w42a_efct_disp_04_noiz_alp_add_ovl") )
#define NOISE_TEX5		( GV_StrCode("w42a_efct_disp_05_noiz_alp_add_ovl") )


#define	NOISE_NUM		(6)


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int 	map;

    int		noise_flag[NOISE_NUM];
    int		noise_count[NOISE_NUM];
    
} Work ;





static void Act( Work *work )
{

	int i;
	
	for ( i = 0 ; i < NOISE_NUM ; i++ ) {

		if ( work->noise_count[i] < DIRECT_TICK( 30 ) ) {
			work->noise_flag[i] = 1;
		}
		else {
			work->noise_flag[i] = 0;
		}

		if ( work->noise_count[i] == 0 ) {
			work->noise_count[i] = DIRECT_TICK( 150 + irnd()%150 );
		}
		else {
			work->noise_count[i]--;
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


	// 空中文字 1-1
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-24265.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字 1-2
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-36263.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字 1-3
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-48265.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );


	// 空中文字 1-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-27240.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字 1-5
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-39240.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字 1-6
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-51240.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6750.0f;
	offset_y = 5508.0f;
	width    = 1625.0f;
	height   = 850.0f; 
	scroll_x = 0;
	scroll_y = 222;
	scroll_w = 1000;
	scroll_h = 653;
	v_start = 0;
	v_end   = 480;
	v_width = 520;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX1,
						 MONITOR_TEX1,
						 NOISE_TEX1,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );


	// 空中文字 2-1
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-28350.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-2
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-40350.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 2-3
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-52350.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-22400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 2-5
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-34400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 2-6
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-46400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-7
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-29010.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-8
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-41010.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 2-9
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-53010.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7240.0f;
	offset_y = 7958.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-10
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-23150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 2-11
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-35150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 2-12
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-47150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 6500.0f;
	offset_y = 6857.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 618;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX2,
						 MONITOR_TEX2,
						 NOISE_TEX2,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );


	// 空中文字 3-1
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-21920.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 3-2
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-33920.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 3-3
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-45920.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 3-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-29580.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 3-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-41580.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 3-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-53580.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 7505.0f;
	offset_y = 7730.0f;
	width    = 1000.0f;
	height   = 650.0f; 
	scroll_x = 0;
	scroll_y = 295;
	scroll_w = 1000;
	scroll_h = 612;
	v_start = 0;
	v_end   = 350;
	v_width = 650;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX3,
						 MONITOR_TEX3,
						 NOISE_TEX3,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 4-1
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-29095.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 4-2
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-41095.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 4-3
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-53095.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 3072;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 4-4
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-22410.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 4-5
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-34410.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 4-6
    pos.vx   = 13500.0f;
    pos.vy   = 0.0f;
    pos.vz   =-46410.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 1024;
	rot.vz   = 0;
	radius   = 5740.0f;
	offset_y = 8358.0f;
	width    = 800.0f;
	height   = 550.0f; 
	scroll_x = 0;
	scroll_y = 192;
	scroll_w = 1000;
	scroll_h = 622;
	v_start = 0;
	v_end   = 417;
	v_width = 583;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX4,
						 MONITOR_TEX4,
						 NOISE_TEX4,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 5-1
    pos.vx   = 9000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-29150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 0;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[0] );

	// 空中文字 5-2
    pos.vx   = 9000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-41150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 0;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[1] );

	// 空中文字 5-3
    pos.vx   = 9000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-53150.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 0;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[2] );

	// 空中文字 5-4
    pos.vx   = 18000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-22400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 2048;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[3] );

	// 空中文字 5-5
    pos.vx   = 18000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-34400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 2048;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[4] );

	// 空中文字 5-6
    pos.vx   = 18000.0f;
    pos.vy   = 0.0f;
    pos.vz   =-46400.0f;
    pos.vw   = 1.0f;
	rot.vx   = 0;
	rot.vy   = 2048;
	rot.vz   = 0;
	radius   = 1500.0f;
	offset_y = 7100.0f;
	width    = 1000.0f;
	height   = 500.0f; 
	scroll_x = 0;
	scroll_y = 94;
	scroll_w = 1000;
	scroll_h = 806;
	v_start = 0;
	v_end   = 438;
	v_width = 562;
	v_time  = 3000;
	NewTengStageMonitor( &pos, &rot, radius, offset_y, width, height,
						 FLAME_TEX5,
						 MONITOR_TEX5,
						 NOISE_TEX5,
						 scroll_x, scroll_y, scroll_w, scroll_h,
						 v_start, v_end, v_width, v_time, &work->noise_flag[5] );


}



static int GetResources( Work *work )
{

    int 	i;
    
    for ( i = 0 ; i < NOISE_NUM ; i++ ) {
		work->noise_flag[i]  = 0;
		work->noise_count[i] = 0;
    }

	/* モニター生成 */
    CallStageMonitor( work );
	
    return 0;

}



void *NewTengMonitorControlW42a( int name, int map )
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
