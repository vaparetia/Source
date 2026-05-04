//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	command.c
	敵兵統括プログラム
	
	1999/07/07 Y.Korekado
	$Id: command.c,v 1.1.1.3 2002/11/19 11:44:06 Yoshizawa1 Exp $
	
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
#include	"enemy.h"

#include	"command.h"
#include	"data.h"

#include "jimaku.h"

#include "BP_EndianSupport.h"

#include	"BP_Debug.h"   //BP_ENEMY


static	ALIGN16_PRE COMMANDER 	Comm ALIGN16_POST ;

#include	"clearing.h"
#include	"clearing.c"
#include	"speak.h"
#include	"speak.c"

#define		WATCHER_NUM 12
#define		ATTACKER_NUM 0

// AS - Needs to be the same value as defined in vibrate.c
#define VIBRATION_AP_SENSOR 1024

static	int	gameover_delay_count ;
#define GAME_OVER_DELAY ( COUNT_VMODE(30) )
/*--- ------------------------------------------------------------*/
int		DEF_SEARCH_LEVEL ;

/* どっかに移動する予定 */
int		CM_TMPSNAKEAREA ;	/* プレイヤーがクリアリングエリアに入っているとそのエリア番号が入る */
int		ENE_AlertGameLevel ;
int		ENE_GameStatus = 0 ;

int		*COMMANDER_STATUS ;
int		CM_ZZZ_TIME ;
int		CM_FAINT_TIME ;
/*--- ------------------------------------------------------------*/
static void ComProcCall( int proc_no )
{
	ENE_ExecProc( Comm.com_procs[ proc_no ], NULL ) ;
	Comm.com_procs[ proc_no ] = 0 ;/* 一回しか呼ばない */
}
/*--- ------------------------------------------------------------*/
void COM_DiscoverCount(void )
{
	extern int VR_DiscoverCount ;

	if ( !GM_CheckGameStatus( GM_STATUS_DETECT ) &&
		 !(Comm.flame_flag & CMFLAG_DETECT) ) {
		VR_DiscoverCount++ ;

printf("VR DiscoverCount ++++++++++++++ [%d] \n",VR_DiscoverCount ) ;

	}
}

u_int COM_GetRootOfset( void )
{
#if 0
printf("Comm.root_ofset[ %d ]\n",Comm.root_ofset ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[0] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[1] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[2] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[3] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[4] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[5] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[6] ) ;
printf("vr_pat_offset[ %d ]\n",HZX_CurrentHzx->def->vr_pat_offset[7] ) ;
#endif
	
//	return  (u_int)HZX_CurrentHzx->def->vr_pat_offset[Comm.root_ofset] ;
	return	GM_RouteOffset ;
}
u_int COM_GetClearingOfset( void )
{
//	return  (u_int)HZX_CurrentHzx->def->vr_clr_offset[Comm.root_ofset] ;
	return	GM_ClearingOffset ;
}

void COM_SetAvoidTime( int time )
{
	Comm.time = time ;
}

/* 同一チームか？ */
int COM_SameGroupUnit( ENETHINK *entk, ENETHINK *you )
{
	if ( entk->g_id != you->g_id ) return 0 ;
	if ( entk->u_id != you->u_id ) return 0 ;

	return 1 ;
}

/* 同一メンバー数取得 */
int COM_GetGroupNum( ENETHINK *entk )
{
	E_UNIT	*unit ;

	unit = COM_GetUnit( entk->g_id, entk->u_id ) ;
	return unit->enemy_num ;
}

static void PosNearEnemy( FVECTOR *pos, int *id, int *near_dis )
{
	int			i, j, k, dis, min;
	E_GROUP		*group ;
	E_UNIT		*unit ;
	ENETHINK	*entk ;
	COMMANDER	*com ;

	com = &Comm ;
	*id = -1 ;

	min = 1000000 ;
	/*sigeno追加 攻撃兵もループさせる*/
	for(k=0;k<com->enemys.group_num;k++){
		group = com->enemys.group[ k ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[ i ] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( (entk = unit->entk[ j ]) == NULL ) continue ;
				if ( !(entk->act->status & (ACT_STATUS_DAMAGE|ACT_STATUS_UNREAL)) ) {
					dis = _FVecTrgDis( &entk->ctrl->mov, pos ) ;
					if ( min > dis ) {
						min = dis ;
						*id = entk->uniq_id ;
						*near_dis = dis ;
					}
				}
			}
		}
	}

printf ( " investigated !! id[%d] \n",*id ) ;
}


int COM_GetInvestigatedNearID( ENETHINK *entk )
{
	COMMANDER	*com ;

	com = &Comm ;
	if ( (com->investigated) && (entk->uniq_id == com->uniqid_near_investigated) ) return 1 ;

	return 0 ;
}

/*
void COM_SetInvestigated( int mode, FVECTOR *pos,HZX_GROUP_ID gid )
{
	COMMANDER	*com ;

	com = &Comm ;
	com->investigated = mode ;
	com->pos_investigated = *pos ;
	com->groupid_investigated = gid ;
	PosNearEnemy( pos, &com->uniqid_near_investigated, &com->dis_investigated ) ;
}
*/

void COM_SetAlertStatus( int status )
{
	Comm.alertstatus |= status ;
}

int COM_AlertStatus( )
{
	return Comm.alertstatus ;
}

/* 段ボールは、０なら動いていない、１なら動いている */
int COM_CheckBoxPos( FVECTOR *pos )
{
	FVECTOR *old_pos ;
	
	old_pos = &Comm.plpos_box ;
	if ( DG_FABS(pos->vx - old_pos->vx) > 5.0f ) return 1 ;
	if ( DG_FABS(pos->vy - old_pos->vy) > 5.0f ) return 1 ;
	if ( DG_FABS(pos->vz - old_pos->vz) > 5.0f ) return 1 ;
	return 0 ;
}

void COM_SetBoxPos( FVECTOR *pos )
{
	Comm.plpos_box = *pos ;
}

int COM_GetUnitBuff( entk, n )
ENETHINK	*entk ;
int			n ;
{
	E_UNIT	*unit ;
	
	unit = COM_GetUnit( entk->g_id, entk->u_id ) ;
	return unit->u_buff[ n ] ;
}

int	COM_SetAccident( int uniq_id, FVECTOR *pos, HZX_GROUP_ID hzx_id, int delay, int status )
{
	COMMANDER	*com ;
	int i ;

	com = &Comm ;

	/* アクシデント無しステージ */
	if ( com->stage_kind & (ENE_STAGE_NO_ACCIDENT|ENE_STAGE_NO_COMMANDER) ) return -1 ;

	/* 破壊後の確認はなし */
	if ( status & (ENE_ACCIDENT_MECA_BREAK) ) return -1 ;

	/* 危険モード中は受け付けない */
	if ( GM_AlertMode == ALERT_MODE_ALERT ) return -1 ;

	/* 先ず、同じ兵がアクシデントを起こしてないかチェック */
	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if ( com->accident_uniq_id[i] == uniq_id ) {
			if ( com->accident_bit & 1<<i ) {
				/* 既にアクシデント */
				return i ;
			}
			if ( com->accident_delay[i] > 0 ) {
				/* 既に通報済みならdelayの短い方に書き換え */
				if ( (com->accident_delay[i] > COUNT_VMODE(1000)) &&
					 (com->accident_delay[i] > delay) ) {
					com->accident_delay[i] = delay ;
				}
				return i ;
			}
		}
	}

	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if ( !(com->accident_bit & 1<<i) && !(com->accident_uniq_id[i]) ) {
			com->accident_pos[i] = *pos ;
			com->accident_map[i] = hzx_id ;
			com->accident_delay[i] = delay ;
			com->accident_uniq_id[i] = uniq_id ;
			com->accident_status[i] = status ;
printf("ENEMY ACCIDENT!! [%f][%f][%f][%x] [%d] [%d]\n",pos->vx,pos->vy,pos->vz,hzx_id, uniq_id,i );
			return i ;
		}
	}
	
	return -1 ;
}

static int AccidentIdToNum( u_int uniq_id )
{
	int i ;

	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if( Comm.accident_uniq_id[i] == uniq_id ) return i ;
	}
	return -1 ;
}
	

void	COM_UnsetAccident( int id )
{
	int	num ;
	COMMANDER	*com ;

	com = &Comm ;

	num = AccidentIdToNum( id ) ;
	if ( num < 0 ) return ;
	UNSET_FLAG( com->accident_bit, 1<<num ) ;
printf("ENEMY UNSET ACCIDENT!! [%d] \n",id );
	com->accident_delay[ num ] = 0 ;
	com->accident_uniq_id[ num ] = 0 ;
	com->accident_status[ num ] = 0 ;
}

void	COM_FreeReservAccident( int id )
{
	int	num ;
	COMMANDER	*com ;

	com = &Comm ;

	num = AccidentIdToNum( id ) ;
	if ( num < 0 ) return ;
	SET_FLAG( com->free_accident_bit, 1<<num ) ;
}

void COM_ClearAccident( void )
{
	COMMANDER	*com ;
	int i ;

	com = &Comm ;
	com->accident_bit = 0 ;

	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		com->accident_delay[i] = 0 ;
		com->accident_uniq_id[ i ] = 0 ;
		com->accident_status[ i ] = 0 ;
	}
}

int	COM_AccidentToID( int id )
{
	int	num ;
	COMMANDER	*com ;

	com = &Comm ;

	num = AccidentIdToNum( id ) ;
	if ( num < 0 ) return 0 ;

//printf("id[%d] num[%d] accident bit[%x] res[%d]\n",id,num,com->accident_bit,(com->accident_bit & (1<<num)) ) ;
	return (com->accident_bit & (1<<num)) ;
}

int COM_Accident( void )
{
	COMMANDER	*com ;
	int i ;

	com = &Comm ;
	
	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if ( com->accident_bit & 1<<i ) {
			return Comm.accident_uniq_id[i] ;
		}
	}
	return -1 ;
}

FVECTOR *COM_GetAccidentPos( int uniq_id )
{
	COMMANDER	*com ;
	int  num ;

	com = &Comm ;
	num = AccidentIdToNum( uniq_id ) ;

	ASSERT ( num < MAX_ACCIDENT ) ;
	if ( num < 0 ) {
		return NULL ;
	}

	return &com->accident_pos[num] ;
}

HZX_GROUP_ID COM_GetAccidentMap( int uniq_id )
{
	COMMANDER	*com ;
	int  num ;

	com = &Comm ;
	num = AccidentIdToNum( uniq_id ) ;

	ASSERT ( num < MAX_ACCIDENT ) ;
	if ( num < 0 ) {
		return 0 ;
	}

	return com->accident_map[num] ;
}

int COM_GetAccidentStatus( int uniq_id )
{
	COMMANDER	*com ;
	int  num ;

	com = &Comm ;
	num = AccidentIdToNum( uniq_id ) ;

	ASSERT ( num < MAX_ACCIDENT ) ;
	if ( num < 0 ) {
		return 0 ;
	}

	return com->accident_status[num] ;
}

/*--- ------------------------------------------------------------*/
int COM_RouteProcCall( int route, int point, ENETHINK *entk )
{
	ROUTE_PROC	*proc ;
	
	proc = &Comm.route_proc[0] ;

	while( proc->proc > 0 ) {
		if( proc->route == route ) {
			if( proc->point == point ) {
				KR_ProcCallSendName( proc->proc, entk->ctrl->name ) ;
				return 1 ;
			}
		}
		proc ++ ;
	}

	return 0 ;
}

ENETHINK *COM_GetEnemyFromOldUniqID( int id )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	com = &Comm ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->old_uniq_id == id ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

ENETHINK *COM_GetEnemyFromUniqID( int id )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	com = &Comm ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->uniq_id == id ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

ENETHINK *COM_GetEnemyFromName( int name )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	com = &Comm ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->ctrl->name == name ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

static ENETHINK *COM_CaptureModelName( )
{
	COMMANDER	*com ;
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int i, j, g ;

	com = &Comm ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;

				if ( unit->entk[ j ]->act->status & ACT_STATUS_CAPTURE ) {
					return unit->entk[ j ] ;
				}

			}
		}
	}
	return NULL ;
}

int	COM_GetCaptureModelName( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;


	entk = CP_CaptureCorpModelName( ) ; 
	name = 0 ;

	if ( entk == NULL ) entk = COM_CaptureModelName( ) ;

	if ( entk != NULL ) {
		name = entk->name_id.body ;
	}

printf(" get capture model name result [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, name );

	return 1 ;
}

int	COM_GetCaptureName( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;


	entk = CP_CaptureCorpModelName( ) ; 
	name = 0 ;

	if ( entk == NULL ) entk = COM_CaptureModelName( ) ;

	if ( entk != NULL ) {
		name = entk->ctrl->name ;
	}

printf(" get capture ctrl name result [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, name );

	return 1 ;
}

int	COM_GetEnemyThinkStatus( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy status: Not Found Enemy!!\n" ) ;
		return -1 ;
	}
//printf(" get enemy status [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
//	GCL_SetVarRef( &ref, 0, (long64)entk->act->status );
	/* 下位３２ビットしかシナリオで使わないと断定 */
	GCL_SetVarRef( &ref, 0, (int)entk->thk_status );

	return 1 ;
}

int	COM_GetEnemyStatus( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy status: Not Found Enemy!!\n" ) ;
		return -1 ;
	}
//printf(" get enemy status [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
//	GCL_SetVarRef( &ref, 0, (long64)entk->act->status );
	/* 下位３２ビットしかシナリオで使わないと断定 */
	GCL_SetVarRef( &ref, 0, (int)entk->act->status );

	return 1 ;
}

