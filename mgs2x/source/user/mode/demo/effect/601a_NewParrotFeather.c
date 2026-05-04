//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 オウム羽根　のランチャー関数

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
	int     nkill ;
	float   fInitSpdMin ;
	float   fInitSpdMax ;
	float   fSpdMin ;
	float   fSpdMax ;
	float   fGravity ;
	float   fFlow ;
	int     nSeed ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	int     d3 ;
	float   d4 ;
	float   d5 ;
	float   d6 ;
	float   d7 ;
	float   d8 ;
	float   d9 ;
	int     d10 ;
} Data ;

void *NewParrotFeather_601aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewParrotFeather( FVECTOR *,FVECTOR *,int ,int ,float ,float ,float ,float ,float ,float ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_XYZ( &temp, rot, 2.000000  ) ;
	work.vec = &temp ;
	work.nNum = data->d3 ;
	work.nkill = ( int ) 0.000000  ;
	work.fInitSpdMin = data->d4 ;
	work.fInitSpdMax = data->d5 ;
	work.fSpdMin = data->d6 ;
	work.fSpdMax = data->d7 ;
	work.fGravity = data->d8 ;
	work.fFlow = data->d9 ;
	work.nSeed = data->d10 ;

	return NewParrotFeather( work.pos,
	                         work.vec,
	                         work.nNum,
	                         work.nkill,
	                         work.fInitSpdMin,
	                         work.fInitSpdMax,
	                         work.fSpdMin,
	                         work.fSpdMax,
	                         work.fGravity,
	                         work.fFlow,
	                         work.nSeed
) ;
}


/*********************************************

	エフェクト　効果 オウム羽根　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
