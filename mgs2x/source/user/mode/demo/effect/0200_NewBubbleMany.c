//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水中効果_追従バブル　のランチャー関数

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
	int     life ;
	int     color ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
	IVECTOR d5 ;
} Data ;

void *NewBubbleMany_0200Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBubbleMany( FVECTOR *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      max ;
	int      min ;
	IVECTOR * col ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos0 = DM_GetObjectPos( handle, link->vy ) ;
	max = data->d3 ;
	min = data->d4 ;
	work.life = (max - min) ;
	col = &data->d5 ;
	work.color = (((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) ;

	return NewBubbleMany( work.pos0,
	                      work.life,
	                      work.color
) ;
}


/*********************************************

	エフェクト　水中効果_追従バブル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