/* 上位３２ビット */
int	COM_GetEnemyStatus2( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name, status ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy status: Not Found Enemy!!\n" ) ;
		
		return -1 ;
	}
//printf(" get enemy status [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	status =(int)(entk->act->status >> 32 ) ;
	GCL_SetVarRef( &ref, 0, status );

	return 1 ;
}

int	COM_GetEnemyPos( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy pos: Not Found Enemy!!\n" ) ;
		
		return -1 ;
	}
//printf(" get enemy pos [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)entk->ctrl->mov.vx );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)entk->ctrl->mov.vy );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)entk->ctrl->mov.vz );

	return 1 ;
}

int	COM_GetEnemyStSt( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy status: Not Found Enemy!!\n" ) ;
		
		return -1 ;
	}
//printf(" get enemy status [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
//	GCL_SetVarRef( &ref, 0, (long64)entk->act->status );
	/* 下位３２ビットしかシナリオで使わないと断定 */
	GCL_SetVarRef( &ref, 0, (int)entk->act->status_status );

	return 1 ;
}

int	COM_GetEnemyAction( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;
	ROUTENAVI	*rnavi ;
	

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy pos: Not Found Enemy!!\n" ) ;
		return -1 ;
	}
//printf(" get enemy pos [%d]\n",name ) ;
	rnavi = entk->rnavi ;
	if ( rnavi == NULL ) {
		printf(" get enemy pos: Not Found Root Action!!\n" ) ;
		return -1 ;
	}

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)rnavi->c_route );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)rnavi->next_node );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)rnavi->pa_action[(int)rnavi->next_node] );

	return 1 ;
}


int	COM_GetEnemyNoticeMode( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	ENETHINK	*entk ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" get enemy status: Not Found Enemy!!\n" ) ;
		
		return -1 ;
	}
//printf(" get enemy status [%d]\n",name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
//	GCL_SetVarRef( &ref, 0, (long64)entk->act->status );
	GCL_SetVarRef( &ref, 0, (int)entk->c_notice );

	return 1 ;
}

int	COM_GetCleRoute( id )
int	id ;
{
	return Comm.cle_route[ id ] ;
}
int	COM_GetClePoint( id )
int	id ;
{
	return Comm.cle_point[ id ] ;
}

int	COM_SetClearingData( void )
{
	int num, route[ATENEMY_MAX], i ;
	char	*opt ;

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		num=Comm.tmp_cle_proc_num = GetClearingProc( &Comm.tmp_cle_proc[0] ) ;
		if ( num >= MAX_CLE_PROC ) {
			printf("command.c: Err!! Clearing Proc Num Over[%d]!! \n",num);
			return -1 ;
		}
	} else {
		Comm.tmp_cle_proc_num = 0 ;
	}

	if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){
		num = ENE_GclGetInt( opt, &route[ 0 ] ) ;
		for( i=0; i<num; i++ ) {
			Comm.cle_route[ i ] = (short)route[ i ] ;
/*			if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
printf( " cle_route[%d] + COM_GetClearingOfset[%d]=n", Comm.cle_route[ i ], COM_GetClearingOfset( ) ) ;
				Comm.cle_route[ i ] += COM_GetClearingOfset( ) ;
			}*/
printf( " enemu[%d] clearing route[%d]\n", i, Comm.cle_route[ i ] ) ;
		}
	}

	return 1 ;
}

int	COM_ChangeClearingRoot( void )
{
	ENETHINK	*entk ;
	int	name, route, point ;
	
	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (entk = COM_SetSupportEnemy( name )) == NULL ) {
//		printf(" command.c:Not Found Enemy[%d]!!\n",name ) ;
		
		return -1 ;
	}

	if ( (route = GCL_GetOptionValue( 'r', -1 )) < 0  ) {
		return 0 ;
	}
	point = GCL_GetOptionValue( 'p', 0 ) ;

	printf("clearing: id[%d] change route[%d] point[%d]\n",entk->id, route, point ) ;

	Comm.cle_route[ entk->id ] = route ;
/*	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		Comm.cle_route[ entk->id ] += COM_GetClearingOfset( ) ;
	}*/
	Comm.cle_point[ entk->id ] = point ;

	return 1 ;
}

void COM_InitTmpCleProc( void )
{
	int i ;

	Comm.tmp_cle_proc_num = 0 ;
	for ( i=0; i<ATENEMY_MAX; i++ ) {
		Comm.cle_route[ i ] = i ;
		Comm.cle_point[ i ] = 0 ;
	}
}


void COM_CleProc( area, route, point )
int area ;
int route ;
int point ;
{
	CLEARING_PROC	*proc ;
	int i ;
	
	proc = &Comm.cle_proc[0] ;
	for( i=0; i<Comm.cle_proc_num; i++, proc++ ) {
		if( proc->area == area) {
			if( proc->route == route ) {
				if( proc->point == point ) {
printf("call proc area[%d] route[%d] point[%d]\n",area,route,point) ;
printf("set proc[%d] \n",proc->proc) ;
					ENE_ExecProc( proc->proc, NULL ) ;
				}
			}
		}
	}
	proc = &Comm.tmp_cle_proc[0] ;
	for( i=0; i<Comm.tmp_cle_proc_num; i++, proc++ ) {
		if( proc->area == area) {
			if( proc->route == route ) {
				if( proc->point == point ) {
printf("call tmp proc area[%d] route[%d] point[%d]\n",area,route,point) ;
printf("set proc[%d] \n",proc->proc) ;
					ENE_ExecProc( proc->proc, NULL ) ;
				}
			}
		}
	}

}


void COM_SetFlameFlag( flag )
int	flag ;
{
	Comm.flame_flag |= flag ;
}

int COM_GetNoisePower( )
{
	return Comm.noise ;
}

int COM_GetNoiseNearID( entk )
ENETHINK	*entk ;
{
	COMMANDER	*com ;
	int			dis, hearing ;

	com = &Comm ;
#if 1
//sigeno add 完全 耳無し兵
	if ( entk->act->bodyp.type & ENE_TYPE_NO_HEAR ) {
		return 0 ;
	}
#endif

	switch( com->noise ) {
		case NOISE_HOLD :
			if ( !(ENE_PlayerHoldCheck( entk )) ) return 0 ;
			if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &entk->ctrl->mov, &GM_PlayerPosition ) ){
				return 0 ;
			}
			break ;
		case NOISE_SS :
			if ( entk->g_id != (int)com->noise_near_id[0] ) return 0 ;
			if ( entk->u_id != (int)com->noise_near_id[1] ) return 0 ;
			if ( entk->id != (int)com->noise_near_id[2] ) return 0 ;
			hearing = entk->sense.hearing ;
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				hearing = ( entk->status & ENE_STATUS_NORINORI ) ? 
							NORINORI_HEARING : entk->sense.hearing ;
			}
			if ( com->noise_dis > hearing/4 ) return 0 ;
			break ;
		case NOISE_S :
		case NOISE_MM :
			if ( entk->g_id != (int)com->noise_near_id[0] ) return 0 ;
			if ( entk->u_id != (int)com->noise_near_id[1] ) return 0 ;
			if ( entk->id != (int)com->noise_near_id[2] ) return 0 ;
			break ;
		case NOISE_SCREAM :
		case NOISE_M :
			if ( entk->act->bodyp.type & ENE_TYPES_EAR_NARROW ) {
printf("ENE_TYPES_EAR_NARROW [%f]\n",DG_FABS( entk->ctrl->mov.vy - GM_NoisePosition.vy )) ;
				if ( DG_FABS( entk->ctrl->mov.vy - GM_NoisePosition.vy ) > 2000.0f ) {
					return 0 ;
				}
			}
			dis = _FVecTrgDis( &entk->ctrl->mov, &GM_NoisePosition ) ;
			hearing = entk->sense.hearing ;
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				hearing = ( entk->status & ENE_STATUS_NORINORI ) ? 
						NORINORI_HEARING : entk->sense.hearing ;
			}

			if ( com->noise == NOISE_M ) hearing *= 2 ;

			if( dis > hearing ) return 0 ;
			break ;
		case NOISE_L :
			break ;
	}

	return com->noise ;
}

/* 重い！要改良 */
ENETHINK	*COM_GetDamageEnemy( entk )
ENETHINK	*entk ;
{
	COMMANDER	*com ;
	ENETHINK	*dam_entk ;
	EYEINFO		eyei ;
	int			i ;

	com = &Comm ;
	for ( i=0; i<com->dam_en_num; i++ ) {
		dam_entk = com->dam_entk[i] ;
		if ( entk == dam_entk ) continue ;
		if ( dam_entk->act->bodyp.type & ENE_TYPE_CAMERA_USHOLD ) continue ;

//printf("1:[%f][%f][%f]\n",entk->ctrl->mov.vx,entk->ctrl->mov.vy,entk->ctrl->mov.vz);
//printf("2:[%f][%f][%f]\n",dam_entk->ctrl->mov.vx,dam_entk->ctrl->mov.vy,dam_entk->ctrl->mov.vz);
//printf("3:[%d][%d][%d]\n",dam_entk->g_id,dam_entk->u_id,dam_entk->id);

		ENE_SetEyeInfo( &eyei, &dam_entk->ctrl->mov, &dam_entk->ctrl->addr, 0, &dam_entk->ctrl->hzx_id ) ;

#if 0
		ENE_EyeInfoCheck( entk, &eyei ) ;
#else
		{
			int w_hzx_id ;
			w_hzx_id = entk->ctrl->hzx_id | dam_entk->ctrl->hzx_id ;
			ENE_EyeInfoCheckWhzxid( entk, &eyei, w_hzx_id ) ;
		}
#endif
//printf("enedamcorp sight[%d] dis[%d] dir[%d] \n",eyei.sight, eyei.dis, eyei.dir ) ;
		if ( eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( (entk->status & ENE_STATUS_ACCDNT_REPO) 
				|| (eyei.dis < entk->sense.eye_s) ) { /* 双眼鏡は無視 */
				return dam_entk ;
			}
		}
	}

	return NULL ;
}

/* 重い！要改良 */
int COM_GetCorpID( entk )
ENETHINK	*entk ;
{
	return CP_GetCorpID( entk ) ;
}

int COM_UnitThkStatus( int g_id, int u_id )
{
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			i,status ;

	un = Comm.enemys.group[ g_id ]->unit[ u_id ] ;
	status = 0 ;
	for ( i=0; i<un->enemy_num; i++ ) {
		if ( (entk = un->entk[ i ]) == NULL ) continue ;
		status |=  entk->thk_status ;
	}

	return status ;
}

ENETHINK *COM_NearEnemyThkStatus( ENETHINK *fromene, long64 status, int *min )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk, *out ;
	int			g, u, i, dis ;

	com = &Comm ;
	*min = 1000000 ;
	out = NULL ;
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk == fromene ) continue ;	/* 自分だったら */
				if ( entk->thk_status & status ) {
					dis = _FVecTrgDis( &fromene->ctrl->mov, &entk->ctrl->mov ) ;
//printf("entk->corp_id =[%d]\n",entk->corp_id );
					if ( dis < *min ) {
						*min = dis ;
						out = entk ;
					}
				}
			}
		}
	}

	return out ;
}

ENETHINK *COM_NearEnemyNotActStatus( FVECTOR *pos, long64 status, int *min )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk, *out ;
	int			g, u, i, dis ;

	com = &Comm ;
	*min = 1000000 ;
	out = NULL ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( !(entk->act->status & status) ) {
					dis = _FVecTrgDis( pos, &entk->ctrl->mov ) ;
					if ( dis < *min ) {
						*min = dis ;
						out = entk ;
					}
				}
			}
		}
	}

	return out ;
}

/*--- ------------------------------------------------------------*/
void	COM_UnitMemberDie( g_id, u_id )
int		g_id, u_id ;
{
	printf("group[%d] unit[%d]\n",g_id, u_id);
	Comm.enemys.group[ g_id ]->unit[ u_id ]->die_num++ ;
}

int		COM_GetUnitMemberDieNum( g_id, u_id )
int		g_id, u_id ;
{
	return Comm.enemys.group[ g_id ]->unit[ u_id ]->die_num ;
}

ENETHINK	*COM_GetMemberFromGUI( g_id, u_id, id )
int		g_id, u_id, id ;
{
	return Comm.enemys.group[ g_id ]->unit[ u_id ]->entk[ id ] ;
}

void	COM_SetSearchLevel( level )
int		level ;
{
	Comm.searchlevel = level ;
}

int		COM_GetSearchLevel( )
{
	return Comm.searchlevel ;
}

void	COM_WaitingPosition( pos, map )
FVECTOR	*pos ;
int		*map ;
{
#if 1
	*pos = Comm.waiting_pos[Comm.waiting_cur] ;
	*map = Comm.wait_map[Comm.waiting_cur] ;
#else
	int	i, dis, max, wt ;
	FVECTOR	vec ;
	
	max = 0 ;
	wt = 0 ;
	for( i=0; i<Comm.waiting_num; i++ ) {
		sceVu0SubVector(  &vec, &Comm.waiting_pos[i], &GM_PlayerPosition ) ;
		dis = _FVecLen3( &vec ) ;
		if ( max < dis ) {
			max = dis ;
			wt = i ;
		}
	}

	*pos = Comm.waiting_pos[wt] ;
	*map = Comm.wait_map[wt] ;
#endif
}

	/* 復活場所計算(プレイヤー位置から計算) */
