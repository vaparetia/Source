/*
   MDU_mdlUtil.c

   モデルユーティリティ for MGS2 on linux

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/1999

   $Id: MDU_mdlUtil.c,v 1.11 2001/07/06 04:31:49 usr04098 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_kms.h"
#include "fmt_km4.h"
#include "MDU_util.h"

/*-------------------------------------------------------------*/

/* ｋｍｓ内部テクスチャＩＤを変更 */
static	int	FindTexIDfromOld( id, olds, n )
long		id ;
long		*olds ;
int		n ;
{
    int		i ;
    for ( i = 0; i < n; i ++ ) {
	if ( id == *( olds ++ ) ) return i ;
    }
    return -1 ;
}

void		MDU_KmsChangeTexID( sdef, olds, news, n ) 
KMS_DEF		*sdef ;
char		**olds ;
char		**news ;
int		n ;
{
    KMS_OBJ	*obj ;
    KMS_PKT	*pkt ;
    long	oldid[ 4096 ], newid[ 4096 ] ;
    int		n_x_models, n_packs, i, hit ;

    for ( i = 0; i < n; i ++ ) {
	oldid[ i ] = MDU_GetStrCode( olds[ i ] ) ;
	newid[ i ] = MDU_GetStrCode( news[ i ] ) ;
    }
    n_x_models = sdef->n_x_models ;
    obj = sdef->objs ;
    while( -- n_x_models >= 0 ) {
	n_packs = obj->n_packs ;
	pkt = obj->pack ;
	while( -- n_packs >= 0 ) {
	    hit = FindTexIDfromOld( pkt->tid, oldid, n ) ;
	    if ( hit >= 0 ) {
printf( "id change! %ld -> %ld\n", pkt->tid, newid[ hit ] ) ;
		pkt->tid = newid[ hit ] ;
	    }
	    pkt ++ ;
	}
	obj ++ ;
    }
}


void		MDU_Kms2ChangeTexID( sdef, olds, news, n ) 
KMS2_DEF	*sdef ;
char		**olds ;
char		**news ;
int		n ;
{
    KMS2_MDL     *mdl ;
    KMS2_MDLPACK *pack ;
    long	oldid[ 4096 ], newid[ 4096 ] ;
    int		n_x_models, n_packs, hit ;
    int         i ;

printf( "nums %d\n", n ) ;
    for ( i = 0; i < n; i ++ ) {
	oldid[ i ] = MDU_GetStrCode( olds[ i ] ) ;
	newid[ i ] = MDU_GetStrCode( news[ i ] ) ;
    }

    n_x_models = sdef->n_x_models ;
    mdl = sdef->models ;
    while( -- n_x_models >= 0 ) {
	n_packs = mdl->n_packs ;
	pack = mdl->packs ;
	while( -- n_packs >= 0 ) {
	    hit = FindTexIDfromOld( pack->tex_id[0], oldid, n ) ;
	    if ( hit >= 0 ) {
printf( "id change! %d -> %ld\n", pack->tex_id[0], newid[ hit ] ) ;
		pack->tex_id[0] = newid[ hit ] ;
	    }
	    pack ++ ;
	}
	mdl ++ ;
    }
}

