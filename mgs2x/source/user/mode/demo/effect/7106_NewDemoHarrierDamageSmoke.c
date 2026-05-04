//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙_ハリアーダメージ煙　のランチャー関数

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
	FVECTOR *shift ;
	float   radius ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	FVECTOR d4 ;
	float   d5 ;
} Data ;

void *NewDemoHarrierDamageSmoke_7106Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoHarrierDamageSmoke( int ,FMATRIX *,FVECTOR *,float  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.shift = &data->d4 ;
	work.radius = data->d5 ;

	return NewDemoHarrierDamageSmoke( work.name,
	                                  work.world,
	                                  work.shift,
	                                  work.radius
) ;
}


/*********************************************

	エフェクト　煙_ハリアーダメージ煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
