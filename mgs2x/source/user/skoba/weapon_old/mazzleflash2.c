//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	単発銃のマズルフラッシュ 
	2000/1/11  H.Tanaka
	2000/10/18 S.Okajima
	$Id: mazzleflash2.c,v 1.1.1.3 2002/11/19 11:50:35 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "libutl.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"

/*
	補助マクロ
*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define   N_PRIMS	   1
#define   N_TEX_DU	  2
#define   N_TEX_DV	  2
#define   N_TEX_PATERN  4
#define   PRIM_TYPE	 (DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF)

typedef struct
{
	GV_ACT_EX	 actor;

	DG_PRIM2     *prim;
	int	    	 count;
	float       *angle;
} Work ;


static  void  Act(Work *work)
{
	if(work->count > 0)
	{
	GV_DestroyActor(work) ;
	return ;
	}

	DG_VisiblePrim2(work->prim);
// yano
//	DG_InvisiblePrim2Chanl( work->prim , 1 );

	work->count++;
}

static  void  Die(Work *pWork)
{
	if ( pWork->angle != NULL ){
		GV_DelayedFree( pWork->angle );
	}
	if ( pWork->prim != NULL ){
		pWork->prim = OK_FreePrim2( pWork->prim );
	}
}

static  int GetResources( Work *pWork, int n_verts, FMATRIX *world, FVECTOR *shift, float *size, u_char *bright , float space )
{
	DG_PRIM2		           *prim;
	FVECTOR		                *pos;
	DG_PRIM2_UVRGBWH        *uvrgbwh;
	int			              patern;
	float			 scale_u,scale_v;
	FVECTOR		                ftmp;
	FVECTOR                shift_tmp;
	DG_TEX			            *tex;
	int			                   i;

	prim = pWork->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,n_verts) ;
	if(prim == NULL){
		return ( -1 );
	}
	tex = DG_GetTexture( GV_StrCode("socom_f_msk") ) ;
	if(tex == NULL){
		return ( -1 );
	}
	pWork->angle = GV_Malloc( sizeof( float ) * n_verts );
	if ( pWork->angle == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( pWork->angle , sizeof( float ) * n_verts );
	DG_ConfigPrim2Tex(prim, tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0, 2, 0, 1, 0x00)) ;
	
	/* 位置決め */
	pos = prim->pos[0] ;
	uvrgbwh = prim->uvrgb[0] ;
	
	shift_tmp = *shift;
	for ( i = 0 ; i < n_verts ; i++ ){
		DG_SetPos( world );
		DG_PutVector(&shift_tmp , pos , 1);
		shift_tmp.vy += space;
		pos++;
	}
	patern = irnd() % 16 ;
	if(patern > 6) patern = 3 ;
	else if(patern > 4) patern = 2 ;
	else if(patern > 2) patern = 1 ;
	else
	patern = 0 ;

	scale_u = tex->u_scale / N_TEX_DU ;
	scale_v = tex->v_scale / N_TEX_DU ;
	ftmp.vx = (patern % N_TEX_DU) * scale_u + tex->u_offset ;
	ftmp.vy = (patern / N_TEX_DU) * scale_v + tex->v_offset ;
	ftmp.vz = ftmp.vx + scale_u ;
	ftmp.vw = ftmp.vy + scale_v ;
	for( i = 0; i < n_verts; i++ ){
		pWork->angle[ i ] = frnd() * 180.0f * PI / 360.0f;
		uvrgbwh->u0 = FTOI12(ftmp.vx) ;
		uvrgbwh->v0 = FTOI12(ftmp.vy) ;
		uvrgbwh->u1 = FTOI12(ftmp.vz) ;
		uvrgbwh->v1 = FTOI12(ftmp.vw) ;
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		/* WH値は整数 */
		uvrgbwh->w = (int)(size[i] * cosf( pWork->angle[ i ] ));
		uvrgbwh->h = (int)(size[i] * sinf( pWork->angle[ i ] ));
		uvrgbwh->r = bright[i] ;
		uvrgbwh->g = bright[i] ;
		uvrgbwh->b = bright[i] ;
		uvrgbwh->a = 128 ;
		uvrgbwh ++ ;
	}

	pWork->count = 0 ;
	DG_InvisiblePrim2(prim) ;

	return 0 ;
}

void *MazzleFlash2( int n_verts, FMATRIX *world, FVECTOR *shift, float *size, u_char *bright , float space )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work,  n_verts, world, shift,size,bright , space) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;

}
