//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	door.c
	ドア

	1997/09/21 K.Uehara
	$Id: door.c,v 1.4 2002/11/23 12:24:49 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>

//BP #include	"break.h"
#endif
#include	"gameheader.h"

extern	int		DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
extern	void	DG_FreePreshade( DG_OBJS * ) ;

#ifdef KP_XBOX	// 障害計算やめる
#define GM_SeSetMode( se, ppos, mode ) GM_SeSetModeAddr( se, ppos, mode, GM_INVALID_ADDR )
#endif

/*----------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)

/*----------------------------------------------------------------*/

typedef	struct	{
	OBJECT			body ;
	float			max ;
	HZX_D_SEGMENT	*radar_seg[ 4 ] ;
} OtherDoorWork ;

typedef	struct	{
	GV_ACT_EX	actor;
	OBJECT		body;
	FVECTOR		pos;
	SVECTOR		rot;
	int			name;
	int 		state;
	char 		*block;

	int 		open_count;
	float 		ofs;
	float 		dx;
	float 		max;

	HZX_D_SEGMENT	*segments[ 2 ] ;
	OBJECT		*body_sdw ;

	int			flag ;
	int			card_level ;
	int			n_panels ;
	int			panel[ 4 ] ;

	/* 両開き拡張 */
	OtherDoorWork	*other ;

	float		hazard_width_max ;

	R_INTRPT	r_intrpt ;

	int			where ;

	FVECTOR		rintrpt_pos[2];
} Work;

enum {
	DOOR_CLOSED,
	DOOR_OPEN,
	DOOR_OPENED,
	DOOR_CLOSE,
};

enum {
	DOOR_FLAG_PLAYER_CANNOT_OPEN 	=	0x0001,
	DOOR_FLAG_ALERT_LOCKED =			0x0002,
	DOOR_FLAG_PLAYER_INSIDE =			0x0004,
	DOOR_FLAG_DOUBLE_DOOR =				0x0008,
	DOOR_FLAG_PUT_RINTRPT =				0x0010,

	DOOR_FLAG_ENEMYFORCE  =				0x0020,

	DOOR_FLAG_POS_TO_RINTRPT =			0x0040,	/* 指定場所同士をＲ遮断 */
} ;

enum {
	REQ_NO,
	REQ_OPEN,
	REQ_CLOSE,
	REQ_VISIBLE,
	REQ_INVISIBLE,
	REQ_DISP_CHANGE,
	REQ_LOCKED,
	REQ_OPENSTART,

	REQ_ENEMYOPEN, /* ALTANATIVE用に追加した。敵兵は通れる */
	REQ_UNLOCKED,
};

/* ドアランプメッセージ */
enum {
	LAMP_RED	=		1,
	LAMP_BLINK	=		2,
	LAMP_BLUE 	=		3,
} ; 

//static	int	Where ;

static	void	*AlertSegment( Work *work, HZX_GROUP_ID hzx_id, HZX_GROUP_ID hzx_id2, 
							   IVECTOR *v ) ;

/*----------------------------------------------------------------*/

static	void	PutRIntrpt( Work *work )
{
	HZX_ZONE_ADD	zone[ 2 ] ;
	HZX_ZON			*z[ 2 ] ;
	FVECTOR			shift = { 0.0F, 0.0F, 0.0F }, chkpos ;

	if ( work->flag & DOOR_FLAG_PUT_RINTRPT ) return ;

	if ( ( GM_GameStatus & STATE_VR_ANOTHER ) &&
		 ( work->flag & DOOR_FLAG_POS_TO_RINTRPT ) ) {
		int gr, zn ;

		HZX_Pos2Zone( &work->rintrpt_pos[0], &gr, &zn ) ;
		zone[ 0 ] = HZX_AddressNo( gr, zn, zn ) ;
		HZX_Pos2Zone( &work->rintrpt_pos[1], &gr, &zn ) ;
		zone[ 1 ] = HZX_AddressNo( gr, zn, zn ) ;
printf("door r_intrpt zone[%x]<->[%x]\n",zone[ 0 ],zone[ 1 ] );
	} else {
		if ( DG_FABS( work->body.objs->objs[ 0 ].bound_min.vx ) > 
			 DG_FABS( work->body.objs->objs[ 0 ].bound_max.vx ) ) {
			shift.vx = work->body.objs->objs[ 0 ].bound_min.vx ;
		} else {
			shift.vx = work->body.objs->objs[ 0 ].bound_max.vx ;
		}
		shift.vx /= 2.0F ;
		DG_SetPos2( &work->pos, &work->rot ) ;
		DG_PutVector( &shift, &chkpos, 1 ) ;

		HZX_GetInterruptZone( &chkpos, work->rot.vy - 1024, 250, &zone[ 0 ] ) ;
	}

	z[ 0 ] = HZX_GetZoneFromAdd( zone[ 0 ] ) ;
	z[ 1 ] = HZX_GetZoneFromAdd( zone[ 1 ] ) ;

	if ( z[ 0 ] != NULL ) {
		z[ 0 ]->flag |= HZX_ZONE_SLIDEDOOR ;
	}
	if ( z[ 1 ] != NULL ) {
		z[ 1 ]->flag |= HZX_ZONE_SLIDEDOOR ;
	}
	if ( work->card_level > GM_ItemNum( IT_Card ) ) {
		GM_SetRouteIntrpt( &work->r_intrpt, zone[ 0 ], zone[ 1 ], &work->pos, 
						  work->rot.vy - 1024, ROOT_INTRPT_NOKEY | ROOT_INTRPT_NONPC, 0 ) ;
		GM_PutRoteIntrpt( &work->r_intrpt ) ;
		work->flag |= DOOR_FLAG_PUT_RINTRPT ;
	}
}

