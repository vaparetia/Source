//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   zoomcam.c
   カメラズーム制御

   2001/03/19	M.Sonoyama
   $Id: zoomcam.c,v 1.4 2002/11/23 12:42:25 Yoshizawa1 Exp $
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
#include <sifdev.h>
#include <libvu0.h>
#include <libdev.h>
#endif

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

#include	"gameheader.h"
#include	"camera.h"

#include "../raiden/pl_work.h"//BP_INPUT

#define	NAME 	(15192154)	/* ズームカメラ */

extern int PL_PAD_ZOOMIN        ;
extern int PL_PAD_PRESS_ZOOMIN  ;
extern int PL_PAD_ZOOMOUT       ;
extern int PL_PAD_PRESS_ZOOMOUT ;

#define	PAD_ANGLE_INC	(PL_PAD_ZOOMOUT)
#define	PAD_ANGLE_DEC	(PL_PAD_ZOOMIN)

#define	PAD_ANGLE_ANALOG_INC	(PL_PAD_PRESS_ZOOMOUT)
#define	PAD_ANGLE_ANALOG_DEC	(PL_PAD_PRESS_ZOOMIN)

typedef	struct	{
	GV_ACT_EX			actor ;
	GM_CameraSet		*camera ;
	GM_CameraSet		*parent ;
	GV_PAD				*pad ;
	int					mode ;
	int					parent_name ;
	int					vstep_org ;
	int					hstep_org ;
	float				angle_min ;
	float				angle_max ;
	float				angle_width ;
	float				angle_step ;

#if defined(BP_VITA)
   float          prev_angle;
   float          zoom_time;
   float          zoom_start_y;
   int            zoom_prev_dir;
#endif
} Work ;

/*----------------------------------------------------------------*/
static float SK_OutAngle; // 外部から影響を受けることなく値をいじる

void SK_OutAngleControl( float angle )
{
	SK_OutAngle = angle;
}

void SK_OutAngleControlInitialize( void )
{
	SK_OutAngle = 0.0f;
}

static inline int PressureToZoomSpeed( int pressure )	// ズームスピード算出
{
	if ( pressure >= 60 ){
		return ( ( pressure / 60 ) );
	} else {
		return ( 1 );
	}
}

//BP_INPUT - X360 control overrides
static inline float PressureToZoomInSpeed( int pressure )
{
   if( PlayerPad.enable )
   {
      return pressure ? PL_SCOPE_ZOOM_IN_SPEED : 0;
   }
   else
   {
      return (float)PressureToZoomSpeed( pressure );
   }
}

static inline float PressureToZoomOutSpeed( int pressure )
{
   if( PlayerPad.enable )
   {
      return pressure ? PL_SCOPE_ZOOM_OUT_SPEED : 0;
   }
   else
   {
      return (float)PressureToZoomSpeed( pressure );
   }
}
//BP_INPUT - X360 control overrides

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
   extern int gBP_UseFrontTouchForZooming;
   extern int gBP_TouchZoomLikeJoystick;

	GM_CameraSet	*cam ;
	GV_PAD			*pad ;
	int				status;
	float			angle, adj, step ;
   float          pressure;//BP_INPUT - X360 remap - switched type to float

   if ( GV_PauseLevel & GV_PAUSE_DEBUG )
   {
      // BP - Only the debug cam should update in debug
      return;
   }

#if 0
	/* 重いが、親の存在チェックを毎フレーム入れる */
	if ( GM_FindCameraSet( work->parent_name, work->camera->chanl ) == NULL ) {
		GV_DestroyActor( work ) ;
		return ;
	}
#endif
	if ( work->parent->on == 0 ) return ;
	if ( GV_PauseLevel != 0 ) return ;

    cam = work->camera ;

	DG_COPY_VEC( &cam->position, &work->parent->position ) ;
	DG_COPY_VEC( &cam->target, &work->parent->target ) ;
	cam->rotate = work->parent->rotate ;
	cam->track = work->parent->track ;

    pad = work->pad ;
	status = pad->status;

    angle = cam->angle ;

    adj = work->angle_max - angle ; 
    if ( adj == 0.0F ) adj = 1.0F ;
    adj = work->angle_step * ( work->angle_width ) / adj ;
    if ( status & PAD_ANGLE_INC ) {
       pressure = pad->pressure[ PAD_ANGLE_ANALOG_INC ] ;
       pressure = PressureToZoomInSpeed( (int)pressure );//BP_INPUT - X360 controller overrides
       angle += ( adj * pressure );
       if ( ( angle < work->angle_max ) && ( ( GV_Time & 6 ) == 0 ) && ( pressure != 0 ) ) {
          GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
       }
    } else if ( status & PAD_ANGLE_DEC ) {
       pressure = pad->pressure[ PAD_ANGLE_ANALOG_DEC ] ;
       pressure = PressureToZoomOutSpeed( (int)pressure );//BP_INPUT - X360 controller overrides
       angle -= ( adj * pressure );
       if ( ( angle > work->angle_min ) && ( ( GV_Time & 6 ) == 0 ) && ( pressure != 0 ) ) {
          GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
       }
    }

