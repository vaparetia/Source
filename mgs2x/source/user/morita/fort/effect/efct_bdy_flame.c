//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"
#include "../../brk_hzd/brk_hazard.h"


#define FLAME_SIZE   150.0f
#define FLAME_N_FRAME 32
#define FLAME_N_PRIM  FLAME_N_FRAME
#define FLAME_TIME    60*10

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct vol_t
{
    float     vel [FLAME_N_FRAME] ;
    short     tics[FLAME_N_FRAME] ;
    short     v_idx[FLAME_N_FRAME] ;
    short     joint[FLAME_N_FRAME] ;
    int       alpha   ;
    DG_PRIM2 *prim ;
} VOLUME ;

typedef struct Work_t
{
    GV_ACT_EX actor ;

    TARGET	 off    ;
    POWER_TARGET pt_off ;
    
    VOLUME    flame[5] ;

    DG_OBJS  *body ;
    CV2_DEF  *cdef ;

    int       joint ;
    int       v_idx ;
    int       v_max ;

    int       time  ;
} Work ;

/* 一つしか上がらないようにする */
static Work *Work_NewFortBodyFlame = NULL ;

/* 面が上を向いている頂点のみ検索する */
static int GetNextVertex( Work *work )
{
    CV2_MDL *mdl ;
    DG_OBJ  *obj ;
    FVECTOR v ;

    if ( !work->body )
	return 1 ;
    while( work->joint < 21 )
    {
	obj = &work->body->objs[work->joint]   ;
	mdl = &work->cdef->models[work->joint] ;
	_sceVu0ApplyMatrix( &v, &obj->world, &mdl->norms[work->v_idx] ) ;
	if ( v.vy < -0.7f )
	    return 1 ;
	if ( ++work->v_idx > mdl->n_verts )
	    work->v_max=work->v_idx=0, work->joint++ ;
    }
    return 0 ;
}

static int ActFlame( Work *work, VOLUME *vol )
{
    FVECTOR          *prv_p, *nxt_p ;
    DG_PRIM2_UVRGBWH *prv_u, *nxt_u ;
    int   i ;
    DG_PRIM2 *p = vol->prim ;
    FVECTOR   v ;
    int   flag = 0 ;

    prv_p = p->pos  [p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
	if ( GetNextVertex( work ) )
	{
	    if ( vol->tics[i] < 0 )
		vol->tics[i]++ ;
	    else if ( (nxt_u[i].a = prv_u[i].a) )
	    {
		CV2_MDL *mdl = &work->cdef->models[vol->joint[i]] ;
		DG_OBJ  *obj = &work->body->objs[vol->joint[i]]   ;
		float rate ;

		_sceVu0ApplyMatrix( &v, &obj->world, &mdl->verts[vol->v_idx[i]] ) ;

		nxt_u[i].a += vol->tics[i]++<10 ? 1 : -1 ;

		if ( GM_PlayerStatus & PLAYER_MOVE  &&  nxt_u[i].a > vol->alpha-16 )
		{
		    /* 揺らし表現のため ランダムを使っている */
		    rate = 1.0f + rnd()*0.025f ;
		    nxt_u[i].w = nxt_u[i].h = (short)prv_u[i].w ;
		}
		else
		{
		    rate = 0.125f ;
		    nxt_u[i].w = nxt_u[i].h = (short)(prv_u[i].w + 2.0f) ;
		}
		nxt_p[i].vx = prv_p[i].vx + (v.vx - prv_p[i].vx) * rate ;
		nxt_p[i].vy = prv_p[i].vy + vol->vel[i] ;
		nxt_p[i].vz = prv_p[i].vz + (v.vz - prv_p[i].vz) * rate ;
	    }
	    else if ( work->time >= 0 )
	    {
		CV2_MDL *mdl = &work->cdef->models[work->joint] ;
		DG_OBJ  *obj = &work->body->objs[work->joint]   ;

		vol->tics[i] = 0 ;

		vol->joint[i] = work->joint ;
		vol->v_idx[i] = work->v_idx ;
		_sceVu0ApplyMatrix( &nxt_p[i], &obj->world, &mdl->verts[work->v_idx] ) ;
		nxt_u[i].a = vol->alpha-10 ;
		nxt_u[i].w = nxt_u[i].h = (short)FLAME_SIZE ;
	    }
	    work->v_idx++ ;
	    work->v_max++ ;
	    if ( work->v_max > 16 )
		work->v_max = work->v_idx = 0, work->joint++ ;
	}
	else
	    nxt_u[i].a = 0 ;
	flag |= nxt_u[i].a ;
    }

    return flag ;
}

