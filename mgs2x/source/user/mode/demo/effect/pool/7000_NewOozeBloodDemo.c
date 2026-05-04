/*********************************************

	エフェクト　血 敵兵滲み血　のランチャー関数

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
	void    *objs ;
	int     model ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewOozeBloodDemo_7000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewOozeBloodDemo( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.model = data->d4 ;

	return NewOozeBloodDemo( work.name,
	                         work.objs,
	                         work.model
) ;
}


/*********************************************

	エフェクト　血 敵兵滲み血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
