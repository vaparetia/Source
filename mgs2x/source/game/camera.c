//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   camera.c
   カメラ更新関数
   
   1999/07/13 M.Sonoyama
   $Id: camera.c,v 1.1.1.3 2002/11/19 11:41:46 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
#include	"camera.h"

#include "bp_vector.h"


/*--------------------------------------------------------------*/
//BP_CAMERA - extra camera functionality
/*--------------------------------------------------------------*/

// 1st person camera overrides
int      gBP_1stPersonCamera_Override   = 0;             // Override game default?
int      gBP_1stPersonCamera_Move       = 1;             // Move mode: 0=off, >0 = on
int      gBP_1stPersonCamera_Toggle     = 0;             // Toggle mode: 0=off, 1=on
int      gBP_1stPersonCamera_Active     = 0;             // Runtime state: 0=3rd person, 1=1st person

// 3rd person camera overrides
int      gBP_3rdPersonCamera_Override  = 0;              // Override game default?
FVECTOR  gBP_3rdPersonCamera_Target    = {0,0,0,0};      // Target position that camera looks at
FVECTOR  gBP_3rdPersonCamera_Eye       = {0,0,0,0};      // Eye position where camera is placed
SVECTOR  gBP_3rdPersonCamera_Rot       = {0,2048,0,0};   // Rotation around player
int      gBP_3rdPersonCamera_Dist      = 4000;           // Distance from player
float    gBP_3rdPersonCamera_HSpeed    = 0.6f;           // Horizontal rotation speed
float    gBP_3rdPersonCamera_VSpeed    = 0.4f;           // Vertical rotation speed

// Camera overrides
int      gBP_Camera_InheritRot         = 0;              // Inherit rotation between cameras?

// Camera functions
int BP_NormalizeIntegerRot( int r )
{
   while( r < -2048 )
   {
      r += 4096;
   }
   while( r > 2048 )
   {
      r -= 4096;
   }
   return r;
}

/*--------------------------------------------------------------*/


#define	TRACE_CUSHION	(750.0F)
#define	TRACE_CUSHION_Y	(300.0F)

/*--------------------------------------------------------------*/

/* Global */

/* Cameras follow characters */
FVECTOR	GM_CameraTargetValue[ MAX_CHANLS ] ;
SVECTOR	GM_CameraDirValue[ MAX_CHANLS ] ;
SVECTOR	GM_CameraRotateValue[ MAX_CHANLS ] ;
float	GM_CameraAngleValue[ MAX_CHANLS ] ;
int	GM_CameraTrackValue[ MAX_CHANLS ] ;
int	GM_CameraModeValue[ MAX_CHANLS ] ;

FVECTOR	*GM_CameraTargetPtr[ MAX_CHANLS ] = { NULL } ;

#if 0
FVECTOR	GM_CameraTarget ;
SVECTOR	GM_CameraDir ;
SVECTOR	GM_CameraRotate ;
float	GM_CameraAngle ;
int	GM_CameraTrack ;
int	GM_CameraMode ;
#endif

typedef struct {
    GV_ACT		actor ;
    GM_CameraSet	camera ;
} Work ;

/* Wait for the camera*/
void	*GM_DelayCameraWork ;
int		GM_CameraDelayTime[ MAX_CHANLS ] ;

/*--------------------------------------------------------------*/

/* If there is no set play */
static	void	NoCushionAct( target, cushion, flag, chanl )
FVECTOR		*target, *cushion ;
int		flag, chanl ;
{
    if ( flag & CAM_FLAG_NO_CUSHION_X ) {
		target->vx = GM_CameraTargetValue[ chanl ].vx ;		
    } else {
		target->vx = GV_NearRangeF( target->vx, 
								   GM_CameraTargetValue[ chanl ].vx, cushion->vx ) ;
    }
    if ( flag & CAM_FLAG_NO_CUSHION_Z ) {
		target->vz = GM_CameraTargetValue[ chanl ].vz ;		
    } else {
		target->vz = GV_NearRangeF( target->vz, 
								   GM_CameraTargetValue[ chanl ].vz, cushion->vz ) ;
    }	    
    if ( flag & CAM_FLAG_NO_CUSHION_Y ) {
		target->vy = GM_CameraTargetValue[ chanl ].vy ;		
    } else {
		target->vy = GV_NearRangeF( target->vy, 
								   GM_CameraTargetValue[ chanl ].vy, cushion->vy ) ;
    }	    
}

