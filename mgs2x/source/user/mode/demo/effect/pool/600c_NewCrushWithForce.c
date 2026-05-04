/*********************************************

	エフェクト　効果 ライン飛び散り　のランチャー関数

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
	FVECTOR *pos ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
} Data ;

void *NewCrushWithForce_600cLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCrushWithForce( FVECTOR * ) ;


	work.pos = &data->d0 ;

	return NewCrushWithForce( work.pos
) ;
}


/*********************************************

	エフェクト　効果 ライン飛び散り　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
