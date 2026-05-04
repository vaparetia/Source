//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　パーティクル フィックス　のランチャー関数

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
	int     particle_name ;
	FVECTOR *pos ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
	int     d2 ;
} Data ;

void *NewToolParticle_DemoFix_c001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewToolParticle_DemoFix( int ,FVECTOR * ) ;


	work.particle_name = data->d0 ;
	work.pos = &data->d1 ;
	data->d2 ;

	return NewToolParticle_DemoFix( work.particle_name,
	                                work.pos
) ;
}


/*********************************************

	エフェクト　パーティクル フィックス　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
