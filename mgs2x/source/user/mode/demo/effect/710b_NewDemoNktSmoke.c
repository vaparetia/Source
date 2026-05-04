//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙_ニキータ煙　のランチャー関数

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
	FMATRIX *world ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
} Data ;

void *NewDemoNktSmoke_710bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoNktSmoke( int ,FMATRIX * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;

	return NewDemoNktSmoke( work.name,
	                        work.world
) ;
}


/*********************************************

	エフェクト　煙_ニキータ煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