static	void	LocalSendMessage( int to, int n, int v1, int v2 )
{
	GV_MSG		msg ;
	int			message[ 2 ] ;

	message[ 0 ] = v1 ;
	message[ 1 ] = v2 ;

	msg.address = to ;
	msg.message = message ;
	msg.message_len = n ;
	GV_SendMessage( &msg ) ;
}

static	void	SendMessageToPanel( Work *work, int v1, int v2 )
{
	int			i ;

	for ( i = 0; i < work->n_panels; i ++ ) {
		if ( work->panel[ i ] > 0 ) {
			LocalSendMessage( work->panel[ i ], 2, v1, v2 ) ;
		}
	}
}

static	inline	int	GetMessageValue( GV_MSG *msg, int n, int def )
{
	if ( msg->message_len >= n + 1 ) return msg->message[ n ] ;
	return def ;
}

static	inline	int	IsPlayer( int name )
{
	if ( GM_PlayerControl != NULL && GM_PlayerControl->name == name ) return 1 ;
	return 0 ;
}

static int check_message( Work *work, GV_MSG *msg, int n_msg, int comm )
{
	int count = 0;
	int	chara, mesg_ok ;
	int	alert_check ;

	for( ; n_msg > 0; n_msg -- ){
		mesg_ok = 0 ;
		if( msg->message[ 0 ] == comm ){
			chara = GetMessageValue( msg, 1, 0 ) ;

			if ( comm == REQ_ENEMYOPEN ) {
				printf( "door.c : [%d] enemy open\n", GV_Time ) ;
				work->flag |= DOOR_FLAG_ENEMYFORCE ;
				if ( work->flag & DOOR_FLAG_PUT_RINTRPT ) {
					work->flag &= ~DOOR_FLAG_PUT_RINTRPT ;
					GM_FreeRouteIntrpt( &work->r_intrpt ) ;
				}
			} else if ( comm == REQ_OPEN ) {
				if ( IsPlayer( chara ) ) {		/* 開く */
					work->flag |= DOOR_FLAG_PLAYER_INSIDE ;
					/* プレイヤー入る */
					alert_check = ( ( work->flag & DOOR_FLAG_ALERT_LOCKED ) && 
								    ( GM_AlertMode == ALERT_MODE_ALERT ) ) ;
					if ( alert_check ||
						 //PL_GetPlayerItem() != IT_Card ||	/* カードは持っていれば良い */
						 work->card_level > GM_ItemNum( IT_Card ) ) {
						if ( work->state != DOOR_CLOSED ) {
							/* すでに開いていればＯＫとする */
							mesg_ok = 1 ;
							printf( "door.c : [%d] player open( already opened )\n", GV_Time ) ;
						} else {
							/* 開けられない */
							work->flag |= DOOR_FLAG_PLAYER_CANNOT_OPEN ;
							printf( "door.c : [%d] player cannot open\n", GV_Time ) ;
							GM_BuzzerPos( &work->pos ) ;
							SendMessageToPanel( work, LAMP_BLINK, DIRECT_TICK( 90 ) ) ;
						}
					} else {
						/* 開ける */
						mesg_ok = 1 ;
						printf( "door.c : [%d] player open\n", GV_Time ) ;					
					}
				} else {
					/* その他入る */
					if ( work->card_level > GM_ItemNum( IT_Card ) &&
						!(work->flag & DOOR_FLAG_ENEMYFORCE) ) {
						/* プレイヤーのカードレベルが低いので開かない */
						printf( "door.c : [%d] cannot open\n", GV_Time ) ;
					} else {
						/* 開ける */
						mesg_ok = 1 ;
						printf( "door.c : [%d] open\n", GV_Time ) ;
					}
				}
			} else if ( comm == REQ_CLOSE ) {	/* 閉まる */
				if ( IsPlayer( chara ) ) {
					work->flag &= ~DOOR_FLAG_PLAYER_INSIDE ;
					if ( work->flag & DOOR_FLAG_PLAYER_CANNOT_OPEN ) {
						work->flag &= ~DOOR_FLAG_PLAYER_CANNOT_OPEN ;
						printf( "door.c : [%d] player close msg ignored\n", GV_Time ) ;
					} else {
						mesg_ok = 1 ;
						printf( "door.c : [%d] player close\n", GV_Time ) ;
					}
				} else {
					if ( work->card_level > GM_ItemNum( IT_Card ) &&
						!(work->flag & DOOR_FLAG_ENEMYFORCE) ) {
						/* プレイヤーのカードレベルが低いので開いてないから閉じない */
						printf( "door.c : [%d] close msg ignored\n", GV_Time ) ;
					} else {
						mesg_ok = 1 ;
						printf( "door.c : [%d] close\n", GV_Time ) ;
					}
				}
			} else if ( comm == REQ_LOCKED ) {
				/* 鍵をかける */
				/* カードレベルを９９９９にする */
				printf( "door.c : [%d] door locked!\n", GV_Time ) ;
				work->card_level = 9999 ;
				PutRIntrpt( work ) ;
				/* 開けられないドアは、NO_PEEPINTRPT を外す */
				HZX_DynamicSegmentSetAttribute( work->segments[ 0 ], 
							   work->segments[ 0 ]->atr & ~HZX_SEG_NO_PEEPINTRPT ) ;
				HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], 
							   work->segments[ 1 ]->atr & ~HZX_SEG_NO_PEEPINTRPT ) ;
			} else {
				printf( "door.c : [%d] door illeagal message %d\n", GV_Time, comm ) ;
			}
			if ( mesg_ok ) count ++;
		}
		msg++;
	}
	return count;
}

