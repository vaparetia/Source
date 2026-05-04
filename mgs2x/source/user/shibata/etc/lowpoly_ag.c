//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    clear.c
    クリアコード表示
	2000/11/10 T.Shibata

	$Id: lowpoly_ag.c,v 1.1.1.3 2002/11/19 11:48:43 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"camera.h"
#include 	"sprite_2d.h"
#include	"../util/ts_util.h"
#include	"rankenc.h"

#define		MAX_CODECHAR	(12)

typedef	struct {
	GV_ACT_EX		actor;
} Work;

#define		N_DEMO	(8)
#define		N_CHARA	(4)

#define		NO_CHARA	(0xffffffff)
static const int LowPoly_Flag[N_DEMO][N_CHARA] = {
	{
		0x00001fa5,
		0x00001faf,
		NO_CHARA,
		NO_CHARA,
	},
	{
		0x00001fa7,
		0x00001fa5,
		NO_CHARA,
		NO_CHARA,
	},
	{
		0x00001f87,
		0x000001a3,
		0x00001f87,
		NO_CHARA,
	},
	{
		0x00001faf,
		0x00001faf,
		0x00001faf,
		0x00001faf,
	},
	{
		0x00001fbf,
		0x00001fbf,
		0x000011a5,
		0x00001fbd,
	},
	{
		0x00001fbf,
		0x00001fbf,
		0x000011bf,
		0x000011a5,
	},
	{
		0x00001fff,
		NO_CHARA,
		NO_CHARA,
		NO_CHARA,
	},
	{
		0x00001fff,
		0x000011ff,
		NO_CHARA,
		NO_CHARA,
	},
};

static int GetResources( )
{
	int		proc_arg[5];
	int		flag;
	int		step;
	int		proc;
	int		i;

	proc = GCL_GetOptionValue( 'p', 0 );
	step = GCL_GetOptionValue( 's', 0 );
	proc_arg[0] = GCL_GetOptionValue( 'd', 0 );

	for( i = 0; i < 4; i++ ){
		proc_arg[i+1] = 0;
		flag = LowPoly_Flag[proc_arg[0]-1][i];
		if( flag != NO_CHARA ){
			if( flag & (1<<step) ){
				proc_arg[i+1] = step+1;
			}
		}
	}
	printf("------------------------------------------------\n");
	printf("demo %d: OBJ_num[%d][%d][%d][%d]\n", proc_arg[0], proc_arg[1], proc_arg[2], proc_arg[3], proc_arg[4] );
	printf("------------------------------------------------\n");
	if( proc ){
		GCL_ARGS	gcl_args;
		gcl_args.argc = 5;
		gcl_args.argv = proc_arg;
		GCL_ExecProc( proc, &gcl_args );
	}
	return 0;
}

void *NewLowPolyAgingCall( int name, int map )
{
	GetResources();
	return (void*)1;
}

