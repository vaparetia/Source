//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wccomm.c
	警備兵統括プログラム
	
	1999/08/24 Y.Korekado
	$Id: wccomm.c,v 1.1.1.3 2002/11/19 11:44:31 Yoshizawa1 Exp $

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
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

/*--- ------------------------------------------------------------*/
#define	MAX_GRP_ZONE	(4)	/* チーム内で同時に分かれるグループ */

static int CheckZone[]={0x01313, 0x01212, 0, 0x00303,-1} ;
//static int CheckZone[]={0x0e4e4, 0x0b0b0, 0x0d1d1, 0x06464, 0xd3d3, 0x0b1b1, 0x0b6b6, -1} ;

typedef	struct	{
	GV_ACT		actor ;

	E_GROUP		group ;
	E_UNIT		unit[ MAX_UNIT ] ;
	COMMANDER	*com ;

	int			c_zone ;	/* */
} Work ;

static Work *wccom ;

/*--- ------------------------------------------------------------*/
void WCOMM_ClearCheckZone( void )
{
	Work	*work = wccom ;

	if ( CheckZone[ ++work->c_zone ] < 0 ) work->c_zone = 0 ;
//	if ( KR_RandU(2) ) return ;
//	if ( CheckZone[ ++work->c_zone ] < 0 ) work->c_zone = 0 ;
}

int WCOMM_GetCurrentCheckZone( void )
{
	Work	*work = wccom ;
	return CheckZone[ work->c_zone ] ;
}

int	WCOMM_SearchCheckZone( ENETHINK	*my, int c_zone )
{
	Work	*work = wccom ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int i, j ;
	
	for ( j=0; j<work->group.unit_num; j++ ) {
		un = work->group.unit[ j ] ;
		for ( i=0; i<un->enemy_num; i++ ) {
			if ( (entk = un->entk[ i ]) == NULL ) continue ;
			if ( entk == my ) continue ;
			if ( !(entk->thk_status & THK_STATUS_CHECKZONE) ) continue ;
			/* 既にチェック中かどうか */
			if ( entk->checkzone == c_zone ) return 1 ;
		}
	}
	return 0 ;
}

ENETHINK	*WCOMM_GetFrontFriend( ENETHINK *my, HZX_ZONE_ADD to )
{
	Work	*work = wccom ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int i, j, fr_rout, my_rout ;

	my_rout = HZX_GetRouteCrossGroup( my->ctrl->addr, to) ;
	for ( j=0; j<work->group.unit_num; j++ ) {
		un = work->group.unit[ j ] ;
		for ( i=0; i<un->enemy_num; i++ ) {
			if ( (entk = un->entk[ i ]) == NULL ) continue ;
			if ( entk == my ) continue ;

			fr_rout = HZX_GetRouteCrossGroup( entk->ctrl->addr, to) ;
//printf( "friend [%d] my [%d] \n",fr_rout, my_rout ) ;
			if ( fr_rout < my_rout ) return entk ;
			/* 仲間と同じゾーンにいたら */
			if ( my->ctrl->addr == entk->ctrl->addr ) {
				int	next_zone, mydis, frdis ;
				FVECTOR	pos ;

				next_zone = HZX_NextZoneCrossGroup2( my->ctrl->addr, to ) ;
				ENE_Zoneadd2Pos( &pos, next_zone ) ;
				mydis = _FVecTrgDis( &my->ctrl->mov, &pos ) ;
				frdis = _FVecTrgDis( &entk->ctrl->mov, &pos ) ;
				if ( frdis < mydis ) return entk ;
			}
		}
	}
	return NULL ;
}

/*--- ------------------------------------------------------------*/
static void Act( Work *work )
{
}
/*--- ------------------------------------------------------------*/
static void Die( Work *work )
{
}
/*--- ------------------------------------------------------------*/
static int WatcherUnitCall( char *top )
{
	int i, id ;
	char *p ;

	for( i = 0; (p=GCL_NextStr()) != NULL; i++ ){
		if ( i >= MAX_UNIT ) return (-1) ;
		/* ユニット登録 */
		if ( COM_SetUnit( &wccom->group, &wccom->unit[i] ) < 0 ) return (-1) ;
		id = GCL_GetInt( p ) ;
		p = GCL_NextStr();
		GCL_ExecProc( id, NULL );
		GCL_SetArgTop( p );
	}
	return i;
}

static int GetResources( Work *work, int where )
{
	char	*opt ;

	wccom = work ;
	wccom->com = COM_GetCommander( ) ;
	wccom->c_zone = 0 ;

	/* グループ登録 */
	if ( COM_SetGroup( &wccom->com->enemys, &wccom->group ) < 0 ) return (-1) ;

	/* 警備兵コール */
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
			if ( WatcherUnitCall( opt ) < 0 ) return (-1) ;
	}
	
	return 0 ;
}

/*--- ------------------------------------------------------------*/
void *NewWcCommander( name, where )
int	name ;
int	where ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), SUBCOMM_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
