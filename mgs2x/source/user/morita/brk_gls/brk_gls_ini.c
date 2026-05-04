//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_ini.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_gls_ini.c,v 1.1.1.3 2002/11/19 11:45:29 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "def_dma.h"

#include "brk_glass.h"



#define SIZEOF_QWORD(_v)	(sizeof(_v)/sizeof(u_long128))

#define PRIORITY_MIRROR_MASK	(160)

#if 0 //BP_PS2 def PSX2 //BP
//#ifdef __GNUC__	

static SCREEN_DRAW def_screen_draw = {
   /* .dmatag = */ {
      /*.qwc = */ DMATAG_SET_QWC( DMATAG_ID_RET, SIZEOF_QWORD(struct _gif_packet) ),
      /* .addr = */ NULL,
	   /* .vifcode = */ {
	    SCE_VIF1_SET_NOP( 0 ),
	    SCE_VIF1_SET_DIRECT( SIZEOF_QWORD(struct _gif_packet), 0)
	   }
   },
   /* .gif_packet = */ {
	   /* .giftag = */ { /* .tag = */ SCE_GIF_SET_TAG(SIZEOF_QWORD(struct _gif_data), 1, 0, 0, 0, 1), /* .regs = */ GS_REGS_AD },
	   /* .gif_data = */ {
	    /* .alpha = */ { /* .reg = */ SCE_GS_ALPHA_1, /* .data = */ SCE_GS_SET_ALPHA(1,1,2,1,0x80) },
	    /* .clamp = */ { /* .reg = */ SCE_GS_CLAMP_1, /* .data = */ SCE_GS_SET_CLAMP(2,2,0,DRAW_WIDTH-1,1,DRAW_HEIGHT-2) },
	    /* .tex0  = */ { /* .reg = */ SCE_GS_TEX0_1,  /* .data = */ SCE_GS_SET_TEX0(0,8,0,9,8,0,0,0,0,0,0,0) },
	    /* .test1 = */ { /* .reg = */ SCE_GS_TEST_1,  /* .data = */ SCE_GS_SET_TEST(0,0,0,0,0,0,1,1) },
	    /* .prim  = */ { /* .reg = */ SCE_GS_PRIM,    /* .data = */ SCE_GS_SET_PRIM(SCE_GS_PRIM_SPRITE,0,0,0,1,0,1,0,0) },
	    /* .rgbq  = */ { /* .reg = */ SCE_GS_RGBAQ,   /* .data = */ SCE_GS_SET_RGBAQ(128,128,128,128,0) },
	    /* .uv0   = */ { /* .reg = */ SCE_GS_UV,      /* .data = */ SCE_GS_SET_UV(0+8,0+8) },
	    /* .xyz0  = */ { /* .reg = */ SCE_GS_XYZ2,
		       /* .data = */ SCE_GS_SET_XYZ( ((2048-DRAW_WIDTH/2)*16),
					       ((2048-DRAW_HEIGHT/2)*16),
					       0xffffffff) },
	    /* .uv1   = */ { /* .reg = */ SCE_GS_UV,      /* .data = */ SCE_GS_SET_UV(DRAW_WIDTH*16+16-8,DRAW_HEIGHT*16+16-8)},
	    /* .xyz1  = */ { /* .reg = */ SCE_GS_XYZ2,
		       /* .data = */ SCE_GS_SET_XYZ( ((2048+DRAW_WIDTH/2)*16),
					       ((2048+DRAW_HEIGHT/2)*16),
					       0xffffffff) },
	    /* .test2 = */ { /* .reg = */ SCE_GS_TEST_1,  /* .data = */ SCE_GS_SET_TEST( 1, 5, 64, 1, 0, 0, 1, 1 ) },
	}
    }
} ;

#endif

