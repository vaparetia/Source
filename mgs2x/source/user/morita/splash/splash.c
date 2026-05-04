//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splash.c
	    波涛メイン

	1999/11/04 T.Morita
	$Id: splash.c,v 1.1.1.3 2002/11/19 11:46:34 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include "libdg.h"
#include "libdg.cnf"
#include "libutl.h"
#include "gameheader.h"
#include "camera.h"

#define _MAIN_FILE_
#include "splash.h"

extern int BP_FRAMES_PER_SEC();

static void init_param() ;

static inline void add_svec_to_fvec( FVECTOR *a, FVECTOR *v, SVECTOR *t )
{
#ifdef BP_PSX2_ASM
    asm volatile( "
         ld           $8 ,0(%1)      /* load <vel[i]> to $8      */
         lqc2         vf1,0(%0)      /* load <pos[i]> to cop2    */
         pextlh       $8 ,$8 ,$0     /* SVECTOR -> IVECTOR(16bitfix) */
         psraw        $8 ,$8 ,4      /* IVECTOR(16) -> IVECTOR(12bitfix) */
         qmtc2.ni     $8 ,vf2        /* send to cop2             */
         vitof12.xyz  vf2,vf2        /* IVECTOR(12) -> FVECTOR   */
         vadd.xyz     vf1,vf1,vf2    /* add <vel[i]> to <pos[i]> */
         sqc2         vf1,0(%2)      /* store <pos[i]>           */
        " : : "r"(v), "r"(t), "r"(a) : "$8" ) ;
#else
	a->vx = v->vx + t->vx ;
	a->vy = v->vy + t->vy ;
	a->vz = v->vz + t->vz ;
#endif
}

#if 0 /*この辺からばっさり切る*/


#define SPSH_ALPH 1
static void SPH_ActBrkShow( VOL1 *s, int i, int base_alp )
{
    float tx, ty, tz, l ;
    FVECTOR *v=(FVECTOR *)SCRPAD_ADDR, *u=v+1 ;
    FMATRIX pers ;
    int col ;

    DG_PRIM2_UVRGB *p ;
    FVECTOR *pos, *prv ;
    SVECTOR *vel ;

    DG_SwitchBuffPrim2( s->prim );

    pos = s->prim->pos[s->prim->buffer_clock  ] ;
    prv = s->prim->pos[s->prim->buffer_clock^1] ;
    vel = s->vel ;
    p = s->prim->uvrgb[s->prim->buffer_clock] ;

    if( (int)(v->vx/v->vw/1.2f)||(int)(v->vy/v->vw/1.2f) || (int)(v->vz/v->vw) < 0 )
	DG_InvisiblePrim2( s->prim ) ;
    else
    {
	DG_VisiblePrim2  ( s->prim ) ;

	_sceVu0MulMatrix( &pers, &DG_Chanls->eye_pers, &s->prim->world ) ;
	DG_SetPos( &pers ) ;
	DG_PutVector( s->pos, SCRPAD_ADDR, i*2 ) ;

	for ( ; --i>=0 ; p++, u+=2, v+=2 )
	{
	    v->vx = (v->vx/v->vw)*DG_Chanl(0)->width /2 + DG_Chanl(0)->offset_x + 2048 ;
	    v->vy = (v->vy/v->vw)*DG_Chanl(0)->height/2 + DG_Chanl(0)->offset_y + 2048 ;
	    u->vx = (u->vx/u->vw)*DG_Chanl(0)->width /2 + DG_Chanl(0)->offset_x + 2048 ;
	    u->vy = (u->vy/u->vw)*DG_Chanl(0)->height/2 + DG_Chanl(0)->offset_y + 2048 ;

	    tx = u->vx - v->vx ;
	    ty = u->vy - v->vy ;
	    tz = u->vz/u->vw*DRAW_Z_SCALE + DRAW_Z_OFFSET ;

	    l = fpu_Sqrt( tx*tx + ty*ty )/300000*v->vw ;
	    tx /= l ; ty /= l ;
	    if ( l < 1.0f )
		u->vx = v->vx+tx*2*ASPECT_RATIO, u->vy=v->vy+ty ;

#if 0
	    if ( (u->vx *= 16) > 4095.0f )
		u->vx = 4095.0f ;
	    if ( (u->vy *= 16) > 4095.0f )
		u->vy = 4095.0f ;
	    DG_SET_XYZF1( p, u->vx, u->vy, tz*16, 255*16 ) ;
	    if ( (u->vx = (v->vx-s->width*ty*2*ASPECT_RATIO)*16 ) > 4095.0f )
		u->vx = 4095.0f ;
	    if ( (u->vy = (v->vy+s->width*tx)*16 )                > 4095.0f )
		u->vy = 4095.0f ;
	    DG_SET_XYZF2( p, u->vx, u->vy, tz*16, 255*16 ) ;
	    if ( (u->vx = (v->vx+s->width*ty*2*ASPECT_RATIO)*16 ) > 4095.0f )
		u->vx = 4095.0f ;
	    if ( (u->vy = (v->vy-s->width*tx)*16  )               > 4095.0f )
		u->vy = 4095.0f ;
	    DG_SET_XYZF3( p, u->vx, u->vy, tz*16, 255*16 ) ;
	    if ( (u->vx = (v->vx-         tx*2*ASPECT_RATIO)*16 ) > 4095.0f )
		u->vx = 4095.0f ;
	    if ( (u->vy = (v->vy-ty         )*16  )               > 4095.0f )
		u->vy = 4095.0f ;
	    DG_SET_XYZF4( p, u->vx, u->vy, tz*16, 255*16 ) ;
#endif
	    col = s->rgb[K]/SPSH_ALPH + (128*i/(SPSH_MAX_BRK-1)-64)*(128*i/(SPSH_MAX_BRK-1)-64)/128 + base_alp ;
	    p[0].r = 115, p[0].g = 128, p[0].b = 128, p[0].a = col>0?col:0 ;
	    col = s->rgb[K]/SPSH_ALPH + base_alp>0 ? s->rgb[K]/SPSH_ALPH + base_alp : 0 ;
	    p[1].r = s->rgb[R], p[1].g = s->rgb[G], p[1].b = s->rgb[B], p[1].a = col ;
	    p[2].r = s->rgb[R], p[2].g = s->rgb[G], p[2].b = s->rgb[B], p[2].a = col ;
	    p[3].r =         0, p[3].g =         0, p[3].b =         0, p[3].a = col ;
	}
    }
}

static int SPH_ActBrkWave( VOL1 *s, int i )
{
    if ( s->rgb[K] <= -127 )
	return 1 ;
    for ( ; --i>=0 ; )
    {
	if ( s->rgb[K] > 0 )
	    s->vel[i].vy -= s->rgb[K]/20 ;
//	if ( s->pos[i*2+1].vy >0 )
	    add_svec_to_fvec( &s->pos[i*2+1],&s->pos[i*2+1], &s->vel[i] ) ;
    }
    if ( s->rgb[K] < 90 )
	s->width += 0.008f ;
    if ( s->rgb[K] < 0 )
	s->rgb[K] -= 2 ;
    else if ( s->vel[0].vy < 5 )
	s->rgb[K]-- ;
    return 0 ;
}


#endif /*この辺からバッサり切るの終り */

static int SPH_ActBrkVol1( VOL2 *s, int i, int mode, int base_alp )
{
    int ret = 0, col ;
    FVECTOR *pos, *prv ;
    SVECTOR *vel ;
    DG_PRIM2_UVRGBWH *p ;

    prv = s->prim->pos[s->prim->buffer_clock] ;
    DG_SwitchBuffPrim2( s->prim );
    pos = s->prim->pos[s->prim->buffer_clock] ;
    p   = s->prim->uvrgb[s->prim->buffer_clock] ;
    vel = s->vel ;

    if ( (s->size -= (float)s->vel[0].pad/90) < 1 )
	s->size = 1 ;
    for ( ; --i>=0 ; p++, pos++, prv++, vel++ )
	if ( vel->pad > 0 )
	{
	    if ( (vel->vy -= vel->pad/BP_FRAMES_PER_SEC()) < 10 )
		vel->pad-- ;
	    if ( vel->vy < -4 && mode )
		vel->vy = -4 ;
//if ( !i ) printf( "%d\n", vel->vy ) ;
	    p->w = p->h += (short)s->size ;
	    add_svec_to_fvec( pos, prv, vel ) ;
	    col = (int)(vel->pad / s->alph) + base_alp ;
	    p->a = col > 0 ? (col & 255) : 0 ;
	    ret |= vel->pad ;
	}
    return ret ;
}

static void Act( Work *work )
{
    int alive = 0 ;

    //alive  = SPH_ActBrkWave ( &work->spsh, SPSH_MAX_BRK  ) ;
    alive |= SPH_ActBrkVol1( &work->bubl, SPSH_MAX_SPSH, 0, work->base_alp ) ;
    alive |= SPH_ActBrkVol1( &work->fog , SPSH_MAX_FOG , 1, work->base_alp ) ;

    //SPH_ActBrkShow ( work, SPSH_MAX_BRK  ) ;
    if ( !alive )
	GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
    if ( work->spsh.prim )
	GM_FreePrim2( work->spsh.prim ) ;
    if ( work->bubl.prim )
	GM_FreePrim2( work->bubl.prim ) ;
    if ( work->fog.prim )
	GM_FreePrim2( work->fog.prim ) ;
    *work->life = 0 ;
}




static void init_param( Work *work )
{
    int j ;
    int px, pz ;

    work->bubl.size = 40 ;
    work->fog.size  = 40 ;
    work->bubl.alph =  3 ;
    work->fog.alph  =  2 ;

    work->spsh.size =  0 ;
    work->spsh.width = 1.0f ;
    work->spsh.rgb[R] = work->spsh.rgb[G] = work->spsh.rgb[B] = 128 ;
    work->spsh.rgb[K] = 60 ;

    for ( j=SPSH_MAX_BRK ; --j>=0 ; )
    {
		px  = 2047*j/(SPSH_MAX_BRK-1)-1024 + (irnd()&127) ;
		pz  =  irnd()&1023 ;

	work->spsh.pos[j*2+0].vx = (float)px ;
	work->spsh.pos[j*2+0].vy = (float)0  ;
	work->spsh.pos[j*2+0].vz = (float)pz ;
	work->spsh.pos[j*2+0].vw = (float)1  ;

	work->spsh.pos[j*2+1].vx = (float)(px*2) ;
	work->spsh.pos[j*2+1].vy = (float)(irnd()&255) ;
	work->spsh.pos[j*2+1].vz = (float)(10+pz*2) ;
	work->spsh.pos[j*2+1].vw = (float)(1) ;

	work->spsh.vel[j].vx = (short)(px/120) ;
	work->spsh.vel[j].vy = (short)((2000+(irnd()&255)-px*px/2000.0f)/14) ;
	work->spsh.vel[j].vz = (short)((1600+            -px*px/2000.0f)/48) ;
    }
    for ( j=SPSH_MAX_SPSH ; --j>=0 ; )
    {
	px  = 2047*j/(SPSH_MAX_SPSH-1)-1024 + (irnd()&127) ;
	pz  =  irnd()&1023 ;

	work->bubl.prim->pos[0][j].vx  = (float)px ;
	work->bubl.prim->pos[0][j].vy  = (float)0  ;
	work->bubl.prim->pos[0][j].vz  = (float)pz ;
	work->bubl.prim->pos[1][j].vx  = (float)px ;
	work->bubl.prim->pos[1][j].vy  = (float)0  ;
	work->bubl.prim->pos[1][j].vz  = (float)pz ;
	work->bubl.vel[j].vx  = (short)(px/120) ;
	work->bubl.vel[j].vy  = (short)((1800+(irnd()&255)-px*px/2200.0f )/20) ;
	work->bubl.vel[j].vz  = (short)(pz/BP_FRAMES_PER_SEC() ) ;
	work->bubl.vel[j].pad = (short)(120   ) ;
    }
    for ( j=SPSH_MAX_FOG ; --j>=0 ; )
    {
		px = 2047*j/(SPSH_MAX_FOG-1)-1024 + (irnd()&127) ;
		work->fog.prim->pos[0][j].vx = (float)px ;
		work->fog.prim->pos[0][j].vy = (float)0  ;
		work->fog.prim->pos[0][j].vz = (float)pz ;
		px = 2047*j/(SPSH_MAX_FOG-1)-1024 + (irnd()&127) ;
		work->fog.prim->pos[1][j].vx = (float)px ;
		work->fog.prim->pos[1][j].vy = (float)0  ;
		work->fog.prim->pos[1][j].vz = (float)pz ;
		work->fog.vel[j].vx  = (short)(px/480) ;
		work->fog.vel[j].vy  = (short)((2200+(irnd()&255)-px*px/2000.0f)/40 ) ;
		work->fog.vel[j].vz  = (short)((1600+(irnd()&255)-px*px/2000.0f)/120) ;
		work->fog.vel[j].pad = (short)(120)   ;
    }
}

#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static int InitSprt( DG_PRIM2 *prim, FMATRIX *root, DG_TEX *t, int i, int alpha,
		     int wh, int rnd_msk, int col )
{
    DG_PRIM2_UVRGBWH *u0 = prim->uvrgb[0], *u1 = prim->uvrgb[1] ;

    prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    prim->as_world = *root ;
    DG_ConfigPrim2Tex( prim, t ) ;
    DG_SetPrim2Alpha( prim, alpha ) ;

    for ( ; --i>=0 ; u0++, u1++ )
    {
        u0->u0 = u1->u0 = FTOI12( 0.0F * t->u_scale + t->u_offset ) ;
        u0->u1 = u1->u1 = FTOI12( 1.0F * t->u_scale + t->u_offset ) ;
        u0->v0 = u1->v0 = FTOI12( 0.0F * t->v_scale + t->v_offset ) ;
        u0->v1 = u1->v1 = FTOI12( 1.0F * t->v_scale + t->v_offset ) ;
        u0->q0 = u1->q0 = u0->q1 = u1->q1 = 4096 ;
        u0->f0 = u1->f0 = u0->f1 = u1->f1 = 0x0fff ;
	u0->w  = u1->w  = u0->h  = u1->h  = wh + (irnd()&rnd_msk) ;
	u0->r  = u1->r  = (col>>24)&0xff;
	u1->g  = u0->g  = (col>>16)&0xff;
	u0->b  = u1->b  = (col>> 8)&0xff;
	u0->a  = u1->a  = 128;
    }
    return 0 ;
}

