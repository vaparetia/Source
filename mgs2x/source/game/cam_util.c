//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   cam_util.c
   カメラシステムユーティリティ
   
   1999/07/13 M.Sonoyama
   $Id: cam_util.c,v 1.1.1.3 2002/11/19 11:41:45 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"util.h"
#include	"camera.h"

#include "bp_vector.h"

/*---------------------------------------------------------------------------*/

/* カメラを登録 */
void	GM_EntryCamera( cam, name, chanl, level, prio )
GM_CameraSet	*cam ;
int		name, chanl, level, prio ;
{
    GM_CameraSet	*list, *now, *prev ;

	if ( GM_Camera == NULL ) return ;

    ASSERT( chanl >= 0 && chanl < MAX_CHANLS ) ;
    ASSERT( level >= GM_CAMERA_CUT_IN && level < GM_CAMERA_MAX ) ;
    ASSERT( prio >= 0 && prio < GM_CAMERA_PRIO_MAX ) ;

    GM_EntryCameraList( cam ) ;

    list = &( GM_Camera->chanl[ chanl ].list[ level ] ) ;
    prev = list ;
    now = prev->next ;
    while( 1 ) {
		if ( now == NULL ) break ;
		if ( (u_int)prio <= now->priority ) {
			cam->next = now ;
			prev->next = cam ;
			goto link_ok ;
		}	
		prev = now ;
		now = now->next ;
    }
    prev->next = cam ;
    cam->next = NULL ;
link_ok :
    cam->name = name ;
    cam->chanl = chanl ;
    cam->level = level ;
    cam->priority = prio ;
    /* スライドカメラ最初は無し */
    cam->slide_type = 0 ;
}


/* カメラを削除 */
void	GM_DeleteCamera( GM_CameraSet	*cam )
{
    GM_CameraSet	*list, *now, *prev ;

	if ( GM_Camera == NULL || cam == NULL ) return ;
	/* 消去ずみ */
	if ( cam->flag & CAM_FLAG_DELETED ) return ;

    ASSERT( cam->chanl >= 0 && cam->chanl < MAX_CHANLS ) ;
    ASSERT( cam->level >= GM_CAMERA_CUT_IN && cam->level < GM_CAMERA_MAX ) ;

	/* 補完カメラ後始末 */
	GM_FreeInterpCameraWork( cam ) ;

	/* 現在有効中のカメラを消す場合 */
	if ( GM_GetCurrentCameraSet( cam->chanl ) == cam ) {
		GM_ChangeCamera( cam->chanl ) ;
	}

    GM_DeleteCameraList( cam->name ) ;

    list = &( GM_Camera->chanl[ cam->chanl ].list[ cam->level ] ) ;
    prev = list ;
    now = prev->next ;
    while( now != NULL ) {
		if ( now == cam ) {
			prev->next = cam->next ;
			return ;
		}	
		prev = now ;
		now = now->next ;
    }
	//printf( "cam %x cannot be deleted\n", cam ) ;
}

/* 補完関数 */

/* 補完終了チェック */

static	int	DiffVec( FVECTOR *v1, FVECTOR *v2 )
{
	if ( DG_FABS( v1->vx - v2->vx ) > 64.0F ||
		 DG_FABS( v1->vy - v2->vy ) > 64.0F ||
		 DG_FABS( v1->vz - v2->vz ) > 64.0F ) return 1 ;
	return 0 ;
}

static	void	CheckInterpEnd( GM_CameraChanl *chanl, GM_CameraSet *cam1, GM_CameraSet *cam2 )
{
    if ( -- chanl->time > 0 ) return ;
	if ( DiffVec( &cam1->position, &cam2->position ) ||
		 DiffVec( &cam1->target, &cam2->target ) ) {
//		printf( "interp continue\n" ) ;
		return ;
	}
    chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
}

void	GM_CameraInterpQuick( chanl, cam1, cam2 ) 
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    memcpy( cam1, cam2, sizeof( GM_CameraSet ) ) ;
    chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
}