/* 頂点＆法線を共有化する */
void		MDU_JointModel( def )
KM3_DEF		*def ;
{
    FVECTOR	*verts, *norms, *verts2, *norms2, *v, *n ;
    FVECTOR	*envs, *envs2, *nenvs ;
    short	*vid, *nid, *vid2, *nid2, *nvid, *nnid ;
    int		n_verts2, n_norms2 ;
    KM3_MDL	*mdl ;
    int		n_prims, n_models, i, j ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    while( -- n_models >= 0 ) {
	n_verts2 = n_norms2 = 0 ;
	n_prims = mdl->n_prims ;
	verts = mdl->verts ;
	envs = mdl->envs ;
	norms = mdl->norms ;
	vid = mdl->vid ;
	nid = mdl->nid ;
	verts2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
	norms2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;    
	nvid = vid2 = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_verts * 4 ) ;
	nnid = nid2 = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_norms * 4 ) ;
	if ( envs != NULL ) {
	    nenvs = envs2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
	}
	while( -- n_prims >= 0 ) {
	    for ( i = 0; i < 4; i ++ ) {
		v = verts + vid[ i ] ;
		for ( j = 0; j < n_verts2; j ++ ) {
		    if ( v->vx == verts2[ j ].vx &&
			v->vy == verts2[ j ].vy &&
			v->vz == verts2[ j ].vz &&	
			v->vw == verts2[ j ].vw ) {
			vid2[ i ] = j ;
			break ;
		    }
		}
		if ( j != n_verts2 ) continue ;
		verts2[ n_verts2 ] = *v ;
		if ( envs != NULL ) {
		    envs2[ n_verts2 ] = *( envs + vid[ i ] ) ;
		}
		vid2[ i ] = n_verts2 ;
		n_verts2 ++ ;
	    }
	    for ( i = 0; i < 4; i ++ ) {
		n = norms + nid[ i ] ;
		for ( j = 0; j < n_norms2; j ++ ) {
		    if ( n->vx == norms2[ j ].vx &&
			n->vy == norms2[ j ].vy &&
			n->vz == norms2[ j ].vz &&	
			n->vw == norms2[ j ].vw ) {
			nid2[ i ] = j ;
			break ;
		    }
		}
		if ( j != n_norms2 ) continue ;
		norms2[ n_norms2 ] = *n ;
		nid2[ i ] = n_norms2 ;
		n_norms2 ++ ;
	    }
	    vid += 4 ;
	    nid += 4 ;
	    vid2 += 4 ;
	    nid2 += 4 ;
	}
	mdl->n_verts = n_verts2 ;
	mdl->n_norms = n_norms2 ;	
	MDU_Free( mdl->verts ) ;	
	MDU_Free( mdl->norms ) ;
	if ( mdl->envs != NULL ) MDU_Free( mdl->envs ) ;
	MDU_Free( mdl->vid ) ;
	MDU_Free( mdl->nid ) ;
	mdl->verts = verts2 ;
	mdl->envs = envs2 ;
	mdl->norms = norms2 ;
	mdl->vid = nvid ;
	mdl->nid = nnid ;
	mdl ++ ;
    }
}

void		MDU_Km4JointModel( def )
KM4_DEF		*def ;
{
    FVECTOR	*verts, *norms, *verts2, *norms2, *v, *n ;
    FVECTOR	*envs, *envs2, *nenvs ;
    short	*vid, *nid, *vid2, *nid2, *nvid, *nnid ;
    int		n_verts2, n_norms2 ;
    KM4_MDL	*mdl ;
    int		n_prims, n_models, i, j ;

    VERTEX_USERDATA *vert_usrdata, *vert_usrdata2 ;
    NORMAL_USERDATA *norm_usrdata, *norm_usrdata2 ;

    n_models = def->n_x_models ;
    mdl = def->models ;
    while( -- n_models >= 0 ) {
	n_verts2 = n_norms2 = 0 ;
	n_prims = mdl->n_prims ;
	verts = mdl->verts ;
	envs = mdl->envs ;
	norms = mdl->norms ;
	vert_usrdata = mdl->vert_usrdata ;
	norm_usrdata = mdl->norm_usrdata ;
	vid = mdl->vid ;
	nid = mdl->nid ;
	verts2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
	norms2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;    
	nvid = vid2 = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_verts * 4 ) ;
	nnid = nid2 = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_norms * 4 ) ;
	if ( envs != NULL ) {
	    nenvs = envs2 = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
	}
	if( vert_usrdata != NULL ){
	    vert_usrdata2 = ( VERTEX_USERDATA * )MDU_Alloc( sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
	}
	if( norm_usrdata != NULL ){
	    norm_usrdata2 = ( NORMAL_USERDATA * )MDU_Alloc( sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
	}
	while( -- n_prims >= 0 ) {
	    for ( i = 0; i < 4; i ++ ) {
		v = verts + vid[ i ] ;
		for ( j = 0; j < n_verts2; j ++ ) {
		    if ( v->vx == verts2[ j ].vx &&
			v->vy == verts2[ j ].vy &&
			v->vz == verts2[ j ].vz &&	
			v->vw == verts2[ j ].vw ) {
			vid2[ i ] = j ;
			break ;
		    }
		}
		if ( j != n_verts2 ) continue ;
		verts2[ n_verts2 ] = *v ;
		if ( envs != NULL ) {
		    envs2[ n_verts2 ] = *( envs + vid[ i ] ) ;
		}
		vid2[ i ] = n_verts2 ;

		if ( vert_usrdata != NULL ){
		    vert_usrdata2[ n_verts2 ] = *( vert_usrdata + vid[ i ] ) ;
		}

		n_verts2 ++ ;
	    }
	    for ( i = 0; i < 4; i ++ ) {
		n = norms + nid[ i ] ;
		for ( j = 0; j < n_norms2; j ++ ) {
		    if ( n->vx == norms2[ j ].vx &&
			n->vy == norms2[ j ].vy &&
			n->vz == norms2[ j ].vz &&	
			n->vw == norms2[ j ].vw ) {
			nid2[ i ] = j ;
			break ;
		    }
		}
		if ( j != n_norms2 ) continue ;
		norms2[ n_norms2 ] = *n ;
		nid2[ i ] = n_norms2 ;

		if ( norm_usrdata != NULL ){
		    norm_usrdata2[ n_norms2 ] = *( norm_usrdata + nid[ i ] ) ;
		}

		n_norms2 ++ ;
	    }
	    vid += 4 ;
	    nid += 4 ;
	    vid2 += 4 ;
	    nid2 += 4 ;
	}
	mdl->n_verts = n_verts2 ;
	mdl->n_norms = n_norms2 ;	
	MDU_Free( mdl->verts ) ;	
	MDU_Free( mdl->norms ) ;
	if ( mdl->envs != NULL ) MDU_Free( mdl->envs ) ;
	MDU_Free( mdl->vid ) ;
	MDU_Free( mdl->nid ) ;
	mdl->verts = verts2 ;
	mdl->envs = envs2 ;
	mdl->norms = norms2 ;
	mdl->vid = nvid ;
	mdl->nid = nnid ;

	if( mdl->vert_usrdata != NULL ) MDU_Free( mdl->vert_usrdata );
	if( mdl->norm_usrdata != NULL ) MDU_Free( mdl->norm_usrdata );
	mdl->vert_usrdata = vert_usrdata2 ;
	mdl->norm_usrdata = norm_usrdata2 ;

	mdl ++ ;
    }
}

