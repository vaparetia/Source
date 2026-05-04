//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shiftmem.c
	ダミーメモリ確保キャラ（メモリ解析防止用）

	2001/09/07 K.Takabe
	$Id: shiftmem.c,v 1.1.1.3 2002/11/19 11:51:19 Yoshizawa1 Exp $

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


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
} Work ;


int	DummyMemOffset ;
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	GV_WaitMessage( work, 0 );
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}
/* ---------------------------------------------------------------- */
int NewAllocDummyMemory( void )
{
	Work		*work ;
	int		size ;

	OPERATOR() ;

	size = GV_Time & 0xff ;
	DummyMemOffset = size ^ 0x55 ;	/* 不具合再現のため暗号化して保存 */

	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) + size ) ;
#ifdef DEBUG_MODE
	printf("alloc mem : %08x, (%08x)", size, ( size + 15 ) & 0xf0 );
#endif
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
	}
	return ( 0 );
}
