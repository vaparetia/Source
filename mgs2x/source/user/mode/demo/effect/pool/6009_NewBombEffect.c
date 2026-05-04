/*********************************************

	エフェクト　効果 Ｃ４爆発　のランチャー関数

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
	int     mode ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	int     d1 ;
} Data ;

void *NewBombEffect_6009Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBombEffect( FVECTOR *,int  ) ;


	work.pos = &data->d0 ;
	work.mode = data->d1 ;

	return NewBombEffect( work.pos,
	                      work.mode
) ;
}


/*********************************************

	エフェクト　効果 Ｃ４爆発　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
