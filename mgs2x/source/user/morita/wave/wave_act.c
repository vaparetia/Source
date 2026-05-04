//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	action.c
	    波アクション

	1999/08/10 T.Morita
	$Id: wave_act.c,v 1.1.1.3 2002/11/19 11:46:40 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libvu0.h>
#include <libdma.h>
#endif

#include	"libutl.h"
#include	"gameheader.h"

#include	"wave.h"



#if 0
static void WAV_ActMoveModel( Work *w, MDL *m )
{
    int x, z ;
    DG_OBJS **o = m->objs ;
#if 1
    static float rot = 0 ;

    if ( (rot += M_PI/4096) > M_PI )
	rot -= 2*M_PI ;
    sceVu0RotMatrixY( (sceVu0FMATRIX*)&w->root, (sceVu0FMATRIX*)&DG_UnitMatrix, rot ) ;
    for ( z=w->z_blk ; --z>=0 ; )
	for ( x=w->x_blk ; --x>=0 ; o++ )
	{
	    (*o)->world = DG_UnitMatrix ;
	    (*o)->world.m[3][X] = w->center.vx + WAVE_SIZE*WAVE_BASE*(x - w->x_blk/2) ;
	    (*o)->world.m[3][Y] = w->center.vy ;
	    (*o)->world.m[3][Z] = w->center.vz + WAVE_SIZE*WAVE_BASE*(z - w->z_blk/2) ;
	    sceVu0MulMatrix( (sceVu0FMATRIX*)&(*o)->world,
			     (sceVu0FMATRIX*)&w->root,
			     (sceVu0FMATRIX*)&(*o)->world ) ;
	}
#else
    static int pos = 0 ;

    if ( ++pos > WAVE_SIZE* WAVE_BASE/20 )
    {
	pos = 0 ;
	for ( z=w->z_blk ; --z>=0 ; )
	    for ( x=w->x_blk ; --x>=0 ; o++ )
		(*o)->world.m[3][Z] = w->center.vz + WAVE_SIZE*WAVE_BASE*(z - w->z_blk/2) ;
    }
    else
	for ( i=w->z_blk*w->x_blk ; --i>=0 ; o++ )
	    (*o)->world.m[3][Z] += 20.0f ;
#endif
}

static void start_actbrkwave( Work *w, VOL *v, SVECTOR *pos, SVECTOR *vel, int flag )
{
    int   i ;
    DG_OBJS **o = w->m_wave.objs ;
    DG_PRIM2_UVRGBWH *p ;
    float x, z ;
    FVECTOR *vtx ;

    vtx = v->prim->pos  [v->prim->buffer_clock] ;
    p   = v->prim->uvrgb[v->prim->buffer_clock] ;
    //printf( "%x %x %x\n",  v, vtx, p) ;
    for ( i=w->n_blk ; --i>=0 ; o++ )
    {
	x = pos->vx + (*o)->world.m[3][X] ;
	z = pos->vz + (*o)->world.m[3][Z] ;
	if ( (int)((x - w->eyex)/SQR_SIZE)==0 &&
	     (int)((z - w->eyez)/SQR_SIZE)==0  )
	{
	    if (((int)((x-w->blind_px)/w->blind_w)==0 &&
		 (int)((z-w->blind_pz)/w->blind_h)==0  ) || v->n_idx < 0 )
		return ;
	    i = v->vel[v->n_idx--].pad ;
	    vtx[i].vx = x + (irnd()&2047) ;
	    vtx[i].vy = pos->vy + (*o)->world.m[3][Y] + 200 ;
	    vtx[i].vz = z + (irnd()&2047) ;
	    p[i].w = p[i].h = flag * ( flag>0 ? pos->vy-100 : 1600 ) ;
	    v->vel[i].vx  =  vel->vx ;
	    v->vel[i].vy  = -vel->vy/4 + (pos->vy-1000)/100 + (irnd()&63) ;
	    v->vel[i].vz  =  vel->vz ;
	    p[i].a = 60 ;
	    break ;
	}
    }
}

