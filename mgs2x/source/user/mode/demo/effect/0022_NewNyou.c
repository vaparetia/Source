//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　失禁　のランチャー関数

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
	int     life ;
	float   size ;
	int     col ;
	int     yuge_flag ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	int     d1 ;
	float   d2 ;
	IVECTOR d3 ;
	int     d4 ;
} Data ;

void *NewNyou_0022Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewNyou( FVECTOR *,int ,float ,int ,int  ) ;

	IVECTOR * col ;

	work.pos = &data->d0 ;
	work.life = data->d1 ;
	work.size = data->d2 ;
	col = &data->d3 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.yuge_flag = data->d4 ;

	return NewNyou( work.pos,
	                work.life,
	                work.size,
	                work.col,
	                work.yuge_flag
) ;
}


/*********************************************

	エフェクト　失禁　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
