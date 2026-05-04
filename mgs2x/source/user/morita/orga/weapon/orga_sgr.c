//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_sgr.c 
   オルガ 武器関数(手榴弾)

   2000/01/11 T.Morita
   $Id: orga_sgr.c,v 1.1.1.3 2002/11/19 11:46:28 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "orga_wep.h"
#include "../include/orga_se.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"


#define ORGA_SGR_CURVE    (500.0f*7.0f)
#define ORGA_SGR_GRAVITY  6
#define ORGA_SGR_BOUNCE   0.48f

#define ORGA_SGR_N_PACKS  4
#define ORGA_SGR_N_VERTS  32/*軌跡の頂点数（多くすれば軌跡は伸びる）*/

#define ORGA_SGR_SPHERE   40
#define ORGA_SGR_R_VEL    0.48f
#define ORGA_SGR_R_VEL2   0.80f

#define ORGA_MAX_GRENADE  5



typedef	struct work_sgr_t Work ;
typedef	struct grenade_t Grenade ;

struct grenade_t
{
    FVECTOR      pos    ;  /* 現在の位置 */
    FVECTOR      vel    ;  /* 現在の速度 */
    short        rot    ;  /**/
    short        rot_v  ;  /**/

    DG_PRIM2    *buller ;  /*軌跡処理 */
    FVECTOR      org[ORGA_SGR_N_PACKS] ;/* 軌跡発生位置（グレネードローカル座標） */
    FVECTOR      trgt   ;
    int          frict  ;
    DG_OBJS     *weapon ;
} ;

struct work_sgr_t
{
    GV_ACT	 actor  ;

    FVECTOR     *trgt   ; /* vw != 1.0f のときアクト開始 */
    HZX_GROUP_ID hzx    ;

    FMATRIX     *world  ;
    FVECTOR      offset ;
    Grenade      grenade[ORGA_MAX_GRENADE] ;
    int          n_grenade ;

    int          frict  ;

    int          damage ;
    int          dmg_rad ;
    int          blw_rad ;
} ;

static FVECTOR BounceVector = { -1.0, -ORGA_SGR_BOUNCE, -1.0 } ;


extern void *NewBlast( FVECTOR *pos,int side,int range1,int range2,int damage,int faint,int wp ) ;
extern void *NewBombEffect( FVECTOR *center, int effect ) ;
extern void *NewOrgaRippleSplash( FVECTOR *pos, float width, int number, int time ) ;
extern void *NewSplashParts_Demo( FVECTOR *center, SVECTOR *rot, float intense ) ;           /* okajima/demo_effect/d_splash_parts.c*/


static void ActStartBuller( DG_PRIM2 *prim, FMATRIX *m ) 
{
    int     i ;
    FVECTOR *p ;

    p = prim->pos[0] ;
    for ( i=ORGA_SGR_N_PACKS*ORGA_SGR_N_VERTS ; --i>=0 ; p++ )
	_sceVu0CopyVector( p, (FVECTOR*)&m->m[3] ) ;
    p = prim->pos[1] ;
    for ( i=ORGA_SGR_N_PACKS*ORGA_SGR_N_VERTS ; --i>=0 ; p++ )
	_sceVu0CopyVector( p, (FVECTOR*)&m->m[3] ) ;
}

static void ActBuller( DG_PRIM2 *prim, FMATRIX *m, FVECTOR *offset, float flag ) 
{
    int     i,  j ;
    FVECTOR t, *p ;

    DG_SwitchBuffPrim2( prim ) ;
    p = prim->pos[prim->buffer_clock] ;

    if ( p->vx == (p+ORGA_SGR_N_VERTS-1)->vx &&
	 p->vy == (p+ORGA_SGR_N_VERTS-1)->vy &&
	 p->vz == (p+ORGA_SGR_N_VERTS-1)->vz )
	DG_InvisiblePrim2( prim ) ;
    for ( i=ORGA_SGR_N_PACKS ; --i>=0 ; p++ )
    {
	for ( j=ORGA_SGR_N_VERTS ; --j>0 ; p++ )
	    _sceVu0CopyVector( p, p+1 ) ;
	if ( flag != 0.0f )
	{
	    _sceVu0ApplyMatrix( &t, m, offset++ ) ;
	    _sceVu0AddVector( p, &t, (FVECTOR*)&m->m[3] ) ;
	}
    }
}

