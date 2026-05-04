/*********************************************

	エフェクト　装備品 海兵隊　のランチャー関数

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
	void    *object ;
	short   ID ;
	int     flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
} Data ;

void *NewCreateEquipment_1012Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCreateEquipment( void *,short ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.ID = ( short ) 0.000000  ;
	work.flag = data->d3 ;

	return NewCreateEquipment( work.object,
	                           work.ID,
	                           work.flag
) ;
}


/*********************************************

	エフェクト　装備品 海兵隊　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
