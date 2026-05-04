//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_lnr.c 
   フォーチュン 武器関数(LNR)

   2000/12/18 T.Morita
   $Id: fort_lnr.c,v 1.1.1.3 2002/11/19 11:46:19 Yoshizawa1 Exp $
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
// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"


#include "fort_wep.h"
#include "../include/fort_se.h"

typedef	struct work_lnr_t Work ;
struct work_lnr_t
{
    GV_ACT_EX actor   ;

    FMATRIX   offset  ;

    FMATRIX   l_blow[2] ;
    FMATRIX   l_main[2] ;

    DG_OBJS  *weapon  ;
    FMATRIX  *world   ;
	    
    FVECTOR  *trigger ;
    short    *bullet  ;
    int       damage  ;
    int       where   ;
} ;

enum
{
    R=0, G, B, A, RGBA
};
enum
{
    FRT_LNR_HIDE=-3,
    FRT_LNR_SHOW,
    FRT_LNR_IDLE,
    FRT_LNR_SHOOT,
} ;

extern FVECTOR FRT_LNR_NozzleOfst ;

#define FORT_LNR_FRAMES  10
#define FORT_LNR_SCALE   (250/FORT_LNR_FRAMES)
#define SLING_WIDTH 40.0f


/* ブローバック */
static inline void CalcBlowBack( Work * work )
{
    float   blowback ;

    blowback = -200.0f ;
    if ( work->trigger->vw >= 1.0f )
    {
	blowback += (FORT_LNR_FRAMES - work->trigger->vw) * FORT_LNR_SCALE ;
	if ( (work->trigger->vw += 1.0f) > FORT_LNR_FRAMES )
	    work->trigger->vw = 0.0f ;
    }
    ScaleAndAddVector( (FVECTOR*)work->weapon->objs[1].world.m[W],
		       (FVECTOR*)work->weapon->objs[1].world.m[Y],
		       blowback ) ;
}

/* 光源計算 */
static inline void CalcLightMatrix( Work *work )
{
    BRK_UTL_GetLightMatrix( (FVECTOR *)work->weapon->objs[0].world.m[W],
			    work->l_main, 1.0f, work->where ) ;

#if 0
    work->l_blow[0].m[0][X] = 
    work->l_blow[0].m[0][Y] =
    work->l_blow[0].m[0][Z] =
    work->l_blow[0].m[0][W] = 1.0f ;
    _sceVu0CopyVector( (FVECTOR *)work->l_blow[0].m[1], (FVECTOR *)work->l_blow[0].m[0] ) ;
    _sceVu0CopyVector( (FVECTOR *)work->l_blow[0].m[2], (FVECTOR *)work->l_blow[0].m[0] ) ;

    work->l_blow[1].m[3][A] = 
    work->l_blow[1].m[3][R] = 
    work->l_blow[1].m[3][B] =
    work->l_blow[1].m[3][G] = 255.0f * sin( (GM_StagePlayTime & 0xff)*(float)M_PI/128.0f ) ;
    work->l_blow[1].m[3][R] /= 16.0f ;
    _sceVu0CopyVector( (FVECTOR *)work->l_blow[1].m[0], (FVECTOR *)work->l_blow[1].m[3] ) ;
    _sceVu0CopyVector( (FVECTOR *)work->l_blow[1].m[1], (FVECTOR *)work->l_blow[1].m[3] ) ;
    _sceVu0CopyVector( (FVECTOR *)work->l_blow[1].m[2], (FVECTOR *)work->l_blow[1].m[3] ) ;
#endif

}

/* OBJのマトリックス計算 */
static inline void CalcObjsMatrix( Work *work )
{
    int     i ;
    DG_OBJ *o ;
    FVECTOR v ;

    _sceVu0MulMatrix( &work->weapon->world, work->world, &work->offset ) ;
    for ( i=work->weapon->def->n_models, o=work->weapon->objs ; --i>=0 ; o++ )
    {
	o->world = work->weapon->world ;
	v.vx = o->model->tx ;
	v.vy = o->model->ty ;
	v.vz = o->model->tz ;
	v.vw = 0.0f ;
	_sceVu0ApplyMatrix( &v, &work->weapon->world, &v ) ;
	_sceVu0AddVector( (FVECTOR *)o->world.m[W], (FVECTOR *)o->world.m[W], &v ) ;
    }
}