/* Slide the camera left and right */
static	void	LRSlideCameraAct( cam )
GM_CameraSet	*cam ;
{
    short	dir ;	
    int		count ;
    float	v ;

    dir = GM_CheckPlayerStatus( PLAYER_CAUTION ) ? GM_PlayerCautionDir : GM_PlayerDir ;
	if ( GM_Camera->chanl[ cam->chanl ].flag & GM_FLAG_CAMERA_SLIDECAMDIR_REVERSE ) {
		dir += 2048 ;
	}
    dir &= 4095 ;
    if ( GV_DiffDirAbs( dir, 1024 ) < 512 ) {
		/* Right */
		if ( cam->slide_wait2 < 255 ) cam->slide_wait2 ++ ; /* Once with a limit */
		count = cam->slide_wait2 - cam->slide_wait_count ;
		if ( count > 0 ) {
			cam->slide_interp += cam->slide_step ;
			if ( cam->slide_interp > 1.0F ) cam->slide_interp = 1.0F ;
		}
    } else if ( GV_DiffDirAbs( dir, 3072 ) < 512 ) {
		/* Left */
		if ( cam->slide_wait1 < 255 ) cam->slide_wait1 ++ ; /* Once with a limit */
		count = cam->slide_wait1 - cam->slide_wait_count ;
		if ( count > 0 ) {
			cam->slide_interp -= cam->slide_step ;
			if ( cam->slide_interp < 0.0F ) cam->slide_interp = 0.0F ;
		}
    } else {
		cam->slide_wait1 = cam->slide_wait2 = 0 ;
    }
    v = cam->slide1 + ( cam->slide2 - cam->slide1 ) * cam->slide_interp ;
    cam->position.vx = cam->target.vx + v ;
    cam->target.vx += v ;
    GM_CameraBoundTrace( &( cam->target ), &( cam->limit1 ), &( cam->limit2 ) ) ;
    GM_CameraMakeRotate( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
}

/* Slide the camera back and forth */
static	void	FBSlideCameraAct( cam )
GM_CameraSet	*cam ;
{
    short	dir ;	
    int		count ;
    float	v ;

    dir = GM_CheckPlayerStatus( PLAYER_CAUTION ) ? GM_PlayerCautionDir : GM_PlayerDir ;
	if ( GM_Camera->chanl[ cam->chanl ].flag & GM_FLAG_CAMERA_SLIDECAMDIR_REVERSE ) {
		dir += 2048 ;
	}
    dir &= 4095 ;
    if ( GV_DiffDirAbs( dir, 0 ) < 512 ) {
		/* Positive */
		if ( cam->slide_wait1 < 255 ) cam->slide_wait1 ++ ; /* Once with a limit */
		count = cam->slide_wait1 - cam->slide_wait_count ;
		if ( count > 0 ) {
			cam->slide_interp -= cam->slide_step ;
			if ( cam->slide_interp < 0.0F ) cam->slide_interp = 0.0F ;
		}
    } else if ( GV_DiffDirAbs( dir, 2048 ) < 512 ) {
		/* Backward */
		if ( cam->slide_wait2 < 255 ) cam->slide_wait2 ++ ; /* Once with a limit*/
		count = cam->slide_wait2 - cam->slide_wait_count ;
		if ( count > 0 ) {
			cam->slide_interp += cam->slide_step ;
			if ( cam->slide_interp > 1.0F ) cam->slide_interp = 1.0F ;
		}
    } else {
		cam->slide_wait1 = cam->slide_wait2 = 0 ;
    }
    v = cam->slide1 + ( cam->slide2 - cam->slide1 ) * cam->slide_interp ;
    cam->position.vz = cam->target.vz + v ;
    GM_CameraMakeRotate( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
}

/* Tracking camera processing */
static	void	TraceCameraAct( work, flag )
Work		*work ;
int		flag ;
{
    static FVECTOR	Cushion = { TRACE_CUSHION, TRACE_CUSHION_Y,
									TRACE_CUSHION } ;
    FVECTOR		diff, cushion ;
    GM_CameraSet	*cam ;
    int			chanl, rx, ry ;

    cam = &( work->camera ) ;
    chanl = cam->chanl ;
    cushion = Cushion ;
    if ( cam->track < 2000 ) {
		cushion.vx -= ( 2000.0F - cam->track ) / 2.0F ;
		if ( cushion.vx < 250.0F ) {
			cushion.vx = 250.0F ;
		}
		cushion.vz = cushion.vx ;
    }

    if ( cam->type == GM_CAM_TYPE_TARGET_AND_ROTATE ) {
		if ( cam->flag & CAM_FLAG_NO_CUSHION ) {
			NoCushionAct( &cam->target, &cushion, cam->flag, cam->chanl ) ;
		} else {
			GM_CameraCushionTrace( &( cam->target ), &GM_CameraTargetValue[ chanl ], &cushion ) ;
		}
		if ( flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &( cam->target ), &( cam->limit1 ), 
								&( cam->limit2 ) ) ;
		}
		if ( flag & CAM_FLAG_DIR_TRACE ) cam->rotate = GM_CameraDirValue[ chanl ] ;
		if ( cam->slide_type != 0 ) {
			GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
								&( cam->slide_rot ), &( cam->slide_track ) ) ;
			if ( cam->slide_type == 1 ) {
				/* Slide the camera left and right */
				LRSlideCameraAct( cam ) ;
			} else {
				/* Slide the camera back and forth */
				FBSlideCameraAct( cam ) ;
			}
		} else {
			GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
								&( cam->rotate ), &( cam->track ) ) ;
		}
		if ( flag & CAM_FLAG_BOUND ) {
			GM_CameraBoundTrace( &( cam->position ), &( cam->bound1 ), 
								&( cam->bound2 ) ) ;
		}
    } else if ( cam->type == GM_CAM_TYPE_CAMERA_AND_TARGET ) {
		if ( flag & CAM_FLAG_FIX_TARGET_TRACE ) {
			/* Fixed Target Tracking */
			_sceVu0SubVector( &diff, &( cam->target ), &GM_CameraTargetValue[ chanl ] ) ;
			GV_LenVec3F( &diff, &diff, GV_VecLen3F( &diff ), ( float )cam->track ) ;
			_sceVu0SubVector( &( cam->position ), &( cam->target ), &diff ) ;
			if ( flag & CAM_FLAG_BOUND ) {
				GM_CameraBoundTrace( &( cam->position ), &( cam->bound1 ), 
									&( cam->bound2 ) ) ;
			}	    
			GM_CameraMakeRotate2( &( cam->position ), &( cam->target ), 
								 &( cam->rotate ), &( cam->track ) ) ;
		} else {
			/* Fixed Camera Tracking */
			if ( cam->flag & CAM_FLAG_NO_CUSHION ) {
				NoCushionAct( &cam->target, &cushion, cam->flag, cam->chanl ) ;
			} else {
				GM_CameraCushionTrace( &( cam->target ), &GM_CameraTargetValue[ chanl ], &cushion ) ;
			}
			if ( flag & CAM_FLAG_LIMIT ) {
				GM_CameraBoundTrace( &( cam->target ), &( cam->limit1 ), 
									&( cam->limit2 ) ) ;
			}
			rx = cam->rotate.vx ;
			ry = cam->rotate.vy ;
			GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
								&( cam->rotate ), &( cam->track ) ) ;
			if ( flag & CAM_FLAG_FIX_X_TRACE ) {
				/* Have not made ??yet */
			}
			if ( flag & CAM_FLAG_YROT_FIX_TRACE ) {
				if ( GV_DiffDirAbs( cam->rotate.vy, ry ) > 960 ) {
					cam->rotate.vx = rx ;
				} 
				if ( cam->rotate.vx < -960 ) cam->rotate.vx = rx ;
				if ( cam->rotate.vx > 960 ) cam->rotate.vx = rx ;
				cam->rotate.vy = ry ;
				GM_CameraMakeTarget( &( cam->position ), &( cam->target ), 
									 &( cam->rotate ), &( cam->track ) ) ;
			}
		}
    } else if ( cam->type == GM_CAM_TYPE_TARGET_AND_ROTATE ) {
		if ( cam->flag & CAM_FLAG_NO_CUSHION ) {
			NoCushionAct( &cam->target, &cushion, cam->flag, cam->chanl ) ;
		} else {
			GM_CameraCushionTrace( &( cam->target ), &GM_CameraTargetValue[ chanl ], &cushion ) ;
		}
		if ( flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &( cam->target ), &( cam->limit1 ), 
								&( cam->limit2 ) ) ;
		}
		GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;
    }

    //BP_CAMERA - override 3rd person
    if(gBP_3rdPersonCamera_Override)
    {
       // Override camera
       BP_Vec3_Copy( &cam->target, &gBP_3rdPersonCamera_Target );             //target
       BP_Vec3_Copy( &cam->position, &gBP_3rdPersonCamera_Eye );              //position
       cam->track     = gBP_3rdPersonCamera_Dist;                             //tracking dist
       cam->rotate.vx = BP_NormalizeIntegerRot( gBP_3rdPersonCamera_Rot.vx ); //pitch
       cam->rotate.vy = BP_NormalizeIntegerRot( gBP_3rdPersonCamera_Rot.vy ); //yaw
       cam->rotate.vz = BP_NormalizeIntegerRot( gBP_3rdPersonCamera_Rot.vz ); //roll

       // Set pad rotation adjust so player moves in the correct direction
       cam->flag |= CAM_FLAG_PAD_ADJUST_BP_3RD_PERSON;
    }
    else
    {
       // Remove pad rotation adjust
       cam->flag &= ~CAM_FLAG_PAD_ADJUST_BP_3RD_PERSON;
    }
    //BP_CAMERA - override 3rd person
}

