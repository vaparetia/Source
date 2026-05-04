//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  orga_sph.c
  物からの飛沫

  1999/10/23 S.Okajima
  2000/06/09 T.Morita revised
  $Id: orga_sph.c,v 1.1.1.3 2002/11/19 11:46:27 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#include "../../../okajima/etc/ok_util.h"

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2	32
#define	N_PRIMS2	6
#define	RAND_WIDTH	80.0f

#define	SCALE		0.95f

#define	P_ALPHA_MAX	64
#define	P_RGB_MAX	64

//#define	SIZE		50
#define	SIZE		80

#define	SHIFT_ROT	256.0f
#define	NORMS_LIMIT	0.8f

extern SVECTOR G_wind_rot;	/* 風向 */

typedef	struct	{
    GV_ACT_EX	actor ;
    int		name;
    int		map;
		
    DG_OBJS	   *obj ;
    DG_VERTS_ANIME *anm ;
    CV2_DEF	   *cvd ;
    SVECTOR	before_rot;
    char        *index ;
    DG_PRIM2	*prim  ;
} Work ;

float InnerProduct( SVECTOR *a, FVECTOR *b )
{
    return a->vx*b->vx +a->vy*b->vy +a->vz*b->vz ;
}

void ORG_SPH_VisibleSplash( Work *work, int flag )
{
    if ( flag )
	DG_VisiblePrim2( work->prim ) ;
    else
	DG_InvisiblePrim2( work->prim ) ;
}    

static void Act( Work *work )
{
    int         i, clock;
    FVECTOR	*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
    int		n_verts ;
#ifdef PSX2
    SVECTOR	*verts,  *norms  ;
#else
	DG_VERTEX_KMSS *vbuff ;
#endif
    FVECTOR	direction = { 0.0f, 0.0f, -1.0f, 1.0f } ;
    SVECTOR	rot;

    if ( work->obj == NULL )
    {
	GV_DestroyActor( work ) ;
	return ;
    }
    rot.vx = (G_wind_rot.vx + 512-256) & 4095 ;
    rot.vy = (G_wind_rot.vy + 2048 + (short)(frnd()*SHIFT_ROT)) & 4095 ;
    rot.vz = 0 ;
    OK_DirectionSmoother( &work->before_rot, &rot, 0.95f ) ;
    DG_SetPos2( &DG_ZeroVector, &work->before_rot ) ;
    DG_RotVector( &direction, &direction, 1 ) ;

    DG_SwitchBuffPrim2( work->prim ) ;
    clock = work->prim->buffer_clock ;
    pos     = work->prim->pos  [clock] ;
    uvrgbwh = work->prim->uvrgb[clock] ;

#ifdef PSX2
    verts   = work->obj->objs[0].verts ;
    norms   = work->obj->objs[0].norms ;
    n_verts = work->cvd->models[0].n_verts ;
#else
	vbuff   = work->obj->objs[0].vbuff ;
#endif
    for ( i=N_PRIMS2*N_VERTS2 ; --i>=0 ; uvrgbwh++, pos++ ) {
		FVECTOR v ;

		while(
#ifdef PSX2
			  !(norms->vy/(int)(NORMS_LIMIT*4096.0f))
#else
			  !(vbuff->ny/(int)(NORMS_LIMIT*4096.0f))
#endif
			  ){
			if ( --n_verts >= 0 ) {
#ifdef PSX2
				verts++, norms++ ;
#else
				vbuff++ ;
#endif
			} else {
				goto abort_splash ;
			}
		}
#ifdef PSX2
        vu0_SV0toFV( verts, &v ) ;
#else
		v.vx = vbuff->vx ;
		v.vy = vbuff->vy ;
		v.vz = vbuff->vz ;
#endif
		v.vw = 1.0f ;
		_sceVu0ApplyMatrix( pos, &work->obj->world, &v ) ;
		pos->vx += frnd()*RAND_WIDTH ;
		pos->vy += frnd()*RAND_WIDTH ;
		pos->vz += frnd()*RAND_WIDTH ;
		uvrgbwh->a = P_ALPHA_MAX;

		if ( --n_verts >= 0 ) {
#ifdef PSX2
			verts++, norms++ ;
#else
			vbuff++ ;
#endif
		} else {
			goto abort_splash ;
		}
    }

abort_splash:
    while( --i>=0 )
	(uvrgbwh++)->a = 0 ;    /* あまったPRIMは消す */
}


static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    if ( work->index )
	GV_Free( work->index ) ;
}

//#define FTOI12(_f) DG_FTOI( ( (float)(_f)*4096.0f) )
static void InitPacket2( DG_PRIM2 *prim, DG_TEX *tex )
{
    DG_PRIM2_UVRGBWH *u0, *u1 ;	/* スプライト用 */
    int		      i ;
    FVECTOR          *p0, *p1 ;

    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

    u0 = prim->uvrgb[0] ;
    u1 = prim->uvrgb[1] ;
    p0 = prim->pos  [0] ;
    p1 = prim->pos  [1] ;
    for ( i=N_PRIMS2*N_VERTS2 ; --i>=0 ;  )
    {
	u0->u0 = u1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	u0->v0 = u1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	u0->u1 = u1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	u0->v1 = u1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	u0->q0 = u1->q0 = u0->q1 = u1->q1 = 4096 ;
	u0->f0 = u1->f0 = u0->f1 = u1->f1 = 0x0fff ;
	u0->w  = u1->w  = u0->h  = u1->h  = SIZE ;
	u0->r  = u1->r  = P_RGB_MAX ;
	u0->g  = u1->g  = P_RGB_MAX ;
	u0->b  = u1->b  = P_RGB_MAX ;
	u0->a  = u1->a  = P_ALPHA_MAX ;
	u0++, u1++ ;

	_sceVu0CopyVector( p0, &DG_ZeroVector ) ;
	_sceVu0CopyVector( p1, &DG_ZeroVector ) ;
	p0++, p1++ ;
    }
}

static int GetResources( Work *work, DG_OBJS *obj, int cvd_id, DG_VERTS_ANIME *anm )
{
    int i ;
    /* アニメ構造体のポインタを取得 */
    work->anm = anm ;

    /* モデルの共有頂点データ取得 */
    if ( !(work->cvd = GV_GetCache( GV_CacheID( cvd_id, 'c' ) )) )
	PERROR( "Can't find CV2 file in DATA.CNF :NewObjectSplash\n" ) ;

    if ( !(work->obj = obj ) )
        PERROR( "OBJS is NULL!! :NewObjectSplash\n" ) ;

    /* スプライトの初期化 */
    if ( !(work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				      N_PRIMS2, N_VERTS2 ) ) )
        PERROR( "Can't create DG_PRIM2( Maybe no memory ) :NewObjectSplash\n" ) ;
    InitPacket2( work->prim, DG_GetTexture( GV_StrCode("splash06_msk") ) ) ;

    if ( !(work->index = GV_Malloc( i=work->cvd->models[0].n_verts_index )) )
	PERROR( "Can't alloc memory :NewObjectSplash\n" ) ;
    while( --i>=0 )
	work->index[i] = work->prim->buffer_clock ;

    return 0 ;
}


void *NewObjectSplash( DG_OBJS *objs, int model_id, DG_VERTS_ANIME *anm )
{
    Work *work ;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, objs, model_id, anm ) < 0 ) 
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