static void Act( Work *work )
{
    extern void *NewFortBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
    extern void *NewFortBullet2( FVECTOR*, FVECTOR*, u_int, u_int, u_int, u_int, u_int, int ) ;
    extern void  AN_MazzleUSP( FMATRIX *, int ) ;
    int     se ;
    int pan;
    float bp_angle;

    CalcObjsMatrix( work ) ;
    switch ( (int)work->trigger->vw )
    {
    case -3:   /*表示*/
	DG_VisibleObjs( work->weapon ) ;
	work->trigger->vw = 0.0f ;
	break ;
    case -2:   /*非表示*/
	DG_InvisibleObjs( work->weapon ) ;
	work->trigger->vw = 0.0f ;
	break ;
    case -1:   /*終了*/
	GV_DestroyActor( work ) ;
	break ;

    case 1:
	if ( --(*work->bullet) >= 0 )
	{
#if 1
	    FMATRIX dir = DG_UnitMatrix ;
	    int damage ;

	    damage = work->damage ;
	    if ( damage > GM_Vitality )
		if ( GM_Vitality > GM_VitalityMax/5 )
		    damage = GM_Vitality - 3 ;

	    _sceVu0ApplyMatrix( (FVECTOR*)dir.m[W], work->world, &FRT_LNR_NozzleOfst ) ;
	    _sceVu0SubVector( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[W], work->trigger ) ;
	    _sceVu0Normalize( (FVECTOR*)dir.m[Y], (FVECTOR*)dir.m[Y] ) ;
	    NewFortBullet( &dir, BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE, PLAYER_SIDE,
			   25, damage, 15000, 800*TIME_BASE/5, WP_Nikita ) ;
	    printf( "Fort: NewFortBullet(%.0f %.0f %.0f)\n",
		    work->trigger->vx,work->trigger->vy,work->trigger->vz ) ;

#else
	    NewFortBullet2( (FVECTOR*)work->world->m[W], work->trigger,
			    BUL_TYPE_SPARK|BUL_TYPE_SCAR, PLAYER_SIDE,
			    25, work->damage, 1000*TIME_BASE/5, WP_Nikita ) ;
#endif
	    AN_MazzleUSP( &dir, 0 ) ;
	    se = SD_E_LGUNFIRE ;
	}
	else
	    se = SD_W_EMPTY01 ;
	work->trigger->vw += 1.0f ;
   pan = GM_SeGetPan( (FVECTOR *)work->world->m[W], GM_SEMODE_BOMB, &bp_angle );
	GM_SeSet3D( pan, GM_MAX_VOL, se, bp_angle ) ;
    default:
	break ;
    }

    CalcBlowBack( work ) ;
    CalcLightMatrix( work ) ;
}


static void Die( Work *work )
{
    if ( work->weapon )
	DG_DequeueObjs( work->weapon ), DG_FreeObjs( work->weapon ) ;
}


static int GetResources( Work *work,
			 int where, DG_OBJS *body,
			 FVECTOR *trigger, short *bullet, int damage )
{
    DG_DEF  *def = GV_GetCache( GV_CacheID( GV_StrCode( "lnr" ), 'k' ) ) ;

    if ( !(work->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	return -1 ;
    DG_QueueObjs( work->weapon ) ;
    work->trigger = trigger ;
    work->bullet  = bullet  ;
    work->damage  = damage  ;
    work->offset  = DG_UnitMatrix ;
    work->world   = &body->objs[HUMAN21_MIGI_TE].world ;
    work->where   = where   ;

    /* ライトマトリックスを指定 */
    work->weapon->light         = work->l_main ;
    work->weapon->objs[0].light = work->l_main ;
    work->weapon->objs[1].light = work->l_main ;/*旧l_blow*/

    /*エフェクト*/
    {
	void *NewFortElec( DG_OBJS *objs, int cv2_id, int joint ) ;
	//NewFortElec( work->weapon, GV_StrCode( "lnr" ), 1 ) ;
    }

    /* スリング */
    {
	void *NewFortJointParts( int id, int where,
				 FMATRIX *world1, FVECTOR *pos1,
				 FMATRIX *world2, FVECTOR *pos2  ) ;
	FVECTOR FortShoulder[2]  = { {109.3f, 208.4f, 49.2f, 1.0f },
				     {134.0f, 205.0f, 43.0f, 1.0f } } ;
	FVECTOR SlingLnrFront[2] = { { 8.5f, -1010.0f, 244.0f, 1.0f },
				     {33.5f, -1010.0f, 244.0f, 1.0f } } ;
	FVECTOR FortWaist[2]     = { {-137.7f,-39.8f,29.8f, 1.0f },
				     {-119.9f,-57.1f,28.4f, 1.0f } } ;
	FVECTOR SlingLnrRear[2]  = { { 55.0f, 48.5f, 132.0f, 1.0f },
				     { 55.0f, 73.5f, 132.0f, 1.0f } } ;

	GV_SetActorChild( work,
			  NewFortJointParts( GV_StrCode( "for_lnr_sling" ), where,
					     &work->weapon->world, SlingLnrFront,
					     &body->objs[HUMAN21_MUNE].world, FortShoulder ) ) ;
	GV_SetActorChild( work,
			  NewFortJointParts( GV_StrCode( "for_lnr_sling" ), where,
					     &work->weapon->world, SlingLnrRear,
					     &body->objs[HUMAN21_MUNE].world, FortWaist ) ) ;
    }

    /* レーザーサイト */
    {
	extern void *NewFortLaserSight( FMATRIX *world, FVECTOR *aim, FVECTOR *offset ) ;
	static FVECTOR LaserPointer = { -24.0f, -1129.501831f, 75.0f, 1.0f } ;

	GV_SetActorChild( work,
			  NewFortLaserSight( &work->weapon->world, trigger, &LaserPointer ) ) ;
    }

	{
		extern void *NewLinerGunInitEffect( FMATRIX *world );
		GV_SetActorChild( work,
			NewLinerGunInitEffect( &work->weapon->objs[1].world )    );
	}

    return 0 ;
}

void *NewFortWeaponLNR( int where, DG_OBJS *body,
			FVECTOR *trigger, short *bullet, int damage )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
        if( GetResources( work, where, body, trigger, bullet, damage ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
