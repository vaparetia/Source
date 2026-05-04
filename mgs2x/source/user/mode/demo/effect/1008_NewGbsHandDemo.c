//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　ゴルルコ兵 フィンガーサイン　のランチャー関数

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
	void    *human ;
	int     name ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
} Data ;

void *NewGbsHandDemo_1008Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewGbsHandDemo( void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.human = DM_GetObjectObject( handle ) ;
	work.name = data->d3 ;

	return NewGbsHandDemo( work.human,
	                       work.name
) ;
}


/*********************************************

	エフェクト　ゴルルコ兵 フィンガーサイン　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
