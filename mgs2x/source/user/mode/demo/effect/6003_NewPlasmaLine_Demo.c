//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 プラズマ（ライン）　のランチャー関数

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
	int     radius ;
	int     life ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	int     d2 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewPlasmaLine_Demo_6003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPlasmaLine_Demo( FVECTOR *,FVECTOR *,int ,int  ) ;

	FVECTOR * temp ;
	int      max ;
	int      min ;

	temp = &data->d0 ;
	work.pos0 = temp ;
	work.pos1 = &data->d1 ;
	work.radius = data->d2 ;
	max = data->d3 ;
	min = data->d4 ;
	work.life = (max - min) ;

	return NewPlasmaLine_Demo( work.pos0,
	                           work.pos1,
	                           work.radius,
	                           work.life
) ;
}


/*********************************************

	エフェクト　効果 プラズマ（ライン）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