static int InitDmapack( Work *work )
{
    DG_DMAPACK	*d ;

    work->dmapack = d =
	(DG_DMAPACK *)DG_MakeDmapack2( DG_DMAPACK_NORMAL,
				       DG_PLUGIN_PHASE_AFTER,
				       PRIORITY_MIRROR_MASK ) ;
    if( d == NULL )
	return -1 ;
    DG_QueueDmapack( d ) ;
    work->dmapack->flag |= DG_DMAPACK_INVISIBLEMENU|DG_DMAPACK_INVISIBLE0 ;

#if 0 //BP_PS2 def PSX2
    work->packet = GV_Malloc( sizeof(SCREEN_DRAW) * 2 ) ;
    if ( work->packet )
    {
		SCREEN_DRAW	*p= work->packet ;

      memcpy( d->packet[0] = &p[0], &def_screen_draw, sizeof(SCREEN_DRAW) ) ;
		memcpy( d->packet[1] = &p[1], &def_screen_draw, sizeof(SCREEN_DRAW) ) ;

		return 0 ;
    }
#else
    work->packet = GV_Malloc( sizeof(int)*16 ) ;
    if ( work->packet )
    {
		void	*p = work->packet ;

		p = DG_SetDmapackAlpha( p, SCE_GS_SET_ALPHA(0,0,2,1,0x80) ) ;
		p = DG_SetDmapackBox( p, 
							  0,0, DRAW_WIDTH, DRAW_HEIGHT,
							  DG_MakeDmaPackColorFromInt(0x80808080) ) ;
		p = DG_SetDmapackEnd( p );
		d->autopacket = work->packet ;
		
		return 0 ;
    }
#endif
    return -1 ;
}



/* モデル内のポリゴン数を数える  */
static int CountPolygon( DG_MDL *mdl )
{
    int      i, j ;
    int      n_poly = 0 ;
#ifdef PSX2
    SVECTOR *n ;
#endif

    for ( j=mdl->n_packs ; --j>=0 ; )
    {
#ifdef PSX2
	n = (SVECTOR *)mdl->packs[j].norms ;
	for ( i=mdl->packs[j].n_verts ; --i>=0 ; n++ )
	    if ( !(n->pad & 0x8000) )
		n_poly++ ;
#else
		int v0, v1, v2 ;

		v0 = v1 = v2 = *(short*)mdl->packs[j].index ;
		for ( i=0 ; i<mdl->packs[j].n_indices ; i++ ) {
			v2 = v1 ;
			v1 = v0 ;
			v0 = *((short*)mdl->packs[j].index + i) ;
			if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
				n_poly++ ;
			}
		}
#endif
    }
    return n_poly ;
}

/* ポリゴンq の共有辺を捜す  */
static POLY* SearchPolydata( Work *work, POLY *q, FVECTOR *v0, FVECTOR *v1 )
{
    POLY *p = work->poly   ;
    int   i = work->n_poly ;

    for( ; --i>=0 ; p++ )
	if ( p != q )
	    if ( (p->v0==v0 && (p->v1==v1 || p->v2==v1)) ||
		 (p->v1==v0 && (p->v0==v1 || p->v2==v1)) ||
		 (p->v2==v0 && (p->v0==v1 || p->v1==v1)) )
		return p ;
    return NULL ;
}

