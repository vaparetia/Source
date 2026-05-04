//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 リンク汎用点滅　のランチャー関数

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
	int     tex_code ;
	FMATRIX *world ;
	FVECTOR *pos ;
	float   shift ;
	int     color ;
	int     w ;
	int     life ;
	int     mode ;
	int     act_time ;
	int     sleep_time ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
   int bp_pad[2];
	IVECTOR d2 ;
	FVECTOR d5 ;
	float   d6 ;
   int bp_pad1[3];
	IVECTOR d7 ;
	int     d8 ;
	int     d9 ;
	int     d10 ;
	int     d11 ;
	int     d12 ;
} Data ;

void *NewGeneralSprite2_6011Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewGeneralSprite2( int ,int ,FMATRIX *,FVECTOR *,float ,int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;
	int      temp0 ;
	int      temp1 ;

	work.name = data->d0 ;
	work.tex_code = data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.pos = &data->d5 ;
	work.shift = data->d6 ;
	col = &data->d7 ;
	work.color = ((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;
	work.w = data->d8 ;
	work.life = ( int ) 0.000000  ;
	temp0 = data->d9 ;
	temp1 = data->d10 ;
	work.mode = (((temp0 * ( int ) 256.000000 ) * ( int ) 256.000000 ) | temp1) ;
	work.act_time = data->d11 ;
	work.sleep_time = data->d12 ;

	return NewGeneralSprite2( work.name,
	                          work.tex_code,
	                          work.world,
	                          work.pos,
	                          work.shift,
	                          work.color,
	                          work.w,
	                          work.life,
	                          work.mode,
	                          work.act_time,
	                          work.sleep_time
) ;
}


/*********************************************

	エフェクト　効果 リンク汎用点滅　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
