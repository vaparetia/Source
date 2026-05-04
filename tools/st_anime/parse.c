/*
	XSI parser

	XSIの文法用の構文解析用 補助関数

	T.Morita  Feb 21 2000
	$Id: parse.c,v 1.5 2000/08/14 06:35:52 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "alltypes.h"
#include "allvars.h"

int pre_animationset( char *name )
{
    int      i, j, k=0 ;
    FVECTOR *m, *p ;
    LIST *l ;
    ANIM *a ;

    for ( l=list ; l ; l=l->next )
    {
	if ( l->n_patch && l->n_mesh )
	{
	    l->n_anime = 0 ;
	    if ( !(l->anime = l->para = malloc( l->n_mesh * sizeof(ANIM) )) )
		return -1 ;
	    for ( i=0, p=l->patch ; i<l->n_patch ; i++, p++ )/* 有効な頂点を探す */
		for ( j=0, m=l->mesh ; j<l->n_mesh ; j++,m++ )
		    if ( m->vx == p->vx && m->vy == p->vy && m->vz == p->vz && m->vw == -1 )
		    {
			m->vw = i+1 ;
			k++ ;
			break ;
		    }
	    printf( "%s\t%d vertex animation (%d valid)\n", l->name, l->n_mesh, k ) ;
	}
	else if ( l->n_mesh )
	{
	    l->n_anime = 0 ;
	    if ( !(l->anime = l->para = malloc( l->n_mesh * sizeof(ANIM) )) )
		return -1 ;
	    for ( j=0, m=l->mesh ; j<l->n_mesh ; j++, m++, k++ )
		m->vw = j+1 ;
	    printf( "%s\t%d vertex animation (%d valid)\n", l->name, l->n_mesh, k ) ;
	}

	/* アニメの初期化 */
	for ( a=l->anime, i=l->n_mesh ; --i>=0 ; a++ )
	    a->vert_id = -1 ;
    }

    return 0 ;
}


/*

mesh
meshmateriallist
texturefilename
si_ambience
si_angle
si_animationkey
si_animationparamkey
si_camera
si_coordinatesystem
si_envelope
si_envelopelist
si_fog
si_framebaseposematrix
si_light
si_material
si_meshface
si_meshnormals
si_patchsurface
si_meshtexturecoords
si_meshvertexcolors
si_spotlightparam
si_vertexweight
*/


int (*post_actions[])( char *name ) =
{
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
} ;
int (*pre_actions[])( char *name ) =
{
    pre_animationset, NULL,NULL,NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
} ;