/* 誤差をなくすために、頂点座標を整数にしてしまう */
#define	ADJUST	(0.5F) 
void		MDU_ModelVertsInt( def )
KM3_DEF		*def ;
{
    int		i, j, n_models ;
    FVECTOR	*verts ;
    FVECTOR	*trans ;
    SVECTOR	*itrans ;
    KM3_MDL	*mdl ;
    float	tx, ty, tz ;
    int		itx, ity, itz, vx, vy, vz ;

    mdl = def->models ;
    n_models = def->n_x_models ;
    tx = ty = tz = 0.0F ;
    itx = ity = itz = 0 ;
    trans = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * n_models ) ;
    itrans = ( SVECTOR * )MDU_Alloc( sizeof( SVECTOR ) * n_models ) ;
    /* 一度全部グローバル座標にする */
    for ( i = 0; i < n_models; i ++ ) {
	trans[ i ].vx = mdl->tx ; 
	trans[ i ].vy = mdl->ty ; 
	trans[ i ].vz = mdl->tz ; 
	if ( mdl->parent != -1 ) {
	    trans[ i ].vx += trans[ mdl->parent ].vx ;
	    trans[ i ].vy += trans[ mdl->parent ].vy ;
	    trans[ i ].vz += trans[ mdl->parent ].vz ;
	}
	tx = trans[ i ].vx ;
	ty = trans[ i ].vy ;
	tz = trans[ i ].vz ;
	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    verts->vx += tx ;
	    verts->vy += ty ;
	    verts->vz += tz ;
	    verts ++ ;
	}
	mdl ++ ;
    }
    /* ローカル座標に戻す時に整数化 */
    mdl = def->models ;  
    for ( i = 0; i < n_models; i ++ ) {
	itrans[ i ].vx = ( int )( mdl->tx + ADJUST ) ;
	itrans[ i ].vy = ( int )( mdl->ty + ADJUST ) ;
	itrans[ i ].vz = ( int )( mdl->tz + ADJUST ) ;
	mdl->tx = ( float )itrans[ i ].vx ;
	mdl->ty = ( float )itrans[ i ].vy ;
	mdl->tz = ( float )itrans[ i ].vz ;
	if ( mdl->parent != -1 ) {
	    itrans[ i ].vx += itrans[ mdl->parent ].vx ;
	    itrans[ i ].vy += itrans[ mdl->parent ].vy ;
	    itrans[ i ].vz += itrans[ mdl->parent ].vz ;
	}
	itx = itrans[ i ].vx ;
	ity = itrans[ i ].vy ;
	itz = itrans[ i ].vz ;
	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    vx = ( int )( verts->vx + ADJUST ) ;
	    vy = ( int )( verts->vy + ADJUST ) ;
	    vz = ( int )( verts->vz + ADJUST ) ;
	    verts->vx = ( float )( vx - itx ) ;
	    verts->vy = ( float )( vy - ity ) ;
	    verts->vz = ( float )( vz - itz ) ;
	    verts ++ ;
	}
	mdl ++ ;
    }    
    MDU_Free( trans ) ;
    MDU_Free( itrans ) ;
}