void	GM_CameraInterpExp4( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    GV_NearExp4BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
    GV_NearExp4VF( &cam1->position, &cam2->position, 3 ) ;
    GV_NearExp4VF( &cam1->target, &cam2->target, 3 ) ;
    cam1->rotate.vz = GV_NearExp4P( cam1->rotate.vz, cam2->rotate.vz ) ;
	cam1->angle = GV_NearExp4F( cam1->angle, cam2->angle ) ;
//	cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, chanl->time / 8 ) ;
//    if ( -- chanl->time <= 0 ) chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpExp8( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    GV_NearExp8BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
    GV_NearExp8VF( &cam1->position, &cam2->position, 3 ) ;
    GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;
//    cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, chanl->time / 4 ) ;
    cam1->rotate.vz = GV_NearExp8P( cam1->rotate.vz, cam2->rotate.vz ) ;
    cam1->angle = GV_NearExp8F( cam1->angle, cam2->angle ) ;
//    if ( -- chanl->time <= 0 ) chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpExp16( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    GV_NearExp16BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
    GV_NearExp16VF( &cam1->position, &cam2->position, 3 ) ;
    GV_NearExp16VF( &cam1->target, &cam2->target, 3 ) ;
//    cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, chanl->time / 2 ) ;
	cam1->angle = GV_NearExp16F( cam1->angle, cam2->angle ) ;
    cam1->rotate.vz = GV_NearExp16P( cam1->rotate.vz, cam2->rotate.vz ) ;
//    if ( -- chanl->time <= 0 ) chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpLinear( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    int		time ;
    
    time = chanl->time ;
	if ( time >= 0 ) {
      GV_NearTimeBP( &cam1->bp_settings, &cam2->bp_settings, time ); //BP_CAMERA - interpolate extra BP tweakables for HD
		GV_NearTimeVF( &cam1->position, &cam2->position, time, 3 ) ;
		GV_NearTimeVF( &cam1->target, &cam2->target, time, 3 ) ;
	} else {
      cam1->bp_settings = cam2->bp_settings;   //BP_CAMERA - interpolate extra BP tweakables for HD
		DG_COPY_VEC( &cam1->position, &cam2->position ) ;
		DG_COPY_VEC( &cam1->target, &cam2->target ) ;
	}
    if ( time > 0 ) cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, time ) ;
	else			cam1->angle = cam2->angle ;
//    if ( -- chanl->time < 0 ) chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpBehind( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
#if 0
    static int	Dir[ 2 ], Track ; 
    SVECTOR	rot ;
    FVECTOR	diff ;

    if ( chanl->time == 24 ) {
		_sceVu0SubVector( &diff, &cam1->target, &cam1->position ) ;
		Track = GV_VecLen3F( &diff ) ;
		Dir[ 0 ] = GV_VecDir2( &diff ) ;
		_sceVu0SubVector( &diff, &cam2->target, &cam2->position ) ;
		Dir[ 1 ] = GV_VecDir2( &diff ) ;
    }
    if ( GV_DiffDirAbs( Dir[ 0 ], Dir[ 1 ] ) > 1400 ) {
		rot.vx = cam2->rotate.vx ;
		rot.vz = cam2->rotate.vz ;
		rot.vy = Dir[ 0 ] = GV_NearExp4P( Dir[ 0 ], Dir[ 1 ] ) ;
		Track = GV_NearExp8( Track, cam2->track ) ;
		GM_CameraMakeTarget( &cam1->position, &cam1->target, 
							&rot, &Track ) ;
    } else {
		GV_NearExp8VF( &cam1->position, &cam2->position, 3 ) ;
		GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;
		cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, chanl->time / 4 ) ;
		cam1->rotate.vz = GV_NearExp8P( cam1->rotate.vz, cam2->rotate.vz ) ;
    }
#else
	static SVECTOR 	Rot[ 2 ] ;
	static int		Track ;
	FVECTOR			diff ;

    if ( chanl->time == 24 ) {
		_sceVu0SubVector( &diff, &cam1->target, &cam1->position ) ;
		Track = (int)GV_VecLen3F( &diff ) ;
		GV_VecToRot( &diff, &Rot[ 0 ] ) ;
		_sceVu0SubVector( &diff, &cam2->target, &cam2->position ) ;
		GV_VecToRot( &diff, &Rot[ 1 ] ) ;
    }	

   GV_NearExp8BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
	GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;
//	cam1->angle = GV_NearTimeF( cam1->angle, cam2->angle, chanl->time / 4 ) ;
	cam1->angle = GV_NearExp4F( cam1->angle, cam2->angle ) ;
	cam1->rotate.vz = GV_NearExp8P( cam1->rotate.vz, cam2->rotate.vz ) ;	
	
	if ( GV_DiffDirAbs( Rot[ 0 ].vy, Rot[ 1 ].vy ) > 160 ) {
		Track = GV_NearExp8( Track, cam2->track ) ;
		GV_NearExp4PV( &Rot[ 0 ], &Rot[ 1 ], 3 ) ;	
		GM_CameraMakeCamera( &cam1->position, &cam1->target,
							 &Rot[ 0 ], &Track ) ;
	} else {
		GV_NearExp8VF( &cam1->position, &cam2->position, 3 ) ;
	}

#endif
    if ( cam2->flag & CAM_FLAG_BOUND ) {
		GM_CameraBoundTrace( &( cam1->position ), &( cam2->bound1 ), 
							&( cam2->bound2 ) ) ;
    }
    if ( cam2->flag & CAM_FLAG_LIMIT ) {
		GM_CameraBoundTrace( &( cam1->target ), &( cam2->limit1 ), 
							&( cam2->limit2 ) ) ;
    }    
