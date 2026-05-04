/*
  geo_convert.c
  Scn(VRS) -> Geo(MGS2) コンバータ
   
  2002.11.6 T.Morita
   
  $Id: geo_file.c,v 1.7 2002/12/02 06:38:13 usr04098 Exp $
  */

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<fcntl.h>
#include 	<math.h>
#include	<sys/types.h> 

#include	"scn2geo.h"



static LRESULT file_read_interrupt( WPARAM wParam, LPARAM lParam)	
{
    printf( "." ) ;
    return 1;
}

/*

  SCNファイルをロード

  */
HP3DMODEL file_scn_load( char *file_name )
{
    HCNMSCENE      cnm_scn;
    HP3DSCENE      scn;
    HP3DSCENEACTOR scn_actor;
    HP3DACTOR actor ;
    HP3DMODEL model ;

    printf( "Reading....." ) ;
    if ( (cnm_scn = CnmReadFileScene( file_name,
									  file_read_interrupt,
									  (LPARAM)"reading")) == 0 )
		return 0 ;
    printf( "Done.\n" ) ;

    scn = CnmGetSceneP3DScene( cnm_scn ) ;
    scn_actor = P3DGetFirstSceneActor( scn ) ;
    actor = P3DGetSceneActorActor( scn_actor ) ;
    model = P3DGetActorModel( actor ) ;
    P3DInitMovementObject( model, NULL, TRUE ) ;
    P3DInitObjectMatrix( model, NULL, TRUE ) ;
    P3DObjectMatrix( model, NULL, TRUE ) ;
    P3DObjectMatrixCoord( model, NULL, TRUE ) ;

    return model ;
}

/* 

   GEO_DEF ファイルセーブ

 */
int file_geo_save( GEO_DEF *def, char *file_name )
{
    FILE *fp  ;
    int   i   ;

    fp = fopen( file_name, "wb" ) ;
    if ( fp == NULL ){
		printf( "file_geo_save : cannot open file<%s>\n", file_name ) ;
		return -1 ;
    }

    /* 実アドレスをオフセットに変える */
    core_geodef_addr_to_offset( def ) ;

    /* とりあえずヘッダを入れてしまう */
    fwrite( def, def->size, 1, fp ) ;

    fclose( fp ) ;

    return 0 ;
}

static int file_get_filesize( FILE *fp )
{
    int size ;

    fseek( fp, 0, SEEK_END ) ;
    size = ftell( fp ) ;
    rewind( fp ) ;
    return size ;
}

/*

  GEO_DEF の file をロードする

*/
GEO_DEF *file_geo_load( char *file_name )
{
    FILE    *fp   ;
    int      size ;
    GEO_DEF *def  ;
    GEO_DEF  head ;

    fp = fopen( file_name, "rb" ) ;
    if ( fp == NULL ){
		printf( "file_geo_load : cannot open file<%s>\n", file_name ) ;
		return NULL ;
    }

    size = file_get_filesize( fp ) ;
    def  = mem_alloc( size ) ;
    if ( def == NULL ) {
		printf( "file_geo_load : no memory for load.\n" ) ;
		return NULL ;
    }

    if ( fread( def, size, 1, fp ) != 1 ){
		printf( "file_geo_load : reading error file<%s>.\n", file_name ) ;
		fclose( fp ) ;
		return NULL ;
    }

    fclose( fp ) ;

    /* オフセットを実アドレスに変える */
    core_geodef_offset_to_addr( def ) ;

    return def ;
}




FVECTOR vvv[2048] ;
int     n_vvv = 0 ;
int     n_vvv_total = 0 ;
int     n_vv = 0 ;
int     n_vv_total = 0 ;
void test_regist( SVECTOR *p, FVECTOR *cen ) 
{
    int j ;
    FVECTOR v ;

    ADD_FVECTOR( &v, p, cen  ) ;
    for ( j=n_vvv ; --j>=0 ; ) {
		if ( fabs(v.vx-vvv[j].vx) < 0.8f &&
			 fabs(v.vy-vvv[j].vy) < 0.8f &&
			 fabs(v.vz-vvv[j].vz) < 0.8f ) return ;
    }
    COPY_FVECTOR( &vvv[n_vvv], &v ) ;
    n_vvv++ ;
}


/*

  GEOファイルをダンプする

 */
