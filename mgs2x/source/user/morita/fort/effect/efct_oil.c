//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   efct_oil.c
   フォーチュン戦 オイル燃え（湧き出る部分から）

   2000/12/18 T.Morita
   $Id: efct_oil.c,v 1.1.1.3 2002/11/19 11:46:10 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#define MAKING 0

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"

typedef struct work_t
{
    GV_ACT_EX  actor ;

    FVECTOR    pos  ;
    int        time ;
    int        life ;
    short      se_tic1 ;
    short      se_tic2 ;

    void      *wireframe ;
} Work ;

#define FLAME_SIZE   (1000.0f*1000.0f)

extern void *NewFortFlame( FVECTOR *pos, int life, int already ) ;
extern void *NewBookFire( FMATRIX *mat );
extern void *PutCBoxBreakBody( FMATRIX *world, int map ) ;
extern void *NewFortBodyFlame( DG_OBJS *body, int cv2_id ) ;
extern int  FireBoxFlag ;

int FRT_OilSpreadCount = 0 ;

extern int FRT_GasFire    ;
extern int FRT_GasExplode ;


/* 本が炎の中にあるかどうか */
static inline void SearchBookFire( Work *work )
{
    GM_BOMB	*b;
    float       x, z ;
    FMATRIX	mat;

    for ( b=GM_BombList.next ; b!=NULL ; b=b->next )
	if ( b->weapon == WP_Book )
	{
	    x = b->mov->vx - work->pos.vx ;
	    z = b->mov->vz - work->pos.vz ;
	    if ( x*x + z*z < FLAME_SIZE )
		if ( !( b->flag & GM_BMB_FLAG_DESTROY ) )
		{
		    b->flag |= GM_BMB_FLAG_DESTROY;	
		    DG_COPY_MAT( &mat, &DG_UnitMatrix );
		    DG_COPY_VEC( (FVECTOR *)mat.m[3], b->mov );
		    NewBookFire( &mat );
		    break ;
		}
	}
}

/* プレーヤーが火の中にいるかどうか */
static inline void SearchPlayerFire( Work *work )
{
    float       x, z ;
    int         item ;

    if ( !GM_CheckPlayerStatus( PLAYER_ROLLING|PLAYER_DAMAGED ) &&
	 !GM_CheckGameStatus( STATE_DEMO ) )
    {
	x = GM_PlayerControl->mov.vx - work->pos.vx ;
	z = GM_PlayerControl->mov.vz - work->pos.vz ;
	if ( x*x + z*z < FLAME_SIZE )
	{
	    /* ダンボール装備中処理 */
	    item = PL_GetPlayerItem() ;
	    if ( GM_ItemTypes[item] & IT_TYPE_CBBOX )
	    {
		/* ダンボール燃えフラグ立てておく */
		FireBoxFlag = 1;
		/* ダンボール壊れ呼び出し */
		PutCBoxBreakBody( &GM_PlayerBody->objs->world,
				  GM_PlayerControl->map ) ;
		/* ダンボール消滅 ->> 100減らせばいいでしょう */
		GM_DecrementItem( item, 100 );	
	    }
	    else
		NewFortBodyFlame( GM_PlayerBody->objs, 3075579/*sna_skl3*/ ) ;
	}
    }

}

static void Die( Work *work )
{
#if MAKING
    if ( work->wireframe )
	GV_DestroyOtherActor( work->wireframe ) ;
#endif

    FRT_OilSpreadCount-- ;
}

