//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   elevator.c
   エレベータ
   1ステージに１つのみ
   2001/03/01 M.Sonoyama
   $Id: elevator.c,v 1.4 2002/11/23 12:24:50 Yoshizawa1 Exp $
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
#include	"camera.h"

#include	"../raiden/pl_define.h"
#include	"../raiden/pl_work.h"
#include	"../raiden/pl_inline.h"

extern	void	*PL_DamageCamera( int, long64 ) ;

/*----------------------------------------------------------------*/

extern	void 			*NewMakeObjectBoundHazard2( HZX_GROUP_ID hzx_id, DG_OBJS *objs, 
												   int seg_atr, int flr_atr, int disp ) ;
extern	void			PL_UpdateObjectBoundHazard( void *work, FMATRIX *world ) ;

extern	int				DG_MakePreshade( DG_OBJS *objs, LIT_DEF *lit_def ) ;
extern	void			DG_FreePreshade( DG_OBJS *objs ) ;

static 	int 			Now = 0, StartNow ;
static	int				ElvFlag = 0 ;

/*----------------------------------------------------------------*/

typedef	struct	{
	DG_OBJS			*objs ;
	float			length ;
	float			speed ;
	int				time ;
	int				open_time ;
	void			*objhzx ;
} Door ;

typedef	struct	{
	GV_ACT			actor ;
	Door			door1 ;
	Door			door2 ;
	HZX_D_SEGMENT	*segment ;
	int				name ;
	int				trap ;
	int				panel_trap ;
	int				map_id ;
	HZX_GROUP_ID	hzx_id ;
	int				status ;
	int				time ;
	int				open_time ;
	int				close_time ;
	int				open_proc[ 2 ] ;
	int				close_proc[ 2 ] ;
	int				panel_proc ;
	int				npc_name ;
	int				panel_name ;
	int				lamp_name ;
	int				type ;
	R_INTRPT		r_intrpt ;
} Work ;

enum {
	ELV_STATE_CLOSE =		0x0000,
	ELV_STATE_OPENING =		0x0001,
	ELV_STATE_CLOSING =		0x0002,
	ELV_STATE_OPEN =		0x0004,
	ELV_STATE_MOVE =		0x0008,
} ;

enum {
	ELV_FLAG_NONE			=		0x0000,
	ELV_FLAG_ALWAYSOPEN		=		0x0001,
	ELV_FLAG_LOADCLOSE 		=		0x0002,
	ELV_FLAG_STAGESTART 	=		0x0004,
	ELV_FLAG_TALES_LOADCLOSE 	=	0x0008,
} ;

enum {
	ELV_MSG_OPEN =			0,
	ELV_MSG_CLOSE =			1,
	ELV_MSG_STAGESTART =	2,
	ELV_MSG_ALWAYSOPEN =	3,
	ELV_MSG_LOADCLOSE =		4,

	ELV_MSG_TALES_STAGESTART =	5,
	ELV_MSG_TALES_LOADCLOSE =	6,
} ;

/* ドアランプメッセージ */
enum {
	LAMP_RED	=		1,
	LAMP_BLINK	=		2,
	LAMP_BLUE 	=		3,
	LAMP_NOUSE 	=		4,
	LAMP_YELLOW	=		5,
	LAMP_YELLOW_BLINK =	6,
} ; 

static	Work			*ElvWork = NULL ;
static	GM_CameraSet	*ElvCamera = NULL ;
static	FVECTOR			ElvCameraPosDef ;
static	FVECTOR			ElvCameraTrgDef ;

static	void			OpenDoorFast( Work *work ) ;

/*----------------------------------------------------------------*/

static	void	SeSetScale( int se, float scale )
{
	int			vol, pan ;
	FVECTOR		*pos ;
   float bp_angle;

	pos = ( FVECTOR * )ElvWork->door1.objs->world.m[ 3 ] ;
	/* あんまりとおくのときは鳴らさない */
	if ( GM_CheckPlayerStatus( PLAYER_IN_THE_WATER ) ||
		 GV_VecLen3F2( pos, &GM_GetCurrentCamera( 0 )->target ) > 8000.0F ) {
		return ;
	}
	GM_SeGetVolPan( pos, GM_SEMODE_NORMAL, &vol, &pan, &bp_angle ) ;
	vol = ( int )( ( float )vol * scale ) ;	
	if ( vol > GM_MAX_VOL ) vol = GM_MAX_VOL ;
	GM_SeSet3D( pan, vol, se, bp_angle ) ;
}

static	void	SeSet( int se )
{
	SeSetScale( se, 2.0F ) ;
	//GM_SeSetMode( se, ( FVECTOR * )ElvWork->door1.objs->world.m[ 3 ], GM_SEMODE_NORMAL ) ;
	//GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, se ) ;
}

static	void	LocalSendMessage( int to, int n, int v1, int v2, int v3 )
{
	GV_MSG		msg ;
	int			message[ 4 ] ;

	message[ 0 ] = v1 ;
	message[ 1 ] = v2 ;
	message[ 2 ] = v3 ;

	msg.address = to ;
	msg.message = message ;
	msg.message_len = n ;
	GV_SendMessage( &msg ) ;
}

