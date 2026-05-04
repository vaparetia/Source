//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  rainbow.c
  虹
  2001/05/28 Yuuta Kunibe	
  $Id: rainbow.c,v 1.1.1.3 2002/11/19 11:44:46 Yoshizawa1 Exp $
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



#define N_PRIMS			(16)
#define N_VERTS 		(18)

#define ALPHA			(128)

#define SCR_POS 		SCRPAD_ADDR
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define LIFE			(200)



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	pos;

} Work;







/* アクト関数 */
static void Act( Work *work )
{

}


static void Die(Work *work )
{

    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{


    int			i,j;
    FVECTOR		*pos;	
    DG_PRIM2_UVRGB	*uvrgb;
    int			alpha;

    FMATRIX		mat;
    FVECTOR		vec;
    FVECTOR		vec_pre;
    SVECTOR		rot;

    FVECTOR		vectmp;



    //DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


    DG_COPY_MAT( &mat, &DG_UnitMatrix );

    rot.vx = 0;
    rot.vy = 0;
    rot.vz =-1024/N_PRIMS;

    _sceVu0ScaleVector( &vec, (FVECTOR *)mat.m[1], 8000.0f );    

        
    pos   	= SCR_POS;
    uvrgb	= SCR_UVS;

    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	DG_COPY_VEC( &vec_pre, &vec );
	DG_SetPos( &mat );
	DG_RotatePos( &rot );
	DG_GetPos( &mat );
	_sceVu0ScaleVector( &vec, (FVECTOR *)mat.m[1], 8000.0f );	

	for ( j = 0 ; j < N_VERTS ; j++ ) {

	    if ( j % 2 ) {
		_sceVu0ScaleVector( &vectmp, &vec, 0.8f + ( (float)( 9 - j/2 ) / 9.0f ) * 0.2f );
	    }
	    else {
		_sceVu0ScaleVector( &vectmp, &vec_pre, 0.8f + ( (float)( 9 - j/2 ) / 9.0f ) * 0.2f );
	    }
	    _sceVu0AddVector( pos, &work->pos, &vectmp );


	    
	    uvrgb->q = 4096;
	    if ( j/2 == 0 ) {
		uvrgb->f = 0x8fff;
	    }
	    else {		
		uvrgb->f = 0x0fff;
	    }


	    if ( i < 3 ) {
		alpha = i*2;
	    }
	    else if ( i > 15-3 ) {
		alpha = (15-i)*2;
	    }
	    else {
		alpha = 6;
	    }

	    uvrgb->a = alpha;


	    /* 虹色グラデーション */
	    switch ( j / 2 ) {
	    case 0:
		uvrgb->r = 0;
		uvrgb->g = 0;
		uvrgb->b = 0;
		break;
	    case 1:
		uvrgb->r = 255;
		uvrgb->g = 0;
		uvrgb->b = 0;
		break;
	    case 2:
		uvrgb->r = 192;
		uvrgb->g = 64;
		uvrgb->b = 0;
		break;
	    case 3:
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 0;
		break;
	    case 4:
		uvrgb->r = 0;
		uvrgb->g = 255;
		uvrgb->b = 0;
		break;
	    case 5:
		uvrgb->r = 0;
		uvrgb->g = 0;
		uvrgb->b = 255;
		break;
	    case 6:
		uvrgb->r = 64;
		uvrgb->g = 0;
		uvrgb->b = 192;
		break;
	    case 7:
		uvrgb->r = 128;
		uvrgb->g = 0;
		uvrgb->b = 128;
		break;
	    case 8:
		uvrgb->r = 0;
		uvrgb->g = 0;
		uvrgb->b = 0;
		break;
	    }		

	    pos++;
	    uvrgb++;
	    
	}

    }


    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );	

    prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );


    return 1;


}



static int GetResources( Work *work, FVECTOR *pos )
{
    
    DG_TEX 	*tex;
    DG_PRIM2	*prim;

    work->pos = *pos;
    work->pos.vz += 10000.0f;

    tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA |DG_PRIM2_SHADE, N_PRIMS, N_VERTS );

    InitPacket( work, prim, tex );
    
    return 0;

}



void *NewRainbow( FVECTOR *pos )
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



