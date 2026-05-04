//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  line_test.c
  ライン表示テスト
  2001/03/13 Yuuta Kunibe
	
  $Id: line_test.c,v 1.1.1.3 2002/11/19 11:44:55 Yoshizawa1 Exp $
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

#define COL_R	128
#define COL_G	128
#define COL_B	128
#define ALPHA	64

#define N_PRIMS	1
#define N_VERTS 2

#define SCR_POS SCRPAD_ADDR
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	60


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int		map;

    DG_PRIM2	*prim;
    FVECTOR	center;

    int life;

} Work;


/*	
	スプライトは２頂点の指定ではなく
	スプライトの中心点と幅,高さ及び
	特別に DG_PRIM2_UVRGBWH で２頂点のパラメータを指定するところに注意！
*/
static void Act( Work *work )
{

    int	  	clock;
    FVECTOR  	*pos;

    /* スプライト中心座標取得 */
    work->center = GM_PlayerPosition;

    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos = work->prim->pos[clock];
	    
    pos->vx  = work->center.vx;
    pos->vy  = work->center.vy;
    pos->vz  = work->center.vz;
    pos->vw  = 1.0f;

    pos++;
    
    DG_COPY_VEC( pos, &DG_ZeroVector );

    if( --work->life < 0 ){
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
	int		i;
	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;

	prim->raise = 0;

	//DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;

	for ( i = 0 ; i < N_VERTS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );
	    pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r  = COL_R;
	    uvrgb->g  = COL_G;
	    uvrgb->b  = COL_B;
	    uvrgb->a  = ALPHA;

	    pos++;
	    uvrgb++ ;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->life = GCL_GetOptionValue( 'l', -1 );
    work->center = GM_PlayerPosition;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA, 
				      N_PRIMS, 2 );

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
    InitPacket( work, prim, tex );
    
    if( work->life < 0 ) return -1;
    return 0;
}


void *NewLineTest( int name, int map )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->map = map;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





