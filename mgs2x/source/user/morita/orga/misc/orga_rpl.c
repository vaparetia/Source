//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  orga_rpl.c
  リップル

  2000/09/30 T.Morita 
  $Id: orga_rpl.c,v 1.1.1.3 2002/11/19 11:46:27 Yoshizawa1 Exp $
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


/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2	32
#define	N_PRIMS2	6
#define	N_VERTS	4
#define	N_PRIMS	5

#define	P_ALPHA_MAX	64
#define	P_RGB_MAX	64

#define	RIPPLE_VEL	0.25f
#define	RIPPLE_ALPH_VEL	2
#define	SIZE	80.0f

extern SVECTOR G_wind_rot;	/* 風向 */

typedef	struct	{
    GV_ACT_EX	actor ;

    FVECTOR      center ;
    float        width  ;
    int          n_prim ;
    int          time   ;
    DG_PRIM2	*splash ;
    DG_PRIM2	*add_rpl ;
    DG_PRIM2	*sub_rpl ;
} Work ;


static void Act( Work *work )
{
    int         i, j ;
    int         flag = 1 ;
    FVECTOR     center ;
    FVECTOR	*p_cur, *p_prv ;
    DG_PRIM2_UVRGB   *u_cur , *u_prv  ;	/* リップル用 */

    p_prv = work->add_rpl->pos  [work->add_rpl->buffer_clock] ;
    u_prv = work->add_rpl->uvrgb[work->add_rpl->buffer_clock] ;
    DG_SwitchBuffPrim2( work->add_rpl ) ;
    p_cur = work->add_rpl->pos  [work->add_rpl->buffer_clock] ;
    u_cur = work->add_rpl->uvrgb[work->add_rpl->buffer_clock] ;

    for ( i=work->n_prim ; --i>=0 ;  )
	if ( u_prv->a )
	{
	    flag = 0 ;

	    for ( j=N_VERTS ; --j>=0 ; p_cur++, p_prv++, u_cur++, u_prv++ )
	    {
		u_cur->a  = u_prv->a>RIPPLE_ALPH_VEL ? u_prv->a - RIPPLE_ALPH_VEL : 0  ;
		p_cur->vx = p_prv->vx + (float)(j&1 ? u_cur->a : -u_cur->a)*RIPPLE_VEL ;
		p_cur->vz = p_prv->vz + (float)(j&2 ? u_cur->a : -u_cur->a)*RIPPLE_VEL ;
	    }
	}
	else if ( work->time > 0 )
	{
	    flag = 0 ;

	    center.vx = work->center.vx + frnd() * work->width ;
	    center.vy = work->center.vy +  rnd() * 3.0f        ;
	    center.vz = work->center.vz + frnd() * work->width ;
	    center.vw = 1.0f ;
	    for ( j=N_VERTS ; --j>=0 ; p_cur++, p_prv++, u_cur++, u_prv++ )
	    {
		u_cur->a = u_prv->a = P_ALPHA_MAX ;
		_sceVu0CopyVector( p_prv, &center ) ;
		_sceVu0CopyVector( p_cur, &center ) ;
	    }
	}

    work->time-- ;
    if ( flag )
	GV_DestroyActor( work ) ;
}


static void Die( Work *work )
{
    if ( work->splash )
	GM_FreePrim2( work->splash ) ;
    if ( work->add_rpl )
	GM_FreePrim2( work->add_rpl ) ;
    if ( work->sub_rpl )
	GM_FreePrim2( work->sub_rpl ) ;
}

#define FTOI12(_f) DG_FTOI( ( (float)(_f)*4096.0f) )

