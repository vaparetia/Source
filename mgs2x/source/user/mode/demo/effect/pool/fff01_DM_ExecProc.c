/*********************************************

	エフェクト　システム プロック呼出し　のランチャー関数

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
	int     proc_id ;
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *DM_ExecProc_fff01Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_ExecProc( int  ) ;


	work.proc_id = data->d0 ;

	return DM_ExecProc( work.proc_id
) ;
}


/*********************************************

	エフェクト　システム プロック呼出し　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
