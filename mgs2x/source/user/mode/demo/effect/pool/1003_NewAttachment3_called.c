/*********************************************

	エフェクト　装備品 横揺れ　のランチャー関数

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
	int     model_name ;
	SVECTOR *rot ;
	void    *object ;
	int     objnum ;
	FVECTOR *pos ;
	int     frames ;
} Work ;

typedef struct data_t {
	int     d0 ;
	SVECTOR d1 ;
	IVECTOR d2 ;
	FVECTOR d5 ;
	int     d6 ;
} Data ;

void *NewAttachment3_called_1003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewAttachment3_called( int ,SVECTOR *,void *,int ,FVECTOR *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.model_name = data->d0 ;
	work.rot = &data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.objnum = link->vy ;
	work.pos = &data->d5 ;
	work.frames = data->d6 ;

	return NewAttachment3_called( work.model_name,
	                              work.rot,
	                              work.object,
	                              work.objnum,
	                              work.pos,
	                              work.frames
) ;
}


/*********************************************

	エフェクト　装備品 横揺れ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
