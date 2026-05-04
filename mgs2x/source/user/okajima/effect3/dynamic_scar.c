//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dynamic_scar.c
	移動物に男根
	2001/06/13 S.Okajima
	$Id: dynamic_scar.c,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $
*/


#ifdef PSX2	///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/
#define	N_PRIMS			(1)
#define	N_POLYS			(1)
#define	N_VERTS			(N_POLYS*4)

#define	RAISE		(0)
#define	PYS_RAISE	(2.0f)

#define	SIZE		(20.0f)

#define	COL_R			(0)
#define	COL_G			(0)
#define	COL_B			(0)
#define	COL_A			(255)

#define	MAX_DYNAMIC_SCAR_NUM (64)

#define	MODEL_NAME ( 13925923 /*"cbx_a_frg1_cm"*/ )

typedef	struct	{
	GV_ACT_EX	actor;
	DG_PRIM2	*prim;
	int			my_num;
} Work ;

static int OK_DynamicScarNum;
static int OK_DynamicScarNum_next;
static int OK_DynamicScar_Flag;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim ;

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

	if( work->my_num == OK_DynamicScarNum_next ){
		GV_DestroyActor( work ) ;
	}

	OK_DynamicScar_Flag = 0;
}

static void Die( Work *work )
{
	work->prim  = OK_FreePrim2( work->prim  );
}


/*----------------------------------------------------------------*/
static int InitPacketSingle( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	uvrgb = prim->uvrgb[0] ;
	for ( i=0; i<N_PRIMS*N_POLYS; i++ ){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = COL_A;
		uvrgb++;
	}

	return 1;
}


/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world, FVECTOR *norm, FVECTOR *center )
{
	FVECTOR		pos[4];
	SVECTOR		rot;
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL;

	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 14845995 /*"dankon2_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SINGLEBUFF, 1, 4 );
	if(prim==NULL){
		return -1;
	}
	InitPacketSingle( work, prim, tex );

printf("%f %f %f\n",norm->vx,norm->vy,norm->vz);

	OK_DirVecXY( &DG_ZeroVector, norm, &rot );
	pos[0].vx = SIZE;
	pos[0].vy = SIZE;
	pos[0].vz = PYS_RAISE;
	pos[1].vx =-SIZE;
	pos[1].vy = SIZE;
	pos[1].vz = PYS_RAISE;
	pos[2].vx = SIZE;
	pos[2].vy =-SIZE;
	pos[2].vz = PYS_RAISE;
	pos[3].vx =-SIZE;
	pos[3].vy =-SIZE;
	pos[3].vz = PYS_RAISE;
	DG_SetPos2( center, &rot );
	DG_PutVector( pos, prim->pos[0], 4 );
	DG_VisiblePrim2( prim );
	prim->root = world;

	return 0 ;
}

//world:引っ付け先マトリクス（参照しつづける）
//norm:引っ付け先からの相対法線
//pos:引っ付け先からの相対位置
void *DynamicDanKon( FMATRIX *world, FVECTOR *norm, FVECTOR *pos )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_DynamicScar_Flag ) return NULL;
	OK_DynamicScar_Flag = 1;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, world, norm, pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		work->my_num = OK_DynamicScarNum;
		OK_DynamicScarNum++;
		if( OK_DynamicScarNum >= MAX_DYNAMIC_SCAR_NUM ) OK_DynamicScarNum = 0;
		OK_DynamicScarNum_next = OK_DynamicScarNum + 1;
		if( OK_DynamicScarNum_next >= MAX_DYNAMIC_SCAR_NUM ) OK_DynamicScarNum_next = 0;
	}
	return (void *)work ;
}