static int SPH_SetSound( FMATRIX *root )
{
    FVECTOR pos, vec ;
    int se_id, vol, pan ;
    float bp_angle;
    float dis ;
    extern GM_CameraSet  *GM_GetCurrentCamera( int cn ) ;

    /* ザッパーンの音 */
    _sceVu0CopyVector( &pos, (FVECTOR *)root->m[W] ) ;
    _sceVu0ScaleVector( &pos, &pos, 1.0f/root->m[W][W] ) ;
    pos.vy += 6000.0f ;
    if ( root->m[W][W] <= 1.0f )
	se_id = SD_A_WAVE01 ;
    else
	se_id = SD_A_WAVE02 ;

    _sceVu0SubVector( &vec, &pos, &GM_GetCurrentCamera( 0 )->position ) ;
    dis = (float)(GV_VecLen3F( &vec ) - GM_SE_MIN_DIS_CAMTRG) ;
    vol = GM_MAX_VOL * ( dis < 0 ? 1 : (30*GM_SE_MAX_DIS_CAMTRG - dis) / (30 * GM_SE_MAX_DIS_CAMTRG) ) ;
//printf( "VOL%d MAXVOL%d * (DIS_CAM%d - dis%f)/%d\n", vol, GM_MAX_VOL, 30*GM_SE_MAX_DIS_CAMTRG, dis, 30*GM_SE_MAX_DIS_CAMTRG ) ;
    if ( vol <= 0 )  /* ０なら鳴らさない */
	return 0 ;
    else if ( vol > GM_MAX_VOL )
	vol = GM_MAX_VOL ;
//printf( "VOL%d\n", vol ) ;
    pan = GM_SeGetPan( &pos, GM_SEMODE_NORMAL, &bp_angle ) ;
    GM_SeSet3D( pan, vol , se_id, bp_angle ) ;
    GM_SeSetMode( se_id, &pos, GM_SEMODE_NORMAL ) ;
    return  0 ;
}