void		MDU_Km4ModelVertsInt( def )
KM4_DEF		*def ;
{
    int		i, j, n_models ;
    FVECTOR	*verts ;
    FVECTOR	*trans ;
    SVECTOR	*itrans ;
    KM4_MDL	*mdl ;
    float	tx, ty, tz ;
    int		itx, ity, itz, vx, vy, vz ;

    mdl = def->models ;
    n_models = def->n_x_models ;
    tx = ty = tz = 0.0F ;
    itx = ity = itz = 0 ;
    trans = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * n_models ) ;
    itrans = ( SVECTOR * )MDU_Alloc( sizeof( SVECTOR ) * n_models ) ;
    /* 一度全部グローバル座標にする */
    for ( i = 0; i < n_models; i ++ ) {
	trans[ i ].vx = mdl->tx ; 
	trans[ i ].vy = mdl->ty ; 
	trans[ i ].vz = mdl->tz ; 
	if ( mdl->parent != -1 ) {
	    trans[ i ].vx += trans[ mdl->parent ].vx ;
	    trans[ i ].vy += trans[ mdl->parent ].vy ;
	    trans[ i ].vz += trans[ mdl->parent ].vz ;
	}
	tx = trans[ i ].vx ;
	ty = trans[ i ].vy ;
	tz = trans[ i ].vz ;
	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    verts->vx += tx ;
	    verts->vy += ty ;
	    verts->vz += tz ;
	    verts ++ ;
	}
	mdl ++ ;
    }
    /* ローカル座標に戻す時に整数化 */
    mdl = def->models ;  
    for ( i = 0; i < n_models; i ++ ) {
	itrans[ i ].vx = ( int )( mdl->tx + ADJUST ) ;
	itrans[ i ].vy = ( int )( mdl->ty + ADJUST ) ;
	itrans[ i ].vz = ( int )( mdl->tz + ADJUST ) ;
	mdl->tx = ( float )itrans[ i ].vx ;
	mdl->ty = ( float )itrans[ i ].vy ;
	mdl->tz = ( float )itrans[ i ].vz ;
	if ( mdl->parent != -1 ) {
	    itrans[ i ].vx += itrans[ mdl->parent ].vx ;
	    itrans[ i ].vy += itrans[ mdl->parent ].vy ;
	    itrans[ i ].vz += itrans[ mdl->parent ].vz ;
	}
	itx = itrans[ i ].vx ;
	ity = itrans[ i ].vy ;
	itz = itrans[ i ].vz ;
	verts = mdl->verts ;
	for ( j = 0; j < mdl->n_verts; j ++ ) {
	    vx = ( int )( verts->vx + ADJUST ) ;
	    vy = ( int )( verts->vy + ADJUST ) ;
	    vz = ( int )( verts->vz + ADJUST ) ;
	    verts->vx = ( float )( vx - itx ) ;
	    verts->vy = ( float )( vy - ity ) ;
	    verts->vz = ( float )( vz - itz ) ;
	    verts ++ ;
	}
	mdl ++ ;
    }    
    MDU_Free( trans ) ;
    MDU_Free( itrans ) ;
}

/*-------------------------------------------------------------*/

/* デバッグ関係 */

