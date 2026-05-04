//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	optcmflg.c
	光学迷彩エフェクト

	2000/03/24 K.Takabe
	$Id: optcmflg.c,v 1.1.1.3 2002/11/19 11:51:12 Yoshizawa1 Exp $

*/
/*

＜プログラム呼び出しインターフェイス＞
	void *NewOpticalCamouflage( DG_OBJS *objs, int flag );
	DG_OBJS	*objs ;		光学迷彩をかけるオブジェクト
	int		flag ;		フラグ

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "bp_matrix.h"
#include "BP_Debug.h"

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;
	FMATRIX		optcmf_param ;
	FMATRIX		optcmf_rotmat ;
	DG_OBJS		*objs ;
	int			flag ;
	int			old_flag ;
} Work ;


/* ---------------------------------------------------------------- */
static void SetOpticalCamouflageParam( Work *work )
{
	/* 光学迷彩パラメータの設定 */
	work->optcmf_param.m[0][0] = ( DRAW_WIDTH - 32 ) / 2.0f ;
	work->optcmf_param.m[0][1] = ( DRAW_HEIGHT - 16 ) / 2.0f ;
	work->optcmf_param.m[1][0] = DRAW_WIDTH / 2.0f ;
	work->optcmf_param.m[1][1] = DRAW_HEIGHT / 2.0f ;
	work->optcmf_param.m[2][0] = 16.0f ;
	work->optcmf_param.m[2][1] = 8.0f ;

	if ( work->flag & 1 ){
		/* 色付き光学迷彩 */
		work->optcmf_param.m[3][0] = 120.0f ;
		work->optcmf_param.m[3][1] = 150.0f ;
		work->optcmf_param.m[3][2] = 100.0f ;
		work->optcmf_param.m[3][3] = 96.0f ;
	} else {
		/* 無色光学迷彩 */
		work->optcmf_param.m[3][0] = 128.0f ;
		work->optcmf_param.m[3][1] = 128.0f ;
		work->optcmf_param.m[3][2] = 128.0f ;
		work->optcmf_param.m[3][3] = 96.0f ;
	}
#if 1 // BP_KP_Cheats
	if ( work->flag == 2 ){
      if( gBP_KP_Cheat_OptCmfMode == kBP_KP_Cheat_OptCmf_Green )
      {
         work->optcmf_param.m[3][0] = 33.0f ;
         work->optcmf_param.m[3][1] = 255.0f ;
         work->optcmf_param.m[3][2] = 200.0f ;
         work->optcmf_param.m[3][3] = 128.0f ;
      }
      else
      {
         work->optcmf_param.m[3][0] = 0.0f;
         work->optcmf_param.m[3][1] = 0.0f;
         work->optcmf_param.m[3][2] = 0.0f;
         work->optcmf_param.m[3][3] = 0.0f;
      }
	}
#endif
}

static float InterpValue( float a, float b, float t )
{
	return ( ( b - a ) * t + a );
}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
#if 1
   if( gBP_KP_Cheat_OptCmfMode == kBP_KP_Cheat_OptCmf_Normal )
   {
      switch ( work->flag ){
        case 0:
           GV_WaitMessage( work, 0 );
           break ;
        case 1:
           /* ステータス状態により色をアニメーションさせる */
           if ( !( GM_CheckPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH ) ) ) {
              /* 通常状態へ移行 */
              work->optcmf_param.m[3][0] = InterpValue( work->optcmf_param.m[3][0], 120.0f, 0.1f );
              work->optcmf_param.m[3][1] = InterpValue( work->optcmf_param.m[3][1], 150.0f, 0.1f );
              work->optcmf_param.m[3][2] = InterpValue( work->optcmf_param.m[3][2], 100.0f, 0.1f );
              work->optcmf_param.m[3][3] = InterpValue( work->optcmf_param.m[3][3], 96.0f, 0.1f );
           } else {
              /* 可視状態へ移行 */
              work->optcmf_param.m[3][0] = InterpValue( work->optcmf_param.m[3][0], 120.0f, 0.1f );
              work->optcmf_param.m[3][1] = InterpValue( work->optcmf_param.m[3][1], 150.0f, 0.1f );
              work->optcmf_param.m[3][2] = InterpValue( work->optcmf_param.m[3][2], 100.0f, 0.1f );
              work->optcmf_param.m[3][3] = InterpValue( work->optcmf_param.m[3][3], 250.0f, 0.1f );
           }
           break ;
        default:
           break ;
      }
   }
#else
	{
		static int	data[4] = {120,150,100,96};
		if ( GV_PadData[0].press & PAD_X ) printf("%s:color addr = %08x\n", __FILE__, data );
		work->optcmf_param.m[3][0] = data[0] ;
		work->optcmf_param.m[3][1] = data[1] ;
		work->optcmf_param.m[3][2] = data[2] ;
		work->optcmf_param.m[3][3] = data[3] ;
	}
#endif
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* 光学迷彩ＯＦＦ */
	//work->objs->flag &= ~DG_FLAG_OPTCMF ;
	work->objs->flag = work->old_flag ;
	if ( work->objs->extend_data == &work->optcmf_param ) work->objs->extend_data = NULL ;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, DG_OBJS *objs, int flag )
{
	work->flag = flag ;

	work->objs = objs ;
	work->old_flag = objs->flag ;
	/* 光学迷彩ＯＮ */
	objs->flag |= DG_FLAG_OPTCMF | DG_FLAG_DELAYED ;
	/* パラメータ設定マトリクス */
	objs->extend_data = &work->optcmf_param ;
	/* パラメータ設定 */
	SetOpticalCamouflageParam( work );
	_sceVu0UnitMatrix( &work->optcmf_rotmat );


	return (0);
}


/* ---------------------------------------------------------------- */
void *NewOpticalCamouflage( DG_OBJS *objs, int flag )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, objs, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
