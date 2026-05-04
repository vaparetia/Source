//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙_ライン煙り　のランチャー関数

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
	FMATRIX *world ;
	FVECTOR *pos ;
	int     life ;
	float   pow ;
	float   max_size ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d3 ;
	int     d4 ;
	int     d5 ;
	float   d6 ;
	float   d7 ;
} Data ;

void *NewLineSmoke_7103Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewLineSmoke( FMATRIX *,FVECTOR *,int ,float ,float  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      max ;
	int      min ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.pos = &data->d3 ;
	max = data->d4 ;
	min = data->d5 ;
	work.life = (max - min) ;
	work.pow = data->d6 ;
	work.max_size = data->d7 ;

	return NewLineSmoke( work.world,
	                     work.pos,
	                     work.life,
	                     work.pow,
	                     work.max_size
) ;
}


/*********************************************

	エフェクト　煙_ライン煙り　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
