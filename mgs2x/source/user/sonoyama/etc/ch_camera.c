/*
   ch_camera.c
   キャラ追従カメラ

   1999/11/08 M.Sonoyama
   $Id: ch_camera.c,v 1.1.1.3 2002/11/19 11:50:41 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef _XBOX
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

#define		PAD_ONOFF	(PAD_Y)

typedef	struct {
    GV_ACT	actor ;
    FVECTOR		shift ;
    SVECTOR		rot ;
    CONTROL		*ctrl ;
    GM_CameraSet	*camera ;
} Work ;

/*-----------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    GM_CameraSet	*cam ;

    if ( work->ctrl == NULL ) {
	GV_DestroyActor( work ) ;
	return ;
    }
    cam = work->camera ;
    _sceVu0AddVector( &cam->position, &work->ctrl->mov, &work->shift ) ;
    cam->track = 4000 ;
    GM_CameraMakeTarget( &cam->position, &cam->target, &work->rot, &cam->track ) ;

    if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
	 GM_PlayerDebugMode == GM_PDM_CH_CAMERA ) {
	int	status ;    

	cam->on = 1 ;
	GM_ChangeCamera( work->camera->chanl ) ;
	status = GV_PadData[ 1 ].status ;
	if ( !( status & PAD_R1 ) ) {
	    if ( status & PAD_U ) work->shift.vy += 10.0F ;
	    if ( status & PAD_D ) work->shift.vy -= 10.0F ;
	    if ( status & PAD_L ) work->shift.vx += 10.0F ;
	    if ( status & PAD_R ) work->shift.vx -= 10.0F ;
	    if ( status & PAD_X ) work->shift.vz += 10.0F ;
	    if ( status & PAD_B ) work->shift.vz -= 10.0F ;
	} else {
	    if ( status & PAD_U ) work->rot.vx -= 2 ;
	    if ( status & PAD_D ) work->rot.vx += 2 ;
	    if ( status & PAD_L ) work->rot.vy -= 2 ;
	    if ( status & PAD_R ) work->rot.vy += 2 ;
	    if ( status & PAD_X ) work->rot.vz -= 2 ;
	    if ( status & PAD_B ) work->rot.vz += 2 ;
	}
	work->rot.vx &= 4095 ;
	work->rot.vy &= 4095 ;
	work->rot.vz &= 4095 ;
	MENU_Locate( 260, 148, 0 ) ;	
	MENU_Printf( "CHARA CAM %.1f %.1f %.1f\n",
		      work->shift.vx, work->shift.vy, work->shift.vz ) ;
	MENU_Printf( "CHARA CAM ROT %d %d %d\n", cam->rotate.vx, cam->rotate.vy, cam->rotate.vz ) ;
    } else {
	if ( GV_PadData[ 1 ].press & PAD_ONOFF ) {
	    cam->on = 1 - cam->on ;
	    GM_ChangeCamera( work->camera->chanl ) ;
	}
    }
}

static	void	Die( work )
Work		*work ;
{
    work->camera->on = 0 ;
    GM_ChangeCamera( work->camera->chanl ) ;
}

/*-----------------------------------------------------------*/

static	int	GetResources( work, name, where )
Work		*work ;
int		name, where ;
{
    int			chara, buf[ 3 ] ;
    GM_CameraSet	*cam ;
    CONTROL		*ctrl ;
    
    /* キャラカメラ起動 */
    /* ビハインドより強い */
    work->camera = cam = NewProgramCamera( name, 0, GM_CAMERA_PROG2, 128 ) ;
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_NO_SET, 0, 0 ) ;
    cam->on = 0 ;

    /* 追従キャラの検索 */
    if ( GCL_GetOption( 'c' ) == NULL ) ASSERT( 0 ) ;
    chara = GCL_GetNextInt() ;
    ctrl = GM_SearchWhere( chara ) ;
    ASSERT( ctrl != NULL ) ;
    work->ctrl = ctrl ;

    /* シフト量 */
    work->shift = DG_ZeroVector ;
    if ( GCL_GetOption( 's' ) != NULL ) {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->shift.vx = buf[ 0 ] ;
	work->shift.vy = buf[ 1 ] ;
	work->shift.vz = buf[ 2 ] ;
    } 
    /* 回転 */
    work->rot = DG_ZeroSVector ;
    if ( GCL_GetOption( 'r' ) != NULL ) {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->rot.vx = buf[ 0 ] ;
	work->rot.vy = buf[ 1 ] ;
	work->rot.vz = buf[ 2 ] ;
	cam->rotate.vx = work->rot.vx ;
	cam->rotate.vy = work->rot.vy ;
	cam->rotate.vz = work->rot.vz ;
    }     
    cam->track = 4000 ; /* 適当 */
    return 0 ;
}

void	*NewCharaTraceCamera( name, where )
int	name, where ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT,
				     sizeof( Work ), CAMERA_PRIO ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	if ( GetResources( work, name, where ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    } 
    return ( void * )work ;
}
