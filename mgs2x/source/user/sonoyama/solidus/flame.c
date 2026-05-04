//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   flame.c
   炎の軌跡
   
   2001/04/12	M.Sonoyama
   $Id: flame.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
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

//#include "../../morita/include/libdg_x.h"
//#include "../../morita/brk_utl/brk_utl.x"
//#include "../../morita/brk_hzd/brk_hazard.h"

extern DG_PRIM2* BRK_UTL_MakeLINE( int n_prim, int tex_id, u_long64 alpha, int rgba0, int rgba1 );
extern DG_PRIM2* BRK_UTL_MakeRSPRTWH( int n_prim, int tex_id, u_long64 alpha, float size, int rgba );

#define FLAME_SIZE   600.0f
#define FLAME_SPEED  30.0f
#define FLAME_N_FRAME 8
#define FLAME_N_PRIM  (FLAME_N_FRAME*2)
#define BRK_GRAVITY   6.0f

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct vol_t
{
    float     vel [FLAME_N_FRAME] ;
    short     tics[FLAME_N_FRAME] ;
    short     rot [FLAME_N_FRAME] ;
    int       alpha   ;
    DG_PRIM2 *prim ;
    DG_PRIM2 *fade ;
} VOLUME ;

typedef struct Work_t
{
    GV_ACT_EX actor ;
    
    FVECTOR   pos ;

    VOLUME    flame ;
    VOLUME    smoke ;

    DG_PRIM2 *fly ;

    int       where ;
    int       life ;
} Work ;

#define PERROR(_s) { printf(_s) ; return -1 ; }


static void SwitchBuff( DG_PRIM2 *p,
					   FVECTOR **prv_p1,
					   FVECTOR **nxt_p1, FVECTOR **nxt_p2,
					   DG_PRIM2_UVRGBWH **prv_u1,
					   DG_PRIM2_UVRGBWH **nxt_u1, DG_PRIM2_UVRGBWH **nxt_u2 )
{
    if ( prv_p1 )
		*prv_p1 = p->pos  [p->buffer_clock] ;
    if ( prv_u1 )
		*prv_u1 = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    *nxt_p1 = p->pos  [p->buffer_clock] ;
    *nxt_u1 = p->uvrgb[p->buffer_clock] ;
    *nxt_p2 = p->pos  [p->buffer_clock], *nxt_p2 += FLAME_N_FRAME ;
    *nxt_u2 = p->uvrgb[p->buffer_clock], *nxt_u2 += FLAME_N_FRAME ;
}