//    if ( -- chanl->time <= 0 ) chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpIntoBehind2( GM_CameraChanl *chanl, 
								    GM_CameraSet *cam1, GM_CameraSet *cam2 )
{
	int			dy ;

	if ( chanl->time == 48 ) {
//		cam1->track = 4000 ;
	}
	dy = GV_DiffDirAbs( cam1->rotate.vy, cam2->rotate.vy ) ;
	if ( chanl->time < 24 || dy < 256 ) {
		GV_NearExp8VF( &cam1->position, &cam2->position, 3 ) ;		
		GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;		
		GM_CameraMakeRotate( &( cam1->position ), &( cam1->target ), 
							 &( cam1->rotate ), &( cam1->track ) ) ;
	} else {
		if ( dy < 512 ) {
			GV_NearExp8PV( &cam1->rotate, &cam2->rotate, 3 ) ;
		} else {
			GV_NearExp8PV( &cam1->rotate, &cam2->rotate, 3 ) ;
		}
		if ( chanl->time < 32 ) {
			GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;
		} else {
			GV_NearExp8VF( &cam1->target, &cam2->target, 3 ) ;
		}
		cam1->track = GV_NearExp4( cam1->track, cam2->track ) ;
//		if ( cam2->track <= 4000 && cam1->track > 4000 ) cam1->track = 4000 ;
		GM_CameraMakeCamera( &( cam1->position ), &( cam1->target ), 
							 &( cam1->rotate ), &( cam1->track ) ) ;
	}
   GV_NearExp8BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
	cam1->rotate.vz = GV_NearExp8P( cam1->rotate.vz, cam2->rotate.vz ) ;
	cam1->angle = GV_NearExp8F( cam1->angle, cam2->angle ) ;
#if 0
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, &cam1->target, &cam2->position,
							    HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
		float		len ;
		FVECTOR		hit ;

		HZX_GetOnlineVector( &hit ) ;
		len = GV_VecLen3F( &hit ) ;
		if ( len > 100.0F ) {
			GV_LenVec3F( &hit, &hit, 0.0F, len - 100.0F ) ;
		}
		_sceVu0AddVector( &cam1->position, &cam1->target, &hit ) ;
	} 
#endif

    if ( cam2->flag & CAM_FLAG_BOUND ) {
		GM_CameraBoundTrace( &( cam1->position ), &( cam2->bound1 ), 
							&( cam2->bound2 ) ) ;
    }
    if ( cam2->flag & CAM_FLAG_LIMIT ) {
		GM_CameraBoundTrace( &( cam1->target ), &( cam2->limit1 ), 
							&( cam2->limit2 ) ) ;
    }    
	CheckInterpEnd( chanl, cam1, cam2 ) ;
}

