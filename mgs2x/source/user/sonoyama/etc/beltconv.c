//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   beltconv.c
   ベルトコンベア制御
   
   2001/01/10 M.Sonoyama
   $Id: beltconv.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include "BP_Misc.h"

/*------------------------------------------------------------*/

typedef	struct	{
	FVECTOR		step ;
	FVECTOR		step2 ;
	int			trap ;
	HZX_PTP		*start ;
	HZX_PTP		*point ;
	HZX_PTP		*next ;
	int			chara ;
	short		item ;
	short		area ;
} BELT_DATA ;

static	BELT_DATA	*BeltData = NULL ;
static	int			N_Belts = 0, MAX_Belts = 0 ;

static	int			BeltStatus = 0 ;

float				BeltSpeedScale = 1.0F ;
static	float		BeltSpeedScaleStep = 0.0F ;

enum {
	BELT_STATE_NORMAL 		= 0x0000,
	BELT_STATE_STOP			= 0x0001,	/* ベルト止まっている */
} ;

/*--------------------------------------------------------------------------*/

/* 移動床関数 */
static	int	BeltMoveFunc( FVECTOR *npos, HZX_GROUP_ID hzx_id, FVECTOR *pos, HZX_EVT *evt, 
						  int name )
{
	int			i, j ;
	int			trap ;
	float		ip ;
	FVECTOR		v, *step, scalestep ;
	BELT_DATA	*b = BeltData, *found = NULL ;

	if ( BeltStatus & BELT_STATE_STOP ) {
		/* 停止中 */
		DG_COPY_VEC( npos, pos ) ;
		return 0 ;
	}
	if ( evt == NULL ) {
		for ( i = 0; i < N_Belts; i ++ ) {
			if ( HZX_CheckInsideTrap( hzx_id, pos, b[ i ].trap ) ) {
				found = &b[ i ] ;
				break ;
			}
		}
	} else {
		for ( i = 0; i < evt->n_inside; i ++ ) {
			trap = evt->inside[ i ] ;
			/* 名前有りを先にチェック */
			for ( j = 0; j < N_Belts; j ++ ) {
				if ( trap == b[ j ].trap && 
					 b[ j ].chara != 0 &&
					 name == b[ j ].chara ) {
					/* アイテムチェック */
					if ( b[ j ].item >= 0 ) {
						if ( GM_PlayerControl != NULL &&
							 GM_PlayerControl->name == name &&
							 PL_GetPlayerItem() == b[ j ].item ) {
							found = &b[ j ] ;
							break ;
						}
					} else {
						found = &b[ j ] ;
						break ;
					}
				}
			}
			if ( found != NULL ) break ;
			/* 名前なしをチェック */
			for ( j = 0; j < N_Belts; j ++ ) {
				if ( b[ j ].chara == 0 && 
					 trap == b[ j ].trap ) {
					found = &b[ j ] ;
					break ;
				}
			}
			if ( found != NULL ) break ;
		}
#if 0
		if ( found == NULL ) {
			/* 位置でもう一度全チェック */
			/* 名前有りを先にチェック */
			for ( i = 0; i < N_Belts; i ++ ) {
				if ( b[ i ].chara == 0 || name != b[ i ].chara ) continue ;
				/* アイテムチェック */
				if ( b[ i ].item >= 0 ) {
					if ( GM_PlayerControl == NULL ||
						 GM_PlayerControl->name != name ||
						 PL_GetPlayerItem() != b[ i ].item ) {
						continue ;
					}
				} 
				if ( HZX_CheckInsideTrap( hzx_id, pos, b[ i ].trap ) ) {
					found = &b[ i ] ;
					break ;
				}
			}
			if ( found != NULL ) goto found_ok ;
			/* 名前なしをチェック */
			for ( i = 0; i < N_Belts; i ++ ) {
				if ( b[ i ].chara != 0 ) continue ;
				if ( HZX_CheckInsideTrap( hzx_id, pos, b[ i ].trap ) ) {
					found = &b[ i ] ;
					break ;
				}			   
			}
		}
#endif
	}
	if ( found == NULL ) return 0 ;

//found_ok :

	step = &found->step ;

	v.vx = found->point->x - pos->vx ;
	v.vy = found->point->y - pos->vy ;
	v.vz = found->point->z - pos->vz ;
	ip = _sceVu0InnerProduct( step, &v ) ;

	if ( ip <= 0.0F ) {
		if ( found->next != NULL ) {
			step = &found->step2 ;
		} else {
			step = &DG_ZeroVector ;
			npos->vx = found->point->x ;
			npos->vy = found->point->y ;
			npos->vz = found->point->z ;
			return 0 ;
		}
	}

	_sceVu0ScaleVector( &scalestep, step, BeltSpeedScale ) ;

   if ( BP_IsPAL()==TRUE )
      _sceVu0ScaleVector( &scalestep, &scalestep, 60.0F / 50.0F ) ;

   _sceVu0AddVector( npos, pos, &scalestep ) ;

	/* 上下処理 */
	/* 後で根本的解決策を考える */
	if ( step == &found->step ) {
		if ( step->vy < 0.0F && npos->vy < found->point->y ) {
			npos->vy = found->point->y ;
		} else if ( step->vy > 0.0F && npos->vy > found->point->y ) {
			npos->vy = found->point->y ;
		}
	}
	
//	if ( DG_FABS( step->vx ) < 0.10F ) npos->vx = found->point->x ;
	if ( DG_FABS( step->vy ) < 0.10F ) npos->vy = found->point->y ;
//	if ( DG_FABS( step->vz ) < 0.10F ) npos->vz = found->point->z ;

	return 1 ;
}

