//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gcl.c
	GCL

	2000/06/30 Y.Korekado
	$Id: gcl.c,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $
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

#include	"libgv.h"
#include	"libmt.h"
#include	"libgcl.h"
#include	"libutl.h"
#include	"gameheader.h"
#include	"define.h"

/*--- GCL ----------------------*/
void	KR_ExecProc( int proc_id, GCL_ARGS *arg )
{
	if ( proc_id != 0 ) {
		GCL_ExecProc( proc_id, arg ) ;
	}
}

int	KR_GetNextFV( FVECTOR *fvec )
{
	int	vec[ 3 ] ;

	if ( GCL_NextStr() != NULL ){
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
		return 0 ;
	}
	
	return -1 ;
}

int	KR_GetNextFVS( FVECTOR *fvec )
{
	int	vec[ 3 ], num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
		
		fvec++ ;
		num++ ;
	}

	return num ;
}


int	KR_GCL_GetFVandM( FVECTOR *fvec, int *map )
{
	int	vec[ 3 ], num, mapname ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		GCL_GetNextIV( vec ) ;
		fvec->vx = (float)vec[0] ;
		fvec->vy = (float)vec[1] ;
		fvec->vz = (float)vec[2] ;
if ( GCL_NextStr() == NULL ) {
	extern void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
	int g, z ;
	HZX_Pos2Zone( fvec, &g, &z );
	*map = GM_GetBit( g ) ;
} else {
		mapname = GCL_GetNextInt( ) ;
		*map = GM_GetHzxGroupID( GM_GetMapID( mapname ) );
}
		fvec++ ;
		map++ ;
		num++ ;
	}

	return num ;
}

int KR_GclCallProcs( char *top )
{
	int i, id;
	char *p ;

	for( i = 0; GCL_NextStr() != NULL; i++ ){
		id = GCL_GetNextInt( ) ;
		p = GCL_NextStr() ;
		GCL_ExecProc( id, NULL );
		GCL_SetNextStr( p ) ;
	}
	return i;
}

int KR_GclGetInt( char *top, int *buff ) 
{
	int i ;
	
#if 1
	for( i = 0; GCL_NextStr() != NULL; i++ ){
		*buff = GCL_GetNextInt( ) ;
printf("buff=%d\n",*buff);	
		buff++ ;
	}
#else
	i = 0 ;
	while ( top != NULL ) {
		*buff = GCL_GetInt( top ) ;
		buff++ ;
		i++ ;
		top = GCL_NextStr() ;
	}
#endif
printf("iiiiiiiiiii=%d\n",i);	
	return i ;
}

int KR_GclGetProc( proc, argv )
int *proc ;
int	*argv ;
{
	int	argc, i ;

	*proc = 0 ;
	argc = 0 ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	*proc = GCL_GetNextInt( ) ;

	for( i = 0; GCL_NextStr() != NULL; i++, argc++ ){
		*argv++ = GCL_GetNextInt( ) ;
	}
	
	return argc ;
}
