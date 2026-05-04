/*
   brk_lck_clb.c
   ロッカー壊れ コールバック

   1999/12/08 T. Morita
   $Id: brk_lck_clb.c,v 1.1.1.3 2002/11/19 11:45:33 Yoshizawa1 Exp $
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


FVECTOR dbg_shape[BRK_N_DBG_LN*2] ;
static void ShowDebugNormal( DOOR *d )
{
    int i,j, idx ;

    for ( j=d->obj->n_packs, idx=0 ; --j>=0 ; )
    {
	SVECTOR *n = &d->obj->norms[d->obj->packets[j].norms_offset*2] ;
	SVECTOR *v = &d->obj->verts[d->obj->packets[j].verts_offset*2] ;
	i = d->mdl->packs[j].n_verts ;
	for ( ; --i>=0 ; v++, n++, idx+=2 )
	{
	    dbg_shape[idx  ].vx = v->vx + d->world->m[3][X] ;
	    dbg_shape[idx  ].vy = v->vy + d->world->m[3][Y] ;
	    dbg_shape[idx  ].vz = v->vz + d->world->m[3][Z] ;
	    dbg_shape[idx+1].vx = v->vx + d->world->m[3][X] - n->vx/16 ;
	    dbg_shape[idx+1].vy = v->vy + d->world->m[3][Y] - n->vy/16 ;
	    dbg_shape[idx+1].vz = v->vz + d->world->m[3][Z] - n->vz/16 ;
	}
    }
}

static void CalcNormalFromVertex( FVECTOR *norm, FVECTOR *v00, FVECTOR *v10, FVECTOR *v20 )
{
    FVECTOR a0, a1, v ;
    float l ;

    a0.vx = v10->vx - v00->vx ;    a1.vx = v20->vx - v10->vx ;
    a0.vy = v10->vy - v00->vy ;    a1.vy = v20->vy - v10->vy ;
    a0.vz = v10->vz - v00->vz ;    a1.vz = v20->vz - v10->vz ;

    v.vx  = a0.vy*a1.vz - a0.vz*a1.vy ;
    v.vy  = a0.vz*a1.vx - a0.vx*a1.vz ;
    v.vz  = a0.vx*a1.vy - a0.vy*a1.vx ;

    l = fpu_Rsqrt( v.vx*v.vx + v.vy*v.vy + v.vz*v.vz, 1.0f ) ;
    if ( norm->vx*v.vx + norm->vy*v.vy + norm->vz*v.vz > 0 )
	norm->vx =  v.vx*l, norm->vy =  v.vy*l, norm->vz =  v.vz*l ;
    else
	norm->vx = -v.vx*l, norm->vy = -v.vy*l, norm->vz = -v.vz*l ;
    norm->vw = 1.0f ;
}




static void BRK_LCK_None( TARGET *off, TARGET *def, DOOR *d )
{
}


static void BRK_LCK_WeaponGun( TARGET *off, TARGET *def, DOOR *d )
{
    int      i, idx ;
    FVECTOR *v, *n ;
    float    dx, dy, dz, dis ;
    CV2_MDL *c = d->cv2 ;
    POLY    *p ;

    GM_SeSetMode( SD_W_RICOCH01, &off->hit, GM_SEMODE_NORMAL ) ;

    /*モデル内相対位置の算出*/
    off->hit.vx -= d->world->m[3][X] ;
    off->hit.vy -= d->world->m[3][Y] ;
    off->hit.vz -= d->world->m[3][Z] ;

    /*モデル内最近頂点の算出*/
    for ( i=c->n_verts, v=c->verts+i ; --i>=0 ; )
    {
	dx = off->hit.vx - (--v)->vx ;
	dy = off->hit.vy - (  v)->vy ;
	dz = off->hit.vz - (  v)->vz ;

	dis = dx*dx + dy*dy + dz*dz ;
	if ( (dis = 6000.0f/dis) >= 0.3f )
	{
	    dis = dis>1.0f ? 1.0f : dis ;

	    v->vx += off->power->force.vx * d->dir_x*dis ;
	    if ( d->dir_x > 0.0f )
		v->vx =  v->vx>160.0f ? 160.0f : v->vx<-160.0f ? 160.0f : v->vx ;
	    v->vy += off->power->force.vy * d->dir_y*dis ;
	    v->vz += off->power->force.vz * d->dir_z*dis ;
	    if ( d->dir_z > 0.0f )
		v->vz =  v->vz>160.0f ? 160.0f : v->vz<-160.0f ? 160.0f : v->vz ;

	    /*変化した頂点の法線を計算する*/
	    for ( idx=d->n_poly, p=d->poly ; --idx>=0 ; p++ )
		if ( p->i0==i || p->i1==i || p->i2==i )
		    CalcNormalFromVertex( &p->nrm, p->v0,p->v1,p->v2 ) ;
	}
    }

    /*頂点のストリップに書き出し*/
    DG_RegistCommonVertex( c->verts, c->n_verts ) ;
    DG_RefineStripVertex( &d->v_anm, c->verts_index ) ; 


    n = SCRPAD_ADDR ;
    for ( i=d->n_poly ; --i>=0 ; n++ )
	ZeroVector( n ) ;

    /*各法線の合計を取る*/
    n = SCRPAD_ADDR ;
    for ( i=d->n_poly, p=d->poly ; --i>=0 ; p++ )
    {
	AddVector( &n[p->i0], &p->nrm ) ;
	AddVector( &n[p->i1], &p->nrm ) ;
	AddVector( &n[p->i2], &p->nrm ) ;
    }
    /*各法線の平均を取る*/
    for ( i=d->n_poly ; --i>=0 ; n++ )
	n->vx /= n->vw,	n->vy /= n->vw,	n->vz /= n->vw ;
    /*平均法線との内積よりスムーズするかどうか決める*/
    n = SCRPAD_ADDR ;
    for ( i=d->n_poly, p=d->poly ; --i>=0 ; p++ )
    {
	p->flag = 0 ;
	if ( n[p->i0].vx*p->nrm.vx + n[p->i0].vy*p->nrm.vy + n[p->i0].vz*p->nrm.vz < 0.68f )
	    p->flag |= 0x1 ;
	if ( n[p->i1].vx*p->nrm.vx + n[p->i1].vy*p->nrm.vy + n[p->i1].vz*p->nrm.vz < 0.68f )
	    p->flag |= 0x2 ;
	if ( n[p->i2].vx*p->nrm.vx + n[p->i2].vy*p->nrm.vy + n[p->i2].vz*p->nrm.vz < 0.68f )
	    p->flag |= 0x4 ;
    }

    /*ストリップにスムーズ（平均化）法線を入れる*/
    DG_RegistCommonNormal( SCRPAD_ADDR, d->cv2->n_verts ) ;
    DG_RefineStripNormal( &d->v_anm, d->cv2->verts_index ) ; 
    /*ストリップに非スムーズ法線を入れる*/
    for ( i=d->n_poly, p=d->poly ; --i>=0 ; p++ )
    {
	if ( p->flag & 1 )
	    p->n0->vx = p->nrm.vx*4096, p->n0->vy = p->nrm.vy*4096, p->n0->vz = p->nrm.vz*4096 ;
	if ( p->flag & 2 )
	    p->n1->vx = p->nrm.vx*4096, p->n1->vy = p->nrm.vy*4096, p->n1->vz = p->nrm.vz*4096 ;
	if ( p->flag & 4 )
	    p->n2->vx = p->nrm.vx*4096, p->n2->vy = p->nrm.vy*4096, p->n2->vz = p->nrm.vz*4096 ;
    }

    ShowDebugNormal( d ) ;
}