#define	RESURRECTION_DIS	8000
void	COM_ResurrectionPosition( pos, map )
FVECTOR	*pos ;
int		*map ;
{
	int dis, rnd, i, num ;

	num = Comm.res_num ;
	rnd = (int)( BP_PS2_rand()%num ) ;
	
	for( i=0; i<num; i++ ) {
		dis = _FVecTrgDis( &Comm.res_pos[ rnd ], &GM_PlayerPosition ) ;
		/* 非表示マップならＯＫ */
		if ( !(HZX_CurrentGroupID & Comm.res_map[ rnd ]) ) break ;

		/* 距離チェック */
		if ( dis > RESURRECTION_DIS ) {
			/* 画面外チェック */
			if ( (DG_PointCheckOne( &Comm.res_pos[ rnd ], 0 )) ) {
printf("GAMENGA=====I!!");
				break ;
			} else {
printf("GAMENNNA=====I!!");
			}
		} else {
printf("TIKASUGI---I!!");
		}
		if ( ++rnd >= num ) rnd = 0 ;
	}
	
	*pos = Comm.res_pos[ rnd ] ;
	*map = Comm.res_map[ rnd ] ;
}
/*重野追加 */
void	COM_GetResPos( pos, map ,num )
FVECTOR	*pos ;
int		*map ;
int		num ;
{
	ASSERT(num < Comm.res_num) ;
	*pos = Comm.res_pos[ num ] ;
	*map = Comm.res_map[ num ] ;
}


int	COM_ResurrectionPermit( ENETHINK *entk )
{
	if ( !(entk->receive & ENE_ORDER_RESURRECT) ) {
		if ( GM_AlertMode == ALERT_MODE_SNEAK ) return 0 ;
		if ( GM_AlertMode == ALERT_MODE_SEARCH ) return 0 ;
		if ( GM_AlertMode == ALERT_MODE_AVOID ) return 0 ;
	}

	/* 一度に存在できる人数以上 */
	if ( Comm.enemy_count >= Comm.enemy_appear_max ) return 0 ;
	/* ステージで復活できる人数以上 */
	if ( Comm.res_count >= Comm.max_res_num ) return 0 ;
	/* 危険モードで復活できる人数以上 */
	if ( Comm.res_count_in_alert >= Comm.max_res_in_alert ) return 0 ;
	Comm.res_count_in_alert++ ;
	Comm.res_count++ ;

	printf(" Enemy Appear Count [%d] \n",Comm.enemy_count);
	printf(" Enemy Resurrectuin!! \n");
	return 1 ;
}

void	COM_CypherDestroy( )
{
	if ( (++Comm.cyp_kill_count) >= (Comm.cyp_res_count+Comm.cyp_num ) ) {
		ComProcCall( ComProcsCyperAllDestroy ) ;
	}
}

int	COM_SetCypher( )
{
	return Comm.cyp_num ++ ;
}

int	COM_CypherResurrectionPermit( )
{
	if ( GM_AlertMode == ALERT_MODE_SNEAK ) return 0 ;
	if ( GM_AlertMode == ALERT_MODE_SEARCH ) return 0 ;
	if ( GM_AlertMode == ALERT_MODE_AVOID ) return 0 ;

	if ( Comm.cyp_res_count >= Comm.cyp_max_res_num ) return 0 ;
	if ( Comm.cyp_res_count_in_alert >= Comm.cyp_max_res_in_alert ) return 0 ;
	Comm.cyp_res_count_in_alert++ ;
	Comm.cyp_res_count++ ;

	printf(" Cypher Resurrectuin!! \n") ;
	return 1 ;
}

void COM_GetPlayerLastPos( l_pos, l_map )
FVECTOR	*l_pos ;
int		*l_map ;
{
	 *l_pos = Comm.player_lastpos ;
	 *l_map = Comm.player_lastmap ;
}

void COM_SetPlayerLastPos( l_pos, l_map )
FVECTOR	*l_pos ;
int		l_map ;
{
	Comm.player_lastpos = *l_pos ;
	Comm.player_lastmap = l_map ;
}


//	H.Satoyoshi 2001.8.1
void COM_SetPureSecurityAlertLevel( int level )
{
	Comm.security_alert = level ;
}

void COM_SetSecurityAlert( pos, map )
FVECTOR	*pos ;
int		map ;
{
	Comm.security_alert = ALERT_LEVEL_MAX ;
	Comm.player_lastpos = *pos;
	Comm.player_lastmap = map ;
}

void COM_SetSecurityAlertLevel( FVECTOR	*pos, int map, int level )
{
	Comm.security_alert = level ;
	Comm.player_lastpos = *pos;
	Comm.player_lastmap = map ;
}

COMMANDER	*COM_GetCommander()
{
	return &Comm ;
}
E_GROUP	*COM_GetGroup( g_id )
int	g_id ;
{
	return Comm.enemys.group[ g_id ] ;
}
E_UNIT	*COM_GetUnit( g_id, u_id )
int	g_id ;
int	u_id ;
{
	return Comm.enemys.group[ g_id ]->unit[ u_id ] ;
}
int		COM_SetGroup( enemys, group )
ENEMYS	*enemys ;
E_GROUP	*group ;
{
	int	i ;
	
	if ( enemys->group_num >= MAX_GROUP ) return (-1) ;
	enemys->group[ enemys->group_num ] = group ;
	group->group_id = enemys->group_num ;
	group->unit_num = 0;
	for ( i=0; i<MAX_UNIT; i++ ) {
		group->unit[ i ] = NULL ;
	}

	enemys->group_num ++ ;

	return 0 ;
}
int		COM_SetUnit( group, unit )
E_GROUP	*group ;
E_UNIT	*unit ;
{
	int	i ;
	
	if ( group->unit_num >= MAX_UNIT ) return (-1) ;
	group->unit[ group->unit_num ] = unit ;
	unit->unit_id = group->unit_num ;
	unit->enemy_num = 0;
	unit->die_num = 0;
	for ( i=0; i<MAX_ENEMY; i++ ) {
		unit->entk[ i ] = NULL ;
	}

	group->unit_num ++ ;

	return 0 ;
}

void COM_UnsetEnethinkWork( entk )
ENETHINK	*entk ;
{
	E_UNIT	*unit ;

	unit = Comm.enemys.group[ entk->g_id ]->unit[ entk->u_id ] ;
	unit->entk[ entk->id ] = NULL ;
}

int COM_SetEnethinkWork( entk, group_id, unit_id )
ENETHINK	*entk ;
int			group_id ;
int			unit_id ;
{
	E_UNIT	*unit ;
	int	i ;

	unit = Comm.enemys.group[ group_id ]->unit[ unit_id ] ;

	for ( i=0; i<MAX_ENEMY; i++ ) {
		if ( unit->entk[ i ] == NULL ) break ;
	}
	if ( i == MAX_ENEMY ) return (-1) ;
	unit->entk[ i ] = entk ;
	if ( i >= unit->enemy_num ) unit->enemy_num = i+1 ;

printf("max ene[%d] id [%d] \n",unit->enemy_num, i) ;

	return i ;
}

int COM_GetUniqID( void )
{

printf("enemy;: unique id [%d]\n",Comm.uniq_id ) ;
	ASSERT( Comm.uniq_id >= 0 ) ;
	if ( Comm.uniq_id < 0 ) Comm.uniq_id = 1 ;
	return Comm.uniq_id++ ;
}

int COM_StageKind( void )
{
	return Comm.stage_kind ;
}

/* GetResources Only!!*/
void	COM_SetIDNumber( entk )
ENETHINK	*entk ;
{
	entk->g_id = Comm.enemys.group_num - 1 ;
	entk->u_id = Comm.enemys.group[ entk->g_id ]->unit_num - 1 ;
	entk->id = COM_SetEnethinkWork( entk, entk->g_id, entk->u_id ) ;
	entk->uniq_id = COM_GetUniqID( ) ;
	entk->old_uniq_id = -1 ;
printf("g_id[%d] u_id[%d] e_id[%d]\n",entk->g_id,entk->u_id,entk->id ) ;

	entk->com = COM_GetCommander() ;
}

ENETHINK *COM_SetSupportEnemy( name )
int	name ;
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	for ( g=0; g<Comm.enemys.group_num; g++ ) {
		gp = Comm.enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				entk = un->entk[ i ] ;
				if ( entk->ctrl->name == name ) {
					return entk ;
				}
			}
		}
	}
	return NULL ;
}

/*--- ------------------------------------------------------------*/
static	void	InitEnemys( com )
COMMANDER	*com ;
{
	ENEMYS	*enemys ;
	int	i ;
	
	enemys = &com->enemys ;
	enemys->group_num = 0 ;
	for ( i=0; i<MAX_GROUP; i++ ) {
		enemys->group[ i ] = NULL ;
	}

	com->security_alert = 0 ;
}


/* 所属以外で一番近い敵 */
ENETHINK	*ENE_NearEnemy_OtherUnit( entk )
ENETHINK	*entk ;
{
#if 0
	int 		i, j, min, dis ;
	FVECTOR		vec ;
	ENETHINK	*trgene ;
	
	min = 10000000 ;
	trgene = NULL ;

	/* 他ユニットとの距離 */
	for ( i=0; i< MAX_UNIT; i++ ) {
		if ( i != entk->unit ) {
			for ( j=0; j<Comm.enemy_num[ i ]; j++ ) {
				sceVu0SubVector(  &vec, &Comm.entk[ i ][ j ]->ctrl->mov, &(entk->ctrl->mov) ) ;
				dis = _FVecLen3( &vec ) ;
				if ( min > dis ) {
					min = dis ;
					trgene = Comm.entk[ i ][ j ] ;
				}
			}
		}
	}

	return trgene ;
#else
	return 0 ;
#endif
}

/*--- ------------------------------------------------------------*/
static void COM_CaptureKill( )
{
	COMMANDER	*com ;
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int i, j, g ;

	com = &Comm ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;

				if ( unit->entk[ j ]->act->status & ACT_STATUS_CAPTURE ) {
					SET_FLAG( unit->entk[ j ]->receive, ENE_ORDER_DIE_ERASE ) ;
					return ;
				}

			}
		}
	}
	
}