static	void	SendMessageToPanel( Work *work, int v1, int v2 )
{
	if( work->lamp_name != 0 ) LocalSendMessage( work->lamp_name, 2, v1, v2, 0 ) ;
}

static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, code ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	if ( n_msg == 0 ) return ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		switch( code ) {
		case ELV_MSG_OPEN :
			if ( work->status == ELV_STATE_CLOSE ) {
				work->time = 0 ;
				work->open_time = msg->message[ 1 ] ;
				if ( work->open_time < 0 ) {
					if ( GV_Time & 1 ) 		work->open_time = 6 * 300 / TIME_BASE ;
					else if ( GV_Time & 2 ) work->open_time = 10 * 300 / TIME_BASE ;
					else if ( GV_Time & 4 )	work->open_time = 3 * 300 / TIME_BASE ;
					else					work->open_time = 0 ;
				}
				if ( work->open_time <= 0 ) work->open_time = DIRECT_TICK( 48 ) + 1 ;
				/* 到着音対策 */
#if 0
				if ( !( ElvFlag & ELV_FLAG_STAGESTART ) ) {
					if ( work->open_time < DIRECT_TICK( 75 + 48 ) + 1 ) {
						work->open_time = DIRECT_TICK( 75 + 48 ) + 1 ;
					}
				}
#endif
				SendMessageToPanel( work, LAMP_YELLOW_BLINK, -1 ) ;
			} else if ( work->status == ELV_STATE_OPENING ) {
				/* 二度押し */
				work->time = 0 ;
				if ( work->open_time > DIRECT_TICK( 75 + 48 ) + 1 ) {
					work->open_time = DIRECT_TICK( 75 + 48 ) + 1 ;
					SendMessageToPanel( work, LAMP_YELLOW_BLINK, -1 ) ;
				}
			} else if ( work->status == ELV_STATE_CLOSING ) {
				/* 閉じ中 */
				work->time = 0 ;
				work->open_time = 1 ;
			} else if ( work->status == ELV_STATE_OPEN ) {
				/* もう開いている */
				work->time = work->close_time ;
				SendMessageToPanel( work, LAMP_BLUE, -1 ) ;
				break ;
			}
			work->open_proc[ 0 ] = msg->message[ 2 ] ;
			work->open_proc[ 1 ] = msg->message[ 3 ] ;
			work->status = ELV_STATE_OPENING ;
			break ;
		case ELV_MSG_TALES_LOADCLOSE :
			ElvFlag |= ELV_FLAG_TALES_LOADCLOSE ;
		case ELV_MSG_LOADCLOSE :
			ElvFlag |= ELV_FLAG_LOADCLOSE ;
		case ELV_MSG_CLOSE :
			work->close_proc[ 0 ] = msg->message[ 1 ] ;
			work->close_proc[ 1 ] = msg->message[ 2 ] ;
			if ( work->close_proc[ 0 ] > 0 ) GM_ExecProc( work->close_proc[ 0 ], NULL ) ;
			HZX_DynamicSegmentResetSkip( work->segment ) ;
			work->status = ELV_STATE_CLOSING ;
			SeSet( SD_P_ELEOPN01 ) ;
			SendMessageToPanel( work, LAMP_YELLOW_BLINK, -1 ) ;
			break ;
		case ELV_MSG_STAGESTART :
		case ELV_MSG_TALES_STAGESTART :
			ElvCamera->on = 1 ;
			ElvFlag |= ELV_FLAG_STAGESTART ;
			GM_SetCameraInterpMode( ElvCamera, GM_CAM_INTERP_QUICK,
								    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;		  
			GM_ChangeCamera( 0 ) ;
			//SeSet( SD_A_EVCALL01 ) ;
			//SeSet( SD_A_EVSTOP01 ) ;
			if ( code == ELV_MSG_TALES_STAGESTART ) {
				GM_SdSet( SD_A_ELEMOV12 ) ;
			} else {
				GM_SdSet( SE_JOUCHUU_OFF ) ;
			}
			PL_DamageCamera( DIRECT_TICK( 12 ), I64(0) ) ;
			SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
			//NewPadVibration2( GV_StrCode( "core_ele_03" ), 0 ) ;
			/* プレイヤーを無理矢理エレベータパネル状態にする */
			if ( GM_PlayerControl != NULL ) {
				LocalSendMessage( GM_PlayerControl->name, 3, PL_MSG_ELEVATOR, GM_STRCODE_ENTER, -1 ) ;
			}
			break ;
		case ELV_MSG_ALWAYSOPEN :
			ElvFlag |= ELV_FLAG_ALWAYSOPEN ;
			/* 瞬時に開ける */
			OpenDoorFast( work ) ;
			SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
			break ;
		default :
		  ;
		}
		msg ++ ;
	}
}