/*  */
int BRK_GLS_SetupPolydata( Work *work )
{
    int       i, j ;
    POLY     *p ;
    CV2_MDL  *c   = work->cdef->models ;
    DG_MDL   *mdl = work->def->models  ;
#ifdef PSX2
    short    *idx = c->verts_index ;
#endif

    /* ポリゴン数を調べて,メモリ確保 */
    if ( !(work->n_poly = CountPolygon( mdl )) )
	PERROR( "No polygon-mesh illeagal model : NewPutGlassObject\n" ) ;
    if ( !(p = work->poly = GV_Malloc( sizeof(POLY) * work->n_poly )) )
	PERROR( "No memory Polygon-Data : NewPutGlassObject\n" ) ;

    /* 頂点の共有情報 cv2とmdlからポリゴン情報を収集する */
    for( j=0 ; j<(int)mdl->n_packs ; j++ )
    {
#ifdef PSX2
    SVECTOR  *n    ;

	n = (SVECTOR *)mdl->packs[j].norms ;
	for( i=mdl->packs[j].n_verts ; --i>=0 ; n++, idx++ )
	    if ( !(n->pad & 0x8000) )
	    {
		p->flag = 0 ;
		p->v0 = &c->verts[p->i0 = *(idx-0)] ;
		p->v1 = &c->verts[p->i1 = *(idx-1)] ;
		p->v2 = &c->verts[p->i2 = *(idx-2)] ;
		p->v0->vz = p->v1->vz = p->v2->vz = 0.0f ;
		p++ ;
	    }
	if ( mdl->packs[j].n_verts & 1 )
	    idx++ ;
#else
		int v0, v1, v2 ;

		v0 = v1 = v2 = *(short*)mdl->packs[j].index ;
		for ( i=0 ; i<mdl->packs[j].n_indices ; i++ ) {
			v2 = v1 ;
			v1 = v0 ;
			v0 =*((short*)mdl->packs[j].index + i) ;
			if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
				p->flag = 0 ;
				p->v0 = &c->verts[p->i0 = v0] ;
				p->v1 = &c->verts[p->i1 = v1] ;
				p->v2 = &c->verts[p->i2 = v2] ;
				p->v0->vz = p->v1->vz = p->v2->vz = 0.0f ;
				p++ ;
			}

		}
#endif
    }

    /* 辺の共有情報 */
    for( p=work->poly, j=work->n_poly ; --j>=0 ; p++ )
    {
	p->p0 = SearchPolydata( work, p, p->v0, p->v1 ) ;
	p->p1 = SearchPolydata( work, p, p->v1, p->v2 ) ;
	p->p2 = SearchPolydata( work, p, p->v2, p->v0 ) ;
    }
    return 0 ;
}

