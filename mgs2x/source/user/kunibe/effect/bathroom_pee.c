//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    bathroom_pee.c
    トイレ(w16a)放尿エフェクト
    2001/05/30 Yuuta Kunibe	
    $Id: bathroom_pee.c,v 1.1.1.3 2002/11/19 11:44:32 Yoshizawa1 Exp $
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


#define COL_R	(128)
#define COL_G	(128)
#define COL_B	(128)
#define ALPHA	(2)

#define SIZE	(20.0f)

#define N_PRIMS	(592)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#if 0
#define INIT_PX	(13708.0f)
#define INIT_PY	( 3500.0f)
#define INIT_PZ	(-4400.0f)
#else
#define INIT_PX	( 2000.0f)
#define INIT_PY	( 2000.0f)
#define INIT_PZ	(-157000.0f)
#endif


#define	BATH_WALL	(-87160.0f)
#define BATH_HEIGHT	(500.0f)
#define	BATH_EDGE	(-86960.0f)
#define	FLOOR_HEIGHT	(0.0f)

#define	N_NEW		(16)//(32)//(16)

#define INIT_VZ (20.0f)//(28.f)//(20.0f)
#define GRAVITY	(1.0f)//(1.5f)//(1.0f)

#define HOLD	(0x00)
#define SPLASH	(0x01)
#define HOMING	(0x02)
#define	FINISH	(0x04)


extern void *NewPeeShower( OBJECT *body, FVECTOR *target_pos, int *flag );


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;
    DG_PRIM2	*prim_light;

    OBJECT	*body;
    FVECTOR	pre_pos;

    FVECTOR	vec[N_PRIMS];
    int		ref_flag[N_PRIMS];
    int 	id;
    int 	pre_id;

    float	init_speed;
    SVECTOR	rot;
    FVECTOR	vec_pre;

    FVECTOR	vecbody;
    
    int 	step;

    int		cnt;
    int 	off_flag;

    int		*flag;
    int		pre_flag;

} Work;