static	void	FixCameraAct( work, flag )
Work		*work ;
int		flag ;
{
    GM_CameraSet	*cam ;
    int			chanl ;

    cam = &( work->camera ) ;
    chanl = cam->chanl ;
    if ( cam->type == GM_CAM_TYPE_CAMERA_AND_ROTATE ) {
		if ( flag & CAM_FLAG_DIR_TRACE ) cam->rotate = GM_CameraDirValue[ chanl ] ;
		GM_CameraMakeTarget( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;
    } else if ( cam->type == GM_CAM_TYPE_CAMERA_AND_TARGET ) {
		GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;
    } else if ( cam->type == GM_CAM_TYPE_TARGET_AND_ROTATE ) {
		GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;
    }
}

static	void	InterpCameraAct( GM_CameraSet *cam )
{
	GM_CameraSet	*aim ;
	int				time ;

	/* Do not update while paused */
	if ( GV_PauseLevel != 0 ) return ;

	time = cam->interp_camera_time ;
	if ( time <= 0 ) {
		GM_FreeInterpCameraWork( cam ) ;
		cam->flag &= ~CAM_FLAG_INTERP_CAMERA_SET ;
	} else {
		aim = cam->interp_camera_aim ;
		GV_NearTimeVF( &cam->position, &aim->position, time, 3 ) ;
		GV_NearTimeVF( &cam->target, &aim->target, time, 3 ) ;
		cam->angle = GV_NearTimeF( cam->angle, aim->angle, time ) ;
		-- cam->interp_camera_time ;
		GM_SetCameraQuick( cam->chanl ) ;
	}
}

/*--------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GM_CameraSet	*cam ;
    int				flag ;

	if ( GM_Camera == NULL ) return ;

	cam = &work->camera ;
    flag = cam->flag ;
    if ( flag & CAM_FLAG_KILL ) {
		GM_DeleteCamera( cam ) ;
		GV_DestroyActor( work ) ;
		return ;
    }

	if ( flag & CAM_FLAG_DELETED ) {
		GV_DestroyActor( work ) ;
		return ;		
	}

    /* trap for special handling measures */
    if ( flag & CAM_FLAG_ON_NOW ) cam->flag &= ~CAM_FLAG_ON_NOW ;

    if ( cam->on == 0 ) return ;

    /* Target Check */
    if ( GM_CameraTargetPtr[ cam->chanl ] != NULL ) {
		_sceVu0CopyVector( &GM_CameraTargetValue[ cam->chanl ],
						   GM_CameraTargetPtr[ cam->chanl ] ) ;
    }

	if ( flag & CAM_FLAG_INTERP_CAMERA_SET ) {
		InterpCameraAct( cam ) ;
	} else if ( flag & CAM_FLAG_TRACE ) {
		TraceCameraAct( work, flag ) ;
    } else {
		FixCameraAct( work, flag ) ;
    }

//if ( work->camera.name == GV_StrCode( "ディレイカメラ" ) ) {
//	printf( "[%d] delay camera %x\n", GV_Time, &work->camera ) ;
//    GM_DumpCamera( &( work->camera ) ) ;
//}
}

