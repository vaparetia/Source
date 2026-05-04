//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面_ＰＳＧ１サイト　のランチャー関数

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
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *NewPsg1Sight_001cLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPsg1Sight( int  ) ;


	work.name = data->d0 ;

	return NewPsg1Sight( work.name
) ;
}


/*********************************************

	エフェクト　全画面_ＰＳＧ１サイト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