static void InitPacket( DG_PRIM2 *prim, DG_TEX *tex, int i, int alpha, FVECTOR *pos )
{
    DG_PRIM2_UVRGB *u0, *u1 ;
    int		    j ;
    FVECTOR        *p0, *p1 ;

    DG_ConfigPrim2Tex( prim, tex ) ;
    DG_SetPrim2Alpha( prim, alpha );

    u0 = prim->uvrgb[0] ;
    u1 = prim->uvrgb[1] ;
    p0 = prim->pos[0] ;
    p1 = prim->pos[1] ;
    for( ; --i>=0 ; u0+=4, u1+=4 )
    {
	u0[0].u = u0[2].u = u1[0].u = u1[2].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	u0[1].u = u0[3].u = u1[1].u = u1[3].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	u0[0].v = u0[1].v = u1[0].v = u1[1].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	u0[2].v = u0[3].v = u1[2].v = u1[3].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	u0[0].q = u0[1].q = u0[2].q = u0[3].q =
	    u1[0].q = u1[1].q = u1[2].q = u1[3].q = 4096 ;
	u0[0].f = u0[1].f = u1[0].f = u1[1].f = 0x8fff ;
	u0[2].f = u0[3].f = u1[2].f = u1[3].f = 0x0fff ;
	u0[0].r = u0[1].r = u0[2].r = u0[3].r = 
	    u1[0].r = u1[1].r = u1[2].r = u1[3].r = P_RGB_MAX ;
	u0[0].g = u0[1].g = u0[2].g = u0[3].g = 
	    u1[0].g = u1[1].g = u1[2].g = u1[3].g = P_RGB_MAX ;
	u0[0].b = u0[1].b = u0[2].b = u0[3].b = 
	    u1[0].b = u1[1].b = u1[2].b = u1[3].b = P_RGB_MAX ;
	u0[0].a = u0[1].a = u0[2].a = u0[3].a = 
	    u1[0].a = u1[1].a = u1[2].a = u1[3].a = (i ? irnd()&(P_ALPHA_MAX - 1) : P_ALPHA_MAX) ;

	for ( j=N_VERTS ; --j>=0 ; p0++, p1++ )
	{
	    _sceVu0CopyVector( p0, (i ? &DG_ZeroVector : pos) ) ;
	    _sceVu0CopyVector( p1, (i ? &DG_ZeroVector : pos) ) ;
	}
    }
}

static int GetResources( Work *work, FVECTOR *pos, float width, int number, int time )
{
    work->n_prim =  number ;
    work->width  =  width  ;
    work->time   =  time   ;
    work->center = *pos    ;

    /* リップル */
    if ( !(work->add_rpl = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_SHADE| DG_PRIM2_TEX| DG_PRIM2_ALPHA,
					work->n_prim, N_VERTS ) ) )
        PERROR( "Can't create DG_PRIM2( Maybe no memory ) :NewRippleSplash\n" ) ;
    InitPacket( work->add_rpl,
		DG_GetTexture( GV_StrCode(/*"hamon04_add_msk"*/"hamon06_add_msk") ),
		work->n_prim,
		SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ),
		pos
	) ;

#if 0
    if ( !(work->sub_rpl = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_SHADE| DG_PRIM2_TEX| DG_PRIM2_ALPHA,
					  work->n_prim, N_VERTS ) ) )
        PERROR( "Can't create DG_PRIM2( Maybe no memory ) :NewRippleSplash\n" ) ;
    InitPacket( work->sub_rpl,
		DG_GetTexture( GV_StrCode("hamon06h_add") ),
		work->n_prim,
		SCE_GS_SET_ALPHA( 0, 1, 0, 2, 0x00 ),
		pos
	) ;

    /* スプライトの初期化 */
    if ( !(work->splash = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				      N_PRIMS2, N_VERTS2 ) ) )
        PERROR( "Can't create DG_PRIM2( Maybe no memory ) :NewRippleSplash\n" ) ;
    InitPacket2( work->splash, DG_GetTexture( GV_StrCode("splash06_msk") ) ) ;
#endif

    return 0 ;
}

/*
  pos    場所の中心
  width  ランダムに出る広さ（mm単位）
  number 波紋の個数
  time   波紋の持続時間
*/
void *NewOrgaRippleSplash( FVECTOR *pos, float width, int number, int time )
{
    Work *work ;

    work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, pos, width, number, time ) < 0 ) 
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