/* Ｋｍ３ダンプ */
static	void	DumpKm3Def( def )
KM3_DEF		*def ;
{
    printf( "n_models	%d\n", def->n_models ) ;
    printf( "n_x_models	%d\n", def->n_x_models ) ;
    printf( "bound min %.2f %.2f %.2f\n", def->lx, def->ly, def->lz ) ;
    printf( "bound max %.2f %.2f %.2f\n", def->ux, def->uy, def->uz ) ;
}

static	void	DumpKm3Tex( def2 )
KM3_DEF2	*def2 ;
{
    int		i ;
    printf( "%d textures\n", def2->n_texs ) ;
    for ( i = 0; i < def2->n_texs; i ++ ) {
	printf( "%s\n", def2->texs[ i ] ) ;
    }
}

static	void	DumpKm3Verts( mdl )
KM3_MDL		*mdl ;
{
    int		i ;
    FVECTOR	*v, *v2 ;
    ENVDATA	*ed ;

    i = mdl->n_verts ;
    printf( "\t %d verts\n", i ) ;
    v = mdl->verts ; v2 = mdl->envs ;
    while( -- i >= 0 ) {
	printf( "\t [ %.2f, %.2f %.2f ]", v->vx, v->vy, v->vz ) ;
	ed = ( ENVDATA * )( &( v->vw ) ) ; 
	if ( ed->parent != 255 ) {
	    printf( "[ %.2f %.2f %.2f %.2f ]", v2->vx, v2->vy, v2->vz, v2->vw ) ;
	}
	printf( "\n" ) ;
	v ++ ; v2 ++ ;
    }
}

static	void	DumpKm3Norms( mdl )
KM3_MDL		*mdl ;
{
    int		i ;
    FVECTOR	*v ;

    i = mdl->n_norms ;
    printf( "\t %d norms\n", i ) ;
    v = mdl->norms ;
    while( -- i >= 0 ) {
	printf( "\t [ %.2f, %.2f %.2f ]\n", v->vx, v->vy, v->vz ) ;
	v ++ ;
    }
}

static	void	DumpKm3Polys( def2, mdl )
KM3_DEF2	*def2 ;
KM3_MDL		*mdl ;
{
    int		i, j ;
    u_short	*vid, *nid ;
    u_int	*tid ;
    TVECTOR	*uvs ;

    i = mdl->n_prims ;
    printf( " \t %d polys\n", i ) ;
    vid = mdl->vid ; 
    nid = mdl->nid ;
    uvs = mdl->uvs ;
    tid = mdl->tid ;
    while( -- i >= 0 ) {
	printf( "\t[ %s ]\n", def2->texs[ *tid ] ) ;
	for ( j = 0; j < 4; j ++ ) {
	    printf( "\t[%d %d %.2f %.2f]\n", *vid, *nid, uvs->u, uvs->v ) ;
	    vid ++ ; nid ++ ; uvs ++ ;
	}
	printf( "\n" ) ;
	tid ++ ;
    }
}

static	void	DumpKm3Mdl( def2, mdl )
KM3_DEF2	*def2 ;
KM3_MDL		*mdl ;
{
    printf( "type 	%x\n", mdl->type ) ;
    printf( "n_prims 	%d\n", mdl->n_prims ) ;
    printf( "bound min  %.2f %.2f %.2f\n", mdl->lx, mdl->ly, mdl->lz ) ;
    printf( "bound max  %.2f %.2f %.2f\n", mdl->ux, mdl->uy, mdl->uz ) ;
    printf( "trans 	%.2f %.2f %.2f\n", mdl->tx, mdl->ty, mdl->tz ) ;
    printf( "parent	%d\n", mdl->parent ) ;
    printf( "extend	%d\n", mdl->extend ) ;
    printf( "n_verts 	%d\n", mdl->n_verts ) ;
    printf( "n_norms 	%d\n", mdl->n_norms ) ;
    DumpKm3Verts( mdl ) ;
    DumpKm3Norms( mdl ) ;      
    DumpKm3Polys( def2, mdl ) ;
}

void		MDU_DumpKm3( def2 )
KM3_DEF2	*def2 ;
{
    int		i ;
    KM3_MDL	*mdl ;

    DumpKm3Def( def2->def ) ;
    DumpKm3Tex( def2 ) ;
    mdl = def2->def->models ;
    for ( i = 0; i < def2->def->n_models; i ++, mdl ++ ) {
	printf( "model No %d\n", i ) ;
	DumpKm3Mdl( def2, mdl ) ;
    }
}

