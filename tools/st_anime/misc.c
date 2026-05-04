/*
	ストリーム頂点アニメ

	出力用関数群

	T.Morita  Feb 21 2000
	$Id: misc.c,v 1.1 2000/08/14 06:35:52 usr04098 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#include "alltypes.h"
#include "allvars.h"

void apply_matrix( FVECTOR *o, FMATRIX *m, FVECTOR *i ) 
{
    o->vx = m->m[X][X]*i->vx + m->m[X][Y]*i->vy + m->m[X][Z]*i->vz + m->m[X][W]*i->vw ;
    o->vy = m->m[Y][X]*i->vx + m->m[Y][Y]*i->vy + m->m[Y][Z]*i->vz + m->m[Y][W]*i->vw ;
    o->vz = m->m[Z][X]*i->vx + m->m[Z][Y]*i->vy + m->m[Z][Z]*i->vz + m->m[Z][W]*i->vw ;
    o->vw = m->m[W][X]*i->vx + m->m[W][Y]*i->vy + m->m[W][Z]*i->vz + m->m[W][W]*i->vw ;
}
