/*********************************************

	エフェクト　プラント海面強制波発生　のランチャー関数

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
	FVECTOR *pos0 ;
	FVECTOR *pos1 ;
	float   pow ;
	int     flags ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d2 ;
	int     d3 ;
} Data ;

void *TAKABE_RiseWaveEx_b040Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *TAKABE_RiseWaveEx( FVECTOR *,FVECTOR *,float ,int  ) ;


	work.pos0 = &data->d0 ;
	work.pos1 = &data->d1 ;
	work.pow = data->d2 ;
	work.flags = data->d3 ;

	return TAKABE_RiseWaveEx( work.pos0,
	                          work.pos1,
	                          work.pow,
	                          work.flags
) ;
}


/*********************************************

	エフェクト　プラント海面強制波発生　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
