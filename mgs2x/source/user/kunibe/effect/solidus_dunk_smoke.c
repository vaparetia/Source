//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    landing_smoke.c
    着地煙
    2001/03/17 Yuuta Kunibe	
    $Id: solidus_dunk_smoke.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
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
#define ALPHA	(16)

#define N_PRIMS	(100)
#define	LIFE	(150)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define HALF_SEARCH_LENGTH	(3000.f)


extern void *NewDebris_Cm( FVECTOR *bound, FVECTOR *force,
					int num, int objcode,
					int color, float scale, int flags );


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vec[N_PRIMS];
    float	rot[N_PRIMS];
    float	rot_add[N_PRIMS];
    float	rad[N_PRIMS];

    float	limit_spd;
    float	add_size;

    int		cnt;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int		i;
    int	  	clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    int			alpha_cnt;



    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];
    vec 	= work->vec;


    alpha_cnt = work->cnt % 8;


    /*----------------*/
    /* スプライト更新 */
    /*----------------*/
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	if ( work->cnt > 3 ) {

	    if ( vec->vx * vec->vx + vec->vz * vec->vz > work->limit_spd ) {
		//vec->vx *= 0.25f;
		//vec->vz *= 0.25f;
		_sceVu0ScaleVector( vec, vec, 0.25f );

		work->rot[i] += work->rot_add[i];
		if ( work->rot[i] >= 1.0f ) {
		    work->rot[i] = work->rot[i] - 1.0f;
		}
		else if ( work->rot[i] < 0.0f ) {
		    work->rot[i] = work->rot[i] + 1.0f;
		}		
	    }
	    else {
		//vec->vx *= 0.98f;
		//vec->vz *= 0.98f;
		_sceVu0ScaleVector( vec, vec, 0.98f );

		work->rad[i] += work->add_size;
		work->rot_add[i] *= 0.95f;
		work->rot[i] += work->rot_add[i];
		if ( work->rot[i] >= 1.0f ) {
		    work->rot[i] = work->rot[i] - 1.0f;
		}
		else if ( work->rot[i] < 0.0f ) {
		    work->rot[i] = work->rot[i] + 1.0f;
		}

		uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
		uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );


		//if ( !( work->cnt % 8 ) ) {
		if ( i % 8 == alpha_cnt ) {
		    if ( uvrgbwh->a > 0 ) {
			uvrgbwh->a = uvrgbwh->a - 1;
		    }
		}
		else {
		    uvrgbwh->a = uvrgbwh_pre->a;
		}
	    }
	}

	/* スプライト更新 */
	uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
	uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );

	/*pos->vx = pos_pre->vx + vec->vx;
	pos->vz = pos_pre->vz + vec->vz;
	pos->vw = 1.0f;*/

	_sceVu0AddVector( pos, pos_pre, vec );
	
	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;

    }

    if ( ++work->cnt >= LIFE ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/*-------- GetFloorMatrix : 床マトリクス取得関数 ----------
     HZX_FLR *floor		: 参照する床
     FVECTOR *floor_point	: 床座標
     FMATRIX *floor_mat		: 床マトリクス (出力)
---------------------------------------------------------*/     
static void GetFloorMatrix( HZX_FLR *floor, FVECTOR *floor_point, FMATRIX *floor_mat )
{
	FVECTOR	to;
	SVECTOR	floor_rot;

	/* 法線取得 */
	to.vx = floor->p1.h;
	to.vy = floor->p3.h;
	to.vz = floor->p2.h;

	/* 回転角度取得 */
	OK_DirVecXY( &to, &DG_ZeroVector, &floor_rot );

	/* 床マトリクス生成 */
	DG_SetPos2( floor_point, &floor_rot );
	DG_GetPos( floor_mat );
}



/*-------- SearchFloor : 床検索関数 ----------
       FMATRIX *mat	: 床マトリクス (出力)
       FVECTOR *pos	: 床検索参照座標 (ソリダスの足座標)
--------------------------------------------*/  
static void SearchFloor( FMATRIX *mat, FVECTOR *pos )
{

    HZX_FLR		seg;
    u_int		atr;
    int			hazard_flag;

    FVECTOR		from;
    FVECTOR		to;
    FVECTOR		floor_point;


    /* 検索点から上にシフト */
    DG_COPY_VEC( &from, pos );
    from.vy += HALF_SEARCH_LENGTH;
    DG_COPY_VEC( &to, pos );
    to.vy -= HALF_SEARCH_LENGTH;

    /* ハザードチェック */
    hazard_flag = HZX_OnlineHazardCheck( 
			    GM_GetHzxGroupID( GM_CurrentStageMap ),
			    &from,
			    &to,
			    HZX_CHK_FIX,
			    HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
			    HZX_FLOOR_NO_PLAYER );

    if ( hazard_flag == 2 ) {	/* 床検索成功 */

	/* 床情報取得 */
	HZX_GetOnlineHazard( &seg, &atr ) ;
	/* 床座標取得 */
	HZX_GetOnlinePoint( &floor_point );

	/* 床マトリクス生成 */
	GetFloorMatrix( &seg, &floor_point, mat );

    }
    else {			/* 床検索失敗 */
	printf("solidas_dash_fire.c L168 : fail_search_floor!\n");
    }

}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR* center, float radius, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float			base_spd;
	float			base_size;
	float 			ftmp;
	float			fcos;
	float			fsin;
	float			angle;

	FMATRIX			floor_mat;

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	base_spd  = radius / 4.0f;
	base_size = radius / 2.0f;

	work->limit_spd = base_spd / 5.f;
	work->limit_spd = work->limit_spd * work->limit_spd;
	work->add_size = base_size / 40.f;


	SearchFloor( &floor_mat, center );
	DG_SetPos( &floor_mat );
	
	
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( pos, (FVECTOR *)floor_mat.m[3] );
	    pos->vy += 300.f;

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = ( 0.8f + frnd()*0.2f );
	    uvrgbwh->r  = (u_short)( (float)COL_R * ftmp );
	    uvrgbwh->g  = (u_short)( (float)COL_G * ftmp );
	    uvrgbwh->b  = (u_short)( (float)COL_B * ftmp );
	    uvrgbwh->a  = (u_short)( (float)ALPHA * ( 0.8f + frnd()*0.2f ) );

	    work->rot[i] = frnd();
	    work->rad[i] = base_size;
	    uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
	    uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );

	    ftmp = base_spd * frnd();//( 0.80f + frnd() * 0.40f );
	    angle = TPI * frnd();
	    fcos = vu0_Cos( angle );
	    fsin = vu0_Sin( angle );
	    vec->vx = ftmp * fcos;
	    //vec->vy = 0.f;
	    //vec->vz = ftmp * fsin;
	    vec->vy = ftmp * fsin;
	    vec->vz = 0.f;
	    DG_RotVector( vec, vec, 1 );

	    work->rot_add[i] =-0.01f * fcos;
	    
	    pos++;
	    uvrgbwh++;
	    vec++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, FVECTOR* center, float radius )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    /* 柴田破片用変数 */
    FVECTOR 	bound[2];
    FVECTOR 	force;
    int		num;
    int 	objcode;
    int 	color;
    float	scale;
    int 	flags;


    work->cnt = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 10, 10 );
	if ( prim == NULL ) {
		return -1;
	}

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, center, radius, prim, tex );    


    /* 柴田破片コモデル呼び出し ->> 後に軽い破片を作成 */	
    DG_COPY_VEC( &bound[0], center );
    bound[0].vx -= 2000.f;
    bound[0].vy -= 2500.f;
    bound[0].vz -= 2000.f;
    
    DG_COPY_VEC( &bound[1], center );
    bound[1].vx += 2000.f;
    bound[1].vy += 2500.f;
    bound[1].vz += 2000.f;
    
    force.vx = 0.f;
    force.vy = 1.f;
    force.vz = 0.f;
    force.vw = 100.f;
    
    num = 50;
	
    objcode = GV_StrCode("usp_emb");
	
    color = ( (128<<24)&(128<<16)&(128<<8) );

    scale = 2.0f;

    flags = 0;
	
    NewDebris_Cm( bound, &force, num, objcode, color, scale, flags );


    return 0;

}


void *NewSolidusDunkSmoke( FVECTOR* center, float radius )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, center, radius ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


