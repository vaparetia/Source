//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	strmfadr.c
	擬音フェーダー

	2001/03/27 Y.Korekado
	$Id: strmfadr.c,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
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

#include	"gameheader.h"
#include 	"libfs.h"
#include	"strctrl.h"
#include	"jimaku.h"
/*----------------------------------------------------------------*/


/*----------------------------------------------------------------*/
#define	MAX_STRM_CTRL	(8)
typedef	struct	{
	GV_ACT_EX	actor ;

	int			name[ MAX_STRM_CTRL ] ;
	CONTROL		*strm_ctrl[ MAX_STRM_CTRL ] ;
	int			num ;
	int			status ;
	int			pan ;
	int			vol ;
	int			mode ;
	int			no_name_zero_delay ;
} Work ;

static	Work *WORK ;
/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */

#define NO_NAME_ZERO_DELAY	(4)	/* 名前がないときに音量継続フレーム */
/*----------------------------------------------------------------*/
#define STRM_FADAR_CTRL	0x00000001	/* コントロール中 */

/*----- -----*/
#define	STRFAD_MODE_NO_JIMAKU 0x00000001
/*----------------------------------------------------------------*/
int	KR_GetStrmFadarValue( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int name ;
	CONTROL	*ctrl ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

//printf(" get enemy pos [%d]\n",name ) ;
	ctrl = GM_SearchWhere( name ) ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)ctrl->mov.vx );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)ctrl->mov.vy );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)ctrl->mov.vz );

	return 1 ;
}

int	KR_GetStrmFadarVolume( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)WORK->vol );
	
	return 0 ;
}

#define MAX_JIMAKU_VOL	(0x38)
#define	SMALL_JIMAKU	(64)

static void JimakuZoom( int	vol ) 
{
	int rate ;
	
	if( vol > MAX_JIMAKU_VOL ) vol = MAX_JIMAKU_VOL ;
	rate = SMALL_JIMAKU + ((256-SMALL_JIMAKU) * vol / MAX_JIMAKU_VOL) ;
	GM_JimakuSetZoom( rate ) ;
}

#define PLAYER_MIC_READY ((Ply_GetPlayerWeapon() == WP_Mic)||(Ply_GetPlayerWeapon() == WP_DemoMic))
/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	int vol, pan, i, ctrl_flag ;
   float bp_angle;

	ctrl_flag = 0 ;
	for ( i=0; i<work->num; i++ ) {
		if ( work->name[ i ] == GM_StreamCaptionCurrentName ) {
			GM_SeGetVolPan( &work->strm_ctrl[ i ]->mov, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;
			work->pan = pan ;
			work->vol = vol ;
//			GM_VoxStreamSetPan( GM_StreamCaptionCurrentHandler, work->vol, work->pan ) ;
			GM_VoxStreamSetParam( GM_StreamCaptionCurrentHandler, &work->strm_ctrl[ i ]->mov,
								  GM_INVALID_ADDR,
								  work->vol, work->pan, bp_angle ) ;

			if( !(work->mode & STRFAD_MODE_NO_JIMAKU) ) {
				JimakuZoom( work->vol ) ;
			}

			work->status |= STRM_FADAR_CTRL ;
			work->no_name_zero_delay = NO_NAME_ZERO_DELAY ;
			ctrl_flag = 1 ;
//printf( " strmfadar: vol[%d] pan[%x] name[%d] wp[%d]\n",vol,pan,work->name[i],Ply_GetPlayerWeapon() ) ;
			break ;
		}
	}

	/* 指定名以外の台詞はノーマル再生 */
	if ( !(ctrl_flag) ) {
		if ( GM_StreamCaptionCurrentName == 0 ) {
			if ( (work->no_name_zero_delay--) <= 0 ) {
				work->no_name_zero_delay = 0 ;
				if ( work->status & STRM_FADAR_CTRL ) {
					GM_VoxStreamSetPan( GM_StreamCaptionCurrentHandler, GM_MAX_VOL, GM_PAN_CENTER, 0.f ) ;
					work->status &= ~STRM_FADAR_CTRL ;
				}
			} else {
//printf( " none name delay time[%d] \n",work->no_name_zero_delay ) ;
			}
		} else {
			if ( work->status & STRM_FADAR_CTRL ) {
				GM_VoxStreamSetPan( GM_StreamCaptionCurrentHandler, GM_MAX_VOL, GM_PAN_CENTER, 0.f ) ;
				work->status &= ~STRM_FADAR_CTRL ;
			}
		}
	}
}

static	void	Die( work )
Work		*work ;
{
}

/*----------------------------------------------------------------*/
static	int	GetResources( work )
Work	*work ;
{
	int i ;


	i = 0 ;
	if ( GCL_GetOption( 'n' ) != NULL ){
		while ( GCL_NextStr() != NULL ){
			work->name[ i++ ] = GCL_GetNextInt( ) ;
			printf(" SET Strm Ctrl name[%d] \n",work->name[ i-1 ] ) ;
		}
	} else {
		printf("strmfadr.c:No Name!! \n");
		return -1 ;
	}
	work->num = i ;

	for ( i=0; i<work->num; i++ ) {
		if ( (work->strm_ctrl[ i ] = GM_SearchWhere( work->name[ i ] )) == NULL ) {
			printf("strmfadr.c:No Control!! name[%d]\n", work->name[ i ] );
			return -1 ;
		}
	}
	work->status = 0 ;

	work->mode = GCL_GetOptionValue( 'm', 0 ) ;

	work->pan = GM_PAN_CENTER ;
	work->vol = 0 ;

	work->no_name_zero_delay = 0 ;

	WORK = work ;

	return 0 ;
}

void	*NewStrmFader( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
