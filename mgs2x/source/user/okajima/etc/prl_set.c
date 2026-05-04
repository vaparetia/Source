//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	amb_set.c
	平行光源セット
	1999/08/17 S.Okajima
	$Id: prl_set.c,v 1.1.1.3 2002/11/19 11:47:41 Yoshizawa1 Exp $
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"

SVECTOR		okajima_prl_light_dir;

void *NewParallelSet( void )
{
	u_char	r;
	u_char	g;
	u_char	b;
	SVECTOR	dir;
	int		pos_x=0;
	int		pos_y=0;
	int		pos_z=0;
	FVECTOR	f_temp;
	int		dir_flag;
	int		pos_flag;

	dir_flag = 0;
	pos_flag = 0;

	if ( GCL_GetOption( 'c' ) != NULL ) {
		r = ( u_char )(GCL_GetNextInt() & 255);
		g = ( u_char )(GCL_GetNextInt() & 255);
		b = ( u_char )(GCL_GetNextInt() & 255);
		DG_SetMainLightCol( r, g, b );
	}

	/* 方向 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		dir_flag = 1;
		dir.vx = ( short )(GCL_GetNextInt() & 4095);
		dir.vy = ( short )(GCL_GetNextInt() & 4095);
		dir.vz = ( short )(GCL_GetNextInt() & 4095);
	}
	/* 座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos_flag = 1;
		pos_x = GCL_GetNextInt();
		pos_y = GCL_GetNextInt();
		pos_z = GCL_GetNextInt();
	}

	if( dir_flag  &&  pos_flag ){
		printf("ERR!!::Double data sets  -d  -p \n");
		printf("ＤオプションとＰオプションが同時にセットされてます\n");
		ASSERT(0);
	}

	if( dir_flag ){
		okajima_prl_light_dir=dir;

		f_temp.vx=0;
		f_temp.vy=0;
		f_temp.vz=4096;
		DG_SetPos2( &DG_ZeroVector, &dir );
		DG_PutVector( &f_temp, &f_temp, 1 );
		DG_SetMainLightDir( (int)f_temp.vx, (int)f_temp.vy, (int)f_temp.vz );
	}else if( pos_flag ){
		DG_SetMainLightDir( pos_x, pos_y, pos_z );
	}

	return NewParallelSet;
}
