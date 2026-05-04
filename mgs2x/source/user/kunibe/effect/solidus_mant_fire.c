//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_mant_fire.c
    ソリダスシュラウド燃え
    2001/06/21 Yuuta Kunibe	
    $Id: solidus_mant_fire.c,v 1.1.1.3 2002/11/19 11:44:50 Yoshizawa1 Exp $
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


#define N_PRIMS		(48)
#define N_VERTS		(16)
#define SPRT_NUM	(N_PRIMS*N_VERTS)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define SIZE		(100.0f)
#define RISING_SPEED	(30.0f)

#define ALPHA_MIN	(40)
#define	ALPHA_WIDTH	(16)

#define	POINT_NUM	(400)
#define FIRE_TEX	( GV_StrCode( "blood_2bw_msk" ) )
#define	LIFE		(4)



#define N_PRIMS2	(8)
#define N_VERTS2	(32)
#define POWDER_NUM	(N_PRIMS2*N_VERTS2)
#define	POWDER_TEX	( GV_StrCode( "bombpowder7_msk" ) )



extern void *NewPointFire( FVECTOR* pos );


typedef struct {

    FVECTOR	*pos;
    int		cnt;

} POINT_PARAM;


typedef struct {

    FVECTOR	vec;
    float	rad;
    float	rot;
    float	rot_add;

} FIRE_PARAM;
    

typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;
    DG_PRIM2	*prim_powder;
    
    FIRE_PARAM	param[SPRT_NUM];

    POINT_PARAM	p_param[POINT_NUM];
    int		barning_point[POINT_NUM];
    int		barning_num;    

    int 	id;
    
} Work;



static Work *Work_SolMantFire = NULL;



/*-------- void SolidusMantGetFirePoint( FVECTOR *new_pos, int num ) ----------
                          炎発生ポイント取得関数
     FVECTOR *new_pos : 炎発生ポイントへのポインタ
     int     num      : 取得するポイント数
-----------------------------------------------------------------------------*/
void SolidusMantGetFirePoint( FVECTOR *new_pos, int num ) 
{

    int		i;
    Work	*work;
    POINT_PARAM	*p_param;


    if ( ( work = Work_SolMantFire ) == NULL ) {
	printf("error!-------------solidus_mant_fire not found!\n");
	ASSERT(0);
    }

    p_param = &work->p_param[work->id];

    for ( i = 0 ; i < num ; i++ ) {
	p_param->pos = new_pos;
	//AN_Test_Eye2( p_param->pos, 500.f );
	
	NewPointFire( p_param->pos );	

	p_param->cnt = LIFE+irnd()%LIFE;
	new_pos++;
	p_param++;
	if ( ++work->id >= POINT_NUM ) {
	    work->id = 0;
	    p_param = work->p_param;
	}

    }

}



