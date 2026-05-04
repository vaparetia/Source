/*********************************************

	エフェクト　外部出力 振動再生　のランチャー関数

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
	int     vib_file ;
	int     frame ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewPadVibration2_9000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPadVibration2( int ,int  ) ;

	int      mode ;
	int      max ;
	int      min ;

	work.vib_file = data->d0 ;
	mode = data->d1 ;
	max = data->d2 ;
	min = data->d3 ;
	work.frame = ((max - min) * mode) ;

	return NewPadVibration2( work.vib_file,
	                         work.frame
) ;
}


/*********************************************

	エフェクト　外部出力 振動再生　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
