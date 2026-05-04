/*********************************************

	エフェクト　水飛沫 どぼん　のランチャー関数

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
	float   radius ;
	float   intense ;
	int     alpha ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	float   d1 ;
	float   d2 ;
} Data ;

void *NewSphereSplush_0106Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSphereSplush( FVECTOR *,float ,float ,int  ) ;


	work.pos = &data->d0 ;
	work.radius = data->d1 ;
	work.intense = data->d2 ;
	work.alpha = ( int ) 0.000000  ;

	return NewSphereSplush( work.pos,
	                        work.radius,
	                        work.intense,
	                        work.alpha
) ;
}


/*********************************************

	エフェクト　水飛沫 どぼん　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