static void WAV_ActBrkWave( VOL *v, int j )
{
    FVECTOR wind = { G_wind.vx/2, 0, G_wind.vz/2, 0 } ;
    DG_PRIM2_UVRGBWH *p, *n ;
    FVECTOR *pos, *prv ;
    SVECTOR *vel ;

    DG_SwitchBuffPrim2( v->prim );
    n   = v->prim->uvrgb[v->prim->buffer_clock  ] ;
    p   = v->prim->uvrgb[v->prim->buffer_clock^1] ;
    pos = v->prim->pos[v->prim->buffer_clock  ] ;
    prv = v->prim->pos[v->prim->buffer_clock^1] ;
    vel = v->vel ;
    for ( ; --j>=0 ; n++, p++, pos++, prv++, vel++ )
	if ( p->a > 0 )
	{
	    if ( (n->a = p->a-1) == 0 )
		v->vel[++v->n_idx].pad = WAVE_N_BRK - j -1, n->a = p->a = 0 ;
	    if ( p->w < 0 )
		pos->vx = prv->vx + wind.vx,
		pos->vz = prv->vz + wind.vz ;
	    if ( (vel->vy -= p->a/20) > 10 )
		n->h = p->h + vel->vy/2 ;
	    else
		n->w = p->w + 5, n->h = p->h + 5 ;
	    asm volatile( "
             ld           $8 ,0(%2)      /* load <vel> to $8 */
             lqc2         vf1,0(%1)      /* load <prv> */
             pextlh       $8 ,$8 ,$0     /* SVECTOR        -> IVECTOR(16bitFix) */
             psraw        $8 ,$8 ,4      /* IVECTOR(16Bit) -> IVECTOR(12bitFix) */
             qmtc2.ni     $8 ,vf2        /* send to cop2    */
             vitof12.xyz  vf2,vf2        /* IVECTOR(12Bit) -> FVECTOR */
             vadd.xyz     vf1,vf1,vf2    /* add <vel> to <prv> */
             sqc2         vf1,0(%0)      /* store <pos>  */
             " : : "r"(pos), "r"(prv), "r"(vel) : "$8" ) ;
	}
}



#if 0
/*
  Get the list of 4 objects in the SQR_SIZE of the sight.
  */
static void get_visible_obj( Work *w, OBJECT *o, OBJECT *vis[4], short xy[XY] )
{
    int i ;

    for( i=w->n_blk ; --i>=0 ; o++ )
    {
	xy[X] = o->objs->world.m[3][X] - w->eyex ;
	xy[Y] = o->objs->world.m[3][Z] - w->eyez ;
	if ( (int)(xy[X]/SQR_SIZE)==0 && (int)(xy[Y]/SQR_SIZE)==0  )
	{
	    xy[X] += SQR_SIZE ; /* cordinate to  */
	    xy[Y] += SQR_SIZE ;
	    vis[0] = o-1 ;
	    vis[1] = o   ;
	    vis[2] = o-1 + w->x_blk ;
	    vis[3] = o   + w->x_blk ;
	    break ;
	}
    }
}
#endif




static void WAV_ActWaveBuble( Work *w, MDL *m )
{
    int x, z ;
    short *mp, *ini ;
    short *uvs, *vtx, *vt, *uv0, *uv1 ;

#if 1
    mp  = &m->map [2*m->row*m->col] ;
    ini = &m->init[m->row*m->col] ;
    vtx = m->vtx[DG_Clock]+Y ;
    uvs = m->uvs[DG_Clock]+Y ;
    for ( z=m->row ; --z>=0 ; )
	for (  x=m->col ; --x>=0 ; )
	{
	    uv0 = uvs + *--mp/2 ;
	    vt  = vtx + *  mp   ;
	    uv1 = uvs + *--mp/2 ;
	    *uv0 = *uv1 = 2700 - *vt + *--ini/4 ;
	}
#else
    mp  = &m->map [2*m->row*m->col] ;
    ini = &m->init[m->row*m->col] ;
    vtx = m->vtx[DG_Clock]+Y ;
    uvs = m->uvs[DG_Clock]+Y ;
    for ( z=m->row ; --z>=0 ; )
	for (  x=m->col ; --x>=0 ; mp-=2 )
	    *(uvs + *(mp-1)/2) = *(uvs + *(mp-2)/2) = 2700 - *(vtx + *(mp-1)) + *--ini/4 ;
#endif
}



#include "uvs_back.h"

static inline void WAV_ActFlowBackWave( MDL *m, float scale, float offset )
{
    int    x = m->row*m->col ;
    short *mp, *uvs, *uv ;

    mp  = &m->map[2*x] ;
    uv  = &uvs_back[2*x-1] ;
    uvs = m->uvs[DG_Clock]+Y ;
    for ( ; --x>=0 ; mp-=2, uv-=2 )
	*(uvs + *(mp-1)/2) = *(uvs + *(mp-2)/2) = *uv*scale/2 + offset ;
}

static void WAV_ActBaseBackWave( Work *w, MDL *m, MDL *n )
{
    int x, z ;
    short *mp ;
    short *vtx0, *vtx1, *vt0, *vt1, *vt ;
    static SVECTOR v = { 0, 80, 40 } ;

    mp   = &m->map[2*m->row*m->col] ;
    vtx0 = m->vtx[DG_Clock] ;
    vtx1 = n->vtx[DG_Clock] ;
    for ( z=m->row ; --z>=0 ;  )
	for (  x=m->col ; --x>=0 ; )
	    if ( z )
	    {
		vt0 = (vtx1   + *--mp) ;
		vt  = (vtx0   + *  mp) ;
		vt1 = (vtx1   + *--mp) ;
#if 0
		if ( z > 10 )
		    start_actbrkwave( w, &w->brk, (SVECTOR *)vt0, &v,  1 ) ;
#endif
		if ( n->init[x+m->col*z] <= 0 )
		{
		    *vt1   = *vt ++ ;
		    *vt0++ = *vt1++ ;
		    *vt1   = *vt ++ ;
		    *vt0++ = *vt1++ ;
		    *vt0   = *vt1 = *vt ;
		}
		else// if ( n->init[x+m->col*z] <= 80 )
		{
		    *vt1   = *vt++/4 + (4096*WAVE_SIZE/m->col)*x*3/4 ;
		    *vt0++ = *vt1++ ;
		    *vt1   = *vt++/2 + n->init[x+m->col*z] ;
		    *vt0++ = *vt1++ ;
		    *vt1   = *vt  /4 + (4096*WAVE_SIZE/m->col)*z*3/4 ;
		    *vt0   = *vt1 ;
		}

	    }
	    else if ( n == &w->m_back[0] || n == &w->m_back[2] )
	    {
		mp-- ;
		*(vtx1+*mp+X) = *(vtx0+*mp+X) ;
		*(vtx1+*mp+Y) = *(vtx0+*mp+Y) ;
		*(vtx1+*mp+Z) = *(vtx0+*mp+Z) ;
		mp-- ;
	    }
	    else if ( n->init[x] <= 0 )
		*(vtx1+*--mp+Z) -= WAVE_BASE*WAVE_SIZE, mp-- ;
	    else
	    {
		mp-- ;
		*(vtx1+*mp+X) = *(w->m_back[0].vtx[DG_Clock] + m->map[2*m->row*(m->col-1)+x*2+1] + X) ;
		*(vtx1+*mp+Y) = *(w->m_back[0].vtx[DG_Clock] + m->map[2*m->row*(m->col-1)+x*2+1] + Y) ;
		*(vtx1+*mp+Z) = *(w->m_back[0].vtx[DG_Clock] + m->map[2*m->row*(m->col-1)+x*2+1] + Z) -WAVE_BASE*WAVE_SIZE ;
		mp-- ;
	    }
}

void WAV_ActAllBackWave( Work *work )
{
    float scale  = (work->m_back[2].tex->v_scale*256.0f/255.0f) ;
    float offset0, offset1 ;

    work->v_start0 = (work->v_start0+1)%400 ; /* fast */
    work->v_start1 = (work->v_start1+1)&511 ;
    offset0 = ((0.5f - work->v_start0/800.0f )*scale + work->m_back[2].tex->v_offset)*4096 ;
    offset1 = ((0.5f - work->v_start1/1023.0f)*scale + work->m_back[2].tex->v_offset)*4096 ;

    WAV_ActBaseBackWave( work, &work->m_wave, &work->m_back[0] ) ;  /* make the base shape */
    WAV_ActBaseBackWave( work, &work->m_wave, &work->m_back[1] ) ;
    WAV_ActFlowBackWave( &work->m_back[2], scale, offset0 ) ; /* fast */
    WAV_ActFlowBackWave( &work->m_back[3], scale, offset1 ) ; /* slow */
    WAV_ActFlowBackWave( &work->m_back[4], scale, offset0 ) ; /* fast */
    WAV_ActFlowBackWave( &work->m_back[5], scale, offset1 ) ; /* slow */
}



#include "uvs_side.h"
static inline void WAV_ActFlowSideWave( MDL *m, int flg, float scale, float offset )
{
    int    x, z ;
    short *mp, *uvs ;

    mp  = &m->map[2*m->row*m->col] ;
    uvs = m->uvs[DG_Clock]+Y ;
    for ( z=m->row ; --z>=0 ; )
	for ( x=m->col ; --x>=0 ; mp-=2 )
	    *(uvs + *(mp-1)/2) = *(uvs + *(mp-2)/2) =
		uvs_side[2*(m->row*z+(flg?x:m->col-1-x))+1]*scale/2 + offset ;
}

static void WAV_ActSideWave( Work *w, MDL *m, MDL *n, int lr, int ofst_z )
{
    int x, z, tx, tz, bf ;
    int t ;
    short *mp ;
    short *vtx0, *vtx1 ;
    SVECTOR *vt0, *vt1, *vt ;

    bf = w->wave0*1024/w->speed ;

    mp   = &m->map[2*n->row*n->col] ;
    vtx0 = m->vtx[DG_Clock] ;
    vtx1 = n->vtx[DG_Clock] ;
    for ( z=n->row ; --z>=0 ; )
	for (  x=n->col ; --x>=0 ; )
	{
	    vt0 = (SVECTOR *)(vtx1 + *--mp) ;
	    vt  = (SVECTOR *)(vtx0 + *  mp) ;
	    vt1 = (SVECTOR *)(vtx1 + *--mp) ;

	    if ( !x || x==n->col-1 || (ofst_z && z==n->col-1) )
		*vt0 = *vt1 = *vt ;
	    else if ( z )
	    {
		if ( lr > 0 )
		    tx = x ;
		else
		    tx = m->row-1-x ;
		tz = ofst_z + z ;

		t = M_SIN( tx*tx*140 - bf*2 + tz*1024/(m->col-1) + M_SIN(tz) ) ;
		vt0->vx = vt1->vx = (4096*WAVE_SIZE/m->col)*x ;
		vt0->vy = vt1->vy = vt->vy*4/(tx+3) - (tx*2048+ t*tx)*120/4096 ;
		vt0->vz = vt1->vz = (4096*WAVE_SIZE/m->col)*z ;
	    }
	    else 
	    {
		if ( ofst_z )
		{
		    vt1 = (SVECTOR*)(w->m_side[lr>0?0:1].vtx[DG_Clock] + m->map[2*m->row*(m->col-1)+x*2+1]) ;
		    *vt0 = *vt1 ;
		}
		vt0->vz -= WAVE_BASE*WAVE_SIZE ;
	    }
	}
}

void WAV_ActAllSideWave( Work *work )
{
    float scale  = (work->m_foam[0].tex->v_scale*256.0f/255.0f) ;
    float offset = ((0.5f - work->v_start0/800.0f )*scale + work->m_foam[0].tex->v_offset)*4096 ;

    WAV_ActSideWave( work, &work->m_wave, &work->m_side[0],  1, 0  ) ;/*left */
    WAV_ActSideWave( work, &work->m_wave, &work->m_side[1], -1, 0  ) ;/*right*/
    WAV_ActSideWave( work, &work->m_wave, &work->m_side[2],  1, WAVE_SIDE_ROW-1 ) ;/* back left */
    WAV_ActSideWave( work, &work->m_wave, &work->m_side[3], -1, WAVE_SIDE_ROW-1 ) ;/* back right*/

    WAV_ActFlowSideWave( &work->m_foam[0], 0, scale, offset ) ;/*left*/
    WAV_ActFlowSideWave( &work->m_foam[1], 0, scale, offset ) ;/*left*/
    WAV_ActFlowSideWave( &work->m_foam[2], 1, scale, offset ) ;/*right*/
    WAV_ActFlowSideWave( &work->m_foam[3], 1, scale, offset ) ;/*right*/
}






#include "wave_sin.h"

static void WAV_ActWave( Work *w )
{
    int x, z, bf ;
    short *mp, *mp1 ;
    MDL *m = &w->m_wave ;
    int ofst = 1024/(m->col-1) ;
    SVECTOR *vt0, *vt1 ;
    short *vtx ;
    //float scale  = m->tex->v_scale*256.0f/255.0f ;
    //float offset = m->tex->v_offset-scale*0.5f/256.0f ;

    bf = w->wave0*1024/w->speed ;
    w->wave0 = (w->wave0+1)%w->speed ;
    //w->wave1 = (w->wave1+1)%1023 ;

    mp = &m->map[2*m->row*m->col] ;
    vtx = m->vtx[DG_Clock] ;
//    uvs = m->uvs[DG_Clock]+Y ;
    for ( z=m->row ; --z>0 ; )
    {
	mp1 = mp ;
//	uv = ( z*scale/(m->row-1)/2 + (0.5 - w->wave1/2047.0f)*scale + offset)*4096 ;
	for (  x=m->col ; --x>0 ; )
	{
	    vt0 = (SVECTOR*)(vtx + *--mp) ; //nr0 = (SVECTOR*)(m->nrm + *mp) ; *(uvs + *mp/2) = uv ;
	    vt1 = (SVECTOR*)(vtx + *--mp) ; //nr1 = (SVECTOR*)(m->nrm + *mp) ; *(uvs + *mp/2) = uv ;

	    vt1->vx = (4096*WAVE_SIZE/m->col)*x + (M_SIN(bf +     x*ofst)+
						   M_SIN(bf + (x+z)*ofst))/4 ;
	    //v.vx = vt1->vx - vt0->vx ;
	    vt0->vx = vt1->vx ;
	    vt1->vy = -(M_COS( bf-125+  x*ofst ) + M_COS(-bf+125+    z*ofst ) +
			M_COS( bf-125+2*z*ofst ) + M_COS( bf-125+(x+z)*ofst ))/6 ;
	    //v.vy = vt1->vy - vt0->vy ;
	    vt0->vy = vt1->vy ;
	    vt1->vz = (4096*WAVE_SIZE/m->col)*z + (M_SIN(-bf +     z*ofst)+
						   M_SIN( bf +   2*z*ofst)+
						   M_SIN( bf + (z+x)*ofst) )/4 ;
	    //v.vz = vt1->vz - vt0->vz ;
	    vt0->vz = vt1->vz ;
	}

	*(vtx + *--mp) = *(vtx + *--mp1)-WAVE_BASE*WAVE_SIZE ;
	*(vtx + *mp+Y) = *(vtx + *mp1+Y) ;
	*(vtx + *mp+Z) = *(vtx + *mp1+Z) ;

	*(vtx + *--mp) = *(vtx + *--mp1)-WAVE_BASE*WAVE_SIZE ;
	*(vtx + *mp+Y) = *(vtx + *mp1+Y) ;
	*(vtx + *mp+Z) = *(vtx + *mp1+Z) ;
    }

    for (  x=m->col ; --x>=0 ; mp-- )
	*(vtx + *--mp+Z) -= WAVE_BASE*WAVE_SIZE; //, *(uvs + *mp/2) = uv ;
}


void ORG_SendWaveMessage2()
{
    int buffer[] = { 1, 1000,2000,20000, 90, 800,0 } ; /* 弾痕にランダムの音を鳴らすメッセージを送る */
    GV_MSG msg ;

    msg.address = GV_StrCode( "wave" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

void ORG_SendWaveMessage1()
{
    int buffer[] = { 2, 3000 } ; /* 弾痕にランダムの音を鳴らすメッセージを送る */
    GV_MSG msg ;

    msg.address = GV_StrCode( "wave" ) ;
    msg.message = buffer ;
    msg.message_len = sizeof(buffer)/sizeof(int) ;
    GV_SendMessage( &msg ) ;
}

#endif


extern void *NewSplash( FMATRIX *, int, short *, int ) ;

static void WAV_ActBigSplash( Work *work )
{
    int i ;

    for ( i=work->n_big ; --i>=0 ; )
	if ( !work->big_pos[i].pad )
	{
	    //printf( "[%d]%d\n", i, w->big_cnt[i] ) ;
	    if ( --work->big_cnt[i] <= 0 )
	    {
		work->big_cnt[i] = work->speed * 4 + (irnd() & 255) ; 
		work->big_pos[i].pad = 1 ;
		NewSplash( &work->big_mtx[i], work->big_alp[i], &work->big_pos[i].pad, work->no_sound ) ;
	    }
	}
}


void WAV_ActAllWave( Work *work )
{
    int     i, j ;
    GV_MSG *msg ;
    static short life ;

#if 0
    if ( GV_PadData[1].press & PAD_L1 )
	ORG_SendWaveMessage1() ;
    if ( GV_PadData[1].press & PAD_L2 )
	ORG_SendWaveMessage2() ;
#endif

    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
	switch( msg->message[0] )
	{
	    FMATRIX mtx ;
	    float   scale, rad ;
	case 1:
#if 0
	    printf( "KIDOU MGS (%d %d %d) rot%d size%d alp%d \n",
		    msg->message[1],
		    msg->message[2],
		    msg->message[3],
		    msg->message[4],
		    msg->message[5],
		    msg->message[6] ) ;
#endif
	    rad = (float)M_PI * msg->message[4] / 180.0f ;
	    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, rad ) ;
	    scale = msg->message[5]/100.0f ;
	    mtx.m[W][X]  = msg->message[1]/scale ;
	    mtx.m[W][Y]  = msg->message[2]/scale ;
	    mtx.m[W][Z]  = msg->message[3]/scale ;
	    mtx.m[W][W] /= scale ;
	    NewSplash( &mtx, msg->message[6], &life, work->no_sound ) ;
	    break ;

	case 2:
	    /*printf( "CHANG MGS %d\n", msg->message[1] ) ;*/
	    for ( j=work->n_big ; --j>=0 ; )
		work->big_mtx[j].m[W][Y] = msg->message[1] * work->big_mtx[j].m[W][W] ;
	    break ;
	}

    WAV_ActBigSplash( work ) ;
}
