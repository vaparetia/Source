//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　血 ソリダス斬られ血　のランチャー関数

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
	FMATRIX *root ;
	FMATRIX *node1 ;
	FMATRIX *node2 ;
	FVECTOR *vec ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	IVECTOR d4 ;
	IVECTOR d7 ;
	FVECTOR d10 ;
} Data ;

void *NewDemoSplashBlood_700dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSplashBlood( int ,FMATRIX *,FMATRIX *,FMATRIX *,FVECTOR * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.root = DM_GetObjectMatrix( handle, link->vy ) ;
	link = &data->d4 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.node1 = DM_GetObjectMatrix( handle, link->vy ) ;
	link = &data->d7 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.node2 = DM_GetObjectMatrix( handle, link->vy ) ;
	work.vec = &data->d10 ;

	return NewDemoSplashBlood( work.name,
	                           work.root,
	                           work.node1,
	                           work.node2,
	                           work.vec
) ;
}


/*********************************************

	エフェクト　血 ソリダス斬られ血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
