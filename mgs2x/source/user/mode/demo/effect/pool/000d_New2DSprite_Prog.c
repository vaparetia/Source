/*********************************************

	エフェクト　２Ｄ　スプライト表示　のランチャー関数

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
	int     tri_code ;
	int     tex_code ;
	FVECTOR *data ;
	int     wide ;
	int     high ;
	SVECTOR *time ;
	int     flags ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	int     d7 ;
	int     d8 ;
	SVECTOR d9 ;
	int     d10 ;
} Data ;

void *New2DSprite_Prog_000dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *New2DSprite_Prog( int ,int ,FVECTOR *,int ,int ,SVECTOR *,int  ) ;

	FVECTOR  temp ;

	work.tri_code = data->d0 ;
	work.tex_code = data->d1 ;
	DM_InFVector( &temp, data->d2, data->d3, data->d4, data->d5 ) ;
	work.data = &temp ;
	work.wide = data->d7 ;
	work.high = data->d8 ;
	work.time = &data->d9 ;
	work.flags = data->d10 ;

	return New2DSprite_Prog( work.tri_code,
	                         work.tex_code,
	                         work.data,
	                         work.wide,
	                         work.high,
	                         work.time,
	                         work.flags
) ;
}


/*********************************************

	エフェクト　２Ｄ　スプライト表示　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
