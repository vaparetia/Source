//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	amb_set.c
	フォグセット
	1999/08/17 S.Okajima
	$Id: fog_set.c,v 1.1.1.3 2002/11/19 11:47:40 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include	"libmt.h"

extern void *NewFogSet_Demo( int col_r, int col_g, int col_b, float fNear, float fFar, int time );

// 参照用フォグ値：最終変化目標の値で、シナリオ指定
CVECTOR		OK_FogColorBase = {0,0,0,0};
float		OK_FogNear = 20*65536.0f;
float		OK_FogFar  = 21*65536.0f;


void *NewFogSet( void )
{
	int		col_r;
	int		col_g;
	int		col_b;
	int		time;
	float	fNear;
	float	fFar;


	col_r = OK_FogColorBase.r;
	col_g = OK_FogColorBase.g;
	col_b = OK_FogColorBase.b;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		col_r = GCL_GetNextInt() & 255;
		col_g = GCL_GetNextInt() & 255;
		col_b = GCL_GetNextInt() & 255;
	}
	OK_FogColorBase.r = col_r;
	OK_FogColorBase.g = col_g;
	OK_FogColorBase.b = col_b;


	/* ＮＥＡＲ値 */
	fNear  = OK_FogNear;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		fNear = ( float )(GCL_GetNextInt());
	}
	OK_FogNear = fNear;

	/* ＦＡＲ値 */
	fFar = OK_FogFar;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		fFar = ( float )(GCL_GetNextInt());
	}
	OK_FogFar = fFar;

	/* ＦＡＲクリップ値 */
	if ( GCL_GetOption( 'z' ) != NULL ) {
		DG_SetClipParam( DRAW_NEAR_CLIP, fFar + 10000.0f );
	}

	time = 0;
	/* 変化時間 */
	if ( GCL_GetOption( 't' ) != NULL ) {
		time = GCL_GetNextInt();
	}
	return NewFogSet_Demo( col_r, col_g, col_b, fNear, fFar, time );
}
