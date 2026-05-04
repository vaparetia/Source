/*********************************************

	エフェクト　血 カメラ血（常駐）　のランチャー関数

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



void *NewBloodCamera_7005Launch( int id, void *argv )
{

	extern void *NewBloodCamera( ) ;



	return NewBloodCamera( ) ;
}


/*********************************************

	エフェクト　血 カメラ血（常駐）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
