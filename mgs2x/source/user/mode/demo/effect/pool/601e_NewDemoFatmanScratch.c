/*********************************************

	エフェクト　効果 ファットマン足傷　のランチャー関数

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
	FMATRIX *left ;
	FMATRIX *right ;
	int     name ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
	int     d6 ;
} Data ;

void *NewDemoFatmanScratch_601eLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoFatmanScratch( FMATRIX *,FMATRIX *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.left = DM_GetObjectMatrix( handle, link->vy ) ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.right = DM_GetObjectMatrix( handle, link->vy ) ;
	work.name = data->d6 ;

	return NewDemoFatmanScratch( work.left,
	                             work.right,
	                             work.name
) ;
}


/*********************************************

	エフェクト　効果 ファットマン足傷　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