#ifdef BP_VITA
    // Arm fix:
    // don't consider the touch input when in a pad demo
    // fixes MGSTWO-2815
    if (!GM_CheckGameStatus(STATE_PAD_DEMO))
    {
       if (gBP_UseFrontTouchForZooming)
       {
          int drag_dir = GestureGetRightDragDir(kGesture_TouchScreen_Front, kGPR_Filtered);

          if (GestureGetFrontDragging(kGPR_Filtered))
          {
             float x, y;
             float curr_angle;

             if (gBP_TouchZoomLikeJoystick)
             {
                GestureGetFrontTouchPressedPos(&x, &y, 1.0f);

                if (y < 0.75f && x > 0.8f)
                {
                   if (work->zoom_start_y == 0.0f ||  drag_dir != work->zoom_prev_dir)
                   {
                      work->zoom_start_y = y;
                      work->zoom_prev_dir = drag_dir;
                   }

                   if (drag_dir < 0)
                      angle += PL_SCOPE_ZOOM_IN_SPEED * adj;
                   else if (drag_dir > 0)
                      angle -= PL_SCOPE_ZOOM_IN_SPEED * adj;

                   if (angle > work->angle_min && angle < work->angle_max && ((GV_Time & 6) == 0))
                   {
                      GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
                   }
                }
             }
             else
             {
                GestureGetFrontTouchPressedPos(&x, &y, 0.5f);
                y = (1.35f - y);

                if (x > 0.8f)
                {
                   curr_angle = work->angle_min + y * (work->angle_max - work->angle_min);
                   angle = (1.0f - work->zoom_time) * work->prev_angle + (work->zoom_time * curr_angle);

                   work->prev_angle = angle;
                   work->zoom_time += 0.01f;
                   if (work->zoom_time >= 1.0f)
                      work->zoom_time = 1.0f;

                   if (angle > work->angle_min && angle < work->angle_max && ((GV_Time & 6) == 0))
                   {
                      GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
                   }
                }
             }
          }
          else
          {
             work->zoom_time = 0.0f;
             work->zoom_start_y = 0.0f;
             work->zoom_prev_dir = 0;
          }
       }
    }
#endif

    if ( angle > work->angle_max ) angle = work->angle_max ;
    if ( angle < work->angle_min ) angle = work->angle_min ;

	if ( SK_OutAngle != 0.0f ){ // by koba4 2001/07/06
		cam->angle = SK_OutAngle;
	} else {
		cam->angle = angle ;
	}
	/* 主観回転速度をいじる */
	if ( work->mode == 0 ) {
		step = 16.0F - 16.0F * ( ( angle - work->angle_min ) / work->angle_width ) ;
		if ( angle > 20.0F ) step /= 3.0F ;
		else if ( angle > 10.0F ) step /= 2.0F ;
		//if ( status & PAD_L1 ) step /= 2.0F ;	/* 謎の機能 */
		if ( step < 1.0F ) {
			if ( GV_Time & 1 ) GM_SubjectVStepTmp = 1 ;
			else		   GM_SubjectVStepTmp = 0 ;
		} else {
			GM_SubjectVStepTmp = ( int )step ;
		}
		GM_SubjectHStepTmp = GM_SubjectVStepTmp ;	
	}
}

static	void	Die( Work *work )
{
	GM_DeleteCamera( work->camera ) ;
	if ( work->camera != NULL ) GM_ChangeCamera( work->camera->chanl ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, int name )
{
	GM_CameraSet	*cam ;

	work->camera = cam = NewProgramCamera( name, work->parent->chanl, GM_CAMERA_SUBJECT,
										   ZOOM_CTRL_CAMERA_PRIO ) ;
	if ( cam == NULL ) return -1 ;

	GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX ) ;
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT, GM_CAM_INTERP_OUT_SUBJECT, 
						    0, 0 ) ;
	GM_CopyCamera( cam, work->parent ) ;
	cam->on = 1 ;
	GM_ChangeCamera( cam->chanl ) ;
	return 0 ;
}