static int ActFly( Work *work, VOLUME *vol )
{
    FVECTOR        *prv_p, *nxt_p ;
    DG_PRIM2_UVRGB *prv_u, *nxt_u ;
    DG_PRIM2 *p = work->fly ;
    int       i ;
    int       flag = 0 ;

    prv_p = p->pos  [p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
     //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( p ) )
   {
      return;
   }
    nxt_p = p->pos  [p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
		if ( (nxt_u->a = prv_u->a) )
		{
			nxt_p->vx = prv_p->vx + 5.0f * frnd() ;
			nxt_p->vy = prv_p->vy + vol->vel[i] ;
			nxt_p->vz = prv_p->vz + 5.0f * frnd() ;
			if ( nxt_p->vy - work->pos.vy > 250.0f )
				nxt_u->a-- ;
			else if ( nxt_u->a < 200 )
				nxt_u->a++ ;
		}
		else if ( work->life )
		{
			nxt_u->a = 1 ;
			_sceVu0CopyVector( nxt_p, &work->pos ) ;
			nxt_p->vx += FLAME_SIZE * 0.5f * frnd() ;
			nxt_p->vz += FLAME_SIZE * 0.5f * frnd() ;
		}
		_sceVu0CopyVector( nxt_p+1, prv_p ) ;
		flag |= nxt_u->a ;
		nxt_p += 2 ; prv_p += 2 ;
		nxt_u += 2 ; prv_u += 2 ;
    }

    return flag ;
}


static int ActFlame( Work *work, VOLUME *vol, float offset_y )
{
    static FVECTOR    ofst = { FLAME_SIZE/2.0f, FLAME_SIZE/2.0f, FLAME_SIZE/2.0f, 0.0f }  ;
    FVECTOR          *prv_p1, *nxt_p1, *nxt_p2,           *nxt_fp1, *nxt_fp2 ;
    DG_PRIM2_UVRGBWH *prv_u1, *nxt_u1, *nxt_u2, *prv_fu1, *nxt_fu1, *nxt_fu2 ;
    int   i ;
    int   flag = 0 ;
    //AR_PARTICLE_HALF
    if(!AS_WillPrimBuffSwitch())
    {
       return;
    }

    SwitchBuff( vol->prim, &prv_p1, &nxt_p1 , &nxt_p2 , &prv_u1 , &nxt_u1 , &nxt_u2  ) ;
    SwitchBuff( vol->fade,  NULL  , &nxt_fp1, &nxt_fp2, &prv_fu1, &nxt_fu1, &nxt_fu2 ) ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
		nxt_u1 [i].a = prv_u1 [i].a ;
		nxt_fu1[i].a = prv_fu1[i].a ;

		if ( vol->tics[i] < 0 )
			vol->tics[i]++ ;
		else if ( nxt_u1[i].a || nxt_fu1[i].a )
		{
			float r, a ;

			if ( ++vol->tics[i] > 60*2 )
			{
				if ( nxt_u1[i].a > 0 )
					nxt_u1[i].a--, nxt_fu1[i].a++ ;
				else
					nxt_fu1[i].a-- ;
			}
			else if ( nxt_u1[i].a < vol->alpha )
				nxt_u1[i].a++ ;

#define ANGtoRAD(_a,_b) (((_a)&(_b))*(float)M_PI/(((_b) >> 1)+1.0f))
			a = ANGtoRAD( vol->tics[i], vol->rot[i] ) + ((int)work & 0xff) ;
			r = vol->tics[i] + FLAME_SIZE ;

			nxt_fu1[i].w = nxt_u1[i].w = r * sinf( a + i ) ;
			nxt_fu1[i].h = nxt_u1[i].h = r * cosf( a + i ) ;
			nxt_fu2[i].w = nxt_u2[i].w = r * sinf(-a + i ) ;
			nxt_fu2[i].h = nxt_u2[i].h = r * cosf(-a + i ) ;
			_sceVu0CopyVector( &nxt_p1[i], &prv_p1[i] ) ;
			nxt_p1[i].vy += vol->vel[i] + 1024.0f*0.25f/(float)vol->rot[i] ;
		}
		else if ( work->life )
		{
			nxt_u1[i].a = 1 ;
			nxt_u1[i].w = nxt_u1[i].h = nxt_u2[i].w = nxt_u2[i].h = 0 ;
			_sceVu0CopyVector( &nxt_p1[i], &work->pos ) ;
			nxt_p1[i].vx += FLAME_SIZE * 0.5f * frnd() ;
			nxt_p1[i].vy += offset_y ;
			nxt_p1[i].vz += FLAME_SIZE * 0.5f * frnd() ;
			nxt_fu1[i].a = 0 ;

			vol->tics[i] = 0 ;
		}

		_sceVu0ScaleVector( &nxt_p2[i], &ofst, 0.25f + (float)vol->tics[i]/410.0f ) ;
		_sceVu0AddVector( &nxt_p2[i], &nxt_p2[i], &nxt_p1[i] ) ;
		nxt_u2[i].a = nxt_u1[i].a>8 ? nxt_u1[i].a-8 : 0 ;
		if ( !nxt_u1[i].a )
			nxt_u1[i].w = nxt_u1[i].h = nxt_u2[i].w = nxt_u2[i].h = 0 ;

		_sceVu0CopyVector( &nxt_fp1[i], &nxt_p1[i] ) ;
		_sceVu0CopyVector( &nxt_fp2[i], &nxt_p2[i] ) ;
		nxt_fu2[i].a = nxt_fu1[i].a>8 ? nxt_fu1[i].a-8 : 0 ;
		if ( !nxt_fu1[i].a )
			nxt_fu1[i].w = nxt_fu1[i].h = nxt_fu2[i].w = nxt_fu2[i].h = 0 ;

		flag |= nxt_u1[i].a | nxt_fu1[i].a ;
    }

    return flag ;
}

