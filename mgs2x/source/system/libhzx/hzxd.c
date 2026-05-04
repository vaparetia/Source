//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   hzxd.c
   ハザード処理関係初期化

   1999/07/07 M.Sonoyama
   $Id: hzxd.c,v 1.1.1.3 2002/11/19 11:42:47 Yoshizawa1 Exp $			
*/

/*-------------------------------------------------------

### 初期化 ###
void	HZX_Initialize( void ) 

---------------------------------------------------------*/
#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <devgif.h>

#include	"gameheader.h"
#endif

#ifdef KP_XBOX
#include <xtl.h>
#include	"mgsx_type.h"
#include	"libgv.h"
#include	"libhzx.h"
#include	"libgcl.h"

#endif

#if 0 //BP_PS2
//#ifdef PSX2
sceDmaChan		*HZX_Vif0Dma ;
#endif
HZX_HDL			*HZX_CurrentHzx = NULL ;
HZX_GROUP_ID 	HZX_CurrentGroupID = 0 ;
HZX_GROUP_ID	HZX_AllMapID = 0 ;
HZX_GROUP_ID	HZX_GroupAdd[ HZX_MAX_GROUPS ] = { 0 } ;

int				HZX_N_MapLinks ;
HZX_MAPLINK		*HZX_MapLink ;

/*----------------------------------------------------------

  トラップＯＮ／ＯＦＦシステム

----------------------------------------------------------*/

typedef	struct _TRAP_SWITCH_LIST {
	struct _TRAP_SWITCH_LIST	*next ;
	int		trap_id ;
	int		on ;
	int		pad ;
} TRAP_SWITCH_LIST ;

typedef	struct	{
	GV_ACT					actor ;
	TRAP_SWITCH_LIST		list ;
} TRAP_SWITCH_SYSTEM_WORK ;

static	TRAP_SWITCH_SYSTEM_WORK	TrapSwitchSystemWork ;

static	void	TrapSwitchAct( work )
TRAP_SWITCH_SYSTEM_WORK	*work ;
{
	TRAP_SWITCH_LIST	*list, *next ;

	list = work->list.next ;
	while( list != NULL ) {
		next = list->next ;
		HZX_SwitchTrapQuick( 0, list->trap_id, list->on ) ;
		GV_Free( list ) ;
		list = next ;		
	}	
	work->list.next = NULL ;
}

static	void	TrapSwitchDie( work )
TRAP_SWITCH_SYSTEM_WORK	*work ;
{
	TRAP_SWITCH_LIST	*list, *next ;

	list = work->list.next ;
	while( list != NULL ) {
		next = list->next ;
		GV_Free( list ) ;
		list = next ;
	}
	work->list.next = NULL ;
}

void		*HZX_StartTrapSwitchSystem( void )
{
	TRAP_SWITCH_SYSTEM_WORK	*work ;

	work = &TrapSwitchSystemWork ;

	GV_ZeroMemory( &work->actor, sizeof( TRAP_SWITCH_SYSTEM_WORK ) ) ;
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_SYSTEM ) ;
	GV_SetActorKillLevel( &work->actor, GV_KILL_LEVEL_NORMAL ) ;
	GV_InsertActorPriority( GV_ACTOR_MANAGER, &work->actor, 0 ) ;

	GV_SetActor( &work->actor, TrapSwitchAct, TrapSwitchDie ) ;

	work->list.next = NULL ;

	/* デフォルトセット */
	HZX_SwitchTrapDefault() ;

	return work ;					
}

int		HZX_SwitchTrap( id, on )
int		id, on ;
{
	TRAP_SWITCH_LIST	*this, *list ;

	this = ( TRAP_SWITCH_LIST * )GV_Malloc( sizeof( TRAP_SWITCH_LIST ) ) ;
	if ( this == NULL ) return 0 ;
	
	list = &TrapSwitchSystemWork.list ;
	while( list->next != NULL ) list = list->next ;
	
	this->trap_id = id ;
	this->on = on ;
	this->next = NULL ;

	list->next = this ;
	return 1 ;
}

/*----------------------------------------------------------

  ＨＺＸシステム初期化
  
----------------------------------------------------------*/

/* Vu0 を初期化 */
static	void	HZX_InitVu0( void )
{
#if 0 //BP_PS2
//#ifdef PSX2	
//    sceDevVu0Reset() ;
//    sceDevVif0Reset() ;
    HZX_Vif0Dma = sceDmaGetChan( SCE_DMA_VIF0 ) ;
    HZX_Vif0Dma->chcr.TTE = 1 ;
#endif	
}

void 	HZX_ResetGroupAdd( void )
{
	int		i ;

	for ( i = 0; i < HZX_MAX_GROUPS; i ++ ) {
		HZX_GroupAdd[ i ] = 0 ;
	}
}

void HZX_ResetSystem( void )
{
    HZX_InitVu0() ;
    HZX_InitTrapBind() ;
	HZX_ResetGroupAdd() ;
}

void HZX_ResetMemory( void )
{
    HZX_InitHzxCache() ;
	HZX_FreeHandler() ;
    HZX_CurrentHzx = NULL ;
	HZX_ResetCurrentGroup() ;
	HZX_AllMapID = 0 ;
	HZX_N_MapLinks = 0 ;
	HZX_MapLink = NULL ;
}

void	HZX_Initialize( void ) 
{
	HZX_CurrentHzx = NULL ;
	HZX_MapLink = NULL ;
    HZX_ResetSystem();
    HZX_ResetMemory();
}

void HZX_StartDaemon( void )
{
    GV_SetLoader( 'h', ( GV_LOADFUNC )HZX_LoadHzx ) ;
	HZX_Initialize() ;
}

/*----------------------------------------------------------

  ＨＺＸユーティリティ
  
----------------------------------------------------------*/

/* トラップＯＮ／ＯＦＦコマンド */
int		NewTrapSwitchCommand( void )
{
	int	trap, on ;

	GCL_GetOption( 't' ) ;
	trap = GCL_GetNextInt() ;
	GCL_GetOption( 's' ) ;
	on = GCL_GetNextInt() ;
	HZX_SwitchTrap( trap, on ) ;
	return 0 ;
}

/* 追加グループの登録 */
int				NewHzxSetGroupAdd( void ) 
{
	HZX_GROUP_ID	org, add ;

	org = GCL_GetOptionValue( 'h', -1 ) ;
	add = GCL_GetOptionValue( 'a', -1 ) ;
	HZX_GroupAdd[ org ] |= GV_GetBit( add ) ;
	return 0 ;
}

/* 追加グループの削除 */
int				NewHzxResetGroupAdd( void ) 
{
	HZX_GROUP_ID	org, add ;

	org = GCL_GetOptionValue( 'h', -1 ) ;
	add = GCL_GetOptionValue( 'a', -1 ) ;
	HZX_GroupAdd[ org ] &= ~GV_GetBit( add ) ;
	return 0 ;
}

/* 指定グループＩＤに追加グループＩＤを追加して返す */
HZX_GROUP_ID	HZX_AddGroupID( HZX_GROUP_ID hzx_id_in ) 
{
	HZX_GROUP_ID		hzx_id_out ;
	int					group ;

	hzx_id_out = hzx_id_in ;
	while( hzx_id_in != 0 ) {
		group = GV_GetNo( hzx_id_in ) ;
		hzx_id_in &= ~GV_GetBit( group ) ;
		hzx_id_out |= HZX_GroupAdd[ group ] ;
	}
	return hzx_id_out ;
}