static void DebugMode( COMMANDER	*com )
{
#if 0
	int pad;
	int	p, l, n ;
	static	int	se=0x80 ;

	pad = GV_PadData[ 1 ].press;

	if( pad & PAD_R1 ){
		se ++ ;
		if ( se > 0x96 ) se = 0x80 ;
	}
	if( pad & PAD_L1 ){
		se -- ;
		if ( se < 0x80 ) se = 0x96 ;
	}
	if( pad & PAD_X ){
		p = 0x00;		l = 0xff;
		if ( l > 0x3f ) l = 0x3f ;
		n = se ;
		sd_set_cli( p << 16 | l << 8 | n ) ;

		printf( "se call 0x%x\n",n );
	}
#endif

#ifdef DEBUG_MODE
	if( GM_DebugModeEnable ){
		if ( GV_PadData[1].press & PAD_R2 && GV_PadData[1].status & PAD_X ){
			if ( com->status & CMST_ENEMY_SIGHT_OFF ) {
				com->status &= ~CMST_ENEMY_SIGHT_OFF ;
				printf("ENEMY EYE OPEN\n");
			} else {
				com->status |= CMST_ENEMY_SIGHT_OFF ;
				printf("ENEMY EYE CLOSE\n");
			}
		}
if ( GV_PadData[1].press & PAD_R2 && GV_PadData[1].press & PAD_R1 && GV_PadData[1].status & PAD_X ){
SET_FLAG( GM_VRStatus, GM_VR_ENEMY_END ) ;
}
		if ( GV_PadData[1].press & PAD_R1 && GV_PadData[1].status & PAD_X ){
			if ( com->status & CMST_ENEMY_BODY_LIGHT ) {
				com->status &= ~CMST_ENEMY_BODY_LIGHT ;
				printf("ENEMY LIGHT OFF\n");
			} else {
				com->status |= CMST_ENEMY_BODY_LIGHT ;
				printf("ENEMY LIGHT ON\n");
			}
		}
		if ( GV_PadData[1].press & PAD_L2 && GV_PadData[1].status & PAD_X ){
			if ( com->status & CMST_ENEMY_IK_STOP ) {
				com->status &= ~CMST_ENEMY_IK_STOP ;
				printf("ENEMY IK ON\n");
			} else {
				com->status |= CMST_ENEMY_IK_STOP ;
				printf("ENEMY IK OFF\n");
			}
		}
		if ( GV_PadData[1].press & PAD_AL && GV_PadData[1].status & PAD_X ){
			if ( com->status & CMST_ENEMY_GUN_LINE ) {
				com->status &= ~CMST_ENEMY_GUN_LINE ;
				printf("ENEMY GUN LINE ON\n");
			} else {
				com->status |= CMST_ENEMY_GUN_LINE ;
				printf("ENEMY GUN LINE OFF\n");
			}
		}
		if ( GV_PadData[1].press & PAD_AR && GV_PadData[1].status & PAD_X ){
			if ( com->status & CMST_ENEMY_SIGHT_VIEW ) {
				com->status &= ~CMST_ENEMY_SIGHT_VIEW ;
				printf("ENEMY SIGHT VIEW CLOSE\n");
			} else {
				com->status |= CMST_ENEMY_SIGHT_VIEW ;
				printf("ENEMY SIGHT VIEW OPEN\n");
			}
		}
		if ( GV_PadData[1].press & PAD_L1 && GV_PadData[1].status & PAD_X ){
			int	tmplight ;
			
			tmplight = com->status & (CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
			tmplight += CMST_ENEMY_TMP_LIGHT1 ;
			tmplight &= (CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
			com->status &= ~(CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
			com->status |= tmplight ;
			tmplight = com->status & (CMST_ENEMY_TMP_LIGHT1|CMST_ENEMY_TMP_LIGHT2) ;
			printf("TMP LIGHT MODE[%d] \n",tmplight >> 28 );
		}
		if ( GV_PadData[1].press & PAD_L1 && GV_PadData[1].press & PAD_L2 ){
			if ( com->status & CMST_ENEMY_GUNLIGHT_OFF ) {
				com->status &= ~CMST_ENEMY_GUNLIGHT_OFF ;
			} else {
				com->status |= CMST_ENEMY_GUNLIGHT_OFF ;
			}
		}

		if ( GV_PadData[1].press & PAD_R1 && GV_PadData[1].status & PAD_Y ){
			if ( com->status & CMST_ENEMY_GUN_CUT ) {
				com->status &= ~CMST_ENEMY_GUN_CUT ;
				printf("ENEMY GUN START\n");
			} else {
				com->status |= CMST_ENEMY_GUN_CUT ;
				printf("ENEMY GUN CUT\n");
			}
		}

	}
#endif

}


enum {
	MESSAGE_EYE_OFF=0,
	MESSAGE_EYE_ON,
	MESSAGE_WAITPOS_CHANGE,
	MESSAGE_CAUTION_CHANGE,
	MESSAGE_CAPTURE_KILL,	/* プレイヤーに捕まえられてるキャラを消す */
	MESSAGE_ENDLES_ALERT_ON,
	MESSAGE_ENDLES_ALERT_OFF,
	MESSAGE_ENDLES_SEARCH_ON,
	MESSAGE_ENDLES_SEARCH_OFF,
	MESSAGE_CLE_AREA_SKIP_ON,
	MESSAGE_CLE_AREA_SKIP_OFF,
	MESSAGE_CLE_AREA_ENECHK_SKIP_ON,
	MESSAGE_CLE_AREA_ENECHK_SKIP_OFF,
	MESSAGE_HEARING_OFF,
	MESSAGE_HEARING_ON,
	MESSAGE_ALERT_AND_PLAYERDETECT,
	MESSAGE_DESTROY_ALERT_ON,
	MESSAGE_DESTROY_ALERT_OFF,
	MESSAGE_PLAYERDETECT,
};

static	void	CheackMessage( COMMANDER *com )
{
#if 1
	GV_MSG *msg ;
	int mes_num ;

	mes_num=GV_ReceiveMessage( com->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
			case MESSAGE_EYE_OFF:
				SET_FLAG( com->status, CMST_ENEMY_SIGHT_OFF ) ;
				printf("MESSAGE ENEMY EYE CLOSE\n");
			break;
			case MESSAGE_EYE_ON:
				UNSET_FLAG( com->status, CMST_ENEMY_SIGHT_OFF ) ;
				printf("MESSAGE ENEMY EYE OPEN\n");
			break;

			case MESSAGE_WAITPOS_CHANGE :
				com->waiting_cur = (u_char)msg->message[ 1 ] ;
//				Comm.waiting_pos[ i ].vx = (float)msg->message[ 2 ] ;
//				Comm.waiting_pos[ i ].vy = (float)msg->message[ 3 ] ;
//				Comm.waiting_pos[ i ].vz = (float)msg->message[ 4 ] ;
//				Comm.wait_map[ i ] = GM_GetHzxGroupID( ( GM_GetMapID( msg->message[ 5 ] ) ) ) ;
//printf("change wait pos id[%d] wait[%d][%d][%d] map[%x]\n",
//		i, msg->message[ 2 ],msg->message[ 3 ],msg->message[ 4 ],Comm.wait_map[i] ) ;
			break ;

			case MESSAGE_CAUTION_CHANGE:
				StrmCurrentCaution = (u_char)msg->message[ 1 ] ;
				printf("MESSAGE CAUTION CHANGE[%d]\n", StrmCurrentCaution);
			break;
			case MESSAGE_CAPTURE_KILL:
				if( CP_CaptureCorpKill( ) == 0 ) {
					COM_CaptureKill( ) ;
				}
			break;
			case MESSAGE_ENDLES_ALERT_ON:
				SET_FLAG( com->status, CMST_ENEMY_ENDLES_ALERT ) ;
				printf("MESSAGE ENEMY ENDLESS ALERT\n");
			break;
			case MESSAGE_ENDLES_ALERT_OFF:
				UNSET_FLAG( com->status, CMST_ENEMY_ENDLES_ALERT ) ;
				printf("MESSAGE ENEMY ENDLESS ALERT\n");
			break;
			case MESSAGE_ENDLES_SEARCH_ON:
				SET_FLAG( com->status, CMST_ENEMY_ENDLES_SEARCH ) ;
				printf("MESSAGE ENEMY ENDLESS SEARCH ON\n");
			break;
			case MESSAGE_ENDLES_SEARCH_OFF:
				UNSET_FLAG( com->status, CMST_ENEMY_ENDLES_SEARCH ) ;
				printf("MESSAGE ENEMY ENDLESS ALERT OFF\n");
			break;
			case MESSAGE_DESTROY_ALERT_ON:
				SET_FLAG( com->status, CMST_ENEMY_DESTROY_ALERT ) ;
				printf("MESSAGE ENEMY DESTROY ALERT ON\n");
			break;
			case MESSAGE_DESTROY_ALERT_OFF:
				UNSET_FLAG( com->status, CMST_ENEMY_DESTROY_ALERT ) ;
				printf("MESSAGE ENEMY DESTROY ALERT OFF\n");
			break;
			case MESSAGE_CLE_AREA_SKIP_ON :
				SET_FLAG( com->cle_area_status[ msg->message[ 1 ] + COM_GetClearingOfset( ) ], CLE_AREA_ST_SKIP ) ;
			break;
			case MESSAGE_CLE_AREA_SKIP_OFF:
				UNSET_FLAG( com->cle_area_status[ msg->message[ 1 ] + COM_GetClearingOfset( ) ], CLE_AREA_ST_SKIP ) ;
			break;
			case MESSAGE_CLE_AREA_ENECHK_SKIP_ON :
				SET_FLAG( com->cle_area_status[ msg->message[ 1 ] + COM_GetClearingOfset( ) ], CLE_AREA_ST_ENECHK_SKIP ) ;
			break;
			case MESSAGE_CLE_AREA_ENECHK_SKIP_OFF:
				UNSET_FLAG( com->cle_area_status[ msg->message[ 1 ] + COM_GetClearingOfset( ) ], CLE_AREA_ST_ENECHK_SKIP ) ;
			break;
			case MESSAGE_HEARING_OFF:
				SET_FLAG( com->status, CMST_ENEMY_HEARING_OFF ) ;
				printf("MESSAGE ENEMY HEARING OFF\n");
			break;
			case MESSAGE_HEARING_ON:
				UNSET_FLAG( com->status, CMST_ENEMY_HEARING_OFF ) ;
				printf("MESSAGE ENEMY HEARING ON\n");
			break;
			case MESSAGE_ALERT_AND_PLAYERDETECT:
			/*重野追加 2002.06.04*/
			/*プレイヤを発見済みにする VRザコサバイバルなど用*/
				SET_FLAG( com->status, CMST_ENEMY_ENDLES_ALERT ) ;
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			break;
			case MESSAGE_PLAYERDETECT:
			/*重野追加 2002.06.19*/
			/*プレイヤを発見済みにする*/
				COM_SetAlertStatus(COM_ALERT_PLAYER_DETECT) ;
			break;
		}
		msg--;
	}
#endif
}


#define LISTEN_NOISE_ACT_STATUS	(ACT_STATUS_DAMAGE|ACT_STATUS_UNREAL|ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP)

static void NoiseNearEnemy( com )
COMMANDER	*com ;
{
	int i, j,k, dis, min, hearing ;
	E_GROUP		*group ;
	E_UNIT		*unit ;
	ENETHINK	*entk ;

	com->noise_near_id[0] = -1 ;

	min = 1000000 ;
	/*sigeno追加 攻撃兵もループさせる*/
	for(k=0;k<com->enemys.group_num;k++){
		group = com->enemys.group[ k ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[ i ] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( (entk = unit->entk[ j ]) == NULL ) continue ;
				if ( entk->act->bodyp.type & ENE_TYPES_EAR_NARROW ) {
					if ( DG_FABS( entk->ctrl->mov.vy - GM_NoisePosition.vy ) > 2000.0f ) {
						continue ;
					}
				}

				if ( !(entk->act->status & LISTEN_NOISE_ACT_STATUS) ) {
					if ( !(entk->status & ENE_STATUS_EVER_UNREAL) ) {
						if ( KR_MapConnect( GM_NoiseMap, entk->ctrl->map ) ) {
							dis = _FVecTrgDis( &entk->ctrl->mov, &GM_NoisePosition ) ;
							if ( min > dis ) {
								min = dis ;

								hearing = entk->sense.hearing ;
								if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
									hearing = ( entk->status & ENE_STATUS_NORINORI ) ? 
											NORINORI_HEARING : entk->sense.hearing ;
								} 

								if ( com->noise == NOISE_MM ) hearing = (hearing*2)/1 ;
								if ( dis < hearing ) {
#if 0//サポートしている兵も反応するようにした
									if ( entk->thk_status & (THK_STATUS_CLE_SUPPORT) ) {
										if ( com->noise_near_id[0] >= 0 ) continue ;
									}
#endif
									com->noise_near_id[0] = k ;
									com->noise_near_id[1] = i ;
									com->noise_near_id[2] = j ;
									com->noise_dis = dis ;
printf ( " noize !! gp[%d] un[%d] en[%d] \n",k,i,j ) ;
								}
							}
						}
					}
				}
			}
		}
	}
}

static void PlayerPositionInZone( com )
COMMANDER	*com ;
{
	HZX_ZON		*zone ;
	FVECTOR		pos ;
	int num ;
	int	zone1, zone2, zoneadd ;
	extern FVECTOR GM_PlayerPosition ;
	
	zone1 = 255 & GM_PlayerAddress ;
	zone2 = 255 & ( GM_PlayerAddress >> 8 ) ;
	num = 0 ;

	com->pl_pos_num = 1 ;
	if ( zone1 == zone2 && zone1 != 255 && !(GM_PlayerStatus & PLAYER_FORCE) ) { 
		com->plzone_in_zone[ num ] = GM_PlayerAddress ;
		com->plmap_in_zone[ num ] = GM_PlayerHzxID ;
		com->plpos_in_zone[ num ] = GM_PlayerPosition ;
//printf("c[%f][%f] ",GM_PlayerPosition.vx,GM_PlayerPosition.vz) ;
//printf(" [%f][%f]\n",com->plpos_in_zone[ num ].vx,com->plpos_in_zone[ num ].vz) ;

	}

	/* プレイヤーがゾーン内にいるかチェック */
	if ( zone1 == 255 || zone2 == 255 ) {
		com->pl_in_zone[ num ] = 0 ;
	} else {
		com->pl_in_zone[ num ] = 1 ;
	}

#if 0
	if ( GM_PlayerStatus & PLAYER_BEYOND ) 
#else
	if (
	( GM_PlayerStatus & PLAYER_BEYOND ) 
	&&(!( GM_PlayerStatus & PLAYER_FORCE ) )
	)
#endif
	{	/* ぶら下がり */
		
		pos = GM_PlayerPosition ;
		pos.vy += 1000 ;	/* 手の高さ */
		zoneadd = HZX_GetAddress( GM_PlayerHzxID, &pos, -1 ) ;

		com->pl_pos_num = 1 ;
		num = PL_POS_CENTER ;
		zone = HZX_GetZone( GM_PlayerHzxID, (255&zoneadd) ) ;
		com->plpos_in_zone[ num ].vx = (float)zone->x ;
		com->plpos_in_zone[ num ].vy = (float)zone->y ;
		com->plpos_in_zone[ num ].vz = (float)zone->z ;
		com->plzone_in_zone[ num ] = zoneadd ;
		com->plmap_in_zone[ num ] = GM_PlayerHzxID ;

#if 0
		pos.vy -= 2000 ;	/* 足の高さ */
		zoneadd = HZX_GetAddress( GM_PlayerHzxID, &pos, -1 ) ) ;

		com->pl_pos_num = 2 ;
		num = 1 ;
		zone = HZX_GetZone( GM_PlayerHzxID, (255&zoneadd) ) ;
		com->plpos_in_zone[ num ].vx = (float)zone->x ;
		com->plpos_in_zone[ num ].vy = (float)zone->y ;
		com->plpos_in_zone[ num ].vz = (float)zone->z ;
		com->plzone_in_zone[ num ] = zoneadd ;
		com->plmap_in_zone[ num ] = GM_PlayerHzxID ;
#endif
	}
}

int COM_EnemyActStatusNum( long64 status )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;

	com = &Comm ;
	num = 0 ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->act->status & status ) {
					num ++ ;
				}
			}
		}
	}

	return num ;
}

ENETHINK *COM_EnemyActStatus( long64 status )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;

	com = &Comm ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->act->status & status ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

int COM_EnemyActNotStatusNum( long64 status )
{
	COMMANDER	*com ;
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;

	com = &Comm ;
	num = 0 ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( !(entk->act->status & status) ) {
					num ++ ;
				}
			}
		}
	}

	return num ;
}