void	GM_CameraInterpIntoSubject( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    int			time ;
	FVECTOR		tmpPos ;

    time = chanl->time ;

	/* 最初だけ何もしない */
	if ( time == TIME_INTO_SUBJECT ) {
		-- chanl->time ;
		return ;
	}

   GV_NearExp4BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD

    if ( time > TIME_INTO_SUBJECT / 2 ) {
		GV_NearExp4VF( &cam1->target, &cam2->position, 3 ) ;
		//GV_NearExp4PV( &cam1->rotate , &cam2->rotate, 3 ) ;
		cam1->rotate.vx = GV_NearExp2P( cam1->rotate.vx, cam2->rotate.vx ) ;
		cam1->rotate.vy = GV_NearExp4P( cam1->rotate.vy, cam2->rotate.vy ) ;
		cam1->rotate.vz = GV_NearExp4P( cam1->rotate.vz, cam2->rotate.vz ) ;
		cam1->track = GV_NearExp16( cam1->track, cam2->track ) ;
		GM_CameraMakeCamera( &( cam1->position ), &( cam1->target ), 
							 &( cam1->rotate ), &( cam1->track ) ) ;
    } else {
		GV_NearExp4VF( &cam1->position, &cam2->position, 3 ) ;
		GV_NearExp4PV( &cam1->rotate, &cam2->rotate, 3 ) ;
		cam1->track = GV_NearExp4( cam1->track, cam2->track ) ;
		GM_CameraMakeTarget( &( cam1->position ), &( cam1->target ), 
							&( cam1->rotate ), &( cam1->track ) ) ;
    }

    if ( cam1->flag & CAM_FLAG_BOUND ) {
//		GM_CameraBoundTrace( &( cam1->position ), &( cam1->bound1 ), 
//							&( cam1->bound2 ) ) ;
#if 0
        /* Ｙだけやってみる */
        if ( cam1->position.vy < cam1->bound1.vy ) 
			cam1->position.vy = cam1->bound1.vy ;
		else if ( cam1->position.vy > cam1->bound2.vy ) 
			cam1->position.vy = cam1->bound2.vy ;
#endif
    }
    if ( cam1->flag & CAM_FLAG_LIMIT ) {
//		GM_CameraBoundTrace( &( cam1->target ), &( cam1->limit1 ), 
//							&( cam1->limit2 ) ) ;
#if 0
        /* Ｙだけやってみる */
        if ( cam1->target.vy < cam1->limit1.vy ) 
			cam1->target.vy = cam1->limit1.vy ;
		else if ( cam1->target.vy > cam1->limit2.vy ) 
			cam1->target.vy = cam1->limit2.vy ;
#endif
    }    
#if 0
    if ( cam1->flag & CAM_FLAG_BOUND ) {
		GM_CameraBoundTrace( &( cam2->position ), &( cam1->bound1 ), 
							&( cam1->bound2 ) ) ;
    }
    if ( cam1->flag & CAM_FLAG_LIMIT ) {
		GM_CameraBoundTrace( &( cam2->target ), &( cam1->limit1 ), 
							&( cam1->limit2 ) ) ;
    }
#endif
	/* 水面チェック */
	if ( GM_WaterLevel != GM_WORLD_LIMIT_BOTTOM ) {
		if ( cam2->position.vy > GM_WaterLevel && 
			 cam1->position.vy < GM_WaterLevel ) {
			cam1->position.vy = GM_WaterLevel + 1.0F ;
		} else if ( cam2->position.vy < GM_WaterLevel && 
				    cam1->position.vy > GM_WaterLevel ) {
			cam1->position.vy = GM_WaterLevel - 1.0F ;
		}
	}
#if 1
	/* 壁チェック */
	_sceVu0SubVector( &tmpPos, &cam1->position, &cam2->position ) ;
	GV_LenVec3F( &tmpPos, &tmpPos, 0.0F, GV_VecLen3F( &tmpPos ) + 200.0F ) ;
	_sceVu0AddVector( &tmpPos, &cam2->position, &tmpPos ) ;
	if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &cam2->position, &tmpPos,
							    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
							    //HZX_SEG_NO_PLAYER , HZX_FLOOR_NO_PLAYER ) ) {
		                        HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ) {
		FVECTOR		v ;
		float		len ;

		HZX_GetOnlineVector( &v ) ;
		len = GV_VecLen3F( &v ) ;
		if ( len > 200.0F ) {
			GV_LenVec3F( &v, &v, 0.0F, len - 200.0F ) ;
		} else if ( len > 100.0F ) {
			GV_LenVec3F( &v, &v, 0.0F, len - 100.0F ) ;
		}
		_sceVu0AddVector( &cam1->position, &cam2->position, &v ) ;
		//GM_CameraMakeRotate( &( cam1->position ), &( cam1->target ), 
							   //&( cam1->rotate ), &( cam1->track ) ) ;
		GM_CameraMakeTarget( &( cam1->position ), &( cam1->target ), 
							 &( cam1->rotate ), &( cam1->track ) ) ;
	}
#endif
    cam1->angle = GV_NearExp4F( cam1->angle, cam2->angle ) ;
    if ( -- chanl->time < 0 ) {
		cam1->rotate = cam2->rotate ;
		cam1->track = cam2->track ;
//		chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
		CheckInterpEnd( chanl, cam1, cam2 ) ;
    }    
}