void BRK_LCK_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    DOOR    *d = (DOOR *)ptr ;
    static void (*weapon_act[])( TARGET *off, TARGET *def, DOOR *d ) = {
	BRK_LCK_None,
	BRK_LCK_WeaponGun,/*M9*/
	BRK_LCK_WeaponGun,/*USP*/
	BRK_LCK_WeaponGun,/*Socom*/
	BRK_LCK_WeaponGun,/*Famas*/
	BRK_LCK_WeaponGun,/*PSG-1*/
	BRK_LCK_WeaponGun,/*SPP1M*/
	BRK_LCK_WeaponGun,/*Nikita*/
	BRK_LCK_WeaponGun,/*Stinger*/

	BRK_LCK_WeaponGun,/*M9*/
	BRK_LCK_WeaponGun,/*USP*/
	BRK_LCK_WeaponGun,/*Socom*/
	BRK_LCK_WeaponGun,/*Famas*/
	BRK_LCK_WeaponGun,/*PSG-1*/
	BRK_LCK_WeaponGun,/*SPP1M*/
	BRK_LCK_WeaponGun,/*Nikita*/
	BRK_LCK_WeaponGun,/*Stinger*/
	BRK_LCK_WeaponGun,/*Stinger*/
    } ;

    if ( def->damaged & TARGET_POWER )
    {
	if ( off->weapon_type < 17 )
	    (weapon_act[off->weapon_type])( off, def, d ) ;

//printf( "weptyp = %d\n", off->weapon_type ) ;
	/* clear damage */
	GM_ClearTargetDamage( def ) ;
    }
}