static void ActGrenade( Work *work, Grenade *g )
{
    ActBuller( g->buller, &g->weapon->world, g->org, g->trgt.vw ) ;

    switch ( (int)g->trgt.vw )
    {
    case -3:/*外部*/
	DG_VisibleObjs( g->weapon ) ;
	g->trgt.vw = -1.0f ;
	break ;

    case -2:/*内部*/
	DG_InvisibleObjs( g->weapon ) ;
	g->trgt.vw = 0.0f ;
	g->frict  = 0 ;
	break ;

    case -1: /*表示後は -1 になるはず*/
    case  0: /*非表示は 0 になるはず  */
	g->vel.vx = 0.0f ;/*（手の中） 内部*/
	g->weapon->world = *work->world ;
	break ;

    case 1: /* 下がるまで(当たりを見ない) */
	DG_VisiblePrim2( g->buller ) ;
	if ( (g->vel.vy -= ORGA_SGR_GRAVITY) < 0.0f ) /* 重力加速度   */
	    g->trgt.vw = 2.0f ; /* 当たりを見る飛ばし */
	_sceVu0AddVector( &g->pos, &g->pos, &g->vel ) ;/* 位置更新     */
	g->weapon->world = DG_UnitMatrix ;
	TransMatrix( &g->weapon->world, &work->offset ) ;
	RotateMatrixX( &g->weapon->world, &g->weapon->world, g->rot+=g->rot_v ) ;
	TransMatrix( &g->weapon->world, &g->pos   ) ;
	break ;

    case 2: /* 跳ね終るまで */
	g->vel.vy -= ORGA_SGR_GRAVITY ;  /* 重力加速度   */
	if ( ORG_HzdOnlineCheck( work->hzx, &g->pos, &g->vel, 0.5f, &BounceVector ) )
	{
	    _sceVu0ScaleVector( &g->vel, &g->vel, 0.97f ) ;/* 摩擦処理 */
	    //if ( (int)((g->vel.vy-g->vel.vw)/8) )
	    if ( (int)(g->vel.vy/4.0f) )
	    {
		NewOrgaRippleSplash( &g->pos, 100.0f, 4, 10 ) ;
		NewSplashParts_Demo( &g->pos, &DG_ZeroSVector, 200.0f ) ;
		/* 摩擦処理 */
		_sceVu0ScaleVector( &g->vel, &g->vel,
				    g->frict ? ORGA_SGR_R_VEL2 : ORGA_SGR_R_VEL ) ;

		GM_SeSetMode( ORGA_SE_SGR_BOUND, &g->pos, GM_SEMODE_NORMAL ) ;
		g->rot_v = (short)(g->rot_v * -0.4f) ;
	    }
	    else
		g->vel.vy=0.0f, g->rot_v=0, g->rot=0 ;

	    if ( !(int)(g->vel.vx) && g->vel.vy==0.0f && !(int)(g->vel.vz) )
	    {
		g->trgt.vw = -2.0f ;    /* 消してから終了 */
		NewBombEffect( &g->pos, 2 ) ;/* 爆破 */
		NewBlast( &g->pos, BOTH_SIDE, work->dmg_rad,work->blw_rad, work->damage, 10, WP_C4Bomb ) ;
	    }
	    g->rot_v = (0 - g->rot)/8 ;

	    //g->vel.vw = g->vel.vy ;
	}
	else
	    _sceVu0AddVector( &g->pos, &g->pos, &g->vel ) ;/* 位置更新     */
	g->weapon->world = DG_UnitMatrix ;
	TransMatrix( &g->weapon->world, &work->offset ) ;
	RotateMatrixX( &g->weapon->world, &g->weapon->world, g->rot+=g->rot_v ) ;
	TransMatrix( &g->weapon->world, &g->pos   ) ;
	break ;

    default:
	/* なげ始め */
	g->rot    = 0   ;
	g->rot_v  = 300 ;
	g->pos    = *(FVECTOR *)&work->world->m[W] ;                   /* 位置初期化 */
	g->trgt.vw = (g->trgt.vw < 60.0f && g->trgt.vw > 20.0f) ? 60.0f : g->trgt.vw ;
	g->vel.vy =  0.95f * g->trgt.vw * ORGA_SGR_GRAVITY / 2 ;
	g->trgt.vw = (g->trgt.vw < 28.0f) ? 28.0f : g->trgt.vw ;
	g->vel.vx = (g->trgt.vx - g->pos.vx) / g->trgt.vw ; /* 速度初期化 */
	g->vel.vz = (g->trgt.vz - g->pos.vz) / g->trgt.vw ;
	ActStartBuller( g->buller, work->world ) ;
	g->trgt.vw = 1.0f ; /* 当たりを見ない飛ばし */
    }
}


static void Die( Work *work )
{
    int i ;

    for ( i=ORGA_MAX_GRENADE ; --i>=0 ; )
    {
	if ( work->grenade[i].weapon )
	    DG_DequeueObjs( work->grenade[i].weapon ), DG_FreeObjs( work->grenade[i].weapon ) ;
	if ( work->grenade[i].buller )
	    GM_FreePrim2( work->grenade[i].buller ) ;
    }
}

