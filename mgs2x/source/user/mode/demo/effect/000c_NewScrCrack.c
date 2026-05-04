//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面　カメラ割れ　のランチャー関数

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
	int     mode ;
	int     camera_num ;
} Work ;

void *NewScrCrack_000cLaunch( int id, void *argv )
{
	Work  work ;

	extern void *NewScrCrack( int ,int  ) ;


	work.mode = ( int ) 0.000000  ;
	work.camera_num = ( int ) 0.000000  ;

	return NewScrCrack( work.mode,
	                    work.camera_num
) ;
}


/*********************************************

	エフェクト　全画面　カメラ割れ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
