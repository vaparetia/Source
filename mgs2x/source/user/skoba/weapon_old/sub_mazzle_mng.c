//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sub_mazzle.c
	主観用まずる
	2001/05/10 S.Kobayashi
	$Id: sub_mazzle_mng.c,v 1.1.1.3 2002/11/19 11:50:38 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"
#include	"../test/etc.h"


typedef	struct _work {
	GV_ACT_EX		actor ;

	FMATRIX *pWorld;
	FVECTOR shift;
	FVECTOR power;
	int deg;
	int mode;
	int life;
	int num;
	float size_rnd;
	float alpha;
	void (*act)( struct _work * );
} Work ;

enum {
	SK_MODE_NORMAL = 0 , 
	SK_MODE_RED , 
	SK_MODE_GREEN , 
};

// プロトタイプ 
extern void *NewSubMazzleFlushNormal( FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );
extern void *NewSubMazzleFlushRed( FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );
extern void *NewSubMazzleFlushGreen( FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );

///////////////////////////////////// coding
static void NormalAct( Work *pWork )
{
	GV_DestroyActor( pWork );
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
}

/*----------------------------------------------------------------*/
static int GetResources( Work *pWork )
{
	FVECTOR power;
	float size;
	float raute;
	int i;

	for ( i = 0 ; i < pWork->num ; i++ ){
		// 力 
		DG_COPY_VEC( &power , &pWork->power );
		raute = rnd();
		_sceVu0ScaleVector( &power , &power , raute ); 
		// サイズ 
		size = rnd() * pWork->size_rnd;
		switch ( pWork->mode ){
		case SK_MODE_NORMAL : 
		  	NewSubMazzleFlushNormal( pWork->pWorld , &pWork->shift , &power , pWork->life , size , pWork->alpha , pWork->deg );
			break;
		case SK_MODE_RED : 
			NewSubMazzleFlushRed( pWork->pWorld , &pWork->shift , &power , pWork->life , size , pWork->alpha , pWork->deg );
			break;
		case SK_MODE_GREEN : 
			NewSubMazzleFlushGreen( pWork->pWorld , &pWork->shift , &power , pWork->life , size , pWork->alpha , pWork->deg );
			break;
		}
	}

	return 0 ;
}

void *NewSubMazzleFlush_mng( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , int life , float size_rnd , float alpha , int deg , int mode )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act , Die ) ;
		GV_ActorEX( &pWork->actor );
		pWork->pWorld = world;
		DG_COPY_VEC( &pWork->shift , shift );
		DG_COPY_VEC( &pWork->power , pPower );
		pWork->life = life;
		pWork->size_rnd = size_rnd;
		pWork->alpha = alpha;
		pWork->deg = deg;
		pWork->mode = mode;
		pWork->num = num;
		pWork->act = ( void * )NormalAct;
		if ( GetResources( pWork ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewSubMazzleFlushMngNormal( int num , FMATRIX *pWorld , FVECTOR *shift , FVECTOR *pPower , int life , float size_rnd , float alpha , int deg )
{
	return ( NewSubMazzleFlush_mng( num , pWorld, shift, pPower , life , size_rnd , alpha , deg , SK_MODE_NORMAL ) );
}

void *NewSubMazzleFlushMngRed( int num , FMATRIX *pWorld , FVECTOR *shift , FVECTOR *pPower , int life , float size_rnd , float alpha , int deg )
{
	return ( NewSubMazzleFlush_mng( num , pWorld, shift, pPower , life , size_rnd , alpha , deg , SK_MODE_RED ) );
}

void *NewSubMazzleFlushMngGreen( int num , FMATRIX *pWorld , FVECTOR *shift , FVECTOR *pPower , int life , float size_rnd , float alpha , int deg )
{
	return ( NewSubMazzleFlush_mng( num , pWorld, shift, pPower , life , size_rnd , alpha , deg , SK_MODE_GREEN ) );
}
