//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	padproc.c
	デバッグ用 パッドでproc実行 *NewSigPadProc
	2000/04/14 K.Sigeno
	$Id: padproc.c,v 1.1.1.3 2002/11/19 11:49:07 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"

#define	MAX_PROC	(16)
/* ワーク */
typedef	struct _Work {
	GV_ACT		actor ;
	int			proc_id[MAX_PROC];
	int			proc_trig[MAX_PROC];
	int			pad_num;
	int			checkpad ; /*チェック必要なキーリスト*/
} Work ;

static void Act(Work *work)
{
	int proc_num ;

	if(GV_PadData[ work->pad_num ].press & work->checkpad ){
		for(proc_num=0;proc_num<MAX_PROC;proc_num++){
			if(GV_PadData[ work->pad_num ].press 
			& work->proc_trig[proc_num]){
				if( work->proc_id[proc_num] !=-1 ){
					GM_ExecProc( work->proc_id[proc_num], NULL );
				}
			}
		}
	}
}
static void Die(Work *work)
{
}
static int GetResources(Work *work, int name, int where)
{
    int i;
//	int			proc_id[MAX_PROC];
//	int			proc_trig[MAX_PROC];
//	int			pad_num[MAX_PROC];

	work->checkpad = 0 ;

	/*シナリオリード部*/
	if ( GCL_GetOption( 'p' ) != NULL ){
		for(i=0;i<MAX_PROC;i++){
			if( GCL_NextStr() != NULL ){
				work->proc_id[i] = GCL_GetNextInt();
			}else {
				work->proc_id[i] = -1;
			}
		}
	}else {
		return 0;
	}
	if ( GCL_GetOption( 'c' ) != NULL ){
		if( GCL_NextStr() != NULL ){
				work->pad_num = GCL_GetNextInt();
		}
	}
	if ( GCL_GetOption( 'k' ) != NULL ){
		for(i=0;i<MAX_PROC;i++){
			if( GCL_NextStr() != NULL ){
				work->proc_trig[i] = GCL_GetNextInt();
				work->checkpad |= work->proc_trig[i];
			}
		}
	}
	return 1;
}

/* 初期化部メイン */
void *NewSigPadProc( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
