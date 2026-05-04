/*
  Event Routines

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: output.c,v 1.6 2002/06/14 05:21:25 usr04098 Exp $
  */
#ifndef _WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "alltypes.h"


#ifdef _XBOX /* 偶数でなければならない */
#define N_VERTS 6
#else
#define N_VERTS 10
#endif


KMS2_DEF *make_kms( Xdir *p )
{
    KMS2_DEF *def ;
    KMS2_MDL *mdl ;
    KMS2_MDLPACK *packs ;
    int  i, j ;
    int n_verts, n_index, all_verts ;
    FVECTOR min, max, cen, *t  ;
    short   *ids ;
    static SVECTOR vt[] = {
	{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },
	{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },
	{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },{ 0,0,0,4096 },
    } ;
    static SVECTOR nr[] = {
	{ 0,4096,0,0x8fff }, { 0,4096,0,0x8fff }, { 0,4096,0,0x0fff }, { 0,0,0,0x0fff },
	{ 0,4096,0,0x0fff }, { 0,4096,0,0x0fff }, { 0,4096,0,0x0fff }, { 0,0,0,0x0fff },
	{ 0,4096,0,0x0fff }, { 0,4096,0,0x0fff }, { 0,4096,0,0x0fff }, { 0,0,0,0x0fff },
    } ;
    static short uv[] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    } ;
#ifdef _XBOX
    static short idx[] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
    } ;
    KMSS_VERTEX *vbuff ;
#else
    SVECTOR *v, *n, *u ;
#endif

    all_verts= N_VERTS * n_poly ; /* 偶数でなければならない */

    if ( !(def = calloc( 1, (sizeof(KMS2_DEF) +
			     sizeof(KMS2_MDL)     * n_poly +
			     sizeof(KMS2_MDLPACK) * n_poly +
#ifdef _XBOX
			     sizeof(short)        * 10* n_poly +
			     sizeof(KMSS_VERTEX  )* N_VERTS*n_poly
#else
			     sizeof(SVECTOR)      * all_verts *3
#endif
	)  )) )
	return NULL ;

#ifdef _XBOX
    def->data_format = MGS_MODEL_NORM | MGS_MODEL_FLAG_INDEX ;
#else
    def->data_format = MGS_MODEL_NORM ;
#endif
    def->n_models    = n_poly ;
    def->n_x_models  = n_poly ;
    def->texture     = tri_id ;
    mdl = def->models ;
    packs = (KMS2_MDLPACK *)(mdl + n_poly) ;
#ifdef _XBOX
    vbuff = (KMSS_VERTEX *)(packs + n_poly) ;
    ids   = (short   *)(vbuff + N_VERTS*n_poly) ;
#else
    v     = (SVECTOR *)(packs + n_poly) ;
    n     = (SVECTOR *)(v + all_verts) ;
    u     = (SVECTOR *)(n + all_verts) ;
#endif
    for ( i=n_poly ; --i>=0 ; mdl++ )
    {
	if ( poly[i].id[3] != -1 )
	    fprintf( stderr, "sqaure Polygon not supported.\n" ) ;
	n_verts = 3 ;

	/* 頂点のバウンディング計算 */
	cen.vx = cen.vy = cen.vz =  0 ;
	min.vx = min.vy = min.vz =  3000.0f ;
	max.vx = max.vy = max.vz = -3000.0f ;
	for ( j=n_verts ; --j>=0 ; )
	{
	    t = &vrtx[poly[i].id[j]] ;
	    cen.vx += t->vx ;
	    cen.vy += t->vy ;
	    cen.vz += t->vz ;
	    min.vx = min.vx>t->vx ? t->vx : min.vx ;
	    min.vy = min.vy>t->vy ? t->vy : min.vy ;
	    min.vz = min.vz>t->vz ? t->vz : min.vz ;
	    max.vx = max.vx<t->vx ? t->vx : max.vx ;
	    max.vy = max.vy<t->vy ? t->vy : max.vy ;
	    max.vz = max.vz<t->vz ? t->vz : max.vz ;
	}
	cen.vx /= (float)n_verts ;
	cen.vy /= (float)n_verts ;
	cen.vz /= (float)n_verts ;

	n_verts = create_bevel( &poly[i], vt, nr, uv, 
#ifdef _XBOX
				idx,
#endif
				&cen, thick, Z ) ;/* ベヴェル処理 */
	max.vz -= thick ;
	min.vz += thick ;

	n_index = n_verts >> 16 ;
	n_verts &= 0xffff ;

	/* KMS2_MDLの初期化 */
#ifdef _XBOX
	mdl->type    = DG_TYPE_GT4 |DG_TYPE_OVERLAY0/* | DG_TYPE_EXTEND*/ ;
	mdl->vbuff   = vbuff ;
	mdl->n_verts = N_VERTS ;
	mdl->stride  = sizeof(KMSS_VERTEX) ;
#else
	mdl->type    = DG_TYPE_GT4 |DG_TYPE_OVERLAY0 ;
#endif
	mdl->n_packs = 1 ;
	mdl->lx = min.vx - cen.vx ; mdl->ux = max.vx - cen.vx ; mdl->tx = cen.vx - p->center.vx ;
	mdl->ly = min.vy - cen.vy ; mdl->uy = max.vy - cen.vy ;	mdl->ty = cen.vy - p->center.vy ;
	mdl->lz = min.vz - cen.vz ; mdl->uz = max.vz - cen.vz ;	mdl->tz = cen.vz - p->center.vz ;
	mdl->parent  = -1 ;

	/* KMS2_MDLPACKのアドレス初期化 */
	mdl->packs          = packs ;
	mdl->packs->flag    = DG_PACKFLAG_TEX0| DG_PACKFLAG_UV0 ;
#ifdef _XBOX
	mdl->packs->n_verts = 0 ;
	mdl->packs->n_indices = n_index ;
	mdl->packs->verts   = mdl->packs->norms   = 
	mdl->packs->uvs[0]  = mdl->packs->uvs[1]  = mdl->packs->uvs[2] = NULL ;
#else
	mdl->packs->n_verts = n_verts ;
	mdl->packs->verts   = (short *)v ;
	mdl->packs->norms   = (short *)n ;
	mdl->packs->uvs[0]  = (short *)u ;
	mdl->packs->uvs[1]  = mdl->packs->uvs[2] = NULL ;
#endif

#ifdef _XBOX
	mdl->packs->index   = (short *)ids ;
#endif
	mdl->packs->rgbs    = NULL ;
	mdl->packs->tex_id[0] = tex_id ;
	mdl->packs->tex_id[1] = mdl->packs->tex_id[2] = 0 ;

	/* KMS2_MDLPACKのデータ初期化 */
#ifdef _XBOX
	memcpy( mdl->packs->index, idx, sizeof(short)* n_index ) ;
	for ( j=0 ; j<N_VERTS ; j++ )
	{
	    vbuff->vx = vt[j].vx ;
	    vbuff->vy = vt[j].vy ;
	    vbuff->vz = vt[j].vz ;
	    vbuff->wt = 4096 ;

	    vbuff->nx = nr[j].vx ;
	    vbuff->ny = nr[j].vy ;
	    vbuff->nz = nr[j].vz ;

	    vbuff->u0 = uv[j*2+0] ;
	    vbuff->v0 = uv[j*2+1] ;
	    vbuff++ ;
	}
	ids += n_index   ;
#else
	memcpy( mdl->packs->verts , vt, sizeof(SVECTOR)*n_verts   ) ;
	memcpy( mdl->packs->norms , nr, sizeof(SVECTOR)*n_verts   ) ;
	memcpy( mdl->packs->uvs[0], uv, sizeof(SVECTOR)*n_verts/2 ) ;
	v   += n_verts   ;
	n   += n_verts   ;
	u   += n_verts/2 ;
#endif
	packs++ ;
    }

    return def ;
}


