//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　血_滴り　のランチャー関数

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
	FVECTOR *pos ;
	float   height ;
	int     size ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	FVECTOR d4 ;
	float   d5 ;
	int     d6 ;
} Data ;

void *NewBloodDropDemo_700aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBloodDropDemo( int ,FMATRIX *,FVECTOR *,float ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.pos = &data->d4 ;
	work.height = data->d5 ;
	work.size = data->d6 ;

	return NewBloodDropDemo( work.name,
	                         work.world,
	                         work.pos,
	                         work.height,
	                         work.size
) ;
}


/*********************************************

	エフェクト　血_滴り　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
