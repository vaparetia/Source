//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_vernier_smoke.c
    バーニア舞い上がり煙
    2001/04/23 Yuuta Kunibe	
    $Id: harrier_vernier_smoke.c,v 1.1.1.3 2002/11/19 11:44:43 Yoshizawa1 Exp $
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


#define COL_R	(64)
#define COL_G	(64)
#define COL_B	(64)
#define ALPHA	(32)

#define N_PRIMS	(96)
#define	LIFE	(30)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	*center;
    
    FVECTOR	vec[N_PRIMS];
    float	rot[N_PRIMS];
    float	rot_add[N_PRIMS];
    float	rad[N_PRIMS];
    int		cnt[N_PRIMS];

    float	limit_spd;
    float	add_size;

    float		alpha_param;

    int 	life;
    char 	*flag; 

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;

    float 	    	ftmp;
    float	      	fcos;
    float	       	fsin;
    float	     	angle;

    int			num;


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

    num = 0;

    /* フラグでフェードインアウト */
    if ( !( *work->flag ) ) {
	if ( work->alpha_param > 0.0f ) {
	    work->alpha_param -= 0.01f;
	}
    }
    else {
	if ( work->alpha_param < 1.0f ) {
	    work->alpha_param += 0.02f;
	}
    }
    
    /*----------------*/
    /* スプライト更新 */
    /*----------------*/
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	/* スプライト更新 */
	uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
	uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );
	work->rad[i] += work->add_size;

	pos->vx = pos_pre->vx + vec->vx;
	pos->vy = pos_pre->vy + vec->vy;
	pos->vz = pos_pre->vz + vec->vz;


	if ( work->cnt[i] >= 3 ) {

	    if ( vec->vx * vec->vx + vec->vz * vec->vz > work->limit_spd ) {
		vec->vx *= 0.5f;
		vec->vz *= 0.5f;

		work->rot[i] += work->rot_add[i];
		if ( work->rot[i] >= 1.0f ) {
		    work->rot[i] = work->rot[i] - 1.0f;
		}
		else if ( work->rot[i] < 0.0f ) {
		    work->rot[i] = work->rot[i] + 1.0f;
		}

		uvrgbwh->a = uvrgbwh_pre->a;
		if ( uvrgbwh->a == 0 ) num++;

	    }
	    else {

		vec->vx *= 0.98f;
		vec->vz *= 0.98f;
		vec->vy += 15.f;

		work->rot_add[i] *= 0.95f;
		work->rot[i] += work->rot_add[i];
		if ( work->rot[i] >= 1.0f ) {
		    work->rot[i] = work->rot[i] - 1.0f;
		}
		else if ( work->rot[i] < 0.0f ) {
		    work->rot[i] = work->rot[i] + 1.0f;
		}

		if ( !( work->cnt[i] % 2 ) ) {

		    if ( uvrgbwh_pre->a > 0 ) {
			uvrgbwh->a = uvrgbwh_pre->a - 1;
		    }
		    else {

			/* 煙スプライト初期化 */
			DG_COPY_VEC( pos, work->center );
			pos->vy = -300.f;
			
			work->rot[i] = frnd();
			work->rad[i] = 1500.f;
			uvrgbwh->w   = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
			uvrgbwh->h   = work->rad[i] * vu0_Sin( work->rot[i]*TPI );
			uvrgbwh->a   = 0;
			
			ftmp  = 1500.f / 2.f * ( 1.0f + frnd() * 0.40f );
			angle = TPI * frnd();
			fcos  = vu0_Cos( angle );
			fsin  = vu0_Sin( angle );
			vec->vx = ftmp * fcos;
			vec->vy = 0.f;
			vec->vz = ftmp * fsin;
			
			work->rot_add[i] =-0.01f * fcos;
			
			work->cnt[i] = 0;

		    }

		}
		else {
		    uvrgbwh->a = uvrgbwh_pre->a;
		}

		if ( uvrgbwh->a == 0 ) num++;

	    }
	    
	}
	else if ( work->cnt[i] >= 0 ) {
	    uvrgbwh->a = work->cnt[i] * work->cnt[i] * 3 * work->alpha_param;
	    if ( uvrgbwh->a == 0 ) num++;
	}

	
	/* ステージ判定 */
	if ( pos->vz < -144000.f ) {
	    if ( pos->vx < -2000.f && vec->vx < 0.f ) {
		pos->vx = -2000.f;
		vec->vx = 0.0f;
		if ( work->cnt[i] < 3 ) {
		    work->cnt[i] = 3;
		}
	    }
	    if ( pos->vz > -145000.f && vec->vz > 0.f ) {
		if ( pos->vx > 800.f || pos->vx < -800.f ) {
		    pos->vz = -145000.f;
		    vec->vz = 0.0f;
		    if ( work->cnt[i] < 3 ) {
			work->cnt[i] = 3;
		    }
		}
	    }
	}
	else {
	    if ( pos->vx < -800.f && vec->vx < 0.f ) {
		pos->vx = -300.f;
		vec->vx = 0.0f;
		if ( work->cnt[i] < 3 ) {
		    work->cnt[i] = 3;
		}
	    }
	    else if ( pos->vx > 800.f && vec->vx > 0.f ) {
		pos->vx = 300.f;
		vec->vx = 0.0f;
		if ( work->cnt[i] < 3 ) {
		    work->cnt[i] = 3;
		}
	    }
	}
	    

	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;

	work->cnt[i]++;

    }


