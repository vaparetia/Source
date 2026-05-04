//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　人体 ＮＹ市警　のランチャー関数

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
	void    *object ;
	int     frame ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewDemoArkms_NYPD_1011Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoArkms_NYPD( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.frame = data->d4 ;

	return NewDemoArkms_NYPD( work.name,
	                          work.object,
	                          work.frame
) ;
}


/*********************************************

	エフェクト　人体 ＮＹ市警　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
