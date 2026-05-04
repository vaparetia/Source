/*********************************************

	エフェクト　火器 着弾簡易エフェクト　のランチャー関数

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
	int     arms_id ;
	FMATRIX *world ;
	int     conname ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	IVECTOR d2 ;
	int     d5 ;
} Data ;

void *NewDemoArmControl_101bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoArmControl( int ,int ,FMATRIX *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.arms_id = data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.conname = data->d5 ;

	return NewDemoArmControl( work.name,
	                          work.arms_id,
	                          work.world,
	                          work.conname
) ;
}


/*********************************************

	エフェクト　火器 着弾簡易エフェクト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