/* カメラズームコントロール */
/* mode == 0 : プレイヤーカメラ
   mode != 0 : その他 */
void		*NewCameraZoomControl( GM_CameraSet **res, GM_CameraSet *parent, GV_PAD *pad, 
								   float angle_min, float angle_max,
								   float angle_step, int mode, int name )
{
	Work			*work ;

	/* 他のカメラの計算の後でカメラデーモンの前 */
	work = ( Work * )GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, sizeof( Work ), 
									 CAMERA_ZOOM_CTRL_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		
		work->mode = mode ;
		work->parent = parent ;
		work->parent_name = parent->name ;
		work->pad = pad ;
		work->angle_min = angle_min ;
		work->angle_max = angle_max ;
		work->angle_step = angle_step ;
		work->angle_width = angle_max - angle_min ;

#if defined(BP_VITA)
      work->prev_angle = work->angle_min;
      work->zoom_time = 0.0f;
      work->zoom_start_y = 0.0f;
      work->zoom_prev_dir = 0;
#endif

		ASSERT( work->angle_width > 0.0F ) ;

		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		*res = work->camera ;
	}
	return work ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* ぼかし制御 */

extern	void *NewFarFocusEffect( int name, int max_plane, int near, int far ) ;

#define	FOCUS		(8478436)	/* 共用ぼかし */

typedef	struct	{
	GV_ACT_EX		actor ;
	GM_CameraSet	*camera ;
	void			*focus ;
	float			pre_angle ;
	int				delay ;
} FFOCUS_CTRL_WORK ;

static	void	LocalSendMessage( int to, int *mesg, int len )
{
    GV_MSG	msg ;

    msg.address = to ;
    msg.message = mesg ;
    msg.message_len = len ;
    GV_SendMessage( &msg ) ;
}

static	void	FFWorkAct( FFOCUS_CTRL_WORK *work )
{
	GM_CameraSet	*cam ;
	int				buf[ 5 ] ;

	if ( GV_PauseLevel != 0 ) return ;

	cam = work->camera ;
	if ( cam == NULL ) cam = GM_GetCurrentCameraSet( 0 ) ;

	if ( DG_FABS( work->pre_angle - cam->angle ) > 0.1f ) {
		/* フォーカス */
		if ( work->delay < 0 ) work->delay = 0 ;
		if ( work->delay < 16 ) ++ work->delay ;
	} else {
		if ( work->delay > 0 ) {
			if ( -- work->delay == 0 ) {
				buf[ 0 ] = 0 ;
				/* フォーカスＯＦＦ */
				LocalSendMessage( FOCUS, buf, 1 ) ;
			}
		}		
	}
	/* フォーカスにメッセージ */
    if ( work->delay != 0 ) {
		buf[ 0 ] = 2 ;
		buf[ 1 ] = 0 ;
		buf[ 2 ] = 0 ;
		buf[ 4 ] = 100000 ;
		if ( work->delay < 8 ) {
			buf[ 3 ] = 19300 - work->delay * 2400 ;
		} else {
			buf[ 3 ] = 100 + ( work->delay - 8 ) * 500 ;
		}
		LocalSendMessage( FOCUS, buf, 5 ) ;
    }
	work->pre_angle = cam->angle ;
}

static	void	FFWorkDie( FFOCUS_CTRL_WORK *work )
{
	
}

void	*NewFarFocusControl( GM_CameraSet *camera )
{
	FFOCUS_CTRL_WORK	*work ;

	work = ( FFOCUS_CTRL_WORK * )GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, sizeof( Work ), 
												 CAMERA_ZFOCUS_CTRL_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, FFWorkAct, FFWorkDie ) ;
		GV_ActorEX( &work->actor ) ;	
		work->camera = camera ;
		if ( camera == NULL ) work->pre_angle = 2.0F ;
		else				  work->pre_angle = camera->angle ;
		work->delay = 0 ;

		/* ぼかし起動 */
		{	
			int		buf[ 1 ] ;

			work->focus = NewFarFocusEffect( FOCUS, 8, 1000, 1000 ) ;
			if ( work->focus == NULL ) {
				printf( "warning : focus effect could not start\n" ) ;
				return NULL ;
			}
			GV_SetActorChild( &work->actor, work->focus ) ;	
			/* 最初はＯＦＦ */
			buf[ 0 ] = 0 ;
			LocalSendMessage( FOCUS, buf, 1 ) ;
		}
	}
	return work ;
}
