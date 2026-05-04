/*
   MDU_mdlSave.c

   モデルセーブ関数

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/1999

   $Id: MDU_mdlSave.c,v 1.22 2002/06/04 13:45:38 usr01363 Exp $
   
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
#include "MDU_mdl.h"

static	u_int	VertsAddr, NormsAddr, EnvsAddr ;
static	u_int	VidAddr, NidAddr, TidAddr, UvsAddr ;
static	u_int	VertsAddr2, NormsAddr2, EnvsAddr2 ;
static	u_int	VidAddr2, NidAddr2, TidAddr2, UvsAddr2 ;
static	u_int	N_Verts ;

/* 追加 */
static  u_int	VertUsrdataAddr, NormUsrdataAddr, PrimUsrdataAddr ;
static  u_int	VertUsrdataAddr2, NormUsrdataAddr2, PrimUsrdataAddr2 ;
static  u_int	VertsIndexAddr, NormsIndexAddr, UvsIndexAddr;
static  u_int	VertsIndexAddr2, NormsIndexAddr2, UvsIndexAddr2;

/*---------------------------------------------------------------*/

static int FileWriteAll(const char * const filename,const void * const buf,const long size)
{
    FILE *fp;
    int ans=0;

    if((fp=fopen(filename,"wb"))==NULL) return 0;
    if(fwrite(buf,1,size,fp)==size) ans=1;
    fclose(fp);

    return ans;
}

/*---------------------------------------------------------------*/

/*
   .km3 ファイル形式

   KM3_DEF	header ;
   KM3_MDL	models[ n_x_models ] ;
   u_int	テクスチャ数 ;
   char		テクスチャ名列 ;
   FVECTOR	*verts ;
   FVECTOR	*norms ;
   FVECTOR	*envs ;
   u_short	*vid ;
   u_short	*nid ;
   u_int	*tid ;
   TVECTOR	*uvs ;

   .km4 ファイル形式
   .km3 +
   VERTEX_USERDATA	*vert_usrdata;
   NORMAL_USERDATA	*norm_usrdata;
   PRIM_USERDATA	*prim_usrdata;
*/

/* .km3でセーブ */
static	void	Km3WriteTexNames( def2, fp )
	KM3_DEF2	*def2 ;
