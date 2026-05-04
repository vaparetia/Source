/*
	init.c
	    波 初期化関数群

	1999/11/10 T.Morita
	$Id: water_init.c,v 1.1.1.3 2002/11/19 11:46:39 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#define _INIT_FILE_
#include "water.h"




static void init_model( MDL *dst, MDL *src, int col, int row, DG_TEX *t, int type, int flag )
{
    int    i, x, z ;
    short  *mp ;
    DG_MDLPACK *o, *p ;

    dst->def  = def ;
    dst->def.models[0] = def.models[0] ;
    if ( src )
	*dst = *src ;
    dst->flag = flag ; 
    dst->col  = col ;
    dst->row  = row ;
    if ( flag & MDL_INIT_TYP )
	dst->def.models[0].type = type ;
    dst->def.models[0].n_packs = i = row ;
    dst->def.models[0].packs = WTR_MemAlloc( sizeof(DG_MDLPACK)*i ) ;
    if ( flag & MDL_INIT_VTX )
	dst->vtx[0]= WTR_MemAlloc( sizeof(short) * (col*row*2*4)*2 ),
	    dst->vtx[1]= dst->vtx[0] + (col*row*2*4) ;
    if ( flag & MDL_INIT_NRM )
	dst->nrm   = WTR_MemAlloc( sizeof(short) * (col*row*2*4) ) ;
    if ( flag & MDL_INIT_UVS )
	dst->uvs[0] = WTR_MemAlloc( sizeof(short) * (col*row*2*2) ),
	    dst->uvs[1] = dst->uvs[0] ;
    if ( flag & MDL_INIT_UVS2 )
	dst->uvs[0] = WTR_MemAlloc( sizeof(short) * (col*row*2*2)*2 ),
	    dst->uvs[1] = dst->uvs[0] + (col*row*2*2) ;
    if ( flag & MDL_INIT_MAP )
	dst->map   = WTR_MemAlloc( sizeof(short) * (col*row*2  ) ) ;
    if ( flag & MDL_INIT_INI )
	dst->init  = WTR_MemAlloc( sizeof(short) * (col*row    ) ) ;

    t ? (dst->tex=t) : (t=dst->tex) ;
    o =       dst->def.models[0].packs + i     ;
    p = src ? src->def.models[0].packs + i : o ;
    for ( ; --i>=0 ; )
    {
	o--, p-- ;
	o->flag  = DG_PACKFLAG_NORMAL ;
	o->n_verts = col*2 + (i==row-1?0:2)*0 ;
	(DG_TEX*)(o->tex_id) = t ;
	o->verts = flag&MDL_INIT_VTX ? dst->vtx[0] + col*2 *4*i : p->verts ;
	o->norms = flag&MDL_INIT_NRM ? dst->nrm    + col*2 *4*i : p->norms ;
	o->uvs   = flag&(MDL_INIT_UVS|MDL_INIT_UVS2) ? dst->uvs[0] + col*2 *2*i : p->uvs   ;
    }

    mp = &dst->map[2*row*col-2] ;
    for ( z=row ; --z>=0 ; )
	for ( x=col ; --x>=0 ; mp -= 2 )
	{
	    /* initialize vertex map XZcoordinate=>strip index */
	    if ( flag & MDL_INIT_MAP )
	    {
		*(mp  ) = 4*( (col*2)*z + x*2 ) ;
		*(mp+1) = 4*( *(mp)/4 - (col*2-1) ) ;
		if ( z==row-1 )
		    *(mp) = 4*( x*2 ) ;
		if ( z==0     )
		    *(mp+1) = *mp ;
	    }
	    if ( flag & MDL_INIT_NRM )
	    {
		dst->nrm[*(mp  )+X] = dst->nrm[*(mp  )+Z] = dst->nrm[*(mp  )+Y] = 128 ;
		dst->nrm[*(mp+1)+X] = dst->nrm[*(mp+1)+Z] = dst->nrm[*(mp+1)+Y] = 128 ;
		dst->nrm[*(mp  )+W] = dst->nrm[*(mp+1)+W] = x ? 0x0fff/*disp*/ : 0x8fff/*undisp*/ ;
	    }
	    /* initialize model data */
	    if ( flag & (MDL_INIT_UVS|MDL_INIT_UVS2) )
	    {
		dst->uvs[0][*mp/2+X] = dst->uvs[0][*(mp+1)/2+X] =
		dst->uvs[1][*mp/2+X] = dst->uvs[1][*(mp+1)/2+X] =
		    ( x*t->u_scale/(col-1) + t->u_offset ) * 4096 ;
		dst->uvs[0][*mp/2+Y] = dst->uvs[0][*(mp+1)/2+Y] =
		dst->uvs[1][*mp/2+Y] = dst->uvs[1][*(mp+1)/2+Y] =
		    ( z*t->v_scale/(row-1) + t->v_offset ) * 4096 ;
	    }
	    if ( flag & MDL_INIT_VTX )
	    {
		dst->vtx[0][*mp+X] = dst->vtx[0][*(mp+1)+X] =
		dst->vtx[1][*mp+X] = dst->vtx[1][*(mp+1)+X] = 4096*WATR_SIZE*x/col ;
		dst->vtx[0][*mp+Z] = dst->vtx[0][*(mp+1)+Z] =
		dst->vtx[1][*mp+Z] = dst->vtx[1][*(mp+1)+Z] = 4096*WATR_SIZE*z/row ;
		dst->vtx[0][*mp+W] = dst->vtx[0][*(mp+1)+W] =
		dst->vtx[1][*mp+W] = dst->vtx[1][*(mp+1)+W] = 4096 ;
	    }
	    if ( flag & MDL_INIT_INI )
	    {
		dst->init[x+col*z] = irnd()&128 ;
		if ( z == 0 )
		    dst->init[x    ] = dst->init[x    +row*(col-1)] ;
		if ( x == 0 )
		    dst->init[row*z] = dst->init[col-1+row*z      ] ;
	    }
	}
}

