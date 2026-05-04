//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_thunder.c
	稲光（呼び出し部）
	2000/04/06 S.Okajima
	$Id: d_thunder.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

/* 画面前基準Ｚ */
#define	SCREEN_NEAR	(51.0f)

#define	HEIGHT	 (200000.0F)

#define	BASE_WIDTH (SCREEN_NEAR*0.03f)


extern int big_thund_flash_flag ;
extern void OK_RemoveDynamicLight( FVECTOR *pos );
extern void OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot );
extern FVECTOR big_thund_flash_force_vector;
extern float DG_ClipFar;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR		sc_pos[2];
//	FVECTOR		light_source;
} Work ;

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_DestroyActor( work ) ;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
//	OK_RemoveDynamicLight( &work->light_source );
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, float sx, float sy, float ex, float ey )
{
	FVECTOR	cam;
	FVECTOR	fvtemp;
	FVECTOR	fvtemp1;
	FVECTOR	fvcalc[2];
	float	len;
	float	inv_screen_near_x;
	float	inv_screen_near_y;

	float distance;
	int branch_num;
	int life ;

	big_thund_flash_flag = 1;

//	distance = DG_ClipFar * 0.5f;
	distance = 1000000.0f;
	branch_num = 2;
	life = 24;

	inv_screen_near_x = DG_Chanls->screen * distance * 0.25f / (ASPECT_X()                                           ) ;
	inv_screen_near_y = DG_Chanls->screen * distance * 0.25f / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) ;
	work->sc_pos[0].vx = sx * inv_screen_near_x;
	work->sc_pos[0].vy = sy * inv_screen_near_y;
	work->sc_pos[0].vz = distance;
	work->sc_pos[1].vx = ex * inv_screen_near_x;
	work->sc_pos[1].vy = ey * inv_screen_near_y;
	work->sc_pos[1].vz = distance;


//	if( work->sc_pos[1].vy < 500000.0f ) work->sc_pos[1].vy = 500000.0f;


	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	/* カメラ座標をワールド座標に変換 */
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( work->sc_pos, work->sc_pos, 2 );

//	_sceVu0SubVector( &fvtemp, &work->sc_pos[0], &cam ) ;
//	_sceVu0Normalize( &fvtemp, &fvtemp );
//	_sceVu0ScaleVector( &fvtemp, &fvtemp, 3000.0f );
//	_sceVu0AddVector( &work->light_source, &fvtemp, &cam ) ;
//	AN_Test_Eye2( &fvtemp, 2 );
//	OK_SetDynamicLight( &work->light_source, &DG_ZeroSVector );

	_sceVu0SubVector( &fvtemp, &work->sc_pos[0], &cam ) ;
	len = GV_VecLen3F( &fvtemp );
	fvcalc[0].vx =-BASE_WIDTH;
	fvcalc[0].vy = 0.0f;
	fvcalc[0].vz = len;
	fvcalc[1].vx = BASE_WIDTH;
	fvcalc[1].vy = 0.0f;
	fvcalc[1].vz = len;

	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( fvcalc, fvcalc, 2 );

	_sceVu0SubVector( &fvtemp, &fvcalc[0], &fvcalc[1] ) ;
	len = GV_VecLen3F( &fvtemp );

//printf(":::::::::::::::::::::::::::::::::::::len:%f\n",len);


	fvtemp.vx = work->sc_pos[1].vx - work->sc_pos[0].vx;
	fvtemp.vy = work->sc_pos[1].vy - work->sc_pos[0].vy;
	fvtemp.vz = work->sc_pos[1].vz - work->sc_pos[0].vz;

	_sceVu0SubVector( &fvtemp, &work->sc_pos[0], &cam ) ;
	_sceVu0Normalize( &fvtemp1, &fvtemp );
	_sceVu0ScaleVector( &fvtemp, &fvtemp1, HEIGHT-cam.vy );
	_sceVu0AddVector( &big_thund_flash_force_vector, &fvtemp, &cam ) ;

	DG_COPY_VEC( &big_thund_flash_force_vector, &work->sc_pos[0] );
	{
		extern void *NewThunderParts_Demo( FVECTOR *pos0, FVECTOR *pos1, float radius, int branch_num, int life );
		NewThunderParts_Demo( &work->sc_pos[0], &work->sc_pos[1], len, branch_num, life - 1 );
	}

	return (0);
}

/* ---------------------------------------------------------------- */
void *NewThunder_Demo( float sx, float sy )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, sx, sy, sx + frnd()*0.1f, sy + rnd()*0.2f + 0.2f ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
void *NewFreeThunder( int name, int where )
{
	Work		*work ;
	float	fx0,fy0,fx1,fy1;

	OPERATOR() ;

	fx0 = 0.0f;
	if ( GCL_GetOption( 'x' ) != NULL ){
		fx0 = (float)GCL_GetNextInt() / 4096.0f ;
	}
	fy0 = 0.0f;
	if ( GCL_GetOption( 'y' ) != NULL ){
		fy0 = (float)GCL_GetNextInt() / 4096.0f ;
	}
	fx1 = 0.0f;
	if ( GCL_GetOption( 'u' ) != NULL ){
		fx1 = (float)GCL_GetNextInt() / 4096.0f ;
	}
	fy1 = 0.0f;
	if ( GCL_GetOption( 'v' ) != NULL ){
		fy1 = (float)GCL_GetNextInt() / 4096.0f ;
	}

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, fx0,fy0,fx1,fy1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}