static void Act( Work *work )
{
    int i ;

    /* キャラを殺す */
    if ( work->trgt->vw == -1.0f )
    {
	GV_DestroyActor( work ) ;
	return ;
    }

    /* 非表示 */
    else if ( work->trgt->vw == -2.0f )
    {
	for ( i=ORGA_MAX_GRENADE ; --i>0 ; )
	    if ( work->grenade[i].trgt.vw <= 0.0f &&
		 !(work->grenade[i].weapon->flag & DG_PRIM2_INVISIBLE) )
		work->grenade[i].trgt = *work->trgt ;
    }

    /* 表示 */
    else if ( work->trgt->vw == -3.0f )
    {
	for ( i=ORGA_MAX_GRENADE ; --i>0 ; )
	    if ( work->grenade[i].trgt.vw <= 0.0f &&
		 work->grenade[i].buller->flag & DG_PRIM2_INVISIBLE )
		break ;
	work->grenade[i].trgt = *work->trgt ;
    }

    /* デモなので全て止め */
    else if ( work->trgt->vw == -4.0f )
    {
	for ( i=ORGA_MAX_GRENADE ; --i>0 ; )
	    if ( work->grenade[i].trgt.vw != 0.0f )
	    {
		work->grenade[i].trgt.vw = -2.0f ;
		DG_InvisiblePrim2( work->grenade[i].buller ) ;
	    }
    }

    /* 今回投げるのは摩擦無しのグレネード */
    else if ( work->trgt->vw == -5.0f )
	work->frict = 1 ;

    /* 投げる */
    else if ( work->trgt->vw > 2.0f )
    {
	for ( i=ORGA_MAX_GRENADE ; --i>0 ; )
	    if ( work->grenade[i].trgt.vw == -1.0f )
		break ;
	work->grenade[i].trgt  = *work->trgt ;
	work->grenade[i].frict = work->frict ;
	work->frict = 0 ;
    }

    work->trgt->vw = 0.0f ;
    for ( i=ORGA_MAX_GRENADE ; --i>=0 ; )
	ActGrenade( work, &work->grenade[i] ) ;
}

static void InitLinePrim( DG_PRIM2_UVRGB *u, int i, int n_verts, u_int rgba )
{
    int j ;

    for ( ; --i>=0 ; )
	for ( j=0 ; j<n_verts ; j++, u++ )
	{
	    u->f = j ? 0x0fff : 0x8fff ;
	    u->r =  (rgba >>  0) & 0xff ;
	    u->g =  (rgba >>  8) & 0xff ;
	    u->b =  (rgba >> 16) & 0xff ;
	    u->a = ((rgba >> 24) & 0xff)*j/n_verts ;
	}
}

static int GetResources( Work *work, FMATRIX *world, FVECTOR *trigger,
			 int damage, int dmg_rad, int blw_rad )
{
    DG_DEF  *def ;
    Grenade *g ;
    int i ;

    if ( !(def = GV_GetCache( GV_CacheID( GV_StrCode( "sgr" ), 'k' ) )) )
	PERROR( "'sgr.kms' has not been found!!(not in data.cnf) : NewOrgaWeaponSGR\n" ) ;
    for ( i=ORGA_MAX_GRENADE ; --i>=0 ; )
    {
	g = &work->grenade[i] ;
	if ( !(g->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "No memory for Grenade DG_OBJS : NewOrgaWeaponSGR\n" ) ;
	DG_QueueObjs( g->weapon ) ;
	DG_InvisibleObjs( g->weapon ) ;
	g->weapon->world = *world ;
	g->vel = DG_ZeroVector ;

	if ( !(g->buller = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_ALPHA,
					 ORGA_SGR_N_PACKS, ORGA_SGR_N_VERTS ) ) )
	    return -1 ;
	InitLinePrim( g->buller->uvrgb[0], ORGA_SGR_N_PACKS, ORGA_SGR_N_VERTS, 0x80202a20  ) ;
	InitLinePrim( g->buller->uvrgb[1], ORGA_SGR_N_PACKS, ORGA_SGR_N_VERTS, 0x80202a20  ) ;
	DG_InvisiblePrim2( g->buller ) ;
	DG_SetPrim2Alpha( g->buller, SCE_GS_SET_ALPHA(0,2,0,1,0) ) ;

	g->org[0].vx = def->lx ; g->org[0].vy = def->ly ; g->org[0].vz = def->lz ;
	g->org[1].vx = def->ux ; g->org[1].vy = def->uy ; g->org[1].vz = def->uz ;
	g->org[2].vx = def->lx ; g->org[2].vy = def->uy ; g->org[2].vz = def->lz ;
	g->org[3].vx = def->ux ; g->org[3].vy = def->ly ; g->org[3].vz = def->uz ;
	g->trgt.vw = -2.0f ;
	g->frict  = 0 ;
    }

    _sceVu0AddVector( &work->offset, &work->grenade[0].org[0], &work->grenade[0].org[1] ) ;
    _sceVu0ScaleVector( &work->offset, &work->offset, -0.5f ) ;

    work->damage  = damage  ;
    work->dmg_rad = dmg_rad ? dmg_rad : 1200 ;
    work->blw_rad = blw_rad ? blw_rad : 2000 ;
    work->world   = world   ;
    work->trgt    = trigger ;
    work->hzx     = GM_GetHzxGroupID( GM_CurrentMap ) ;

    return 0 ;
}

void *NewOrgaWeaponSGR( FMATRIX *world, FVECTOR *trigger, int damage, int dmg_rad, int blw_rad )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &(work->actor), Act, Die ) ;
        if( GetResources( work, world, trigger, damage, dmg_rad, blw_rad ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