FILE		*fp ;
{
    int		len, i, c ;
    char	*str ;

    c = 0 ;
    MDU_FwriteEndianLong( fp, ( long * )( &( def2->n_texs ) ), 1, 1 ) ;
    for ( i = 0; i < def2->n_texs; i ++ ) {
		str = def2->texs[ i ] ;
		len = strlen( str ) + 1 ;
		fwrite( str, 1, len, fp ) ;
		while( ( ftell( fp ) % 4 ) != 0 ) putc( c, fp ) ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    VertsAddr2 = ftell( fp ) ;
}

static	void	Km3WriteVerts( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM3_MDL	*mdl ;
    FVECTOR	*verts ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, VertsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		verts = mdl->verts ;
		MDU_EndianLong( ( long * )verts, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( verts, sizeof( FVECTOR ), mdl->n_verts, fp ) ;
		MDU_EndianLong( ( long * )verts, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, VertsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
		fwrite( &VertsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		VertsAddr2 += mdl->n_verts * sizeof( FVECTOR ) ;
		VertsAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    NormsAddr2 = ftell( fp ) ;
}

static	void	Km3WriteNorms( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM3_MDL	*mdl ;
    FVECTOR	*norms ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, NormsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		norms = mdl->norms ;
		MDU_EndianLong( ( long * )norms, 
						mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( norms, sizeof( FVECTOR ), mdl->n_norms, fp ) ;
		MDU_EndianLong( ( long * )norms, 
						mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, NormsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
		fwrite( &NormsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		NormsAddr2 += mdl->n_norms * sizeof( FVECTOR ) ;
		NormsAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    EnvsAddr2 = ftell( fp ) ;
}

static	void	Km3WriteEnvs( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM3_MDL	*mdl ;
    FVECTOR	*envs ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, EnvsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		envs = mdl->envs ;
		MDU_EndianLong( ( long * )envs, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( envs, sizeof( FVECTOR ), mdl->n_verts, fp ) ;
		MDU_EndianLong( ( long * )envs, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, EnvsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&EnvsAddr2, 1 ) ;
		fwrite( &EnvsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&EnvsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		EnvsAddr2 += mdl->n_verts * sizeof( FVECTOR ) ;
		EnvsAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    VidAddr2 = ftell( fp ) ;
}

static	void	Km3WriteVid( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM3_MDL	*mdl ;
    u_short	*vid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, VidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		vid = mdl->vid ;
		MDU_EndianShort( vid, mdl->n_prims * 4 ) ;
		fwrite( vid, sizeof( u_short ), mdl->n_prims * 4, fp ) ;
		MDU_EndianShort( vid, mdl->n_prims * 4 ) ;
		now = ftell( fp ) ;
		fseek( fp, VidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&VidAddr2, 1 ) ;
		fwrite( &VidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&VidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		VidAddr2 += mdl->n_prims * 4 * sizeof( u_short ) ;
		VidAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    NidAddr2 = ftell( fp ) ;
}

static	void	Km3WriteNid( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM3_MDL	*mdl ;
    u_short	*nid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, NidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		nid = mdl->nid ;
		MDU_EndianShort( nid, mdl->n_prims * 4 ) ;
		fwrite( nid, sizeof( u_short ), mdl->n_prims * 4, fp ) ;
		MDU_EndianShort( nid, mdl->n_prims * 4 ) ;
		now = ftell( fp ) ;
		fseek( fp, NidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&NidAddr2, 1 ) ;
		fwrite( &NidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&NidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		NidAddr2 += mdl->n_prims * 4 * sizeof( u_short ) ;
		NidAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    TidAddr2 = ftell( fp ) ;
}

static	void	Km3WriteTid( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM3_MDL	*mdl ;
    u_int	*tid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, TidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		tid = mdl->tid ;
		MDU_EndianLong( tid, mdl->n_prims ) ;
		fwrite( tid, sizeof( u_int ), mdl->n_prims, fp ) ;
		MDU_EndianLong( tid, mdl->n_prims ) ;
		now = ftell( fp ) ;
		fseek( fp, TidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&TidAddr2, 1 ) ;
		fwrite( &TidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&TidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		TidAddr2 += mdl->n_prims * sizeof( u_int ) ;
		TidAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    UvsAddr2 = ftell( fp ) ;
}

static	void	Km3WriteUvs( def, fp )
	KM3_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM3_MDL	*mdl ;
    TVECTOR	*uvs ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, UvsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		uvs = mdl->uvs ;
		MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		fwrite( uvs, sizeof( TVECTOR ), mdl->n_prims * 4, fp ) ;
		MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, UvsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
		fwrite( &UvsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		UvsAddr2 += mdl->n_prims * 4 * sizeof( TVECTOR ) ;
		UvsAddr += sizeof( KM3_MDL ) ;
		mdl ++ ;
    }
}

void	MDU_SaveKm3( name, def2 )
	char		*name ;
KM3_DEF2	*def2 ;
{
    FILE	*fp ;
    KM3_DEF	*def ;	
    KM3_MDL	*mdl ;
    int		n_x_models ;

    fp = fopen( name, "wb" ) ;
    if ( fp == NULL ) return ;

    def = def2->def ;
    n_x_models = def->n_x_models ;
    
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * n_x_models ) / sizeof( u_long ) ) ;
    fwrite( def, sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * n_x_models, 1, fp ) ;
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * n_x_models ) / sizeof( u_long ) ) ;

    mdl = def->models ;
    VertsAddr = ( u_int )&mdl->verts - ( u_int )def ;
    NormsAddr = ( u_int )&mdl->norms - ( u_int )def ;
    EnvsAddr = ( u_int )&mdl->envs - ( u_int )def ;
    VidAddr = ( u_int )&mdl->vid - ( u_int )def ;
    NidAddr = ( u_int )&mdl->nid - ( u_int )def ;
    TidAddr = ( u_int )&mdl->tid - ( u_int )def ;
    UvsAddr = ( u_int )&mdl->uvs - ( u_int )def ;

    Km3WriteTexNames( def2, fp ) ;
    Km3WriteVerts( def, fp ) ;
    Km3WriteNorms( def, fp ) ;
    Km3WriteEnvs( def, fp ) ;

    Km3WriteVid( def, fp ) ;
    Km3WriteNid( def, fp ) ;
    Km3WriteTid( def, fp ) ;
    Km3WriteUvs( def, fp ) ;

    fclose( fp ) ;
}

/*---------------------------------------------------------------*/
/* 追加                                                          */
/*---------------------------------------------------------------*/

/* .km4でセーブ */
static	void	Km4WriteTexNames( def2, fp )
	KM4_DEF2	*def2 ;
FILE		*fp ;
{
    int		len, i, c ;
    char	*str ;

    c = 0 ;
    MDU_FwriteEndianLong( fp, ( long * )( &( def2->n_texs ) ), 1, 1 ) ;
    for ( i = 0; i < def2->n_texs; i ++ ) {
		str = def2->texs[ i ] ;
		len = strlen( str ) + 1 ;
		fwrite( str, 1, len, fp ) ;
		while( ( ftell( fp ) % 4 ) != 0 ) putc( c, fp ) ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    VertsAddr2 = ftell( fp ) ;
}

static	void	Km4WriteVerts( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM4_MDL	*mdl ;
    FVECTOR	*verts ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, VertsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		verts = mdl->verts ;
		MDU_EndianLong( ( long * )verts, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( verts, sizeof( FVECTOR ), mdl->n_verts, fp ) ;
		MDU_EndianLong( ( long * )verts, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, VertsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
		fwrite( &VertsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		VertsAddr2 += mdl->n_verts * sizeof( FVECTOR ) ;
		VertsAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    NormsAddr2 = ftell( fp ) ;
}

static	void	Km4WriteNorms( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM4_MDL	*mdl ;
    FVECTOR	*norms ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, NormsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		norms = mdl->norms ;
		MDU_EndianLong( ( long * )norms, 
						mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( norms, sizeof( FVECTOR ), mdl->n_norms, fp ) ;
		MDU_EndianLong( ( long * )norms, 
						mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, NormsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
		fwrite( &NormsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		NormsAddr2 += mdl->n_norms * sizeof( FVECTOR ) ;
		NormsAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    EnvsAddr2 = ftell( fp ) ;
}

static	void	Km4WriteEnvs( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    KM4_MDL	*mdl ;
    FVECTOR	*envs ;
    
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, EnvsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		envs = mdl->envs ;
		MDU_EndianLong( ( long * )envs, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		fwrite( envs, sizeof( FVECTOR ), mdl->n_verts, fp ) ;
		MDU_EndianLong( ( long * )envs, 
						mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, EnvsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&EnvsAddr2, 1 ) ;
		fwrite( &EnvsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&EnvsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		EnvsAddr2 += mdl->n_verts * sizeof( FVECTOR ) ;
		EnvsAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    VidAddr2 = ftell( fp ) ;
}

static	void	Km4WriteVid( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    u_short	*vid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, VidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		vid = mdl->vid ;
		MDU_EndianShort( vid, mdl->n_prims * 4 ) ;
		fwrite( vid, sizeof( u_short ), mdl->n_prims * 4, fp ) ;
		MDU_EndianShort( vid, mdl->n_prims * 4 ) ;
		now = ftell( fp ) ;
		fseek( fp, VidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&VidAddr2, 1 ) ;
		fwrite( &VidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&VidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		VidAddr2 += mdl->n_prims * 4 * sizeof( u_short ) ;
		VidAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    NidAddr2 = ftell( fp ) ;
}

static	void	Km4WriteNid( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    u_short	*nid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, NidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		nid = mdl->nid ;
		MDU_EndianShort( nid, mdl->n_prims * 4 ) ;
		fwrite( nid, sizeof( u_short ), mdl->n_prims * 4, fp ) ;
		MDU_EndianShort( nid, mdl->n_prims * 4 ) ;
		now = ftell( fp ) ;
		fseek( fp, NidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&NidAddr2, 1 ) ;
		fwrite( &NidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&NidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		NidAddr2 += mdl->n_prims * 4 * sizeof( u_short ) ;
		NidAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    TidAddr2 = ftell( fp ) ;
}

static	void	Km4WriteTid( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    u_int	*tid ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, TidAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		tid = mdl->tid ;
		MDU_EndianLong( tid, mdl->n_prims ) ;
		fwrite( tid, sizeof( u_int ), mdl->n_prims, fp ) ;
		MDU_EndianLong( tid, mdl->n_prims ) ;
		now = ftell( fp ) ;
		fseek( fp, TidAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&TidAddr2, 1 ) ;
		fwrite( &TidAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&TidAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		TidAddr2 += mdl->n_prims * sizeof( u_int ) ;
		TidAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    UvsAddr2 = ftell( fp ) ;
}

static	void	Km4WriteUvs( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    TVECTOR	*uvs ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, UvsAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		uvs = mdl->uvs ;
		MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		fwrite( uvs, sizeof( TVECTOR ), mdl->n_prims * 4, fp ) ;
		MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, UvsAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
		fwrite( &UvsAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		UvsAddr2 += mdl->n_prims * 4 * sizeof( TVECTOR ) ;
		UvsAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    VertUsrdataAddr2 = ftell( fp ) ;
}

static	void	Km4WriteVertUsrdata( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    VERTEX_USERDATA	*vert_usrdata ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, VertUsrdataAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		vert_usrdata = mdl->vert_usrdata ;

		MDU_EndianLong( vert_usrdata,
						mdl->n_verts * sizeof( VERTEX_USERDATA ) / sizeof( long ) ) ;
		fwrite( vert_usrdata, sizeof( VERTEX_USERDATA ), mdl->n_verts, fp ) ;
		MDU_EndianLong( vert_usrdata,
						mdl->n_verts * sizeof( VERTEX_USERDATA ) / sizeof( long ) ) ;

		now = ftell( fp ) ;
		fseek( fp, VertUsrdataAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&VertUsrdataAddr2, 1 ) ;
		fwrite( &VertUsrdataAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&VertUsrdataAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		VertUsrdataAddr2 += mdl->n_verts * sizeof( VERTEX_USERDATA ) ;
		VertUsrdataAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    NormUsrdataAddr2 = ftell( fp ) ;
}

static	void	Km4WriteNormUsrdata( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    NORMAL_USERDATA	*norm_usrdata ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, NormUsrdataAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		norm_usrdata = mdl->norm_usrdata ;
		//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		//fwrite( norm_usrdata, sizeof( NORMAL_USERDATA ), mdl->n_norms, fp ) ;
		//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, NormUsrdataAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&NormUsrdataAddr2, 1 ) ;
		fwrite( &NormUsrdataAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&NormUsrdataAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		NormUsrdataAddr2 += mdl->n_norms * sizeof( NORMAL_USERDATA ) ;
		NormUsrdataAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	
    PrimUsrdataAddr2 = ftell( fp ) ;
}

static	void	Km4WritePrimUsrdata( def, fp )
	KM4_DEF		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    KM4_MDL	*mdl ;
    PRIMITIVE_USERDATA	*prim_usrdata ;

    c = 0 ;
    n_models = def->n_x_models ;
    mdl = def->models ;
    fseek( fp, PrimUsrdataAddr2, SEEK_SET ) ;
    for ( i = 0; i < n_models; i ++ ) {
		prim_usrdata = mdl->prim_usrdata ;
		//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		//fwrite( prim_usrdata, sizeof( NORMAL_USERDATA ), mdl->n_prims, fp ) ;
		//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
		now = ftell( fp ) ;
		fseek( fp, PrimUsrdataAddr, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&PrimUsrdataAddr2, 1 ) ;
		fwrite( &PrimUsrdataAddr2, sizeof( u_int ), 1, fp ) ;
		MDU_EndianLong( ( long * )&PrimUsrdataAddr2, 1 ) ;
		fseek( fp, now, SEEK_SET ) ;
		PrimUsrdataAddr2 += mdl->n_prims * sizeof( PRIMITIVE_USERDATA ) ;
		PrimUsrdataAddr += sizeof( KM4_MDL ) ;
		mdl ++ ;
    }
}

void	MDU_SaveKm4( name, def2 )
	char		*name ;
KM4_DEF2	*def2 ;
{
    FILE	*fp ;
    KM4_DEF	*def ;	
    KM4_MDL	*mdl ;
    int		n_x_models ;

    fp = fopen( name, "wb" ) ;
    if ( fp == NULL ) return ;

    def = def2->def ;
    n_x_models = def->n_x_models ;
    
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * n_x_models ) / sizeof( u_long ) ) ;
    fwrite( def, sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * n_x_models, 1, fp ) ;
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * n_x_models ) / sizeof( u_long ) ) ;

    mdl = def->models ;
    VertsAddr = ( u_int )&mdl->verts - ( u_int )def ;
    NormsAddr = ( u_int )&mdl->norms - ( u_int )def ;
    EnvsAddr = ( u_int )&mdl->envs - ( u_int )def ;
    VidAddr = ( u_int )&mdl->vid - ( u_int )def ;
    NidAddr = ( u_int )&mdl->nid - ( u_int )def ;
    TidAddr = ( u_int )&mdl->tid - ( u_int )def ;
    UvsAddr = ( u_int )&mdl->uvs - ( u_int )def ;
    VertUsrdataAddr = ( u_int )&mdl->vert_usrdata - ( u_int )def ;
    NormUsrdataAddr = ( u_int )&mdl->norm_usrdata - ( u_int )def ;
    PrimUsrdataAddr = ( u_int )&mdl->prim_usrdata - ( u_int )def ;

    Km4WriteTexNames( def2, fp ) ;
    Km4WriteVerts( def, fp ) ;
    Km4WriteNorms( def, fp ) ;
    Km4WriteEnvs( def, fp ) ;

    Km4WriteVid( def, fp ) ;
    Km4WriteNid( def, fp ) ;
    Km4WriteTid( def, fp ) ;
    Km4WriteUvs( def, fp ) ;

    Km4WriteVertUsrdata( def, fp );
    Km4WriteNormUsrdata( def, fp );
    Km4WritePrimUsrdata( def, fp );

    fclose( fp ) ;
}

/*---------------------------------------------------------------*/

static	void	Cv2WriteVerts( def, fp )
	CV2_OBJS	*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    FVECTOR	*verts ;

    if(def->flag & COMMON_VERTS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, VertsAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Verts = %d\n",i,mdl->n_verts);
			printf("\t[%d] Verts Addr = %d\n",i,VertsAddr2);
#endif

			verts = mdl->verts ;
			MDU_EndianLong( ( long * )verts, 
							mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
			fwrite( verts, sizeof( FVECTOR ), mdl->n_verts, fp ) ;
			MDU_EndianLong( ( long * )verts, 
							mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
			now = ftell( fp ) ;
			fseek( fp, VertsAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
			fwrite( &VertsAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&VertsAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			VertsAddr2 += mdl->n_verts * sizeof( FVECTOR ) ;
			VertsAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
    }
    NormsAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteNorms( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    FVECTOR	*norms ;

    if(def->flag & COMMON_NORMS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, NormsAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Norms = %d\n",i,mdl->n_norms);
			printf("\t[%d] Norms Addr = %d\n",i,NormsAddr2);
#endif
			norms = mdl->norms ;
			MDU_EndianLong( ( long * )norms, 
							mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
			fwrite( norms, sizeof( FVECTOR ), mdl->n_norms, fp ) ;
			MDU_EndianLong( ( long * )norms, 
							mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
			now = ftell( fp ) ;
			fseek( fp, NormsAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
			fwrite( &NormsAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&NormsAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			NormsAddr2 += mdl->n_norms * sizeof( FVECTOR ) ;
			NormsAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
    }
    UvsAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteUvs( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    FVECTOR	*uvs ;

    c = 0 ;
    if(def->flag & COMMON_UVS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, UvsAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Uvs = %d\n",i,mdl->n_uvs);
			printf("\t[%d] Uvs Addr = %d\n",i,UvsAddr2);
#endif

			uvs = mdl->uvs ;
			MDU_EndianLong( uvs, mdl->n_uvs * sizeof( FVECTOR ) / sizeof( long ) ) ;
			fwrite( uvs, sizeof( FVECTOR ), mdl->n_uvs, fp ) ;
			MDU_EndianLong( uvs, mdl->n_uvs * sizeof( FVECTOR ) / sizeof( long ) ) ;
			now = ftell( fp ) ;
			fseek( fp, UvsAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
			fwrite( &UvsAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&UvsAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			UvsAddr2 += mdl->n_uvs * sizeof( FVECTOR ) ;
			UvsAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
		while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
    }
    VertUsrdataAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteVertUsrdata( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    VERTEX_USERDATA	*vert_usrdata ;

    c = 0 ;
    if(def->flag & COMMON_VERT_USRDATA){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, VertUsrdataAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {
			int j;

			vert_usrdata = mdl->vert_usrdata ;

			for(j=0;j<mdl->n_verts;j++){
				MDU_EndianLong( &((vert_usrdata+j)->nVertexSwing),
								sizeof( int ) / sizeof( long ) ) ;
				MDU_EndianShort( (vert_usrdata+j)->dWeight,
								 sizeof( short [4] ) / sizeof( short ) ) ;
			}

			fwrite( vert_usrdata, sizeof( VERTEX_USERDATA ), mdl->n_verts, fp ) ;

			for(j=0;j<mdl->n_verts;j++){
				MDU_EndianLong( &((vert_usrdata+j)->nVertexSwing),
								sizeof( int ) / sizeof( long ) ) ;
				MDU_EndianShort( (vert_usrdata+j)->dWeight,
								 sizeof( short [4] ) / sizeof( short ) ) ;
			}


			now = ftell( fp ) ;
			fseek( fp, VertUsrdataAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&VertUsrdataAddr2, 1 ) ;
			fwrite( &VertUsrdataAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&VertUsrdataAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			VertUsrdataAddr2 += mdl->n_verts * sizeof( VERTEX_USERDATA ) ;
			VertUsrdataAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
		while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
    }
    NormUsrdataAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteNormUsrdata( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    NORMAL_USERDATA	*norm_usrdata ;

    c = 0 ;
    if(def->flag & COMMON_NORM_USRDATA){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, NormUsrdataAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {
			norm_usrdata = mdl->norm_usrdata ;
			//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
			//fwrite( norm_usrdata, sizeof( NORMAL_USERDATA ), mdl->n_norms, fp ) ;
			//MDU_EndianLong( uvs, mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
			now = ftell( fp ) ;
			fseek( fp, NormUsrdataAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&NormUsrdataAddr2, 1 ) ;
			fwrite( &NormUsrdataAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&NormUsrdataAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			NormUsrdataAddr2 += mdl->n_norms * sizeof( NORMAL_USERDATA ) ;
			NormUsrdataAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
		while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
    }
    VertsIndexAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteVertsIndex( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    u_short	*verts_index ;

    c = 0 ;
    if(def->flag & COMMON_VERTS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, VertsIndexAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Verts_Index = %d\n",i,mdl->n_verts_index);
			printf("\t[%d] Verts Index Addr = %d\n",i,VertsIndexAddr2);
#endif

			verts_index = mdl->verts_index ;
			MDU_EndianShort( verts_index, mdl->n_verts_index ) ;
			fwrite( verts_index, sizeof( u_short ), mdl->n_verts_index, fp ) ;
			MDU_EndianShort( verts_index, mdl->n_verts_index ) ;

			while( ( ftell( fp ) % 4 ) != 0 ) putc( c, fp ) ;

			now = ftell( fp ) ;
			fseek( fp, VertsIndexAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&VertsIndexAddr2, 1 ) ;
			fwrite( &VertsIndexAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&VertsIndexAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			VertsIndexAddr2 += ((mdl->n_verts_index+1) & ~1) * sizeof( u_short ) ;
			VertsIndexAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
    }
    NormsIndexAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteNormsIndex( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    u_short	*norms_index ;

    c = 0 ;
    if(def->flag & COMMON_NORMS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, NormsIndexAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Norms_Index = %d\n",i,mdl->n_norms_index);
			printf("\t[%d] Norms Index Addr = %d\n",i,NormsIndexAddr2);
#endif
			norms_index = mdl->verts_index ;
			MDU_EndianShort( norms_index, mdl->n_norms_index ) ;
			fwrite( norms_index, sizeof( u_short ), mdl->n_norms_index, fp ) ;
			MDU_EndianShort( norms_index, mdl->n_norms_index ) ;

			while( ( ftell( fp ) % 4 ) != 0 ) putc( c, fp ) ;

			now = ftell( fp ) ;
			fseek( fp, NormsIndexAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&NormsIndexAddr2, 1 ) ;
			fwrite( &NormsIndexAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&NormsIndexAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			NormsIndexAddr2 += ((mdl->n_norms_index+1) & ~1) * sizeof( u_short ) ;
			NormsIndexAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
    }
    while( ( ftell( fp ) % 4 ) != 0 ) putc( c, fp ) ;
    UvsIndexAddr2 = ftell( fp ) ;
}

static	void	Cv2WriteUvsIndex( def, fp )
	CV2_OBJS		*def ;
FILE		*fp ;
{
    int		n_models, i, c ;
    u_int	now ;
    CV2_OBJ	*mdl ;
    u_short	*uvs_index ;

    c = 0 ;
    if(def->flag & COMMON_UVS){
		n_models = def->n_objs ;
		mdl = def->objs ;
		fseek( fp, UvsIndexAddr2, SEEK_SET ) ;
		for ( i = 0; i < n_models; i ++ ) {

#if 0
			printf("[%d] N_Uvs_Index = %d\n",i,mdl->n_uvs_index);
			printf("\t[%d] Uvs Index Addr = %d\n",i,UvsIndexAddr2);
#endif

			uvs_index = mdl->uvs_index ;
			MDU_EndianShort( uvs_index, mdl->n_uvs_index ) ;
			fwrite( uvs_index, sizeof( u_short ), mdl->n_uvs_index, fp ) ;
			MDU_EndianShort( uvs_index, mdl->n_uvs_index ) ;

			while( ( ftell( fp ) % 8 ) != 0 ) putc( c, fp ) ;

			now = ftell( fp ) ;
			fseek( fp, UvsIndexAddr, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&UvsIndexAddr2, 1 ) ;
			fwrite( &UvsIndexAddr2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&UvsIndexAddr2, 1 ) ;
			fseek( fp, now, SEEK_SET ) ;
			UvsIndexAddr2 += ((mdl->n_uvs_index+3) & ~3) * sizeof( u_short ) ;
			UvsIndexAddr += sizeof( CV2_OBJ ) ;
			mdl ++ ;
		}
    }
}

void	MDU_SaveCv2( name, def )
	char		*name ;
CV2_OBJS	*def ;
{
    FILE	*fp ;
    CV2_OBJ	*mdl ;
    int		n_x_models ;

#if 0
    def->id = 705644; /* = StrCode("Common Vertex Data ver1.1") */
#else
    def->id = 1160797; /* = StrCode("Common Vertex Data ver102") */
#endif

    fp = fopen( name, "wb" ) ;
    if ( fp == NULL ) return ;

    n_x_models = def->n_objs ;
    
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( CV2_OBJS ) + sizeof( CV2_OBJ ) * n_x_models ) / sizeof( u_long ) ) ;
    fwrite( def, sizeof( CV2_OBJS ) + sizeof( CV2_OBJ ) * n_x_models, 1, fp ) ;
    MDU_EndianLong( ( u_long * )def, 
					( sizeof( CV2_OBJS ) + sizeof( CV2_OBJ ) * n_x_models ) / sizeof( u_long ) ) ;
    while( ( ftell( fp ) % 16 ) != 0 ) putc( 0, fp ) ;

    mdl = def->objs ;
    VertsAddr = ( u_int )&mdl->verts - ( u_int )def ;
    VertsIndexAddr = ( u_int )&mdl->verts_index - ( u_int )def ;
    NormsAddr = ( u_int )&mdl->norms - ( u_int )def ;
    NormsIndexAddr = ( u_int )&mdl->norms_index - ( u_int )def ;
    UvsAddr = ( u_int )&mdl->uvs - ( u_int )def ;
    UvsIndexAddr = ( u_int )&mdl->uvs_index - ( u_int )def ;
    VertUsrdataAddr = ( u_int )&mdl->vert_usrdata - ( u_int )def ;
    NormUsrdataAddr = ( u_int )&mdl->norm_usrdata - ( u_int )def ;

    VertsAddr2 = ftell( fp ) ;

    Cv2WriteVerts( def, fp ) ;
    Cv2WriteNorms( def, fp ) ;
    Cv2WriteUvs( def, fp ) ;

    Cv2WriteVertUsrdata( def, fp );
    Cv2WriteNormUsrdata( def, fp );

    Cv2WriteVertsIndex( def, fp );
    Cv2WriteNormsIndex( def, fp );
    Cv2WriteUvsIndex( def, fp );

    fclose( fp ) ;
}

/*---------------------------------------------------------------*/

/*
   .kms ファイル形式

   KMS_DEF	header ;
   KMS_OBJ	objs[ n_models ] ;
   SVECTOR	*verts ;
   SVECTOR	*norms ;
   TVECTOR_S	*uvs ;
*/

/* .kmsでセーブ */

/* パケットデータを書き込み */
/* ついでに総頂点数を数えておく */
static	void	KmsWritePacket( sdef, fp )
	KMS_DEF		*sdef ;
FILE		*fp ;
{
    int		p1, p2 ;
    int		i, j ;
    KMS_OBJ	*obj ;
    KMS_PKT	*pkt ;

    p1 = ( u_int )sdef->objs + sizeof( long ) * 3 + 
		sizeof( float ) * 9 - ( u_int )sdef ;
    p2 = ftell( fp ) ;

    VertsAddr = p2 + sizeof( long ) * 4 ;
    N_Verts = 0 ;

    obj = sdef->objs ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
		pkt = obj->pack ;
		for ( j = 0; j < obj->n_packs; j ++ ) {
			N_Verts += pkt->n_verts ;
			MDU_EndianLong( ( long * )pkt, sizeof( KMS_PKT ) / sizeof( long ) ) ;
			fwrite( pkt, sizeof( KMS_PKT ), 1, fp ) ;
			MDU_EndianLong( ( long * )pkt, sizeof( KMS_PKT ) / sizeof( long ) ) ;
			pkt ++ ;
		}
		fseek( fp, p1, SEEK_SET ) ;
		MDU_EndianLong( ( long * )&p2, 1 ) ;
		fwrite( &p2, sizeof( long ), 1, fp ) ;
		MDU_EndianLong( ( long * )&p2, 1 ) ;
		p2 += sizeof( KMS_PKT ) * obj->n_packs ;
		p1 += sizeof( KMS_OBJ ) ;
		fseek( fp, p2, SEEK_SET ) ;
		obj ++ ;
    }
}

static	void	KmsWriteVerts( sdef, fp ) 
	KMS_DEF		*sdef ;
FILE		*fp ;
{
    char	c ;
    int		i, j ;
    KMS_OBJ	*obj ;
    KMS_PKT	*pkt ;
    SVECTOR	*verts ;
    u_int	v1, v2, n_verts ;

    c = 0 ;
    v1 = VertsAddr ;
    v2 = ftell( fp ) ;

    obj = sdef->objs ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
		pkt = obj->pack ;
		for ( j = 0; j < obj->n_packs; j ++ ) {
			verts = pkt->verts ;
			n_verts = pkt->n_verts ;
			MDU_EndianShort( ( short * )verts, 
							 sizeof( SVECTOR ) * n_verts / sizeof( short ) ) ;
			fwrite( verts, sizeof( SVECTOR ), n_verts, fp ) ;
			MDU_EndianShort( ( short * )verts, 
							 sizeof( SVECTOR ) * n_verts / sizeof( short ) ) ;
			fseek( fp, v1, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			fwrite( &v2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			v1 += sizeof( KMS_PKT ) ;
			v2 += sizeof( SVECTOR ) * n_verts ;
			fseek( fp, v2, SEEK_SET ) ;
			while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
			v2 = ftell( fp ) ;
			pkt ++ ;
		}
		obj ++ ;
    }
}

static	void	KmsWriteNorms( sdef, fp ) 
	KMS_DEF		*sdef ;
FILE		*fp ;
{
    char	c ;
    int		i, j ;
    KMS_OBJ	*obj ;
    KMS_PKT	*pkt ;
    SVECTOR	*norms ;
    u_int	v1, v2, n_verts ;

    c = 0 ;
    v1 = VertsAddr + sizeof( SVECTOR * ) ;
    v2 = ftell( fp ) ;

    obj = sdef->objs ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
		pkt = obj->pack ;
		for ( j = 0; j < obj->n_packs; j ++ ) {
			norms = pkt->norms ;
			n_verts = pkt->n_verts ;
			MDU_EndianShort( ( short * )norms, 
							 sizeof( SVECTOR ) * n_verts / sizeof( short ) ) ;
			fwrite( norms, sizeof( SVECTOR ), n_verts, fp ) ;
			MDU_EndianShort( ( short * )norms, 
							 sizeof( SVECTOR ) * n_verts / sizeof( short ) ) ;
			fseek( fp, v1, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			fwrite( &v2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			v1 += sizeof( KMS_PKT ) ;
			v2 += sizeof( SVECTOR ) * n_verts ;
			fseek( fp, v2, SEEK_SET ) ;
			while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
			v2 = ftell( fp ) ;
			pkt ++ ;
		}
		obj ++ ;
    }
}

static	void	KmsWriteUvs( sdef, fp ) 
	KMS_DEF		*sdef ;
FILE		*fp ;
{
    char	c ;
    int		i, j ;
    KMS_OBJ	*obj ;
    KMS_PKT	*pkt ;
    TVECTOR_S	*uvs ;
    u_int	v1, v2, n_verts ;

    c = 0 ;
    v1 = VertsAddr + sizeof( SVECTOR * ) * 2 ;
    v2 = ftell( fp ) ;

    obj = sdef->objs ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
		pkt = obj->pack ;
		for ( j = 0; j < obj->n_packs; j ++ ) {
			uvs = pkt->uvs ;
			n_verts = pkt->n_verts ;
			MDU_EndianShort( ( short * )uvs, 
							 sizeof( TVECTOR_S ) * n_verts / sizeof( short ) ) ;
			fwrite( uvs, sizeof( TVECTOR_S ), n_verts, fp ) ;
			MDU_EndianShort( ( short * )uvs, 
							 sizeof( TVECTOR_S ) * n_verts / sizeof( short ) ) ;
			fseek( fp, v1, SEEK_SET ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			fwrite( &v2, sizeof( u_int ), 1, fp ) ;
			MDU_EndianLong( ( long * )&v2, 1 ) ;
			v1 += sizeof( KMS_PKT ) ;
			v2 += sizeof( TVECTOR_S ) * n_verts ;
			fseek( fp, v2, SEEK_SET ) ;
			while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;
			v2 = ftell( fp ) ;
			pkt ++ ;
		}
		obj ++ ;
    }
}

void		MDU_SaveKms( name, sdef )
	char		*name ;
KMS_DEF		*sdef ;
{
    FILE	*fp ;
    KMS_OBJ	*obj ;
    int		n_models ;
    char	c ;

    fp = fopen( name, "wb" ) ;
    if ( fp == NULL ) return ;

    n_models = sdef->n_x_models ;

    /* ヘッダ＆オブジェクト書き込み */
    MDU_EndianLong( ( u_long * )sdef, 
					( sizeof( KMS_DEF ) + sizeof( KMS_OBJ ) * n_models ) / sizeof( u_long ) ) ;
    fwrite( sdef, sizeof( KMS_DEF ) + sizeof( KMS_OBJ ) * n_models, 1, fp ) ;
    MDU_EndianLong( ( u_long * )sdef, 
					( sizeof( KMS_DEF ) + sizeof( KMS_OBJ ) * n_models ) / sizeof( u_long ) ) ;
    c = 0 ;
    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	    
    
    obj = sdef->objs ;

    KmsWritePacket( sdef, fp ) ;

    while( ( ftell( fp ) % 16 ) != 0 ) putc( c, fp ) ;	    

    KmsWriteVerts( sdef, fp ) ;
    KmsWriteNorms( sdef, fp ) ;
    KmsWriteUvs( sdef, fp ) ;

    fclose( fp ) ;
}

/*---------------------------------------------------------------*/

static long Kms2_CalcSize(KMS2_DEF *sdef,int multi_uv_flag)
{
    KMS2_MDL *smdl;
    long fsize,size;
    int i,j;

    fsize=0;

    fsize+=sizeof(KMS2_DEF)+sizeof(KMS2_MDL)*sdef->n_x_models;

    /* パケットデータの展開 */
    smdl=sdef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++){
		fsize+=sizeof(KMS2_MDLPACK)*smdl->n_packs;
    }

    fsize=(fsize+16-1) & ~(16-1);

    /* 頂点データの展開 */
    smdl=sdef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++){
		KMS2_MDLPACK *spack;

		spack=smdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++){
			if(spack->verts!=NULL){
				size=sizeof(short)*spack->n_verts*4;

				size=(size+16-1) & ~(16-1);
				fsize+=size;
			}
		}
    }

    /* 法線データの展開 */
    smdl=sdef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++){
		KMS2_MDLPACK *spack;

		spack=smdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++){
			if(spack->norms!=NULL){
				size=sizeof(short)*spack->n_verts*4;

				size=(size+16-1) & ~(16-1);
				fsize+=size;
			}
		}
    }

    /* UVデータの展開 */
    smdl=sdef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++){
		KMS2_MDLPACK *spack;

		spack=smdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++){
			size=sizeof(short)*spack->n_verts*2;

			size=(size+16-1) & ~(16-1);

			if(spack->uvs[0]!=NULL){
#if 1
				if(spack->flag & DG_PACKFLAG_UV0) fsize+=size;
#else
				/* ダミーデータを入れる場合 */
				fsize+=size;
#endif
			}

#if 0
			if(spack->uvs[1]!=NULL){
				if((spack->flag & DG_PACKFLAG_UV1) && multi_uv_flag) fsize+=size;
			}
			if(spack->uvs[2]!=NULL){
				if((spack->flag & DG_PACKFLAG_UV2) && multi_uv_flag) fsize+=size;
			}
#else
			if(spack->uvs[1]!=NULL){
				if(spack->flag & DG_PACKFLAG_UV1) fsize+=size;
			}
			if(spack->uvs[2]!=NULL){
				if(spack->flag & DG_PACKFLAG_UV2) fsize+=size;
			}
#endif
		}
    }

#if 0
    /* カラーデータ */
    smdl=sdef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++){
		KMS2_MDLPACK *spack;

		spack=smdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++){
			if(spack->rgbs!=NULL){
				size=sizeof(CVECTOR)*spack->n_verts;

				size=(size+16-1) & ~(16-1);
				fsize+=size;
			}
		}
    }
#endif

	/* 2002/6/4  K.Kano
	   X-BOX用共有頂点情報の処理の追加 */
    smdl=sdef->models;
    for(i=0;i<sdef->n_x_models;i++,smdl++){
		if(smdl->vbuff!=NULL){
			size=smdl->stride*smdl->n_verts;

			size=(size+16-1) & ~(16-1);
			fsize+=size;
		}
    }

	/* インデックスデータ */
    smdl=sdef->models;
    for(i=0;i<sdef->n_x_models;i++,smdl++){
		KMS2_MDLPACK *spack;

		spack=smdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++){
			if(spack->index!=NULL){
				size=sizeof(unsigned short)*spack->n_indices;
				fsize+=size;
			}
		}
    }

    return fsize;
}

static void Kms2_Relocate(void *buf,KMS2_DEF *sdef,int multi_uv_flag)
{
    KMS2_DEF *ddef;
    KMS2_MDL *smdl,*dmdl;
    long size;
    long index;
    int i,j;

    index=0;

    size=sizeof(KMS2_DEF)+sizeof(KMS2_MDL)*sdef->n_x_models;
    memcpy(buf,sdef,size);
    ddef=(KMS2_DEF *)buf;

    buf=(void *)((long)buf+size);
    index+=size;

    /* パケットデータの展開 */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		size=sizeof(KMS2_MDLPACK)*smdl->n_packs;
		if(size!=0){
			memcpy(buf,smdl->packs,size);
			dmdl->packs=(KMS2_MDLPACK *)buf;
		}
		else{
			dmdl->packs=NULL;
		}
		buf=(void *)((long)buf+size);
		index+=size;
    }

    if((index & (16-1))!=0){
		long d;
		d=((index+16-1) & ~(16-1))-index;
		buf=(void *)((long)buf+d);
		index+=d;
    }

    /* 頂点データの展開 */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		KMS2_MDLPACK *spack,*dpack;

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			if(spack->verts!=NULL){
				size=sizeof(short)*spack->n_verts*4;
				memcpy(buf,spack->verts,size);
				dpack->verts=buf;

				size=(size+16-1) & ~(16-1);
				buf=(void *)((long)buf+size);
				index+=size;
			}
		}
    }

    /* 法線データの展開 */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		KMS2_MDLPACK *spack,*dpack;

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			if(spack->norms!=NULL){
				size=sizeof(short)*spack->n_verts*4;
				memcpy(buf,spack->norms,size);
				dpack->norms=buf;

				size=(size+16-1) & ~(16-1);
				buf=(void *)((long)buf+size);
				index+=size;
			}
		}
    }

    /* UVデータの展開 */
    smdl=sdef->models;
    dmdl=ddef->models;

    /* UVデータの展開規則として、UV0のみ、UV1のみ、UV2のみと
       いった順序でデータを並べる必要がある。*/
    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		KMS2_MDLPACK *spack,*dpack;

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			long xsize;

			size=sizeof(short)*spack->n_verts*2;
			xsize=(size+16-1) & ~(16-1);

#if 1
			dpack->uvs[0]=NULL;
			if(spack->flag & DG_PACKFLAG_UV0){
				if(spack->uvs[0]!=NULL){
					memcpy(buf,spack->uvs[0],size);
					dpack->uvs[0]=buf;

					buf=(void *)((long)buf+xsize);
					index+=xsize;
				}
			}
#else
			/* ダミーデータを入れる場合 */
			if(spack->flag & DG_PACKFLAG_UV0){
				memcpy(buf,spack->uvs[0],size);
			}
			else{
				dpack->flag|=DG_PACKFLAG_UV0;
				memset(buf,0x00,size);
			}
			dpack->uvs[0]=buf;

			buf=(void *)((long)buf+xsize);
			index+=xsize;
#endif
		}

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			long xsize;

			size=sizeof(short)*spack->n_verts*2;
			xsize=(size+16-1) & ~(16-1);

			dpack->uvs[1]=NULL;
			if(spack->flag & DG_PACKFLAG_UV1){
				if(multi_uv_flag && spack->uvs[1]!=NULL){
					memcpy(buf,spack->uvs[1],size);
					dpack->uvs[1]=buf;

					buf=(void *)((long)buf+xsize);
					index+=xsize;
				}
				else{
#if 0
					dpack->uvs[1]=dpack->uvs[0];
#else
					if(spack->uvs[0]!=NULL){
						memcpy(buf,spack->uvs[0],size);
						dpack->uvs[1]=buf;

						buf=(void *)((long)buf+xsize);
						index+=xsize;
					}
#endif
				}
			}
		}

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			long xsize;

			size=sizeof(short)*spack->n_verts*2;
			xsize=(size+16-1) & ~(16-1);

			dpack->uvs[2]=NULL;
			if(spack->flag & DG_PACKFLAG_UV2){
				if(multi_uv_flag && spack->uvs[2]!=NULL){
					memcpy(buf,spack->uvs[2],size);
					dpack->uvs[2]=buf;

					buf=(void *)((long)buf+xsize);
					index+=xsize;
				}
				else{
#if 0
					dpack->uvs[2]=dpack->uvs[0];
#else
					if(spack->uvs[2]!=NULL){
						memcpy(buf,spack->uvs[0],size);
						dpack->uvs[2]=buf;

						buf=(void *)((long)buf+xsize);
						index+=xsize;
					}
#endif
				}
			}
		}
    }

#if 0
    /* カラーデータ */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		KMS2_MDLPACK *spack,*dpack;

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			if(spack->rgbs!=NULL){
				size=sizeof(CVECTOR)*spack->n_verts;
				memcpy(buf,spack->rgbs,size);
				dpack->rgbs=buf;

				size=(size+16-1) & ~(16-1);
				buf=(void *)((long)buf+size);
				index+=size;
			}
		}
    }
#endif

	/* 2002/6/4  K.Kano
	   X-BOX用共有頂点情報の処理の追加 */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		if(smdl->vbuff!=NULL){
			size=smdl->stride*smdl->n_verts;
			memcpy(buf,smdl->vbuff,size);
			dmdl->vbuff=buf;

			size=(size+16-1) & ~(16-1);
			buf=(void *)((long)buf+size);
			index+=size;
		}
    }


    /* インデックスデータ */
    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
		KMS2_MDLPACK *spack,*dpack;

		spack=smdl->packs;
		dpack=dmdl->packs;

		for(j=0;j<smdl->n_packs;j++,spack++,dpack++){
			if(spack->index!=NULL){
				size=sizeof(unsigned short)*spack->n_indices;
				if( size != 0 ) {
					memcpy(buf,spack->index,size);
					dpack->index=buf;
					buf=(void *)((long)buf+size);
					index+=size;
				} else {
					dpack->index = NULL;
				}
			}
		}
    }
}

static void Kms2_CalcOffsetAndChangeEndian(KMS2_DEF *def)
{
    KMS2_MDL *mdl;
    int i,j;

    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
		KMS2_MDLPACK *pack;

		pack=mdl->packs;
		for(j=0;j<mdl->n_packs;j++,pack++){
			if(pack->verts!=NULL) MDU_EndianShort( ( short * )(pack->verts),pack->n_verts*4 ) ;
			if(pack->norms!=NULL) MDU_EndianShort( ( short * )(pack->norms),pack->n_verts*4 ) ;
			if(pack->uvs[0]!=NULL) MDU_EndianShort( ( short * )(pack->uvs[0]),pack->n_verts*2 ) ;
			if(pack->uvs[1]!=NULL && (long)(pack->uvs[1])!=(long)(pack->uvs[0])){
				MDU_EndianShort( ( short * )(pack->uvs[1]),pack->n_verts*2 ) ;
			}
			if(pack->uvs[2]!=NULL && (long)(pack->uvs[2])!=(long)(pack->uvs[0])){
				MDU_EndianShort( ( short * )(pack->uvs[2]),pack->n_verts*2 ) ;
			}
			
			if(	pack->index != NULL ) MDU_EndianShort( ( short * )( pack->index ), pack->n_indices );

			if(pack->verts!=NULL) pack->verts=(short *)((long)(pack->verts)-(long)def);
			if(pack->norms!=NULL) pack->norms=(short *)((long)(pack->norms)-(long)def);
			if(pack->uvs[0]!=NULL) pack->uvs[0]=(short *)((long)(pack->uvs[0])-(long)def);
			if(pack->uvs[1]!=NULL) pack->uvs[1]=(short *)((long)(pack->uvs[1])-(long)def);
			if(pack->uvs[2]!=NULL) pack->uvs[2]=(short *)((long)(pack->uvs[2])-(long)def);
			if(	pack->index != NULL ) pack->index = (unsigned short*)((long)(pack->index)-(long)def);
#if 0
			pack->rgbs=(short *)((long)(pack->rgbs)-(long)def);
#endif
#if 1		// short のメンバが増えたのでこちらの方法で
			MDU_EndianLong((long *)(&pack->flag), 		1 );	// flag
			MDU_EndianShort((short*)(&pack->n_verts), 	2 );	// n_verts, n_indices
			MDU_EndianLong((long *)(&pack->tex_id[0]),	sizeof(KMS2_MDLPACK)/sizeof(long) - 2 );
#endif
		}
#if 0	// short のメンバが増えたのでこの方法ではやらない
		if(mdl->n_packs!=0){
			MDU_EndianLong((long *)(mdl->packs),
						   sizeof(KMS2_MDLPACK)*mdl->n_packs/sizeof(long));
		}
#endif
		if(mdl->packs!=NULL) mdl->packs=(KMS2_MDLPACK *)((long)(mdl->packs)-(long)def);

		/* 2002/6/4  K.Kano
		   X-BOX用共有頂点情報の処理の追加 */
		if(mdl->vbuff!=NULL){
			MDU_EndianShort((short *)(mdl->vbuff),mdl->stride*mdl->n_verts/sizeof(short));
			mdl->vbuff=(void *)((long)(mdl->vbuff)-(long)def);
		}
    }

    i=(sizeof(KMS2_DEF)+sizeof(KMS2_MDL)*def->n_x_models)/sizeof(long);
    MDU_EndianLong((long *)def,i);
}

int MDU_SaveKms2(char *name,KMS2_DEF *sdef,int multi_uv_flag)
{
    void *buf;
    long size;
    int ans;

    size=Kms2_CalcSize(sdef,multi_uv_flag);
    buf=MDU_Alloc(size);
    if(buf==NULL) return 0;

    memset(buf,0x00,size);

    Kms2_Relocate(buf,sdef,multi_uv_flag);
    Kms2_CalcOffsetAndChangeEndian((KMS2_DEF *)buf);
    ans=FileWriteAll(name,buf,size);

    MDU_Free(buf);

    return ans;
}

/*---------------------------------------------------------------*/

static long Evm_CalcSize(EVM_DEF *sdef,int multi_uv_flag)
{
    EVM_PACK *spack;
    long size;
    long fsize;
    int i;

    fsize=0;

    fsize+=sizeof(EVM_DEF)+sizeof(EVM_SKEL)*sdef->n_x_models;

    fsize+=sizeof(EVM_PACK)*sdef->n_packs;

    fsize=(fsize+16-1) & ~(16-1);

    /* 頂点データ */
    spack=sdef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++){
		size=sizeof(short)*spack->n_verts*4;

		size=(size+16-1) & ~(16-1);
		fsize+=size;
    }

    /* 法線データ */
    spack=sdef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++){
		size=sizeof(short)*spack->n_verts*4;

		size=(size+16-1) & ~(16-1);
		fsize+=size;
    }

    /* ＵＶデータ */
    spack=sdef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++){
		size=sizeof(short)*spack->n_verts*4;

		size=(size+16-1) & ~(16-1);
#if 1
		if(spack->flag & DG_PACKFLAG_UV0) fsize+=size;
#else
		/* ダミーデータを入れる場合 */
		fsize+=size;
#endif

#if 0
		if((spack->flag & DG_PACKFLAG_UV1) && multi_uv_flag) fsize+=size;
		if((spack->flag & DG_PACKFLAG_UV2) && multi_uv_flag) fsize+=size;
#else
		if(spack->flag & DG_PACKFLAG_UV1) fsize+=size;
		if((spack->flag & DG_PACKFLAG_UV2) ||
		   (spack->flag & DG_PACKFLAG_BMAP)) fsize+=size;
#endif
    }

    spack=sdef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++){
		size=sizeof(char)*spack->n_verts*8;

		size=(size+16-1) & ~(16-1);
		fsize+=size;
    }

	/* インデックスデータ */
    spack=sdef->packet;
	for( i=0; i<sdef->n_packs; i++, spack++ ) {
		size=sizeof(unsigned short)*spack->n_indices;
		fsize+=size;
	}

    return fsize;
}

static void Evm_Relocate(void *buf,EVM_DEF *sdef,int multi_uv_flag)
{
    EVM_DEF *ddef;
    EVM_PACK *spack,*dpack;
    long size;
    long index;
    int i;

    index=0;

    size=sizeof(EVM_DEF)+sizeof(EVM_SKEL)*sdef->n_x_models;
    memcpy(buf,sdef,size);
    ddef=(EVM_DEF *)buf;
    buf=(void *)((long)buf+size);
    index+=size;

    size=sizeof(EVM_PACK)*sdef->n_packs;
    memcpy(buf,sdef->packet,size);
    ddef->packet=(EVM_PACK *)buf;
    buf=(void *)((long)buf+size);
    index+=size;

    if((index & (16-1))!=0){
		long d;
		d=((index+16-1) & ~(16-1))-index;
		buf=(void *)((long)buf+d);
		index+=d;
    }

    /* 頂点データ */
    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		size=sizeof(short)*spack->n_verts*4;
		memcpy(buf,spack->verts,size);
		dpack->verts=(void *)buf;

		size=(size+16-1) & ~(16-1);
		buf=(void *)((long)buf+size);
		index+=size;
    }

    /* 法線データ */
    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		size=sizeof(short)*spack->n_verts*4;
		memcpy(buf,spack->norms,size);
		dpack->norms=(void *)buf;

		size=(size+16-1) & ~(16-1);
		buf=(void *)((long)buf+size);
		index+=size;
    }

    /* ＵＶデータ */
    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		long xsize;

		size=sizeof(short)*spack->n_verts*4;
		xsize=(size+16-1) & ~(16-1);

#if 1
		dpack->uvs[0]=NULL;
		if(spack->flag & DG_PACKFLAG_UV0){
			memcpy(buf,spack->uvs[0],size);
			dpack->uvs[0]=(void *)buf;

			buf=(void *)((long)buf+xsize);
			index+=xsize;
		}
#else
		/* ダミーデータを入れる場合 */
		if(spack->flag & DG_PACKFLAG_UV0){
			memcpy(buf,spack->uvs[0],size);
		}
		else{
			dpack->flag|=DG_PACKFLAG_UV0;
			memset(buf,0x00,size);
		}
		dpack->uvs[0]=(void *)buf;

		buf=(void *)((long)buf+xsize);
		index+=xsize;
#endif
    }

    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		long xsize;

		size=sizeof(short)*spack->n_verts*4;
		xsize=(size+16-1) & ~(16-1);

		dpack->uvs[1]=NULL;
		if(spack->flag & DG_PACKFLAG_UV1){
			if(multi_uv_flag){
				memcpy(buf,spack->uvs[1],size);
				dpack->uvs[1]=(void *)buf;

				buf=(void *)((long)buf+xsize);
				index+=xsize;
			}
			else{
#if 0
				dpack->uvs[1]=dpack->uvs[0];
#else
				memcpy(buf,spack->uvs[0],size);
				dpack->uvs[1]=(void *)buf;

				buf=(void *)((long)buf+xsize);
				index+=xsize;
#endif
			}
		}
    }

    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		long xsize;

		size=sizeof(short)*spack->n_verts*4;
		xsize=(size+16-1) & ~(16-1);

		dpack->uvs[2]=NULL;
		if(spack->flag & DG_PACKFLAG_UV2){
			if(multi_uv_flag){
				memcpy(buf,spack->uvs[2],size);
				dpack->uvs[2]=(void *)buf;

				buf=(void *)((long)buf+xsize);
				index+=xsize;
			}
			else{
#if 0
				dpack->uvs[2]=dpack->uvs[0];
#else
				memcpy(buf,spack->uvs[0],size);
				dpack->uvs[2]=(void *)buf;

				buf=(void *)((long)buf+xsize);
				index+=xsize;
#endif
			}
		}
		else if(spack->flag & DG_PACKFLAG_BMAP){
			memcpy(buf,spack->uvs[2],size);
			dpack->uvs[2]=(void *)buf;

			buf=(void *)((long)buf+xsize);
			index+=xsize;
		}
    }

    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
		size=sizeof(char)*spack->n_verts*8;
		memcpy(buf,spack->weight,size);
		dpack->weight=(void *)buf;

		size=(size+16-1) & ~(16-1);
		buf=(void *)((long)buf+size);
		index+=size;
    }

    /* インデックスデータ */
    spack=sdef->packet;
    dpack=ddef->packet;
    for( i = 0; i < sdef->n_packs; i++, spack++, dpack++ ) {
		size=sizeof(unsigned short)*spack->n_indices;
		if( size != 0 ) {
			memcpy(buf, spack->index, size);
			dpack->index=(unsigned short *)buf;
			buf=(void *)((long)buf+size);
			index+=size;
		} else {	
			dpack->index = NULL;
		}
    }
}

static void Evm_CalcOffsetAndChangeEndian(EVM_DEF *def)
{
    EVM_PACK *pack;
    int i;

    pack=def->packet;
    for(i=0;i<def->n_packs;i++,pack++){
		MDU_EndianShort((short *)(pack->verts),pack->n_verts*4);
		MDU_EndianShort((short *)(pack->norms),pack->n_verts*4);
		if(pack->uvs[0]!=NULL) MDU_EndianShort((short *)(pack->uvs[0]),pack->n_verts*4);
		if(pack->uvs[1]!=NULL && (long)(pack->uvs[1])!=(long)(pack->uvs[0])){
			MDU_EndianShort((short *)(pack->uvs[1]),pack->n_verts*4);
		}
		if(pack->uvs[2]!=NULL && (long)(pack->uvs[2])!=(long)(pack->uvs[0])){
			MDU_EndianShort((short *)(pack->uvs[2]),pack->n_verts*4);
		}

		if(	pack->index != NULL ) MDU_EndianShort( ( short * )( pack->index ), pack->n_indices );

		pack->verts=(void *)((long)(pack->verts)-(long)def);
		pack->norms=(void *)((long)(pack->norms)-(long)def);
		if(pack->uvs[0]!=NULL) pack->uvs[0]=(void *)((long)(pack->uvs[0])-(long)def);
		if(pack->uvs[1]!=NULL) pack->uvs[1]=(void *)((long)(pack->uvs[1])-(long)def);
		if(pack->uvs[2]!=NULL) pack->uvs[2]=(void *)((long)(pack->uvs[2])-(long)def);
		pack->weight=(void *)((long)(pack->weight)-(long)def);
		if(	pack->index != NULL ) pack->index = (unsigned short*)((long)(pack->index)-(long)def);
#if 0
		pack->rgbs=(void *)((long)(pack->rgbs)-(long)def);
#endif
#if 1		// short のメンバが増えたのでこちらの方法で
		MDU_EndianLong((long *)(&pack->flag), 		4 );	// flag , tex_id [3]
		MDU_EndianShort((short*)(&pack->n_verts), 	2 );	// n_verts, n_indices
		MDU_EndianLong((long *)(&pack->tex_id[0]),	sizeof(KMS2_MDLPACK)/sizeof(long) - 5 );
#endif
    }
#if 0	// short のメンバが増えたのでこの方法ではやらない
    MDU_EndianLong((long *)(def->packet),sizeof(EVM_PACK)*def->n_packs/sizeof(long));
#endif
    def->packet=(EVM_PACK *)((long)(def->packet)-(long)def);

    i=sizeof(EVM_DEF)+sizeof(EVM_SKEL)*def->n_x_models;
    MDU_EndianLong((long *)def,i);
}

int MDU_SaveEvm(char *name,EVM_DEF *sdef,int multi_uv_flag)
{
    void *buf;
    long size;
    int ans;

    size=Evm_CalcSize(sdef,multi_uv_flag);
    buf=MDU_Alloc(size);
    if(buf==NULL) return 0;

    memset(buf,0x00,size);

    Evm_Relocate(buf,sdef,multi_uv_flag);
    Evm_CalcOffsetAndChangeEndian((EVM_DEF *)buf);
    ans=FileWriteAll(name,buf,size);
    MDU_Free(buf);

    return ans;
}

/*---------------------------------------------------------------*/
