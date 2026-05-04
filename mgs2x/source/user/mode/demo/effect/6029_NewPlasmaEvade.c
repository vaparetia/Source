//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 プラズマ回避エフェクト　のランチャー関数

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
	FMATRIX *pmatOrg ;
	FMATRIX *pmatRepel ;
	int     straight_node ;
	int     branch_node ;
	float   fOneDist ;
	float   fOneRad ;
	int     nBaseAng ;
	int     nAddAngMin ;
	int     nAddAngMax ;
	int     nRandSeed ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	FVECTOR d5 ;
	FVECTOR d6 ;
	float   d9 ;
	float   d10 ;
	int     d11 ;
	int     d12 ;
} Data ;

void *NewPlasmaEvade_6029Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPlasmaEvade( int ,FMATRIX *,FMATRIX *,int ,int ,float ,float ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	FMATRIX  mat2 ;
	FVECTOR * pos2 ;
	FVECTOR * rot2 ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pmatOrg = DM_GetObjectMatrix( handle, link->vy ) ;
	_sceVu0UnitMatrix( &mat2 ) ;
	pos2 = &data->d5 ;
	rot2 = &data->d6 ;
	_sceVu0RotMatrix( &mat2, &mat2, rot2 ) ;
	_sceVu0TransMatrix( &mat2, &mat2, pos2 ) ;
	work.pmatRepel = &mat2 ;
	work.straight_node = ( int ) 5.000000  ;
	work.branch_node = ( int ) 7.000000  ;
	work.fOneDist = data->d9 ;
	work.fOneRad = data->d10 ;
	work.nBaseAng = data->d11 ;
	work.nAddAngMin = ( int ) -164.000000  ;
	work.nAddAngMax = ( int ) 164.000000  ;
	work.nRandSeed = data->d12 ;

	return NewPlasmaEvade( work.name,
	                       work.pmatOrg,
	                       work.pmatRepel,
	                       work.straight_node,
	                       work.branch_node,
	                       work.fOneDist,
	                       work.fOneRad,
	                       work.nBaseAng,
	                       work.nAddAngMin,
	                       work.nAddAngMax,
	                       work.nRandSeed
) ;
}


/*********************************************

	エフェクト　効果 プラズマ回避エフェクト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