static void MakeBrokenPrim( Work *work,
			    DG_PRIM2 **bro, u_long64 bro_alpha,
			    DG_PRIM2 **thi , u_long64 thi_alpha,
			    POLY   *p, int n_poly, int flag,
			    DG_TEX *t, int rgba, float thickness )
{
    int             i, j ;
    DG_PRIM2_UVRGB *f_gls_u, *b_gls_u, *thick_u ;
    FVECTOR        *f_gls_p, *b_gls_p, *thick_p ;
    FVECTOR         v, *vert ;
    float           size_x, size_y, pos_x, pos_y ;
    float  face ;
    //int    flag ;

    size_x = work->def->models->ux - work->def->models->lx ;
    size_y = work->def->models->uy - work->def->models->ly ;

    flag |= DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_SINGLEBUFF|DG_PRIM2_ALPHA | DG_PRIM2_NOMSAA;
    if ( t )
	flag |= DG_PRIM2_TEX ;
    if ( !(*thi = GM_MakePrim2( flag & ~DG_PRIM2_NOBUFFER, n_poly, BRK_GLS_NVERTS+1 ) ) )
	return ;
    if ( t )
	DG_ConfigPrim2Tex( *thi, t ) ;
    DG_SetPrim2Alpha( *thi, thi_alpha ) ;
    DG_InvisiblePrim2( *thi ) ;

    flag |= DG_PRIM2_CULLPOLY ;
    if ( !(*bro = GM_MakePrim2( flag & ~DG_PRIM2_NOBUFFER, n_poly, BRK_GLS_NVERTS*2 ) ) )
	return ;
    if ( t )
	DG_ConfigPrim2Tex( *bro, t ) ;
    DG_SetPrim2Alpha( *bro, bro_alpha ) ;
    DG_InvisiblePrim2( *bro ) ;

    if ( !(flag & DG_PRIM2_NOBUFFER) && t )
	for( i=0 ; i<n_poly ; i++, p++ )
	{
	    f_gls_u = (*bro)->uvrgb[0], f_gls_u +=  i*2   *BRK_GLS_NVERTS   ;
	    f_gls_p = (*bro)->pos  [0], f_gls_p +=  i*2   *BRK_GLS_NVERTS   ;
	    b_gls_u = (*bro)->uvrgb[0], b_gls_u += (i*2+1)*BRK_GLS_NVERTS   ;
	    b_gls_p = (*bro)->pos  [0], b_gls_p += (i*2+1)*BRK_GLS_NVERTS   ;
	    thick_u = (*thi)->uvrgb[0], thick_u +=  i * (BRK_GLS_NVERTS+1) ;
	    thick_p = (*thi)->pos  [0], thick_p +=  i * (BRK_GLS_NVERTS+1) ;
	    if ( t )
	    {
		p->f_gls_p = f_gls_p, p->f_gls_u = f_gls_u ;
		p->b_gls_p = b_gls_p, p->b_gls_u = b_gls_u ;
		p->thick_p = thick_p, p->thick_u = thick_u ;
	    }

	    /* 表向き？裏向き？ */
	    face = BackFaceCullingXY( p->v0, p->v1, p->v2 ) ;

	    for( j=0 ; j<BRK_GLS_NVERTS ; j++ )
	    {
		/* 基本描画設定 */
		thick_u->q = f_gls_u->q = b_gls_u->q = 4096 ;
		f_gls_u->f = (!j || j&1 ? 0x8fff  : /*疑似ファンストリップ*/
			      face<0.0f ? 0x0000  : /*左回り*/
			      0x0020) ; /*右回り*/
		b_gls_u->f = (!j || j&1 ? 0x8fff  : /*疑似ファンストリップ*/
			      face<0.0f ? 0x0020  : /*右回り*/
			      0x0000) ; /*左回り*/
		thick_u->f = j<=1 ? 0x8fff : 0x0fff ;
		f_gls_u->r = b_gls_u->r = thick_u->r = (rgba >> 0 ) & 0xff ;
		f_gls_u->g = b_gls_u->g = thick_u->g = (rgba >> 8 ) & 0xff ;
		f_gls_u->b = b_gls_u->b = thick_u->b = (rgba >> 16) & 0xff ;
		f_gls_u->a = b_gls_u->a = thick_u->a = (rgba >> 24) & 0xff ;

		/* 形を作る */
		vert = (!j ? p->v0 : j&1 ? p->v1 : p->v2) ;
		_sceVu0ApplyMatrix( b_gls_p, &work->world, vert ) ;
		_sceVu0ScaleVector( &v, &work->world.m[Z], thickness ) ;
		_sceVu0AddVector( f_gls_p, b_gls_p, &v ) ;
		_sceVu0SubVector( b_gls_p, b_gls_p, &v ) ;
		_sceVu0CopyVector( thick_p, (*bro)->pos[0] ) ;

		/* テクスチャーUV設定 */
		if ( t )
		{
		    pos_x = vert->vx - work->def->models->lx ;
		    pos_y = vert->vy - work->def->models->ly ;
		    f_gls_u->u = FTOI12(pos_x/size_x*t->u_scale + t->u_offset);
		    f_gls_u->v = FTOI12(pos_y/size_y*t->v_scale + t->v_offset);
		    thick_u->u = FTOI12( 0.5F*t->u_scale + t->u_offset ) ;
		    thick_u->v = FTOI12( 0.5F*t->v_scale + t->v_offset ) ;
		    b_gls_u->u = f_gls_u->u ;
		    b_gls_u->v = f_gls_u->v ;
		}
		f_gls_u++, f_gls_p++ ;
		b_gls_u++, b_gls_p++ ;
		thick_u++, thick_p++ ;
	    }
	    /* 厚みprimは,1頂点多くなる */
	    thick_u->q = 4096   ;
	    thick_u->f = 0x0fff ;
	    thick_u->r = (rgba >> 0 ) & 0xff ;
	    thick_u->g = (rgba >> 8 ) & 0xff ;
	    thick_u->b = (rgba >> 16) & 0xff ;
	    thick_u->a = (rgba >> 24) & 0xff ;
	    thick_u->u = FTOI12( 0.5F*t->u_scale + t->u_offset ) ;
	    thick_u->v = FTOI12( 0.5F*t->v_scale + t->v_offset ) ;
	    _sceVu0CopyVector( thick_p, (*bro)->pos[0] ) ;
	}
}