/*--------------------------------------------------------------------------*/

/* ベルトコンベア管理 */
typedef	struct	{
	GV_ACT		actor ;
	int			lamp_name ;
	int			n_lamps ;
	int			sevol ;
	int			sevol_aim ;
	int			mode ;
} BCM_Work ;

enum {
	LAMP_BLUE	=	0,
	LAMP_RED	=	1,
	LAMP_YELLOW	=	2,
} ;

/* ＳＥ位置 */
static	FVECTOR		SePos ;

/* ランプ位置 */
static 	FVECTOR		LampPos[] = {
	{ 48516.0F, 839.0F, -78928.0F },
	{ 42928.0F, -3661.0F, -94734.0F },
} ;

static	int			LampSe[] = {
	SD_A_LED_BLUE,
	SD_A_LED_YELL,
	SD_A_LED_RED1,
} ;

static	BCM_Work	BcmWork ;

static	void	SeSetLamp( int mode )
{
	FVECTOR		*pos ;
	float		len, lenmin ;
	int			i ;

	lenmin = 1000000.0F ;
	pos = NULL ;

	for ( i = 0; i < BcmWork.n_lamps; i ++ ) {
		len = GV_VecLen3F2( &GM_PlayerPosition, &LampPos[ i ] ) ;
		if ( len < lenmin ) {
			lenmin = len ;
			pos = &LampPos[ i ] ;
		}
	}
	if ( pos == NULL ) return ;
	GM_SeSetMode( LampSe[ mode ], pos, GM_SEMODE_BOMB ) ;
}

static	void	SendMessageToLamp( int mode, int semode )
{
	BCM_Work	*work ;
	GV_MSG		msg ;
	int			buf[ 4 ] ;
	int			i ;

	work = &BcmWork ;

	msg.address = work->lamp_name ;
	msg.message_len = 3 ;
	msg.message = buf ;
	buf[ 0 ] = 2 ;	/* 切り替え */
	buf[ 2 ] = mode ;
	for ( i = 0; i < work->n_lamps; i ++ ) {
		buf[ 1 ] = i ;
		GV_SendMessage( &msg ) ;
	}
	//printf( "[%d] belt lamp mode %d\n", GV_Time, mode ) ;
	if ( work->mode != mode ) {
		if ( semode > 0 ) SeSetLamp( mode ) ;
		work->mode = mode ;
	}
}

