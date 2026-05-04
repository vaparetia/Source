/*********************************************

	エフェクト　パーティクル リンク　のランチャー関数

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
	void    *tmat ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
	IVECTOR d2 ;
	int     d5 ;
} Data ;

void *NewToolParticle_DemoLink_c000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewToolParticle_DemoLink( int ,FVECTOR *,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.particle_name = data->d0 ;
	work.pos = &data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.tmat = DM_GetObjectMatrix( handle, link->vy ) ;
	data->d5 ;

	return NewToolParticle_DemoLink( work.particle_name,
	                                 work.pos,
	                                 work.tmat
) ;
}


/*********************************************

	エフェクト　パーティクル リンク　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