void	GM_CameraInterpOutSubject( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    int		time ;
    int		interp ;
    int		ret;

    time = TIME_OUT_SUBJECT - chanl->time ;
    ret = 0;

	if ( time < 4 ) {
		interp = 4 - time ;

		GV_NearTimeVF( &( cam1->target ), &cam2->target, interp, 3 ) ;
		GV_NearTimePV( &( cam1->rotate ), &cam2->rotate, interp, 3 ) ;
    } else {
		int ( *nearfunc )( int, int );
		void ( *nearfuncV )( void *, void *, int );
		void ( *nearfuncPV )( void *, void *, int );

		if( time > TIME_OUT_SUBJECT - 8 ){
			nearfuncV = GV_NearExp4VF ;
			nearfuncPV = GV_NearExp2PV ;
			nearfunc = GV_NearExp4 ;
		} else {
			nearfuncV = GV_NearExp4VF;
			nearfuncPV = GV_NearExp4PV;
			nearfunc = GV_NearExp4;
		}
		( *nearfuncV )( &cam1->target, &cam2->target, 3 );
		( *nearfuncPV )( &cam1->rotate, &cam2->rotate, 3 );
		cam1->track = ( *nearfunc )( cam1->track, cam2->track );
    }
    GV_NearExp4BP( &cam1->bp_settings, &cam2->bp_settings ); //BP_CAMERA - interpolate extra BP tweakables for HD
    cam1->angle = GV_NearExp4F( cam1->angle, cam2->angle ) ;
    GM_CameraMakeCamera( &( cam1->position ), &( cam1->target ), 
						&( cam1->rotate ), &( cam1->track ) ) ;

	/* 壁チェック */
	/* 移行先のカメラが位置ー注視点間に壁を持たないときのみ */
	/* 角度が大きく異なるときのみ */
	if ( ( GV_DiffDirAbs( cam1->rotate.vy, cam2->rotate.vy ) > 128 ||
		   GV_DiffDirAbs( cam1->rotate.vx, cam2->rotate.vx ) > 128 ) &&
		 !HZX_OnlineHazardCheck( HZX_CurrentGroupID, &cam2->target, &cam2->position,
								 HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
								 HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &cam2->target, &cam1->position,
								    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
								    HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
			FVECTOR		v ;
			float		len ;
#if 0			
			{
				HZX_HZD	hzd ;
				FVECTOR	hit ;

				HZX_GetOnlineHazard( &hzd, NULL ) ;
				HZX_ViewHazard( &hzd ) ;
				HZX_GetOnlinePoint( &hit ) ;
				DumpVec( &hit ) ;
			}
#endif
			HZX_GetOnlineVector( &v ) ;
			len = GV_VecLen3F( &v ) ;
			if ( DG_FABS( len ) < 10.0F ) {
				_sceVu0SubVector( &v, &cam2->position, &cam2->target ) ;
				GV_LenVec3F( &v, &v, 0.0F, 10.0F ) ;
			} else if ( len > 200.0F ) {
				GV_LenVec3F( &v, &v, 0.0F, len - 200.0F ) ;
			} else if ( len > 100.0F ) {
				GV_LenVec3F( &v, &v, 0.0F, len - 100.0F ) ;
			} else {
				GV_LenVec3F( &v, &v, 0.0F, len - 10.0F ) ;
			}
			_sceVu0AddVector( &cam1->position, &cam2->target, &v ) ;
			GM_CameraMakeTarget( &( cam1->position ), &( cam1->target ), 
								&( cam1->rotate ), &( cam1->track ) ) ;
		}
	} else {
#if 0
		HZX_HZD	hzd ;

		HZX_GetOnlineHazard( &hzd, NULL ) ;
		HZX_ViewHazard( &hzd ) ;
#endif
	}

	{
		FVECTOR			prepos, pretrg ;

		DG_COPY_VEC( &prepos, &cam1->position ) ;
		DG_COPY_VEC( &pretrg, &cam1->target ) ;

		if ( cam2->flag & CAM_FLAG_BOUND ) {
			GM_CameraBoundTrace( &( cam1->position ), &( cam2->bound1 ), 
								&( cam2->bound2 ) ) ;
		}
		if ( cam2->flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &( cam1->target ), &( cam2->limit1 ), 
								&( cam2->limit2 ) ) ;
		}
		if ( DG_FABS( prepos.vx - cam1->position.vx ) > 1.0F ||
			 DG_FABS( prepos.vy - cam1->position.vy ) > 1.0F ||
			 DG_FABS( prepos.vz - cam1->position.vz ) > 1.0F ||
			 DG_FABS( pretrg.vx - cam1->target.vx ) > 1.0F ||
			 DG_FABS( pretrg.vy - cam1->target.vy ) > 1.0F ||
			 DG_FABS( pretrg.vz - cam1->target.vz ) > 1.0F ) {
			/* バウンドに引っかかったら瞬時にする */
			DG_COPY_VEC( &cam1->position, &cam2->position ) ;
			DG_COPY_VEC( &cam1->target, &cam2->target ) ;
			GM_CameraMakeRotate( &( cam1->position ), &( cam1->target ), 
								 &( cam1->rotate ), &( cam1->track ) ) ;
			if ( chanl->time > 1 ) chanl->time = 1 ;
		}
	}

	/* 水面チェック */
	if ( GM_WaterLevel != GM_WORLD_LIMIT_BOTTOM ) {
		if ( cam2->position.vy > GM_WaterLevel && 
			 cam1->position.vy < GM_WaterLevel ) {
			cam1->position.vy = GM_WaterLevel + 1.0F ;
		} else if ( cam2->position.vy < GM_WaterLevel && 
				    cam1->position.vy > GM_WaterLevel ) {
			cam1->position.vy = GM_WaterLevel - 1.0F ;
		}
	}
    if ( -- chanl->time < 0 ) {
		cam1->rotate = cam2->rotate ;
		cam1->track = cam2->track ;
//		chanl->flag &= ~GM_FLAG_CAMERA_INTERP ;
		CheckInterpEnd( chanl, cam1, cam2 ) ;
    }    
	cam1->flag |= CAM_FLAG_FINISHCALC ;
}

