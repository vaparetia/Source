/*
   MDU_mdlLoad.c

   モデルロード関数 [[Model load function]]

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/1999

   $Id: MDU_mdlLoad.c,v 1.20 2002/06/18 09:21:08 usr04098 Exp $
   
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
//#include <SFDLINUX.h>

//#include "METALGEAR.h"
#include "fmt_kms.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"

#include "block.h"

/*------------------------------------------------------------------*/

static void *FileReadAll(const char * const filename)
{
    FILE *fp;
    long size;
    void *buf;

    if((fp=fopen(filename,"rb"))==NULL) return NULL;

    fseek(fp,0,SEEK_END);
    size=ftell(fp);
    fseek(fp,0,SEEK_SET);

    if((buf=(void *)MDU_Alloc(size))==NULL){
	fclose(fp);
	return NULL;
    }
    if(fread(buf,1,size,fp)!=size){
	MDU_Free(buf);
	buf=NULL;
    }
    fclose(fp);

    return buf;
}

/*------------------------------------------------------------------*/

/* .km3ロード [[load]] */
KM3_DEF2	*MDU_LoadKm3( name )
char		*name ;
{
    KM3_DEF2	*def2 ;
    KM3_DEF	*def ;
    KM3_MDL	*mdl ;
    FILE	*fp ;
    int		size, i, n ;
    char	*buf ;
    char	*str ;

    def2=(KM3_DEF2 *)MDU_Alloc(sizeof(KM3_DEF2));
    if(def2==NULL) return NULL;

    def=(KM3_DEF *)FileReadAll(name);
    if(def==NULL){
	MDU_Free(def2);
	return NULL ;
    }

    MDU_EndianLong( ( long * )def, sizeof( KM3_DEF ) / sizeof( long ) ) ;
    mdl = def->models ;
    for ( i = 0; i < def->n_x_models; i ++ ) {
	MDU_EndianLong( ( long * )mdl, sizeof( KM3_MDL ) / sizeof( long ) ) ;	
	mdl->verts = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->verts ) ;
	mdl->norms = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->norms ) ;
	mdl->envs = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->envs ) ;
	mdl->vid = ( u_short * )( ( u_int )def + ( u_int )mdl->vid ) ;
	mdl->nid = ( u_short * )( ( u_int )def + ( u_int )mdl->nid ) ;
	mdl->tid = ( u_int * )( ( u_int )def + ( u_int )mdl->tid ) ;
	mdl->uvs = ( TVECTOR * )( ( u_int )def + ( u_int )mdl->uvs ) ;
	MDU_EndianLong( ( long * )mdl->verts, 
		        mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->norms, 
		        mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->envs, 
		        mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->tid, 
		        mdl->n_prims * sizeof( u_int ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->uvs, 
		        mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
	MDU_EndianShort( ( short * )mdl->vid, mdl->n_prims * 4 ) ;
	MDU_EndianShort( ( short * )mdl->nid, mdl->n_prims * 4 ) ;
	mdl ++ ;
    }
    buf = ( char * )def ;	
    buf += sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * def->n_x_models ;
    def2->n_texs = n = *( int * )buf ; MDU_EndianLong( &n, 1 ) ;
    buf += sizeof( u_int ) ;
    str = ( char * )buf ;
    for ( i = 0; i < n; i ++ ) {
	strcpy( def2->texs[ i ], str ) ;
	str += strlen( str ) + 1 ;
	while( ( ( u_int )str - ( u_int )def ) % 4 != 0 ) {
	    str ++ ;
	}
    }
    def2->def = def ;

    /* ちゃんとメモリブロックに振り分ける [[Properly allocating the memory block]] */
    {
	KM3_DEF	*ndef ;
	KM3_MDL	*nmdl ;
	FVECTOR	*v1, *v2 ;
	u_short	*id1, *id2 ;
	TVECTOR	*uv1, *uv2 ;
	u_int	*tid1, *tid2 ;
	int	i ;

	ndef = ( KM3_DEF * )MDU_Alloc( sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * def->n_x_models ) ;
	memcpy( ndef, def, sizeof( KM3_DEF ) + sizeof( KM3_MDL ) * def->n_x_models ) ;
	mdl = def->models ;
	nmdl = ndef->models ;
	for ( i = 0; i < def->n_x_models; i ++, mdl ++, nmdl ++ ) {
	    if ( mdl->n_verts != 0 ) {
		v1 = mdl->verts ; 
		v2 = nmdl->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_verts ) ;
		id1 = mdl->vid ;
		id2 = nmdl->vid = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_prims * 4 ) ;
		memcpy( id2, id1, sizeof( u_short ) * mdl->n_prims * 4 ) ;

		v1 = mdl->norms ; 
		v2 = nmdl->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_norms ) ;
		id1 = mdl->nid ;
		id2 = nmdl->nid = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_prims * 4 ) ;
		memcpy( id2, id1, sizeof( u_short ) * mdl->n_prims * 4 ) ;

		uv1 = mdl->uvs ;
		uv2 = nmdl->uvs = ( TVECTOR * )MDU_Alloc( sizeof( TVECTOR ) * mdl->n_prims * 4 ) ;
		memcpy( uv2, uv1, sizeof( TVECTOR ) * mdl->n_prims * 4 ) ;

		tid1 = mdl->tid ;
		tid2 = nmdl->tid = ( u_int * )MDU_Alloc( sizeof( u_int ) * mdl->n_prims ) ;
		memcpy( tid2, tid1, sizeof( u_int ) * mdl->n_prims ) ;

		v1 = mdl->envs ; 
		v2 = nmdl->envs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_verts ) ;		
	    } else {
		nmdl->verts = NULL ;
		nmdl->vid = NULL ;
		nmdl->norms = NULL ;
		nmdl->nid = NULL ;
		nmdl->uvs = NULL ;
		nmdl->tid = NULL ;
		nmdl->envs = NULL ;
	    }
	}
	MDU_Free( def ) ;
	def2->def = ndef ;
    }

    return def2 ;
}


