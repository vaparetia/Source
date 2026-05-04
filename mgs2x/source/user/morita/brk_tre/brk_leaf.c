//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tre_act.c
   植物揺れ アクト

   1999/12/17 T. Morita
   $Id: brk_leaf.c,v 1.1.1.3 2002/11/19 11:45:48 Yoshizawa1 Exp $
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

#include "../brk_utl/brk_utl.x"
#include "../include/util.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct work_t
{
    GV_ACT    actor   ;

    FVECTOR   pos     ;
    FVECTOR   pos_v   ;
    short     rot_x   ;
    short     rot_y   ;
    short     rot_vx  ;
    short     rot_vy  ;

    short     rot_dy  ; /* 目標Y回転 */
	short     rot_claced ;
    short    *life    ;

    DG_COMDL     *leaf ;
    DG_COMDL_POS *cpos ;

    HZX_GROUP_ID  hzx ;
} Work ;

#define BRK_HZX_S_SPHERE 1000
#define BRK_HZX_SPHERE   200


static inline void TransMatrix( FMATRIX *mtx, FVECTOR *pos  )
{
    mtx->m[3][0] += pos->vx ;
    mtx->m[3][1] += pos->vy ;
    mtx->m[3][2] += pos->vz ;
}
static inline void RotateMatrixXY( FMATRIX *out, FMATRIX *in, short rot_x, short rot_y  )
{
    int r ;
    r = rot_x & 0x0fff ;
    _sceVu0RotMatrixZ( out, in , (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    r = rot_y & 0x0fff ;
    _sceVu0RotMatrixY( out, out, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
}



int BRK_LEF_HzdSegLineCheck( HZX_SEG *seg, FVECTOR *p1, FVECTOR *p2 )
{
    FVECTOR p ;
    float sx, sz, tx, tz ;
    float r, s, rd ;

    _sceVu0SubVector( &p, p2, p1 ) ;
    sx = seg->p2.x - seg->p1.x ; sz = seg->p2.z - seg->p1.z ;
    if ( (rd = sx*p.vz - sz*p.vx) == 0.0f )                                
        return 0 ;
    tz = seg->p1.z - p1->vz ;
    tx = seg->p1.x - p1->vx ;
    r = (tz*p.vx - tx*p.vz)/rd ;
    if ( r > 1.0f || r < 0.0f )               
        return 0 ;
    s = (tz*sx - tx*sz)/rd ;
    if ( s > 1.0f || s < 0.0f )
        return  0 ;
    _sceVu0ScaleVector( &p, &p, r ) ;
    _sceVu0AddVector( &p, &p, p1 ) ;
    if ( seg->p1.y > p.vy || seg->p1.y+seg->p1.h < p.vy ||
         seg->p2.y > p.vy || seg->p1.y+seg->p2.h < p.vy )
        return  0 ;
    return 1 ;
}

float BRK_LEF_HzdSegReactVector( HZX_SEG *seg, FVECTOR *p  )
{
    float dx, dz, f ;

    dx =  seg->p2.x - seg->p1.x ;
    dz =  seg->p2.z - seg->p1.z ;
    f = 2*( dz*p->vx - dx*p->vz )/(dx*dx + dz*dz) ;
    p->vx -= dz * f ;
    p->vz += dx * f ;

    return atan2f( dz,-dx )*2048.0f/M_PI ;
}

int BRK_LEF_HzdCheck( Work *work, FVECTOR *min, FVECTOR *bounce, float sphere )
{
    int     atr[2] ;
    float   flr[2] ;
    HZX_SEG seg[2] ;
    int     flag = 0 ;
    FVECTOR cur ;
    FVECTOR new ;

    _sceVu0AddVector( &new, &work->pos, &work->pos_v ) ;
    _sceVu0AddVector( &new, &new, min ) ;
    _sceVu0AddVector( &cur, &work->pos, min ) ;
    if ( HZX_NearHazardCheck( work->hzx, &cur, BRK_HZX_S_SPHERE,
							  HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
							  HZX_SEG_RECOIL_TYPE, (int)sphere ) ) {
        HZX_GetNearHazard( seg, atr ) ;
        if ( BRK_LEF_HzdSegLineCheck( seg, &cur, &new ) || 1 ) {
			if ( work->rot_claced == 0 ) {
				work->rot_dy = (short)BRK_LEF_HzdSegReactVector( seg, &work->pos_v ) ;
				work->rot_dy = (work->rot_dy + 1024-256 + (irnd()&511) ) & 4095 ;
				work->rot_dy -= 2048<work->rot_dy ? 4096 : 0 ;
				work->rot_claced = 1 ;
			}
			flag |= 1 ;
		}
    }

    if ( HZX_LevelHazardCheck( work->hzx, &work->pos,
							   HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
							   HZX_FLOOR_RECOIL_TYPE ) ) {
        HZX_GetLevelHeight( flr ) ;
        if ( new.vy < (flr[0] -= min->vy) ) {
            work->pos.vy = flr[0] ;
			work->pos_v.vy *= bounce->vy ;
			flag |= 2 ;
		}
    }

    return flag ;
}

static void StickPartsOnFloor( DG_COMDL *c, FVECTOR *min, FMATRIX *mtx )
{
	static FVECTOR bottom = { 0.0f, -248.0f, 0.0f, 0.0f } ;

	_sceVu0ApplyMatrix( min, mtx, &bottom ) ;
	if ( min->vy > -BRK_HZX_SPHERE )
	  min->vy = -BRK_HZX_SPHERE ;
}

static void ActLeaf( Work *work, int next, int gravity )
{
    static FVECTOR Bounce = { 0, 0, 0, 0 } ;
    FVECTOR min ;

    work->rot_y += work->rot_vy ;
	work->rot_y &= 4095 ;
    RotateMatrixXY( &work->cpos->world, &DG_UnitMatrix, work->rot_x, work->rot_y ) ;
    work->pos_v.vx *= 0.6f ;
    work->pos_v.vy = (float)gravity  ;
    work->pos_v.vz *= 0.6f ;
    StickPartsOnFloor( work->leaf, &min, &work->cpos->world ) ;
    if ( BRK_LEF_HzdCheck( work, &min, &Bounce, BRK_HZX_SPHERE ) & 2 )
	  *work->life = next ;
    _sceVu0AddVector( &work->pos, &work->pos, &work->pos_v ) ;
    TransMatrix( &work->cpos->world, &work->pos ) ;
}

#define BRK_ROT_R   32
static void Act( Work *work )
{
    int i ;

    BRK_UTL_ComdlColor2( work->cpos, Y ) ;

    switch( *work->life ) {
	  case  5:
		work->rot_vy  = work->rot_dy - work->rot_y ;
        work->rot_vy &= 4095 ;
        work->rot_vy -= 2048<work->rot_vy ? 4096 : 0 ;
		for( i=3 ; --i>=0 && work->rot_vy/2 ; work->rot_vy/=2 ) ;

		ActLeaf( work, 5, -200 ) ;
		if ( !(work->rot_x = work->rot_x * (BRK_ROT_R-1) / BRK_ROT_R) )
		  *work->life = 0 ;
		break ;

	  case  3:
		work->rot_x += work->rot_vx -= (work->rot_vx+work->rot_x)/32 ;
		ActLeaf( work, 5, -6 ) ;
		break ;

	  case  2:
		if ( work->rot_vx/128 )
		  work->rot_vx = work->rot_vx * (BRK_ROT_R-1) / BRK_ROT_R ;
		work->rot_x += work->rot_vx ;
		if ( !(work->rot_vx/128) && !(work->rot_x%4096/1024) )
		  *work->life = 3, work->rot_x %= 4096 ;
		ActLeaf( work, 5, -6 ) ;
		break ;

	  case  0:
	  case  1:
		GV_DestroyActor( work ) ;
    }
}


static int GetResources( Work *work, DG_COMDL *leaf, DG_COMDL_POS *pos,
			 FVECTOR *i_pos, FVECTOR *i_dir, short *life,
			 HZX_GROUP_ID  hzx )
{
    work->hzx   =  hzx   ;
    work->life  =  life  ;

    work->pos      = *i_pos ;
    work->pos_v.vx = i_dir->vx + frnd()*100.0f ;
    work->pos_v.vy = i_dir->vy + frnd()*100.0f ;
    work->pos_v.vz = i_dir->vz + frnd()*100.0f ;

    GM_SeSetMode( SD_A_LEAF01, &work->pos, GM_SEMODE_NORMAL ) ;

    *work->life =  3     ;
    work->cpos  =  pos   ;
    work->leaf  =  leaf  ;
    _sceVu0ScaleVector( &work->pos_v, &work->pos_v, 0.5f ) ;
    work->rot_vx   = (irnd()&128)+100 ;
    work->rot_vy   = (irnd()&128)-63  ;
    work->rot_x    = 0   ;
    work->rot_y    = irnd()&4095 ;
    work->rot_dy   = (irnd()&512)-256 + work->rot_y ;
	work->rot_dy  &= 4095 ;
	work->rot_dy  -= 2048<work->rot_dy ? 4096 : 0 ;
    work->rot_claced = 0 ;

    work->cpos->color.vw = 128 ;

    return 0 ;
}

void *NewLeafFly( DG_COMDL *leaf, DG_COMDL_POS *pos,
		  FVECTOR *i_pos, FVECTOR *i_dir, short *life, HZX_GROUP_ID hzx )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, NULL ) ;
        if( GetResources( work, leaf, pos, i_pos, i_dir, life, hzx ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
