//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   paddemo.c
   パッドデモ再生

   2000/10/03	M.Sonoyama
   $Id: paddemo.c,v 1.1.1.3 2002/11/19 11:50:44 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
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
#include "BP_EndianSupport.h"

extern int MENU_AutoSelectItem;
extern int MENU_AutoSelectWeapon;
extern void MENU_QuickChangeItem();
extern void MENU_QuickChangeWeapon();

/*----------------------------------------------------------------*/

/* SCE のパッドバッファを読むための構造体 */

#define PRESSURE_SIZE 12

/* system/libgv/pad.c と同じにすること */
typedef struct {
	unsigned char flag;
	unsigned char id;
	unsigned char button[ 2 ];
	struct {
		unsigned char data[ 4 ];
	} analog;
	struct {
		unsigned char data[ PRESSURE_SIZE ];
	} pressure;
} PADBUF_DATA;

/* padrec.c と同じにすること */
typedef	struct	{
	u_char		magic ;
	u_char		version ;
	u_short		length ;
	u_int		flag ;
	u_char		*type ;
	u_short		*button_le ;

	u_char		*analog ;
	u_short		*press_button_le ;
	u_char		*press_data ;	
	int			start_turn ;

	FVECTOR		start_pos ;
	int			start_time ;

   // size is 52 to this point
   // SV_PAD_HEADER is 16 byte aligned
   u_char      *command;
   char         pad[8];
} SV_PAD_HEADER ;

enum {
	SV_TYPE_NONE =					0x00,
	SV_TYPE_BUTTON =				0x01,
	SV_TYPE_ANALOG_L =				0x02,
	SV_TYPE_ANALOG_R =				0x04,	
	SV_TYPE_PRESS =					0x08,
	SV_TYPE_BUTTON_CHANGED = 		0x10,
	SV_TYPE_PRESS_BUTTON_CHANGED = 	0x20,
	SV_TYPE_PAUSE_MENU =			0x40,
   SV_TYPE_COMMAND =          0x80
} ;

enum
{
   SV_PADDEMO_COMMAND_SWITCHWEAPON,
   SV_PADDEMO_COMMAND_SWITCHITEM
};

/*----------------------------------------------------------------*/

typedef	struct _paddemo_Work	{
	GV_ACT				actor ;

	SV_PAD_HEADER		*header ;
	int					count ;
	u_char				*type ;
	u_short				*button_le ;

	u_char				*analog ;
	u_short				*press_button_le ;
	u_char				*press_data ;
	u_short				button_cur ;
	u_short				press_button_cur ;
   u_char            *command ;

	int					which ;
	int					proc ;
	int					name ;
	int					mode ;
} SPadDemoWork ;

enum {
	PD_MODE_READY = 0,
	PD_MODE_PLAY,
	PD_MODE_DESTROY,
} ;

enum {
	PD_MSG_START = 0,
	PD_MSG_STOP,
	PD_MSG_KILL
} ;

/*----------------------------------------------------------------*/

static	void	CheckMessage( SPadDemoWork *work, int mode )
{
	GV_MSG		*msg ;
	int			n_msg ;

	/* 同フレーム複数メッセージは不許可な作りにしてある */
	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		switch( msg->message[ 0 ] ) {
		case PD_MSG_START :
			if ( mode == PD_MODE_READY ) work->mode = PD_MODE_PLAY ;
			return ;
		case PD_MSG_STOP :
			if ( mode == PD_MODE_PLAY ) work->mode = PD_MODE_READY ;
			return ;
		case PD_MSG_KILL :
			work->mode = PD_MODE_DESTROY ;
			work->proc = 0 ; /* メッセージできられた場合は
								procコール無し */
			return ;
		default :
		  ;
		}
		msg ++ ;
	}
}