int file_dump_geom( GEOM *geom )
{
    void *addr = geom ;
    int i, n_prim ;

    printf( "    GEOM[%x]\n", geom ) ;

    {
		printf( "      HEAD FLG(%08x) PRV(%d) NXT(%d) CHLD(%d)\n",
				geom->flag, geom->prev, geom->next, geom->child ) ;
		printf( "           NAME(%08x) N_PRIM(%d)\n",
				GEO_GET_NAME(geom), GEO_GET_NPRIM(geom) ) ;
		addr = GEO_GeomGetPrim( geom ) ;

		addr = geom+1 ;
    }

    i = n_prim = GEO_GET_NPRIM( geom ) ;
    while( --i>=0  )
    {
		printf( "      PRIM%3d ", n_prim-i ) ;
		switch( GEO_GeomGetPrimType( geom ) ) {
		case GEO_F_PRIM_NONE:
		{
			printf( "PrimNone" ) ;
			break ;
		}
		case GEO_F_PRIM_DOT :
		{
			GEO_DOT *p = addr ;
			printf( " DOT ATTR %08x ROOT %08x\n",
					p->size.attribute,
					p->pos.root ) ;
			printf( "               OFFS(%.1f %.1f %.1f) SIZE(%.1f %.1f %.1f)\n",
					p->pos.vx,p->pos.vy,p->pos.vz,
					p->size.vx,p->size.vy,p->size.vz ) ;
			addr = p+1 ;
			break ;
		}
		case GEO_F_PRIM_LINE:
		{
			GEO_LINE *p = addr ;
			printf( "LINE ATTR %08x ROOT %08x\n",
					p->size.attribute,
					p->from.root ) ;
			printf( "                FROM(%.1f %.1f %.1f) TO(%.1f %.1f %.1f)\n",
					p->from.vx,p->from.vy,p->from.vz,
					p->to.vx,p->to.vy,p->to.vz ) ;
			printf( "                SIZE(%.1f %.1f %.1f)\n",
					p->size.vx,p->size.vy,p->size.vz ) ;
			addr = p+1 ;
			break ;
		}
		case GEO_F_PRIM_POLY:
		{
			GEO_POLY *p = addr ;
			SVECTOR  *sv ;
			int      idx[6] ;

			idx[0] =  (p->index.low  >> 0 ) & 0x3ff ;
			idx[1] =  (p->index.low  >> 10) & 0x3ff ;
			idx[2] =  (p->index.low  >> 20) & 0x3ff ;
			idx[3] = ((p->index.low  >> 30) & 0x003) | ((p->index.high<<2) & 0x3fc) ;
			idx[4] =  (p->index.high >>  8) & 0x3ff ;
			idx[5] =  (p->index.high >> 18) & 0x3ff ;

			printf( "POLY  ATTR %08x ROOT %08x\n",
					p->attribute, p->root ) ;
			printf( "                 CENTER1(%d %d %d) r%d\n",
					p->cen1.vx,p->cen1.vy,p->cen1.vz, p->cen1.pad ) ;
			printf( "                 CENTER2(%d %d %d) r%d\n",
					p->cen2.vx,p->cen2.vy,p->cen2.vz, p->cen2.pad ) ;
			printf( "                 INDEX %08x%08x ->v%d %d %d %d n%d base%d\n",
					p->index.high, p->index.low,
					idx[0] ,idx[1] ,idx[2] ,
					idx[3] ,idx[4] ,idx[5] ) ;
			sv = (void*)p ;
			printf( "                 POS0[%d](%d %d %d)\n",
					idx[0],(sv+idx[0])->vx,(sv+idx[0])->vy,(sv+idx[0])->vz ) ;
			printf( "                 POS1[%d](%d %d %d)\n",
					idx[1],(sv+idx[1])->vx,(sv+idx[1])->vy,(sv+idx[1])->vz ) ;
			printf( "                 POS2[%d](%d %d %d)\n",
					idx[2],(sv+idx[2])->vx,(sv+idx[2])->vy,(sv+idx[2])->vz ) ;
			printf( "                 POS3[%d](%d %d %d)\n",
					idx[3],(sv+idx[3])->vx,(sv+idx[3])->vy,(sv+idx[3])->vz ) ;
			printf( "                 NORMAL[%d](%d %d %d)\n",
					idx[4],(sv+idx[4])->vx,(sv+idx[4])->vy,(sv+idx[4])->vz ) ;
			printf( "                 BASE[%d](%f %f %f)\n",
					idx[5],((FVECTOR*)(sv+idx[5]))->vx,((FVECTOR*)(sv+idx[5]))->vy,((FVECTOR*)(sv+idx[5]))->vz ) ;

			n_vvv_total += 3 ;

			addr = p+1 ;
			break ;
		}
		case GEO_F_PRIM_BOX :
		{
			GEO_BOX *p = addr ;
			printf( "BOX ATTR %08x ROOT %08x\n",
					p->size.attribute,
					p->pos.root ) ;
			printf( "               OFFS(%.1f %.1f %.1f) SIZE(%.1f %.1f %.1f)\n",
					p->pos.vx,p->pos.vy,p->pos.vz,
					p->size.vx,p->size.vy,p->size.vz ) ;
			addr = p+1 ;
			break ;
		}
		case GEO_F_PRIM_FIELD:
		{
			GEO_FIELD *p = addr ;
			printf( "FIELD  ATTR %08x ROOT %08x\n",
					p->size.attribute,
					p->pos.root ) ;
			printf( "                  OFFS(%.1f %.1f %.1f)\n"
					"                  SIZE(%.1f %.1f %.1f)\n",
					p->pos.vx,p->pos.vy,p->pos.vz,
					p->size.vx,p->size.vy,p->size.vz ) ;
			addr = p+1 ;
			break ;
		}
		default:
			printf( "undefined PrimType\n" ) ;
		}
	
		if ( geom->flag & GEO_F_HAS_CALLBACK ) {
			GEO_CALLUNIT *p = addr ;
			printf( "      CALLUNIT : %08x %08x\n",
					p->callback, p->calldata ) ;
			addr = p+1 ;
		}

#if 0
		if ( geom->flag & GEO_F_HAS_UNITDATA ) {
			GEO_PLUGIN *p = GEO_GeomGetPluginFrom( geom ) ;
			int         i ;
			u_int      *data = addr ;
	    
			printf( "  UNIT : \n" ) ;
			for( i=0 ; i<(p->unit_size+3)/4 ; i++ ){
				if ( (i&3) == 0 ) printf( "        " ) ;
				printf( " %x", *data++ ) ;
				if ( (i&3) == 3 ) printf( "\n" ) ;
			}
		}
#endif

    }
    return n_prim ;
}