static void ActPrim(Work *work)
{

    int			i,j;
    int			clock;
    int			point_id;
    DG_PRIM2		*prim;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    DG_PRIM2_UVRGBWH	*uvrgbwh_pre;
    FIRE_PARAM		*param;

    float		cos;
    float		ftmp;


    prim = work->prim;

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = prim->buffer_clock;

    pos		= prim->pos[clock];
    pos_pre	= prim->pos[1-clock];
    uvrgbwh	= prim->uvrgb[clock];
    uvrgbwh_pre = prim->uvrgb[1-clock];
    param       = work->param;


    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	for ( j = 0 ; j < N_VERTS ; j++ ) {

	    /* スプライト更新 */
	    if ( uvrgbwh_pre->a > ALPHA_MIN ) {
	    
		/* 位置更新 */
		_sceVu0AddVector( pos, pos_pre, &param->vec );	    

		/* 速度更新 */
		param->vec.vx = 12.5f * frnd();
		param->vec.vz = 12.5f * frnd();

		/* 幅,高さ更新 */
		cos = vu0_Cos( param->rot );
		uvrgbwh->w = (short)( param->rad * cos );
		uvrgbwh->h = (short)( param->rad * cos );


		/* 角度更新 */
		param->rot += param->rot_add;
		if ( param->rot > PI ) {
		    param->rot -= TPI;
		}
		else if ( param->rot < -PI ) {
		    param->rot += TPI;
		}


		if ( uvrgbwh->a > ALPHA_MIN - 5 ) {

		    param->rad *= 0.985f;
		
		    param->vec.vy += 0.5f;

		    uvrgbwh->a = uvrgbwh_pre->a - 1;

		}
		else {

		    param->rad *= 0.80f;
		    param->vec.vy += 0.50f;
		    uvrgbwh->a = uvrgbwh_pre->a - 1;

		}
	    
	    }
	    /* スプライト初期化 */
	    else {

		if ( work->barning_num != 0 ) {

		    /* 位置初期化 */
		    point_id = work->barning_point[irnd()%work->barning_num];
		    DG_COPY_VEC( pos, work->p_param[point_id].pos );

		    /* パラメータ初期化 */
		    ftmp = ( 0.5f + frnd() * 0.50f );
		    param->rad = SIZE * ( 0.5f + ftmp );
		    param->rot = PI / 2.0f;

		    param->vec.vx = RISING_SPEED * 25.0f / 30.0f * frnd();
		    param->vec.vy = RISING_SPEED * ( 1.0f - ftmp ) / 1.0f + RISING_SPEED * 2.f / 3.f; 
		    param->vec.vz = RISING_SPEED * 25.0f / 30.0f * frnd();
		    _sceVu0ScaleVector( &param->vec, &param->vec, 0.50f );

		    /* 幅,高さ,アルファ初期化 */
		    cos = vu0_Cos( param->rot );
		    uvrgbwh->w = (short)( param->rad * cos );
		    uvrgbwh->h = (short)( param->rad * cos );
		    uvrgbwh->a = ALPHA_MIN + irnd()%ALPHA_WIDTH;

		}
		else {
		    uvrgbwh->w = 0;
		    uvrgbwh->h = 0;
		    uvrgbwh->a = 0;
		}
	    }

	    pos++;
	    pos_pre++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	    param++;
	
	}

    }
    
}


static void ActPrim2(Work *work)
{

    int			i,j;
    int			clock;
    int			point_id;
    DG_PRIM2		*prim;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    DG_PRIM2_UVRGBWH	*uvrgbwh_pre;



    prim = work->prim_powder;


    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = prim->buffer_clock;

    pos		= prim->pos[clock];
    pos_pre	= prim->pos[1-clock];
    uvrgbwh	= prim->uvrgb[clock];
    uvrgbwh_pre = prim->uvrgb[1-clock];


    for ( i = 0 ; i < N_PRIMS2 ; i++ ) {

	for ( j = 0 ; j < N_VERTS2 ; j++ ) {

	    if ( uvrgbwh_pre->a > 0 ) {

		DG_COPY_VEC( pos, pos_pre );
		pos->vx += frnd()*15.0f;
		pos->vy += 45.0f + frnd()*15.0f;
		pos->vz += frnd()*15.0f;

		if ( uvrgbwh_pre->a > 2 ) {
		    uvrgbwh->a = uvrgbwh_pre->a - 2;
		}
		else {
		    uvrgbwh->a = 0;
		}

	    }
	    else {

		if ( work->barning_num != 0 ) {
 
		    point_id = work->barning_point[irnd()%work->barning_num];
		    DG_COPY_VEC( pos, work->p_param[point_id].pos );

		    uvrgbwh->w = 150.0f;
		    uvrgbwh->h = 150.0f;
		    uvrgbwh->a = 32 + irnd()%16;

		}
		else {
		    DG_COPY_VEC( pos, &DG_ZeroVector );
		    uvrgbwh->w = 0;
		    uvrgbwh->h = 0;
		    uvrgbwh->a = 32 + irnd()%32;
		}

	    }

	    pos++;
	    pos_pre++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	
	}

    }
    
}




static void Act( Work *work )
{

    int 	i;
    POINT_PARAM	*p_param;
    int		*b_point;


    p_param = work->p_param;
    b_point = work->barning_point;
    work->barning_num = 0;


    for ( i = 0 ; i < POINT_NUM ; i++ ) {

	if ( p_param->cnt > 0 ) {
	    *b_point = i;
	    b_point++;
	    work->barning_num++;

	    p_param->cnt--;
	}

	p_param++;

    }

    ActPrim( work );
    ActPrim2( work );
    
}




static void Die( Work *work )
{
    work->prim = OK_FreePrim2( work->prim );    
    work->prim_powder = OK_FreePrim2( work->prim_powder );    
    Work_SolMantFire = NULL;
}




