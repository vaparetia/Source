//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_obj.c 
   オルガ オルガ戦スネーク隠れ専用プットオブジェ

   1999/12/18 T.Morita
   $Id: orga_obj.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $
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

#include "../include/orga.h"

#define ORGA_MAX_SNAHIDE 10

typedef struct ObjWork_t
{
    GV_ACT    actor  ;

    int       status ;
    DG_OBJS  *objs[ORGA_MAX_SNAHIDE]   ;
    TARGET    target[ORGA_MAX_SNAHIDE] ;  
} ObjWork ;




static void Die( ObjWork *work )
{
    int i ;
    DG_OBJS **o = work->objs ;

    for ( i=ORGA_MAX_SNAHIDE ; --i>=0 ; o++ )
	if ( *o )
	    DG_DequeueObjs( *o ), DG_FreeObjs( *o );
}

void ORG_OBJ_SendOrgaHideMessage( ObjWork *work, int status )
{
    GV_MSG msg ;/*ターゲットのポインタを得るメッセージ*/
    int buffer[2] ;

    msg.address = GV_StrCode( "オルガ" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    if ( !work->status && status )
    {
	buffer[0] = 1 ;
	buffer[1] = 0 ;
	printf( "Send IN stat%x new%x\n", work->status, status ) ;
	GV_SendMessage( &msg ) ;
    }
    else if ( work->status && !status )
    {
	buffer[0] = 0 ;
	buffer[1] = 0 ;
	printf( "Send OUT stat%x new%x\n", work->status, status ) ;
	GV_SendMessage( &msg ) ;
    }
}

int ORG_OBJ_CalcOutsideOfRay( DG_OBJS *o, FVECTOR *snake, FVECTOR *orga, int i, int axis )
{
    FVECTOR v ;

    v.vx = i&1 ? o->def->ux : o->def->lx ;
    v.vy = i&2 ? o->def->uy : o->def->ly ;
    v.vz = i&4 ? o->def->uz : o->def->lz ;
    v.vw = 1.0f ;
    _sceVu0ApplyMatrix( &v, &o->world, &v ) ;

    /* XYZ何れかの軸が オルガとスネークの間にあれば,交差計算になる */
    if ( ( ( snake->vx <  orga->vx &&  snake->vx < v.vx && v.vx < orga->vx  ) ||
	   ( snake->vx >= orga->vx &&  orga->vx  < v.vx && v.vx < snake->vx ) ) ||
	 ( ( snake->vy <  orga->vy &&  snake->vy < v.vy && v.vy < orga->vy  ) ||
	   ( snake->vy >= orga->vy &&  orga->vy  < v.vy && v.vy < snake->vy ) ) ||
	 ( ( snake->vz <  orga->vz &&  snake->vz < v.vz && v.vz < orga->vz  ) ||
	   ( snake->vz >= orga->vz &&  orga->vz  < v.vz && v.vz < snake->vz ) ) )
	return axis ? (( (snake->vx - orga->vx) * (orga->vy - v.vy) >
			 (snake->vy - orga->vy) * (orga->vx - v.vx) ) ? 1<<i : 0) :
		      (( (snake->vx - orga->vx) * (orga->vz - v.vz) >
		         (snake->vz - orga->vz) * (orga->vx - v.vx) ) ? 1<<i : 0) ;
    else
	return 0xff ;
}

int ORG_OBJ_CanISeeYou( DG_OBJS *o, FVECTOR *snake, FVECTOR *orga )
{
    int i, flag ;

    if ( o )
    {
	for ( flag=0, i=8 ; --i>=0 ; )
	    flag |= ORG_OBJ_CalcOutsideOfRay( o, snake, orga, i, X ) ;
	if ( flag != 0 && flag != 0x00ff )
	{
	    for ( flag=0, i=8 ; --i>=0 ; )
		flag |= ORG_OBJ_CalcOutsideOfRay( o, snake, orga, i, Y ) ;
	    if ( flag != 0 && flag != 0x00ff )
		return 1 ;
	}
    }

    return 0 ;
}

static void Act( ObjWork *work )
{
    extern FVECTOR *ORG_RecogPlayerPosNoHide( int joint ) ;
    extern Work *ORG_Work ;
    FVECTOR  *snake, *orga ;
    int       i ;
    int       msk = 1, status = 0  ;
    DG_OBJS **o = work->objs ;
    HIDE   *h = ORG_HideBranch ;

    snake = ORG_RecogPlayerPosNoHide( HUMAN21_KUBI ) ;
    orga = BODYPOS( &ORG_Work->body, HUMAN21_ATAMA ) ;

    for ( i=ORGA_MAX_SNAHIDE ; --i>=0 ; msk<<=1, o++ )
	if ( ORG_OBJ_CanISeeYou( *o, snake, orga ) )
	    status |= msk ;
    for ( i=ORGA_N_HIDE_POOL ; --i>=0 ; msk<<=1, h++ )
	if ( ORG_OBJ_CanISeeYou( h->objs, snake, orga ) )
	    status |= msk ;
    ORG_OBJ_SendOrgaHideMessage( work, status ) ;
    work->status = status ;
}

static void ORG_OBJ_InitTarget( TARGET *t, int map, DG_OBJS *objs )
{
    FVECTOR t_size, t_pos ;
    FVECTOR uv = { objs->def->ux, objs->def->uy, objs->def->uz, 0 } ;
    FVECTOR lv = { objs->def->lx, objs->def->ly, objs->def->lz, 0 } ;

    t_size.vx = (uv.vx - lv.vx)*0.5f ;
    t_size.vy = (uv.vy - lv.vy)*0.5f ;
    t_size.vz = (uv.vz - lv.vz)*0.5f ;
    t_pos.vx  = (uv.vx + lv.vx)*0.5f + objs->def->tx ;
    t_pos.vy  = (uv.vy + lv.vy)*0.5f + objs->def->ty ;
    t_pos.vz  = (uv.vz + lv.vz)*0.5f + objs->def->tz ;

    GM_SetTarget( t, TARGET_DEFENSE|TARGET_ROTATE, map, ENEMY_SIDE, &t_size, &t_pos ) ;
    GM_MoveTarget2( t, &objs->world ) ;
    GM_PutTarget( t ) ;
    //NewTargetView( t, 200, 50, 32 ) ;
}

static int GetResources( ObjWork *work, int name, int where )
{
    int       i, buf[3] ;
    LIT_DEF  *lit_def = NULL ;
    FMATRIX   mtx ;
    DG_DEF   *def ;
    DG_OBJS **objs = work->objs   ;
    TARGET   *trgt = work->target ;

    while( (i = GCL_GetNextOption()) )
	switch( i )
	{
	case 'r':
	    GCL_GetIV( GCL_NextStr(), buf ) ;
	    i = buf[X] & 0x0fff ;
	    _sceVu0RotMatrixX( &mtx, &DG_UnitMatrix, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    i = buf[Y] & 0x0fff ;
	    _sceVu0RotMatrixY( &mtx, &mtx, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    i = buf[Z] & 0x0fff ;
	    _sceVu0RotMatrixZ( &mtx, &mtx, (i-4096*(i>>11))*(float)M_PI/2048.0F ) ;
	    break ;

	case 'p':
	    GCL_GetIV( GCL_NextStr(), buf ) ;
            vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&mtx.m[W] ) ;
	    mtx.m[W][W] = 1.0f ;
	    break ;

	case 'm':
	    if ( !(def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
		PERROR( "No KMS-data in data.cnf!! :NewPutSnakeHideObject" ) ;
	    if ( !(*objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
		PERROR( "Cannot make DG_OBJS(Maybe no memory!!) :NewPutSnakeHideObject" ) ;
	    DG_QueueObjs( *objs ) ;
	    break ;

	case 'l':
	    if ( !(lit_def = (LIT_DEF *)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'l' )) ) )
		PERROR( "No LT2-data in data.cnf!! :NewPutSnakeHideObject" ) ;
	    (*objs)->world = mtx ;
	    DG_MakePreshade( *objs, lit_def ) ;
	    ORG_OBJ_InitTarget( trgt++, where, *objs++ ) ;
	    break ;
	}

    return 0 ;
}

void *NewPutSnakeHideObject( int name, int where )
{
    ObjWork *work ;

    work = (ObjWork *)GV_NewActor( GV_ACTOR_USER, sizeof(ObjWork) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if ( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
