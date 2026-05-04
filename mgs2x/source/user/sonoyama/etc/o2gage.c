//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   o2gage.c
   Ｏ２ゲージ

   2001/02/07	M.Sonoyama
   $Id: o2gage.c,v 1.3 2003/01/05 04:44:05 takaki Exp $
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
#include	"../raiden/pl_work.h"

#include "BP_Misc.h"

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	GM_GageSet			gage ;
	int					max ;
	float				bear_count ;
	int					invisible_count ;
	int					flag ;
	float				value ;
} Work ;

extern	float			GM_WaterLevel ;

static	Work			*O2GageWork = NULL ;

float			GM_O2RecoverValue ;
static float	GM_O2RecoverValueScn ;

enum {
	O2_FLAG_NONE		=		0x0000,
	O2_FLAG_INVISIBLE	=		0x0001,
	O2_FLAG_VISIBLE		=		0x0002,
} ;


static int GM_O2_PAD_ACTION ;

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GM_GageSet		*gs ;
	float			value, prev, dec, inc ;
	float			diff ;

	gs = &work->gage ;
	if ( GM_IsGameOver() || ( work->flag & O2_FLAG_INVISIBLE ) ) {
		GM_InvisibleGage( gs ) ;
		work->bear_count = 0.0F ;
		return ;
	}

#ifdef KP_XBOX
	if ( PL_PadType == 2 ) {
		GM_O2_PAD_ACTION = PAD_B ;
	} else {
		GM_O2_PAD_ACTION = PAD_X ;
	}
#endif

	prev = work->value ;
	diff = ( float )( GM_O2 - ( int )work->value ) ;
    value = work->value + diff ;

	dec = prev - value ;
	inc = 0.0F ;

	work->gage.flag &= ~GM_GAGE_WARNING ;
	if ( dec > 0.10F ||
		 GM_CheckPlayerStatus( PLAYER_NO_BREATH ) || 
		 GM_WaterLevel > GM_PlayerBody->objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ) {
	    if ( GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) ) return ;
		GM_VisibleGage( gs ) ;
		work->invisible_count = 0 ; 
		if ( ( GM_PlayerWork != NULL && GM_PlayerWork->pad->press & GM_O2_PAD_ACTION ) || 
			( GM_CheckPlayerStatusEX( I64(0), PLAYER2_SUBJECT_DEMO ) && ( GV_PadData->press & GM_O2_PAD_ACTION ) ) ) {
#ifndef KP_WINDOWS
         work->bear_count = (float)BP_BASE_TICK();	/* PL_PAD_ACTIONは我慢ボタン */

#else		// Windows版ではキー入力分解能、感圧エミュレーション等の兼ね合いでボタンの反応が
			// 悪いので補正(2002 12/31)
			work->bear_count = (float)BP_BASE_TICK() * 1.125F ;	/* PL_PAD_ACTIONは我慢ボタン */
#endif
		}
		if ( PL_PadEnable() && work->bear_count <= 0.0F ) {
			dec += 1.0F * ( float )TIME_BASE / 5.0F ;
			/* ライフがＭＡＸでないと減りが早い */
			if ( GM_Vitality < GM_VitalityMax ) dec += 1.0F * ( float )TIME_BASE / 5.0F ;
		}

		if ( work->bear_count > 0.0F ) {
			float		v ;

			if ( GM_O2RecoverValueScn > 0.0F ) {
				v = GM_O2RecoverValueScn ;

            inc += v / (float)BP_BASE_TICK();
				GM_O2RecoverValue = -1.0F ;
			} else {
				v = ( GM_O2RecoverValue > 0.0F ) ? GM_O2RecoverValue : -1.0F ;

            if ( BP_IsPAL()==TRUE )
				   v *= 1.20F;

         }
			work->bear_count -= 1.0F ;
			if ( v < 0.0F ) {
				/* 通常我慢中は減りを止める */
				dec = inc = 0.0F ;
			}
		}

		value = prev - dec + inc ;
		if ( value < ( float )gs->min ) value = ( float )gs->min ;
		else if ( value > ( float )gs->max ) value = ( float )gs->max ;

		gs->value = ( int )value ;
		work->value = value ;

		if ( gs->value == gs->min ) {
			/* 残量なし */
			/* ライフを減らす */
			if ( work->bear_count <= 0.0F &&
				( PL_PadEnable() || GM_CheckPlayerStatusEX( I64(0), PLAYER2_SUBJECT_DEMO ) ) ) {
				if ( GV_Time % DIRECT_TICK( 15 ) == 0 ) {
					GM_VitalityAdjust -= 1 ;
				}
			}
			if ( ( GV_Time % DIRECT_TICK( 90 ) ) == 0 && 
				 !GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) {
				/* 出血中は鳴らさない */
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				if ( GM_PlayerWork != NULL ) GM_PlayerWork->life.flag |= GM_GAGE_WARNING ;
			}
		} else if ( gs->value < gs->max / 3 ) {
			/* 1/3以下 */
			if ( ( GV_Time % DIRECT_TICK( 90 ) ) == 0 ) {
				/* 出血中は鳴らさない */
				if ( !GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) {
					GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				}
				work->gage.flag |= GM_GAGE_WARNING ;
			}
		} 
	} else {
		if ( value < ( float )gs->max ) {
			value += 24.0F * ( ( float )TIME_BASE / 5.0F ) ; 
		} else {
			value = ( float )gs->max ;
			if ( work->invisible_count < ( 300 / TIME_BASE ) * 4 ) {
				if ( ++ work->invisible_count >= ( 300 / TIME_BASE ) * 4 ) {
					GM_InvisibleGage( gs ) ;
				}
			}
		}
		gs->value = ( int )value ;
		work->value = value ;
		work->bear_count = 0.0F ;
	}
    /* Ｏ２無限カツラ */
    if ( PL_GetPlayerItem() == IT_WigA ) gs->value = gs->max ;
    GM_O2 = gs->value ;
    if ( work->flag & O2_FLAG_VISIBLE ) {
		/* 無理矢理表示 */
		GM_VisibleGage( gs ) ;
	}
	//printf( "[%d] %d %f\n", GV_Time, GM_O2, work->value ) ;
}