static	void	GetNearPoint( FVECTOR *pos, BELT_DATA *b )
{
	float		xmin, xmax, zmin, zmax ;

	if ( DG_FABS( b->step.vx ) > 1.0F ) {
		xmin = DG_MIN( b->start->x, b->point->x ) ;
		xmax = DG_MAX( b->start->x, b->point->x ) ;
		if ( GM_PlayerPosition.vx < xmin ) {
			pos->vx = xmin ;
		} else if ( GM_PlayerPosition.vx > xmax ) {
			pos->vx = xmax ;
		} else {
			pos->vx = GM_PlayerPosition.vx ;
		}
		pos->vy = b->point->y ;
		pos->vz = b->point->z ;
	} else if ( DG_FABS( b->step.vz ) > 1.0F ) {
		zmin = DG_MIN( b->start->z, b->point->z ) ;
		zmax = DG_MAX( b->start->z, b->point->z ) ;
		if ( GM_PlayerPosition.vz < zmin ) {
			pos->vz = zmin ;
		} else if ( GM_PlayerPosition.vz > zmax ) {
			pos->vz = zmax ;
		} else {
			pos->vz = GM_PlayerPosition.vz ;
		}
		pos->vy = b->point->y ;
		pos->vx = b->point->x ;		
	} else {
		pos->vx = b->point->x ;
		pos->vz = b->point->z ;
		pos->vy = b->point->y ;
	}
}

static	void	SeSet( BCM_Work *work, FVECTOR *pos, int se )
{
	int		vol, pan ;
   float bp_angle;

	GM_SeGetVolPan( pos, GM_SEMODE_NORMAL, &vol, &pan, &bp_angle ) ;
	if ( GM_AlertMode == ALERT_MODE_ALERT ||
		 GM_AlertMode == ALERT_MODE_AVOID ) {
		work->sevol_aim = vol * 1 / 2 ;
	} else {
		work->sevol_aim = vol ;
	}
	work->sevol = GV_NearExp4( work->sevol, work->sevol_aim ) ;
	GM_SeSet3D( pan, work->sevol, se, bp_angle ) ;
}

static	void	BCM_WorkAct( BCM_Work *work )
{
	if ( GV_PauseLevel != 0 ) return ;
	if ( BeltSpeedScaleStep != 0.0F ) {
		BeltSpeedScale += BeltSpeedScaleStep ;
		if ( BeltSpeedScale <= 0.0F ) {
			BeltSpeedScale = 0.0F ;
			BeltSpeedScaleStep = 0.0F ;
			SendMessageToLamp( LAMP_BLUE, 1 ) ;
			BeltStatus |= BELT_STATE_STOP ;
		} else if ( BeltSpeedScale >= 1.0F ) {
			BeltSpeedScale = 1.0F ;
			BeltSpeedScaleStep = 0.0F ;
		}
	}
	/* 環境音設定 */
	{
		BELT_DATA	*b = BeltData ;
		float		len, lenmin[ 3 ] ;
		FVECTOR		p, pos[ 3 ] ;
		int			i, area ;

		lenmin[ 0 ] = lenmin[ 1 ] = lenmin[ 2 ] = 1000000.0F ;
		for ( i = 0; i < N_Belts; i ++ ) {
			GetNearPoint( &p, &b[ i ] ) ;
			area = b[ i ].area ;
			len = GV_VecLen3F2( &GM_PlayerPosition, &p ) ;
			if ( len < lenmin[ area ] ) {
				lenmin[ area ] = len ;
				DG_COPY_VEC( &pos[ area ], &p ) ;
			}
		}
		//GM_SetCurrentMap( GM_PlayerMap ) ;
		//NewPointView( &pos[ 0 ], 232, 32, 32 ) ;
		//NewPointView( &pos[ 1 ], 32, 232, 32 ) ;
		//NewPointView( &pos[ 2 ], 32, 32, 232 ) ;
		if ( BeltSpeedScale > 0.99F ) {
			if ( ( GV_Time % DIRECT_TICK( 22 ) ) == 0 ) {
				if ( GV_Time / DIRECT_TICK( 22 ) % 2 == 0 ) {
					SeSet( work, &pos[ 0 ], SD_A_BLTCNV1A ) ;
				} else {
					SeSet( work, &pos[ 0 ], SD_A_BLTCNV2A ) ;
				}
			} 
			if ( ( ( GV_Time - DIRECT_TICK( 15 ) ) % DIRECT_TICK( 22 ) ) == 0 ) {
				if ( ( GV_Time - DIRECT_TICK( 15 ) ) / DIRECT_TICK( 22 ) % 2 == 0 ) {
					SeSet( work, &pos[ 1 ], SD_A_BLTCNV1B ) ;
				} else {
					SeSet( work, &pos[ 1 ], SD_A_BLTCNV2B ) ;
				}
			} 
			if ( ( ( GV_Time - DIRECT_TICK( 30 ) ) % DIRECT_TICK( 22 ) ) == 0 ) {
				if ( ( GV_Time - DIRECT_TICK( 30 ) ) / DIRECT_TICK( 22 ) % 2 == 0 ) {
					SeSet( work, &pos[ 2 ], SD_A_BLTCNV1C ) ;
				} else {
					SeSet( work, &pos[ 2 ], SD_A_BLTCNV2C ) ;
				}
			} 
		} 
		len = lenmin[ 0 ] ;
		area = 0 ;
		for ( i = 1; i < 3; i ++ ) {
			if ( len > lenmin[ i ] ) {
				len = lenmin[ i ] ;
				area = i ;
			}
		}
		DG_COPY_VEC( &SePos, &pos[ i ] ) ;
	}
}

