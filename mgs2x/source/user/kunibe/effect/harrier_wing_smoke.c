//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_wing_smoke.c
  ハリアー旋回筋雲
  2001/04/03 Yuuta Kunibe
	
  $Id: harrier_wing_smoke.c,v 1.1.1.3 2002/11/19 11:44:43 Yoshizawa1 Exp $
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


#define N_PRIMS		(6)
#define N_VERTS 	(64)
#define CENTER_NUM 	(N_PRIMS * N_VERTS/2)

#define COLOR		(128)
#define ALPHA		(48.0f)//(96.0f)

#define MIN_WIDTH	(250.f)

#define	SMOKE_TEX	( GV_StrCode( "smoke_lp1_alp" ) )

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define LIFE		(200)



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR*	pos;
    FVECTOR	center[CENTER_NUM];
    
    float	*p_width;
    float	width;
    
    int 	id;
    int		*flg;

    int		pre_zoom_flag;
    
    float	param;

} Work;




static void Act( Work *work )
{

    int 		i,j,n;
    int	  		clock;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;
    FVECTOR		vectmp;
    int 		alpha;

    float		width;


    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    clock = work->prim->buffer_clock;
    pos	      = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];


    /* 筋雲幅更新 */
    if ( GM_CheckGameStatus( STATE_DEMO ) ) {
	width = 50.0f;
    }
    else {
	work->width = work->width * 0.9f + (*work->p_width) * 0.1f;
	width = work->width * 2.0f / DG_Chanls[0].screen;

	if ( width < MIN_WIDTH ) {
	    width = MIN_WIDTH;
	}
    }


    
    /* フェード処理 */
    if ( *work->flg ) {
	work->param += 0.02f;
	if ( work->param > 1.0f ) {
	    work->param = 1.0f;
	}
    }
    else {
	work->param -= 0.02f;
	if ( work->param < 0.0f ) {
	    work->param = 0.0f;
	}
    }


    /* ポリゴン繋ぎ処理 */
    if ( work->id == 0 ) {
	DG_COPY_VEC( &work->center[work->id], &work->center[CENTER_NUM-1] );
	work->id++;
	DG_COPY_VEC( &work->center[work->id], work->pos );
    }
    else if ( !(work->id%32) ) {
	DG_COPY_VEC( &work->center[work->id], &work->center[work->id-1] );
	work->id++;
	DG_COPY_VEC( &work->center[work->id], work->pos );
    }
    else {
	DG_COPY_VEC( &work->center[work->id], work->pos );
    }


#if 0    
    printf("id : %d\n",work->id);
#endif    


    if ( work->param > 0.0f ) {			

	for ( i = 0 ; i < N_PRIMS ; i++ ) {
	    
	    for ( j = 0 ; j < N_VERTS/2 ; j++ ) {

		n = i*32+j;
	    
		if ( n == 0 ) {

		    _sceVu0SubVector( &vectmp, &work->center[0], &work->center[CENTER_NUM-2] );
		    _sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)DG_Chanls[0].eye.m[2] );
		    _sceVu0Normalize( &vectmp, &vectmp );
		    _sceVu0ScaleVector( &vectmp, &vectmp, width );
		
		    _sceVu0AddVector( pos, &work->center[0], &vectmp );
#if 0    
		    printf("%f,%f,%f\n",pos->vx,pos->vy,pos->vz);
#endif		
		    pos++;
		    _sceVu0SubVector( pos, &work->center[0], &vectmp );
#if 0    
		    printf("%f,%f,%f\n",pos->vx,pos->vy,pos->vz);
#endif		
		    pos++;
		    
		}
		else if ( j == 0 ) {

		    DG_COPY_VEC( pos, (pos-2) );
		    pos++;
		    DG_COPY_VEC( pos, (pos-2) );
		    pos++;

		}
		else {

		    _sceVu0SubVector( &vectmp, &work->center[n], &work->center[n-1] );
		    _sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)DG_Chanls[0].eye.m[2] );
		    if ( n == work->id + 1 ) {
			_sceVu0SubVector( &vectmp, &DG_ZeroVector, &vectmp );
		    }
		    _sceVu0Normalize( &vectmp, &vectmp );
		    _sceVu0ScaleVector( &vectmp, &vectmp, width );

		    _sceVu0AddVector( pos, &work->center[n], &vectmp );
		    pos++;
		    _sceVu0SubVector( pos, &work->center[n], &vectmp );
		    pos++;

		}

		alpha = work->id - n;
		if ( alpha < 0 ) alpha += CENTER_NUM;
		if ( alpha == 0 ) {
		    alpha = 0;
		}
		else {
		alpha = CENTER_NUM - alpha;
		alpha = (int)( (float)alpha / (float)CENTER_NUM * ALPHA * work->param );
		}
	    
		if ( work->id == CENTER_NUM-2 ) {
		    if ( n == CENTER_NUM-1 || n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else if ( work->id == CENTER_NUM-1 ) {
		    if ( n == 0 || n == 1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else if ( work->id%32 == 31 ) {
		    if ( n == work->id+1 || n == work->id+2 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		else {		    		    
		    if ( n == work->id+1 ) {
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x8fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		    else {
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
			uvrgb->f = 0x0fff;
			uvrgb->a = alpha;
			uvrgb++;
		    }
		}
		
	    }

	}

    }

    if ( ++work->id >= CENTER_NUM ) {
	work->id = 0;
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	int		i,j;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS; i++ ) {

	    pos   = SCR_POS;
	    uvrgb = SCR_UVS;

	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		DG_COPY_VEC( pos, work->pos );

		pos++;

		if ( j%2 ) {
		    uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		}
		else {
		    uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		}

		uvrgb->v = FTOI12( 1.0f / (float)CENTER_NUM * (float)(i*32+j/2) * tex->v_scale + tex->v_offset );

		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->b = COLOR;
		uvrgb->r = COLOR;
		uvrgb->g = COLOR;
		uvrgb->a = 0;

		uvrgb++;

		if ( j % 2 ) DG_COPY_VEC( &work->center[i*32+j/2], work->pos );

	    }

	    OK_Scr_Mem( &prim->pos[ 0 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	    OK_Scr_Mem( &prim->pos[ 1 ][i*N_VERTS], SCR_POS, sizeof(FVECTOR), N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim->uvrgb[ 0 ]))[i*N_VERTS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGB), N_VERTS );
	    OK_Scr_Mem( &((DG_PRIM2_UVRGB*)(prim->uvrgb[ 1 ]))[i*N_VERTS], SCR_UVS,
			sizeof(DG_PRIM2_UVRGB), N_VERTS );

	    
	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *pos, float *p_width, int *flg )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    
    work->pos = pos;
    work->flg = flg;
    work->p_width = p_width;

    work->width = MIN_WIDTH;
    work->param = 0.f;
    work->pre_zoom_flag = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX | DG_PRIM2_SHADE,
				      N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( SMOKE_TEX );
    InitPacket( work, prim, tex );
    
    return 0;

}



/*-------- void *NewHarrierWingSmoke( FVECTOR *pos, float *p_width, int *flg ) ------------
                          ハリアー筋雲エフェクト呼び出し関数
     FVECTOR *pos     : 筋雲発生点
     float   *p_width : 筋雲幅
     int     *flg     : 筋雲発生フラグ
-----------------------------------------------------------------------------------------*/
void *NewHarrierWingSmoke( FVECTOR *pos, float *p_width, int *flg )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, p_width, flg ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



