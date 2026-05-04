//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   padrec.c
   パッド録画

   2000/10/02	M.Sonoyama
   $Id: padrec.c,v 1.1.1.3 2002/11/19 11:50:45 Yoshizawa1 Exp $
*/

#ifdef DEBUG_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>

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

//#define	VERSION					(1)	/* New!!! */
#define	VERSION					(2)		

#define	PAD_MEMORY_ADDR			(0x02000000)
#define	REC_TIME_MAX			(300/TIME_BASE * 300)	/* 5分 */
#define	PAD_MEMORY_ADDR_END		(PAD_MEMORY_ADDR + sizeof( GV_PAD ) * REC_TIME_MAX)

#define	EXPLAIN_X				(8)
#define	EXPLAIN_Y				(24)

#define	FILE_NAME				"host:./paddemo.rpd"

extern	int		pcOpen( char *, int ) ;
extern	int		pcWrite( int, void *, int ) ;
extern	int		pcClose( int ) ;

static	int		PadRecActor = 0 ;

enum {
	REC_MODE_STOP = 0,
	REC_MODE_READY,
	REC_MODE_REC,
	REC_MODE_PLAY,
	REC_MODE_SAVE,
} ;

enum {
	SAVE_MODE_NO_PRESSURE = 0,
	SAVE_MODE_ALL,
} ;

typedef	struct	{
	u_char		magic ;
	u_char		version ;
	u_short		length ;
	u_int		flag ;
	u_char		*type ;
	u_short		*button ;

	u_char		*analog ;
	u_short		*press_button ;
	u_char		*press_data ;	
	int			start_turn ;

	FVECTOR		start_pos ;
	int			start_time ;
} SV_PAD_HEADER ;

typedef	struct	{
	int			type_count ;
	int			button_count ;
	int			analog_count ;
	int			press_button_count ;

	int			press_data_count ;
	u_char		*type ;
	u_short		*button ;
	u_char		*analog ;

	u_short		*press_button ;
	u_char		*press_data ;
	
	void		*save_header ;
	int			save_size ;
} SV_WORK ;

enum {
	SV_TYPE_NONE =					0x00,
	SV_TYPE_BUTTON =				0x01,
	SV_TYPE_ANALOG_L =				0x02,
	SV_TYPE_ANALOG_R =				0x04,	
	SV_TYPE_PRESS =					0x08,
	SV_TYPE_BUTTON_CHANGED = 		0x10,
	SV_TYPE_PRESS_BUTTON_CHANGED = 	0x20,
	SV_TYPE_PAUSE_MENU =			0x40,
} ;

typedef	struct	{
	GV_ACT_EX		actor ;
	FVECTOR			start_player_pos ;
	int				start_player_turn ;
	GV_PAD			*pad ;
	void			*memory ;
	int				which ;

	int				mode ;
	int				delay ;
	int				count ;
	int				rec_count ;

	int				rec ;
	int				save_mode ;
	int				start_time ;

	SV_WORK			save_work ;
} Work ;

//static	Work	PadRecWork __attribute__((section(".data"))) ;
static	Work	PadRecWork ;

static	inline	int	Align4( int size )
{
	return ( ( size + 3 ) / 4 * 4 ) ;
}

