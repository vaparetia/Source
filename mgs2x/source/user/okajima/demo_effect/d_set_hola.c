//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	set_t_fc.c
	カメラに平行にテクスチャセット（クロスフェード）
	1999/11/09 S.Okajima
	$Id: d_set_hola.c,v 1.1.1.3 2002/11/19 11:46:54 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS0		(SCRPAD_ADDR + 0x2000)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			life;
	int			life_max;
	float		size;
	FVECTOR		center;
	FVECTOR		col;
	DG_PRIM2	*prim ;
} Work ;
/*----------------------------------------------------------------*/
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, FVECTOR *col )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	float		angle ;

	angle = TPI*rnd();

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	DG_COPY_VEC( &work->col, col );
	DG_COPY_VEC( &work->center, center );

	uvrgb = SCR_UVS0 ;

	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x8fff ;
	uvrgb->r = (int)col->vx ;
	uvrgb->g = (int)col->vy ;
	uvrgb->b = (int)col->vz ;
	uvrgb->a = (int)col->vw ;
	uvrgb++;

	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x8fff ;
	uvrgb->r = (int)col->vx ;
	uvrgb->g = (int)col->vy ;
	uvrgb->b = (int)col->vz ;
	uvrgb->a = (int)col->vw ;
	uvrgb++;

	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x0fff ;
	uvrgb->r = (int)col->vx ;
	uvrgb->g = (int)col->vy ;
	uvrgb->b = (int)col->vz ;
	uvrgb->a = (int)col->vw ;
	uvrgb++;

	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096 ;
	uvrgb->f = 0x0fff ;
	uvrgb->r = (int)col->vx ;
	uvrgb->g = (int)col->vy ;
	uvrgb->b = (int)col->vz ;
	uvrgb->a = (int)col->vw ;

	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), 4 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), 4 );

	return 1;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		clock;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR	fvtemp[4];
	FMATRIX	mat;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	fvtemp[0].vx = work->size;
	fvtemp[0].vy = work->size;
	fvtemp[0].vz = 0;
	fvtemp[1].vx =-work->size;
	fvtemp[1].vy = work->size;
	fvtemp[1].vz = 0;
	fvtemp[2].vx = work->size;
	fvtemp[2].vy =-work->size;
	fvtemp[2].vz = 0;
	fvtemp[3].vx =-work->size;
	fvtemp[3].vy =-work->size;
	fvtemp[3].vz = 0;

	DG_COPY_MAT( &mat, &DG_Chanls->eye );
	DG_COPY_VEC( (FVECTOR *)mat.m[3], &work->center );
	DG_SetPos( &mat );
	DG_PutVector( fvtemp, (FVECTOR *)work->prim->pos[ clock ], 4 );

	uvrgb = work->prim->uvrgb[ clock ];
	uvrgb->a = (int)(work->col.vw) * work->life / work->life_max ;

	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int GetResources( Work *work, FVECTOR *pos, FVECTOR *col, float size, int life )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->life = life;
	if( work->life < 16 ) work->life = 16;

	work->life_max = life;
	work->size = size;

	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
	if(prim==NULL) return -1;
	InitPacket( work, prim, tex, pos, col );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewSetHola_Demo( FVECTOR *pos, FVECTOR *col, float size, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, col, size, life  ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
