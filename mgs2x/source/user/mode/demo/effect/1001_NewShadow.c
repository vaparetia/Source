//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　影 足元　のランチャー関数

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
	void    *l_foot ;
	void    *r_foot ;
	void    *control ;
	FMATRIX *lights ;
	int     *flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
} Data ;

void *NewShadow_1001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewShadow( void *,void *,void *,FMATRIX *,int * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.l_foot = DM_GetObjectDgObj( handle, link->vy ) ;
	link = &data->d3 ;
	work.r_foot = DM_GetObjectDgObj( handle, link->vy ) ;
	work.control = DM_GetObjectControl( handle ) ;
	work.lights = DM_GetObjectLightMtx( handle ) ;
	work.flag = DM_NULL(  ) ;

	return NewShadow( work.l_foot,
	                  work.r_foot,
	                  work.control,
	                  work.lights,
	                  work.flag
) ;
}


/*********************************************

	エフェクト　影 足元　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
