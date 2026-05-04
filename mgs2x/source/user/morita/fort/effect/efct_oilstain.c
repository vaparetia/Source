//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   efct_oil.c
   フォーチュン戦 オイル燃え（湧き出る部分から）

   2000/12/18 T.Morita
   $Id: efct_oilstain.c,v 1.1.1.3 2002/11/19 11:46:10 Yoshizawa1 Exp $
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

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
typedef struct work_t
{
    GV_ACT_EX  actor ;

    FMATRIX    lights[2] ;
    FVECTOR    pos ;

    DG_OBJS   *objs ;
    int        life  ;
    short      se_tic1 ;
    short      se_tic2 ;
    
} Work ;

extern void *NewFortFlame( FVECTOR *pos, int life, int already ) ;

static void Die( Work *work )
{
    if ( work->objs )
	DG_DequeueObjs( work->objs ), DG_FreeObjs( work->objs ) ;
}

static void Act( Work *work )
{
    static FVECTOR SeOffset1 = {-200.0f, 0.0f,-200.0f, 0.0f } ;
    static FVECTOR SeOffset2 = { 200.0f, 0.0f, 200.0f, 0.0f } ;
    FVECTOR v ;

    if ( work->life > 0 )
    {
	work->life-- ;
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

static int GetResources( Work *work, FVECTOR *pos, int where,
			 int life )
{
    static FVECTOR OilScale = { 2.0f, 0.0f, 2.0f, 0.0f } ;
    DG_DEF *def ;
    static FVECTOR Offset1 = {-400.0f, 0.0f, 400.0f, 0.0f } ;
    static FVECTOR Offset2 = { 400.0f, 0.0f, 400.0f, 0.0f } ;
    static FVECTOR Offset3 = {-400.0f, 0.0f,-400.0f, 0.0f } ;
    static FVECTOR Offset4 = { 400.0f, 0.0f,-400.0f, 0.0f } ;
    FVECTOR v ;
    FVECTOR oil_pos ;

    if ( !(def = GV_GetCache( GV_CacheID( 16615395/*w11c2_oilspot1*/,'k'))) )
	PERROR( "w11c2_oilspot1.kms not in data.cnf : NewOilStained\n" ) ;
    if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 )))
	PERROR( "Cannot create DG_OBJS(Maybe no memory) :: NewOilStained\n" ) ;
    DG_QueueObjs( work->objs ) ;
    work->objs->objs[0].flag |= DG_FLAG_INVISIBLE ;
    work->objs->objs[1].flag |= DG_FLAG_INVISIBLE ;
    work->objs->world.m[X][X] = OilScale.vx ;
    work->objs->world.m[Z][Z] = OilScale.vz ;
    _sceVu0CopyVector( &oil_pos, pos ) ;
    if ( HZX_LevelHazardCheck( GM_GetHzxGroupID(where), pos,
			       HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	oil_pos.vy = HZX_GetFloorLevel() + 3.0f ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)work->objs->world.m[W], &oil_pos ) ;
    DG_SetLightMatrix( work->objs, work->lights ) ; /* シェード */
    DG_GetLightMatrixFix( &oil_pos, work->lights ) ;

    _sceVu0CopyVector( &work->pos, pos ) ;
    work->se_tic1 = 6 + (irnd() & 0x7) ;
    work->se_tic2 = 6 + (irnd() & 0x7) ;
    work->life    = life ;
    if ( life )
    {
	NewFortFlame( &oil_pos, life, 1 ) ;
	_sceVu0AddVector( &v, &oil_pos, &Offset1 ) ;
	NewFortFlame( &v, life, 1 ) ;
	_sceVu0AddVector( &v, &oil_pos, &Offset2 ) ;
	NewFortFlame( &v, life, 1 ) ;
	_sceVu0AddVector( &v, &oil_pos, &Offset3 ) ;
	NewFortFlame( &v, life, 1 ) ;
	_sceVu0AddVector( &v, &oil_pos, &Offset4 ) ;
	NewFortFlame( &v, life, 1 ) ;
    }

    return 0 ;
}

void *NewOilStained( FVECTOR *pos, int where, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;

        if( GetResources( work, pos, where, life ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