static	int		MakeSaveData( Work *work )
{
	GV_PAD		*mem ;
	SV_WORK		*sw ;
	int			c, size ;
	int			count, status, status2, ai ;
	int			save_mode ;
	//int			pauselevel ;
	SV_PAD_HEADER		*header ;
	u_char				*type, *analog, *press_data ;
	u_short				*press_button, *button ;
	u_char				tdata, *prs ;
	u_short				pbdata, prev_button, prev_press_button ;

	mem = work->memory ;
	count = work->rec_count ;
	save_mode = work->save_mode ;

	sw = &work->save_work ;
	
	sw->type_count = 0 ;
	sw->button_count = 0 ;
	sw->analog_count = 0 ;	
	sw->press_button_count = 0 ;
	sw->press_data_count = 0 ;
	prev_button = 0 ;
	prev_press_button = 0 ;

	/* 領域確保のためにデータ数をカウント */
	while( -- count >= 0 ) {
		sw->type_count ++ ;

		status = mem->status ;
		ai = mem->analog_input ;

		if ( ( status & ~PAD_UDLR ) ||	/* 十字キー以外のキー */
			 ( ( status & PAD_UDLR ) && !( ai & GV_PAD_ANALOG_L_USE ) ) ) {	/* デジタル十字キー */
			if ( ai & GV_PAD_ANALOG_L_USE ) {
				status2 = status & ~PAD_UDLR ;
			} else {
				status2 = status ;
			}
			if ( prev_button != status2 ) {
				sw->button_count ++ ;
			}
			c = 0 ;
			if ( mem->type >= GV_PAD_DUALSHOCK2 && save_mode == SAVE_MODE_ALL ) {	
				/* 感圧セーブ */
				if ( ( status & PAD_UDLR ) && !( ai & GV_PAD_ANALOG_L_USE ) ) {
					/* 十字キー */
					if ( status & PAD_U ) sw->press_data_count ++ ;
					if ( status & PAD_D ) sw->press_data_count ++ ;
					if ( status & PAD_L ) sw->press_data_count ++ ;
					if ( status & PAD_R ) sw->press_data_count ++ ;
					c |= ( status & PAD_UDLR ) ;
				}
				if ( status & ~( PAD_UDLR | PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ) {
					/* 十字キー以外の感圧キー */
					if ( status & PAD_A ) sw->press_data_count ++ ;
					if ( status & PAD_B ) sw->press_data_count ++ ;
					if ( status & PAD_X ) sw->press_data_count ++ ;
					if ( status & PAD_Y ) sw->press_data_count ++ ;
					if ( status & PAD_L1 ) sw->press_data_count ++ ;
					if ( status & PAD_R1 ) sw->press_data_count ++ ;
					if ( status & PAD_L2 ) sw->press_data_count ++ ;
					if ( status & PAD_R2 ) sw->press_data_count ++ ;
					c |= ( status & ~( PAD_UDLR | PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ) ;
				}
				if ( c != 0 && c != prev_press_button ) {
					sw->press_button_count ++ ;
				}
			}
		}
		/* アナログスティック */
		if ( ai & GV_PAD_ANALOG_L_USE ) sw->analog_count += 2 ;
		if ( ai & GV_PAD_ANALOG_R_USE ) sw->analog_count += 2 ;

		if ( ai & GV_PAD_ANALOG_L_USE ) {
			prev_button = status & ~PAD_UDLR ;
			prev_press_button = status & ~( PAD_UDLR | PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ;
		} else {
			prev_button = status ;
			prev_press_button = status & ~( PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ;
		}
		mem ++ ;
#if 0
		/* ポーズレベルの分 */
		mem = ( GV_PAD * )( ( int )mem + sizeof( int ) ) ;
#endif
	}

	/* 領域確保 */
	size = Align4( sizeof( SV_PAD_HEADER ) ) 
		+ Align4( sizeof( u_char ) * sw->type_count ) 
		+ Align4( sizeof( u_short ) * sw->button_count ) 
		+ Align4( sizeof( u_char ) * sw->analog_count ) 			
		+ Align4( sizeof( u_short ) * sw->press_button_count ) 
		+ Align4( sizeof( u_char ) * sw->press_data_count ) ;

	sw->save_header = GV_Malloc( size ) ;
	GV_ZeroMemory( sw->save_header, size ) ;
	sw->save_size = size ;
	printf( "padrec : save size %d\n", size ) ;

	if ( sw->save_header == NULL ) return -1 ;
	header = ( SV_PAD_HEADER * )sw->save_header ;
	sw->type = type = ( u_char * )( ( int )header + Align4( sizeof( SV_PAD_HEADER ) ) ) ;
	sw->button = button = ( u_short * )( ( int )sw->type 
										+ Align4( sizeof( u_char ) * sw->type_count ) ) ;
	sw->analog = analog = ( u_char * )( ( int )sw->button 
									   + Align4( sizeof( u_short ) * sw->button_count ) ) ;
	sw->press_button = press_button 
		= ( u_short * )( ( int )sw->analog + Align4( sizeof( u_char ) * sw->analog_count ) ) ;
	sw->press_data = press_data 
		= ( u_char * )( ( int )sw->press_button 
					   + Align4( sizeof( u_short ) * sw->press_button_count ) ) ;
	
	/* ヘッダーデータセット */
	header->magic = GV_StrCode( "MGS2PadData" ) & 0xff ;
	header->version = VERSION ;
	header->length = work->rec_count ;
	header->flag = 0 ;
	header->type = ( u_char * )( Align4( sizeof( SV_PAD_HEADER ) ) ) ;
	header->button = ( u_short * )( ( int )header->type 
								  + Align4( sizeof( u_char ) * sw->type_count ) ) ;
	header->analog = ( u_char * )( ( int )header->button 
								  + Align4( sizeof( u_short ) * sw->button_count ) ) ;
	header->press_button = ( u_short * )( ( int )header->analog 
										 + Align4( sizeof( u_char ) * sw->analog_count ) ) ;
	header->press_data = ( u_char * )( ( int )header->press_button
									  + Align4( sizeof( u_short ) * sw->press_button_count ) ) ;
	
	header->start_turn = work->start_player_turn ;
	DG_COPY_VEC( &header->start_pos, &work->start_player_pos ) ;
	header->start_time = work->start_time ;

	/* データセット */
	mem = work->memory ;
	count = work->rec_count ;	
	prev_button = 0 ;
	prev_press_button = 0 ;
	while( -- count >= 0 ) {
		tdata = 0 ;
		pbdata = 0 ;
		status = mem->status ;
		ai = mem->analog_input ;
		prs = mem->pressure ;
		if ( ( status & ~PAD_UDLR ) ||	/* 十字キー以外のキー */
			 ( ( status & PAD_UDLR ) && !( ai & GV_PAD_ANALOG_L_USE ) ) ) {	/* デジタル十字キー */
			tdata |= SV_TYPE_BUTTON ;
			if ( ai & GV_PAD_ANALOG_L_USE ) {
				status2 = status & ~PAD_UDLR ;
			} else {
				status2 = status ;
			}
			if ( prev_button != status2 ) {
				tdata |= SV_TYPE_BUTTON_CHANGED ;
				*( button ++ ) = status2 ;
			}
			if ( mem->type >= GV_PAD_DUALSHOCK2 && save_mode == SAVE_MODE_ALL ) {	
				/* 感圧セーブ */
				if ( ( status & PAD_UDLR ) && !( ai & GV_PAD_ANALOG_L_USE ) ) {
					/* 十字キー */
					if ( status & PAD_U ) *( press_data ++ ) = prs[ PAD_PRESS_U ] ;
					if ( status & PAD_D ) *( press_data ++ ) = prs[ PAD_PRESS_D ] ;
					if ( status & PAD_L ) *( press_data ++ ) = prs[ PAD_PRESS_L ] ;
					if ( status & PAD_R ) *( press_data ++ ) = prs[ PAD_PRESS_R ] ;
					pbdata |= status & PAD_UDLR ;
					tdata |= SV_TYPE_PRESS ;
				}
				if ( status & ~( PAD_UDLR | PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ) {
					/* 十字キー以外の感圧キー */
					if ( status & PAD_A ) *( press_data ++ ) = prs[ PAD_PRESS_A ] ;
					if ( status & PAD_B ) *( press_data ++ ) = prs[ PAD_PRESS_B ] ;
					if ( status & PAD_X ) *( press_data ++ ) = prs[ PAD_PRESS_X ] ;
					if ( status & PAD_Y ) *( press_data ++ ) = prs[ PAD_PRESS_Y ] ;
					if ( status & PAD_L1 ) *( press_data ++ ) = prs[ PAD_PRESS_L1 ] ;
					if ( status & PAD_R1 ) *( press_data ++ ) = prs[ PAD_PRESS_R1 ] ;
					if ( status & PAD_L2 ) *( press_data ++ ) = prs[ PAD_PRESS_L2 ] ;
					if ( status & PAD_R2 ) *( press_data ++ ) = prs[ PAD_PRESS_R2 ] ;
					pbdata |= status & ( PAD_ABXY | PAD_LR ) ;
					tdata |= SV_TYPE_PRESS ;
				}
				if ( pbdata != 0 && pbdata != prev_press_button ) {
					*( press_button ++ ) = pbdata ;
					tdata |= SV_TYPE_PRESS_BUTTON_CHANGED ;
				}
			}
		}
		/* アナログスティック */
		if ( ai & GV_PAD_ANALOG_L_USE ) {
			*( analog ++ ) = mem->left_dx ;
			*( analog ++ ) = mem->left_dy ;
			tdata |= SV_TYPE_ANALOG_L ;
		}
		if ( ai & GV_PAD_ANALOG_R_USE ) {
			*( analog ++ ) = mem->right_dx ;
			*( analog ++ ) = mem->right_dy ;
			tdata |= SV_TYPE_ANALOG_R ;
		}

		if ( ai & GV_PAD_ANALOG_L_USE ) {
			prev_button = status & ~PAD_UDLR ;
			prev_press_button = status & ~( PAD_UDLR | PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ;
		} else {
			prev_button = status ;
			prev_press_button = status & ~( PAD_STA | PAD_SEL | PAD_AR | PAD_AL ) ;
		}

		mem ++ ;
#if 0
		pauselevel = *( int * )mem ;
		mem = ( GV_PAD * )( ( int )mem + sizeof( int ) ) ;
		if ( pauselevel & GV_PAUSE_MENU ) tdata |= SV_TYPE_PAUSE_MENU ;
#endif
		*( type ++ ) = tdata ;
	}
	return 0 ;
}

static	void	Act( Work *work )
{
	int			fd ;

	if ( GM_LoadRequest & 1 ) {
		PadRecActor = 0 ;
		GV_DestroyActor( work ) ;
		return ;
	}

	switch( work->mode ) {
	case REC_MODE_STOP :
		if ( GM_Debug2PMode != GM_DEBUG_MODE_PADREC ) return ;
		DEBUG_Locate( EXPLAIN_X, EXPLAIN_Y, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "PAD RECORDER : STOP\n" ) ;
		DEBUG_Printf( "Press 2P-A to Record\n" ) ;
		if ( work->rec ) {
			DEBUG_Printf( "Press 2P-X to Play\n" ) ;
			DEBUG_Printf( "Press 2P-Y to Save\n" ) ;
		}
		if ( GV_PadData[ 1 ].press & PAD_A ) {
			GV_PadReleaseOn( work->which ) ;
			work->mode = REC_MODE_READY ;
			work->delay = 120 ;
		} else if ( work->rec && GV_PadData[ 1 ].press & PAD_X ) {
			if ( GM_PlayerControl != NULL ) {
				GM_ResetControlPosition( GM_PlayerControl, &work->start_player_pos ) ;
				GM_PlayerControl->turn.vy = GM_PlayerControl->rot.vy = work->start_player_turn ;
			}
			GV_Time = work->start_time - 1 ;
			work->mode = REC_MODE_PLAY ;
			work->memory = ( void * )PAD_MEMORY_ADDR ;
			work->count = 0 ;
		} else if ( work->rec && GV_PadData[ 1 ].press & PAD_Y ) {
			work->mode = REC_MODE_SAVE ;
			work->memory = ( void * )PAD_MEMORY_ADDR ;
			work->count = 0 ;
			if ( GV_PadData[ 1 ].status & PAD_R1 ) {
				work->save_mode = SAVE_MODE_ALL ;
				printf( "save all\n" ) ;
			} else {
//				work->save_mode = SAVE_MODE_NO_PRESSURE ;
				work->save_mode = SAVE_MODE_ALL ;
			}
		}
		break ;
	case REC_MODE_READY :
		DEBUG_Locate( EXPLAIN_X, EXPLAIN_Y, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "PAD RECORDER : READY\n" ) ;		
		if ( -- work->delay <= 0 ) {
			printf( "pad rec start!\n" ) ;
			GV_PadReleaseOff( work->which ) ;
			if ( GM_PlayerControl != NULL ) {
				DG_COPY_VEC( &work->start_player_pos, &GM_PlayerControl->mov ) ;
				work->start_player_turn = GM_PlayerControl->rot.vy ;
			}
			work->start_time = GV_Time ;
			work->mode = REC_MODE_REC ;
			work->memory = ( void * )PAD_MEMORY_ADDR ;
			work->rec_count = 0 ;
			work->rec = 1 ;
		}
		break ;
	case REC_MODE_REC :
		GM_SetGameStatus( STATE_PAD_DEMO ) ;
		GM_PadDemoVersion = VERSION ;
		DEBUG_Locate( EXPLAIN_X, EXPLAIN_Y, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "PAD RECORDER : RECORDING %d\n", work->rec_count / ( 300 / TIME_BASE ) ) ;
		DEBUG_Printf( "Press 2P-A to Print Time\n" ) ;
		DEBUG_Printf( "Press 2P-B to Stop\n" ) ;
		memcpy( work->memory, work->pad, sizeof( GV_PAD ) ) ;
		work->memory = (void *)( (int)work->memory + sizeof( GV_PAD ) ) ;
#if 0
		/* ポーズレベル */
		memcpy( work->memory, &GV_PauseLevel, sizeof( int ) ) ;
		work->memory += sizeof( int ) ;
#endif
		if ( GV_PadData[ 1 ].press & PAD_A ) {
			printf( "pad rec time : %d\n", work->rec_count ) ;
		}
		if ( ++ work->rec_count >= REC_TIME_MAX ||
			 GV_PadData[ 1 ].press & PAD_B ) {
			printf( "pad rec end!\n" ) ;
			work->mode = REC_MODE_STOP ;
			GM_ResetGameStatus( STATE_PAD_DEMO ) ;
		}
		break ;
	case REC_MODE_PLAY :
		GM_SetGameStatus( STATE_PAD_DEMO ) ;
		GM_PadDemoVersion = VERSION ;
		DEBUG_Locate( EXPLAIN_X, EXPLAIN_Y, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "PAD RECORDER : PLAY\n" ) ;
		DEBUG_Printf( "Press 2P-A to Print Time\n" ) ;
		DEBUG_Printf( "Press 2P-B to Stop\n" ) ;		
		DEBUG_Printf( "\n" ) ;		
		memcpy( work->pad, work->memory, sizeof( GV_PAD ) ) ;
		work->memory = (void *)( (int)work->memory + sizeof( GV_PAD ) ) ;
		DEBUG_Printf( "%x \n", work->pad->status ) ;
#if 0
		/* ポーズレベルの分は進める */
		work->memory += sizeof( int ) ;
		if ( GV_PadData[ 1 ].press & PAD_A ) {
			printf( "pad play time : %d\n", work->count ) ;
		}
#endif
		if ( ++ work->count >= work->rec_count ||
			 GV_PadData[ 1 ].press & PAD_B ) {
			printf( "pad play end!\n" ) ;
			work->mode = REC_MODE_STOP ;
			GM_ResetGameStatus( STATE_PAD_DEMO ) ;
		}			 
		break ;
	case REC_MODE_SAVE :
		DEBUG_Locate( EXPLAIN_X, EXPLAIN_Y, 0 ) ;
		DEBUG_Color( 255, 255, 255, 128 ) ;
		DEBUG_Printf( "PAD RECORDER : SAVING ...\n" ) ;		

		if ( MakeSaveData( work ) < 0 ) {
			printf( "padrec : save failed\n" ) ;
			work->mode = REC_MODE_STOP ;
			break ;			
		}

#ifdef PSX2
//んーーーやる？
		FlushCache( 0 ) ;
		fd = pcOpen( FILE_NAME, SCE_WRONLY|SCE_TRUNC|SCE_CREAT ) ;
		if ( fd < 0 ) {
			printf( "padrec : save failed\n" ) ;
			work->mode = REC_MODE_STOP ;
			break ;
		}
		if ( pcWrite( fd, work->save_work.save_header, work->save_work.save_size ) < 0 ) {
			printf( "padrec : save failed\n" ) ;
		} else {
			printf( "padrec : saving paddemo.rpd[ %d -> %d ]\n", 
				    sizeof( GV_PAD ) * work->rec_count, work->save_work.save_size ) ;
		}
		pcClose( fd ) ;
#endif
		GV_Free( work->save_work.save_header ) ;
		work->mode = REC_MODE_STOP ;
		break ;
	default :
	  ;
	}
}

static	void	Die( Work *work )
{
	PadRecActor = 0 ;
	GM_ResetGameStatus( STATE_PAD_DEMO ) ;
}

void	*NewPadRecorder( int which )
{
	Work		*work ;

	if ( PadRecActor != 0 ) return NULL ;
	PadRecActor = 1 ;
	ASSERT( which != 1 ) ;

	work = &PadRecWork ;
	GV_ZeroMemory( work, sizeof( Work ) ) ;
	GV_SetActorFreeFunc( &work->actor, NULL ) ;
	GV_SetActorClass( &work->actor, GV_CLASS_OBJECT ) ;
	GV_InsertActorPriority( GV_ACTOR_MANAGER, &work->actor, 0 ) ;

	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_SetActorKillLevel( &work->actor, GV_KILL_LEVEL_NORMAL ) ;
		GV_ActorEX( &work->actor ) ;	
		work->which = which ;
		work->pad = GV_PadData + which ;
		work->memory = ( void * )PAD_MEMORY_ADDR ;
		work->mode = REC_MODE_STOP ;
		work->delay = 0 ;
		work->rec = 0 ;
	}
	return work ;
}

#endif	/* DEBUG_MODE */
