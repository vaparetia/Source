//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_flash.c
  ドッグタグのキラキラ
  2001/04/30 Yuuta Kunibe	
  $Id: harrier_flash.c,v 1.1.1.3 2002/11/19 11:44:41 Yoshizawa1 Exp $
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
#define ALPHA	(128)

#define N_PRIMS	(1)

#define	SIZE	(10000)

#define	SHIFT_Y (100.0f)
#define	SHIFT_Z (150.0f)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    DG_TEX	*tex;

    FVECTOR	*pos;
    int 	count;

} Work;


static void Act( Work *work )
{

    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;


    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos	    = work->prim->pos[clock];		
    uvrgbwh = work->prim->uvrgb[clock];

    /* 位置更新 */
    DG_COPY_VEC( pos, work->pos );

    switch ( work->count ) {
    case 0:
	uvrgbwh->u0 = FTOI12( 0.0F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * work->tex->v_scale + work->tex->v_offset );
	uvrgbwh->u1 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v1 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
	break;
    case 1:
	uvrgbwh->u0 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * work->tex->v_scale + work->tex->v_offset );
	uvrgbwh->u1 = FTOI12( 1.0F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v1 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
	break;
    case 2:
	uvrgbwh->u0 = FTOI12( 0.0F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.5F * work->tex->v_scale + work->tex->v_offset );
	uvrgbwh->u1 = FTOI12( 0.5F * work->tex->u_scale + work->tex->u_offset );
	uvrgbwh->v1 = FTOI12( 1.0F * work->tex->v_scale + work->tex->v_offset );
	break;
    }
    
    if (++work->count >= 3 ) {
	GV_DestroyActor( work );
    }
    
}


static void Die(Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
    
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	DG_COPY_VEC( pos, &DG_ZeroVector );

	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	uvrgbwh->q0 = 4096;
	uvrgbwh->f0 = 0x0fff;
	
	uvrgbwh->u1 = FTOI12( 0.5F * tex->u_scale + tex->u_offset );
	uvrgbwh->v1 = FTOI12( 0.5F * tex->v_scale + tex->v_offset );
	uvrgbwh->q1 = 4096;
	uvrgbwh->f1 = 0x0fff;
	
	uvrgbwh->r  = COL_R;
	uvrgbwh->g  = COL_G;
	uvrgbwh->b  = COL_B;
	uvrgbwh->a  = ALPHA;
	
	uvrgbwh->w  = SIZE;
	uvrgbwh->h  = SIZE;
	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *pos )
{
    
    DG_PRIM2		*prim;

    work->pos    = pos;
    work->count  = 0;    
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );

    work->tex = DG_GetTexture( GV_StrCode( "kirari_alp_ovl" ) );
    InitPacket( work, prim, work->tex );    
    
    return 0;
}


/*
  void *NewDogtagFlash( OBJECT *object ) : 呼び出し関数
  
*/
void *NewHarrierFlash( FVECTOR *pos )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





