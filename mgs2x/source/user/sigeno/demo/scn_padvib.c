//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scn_padvib.c
	command パッド振動
	2002/08/20 K.Sigeno
	$Id: scn_padvib.c,v 1.2 2002/11/25 01:48:49 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

//static	u_char	DamageVibHangL[] = { 255, 8, 0, 0 } ;


extern void	*NewPadVibration( char *script, int type ) ;

#define	MAX_SCRIPT	(32)

typedef	struct _Work {
	GV_ACT_EX	actor ;
	void		*child ;
	int			name ;
	int			cnt ;
	int			total_time ;
	int			mode ;
	char		script[MAX_SCRIPT] ;
} Work ;
static void Act(Work *work)
{
	if(work->total_time > work->cnt){
		work->cnt++ ;
	}else {
		if(GV_SearchActor( work->child ) == NULL ){
			/*振動が終わっていたら終了*/
			printf("NewScnPadVib END \n");
			GV_DestroyActor(work) ;
		}
	}
}
static void Die(Work *work)
{
}
static int GetResources(Work *work, int name, int where)
{
	int i = 0 ;

	work->name = name ;
	work->cnt = 0 ;
	work->total_time = 0 ;

	if ( GCL_GetOption( 'm' ) != NULL ) {
		work->mode = GCL_GetNextInt() ;
	}else {
		work->mode = 1 ;
	}
	if ( GCL_GetOption( 'd' ) != NULL ) {
		for(i=0;i<(MAX_SCRIPT - 2);i++){
			if( GCL_NextStr() != NULL ){
				work->script[ i ] = GCL_GetNextInt() ;
				if(i&1){
					/*TIME CNT*/
					work->total_time = work->script[ i ] ;
				}
			}else {
				break ;
			}
		}
	}
#ifdef DEBUG_MODE 
	if(i&1){
		printf("VIB SCRIPT ERROR!!!\n");
		ASSERT(0) ;
	}
#endif
	work->script[ i ] = 0 ;
	work->script[ i+1 ] = 0 ;

	GV_SetActorChild( work, work->child = (void *) NewPadVibration( work->script, work->mode )) ;

	return 1;
}

void *NewScnPadVib( name , where )
int	name ;
int	where ;
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources( work,name,where )){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