#if 0    
    printf("num %d : flag %d ; alpha_param %f\n",num, *work->flag, work->alpha_param);
#endif    

    if ( !(*work->flag) && num >= N_PRIMS ) {
#if 0	
	printf("Destroy Smoke\n");
#endif    
	GV_DestroyActor( work );
    }
	
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR* center, DG_PRIM2 *prim, DG_TEX *tex )
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

	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	base_spd  = 1500.f;
	base_size = 2000.f;//1500.f;

	work->limit_spd = base_spd / 5.f;
	work->limit_spd = work->limit_spd * work->limit_spd;
	work->add_size = base_size / 20.f;
	
	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( pos, work->center );
	    pos->vy = -300.f;

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = 1.0f + frnd()*0.2f;
	    uvrgbwh->r  = (u_short)( (float)COL_R * ftmp );
	    uvrgbwh->g  = (u_short)( (float)COL_G * ftmp );
	    uvrgbwh->b  = (u_short)( (float)COL_B * ftmp );
	    uvrgbwh->a  = 0;

	    work->rot[i] = frnd();
	    work->rad[i] = base_size;
	    uvrgbwh->w  = work->rad[i] * vu0_Cos( work->rot[i]*TPI );
	    uvrgbwh->h  = work->rad[i] * vu0_Sin( work->rot[i]*TPI );

	    ftmp = base_spd * ( 0.80f + frnd() * 0.40f );
	    angle = TPI * frnd();
	    fcos = vu0_Cos( angle );
	    fsin = vu0_Sin( angle );
	    vec->vx = ftmp * fcos;
	    vec->vy = 0.f;
	    vec->vz = ftmp * fsin;

	    work->rot_add[i] =-0.01f * fcos;

	    work->cnt[i] =-( 3 + irnd()%30 );	/* 初期のばらつき持たせるため */
	    
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


static int GetResources( Work *work, FVECTOR* center, char *flag )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->center = center;
    work->life = 0;
    work->flag = flag;
    work->alpha_param = 0.0f;

    //prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 6, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, center, prim, tex );    

    return 0;

}


/*
  呼びだし関数
  FVECTOR* center : 煙発生点
  float    radius : 拡散半径	
                      人   -> 800.f～1000.f 
                    メタル -> 10000.f 
 */
void *NewHarrierVernierSmoke( FVECTOR* center, char *flag )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, center, flag ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