static int GetResources( Work *work, FMATRIX *root, int base_alp, short *life, int no_sound )
{
    int flag = 0 ;

    work->life      = life     ;
    work->base_alp  = (char)(base_alp&0xff) ;

    /* 音を鳴らす */
    if ( !GM_CheckGameStatus( STATE_DEMO ) && !no_sound )
	if ( SPH_SetSound( root ) < 0 )
	    return -1 ;

#if 0
    work->spsh.prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, SPSH_MAX_BRK, 4 ) ;
    if ( work->spsh.prim == NULL )
	return -1 ;
    InitUVRGB( work->spsh.prim,
	       root,
	       DG_GetTexture(GV_StrCode("splash03") ),
	       SPSH_MAX_BRK,
	       SCE_GS_SET_ALPHA(0,2,0,1,0)
	) ;
#endif

    work->bubl.prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, SPSH_MAX_SPSH ) ;
    if ( work->bubl.prim == NULL )
	return -1 ;
    InitSprt( work->bubl.prim,
	      root,
	      DG_GetTexture(GV_StrCode("splash05_alp") ),
	      SPSH_MAX_SPSH,
	      SCE_GS_SET_ALPHA(0,2,0,1,0),
	      400, 63, base_alp
	) ;

    work->fog.prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, SPSH_MAX_FOG ) ;
    if ( work->fog.prim == NULL )
	return -1 ;
    InitSprt( work->fog.prim,
	      root,
	      DG_GetTexture(GV_StrCode("splash03_alp") ),
	      SPSH_MAX_FOG,
	      SCE_GS_SET_ALPHA(0,2,0,1,0),
	      800, 0, base_alp
	) ;

    init_param( work ) ;

    return flag ;
}

void *NewSplash( FMATRIX *root, int base_alp, short *life, int no_sound )
{
    Work *work ;

    if ( (work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, root, (0x80808000|(base_alp&0xff)), life, no_sound ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    else
	*life = 0 ;

    return (void *)work ;
}

void *NewWaveSplash_Demo( FMATRIX *root, float scale, int base_alp )
{
    Work *work ;
    static short life ;

    root->m[W][X] /= scale ;
    root->m[W][Y] /= scale ;
    root->m[W][Z] /= scale ;
    root->m[W][W] /= scale ;
    if ( (work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, root, (0x80808000|(base_alp&0xff)), &life, 1 ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }

    return (void *)work ;
}

void *NewWaveSplash_Demo2( FMATRIX *root, float scale, int base_alp )
{
    Work *work ;
    static short life ;

    root->m[W][X] /= scale ;
    root->m[W][Y] /= scale ;
    root->m[W][Z] /= scale ;
    root->m[W][W] /= scale ;
    if ( (work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) )) != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
        GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, root, base_alp, &life, 1 ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }

    return (void *)work ;
}
