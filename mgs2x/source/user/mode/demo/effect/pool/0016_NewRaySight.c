/*********************************************

	エフェクト　全画面_レイ主観　のランチャー関数

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



void *NewRaySight_0016Launch( int id, void *argv )
{

	extern void *NewRaySight( ) ;



	return NewRaySight( ) ;
}


/*********************************************

	エフェクト　全画面_レイ主観　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
