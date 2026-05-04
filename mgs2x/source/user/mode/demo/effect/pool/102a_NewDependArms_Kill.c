/*********************************************

	エフェクト　火器_状況依存_ＮＯＴ_Ｍ９　のランチャー関数

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
	int     ng_mdl ;
	int     flags ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewDependArms_Kill_102aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDependArms_Kill( int ,int ,int  ) ;


	work.name = data->d0 ;
	work.ng_mdl = data->d1 ;
	work.flags = ( int ) 0.000000  ;

	return NewDependArms_Kill( work.name,
	                           work.ng_mdl,
	                           work.flags
) ;
}


/*********************************************

	エフェクト　火器_状況依存_ＮＯＴ_Ｍ９　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