static DG_PRIM2 *MakeSquarePrim( Work *work, DG_TEX *t,
				 u_long64 alpha, int rgba, float thick )
{
    DG_PRIM2_UVRGB *u ;
    FVECTOR        *p ;
    FVECTOR         v ;
    int             i, flag ;
    DG_PRIM2       *prim ;

    flag = DG_PRIM2_POLY| DG_PRIM2_SHADE| DG_PRIM2_SINGLEBUFF| DG_PRIM2_ALPHA | DG_PRIM2_NOMSAA;
    if ( t )
	flag |= DG_PRIM2_TEX ;
    if ( (prim = GM_MakePrim2( flag, 1, 4 ) ) )
    {
	if ( t )
	    DG_ConfigPrim2Tex( prim, t ) ;
	DG_SetPrim2Alpha( prim, alpha ) ;

	u = prim->uvrgb[0] ;
	p = prim->pos[0]   ;
	v.vw = 1.0f ;
	v.vz = thick ;
	for ( i=0 ; i<4 ; i++ )
	{
	    v.vx = i&1 ? work->def->models->ux : work->def->models->lx ;
	    v.vy = i>1 ? work->def->models->uy : work->def->models->ly ;
	    _sceVu0ApplyMatrix( p, &work->world, &v ) ;
	    u->q = 4096 ;
	    u->f = i>1 ? 0x0fff : 0x8fff ;
	    u->r = (rgba >> 0 ) & 0xff ;
	    u->g = (rgba >> 8 ) & 0xff ;
	    u->b = (rgba >> 16) & 0xff ;
	    u->a = (rgba >> 24) & 0xff ;
	    if ( t )
	    {
		u->u = FTOI12( (i&1? 0.0f:1.0f) * t->u_scale + t->u_offset ) ;
		u->v = FTOI12( (i>1? 0.0f:1.0f) * t->v_scale + t->v_offset ) ;
	    }
	    u++ ;
	    p++ ;
	}
    }
    return prim ;
}


static DG_PRIM2 *MakeMirrorPrim( Work *work, DG_TEX *t,
				 u_long64 alpha, int rgba, float thick )
{
    DG_PRIM2_UVRGB *u ;
    FVECTOR        *p ;
    FVECTOR         v ;
    int             i, flag ;
    DG_PRIM2       *prim ;

#define N_PRIMS 5
    flag = DG_PRIM2_POLY| DG_PRIM2_SHADE| DG_PRIM2_SINGLEBUFF| DG_PRIM2_ALPHA | DG_PRIM2_NOMSAA;
    if ( t )
	flag |= DG_PRIM2_TEX ;
    if ( (prim = GM_MakePrim2( flag, 1, N_PRIMS ) ) )
    {
	if ( t )
	    DG_ConfigPrim2Tex( prim, t ) ;
	DG_SetPrim2Alpha( prim, alpha ) ;

	u = prim->uvrgb[0] ;
	p = prim->pos[0]   ;
	v.vw = 1.0f ;
	v.vz = 0.0f ;
	for ( i=0 ; i<N_PRIMS ; i++ )
	{
	    if ( i < 4 )
	    {
		v.vx = i&1 ? work->def->models->ux : work->def->models->lx ;
		v.vy = i>1 ? work->def->models->uy : work->def->models->ly ;
	    }
	    else
	    {
		v.vx = i==5 ? 20000.0f : -20000.0f ;
		v.vy = 0.0f ;
		v.vz = thick*50000.0f ;
	    }
	    _sceVu0ApplyMatrix( p, &work->world, &v ) ;
	    u->q = 4096 ;
	    u->f = i>1 && i<4 ? 0x0fff : 0x8fff ;
	    u->r = (rgba >> 0 ) & 0xff ;
	    u->g = (rgba >> 8 ) & 0xff ;
	    u->b = (rgba >> 16) & 0xff ;
	    u->a = (rgba >> 24) & 0xff ;
	    if ( t )
	    {
		u->u = FTOI12( (i&1? 0.0f:1.0f) * t->u_scale + t->u_offset ) ;
		u->v = FTOI12( (i>1? 0.0f:1.0f) * t->v_scale + t->v_offset ) ;
	    }
	    u++ ;
	    p++ ;
	}
    }
    return prim ;
}