static	void	Die( work )
Work		*work ;
{
	//    printf( "delay camera end\n" ) ;
	printf( "camera %x die\n", work->camera.name ) ;
    GM_DeleteCamera( &work->camera ) ;
}

/*--------------------------------------------------------------*/

static	int	GetResources( work, name )
Work		*work ;
int name;
{
    static int	Level[] = { GM_CAMERA_CUT_IN, GM_CAMERA_PROG2,
								GM_CAMERA_AREA, GM_CAMERA_DEFAULT } ;
    GM_CameraSet	*cam ;
    int			chanl, level, prio ;

    chanl = level = prio = 0 ;
    if ( GCL_GetOption( 'c' ) != NULL ) {
		chanl = GCL_GetNextInt() ;
    }
    if ( GCL_GetOption( 'l' ) != NULL ) {
		level = Level[ GCL_GetNextInt() ] ;
    }
    if ( GCL_GetOption( 'p' ) != NULL ) {
		prio = GCL_GetNextInt() ;
    }
    cam = &( work->camera ) ;
	if ( GM_FindCameraSet( name, chanl ) != NULL ) {
		printf( "warning : camera %x is already exists\n" ) ;
		return 255 ;
	}
    GM_EntryCamera( cam, name, chanl, level, prio ) ;
    return 0 ;
}

