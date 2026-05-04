//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面_VTR主観　のランチャー関数

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



void *NewVtrSight_0017Launch( int id, void *argv )
{

	extern void *NewVtrSight( ) ;



	return NewVtrSight( ) ;
}


/*********************************************

	エフェクト　全画面_VTR主観　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