static	int		CheckEnemy( Work *work, int mode )
{
	HOMING_TRG		*hom ;

	hom = GM_GetHoming() ;
	while( hom != NULL ) {
		if ( ( hom->status & ( HOMING_ENEMY | HOMING_DEAD ) ) &&
			 !( hom->status & HOMING_UNREAL ) ) {
			if ( GM_CheckInsideTrap( hom->ctrl, &work->trap, 1, 0 ) ) {
				printf( "elevator : enemy inside\n" ) ;
				return 1 ;
			}
		}
		hom = hom->next ;
	}

	if ( mode == 1 ) return 0 ;

	/* NPCチェック */
	if ( work->npc_name != 0 ) {
		CONTROL		*ctrl ;

		ctrl = GM_SearchWhere( work->npc_name ) ;
		if ( ctrl == NULL ) return 0 ;
		if ( HZX_CheckInsideTrap( ctrl->hzx_id, &ctrl->mov, work->trap ) ) {
			return 1 ;
		}
	}
	return 0 ;
}

static	int		CheckPlayer( Work *work )
{
	if ( GM_PlayerControl == NULL ) return 0 ;
	if ( GM_CheckInsideTrap( GM_PlayerControl, &work->trap, 1, 0 ) ) {
		return 1 ;
	}
	return 0 ;
}

/*----------------------------------------------------------------*/

static	void	OpenDoorFast( Work *work ) 
{
	Door		*dr ;

	work->open_time = 0 ;

	dr = &work->door1 ;
	dr->objs->world.m[ 3 ][ 0 ] -= dr->speed * ( dr->open_time - dr->time ) ;	
	PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	dr->time = dr->open_time ;

	dr = &work->door2 ;
	dr->objs->world.m[ 3 ][ 0 ] -= dr->speed * ( dr->open_time - dr->time ) ;	
	PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	dr->time = dr->open_time ;

	HZX_DynamicSegmentSetSkip( work->segment ) ;
	work->open_proc[ 0 ] = work->open_proc[ 1 ] = 0 ;
	work->time = work->close_time ;
	work->status = ELV_STATE_OPEN ;
	work->r_intrpt.status |= ROOT_INTRPT_OPEN ;
	work->r_intrpt.status &= ~ROOT_INTRPT_CLOSE ;
}

static	void	OpenDoor( Work *work ) 
{
	Door		*dr ;
	int			opened = 0 ; 

	if ( work->open_time > 0 ) {
		/* 到着待ち */
		//printf( "wait %d\n", work->open_time ) ;
		if ( -- work->open_time == 0 ) {
			if ( work->open_proc[ 0 ] > 0 ) {
				GM_ExecProc( work->open_proc[ 0 ], NULL ) ;
			}
			//SeSet( SD_A_EVCALL01 ) ;
			SeSet( SD_P_ELEOPN01 ) ;
			SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
		} else if ( work->open_time == DIRECT_TICK( 48 + 75 ) ) {
			/* エレベータ到着 */
			if ( !( ElvFlag & ELV_FLAG_STAGESTART ) ) {
				//GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_A_ELESTP01 ) ;
				SeSetScale( SD_A_ELESTP01, 2.5F ) ;
			}
		} else if ( work->open_time == DIRECT_TICK( 8 ) ) {
			/* 到着音 */
			if ( !( ElvFlag & ELV_FLAG_STAGESTART ) ) {
				SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
				SeSet( SD_A_ELECHM01 ) ;
			}
		}
		return ;
	}

	dr = &work->door1 ;
	if ( dr->time == dr->open_time ) {
		opened |= 1 ;
	} else {
		dr->time ++ ;
		dr->objs->world.m[ 3 ][ 0 ] -= dr->speed ;
		PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	}
	dr = &work->door2 ;
	if ( dr->time == dr->open_time ) {
		opened |= 2 ;
	} else {
		dr->time ++ ;
		dr->objs->world.m[ 3 ][ 0 ] -= dr->speed ;
		PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	}
	if ( opened == 3 ) {
		if ( work->open_proc[ 1 ] > 0 ) GM_ExecProc( work->open_proc[ 1 ], NULL ) ;
		HZX_DynamicSegmentSetSkip( work->segment ) ;
		work->open_proc[ 0 ] = work->open_proc[ 1 ] = 0 ;
		work->time = work->close_time ;
		work->status = ELV_STATE_OPEN ;
		SeSet( SD_P_ELEOPED1 ) ;
		work->r_intrpt.status |= ROOT_INTRPT_OPEN ;
		work->r_intrpt.status &= ~ROOT_INTRPT_CLOSE ;
	}
}