/* 補完タイプをセット */
void	GM_SetCameraInterpMode( cam, mode1, mode2, time1, time2 )
GM_CameraSet		*cam ;
int			mode1, mode2, time1, time2 ;
{
    INTERP_FUNC		func = NULL ;

    switch ( mode1 ) {
    case GM_CAM_INTERP_QUICK :
		time1 = 0 ;
		func = GM_CameraInterpQuick ;
		break ;
    case GM_CAM_INTERP_EXP4 :
		time1 = 32 ;
		func = GM_CameraInterpExp4 ;
		break ;
    case GM_CAM_INTERP_EXP8 :
		time1 = 48 ;
		func = GM_CameraInterpExp8 ;
		break ;
    case GM_CAM_INTERP_EXP16 :
		time1 = 96 ;
		func = GM_CameraInterpExp16 ;
		break ;
    case GM_CAM_INTERP_LINEAR :
		func = GM_CameraInterpLinear ;
//#ifdef PAL
		time1 = DIRECT_TICK( time1 ) ;
//#endif
		break ;
    case GM_CAM_INTERP_INTO_SUBJECT :
		func = GM_CameraInterpIntoSubject ;
		time1 = TIME_INTO_SUBJECT ;
		break ;
    case GM_CAM_INTERP_BEHIND :
		func = (INTERP_FUNC)GM_CameraInterpBehind ;
		time1 = 24 ;
		break ;
	case GM_CAM_INTERP_INTO_BEHIND2 :
		func = GM_CameraInterpIntoBehind2 ;
		time1 = 48 ;
		break ;
    default :
		printf( "interp func error%d\n", mode1 ) ;
		ASSERT( 0 ) ;
    }
    cam->interp_time1 = time1 ;
    cam->interp_func1 = func ;

    switch ( mode2 ) {
    case GM_CAM_INTERP_NO_SET :
		time2 = 0 ;
		func = NULL ;
		break ;
    case GM_CAM_INTERP_QUICK :
		time2 = 0 ;
		func = GM_CameraInterpQuick ;
		break ;
    case GM_CAM_INTERP_EXP4 :
		time2 = 32 ;
		func = GM_CameraInterpExp4 ;
		break ;
    case GM_CAM_INTERP_EXP8 :
		time2 = 48 ;
		func = GM_CameraInterpExp8 ;
		break ;
    case GM_CAM_INTERP_EXP16 :
		time2 = 96 ;
		func = GM_CameraInterpExp16 ;
		break ;
    case GM_CAM_INTERP_LINEAR :
		func = GM_CameraInterpLinear ;
//#ifdef PAL
		time2 = DIRECT_TICK( time2 ) ;
//#endif
		break ;
    case GM_CAM_INTERP_OUT_SUBJECT :
		func = GM_CameraInterpOutSubject ;
		time2 = TIME_OUT_SUBJECT ;
		break ;
    case GM_CAM_INTERP_BEHIND :
		time2 = 24 ;
		func = (INTERP_FUNC)GM_CameraInterpBehind ;
		break ;
	case GM_CAM_INTERP_INTO_BEHIND2 :
		func = GM_CameraInterpIntoBehind2 ;
		time2 = 48 ;
		break ;
    default :
		printf( "interp2 func error %d\n", mode2 ) ;
		ASSERT( 0 ) ;
    }
    cam->interp_time2 = time2 ;
    cam->interp_func2 = func ;
}

/* カメラセット初期化 */
void	GM_InitCameraSet( GM_CameraSet *cam, FVECTOR *pos, FVECTOR *trg, 
						  float angle, int type, int on )
{
    cam->position = *pos ;
    cam->target = *trg ;
    cam->angle = angle ;
    cam->type = type ;
    cam->on = on ;
    cam->next = NULL ;
}

/*-----------------------------------------------------------------*/

void	GM_ChangeCamera( chanl )
int	chanl ;
{
	if ( GM_Camera == NULL ) return ;
    GM_Camera->chanl[ chanl ].flag |= GM_FLAG_CAMERA_CHANGED ;
}

/* 補完なしで現在のカメラをセットする */
/* 要毎フレームセット */
void	GM_SetCameraQuick( chanl )
int	chanl ;
{
	if ( GM_Camera == NULL ) return ;
    GM_Camera->chanl[ chanl ].flag |= GM_FLAG_CAMERA_QUICK ;
}

void	GM_UseCamera( chanl )
int	chanl ;
{
	if ( GM_Camera == NULL ) return ;
    if ( GM_Camera->chanl[ chanl ].flag & GM_FLAG_CAMERA_NO_USE ) {
		GM_Camera->chanl[ chanl ].flag &= ~GM_FLAG_CAMERA_NO_USE ;
		GM_Camera->chanl[ chanl ].flag |= GM_FLAG_CAMERA_FIRST ;
    }
}

