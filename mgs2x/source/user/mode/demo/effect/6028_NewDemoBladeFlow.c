//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 剣軌跡　のランチャー関数

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
	int     bld_mode ;
	int     n_disp ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewDemoBladeFlow_6028Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoBladeFlow( int ,FMATRIX *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.root = DM_GetObjectMatrix( handle, link->vy ) ;
	work.bld_mode = data->d4 ;
	work.n_disp = data->d5 ;

	return NewDemoBladeFlow( work.name,
	                         work.root,
	                         work.bld_mode,
	                         work.n_disp
) ;
}


/*********************************************

	エフェクト　効果 剣軌跡　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
