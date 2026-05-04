//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_usp.c 
   オルガ 武器関数(USPカートリッジ)

   1999/12/18 T.Morita
   $Id: orga_amo.c,v 1.1.1.3 2002/11/19 11:46:27 Yoshizawa1 Exp $
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


#include "orga_wep.h"
#include "../include/orga_se.h"

#define ORGA_AMO_GRAVITY  3
#define ORGA_AMO_BOUNCE   0.58f
#define ORGA_AMO_SPHERE   40
#define ORGA_AMO_R_VEL    0.8f

static FVECTOR BounceVector = { -1.0, -ORGA_AMO_BOUNCE, -1.0 } ;


typedef	struct work_uspamo_t Work ;
struct work_uspamo_t
{
    GV_ACT	 actor  ;
    HZX_GROUP_ID hzx    ;

    DG_OBJS     *weapon ;
    DG_OBJS     *drop   ;
    FVECTOR      pos    ;
    FVECTOR      vel    ;
    short        rot    ;
    short        rot_v  ;

    FMATRIX     *right  ;
    FMATRIX     *left   ;

    FMATRIX      hand   ;/* 落す瞬間の右手のマトリックス */

    short       *flag ;
    int          se_on  ;
} ;


static void Die( Work *work )
{
    if ( work->weapon )
	DG_DequeueObjs( work->weapon ), DG_FreeObjs( work->weapon ) ;
    if ( work->drop   )
	DG_DequeueObjs( work->drop   ), DG_FreeObjs( work->drop   ) ;
}

static void Act( Work *work )
{
    static FVECTOR l_hnd_ofs = {-87.5f,-120.0f, 0.0f, 1.0f } ;
    static FVECTOR r_hnd_ofs = { 17.5f,-110.0f,-1.2f, 9.0f/*pos.vwに入れる値（スライドするフレーム数）*/ } ;
    static FMATRIX mtx = { { { 0.0f, 1.0f, 0.0f, 0.0f },
			     { 1.0f, 0.0f, 0.0f, 0.0f },
			     { 0.0f, 0.0f, 1.0f, 0.0f },
			     { 0.0f, 0.0f, 0.0f, 1.0f } } } ;
    FVECTOR v ;

    _sceVu0RotMatrixY( &work->weapon->world, &DG_UnitMatrix, -(float)M_PI*0.5f ) ;
    _sceVu0CopyVector( (FVECTOR*)&work->weapon->world.m[W], &l_hnd_ofs ) ;
    _sceVu0MulMatrix( &work->weapon->world, work->left, &work->weapon->world ) ;

    switch( *work->flag )
    {
    case -4:
	_sceVu0CopyVector( &work->pos, &r_hnd_ofs ) ;
	work->rot    = 0  ;
	work->rot_v  = 30 ;
	work->vel.vy = 0    ;
	*work->flag  = 0    ;
	break ;
    case -3:
	DG_VisibleObjs( work->weapon ) ;
	*work->flag = 0 ;
	break ;
    case -2:
	DG_InvisibleObjs( work->weapon ) ;
	*work->flag = 0 ;
	break ;
    case -1:
	GV_DestroyActor( work ) ;
    }

    switch( (int)work->pos.vw )
    {
    case 0:
	break ;
    case 1:
	work->vel.vy -= ORGA_AMO_GRAVITY ;  /* 重力加速度 */
	RotateMatrixY( &work->drop->world, &DG_UnitMatrix, work->rot+=work->rot_v ) ;
	_sceVu0MulMatrix( &work->drop->world, &work->hand, &work->drop->world ) ;
	TransMatrix( &work->drop->world, &work->pos ) ;

	if ( ORG_HzdCheck( work->hzx, (FVECTOR*)&work->drop->world.m[W], &work->vel,
			   -ORGA_AMO_SPHERE, &BounceVector, ORGA_AMO_SPHERE ) & 2 )
	{
	    _sceVu0ScaleVector( &work->vel, &work->vel, ORGA_AMO_R_VEL ) ;/* 摩擦処理 */
	    if ( work->vel.vy > 4.0f ) /* 跳ねの処理 */
	    {
		if ( work->se_on-- )
		    GM_SeSetMode( ORGA_SE_AMO_BOUND, &work->pos, GM_SEMODE_NORMAL ) ;
		work->rot_v = -work->rot/8 ;
	    }
	    if ( !(int)(work->vel.vx/2) && !(int)(work->vel.vy/4) && !(int)(work->vel.vz/2) )
		work->pos.vw = 0.0f, work->se_on = 3 ; /* 終了 */
	}
	TransMatrix( &work->drop->world, &work->vel ) ;
	_sceVu0AddVector( &work->pos, &work->pos, &work->vel ) ;/* 位置更新 */

	_sceVu0SubVector( &v, (FVECTOR*)&mtx.m[X], (FVECTOR*)&work->hand.m[X] ) ;
	_sceVu0ScaleVector( &v, &v, 0.125f ) ;
	_sceVu0AddVector( (FVECTOR*)&work->hand.m[X], (FVECTOR*)&work->hand.m[X], &v ) ;
	_sceVu0SubVector( &v, (FVECTOR*)&mtx.m[Y], (FVECTOR*)&work->hand.m[Y] ) ;
	_sceVu0ScaleVector( &v, &v, 0.125f ) ;
	_sceVu0AddVector( (FVECTOR*)&work->hand.m[Y], (FVECTOR*)&work->hand.m[Y], &v ) ;
	_sceVu0SubVector( &v, (FVECTOR*)&mtx.m[Z], (FVECTOR*)&work->hand.m[Z] ) ;
	_sceVu0ScaleVector( &v, &v, 0.125f ) ;
	_sceVu0AddVector( (FVECTOR*)&work->hand.m[Z], (FVECTOR*)&work->hand.m[Z], &v ) ;
	break ;

    default:/*銃の柄の部分をスライドしている*/
	work->drop->world = *work->right ;
	work->vel.vy -= ORGA_AMO_GRAVITY ;  /* 重力加速度 */
	work->pos.vy -= work->vel.vy*0.28f ;
	work->pos.vz += work->vel.vy ;
	ApplyMatrixXYZ( &v, work->right, &work->pos ) ;
	TransMatrix( &work->drop->world, &v ) ;
	if ( (work->pos.vw -= 1.0f) == 1.0f ) {
	    _sceVu0CopyVector( &work->pos, &DG_ZeroVector ) ;
		_sceVu0CopyMatrix( &work->hand, &work->drop->world ) ;
	}
	break ;
    }
}

static int GetResources( Work *work, FMATRIX *right, FMATRIX *left, short *flag, int model )
{
    DG_DEF  *def ;

	def = GV_GetCache( GV_CacheID( model, 'k' ) ) ;
    if ( !(work->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	return -1 ;
    DG_QueueObjs( work->weapon ) ;
    if ( !(work->drop   = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )) )
	return -1 ;
    DG_QueueObjs( work->drop   ) ;
    work->left  = left  ;
    work->right = right ;
    work->flag  = flag  ;

    work->se_on = 2 ;

    work->hzx = GM_GetHzxGroupID( GM_CurrentMap ) ;

    return 0 ;
}

void *NewOrgaWeaponUSPAMO( FMATRIX *right, FMATRIX *left, short *flag, int model )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, right, left, flag, model ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
