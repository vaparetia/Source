//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	str_disp.c
	３Ｄ座標を透視変換し、そこに文字列表示

	2001/03/13 S.Okajima
	$Id: str_disp.c,v 1.1.1.3 2002/11/19 11:47:26 Yoshizawa1 Exp $
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
#include <string.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"
#include	"libgv.h"
#include	"libfs.h"
#include	"game.h"

#define	BUFF_SIZE	(64)

typedef	struct	{
	GV_ACT_EX	actor ;

	int		display_information_flag;
	int		display_information_count;

	FVECTOR	center;
	CVECTOR	col;
	char	strings[BUFF_SIZE];
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CVECTOR	col;

	col = work->col;
	switch( work->display_information_flag ){
	  case 1:
		col.cd = OK_MENU_COL_A * ( OK_DISPLAY_INFORMATION_TIME0 - work->display_information_count ) / OK_DISPLAY_INFORMATION_TIME0;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 2;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME1;
		}
		break;
	  case 2:
		col.cd = OK_MENU_COL_A;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			work->display_information_flag = 3;
			work->display_information_count = OK_DISPLAY_INFORMATION_TIME2;
		}
		break;
	  case 3:
		col.cd = OK_MENU_COL_A * work->display_information_count / OK_DISPLAY_INFORMATION_TIME2;
		work->display_information_count--;
		if( work->display_information_count <= 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
		break;
	}

	OK_Print3D_perse( &work->center, work->strings, col );
}

static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	work->display_information_flag = 1;
	return 0;
}

void *NewDispStringsWorld2Screen( FVECTOR *pos, char *strings, CVECTOR col )
{
	Work		*work ;

	OPERATOR() ;

	if( strlen( strings ) >= BUFF_SIZE ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, pos );
		work->col = col;
		strcpy( work->strings, strings );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}

