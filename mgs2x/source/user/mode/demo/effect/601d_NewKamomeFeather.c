//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 カモメ羽根　のランチャー関数

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
	FVECTOR *vec ;
	int     nNum ;
	int     nSeed ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewKamomeFeather_601dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewKamomeFeather( FVECTOR *,FVECTOR *,int ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_XYZ( &temp, rot, 2.000000  ) ;
	work.vec = &temp ;
	work.nNum = data->d3 ;
	work.nSeed = data->d4 ;

	return NewKamomeFeather( work.pos,
	                         work.vec,
	                         work.nNum,
	                         work.nSeed
) ;
}


/*********************************************

	エフェクト　効果 カモメ羽根　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