static int check_message_exist( GV_MSG *msg, int n_msg, int comm )
{
	for( ; n_msg > 0; n_msg -- ){
		if( msg->message[ 0 ] == comm ){
			return 1 ;
		}
		msg++;
	}
	return 0 ;
}

static int check_message_disp( GV_MSG *msg, int n_msg, int comm )
{
	for( ; n_msg > 0; n_msg -- ){
		if( msg->message[ 0 ] == comm ){
			return msg->message[ 1 ] ;
		}
		msg++;
	}
	return 0 ;
}

#define CALL_OPEN	1
#define CALL_CLOSED	0

static void call_block( Work *work, int flag )
{
	if( work->block != NULL ){
		GCL_ARGS arg;
		int buffer[ 1 ];
		arg.argc = 1;
		arg.argv = buffer;
		buffer[ 0 ] = flag;

		GCL_ExecBlock( work->block, &arg );
	}
}

static void door_closed( Work *work )
{
	work->ofs = 0.0F;
	if ( work->state != DOOR_CLOSED ) {
		GM_SeSetMode( SD_A_DOORCLO1, &work->pos, GM_SEMODE_NORMAL );
	}
	work->state = DOOR_CLOSED;
	call_block( work, CALL_CLOSED );
}

static void door_opend( Work *work )
{
	work->ofs = work->max;
	if ( work->state != DOOR_OPENED ) {
		GM_SeSetMode( SD_A_DOOROPN1, &work->pos, GM_SEMODE_NORMAL );
	}
	work->state = DOOR_OPENED;
}

static	void	AddIVec( v3, v1, v2 )
IVECTOR			*v3, *v1, *v2 ;
{
	v3->vx = v1->vx + v2->vx ;
	v3->vy = v1->vy + v2->vy ;
	v3->vz = v1->vz + v2->vz ;
	v3->vw = v1->vw ;
}