int BRK_GLS_InitPrimitive( Work *work )
{
    /* 壊れているガラス */
    MakeBrokenPrim( work,
		    &work->broken, SCE_GS_SET_ALPHA(0,1,0,1, 0x00),
		    &work->thick , SCE_GS_SET_ALPHA(0,1,2,1, 0x16),
		    work->poly, work->n_poly, 0,
		    work->broken_tx, 0x7f7f7f7f, work->thickness ) ;
#if DEBUG_MODE
    if ( !work->thick )
	PERROR( "No memory for PRIM2(work->thick) : NewPutGlassObject\n" ) ;
    if ( !work->broken )
	PERROR( "No memory for PRIM2(work->broken) : NewPutGlassObject\n" ) ;
#endif

    /* 壊れていないガラス */
    work->glass = MakeSquarePrim( work, 
				  work->glass_tx,
				  SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ),
				  0x7f7f7f7f,
				  0.0f ) ;

    /* 鏡の場合 */
    if ( work->mirror_tx )
    {
	/* 鏡の裏地に書き込むプリミティブ Chan0->On Chan1->Off
	 デスティネーションのαを使う*/
	work->mirror    = MakeMirrorPrim( work, 
					  work->mirror_tx,
					  SCE_GS_SET_ALPHA( 0, 1, 1, 1, 0x00 ),
					  0x7f7f7f7f,
					  -1.0f ) ;
	DG_InvisiblePrim2( work->mirror ) ;
	work->mirror->raise = 0 ;

#if 0
	/* αを128にするためのプリミティブ Chan0->Off Chan1->On */
	work->mirror_bg = MakeMirrorPrim( work, 
					  /*work->mirror_tx*/NULL,
					  SCE_GS_SET_ALPHA( 1, 1, 2, 1, 0x80 ),
					  0x7f7f7f7f,
					  1.0f ) ;
	DG_InvisiblePrim2( work->mirror_bg ) ;
	work->mirror_bg->raise = 60000 ;
#endif
	InitDmapack( work ) ;

	/* 鏡の場合の他のプリミティブの設定 */
	work->glass->flag |= DG_PRIM2_INVISIBLE1 ;
	work->broken->raise = -20000 ;
	work->thick->raise  = -2000 ;

#if 0
#if 1
	/* 壊れているガラス */
	MakeBrokenPrim( work,
			&work->mirror_br, SCE_GS_SET_ALPHA(0,1,2,1, 0x00),
			&work->mirror_th, SCE_GS_SET_ALPHA(0,1,2,1, 0x00),
			work->poly, work->n_poly, DG_PRIM2_NOBUFFER,
			work->broken_tx/*NULL*/, 0x7f7f7f7f, work->thickness ) ;
	DG_SetPrim2Buffer( work->mirror_br,
			   work->broken->pos[0],   work->broken->pos[1],
			   work->broken->uvrgb[0], work->broken->uvrgb[1] ) ;
	DG_SetPrim2Buffer( work->mirror_th,
			   work->thick->pos[0],   work->thick->pos[1],
			   work->thick->uvrgb[0], work->thick->uvrgb[1] ) ;
	DG_InvisiblePrim2( work->mirror_br ) ;
	DG_InvisiblePrim2( work->mirror_th ) ;
	work->mirror_br->raise = -20000.0f ;
	work->mirror_th->raise = -2000.0f ;
#else

	flag = DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_ALPHA ;
	if ( !(work->mirror_br = GM_MakePrim2( flag|DG_PRIM2_NOBUFFER,
					       n_poly, BRK_GLS_NVERTS*2 ) ) )
	    return ;
	DG_SetPrim2Alpha( work->mirror_br, SCE_GS_SET_ALPHA(0,1,2,1, 0x00) ) ;
	if ( !(work->mirror_th = GM_MakePrim2( flag|DG_PRIM2_NOBUFFER,
					       n_poly, BRK_GLS_NVERTS+1 ) ) )
	    return ;
	DG_SetPrim2Alpha( work->mirror_th, SCE_GS_SET_ALPHA(0,1,2,1, 0x00) ) ;

	DG_SetPrim2Buffer( work->mirror_br,
			   work->broken->pos[0],   work->broken->pos[1],
			   work->broken->uvrgb[0], work->broken->uvrgb[1] ) ;
	DG_SetPrim2Buffer( work->mirror_th,
			   work->thick->pos[0],   work->thick->pos[1],
			   work->thick->uvrgb[0], work->thick->uvrgb[1] ) ;
	DG_InvisiblePrim2( work->mirror_br ) ;
	DG_InvisiblePrim2( work->mirror_th ) ;

#endif
#endif
    }

    return 0 ;
}




