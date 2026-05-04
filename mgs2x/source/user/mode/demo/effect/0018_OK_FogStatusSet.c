//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　フォグ 変更可＆不可　のランチャー関数

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
	int     command ;
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *OK_FogStatusSet_0018Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *OK_FogStatusSet( int  ) ;


	work.command = data->d0 ;

	return OK_FogStatusSet( work.command
) ;
}


/*********************************************

	エフェクト　フォグ 変更可＆不可　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
