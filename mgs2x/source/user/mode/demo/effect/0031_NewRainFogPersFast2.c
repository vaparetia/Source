//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　高速移動カメラ前ガス（風対応）　のランチャー関数

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
	int     color ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
} Data ;

void *NewRainFogPersFast2_0031Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRainFogPersFast2( int ,int  ) ;

	IVECTOR * col ;

	work.name = data->d0 ;
	col = &data->d1 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewRainFogPersFast2( work.name,
	                            work.color
) ;
}


/*********************************************

	エフェクト　高速移動カメラ前ガス（風対応）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
