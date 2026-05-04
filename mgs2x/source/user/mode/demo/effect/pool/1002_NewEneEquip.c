/*********************************************

	エフェクト　装備品 ゴルルコビッチ兵　のランチャー関数

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
	void    *parent ;
	void    *object ;
	int     flag ;
	int     wpname ;
	int     left_wp ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d2 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewEneEquip_1002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEneEquip( int ,void *,void *,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.parent = DM_NULL(  ) ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.flag = data->d5 ;
	work.wpname = data->d6 ;
	work.left_wp = data->d7 ;

	return NewEneEquip( work.name,
	                    work.parent,
	                    work.object,
	                    work.flag,
	                    work.wpname,
	                    work.left_wp
) ;
}


/*********************************************

	エフェクト　装備品 ゴルルコビッチ兵　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
