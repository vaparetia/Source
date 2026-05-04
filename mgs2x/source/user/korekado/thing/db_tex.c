//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	db_tex.c
	デバッグ用テクスチャ表示

	2000/08/10 Y.Korekado
	$Id: db_tex.c,v 1.1.1.3 2002/11/19 11:44:26 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewDbTexSphere( name, where )
	座標に水平な円を表示

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#include <string.h>
#endif

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

/*----------------------------------------------------------------*/
#define N_PRIMS	(1)		/* １枚 */
#define N_VERTS (4)		/* 四角ポリゴン */
#define PRIO	0x60	/* 敵兵より後に処理 */

//#define TEX_NAME GV_StrCode("gbs_ak_sling2")
#define TEX_NAME GV_StrCode("rcm_l_msk")
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;

	FVECTOR		pos[ N_PRIMS*N_VERTS ] ;
	FVECTOR		*center ;
	int			range ;

	int			count ;
} Work ;

/*----------------------------------------------------------------*/
static	void	SetPos( work )
Work	*work ;
{
	float r ;

	r = (float)work->range ;

	work->pos[0].vx = work->center->vx - r ;
	work->pos[0].vy = work->center->vy ;
	work->pos[0].vz = work->center->vz - r ;

	work->pos[1].vx = work->center->vx + r ;
	work->pos[1].vy = work->center->vy ;
	work->pos[1].vz = work->center->vz - r ;

	work->pos[2].vx = work->center->vx - r ;
	work->pos[2].vy = work->center->vy ;
	work->pos[2].vz = work->center->vz + r ;

	work->pos[3].vx = work->center->vx + r ;
	work->pos[3].vy = work->center->vy ;
	work->pos[3].vz = work->center->vz + r ;

	memcpy( work->prim->pos[work->prim->buffer_clock], &work->pos[0], sizeof(FVECTOR) * N_PRIMS*N_VERTS ) ;
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	if ( work->count >= 0 ) {
		if ( work->count-- == 0 ) GV_DestroyActor( work ) ;
	}
	
	DG_SwitchBuffPrim2( work->prim );
	SetPos( work ) ;

#if 0
if ( !(GV_Time & 0x80) ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[0], 3 );
	} else {
		AN_Test_Eye2( &work->pos[1], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[2], 3 );
	} else {
		AN_Test_Eye2( &work->pos[3], 3 );
	}
}

if ( GV_Time & 0x80 ) {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[4], 3 );
	} else {
		AN_Test_Eye2( &work->pos[5], 3 );
	}
} else {
	if ( GV_Time & 0x40 ) {
		AN_Test_Eye2( &work->pos[6], 3 );
	} else {
		AN_Test_Eye2( &work->pos[7], 3 );
	}
}
#endif

}

static	void	Die( work )
Work		*work ;
{
	GM_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* パケットの初期化 */
static void InitPacket( DG_PRIM2 *prim, DG_TEX *tex )
{
	int	i ;
	DG_PRIM2_UVRGB	*uvrgb ;

	uvrgb = SCRPAD_ADDR ;	/* スクラッチパッド利用 */
	for( i=0; i<N_PRIMS; i++ ) {
		/* ＵＶ値は1.3.12の固定小数点なので注意！ */
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = 128 ;
		uvrgb->g = 128 ;
		uvrgb->b = 128 ;
		uvrgb->a = 128 ;
		uvrgb ++ ;
	}
	KR_MemCopy( prim->uvrgb[ 0 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
	KR_MemCopy( prim->uvrgb[ 1 ], SCRPAD_ADDR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS ) ;
}

static	int	GetResources( Work *work, FVECTOR *pos, int range, int count )
{
	DG_TEX	*tex ;
	DG_PRIM2	*prim ;

	work->count = count ;
	work->range = range ;
	work->center = pos ;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_FOG|DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_SHADE,
			N_PRIMS, N_VERTS ) ;
	tex = DG_GetTexture( TEX_NAME ) ;
	DG_ConfigPrim2Tex( prim, tex ) ;
	prim->group_id = 0 ; /* 0で全グループ表示になるはず */

	InitPacket( prim, tex ) ;

	SetPos( work ) ;

	return 0 ;
}

void	*NewDbTexSphere( FVECTOR *pos, int range, int count )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, pos, range, count ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