static void set_double_buffer( DG_OBJS *o, MDL *m )
{
    int i ;
    DG_SUBPACK *sub[2] ;

    sub[0] = o->objs->sub_packet[0] ;
    sub[1] = o->objs->sub_packet[1] ;
    for( i=o->objs->n_packs ; --i>=0 ; sub[0]++, sub[1]++ )
    {
	sub[1]->packet.verts_tag.addr =
	    sub[0]->packet.verts_tag.addr + sizeof(short)*(m->col*m->row*2*4) ;
	if ( m->flag & MDL_INIT_UVS2 )
	    sub[1]->packet.uvs_tag.addr =
		sub[0]->packet.uvs_tag.addr + sizeof(short)*(m->col*m->row*2*2) ;
    }
}

static void init_obj_mesh( Work *work, MDL *m, int flag, int x_size, int z_size )
{
    DG_OBJS **o ;

    o = m->objs = (DG_OBJS **)WTR_MemAlloc( sizeof(DG_OBJS *) ) ;

    DG_QueueObjs( *o=DG_MakeObjs( &m->def, flag, 0 ) ) ;
    (*o)->world.m[3][X] = work->center.vx ;
    (*o)->world.m[3][Y] = work->center.vy ;
    (*o)->world.m[3][Z] = work->center.vz ;
    set_double_buffer( *o++, m ) ;
}










int WTR_InitModelsAndPrims( Work *work )
{
    DG_TEX *t ;

    /* Main Wave Shape */
    t = DG_GetTexture( GV_StrCode( "wave07" ) ) ;
    init_model( &work->m_wave, NULL,
		WATR_WAVE_COL, WATR_WAVE_ROW,
		t,
		WATR_TYPE,
		MDL_INIT_ALL2
	) ;
    init_obj_mesh( work, &work->m_wave, WATR_FLAG, work->x_blk, work->z_blk ) ;
#if 0
    t = DG_GetTexture( GV_StrCode( "wave03" ) ) ;
    t->alpha = SCE_GS_SET_ALPHA(0,1,2,1,64) ;
    init_model( &work->m_bubl, &work->m_wave,
		WATR_WAVE_COL, WATR_WAVE_ROW,
		t,
		WATR_TYPE|DG_TYPE_TRANS,
		MDL_INIT_UVS2|MDL_INIT_TYP|MDL_INIT_INI
	) ;
    init_obj_mesh( work, &work->m_bubl, WATR_FLAG, work->x_blk, work->z_blk ) ;
#endif

    return 1 ;
}