int file_out( KMS2_DEF *def )
{
    FILE *fp ;
    KMS2_MDLPACK *packs[MAX_POLY], pack ;
    int  i ;
#ifdef _XBOX
    short       *idx   ;
    KMSS_VERTEX *vbuff ;
#else
    SVECTOR *v, *n ;
    short   *u ;
#endif

    if ( !def )
    {
	fprintf( stderr, "Cannot create KMS.(May be out of memory.)\n" ) ;
	return -1 ;
    }
    if ( !(fp = fopen( fname, "wb" )) )
    {
	fprintf( stderr, "Cannot open file name<%s>.\n", fname ) ;
	free( def ) ;
	return -1 ;
    }

#ifdef _XBOX
    idx   = (short   *)def->models[0].packs->index ;
    vbuff = def->models[0].vbuff ;
#else
    v = (SVECTOR *)def->models[0].packs->verts  ;
    n = (SVECTOR *)def->models[0].packs->norms  ;
    u = (short   *)def->models[0].packs->uvs[0] ;
#endif
    for ( i=n_poly ; --i>=0 ; )
    {
	packs[i] = def->models[i].packs ;
	def->models[i].packs = (KMS2_MDLPACK *)( (int)packs[i] - (int)def ) ;/*先頭からの差分*/
#ifdef _XBOX
	def->models[i].vbuff = (KMS2_MDLPACK *)( (int)def->models[i].vbuff - (int)def ) ;
#endif
    }
    fwrite( def, sizeof(KMS2_DEF) + sizeof(KMS2_MDL)*n_poly, 1, fp ) ;

    for ( i=0 ; i<n_poly ; i++ )
    {
	pack = *packs[i] ;
#ifdef _XBOX
	packs[i]->index = (short *)( (int)packs[i]->index   - (int)def ) ;
	packs[i]->verts  = packs[i]->norms  = 
	packs[i]->uvs[0] = packs[i]->uvs[1] = packs[i]->uvs[2] = NULL ;
#else
	packs[i]->verts  = (short *)( (int)packs[i]->verts  - (int)def ) ;
	packs[i]->norms  = (short *)( (int)packs[i]->norms  - (int)def ) ;
	packs[i]->uvs[0] = (short *)( (int)packs[i]->uvs[0] - (int)def ) ;
#endif
    }

    fwrite( packs[0], sizeof(KMS2_MDLPACK), n_poly, fp ) ;
#ifdef _XBOX
    fwrite( vbuff, sizeof(KMSS_VERTEX), N_VERTS*n_poly, fp ) ;
    fwrite( idx, sizeof(short), 10*n_poly, fp ) ;
#else
    fwrite( v, sizeof(SVECTOR), 10*n_poly, fp ) ;
    fwrite( n, sizeof(SVECTOR), 10*n_poly, fp ) ;
    fwrite( u, sizeof(short)*2, 10*n_poly, fp ) ;
#endif
    fclose( fp ) ;

    free( def ) ;

    return 0 ;
}

int output( Xdir *p )
{
    printf( "file out has started.<%s>\n", fname ) ;
    if ( thick < 0 )
	printf( "Do not create UV\n" ) ;
    if ( file_out( make_kms( p ) ) < 0 )
	return -1 ;
    printf( "done.\n" ) ;
    return 0 ;
}
