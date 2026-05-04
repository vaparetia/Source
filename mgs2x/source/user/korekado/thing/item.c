//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	item.c
	アイテム

	2000/06/16 Y.Korekado
	$Id: item.c,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
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
#include	"korekado/conv/define.h"
#include	"item.h"



/*----------------------------------------------*/
void KRTH_GetItemProc( ITEM_PROC	*item_proc, int name )
{
	int	num ;

	num = 0 ;

	while ( GCL_NextStr() != NULL ){
		item_proc->proc[ num ] = GCL_GetNextInt( ) ;
		item_proc->probability[ num ] = (u_char)GCL_GetNextInt( ) ;

		num++ ;
		if( num >= KR_MAX_ITEM ) break ;
	}
	item_proc->n_proc = num ;
	item_proc->c_proc = 0 ;
	item_proc->name = name ;
}

/* アイテム放出 */
int KRTH_PutDogTagItemProc( ITEM_PROC	*item_proc, FVECTOR *pos, int status, float n, int dogtag )
{
	GCL_ARGS arg ;
	int	argv[ GCL_MAX_ARGS ], c, pbb ;

printf(" item proc[%d] n_proc[%d]\n",item_proc->c_proc, item_proc->n_proc ) ;
	if( item_proc->c_proc >= item_proc->n_proc ) return 0 ;

	c = item_proc->c_proc ;
	pbb = (int)((float)item_proc->probability[ c ] * n) ;

printf("rand[%d] prob[%d]\n",(GV_Time%100), pbb ) ;
	if( (GV_Time%100) >= pbb ) return 0 ;

	argv[0] = GM_GetMapName( GM_CurrentMap ) ;
	argv[1] = (int)pos->vx ;
	argv[2] = (int)pos->vy ;
	argv[3] = (int)pos->vz ;
	argv[4] = 0 ;
	if ( status & 0x00000008 /*ENE_STATUS_BODYSPLASH*/ ) {
		argv[4] = 1 ;
	}
	argv[5] = dogtag ;
	argv[6] = (int)item_proc->str ;
	argv[7] = (int)item_proc->name ;

	arg.argc = 8 ;
	arg.argv = &argv[0] ;
printf("PUT ITEM [%d]!!\n", argv[0] ) ;
	if ( item_proc->proc[ c ] != 0 ) {
		GCL_ExecProc( item_proc->proc[ c ], &arg ) ;
	}
	item_proc->c_proc ++ ;
	
	return 1 ;
}

void KRTH_PutItemProc( ITEM_PROC	*item_proc, FVECTOR *pos, int status, float n )
{
	KRTH_PutDogTagItemProc( item_proc, pos, status, n, 0 ) ;
}