void file_dump_geo(GEO_DEF *def )
{
    int i, j ;
    GEO_CHANK *chank ;
    GEO_GROUP *grp ;

    printf( "GEO_DEF version  %08x\n", def->version  ) ;
    printf( "        size     %d\n"  , def->size     ) ;
    printf( "        n_chanks %d\n"  , def->n_chanks ) ;
    printf( "        chanks   %08x\n", def->chanks   ) ;

    chank = &def->chanks[0] ;
    printf( "GEO_CHANK type  %04x\n", chank->type  ) ;
    printf( "          dummy %04x\n", chank->dummy ) ;
    printf( "          size  %d\n"  , chank->size  ) ;

    grp = (GEO_GROUP *)chank->data ;
    for ( ; ; ) {
		int n_block = 0 ;
		int n_geom  = 0 ;
		int n_prim  = 0 ;

        printf( "  GEO_GROUP base  %.0f %.0f %.0f\n", grp->base.vx, grp->base.vy, grp->base.vz ) ;
        printf( "            div   %d %d %d\n", grp->div.vx, grp->div.vy, grp->div.vz ) ;
        printf( "            max   %d %d %d\n", grp->max.vx, grp->max.vy, grp->max.vz ) ;
        printf( "            flag       %08x\n", grp->flag ) ;
        printf( "            n_types    %d\n", grp->n_types ) ;
        printf( "            radix_size %d\n", grp->radix_size ) ;

		for ( i=0 ; i<grp->max.vx*grp->max.vy*grp->max.vz ; i++ ) {
			GEO_RADIX *radix ;
			GEO_BLOCK *block ;
			GEOM      *g ;
			int offset ;

			radix = (GEO_RADIX *)((u_int)grp->radix + grp->radix_size * i) ;
			if ( radix->offset == GEO_RADIX_NONE ) continue ;

			for ( j=0 ; j<grp->n_types ; j++ ){
				offset = core_group_radix_get_offset( radix, j ) ;
				if ( offset || !j ) {
					block = grp->block + radix->offset + offset ;

					printf( "    -------------------------------------------------\n" )  ;
					printf( "    GEO_BLOCK [%08x] %d %d\n", block, radix->offset, offset ) ;
					printf( "              flag   %08x\n", block->flag   ) ;
					printf( "              size   %d\n"  , block->size   ) ;
					printf( "              n_geom %d\n"  , block->n_geom ) ;
					printf( "              geom   %x\n"  , block->geom   ) ;
					while( block->size >= 0 ) {
						for ( g=block->geom ; g ; g=GEO_LINKNEXT(g) ){
							n_prim += file_dump_geom( g ) ;
							n_geom++ ;	    
						}

						if ( block->flag & GEO_BLK_END ) break ;
						block++ ;
						n_block++ ;
					}
					printf( "    GEO_POLY vertex min/total  %d/%d %f%%\n", n_vvv, n_vvv_total, 100.0f*n_vvv/n_vvv_total ) ;
					n_vv_total += n_vvv_total ;
					n_vv       += n_vvv       ;
					n_vvv_total = n_vvv = 0 ;
				}
			}
		}
		printf( "  %d blocks\n  %d geoms\n  %d prims\n", n_block, n_geom, n_prim ) ;
		printf( "  vertex min/total  %d/%d %f%%\n", n_vv, n_vv_total, 100.0f*n_vv/n_vv_total ) ;
		n_vv_total = n_vv = 0 ;

		if ( grp->flag & GEO_GRP_END ) break ;
        grp++ ;
    }

    for ( i=1 ; i<def->n_chanks ; i++ ) {
		chank = &def->chanks[i]  ;

		printf( "GEO_CHANK type  %04x\n", chank->type  ) ;
		printf( "GEO_CHANK dummy %04x\n", chank->dummy ) ;
		printf( "GEO_CHANK size  %d\n"  , chank->size  ) ;
    }
}


