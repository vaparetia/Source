//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bgmfader.c
	ＢＧＭフェーダー

	2000/06/30 Y.Korekado
	$Id: bgmfader.c,v 1.2 2002/12/26 11:04:53 Yoshizawa1 Exp $
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
#include	"korekado/conv/define.h"
#include	"camera.h"
/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */

#define MAX_AREA	8	/*  */
#define NO_PAN		(0x20)	/* パン０ */
#define MAX_TRACK	(32)
/*----------------------------------------------------------------*/
typedef struct {
	FVECTOR	pos ;
	SVECTOR	size ;
	int		group_id ;
	int		*base_vol ;	/* ０～１００％ */
} NOISE_AREA ;

typedef	struct	{
	GV_ACT_EX	actor ;

	NOISE_AREA	noisearea[8] ;
	u_char	n_area ;
	u_char	track ;
	char	pan ;
	char	vol ;

	int		status ;
	int		mode ;
} Work ;

/*----------------------------------------------------------------*/
/* status */
#define BGM_FADER_REVERS	0x00000001	/* 反転フェーダー */
#define BGM_FADER_NO_PAN	0x00000002	/* ＰＡＮ真ん中で固定 */

/*----------------------------------------------------------------*/
extern void BGM_SetFaderWork( int track, void *work ) ;
extern void BGM_UnsetFaderWork( int track ) ;
extern void *BGM_GetFaderWork( int track ) ;
extern int BGM_GetFaderAreaNum( int track ) ;
/*----------------------------------------------------------------*/
static int ChangeValue( int from, int to, int interp )
{
	if ( to > from + interp ) {
		return (from + interp) ;
	} else if ( to < from - interp ) {
		return (from - interp) ;
	}
	return to ;
}

static	void	BoxNearPoint( FVECTOR *from, FVECTOR *to, FVECTOR *vec, SVECTOR *size )
{
    _sceVu0SubVector( vec, to, from ) ;
//printf("0:[%f][%f][%f] [%f][%f][%f]\n",to->vx, to->vy, to->vz, vec->vx, vec->vy, vec->vz  ) ;

    if( vec->vx > (float)size->vx ) {
		vec->vx = from->vx + (float)size->vx ;
    } else if ( vec->vx < (float)(-size->vx) ) {
		vec->vx = from->vx - (float)size->vx ;
	} else {
		vec->vx = to->vx ;
	}

    if( vec->vy > (float)size->vy ) {
		vec->vy = from->vy + (float)size->vy ;
    } else if ( vec->vy < (float)(-size->vy) ) {
		vec->vy = from->vy - (float)size->vy ;
	} else {
		vec->vy = to->vy ;
	}

    if( vec->vz > (float)size->vz ) {
		vec->vz = from->vz + (float)size->vz ;
    } else if ( vec->vz < (float)(-size->vz) ) {
		vec->vz = from->vz - (float)size->vz ;
	} else {
		vec->vz = to->vz ;
	}
}

static	void	ReversBoxNearPoint( FVECTOR *from, FVECTOR *to, FVECTOR *vec, SVECTOR *size )
{
    _sceVu0SubVector( vec, to, from ) ;
//printf("0:[%f][%f][%f] [%f][%f][%f]\n",to->vx, to->vy, to->vz, vec->vx, vec->vy, vec->vz  ) ;

    if( vec->vx > 0 ) {
	    if( vec->vx > (float)size->vx ) {
			vec->vx = to->vx ;
		} else {
			vec->vx = from->vx + (float)size->vx ;
		}
	} else {
	    if( vec->vx < (float)(-size->vx) ) {
			vec->vx = to->vx ;
		} else {
			vec->vx = from->vx - (float)size->vx ;
		}
	}

    if( vec->vy > 0 ) {
	    if( vec->vy > (float)size->vy ) {
			vec->vy = to->vy ;
		} else {
			vec->vy = from->vy + (float)size->vy ;
		}
	} else {
	    if( vec->vy < (float)(-size->vy) ) {
			vec->vy = to->vy ;
		} else {
			vec->vy = from->vy - (float)size->vy ;
		}
	}

    if( vec->vz > 0 ) {
	    if( vec->vz > (float)size->vz ) {
			vec->vz = to->vz ;
		} else {
			vec->vz = from->vz + (float)size->vz ;
		}
	} else {
	    if( vec->vz < (float)(-size->vz) ) {
			vec->vz = to->vz ;
		} else {
			vec->vz = from->vz - (float)size->vz ;
		}
	}
}