static	void	BCM_WorkDie( BCM_Work *work )
{
	GV_Free( BeltData ) ;
	BeltData = NULL ;
	PL_MoveFloorFunc = NULL ;
}

int		NewBeltConveyerManager( void )
{
	BCM_Work		*work ;

	work = &BcmWork ;
	GCL_GetOption( 'n' ) ;
	MAX_Belts = GCL_GetNextInt() ;
	BeltData = ( BELT_DATA * )GV_Malloc( sizeof( BELT_DATA ) * MAX_Belts ) ;
	ASSERT( BeltData != NULL ) ;
	GV_ZeroMemory( BeltData, sizeof( BELT_DATA ) * MAX_Belts ) ;
	N_Belts = 0 ;
	BeltStatus = 0 ;
	GV_ZeroMemory( work, sizeof( BCM_Work ) ) ;
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_OBJECT ) ;
	GV_InsertActorPriority( GV_ACTOR_MANAGER, &work->actor, 0 ) ;
	GV_SetActor( &work->actor, BCM_WorkAct, BCM_WorkDie ) ;
	GV_SetActorKillLevel( &work->actor, GV_KILL_LEVEL_NORMAL ) ;
	PL_MoveFloorFunc = BeltMoveFunc ;

	if ( GCL_GetOption( 'l' ) != NULL ) {
		work->lamp_name = GCL_GetNextInt() ;
		work->n_lamps = GCL_GetNextInt() ;
	}

	SendMessageToLamp( LAMP_RED, 0 ) ;

	BeltSpeedScale = 1.0F ;
	BeltSpeedScaleStep = 0.0F ;

	return 0 ;
}

