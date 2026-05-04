/*********************************************

	エフェクト　装備品 フォーチュン　のランチャー関数

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
	void    *body ;
	int     flag ;
	int     wpname ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewFortEquip_1014Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFortEquip( int ,void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.flag = data->d4 ;
	work.wpname = data->d5 ;

	return NewFortEquip( work.name,
	                     work.body,
	                     work.flag,
	                     work.wpname
) ;
}


/*********************************************

	エフェクト　装備品 フォーチュン　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
