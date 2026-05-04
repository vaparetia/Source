//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enetrapchk.c
	指定されたトラップに巡回中敵が入ったらPROC
	2002/06/07 K.Sigeno
	$Id: enetrapchk.c,v 1.1.1.3 2002/11/19 11:49:03 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif


#include	"gameheader.h"
#include	"libutl.h"
#include	"korekado/enemy/enemy.h"

typedef	struct	{
	GV_ACT_EX	actor ;
	COMMANDER	*com ;
	int		proc_id ;
	int		trap_name ;
	int		status ;
} Work ;
//COMMANDER	*COM_GetCommander()

/*任意のtrapに特定のステータスの敵兵がいるか？*/
static int COM_EnemyTrapCheckStatus( COMMANDER	*com ,int trap_name_id , int status )
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( !(entk->thk_status & status) )  continue ;
				if(HZX_CheckInsideTrap( entk->ctrl->hzx_id, &entk->ctrl->mov, trap_name_id )){
					return 1 ;
				}
			}
		}
	}
	return 0 ;
}

static	void	Act( work )
Work		*work ;
{
	if(COM_EnemyTrapCheckStatus( work->com ,work->trap_name , work->status )){
		GM_ExecProc( work->proc_id, NULL );
		GV_DestroyActor( work ) ;
	}
}
static	void	Die( work )
Work		*work ;
{
}
/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ,int name, int where ){
	work->com = COM_GetCommander();
	if ( GCL_GetOption( 'p' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->proc_id  = GCL_GetNextInt();
		}
	}
	if ( GCL_GetOption( 't' ) != NULL ){
		if( GCL_NextStr() != NULL ){
			work->trap_name = GCL_GetNextInt();
		}
	}
	work->status = THK_STATUS_ROOT_ACTION ;
	return 0 ;
}
void	*NewCheckEneTrap( int name, int where )
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work,name,where) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
