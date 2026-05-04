/*
   brk_lck_ini.c
   ロッカー壊れ 初期化

   1999/12/08 T. Morita
   $Id: brk_lck_ini.c,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_locker.h"

static void init_g2pkt( DG_PRIM_PACKET *packet, int i )
{
    DG_LINE_G2 *p ;

    *(u_long*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA(0,2,0,1,0) ;
    packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
    p = (DG_LINE_G2*)packet->prim_top ;
    for ( ; --i>=0 ; p++ )
    {
        DG_SET_RGBA1( p, 128,128,128, 64 ) ;
        DG_SET_RGBA2( p, 128,128,128,128 ) ;
    }
}

int BRK_LCK_InitDbgPrimitive( Work *work )
{
    if ( !(work->dbg = GM_MakePrim( DG_PRIM_ON_WORLD|DG_PRIM_LINE_G2,
				    1, BRK_N_DBG_LN,
				    dbg_shape, NULL )) )
        return -1 ;
    init_g2pkt( (DG_PRIM_PACKET*)work->dbg->packs[0], BRK_N_DBG_LN ) ;
    init_g2pkt( (DG_PRIM_PACKET*)work->dbg->packs[1], BRK_N_DBG_LN ) ;

    return 0 ;
}

int BRK_LCK_InitTarget( Work *work )
{
    FVECTOR t_size ;
    FVECTOR t_pos  ;
    DOOR *d ;
    int i ;
    TARGET *t ;

    t_size.vx = work->obj->def->ux - work->obj->def->lx ;
    t_size.vx = t_size.vx<80.0f ? 40.0f : t_size.vx*0.5f ;
    t_size.vy = work->obj->def->uy - work->obj->def->ly ;
    t_size.vy = t_size.vy<80.0f ? 40.0f : t_size.vy*0.5f ;
    t_size.vz = work->obj->def->uz - work->obj->def->lz ;
    t_size.vz = t_size.vz<80.0f ? 40.0f : t_size.vz*0.5f ;

    t_pos.vx = work->world.m[3][X] + work->obj->def->lx + t_size.vx ;
    t_pos.vy = work->world.m[3][Y] + work->obj->def->ly + t_size.vy ;
    t_pos.vz = work->world.m[3][Z] + work->obj->def->lz + t_size.vz ;

    GM_SetTarget( &work->target, TARGET_DEFENSE, 0, ENEMY_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( &work->target, &work->power, POWER_DECREASE, GM_Vitality, 0, 0,&DG_ZeroVector) ;
    GM_PutTarget( &work->target ) ;

    NewTargetView( &work->target,  200, 50, 32 ) ;

    t = work->child = GV_Malloc( sizeof(TARGET)*work->n_doors ) ;
    d = work->doors ;
    for ( i=work->n_doors ; --i>=0 ; d++, t++ )
    {
	t_size.vx = d->mdl->ux - d->mdl->lx ;
	d->dir_x = t_size.vx<80.0f ? 0.1f : 0.0f ;
	t_size.vx = t_size.vx<80.0f ? 40.0f : t_size.vx*0.5f ;

	t_size.vy = d->mdl->uy - d->mdl->ly ;
	d->dir_y = t_size.vy<80.0f ? 0.1f : 0.0f ;
	t_size.vy = t_size.vy<80.0f ? 40.0f : t_size.vy*0.5f ;

	t_size.vz = d->mdl->uz - d->mdl->lz ;
	d->dir_z = t_size.vz<80.0f ? 0.1f : 0.0f ;
	t_size.vz = t_size.vz<80.0f ? 40.0f : t_size.vz*0.5f ;

	t_pos.vx = work->world.m[3][X] + d->mdl->lx + t_size.vx + d->mdl->tx ;
	t_pos.vy = work->world.m[3][Y] + d->mdl->ly + t_size.vy + d->mdl->ty ;
	t_pos.vz = work->world.m[3][Z] + d->mdl->lz + t_size.vz + d->mdl->tz ;

	GM_SetTarget( t, TARGET_DEFENSE, 1, ENEMY_SIDE, &t_size, &t_pos ) ;
	GM_SetPowerTarget( t, &d->power, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( t, BRK_LCK_TargetCallBack, d ) ;
	NewTargetView( t,  50, 200, 32 ) ;
    }
    GM_SetTargetParts( &work->target, work->child, work->n_doors, 0 ) ;

    return 0 ;
}

int BRK_LCK_SetupPolydata( Work *work )
{
    int i, j, k ;
    DOOR       *d ;
    SVECTOR    *n ;
    POLY       *p ;
    CV2_MDL    *c ;
    DG_OBJ_PACKET *o ;
    short      *idx ;
int cnt=0 ;

#define BRK_N_POLY 3000
    if ( !(work->poly = GV_Malloc( sizeof(POLY)*BRK_N_POLY )) )
	return -1 ;

    d = work->doors ;
    p = work->poly  ;
    for ( i=work->n_doors ; --i>=0 ; d++ )
    {
	d->poly   = p ;
	d->n_poly = 0 ;
	c = d->cv2 ;
	idx = c->verts_index ;
	o = d->obj->packets ;
	for ( j=d->obj->n_packs ; --j>=0 ; o++ ) /* <o> or <s> either one */
	{
	    n = d->obj->norms + o->norms_offset*2 ;
	    for ( k=o->n_verts ; --k>=0 ; n++, idx++ )
		if ( n->pad==0xfff )
		{
		    p->v0 = &c->verts[p->i0 = *(idx-0)] ;
		    p->v1 = &c->verts[p->i1 = *(idx-1)] ;
		    p->v2 = &c->verts[p->i2 = *(idx-2)] ;
		    p->n0 = n-0 ;
		    p->n1 = n-1 ;
		    p->n2 = n-2 ;
		    p->nrm.vx = n->vx/4096.0f ;
		    p->nrm.vy = n->vy/4096.0f ;
		    p->nrm.vz = n->vz/4096.0f ;
		    p->nrm.vw = 1.0f ;
		    p++ ;
		    d->n_poly++ ;
cnt++ ;
		}
	    if ( o->n_verts & 1 )
		idx++ ;
	}
    }