/*---------------------------------------------------------------*/
/* 追加 [[Added]]                                                         */
/*---------------------------------------------------------------*/

/* .km4ロード [[load]] */
KM4_DEF2	*MDU_LoadKm4( name )
char		*name ;
{
    KM4_DEF2	*def2 ;
    KM4_DEF	*def ;
    KM4_MDL	*mdl ;
    FILE	*fp ;
    int		size, i, n ;
    char	*buf ;
    char	*str ;

    def2=(KM4_DEF2 *)MDU_Alloc(sizeof(KM4_DEF2)) ;
    if(def2==NULL) return NULL;

    def=(KM4_DEF *)FileReadAll(name);
    if(def==NULL){
	MDU_Free(def2);
	return NULL ;
    }

    MDU_EndianLong( ( long * )def, sizeof( KM4_DEF ) / sizeof( long ) ) ;
    mdl = def->models ;
    for ( i = 0; i < def->n_x_models; i ++ ) {
	MDU_EndianLong( ( long * )mdl, sizeof( KM4_MDL ) / sizeof( long ) ) ;	
	mdl->verts = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->verts ) ;
	mdl->norms = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->norms ) ;
	mdl->envs = ( FVECTOR * )( ( u_int )def + ( u_int )mdl->envs ) ;
	mdl->vid = ( u_short * )( ( u_int )def + ( u_int )mdl->vid ) ;
	mdl->nid = ( u_short * )( ( u_int )def + ( u_int )mdl->nid ) ;
	mdl->tid = ( u_int * )( ( u_int )def + ( u_int )mdl->tid ) ;
	mdl->uvs = ( TVECTOR * )( ( u_int )def + ( u_int )mdl->uvs ) ;
	mdl->vert_usrdata = ( VERTEX_USERDATA * )( ( u_int )def + ( u_int )mdl->vert_usrdata ) ;
	mdl->norm_usrdata = ( NORMAL_USERDATA * )( ( u_int )def + ( u_int )mdl->norm_usrdata ) ;
	mdl->prim_usrdata = ( PRIMITIVE_USERDATA * )( ( u_int )def + ( u_int )mdl->prim_usrdata ) ;
	MDU_EndianLong( ( long * )mdl->verts, 
		        mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->norms, 
		        mdl->n_norms * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->envs, 
		        mdl->n_verts * sizeof( FVECTOR ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->tid, 
		        mdl->n_prims * sizeof( u_int ) / sizeof( long ) ) ;
	MDU_EndianLong( ( long * )mdl->uvs, 
		        mdl->n_prims * 4 * sizeof( TVECTOR ) / sizeof( long ) ) ;
	MDU_EndianShort( ( short * )mdl->vid, mdl->n_prims * 4 ) ;
	MDU_EndianShort( ( short * )mdl->nid, mdl->n_prims * 4 ) ;
	MDU_EndianLong( ( long * )mdl->vert_usrdata, 
		        mdl->n_verts * sizeof( VERTEX_USERDATA ) / sizeof( long ) ) ;
	mdl ++ ;
    }
    buf = ( char * )def ;	
    buf += sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * def->n_x_models ;
    def2->n_texs = n = *( int * )buf ; MDU_EndianLong( &n, 1 ) ;
    buf += sizeof( u_int ) ;
    str = ( char * )buf ;
    for ( i = 0; i < n; i ++ ) {
	strcpy( def2->texs[ i ], str ) ;
	str += strlen( str ) + 1 ;
	while( ( ( u_int )str - ( u_int )def ) % 4 != 0 ) {
	    str ++ ;
	}
    }
    def2->def = def ;

    /* ちゃんとメモリブロックに振り分ける [[Properly allocating the memory block]] */
    {
	KM4_DEF	*ndef ;
	KM4_MDL	*nmdl ;
	FVECTOR	*v1, *v2 ;
	u_short	*id1, *id2 ;
	TVECTOR	*uv1, *uv2 ;
	u_int	*tid1, *tid2 ;
	VERTEX_USERDATA	*vu1, *vu2 ;
	NORMAL_USERDATA	*nu1, *nu2 ;
	PRIMITIVE_USERDATA	*pu1, *pu2 ;
	int	i ;

	ndef = ( KM4_DEF * )MDU_Alloc( sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * def->n_x_models ) ;
	memcpy( ndef, def, sizeof( KM4_DEF ) + sizeof( KM4_MDL ) * def->n_x_models ) ;
	mdl = def->models ;
	nmdl = ndef->models ;
	for ( i = 0; i < def->n_x_models; i ++, mdl ++, nmdl ++ ) {
	    if ( mdl->n_verts != 0 ) {
		v1 = mdl->verts ; 
		v2 = nmdl->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_verts ) ;
		id1 = mdl->vid ;
		id2 = nmdl->vid = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_prims * 4 ) ;
		memcpy( id2, id1, sizeof( u_short ) * mdl->n_prims * 4 ) ;

		v1 = mdl->norms ; 
		v2 = nmdl->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_norms ) ;
		id1 = mdl->nid ;
		id2 = nmdl->nid = ( u_short * )MDU_Alloc( sizeof( u_short ) * mdl->n_prims * 4 ) ;
		memcpy( id2, id1, sizeof( u_short ) * mdl->n_prims * 4 ) ;

		uv1 = mdl->uvs ;
		uv2 = nmdl->uvs = ( TVECTOR * )MDU_Alloc( sizeof( TVECTOR ) * mdl->n_prims * 4 ) ;
		memcpy( uv2, uv1, sizeof( TVECTOR ) * mdl->n_prims * 4 ) ;

		tid1 = mdl->tid ;
		tid2 = nmdl->tid = ( u_int * )MDU_Alloc( sizeof( u_int ) * mdl->n_prims ) ;
		memcpy( tid2, tid1, sizeof( u_int ) * mdl->n_prims ) ;

		v1 = mdl->envs ; 
		v2 = nmdl->envs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_verts ) ;		
	    } else {
		nmdl->verts = NULL ;
		nmdl->vid = NULL ;
		nmdl->norms = NULL ;
		nmdl->nid = NULL ;
		nmdl->uvs = NULL ;
		nmdl->tid = NULL ;
		nmdl->envs = NULL ;
	    }
	    /* KM4追加分 [[KM4 Additions]] */
	    if ( mdl->n_verts != 0 ) {
		vu1 = mdl->vert_usrdata ;
		vu2 = nmdl->vert_usrdata 
		    = ( VERTEX_USERDATA * )MDU_Alloc( sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
		memcpy( vu2, vu1, sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
	    } else {
		nmdl->vert_usrdata = NULL ;
	    }
	    if ( mdl->n_norms != 0 ) {
		nu1 = mdl->norm_usrdata ;
		nu2 = nmdl->norm_usrdata 
		    = ( NORMAL_USERDATA * )MDU_Alloc( sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
		memcpy( nu2, nu1, sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
	    } else {
		nmdl->norm_usrdata = NULL ;
	    }
	    if ( mdl->n_prims != 0 ) {
		pu1 = mdl->prim_usrdata ;
		pu2 = nmdl->prim_usrdata 
		    = ( PRIMITIVE_USERDATA * )MDU_Alloc( sizeof( PRIMITIVE_USERDATA ) * mdl->n_prims ) ;
		memcpy( pu2, pu1, sizeof( PRIMITIVE_USERDATA ) * mdl->n_prims ) ;
	    } else {
		nmdl->prim_usrdata = NULL ;
	    }
	}
	MDU_Free( def ) ;
	def2->def = ndef ;
    }

    return def2 ;
}

/* .cv2ロード */
CV2_OBJS	*MDU_LoadCv2( name )
char		*name ;
{
    CV2_OBJS	*def ;
    CV2_OBJ	*mdl ;
    FILE	*fp ;
    int		size, i ;

    def = ( CV2_OBJS * )FileReadAll(name);
    if ( def == NULL ) return NULL ;

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
			= (VERTEX_USERDATA *)MDU_Alloc(sizeof(VERTEX_USERDATA)*mdl->n_verts);
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

    /* メモリブロックにふりわけ [[Memory block furiwake??]] */
    {
	int		i ;
	CV2_OBJS	*def2 ;
	CV2_OBJ		*mdl2 ;
	FVECTOR		*v1, *v2 ;
	short		*s1, *s2 ;
	VERTEX_USERDATA	*vu1, *vu2 ;
	NORMAL_USERDATA	*nu1, *nu2 ;

	def2 = ( CV2_OBJS * )MDU_Alloc( sizeof( CV2_OBJS ) + sizeof( CV2_OBJ ) * def->n_objs ) ;
	memcpy( def2, def, sizeof( CV2_OBJS ) + sizeof( CV2_OBJ ) * def->n_objs ) ;	
	mdl = def->objs ;
	mdl2 = def2->objs ;
	for ( i = 0; i < def->n_objs; i ++, mdl ++, mdl2 ++ ) {
	    mdl2->verts = NULL ;
	    mdl2->verts_index = NULL ;
	    mdl2->norms = NULL ;
	    mdl2->norms_index = NULL ;
	    mdl2->uvs = NULL ;
	    mdl2->uvs_index = NULL ;
	    mdl2->vert_usrdata = NULL ;
	    mdl2->norm_usrdata = NULL ;
	    if ( mdl->n_verts != 0 ) {
		v1 = mdl->verts ;
		v2 = mdl2->verts = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_verts ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_verts ) ;
		vu1 = mdl->vert_usrdata ;
		vu2 = mdl2->vert_usrdata 
		    = ( VERTEX_USERDATA * )MDU_Alloc( sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
		memcpy( vu2, vu1, sizeof( VERTEX_USERDATA ) * mdl->n_verts ) ;
	    }
	    if ( mdl->n_verts_index != 0 ) {
		s1 = mdl->verts_index ;
		s2 = mdl2->verts_index = ( short * )MDU_Alloc( sizeof( short ) * mdl->n_verts_index ) ;
		memcpy( s2, s1, sizeof( short ) * mdl->n_verts_index ) ;
	    }
	    if ( mdl->n_norms != 0 ) {
		v1 = mdl->norms ;
		v2 = mdl2->norms = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_norms ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_norms ) ;
		if(sizeof( NORMAL_USERDATA )!=0){
		    nu1 = mdl->norm_usrdata ;
		    nu2 = mdl2->norm_usrdata 
			= ( NORMAL_USERDATA * )MDU_Alloc( sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
		    memcpy( nu2, nu1, sizeof( NORMAL_USERDATA ) * mdl->n_norms ) ;
		}
	    }
	    if ( mdl->n_norms_index != 0 ) {
		s1 = mdl->norms_index ;
		s2 = mdl2->norms_index = ( short * )MDU_Alloc( sizeof( short ) * mdl->n_norms_index ) ;
		memcpy( s2, s1, sizeof( short ) * mdl->n_norms_index ) ;
	    }
	    if ( mdl->n_uvs != 0 ) {
		v1 = mdl->uvs ;
		v2 = mdl2->uvs = ( FVECTOR * )MDU_Alloc( sizeof( FVECTOR ) * mdl->n_uvs ) ;
		memcpy( v2, v1, sizeof( FVECTOR ) * mdl->n_uvs ) ;
	    }
	    if ( mdl->n_uvs_index != 0 ) {
		s1 = mdl->uvs_index ;
		s2 = mdl2->uvs_index = ( short * )MDU_Alloc( sizeof( short ) * mdl->n_uvs_index ) ;
		memcpy( s2, s1, sizeof( short ) * mdl->n_uvs_index ) ;
	    }	    
	}
    }
    return def ;
}

/*------------------------------------------------------------------*/

/* .kmsロード [[load]] */
KMS_DEF		*MDU_LoadKms( name )
char		*name ;
{
    KMS_DEF	*sdef, *sdef2 ;
    KMS_OBJ	*obj, *obj2 ;
    KMS_PKT	*pkt, *pkt2 ;
    int		size, i, j ;

    sdef = ( KMS_DEF * )FileReadAll(name);
    if ( sdef == NULL ) return NULL ;

    MDU_EndianLong( ( long * )sdef, sizeof( KMS_DEF ) / sizeof( long ) ) ;
    obj = sdef->objs ;
    MDU_EndianLong( ( long * )obj, sizeof( KMS_OBJ ) * sdef->n_x_models / sizeof( long ) ) ;
    for ( i = 0; i < sdef->n_x_models; i ++ ) {
	obj->pack = ( KMS_PKT * )( ( u_int )sdef + ( u_int )obj->pack ) ;
	pkt = obj->pack ;
	MDU_EndianLong( ( long * )pkt, sizeof( KMS_PKT ) * obj->n_packs / sizeof( long ) ) ;
	for ( j = 0; j < obj->n_packs; j ++ ) {
	    pkt->verts = ( SVECTOR * )( ( u_int )sdef + ( u_int )pkt->verts ) ;
	    pkt->norms = ( SVECTOR * )( ( u_int )sdef + ( u_int )pkt->norms ) ;
	    pkt->uvs = ( TVECTOR_S * )( ( u_int )sdef + ( u_int )pkt->uvs ) ;
	    MDU_EndianShort( ( short * )pkt->verts, 
			     sizeof( SVECTOR ) * pkt->n_verts / sizeof( short ) ) ;
	    MDU_EndianShort( ( short * )pkt->norms, 
			     sizeof( SVECTOR ) * pkt->n_verts / sizeof( short ) ) ;
	    MDU_EndianShort( ( short * )pkt->uvs, 
			     sizeof( TVECTOR_S ) * pkt->n_verts / sizeof( short ) ) ;
	    pkt ++ ;
	}
	obj ++ ;
    }

    /* ちゃんとメモリブロックに振り分けましょう [[Properly distribute to the memory block?]] */
    sdef2 = ( KMS_DEF * )MDU_Alloc( sizeof( KMS_DEF ) + sizeof( KMS_OBJ ) * sdef->n_x_models ) ;
    memcpy( sdef2, sdef, sizeof( KMS_DEF ) + sizeof( KMS_OBJ ) * sdef->n_x_models ) ;
    obj2 = sdef2->objs ;
    obj = sdef->objs ;
    memcpy( obj2, obj, sizeof( KMS_OBJ ) * sdef->n_x_models ) ;
    for ( i = 0; i < sdef->n_x_models; i ++, obj ++, obj2 ++ ) {
	if ( obj->n_packs != 0 ) {
	    pkt2 = obj2->pack = ( KMS_PKT * )MDU_Alloc( sizeof( KMS_PKT ) * obj->n_packs ) ;
	    pkt = obj->pack ;
	    memcpy( pkt2, pkt, sizeof( KMS_PKT ) * obj->n_packs ) ;
	    for ( j = 0; j < obj->n_packs; j ++, pkt ++, pkt2 ++ ) {
		SVECTOR	*v1, *v2, *n1, *n2 ;
		TVECTOR_S *uv1, *uv2 ;
		
		if ( pkt->n_verts != 0 ) {
		    v1 = pkt->verts ; 
		    v2 = pkt2->verts = ( SVECTOR * )MDU_Alloc( sizeof( SVECTOR ) * pkt->n_verts ) ;
		    memcpy( v2, v1, sizeof( SVECTOR ) * pkt->n_verts ) ;
		    n1 = pkt->norms ;
		    n2 = pkt2->norms = ( SVECTOR * )MDU_Alloc( sizeof( SVECTOR ) * pkt->n_verts ) ;
		    memcpy( n2, n1, sizeof( SVECTOR ) * pkt->n_verts ) ;		    
		    uv1 = pkt->uvs ;
		    uv2 = pkt2->uvs = ( TVECTOR_S * )MDU_Alloc( sizeof( TVECTOR_S ) * pkt->n_verts ) ;
		    memcpy( uv2, uv1, sizeof( TVECTOR_S ) * pkt->n_verts ) ;
		} else {
		    pkt2->verts = NULL ;
		    pkt2->norms = NULL ;
		    pkt2->uvs = NULL ;
		}
	    }
	} else {
	    obj2->pack = NULL ;
	}
    }
    MDU_Free( sdef ) ;
    sdef = sdef2 ;
    return sdef ;
}

/*-----------------------------------------------------------------*/

static void Kms2_ChangeEndianAndAddOffset(KMS2_DEF *sdef, int baseAddress)
{
    KMS2_MDL *obj;
    int i,j;

    MDU_EndianLong( ( long * )sdef, sizeof( KMS2_DEF ) / sizeof( long ) ) ;
    obj = sdef->models ;
    MDU_EndianLong( ( long * )obj, sizeof( KMS2_MDL ) * sdef->n_x_models / sizeof( long ) ) ;

    for(i=0;i<sdef->n_x_models;i++,obj++){
	KMS2_MDLPACK *pack;

	obj->packs=(KMS2_MDLPACK *)((long)(obj->packs)+(long)baseAddress);
	pack=obj->packs;
#if 0	// short のメンバが増えたのでこの方法ではやらない [[In this way, so members do not do more]]
	MDU_EndianLong( ( long * )packs, sizeof( KMS2_MDLPACK )*obj->n_packs / sizeof( long ) ) ;
#endif

	for(j=0;j<obj->n_packs;j++,pack++){
#if 1		// short のメンバが増えたのでこちらの方法で [[The more members, so this way]]
		MDU_EndianLong((long *)(&pack->flag), 		1 );	// flag
		MDU_EndianShort((short*)(&pack->n_verts), 	2 );	// n_verts, n_indices
		MDU_EndianLong((long *)(&pack->tex_id[0]),	sizeof(KMS2_MDLPACK)/sizeof(long) - 2 );
#endif
	    if(pack->verts!=NULL) pack->verts=(short *)((long)(pack->verts)+(long)baseAddress);
	    if(pack->norms!=NULL) pack->norms=(short *)((long)(pack->norms)+(long)baseAddress);
	    if(pack->uvs[0]!=NULL) pack->uvs[0]=(short *)((long)(pack->uvs[0])+(long)baseAddress);
	    if(pack->uvs[1]!=NULL) pack->uvs[1]=(short *)((long)(pack->uvs[1])+(long)baseAddress);
	    if(pack->uvs[2]!=NULL) pack->uvs[2]=(short *)((long)(pack->uvs[2])+(long)baseAddress);
	    if(pack->index!=NULL) pack->index=(unsigned short *)((long)(pack->index)+(long)baseAddress);

	    if(pack->verts!=NULL) MDU_EndianShort( ( short * )(pack->verts),pack->n_verts*4 ) ;
	    if(pack->norms!=NULL) MDU_EndianShort( ( short * )(pack->norms),pack->n_verts*4 ) ;
	    if(pack->uvs[0]!=NULL) MDU_EndianShort( ( short * )(pack->uvs[0]),pack->n_verts*2 ) ;
	    if(pack->uvs[1]!=NULL && (long)(pack->uvs[1])!=(long)(pack->uvs[0])){
		MDU_EndianShort( ( short * )(pack->uvs[1]),pack->n_verts*2 ) ;
	    }
	    if(pack->uvs[2]!=NULL && (long)(pack->uvs[2])!=(long)(pack->uvs[0])){
		MDU_EndianShort( ( short * )(pack->uvs[2]),pack->n_verts*2 ) ;
	    }

	    if ( !(sdef->data_format & MGS_MODEL_FLAG_INDEX) ) {
		pack->index = NULL ;
		pack->n_indices = 0 ;
	    }

	    if(pack->index!=NULL){
		MDU_EndianShort( ( short * )(pack->index), pack->n_indices );
	    }
	}

	/* 2002/6/4  K.Kano
	   X-BOX用共有頂点情報の処理の追加 [[Additional information is processed for a shared vertex]] */
	if ( (sdef->data_format & MGS_MODEL_FORMAT_MASK) == MGS_MODEL_NORM ) {
	    obj->vbuff = NULL ;
	}
	if(obj->vbuff!=NULL){
	    obj->vbuff=(void *)((long)(obj->vbuff)+(long)baseAddress);
	    MDU_EndianShort((short *)(obj->vbuff),obj->stride*obj->n_verts/sizeof(short));
	}
    }
}

static KMS2_DEF *Kms2_Restruct(KMS2_DEF *sdef)
{
    KMS2_DEF *ddef;
    KMS2_MDL *smdl,*dmdl;
    long size;
    int i,j;

    ddef=Kms2_Alloc(sdef->n_models,sdef->n_x_models,&size);
    if(ddef==NULL) return NULL;
    memcpy(ddef,sdef,size);

    smdl=sdef->models;
    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,dmdl++){
	dmdl->packs=NULL;
	dmdl->vbuff=NULL;
    }

    dmdl=ddef->models;

    for(i=0;i<sdef->n_x_models;i++,smdl++,dmdl++){
	KMS2_MDLPACK *spack,*dpack;

	if((dmdl->packs=Kms2_MdlPack_Alloc(dmdl->n_packs,&size))==NULL){
	    Kms2_Free(ddef);
	    return NULL;
	}
	memcpy(dmdl->packs,smdl->packs,size);

	dpack=dmdl->packs;

	for(j=0;j<dmdl->n_packs;j++,dpack++){
	    dpack->verts=NULL;
	    dpack->norms=NULL;
	    dpack->uvs[0]=NULL;
	    dpack->uvs[1]=NULL;
	    dpack->uvs[2]=NULL;
	    dpack->rgbs=NULL;
	    dpack->index=NULL;
	}

	spack=smdl->packs;
	dpack=dmdl->packs;
	for(j=0;j<dmdl->n_packs;j++,spack++,dpack++){
	    if(spack->verts!=NULL){
		if((dpack->verts=Kms2_Verts_Alloc(dpack->n_verts,&size))==NULL){
		    Kms2_Free(ddef);
		    return NULL;
		}
		memcpy(dpack->verts,spack->verts,size);
	    }

	    if(spack->norms!=NULL){
		if((dpack->norms=Kms2_Norms_Alloc(dpack->n_verts,&size))==NULL){
		    Kms2_Free(ddef);
		    return NULL;
		}
		memcpy(dpack->norms,spack->norms,size);
	    }

	    if(spack->uvs[0]!=NULL){
		if(spack->flag & DG_PACKFLAG_UV0){
		    if((dpack->uvs[0]=Kms2_Uvs_Alloc(dpack->n_verts,&size))==NULL){
			Kms2_Free(ddef);
			return NULL;
		    }
		    memcpy(dpack->uvs[0],spack->uvs[0],size);
		}
	    }

	    if(spack->uvs[1]!=NULL){
		if(spack->flag & DG_PACKFLAG_UV1){
		    if((dpack->uvs[1]=Kms2_Uvs_Alloc(dpack->n_verts,&size))==NULL){
			Kms2_Free(ddef);
			return NULL;
		    }
		    memcpy(dpack->uvs[1],spack->uvs[1],size);
		}
	    }

	    if(spack->uvs[2]!=NULL){
		if(spack->flag & DG_PACKFLAG_UV2){
		    if((dpack->uvs[2]=Kms2_Uvs_Alloc(dpack->n_verts,&size))==NULL){
			Kms2_Free(ddef);
			return NULL;
		    }
		    memcpy(dpack->uvs[2],spack->uvs[2],size);
		}
	    }

	    if(spack->index!=NULL && dpack->n_indices != 0 ) {
		if((dpack->index=MDU_Alloc(dpack->n_indices * sizeof( unsigned short )))==NULL){
		    Kms2_Free(ddef);
		    return NULL;
		}
		memcpy(dpack->index, spack->index, dpack->n_indices * sizeof( unsigned short ));
	    }
#if 0
	    if(spack->rgbs!=NULL){
		size=sizeof(CVECTOR)*dpack->n_verts;
		if((dpack->rgbs=MDU_Alloc(size))==NULL){
		    Kms2_Free(ddef);
		    return NULL;
		}
		memcpy(dpack->rgbs,spack->rgbs,size);
	    }
#endif
	}

#if 0 //BP_XBOX - For PS2 models the data checked here (vbuff in particular) isn't guranteed to be NULL because the variable that vbuff is stored in used to simply be a "pad" variable on the PS2.

	/* 2002/6/4  K.Kano
	   X-BOX用共有頂点情報の処理の追加 [[Additional information is processed for a shared vertex]] */
	if(smdl->vbuff!=NULL && dmdl->n_verts>0){
	    if((dmdl->vbuff=MDU_Alloc(dmdl->stride*dmdl->n_verts))==NULL){
		Kms2_Free(ddef);
		return NULL;
	    }
	    memcpy(dmdl->vbuff,smdl->vbuff,dmdl->stride*dmdl->n_verts);
	}
#endif
    }

    return ddef;
}