static	void	SetPressData( SPadDemoWork *work, PADBUF_DATA *p, u_short button ) 
{
	u_char		*data ;

	data = p->pressure.data ;
	if ( button & PAD_U ) {
		data[ PAD_PRESS_U ] = *( work->press_data ++ ) ;
	}	
	if ( button & PAD_D ) {
		data[ PAD_PRESS_D ] = *( work->press_data ++ ) ;
	}	
	if ( button & PAD_L ) {
		data[ PAD_PRESS_L ] = *( work->press_data ++ ) ;
	}	
	if ( button & PAD_R ) {
		data[ PAD_PRESS_R ] = *( work->press_data ++ ) ;
	}	
	if ( button & PAD_A ) {
		data[ PAD_PRESS_A ] = *( work->press_data ++ ) ;
	}
	if ( button & PAD_B ) {
		data[ PAD_PRESS_B ] = *( work->press_data ++ ) ;
	}
	if ( button & PAD_X ) {
		data[ PAD_PRESS_X ] = *( work->press_data ++ ) ;
	}
	if ( button & PAD_Y ) {
		data[ PAD_PRESS_Y ] = *( work->press_data ++ ) ;
	}
	if ( button & PAD_L1 ) {
		data[ PAD_PRESS_L1 ] = *( work->press_data ++ ) ;	
	}
	if ( button & PAD_R1 ) {
		data[ PAD_PRESS_R1 ] = *( work->press_data ++ ) ;	
	}
	if ( button & PAD_L2 ) {
		data[ PAD_PRESS_L2 ] = *( work->press_data ++ ) ;	
	}
	if ( button & PAD_R2 ) {
		data[ PAD_PRESS_R2 ] = *( work->press_data ++ ) ;	
	}
}

static void EndianSwapPadHeader( SV_PAD_HEADER *pData )
{
   if ( !( pData->version & 0x80 ) )
   {
      BP_LE_SwapSShort_Inp( &pData->length );
      BP_LE_SwapSInt_Inp( &pData->flag );
      BP_LE_SwapPtr_Inp( &( pData->type ) );
      BP_LE_SwapPtr_Inp( &( pData->button_le ) );
      BP_LE_SwapPtr_Inp( &( pData->analog ) );
      BP_LE_SwapPtr_Inp( &( pData->press_button_le ) );
      BP_LE_SwapPtr_Inp( &( pData->press_data ) );
      BP_LE_SwapSInt_Inp( &pData->start_turn );
      BP_LE_SwapFloatArray_Inp( &pData->start_pos.vx, 4 );
      BP_LE_SwapSInt_Inp( &pData->start_time );
      BP_LE_SwapPtr_Inp( &( pData->command ) );

      pData->version |= 0x80;
   }
}

static	void	SetPadData( SPadDemoWork *work )
{
	PADBUF_DATA		pbd, *p ;
	SV_PAD_HEADER	*def ;
	int				ptype ;
	u_char			type ;
	u_short			button ;
	//int				pauselevel, playpauselevel ;

	ptype = GV_PAD_DIGITAL ;

	def = work->header ;
   
	if ( work->count == 0 ) {
		GV_Time = def->start_time - 1 ;
		if ( GM_PlayerControl != NULL ) {
			GM_PlayerControl->rot.vy = GM_PlayerControl->turn.vy 
				= def->start_turn ;
			GM_ResetControlPosition( GM_PlayerControl, &def->start_pos ) ;
		} 
		GV_PadReleaseOn( work->which ) ;
		GM_SetGameStatus( STATE_PAD_DEMO ) ;
		GM_PadDemoVersion = def->version & 0x7F;
		printf( "paddemo version [%d]\n", GM_PadDemoVersion ) ;
	}

	p = &pbd ;
	GV_ZeroMemory( p, sizeof( PADBUF_DATA ) ) ;
	p->button[ 0 ] = p->button[ 1 ] = 0xff ;
	p->analog.data[ 0 ] = p->analog.data[ 1 ] = 128 ;
	p->analog.data[ 2 ] = p->analog.data[ 3 ] = 128 ;

	type = *( work->type ) ;
#if 0
	if ( type & SV_TYPE_PAUSE_MENU ) playpauselevel = GV_PAUSE_MENU ;
	else							 playpauselevel = 0 ;
	pauselevel = GV_PauseLevel & GV_PAUSE_MENU ;

	if ( ( def->version & 0x7f ) >= 3 && 
		 pauselevel != playpauselevel ) {
		/* ポーズ状態が異なっているときはウェイト */
		return ;
	}
#endif
	work->type ++ ;

	if ( type & SV_TYPE_BUTTON ) {
		if ( type & SV_TYPE_BUTTON_CHANGED ) {
			button = BP_LE_SwapUShort( *( work->button_le ++ ) ) ^ 0xffff ;
			work->button_cur = button ;
		} else {
			button = work->button_cur ;			
		}
		p->button[ 0 ] = ( ( button & 0xff00 ) >> 8 ) & 0xff ;
		p->button[ 1 ] = ( button & 0x00ff ) & 0xff ;
	}
	if ( type & SV_TYPE_ANALOG_L ) {
		p->analog.data[ 2 ] = *( work->analog ++ ) ;
		p->analog.data[ 3 ] = *( work->analog ++ ) ;
		ptype = GV_PAD_DUALSHOCK ;
	}
	if ( type & SV_TYPE_ANALOG_R ) {
		p->analog.data[ 0 ] = *( work->analog ++ ) ;
		p->analog.data[ 1 ] = *( work->analog ++ ) ;
		ptype = GV_PAD_DUALSHOCK ;
	}
	if ( type & SV_TYPE_PRESS ) {
		if ( type & SV_TYPE_PRESS_BUTTON_CHANGED ) {
			button = BP_LE_SwapUShort( *( work->press_button_le ++ ) ) ;
			work->press_button_cur = button ;
		} else {
			button = work->press_button_cur ;
		}
		SetPressData( work, p, button ) ;
		ptype = GV_PAD_DUALSHOCK2 ;
	}
   if ( type & SV_TYPE_COMMAND )
   {
      u_char command = *work->command++;
      u_char data = *work->command++;
      switch (command)
      {
      case SV_PADDEMO_COMMAND_SWITCHWEAPON:
         if (data == 0xff)
         {
            MENU_QuickChangeWeapon();
         }
         else
         {
            MENU_AutoSelectWeapon = data;
         }
         break;
      case SV_PADDEMO_COMMAND_SWITCHITEM:
         if (data == 0xff)
         {
            MENU_QuickChangeItem();
         }
         else
         {
            MENU_AutoSelectItem = data;
         }
         break;
      }
   }

	GV_PadSetDemoData( work->which, ptype, p ) ;

	if ( GV_PadDataDirect[ 1 ].press & PAD_A ) {
#ifdef PAL        //BP JG - not gonna covert this. :-)
		printf( "paddemo : time < %d > ( delay time %d )\n", work->count,
			   ( ( work->count * 6 ) - 1 ) / 5 ) ;
#else
		printf( "paddemo : time < %d >\n", work->count ) ;
#endif
	}

	if ( ++ work->count >= def->length ) {
		work->mode = PD_MODE_DESTROY ;
	}
}