int BRK_GLS_InitPolygons( Work *work )
{
    if ( !(work->def  = GV_GetCache( GV_CacheID( work->model_nm, 'k' ) )) )
	PERROR( "Need KMS(%d) : NewPutGlassObject\n", work->model_nm ) ;
    if ( !(work->cdef = GV_GetCache( GV_CacheID( work->model_nm, 'c' ) )) )
	PERROR( "Need CV2(%d) : NewPutGlassObject\n", work->model_nm ) ;

    if ( BRK_GLS_SetupPolydata( work ) < 0 )
	return -1 ;
    if ( BRK_GLS_InitPrimitive( work ) < 0 )
	return -1 ;

    return 0 ;
}

int BRK_GLS_InitTarget( Work *work, int where )
{
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR       uv = { work->def->models->ux,
			 work->def->models->uy,
			 work->def->models->uz, 0.0f } ;
    FVECTOR       lv = { work->def->models->lx,
			 work->def->models->ly,
			 work->def->models->lz, 0.0f } ;
    FVECTOR       t_size, t_pos ;

    _sceVu0SubVector( &t_size, &uv, &lv ) ;
    _sceVu0AddVector( &t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( t, 
		  TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_THROUGH|
		  TARGET_NO_CLAYMORE|TARGET_CALL_CALLBACK_THROUGH_HIT,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality,
		       0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, BRK_GLS_TargetCallBack, work ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2Map( t, &work->world, where ) ;

#if DEBUG_MODE
    if ( GCL_GetOptionValue( 'f', 0 ) )
	NewTargetView( t,  200, 50, 32 ) ;
#endif

    return 0 ;
}

int BRK_GLS_InitHazard( Work *work, int where )
{
    int hzx_id = GCL_GetOptionValue( 'D', BRK_HZD_W16A_DASHBOARD ) ;

    work->hzd = BRK_InitHazard( hzx_id ) ;

    return 0 ;
}

#if 0
int BRK_GLS_InitPieces( Work *work, int where )
{
    int           i   ;
    DG_DEF       *def ;
    DG_COMDL_POS *pos_s, *pos_l, *pos ;

    work->n_piece = BRK_GLS_INACTIVE ;
    if ( !work->lpiece_nm )
	return 0 ;

    if ( !(def = GV_GetCache( GV_CacheID( work->lpiece_nm, 'k' ) )) )
	PERROR( "No Model for COMDL(%d) in data.cnf : NewPutGlassObject\n", work->lpiece_nm ) ;
    if ( !(work->piece_l = DG_MakeComdl( def->models[0].packs,
					 DG_COMDL_SEMITRANS,
					 BRK_N_LPIECE, 0 )) )
	PERROR( "No memory for COMDL(work->piece_l) : NewPutGlassObject\n" ) ;
    DG_QueueComdlObjs( work->piece_l ) ;
    GM_GroupObject( work->piece_l, where ) ;
    work->piece_l->flag |= DG_FLAG_INVISIBLE ;

    if ( !(work->piece_s = DG_MakeComdl( def->models[1].packs,
					 DG_COMDL_SEMITRANS,
					 BRK_N_SPIECE, 0)) )
	PERROR( "No memory for COMDL(work->piece_s) : NewPutGlassObject\n" ) ;
    DG_QueueComdlObjs( work->piece_s ) ;
    GM_GroupObject( work->piece_s, where ) ;
    work->piece_s->flag |= DG_FLAG_INVISIBLE ;

    pos_s = work->piece_s->pos ;
    pos_l = work->piece_l->pos ;
    for ( i=0 ; i<BRK_N_PIECE ; i++ )
    {
	if ( !(i&3) )
	    pos = pos_l++ ;
	else
	    pos = pos_s++ ;
	work->piece[i].comdl = pos ;
	work->piece[i].flag  = 0   ;
        pos->world    = DG_UnitMatrix ;
        pos->color.vx = 128,pos->color.vy = pos->color.vz = 0 ;
        pos->color.vw = 0 ;
    }

    return 0 ;
}
#endif

int BRK_GLS_FogMist( Work *work, int where )
{
    FVECTOR center ;
    extern void *NewGlassMistMain2( FVECTOR *center, FVECTOR *housen,
				    float width, float height ) ;

    _sceVu0ApplyMatrix( &center, &work->world, &work->target.offset ) ;
    _sceVu0AddVector( &center, &center, (FVECTOR *)work->world.m[W] ) ;
    work->fog_mist =
	NewGlassMistMain2( &center,
			   (FVECTOR *)work->world.m[Z],
			   work->target.size.vx,
			   work->target.size.vy ) ;
    return  0 ;
}

void BRK_GLS_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->target.offset, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->target.offset,
		   addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_GLS_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_GLS_BreakAtRandom( work, 0 ) ;
}

int BRK_GLS_GetOptions( Work *work, int where )
{
    FVECTOR pos ;
    IVECTOR buf ;
    extern int BRK_PCE_nModels ;

    BRK_PCE_nModels = 0 ;
    if ( GCL_GetOption( 'r' ) )
    {
	GCL_GetIV( GCL_NextStr(), (int *)&buf ) ;
	_sceVu0RotMatrixZ( &work->world, &DG_UnitMatrix, buf.vz*M_PI/2048.0f ) ;
	_sceVu0RotMatrixX( &work->world, &work->world  , buf.vx*M_PI/2048.0f ) ;
	_sceVu0RotMatrixY( &work->world, &work->world  , buf.vy*M_PI/2048.0f ) ;
    }
    else
	_sceVu0CopyMatrix( &work->world, &DG_UnitMatrix ) ;

    if ( GCL_GetOption( 'm' ) )
    {
	work->model_nm  = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->lpiece_nm = GCL_GetNextInt() ;
    }

    if ( GCL_GetOption( 't' ) )
    {
	work->glass_tx  = DG_GetTexture( GCL_GetNextInt() ) ;
	work->broken_tx = DG_GetTexture( GCL_GetNextInt() ) ;
	if ( GCL_NextStr() )
	    work->mirror_tx = DG_GetTexture( GCL_GetNextInt() ) ;
    }

    if ( GCL_GetOption( 'O' ) )
	work->proc = GCL_GetNextInt() ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetNextIV( (int *)&buf ) ;
	vu0_IV0toFV( &buf, &pos ) ;
	_sceVu0CopyVectorXYZ( (FVECTOR *)work->world.m[W], &pos ) ;
    }

    work->thickness = (float)GCL_GetOptionValue( 'K', 10 ) ;

    return 0 ;
}