static void Act( Work *work )
{
    int flag ;

    flag  = ActFly  ( work, &work->flame ) ;
    flag |= ActFlame( work, &work->flame,    0.0f ) ;
    flag |= ActFlame( work, &work->smoke, 250.0f ) ;

    if ( work->life >0 )
    {
		work->life-- ;

		/* プレーヤーが火の中にいるかどうか */
		if ( !(GM_PlayerStatus & (PLAYER_ROLLING|PLAYER_DAMAGED)) )
		{
			extern void *NewSolBodyFlame( DG_OBJS *body, int cv2_id ) ;
			float x, z ;

			x = GM_PlayerControl->mov.vx - work->pos.vx ;
			z = GM_PlayerControl->mov.vz - work->pos.vz ;
			if ( x*x + z*z < FLAME_SIZE*FLAME_SIZE ) {
				NewSolBodyFlame( GM_PlayerBody->objs, GV_StrCode( "sna_skl2" ) ) ;
			}
		}
    }
    else if ( !work->life && !flag )
		GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->flame.prim )
		GM_FreePrim2( work->flame.prim ) ;
    if ( work->flame.fade )
		GM_FreePrim2( work->flame.fade ) ;
    if ( work->smoke.prim )
		GM_FreePrim2( work->smoke.prim ) ;
    if ( work->smoke.fade )
		GM_FreePrim2( work->smoke.fade ) ;

    if ( work->fly )
		GM_FreePrim2( work->fly ) ;
}

static int InitVolume( VOLUME *vol, int flame_id, int fade_id, u_long64 alpha, u_int rgba, int base )
{
    int i ;

    vol->alpha   = (rgba >> 24) & 0xff ;

    rgba &= 0x00ffffff ;
    if ( !(vol->prim = BRK_UTL_MakeRSPRTWH( FLAME_N_PRIM, flame_id, alpha, 0.0f, rgba )) )
		PERROR( "No Prim(no memory) : NewSolTraceFlame\n" ) ;
    if ( !(vol->fade = BRK_UTL_MakeRSPRTWH( FLAME_N_PRIM, fade_id , alpha, 0.0f, rgba )) )
		PERROR( "No Prim(no memory) : NewSolTraceFlame\n" ) ;

    for ( i=FLAME_N_FRAME ; --i>=0 ; )
    {
		vol->vel[i]  = rnd()*2.0f + 2.0f ;
		vol->rot[i]  = 0x1ff >> (irnd()&1) ;
		vol->tics[i] = -(base + i*32 + (irnd() & 15) ) ;
    }

    return 0 ;
}


static int GetResources( Work *work, FVECTOR *pos, int life )
{
    int  i ;

    if ( InitVolume( &work->flame,
					GV_StrCode( "w11c2_fire3a_alp" ),
					GV_StrCode( "w11c2_fire3b_alp" ),
					SCE_GS_SET_ALPHA(0,2,0,1,0),
					0x207f7f7f, 0 ) < 0 )
		return -1 ;

    if ( InitVolume( &work->smoke,
					GV_StrCode( "w11c2_smoke1_alp" ),
					GV_StrCode( "w11c2_smoke2_alp" ),
					SCE_GS_SET_ALPHA(2,0,0,1,0),
					0x107f7f7f, 120+32 ) < 0 )
		return -1 ;

    if ( !(work->fly = BRK_UTL_MakeLINE( FLAME_N_PRIM,
										0,
										SCE_GS_SET_ALPHA(0,2,0,1,0),
										0x004f7f7f,
										0x004f7f7f )) )
		PERROR( "No Prim(no memory) : NewSolTraceFlame\n" ) ;

    for ( i=FLAME_N_PRIM*2 ; --i>=0 ; )
    {
		FVECTOR *p0 = &work->fly->pos[0][i] ;
		FVECTOR *p1 = &work->fly->pos[1][i] ;

		_sceVu0CopyVector( p0, &work->pos ) ;
		p0->vx += FLAME_SIZE * 0.5f * frnd() ;
		p0->vy +=           250.0f *  rnd() ;
		p0->vz += FLAME_SIZE * 0.5f * frnd() ;
		_sceVu0CopyVector( p1  , p0 ) ;
    }
    _sceVu0CopyVector( &work->pos, pos ) ;
    work->life = life ;

    return 0 ;
}

void *NewSolTraceFlame( FVECTOR *pos, int life )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
        if ( GetResources( work, pos, life ) < 0 )
		{
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return (void *)work ;
}