static	void	CloseDoor( Work *work ) 
{
	Door		*dr ;
	int			closed = 0 ; 

	if ( !( ElvFlag & ELV_FLAG_LOADCLOSE ) ) {
		/* ロードクローズのときは強制クローズ */
		if ( ( CheckPlayer( work ) && CheckEnemy( work, 1 ) ) || /* プレイヤー内＆敵内 */
			 ( !CheckPlayer( work ) && CheckEnemy( work, 0 ) ) ) { 	/* プレイヤー外＆敵ＯＲＮＰＣ内 */
			work->status = ELV_STATE_OPENING ;
			ElvFlag &= ~ELV_FLAG_LOADCLOSE ;
			work->open_time = 0 ;
			SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
			return ;
		}
	}

	dr = &work->door1 ;
	if ( dr->time == 0 ) {
		closed |= 1 ;
	} else {
		dr->time -- ;
		dr->objs->world.m[ 3 ][ 0 ] += dr->speed ;
		PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	}
	dr = &work->door2 ;
	if ( dr->time == 0 ) {
		closed |= 2 ;
	} else {
		dr->time -- ;
		dr->objs->world.m[ 3 ][ 0 ] += dr->speed ;
		PL_UpdateObjectBoundHazard( dr->objhzx, &dr->objs->world ) ;
	}
	if ( closed == 3 ) {
		SeSet( SD_P_ELECLS01 ) ;
		SendMessageToPanel( work, LAMP_YELLOW, 0 ) ;
		if ( !( ElvFlag & ELV_FLAG_LOADCLOSE ) ) {
			if ( work->close_proc[ 1 ] > 0 ) GM_ExecProc( work->close_proc[ 1 ], NULL ) ;
			work->close_proc[ 0 ] = work->close_proc[ 1 ] = 0 ;
			work->time = 0 ;
			work->status = ELV_STATE_CLOSE ;
		} else {
			work->time = 0 ;
			work->status = ELV_STATE_MOVE ;
		}
		work->r_intrpt.status &= ~ROOT_INTRPT_OPEN ;
		work->r_intrpt.status |= ROOT_INTRPT_CLOSE ;
		if ( CheckPlayer( work ) ) {
			NewPadVibration2( GV_StrCode( "core_ele_01" ), 0 ) ;
		}
	}
}

static	void	OpenedDoor( Work *work )
{
	if ( !( ElvFlag & ELV_FLAG_ALWAYSOPEN ) && 
		 !CheckEnemy( work, 0 ) && !CheckPlayer( work ) ) {
		-- work->time ;
//		printf( "close wait %d\n", work->time ) ;
	} else {
		work->time = work->close_time ;
		SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
	}
	if ( work->time <= DIRECT_TICK( 60 ) ) {
		SendMessageToPanel( work, LAMP_YELLOW_BLINK, -1 ) ;
	}
	if ( work->time <= 0 ) {
		work->time = 0 ;
		work->status = ELV_STATE_CLOSING ;
		SeSet( SD_P_ELEOPN01 ) ;
		HZX_DynamicSegmentResetSkip( work->segment ) ;
		work->r_intrpt.status &= ~ROOT_INTRPT_OPEN ;
		work->r_intrpt.status |= ROOT_INTRPT_CLOSE ;
	}
}

