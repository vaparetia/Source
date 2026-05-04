//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   c4manage.c
   特殊Ｃ４管理
   
   2002/04/10 Y.Korekado
   $Id: c4manage.c,v 1.1.1.3 2002/11/19 11:44:05 Yoshizawa1 Exp $
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

#include	"gameheader.h"

/*------------------------------------------------------------*/
#define MAX_C4	(32)

/*
g_define.hに移動
enum {
	C4MAN_STATUS_NONE,
	C4MAN_STATUS_BOMB,
	C4MAN_STATUS_NO_KAITAI,
	C4MAN_STATUS_DROP,
} ;
*/
/*------------------------------------------------------------*/
typedef struct _C4MAN {
	int name ;
	int status ;
	FMATRIX	*world ;
	DG_OBJS *objs ;
	FVECTOR	shift ;
	SVECTOR	rot ;
	int *map ;
} C4MAN ;

static C4MAN	C4Man[MAX_C4] ;
/*------------------------------------------------------------*/

/*------------------------------------------------------------*/
int	C4MAN_Init()
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		C4Man[i].name = 0 ;
		C4Man[i].status = C4MAN_STATUS_NONE ;
		C4Man[i].world = NULL ;
		C4Man[i].objs = NULL ;
		C4Man[i].shift = DG_ZeroVector ;
		C4Man[i].rot = DG_ZeroSVector ;
	}
	
	return 0 ;
}

int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( !C4Man[i].name ) {
			C4Man[i].name = name ;
			C4Man[i].map = map ;
			C4Man[i].world = world ;
			C4Man[i].objs = objs ;
			C4Man[i].shift = *shift ;
			C4Man[i].rot = *rot ;
			return 1 ;
		}
	}
	
	return 0 ;
}

FMATRIX	*C4MAN_GetWorld( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name == name ) {
			return C4Man[i].world  ;
		}
	}
	
	return NULL ;
}
int	*C4MAN_GetMap( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name == name ) {
			return C4Man[i].map  ;
		}
	}
	
	return NULL ;
}

DG_OBJS	*C4MAN_GetObjs( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name == name ) {
			return C4Man[i].objs  ;
		}
	}
	
	return NULL ;
}

void	C4MAN_GetParam( int name, FVECTOR *shift, SVECTOR *rot )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name == name ) {
			*shift = C4Man[i].shift ;
			*rot = C4Man[i].rot ;
		}
	}
}

void	C4MAN_Bomb( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name ) {
			C4Man[i].status = C4MAN_STATUS_BOMB ;
		}
	}
}

void	C4MAN_Drop( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name ) {
			C4Man[i].status = C4MAN_STATUS_DROP ;
		}
	}
}

int		C4MAN_GetStatus( int name )
{
	int i ;

	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name ) {
			return C4Man[i].status ;
		}
	}
	return -1 ;
}

void	C4MAN_SetStatus( int name, int status )
{
	int i ;

	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name ) {
			C4Man[i].status = status ;
		}
	}
}

int	SCN_C4MAN_GetStatus( int name )
{
	int i ;
	
	for ( i=0; i<MAX_C4; i++ ) {
		if ( C4Man[i].name ) {
			return  C4Man[i].status ;
		}
	}
	
	return 0 ;
}


