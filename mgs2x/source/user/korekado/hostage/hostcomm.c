//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	hostcomm.c
	人質コマンダー

	2001/04/8 Y.Korekado
	$Id: hostcomm.c,v 1.1.1.3 2002/11/19 11:44:17 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------
void	*NewHostageCommander( name, where )
----------------------------------------------------------------*/
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
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"korekado/npc/npc.h"
#include	"korekado/enemy/enemy.h"
#include	"../../kano/hostage/hostage.h"


/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
#include	"hostage.h"
/*----------------------------------------------------------------*/
#define	MAX_HOSTAGE	32
#define	MAX_ENE		8
#define	MAX_PROC	16
#define	MAX_VOICE	24

typedef	struct	{
	GV_ACT_EX	actor ;

	Work		*work[MAX_HOSTAGE] ;
	int			num ;				/*人質人数*/
	COMMANDER	*com ;

	int			q_man ;
	int			proc[ MAX_PROC ] ;

	int			voice[ MAX_VOICE ] ;
} HostComm ;

HostComm	*Hcom ;

/*----------------------------------------------------------------*/
/* シナリオプロック */
enum {
	DIE_HOSTAGE,			/* 0人質死亡 */
	DETECT_HOSTAGE,			/* 1敵兵が発見 */
	REACTION_HOSTAGE,		/* 2人質が呼びかけに反応 */
	ENE_DIE_HOSTAGE,		/* 3敵兵死亡 */
	RADIO_BREAK_HOSTAGE,	/* 4無線破壊 */
	HOLDUP_HOSTAGE,			/* 5ホールドアップ */
	FAINT_HOSTAGE,			/* 6気絶 */
	CHAFF_HOSTAGE,			/* 7チャフが使われた */
	CAUTION_HOSTAGE,		/* 8警戒モードになった */
	PUNCHDAMAGE_HOSTAGE,	/* 9打撃ダメージを受けた */
} ;

/*----------------------------------------------------------------*/

int	HSTG_GetHostageStatus( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name, status, i ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	for( i=0; i<Hcom->num; i++ ) {
		if ( Hcom->work[ i ]->control.name == name ) {
			status = Hcom->work[ i ]->npc.action.status ;
			if ( GCL_NextStr() == NULL ) return -1 ;
			GCL_GetNextVarRef( &ref ); // 参照データの取得
			GCL_SetVarRef( &ref, 0, status );
			return 1 ;
		}
	}

	return -1 ;
}

/*----------------------------------------------------------------*/
int	HSTG_CallVox( int vox )
{
//	return GM_VoxStream( Hcom->voice[ vox ], 0 ) ;
	return GM_VoxStream( Hcom->voice[ vox ], GM_STREAM_FLAG_3D ) ;
}

int	HSTG_SetHostage( Work *work)
{
	if ( Hcom->num >= MAX_HOSTAGE ) return -1 ;
	Hcom->work[ Hcom->num ] = work ;

	return Hcom->num ++ ;
}

int	HSTG_NoiseCheck( )
{
	if ( Hcom->com == NULL ) return 0 ;

	return Hcom->com->noise ;
}


int	HSTG_MicQuestion( Work *work )
{
	if ( work->id == Hcom->q_man ) {
		return 1 ;
	}

	return 0 ;
}
/*----------------------------------------------------------------*/
void HSTG_ProcCallReaction( int name )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;

	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ;

	argv[0] = name ;
	arg.argc = 1 ;
	arg.argv = &argv[0] ;
	ENE_ExecProc( Hcom->proc[ REACTION_HOSTAGE ], &arg ) ;
}

static void ProcCall( HostComm *hcom, int name, int proc_no )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;

	if ( GM_CheckGameStatus( STATE_GAMEOVER ) ) return ;

	argv[0] = name ;
	arg.argc = 1 ;
	arg.argv = &argv[0] ;
	ENE_ExecProc( hcom->proc[ proc_no ], &arg ) ;
	if ( proc_no != PUNCHDAMAGE_HOSTAGE ) {
		hcom->proc[ proc_no ] = 0 ;/* 一回しか呼ばない */
	}
printf("hostage com : proc call proc_no[%d]\n",proc_no ) ;
}

