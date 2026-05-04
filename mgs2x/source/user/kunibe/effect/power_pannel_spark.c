//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  power_pannel_spark.c
  電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: power_pannel_spark.c,v 1.1.1.3 2002/11/19 11:44:46 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(2)
#define N_VERTS 		(64)
#define CENTER_NUM 		( N_PRIMS * N_VERTS / 2 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WAVE_HEIGHT		(50.0f)
#define SPEED			(20.0f)

#define	PANNEL_X		(750.0f)
#define	PANNEL_Z		(350.0f)

#define	PLASMA_NUM		(3)


extern void *NewSpritePlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to,
			      float width, float noise_param, FVECTOR *color, int flag, int life );
extern void *NewSpritePlasma2( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color, int life );

extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count );


typedef struct {

    FVECTOR	from;
    FVECTOR	vec_from;
    FVECTOR	to;
    FVECTOR	vec_to;
    int		flag;
    int		cnt;

} PLASMA_PARAM;

typedef	struct {

    GV_ACT_EX		actor;

    FVECTOR		from;
    FVECTOR		to1;
    FVECTOR		to2;

    PLASMA_PARAM	param[PLASMA_NUM];

    FVECTOR		light_pos;
    int			light_cnt;

} Work;



static void MakeFrontPlasma( PLASMA_PARAM *param ) {

    FVECTOR	middle;

    FMATRIX 	mat;
    FVECTOR 	vectmp;
    SVECTOR	rot;

    FVECTOR	color;
    float	width;
    int 	life;

    param->from.vx = 6450.0f + 1300.0f * frnd();
    param->from.vy = 1000.0f + 500.0f * frnd();
    param->from.vz = -246800.0f - 150.0f;
    param->from.vw = 1.0f;

    DG_SetPos( &DG_UnitMatrix );
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = irnd()%4096;
    DG_RotatePos( &rot );
    DG_GetPos( &mat );
    
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], 1000.0f + 200.0f * frnd() );
    _sceVu0AddVector( &param->to, &param->from, &vectmp );

    _sceVu0AddVector( &middle, &param->from, &param->to );
    _sceVu0ScaleVector( &middle, &middle, 0.50f );
    //middle.vz += 200.0f + frnd()*50.0f;
    middle.vz += 300.0f + frnd()*50.0f;
    
    color.vx = 32.0f;
    color.vy = 64.0f;
    color.vz = 255.0f;
    color.vw = 128.0f;
    
    width = 15.0f + frnd()*5.0f;

    /*life = 5 + irnd()%10;
    param->cnt = life + irnd()%30;*/

    if ( irnd()%10 == 0 ) {
	param->vec_from.vx = frnd()*25.0f;
	param->vec_from.vy = frnd()*25.0f;
	param->vec_from.vz = 0.0f;

	param->vec_to.vx = frnd()*25.0f;
	param->vec_to.vy = frnd()*25.0f;
	param->vec_to.vz = 0.0f;

	param->flag = 2;
	life = 20 + irnd()%10;
	param->cnt = life + irnd()%30;
    }
    else {
	life = 5 + irnd()%5;
	param->cnt = life + irnd()%30;
	param->flag = 0;
    }
    
    NewSpritePlasma( &param->from, &middle, &param->to, width, 0.60f, &color, param->flag, life );
    //NewSpritePlasma2( &param->from, &middle, &param->to, width, &color, life );
	
}


/*static void MakeTopPlasma( PLASMA_PARAM *param ) {

    FVECTOR	middle;

    FMATRIX 	mat;
    FVECTOR 	vectmp;
    SVECTOR	rot;

    FVECTOR	color;
    float	width;
    int 	life;
    

    param->from.vx = 6450.0f + 1300.0f * frnd();
    param->from.vy = 2400.0f - 150.0f;
    param->from.vz = -247150.0f + 200.0f * frnd();
    param->from.vw = 1.0f;
    
    DG_SetPos( &DG_UnitMatrix );
    rot.vx = 0;
    rot.vy = irnd()%4096;
    rot.vz = 0;
    DG_RotatePos( &rot );
    DG_GetPos( &mat );
    
    _sceVu0ScaleVector( &vectmp, (FVECTOR *)mat.m[0], 300.0f + 50.0f * frnd() );
    _sceVu0AddVector( &param->to, &param->from, &vectmp );		
    
    _sceVu0AddVector( &middle, &param->from, &param->to );
    _sceVu0ScaleVector( &middle, &middle, 0.50f );
    middle.vy += 250.0f + frnd()*50.0f;
    
    color.vx = 32.0f;
    color.vy = 64.0f;
    color.vz = 255.0f;
    color.vw = 128.0f;
    
    width = 15.0f + frnd()*5.0f;
    
    life = 10 + irnd()%20;
    param->cnt = life + irnd()%60;
    
    NewSpritePlasma( &param->from, &middle, &param->to, width, 0.40f, &color, 0, life );
    //NewSpritePlasma2( &param->from, &middle, &param->to, width, &color, life );

}*/