/* .kmsロード */
KMS2_DEF *MDU_LoadKms2(char *name)
{
   KMS2_DEF	*sdef, *sdef2 ;
   char* topAddr;
   int kmsOffset = 0;

   topAddr = (char*)FileReadAll(name);

   for( kmsOffset = 0; ; ++kmsOffset )
   {
      if( topAddr[kmsOffset] != 0 )
         break;
   }

   sdef = (KMS2_DEF *)(topAddr + kmsOffset);
   if ( sdef == NULL ) return NULL ;

   Kms2_ChangeEndianAndAddOffset(sdef, topAddr);
   sdef2=Kms2_Restruct(sdef);
   
   MDU_Free(topAddr);

   return sdef2;
}

/*-----------------------------------------------------------------*/

static void Evm_ChangeEndianAndAddOffset(EVM_DEF *sdef)
{
    EVM_PACK *pack;
    long size;
    int i;

    size=sizeof(EVM_DEF);
    MDU_EndianLong(sdef,size/sizeof(long));

    size=sizeof(EVM_SKEL)*sdef->n_x_models;
    MDU_EndianLong(sdef->skeleton,size/sizeof(long));

    sdef->packet=(EVM_PACK *)((long)(sdef->packet)+(long)sdef);
    pack=sdef->packet;

    size=sizeof(EVM_PACK)*sdef->n_packs;
    MDU_EndianLong(pack,size/sizeof(long));

    for(i=0;i<sdef->n_packs;i++,pack++){
	if(pack->verts!=NULL) pack->verts=(void *)((long)(pack->verts)+(long)sdef);
	if(pack->norms!=NULL) pack->norms=(void *)((long)(pack->norms)+(long)sdef);
	if(pack->index!=NULL) pack->index=(void *)((long)(pack->index)+(long)sdef);/* Nisino 追加 */
	if(pack->uvs[0]!=NULL) pack->uvs[0]=(void *)((long)(pack->uvs[0])+(long)sdef);
	if(pack->uvs[1]!=NULL) pack->uvs[1]=(void *)((long)(pack->uvs[1])+(long)sdef);
	if(pack->uvs[2]!=NULL) pack->uvs[2]=(void *)((long)(pack->uvs[2])+(long)sdef);
	if(pack->weight!=NULL) pack->weight=(void *)((long)(pack->weight)+(long)sdef);
#if 0
	if(pack->rgbs!=NULL) pack->rgbs=(void *)((long)(pack->rgbs)+(long)sdef);
#endif

	MDU_EndianShort(pack->verts,pack->n_verts*4);
	MDU_EndianShort(pack->norms,pack->n_verts*4);
	MDU_EndianShort(pack->index,pack->n_indeices);/* Nisino 追加 [[Added]] */
	MDU_EndianShort(pack->uvs[0],pack->n_verts*2);
	if((long)(pack->uvs[1])!=(long)(pack->uvs[0])){
	    MDU_EndianShort(pack->uvs[1],pack->n_verts*2);
	}
	if((long)(pack->uvs[2])!=(long)(pack->uvs[0])){
	    MDU_EndianShort(pack->uvs[2],pack->n_verts*2);
	}
    }
}