void	GM_NoUseCamera( chanl )
int	chanl ;
{
	if ( GM_Camera == NULL ) return ;
    GM_Camera->chanl[ chanl ].flag |= GM_FLAG_CAMERA_NO_USE ;
}

/*---------------------------------------------------------------------------*/

/* 領域限定計算 */
static	inline	float	Bound( float v, float l, float h )
{
    if ( v < l ) v = l ;
    if ( v > h ) v = h ;
    return v ;    
}

void	GM_CameraBoundTrace( cam, b1, b2 )
FVECTOR		*cam ;
FVECTOR		*b1, *b2 ;
{
    cam->vx = Bound( cam->vx, b1->vx, b2->vx ) ;
    cam->vy = Bound( cam->vy, b1->vy, b2->vy ) ;
    cam->vz = Bound( cam->vz, b1->vz, b2->vz ) ;
}

/* 遊びつき追従計算 */
void	GM_CameraCushionTrace( trg, mov, cus )
FVECTOR	*trg, *mov ;
FVECTOR	*cus ;
{
    trg->vx = GV_NearRangeF( trg->vx, mov->vx, cus->vx ) ;
    trg->vy = GV_NearRangeF( trg->vy, mov->vy, cus->vy ) ;
    trg->vz = GV_NearRangeF( trg->vz, mov->vz, cus->vz ) ;
}

/* 足りないパラメータを生成 */
void	GM_CameraMakeRotate( cam, trg, rot, track )
FVECTOR	*cam, *trg ;
SVECTOR	*rot ;
int	*track ;
{
    FVECTOR  	tmp ;

    /* カメラ及びターゲット指定タイプから回転距離を求める */
    _sceVu0SubVector( &tmp, trg, cam ) ;
    *track = ( int )GV_VecLen3F( &tmp ) ;
	//  rot->vz = 0 ;
    rot->vy = GV_VecDir2( &tmp ) ;
    rot->vx = ( short )( atan2f( -tmp.vy, 
								sceVu0Sqrt( tmp.vx * tmp.vx + tmp.vz * tmp.vz ) ) 
						* 2048.0F / PI ) ;    
}

void	GM_CameraMakeRotate2( cam, trg, rot, track )
FVECTOR	*cam, *trg ;
SVECTOR	*rot ;
int	*track ;
{
    FVECTOR  	tmp ;

    /* カメラ及びターゲット指定タイプから回転距離を求める */
    _sceVu0SubVector( &tmp, trg, cam ) ;
	//  rot->vz = 0 ;
    rot->vy = GV_VecDir2( &tmp ) ;
    rot->vx = ( short )( atan2f( -tmp.vy, 
								sceVu0Sqrt( tmp.vx * tmp.vx + tmp.vz * tmp.vz ) ) 
						* 2048.0F / PI ) ;    
}

void	GM_CameraMakeTarget( cam, trg, rot, track )
FVECTOR	*cam, *trg ;
SVECTOR	*rot ;
int	*track ;
{
    FVECTOR	tmp ;

    GV_DirVec3( rot, *track, &tmp ) ;
    _sceVu0AddVector( trg, cam, &tmp ) ;
}

void	GM_CameraMakeCamera( cam, trg, rot, track )
FVECTOR	*cam, *trg ;
SVECTOR	*rot ;
int	*track ;
{
    FVECTOR	tmp ;

    GV_DirVec3( rot, *track, &tmp ) ;
    _sceVu0SubVector( cam, trg, &tmp ) ;
}

void	GM_CameraMakeTrack( cam, trg, rot, track )
FVECTOR	*cam, *trg ;
SVECTOR	*rot ;
int	*track ;
{
    FVECTOR	tmp ;

    _sceVu0SubVector( &tmp, cam, trg ) ;
    *track = ( int )GV_VecLen3F( &tmp ) ;
}

/* タイプセット */
void	GM_SetCameraType( cam, type, flag )
GM_CameraSet	*cam ;
int		type, flag ;
{
    cam->type = type ;
    cam->flag = flag ;
}

/* バウンドセット */
void	GM_SetCameraBound( GM_CameraSet *cam, float *bound )
{
    cam->bound1.vx = bound[ 0 ] ;
    cam->bound1.vy = bound[ 1 ] ;
    cam->bound1.vz = bound[ 2 ] ;
    cam->bound2.vx = bound[ 3 ] ;
    cam->bound2.vy = bound[ 4 ] ;
    cam->bound2.vz = bound[ 5 ] ;
}

/* リミットセット */
void	GM_SetCameraLimit( GM_CameraSet *cam, float *bound )
{
    cam->limit1.vx = bound[ 0 ] ;
    cam->limit1.vy = bound[ 1 ] ;
    cam->limit1.vz = bound[ 2 ] ;
    cam->limit2.vx = bound[ 3 ] ;
    cam->limit2.vy = bound[ 4 ] ;
    cam->limit2.vz = bound[ 5 ] ;
}

