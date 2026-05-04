//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　影 キャラ　のランチャー関数

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
	void    *objs ;
	void    *control ;
	FMATRIX *lights ;
	int     *flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewDropShadow_1000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDropShadow( void *,void *,FMATRIX *,int * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectObject( handle ) ;
	work.control = DM_GetObjectControl( handle ) ;
	work.lights = DM_GetObjectLightMtx( handle ) ;
	work.flag = DM_NULL(  ) ;

	return NewDropShadow( work.objs,
	                      work.control,
	                      work.lights,
	                      work.flag
) ;
}


/*********************************************

	エフェクト　影 キャラ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