static	void	LoadMove( Work *work )
{
	++ work->time ;
	if ( work->time == DIRECT_TICK( 48 ) ) {
		if ( ElvFlag & ELV_FLAG_TALES_LOADCLOSE ) {
			SeSet( SD_A_ELEMOV11 ) ;
		} else {
			SeSet( SD_P_ELEMOV01 ) ;
		}
		PL_DamageCamera( DIRECT_TICK( 12 ), I64(0) ) ;
		/* 振動 */
		NewPadVibration2( GV_StrCode( "core_ele_02" ), 0 ) ;
	} else if ( work->time == DIRECT_TICK( 80 ) ) {
		//SeSet( SD_A_EVMOTOR1 ) ;
	} else if ( work->time == DIRECT_TICK( 96 ) ) {
		if ( work->close_proc[ 1 ] > 0 ) GM_ExecProc( work->close_proc[ 1 ], NULL ) ;
		work->close_proc[ 0 ] = work->close_proc[ 1 ] = 0 ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	CheckMessage( work ) ;
	if ( work->status == ELV_STATE_OPENING ) {
		OpenDoor( work ) ;
	} else if ( work->status == ELV_STATE_CLOSING ) {
		CloseDoor( work ) ;
	} else if ( work->status == ELV_STATE_OPEN ) {
		OpenedDoor( work ) ;
	} else if ( work->status == ELV_STATE_MOVE ) {
		LoadMove( work ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	DieDoor( Door *dr )
{
	DG_FreePreshade( dr->objs ) ;
	DG_DequeueObjs( dr->objs ) ;
	DG_FreeObjs( dr->objs ) ;
}

static	void	Die( Work *work )
{
	DieDoor( &work->door1 ) ;
	DieDoor( &work->door2 ) ;
	GM_FreeRouteIntrpt( &work->r_intrpt ) ;	
	HZX_RemoveDynamicSegment( work->segment ) ;
	ElvWork = NULL ;
}

/*----------------------------------------------------------------*/

static	int		ElvRotVy ;

static	int		InitDoorObject( Work *work, Door *dr, int map_id, int hzx_id, char c, int seNo )
{
	int			model ;
	FVECTOR		mov ;
	SVECTOR		rot ;
	DG_DEF		*def ;

	GCL_GetOption( c ) ;
	model = GCL_GetNextInt() ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( model, 'k' ) ) ;
	if ( def == NULL ) return -1 ;
	dr->objs = DG_MakeObjs( def, DG_FLAG_TEXT|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT, 0 ) ;
	if ( dr->objs == NULL ) return -1 ;
	DG_QueueObjs( dr->objs ) ;
	GM_GroupObjs( dr->objs, map_id ) ;
	
	PL_GetNextFV( &mov ) ;
	GV_SetVec3( &rot, 0, GCL_GetNextInt(), 0 ) ;
	DG_SetPos2( &mov, &rot ) ;
	DG_PutObjs( dr->objs ) ;
	DG_MakePreshade( dr->objs, GM_GetMap( map_id )->light ) ;

	ElvRotVy = rot.vy ;

	dr->length = ( float )GCL_GetNextInt() ;
	dr->open_time = GCL_GetNextInt() ;
	dr->speed = dr->length / ( float )dr->open_time ;
	dr->time = 0 ;

	dr->objhzx = NewMakeObjectBoundHazard2( hzx_id, dr->objs, 
										    ( HZX_SEG_ATR_ALL & ~( HZX_SEG_NO_BULLET | HZX_SEG_NO_MISSILE |
																   HZX_SEG_NO_RECOIL | HZX_SEG_NO_KNOCK_SE |
																   HZX_SEG_NO_DISP_RADAR |
																   HZX_SEG_NO_ENEMY_EYES ) ) |
										    HZX_SEG_DOOR | seNo,
										    HZX_FLOOR_ATR_ALL & ~( HZX_FLOOR_NO_BULLET |
																   HZX_FLOOR_NO_MISSILE |
																   HZX_FLOOR_NO_ENEMY_EYES |
																   HZX_FLOOR_IK | HZX_FLOOR_STEP ), 1 ) ;
	if ( dr->objhzx == NULL ) return -1 ;
	return 0 ;
}

static	int		GetResources( Work *work, int name, int where )
{
	int				map_id ;
	HZX_GROUP_ID	hzx_id ;
	IVECTOR			iv[ 2 ] ;
	int				seNo ;

	work->name = name ;

	seNo = GCL_GetOptionValue( 'N', 0 ) ;
	seNo = ( seNo & 0xf ) << 28 ;

	if ( GCL_GetOption( 'm' ) != NULL ) where = GCL_GetNextInt() ;
	work->map_id = map_id = GM_GetMapID( where ) ;
	work->hzx_id = hzx_id = GM_GetHzxGroupID( map_id ) ;

	if ( InitDoorObject( work, &work->door1, map_id, hzx_id, 'd', seNo ) < 0 ) return -1 ;
	if ( InitDoorObject( work, &work->door2, map_id, hzx_id, 'O', seNo ) < 0 ) return -1 ;

	GCL_GetOption( 'w' ) ;
	GCL_GetNextIV( ( int * )&iv[ 0 ] ) ;
	GCL_GetNextIV( ( int * )&iv[ 1 ] ) ;
	iv[ 0 ].vw = iv[ 1 ].vw = GCL_GetNextInt() ;
	work->segment = HZX_AddDynamicSegment( hzx_id, &iv[ 0 ], &iv[ 1 ], 
										   HZX_SEG_NO_BULLET | HZX_SEG_NO_C4 | HZX_SEG_NO_MISSILE |
										   HZX_SEG_NO_KNOCK_SE ) ;

	/* ゾーン遮断 */
	{
		HZX_ZONE_ADD	zone[ 2 ] ;
		HZX_ZON			*z[ 2 ] ;
		FVECTOR			pos ;
		SVECTOR			rot ;

		rot.vy = ElvRotVy ;
		GV_MatToVec( &work->door1.objs->world, &pos ) ;
		HZX_GetInterruptZone( &pos, rot.vy - 1024, 250, &zone[ 0 ] ) ;
		z[ 0 ] = HZX_GetZoneFromAdd( zone[ 0 ] ) ;
		z[ 1 ] = HZX_GetZoneFromAdd( zone[ 1 ] ) ;
		if ( z[ 0 ] != NULL ) {
			z[ 0 ]->flag |= HZX_ZONE_SLIDEDOOR ;
		}
		if ( z[ 1 ] != NULL ) {
			z[ 1 ]->flag |= HZX_ZONE_SLIDEDOOR ;
		}
		GM_SetRouteIntrpt( &work->r_intrpt, zone[ 0 ], zone[ 1 ], &pos, 
						   rot.vy - 1024, ROOT_INTRPT_ELV | ROOT_INTRPT_CLOSE, 0 ) ;
		GM_PutRoteIntrpt( &work->r_intrpt ) ;
	}

	work->close_time = GCL_GetOptionValue( 'c', 10 ) * ( 300 / TIME_BASE ) ;
	GCL_GetOption( 't' ) ;
	work->trap = GCL_GetNextInt() ;
	work->panel_trap = GCL_GetNextInt() ;
	work->panel_proc = GCL_GetOptionValue( 'p', 0 ) ;
	work->npc_name = GCL_GetOptionValue( 'n', 0 ) ;
	work->panel_name = GCL_GetOptionValue( 'L', 0 ) ;
	work->lamp_name = GCL_GetOptionValue( 'M', 0 ) ;
	work->type = GCL_GetOptionValue( 'Y', 0 ) ;

	/* カメラ起動 */
	ElvCamera = NewProgramCamera( GV_StrCode( "エレベータパネルカメラ" ), 0,
								  GM_CAMERA_PROG1, 0 ) ;
	GM_SetCameraType( ElvCamera, GM_CAM_TYPE_CAMERA_AND_ROTATE,
					  CAM_FLAG_DIR_TRACE | CAM_FLAG_PAD_ADJUST ) ;
	GCL_GetOption( 'A' ) ;
	PL_GetNextFV( &ElvCamera->position ) ;
	PL_GetNextFV( &ElvCamera->target ) ;

	DG_COPY_VEC( &ElvCameraPosDef, &ElvCamera->position ) ;
	DG_COPY_VEC( &ElvCameraTrgDef, &ElvCamera->target ) ;

	GM_CameraMakeRotate( &ElvCamera->position, &ElvCamera->target,
						 &ElvCamera->rotate, &ElvCamera->track ) ;
	GM_SetCameraAngle( ElvCamera, 2.0F ) ;
	GM_SetCameraInterpMode( ElvCamera, GM_CAM_INTERP_INTO_SUBJECT,
						    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	ElvCamera->on = 0 ;

	/* 最初に光っている階数 */
	Now = GCL_GetOptionValue( 'f', 0 ) ;
	StartNow = Now ;

	work->status = ELV_STATE_CLOSE ;
	SendMessageToPanel( work, LAMP_YELLOW, 0 ) ;
	ElvFlag = 0 ;

	return 0 ;
}

/*----------------------------------------------------------------*/

/* エレベータ起動 */
void		*NewElevator( int name, int where )
{
	Work			*work ;

	ElvWork = NULL ;
	/* カメラ設定前がいいのでUSERレベル */
	work = ( Work * )GV_NewActorPrio( GV_ACTOR_USER, sizeof( Work ), 254 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		ElvWork = work ;
	}
	return work ;
}

/*----------------------------------------------------------------*/

/* エレベータ状態取得 */
int			PL_COM_GetElevatorStatus( void )
{
	return ElvWork->status ;
}

/*----------------------------------------------------------------*/

/* プレイヤー */

/*----------------------------------------------------------------*/

static	PL_PluginSet	PluginSet ;

static	void	OperateElevatorPanel( PlayerWork *work, int time ) ;
static	void	ElevatorDamageCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* パネル操作終了 */
static	void	EndElevatorPanel( PlayerWork *work ) 
{
	GM_ResetMenuStatus( MENU_MENU_OFF ) ;
	//DG_VisibleObjsChanl( work->body.objs, 0 ) ;
	PL_LeaveSubject( work ) ;
	if ( ElvCamera->on ) {
		ElvCamera->on = 0 ;
		/* 補完関数を通常に戻す */
		GM_SetCameraInterpMode( ElvCamera, GM_CAM_INTERP_INTO_SUBJECT,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
		/* カメラ位置を初期状態に戻す */
		DG_COPY_VEC( &ElvCamera->position, &ElvCameraPosDef ) ;
		DG_COPY_VEC( &ElvCamera->target, &ElvCameraTrgDef ) ;

		GM_ChangeCamera( 0 ) ;
		/* サイト表示再開 */
		GM_ResetSightStatus( SGT_Invisible ) ;
	}
	if ( work->dmg_callback == ElevatorDamageCallback ) {
		work->dmg_callback = NULL ;
	}
	if ( work->current_mar != work->org_motion ) {
		PL_ReturnMotionArc( work ) ;
	}
	ElvFlag &= ~ELV_FLAG_STAGESTART ;
	if ( Now != StartNow ) {
		PL_SendMessage( ElvWork->panel_name, &StartNow, 1 ) ;
	}
}

/* パネル操作 */
static	int	Panel( Work *work, int press ) 
{
	//int			i ;
	int			max ;

	max = ( ElvWork->type == 0 ) ? 2 : 1 ;
	if ( press & PAD_U ) {
		Now -- ;
		if ( Now < 0 ) Now = max ;
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_CUR01 ) ;
	} else if ( press & PAD_D ) {
		Now ++ ;
		if ( Now > max ) Now = 0 ;
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_CUR01 ) ;
	}
#if 0
	for ( i = 0; i < 3; i ++ ) {
		MENU_Locate( 300, 160 + i * 20, MENU_MODE_LEFT ) ;
		if ( i == Now ) MENU_Color( 232, 32, 32, 128 ) ;
		else		    MENU_Color( 255, 255, 255, 128 ) ;
		if ( i != 3 ) MENU_Printf( "%d F", i + 1 ) ;
		else		  MENU_Printf( "Cancel" ) ;
	}		
	MENU_ResetColor() ;
#endif
	/* パネル表示キャラにメッセージ */
	PL_SendMessage( ElvWork->panel_name, &Now, 1 ) ;

	if ( press & ( PAD_OK | PAD_X ) ) return Now ;
	if ( press & PAD_CANCEL ) return 3 ;
	return -1 ;
}

static	void	ExecProc( int proc, int v )
{
	GCL_ARGS	args ;

	if ( proc <= 0 ) return ;
	args.argc = 1 ;
	args.argv = &v ;
	GM_ExecProc( proc, &args ) ;
}

/* パネル操作 */
static	void	OperateElevatorPanel( PlayerWork *work, int time )
{
	int			p ;

	if ( time == 0 ) {
		ElvCamera->on = 1 ;
		GM_ChangeCamera( 0 ) ;
		PL_SetAction( work, PL_MotionSet->change[ Mstand ], 6 ) ;
		work->control.turn.vx = work->control.rot.vx = 0 ;
		work->control.turn.vz = work->control.rot.vz = 0 ;
		work->control.turn.vy = GV_VecDir2FromTo( &ElvCamera->position, &ElvCamera->target ) ;
		work->control.rot.vy = work->control.turn.vy ;
		GM_CameraDir.vy = work->camdir.vy = work->camdir.pad = work->control.rot.vy ;
		GM_CameraDir.vx = 0 ;
		PL_IntoSubject( work ) ;
		/* 即消し */
		DG_InvisibleObjsChanl( work->body.objs, 0 ) ;
		/* トラップ処理の関係で */
		UnsetStatus( PLAYER_WATCH ) ;
		/* サイト表示ＯＦＦ */
		GM_SetSightStatus( SGT_Invisible ) ;
	}

	if ( time == 2 * TIME_BASE && !( ElvFlag & ELV_FLAG_STAGESTART ) ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDISP02 ) ;
	}

	//DG_InvisibleObjsChanl( work->body.objs, 0 ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | 
			 FLAG_CANNOT_CHANGE | FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT |
			 FLAG_NO_STEP | FLAG_FORCE ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;

	/* 開始２フレームは何もさせない（メッセージ対処） */
	if ( time < 2 * TIME_BASE ) return ;

	if ( work->data == 0 &&
		 ( GM_AlertMode == ALERT_MODE_ALERT || 
		   GM_CheckGameStatus( STATE_ENE_SIGHTIN ) ||
		   CheckEnemy( ElvWork, 1 ) ) ) {
		EndElevatorPanel( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	}

	/* ステージ開始時は操作不能 */
	if ( ElvFlag & ELV_FLAG_STAGESTART ) {
		if ( Now != StartNow ) {
			PL_SendMessage( ElvWork->panel_name, &StartNow, 1 ) ;
		}
		work->control.turn.vx = work->control.rot.vx = 0 ;
		work->control.turn.vz = work->control.rot.vz = 0 ;
		work->control.turn.vy = GV_VecDir2FromTo( &ElvCamera->position, &ElvCamera->target ) ;
		work->control.rot.vy = work->control.turn.vy ;
		GM_CameraDir.vy = work->camdir.vy = work->camdir.pad = work->control.rot.vy ;
		GM_CameraDir.vx = 0 ;
		/* 到着音 */
		if ( time == TIME_BASE * 72 ) {
			SeSet( SD_A_ELECHM01 ) ;
		}
		if ( time == TIME_BASE * 8 ) {
			NewPadVibration2( GV_StrCode( "core_ele_03" ), 0 ) ;
		}
		return ;
	}

	{
		int		status_buf ;
		int		pressure_buf[ 4 ] ;
		int		left_buf[ 2 ], left_use ;

		/* 右スティックで主観カメラ制御 */
		status_buf = ( work->pad->status & PAD_UDLR ) ;
		pressure_buf[ 0 ] = work->pad->pressure[ PAD_PRESS_U ] ;
		pressure_buf[ 1 ] = work->pad->pressure[ PAD_PRESS_D ] ;
		pressure_buf[ 2 ] = work->pad->pressure[ PAD_PRESS_L ] ;
		pressure_buf[ 3 ] = work->pad->pressure[ PAD_PRESS_R ] ;
		left_buf[ 0 ] = work->pad->left_dx ;
		left_buf[ 1 ] = work->pad->left_dy ;
		left_use = ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) ;

		work->pad->status &= ~PAD_UDLR ;
		work->pad->pressure[ PAD_PRESS_U ] = 0 ;
		work->pad->pressure[ PAD_PRESS_D ] = 0 ;
		work->pad->pressure[ PAD_PRESS_L ] = 0 ;
		work->pad->pressure[ PAD_PRESS_R ] = 0 ;
		work->pad->left_dx = work->pad->right_dx ;
		work->pad->left_dy = work->pad->right_dy ;
		work->pad->analog_input &= ~GV_PAD_ANALOG_L_USE ;
		if ( work->pad->analog_input & GV_PAD_ANALOG_R_USE ) {
			work->pad->analog_input |= GV_PAD_ANALOG_L_USE ;
			if ( work->pad->right_dx < 128 - ANALOG_MARGIN ) work->pad->status |= PAD_L ;
			else if ( work->pad->right_dx > 128 + ANALOG_MARGIN ) work->pad->status |= PAD_R ;
			if ( work->pad->right_dy < 128 - ANALOG_MARGIN ) work->pad->status |= PAD_U ;
			else if ( work->pad->right_dy > 128 + ANALOG_MARGIN ) work->pad->status |= PAD_D ;
		}
		
		GM_SubjectHMaxTmp[ 0 ] = 480 ;
		GM_SubjectHMaxTmp[ 1 ] = -480 ;
		GM_SubjectVMaxTmp[ 0 ] = 480 ;
		GM_SubjectVMaxTmp[ 1 ] = -480 ;
		PL_SubjectTurn( work ) ;

		work->pad->status |= status_buf ;
		work->pad->pressure[ PAD_PRESS_U ] = pressure_buf[ 0 ] ;
		work->pad->pressure[ PAD_PRESS_D ] = pressure_buf[ 1 ] ;
		work->pad->pressure[ PAD_PRESS_L ] = pressure_buf[ 2 ] ;
		work->pad->pressure[ PAD_PRESS_R ] = pressure_buf[ 3 ] ;
		work->pad->left_dx = left_buf[ 0 ] ;
		work->pad->left_dy = left_buf[ 1 ] ;
		work->pad->analog_input &= ~GV_PAD_ANALOG_L_USE ;
		work->pad->analog_input |= left_use ;
	}

	p = Panel( ElvWork, work->pad->press ) ;
	switch( p ) {
	case 0 :
	case 1 :
	case 2 :	
		/* カメラ回転を戻す */
		work->control.turn.vy = GV_VecDir2FromTo( &ElvCameraPosDef, &ElvCameraTrgDef ) ;
		work->control.rot.vy = work->control.turn.vy ;
		work->camdir.vx = GM_CameraDir.vx = 0 ;
		GM_CameraDir.vy = work->camdir.vy = work->control.rot.vy ;
		ExecProc( ElvWork->panel_proc, p ) ;
		/* この瞬間から無敵 */
		if ( StartNow != Now ) {
			PL_SetInvincible( work, 0 ) ;
			SetStatus( PLAYER_PAD_OFF ) ;
			HZX_DynamicSegmentResetSkip( ElvWork->segment ) ;
			work->data = 1 ;
		}
		break ;
	case 3 :
		EndElevatorPanel( work ) ;
		PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		return ;
	default :
	  ;
	}
}

/* ダメージコールバック */
static	void	ElevatorDamageCallback( PlayerWork *work, TARGET *off, TARGET *def )
{
	EndElevatorPanel( work ) ;
}

/* モードセット */
static	int		SetElevatorPanelAct( PlayerWork *work, GV_MSG *msg, int len )
{
	int			dir ;

	/* 無効状態チェック */
	if ( !( ElvFlag & ELV_FLAG_STAGESTART ) &&
		 ( GM_IsGameOver() ||
		   GM_CheckGameStatus( STATE_PLAY_DEMO ) ||
		   Status( PLAYER_NORMAL_MASK ) ) ) {
		return -1 ;
	}
	if ( msg->message[ 1 ] == GM_STRCODE_ENTER ) {
		if ( GM_AlertMode == ALERT_MODE_ALERT ||
			 GM_CheckGameStatus( STATE_ENE_SIGHTIN ) ||
			 CheckEnemy( ElvWork, 1 ) ) return -1 ;	/* 敵がいる */

		/* なんらかの特殊状態にある */
		if ( work->current_mar != work->org_motion ) {
			printf( "elv enter failed!!\n" ) ;
			return -1 ;
		}

		if ( work->action != OperateElevatorPanel ) {
			PL_LeaveCaution( work ) ;
			//PL_LeaveSubject( work ) ;
			PL_ClearCaptureTarget( work ) ;
			PL_UnequipSpecials() ;
			PL_SetMode2( work, NULL ) ;
			GM_SetMenuStatus( MENU_MENU_OFF ) ;
			work->dmg_callback = ElevatorDamageCallback ;
			PL_SetMode( work, OperateElevatorPanel ) ;
			SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_STEP | FLAG_FORCE ) ;
			//SetStatus( PLAYER_WEAPON_INVISIBLE ) ;
			dir = msg->message[ 2 ] ;
			if ( dir < 0 ) dir = work->control.rot.vy ;
			work->control.turn.vy = work->control.rot.vy = dir ;
			work->camdir.vy = dir ;
			Now = StartNow ;
		} else {
			return -1 ;
		}
	} else {
		EndElevatorPanel( work ) ;
		if ( work->action == OperateElevatorPanel ) {
			PL_SetMode( work, PL_StillMode[ STAND ] ) ;
		}
	}
	return 1 ;
}

/* プラグイン登録 */
int		NewPluginElevator( void )
{
	PL_AddPlugin( &PluginSet, PL_MSG_ELEVATOR, SetElevatorPanelAct, NULL ) ;
	return 0 ;
}