/* アクト関数 */
static void Act( Work *work )
{

    int			i,j;
    int	  		clock;
    int 		flg;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    FMATRIX		mat;
    FVECTOR		fvtmp;
    float		random;

    FVECTOR  		*pos_light;
    DG_PRIM2_UVRGBWH 	*uvrgb_light;
    int 		light_id;

    int			alpha_cnt;
    int 		cnt;

    FVECTOR		vecbody_now;
    
    GM_GroupPrim2( work->prim, work->body->map_name );
    GM_GroupPrim2( work->prim_light, work->body->map_name );

   /* 操作する頂点バッファ取得 */
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
   DG_SwitchBuffPrim2( work->prim_light );
   clock = work->prim->buffer_clock;

    
    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];
    
    vec 	= work->vec;
    
    pos_light   = work->prim_light->pos[work->prim_light->buffer_clock];
    uvrgb_light = work->prim_light->uvrgb[work->prim_light->buffer_clock];

    flg = 0;
    alpha_cnt = 0;
    light_id = irnd()%N_PRIMS;



    if ( ( work->pre_flag == HOLD ) && ( *work->flag & SPLASH ) ) {
	work->init_speed = 30.f;
	work->off_flag = 0;
	work->step = 0;
    }


    work->pre_flag = *work->flag;




    /* 初期スピード操作 */
    switch ( *work->flag ) {
    case HOLD:
	work->init_speed = 0.0f;
	break;
    case SPLASH:
    case HOMING:
	if ( work->init_speed > INIT_VZ ) {
	    work->init_speed *= 0.995f;
	}
	break;
    case FINISH:
	switch ( work->step ) {
	case 0:
	    work->init_speed *= 0.996f;
	    if ( work->init_speed <= 8.0f ) {
		work->init_speed = INIT_VZ * 0.80f;
		work->step = 1;
	    }
	    break;
	case 1:
	    work->init_speed *= 0.996f;
	    if ( work->init_speed <= 9.0f ) {
		work->init_speed = 0.0f;
		work->step = 2;
	    }
	    break;
	case 2:
	    break;
	}
    }



    j = work->cnt % 4;
	
    /* 通常処理 */    
    if ( work->off_flag == 0  ) {


	DG_VisiblePrim2( work->prim );
	DG_VisiblePrim2( work->prim_light );
    
	/*----------------*/
	/* スプライト更新 */
	/*----------------*/
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    /* 重力による加速 */
	    vec->vy -= GRAVITY;

	    /* スプライト位置更新 */
	    _sceVu0AddVector( pos, pos_pre, vec );

	    vec->vx *= 0.99f;
	    vec->vz *= 0.99f;

	    uvrgbwh->a = uvrgbwh_pre->a;
	    if ( uvrgbwh->a == 0 ) alpha_cnt++;

	    if ( j == 0 ) {
		uvrgbwh->w = uvrgbwh_pre->w + 1;
		uvrgbwh->h = uvrgbwh_pre->h + 1;
	    }
	    else {
		uvrgbwh->w = uvrgbwh_pre->w;
		uvrgbwh->h = uvrgbwh_pre->h;
	    }

	    /* トイレとの当たり */
	    if ( work->ref_flag[i] == 0 && uvrgbwh->a > 0 ) {
		if ( pos->vz <= BATH_WALL ) {		/* 奥の壁 */
		    vec->vx = frnd()*20.f;
		    vec->vy = frnd()*10.0f;
		    vec->vz = 10.0f + frnd()*10.0f;

		    if ( i%4==0 ) {
			uvrgbwh->w = 2;
			uvrgbwh->h = 2;
			uvrgbwh->a = 16;
		    }

		    work->ref_flag[i] = 1;
		}
		else if ( pos->vy <= BATH_HEIGHT ) {	/* 底 */
		    vec->vx = frnd()*20.f;
		    vec->vy = 20.f + frnd()*5.0f;
		    vec->vz =-( 8.0f+frnd()*12.0f );

		    if ( i%4==0 ) {
			uvrgbwh->w = 2;
			uvrgbwh->h = 2;
			uvrgbwh->a = 16;
		    }

		    work->ref_flag[i] = 1;
		}
		    
	    }
	    
	
	    if ( i == light_id ) {
		if ( uvrgbwh->a > 0 ) {
		    DG_COPY_VEC( pos_light, pos );
		    uvrgb_light->a = 64;
		}
		else {
		    uvrgb_light->a = 0;
		}
	    }	    

	    /* ポインタ更新 */
	    pos++;
	    pos_pre++;
	    uvrgbwh++;
	    uvrgbwh_pre++;
	    vec++;

	}





	/*------------------------------------------*/
	/* idで指定されたスプライトを初期化し再利用 */
	/*------------------------------------------*/
	pos = work->prim->pos[clock];
	uvrgbwh = work->prim->uvrgb[clock];

	work->rot.vx = irnd()%40 - 20;
	work->rot.vy = irnd()%40 - 20;
	work->rot.vz = 0;
	
	DG_SetPos( &work->body->objs->objs[0].world );
	DG_RotatePos( &work->rot );
	DG_GetPos( &mat );

	/* 現在の本体速度ベクトル取得 */
	_sceVu0SubVector( &vecbody_now, (FVECTOR *)work->body->objs->objs[0].world.m[3], &work->pre_pos );
	
	cnt = 0;
    
	/* パーティクル初期化 */
	for ( i = 0 ; i < N_NEW ; i++ ) {

	    /* 初期位置 */
	    DG_COPY_VEC( &pos[work->id], &DG_ZeroVector );

	    pos[work->id].vx = frnd()*5.f;
	    pos[work->id].vy = frnd()*5.f;
	    pos[work->id].vz = frnd()*5.f;
	    DG_COPY_VEC( &fvtmp, (FVECTOR *)(work->body->objs->objs[0].world.m[3]) );
	    fvtmp.vy -= 120.f;
	    _sceVu0ScaleVector( &fvtmp, &fvtmp, (float)( N_NEW - i )/(float)N_NEW );
	    _sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	    _sceVu0ScaleVector( &fvtmp, &pos[work->pre_id], (float)( i )/ (float)N_NEW );
	    _sceVu0AddVector( &pos[work->id], &pos[work->id], &fvtmp );
	

	    /* 初速度 */
	    DG_COPY_VEC( &work->vec[work->id], &DG_ZeroVector );

	    if ( !( irnd()%3 ) ) {
		random = ( 1.0f * frnd() - 0.5f ) * 0.5f;/**/
	    }
	    else {
		random = 0.f;
	    }
	    _sceVu0ScaleVector( &fvtmp, (FVECTOR *)(mat.m[0]), random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	
	    if ( !( irnd()%3 ) ) {
		random = ( 1.0f * frnd() - 0.5f ) - 1.0f * (float)i / (float)N_NEW;
	    }
	    else {
		random = - 1.0f * (float)i / (float)N_NEW;
	    }
	    DG_COPY_VEC( &fvtmp, &DG_ZeroVector );
	    fvtmp.vy = 1.0f;
	    _sceVu0ScaleVector( &fvtmp, &fvtmp, random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	    random = (float)i / (float)N_NEW;
	    _sceVu0ScaleVector( &fvtmp, (FVECTOR*)(&mat.m[2]), work->init_speed * ( 1.0f - random ) );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );
	    _sceVu0ScaleVector( &fvtmp, &work->vec_pre, work->init_speed * random );
	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &fvtmp );

	    /* 本体ベクトル更新 */
	    work->vecbody.vx = work->vecbody.vx * 0.2f + vecbody_now.vx * 0.8f;
	    work->vecbody.vy = work->vecbody.vy * 0.2f + vecbody_now.vy * 0.8f;
	    work->vecbody.vz = work->vecbody.vz * 0.2f + vecbody_now.vz * 0.8f;

	    _sceVu0AddVector( &work->vec[work->id], &work->vec[work->id], &work->vecbody );
	    
	    /* アルファ初期化 */	
	    if ( work->init_speed == 0.0f ) {
		uvrgbwh[work->id].a = 0;
	    }
	    else if ( work->init_speed < 13.f ) {
		if ( !( irnd()%( (int)( 15.f - work->init_speed ) ) ) ) {
		    uvrgbwh[work->id].a = ALPHA;
		    cnt++;
		}
		else {
		    uvrgbwh[work->id].a = 0;
		}	    		
	    }
	    else {
		uvrgbwh[work->id].a = ALPHA;
		cnt++;
	    }	    
	    uvrgbwh[work->id].w  = SIZE;
	    uvrgbwh[work->id].h  = SIZE;

	    work->ref_flag[work->id] = 0;

	    /* ID更新 */
	    work->id++;

	}






	if ( work->id >= N_PRIMS ) work->id = 0;

	/* 速度ベクトル確保 */
	work->pre_id = work->id - N_NEW;
	if ( work->pre_id < 0 ) work->pre_id += N_PRIMS;
	DG_COPY_VEC( &work->vec_pre, (FVECTOR *)(mat.m[2]) );

	if ( *work->flag & FINISH && alpha_cnt >= N_PRIMS ) {
	    work->off_flag = 1;
	    *work->flag = HOLD;
	    printf("hold\n");
	}



    }
    else {
	DG_InvisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_light );
    }



    DG_COPY_VEC( &work->pre_pos, (FVECTOR *)work->body->objs->objs[0].world.m[3] );
    
    work->cnt++;
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
    work->prim_light = OK_FreePrim2( work->prim_light );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	/* 本体パーティクル初期化 */
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = prim->pos[0];
	uvrgbwh   = prim->uvrgb[0];
	vec	  = work->vec;

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = 0;//ALPHA;
	    
	    uvrgbwh->w  = SIZE;
	    uvrgbwh->h  = SIZE;

	    work->vec[i].vx = 0.f; 
	    work->vec[i].vy = 0.f; 
	    work->vec[i].vz = 0.f;

	    work->ref_flag[i] = 0;

	    pos++;
	    uvrgbwh++;
	    vec++;
	}

	OK_Scr_Mem( prim->pos[ 1 ],   prim->pos[ 0 ], sizeof(FVECTOR),            N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], prim->uvrgb[ 0 ], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );


	/* キラキラ初期化 */
	work->prim_light->raise = 0;

	tex = DG_GetTexture( GV_StrCode( "light05_msk" ) );

	DG_ConfigPrim2Tex( work->prim_light, tex );
	DG_SetPrim2Alpha( work->prim_light, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	
	uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = 128;
	
	uvrgbwh->w  = 20.f;
	uvrgbwh->h  = 20.f;

	OK_Scr_Mem( work->prim_light->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( work->prim_light->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );

	work->prim_light->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, OBJECT *body, FVECTOR *target_pos, int *flag )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->body = body;
    work->flag = flag;
    
    work->id         = 0;
    work->pre_id     = 0;
    work->init_speed = 0.0f;
    work->step       = 0;
    work->cnt 	     = 0;
    work->off_flag   = 1;

    work->pre_flag = HOLD;

    work->rot.vx = 0;
    work->rot.vz = 0;
    work->rot.vy = 0;
    DG_COPY_VEC( &work->vec_pre, &DG_ZeroVector );

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS/16, 16 );
    work->prim_light = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


void *NewBathroomPee( OBJECT *body, FVECTOR *target_pos, int *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, body, target_pos, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


void *NewEnemyPee( OBJECT *body, FVECTOR *target_pos, int *flag, int kind ) {

    if ( kind == 0 ) {
	return ( NewBathroomPee( body, target_pos, flag ) );
    }
    else {
	return ( NewPeeShower( body, target_pos, flag ) );
    }

}