static int InitPacket( Work *work )
{

    int				i,j,k;
    DG_PRIM2			*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGBWH		*uvrgbwh;
    FIRE_PARAM			*param;



    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		return 0;
	}

    prim->raise = 0;

    tex = DG_GetTexture( FIRE_TEX );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

    pos     = SCR_POS;
    uvrgbwh = SCR_UVS;
    param   = work->param;
    
    for ( i = 0 ; i < 4 ; i++ ) {

	pos     = SCR_POS;
	uvrgbwh = SCR_UVS;

	for ( j = 0 ; j < N_PRIMS/4 ; j++ ) {

	    for ( k = 0 ; k < N_VERTS ; k++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );

		switch ( irnd()%4 ) {
		case 0:
		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    break;
		case 1:
		    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    break;
		case 2:
		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    break;
		case 3:
		    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    break;
		}

		uvrgbwh->q0 = 4096;
		uvrgbwh->f0 = 0x0fff;

		uvrgbwh->q1 = 4096;
		uvrgbwh->f1 = 0x0fff;

		uvrgbwh->r  = 128;
		uvrgbwh->g  = 24 + irnd()%24;
		uvrgbwh->b  = 8;
		uvrgbwh->a  = 99 + irnd()%20;

		param->rad = SIZE * ( 1.0f + frnd() * 0.50f );
		param->rot = PI / 2.0f;
		param->rot_add  = TPI * 0.1f * frnd();

		uvrgbwh->w = uvrgbwh->h = 0;//param->rad * vu0_Cos( param->rot );

		DG_COPY_VEC( &param->vec, &DG_ZeroVector );

		pos++;
		uvrgbwh++;
		param++;

	    }
	}

	OK_Scr_Mem( &prim->pos[ 0 ][ i*N_PRIMS/4*N_VERTS ], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
	OK_Scr_Mem( &prim->pos[ 1 ][ i*N_PRIMS/4*N_VERTS ], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
	OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 0 ]) )[ i*N_PRIMS/4*N_VERTS ], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );
	OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 1 ]) )[ i*N_PRIMS/4*N_VERTS ], SCR_UVS,
		    sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );
	
    }		
	

    return 1;
}


static int InitPacket2( Work *work )
{

    int				i;
    DG_PRIM2			*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGBWH		*uvrgbwh;


    prim = work->prim_powder = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
											 N_PRIMS2, N_VERTS2 );
	if ( prim == NULL ) {
		return 0;
	}

    prim->raise = 0;

    tex = DG_GetTexture( POWDER_TEX );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


    pos     = SCR_POS;
    uvrgbwh = SCR_UVS;

    for ( i = 0 ; i < POWDER_NUM ; i++ ) {

	DG_COPY_VEC( pos, &DG_ZeroVector );
	
	switch ( irnd()%4 ) {
	case 0:
	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    break;
	case 1:
	    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    break;
	case 2:
	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    break;
	case 3:
	    uvrgbwh->u0 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->u1 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    break;
	}
	
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;	
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = 128;
	uvrgbwh->g  = 128;
	uvrgbwh->b  = 128;
	uvrgbwh->a  = 99 + irnd()%20;
	
	uvrgbwh->w = uvrgbwh->h = 100.0f;
	
	pos++;
	uvrgbwh++;
	
    }

    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          POWDER_NUM );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          POWDER_NUM );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), POWDER_NUM );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), POWDER_NUM );	

    return 1;

}




static int GetResources( Work *work )
{


    int 	i;
    POINT_PARAM	*p_param;


    work->id = 0;
    work->barning_num = 0;
    p_param = work->p_param;

    for ( i = 0 ; i < POINT_NUM ; i++ ) {
		p_param->pos = NULL;
		p_param->cnt = 0;
		p_param++;
    }

    if ( !( InitPacket( work ) ) ) {
		return -1;
	}
	if ( !( InitPacket2( work ) ) ) {
		return -1;
	}

    return 0;

}


/*---------- void *NewSolidusMantFire( void ) ----------
             ソリダスシュラウド炎呼び出し関数	     
------------------------------------------------------*/  
void *NewSolidusMantFire( void )
{
	Work	*work;

	if ( Work_SolMantFire == NULL ) {
	    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
		    GV_DestroyActor( work );
		    return NULL;
		}
	    }

	    Work_SolMantFire = work;
	    return (void *)work ;
	}
	else {
	    printf("error!-------------------fire still exist!!");
	    return NULL;
	}

}