/*--------------------------------------------------------------*/

/* Camera start */
void	*NewCamera( name, map )
int	name, map ;
{
    Work	*work ;
	int		ret = 0 ;

	if ( GM_Camera == NULL ) return NULL ;

    work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
								  sizeof( Work ), CAMERA_PRIO ) ;
	if ( work == NULL ) return NULL ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( work, Act, Die ) ;
		if ( ( ret = GetResources( work, name ) ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
	if ( ret == 255 ) GV_DestroyActor( work ) ;
	printf( "scn camera %d start!\n", name ) ;
    return work ;
}

GM_CameraSet	*NewProgramCamera( name, chanl, level, prio )
int		name ;
int		chanl, level, prio ;
{
    Work		*work ;
    GM_CameraSet	*cam = NULL ;

	if ( GM_Camera == NULL ) return NULL ;

	if ( GM_FindCameraSet( name, chanl ) != NULL ) {
		printf( "warning : prg camera %x is already exists\n", name ) ;
		return NULL ;
	}

printf( "camera %d start!( %d %d %d )\n", name, chanl, level, prio ) ;

    work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
								  sizeof( Work ), CAMERA_PRIO ) ;
	if ( work == NULL ) return NULL ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( work, Act, Die ) ;
		cam = &( work->camera ) ;
		GM_EntryCamera( cam, name, chanl, level, prio ) ;
    }
    return cam ;
}

/* Wait Cameras */
void	*NewDelayCamera( orgcam, time, prio )
GM_CameraSet	*orgcam ;
int		time ;
int		prio ;
{
    Work		*work ;
    GM_CameraSet	*cam ;

	if ( GM_Camera == NULL ) return NULL ;

    if ( GM_DelayCameraWork != NULL ) GV_DestroyOtherActorQuick( GM_DelayCameraWork ) ;
    GM_DelayCameraWork = NULL ;
	//    if ( GM_DelayCameraWork != NULL ) return NULL ;
//	printf( "delay camera start[%d]\n", GV_Time ) ;
    work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
								  sizeof( Work ), CAMERA_PRIO ) ;
	if ( work == NULL ) return NULL ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( work, Act, Die ) ;
		cam = &( work->camera ) ;
		*cam = *orgcam ;
		GM_EntryCamera( cam, GV_StrCode( "ディレイカメラ" ), 
					   cam->chanl, cam->level, prio ) ;
		GM_DelayCameraWork = work ;
		GM_CameraDelayTime[ orgcam->chanl ] = time + 1 ;
		GM_ChangeCamera( orgcam->chanl ) ;
		Act( work ) ;
    }
    return work ;
}
