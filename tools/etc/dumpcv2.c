/*
   dumpkms.c : kms_file dumper for MGS on linux ( use libMDU )

   by M.Sonoyama 1999 Nov. ～
   $Id: dumpcv2.c,v 1.3 2000/10/25 12:52:22 usr04098 Exp $
 
   Konami Computer Entertainment Japan West CS1
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<unistd.h>
#include	<sys/types.h>

#include	"fmt_km4.h"
#include	"MDU_util.h"
#include	"MDU_mdl.h"
#include	"MDU_tex.h"


/* ＣＶ２ダンプ */
static	void	DumpCv2Def( sdef )
CV2_OBJS	*sdef ;
{
    printf( "id\t\t%d\n", sdef->id ) ;
    printf( "n_models\t%d\n", sdef->n_objs ) ;
    printf( "flag\t\t0x%08x\n", sdef->flag ) ;
    printf( "pad1\t\t%d\n", sdef->pad1 ) ;
}

static	void	DumpCv2ObjVerts( pkt )
CV2_OBJ		*pkt ;
{
    int	  i ;
    FVECTOR *v  ;
    short   *vi ;

    printf( "\tn_verts\t\t%d\n\tn_verts_index\t%d\n", pkt->n_verts, pkt->n_verts_index ) ;
    v  = pkt->verts ;
    vi = pkt->verts_index ;
    for ( i=0 ; i<pkt->n_verts ; i++, v++ )
    {
	printf( "\t\t[%f\t%f\t%f]\t", v->vx, v->vy, v->vz ) ;
	printf( i<pkt->n_verts_index ? "[%4d]\n": "\n", *vi++ ) ;
    }
    for ( ; i<pkt->n_verts_index ; i++ )
	printf( "\t\t[\t\t\t\t\t]\t[%4d]\n", *vi++ ) ;
}


static	void	DumpCv2ObjNorms( pkt )
CV2_OBJ		*pkt ;
{
    int	  i ;
    FVECTOR *v  ;
    short   *vi ;

    printf( "\tn_norms\t\t%d\n\tn_norms_index\t%d\n", pkt->n_norms, pkt->n_norms_index ) ;
    v  = pkt->norms ;
    vi = pkt->norms_index ;
    for ( i=0 ; i<pkt->n_norms ; i++, v++ )
    {
	printf( "\t\t[%f\t%f\t%f]\t", v->vx, v->vy, v->vz ) ;
	printf( i<pkt->n_norms_index ? "[%4d]\n": "\n", *vi++ ) ;
    }
    for ( ; i<pkt->n_norms_index ; i++ )
	printf( "\t\t[\t\t\t\t\t]\t[%4d]\n", *vi++ ) ;
}


static	void	DumpCv2ObjUvs( pkt )
CV2_OBJ		*pkt ;
{
    int	  i ;
    FVECTOR *v  ;
    short   *vi ;

    printf( "\tn_uvs\t\t%d\n\tn_uvs_index\t%d\n", pkt->n_uvs, pkt->n_uvs_index ) ;
    v  = pkt->uvs ;
    vi = pkt->uvs_index ;
    for ( i=0 ; i<pkt->n_uvs ; i++, v++ )
    {
	printf( "\t\t[%f\t%f\t%f]\t", v->vx, v->vy, v->vz ) ;
	printf( i<pkt->n_uvs_index ? "[%4d]\n": "\n", *vi++ ) ;
    }
    for ( ; i<pkt->n_uvs_index ; i++ )
	printf( "\t\t[\t\t\t\t\t]\t[%4d]\n", *vi++ ) ;
}

void		DumpCv2( sdef )
CV2_OBJS	*sdef ;
{
    int		i ;
    CV2_OBJ	*obj ;

    obj = sdef->objs ;
    DumpCv2Def( sdef ) ;
    for ( i = 0; i < sdef->n_objs; i ++ ) {
	printf( "object No.%d\n", i ) ;
	if ( sdef->flag & COMMON_VERTS )
	    DumpCv2ObjVerts( obj ) ;
	if ( sdef->flag & COMMON_NORMS )
	    DumpCv2ObjNorms( obj ) ;
	if ( sdef->flag & COMMON_UVS )
	    DumpCv2ObjUvs( obj ) ;
	obj ++ ;
    }
}

static	void	Usage( void )
{
    fprintf( stderr, "dumpcv2 : cv2_file dumper\n" ) ;
    fprintf( stderr, "usage : dumpcv2 cv2file(*.cv2)\n" ) ;
    exit( -1 ) ;
}