static void Act( Work *work )
{
    static FVECTOR Offset1 = {-400.0f, 0.0f, 400.0f, 0.0f } ;
    static FVECTOR Offset2 = { 400.0f, 0.0f, 400.0f, 0.0f } ;
    static FVECTOR Offset3 = {-400.0f, 0.0f,-400.0f, 0.0f } ;
    static FVECTOR Offset4 = { 400.0f, 0.0f,-400.0f, 0.0f } ;
    static FVECTOR SeOffset1 = {-200.0f, 0.0f,-200.0f, 0.0f } ;
    static FVECTOR SeOffset2 = { 200.0f, 0.0f, 200.0f, 0.0f } ;
    FVECTOR v ;

    if ( !work->time )
    {
	NewFortFlame( &work->pos, work->life, 0 ) ;

	_sceVu0AddVector( &v, &work->pos, &Offset1 ) ;
	NewFortFlame( &v, work->life, 0 ) ;
	_sceVu0AddVector( &v, &work->pos, &Offset2 ) ;
	NewFortFlame( &v, work->life, 0 ) ;

#if 0 //BP
   _sceVu0AddVector( &v, &work->pos, &Offset3 ) ;
	NewFortFlame( &v, work->life, 0 ) ;
	_sceVu0AddVector( &v, &work->pos, &Offset4 ) ;
	NewFortFlame( &v, work->life, 0 ) ;
#endif

#if MAKING
	{
	    extern void *NewDrawCircle( FMATRIX *root,
					FVECTOR *pos,
					float radius, int rbga ) ;
	    work->wireframe = NewDrawCircle( NULL,
					     &work->pos,
					     1000.0f,
					     0x7f007f ) ;
	}
#endif
    }

    if ( work->time >= 0 )
	work->time-- ;
    else if ( work->life )
    {
	SearchBookFire( work ) ;
	SearchPlayerFire( work ) ;

	/* ライフを減らして時間を計る */
	if ( work->life > 0 )
	    if ( --work->life == 0 )
	    {
		FRT_GasFire-- ;
#if MAKING
		if ( work->wireframe )
		    GV_DestroyOtherActor( work->wireframe ), work->wireframe = NULL ;
#endif
	    }

	if ( work->se_tic1 > 0 )
	    if ( --work->se_tic1 == 0 )
	    {
		work->se_tic1 = 6 + (irnd() & 0x7) ;
		_sceVu0AddVector( &v, &work->pos, &SeOffset1 ) ;
		GM_SeSetMode( SD_A_FIRERED1, &v, GM_SEMODE_REAL ) ;
	    }
	if ( work->se_tic2 > 0 )
	    if ( --work->se_tic2 == 0 )
	    {
		work->se_tic2 = 6 + (irnd() & 0x7) ;
		_sceVu0AddVector( &v, &work->pos, &SeOffset2 ) ;
		GM_SeSetMode( SD_A_FIRERED2, &v, GM_SEMODE_REAL ) ;
	    }
    }
}


static int GetResourcesCalled( Work *work, FVECTOR *pos,
			       int time, int where, int life, float lift )
{
    static FVECTOR OilScale = { 2.0f, 0.0f, 2.0f, 0.0f } ;
    extern void *NewPutVanimeObjectCalled( int id, int where,
					   FVECTOR *scale,
					   SVECTOR *rot, FVECTOR *pos,
					   int base, int rand, int mode ) ;

    ASSERT( where ) ;

    work->se_tic1 = 6 + (irnd() & 0x7) ;
    work->se_tic2 = 6 + (irnd() & 0x7) ;

    work->time = time ;
    work->life = life ;
    _sceVu0CopyVector( &work->pos, pos ) ;

    FRT_GasExplode = 6*60*5/TIME_BASE ;
    if ( life > 0 )
	FRT_GasFire++ ;

    if ( HZX_LevelHazardCheck( GM_GetHzxGroupID(where), pos,
			       HZX_CHK_ALL, HZX_FLOOR_ALL ) )
    {
	float floor[2] ;

	HZX_GetLevelHeight( floor ) ;
	work->pos.vy = floor[0] + 3.0f ;
    }
    work->pos.vy += lift ;
    FRT_OilSpreadCount++ ;

    NewPutVanimeObjectCalled( 16615395,/* w11c2_oilspot1*/ 0,
			      &OilScale, NULL, &work->pos,
			      80, 0, 1 ) ;

    return 0 ;
}


static int GetResources( Work *work, int name, int where )
{
    int     buf[4] ;
    FVECTOR pos    ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
    }
    if ( GetResourcesCalled( work,
			     &pos,
			     GCL_GetOptionValue( 't',  0 ),
			     where,
			     GCL_GetOptionValue( 'l', -1 ),
			     GCL_GetOptionValue( 'T', -1 ) ) < 0 )
	return -1 ;
    return 0 ;
}

void *NewOilSpread( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;

        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}

void *NewOilSpreadCalled( FVECTOR *pos, int time, int where, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;

        if( GetResourcesCalled( work, pos, time, where, life, 4.0f ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
