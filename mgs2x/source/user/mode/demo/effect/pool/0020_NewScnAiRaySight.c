/*********************************************

	エフェクト　全画面_ＡＩレイ主観　のランチャー関数

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
	int     con_name ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewScnAiRaySight_0020Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewScnAiRaySight( int ,int  ) ;


	work.name = data->d0 ;
	work.con_name = data->d1 ;

	return NewScnAiRaySight( work.name,
	                         work.con_name
) ;
}


/*********************************************

	エフェクト　全画面_ＡＩレイ主観　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
