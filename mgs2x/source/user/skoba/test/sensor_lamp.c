//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	冷却エフェクトbox  main.c
	2001/03/23 S.Kobayashi
	$Id: sensor_lamp.c,v 1.1.1.3 2002/11/19 11:50:25 Yoshizawa1 Exp $
*/
#include <sys/types.h>

#ifdef PSX2
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "etc.h"

// 小モデル用
#define	COMDL_FLAG	(DG_COMDL_SEMITRANS)
#define N_OBJS	(1000)
#define ICE_WAIT (256)

typedef	struct _work {
	GV_ACT_EX	actor;
	DG_COMDL	*comdl;

	FMATRIX     *pWorld;
	FVECTOR     pos;	
	SVECTOR     rot;
	FVECTOR     color;
	FVECTOR     sensor_pos;
	int         fade_count;
	int         bright_position;
	int         bright_raute;
	float       max;
	int		    map;
	int			count;
	int         num;
	int         time;
	char        flag;

	void ( *act )(struct _work *);
} Work;

enum {
	SK_NORMAL = 0 ,
	SK_DIE    = 0x1,
	SK_DIE_QUICK = 0x2,
	SK_PAKA_ON = 0x4,
	SK_FADE = 0x8,
	SK_SE_ON = 0x10 ,
};

enum {
	SK_SENSER_SE   = 0x01 , 
	SK_PARENT_CALL = 0x02 ,
	SK_SENSER_DIE  = 0x04 ,
	SK_ATTACH_DIE  = 0x08 , 
	SK_ALL_DIE     = 0x10 , 
	SK_SINGLE_DIE  = 0x20 , 
	SK_SENSER_SE_ON = 0x40 ,
};

extern  int DG_QueueComdlObjs( DG_COMDL * );
extern void DG_DequeueComdlObjs( DG_COMDL * );

#define PAKA_MAX (16)
static int bright_table[ PAKA_MAX ] = { 255 , 128 , 12 , 73 , 255 ,  128  , 21 , 189 , 64 , 78 ,
										255 ,  0  , 64 , 32 ,  0  , 255  };
static int bright_raute[ PAKA_MAX ] = {  4 ,   10 ,  2 , 10 ,   8 ,   12  };

static void PakapakaInitialize( Work *pWork )
{
	pWork->bright_position = 0;
	pWork->bright_raute = bright_raute[ pWork->bright_position ];

	pWork->flag |= SK_PAKA_ON;
}

static void PakaPaka( Work *pWork )
{
	DG_COMDL	*comdl;
	float       ftmp;

	if ( !( pWork->flag & SK_PAKA_ON ) ){
		return;
	}
	comdl = pWork->comdl;
	// 明るさ計算
	ftmp = bright_table[ pWork->bright_position ] / 4;
	if ( pWork->bright_raute == 0 ){
		comdl->pos[ 0 ].color.vw = ftmp;
	} else {
		comdl->pos[ 0 ].color.vw += ( ftmp - comdl->pos[ 0 ].color.vw ) / pWork->bright_raute;
		pWork->bright_raute--;
	}
	if ( comdl->pos[ 0 ].color.vw == bright_table[ pWork->bright_position ] / 4 ){
		pWork->bright_position++;
		if ( pWork->bright_position == PAKA_MAX ){
			pWork->flag |= SK_FADE;
			pWork->flag &= ~SK_PAKA_ON;
			if ( pWork->flag & SK_SE_ON ){ // ならしてもいいのなら、ならす。
				// 親に送信
				GV_CallParentSignalFunc( pWork , SK_SENSER_SE_ON , 0 );
			}
			return;
		}
		if ( pWork->bright_position == PAKA_MAX - 1 ){ // last
			pWork->bright_raute = DIRECT_TICK( 60 );
		} else {
			pWork->bright_raute = DIRECT_TICK( ( irnd() % 4 + 4 ) );
		}
	}
}