static	void	BgmFader( work )
Work		*work ;
{
	GM_CameraSet	*cam ;
	FVECTOR	pos, nearpos ;
	int i, mindis, dis, *base, area ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* メイン画面のみ */

	/* 
		一番近い音の発生源を調べる 
		*/
	mindis = 1000000 ;
	nearpos = DG_ZeroVector ;
	base = NULL ;
	area = 0 ;
	for( i=0; i<work->n_area; i++ ) {
		if ( work->status & BGM_FADER_REVERS ) {
			ReversBoxNearPoint( &work->noisearea[ i ].pos, &cam->position, &pos, &work->noisearea[ i ].size ) ;
		} else {
			BoxNearPoint( &work->noisearea[ i ].pos, &cam->position, &pos, &work->noisearea[ i ].size ) ;
		}
		dis = (int)GV_VecLen3F2( &cam->position, &pos) ;

		if ( dis < mindis ) {
			mindis = dis ;
			nearpos = pos ;
			base = work->noisearea[ i ].base_vol ;
			area = i ;
		}
	}
//if( work->track == 0x17 ) printf("area[%d] pos:[%f][%f][%f] ",area, nearpos.vx, nearpos.vy, nearpos.vz ) ;
	if ( work->status & BGM_FADER_NO_PAN ) {
		int vol ;

		vol = GM_SeGetVol( &nearpos, GM_SEMODE_NORMAL ) ;
		if( base != NULL ) vol = vol * (*base) / 100 ;
		work->vol = ChangeValue( work->vol, vol, 0x10 ) ;
		
		GM_MixConvFader( work->track, NO_PAN, work->vol ) ;
	} else {
		int vol, pan ;
      float bp_angle;
		GM_SeGetVolPan( &nearpos, GM_SEMODE_NORMAL, &vol, &pan, &bp_angle ) ;
//if( work->track == 0x17 ) printf("vol[%x] pan[%x]",vol, pan ) ;
		if( base != NULL ) vol = vol * (*base) / 100 ;
//if( work->track == 0x17 ) printf("*base:[%d] = vol[%d]",*base, vol ) ;
		work->vol = (u_char)ChangeValue( work->vol, vol, 0x10 ) ;
		work->pan = (u_char)ChangeValue( work->pan, pan, 0x10 ) ;
		
//if( work->track == 0x17 ) printf(">vol[%x] pan[%x]\n", work->vol, work->pan ) ;
		GM_MixConvFader( work->track, work->pan, work->vol ) ;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	switch ( work->mode ) {
		case ALERT_MODE_SNEAK :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				BgmFader( work ) ;
			} else {
//				GM_InitMixConvFader( ) ;
				work->mode = GM_AlertMode ;
			}
		break ;
		default :
			if ( GM_AlertMode == ALERT_MODE_SNEAK ) {
				work->mode = GM_AlertMode ;
			}
		break ;
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
	extern	int	KR_GetNextFV( FVECTOR *fvec ) ;

	/* トラック番号読み込み */
	if ( (work->track = GCL_GetOptionValue( 't', 0 )) == 0 ) {
		return -1 ;
	}
#ifdef KP_WINDOWS
	if(work->track < 0x10){
		 work->track+= 0x10;
	}
#endif
	/* ノイズエリア読み込み */
	work->n_area = 0 ;
	if ( GCL_GetOption( 'a' ) != NULL ){
		while ( GCL_NextStr() != NULL ){
			if ( work->n_area >= MAX_AREA ) {
				printf(" bgmfader: Noise Area Over!! MAX=8!!") ;
				return -1 ;
			}

			KR_GetNextFV( &work->noisearea[ work->n_area ].pos ) ;
			GCL_GetNextSV( (short *)&work->noisearea[ work->n_area ].size ) ;
			work->noisearea[ work->n_area ].group_id = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt( )) ) ;
			work->noisearea[ work->n_area ].base_vol = NULL ;
			work->n_area++ ;
		}
	} else {
		return -1 ;
	}

	work->status = GCL_GetOptionValue( 's', 0 ) ;
	work->mode = ALERT_MODE_SNEAK ;
	
	work->pan = 0 ;
	work->vol = 0 ;

//	printf("bgmfader: set track[%x] area num[%d]\n",work->track, work->n_area ) ;

	return 0 ;
}

static	int	GetResources2( Work	*work, int track, int *base_vol, int status,
					FVECTOR *pos, SVECTOR *size, int group_id )
{
	work->track = track ;
	work->status = status ;
	work->mode = ALERT_MODE_SNEAK ;

	work->n_area = 1 ;

	work->noisearea[ 0 ].base_vol = base_vol ;
	work->noisearea[ 0 ].pos = *pos ;
	work->noisearea[ 0 ].size = *size ;
	work->noisearea[ 0 ].group_id = group_id ;

	work->pan = 0 ;
	work->vol = 0 ;

	printf("bgmfader: set track[%x] area num[%d]\n",work->track, work->n_area ) ;

	return 0 ;
}

void	*NewBgmFader( int name, int where )
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

void	*NewBgmFaderControl( int track, int *base_vol, int status,
					FVECTOR *pos, SVECTOR *size, HZX_GROUP_ID group_id )
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources2( work, track, base_vol, status, pos, size, group_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*----------------------------------------------------------------*/
int		BGM_SetFader( int track, int *base_vol, int status,
					FVECTOR *pos, SVECTOR *size, HZX_GROUP_ID group_id )
{
	Work *work ;
	
	if( (work = BGM_GetFaderWork( track )) == NULL ) {
		work = NewBgmFaderControl( track, base_vol, status, pos, size, group_id ) ;
		if( work != NULL ) BGM_SetFaderWork( track, work ) ;
		return work->n_area ;
	}

	work->noisearea[ work->n_area ].base_vol = base_vol ;
	work->noisearea[ work->n_area ].pos = *pos ;
	work->noisearea[ work->n_area ].size = *size ;
	work->noisearea[ work->n_area ].group_id = group_id ;
	work->n_area ++ ;

	return work->n_area ;
}

/*----------------------------------------------------------------*/