printf( "N_POLY %d\n", cnt ) ;
    return 0 ;
}

static void BRK_LCK_InitModel( Work *work, int model_id )
{
    DOOR    *d ;
    DG_DEF  *def ;
    DG_MDL  *m ;
    CV2_MDL *c ;
    DG_OBJ  *o ;
    int      i ;

    def = GV_GetCache( GV_CacheID( model_id, 'k' ) ) ;
    work->cv2 = GV_GetCache( GV_CacheID( model_id, 'c' ) );
    work->obj = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 ) ;
    work->obj->world = work->world ;
    DG_QueueObjs( work->obj ) ;

    i = work->n_doors = work->obj->n_models ;
    o = work->obj->objs ;
    m = def->models ;
    d = work->doors = GV_Malloc( sizeof(DOOR)*i ) ;
    c = work->cv2->models ;
    for (  ; --i>=0 ; o++, d++, m++, c++ )
    {
	DG_MakeAnimVertsBuffer( &d->v_anm, o, DG_VANIME_VERTS|DG_VANIME_NORMS|DG_VANIME_SINGLE ) ;
	//DG_MakeAnimVertsBuffer( &d->v_anm, o, DG_VANIME_VERTS|DG_VANIME_SINGLE ) ;
        o->world = DG_UnitMatrix ;
        o->world.m[3][X] = m->tx + work->world.m[3][X] ;
        o->world.m[3][Y] = m->ty + work->world.m[3][Y] ;
        o->world.m[3][Z] = m->tz + work->world.m[3][Z] ;
	d->obj   = o ;
	d->mdl   = m ;
	d->cv2   = c ;
	d->world = &o->world ;
    }
}

int BRK_LCK_GetOptions( Work *work )
{
    int      i, buf[3] ;
    SVECTOR  rot ;
    FMATRIX *mtx ;
    FVECTOR  scale, pos ;

    mtx = &work->world ;
    *mtx = DG_UnitMatrix ;
    if ( GCL_GetOption( 'r' ) != NULL )
    {
	GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	RotateMatrix( mtx, &DG_UnitMatrix, &rot ) ;
    }
    if ( GCL_GetOption( 's' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &scale ) ;
	mtx->m[X][X] *= scale.vx/100.0F ;
	mtx->m[Y][Y] *= scale.vy/100.0F ;
	mtx->m[Z][Z] *= scale.vz/100.0F ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
	TransMatrix( mtx, &pos ) ;
    }

    if ( (i = GCL_GetOptionValue( 'm', 0 )) )
	BRK_LCK_InitModel( work, i ) ;
    else
	return -1 ;

    return 0 ;
    if ( (i = GCL_GetOptionValue( 'l', 0 )) != 0 )
        DG_MakePreshade( work->obj, (LIT_DEF *)GV_GetCache( GV_CacheID( i, 'l' ) ) ) ;

    return 0 ;
}