int		NewSetBeltConveyerMode( void )
{	
	int		time, semode ;

	if ( GCL_GetOption( 'm' ) != NULL ) {
		if ( !( BeltStatus & BELT_STATE_STOP ) ) return 0 ;
		BeltStatus &= ~BELT_STATE_STOP ;
		if ( GCL_NextStr() != NULL ) {
			time = GCL_GetNextInt() * ( 300 / TIME_BASE ) ;
			if ( time == 0 ) {
				BeltSpeedScale = 1.0F ;				
				BeltSpeedScaleStep = 0.0F ;
				semode = 0 ;
			} else {
				BeltSpeedScale = 0.0F ;
				BeltSpeedScaleStep = 1.0F / ( float )time ;
				GM_SeSetMode( SD_A_BLTSTRT1, &SePos, GM_SEMODE_BOMB ) ;
				semode = 1 ;
			}
		} else {
			BeltSpeedScale = 1.0F ;				
			BeltSpeedScaleStep = 0.0F ;			
			semode = 0 ;
		}
		SendMessageToLamp( LAMP_RED, semode ) ;
	} else if ( GCL_GetOption( 's' ) != NULL ) {
		if ( BeltStatus & BELT_STATE_STOP ) return 0 ;
		if ( GCL_NextStr() != NULL ) {
			BeltStatus &= ~BELT_STATE_STOP ;			
			time = GCL_GetNextInt() * ( 300 / TIME_BASE ) ;
			if ( time == 0 ) {
				SendMessageToLamp( LAMP_BLUE, 0 ) ;
				BeltStatus |= BELT_STATE_STOP ;
				BeltSpeedScale = 0.0F ;
				BeltSpeedScaleStep = 0.0F ;
			} else {
				SendMessageToLamp( LAMP_YELLOW, 1 ) ;
				BeltSpeedScale = 1.0F ;
				BeltSpeedScaleStep = -1.0F / ( float )time ;
				GM_SeSetMode( SD_A_BLTSTOP1, &SePos, GM_SEMODE_BOMB ) ;
			}
		} else {
			SendMessageToLamp( LAMP_BLUE, 0 ) ;
			BeltStatus |= BELT_STATE_STOP ;
			BeltSpeedScale = 0.0F ;
			BeltSpeedScaleStep = 0.0F ;
		}
	}
	return 0 ;
}

/*--------------------------------------------------------------------------*/

/* ベルトコンベア登録 */
int		NewSetBeltConveyer( void )
{
	BELT_DATA		*b ;
	FVECTOR			step ;
	HZX_PAT			*pat ;
	HZX_PTP			*p0, *p1, *p2 ;
	int				patNo, ptpNo0, ptpNo1, speed ;

	ASSERT( N_Belts < MAX_Belts ) ;
	b = BeltData + N_Belts ;

	GCL_GetOption( 't' ) ;
	b->trap = GCL_GetNextInt() ;
	GCL_GetOption( 'r' ) ;
	patNo = GCL_GetNextInt() ;
	ptpNo0 = GCL_GetNextInt() ;
	ptpNo1 = GCL_GetNextInt() ;
	GCL_GetOption( 's' ) ;	
	speed = GCL_GetNextInt() ;
	pat = HZX_GetCurrentHzx()->def->patrols + patNo ;

	p0 = pat->points + ptpNo0 ;
	p1 = pat->points + ptpNo1 ;

	b->start = p0 ;
	b->point = p1 ;

	step.vx = p1->x - p0->x ;
	step.vy = p1->y - p0->y ;
	step.vz = p1->z - p0->z ;
	GV_LenVec3F( &step, &b->step, 0.0F, speed ) ;

	if ( GCL_GetOption( 'n' ) == NULL ) {
		if ( pat->n_points > ptpNo1 + 1 ) {
			p2 = p1 + 1 ;
			step.vx = p2->x - p1->x ;
			step.vy = p2->y - p1->y ;
			step.vz = p2->z - p1->z ;
			GV_LenVec3F( &step, &b->step2, 0.0F, speed ) ;
			b->next = p2 ;
		} else {
			b->next = NULL ;
		}
	} else {
		patNo = GCL_GetNextInt() ;
		ptpNo0 = GCL_GetNextInt() ;		
		pat = HZX_GetCurrentHzx()->def->patrols + patNo ;
		p2 = pat->points + ptpNo0 ;
		step.vx = p2->x - p1->x ;
		step.vy = p2->y - p1->y ;
		step.vz = p2->z - p1->z ;
		GV_LenVec3F( &step, &b->step2, 0.0F, speed ) ;
		b->next = p2 ;
	}
	b->chara = GCL_GetOptionValue( 'c', 0 ) ;
	b->item = GCL_GetOptionValue( 'i', -1 ) ;
	b->area = GCL_GetOptionValue( 'a', 0 ) ;

	N_Belts ++ ;
	return 0 ;
}

/*--------------------------------------------------------------------------*/

/* ベルトコンベア状態取得 */
/* ０：稼動中、１：停止中 */
int		PL_COM_GetBeltConvStatus( void )
{
	return ( BeltStatus & BELT_STATE_STOP ) ;	
}
