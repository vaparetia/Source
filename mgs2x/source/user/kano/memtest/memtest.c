//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	memtest.c
		メモリチェックのためのダミーデータ埋め
	2000/05/12 K.Kano
	$Id: memtest.c,v 1.1.1.3 2002/11/19 11:43:19 Yoshizawa1 Exp $

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"


typedef	struct	{
	GV_ACT		actor ;
} Work;


static void Act(Work *work)
{
	int i;
	for(i=0x04000000;i<0x07fffff0;i++){
		unsigned char *a=(unsigned char *)i;
		*a=(unsigned char)(i*2+1);
	}
}

static void Die(Work *work)
{
}

/* 初期化部メイン */
void *NewMemTest(void)
{
    Work *work ;

    work=(Work *)GV_NewActorPrio(GV_ACTOR_DAEMON2,sizeof( Work ),GV_PRIO_MAX-1);

    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
    }
    return (void *)work ;
}
