//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　タバコ　のランチャー関数

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
	FMATRIX *sna_head ;
	FMATRIX *sna_rhand ;
	float   force ;
	FVECTOR *dir ;
} Work ;

typedef struct _000e_NewCigarette_Demo_data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	IVECTOR d4 ;
	IVECTOR d7 ;
	float   d10 ;
   int bp_pad1[3];
	FVECTOR d11 ;
} Data ;

void *NewCigarette_Demo_000eLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCigarette_Demo( int ,FMATRIX *,FMATRIX *,FMATRIX *,float ,FVECTOR * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	link = &data->d4 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.sna_head = DM_GetObjectMatrix( handle, link->vy ) ;
	link = &data->d7 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.sna_rhand = DM_GetObjectMatrix( handle, link->vy ) ;
	work.force = data->d10 ;
	work.dir = &data->d11 ;

	return NewCigarette_Demo( work.name,
	                          work.world,
	                          work.sna_head,
	                          work.sna_rhand,
	                          work.force,
	                          work.dir
) ;
}


/*********************************************

	エフェクト　タバコ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
