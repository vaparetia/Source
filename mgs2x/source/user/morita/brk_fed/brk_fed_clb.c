/*
   brk_fed_clb.c
   雑誌壊れ コールバック関数

   2000/06/20 T. Morita
   $Id: brk_fed_clb.c,v 1.1.1.3 2002/11/19 11:45:27 Yoshizawa1 Exp $
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

#include "brk_fedralex.h"

#define BRK_FED_SCAR_W 50.0f

static void SetScar( FEDRAL *p, TARGET *def )
{
    FMATRIX inv ;
    FVECTOR hit ;
    FVECTOR min = { p->objs->def->lx, p->objs->def->ly, p->objs->def->lz } ;
    FVECTOR max = { p->objs->def->ux, p->objs->def->uy, p->objs->def->uz } ;
    int      i ;
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR        *p0, *p1 ;

    _sceVu0AddVector( &max, &def->offset, &def->size ) ;
    _sceVu0SubVector( &min, &def->offset, &def->size ) ;

    p0 = p1 = p->scar->pos[0]   ;
    u0 = u1 = p->scar->uvrgb[0] ;
    if ( ++p->n_scar >= BRK_FED_N_SCAR )
	p->n_scar = 0 ;
    p0 += p->n_scar*4 ;
    u0 += p->n_scar*4 ;
    if ( ++p->n_scar >= BRK_FED_N_SCAR )
	p->n_scar = 0 ;
    p1 += p->n_scar*4 ;
    u1 += p->n_scar*4 ;

    /*ローカル座標系での着弾位置と着弾方向を求める*/
    _sceVu0CopyVector( &hit, &def->hit ) ;
    hit.vw = 1.0f ;
    _sceVu0InversMatrix( &inv, &p->objs->world ) ;
    _sceVu0ApplyMatrix( &hit, &inv, &hit ) ;

    /* 取りあえずプリミティブの形を作る 表 */
    p0[0].vz = p0[2].vz = hit.vz - BRK_FED_SCAR_W ;
    p0[1].vz = p0[3].vz = hit.vz + BRK_FED_SCAR_W ;
    p0[0].vy = p0[1].vy = hit.vy - BRK_FED_SCAR_W ;
    p0[2].vy = p0[3].vy = hit.vy + BRK_FED_SCAR_W ;
    p0[0].vx = p0[1].vx = p0[2].vx = p0[3].vx = 0*def->offset.vx-14 ;
    p0[0].vw = p0[1].vw = p0[2].vw = p0[3].vw = 1.0f ;
    /* 取りあえずプリミティブの形を作る 裏 カリングを考えて逆回しにする */
    p1[0].vz = p1[1].vz = hit.vz - BRK_FED_SCAR_W ;
    p1[2].vz = p1[3].vz = hit.vz + BRK_FED_SCAR_W ;
    p1[0].vy = p1[2].vy = hit.vy - BRK_FED_SCAR_W ;
    p1[1].vy = p1[3].vy = hit.vy + BRK_FED_SCAR_W ;
    p1[0].vx = p1[1].vx = p1[2].vx = p1[3].vx = 0*def->offset.vx+14 ;
    p1[0].vw = p1[1].vw = p1[2].vw = p1[3].vw = 1.0f ;

    for( i=4 ; --i>=0 ; )
    {
	u0[i].a = u1[i].a = 128 ;
	p0[i].vx = (p0[i].vx>max.vx ? max.vx :
		    p0[i].vx<min.vx ? min.vx : p0[i].vx ) ;
	p0[i].vy = (p0[i].vy>max.vy ? max.vy :
		    p0[i].vy<min.vy ? min.vy : p0[i].vy ) ;
	p0[i].vz = (p0[i].vz>max.vz ? max.vz :
		    p0[i].vz<min.vz ? min.vz : p0[i].vz ) ;

	p1[i].vx = (p1[i].vx>max.vx ? max.vx :
		    p1[i].vx<min.vx ? min.vx : p1[i].vx ) ;
	p1[i].vy = (p1[i].vy>max.vy ? max.vy :
		    p1[i].vy<min.vy ? min.vy : p1[i].vy ) ;
	p1[i].vz = (p1[i].vz>max.vz ? max.vz :
		    p1[i].vz<min.vz ? min.vz : p1[i].vz ) ;
    }
}



void BRK_FED_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    FEDRAL *p = (FEDRAL *)ptr ;
    static FVECTOR pop = { 0.0f, 30.0f, 0.0f, 0.0f } ;
    Work   *work = p->work ;
    FVECTOR v ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    SetScar( p, def ) ;

	if ( off->weapon_type & (long64)(WP_BLOW|WP_BLAST|WP_STUNGRENADE) )
	{
	    _sceVu0SubVector( &v, &def->center, &off->center ) ;

	    if ( (int)(v.vx / off->size.vx) ||
		 (int)(v.vy / off->size.vy) ||
		 (int)(v.vz / off->size.vz) ||
		 (int)(v.vx / 3000.0f) || (int)(v.vy / 3000.0f) || (int)(v.vz / 3000.0f) )
		return ;

	    /* 振動 */
	    BRK_UTL_PK_Vibrate( &def->hit, off ) ;
	}
	else if ( !(off->weapon_type & (WP_BULLET|WP_M92)) )
	    return ;

	BRK_FED_StartActPiece( p->work, 5, &p->mov.pos, &pop, 50.0f, 40.0f ) ;
	NewTs_Min_Fog( &p->mov.pos ) ;

	/* 敵兵見つかり用 */
	if ( work->ene_find.type & (EF_TYPE_FOUND|EF_TYPE_NO_FIND) )
	    GM_SetNoise( NOISE_S, &p->mov.pos, work->where ) ;

	/* SEを鳴らす */
	GM_SeSetMode( SD_A_BOOKBR01, &p->mov.pos, GM_SEMODE_BOMB ) ;

	/* 挙動の初期化 */
	p->mov.pos_v.vy = 90 ;
	p->mov.rot_vx = (irnd()&0x00e0)-128 ;
	p->mov.rot_vy = (irnd()&0x00e0)-128 ;

	/* 当たり判定ありのアクト */
	p->flag = 4 ; /* collision immeadiately */

	/* 雑誌のアクト全体をアクティブにする */
	p->work->n_fedral &= ~BRK_FED_INACTIVE ;
    }
}