void file_dump_radix(GEO_DEF *def )
{
    int i, j, x,y,z ;
    GEO_CHANK *chank ;
    GEO_GROUP *grp ;

    printf( "GEO_DEF version  %08x\n", def->version  ) ;
    printf( "        size     %d\n"  , def->size     ) ;
    printf( "        n_chanks %d\n"  , def->n_chanks ) ;
    printf( "        chanks   %08x\n", def->chanks   ) ;

    chank = &def->chanks[0] ;
    printf( "GEO_CHANK type  %04x\n", chank->type  ) ;
    printf( "          dummy %04x\n", chank->dummy ) ;
    printf( "          size  %d\n"  , chank->size  ) ;

    grp = (GEO_GROUP *)chank->data ;
    for ( ; ; ) {
		float hit ;

        printf( "  GEO_GROUP base  %.0f %.0f %.0f\n", grp->base.vx, grp->base.vy, grp->base.vz ) ;
        printf( "            div   %d %d %d\n", grp->div.vx, grp->div.vy, grp->div.vz ) ;
        printf( "            max   %d %d %d\n", grp->max.vx, grp->max.vy, grp->max.vz ) ;
        printf( "            flag       %08x\n", grp->flag ) ;
        printf( "            n_types    %d\n", grp->n_types ) ;
        printf( "            radix_size %d\n", grp->radix_size ) ;

		printf( "  GEO_RADIX\n" ) ;
		hit = 0 ;
		for ( y=0 ; y<grp->max.vy ; y++ ) {
		for ( z=0 ; z<grp->max.vz ; z++ ) {
		for ( x=0 ; x<grp->max.vx ; x++ ) {
			GEO_RADIX *radix ;

			i = grp->max.vx*grp->max.vz * y + grp->max.vx * z + x ; 
	    
			radix = (GEO_RADIX *)((u_int)grp->radix + grp->radix_size * i) ;
			printf( "      " ) ;
			printf( (radix->offset==GEO_RADIX_NONE ? "NONE" : "%4d"), radix->offset ) ;
			printf( "(%2d,%2d,%2d) : ", x,y,z ) ;
			for ( j=grp->n_types ; --j>=0 ; ){
				int offset = core_group_radix_get_offset( radix, j ) ;
				if ( j%4 == 3 ) {
					printf( "[%d]%d ", j, offset ) ;
				} else {
					printf( "%d ", offset ) ;
				}
			}
			if (radix->offset!=GEO_RADIX_NONE ) hit++ ;

			printf( "\n" ) ;
		}}}
		printf( "     Hit rate %.1f%% = %d/%d\n", 
				100.0f*hit/(grp->max.vx*grp->max.vy*grp->max.vz),
				(int)hit, (int)(grp->max.vx*grp->max.vy*grp->max.vz) ) ;
	    
		if ( grp->flag & GEO_GRP_END ) break ;
        grp++ ;
    }
}