static	void	Act( SPadDemoWork *work )
{
	int			mode, new ;

	mode = work->mode ;
	CheckMessage( work, mode ) ;
	new = work->mode ;
	switch( mode ) {
	case PD_MODE_READY :
		return ;
	case PD_MODE_PLAY :
		SetPadData( work ) ;
#if 0
		if ( GV_PadDataDirect[ 0 ].release & PAD_B ) {
			/* キャンセル */
			work->mode = PD_MODE_DESTROY ;
		}
#endif
		break ;
	case PD_MODE_DESTROY :
		GV_PadReleaseOff( work->which ) ;
		GM_ResetGameStatus( STATE_PAD_DEMO ) ;
		if ( work->proc != 0 ) {
			GM_ExecProc( work->proc, NULL ) ;
			work->proc = 0 ;
		}
		GV_DestroyActor( work ) ;
	}
}

static	void	Die( SPadDemoWork *work )
{
	
}

/*----------------------------------------------------------------*/

static	int		GetResources( SPadDemoWork *work )
{
	int				file, head ;
	SV_PAD_HEADER	*def ;

	work->which = GCL_GetOptionValue( 'w', 0 ) ;
	work->proc = GCL_GetOptionValue( 'e', 0 ) ;
	file = GCL_GetOptionValue( 'f', 0 ) ;
	if ( file == 0 ){
	    printf( "paddemo.c : Need file!!!!!!!!!!!!!!!!!\n" ) ;
	    return -1 ;
	}
	work->header = def = ( SV_PAD_HEADER * )GV_GetCache( GV_CacheID( file, 'r' ) ) ;
	if ( def == NULL ) {
	    printf( "paddemo.c : cannot find PADDEMO-file!!!!!!!!!!!!!!!\n" ) ;
	    return -1 ;
	}

   EndianSwapPadHeader( def );

	head = ( int )def ;
	work->type = ( u_char * )( head + ( int )def->type ) ;
	work->button_le = ( u_short * )( head + ( int )def->button_le ) ;
	work->analog = ( u_char * )( head + ( int )def->analog ) ;
	work->press_button_le = ( u_short * )( head + ( int )def->press_button_le ) ;
	work->press_data = ( u_char * )( head + ( int )def->press_data ) ;
   work->command = (u_char *)( head + ( int )def->command ) ;

	return 0 ;
}

void			*NewPadDemoPlay( int name, int where )
{
	SPadDemoWork		*work ;

	work = ( SPadDemoWork * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( SPadDemoWork ), PLAYER_PADDEMO_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->name = name ;
	}
	return work ;
}
