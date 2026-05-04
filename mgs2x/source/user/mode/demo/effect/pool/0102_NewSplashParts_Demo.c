/*********************************************

	エフェクト　水飛沫 単発　のランチャー関数

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
	FVECTOR *center ;
	SVECTOR *rot ;
	float   intense ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
	float   d2 ;
} Data ;

void *NewSplashParts_Demo_0102Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplashParts_Demo( FVECTOR *,SVECTOR *,float  ) ;


	work.center = &data->d0 ;
	work.rot = &data->d1 ;
	work.intense = data->d2 ;

	return NewSplashParts_Demo( work.center,
	                            work.rot,
	                            work.intense
) ;
}


/*********************************************

	エフェクト　水飛沫 単発　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