/* .cv2ロード */
CV2_OBJS *LoadCv2( char *name )
{
    CV2_OBJS	*def ;
    CV2_OBJ	*mdl ;
    FILE	*fp ;
    int		size, i ;

    fp = fopen( name, "rb" ) ;
    if ( fp == NULL ) return NULL ;
    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
    fseek( fp, 0, SEEK_SET ) ;

    def = ( CV2_OBJS * )malloc( size ) ;
    if ( def == NULL ) return NULL ;
    fread( def, size, 1, fp ) ;
    fclose( fp ) ;
    MDU_EndianLong( ( long * )def, sizeof( CV2_OBJS ) / sizeof( long ) ) ;
    mdl = def->objs ;
    for ( i = 0; i < def->n_objs; i ++ ) {
	MDU_EndianLong( ( long * )mdl, sizeof( CV2_OBJ ) / sizeof( long ) ) ;	
	if(def->flag & COMMON_VERTS){
	    mdl->verts = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->verts ) ;
	    mdl->verts_index = ( short * )( ( u_int )def + ( u_int )mdl->verts_index ) ;
	    MDU_EndianLong( ( long * )mdl->verts, 
			    mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
	    MDU_EndianShort( ( short * )mdl->verts_index, mdl->n_verts_index ) ;
	}
	if(def->flag & COMMON_NORMS){
	    mdl->norms = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->norms ) ;
	    mdl->norms_index = ( short * )( ( u_int )def + ( u_int )mdl->norms_index ) ;
	    MDU_EndianLong( ( long * )mdl->norms, 
			    mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
	    MDU_EndianShort( ( short * )mdl->norms_index, mdl->n_norms_index ) ;
	}
	if(def->flag & COMMON_UVS){
	    mdl->uvs = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->uvs ) ;
	    mdl->uvs_index = ( short * )( ( u_int )def + ( u_int )mdl->uvs_index ) ;
	    MDU_EndianLong( ( long * )mdl->uvs, 
			    mdl->n_uvs * sizeof( FVECTOR ) / sizeof( long ) ) ;
	    MDU_EndianShort( ( short * )mdl->uvs_index, mdl->n_uvs_index ) ;
	}
	if(def->flag & COMMON_VERT_USRDATA){
	    switch(def->id){
	    case 1160797: /* ver1.2 */
		{
		    int j;

		    mdl->vert_usrdata
			= ( VERTEX_USERDATA * )( ( u_int )def + ( u_int )mdl->vert_usrdata ) ;
		    for(j=0;j<mdl->n_verts;j++){
			MDU_EndianLong( &((vert_usrdata+j)->nVertexSwing),
					sizeof( int ) / sizeof( long ) ) ;
			MDU_EndianShort( (vert_usrdata+j)->dWeight,
					 sizeof( short [4] ) / sizeof( short ) ) ;
		    }
		}
		break;
	    case 705644: /* ver1.1 */
		{
		    OLD1_VERTEX_USERDATA *vert;
		    vert = ( OLD1_VERTEX_USERDATA * )( ( u_int )def + ( u_int )mdl->vert_usrdata ) ;
		    MDU_EndianLong( ( long * )vert,
				    mdl->n_verts * sizeof( OLD1_VERTEX_USERDATA ) / sizeof( long ) ) ;
		    mdl->vert_usrdata
			= (VERTEX_USERDATA *)malloc(sizeof(VERTEX_USERDATA)*mdl->n_verts);
		    if(mdl->vert_usrdata!=NULL){
			int j;
			for(j=0;j<mdl->n_verts;j++){
			    (mdl->vert_usrdata+j)->nVertexSwing=(vert+j)->nVertexSwing;
			    (mdl->vert_usrdata+j)->dWeight[0]=0;
			    (mdl->vert_usrdata+j)->dWeight[1]=0;
			    (mdl->vert_usrdata+j)->dWeight[2]=0;
			    (mdl->vert_usrdata+j)->dWeight[3]=0;
			}
		    }
		    else return NULL;
		}
		break;
	    }
	}
	if(def->flag & COMMON_NORM_USRDATA){
	    mdl->norm_usrdata = ( NORMAL_USERDATA * )( ( u_int )def + ( u_int )mdl->norm_usrdata ) ;
	}
	mdl ++ ;
    }
    return def ;
}










int	main( int argc, char **argv )
{
    CV2_OBJS	*sdef ;

    if ( argc < 2 ) Usage() ;
    sdef = LoadCv2( argv[ 1 ] ) ;
    if ( sdef == NULL ) Usage() ;
    DumpCv2( sdef ) ;
    exit( 0 ) ;
}