static void ComodelAct( Work *pWork )
{
	pWork->comdl->pos[ 0 ].world = *pWork->pWorld;
	pWork->comdl->pos[ 0 ].world.m[ 3 ][ 1 ] += 2400.0f;
	if ( pWork->flag & SK_DIE ){
		PakapakaInitialize( pWork );
		pWork->flag &= ~SK_DIE;
	}
#if 0
		 else {
			pWork->color.vx += ( pWork->max - pWork->color.vx ) / 6.0f;
			pWork->color.vy += ( pWork->max - pWork->color.vy ) / 6.0f;
			pWork->color.vz += ( pWork->max - pWork->color.vz ) / 6.0f;
			pWork->comdl->pos[ i ].color.vx = ( int )pWork->color.vx; 
			pWork->comdl->pos[ i ].color.vy = ( int )pWork->color.vy; 
			pWork->comdl->pos[ i ].color.vz = ( int )pWork->color.vz; 
			if ( pWork->color.vx < 1.0f ){
				pWork->max = 128.0f;
			} else if ( pWork->color.vx > 127.0f ){
				pWork->max = 0.0f;
			}
		}
#endif
	if ( pWork->flag & SK_FADE ){
		pWork->color.vx += ( 0.0f - pWork->color.vx ) / pWork->fade_count;
		pWork->color.vy += ( 0.0f - pWork->color.vy ) / pWork->fade_count;
		pWork->color.vz += ( 0.0f - pWork->color.vz ) / pWork->fade_count;
		if ( pWork->fade_count > 1 ){
			pWork->fade_count--;
		}
		pWork->comdl->pos[ 0 ].color.vx = ( int )pWork->color.vx; 
		pWork->comdl->pos[ 0 ].color.vy = ( int )pWork->color.vy; 
		pWork->comdl->pos[ 0 ].color.vz = ( int )pWork->color.vz; 
		if ( pWork->color.vx < 1.0f ){
			pWork->comdl->pos[ 0 ].color.vx = pWork->comdl->pos[ 0 ].color.vy = pWork->comdl->pos[ 0 ].color.vz = 0.0f;
		}
	}
	PakaPaka( pWork );
}

static int ReceiveSignal( void *workp, int signal, int value )
{
	Work *pWork = ( Work * )workp;
	
	switch ( signal ){
	case SK_SENSER_DIE :
		pWork->flag |= SK_DIE;
		break;
	case SK_SENSER_SE :
		pWork->flag |= SK_SE_ON;
		break;
#if 0
	case ?? : // 即死
		pWork->comdl->pos[ 0 ].color.vx = 0.0f;
		pWork->comdl->pos[ 0 ].color.vy = 0.0f;
		pWork->comdl->pos[ 0 ].color.vz = 0.0f;
		break;
#endif		
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

#define STR_COMODEL (3570248)
static int GetResources( Work *pWork , FMATRIX *pWorld )
{
	DG_COMDL	*comdl ;
	DG_DEF		*def ;
	int            i ;

	i = 0;

	pWork->map = GM_CurrentMap;
	pWork->num = 1;                 // 将来をかんがえて
	pWork->time = 0;
	pWork->flag = SK_NORMAL;
    GM_SetCurrentMap( pWork->map ); // 表示設定

	GV_SetActorSignalFunc( pWork , ReceiveSignal );
	def = (DG_DEF*)GV_GetCache( GV_CacheID( STR_COMODEL , 'k' ) );
	/* 通常モデルファイル(*.kms)中の任意のDG_OBJPACKを表示モデルとして初期化を行なう */
	comdl = pWork->comdl = DG_MakeComdl( def->models[0].packs, COMDL_FLAG, 1 , 0 );
	if( !comdl ){
		return ( -1 );
	}
	DG_QueueComdlObjs( comdl );

	GM_GroupObject( comdl , pWork->map );
	pWork->pWorld = pWorld;
	pWork->max = 128;
	comdl->pos[ i ].world = *pWork->pWorld;
	comdl->pos[ i ].world.m[ 3 ][ 1 ] += 2400.0f;
	/* 整数型なので注意！ */
	pWork->color.vx = comdl->pos[i].color.vx = 80;//pWork->max;
	pWork->color.vy = comdl->pos[i].color.vy = 160;//pWork->max;
	pWork->color.vz = comdl->pos[i].color.vz = 128;//pWork->max;
	pWork->color.vw = comdl->pos[i].color.vw = pWork->max;

	pWork->pos.vx = 0;
	pWork->pos.vy = 0;
	pWork->pos.vz = 0;
	pWork->pos.vw = 0;
	pWork->rot.vx = 0;
	pWork->rot.vy = 0;
	pWork->rot.vz = 0;

	pWork->fade_count = DIRECT_TICK( 100 );	

	pWork->act = (void *)ComodelAct;

	return ( 0 );
}

static void *NewLampComodel( FMATRIX *pWorld )
{
	Work		*pWork ;

	pWork = (Work *)GV_NewEffect( GV_ACTOR_EFFECT , sizeof( Work ) ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &( pWork->actor ), Act, Die ) ;
		GV_ActorEX( &pWork->actor );
		if ( GetResources( pWork, pWorld ) < 0 ) {
			GV_DestroyActor( pWork ) ;
			return NULL ;
		}
	}
	return (void *)pWork ;
}

void *NewSensorLampComodel( FMATRIX *pWorld )
{
    return NewLampComodel( pWorld );
}
