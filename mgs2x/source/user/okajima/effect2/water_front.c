//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	water_front.c
	濁流トップ：光学迷彩使用

	2001/01/23 S.Okajima
	$Id: water_front.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


//#define MODEL_NAME ( 123600 /*"usp"*/ )
//#define MODEL_NAME ( 12130941 /*"shock1"*/ )
//#define MODEL_NAME ( 12130942 /*"shock2"*/ )
//#define MODEL_NAME ( 2202283 /*"water_sphere01"*/ )
//#define MODEL_NAME ( 2202284 /*"water_sphere02"*/ )
//#define MODEL_NAME ( 1114067 /*"water_dome01"*/ )
//#define MODEL_NAME ( 16735093 /*"water_dome01x2"*/ )
//#define MODEL_NAME ( 1114068 /*"water_dome02"*/ )
#define MODEL_NAME ( 16736117 /*"water_dome02x2"*/ )


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	FVECTOR		*pos;
	SVECTOR		rot;
	OBJECT		object;

	FMATRIX		optcmf_param ;
	FMATRIX		optcmf_rotmat ;
	int			flag ;
	int			old_flag ;
} Work ;

/* ---------------------------------------------------------------- */
static void SetParam( Work *work )
{
	/* 光学迷彩パラメータの設定 */
	work->optcmf_param.m[0][0] = ( DRAW_WIDTH - 32 ) / 2.0f ;
	work->optcmf_param.m[0][1] = ( DRAW_HEIGHT - 16 ) / 2.0f ;
	work->optcmf_param.m[1][0] = DRAW_WIDTH / 2.0f ;
	work->optcmf_param.m[1][1] = DRAW_HEIGHT / 2.0f ;
	work->optcmf_param.m[2][0] = 16.0f ;
	work->optcmf_param.m[2][1] = 8.0f ;

	work->optcmf_param.m[3][0] = 128.0f ;
	work->optcmf_param.m[3][1] = 128.0f ;
	work->optcmf_param.m[3][2] = 128.0f ;
	work->optcmf_param.m[3][3] = 128.0f ;
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	OBJECT		*object;
	SVECTOR		*rot;

	DG_InvisibleObjs( GM_PlayerBody->objs ) ;

	object = &work->object;
	rot    = &work->rot;

	GM_GroupObjs( object->objs, GM_CurrentStageMap );
	rot->vy+= rot->pad;
	DG_SetPos2( work->pos, rot );
	GM_ActObject( object );

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	OBJECT		*object;

	object = &work->object;

	/* 光学迷彩ＯＦＦ */
	object->flag = work->old_flag ;
	if ( object->objs->extend_data == &work->optcmf_param ) object->objs->extend_data = NULL ;
	GM_FreeObject( object ) ;

}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work )
{
	OBJECT		*object;
	SVECTOR		*rot;

	object = &work->object;
	rot    = &work->rot;

	rot->vx = 0;
	rot->vy = 0;
	rot->vz = 0;
	rot->pad= 0;

	GM_InitObject( object, MODEL_NAME, DG_FLAG_SHADE ) ;
	work->old_flag = object->objs->flag ;
	DG_VisibleObjs( object->objs ) ;

	/* 光学迷彩ＯＮ */
	object->objs->flag |= (DG_FLAG_OPTCMF) ;

	/* パラメータ設定マトリクス */
	object->objs->extend_data = &work->optcmf_param ;

	work->rot.vy = (irnd()>>8)&4095;
	work->rot.pad= 32;

	/* パラメータ設定 */
	SetParam( work );
	_sceVu0UnitMatrix( &work->optcmf_rotmat );

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewWaterFront( FVECTOR *pos )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pos = pos;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