/* 角度セット */
void	GM_SetCameraRotate( cam, rot )
GM_CameraSet	*cam ;
SVECTOR		*rot ;
{
    cam->rotate.vx = rot->vx ;
    cam->rotate.vy = rot->vy ;
    cam->rotate.vz = rot->vz ;
}

/* トラックセット */
void	GM_SetCameraTrack( cam, track )
GM_CameraSet	*cam ;
int		track ;
{
    cam->track = track ;
}

/* アングルセット */
void	GM_SetCameraAngle( GM_CameraSet *cam, float angle )
{
    cam->angle = angle ;
}

/* 現在のカメラ状態 */
GM_CameraSet	*GM_GetCurrentCamera( int cn )
{
	if ( GM_Camera == NULL ) return NULL ;
    return &GM_Camera->chanl[ cn ].master ;
}

GM_CameraSet	*GM_GetCurrentCameraSet( int cn )
{
	if ( GM_Camera == NULL ) return NULL ;
    return GM_Camera->chanl[ cn ].next ;
}

/* 現在有効になっているカメラの補完関数を変える */
void		GM_ChangeCurrentCameraInterpFunc1( int chanl, void *func, int time )
{	
	GM_Camera->chanl[ chanl ].current.interp_func1 = func ;
	GM_Camera->chanl[ chanl ].current.interp_time1 = time ;
}	

void		GM_ChangeCurrentCameraInterpFunc2( int chanl, void *func, int time )
{	
	GM_Camera->chanl[ chanl ].current.interp_func2 = func ;
	GM_Camera->chanl[ chanl ].current.interp_time2 = time ;
}	

/* カメラ状態のコピー */
void		GM_CopyCamera( dst, src )
GM_CameraSet	*dst, *src ;
{
    dst->position = src->position ;
    dst->target = src->target ;
    dst->rotate = src->rotate ;
    dst->track = src->track ;
    dst->angle = src->angle ;
    dst->bp_settings = src->bp_settings; //BP_CAMERA - copy extra BP tweakables for HD
}

/* スライドカメラに渡す方向を反転 */
void		GM_ReverseSlideCameraDir( int chanl )
{
	if ( GM_Camera == NULL ) return ;
	GM_Camera->chanl[ chanl ].flag |= GM_FLAG_CAMERA_SLIDECAMDIR_REVERSE ;
}

/* 補完カメラワーク用意 */
int			GM_InitInterpCameraWork( GM_CameraSet *cam, int time )
{
	GM_CameraSet	*work ;

	/* すでにワークがある場合はそれを使用する */
	if ( cam->interp_camera_aim == NULL ) {
		work = ( GM_CameraSet * )GV_Malloc( sizeof( GM_CameraSet ) ) ;
		if ( work == NULL ) return -1 ;
	} else {
		work = cam->interp_camera_aim ;
	}
	cam->interp_camera_aim = work ;
	cam->interp_camera_time = DIRECT_TICK( time ) ;
	return 1 ;
}

/* 補完カメラワーク削除 */
void		GM_FreeInterpCameraWork( GM_CameraSet *cam )
{
	if ( cam->interp_camera_aim != NULL ) {
		GV_DelayedFree( cam->interp_camera_aim ) ;
		cam->interp_camera_aim = NULL ;
		cam->interp_camera_time = 0 ;
	}
}



/* カメラダンプ */
void	GM_DumpCamera( cam )
GM_CameraSet	*cam ;
{
#if 1
    printf( "---Dump Camera---\n" ) ;
	printf( "[name] %x\n", cam->name ) ;
    printf( "[pos] %.2f %.2f %.2f %.2f\n", cam->position.vx,
		   cam->position.vy, cam->position.vz, cam->position.vw ) ;
    printf( "[trg] %.2f %.2f %.2f\n", cam->target.vx,
		   cam->target.vy, cam->target.vz ) ;
#endif
    printf( "[angle] %f\n", cam->angle ) ;
#if 1
    printf( "[type] %d\n", cam->type ) ;    
    printf( "[bound1] %.2f %.2f %.2f\n", cam->bound1.vx,
		   cam->bound1.vy, cam->bound1.vz ) ;
    printf( "[bound2] %.2f %.2f %.2f\n", cam->bound2.vx,
		   cam->bound2.vy, cam->bound2.vz ) ;
    printf( "[limit1] %.2f %.2f %.2f\n", cam->limit1.vx,
		   cam->limit1.vy, cam->limit1.vz ) ;
    printf( "[limit2] %.2f %.2f %.2f\n", cam->limit2.vx,
		   cam->limit2.vy, cam->limit2.vz ) ;
#endif
    printf( "[rot] %d %d %d\n", cam->rotate.vx,
		   cam->rotate.vy, cam->rotate.vz ) ;
    printf( "[track] %d\n", cam->track ) ;
}