/* ノイズLがなった時点で警備している兵がいたら数フレーム後危険モード */
#define	NOISE_DAM_STATE	(ACT_STATUS_FAINT|ACT_STATUS_DEATH|ACT_STATUS_UNREAL)
#define	NOISE_ALERT_DELAY_TIME	(COUNT_VMODE(60))
static void SetNoiseLAlertTimer( com )
COMMANDER	*com ;
{

	if ( COM_EnemyActNotStatusNum( NOISE_DAM_STATE ) ) {
		if ( !(com->alertstatus & (COM_ALERT_PLAYER_DETECT|COM_ALERT_NPC_DETECT)) ) {
			com->player_lastpos = GM_NoisePosition;
			com->player_lastmap = GM_NoiseHzxID ;
		}

		if ( com->noise_alert_timer != 0 ) return ; /* 既にタイマー発動中 */

		com->noise_alert_timer = NOISE_ALERT_DELAY_TIME ;
		com->noise_time ++ ;
		if ( com->noise_time > ALERT_NOISE_TIME_DETECT_PLAYER ) {
			COM_SetAlertStatus( COM_ALERT_PLAYER_DETECT ) ;
		}

		if ( !(com->stage_kind & ENE_STAGE_NO_WC_GOVER) ) {
//			if ( GM_GameLevel == GM_LEVEL_EXTREME ) {
			if ( GM_Configuration & GM_CONFIG_END_IF_FOUND ) {
				SET_FLAG ( com->status, CMST_ENEMY_GAME_OVER ) ;
			}
		}
	}
}
static void NoiseTimer( com )
COMMANDER	*com ;
{
	if ( com->noise_alert_timer > 0 ) {
		if( GM_AlertMode == ALERT_MODE_ALERT ) {
			com->noise_alert_timer = 1 ;
		}

		if ( com->noise_alert_timer == 1 ) {
			COM_SetPureSecurityAlertLevel( ALERT_LEVEL_MAX ) ;
		}
		com->noise_alert_timer -- ;
	}
}

static void NoiseCheck( com )
COMMANDER	*com ;
{

	com->noise = GM_NoisePower ;
	com->noise_dis = 0 ;

	if( com->status & CMST_ENEMY_HEARING_OFF || (com->stage_kind & ENE_STAGE_NO_NOTICE) ) {
		com->noise = 0 ;
	} else {
		switch( com->noise ) {
			case NOISE_S :
			case NOISE_SS :
			case NOISE_MM :
				NoiseNearEnemy( com ) ;
				break ;
			case NOISE_DEC :
			case NOISE_HOLD :
			case NOISE_MIC_QUEST :
				break ;
			case NOISE_SCREAM :
			case NOISE_M :
				break ;
			case NOISE_L :
				SetNoiseLAlertTimer( com ) ;
				break ;
			default :
				com->noise = 0 ;
				break ;
		}
	}

	NoiseTimer( com ) ;
}

static void Investigated( com )
COMMANDER	*com ;
{
	com->investigated = 0 ;
	com->uniqid_near_investigated = -1 ;
}

#if 1 //01.8.22 ACT_STATUS_FAINT 追加何故今までなかったか謎 要注意！！

#define	DAM_STATE	(ACT_STATUS_DAM_DIR|ACT_STATUS_DAMAGE|ACT_STATUS_DOWN| \
			ACT_STATUS_MASUI_SASARU|ACT_STATUS_HOLD_UP|ACT_STATUS_FAINT)

#else
#define	DAM_STATE	(ACT_STATUS_DAM_DIR|ACT_STATUS_DAMAGE|ACT_STATUS_DOWN| \
			ACT_STATUS_MASUI_SASARU|ACT_STATUS_HOLD_UP)
#endif
static void StateCheck( com )
COMMANDER	*com ;
{
	int 		g, u, i ;
	E_GROUP		*group ;
	E_UNIT		*unit ;
	ENETHINK	*entk ;

	com->live_num = 0 ;
	com->dam_en_num = 0 ;

	GM_ResetGameStatus( STATE_ENE_SIGHTIN ) ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( u=0; u<group->unit_num; u++ ) {
			unit = group->unit[ u ] ;
			for ( i=0; i<unit->enemy_num; i++ ) {
				if ( (entk = unit->entk[ i ]) == NULL ) continue ;
				/* 生存者確認 */
				if ( !(entk->act->status & ACT_STATUS_GHOST) ) {
					com->live_num ++ ;
				}
				/* ダメージ者確認 */
				if ( com->dam_en_num < MAX_DAM_ENE ) {
					if ( entk->act->status & DAM_STATE ||
						(entk->iknow_flag & IKNOW_HELP_ME) ) {
						com->dam_entk[ com->dam_en_num ] = entk ;
						com->dam_en_num ++ ;
//printf("damage!! damage!![%d][%d][%d] > [%d]\n",g,u,i,com->dam_en_num) ;
					}
				}
				
				if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
					GM_SetGameStatus( STATE_ENE_SIGHTIN ) ;
				}

			}
		}
	}

	/* １フレームフラグチェック */
	if ( com->flame_flag & CMFLAG_DETECT ) {
		SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_DETECT ) ;

		if ( !(com->stage_kind & ENE_STAGE_NO_WC_GOVER) ) {
//			if ( GM_GameLevel == GM_LEVEL_EXTREME ) {
			if ( GM_Configuration & GM_CONFIG_END_IF_FOUND ) {
				SET_FLAG ( com->status, CMST_ENEMY_GAME_OVER ) ;
			}
		}

		if ( !GM_CheckGameStatus( GM_STATUS_DETECT ) ) {
			GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/
			GM_SetGameStatus( GM_STATUS_DETECT ) ;
			GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
		}
	} else {
		GM_ResetGameStatus( GM_STATUS_DETECT ) ;
	}
	if ( com->flame_flag & CMFLAG_BIG_SNORE ) {
		GM_SetGameStatus( STATE_BIG_SNORE ) ;
	} else {
		GM_ResetGameStatus( STATE_BIG_SNORE ) ;
	}

	if ( !(com->stage_kind & ENE_STAGE_NO_WC_GOVER) ) {
		if ( GM_Configuration & GM_CONFIG_END_IF_FOUND ) {
			if ( com->status & CMST_ENEMY_GAME_OVER ) {

				if ( gameover_delay_count == 0 ) {
printf("command detecttttttttttttttttttttttttttttttttt\n");
					GM_GameOverProcStart( NULL ) ;
				}
				if ( gameover_delay_count == 4 ) {
					if ( !(ENE_GameStatus&ENE_GMSTATUS_CALL_BIKKURI) ) {
printf("Commander detect SE Call \n");
						GM_SeSet( GM_PAN_CENTER , 0x20, SD_E_BIKKRI01 ) ;
					}
				}
				if ( gameover_delay_count == GAME_OVER_DELAY ) {
					GM_GameOverProcEnd( NULL ) ;
				}
				gameover_delay_count ++ ;
			}
		}
	}
}

static void EnemyCount( com )
COMMANDER	*com ;
{
	int	n_corp_num, o_corp_num ;
	
	n_corp_num = CP_NewCorpNum( ) ;
	o_corp_num = CP_OldCorpNum( ) ;

	com->enemy_count = n_corp_num + o_corp_num + com->live_num ;
}

static void SetAlertLevel( com )
COMMANDER	*com ;
{
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int i, j, g ;

	com->maxalert = 0 ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;
				if ( com->maxalert < unit->entk[ j ]->alert ) {
					com->maxalert = unit->entk[ j ]->alert ;
				}
			}
		}
	}

	if ( com->maxalert < com->security_alert ) {
		com->maxalert = com->security_alert ;
	}
	com->security_alert = 0 ;

	com->alertlevel -= 16 ;

	if( com->alertlevel < 0 ){
		com->alertlevel = 0 ;
	}
	if( com->alertlevel < com->maxalert ){
		com->alertlevel = com->maxalert ;
	}

	if( com->status & CMST_ENEMY_ENDLES_ALERT ) {
		com->alertlevel = ALERT_LEVEL_MAX ;
	}
	if( com->status & CMST_ENEMY_DESTROY_ALERT ) {
		if ( (Comm.res_count_in_alert < Comm.max_res_in_alert) ||
			 (com->live_num) ) {	/* 条件１が偽になり復活するまではすうフレームかかるがそれまではもつはず*/
			com->alertlevel = ALERT_LEVEL_MAX ;
		}
	}

	GM_AlertLevel = com->alertlevel ;
}

static void SetAvoidLevel( com )
COMMANDER	*com ;
{
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int i, j, g ;

	com->maxavoid = 0 ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;
				if ( com->maxavoid < unit->entk[ j ]->avoid ) {
					com->maxavoid = unit->entk[ j ]->avoid ;
				}
			}
		}
	}
	com->avoidlevel = com->maxavoid ;
	if ( com->avoidlevel < 0 ) com->avoidlevel = 0 ;

#if 0
if( GM_AlertMode == ALERT_MODE_AVOID ) {
	printf("com->avoidlevel = %d\n",com->avoidlevel);
}
#endif


}

static void SetSearchLevel( com )
COMMANDER	*com ;
{
	GM_CautionLevel = com->searchlevel ;
}

static void VrStateCheck( com )
COMMANDER	*com ;
{
	if ( com->live_num == 0 ) {	/* 生存者がいなかったら */
		SET_FLAG( GM_VRStatus, GM_VR_ENEMY_END ) ;
	}
	if ( GM_VRStatus & (GM_VR_GAME_OVER) ) {
		if( !(com->status & CMST_ENEMY_VR_GAMEOVER) ) {
			SET_FLAG( com->status, CMST_ENEMY_VR_GAMEOVER ) ;
			GM_JimakuHide( ) ;/* 字幕消す */
			GM_StreamStop( StrmCurrentHandler ) ;/* 字幕消す */
		}
	}
}
/*----------------------------------------------------------------------------*/
static void ChangeAlertToAvoid( COMMANDER	*com )
{
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int i, j, g ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;
				CLEAR_FLAG( unit->entk[ j ]->iknow_flag ) ;
			}
		}
	}
}


#define NO_LIVE_STATUS	(ACT_STATUS_UNREAL|ACT_STATUS_FAINT|ACT_STATUS_DEATH)
static void CheckAlertStatus( com )
COMMANDER	*com ;
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i ;
	int	wc_num, sp_num, at_num ;

	wc_num = 0 ;
	sp_num = 0 ;
	at_num = 0 ;

	if ( (com->res_count < com->max_res_num) &&
		 (com->res_count_in_alert < com->max_res_in_alert) ) return ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->status & ENE_STATUS_EVER_UNREAL ) continue ;
				if ( !(entk->act->status & NO_LIVE_STATUS ) ) {
					if ( entk->act->bodyp.type & ENE_TYPE_ATTACKER ) {
						at_num ++ ;
					} else if ( entk->act->bodyp.type & ENE_TYPE_WATCHER ) {
						wc_num ++ ;
					} else {
						sp_num ++ ;
					}
				}
			}
		}
	}

	if ( at_num == 0 ) {
		COM_SetAlertStatus( COM_ALERT_ATK_TOTAL_DEFEAT ) ;
	} else {
//		ASSERT( !(COM_AlertStatus( ) & COM_ALERT_ATK_TOTAL_DEFEAT) ) 
	}
	if ( wc_num == 0 ) {
		COM_SetAlertStatus( COM_ALERT_WTC_TOTAL_DEFEAT ) ;
	} else {
//		ASSERT( !(COM_AlertStatus( ) & COM_ALERT_WTC_TOTAL_DEFEAT) ) 
	}
	if ( sp_num == 0 ) {
		COM_SetAlertStatus( COM_ALERT_SUP_TOTAL_DEFEAT ) ;
	} else {
//		ASSERT( !(COM_AlertStatus( ) & COM_ALERT_SUP_TOTAL_DEFEAT) ) 
	}
}

static void DefenseMode( com )
COMMANDER	*com ;
{
	int		area ;

	CM_TMPSNAKEAREA = -1 ;
	switch( GM_AlertMode ){
		case	ALERT_MODE_SNEAK:		/* 巡回モード */
			if ( Comm.searchlevel > 0 ) {
				GM_AlertMode = ALERT_MODE_SEARCH ;
				if( !(com->alertlevel >= MAX_ALERT_LEVEL) ){
					SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_SNEAK2CAUTUION ) ;
				}
			}
			if ( !GM_CheckGameStatus( STATE_GAMEOVER ) ) {
				if( com->alertlevel >= MAX_ALERT_LEVEL ){
					extern	void KillEnemyView( void ) ;

					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/
					com->alertlevel = MAX_ALERT_LEVEL ;
					GM_AlertMode = ALERT_MODE_ALERT ;
					SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ALERT ) ;
					com->atk_goalzone = GM_PlayerAddress ;
					SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_PLAYER_FOUND ) ;
					KillEnemyView() ;	/* 子画面出ていたら消去 */
					com->res_count_in_alert = 0;
					com->cyp_res_count_in_alert = 0;
					COM_ClearAccident( ) ;
					KR_AlertCount() ;
					COM_DiscoverCount() ;
					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
				}
			}
		break;

		case	ALERT_MODE_ALERT:			/* 危険モード */
