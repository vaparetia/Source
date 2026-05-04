//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ok_util.c
	おかじの便利集
	1999/09/01 S.Okajima
	$Id: ok_init.c,v 1.1.1.3 2002/11/19 11:47:41 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"./ok_util.h"
#include	"../effect/lit_man.h"

#define	MEM_LIST_MAX	(16)
#define	CHECK_BOUND_NEAR	(300.0f)
#define	CHECK_BOUND_FAR		(1500.0f)


FVECTOR OK_ListSplushWavePos[OK_SPLUSH_MOUNTAIN_MAX];
int OK_ListSplushWaveFlag[OK_SPLUSH_MOUNTAIN_MAX];
float	OK_slow_param;
int		OK_fog_unable_flag;
int		ok_flush_status;
int		OK_foot_splash_flag;
int		OK_IRS_OnFlag;
int		OK_RB_Num;
int		OK_EX0_Flag;

int			ok_lit_data_num;				/* 実データの個数 */
FVECTOR		*ok_lit_pos[ MAX_LIGHT_NUM ] ;
SVECTOR		*ok_lit_rot[ MAX_LIGHT_NUM ] ;
FVECTOR		*ok_lit_pos_sorted[ MAX_LIGHT_NUM ] ;	/* 前詰め */
SVECTOR		*ok_lit_rot_sorted[ MAX_LIGHT_NUM ] ;	/* 前詰め */

int OK_BodySplashFlag;
FVECTOR	G_wind    = { 0.0f, 0.0f, 0.0f, 0.0f };	/* 風速（単位フレームに移動する距離） */
SVECTOR	G_wind_rot= { 0, 0, 0, 0 };	/* 風向 */
int		G_wind_intense=0;	/* 強さ */
int		G_wind_intense_max=1;	/* 強さ */
int		G_wind_sw=0;							/* 突風発生中に立つ */
FMATRIX	G_wind_matrix;


/***************/
void OK_GetSplushWaveInit( void )
{
	int	i;
	int 	*pi;
	pi  = OK_ListSplushWaveFlag;
	for( i=0; i<OK_SPLUSH_MOUNTAIN_MAX; i++ ){
		(*pi) = 0;
		pi++;
	}
}
/***************/

FVECTOR OK_ItemGetBoundary = { CHECK_BOUND_NEAR, CHECK_BOUND_FAR, CHECK_BOUND_NEAR, 1.0f };
/***************/
void OK_EffectInitialize( void )
{
	int	i;

	// スローパラメータ
	OK_slow_param = 1.0f;

	// アイテム収得可能範囲
	OK_ItemGetBoundary.vx = CHECK_BOUND_NEAR;
	OK_ItemGetBoundary.vy = CHECK_BOUND_FAR;
	OK_ItemGetBoundary.vz = CHECK_BOUND_NEAR;
	OK_ItemGetBoundary.vw = 1.0f;

	// 山状水飛沫リスト初期化
	OK_GetSplushWaveInit();

	OK_fog_unable_flag  = 0;
	ok_flush_status     = 0;
	OK_foot_splash_flag = 0;
	OK_IRS_OnFlag       = 0;
	OK_RB_Num           = 0;
	OK_EX0_Flag         = 0;

	ok_lit_data_num=0;
	for( i=0; i<MAX_LIGHT_NUM; i++ ){
		ok_lit_pos[i]        = NULL;
		ok_lit_rot[i]        = NULL;
		ok_lit_pos_sorted[i] = NULL;
		ok_lit_rot_sorted[i] = NULL;
	}
	
	OK_BodySplashFlag =  0;

	// 風パラメータ
	G_wind    = DG_ZeroVector;	/* 風速（単位フレームに移動する距離） */
	G_wind_rot= DG_ZeroSVector;	/* 風向 */
	G_wind_intense = 0;	/* 強さ */
	G_wind_intense_max = 1;	/* 強さ */
	G_wind_sw = 0;							/* 突風発生中に立つ */
}
