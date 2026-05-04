//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_thk3.c
	量産型ＲＡＹ思考処理ルーチン

	2001/05/06 K.Takabe
	$Id: pdr_thk3.c,v 1.1.1.3 2002/11/19 11:51:26 Yoshizawa1 Exp $

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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

#include "pdray.h"
#include "r_common.h"

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */
#if 0
static void Think3_Null( Work *work );
static void Think3_None( Work *work );
static void (*think3_func_list[])( Work *work ) = {
	Think3_Null,		/* THINK3_NULL */
	Think3_None,		/* THINK3_NONE */
};
#endif
/* ---------------------------------------------------------------- */
/* 次の移動アクションを設定 */
int PDRAY_SetNextThink3( Work *work, int next_think3_mode )
{
	//if ( work->think3_mode != work->next_think3_mode ) return ( -1 ) ;
	//work->next_think3_mode = next_think3_mode ;
	return ( 0 );
}
void PDRAY_InitThink3( Work *work )
{
	//work->think3_mode = -1 ;
	//work->next_think3_mode = -1 ;
	//PDRAY_SetNextThink3( work, THINK3_NONE );
}
/* ---------------------------------------------------------------- */