static	void	Die( Work *work )
{
	GM_RemoveGageSet( &work->gage ) ;
	O2GageWork = NULL ;
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work )
{
	GM_GageSet	*gs ;
	int			max, len ;

	max = GM_O2Max ;
	if ( GCL_GetOption( 'm' ) != NULL ) {
		max = GCL_GetNextInt() ;
	}
	GM_O2Max = GM_O2 = max ;
	work->value = ( float )max ;
	gs = &work->gage ;

	GM_O2 = GCL_GetOptionValue( 's', GM_O2 ) ;

	len = GCL_GetOptionValue( 'l', GM_O2Max / 15 ) ;
	GM_InitGageSet( gs, "O2", 16, len, 3, GM_O2, GM_O2Max, 0, 30, 
				    GM_GAGE_LEVEL_PLAYER_O2 ) ;
	//gs->text_len = 23 ;
	//GM_SetGageColor( gs, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_PLAYER_O2 ) ;
	GM_AppendGageSet( gs ) ;
	GM_O2RecoverValue = -1.0F ;
	GM_O2RecoverValueScn = -1.0F ;

#ifndef KP_XBOX
	GM_O2_PAD_ACTION = PAD_X ;
#endif

	return 0 ;
}

/*----------------------------------------------------------------*/

/* Ｏ２ゲージ */
void	*NewO2Gage( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_CHECK_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	O2GageWork = work ;
	return work ;
}

/*----------------------------------------------------------------*/

/* 外部からの操作 */
void	PL_VisibleO2Gage( void )
{
	ASSERT( O2GageWork != NULL ) ;
	O2GageWork->flag |= O2_FLAG_VISIBLE ;
	O2GageWork->flag &= ~O2_FLAG_INVISIBLE ;
}

void	PL_InvisibleO2Gage( void )
{
	ASSERT( O2GageWork != NULL ) ;
	O2GageWork->flag |= O2_FLAG_INVISIBLE ;
	O2GageWork->flag &= ~O2_FLAG_VISIBLE ;
}

int		PL_COM_VisibleO2Gage( void )
{
	PL_VisibleO2Gage() ;
	return 0 ;
}

int		PL_COM_InvisibleO2Gage( void )
{
	PL_InvisibleO2Gage() ;
	return 0 ;
}

int		PL_COM_SetRecoverValue( void )
{
	GM_O2RecoverValueScn = ( float )GCL_GetNextInt() ;	
	return 0 ;
}
