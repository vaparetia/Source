/*
	list.c

	list管理関数

	T.Morita  Feb 21 2000
	$Id: list.c,v 1.2 2001/11/30 04:12:08 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alltypes.h"
#include "allvars.h"

static VAR *pool_head ;
static VAR *pool_put ;
static VAR *pool_get ;
static int  n_pool ;

/*
  VAR管理
 */
void init_varlist( int i )
{
    pool_head = malloc( i*sizeof(VAR) ) ;
    n_pool = i ;
    pool_put = pool_get = pool_head ;
}

void reset_varlist()
{
    pool_put = pool_get = pool_head ;
}


void add_to_varlist( VAR *a )
{
    *pool_put++ = *a ;
}

void free_varlist()
{
    free( pool_head ) ;
}

float get_float_from_varlist( )
{
    return (pool_get++)->f ;
}

float get_nth_float_from_varlist( int n )
{
    return pool_get[n].f ;
}

void get_fvec_from_varlist( FVECTOR *a )
{
    a->vx = pool_get[X].f ;
    a->vy = pool_get[Y].f ;
    a->vz = pool_get[Z].f ;
    a->vw = -1 ;
    pool_get += 3 ;
}

void get_poly_from_varlist( POLY *a )
{
    int i ;
    int n_verts = (int)(pool_put - pool_get) ;

    for ( i=0 ; i<n_verts ; i++ )
	a->v[i] = (int)(pool_get[i].f) ;
    for (  ; i<4 ; i++ )
	a->v[i] = -1 ;
    pool_get += n_verts ;
}

#define ANM_MAX_VEL 4096.0f
#define ANM_SCALE    100.0f
int get_fvec_sub_vert( short *svec, FVECTOR *fvec )
{
    int flag = 0 ;
    static FVECTOR prev = { 0,0,0,0 } ;

    get_fvec_from_varlist( fvec ) ;
#if 0
    if ( svec )
    {
	FVECTOR s ;
	s.vx = (fvec->vx - prev.vx) * ANM_SCALE ;
	s.vy = (fvec->vy - prev.vy) * ANM_SCALE ;
	s.vz = (fvec->vz - prev.vz) * ANM_SCALE ;
	//printf ( "prv(%.2f %.2f %.2f)", prev.vx, prev.vy, prev.vz  ) ;
	//printf ( "pref(%.2f %.2f %.2f)\n", s.vx, s.vy, s.vz  ) ;
	if ( s.vx > ANM_MAX_VEL || s.vx < -ANM_MAX_VEL ||
	     s.vy > ANM_MAX_VEL || s.vy < -ANM_MAX_VEL ||
	     s.vz > ANM_MAX_VEL || s.vz < -ANM_MAX_VEL )
	    flag = 1 ;
	svec[X] = (short)s.vx ;
	svec[Y] = (short)s.vy ;
	svec[Z] = (short)s.vz ;
    }
#else
    svec[X] = (short)(fvec->vx * ANM_SCALE) ;
    svec[Y] = (short)(fvec->vy * ANM_SCALE) ;
    svec[Z] = (short)(fvec->vz * ANM_SCALE) ;
#endif
    prev = *fvec ;
    return flag ;
}


/*
  LIST管理
 */
void make_name( char *dst, char *src )
{
    int   i,  n ;
    char *c, *e ;
    static char *pre[]  = { "frm-", "anim-", } ;
    static char *post[] = { "mesh", "patch", "cdn", } ;

    for ( i=0 ; i<sizeof(pre)/sizeof(char*) ; i++ )
	if ( src == (c=strstr( src, pre[i] )) )
	{
	    src += strlen( pre[i] ) ;
	    break ;
	}
    n = strlen( src ) ;
    for( c=src, e=NULL ; (c=strchr( c, '_' )) ; e=++c ) ;
    if ( e )
	for ( i=0 ; i<sizeof(post)/sizeof(char*) ; i++ )
	    if ( !strcmp( post[i], e ) )
	    {
		n -= strlen( post[i] )+1 ;
		break ;
	    }
    strncpy( dst, src, n ) ;
    dst[n] = '\0' ;
}

int check_name( char *name1, char *name2  )
{
    char n1[64], n2[64] ;

    make_name( n1, name1 ) ;
    make_name( n2, name2 ) ;
    return strcmp( n1, n2 ) ;
}

LIST *newnode_to_list( char *name )
{
    LIST *n = NULL ;

    if ( name )
	if ( (n = calloc( sizeof(LIST), 1 )) )
	{
	    n->next = list ;
	    list    = n    ;
	    make_name( n->name, name ) ;
	}
    return n ;
}

LIST *search_all_list( char *name )
{
    LIST *l ;

    if ( name )
	for( l=list ; l ; l=l->next )
	    if ( !check_name( name, l->name ) )
		return l ;
    return NULL ;
}

void delete_all_list()
{
    LIST *l, *n ;

    for( l=list ; l ; l=n )
    {
	n = l->next ;
	if ( l->patch )
	    free( l->patch ) ;
	if ( l->mesh )
	    free( l->mesh  ) ;
	if ( l->poly )
	    free( l->poly  ) ;
	if ( l->anime )
	{
	    if ( l->anime->verts )
		free( l->anime->verts ) ;
	    if ( l->anime->norms )
		free( l->anime->norms ) ;
	    if ( l->anime->org )
		free( l->anime->org   ) ;
	    free( l->anime ) ;
	}
	free( l ) ;
    }
}
