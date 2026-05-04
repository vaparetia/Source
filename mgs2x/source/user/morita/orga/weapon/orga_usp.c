//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_usp.c 
   オルガ 武器関数(USP)

   1999/12/18 T.Morita
   $Id: orga_usp.c,v 1.1.1.3 2002/11/19 11:46:28 Yoshizawa1 Exp $
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

#define USP_WEAP 123600  /*GV_StrCode( "usp" )    */
#define DSE_WEAP 106181  /*GV_StrCode( "dse" )    */

typedef	struct work_usp_t Work ;
struct work_usp_t
{
    GV_ACT   actor   ;
    DG_OBJS *weapon  ;
    FMATRIX *world   ;
    FMATRIX *right   ;
    FMATRIX *left    ;
    FMATRIX *waist   ;
	    
    FVECTOR *trigger ;
    FMATRIX  offset  ;
    short   *bullet  ;
    int      damage  ;
	int      model   ;
} ;

enum
{
    ORG_USP_CHNG_LEFT = -5,
    ORG_USP_CHNG_RIGHT,
    ORG_USP_HIDE,
    ORG_USP_SHOW,
    ORG_USP_IDLE,
    ORG_USP_SHOOT,
} ;


#define ORGA_USP_FRAMES  7
#define ORGA_USP_SCALE   7

static void Die( Work *work )
{
    if ( work->weapon )
	DG_DequeueObjs( work->weapon ), DG_FreeObjs( work->weapon ) ;
}

