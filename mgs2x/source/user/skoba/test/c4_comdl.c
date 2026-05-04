//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	冷却エフェクトbox  main.c
	2001/03/23 S.Kobayashi
	$Id: c4_comdl.c,v 1.1.1.3 2002/11/19 11:50:24 Yoshizawa1 Exp $
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

#include "libutl.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include    "etc.h"

// 小モデル用
#define	COMDL_FLAG	(DG_COMDL_SEMITRANS)
#define N_OBJS	(1000)
#define ICE_WAIT (256)
#define FLUSH_COLD (0x10)

typedef	struct _work {
	GV_ACT_EX	actor;
	DG_COMDL	*comdl;

	FMATRIX   *pWorld;
	FVECTOR       pos;	
	SVECTOR       rot;
	float       color;
	int		      map;
	int			count;
	int           num;
	int          time;
	void ( *act )(struct _work *);
} Work;

extern  int DG_QueueComdlObjs( DG_COMDL * );
extern void DG_DequeueComdlObjs( DG_COMDL * );

static void ComodelAct( Work *pWork )
{
	int i;

	for ( i = 0 ; i < pWork->num ; i++ ){
		pWork->comdl->pos[ i ].world = *pWork->pWorld;
	}
}

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	float tmp;
	
	switch ( signal ){
	case 0x2 :
		if ( value < DIRECT_TICK( 1500 ) ){
			tmp = ( 120.f - pWork->color ) / DIRECT_TICK( 150.f );
		} else {
			tmp = ( 255.f - pWork->color ) / DIRECT_TICK( 30.f );
		} 
		pWork->color += tmp;
		pWork->comdl->pos[0].color.vw = ( int )pWork->color;
		pWork->comdl->pos[0].color.vw &= 0xff;
		// サウンド
		if ( !(pWork->time % 16) ){
			GM_SeSetMode( SD_A_FREEZE01, (FVECTOR *)pWork->pWorld->m[ 3 ] , GM_SEMODE_BOMB ) ;
			pWork->time = 1;
		} else {
			pWork->time++;
		}
		break;
	case 0x4 : // 完全凍結
		pWork->color = 255;
		break;
	case FLUSH_COLD : // 瞬殺
		pWork->comdl->pos[ 0 ].color.vw = ( int )0xff;
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( 1 );
	}
	return ( 0 );
}

static void Act( Work *pWork )
{
	pWork->act( pWork );
}

static void Die( Work *pWork )
{
	DG_DequeueComdlObjs( pWork->comdl );
	DG_FreeComdl( pWork->comdl );
}

static int GetResources( Work *pWork , FMATRIX *pWorld , int str_code )
{
	DG_COMDL	*comdl ;
	DG_DEF		*def ;
	int            i ;

	i = 0;

	pWork->map = GM_CurrentMap;
	pWork->num = 1;                 // 将来をかんがえて
	pWork->time = 0;
    GM_SetCurrentMap( pWork->map ); // 表示設定

	GV_SetActorSignalFunc( pWork , ReceiveSignal );
	def = (DG_DEF*)GV_GetCache( GV_CacheID( str_code , 'k' ) );
	/* 通常モデルファイル(*.kms)中の任意のDG_OBJPACKを表示モデルとして初期化を行なう */
	comdl = pWork->comdl = DG_MakeComdl( def->models[0].packs, COMDL_FLAG, 1 , 0 );
	if( !comdl ){
		return ( -1 );
	}
	DG_QueueComdlObjs( comdl );

	GM_GroupObject( comdl , pWork->map );
	pWork->pWorld = pWorld;
	comdl->pos[ i ].world = *pWork->pWorld;

	/* 整数型なので注意！ */
	comdl->pos[i].color.vx = 128;
	comdl->pos[i].color.vy = 128;
	comdl->pos[i].color.vz = 128;
	comdl->pos[i].color.vw = 0;

	pWork->pos.vx = 0;
	pWork->pos.vy = 0;
	pWork->pos.vz = 0;
	pWork->pos.vw = 0;
	pWork->rot.vx = 0;
	pWork->rot.vy = 0;
	pWork->rot.vz = 0;

	pWork->color = 0.f;
	pWork->act = (void *)ComodelAct;

	return ( 0 );
}

void *NewComodel( FMATRIX *pWorld , int str_code )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		if ( GetResources( pWork, pWorld , str_code ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewIceComodel( FMATRIX *pWorld , int str_code )
{
    return NewComodel( pWorld , str_code );
}
