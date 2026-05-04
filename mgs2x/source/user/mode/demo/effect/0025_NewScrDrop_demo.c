//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面_水滴ゆがみ　のランチャー関数

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
	int     life ;
} Work ;

void *NewScrDrop_demo_0025Launch( int id, void *argv )
{
	Work  work ;

	extern void *NewScrDrop_demo( int  ) ;


	work.life = ( int ) 0.000000  ;

	return NewScrDrop_demo( work.life
) ;
}


/*********************************************

	エフェクト　全画面_水滴ゆがみ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