static void Act( Work *work )
{
	GV_MSG *msg;

#if 0
//	GM_SetCurrentMap( work->body.map_name ) ;
	GM_SetCurrentMap( GM_PlayerMap ) ;
	HZX_ViewDynamicSegment( work->segments[ 0 ] ) ;
	HZX_ViewDynamicSegment( work->segments[ 1 ] ) ;
	DG_InvisibleObjs( work->body.objs ) ;
#endif
	switch( work->state ){
	  case DOOR_CLOSED:
		break;
	  case DOOR_OPEN:
		work->ofs += work->dx;
		if( work->dx > 0 ){
			if( work->ofs >= work->max ){
				door_opend( work );
			}
		} else {
			if( work->ofs <= work->max ){
				door_opend( work );
			}
		}
		break;
	  case DOOR_OPENED:
		break;
	  case DOOR_CLOSE:
		work->ofs -= work->dx;
		if( work->dx > 0 ){
			if( work->ofs <= 0.0F ){
				door_closed( work );
			}
		} else {
			if( work->ofs >= 0.0F ){
				door_closed( work );
			}
		}
		break;
	}

	{
		int n;
		int	pre = work->open_count ;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			check_message( work, msg, n, REQ_LOCKED ) ;
			check_message( work, msg, n, REQ_ENEMYOPEN ) ;
			work->open_count += check_message( work, msg, n, REQ_OPEN );
			work->open_count -= check_message( work, msg, n, REQ_CLOSE );
#ifdef DEBUG_MODE
			if ( pre != work->open_count ) {
				printf( "door.c : [%d] open count %d->%d\n", GV_Time, pre, work->open_count ) ;
			}
#endif
			if ( check_message_exist( msg, n, REQ_VISIBLE ) ) {
				DG_VisibleObjs( work->body.objs ) ;				
			}
			if ( check_message_exist( msg, n, REQ_INVISIBLE ) ) {
				DG_InvisibleObjs( work->body.objs ) ;
			}
			if ( check_message_exist( msg, n, REQ_UNLOCKED ) ) {
				/* 鍵をかける */
				/* カードレベルを９９９９にする */
				work->card_level = GetMessageValue( msg, 2, 0 ) ; ;
				if ( work->flag & DOOR_FLAG_PUT_RINTRPT ) {
					work->flag &= ~DOOR_FLAG_PUT_RINTRPT ;
					GM_FreeRouteIntrpt( &work->r_intrpt ) ;
				}
				printf( "door.c : [%d] door unlocked %d!\n", GV_Time, work->card_level ) ;
			}

			if ( check_message_exist( msg, n, REQ_DISP_CHANGE ) ) {
				work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
				work->body.objs->flag |= check_message_disp( msg, n, REQ_DISP_CHANGE ) ;
			}
			if ( check_message_exist( msg, n, REQ_OPENSTART ) ) {
				/* 開いた状態からスタート */
				work->ofs = work->max;
				work->state = DOOR_OPENED;				
				if ( work->open_count == 0 ) work->open_count = 1 ;
				printf( "door.c : [%d] openstart %d\n", GV_Time, work->open_count ) ;
			}
		}
		/* プレイヤー再チェック */
		if ( ( work->flag & DOOR_FLAG_PLAYER_CANNOT_OPEN ) ) {
			int	alert_check ;

			alert_check = ( ( work->flag & DOOR_FLAG_ALERT_LOCKED ) && 
						    ( GM_AlertMode == ALERT_MODE_ALERT ) ) ;
			if ( work->state != DOOR_CLOSED ) {
				/* 誰かが開けた */	
				work->flag &= ~DOOR_FLAG_PLAYER_CANNOT_OPEN ;
				work->open_count ++ ;		
				printf( "door.c : [%d] player door open(someone open the door)\n", GV_Time ) ;		
			} else if ( alert_check == 0 &&
					    PL_GetPlayerItem() == IT_Card &&
					    work->card_level <= GM_ItemNum( IT_Card ) ) { 
				/* カードＯＫ、危険モードＯＫ */
				work->flag &= ~DOOR_FLAG_PLAYER_CANNOT_OPEN ;
				work->open_count ++ ;
				printf( "door.c : [%d] player door open(card OK!)\n", GV_Time ) ;
			}
		}
		if ( pre != work->open_count ) {
			if( work->open_count > 0 ){
				if( work->state == DOOR_CLOSED ){
					call_block( work, CALL_OPEN );
				}
				if ( work->state == DOOR_CLOSED || 
					 work->state == DOOR_CLOSE ) {
					GM_SeSetMode( SD_A_DOORMOV1, &work->pos, GM_SEMODE_NORMAL ) ;
					SendMessageToPanel( work, LAMP_BLUE, 0 ) ;
				}
				if( work->state != DOOR_OPENED ){
					// 開ききっていなければ開ける
					work->state = DOOR_OPEN;
				}
			} else {
				if( work->state != DOOR_CLOSE && work->state != DOOR_CLOSED ){
					GM_SeSetMode( SD_A_DOORMOV1, &work->pos, GM_SEMODE_NORMAL );
					SendMessageToPanel( work, LAMP_RED, 0 ) ;
				}
				work->state = DOOR_CLOSE;
			}
		}
	}

	DG_SetPos2( &work->pos, &work->rot );
	{
		FVECTOR shift = { work->ofs, 0.0F, 0.0F };

		/* 壁移動 */
		{
			FVECTOR		shift2, hshift ;
			IVECTOR		iv, p1, p2 ;

			GV_SetVec3( &hshift, work->ofs * work->hazard_width_max / work->max, 0.0F, 0.0F ) ;
			DG_RotVector( &hshift, &shift2, 1 ) ;
			GV_FVtoIV( &shift2, &iv, 3 ) ;
			if ( work->segments[ 0 ] != NULL ) {
				AddIVec( &p1, &work->segments[ 0 ]->def[ 0 ], &iv ) ;
				AddIVec( &p2, &work->segments[ 0 ]->def[ 1 ], &iv ) ;
				HZX_MoveDynamicSegment( work->segments[ 0 ], &p1, &p2 ) ;
			}
			if ( work->segments[ 1 ] != NULL ) {
				AddIVec( &p1, &work->segments[ 1 ]->def[ 0 ], &iv ) ;
				AddIVec( &p2, &work->segments[ 1 ]->def[ 1 ], &iv ) ;
				HZX_MoveDynamicSegment( work->segments[ 1 ], &p1, &p2 ) ;
			}

			if ( work->flag & DOOR_FLAG_DOUBLE_DOOR ) {
				if ( work->other->radar_seg[ 0 ] != NULL ) {
					AddIVec( &p1, &work->other->radar_seg[ 0 ]->def[ 0 ], &iv ) ;
					AddIVec( &p2, &work->other->radar_seg[ 0 ]->def[ 1 ], &iv ) ;
					HZX_MoveDynamicSegment( work->other->radar_seg[ 0 ], &p1, &p2 ) ;
				}
				if ( work->other->radar_seg[ 1 ] != NULL ) {
					AddIVec( &p1, &work->other->radar_seg[ 1 ]->def[ 0 ], &iv ) ;
					AddIVec( &p2, &work->other->radar_seg[ 1 ]->def[ 1 ], &iv ) ;
					HZX_MoveDynamicSegment( work->other->radar_seg[ 1 ], &p1, &p2 ) ;
				}
			}
		}
		DG_SetPos2( &work->pos, &work->rot ) ;
		DG_MovePos( &shift ) ;
	}
	DG_PutObjs( work->body.objs );

	/* 両開き対応 */
	if ( work->flag & DOOR_FLAG_DOUBLE_DOOR ) {
		OtherDoorWork	*otw ;
		FVECTOR			shift ;
		IVECTOR			iv, p1, p2 ;

		otw = work->other ;
		shift.vx = otw->max * ( work->ofs / work->max ) ; 
		shift.vy = shift.vz = 0.0F ;
		DG_SetPos2( &work->pos, &work->rot ) ;		
		DG_MovePos( &shift ) ;
		DG_PutObjs( otw->body.objs ) ;

		DG_RotVector( &shift, &shift, 1 ) ;
		GV_FVtoIV( &shift, &iv, 3 ) ;
		if ( otw->radar_seg[ 2 ] != NULL ) {
			AddIVec( &p1, &otw->radar_seg[ 2 ]->def[ 0 ], &iv ) ;
			AddIVec( &p2, &otw->radar_seg[ 2 ]->def[ 1 ], &iv ) ;
			HZX_MoveDynamicSegment( otw->radar_seg[ 2 ], &p1, &p2 ) ;
		}
		if ( otw->radar_seg[ 3 ] != NULL ) {
			AddIVec( &p1, &otw->radar_seg[ 3 ]->def[ 0 ], &iv ) ;
			AddIVec( &p2, &otw->radar_seg[ 3 ]->def[ 1 ], &iv ) ;
			HZX_MoveDynamicSegment( otw->radar_seg[ 3 ], &p1, &p2 ) ;
		}		
	}

	if( ( work->state == DOOR_CLOSED || work->state == DOOR_OPENED ) &&
	    !( work->flag & DOOR_FLAG_PLAYER_CANNOT_OPEN ) ) {
		GV_WaitMessage( work, work->name );
	}
}

