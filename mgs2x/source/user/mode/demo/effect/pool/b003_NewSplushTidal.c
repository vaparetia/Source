/*********************************************

	エフェクト　水効果_巨大水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/


#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "../libdemo.h"



typedef struct work_t {
	FMATRIX *pos ;
	FVECTOR *vec0 ;
	FVECTOR *vec1 ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d3 ;
	FVECTOR d4 ;
} Data ;

void *NewSplushTidal_b003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplushTidal( FMATRIX *,FVECTOR *,FVECTOR * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos = DM_GetObjectMatrix( handle, link->vy ) ;
	work.vec0 = &data->d3 ;
	work.vec1 = &data->d4 ;

	return NewSplushTidal( work.pos,
	                       work.vec0,
	                       work.vec1
) ;
}


/*********************************************

	エフェクト　水効果_巨大水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