//			if ( (area = ENE_CLBoundCheck( &com->plpos_in_zone[PL_POS_CENTER] )) >= 0) {
			if ( (area = ENE_CLBoundCheck( &com->player_lastpos )) >= 0) {
				CM_TMPSNAKEAREA = area ;
			}
			if( com->alertlevel <= 0 ){
				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/
				com->atk_goalzone = GM_PlayerAddress ;
				ChangeAlertToAvoid( com ) ;
				com->alertstatus = 0 ;
				com->avoidstatus = 0 ;
//				ENE_AlertGameLevel = 0 ;
				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/

				if ( com->stage_kind & ENE_STAGE_ALERT2SNEAK ) {
					GM_AlertMode = ALERT_MODE_SNEAK ;
					com->noise_time = 0 ;
				} else {
					com->time = AVOID_TIME ;
					GM_AlertMode = ALERT_MODE_AVOID ;
				}

				return ;
			}
			if ( com->alertstatus & COM_ALERT_PLAYER_DETECT ) {
				com->player_lastpos = com->plpos_in_zone[PL_POS_CENTER] ;
				com->player_lastmap = com->plmap_in_zone[PL_POS_CENTER] ;
			} else if ( com->alertstatus & COM_ALERT_NPC_DETECT ) {
				com->player_lastpos = com->npc_ctrl->mov ;
				com->player_lastmap = com->npc_ctrl->hzx_id ;
			}
			CheckAlertStatus( com ) ;
		break;

		case	ALERT_MODE_AVOID :			/* 回避モード */
			if ( (GM_GameStatus & STATE_CLEARING ) ) {
				SET_FLAG( com->avoidstatus , COM_AVOID_CLEARING )  ;
			}
			if ( com->time <= 0 ) {
				com->time = 0 ;
				if( com->avoidlevel <= 0 && !(GM_GameStatus & GM_STATUS_DETECT) ){
					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/
					com->avoidlevel = 0 ;
					com->noise_time = 0 ;
					GM_AlertMode = ALERT_MODE_SEARCH ;
					com->searchlevel = DEF_SEARCH_LEVEL ;
					COM_CleProc( -2, -2, -2 ) ;	/* 回避モード終了プロック */

					/* 警戒モードに移るよ命令 */
					COM_SetRadio( EV_RAD_ATK_RETURN, NULL ) ;
					UNSET_FLAG( GM_GameStatus, STATE_CLEARING ) ;

					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
				}
			}
			if( com->alertlevel >= MAX_ALERT_LEVEL ){
				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/

				com->alertlevel = MAX_ALERT_LEVEL ;
				GM_AlertMode = ALERT_MODE_ALERT ;
				SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ALERT ) ;
				com->atk_goalzone = GM_PlayerAddress ;
				com->res_count_in_alert = 0;
				com->cyp_res_count_in_alert = 0;
				COM_CleProc( -2, -2, -2 ) ;	/* 回避モード終了プロック */
				UNSET_FLAG( GM_GameStatus, STATE_CLEARING ) ;
				COM_DiscoverCount() ;

				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
			}
			com->time -- ;
		break;

		case	ALERT_MODE_SEARCH:		/* 探索モード */
#if 1
			if( com->searchlevel <= 0 && !(GM_GameStatus & GM_STATUS_DETECT) ){
#else
			if( com->searchlevel <= 0 ){
#endif
				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/

				GM_AlertMode = ALERT_MODE_SNEAK ;
				com->searchlevel = 0 ;
				/* 潜入モードに移るよ命令 */
				COM_SetRadio( EV_RAD_GHQ_RETURN_SNEAK, NULL ) ;

				GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
			}
			if ( !GM_CheckGameStatus( STATE_GAMEOVER ) ) {
				if( com->alertlevel >= MAX_ALERT_LEVEL ){
					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_QUIT ) ;/*旧アラートモード終了時*/

					com->alertlevel = MAX_ALERT_LEVEL ;
					GM_AlertMode = ALERT_MODE_ALERT ;
					SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ALERT ) ;
					com->atk_goalzone = GM_PlayerAddress ;
					com->res_count_in_alert = 0;
					com->cyp_res_count_in_alert = 0;
					COM_ClearAccident( ) ;
					KR_AlertCount() ;
					COM_DiscoverCount() ;

					GM_CallCallbackProc( GM_CALLBACK_ALERT_MODE_ENTER ) ;/*新アラートモード開始時*/
				}
			}
			if( !(com->status & CMST_ENEMY_ENDLES_SEARCH) ) {
				Comm.searchlevel -- ;
			}
			if ( Comm.searchlevel < 0 ) Comm.searchlevel = 0 ;
		break;
	}
}


static void DebugViewer( com )
COMMANDER	*com ;
{
	if ( !(GM_PlayerStatus & PLAYER_DEBUG) ) {
		MENU_Locate( 170 + 72 , 4, 0 );
		switch( GM_AlertMode ) {
			case	ALERT_MODE_SNEAK:
			break ;
			case	ALERT_MODE_ALERT:
				{
					int	zone_dis ;
					ENETHINK *entk ;
					
					if ( Comm.enemys.group_num > 1 ) {/* 攻撃兵と決め付ける */
//						entk = Comm.enemys.group[ 1 ]->unit[ 0 ]->entk[ 0 ] ;
//						zone_dis = HZX_GetRouteCrossGroup( entk->ctrl->addr, com->atk_goalzone ) ;
if(0)					MENU_Printf( "-DISTANCE[%d]--", zone_dis );
					}
				}
			break ;
			case	ALERT_MODE_AVOID:
if(0)				MENU_Printf( "--AVOID[%d]--\n", com->avoidlevel );
if(0)				MENU_Printf( "--TIME [%d]--\n", com->time );

				{
					int	zone_dis ;
					ENETHINK *entk ;
					
					if ( Comm.enemys.group_num > 1 ) {
//						entk = Comm.enemys.group[ 1 ]->unit[ 0 ]->entk[ 0 ] ;
//						zone_dis = HZX_GetRouteCrossGroup( entk->ctrl->addr, com->atk_goalzone ) ;
if(0)						MENU_Printf( "-DISTANCE[%d]--", zone_dis );
					}
				}
			break ;
			case	ALERT_MODE_SEARCH:
if(0)				MENU_Printf( "--SEARCH[%4d]--", com->searchlevel );
			break ;
		}
	}
}

/* 振動 */
static	u_char	DamageVib1L[] = { 176, 4, 0, 0 } ; /* by nakamura */
static	u_char	DamageVib1LLL[] = { 255, 6, 0, 0 } ; /* by nakamura */
static	u_char	DamageVib2H[] = { 1, 20, 0, 0 } ;
static	u_char	DamageVib2L[] = { 255, 20, 0, 0 } ;

//static	u_char	DamageVibHangH[] = { 1, 4, 0, 0 } ;
static	u_char	DamageVibHangL[] = { 255, 8, 0, 0 } ;

#define NOT_VIB_STATUS (ACT_STATUS_DEATH|ACT_STATUS_DOWN|ACT_STATUS_FAINT|ACT_STATUS_DOWN| \
						ACT_STATUS_UNREAL|ACT_STATUS_CAPTURE )
static int old_vib_time=0 ;


void COM_DetectVibration( void )
{
	extern void	*NewPadVibration( char *script, int type ) ;

	NewPadVibration( DamageVib2H, 1 ) ;
	NewPadVibration( DamageVib2L, 2 ) ;
	/*びっくり音がなったものとする*/
	SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_CALL_BIKKURI) ;
}

void COM_HangVibration( void )
{
	extern void	*NewPadVibration( char *script, int type ) ;
//	NewPadVibration( DamageVibHangH, 1 ) ;
	NewPadVibration( DamageVibHangL, 2 ) ;
}

static int vib_se = 0 ;
static void Vibration( com )
COMMANDER	*com ;
{
	extern	void *NewPadVibration( char *script, int type ) ;
	int dis, vol ;
	ENETHINK *out ;

	if ( GM_AlertMode == ALERT_MODE_ALERT ) return ;
	if ( (GM_GameStatus & GM_STATUS_DETECT) ) return ;
	if ( GM_VRStatus & GM_VR_IDLE ) return ;

	out = COM_NearEnemyNotActStatus( &GM_PlayerPosition, NOT_VIB_STATUS, &dis ) ;

#if 0
/* */
	if ( dis < 2000 || (GM_GameStatus & GM_STATUS_DETECT) ) {
		if ( ENE_ABS(GV_Time-old_vib_time) > 16 ) {
			NewPadVibration( DamageVib1L, 2 ) ;
			old_vib_time = GV_Time ;
		}
	} else if ( dis < 4000 ) {
		if ( ENE_ABS(GV_Time-old_vib_time) > 32 ) {
			NewPadVibration( DamageVib1L, 2 ) ;
			old_vib_time = GV_Time ;
		}
	} else if ( dis < 6000 ) {
		if ( ENE_ABS(GV_Time-old_vib_time) > 48 ) {
			NewPadVibration( DamageVib1L, 2 ) ;
			old_vib_time = GV_Time ;
		}
	} else if ( dis < 9000 ) {
		if ( ENE_ABS(GV_Time-old_vib_time) > 64 ) {
			NewPadVibration( DamageVib1L, 2 ) ;
			old_vib_time = GV_Time ;
		}
	}
#else
/* 滑らかに変化  */
//	if ( dis < 2000 || (GM_GameStatus & GM_STATUS_DETECT) ) {

	if( PL_GetPlayerItem() == IT_VibSensor ) {
		if ( com->stage_kind & ENE_STAGE_CROWD_PEOPLE ) {
			if ( ENE_ABS(GV_Time-old_vib_time) > COUNT_VMODE(16) ) {
#if defined(BP_VITA)
				NewPadVibration( DamageVib1LLL, VIBRATION_AP_SENSOR|VAR_FLAG_FORCE ) ;
#else
            NewPadVibration( DamageVib1LLL, 2|VAR_FLAG_FORCE ) ;
#endif
				old_vib_time = GV_Time ;
				if ( GM_PlayerStatus & (PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
					if ( vib_se ) {
						GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_P_HEARTB01 ) ;
						vib_se = 0 ;
					} else {
						vib_se = 1 ;
					}
				}
			}
		} else {
			if ( dis < 2000 ) {
				if ( ENE_ABS(GV_Time-old_vib_time) > COUNT_VMODE(16) ) {
#if defined(BP_VITA)
               NewPadVibration( DamageVib1LLL, VIBRATION_AP_SENSOR|VAR_FLAG_FORCE ) ;
#else
               NewPadVibration( DamageVib1LLL, 2|VAR_FLAG_FORCE ) ;
#endif
					old_vib_time = GV_Time ;
					if ( GM_PlayerStatus & (PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
						if ( vib_se ) {
							GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_P_HEARTB01 ) ;
							vib_se = 0 ;
						} else {
							vib_se = 1 ;
						}
					}
				}
//			} else if ( dis < 6000 ) {
			} else if ( dis < 10000 ) {
				int diff, b ;

				diff = dis - 2000 ;
				b = COUNT_VMODE(16 + (diff / 125)) ;

//				vol = GM_MAX_VOL/2 + ((GM_MAX_VOL*diff) / 8000) ;
				vol = GM_MAX_VOL/2 + ((GM_MAX_VOL*diff) / 16000) ;
				if ( ENE_ABS(GV_Time-old_vib_time) > b ) {
#if defined(BP_VITA)
               NewPadVibration( DamageVib1L, VIBRATION_AP_SENSOR | VAR_FLAG_FORCE);
#else
					NewPadVibration( DamageVib1L, 2|VAR_FLAG_FORCE ) ;
#endif
					old_vib_time = GV_Time ;
					if ( GM_PlayerStatus & (PLAYER_BEHIND|PLAYER_INTRUDE|PLAYER_WATCH) ) {
						if ( vib_se ) {
							GM_SeSet( GM_PAN_CENTER, vol, SD_P_HEARTB01 ) ;
							vib_se = 0 ;
						} else {
							vib_se = 1 ;
						}
					}
		//printf(" vibration blank [%d] \n",b);
				}
			}
		}
	}
#endif
}


//#define ENE_NOT_FINE	(ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP|ACT_STATUS_DAMAGE|ACT_STATUS_DOWN )
#define ENE_NOT_FINE	(ACT_STATUS_FAINT|ACT_STATUS_HOLD_UP )
static int EnemyFine( int uniq_id ) 
{
	ENETHINK	*entk ;

	entk = COM_GetEnemyFromUniqID( uniq_id ) ;

	if ( entk == NULL ) return 0 ;
	if ( entk->act->status & ENE_NOT_FINE ) return 0 ;

	return 1 ;
}

int	COM_FineAttacker( )
{
	COMMANDER	*com ;
	E_GROUP	*group ;
	E_UNIT	*unit ;
	int 	g, i, j ;

	com = &Comm ;
	for ( g=0; g<com->enemys.group_num; g++ ) {
		group = com->enemys.group[ g ] ;
		for ( i=0; i<group->unit_num; i++ ) {
			unit = group->unit[i] ;
			for ( j=0; j<unit->enemy_num; j++ ) {
				if ( unit->entk[ j ] == NULL ) continue ;
				if ( ENE_PureAttacker( unit->entk[ j ]->act ) ) {
					if ( unit->entk[ j ]->act->status & ACT_STATUS_FAINT ){
						if ( unit->entk[ j ]->act->headmark2 != ACT_HEADMARK2_ANES ) {
							return 1 ;
						}
					} else {
						return 1 ;
					}
				}
			}
		}
	}
	
	return 0 ;
}

