/*********************************************

	エフェクト　システム 文字出し　のランチャー関数

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
	char    *message ;
} Work ;

typedef struct data_t {
	char    d0[128] ;
} Data ;

void *DM_DebugPrint_fff00Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_DebugPrint( char * ) ;


	work.message = data->d0 ;

	return DM_DebugPrint( work.message
) ;
}


/*********************************************

	エフェクト　システム 文字出し　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