static EVM_DEF *Evm_Restruct(EVM_DEF *sdef)
{
    EVM_DEF *ddef;
    EVM_PACK *spack,*dpack;
    long size;
    int i;

    if((ddef=Evm_Alloc(sdef->n_models,sdef->n_x_models,&size))==NULL) return NULL;
    memcpy(ddef,sdef,size);

    if((ddef->packet=Evm_Pack_Alloc(sdef->n_packs,&size))==NULL){
	Evm_Free(ddef);
	return NULL;
    }
    memcpy(ddef->packet,sdef->packet,size);

    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,dpack++){
	dpack->verts=NULL;
	dpack->norms=NULL;
	dpack->uvs[0]=NULL;
	dpack->uvs[1]=NULL;
	dpack->uvs[2]=NULL;
	dpack->weight=NULL;
	dpack->rgbs=NULL;
    }

    spack=sdef->packet;
    dpack=ddef->packet;
    for(i=0;i<sdef->n_packs;i++,spack++,dpack++){
	if((dpack->verts=Evm_Verts_Alloc(dpack->n_verts,&size))==NULL){
	    Evm_Free(ddef);
	    return NULL;
	}
	memcpy(dpack->verts,spack->verts,size);

	if((dpack->norms=Evm_Norms_Alloc(dpack->n_verts,&size))==NULL){
	    Evm_Free(ddef);
	    return NULL;
	}
	memcpy(dpack->norms,spack->norms,size);
	
	/* インデックス用 追加   NISINO [[Additional indices for]] */
	if((dpack->index=Evm_Indicess_Alloc(dpack->n_indices,&size))==NULL){
	    Evm_Free(ddef);
	    return NULL;
	}
	memcpy(dpack->index,spack->index,size);



	if(spack->flag & DG_PACKFLAG_UV0){
	    if((dpack->uvs[0]=Evm_Uvs_Alloc(dpack->n_verts,&size))==NULL){
		Evm_Free(ddef);
		return NULL;
	    }
	    memcpy(dpack->uvs[0],spack->uvs[0],size);
	}

	if(spack->flag & DG_PACKFLAG_UV1){
	    if((dpack->uvs[1]=Evm_Uvs_Alloc(dpack->n_verts,&size))==NULL){
		Evm_Free(ddef);
		return NULL;
	    }
	    memcpy(dpack->uvs[1],spack->uvs[1],size);
	}

	if((spack->flag & DG_PACKFLAG_UV2) ||
	   (spack->flag & DG_PACKFLAG_BMAP)){

	    if((dpack->uvs[2]=Evm_Uvs_Alloc(dpack->n_verts,&size))==NULL){
		Evm_Free(ddef);
		return NULL;
	    }
	    memcpy(dpack->uvs[2],spack->uvs[2],size);
	}

	if((dpack->weight=Evm_Weight_Alloc(dpack->n_verts,&size))==NULL){
	    Evm_Free(ddef);
	    return NULL;
	}
	memcpy(dpack->weight,spack->weight,size);
    }

    return ddef;
}

EVM_DEF *MDU_LoadEvm(char *name)
{
	
    EVM_DEF *sdef,*sdef2 ;

    sdef=(EVM_DEF *)FileReadAll(name);
    if(sdef==NULL) return NULL ;

    Evm_ChangeEndianAndAddOffset(sdef);
    sdef2=Evm_Restruct(sdef);
    MDU_Free(sdef);

    return sdef2;
}

/*---------------------------------------------------------------*/