static void Act( Work *work )
{
    extern void *NewOrgaBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
    extern void *NewOrgaBullet2( FVECTOR*, FVECTOR*, u_int, u_int, u_int, u_int, u_int, int ) ;
    extern void  AN_MazzleUSP( FMATRIX *, int ) ;
	extern void AN_MazzleDSE(FMATRIX *world, int  silence) ;  
    extern void  AN_CartridgeUSP_E( FMATRIX *, OBJECT *, CONTROL * ) ;
    int     se, i ;
    FVECTOR v ;
    DG_OBJ *o ;

    _sceVu0MulMatrix( &work->weapon->world, work->world, &work->offset ) ;
    for ( i=work->weapon->def->n_models, o=work->weapon->objs ; --i>=0 ; o++ )
    {
	o->world = work->weapon->world ;
	v.vx = o->model->tx ; v.vy = o->model->ty ; v.vz = o->model->tz ; v.vw = 0.0f ;
	_sceVu0ApplyMatrix( &v, &work->weapon->world, &v ) ;
	_sceVu0AddVector( (FVECTOR*)&o->world.m[W], (FVECTOR*)&o->world.m[W], &v ) ;
    }

    switch ( (int)work->trigger->vw )
    {
    case -2:   /*非表示*/
	DG_InvisibleObjs( work->weapon ) ;
	work->trigger->vw = 0.0f ;
	break ;
    case -1:   /*終了*/
	GV_DestroyActor( work ) ;
	break ;
    case -3:   /*表示*/
	DG_VisibleObjs( work->weapon ) ;
	work->trigger->vw = 0.0f ;
	break ;
    case -4:   /*右手に表示*/
	work->offset = DG_UnitMatrix ;
	work->world  = work->right ;
	work->trigger->vw = 0.0f ;
	break ;
    case -5:   /*左手に表示*/
	work->offset = DG_UnitMatrix ;
	work->offset.m[W][X] = -40.0f ;
	work->world = work->left ;
	work->trigger->vw = 0.0f ;
	break ;
    case -6:   /*腰のオフセットを計算*/
	_sceVu0InversMatrix( &work->offset, work->waist ) ;
	_sceVu0MulMatrix( &work->offset, &work->offset, &work->weapon->world ) ;
	work->offset.m[W][W] = 1.0f ;
	work->trigger->vw = -7.0f ;
	work->world = work->waist ;/*腰に銃を固定させる*/
	break ;
    case -7:   /*腰に表示*/
	/* 回転を止めるため */
	break ;
    case -8:
	NewOrgaWeaponBarriar( 0, (FVECTOR *)&work->weapon->world.m[W] ) ;/* 強制弾 射出 */
	if ( work->model==USP_WEAP ) {
		AN_MazzleUSP( work->world, 0 ) ;
	} else {
		AN_MazzleDSE( work->world, 0 ) ;
	}
	work->trigger->vw = 2.0f ;
	break ;
    case 1:
	if ( --(*work->bullet) >= 0 )
	{
	    static FVECTOR ofst_usp = { -24.0f, -296.501831f, 75.0f, 1.0f } ;
	    static FVECTOR ofst_dse = { -24.0f, -323.0f, 75.0f, 1.0f } ;
	    FVECTOR trgt_pos = { frnd()*100.0f, 0.0f, frnd()*100.0f, 0.0f } ;
	    FMATRIX dir = DG_UnitMatrix ;

	    _sceVu0ApplyMatrix( (FVECTOR*)&dir.m[W], work->world,
						   work->model==USP_WEAP ? &ofst_usp : &ofst_dse ) ;
	    if ( GM_PlayerStatus & PLAYER_SQUAT ) {
			_sceVu0CopyVector( &trgt_pos, work->trigger ) ;
	    } else {
			_sceVu0AddVector( &trgt_pos, &trgt_pos, work->trigger ) ;
		}
	    _sceVu0SubVector( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[W], &trgt_pos ) ;
	    _sceVu0Normalize( (FVECTOR*)&dir.m[Y], (FVECTOR*)&dir.m[Y] ) ;
	    NewOrgaBullet( &dir,
					   BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE,
					   PLAYER_SIDE,
					   25,
					   work->damage,
					   15000,
					   800*TIME_BASE/5,
					   WP_Usp ) ;

	    if ( work->model==USP_WEAP ) {
			AN_MazzleUSP( work->world, 0 ) ;
		} else {
			AN_MazzleDSE( work->world, 0 ) ;
		}
	    se = ORGA_SE_USP_FIRE ;
	}
	else
	    se = ORGA_SE_USP_FIRE_E ;
	work->trigger->vw += 1.0f ;
	GM_SeSetMode( se, (FVECTOR *)&work->world->m[3], GM_SEMODE_BOMB ) ;
    default:
	break ;
    }
    if ( *work->bullet <= 0 )/* 空なので上がりっぱなし */
	ScaleAndAddVector( (FVECTOR*)&work->weapon->objs[2].world.m[W],
			   (FVECTOR*)&work->weapon->objs[2].world.m[Y],
			   (ORGA_USP_FRAMES - 1.0f) * ORGA_USP_SCALE ) ;
    /* ブローバック */
    else if ( work->trigger->vw >= 1.0f )
    {
	ScaleAndAddVector( (FVECTOR*)&work->weapon->objs[2].world.m[W],
			   (FVECTOR*)&work->weapon->objs[2].world.m[Y],
			   (ORGA_USP_FRAMES - work->trigger->vw) * ORGA_USP_SCALE ) ;
	if ( (work->trigger->vw += 1.0f) > ORGA_USP_FRAMES )
	    work->trigger->vw = 0.0f ;
    }
}

static int GetResources( Work *work,
			 FMATRIX *waist, FMATRIX *right, FMATRIX *left,
			 FVECTOR *trigger, short *bullet, int damage, int model )
{
    DG_DEF  *def ;

	
	def = GV_GetCache( GV_CacheID( model, 'k' ) ) ;

    if ( def == NULL ) {
	printf( "orga_usp.c : no <usp.kms> in data.dar!!!!! She can't shoot!!!\n" ) ;
	return -1 ;
    }

    if ( !(work->weapon = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) ) {
	printf( "orga_usp.c : no memory for USP. She can't shoot!!!\n" ) ;

	return -1 ;
    }
    DG_QueueObjs( work->weapon ) ;
    work->waist   = waist   ;
    work->right   = right   ;
    work->left    = left    ;
    work->trigger = trigger ;
    work->bullet  = bullet  ;
    work->damage  = damage  ;
    work->offset  = DG_UnitMatrix ;
    work->world   = right   ;
	work->model   = model   ;

    return 0 ;
}

void *NewOrgaWeaponUSP( FMATRIX *waist, FMATRIX *right, FMATRIX *left,
			FVECTOR *trigger, short *bullet, int damage, int model )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, waist, right, left, trigger, bullet, damage, model ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
