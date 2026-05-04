//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mapcnct.c
		マップ間接続管理デーモン
	2000/01/19	K.Uehara
	$Id: mapcnct.c,v 1.1.1.3 2002/11/19 11:41:53 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libvu0.h>
#include <libdma.h>
#include <libgraph.h>
#endif

#include "gameheader.h"

typedef struct _work {
	GV_ACT actor;
	struct _work *next;
	int name;
	int status;
	int map1name;
	int map2name;
	int map1id;
	int map2id;
	int connectid;
	int proc;
	int	type ;
	int	status_buf ;
} Work;

static Work *map_connect_list = NULL;
static int	map_connect_flag = 0 ;

enum {
	STATE_OFF = 0,
	STATE_ON = 1
};

enum {
	FLAG_NONE 					=	0x0000,
	FLAG_FORBIDDEN_TYPE1_OFF 	=	0x0001,	/* Type1ステートＯＦＦ変更禁止 */
} ;

static int search_on_map( int req )
{
	Work *wp;
	int on;

	on = req;

	for( ;; ){
		int off = 0;
		for( wp = map_connect_list; wp != NULL; wp = wp->next ){
			if( wp->status == STATE_ON ){
				if( on & wp->connectid ){
					on |= wp->connectid;
				} else {
					off |= wp->connectid;
				}
			}
		}
		if( ( on & off ) == 0 ){
			break;
		}
	}
	return on;
}

void change_map_connection( Work *work, int command )
{
	//int show ;

	if ( work->type != 0 && command == STATE_OFF &&
		 ( map_connect_flag & FLAG_FORBIDDEN_TYPE1_OFF ) ) {
		/* タイプ１ＯＦＦ変更禁止 */
		return ;
	}

	switch( command ){
	  case 0:
		work->status = STATE_OFF;
		break;
	  case 1:
		work->status = STATE_ON;
		break;
	}
#if 0
	show = search_on_map( GM_PlayerMap );
	if( show != GM_CurrentStageMap ){
		GM_ShowMap( show );
	}
#else
	GM_CalcChanlMap();
#endif
}

void GM_ChangeMapConnection( int name, int command )
{
	Work *wp;

	for( wp = map_connect_list; wp != NULL; wp = wp->next ){
		if( wp->name == name ){
			change_map_connection( wp, command );
			return;
		}
	}
}

int GM_GetConnectMapID( int map )
{
	return search_on_map( map );
}

/* 即時型、マップ接合変更 */
int		GM_COM_ChangeMapConnectStatus( void )
{
	int			name ;
	int			command ;

	name = GCL_GetNextInt() ;
	command = GCL_GetNextInt() ;

	if ( command == GM_STRCODE_ON ) command = 1 ;
	else							command = 0 ;

	GM_ChangeMapConnection( name, command ) ;
	return 0 ;
}

static void Act( Work *work )
{
	int n_msg;
	GV_MSG *msg;

	n_msg = GV_ReceiveMessage( work->name, &msg );
	if( n_msg > 0 ){
		for( ; n_msg > 0; n_msg -- ){
			change_map_connection( work, msg->message[ 0 ] );
		}
	}
	//GV_WaitMessage( work, work->name );
}

static	void	Die( Work *work )
{
    map_connect_list = NULL ;
}

static int GetResources( Work *work, int name )
{
	MAP *map;

	work->name = name;

	work->map1name = GCL_GetNextInt();
	work->map2name = GCL_GetNextInt();

	map = GM_GetMap2( work->map1name );
	if( map == NULL ){
		printf( "NewMapConnect: no map %X\n", work->map1name );
		return -1;
	}
	work->map1id = map->id;
	
	map = GM_GetMap2( work->map2name );
	if( map == NULL ){
		printf( "NewMapConnect: no map %X\n", work->map2name );
		return -1;
	}
	work->map2id = map->id;

	if( GCL_GetOption( 'p' ) != NULL ){
		work->proc = GCL_GetNextInt();
	} else {
		work->proc = 0;
	}

	work->connectid = work->map1id | work->map2id;
	work->status = STATE_OFF;

	work->type = GCL_GetOptionValue( 't', 0 ) ;

	/* リストにつなぐ */
	work->next = map_connect_list;
	map_connect_list = work;

	return 0;
}

void *NewMapConnect( int name, int map )
{
	Work *work;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( Work ), 0 );
	GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );

	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		if( GetResources( work, name ) < 0 ){
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return work;
}

/*----------------------------------------------------------------*/

/* プログラム起動 */
void	*NewMapConnectProg( int name, int map1, int map2 )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( Work ), 0 ) ;
	GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		work->name = name ;
		work->status = STATE_OFF ;
		work->map1name = GM_GetMapName( map1 ) ;
		work->map2name = GM_GetMapName( map2 ) ;
		work->map1id = map1 ;
		work->map2id = map2 ;
		work->connectid = map1 | map2 ;
		work->proc = 0 ;

		work->next = map_connect_list;
		map_connect_list = work;
	}
	return work;	
}

void	GM_ChangeMapConnection2( void *ptr, int command )
{
	if ( map_connect_list == NULL ) return ;
	change_map_connection( ( Work * )ptr, command ) ;
}

void	GM_ChangeMapConnectID( void *ptr, int map1, int map2 )
{
	Work		*work ;

	if ( map_connect_list == NULL ) return ;

	work = ( Work * )ptr ;
	work->map1name = GM_GetMapName( map1 ) ;
	work->map2name = GM_GetMapName( map2 ) ;
	work->map1id = map1 ;
	work->map2id = map2 ;
	work->connectid = map1 | map2 ;
	change_map_connection( work, work->status ) ;
}

void	GM_ForbidMapConnectOff( void )
{
	if ( map_connect_flag & FLAG_FORBIDDEN_TYPE1_OFF ) return ;
	map_connect_flag |= FLAG_FORBIDDEN_TYPE1_OFF ;
	/* Ｔｙｐｅ１の現在のステートを保存 */
	{
		Work *wp;
		
		for ( wp = map_connect_list; wp != NULL; wp = wp->next ) {
			wp->status_buf = wp->status ;
		}
	}
}

void	GM_EnableMapConnectOff( void )
{
	if ( !( map_connect_flag & FLAG_FORBIDDEN_TYPE1_OFF ) ) return ;
	map_connect_flag &= ~FLAG_FORBIDDEN_TYPE1_OFF ;
	/* Ｔｙｐｅ１のステートを戻す */
	{
		Work *wp;
		
		for ( wp = map_connect_list; wp != NULL; wp = wp->next ) {
			wp->status = wp->status_buf ;
		}
	}
}

void	GM_InitMapConnect( void )
{
	map_connect_list = NULL ;
	map_connect_flag = 0 ;
}