static inline void ActLifeAndTime( Work *work )
{
    if ( !(GM_StagePlayTime & 0xf) )
	GM_VitalityAdjust = -1 ;
    if ( GM_PlayerStatus & PLAYER_MOVE )
	work->time -= 1 ;
    if ( GM_PlayerStatus & PLAYER_ROLLING )
	work->time -= 2 ;
    work->time-- ;
}

static void Act( Work *work )
{
    int  i, flag ;
    GV_MSG *msg ;

    i = GV_ReceiveMessage( GM_PLAYER_CHAR_BODY_FLAME, &msg ) ;
    for ( ; i>0 ; i--, msg++ )
        if ( !msg->message[0] )
	    work->time = 0 ;

    work->v_idx = 0 ;
    work->joint = 0 ;

    ActLifeAndTime( work ) ;

    flag = 0 ;
    for( i=5 ; --i>=0 ; )
	flag |= ActFlame( work, &work->flame[i] ) ; 
    if ( !flag )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    int i ;

    Work_NewFortBodyFlame = NULL ;
    for ( i=5 ; --i>=0 ; )
	if ( work->flame[i].prim )
	    GM_FreePrim2( work->flame[i].prim ) ;
}

static int InitVolume( VOLUME *vol, int flame_id, u_long64 alpha, u_int rgba )
{
    int i ;

    vol->alpha = (rgba >> 24) & 0xff ;
    rgba &= 0x00ffffff ;
    if ( !(vol->prim = BRK_UTL_MakeSPRTWH( FLAME_N_PRIM, flame_id, alpha, FLAME_SIZE, rgba )) )
	PERROR( "No Prim(no memory) : NewFortFlame\n" ) ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
	vol->vel[i]  = rnd()*4.0f + 8.0f ;
	vol->tics[i] = -( i*4 ) ;
    }

    return 0 ;
}

static int GetResources( Work *work, DG_OBJS *body, int cv2_id )
{
    int i ;

    for ( i=5 ; --i>=0 ; )
	if ( InitVolume( &work->flame[i],
			 GV_StrCode( "w11c2_fire3a_alp" ),
			 SCE_GS_SET_ALPHA(0,2,0,1,0),
			 0x207f7f7f ) < 0 )
	    return -1 ;
    work->body = body ;
    if ( !(work->cdef = (CV2_DEF*)GV_GetCache( GV_CacheID( cv2_id, 'c' ))) )
	PERROR( "Cant find CV2<%d> : NewFortBodyFlame\n", cv2_id ) ;

    work->time = FLAME_TIME * 5 / TIME_BASE ;

    GM_SetTarget( &work->off, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, 
		  &DG_ZeroVector ) ;
    GM_SetPowerTarget( &work->off, &work->pt_off, 
		       POWER_ONCE, 255, 0, 1, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->off, WP_BULLET|WP_NO_BLOOD ) ;
    GM_MoveTarget( &work->off, &GM_PlayerPosition ) ;
    GM_TargetSetDirectAttack( &work->off, GM_PlayerTarget ) ;
    GM_PutTarget( &work->off ) ;

    return 0 ;
}

void *NewFortBodyFlame( DG_OBJS *body, int cv2_id )
{
    Work *work ;

    /* このキャラは一つしか上がらない */
    if ( Work_NewFortBodyFlame )
	return NULL ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    Work_NewFortBodyFlame = work ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, body, cv2_id ) < 0 )
	{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