static void Accident( com )
COMMANDER	*com ;
{
	int i ;

	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if ( com->accident_delay[i] > 0 ) {
			if ( com->accident_status[i] & ENE_ACCIDENT_REPORT ) {
				if ( com->accident_delay[i] == COUNT_VMODE(900) ) {
					if ( EnemyFine( com->accident_uniq_id[i] ) ) {
						ENETHINK	*trgentk ;
						trgentk = COM_GetEnemyFromUniqID( com->accident_uniq_id[i] ) ;
						com->accident_delay[i] = 0 ;	/* 心配中止 */
printf("1kroekroekroekroker sinpai chusi--------------\n");
					} else {
						COM_SetRadio( EV_RAD_GHQ_QUESTION1, NULL ) ;
					}
				}
				if ( com->accident_delay[i] == COUNT_VMODE(600) ) {
					if ( EnemyFine( com->accident_uniq_id[i] ) ) {
						ENETHINK	*trgentk ;
						trgentk = COM_GetEnemyFromUniqID( com->accident_uniq_id[i] ) ;
						com->accident_delay[i] = 0 ;	/* 心配中止 */
printf("2kroekroekroekroker sinpai chusi--------------\n");
					} else {
						ENETHINK	*trgentk ;
						trgentk = COM_GetEnemyFromUniqID( com->accident_uniq_id[i] ) ;
						if ( (trgentk !=NULL) &&	/*生きていて*/
							(trgentk->status & ENE_STATUS_TALK_SLEEP) && 
							(trgentk->act->status_status & (ACT_STST_ZZZ)) &&
							!(trgentk->sw.radio & SW_FLAG_BREAK ) ) {
							SET_FLAG( trgentk->receive, ENE_ORDER_CALL_COMM ) ;
							com->accident_delay[i] = 0 ;	/* 心配中止 */
						} else {
							COM_SetRadio( EV_RAD_GHQ_QUESTION5, NULL ) ;
						}
					}
				}

				if ( com->accident_delay[i] == COUNT_VMODE(300) ) {
					COM_SetRadio( EV_RAD_GHQ_CONFIRM, NULL ) ;
				}

				if ( com->accident_delay[i] == 1 ) {
					com->accident_bit |= 1<<i ;
				}
			} else if ( com->accident_status[i] & ENE_ACCIDENT_DIRECT ) {
				if ( com->accident_delay[i] == COUNT_VMODE(300) ) {
					COM_SetRadio( EV_RAD_GHQ_INVETIGATED, NULL ) ;
				}

				if ( com->accident_delay[i] == 1 ) {
					com->accident_bit |= 1<<i ;
				}
			} else if ( com->accident_status[i] & ENE_ACCIDENT_POS ) {
				/* カメラ系が場所の異常を発見 */
				if ( com->accident_delay[i] == COUNT_VMODE(300) ) {
					COM_SetRadio( EV_RAD_GHQ_MECA_CONFIRM, NULL ) ;
				}

				if ( com->accident_delay[i] == 1 ) {
					com->accident_bit |= 1<<i ;
				}
			} else if ( com->accident_status[i] & ENE_ACCIDENT_MECA_ENEDOWN ) {
				/* カメラ系が敵兵の異常を発見 */
				if ( com->accident_delay[i] == COUNT_VMODE(600) ) {
					COM_SetRadio( EV_RAD_GHQ_QUESTION5, NULL ) ;
				}

				if ( com->accident_delay[i] == COUNT_VMODE(300) ) {
					COM_SetRadio( EV_RAD_GHQ_MECA_CONFIRM, NULL ) ;
				}

				if ( com->accident_delay[i] == 1 ) {
					com->accident_bit |= 1<<i ;
				}

//			} else if ( com->accident_status[i] & ENE_ACCIDENT_RADIO ) {
			} else {
				if ( com->accident_delay[i] == COUNT_VMODE(600) ) {
					COM_SetRadio( EV_RAD_GHQ_QUESTION5, NULL ) ;
				}

				if ( com->accident_delay[i] == COUNT_VMODE(300) ) {
					if ( COM_FineAttacker( ) ) {
						COM_SetRadio( EV_RAD_GHQ_CONFIRM, NULL ) ;
					} else {
						COM_SetRadio( EV_RAD_GHQ_CONFIRM2, NULL ) ;
					}
				}

				if ( com->accident_delay[i] == 1 ) {
					com->accident_bit |= 1<<i ;
	//				COM_SetRadio( EV_RAD_GHQ_QUESTION5, NULL ) ;
				}
			}

			com->accident_delay[i]-- ;
		}
	}

	if ( com->accident_bit ) {
		if ( !COM_FineAttacker( ) ) {
#if 1	/* 攻撃兵が寝てもアクシデント忘れない */
#else	/* 攻撃兵が全員寝たらアクシデント忘れる */
			COM_ClearAccident( ) ;
#endif
		}
	}

	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		if ( (com->free_accident_bit>>i) & 1 ) {
			ENETHINK *trgentk ;
			NEWCORP	*corp ;
			int free_uniq_id ;

			free_uniq_id = Comm.accident_uniq_id[i] ;
			trgentk = COM_GetEnemyFromUniqID( free_uniq_id ) ;
			if ( trgentk == NULL ) {
				if ( (corp = CP_GetCorp( free_uniq_id )) == NULL ) {
					COM_UnsetAccident( free_uniq_id ) ;
					UNSET_FLAG( com->free_accident_bit, 1<<i ) ;
				}
			} else {
				if ( !(trgentk->act->status & ACT_STATUS_FAINT) ) {
					COM_UnsetAccident( free_uniq_id ) ;
					UNSET_FLAG( com->free_accident_bit, 1<<i ) ;
				}
			}
		}
	}

}

int COM_AnyoneStatus_G_U( COMMANDER *com, int g_id, int u_id, long64 status )
{
	E_UNIT	*unit ;
	int j ;

	unit = com->enemys.group[ g_id ]->unit[ u_id ] ;

	for ( j=0; j<unit->enemy_num; j++ ) {
		if ( unit->entk[ j ] == NULL ) continue ;
		if ( unit->entk[ j ]->act->status & status ) {
			return 1 ;
		}
	}
	return 0 ;
}

int COM_AnyoneStatus_G( COMMANDER *com, int g_id, long64 status )
{
	E_GROUP	*group ;
	int i ;

	group = com->enemys.group[ g_id ] ;
	for ( i=0; i<group->unit_num; i++ ) {
		if ( COM_AnyoneStatus_G_U( com, g_id, i, status ) ) return 1 ;
	}

	return 0 ;
}

#if 0
static int AnyoneStatus( COMMANDER *com, long64 status )
{
	int g ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		if ( COM_AnyoneStatus_G( com, g, status ) ) return 1 ;
	}
	return 0 ;
}

static void WakeUp( COMMANDER *com )
{
	if ( GM_AlertMode == ALERT_MODE_ALERT ) com->wakeup_count = COMMAND_WAKEUP_COUNT ;
	
	if ( com->wakeup_count > 0 ) {
		if ( com->wakeup_count == COUNT_VMODE(60) ) {
			if ( AnyoneStatus( com, ACT_STATUS_FAINT ) ) {
				COM_SetRadio( EV_RAD_ATK_RETURN, NULL ) ;
			} else {
				com->wakeup_count = 0 ;
			}
		}
		if ( com->wakeup_count == 1 ) {
			/* 寝ている兵を起こす */
			E_GROUP	*group ;
			E_UNIT	*unit ;
			int i, j, g ;
			
			for ( g=0; g<com->enemys.group_num; g++ ) {
				group = com->enemys.group[ g ] ;
				for ( i=0; i<group->unit_num; i++ ) {
					unit = group->unit[i] ;
					for ( j=0; j<unit->enemy_num; j++ ) {
						if ( unit->entk[ j ] == NULL ) continue ;
						if ( unit->entk[ j ]->act->status & ACT_STATUS_FAINT ) {
							unit->entk[ j ]->act->bodyp.faint_time = COUNT_VMODE(20) ;
						}
					}
				}
			}
		}
		com->wakeup_count -- ;
	}
}
#endif

static void Commander( COMMANDER *com )
{
	Accident( com ) ;
//	WakeUp( com ) ;
}
/*--- ------------------------------------------------------------*/

static void Act( Work *work )
{
	DebugMode( work->com ) ;

#if BP_ENABLE_DEBUG_MENU
   if( gBP_EnemyHearingDisabled == 0 )
      work->com->status &= ~CMST_ENEMY_HEARING_OFF;
   else if( gBP_EnemyHearingDisabled == 1 )
      work->com->status |= CMST_ENEMY_HEARING_OFF;

   if( gBP_EnemySightDisabled == 0 )
      work->com->status &= ~CMST_ENEMY_SIGHT_OFF;
   else if( gBP_EnemySightDisabled == 1 )
      work->com->status |= CMST_ENEMY_SIGHT_OFF;
#endif

	if ( GM_VRStatus & GM_VR_CLEAR ) {
		if( !(work->com->status & CMST_ENEMY_VR_CLEAR) ) {
			SET_FLAG( work->com->status, CMST_ENEMY_VR_CLEAR ) ;
			GM_JimakuHide( ) ;/* 字幕消す */
			GM_StreamStop( StrmCurrentHandler ) ;/* 字幕消す */
		}
		return ;
	}

	CheackMessage( work->com ) ;

	PlayerPositionInZone( work->com ) ;
	NoiseCheck( work->com ) ;
	Investigated( work->com ) ;
	StateCheck( work->com ) ;
	EnemyCount( work->com ) ;
	SetAlertLevel( work->com ) ;
	SetAvoidLevel( work->com ) ;
	SetSearchLevel( work->com ) ;
	DefenseMode( work->com ) ;
	Vibration( work->com ) ;

//	if ( work->com->stage_kind & ENE_STAGE_VR_ANOTHER ) {
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		VrStateCheck( work->com ) ;
	}

	Commander( work->com ) ;

	SpeakManage() ;


	DebugViewer( work->com ) ;

	work->com->flame_flag = 0 ;


//printf("comlast: map[%x] pos[%f][%f][%f]\n",work->com->player_lastmap,
//work->com->player_lastpos.vx,work->com->player_lastpos.vy,work->com->player_lastpos.vz) ;
}

static void Die( Work *work )
{


	GM_ResetGameStatus( GM_STATUS_DETECT ) ;
	GM_ResetGameStatus( STATE_ENE_SIGHTIN ) ;
	GM_JimakuHide( ) ;/* 字幕消す */
	GM_StreamStop( StrmCurrentHandler ) ;/* 字幕消す */

	if ( (GM_AlertMode == ALERT_MODE_ALERT) ||
		 (GM_AlertMode == ALERT_MODE_AVOID) ) {
		GM_CautionLevel = DEF_SEARCH_LEVEL ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		ENE_GameStatus = 0 ;
		work->com->stage_kind = 0 ;
	}
printf(" GM_CautionLevelCount[%d] GM_CautionLevel[%d]\n",GM_CautionLevelCount, GM_CautionLevel ) ;
}

/*--- ------------------------------------------------------------*/
static int GetRouteProc( route_proc )
ROUTE_PROC	*route_proc ;
{
	int	num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		route_proc->route = GCL_GetNextInt( ) ;
		route_proc->point = GCL_GetNextInt( ) ;
		route_proc->proc = GCL_GetNextInt( ) ;
		
		route_proc++ ;
		num++ ;
	}

	return num ;
	
}

static int GetRouteVoice( route_voice )
ROUTE_VOICE	*route_voice ;
{
	int	num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		route_voice->route = (short)GCL_GetNextInt( ) ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			route_voice->route += COM_GetRootOfset( ) ;
		}
		route_voice->point = (short)GCL_GetNextInt( ) ;
		route_voice->strm_code = GCL_GetNextInt( ) ;
		route_voice->strm_time = GCL_GetNextInt( )/TIME_BASE ;
printf("Route Voice route[%d]point[%d]code[%d]time[%d]\n",route_voice->route,route_voice->point,route_voice->strm_code,route_voice->strm_time) ;	
		route_voice++ ;
		num++ ;
	}

	return num ;
}

static int GetClearingVoice( clearing_voice )
CLEARING_VOICE	*clearing_voice ;
{
	int	num ;

	num = 0 ;
	while ( GCL_NextStr() != NULL ){
		clearing_voice->area = (char)GCL_GetNextInt( ) + COM_GetClearingOfset( );
		clearing_voice->route = (short)GCL_GetNextInt( ) ;
		clearing_voice->point = (short)GCL_GetNextInt( ) ;
		clearing_voice->strm_code = GCL_GetNextInt( ) ;
		clearing_voice->strm_time = GCL_GetNextInt( )/TIME_BASE ;
printf("Route Voice route[%d]point[%d]code[%d]time[%d]\n",clearing_voice->route,clearing_voice->point,clearing_voice->strm_code,clearing_voice->strm_time) ;
		clearing_voice++ ;
		num++ ;
	}

	return num ;
}

