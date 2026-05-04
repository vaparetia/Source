/*
	ストリーム頂点アニメ

	出力用関数群

	T.Morita  Feb 21 2000
	$Id: output.c,v 1.5 2001/11/30 04:12:08 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "alltypes.h"
#include "allvars.h"

int match_vertex( ANIM **idx, CV2_OBJS *c, float scale )
{
    LIST *l ;
    int   i, j, k, cnt=0 ;
    FVECTOR  *v, *m   ;
    int   flag = 0 ;
    ANIM  *a ;

    for ( j=n_verts, v=c->objs[0].verts ; --j>=0 ; v++ )
    {
	for ( l=list ; l ; l=l->next )
	    if ( l->n_mesh )
	    {
		for ( k=0, m=l->mesh ; k<l->n_mesh ; k++,m++ )
		    if ( !(int)((m->vx*scale - v->vx)*1.0f) &&
			 !(int)((m->vy*scale - v->vy)*1.0f) &&
			 !(int)((m->vz*scale - v->vz)*1.0f) )
			for ( i=l->n_anime, a=l->anime ; --i>=0 ; a++ )
			    if ( a->vert_id == k )
			    {
				cnt++ ;
				idx[j] = a ;
				goto next_search ;
			    }
		if ( k<0 )
		    printf( "Do not match any vertexes in CV2(%f %f %f)\n",
			    v->vx,v->vy,v->vz ), flag = 1 ;
	    }
    next_search:
	;
    }
    if ( cnt != n_verts )
	printf( "output vertexes not equivalant for scale %d( %d(output) != %d(n_anime)))\n",
		(int)scale, cnt, n_verts ), flag = 1 ;
    return flag ;
}

int output_anm( char *xsi_name, CV2_OBJS *c )
{
    int       i, j ;
    FILE     *out ;
    ANIM    **idx ;
#if 0
    CV2_OBJS *c   ;
    extern CV2_OBJS *MDU_LoadCv2( char *name ) ;

    /* load CV2 */
    if ( !(c = MDU_LoadCv2( cv2_name )) )
    {
	fprintf( stderr, "No Cv2-file<%s>!!\n", cv2_name ) ;
	return 1 ;
    }
#endif
    n_verts = c->objs[0].n_verts ;

    /* ( MESH -> CV2 )のインデックスを作る */
    idx = malloc( sizeof(ANIM*) * c->objs[0].n_verts ) ;
    if ( match_vertex( idx, c, scaling ) )  /* scalingでテストしてみる */
	return 1 ;
    printf( "Successfully indexes are made.\n" ) ;

    /* output to the file */
    if ( open_file_with_ext( &out, xsi_name, "anm", "wb" ) )
	return -2 ;

    //fwrite( &n_frame           , sizeof(int), 1, out ) ;
    //fwrite( &c->objs[0].n_verts, sizeof(int), 1, out ) ;
    printf( "%d frames of Animation\n", n_frame ) ;
    for ( i=0 ; i<n_frame ; i+=skip_frame+1 )
    {
	if ( flag & FLG_VERTEX )
	    for ( j=n_verts ; --j>=0 ; )
		fwrite( &idx[j]->verts[i*XYZ], sizeof(short), XYZ, out ) ;
	if ( flag & FLG_NORMAL )
	    for ( j=n_verts ; --j>=0 ; )
		fwrite( &idx[j]->norms[i*XYZ], sizeof(short), XYZ, out ) ;
	printf( "." ) ;
	fflush( stdout ) ;
    }
    fclose( out ) ;
    printf( "\nSuccessfully written.\n" ) ;

    free( idx ) ;

    return 0 ;
}