/* Ｋｍｓダンプ */
static	void	DumpKmsDef( sdef )
KMS_DEF		*sdef ;
{
    printf( "n_models		%ld\n", sdef->n_models ) ;
    printf( "n_x_models		%ld\n", sdef->n_x_models ) ;
    printf( "lx,ly,lz		%.3f, %.3f, %.3f\n", sdef->lx,sdef->ly,sdef->lz ) ;
    printf( "ux,uy,uz		%.3f, %.3f, %.3f\n", sdef->ux,sdef->uy,sdef->uz ) ;
}

static	void	DumpKmsPkt( pkt )
KMS_PKT		*pkt ;
{
    int		i ;

    printf( "\t\t flag		%lx\n", pkt->flag ) ;
    printf( "\t\t n_verts	%ld\n", pkt->n_verts ) ;
    printf( "\t\t tid		%ld\n", pkt->tid ) ;    
    for ( i = 0; i < pkt->n_verts; i ++ ) {
	printf( "\t\t[%d %d %d %d]", pkt->verts[ i ].vx, pkt->verts[ i ].vy,
	         pkt->verts[ i ].vz, pkt->verts[ i ].vw ) ;
	printf( "[%d %d %d %x]", pkt->norms[ i ].vx, pkt->norms[ i ].vy,
	         pkt->norms[ i ].vz, pkt->norms[ i ].vw ) ;
	printf( "[%d %d]\n", pkt->uvs[ i ].u, pkt->uvs[ i ].v ) ;
    }
}

static	void	DumpKmsObj( obj ) 
KMS_OBJ		*obj ;
{
    int		i ;
    KMS_PKT	*pkt ;

    printf( "\t type 		%lx\n", obj->type ) ;
    printf( "\t n_packs		%ld\n", obj->n_packs ) ;
    printf( "\t lx,ly,lz		%.3f, %.3f, %.3f\n", obj->lx,obj->ly,obj->lz ) ;
    printf( "\t ux,uy,uz		%.3f, %.3f, %.3f\n", obj->ux,obj->uy,obj->uz ) ;
    printf( "\t tx,ty,tz		%.3f, %.3f, %.3f\n", obj->tx,obj->ty,obj->tz ) ;
    printf( "\t parent 		%ld\n", obj->parent ) ;

    pkt = obj->pack ;
    for ( i = 0; i < obj->n_packs; i ++ ) {
	DumpKmsPkt( pkt ) ;
	pkt ++ ;
    }
}

void		MDU_DumpKms( sdef ) 
KMS_DEF		*sdef ;
{
    int		i ;
    KMS_OBJ	*obj ;

    obj = sdef->objs ;
    DumpKmsDef( sdef ) ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
	printf( "object No.%d\n", i ) ;
	DumpKmsObj( obj ) ;
	obj ++ ;
    }
}

/* KM4 Debug code */
void	MDU_CheckVertUsrdata( sdef )
KM4_DEF	*sdef;
{
    int i;
    KM4_MDL *mdl;

    mdl=sdef->models;
    for(i=0;i<sdef->n_x_models;i++,mdl++){
	int j;

	printf("object No.%d\n",i);

	for(j=0;j<mdl->n_verts;j++){
	    VERTEX_USERDATA *vert_usrdata=mdl->vert_usrdata+j;

	    if(vert_usrdata->dWeight[0]<0 ||
	       vert_usrdata->dWeight[0]>=4096){

		printf("Strange0 I = %d\n",vert_usrdata->dWeight[0]);
	    }
	    if(vert_usrdata->dWeight[1]<0 ||
	       vert_usrdata->dWeight[1]>=4096){

		printf("Strange1 I = %d\n",vert_usrdata->dWeight[1]);
	    }
	    if(vert_usrdata->dWeight[2]<0 ||
	       vert_usrdata->dWeight[2]>=4096){

		printf("Strange2 I = %d\n",vert_usrdata->dWeight[2]);
	    }
	    if(vert_usrdata->dWeight[3]<0 ||
	       vert_usrdata->dWeight[3]>=4096){

		printf("Strange3 I = %d\n",vert_usrdata->dWeight[3]);
	    }
	}
    }
}

/*-------------------------------------------------------------*/