static int GetResources( Work *work, int where, int name )
{
	extern CONTROL *EMA_CommandGetControl() ;
	COMMANDER	*com ;
	char		*opt ;
    int			tmplight, num, mode, i, val, map ;

//SET_FLAG(GM_GameStatus, STATE_VR_ANOTHER) ;

	gameover_delay_count = 0 ;

	work->com = &Comm ;
	com = work->com ;

	com->name = name ;
	com->alertlevel = 0;
	com->avoidlevel = 0;
	com->searchlevel = 0;
	com->wakeup_count = 0 ;
	com->maxalert = 0 ;
	com->maxavoid = 0 ;
	com->uniq_id = 1;
	com->status = 0 ;
	com->alertstatus = 0 ;
	com->avoidstatus = 0 ;
	com->noise_near_id[0] = -1 ;
	com->noise_near_id[1] = -1 ;
	com->noise_near_id[2] = -1 ;
	com->noise_alert_timer = 0 ;
	com->noise_time = 0 ;
	com->accident_bit = 0 ;
	com->free_accident_bit = 0 ;
	for ( i=0; i<MAX_ACCIDENT; i++ ) {
		com->accident_delay[i] = 0 ;
	}

	com->flame_flag = 0 ;
	com->dam_en_num = 0 ;
	/*sigeno add 2002.08.21*/
	com->player_lastmap = 0 ;


	/*複数のマップでも最初の一つにする*/
	map = GV_GetBit( GV_GetNo( where ) ) ;

	com->plzone_in_zone[PL_POS_CENTER] = (255&GM_PlayerAddress) | ((255&GM_PlayerAddress) << 8 ) ;
	com->plpos_in_zone[PL_POS_CENTER] = GM_PlayerPosition ;
	com->plmap_in_zone[PL_POS_CENTER] = GM_PlayerHzxID ;
	com->plpos_box = DG_ZeroVector ;

	ENE_GameStatus = 0 ;
	ENE_AlertGameLevel = 0 ;
	COMMANDER_STATUS = &com->status ;
	DEF_SEARCH_LEVEL = SEARCH_LEVEL_MAX ;

	InitEnemys( com ) ;

	CP_InitCorpSystem( ) ;

	/* マップ指定 */
	if ( (val = GCL_GetOptionValue( 'd', -1 )) > 0  ) {
		map = val ;
	}

	/* 警戒モード時間 */
#ifdef PAL
	if ( ( opt = GCL_GetOption( 'h' ) ) != NULL ){
		DEF_SEARCH_LEVEL = DIRECT_TICK( GCL_GetNextInt() ) ;
printf("cuation time scn set [%d] !!!!!\n",DEF_SEARCH_LEVEL ) ;
	} else {
		DEF_SEARCH_LEVEL = SEARCH_LEVEL_MAX ;
printf("cuation time default set [%d] !!!!!\n",DEF_SEARCH_LEVEL ) ;
	}
#else
	DEF_SEARCH_LEVEL = DIRECT_TICK(GCL_GetOptionValue( 'h', SEARCH_LEVEL_MAX )) ;
#endif

	/* 銃ライト設定 */
	if ( (tmplight = GCL_GetOptionValue( 'g', -1 )) < 0  ) {
		tmplight = 0 ;
	}
	com->status |= tmplight<<28 ;

	/* 復活場所 */
	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		num = com->res_num = ENE_GCL_GetFVandM( &com->res_pos[0], &com->res_map[0] ) ;
		if ( num >= MAX_RES_POS ) {
			printf("command.c: Err!! Resurrection Num Over[%d]!! \n",num);
			return -1 ;
		}
		printf(" resuurect num = [%d] \n",num ) ;
		for( num--; num >=0 ; num-- ) {
			com->res_pos[num].vy += 500.0f ;/* ワープ用に床より上げておく */

			printf("res[%f][%f][%f]",com->res_pos[num].vx, com->res_pos[num].vy, com->res_pos[num].vz ); 
			printf(" map [%x]\n",com->res_map[num] ) ;
		}
	} else {
		com->res_pos[0] = DG_ZeroVector ;
		com->res_map[0] = map ;
	}

	/* 待機場所 */
	if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){
		num=com->waiting_num=ENE_GCL_GetFVandM( &com->waiting_pos[0], &com->wait_map[0] ) ;
		if ( num >= MAX_WAIT_POS ) {
			printf("command.c: Err!! Waiting Loom Num Over[%d]!! \n",num);
			return -1 ;
		}
		com->waiting_cur = 0 ;
	} else {
		com->waiting_pos[0] = DG_ZeroVector ;
		com->res_map[0] = map ;
	}

	/* ルートプロック */
	num = 0 ;
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		num = GetRouteProc( &com->route_proc[0] ) ;
		if ( num >= MAX_ROUTE_PROC ) {
			printf("command.c: Err!! Route Proc Num Over[%d]!! \n",num);
			return -1 ;
		}
	}
	com->route_proc[num].proc = -1 ;/* terminate */

	/* ルート音声 */
	num = 0 ;
	if ( ( opt = GCL_GetOption( 'f' ) ) != NULL ){
		num = GetRouteVoice( &RouteVoice[0] ) ;
		if ( num >= MAX_ROUTE_VOICE ) {
			printf("command.c: Err!! Route Voice Num Over[%d]!! \n",num);
			return -1 ;
		}
	}
	RouteVoice[num].strm_code = -1 ;/* terminate */
{
	for(i=0;i<num+1;i++){
printf("r%d p%d code%d\n",RouteVoice[i].route, RouteVoice[i].point, RouteVoice[i].strm_code);
	}
}

	for( i=0; i<MAX_CLE_AREA; i++ ){
		com->cle_area_status[ i ] = 0 ;
	}

	/* クリアリング音声 */
	num = 0 ;
	if ( ( opt = GCL_GetOption( 'j' ) ) != NULL ){
		num = GetClearingVoice( &ClearingVoice[0] ) ;
		if ( num >= MAX_ROUTE_VOICE ) {
			printf("command.c: Err!! Clearing Voice Num Over[%d]!! \n",num);
			return -1 ;
		}
	}
	ClearingVoice[num].strm_code = -1 ;/* terminate */

	/* クリアリングプロック */
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		num=com->cle_proc_num = GetClearingProc( &com->cle_proc[0] ) ;
		if ( num >= MAX_CLE_PROC ) {
			printf("command.c: Err!! Clearing Proc Num Over[%d]!! \n",num);
			return -1 ;
		}
	} else {
		com->cle_proc_num = 0 ;
	}
	com->tmp_cle_proc_num = 0 ;	/* クリアリング一時バッファ初期化 */
{
	int i ;
	
	for( i=0; i<com->cle_proc_num; i++ ){
		printf("area[%d] route[%d] point[%d] proc[%d]\n",
		com->cle_proc[i].area,com->cle_proc[i].route,com->cle_proc[i].point,com->cle_proc[i].proc) ;
	}
}
	/* 台詞システム初期化  */
	if ( GCL_GetOption( 'v' ) != NULL ){
		int i, *ptr_le ;

		ptr_le = GCL_GetNextResource_LE( ) ;
		i = 0 ;
		while ( BP_LE_SwapSInt( *ptr_le ) >= 0 ) {
			StrmCode[ i ] = BP_LE_SwapSInt( *(ptr_le++) );
			StrmTimeTable[ i ] = BP_LE_SwapSInt( *(ptr_le++) )/TIME_BASE ;
//printf(" ene vox [%d] strm code[%x] time[%d]\n",i, StrmCode[ i ], StrmTimeTable[ i ] ) ;
			
			i++ ;
			if ( i > MAX_VOICE_NUM ) {
				printf("ene voice num over\n") ;
				return -1 ;
			}
		}
	}
	for( i=0; i<MAX_GHQAREA; i++ ){
		GHQAreaVoice[ i ].num = 0 ;
	}
	if ( GCL_GetOption( 'x' ) != NULL ){
		int i, *ptr_le, area ;

		ptr_le = GCL_GetNextResource_LE( ) ;
		i = 0 ;
		area = 0 ;
		while ( BP_LE_SwapSInt( *ptr_le ) != -1 ) {
			if ( BP_LE_SwapSInt( *ptr_le ) != -2 ) {
printf(" area [%d] num[%d] strm code[%d]\n",area, i, BP_LE_SwapSInt( *ptr_le ) ) ;
				GHQAreaVoice[ area ].strm_code[ i++ ] =  BP_LE_SwapSInt( *(ptr_le++) ) ;
				GHQAreaVoice[ area ].num = i ;
			} else {
				area++ ;
				i = 0 ;
				ptr_le++ ;
			}
		}
	}
	GHQCautionVoice[ 0 ] = 0 ;
	StrmCurrentCaution = 0 ;
	StrmCurrentPlay = 0 ;
	GHQConfirmVoice = 0 ;
	GHQMecaConfirmVoice = 0 ;
	if ( GCL_GetOption( 'y' ) != NULL ){
		int i, *ptr_le ;

		ptr_le = GCL_GetNextResource_LE( ) ;
		i = 0 ;
		while ( BP_LE_SwapSInt( *ptr_le ) >= 0 ) {
			GHQCautionVoice[ i++ ] = BP_LE_SwapSInt( *(ptr_le++) ) ;
			printf("GHQCautionVoice[%d]\n",GHQCautionVoice[i-1] ) ;
		}
		if ( BP_LE_SwapSInt( *ptr_le ) == -2 ) {
			ptr_le++ ;
			GHQConfirmVoice = BP_LE_SwapSInt( *(ptr_le++) );
			printf("GHQConfirmVoice[%d]\n",GHQConfirmVoice ) ;
		}
		if ( BP_LE_SwapSInt( *ptr_le ) == -3 ) {
			ptr_le++ ;
			GHQMecaConfirmVoice  = BP_LE_SwapSInt( *(ptr_le++) );
			printf("GHQMecaConfirmVoice[%d]\n",GHQMecaConfirmVoice ) ;
		}
	}
	COM_InitSpeak( ) ;

	/* ステージ種類 */
	com->stage_kind = GCL_GetOptionValue( 's', 0 ) ;

	if ( (mode = GCL_GetOptionValue( 'm', -1 )) >= 0  ) {
printf("Start Alert Mode [%d] \n",mode ) ;
		switch( mode ) {
			case ALERT_MODE_ALERT :
				com->alertlevel = MAX_ALERT_LEVEL ;
				GM_AlertMode = ALERT_MODE_ALERT ;
				SET_FLAG( GM_StageHappening, GM_STAGE_HAPPEN_ALERT ) ;
				com->player_lastpos = GM_PlayerPosition ;
				com->player_lastmap = GM_PlayerHzxID ;
			break ;
			case ALERT_MODE_AVOID :
				com->time = AVOID_TIME ;
				GM_AlertMode = ALERT_MODE_AVOID ;

				com->player_lastpos = GM_PlayerPosition ;
				com->player_lastmap = GM_PlayerHzxID ;
			break ;
			case ALERT_MODE_SEARCH :
//				COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
				if ( com->stage_kind & ENE_STAGE_SEARCH_MAX ) {
					COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
				} else {
					if ( GM_CautionLevelCount < DEF_SEARCH_LEVEL ) {
						COM_SetSearchLevel( GM_CautionLevelCount ) ;
					} else {
						COM_SetSearchLevel( DEF_SEARCH_LEVEL ) ;
					}
				}
				GM_AlertMode = ALERT_MODE_SEARCH ;

printf( " SEARCH START Comm.searchlevel[%d] GM_CautionLevelCount[%d]\n",Comm.searchlevel,GM_CautionLevelCount);
			break ;
		}
	}

	CM_ZZZ_TIME = GCL_GetOptionValue( 'z', ZZZ_TIME ) ;
	CM_ZZZ_TIME = COUNT_VMODE(CM_ZZZ_TIME) ;

	CM_FAINT_TIME = GCL_GetOptionValue( 'o', FAINT_TIME ) ;
	CM_FAINT_TIME = COUNT_VMODE(CM_FAINT_TIME) ;

	com->res_count_in_alert = 0 ;
	com->res_count = 0 ;
	com->max_res_in_alert = GCL_GetOptionValue( 'b', DEF_MAX_RES_IN_ALERT ) ;
	com->max_res_num = GCL_GetOptionValue( 'k', ENEMY_MAX_RES ) ;
	com->enemy_appear_max = ENEMY_APPEAR_MAX ;

	com->cyp_res_count_in_alert = 0 ;
	com->cyp_res_count = 0 ;
	com->cyp_max_res_in_alert = GCL_GetOptionValue( 't', 0 ) ;
	com->cyp_max_res_num = GCL_GetOptionValue( 'u', ENEMY_MAX_RES ) ;
	com->cyp_kill_count = 0 ;
	com->cyp_num = 0 ;
	com->root_ofset = GCL_GetOptionValue( 'n', 0 ) ;

	if ( com->stage_kind & ENE_STAGE_TOILET ) {
		SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_TOILET) ;
	}


#if 1
	com->npc_ctrl = EMA_CommandGetControl() ;
	if ( com->npc_ctrl != NULL ) {
		SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_NPC_EMMA) ;
	}
#else
	if ( com->stage_kind & ENE_STAGE_NPC_EMMA ) {
		extern CONTROL *EMA_CommandGetControl() ;
		SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_NPC_EMMA) ;
		com->npc_ctrl = EMA_CommandGetControl() ;
	}
#endif

	if ( com->stage_kind & ENE_STAGE_NPC_SNAKE ) {
		SET_FLAG( ENE_GameStatus, ENE_GMSTATUS_NPC_SNK) ;
	}

	/* コマンダープロック */
	for( i=0; i<MAX_COM_PROC; i++ ) {
		com->com_procs[i] = 0 ;
	}
	if ( GCL_GetOption( 'l' ) != NULL ){
		for( i=0; i<MAX_COM_PROC; i++ ) {
			if( GCL_NextStr() != NULL ) {
				com->com_procs[i] = GCL_GetNextInt( ) ;
			} else {
				break ;
			}
		}
	}

	/* 警備兵コマンダーコール */
	if ( ( opt = GCL_GetOption( 'w' ) ) != NULL ){
		ENE_GclCallProcs( opt );
	}

	/* 攻撃兵コール */
	if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){
		ENE_GclCallProcs( opt );
	}

#ifdef DEBUG_MODE
{
	int g ;
	for ( g=0; g<com->enemys.group_num; g++ ) {
//		ASSERT ( com->enemys.group[ g ]->unit_num > 0 ) ;
		for ( i=0; i<com->enemys.group[ g ]->unit_num; i++ ) {
//			ASSERT ( com->enemys.group[ g ]->unit[ i ]->enemy_num > 0 ) ;
		}
	}
}
#endif

#ifdef DEBUG_MODE
{
	extern void NewEnemyDebugView( COMMANDER * ) ;
	NewEnemyDebugView( com ) ;
}
#endif

printf(" ENEMY SET END\n");

if(0){/* w00aで実験 */
	HZX_ZON	*z ;

	z = HZX_GetZoneFromAdd( 0xdede ) ;
	SET_FLAG( z->flag, HZX_ZONE_ZINTRPT ) ;

	z = HZX_GetZoneFromAdd( 0xdbdb ) ;
	SET_FLAG( z->flag, HZX_ZONE_ZINTRPT ) ;
}

#ifdef DEBUG_MODE
{
//extern void *NewNavites( FVECTOR *, int ) ;
//NewNavites( &GM_PlayerPosition, SD_P_WALL02 ) ;
}
#endif

	return (0);
}


void *NewCommander( name, where )
int	name ;
int	where ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), COMMANDER_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, where, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
/*
ｘ：0
ｙ：-35.0935
ｚ：100.7687


 sna_non_carry_body_start_f.mtn  
スネークの腰から敵兵の腰までのシフト値
ｘ：0
ｙ：-42.6933
ｚ：100.7687

*/
