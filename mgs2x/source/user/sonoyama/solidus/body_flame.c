//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   body_flame.c
   プレイヤーにまとわりつく炎ダメージ付き
   
   2001/04/12	M.Sonoyama
   $Id: body_flame.c,v 1.1.1.3 2002/11/19 11:51:03 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include  "gameheader.h"
#include  "libutl.h"

#if 0
#include "../../morita/include/libdg_x.h"
#include "../../morita/brk_utl/brk_utl.x"
#include "../../morita/brk_hzd/brk_hazard.h"


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
    GV_ACT_EX 		actor ;
	TARGET			offense ;
	POWER_TARGET	pt_offense ;
    
    VOLUME    flame[5] ;

    DG_OBJS  *body ;
    CV2_DEF  *cdef ;

    int       joint ;
    int       v_idx ;
    int       v_max ;

    int       time  ;
} Work ;

/* 一つしか上がらないようにする */
static Work *Work_NewSolBodyFlame = NULL ;

#define PERROR(_s...) ({ printf(_s) ; return -1 ; } )

static inline void ApplyMatrixXYZ( FVECTOR *a, FMATRIX *m, FVECTOR *v )
{
    asm volatile  ("
    lqc2        vf1,0x00(%1)
    lqc2        vf2,0x00(%0)
    lqc2        vf3,0x10(%0)
    lqc2        vf4,0x20(%0)
    vmulax.xyz  ACC,vf2,vf1x
    vmadday.xyz ACC,vf3,vf1y
    vmaddz.xyz  vf2,vf4,vf1z
    sqc2        vf2,0x00(%2)
    " : : "r"(m), "r"(v), "r"(a) );
}


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
		DG_SetPos( &obj->world ) ;
		DG_RotVector( &mdl->norms[ work->v_idx ], &v, 1 ) ;
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
				int rate ;

				_sceVu0ApplyMatrix( &v, &obj->world, &mdl->verts[vol->v_idx[i]] ) ;

				nxt_u[i].a += vol->tics[i]++<10 ? 1 : -1 ;

				if ( GM_PlayerStatus & PLAYER_MOVE  &&  nxt_u[i].a > vol->alpha-16 )
				{
					/* 揺らし表現のため ランダムを使っている */
					rate = 1.0f + rnd()*0.025f ;
					nxt_u[i].w = nxt_u[i].h = prv_u[i].w ;
				}
				else
				{
					rate = 0.125f ;
					nxt_u[i].w = nxt_u[i].h = prv_u[i].w + 2.0f ;
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
				nxt_u[i].w = nxt_u[i].h = FLAME_SIZE ;
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
    if ( !(GV_Time & 0xf) )
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

    Work_NewSolBodyFlame = NULL ;
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
		PERROR( "Cant find CV2<%d> : NewSolBodyFlame\n", cv2_id ) ;

    work->time = FLAME_TIME * 5 / TIME_BASE ;

	{
		GM_SetTarget( &work->offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, 
					  &DG_ZeroVector ) ;
		GM_SetPowerTarget( &work->offense, &work->pt_offense, 
						   POWER_ONCE, 255, 0, 1, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->offense, WP_BULLET | WP_NO_BLOOD ) ;
		GM_MoveTarget( &work->offense, &GM_PlayerPosition ) ;
		GM_TargetSetDirectAttack( &work->offense, GM_PlayerTarget ) ;
		GM_PutTarget( &work->offense ) ;
	}

    return 0 ;
}

void *NewSolBodyFlame( DG_OBJS *body, int cv2_id )
{
    Work *work ;

    /* このキャラは一つしか上がらない */
    if ( Work_NewSolBodyFlame )	return NULL ;
    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    Work_NewSolBodyFlame = work ;
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
#endif

extern	void	*NewFortBodyFlame( DG_OBJS *body, int cv2_id ) ;

typedef	struct	{
	GV_ACT_EX			actor ;
	TARGET				offense ;
	POWER_TARGET		pt_offense ;
} Work ;

static	void		Act( Work *work )
{
	GV_DestroyActor( work ) ;	
}

void 	*NewSolBodyFlame( DG_OBJS *body, int cv2_id )
{
	Work			*work ;

	if( NewFortBodyFlame( body, cv2_id ) == NULL ) {
		return NULL ;
	}

	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, NULL ) ;
		GV_ActorEX( &work->actor ) ;
		GM_SetTarget( &work->offense, TARGET_OFFENSE, 0, PLAYER_SIDE, &DG_ZeroVector, 
					  &DG_ZeroVector ) ;
		GM_SetPowerTarget( &work->offense, &work->pt_offense, 
						  POWER_ONCE, 255, 0, 1, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->offense, WP_BULLET | WP_NO_BLOOD | WP_BOXREMOVE ) ;
		GM_MoveTarget( &work->offense, &GM_PlayerPosition ) ;
		GM_TargetSetDirectAttack( &work->offense, GM_PlayerTarget ) ;
		GM_PutTarget( &work->offense ) ;
	}
	return work ;
}