static void Die( Work *work )
{

	DG_FreePreshade( work->body.objs );
    GM_FreeObject( &work->body ) ;
	if ( work->body_sdw != NULL ) {
		GM_FreeObject( work->body_sdw ) ;
		GV_DelayedFree( work->body_sdw ) ;
	}
	if ( work->other != NULL ) {
		int			i ;

		DG_FreePreshade( work->other->body.objs );
		GM_FreeObject( &work->other->body ) ;
		GV_DelayedFree( work->other ) ;
		for ( i = 0; i < 4; i ++ ) {
			HZX_RemoveDynamicSegment( work->other->radar_seg[ i ] ) ;
		}
	}
	HZX_RemoveDynamicSegment( work->segments[ 0 ] ) ;
	HZX_RemoveDynamicSegment( work->segments[ 1 ] ) ;

	if ( work->flag & DOOR_FLAG_PUT_RINTRPT ) {
		GM_FreeRouteIntrpt( &work->r_intrpt ) ;
	}
}

/*----------------------------------------------------------------*/

static	inline	int		AdjustValue( int v )
{
	v += ( v < 0 ) ? -2 : 2 ;
	return v / 5 * 5 ;
}

static int GetResources( Work *work, int name, int where )
{
	FVECTOR		pos;
	SVECTOR		rot;
	char		*opt;
	int			model;
	int			seNo ;
	int buf[ 3 ];

	work->name = name;

	seNo = GCL_GetOptionValue( 'N', 0 ) ;
	seNo = ( seNo & 0xf ) << 28 ;
	
	if ( (model = GCL_GetOptionValue( 'm', 0 )) != 0 ) {
		GM_InitObject( &work->body, model, BODY_FLAG );
	} else {
		return -1;
	}

	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		pos.vx = ( float )buf[ 0 ];
		pos.vy = ( float )buf[ 1 ];
		pos.vz = ( float )buf[ 2 ];
	} else {
		pos = DG_ZeroVector;
	}

	if ( ( opt = GCL_GetOption( 'd' ) ) != NULL ){
		GCL_GetSV( opt, ( void * )&rot );
	} else {
		rot = DG_ZeroSVector;
	}

	work->pos = pos;
	work->rot = rot;

	/* カードレベル */
	work->card_level = GCL_GetOptionValue( 'c', 0 ) ;

	/* ゾーン遮断 */
	if ( ( opt = GCL_GetOption( 'r' ) ) != NULL ){
		GCL_GetIV( GCL_NextStr(), buf ) ;
		work->rintrpt_pos[0].vx = ( float )buf[ 0 ];
		work->rintrpt_pos[0].vy = ( float )buf[ 1 ];
		work->rintrpt_pos[0].vz = ( float )buf[ 2 ];
		GCL_GetIV( GCL_NextStr(), buf ) ;
		work->rintrpt_pos[1].vx = ( float )buf[ 0 ];
		work->rintrpt_pos[1].vy = ( float )buf[ 1 ];
		work->rintrpt_pos[1].vz = ( float )buf[ 2 ];

		work->flag |= DOOR_FLAG_POS_TO_RINTRPT ;
	}
	PutRIntrpt( work ) ;

	{
		int slide, time;

		slide = GCL_GetOptionValue( 's', 1000 );
		time = GCL_GetOptionValue( 't',  20 );
		work->dx = ( float )slide / time;
		work->max = ( float )slide;

		work->hazard_width_max = slide ;
	}
	work->block = ( char * )GCL_GetOptionValue( 'e', 0 );

	work->ofs = 0.0F;
	work->open_count = 0;
	work->state = DOOR_CLOSED;

	DG_SetPos2( &pos, &rot );
	DG_PutObjs( work->body.objs );

    {
		FVECTOR		pos2 ;

		DG_COPY_VEC( &pos2, &pos ) ;
		pos2.vx += work->body.objs->def->tx ;
		pos2.vy += work->body.objs->def->ty ;
		pos2.vz += work->body.objs->def->tz ;
		DG_SetPos2( &pos2, &rot ) ;
	}

	DG_MakePreshade( work->body.objs, GM_GetMap( where )->light ) ;

	/* 危険モードで鍵 */
	if ( GCL_GetOptionValue( 'a', 0 ) ) {
		work->flag |= DOOR_FLAG_ALERT_LOCKED ;
	}

	/* 壁 */
	{
		IVECTOR			iv[ 4 ] ;
		FVECTOR			v[ 4 ] ;
		HZX_GROUP_ID	hzx_id, hzx_id2, id, hid[ 2 ] ;
		int				n, gn ;
		int				i ;

		v[ 0 ].vx = work->body.objs->def->lx ;
		v[ 0 ].vy = work->body.objs->def->ly ;
		v[ 0 ].vz = work->body.objs->def->uz ;
		v[ 1 ].vx = work->body.objs->def->ux ;
		v[ 1 ].vy = work->body.objs->def->uy ;
		v[ 1 ].vz = work->body.objs->def->uz ;
		v[ 2 ].vx = work->body.objs->def->lx ;
		v[ 2 ].vy = work->body.objs->def->ly ;
		v[ 2 ].vz = work->body.objs->def->lz ;
		v[ 3 ].vx = work->body.objs->def->ux ;
		v[ 3 ].vy = work->body.objs->def->uy ;
		v[ 3 ].vz = work->body.objs->def->lz ;

		if ( GCL_GetOption( 'h' ) != NULL ) {	
			/* 幅をシナリオ指定 */
			v[ 0 ].vx = v[ 2 ].vx = 0 ;
			v[ 1 ].vx = v[ 3 ].vx = GCL_GetNextInt() ;
			work->hazard_width_max = v[ 1 ].vx ;
		}

		DG_PutVector( v, v, 4 ) ;
		GV_FVtoIV( v, iv, 16 ) ;
		iv[ 0 ].vw = iv[ 1 ].vw = iv[ 2 ].vw = iv[ 3 ].vw
			= iv[ 1 ].vy - iv[ 0 ].vy ;
		iv[ 3 ].vy = iv[ 2 ].vy = iv[ 1 ].vy = iv[ 0 ].vy ;

		for ( i = 0; i < 4 ; i ++ ) {
			iv[ i ].vx = AdjustValue( iv[ i ].vx ) ;
			iv[ i ].vy = AdjustValue( iv[ i ].vy ) ;
			iv[ i ].vz = AdjustValue( iv[ i ].vz ) ;
			iv[ i ].vw = AdjustValue( iv[ i ].vw ) ;
		}

		n = 0 ;
		if ( GCL_GetOption( 'b' ) != NULL ) {
			int		map, map2 ;

			map = GM_GetMapID( GCL_GetNextInt() ) ;
			hzx_id = GM_GetHzxGroupID( map ) ;
			map2 = GM_GetMapID( GCL_GetNextInt() ) ;
			hzx_id2 = GM_GetHzxGroupID( map2 ) ;
			GM_GroupObjs( work->body.objs, map | map2 ) ;
			work->body.map_name = map | map2 ;
		} else {
			id = GM_GetHzxGroupID( where ) ;
			hid[ 0 ] = hid[ 1 ] = 0 ;
			while( id != 0 && n < 2 ) {
				gn = GV_GetNo( id ) ;
				hid[ n ] = GV_GetBit( gn ) ;
				id &= ~hid[ n ] ;
				n ++ ;
			}
			hzx_id = hid[ 0 ] ;
			if ( hid[ 1 ] != 0 ) hzx_id2 = hid[ 1 ] ;
			else				 hzx_id2 = hzx_id ;
			GM_GroupObjs( work->body.objs, where ) ;
			work->body.map_name = where ;
		}
		work->segments[ 0 ]
			= HZX_AddDynamicSegment( hzx_id, &iv[ 0 ], &iv[ 1 ], HZX_SEG_NO_PEEPINTRPT |
									 HZX_SEG_NO_C4 | 
									 HZX_SEG_NO_BULLETHOLE | HZX_SEG_DOOR | seNo ) ;
		HZX_DynamicSegmentSetAttribute( work->segments[ 0 ], 
									    work->segments[ 0 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
		if ( hzx_id != hzx_id2 ) {
			work->segments[ 1 ]
				= HZX_AddDynamicSegment( hzx_id2, &iv[ 2 ], &iv[ 3 ], HZX_SEG_NO_PEEPINTRPT |
										 HZX_SEG_NO_C4 | 
										 HZX_SEG_NO_BULLETHOLE | HZX_SEG_DOOR | seNo ) ;
			HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], 
										    work->segments[ 1 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
		} else {
			work->segments[ 1 ]
				= HZX_AddDynamicSegment( hzx_id, &iv[ 2 ], &iv[ 3 ], HZX_SEG_NO_PEEPINTRPT |
										 HZX_SEG_NO_C4 | 
										 HZX_SEG_NO_BULLETHOLE | HZX_SEG_DOOR | seNo ) ;
			HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], 
										    work->segments[ 1 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
		}
		if ( work->flag & DOOR_FLAG_ALERT_LOCKED ) {
			/* 危険モード時プレイヤー防止壁 */
			GV_SetActorChild( work, AlertSegment( work, hzx_id, hzx_id2, iv ) ) ;
		}

		/* 開けられないドアは、NO_PEEPINTRPT を外す */
		if ( work->card_level > GM_ItemNum( IT_Card ) ) {
			HZX_DynamicSegmentSetAttribute( work->segments[ 0 ], 
										    work->segments[ 0 ]->atr & ~HZX_SEG_NO_PEEPINTRPT ) ;
			HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], 
										    work->segments[ 1 ]->atr & ~HZX_SEG_NO_PEEPINTRPT ) ;
		}
	}
	/* 影投影ドア */
	if ( GCL_GetOption( 'l' ) != NULL ) {
		OBJECT		*shadow ;
		int			shadow_id ;

		shadow_id = GCL_GetNextInt() ;
		if ( shadow_id != 0 ) {
			shadow = work->body_sdw = ( OBJECT * )GV_Malloc( sizeof( OBJECT ) ) ;
			GV_ZeroMemory( shadow, sizeof( OBJECT ) ) ;
			ASSERT( shadow != NULL ) ;
			GM_InitObject( shadow, model, DG_FLAG_SHADOWWRITE|DG_FLAG_ONEPIECE ) ;
			GM_ConfigObjectRoot( shadow, &work->body, 0 ) ;
			shadow->objs->shadow_id = shadow_id ;
		}
	}

	/* パネル */
	if ( GCL_GetOption( 'A' ) != NULL ) {
		work->n_panels = 0 ;
		while( GCL_NextStr() != NULL ) {
			work->panel[ work->n_panels ] = GCL_GetNextInt() ;
			if ( ++ work->n_panels >= 4 ) break ;
		}
	}

	/* 両開き対応 */
	if ( GCL_GetOption( 'w' ) != NULL ) {
		OtherDoorWork	*otw ;
		int				omodel, oslide ;

		work->flag |= DOOR_FLAG_DOUBLE_DOOR ;
		omodel = GCL_GetNextInt() ;
		oslide = GCL_GetNextInt() ;
		otw = work->other = ( OtherDoorWork * )GV_Malloc( sizeof( OtherDoorWork ) ) ;
		GV_ZeroMemory( otw, sizeof( OtherDoorWork ) ) ;
		ASSERT( otw != NULL ) ;
		GM_InitObject( &otw->body, omodel, BODY_FLAG ) ;
		ASSERT( otw->body.objs != NULL ) ;
		otw->body.map_name = work->body.map_name ;
		GM_GroupObjs( otw->body.objs, work->body.map_name ) ;
		otw->max = ( float )oslide ;
		DG_SetPos2( &pos, &rot );
		DG_PutObjs( otw->body.objs ) ;
		DG_MakePreshade( otw->body.objs, GM_GetMap( where )->light ) ;

		/* レーダー表示用に動的ハザードを作成 */
		HZX_DynamicSegmentSetAttribute( work->segments[ 0 ], 
									    work->segments[ 0 ]->atr | HZX_SEG_NO_DISP_RADAR ) ;
		HZX_DynamicSegmentSetAttribute( work->segments[ 1 ], 
									    work->segments[ 1 ]->atr | HZX_SEG_NO_DISP_RADAR ) ;
		{
			FVECTOR		v[ 4 ] ;
			IVECTOR		iv[ 4 ] ;

			v[ 0 ].vx = work->body.objs->def->lx ; v[ 1 ].vx = work->body.objs->def->ux ;
			v[ 0 ].vy = work->body.objs->def->ly ; v[ 1 ].vy = v[ 0 ].vy ;
			v[ 0 ].vz = work->body.objs->def->uz ; v[ 1 ].vz = v[ 0 ].vz ;
			DG_COPY_VEC( &v[ 2 ], &v[ 0 ] ) ;
			DG_COPY_VEC( &v[ 3 ], &v[ 1 ] ) ;
			v[ 2 ].vz = v[ 3 ].vz = work->body.objs->def->lz ;

			DG_SetPos( &work->body.objs->world ) ;
			DG_PutVector( v, v, 4 ) ;

			v[ 0 ].vw = v[ 1 ].vw = work->body.objs->def->uy - work->body.objs->def->ly ;
			v[ 2 ].vw = v[ 3 ].vw = v[ 0 ].vw ;
			GV_FVtoIV( v, iv, 16 ) ;

			otw->radar_seg[ 0 ]
				= HZX_AddDynamicSegment( GV_GetBit( work->segments[ 0 ]->tag[ 3 ] ), 
										&iv[ 0 ], &iv[ 1 ], 
										HZX_SEG_ATR_ALL | HZX_SEG_DOOR ) ;
			HZX_DynamicSegmentSetAttribute( otw->radar_seg[ 0 ], 
										    otw->radar_seg[ 0 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
			otw->radar_seg[ 1 ]
				= HZX_AddDynamicSegment( GV_GetBit( work->segments[ 1 ]->tag[ 3 ] ), 
										&iv[ 2 ], &iv[ 3 ], 
										HZX_SEG_ATR_ALL | HZX_SEG_DOOR ) ;
			HZX_DynamicSegmentSetAttribute( otw->radar_seg[ 1 ], 
										    otw->radar_seg[ 1 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;

			v[ 0 ].vx = otw->body.objs->def->lx ; v[ 1 ].vx = otw->body.objs->def->ux ;
			v[ 0 ].vy = otw->body.objs->def->ly ; v[ 1 ].vy = v[ 0 ].vy ;
			v[ 0 ].vz = otw->body.objs->def->uz ; v[ 1 ].vz = v[ 0 ].vz ;
			DG_COPY_VEC( &v[ 2 ], &v[ 0 ] ) ;
			DG_COPY_VEC( &v[ 3 ], &v[ 1 ] ) ;
			v[ 2 ].vz = v[ 3 ].vz = otw->body.objs->def->lz ;

			DG_SetPos( &otw->body.objs->world ) ;
			DG_PutVector( v, v, 4 ) ;

			v[ 0 ].vw = v[ 1 ].vw = otw->body.objs->def->uy - otw->body.objs->def->ly ;
			v[ 2 ].vw = v[ 3 ].vw = v[ 0 ].vw ;
			GV_FVtoIV( v, iv, 16 ) ;

			otw->radar_seg[ 2 ]
				= HZX_AddDynamicSegment( GV_GetBit( work->segments[ 0 ]->tag[ 3 ] ), 
										&iv[ 0 ], &iv[ 1 ], 
										HZX_SEG_ATR_ALL | HZX_SEG_DOOR ) ;
			HZX_DynamicSegmentSetAttribute( otw->radar_seg[ 2 ], 
										    otw->radar_seg[ 2 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
			otw->radar_seg[ 3 ]
				= HZX_AddDynamicSegment( GV_GetBit( work->segments[ 1 ]->tag[ 3 ] ), 
										&iv[ 2 ], &iv[ 3 ], 
										HZX_SEG_ATR_ALL | HZX_SEG_DOOR ) ;
			HZX_DynamicSegmentSetAttribute( otw->radar_seg[ 3 ], 
										    otw->radar_seg[ 3 ]->atr & ~HZX_SEG_NO_DISP_RADAR ) ;
		}


	}

	work->where = where ;
	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		extern int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot ) ;
		C4MAN_Regist( name, &work->where, work->body.objs, &work->body.objs->objs[0].world, &DG_ZeroVector, &DG_ZeroSVector ) ;
	}

	return 0;
}

	/*
		シナリオ呼び出し
	*/
void *NewDoor( int name, int where )
{
	Work		*work;

	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) );
	if( work != NULL ){
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if( GetResources( work, name, where ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------

  危険モード時のプレイヤー壁制御

----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	Work				*pWork ;
	HZX_D_SEGMENT		*alertseg[ 2 ] ;
	int					mode ;
} Work2 ;

enum {
	ALERTSEG_MODE_SKIP = 0,
	ALERTSEG_MODE_NOSKIP,
} ;

static	void	Act2( Work2 *work2 )
{
	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
		/* 危険モード */
		/* 危険モード開始時に
		   プレイヤーがドア付近にいるときは
		   壁を発生させない */
		if ( work2->mode == ALERTSEG_MODE_SKIP ) {
			if ( work2->pWork->flag & DOOR_FLAG_PLAYER_INSIDE ) {
				HZX_DynamicSegmentSetSkip( work2->alertseg[ 0 ] ) ;
				HZX_DynamicSegmentSetSkip( work2->alertseg[ 1 ] ) ;			
			} else {
				HZX_DynamicSegmentResetSkip( work2->alertseg[ 0 ] ) ;
				HZX_DynamicSegmentResetSkip( work2->alertseg[ 1 ] ) ;			
				work2->mode = ALERTSEG_MODE_NOSKIP ;
			}
		} 
	} else if ( work2->mode == ALERTSEG_MODE_NOSKIP ) {
		HZX_DynamicSegmentSetSkip( work2->alertseg[ 0 ] ) ;
		HZX_DynamicSegmentSetSkip( work2->alertseg[ 1 ] ) ;
		work2->mode = ALERTSEG_MODE_SKIP ;
	}
	if ( work2->mode == ALERTSEG_MODE_NOSKIP ) {
		if ( work2->pWork->state == DOOR_CLOSED ) {
			HZX_DynamicSegmentSetAttribute( work2->alertseg[ 0 ], 
										    work2->alertseg[ 0 ]->atr & ~HZX_SEG_NO_HARITSUKI ) ;
			HZX_DynamicSegmentSetAttribute( work2->alertseg[ 1 ], 
										    work2->alertseg[ 1 ]->atr & ~HZX_SEG_NO_HARITSUKI ) ;
		} else {
			HZX_DynamicSegmentSetAttribute( work2->alertseg[ 0 ], 
										    work2->alertseg[ 0 ]->atr | HZX_SEG_NO_HARITSUKI ) ;
			HZX_DynamicSegmentSetAttribute( work2->alertseg[ 1 ], 
										    work2->alertseg[ 1 ]->atr | HZX_SEG_NO_HARITSUKI ) ;
		}
	} 
#if 0
#ifdef DEBUG_MODE
	if ( work2->mode == ALERTSEG_MODE_NOSKIP ) {
		GM_SetCurrentMap( GM_PlayerMap ) ;
		HZX_ViewDynamicSegment( work2->alertseg[ 0 ] ) ;
		HZX_ViewDynamicSegment( work2->alertseg[ 1 ] ) ;
	}
#endif
#endif
}

static	void	Die2( Work2 *work2 )
{
	HZX_RemoveDynamicSegment( work2->alertseg[ 0 ] ) ;
	HZX_RemoveDynamicSegment( work2->alertseg[ 1 ] ) ;
}

static	void	*AlertSegment( Work *work, 
								   HZX_GROUP_ID hzx_id, HZX_GROUP_ID hzx_id2, IVECTOR *v ) 
{
	Work2		*work2 ;

	work2 = ( Work2 * )GV_NewActor( GV_ACTOR_USER, sizeof( Work2 ) ) ;
	if ( work2 == NULL ) ASSERT( 0 ) ;
	GV_SetActor( &work2->actor, Act2, Die2 ) ;
	GV_ActorEX( &work2->actor ) ;
	work2->alertseg[ 0 ] 
		= HZX_AddDynamicSegment( hzx_id, v, v + 1, 
								 HZX_SEG_ATR_ALL & ~HZX_SEG_NO_PLAYER ) ;
	ASSERT( work2->alertseg[ 0 ] != NULL ) ;
	work2->alertseg[ 1 ] 
		= HZX_AddDynamicSegment( hzx_id, v + 2, v + 3, 
								 HZX_SEG_ATR_ALL & ~HZX_SEG_NO_PLAYER ) ;
	ASSERT( work2->alertseg[ 1 ] != NULL ) ;
	HZX_DynamicSegmentSetSkip( work2->alertseg[ 0 ] ) ;
	HZX_DynamicSegmentSetSkip( work2->alertseg[ 1 ] ) ;

	work2->pWork = work ;
	work2->mode = ALERTSEG_MODE_SKIP ;

	return work2 ;
}