/* アクト関数 */
static void Act( Work *work )
{


    int		i;
    FVECTOR	color;
    FMATRIX	mat;
    SVECTOR	rot_vec,rot_wide;

    FVECTOR		light_pos;
    
    PLASMA_PARAM	*param;


    param = work->param;


    for ( i = 0 ; i < PLASMA_NUM ; i++ ) {
    
	if ( --param->cnt == 2 ) {

	    switch ( irnd()%2 ) {
	    case 0:	/* 正面 */
		MakeFrontPlasma( param );
		break;
	    case 1:	/* 上面 */
		MakeFrontPlasma( param );
		break;
	    }	    
	    
	    color.vx = 32.0f;
	    color.vy = 64.0f;
	    color.vz = 255.0f;
	    color.vw = 128.0f;

	    /* たまに火花呼び出し */
	    switch ( irnd()%5 ) {
	    case 0:
		DG_COPY_MAT( &mat, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR *)mat.m[3], &param->to );
		rot_vec.vx = 3800-512;
		rot_vec.vy = 4096-512;
		rot_vec.vz = 1;
		rot_wide.vx = 1024;
		rot_wide.vy = 1024;
		rot_wide.vz = 1;
		NewSpark2( 20,
		       &mat,
		       1.0F,
		       100.0F + 25.0F * frnd(),
		       10.0F,
		       &rot_vec, &rot_wide,
		       &color,
		       1.0F,
		       20 );
		break;
	    case 1:
		DG_COPY_MAT( &mat, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR *)mat.m[3], &param->from );
		rot_vec.vx = 3800-512;
		rot_vec.vy = 4096-512;
		rot_vec.vz = 1;
		rot_wide.vx = 1024;
		rot_wide.vy = 1024;
		rot_wide.vz = 1;
		NewSpark2( 20,
		       &mat,
		       1.0F,
		       125.0F + 25.0F * frnd(),
		       10.0F,
		       &rot_vec, &rot_wide,
		       &color,
		       1.0F,
		       20 );
		break;
	    default:
		break;
	    }

	    work->light_cnt = 5;

	}
	else {

	    if ( param->flag == 2 ) {
		_sceVu0AddVector( &param->from, &param->from, &param->vec_from );
		_sceVu0AddVector( &param->to, &param->to, &param->vec_to );

		_sceVu0ScaleVector( &param->vec_from, &param->vec_from, 0.6f );
		_sceVu0ScaleVector( &param->vec_to, &param->vec_to, 0.6f );
	    }

	}

	param++;

    }


    if ( work->light_cnt > 0 ) {
	light_pos.vx = 6500.0f;
	light_pos.vy = 1250.0f;
	light_pos.vz = -246250.0f;
	light_pos.vw = 1.0f;
	DG_SetTmpLight2 (
			 &light_pos,
			 1000.0f, 2000.0f,
			 0x0f | 0x4f<<8 | 0xff<<16,
			 LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY );
	work->light_cnt--;
    }

}




static void Die(Work *work )
{
}


static void InitWork( Work *work )
{
    int 	i;
    PLASMA_PARAM	*param;

    param = work->param;


    for ( i = 0 ; i < PLASMA_NUM ; i++ ) {
	param->cnt = 10+irnd()%10;
	param->flag = 0;
	param++;
    }

    work->light_cnt = 0;
    
}


void *NewPowerPannelSpark( void )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
	    InitWork( work );
	}

	return (void *)work ;

}




