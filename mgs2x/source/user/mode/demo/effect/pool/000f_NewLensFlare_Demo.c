/*********************************************

	エフェクト　カメラ レンズフレア　のランチャー関数

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
	int     name ;
	FVECTOR *center ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
} Data ;

void *NewLensFlare_Demo_000fLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewLensFlare_Demo( int ,FVECTOR * ) ;


	work.name = data->d0 ;
	work.center = &data->d1 ;

	return NewLensFlare_Demo( work.name,
	                          work.center
) ;
}


/*********************************************

	エフェクト　カメラ レンズフレア　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