static	ENETHINK *EnemyIknowFlag( COMMANDER	*com, int flag )
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;

	num = 0 ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;
				if ( entk->iknow_flag & flag ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

static	ENETHINK *EnemyDamage( COMMANDER	*com )
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;

	num = 0 ;
	
	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;

				if ( entk->act->status & ACT_STATUS_DEATH) {
					return entk ;
				}
				if ( entk->act->status_status & ACT_STST_PIYOPIYO ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

static	ENETHINK *EnemyRadioBreak( COMMANDER	*com )
{
	E_GROUP		*gp ;
	E_UNIT		*un ;
	ENETHINK	*entk ;
	int			g, u, i, num ;

	num = 0 ;

	for ( g=0; g<com->enemys.group_num; g++ ) {
		gp = com->enemys.group[ g ] ;
		for ( u=0; u<gp->unit_num; u++ ) {
			un = gp->unit[ u ] ;
			for ( i=0; i<un->enemy_num; i++ ) {
				if ( (entk = un->entk[ i ]) == NULL ) continue ;

				if ( entk->sw.radio & SW_FLAG_BREAK ) {
					return entk ;
				}
			}
		}
	}

	return NULL ;
}

static int BodyInCamera( OBJECT *body, float f )
{
	FVECTOR	pos_head, pos_west ;
	float f1, f2 ;

	KR_FMatToFvec( &BODYWORLD( body, HUMAN21_ATAMA), &pos_head ) ;
	f1 = KR_InnerProductInCamera( &pos_head ) ;
	KR_FMatToFvec( &BODYWORLD( body, HUMAN21_KOSHI), &pos_west ) ;
	f2 = KR_InnerProductInCamera( &pos_west ) ;

printf("f1[%f] f2[%f]\n",f1,f2 ) ;

	if ( (f1 > f) || (f2 > f) ) return 1 ;

	return 0 ;
}


int	KR_GetHostageTarget( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name, q_man, q_dis, i ;
	Work *work ;

	q_man = -1 ;
	q_dis = 10000000 ;
	name = 0 ;

	for( i=0; i<Hcom->num; i++ ) {
		if ( Hcom->work[ i ]->pl_dis < HSTG_MIC_QUETION_DIS ) {
			work = Hcom->work[ i ] ;
#if 0
			if ( BodyInDir( work, HSTG_MIC_QUETION_DIR ) ) {
#else
			if ( BodyInCamera( &work->body, HOST_SE_COS_15 ) ) {
#endif
				if ( Hcom->work[ i ]->pl_dis < q_dis ) {
					q_man = i ;
					q_dis = Hcom->work[ i ]->pl_dis ;
				}
			}
		}
	}

	if ( q_man >= 0 ) {
		name = Hcom->work[ q_man ]->control.name ;
	}

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, name );

	return 1 ;
}


#if 0//no use
static int BodyInDir( Work *work, int dir )
{
	int d, diff ;

	d = (work->pl_dir + 2048 ) & 4095 ;
	diff = GV_DiffDirS( GM_PlayerControl->rot.vy, d ) ;
	if ( diff < dir ) return 1 ;

	return 0 ;
}
#endif

static void MicQuestion( HostComm	*hcom, int no_mic )
{
	int i, q_dis ;
	Work *work ;

	


	hcom->q_man = -1 ;

	if ( no_mic ) return ;
#if 1
	if ( Hcom->com->noise == NOISE_MIC_QUEST ) {
#else
	if ( Ply_GetPlayerWeapon() == WP_Mic ) {
#endif
		q_dis = 10000000 ;
		for( i=0; i<hcom->num; i++ ) {
			if ( hcom->work[ i ]->pl_dis < HSTG_MIC_QUETION_DIS ) {
				work = hcom->work[ i ] ;
#if 0
				if ( BodyInDir( work, HSTG_MIC_QUETION_DIR ) ) {
#else
				if ( BodyInCamera( &work->body, HOST_SE_COS_15 ) ) {
#endif
					if ( hcom->work[ i ]->pl_dis < q_dis ) {
						hcom->q_man = i ;
						q_dis = hcom->work[ i ]->pl_dis ;
					}
				}
			}
		}

		if ( hcom->q_man >= 0 ) {
			printf(" target hostage [%d]\n",hcom->q_man ) ;
		} else {
			printf(" Player Called !! But No target hostage \n" ) ;
		}
	}
}


/*----------------------------------------------------------------*/
static	void	Act( hcom )
HostComm	*hcom ;
{
	ENETHINK *entk ;
	int i, no_mic ;

	if ( hcom->com == NULL ) return ;

	if ( GM_GameStatus & GM_STATUS_DETECT ) {
		if(	(entk = EnemyIknowFlag( hcom->com, IKNOW_DETECT )) != NULL ){
			ProcCall( hcom, entk->ctrl->name, DETECT_HOSTAGE ) ;
		} else {
			ProcCall( hcom, 0, DETECT_HOSTAGE ) ;
		}
	}

	if(	(entk = EnemyDamage( hcom->com )) != NULL ){
		if ( entk->act->status & ACT_STATUS_DEATH) {
			ProcCall( hcom, entk->ctrl->name, ENE_DIE_HOSTAGE ) ;
		} else {
			ProcCall( hcom, entk->ctrl->name, FAINT_HOSTAGE ) ;
		}
	}

	if(	(entk = EnemyRadioBreak( hcom->com )) != NULL ){
		ProcCall( hcom, entk->ctrl->name, RADIO_BREAK_HOSTAGE ) ;
	}

	if( (entk = COM_EnemyActStatus( ACT_STATUS_HOLD_UP )) != NULL ) {
		ProcCall( hcom, entk->ctrl->name, HOLDUP_HOSTAGE ) ;
	}

	if ( GM_GameStatus & STATE_CHAFF ) {
		ProcCall( hcom, 0, CHAFF_HOSTAGE ) ;
	}

#if 1	//PAL remaster から
	if ( GM_AlertMode == ALERT_MODE_SEARCH ||
		 GM_AlertMode == ALERT_MODE_ALERT ) {
		ProcCall( hcom, 0, CAUTION_HOSTAGE ) ;
	}
#else
	if ( GM_AlertMode == ALERT_MODE_SEARCH ) {
		ProcCall( hcom, 0, CAUTION_HOSTAGE ) ;
	}
#endif

	no_mic = 0 ;
	for( i=0; i<hcom->num; i++ ) {
		if ( hcom->work[ i ]->status & HSTG_STATUS_DIE ) {
			ProcCall( hcom, hcom->work[i]->control.name, DIE_HOSTAGE ) ;
		}
		if ( hcom->work[ i ]->npc.action.status & NPC_ACT_STATUS_PUNCH_DAMAGE ) {
			ProcCall( hcom, hcom->work[i]->control.name, PUNCHDAMAGE_HOSTAGE ) ;
		}
		if ( hcom->work[ i ]->status & HSTG_STATUS_MUST_DIE ) {
			no_mic = 1 ;
		}
	}

	MicQuestion( hcom, no_mic ) ;
}

static	void	Die( hcom )
HostComm	*hcom ;
{
}
/*----------------------------------------------------------------*/
static	int	GetResources( hcom, name, where )
HostComm	*hcom ;
int		name ;
int		where ;
{
	int i ;
	
	Hcom = hcom ;
	Hcom->num = 0 ;

	/* 終了プロック */
	for( i=0; i<MAX_PROC; i++ ) {
		hcom->proc[i] = 0 ;
	}

#ifndef NO_PROC_CALL
	if ( GCL_GetOption( 'e' ) != NULL ){
		for( i=0; i<MAX_PROC; i++ ) {
			if( GCL_NextStr() != NULL ) {
				hcom->proc[i] = GCL_GetNextInt( ) ;
			} else {
				break ;
			}
		}
	}
#endif

	if ( GCL_GetOption( 'v' ) != NULL ){
		for( i=0; i<MAX_VOICE; i++ ) {
			if( GCL_NextStr() != NULL ) {
				hcom->voice[i] = GCL_GetNextInt( ) ;
			} else {
				break ;
			}
		}
	}

	hcom->com = NULL ;
	if ( GCL_GetOption( 'm' ) != NULL ){
		printf("hostcomm: Game Hostage!!\n");
		hcom->com = COM_GetCommander() ;
	}
	hcom->q_man = -1 ;

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewHostageCommander( name, where )
int		name ;
int		where ;
{
	HostComm		*Hcom ;

	OPERATOR() ;
    Hcom = (HostComm *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( HostComm ), COMMANDER_PRIO ) ;
	if ( Hcom != NULL ) {
		GV_SetActor( &( Hcom->actor ), Act, Die ) ;
		GV_ActorEX( &Hcom->actor ) ;
		if ( GetResources( Hcom, name, where ) < 0 ) {
			GV_DestroyActor( Hcom ) ;
			return NULL ;
		}
	}
	return Hcom ;
}
