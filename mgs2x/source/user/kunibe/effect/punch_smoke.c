//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    punch_smoke.c
    パンチキック時の煙
    2001/05/17 Yuuta Kunibe	
    $Id: punch_smoke.c,v 1.1.1.3 2002/11/19 11:44:46 Yoshizawa1 Exp $
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


#define N_PRIMS	(1)
#define N_VERTS	(32)

#define SIZE	(200)

#define COL_R	(64)
#define COL_G	(64)
#define COL_B	(64)
#define ALPHA	(5)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FMATRIX	mat;
    
    FVECTOR	vec[N_PRIMS*N_VERTS];
    /*float	rot[N_PRIMS*N_VERTS];
    float	rot_add[N_PRIMS*N_VERTS];
    float	rad[N_PRIMS*N_VERTS];*/

    float	limit_spd;
    float	add_size;

    int		cnt;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int		i;
    int	  	clock;
    int		invisible_count;
    
    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;


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

    invisible_count = 0;
    
    /*----------------*/
    /* スプライト更新 */
    /*----------------*/
    for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ) {

	if ( uvrgbwh_pre->a > 0 ) {
	    _sceVu0AddVector( pos, pos_pre, vec );

	    if ( work->cnt > 1 ) {
		_sceVu0ScaleVector( vec, vec, 0.75f );
	    }

	    if ( work->cnt > 3 && !(work->cnt % 2) ) {
		uvrgbwh->a = uvrgbwh_pre->a - 1;
	    }
	    else {
		uvrgbwh->a = uvrgbwh_pre->a;
	    }
	}
	else {
	    uvrgbwh->a = 0;
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	    invisible_count++;
	}

	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;

    }

    work->cnt++;

    if ( invisible_count >= N_PRIMS*N_VERTS ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float 			ftmp;
	SVECTOR			rot;
	FMATRIX			rot_mat;
	FVECTOR			vectmp;


	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;

	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ) {

	    rot.vz = irnd()%4096;
	    DG_SetPos( &work->mat );
	    DG_RotatePos( &rot );
	    DG_GetPos( &rot_mat );
	    ftmp = frnd() * 0.5f + 0.5f;
	    _sceVu0ScaleVector( vec, (FVECTOR *)rot_mat.m[0], 30.f + ftmp * 6.f );
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)rot_mat.m[2], -40.0f * ( 1.0f - ftmp ) );
	    _sceVu0AddVector( vec, vec, &vectmp );

	    _sceVu0AddVector( pos, (FVECTOR *)work->mat.m[3], vec );

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
	    uvrgbwh->a  = ALPHA;

	    uvrgbwh->w  = SIZE;
	    uvrgbwh->h  = SIZE;
	    
	    pos++;
	    uvrgbwh++;
	    vec++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}


static int GetResources( Work *work, FMATRIX *mat )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->mat = *mat;
    work->cnt = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}


/*-------- NewPunchSmoke : スプライトプラズマ呼び出し関数 --------
     FVECTOR	*from	: 始点
     FVECTOR	*middle : 中継点
     FVECTOR	*to	: 終点
     float	width	: スプライト幅
     FVECTOR	*color	: 色
------------------------------------------------------------------*/  
void *NewPunchSmoke( FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, mat ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


