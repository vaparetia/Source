//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　汎用ＣＶ２頂点アニメ　のランチャー関数

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
	void    *objs ;
	int     cv2_0 ;
	int     cv2_1 ;
	int     cv2_2 ;
	int     cv2_3 ;
	int     cv2_4 ;
	int     cv2_5 ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
	int     d9 ;
} Data ;

void *NewVADemo_4004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewVADemo( int ,void *,int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.cv2_0 = data->d4 ;
	work.cv2_1 = data->d5 ;
	work.cv2_2 = data->d6 ;
	work.cv2_3 = data->d7 ;
	work.cv2_4 = data->d8 ;
	work.cv2_5 = data->d9 ;

	return NewVADemo( work.name,
	                  work.objs,
	                  work.cv2_0,
	                  work.cv2_1,
	                  work.cv2_2,
	                  work.cv2_3,
	                  work.cv2_4,
	                  work.cv2_5
) ;
}


/*********************************************

	エフェクト　汎用ＣＶ２頂点アニメ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
