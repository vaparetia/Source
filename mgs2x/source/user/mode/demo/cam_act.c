//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	cam_act.c
	カメラ設定アクター

	2000/07/27	K.Takabe
	$Id: cam_act.c,v 1.2 2002/12/11 01:54:05 takaki Exp $

*/
/*

	void *NewDemoCamera( int id, int name, int flag )
	int			id ;	デモ認識ＩＤ
	int			name ;	キャラ名ＩＤ
	int			flag ;	フラグ

	カメラ設定アクター起動

*/
#endif

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

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <def_dma.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>
#include <camera.h>

#include "libdemo.h"

#include "bp_matrix.h"

#include "BP_Debug.h"                        //BP_INPUT - for extra X360 controller logic
#include "../../sonoyama/raiden/pl_work.h"   //BP_INPUT - for extra X360 controller logic
#include "BP_TrophyLogicMGS2.h"

#if defined(BP_VITA)
#include "gesture_vta.h"
#endif

/* デモ中のカメラ操作機能 */
#define INTERACTIVE_CAMERA

enum
{
   kCamState_ZoomIn,
   kCamState_Zoom,
   kCamState_ZoomOut,
   kCamState_Normal,
   kCamState_Pan,
};


/* ---------------------------------------------------------------- */
typedef struct _work{
	GV_ACT_EX	actor ;
	GM_CameraSet	*camera ;	/* カメラ制御用 */
	int			rot_x ;
	int			rot_y ;
   int          camera_zoom_state;
   int          zoom_pressure;
   int          is_dragging;
   float        target_zoom_pos_x;
   float        target_zoom_pos_y;
   float        pan_time;

#ifdef KP_WINDOWS
#ifdef INTERACTIVE_CAMERA
	float		interact_cam_ang_rate ;	/* ボタンが正確にはアナログではないので補間 */
#endif	// INTERACTIVE_CAMERA
#endif	// KP_WINDOWS
} Work ;

#if defined(BP_VITA)
static void BP_ProcessFrontTouchZoom(Work* work)
{
   float const kTimeDelta = 0.1f;
   int const kMaxZoom = 175;

   float press_x;
   float press_y;

   // AS (MCampbell) - Cinematic camera appears to always be on channel 0
   if (work->camera->chanl == 1)
      return;

   GestureGetFrontTouchPressedPos(&press_x, &press_y, 1.0f);
   press_x = press_x * 2048.0f - 1024.0f;
   press_y = press_y * 2048.0f - 1024.0f;

   // If the user taps the front touch, start zooming in. If the user stops engaging the front touch pad zoom out.
   // until the user is done dragging around before zooming out.
   if (GestureGetFrontTouchPressed(kGPR_Direct))
   {
      if (work->camera_zoom_state == kCamState_Normal)
      {
         work->camera_zoom_state = kCamState_ZoomIn;
         work->zoom_pressure = 0;
         work->rot_x = press_x;
         work->rot_y = press_y;
      }
   }
   else
   {
      work->camera_zoom_state = kCamState_ZoomOut;
   }

   work->is_dragging = GestureGetFrontDragging(kGPR_Direct);

   switch (work->camera_zoom_state)
   {
   case kCamState_ZoomIn:
      work->zoom_pressure += 40;
      if (work->zoom_pressure > kMaxZoom)
      {
         work->zoom_pressure = kMaxZoom;
         work->camera_zoom_state = kCamState_Zoom;
      }
      break;

   case kCamState_Pan:
      if (work->is_dragging)
      {
         work->target_zoom_pos_x = press_x;
         work->target_zoom_pos_y = press_y;
      }

      work->rot_x = (work->rot_x * (1.0f - work->pan_time)) + (work->target_zoom_pos_x * work->pan_time);
      work->rot_y = (work->rot_y * (1.0f - work->pan_time)) + (work->target_zoom_pos_y * work->pan_time);

      work->pan_time += kTimeDelta;
      if (work->pan_time >= 1.0f)
         work->camera_zoom_state = kCamState_Zoom;

      break;

   case kCamState_ZoomOut:
      work->zoom_pressure -= 40;
      work->is_dragging = 0;
      if (work->zoom_pressure <= 0)
      {
         work->zoom_pressure = 0;
         work->camera_zoom_state = kCamState_Normal;
      }
      break;
   }
}
#else
static void BP_ProcessFrontTouchZoom(Work* work)
{
}
#endif

/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
static void Act( Work *work )
{
	DEMO_FRAME	*frame ;

   float bp_manual_zoom = 0.f;

	frame = DM_GetFrameData( 0 );
	if ( frame != NULL ){
		/* カメラの設定 */

#ifdef DEBUG_MODE
#ifdef PSX2
		if ( dm_debug.mode1 == 2 ) {
			/* カメラ乗っ取り */
			work->camera->position = dm_debug.fcam_pos ;
			work->camera->target = dm_debug.ftrg_pos ;
			work->camera->angle = 2.0f * dm_debug.zoom ;
//			work->camera->position.vw = frame->z_rotate ;
			work->camera->rotate.vz = ( int )( frame->z_rotate * 4096.0F / TPI ) ;
		} else
#endif
		{
			work->camera->position = frame->camera_pos ;
			work->camera->target = frame->camera_trg ;
			work->camera->angle = frame->screen ;
//			work->camera->position.vw = frame->z_rotate ;
			work->camera->rotate.vz = ( int )( frame->z_rotate * 4096.0F / TPI ) ;
		}
#else
		work->camera->position = frame->camera_pos ;
		work->camera->target = frame->camera_trg ;
		work->camera->angle = frame->screen ;
//		work->camera->position.vw = frame->z_rotate ;
		work->camera->rotate.vz = ( int )( frame->z_rotate * 4096.0F / TPI ) ;
#endif

#ifdef INTERACTIVE_CAMERA
#ifndef KP_WINDOWS

      //BP_INPUT - additional X360 controller remapping
      if( PlayerPad.enable )
      {
#if !defined(BP_VITA)
         // Which method?
         switch( gBP_Input_X360Remap_DemoZoomMethod )
         {
            // - Press and hold RB to activate Zoom
            //    - Press analog RT to control zoom amount
            default:
            case 0:
            {
               if( GV_PadDataDirect[ 0 ].status & PAD_R1 )
               {
                  float rate;
                  rate = GV_PadDataDirect[ 0 ].pressure[ PAD_PRESS_R2 ] * 1.0F / 128 + 1.0F;
                  work->camera->angle = work->camera->angle * rate;
                  bp_manual_zoom = rate;
               }
            }
            break;

            // - Press analog RT to control zoom amount
            case 1:
            {
               float rate;
               rate = GV_PadDataDirect[ 0 ].pressure[ PAD_PRESS_R2 ] * 1.0F / 128 + 1.0F;
               work->camera->angle = work->camera->angle * rate;
               bp_manual_zoom = rate;
            }
            break;
         }
#else
         BP_ProcessFrontTouchZoom(work);
         if (work->zoom_pressure > 0.0f)
         {
            float rate = work->zoom_pressure * 1.0F / 128 + 1.0F;
            work->camera->angle = work->camera->angle * rate;
            bp_manual_zoom = rate;
         }
#endif
      }
      //BP_INPUT - additional X360 controller remapping
		else if( GV_PadDataDirect[ 0 ].status & PAD_R1 )
      {
			float rate;
			rate = GV_PadDataDirect[ 0 ].pressure[ PAD_PRESS_R1 ] * 1.0F / 128 + 1.0F;
			work->camera->angle = work->camera->angle * rate;
         bp_manual_zoom = rate;
		} 
      else 
      {
			/* 完全にボタンを離したらオフセットをリセットする */
			work->rot_x = work->rot_y = 0 ;			/* リセット */
		}
#else
		{
			float	tgt_rate ;
			
			/* Windows版ではコンフィグの関係上、R1,R2どちらでも効く様に */
			if( GV_PadDataDirect[ 0 ].status & (PAD_R1|PAD_R2) ){
				if( GV_PadDataDirect[0].pressure[PAD_PRESS_R1]
				  > GV_PadDataDirect[0].pressure[PAD_PRESS_R2] ) {
					tgt_rate = (float)GV_PadDataDirect[0].pressure[PAD_PRESS_R1] ;
				} else {
					tgt_rate = (float)GV_PadDataDirect[0].pressure[PAD_PRESS_R2] ;
				}
			} else {
				tgt_rate = -1.0f ;	// 完全に補間が終了するように-1.0f
			}
			work->interact_cam_ang_rate += (tgt_rate - work->interact_cam_ang_rate) * 0.6f ;
		}
		if( work->interact_cam_ang_rate > 0.0f ){
			float rate;
			rate = (work->interact_cam_ang_rate / 128.0f) + 1.0F;
			work->camera->angle = work->camera->angle * rate;
		} else {
			/* オフセットをリセットする */
			work->rot_x = work->rot_y = 0 ;			/* リセット */
		}
#endif
		//if( GV_PadDataDirect[ 0 ].press & PAD_AR ){
		//	work->rot_x = work->rot_y = 0 ;			/* リセット */
		//}
		{
#if !defined(BP_VITA)
			int		dx, dy ;
			dx = GV_PadDataDirect[ 0 ].right_dx ;
			dy = GV_PadDataDirect[ 0 ].right_dy ;
			/* アナログスティックの遊びをチェック */
			if ( dx < 128 ){
				dx -= 128 - 32 ;
				if ( dx > 0 ) dx = 0 ;
			} else {
				dx -= 128 + 32 ;
				if ( dx < 0 ) dx = 0 ;
			}
			if ( dy < 128 ){
				dy -= 128 - 32 ;
				if ( dy > 0 ) dy = 0 ;
			} else {
				dy -= 128 + 32 ;
				if ( dy < 0 ) dy = 0 ;
			}
			/* 回転量レートの計算 */
			work->rot_x += dx ;
			work->rot_y += dy ;
#else
         if (work->is_dragging)
         {
            float drag_x;
            float drag_y;

            GestureGetFrontTouchPressedPos(&drag_x, &drag_y, 1.0f);
            work->rot_x = drag_x * 2048.0f - 1024.0f;
            work->rot_y = drag_y * 2048.0f - 1024.0f;
         }
#endif

			if ( work->rot_x > 1024 ) work->rot_x = 1024 ;
			if ( work->rot_x < -1024 ) work->rot_x = -1024 ;
			if ( work->rot_y > 1024 ) work->rot_y = 1024 ;
			if ( work->rot_y < -1024 ) work->rot_y = -1024 ;
		}
		{/* カメラシステムを通さずに直接ＤＧにカメラを設定する */
//#define SCREEN_ASPECT	((float)DRAW_WIDTH/DRAW_HEIGHT*PIXEL_ASPECT)
//#define Y_BASE_TOP	( (float)( (DRAW_HEIGHT/2) - (16*2) ) / (float)(DRAW_HEIGHT/2) )
//#define Y_BASE_BOTTOM	( (float)( (DRAW_HEIGHT-48*2) - (DRAW_HEIGHT/2) ) / (float)(DRAW_HEIGHT/2) )
/* パラメータはＮＴＳＣ時のもので統一する */
#define SCREEN_ASPECT	(512.0f/448.0f*1.121f)
#define Y_BASE_TOP		( (float)( 224 - (16*2) ) / 224.0f / SCREEN_ASPECT )
#define Y_BASE_BOTTOM	( (float)( (448-48*2) - 224 ) / 224.0f / SCREEN_ASPECT )
			static FVECTOR	x_base = { 1.0f, 1.0f, 1.0f, 1.0f };
			static FVECTOR	y_base = { Y_BASE_TOP , Y_BASE_TOP, Y_BASE_BOTTOM, Y_BASE_BOTTOM };
			FVECTOR	angle_x, angle_y, dir ;
			FVECTOR	screen_length ;
			FMATRIX	camera_mat ;
			float	x_rate, y_rate ;

			x_rate = (float)work->rot_x / 1024.0f ;
			y_rate = (float)work->rot_y / 1024.0f ;
			x_rate *= 0.9f ;	/* ちょっとマージンを取ってみる */
			y_rate *= 0.9f ;	/* ちょっとマージンを取ってみる */
			screen_length.vx = frame->screen ;				/* デモに記録されているスクリーン距離 */
			screen_length.vy = work->camera->angle ;		/* ズームによる修正後のスクリーン距離 */
			screen_length.vz = frame->screen ;				/* デモに記録されているスクリーン距離 */
			screen_length.vw = work->camera->angle ;		/* ズームによる修正後のスクリーン距離 */
			MT_Atan2X4( &angle_x, &x_base, &screen_length );	/* 角度を計算 */
			MT_Atan2X4( &angle_y, &y_base, &screen_length );	/* 角度を計算 */
			{/* カメラマトリクスの生成 */
				FMATRIX	mat ;
				/* カメラの向きマトリクス生成 */
				_sceVu0SubVector( &dir, &work->camera->target, &work->camera->position );
				DG_MakeCameraMatrix( &camera_mat, &work->camera->position, &work->camera->target );
#if 1
#if 0
				_sceVu0UnitMatrix( &mat );
				_sceVu0RotMatrixZ( &mat, &mat, -work->camera->rotate.vz * (float)M_PI / 2048.0f ) ;
				_sceVu0MulMatrix( &camera_mat, &camera_mat, &mat );
#else
				work->camera->rotate.vz &= 4095 ;
				if ( work->camera->rotate.vz & 2048 ) work->camera->rotate.vz -= 4096 ;
				_sceVu0InversMatrix( &mat, &camera_mat );
				_sceVu0RotMatrixZ( &mat, &mat, work->camera->rotate.vz * (float)TPI / 4096.0  );
				_sceVu0InversMatrix( &camera_mat, &mat );
#endif
				/* 補正マトリクス生成 */
				_sceVu0UnitMatrix( &mat );
				if ( y_rate < 0.0f ){
					_sceVu0RotMatrixX( &mat, &mat, -( angle_y.vx - angle_y.vy ) * y_rate );
				} else {
					_sceVu0RotMatrixX( &mat, &mat, -( angle_y.vz - angle_y.vw ) * y_rate );
				}
				if ( x_rate < 0.0f ){
					_sceVu0RotMatrixY( &mat, &mat, ( angle_x.vx - angle_x.vy ) * x_rate );
				} else {
					_sceVu0RotMatrixY( &mat, &mat, ( angle_x.vz - angle_x.vw ) * x_rate );
				}
				_sceVu0MulMatrix( &camera_mat, &camera_mat, &mat );
#else
				_sceVu0UnitMatrix( &mat );
				_sceVu0RotMatrixX( &mat, &mat, -( angle.vz - angle.vw ) * y_rate );
				_sceVu0RotMatrixY( &mat, &mat, ( angle.vx - angle.vy ) * x_rate );
				_sceVu0RotMatrixZ( &mat, &mat, work->camera->rotate.vz * (float)M_PI / 2048.0f ) ;
				_sceVu0MulMatrix( &camera_mat, &camera_mat, &mat );
#endif
			}

         //BP_CAMERA - set active camera info for wide screen tweak system
         BP_Camera_SetIdFromDemo( work->camera );
         BP_Camera_SetActiveCamera( work->camera );
         //BP_CAMERA - set active camera info for wide screen tweak system

         //BP_TROPHY - hook for detecting "glimpse VAMP during end cinematic"
         bp_trophy_stream_update( gBP_Demo_StreamName, gBP_Demo_Frame * TIME_BASE, bp_manual_zoom, x_rate, y_rate );

			/* マトリクスをカメラに設定 */
			DG_SetCamera( DG_Chanl(0), &camera_mat, work->camera->angle );
		}
#endif
		/* カメラを一切補間しないように値を更新する */
		GM_SetCameraQuick( 0 );
	}
}
/* ---------------------------------------------------------------- */
	/*
		終了処理
	*/
static void Die( Work *work )
{
	/* オブジェクトの開放 */
	if ( work->camera != NULL ){
		work->camera->on = 0 ;
		GM_ChangeCamera( 0 );
		GM_DeleteCamera( work->camera );
	}
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
static int GetResources( Work *work, int id, int name, int flag )
{
	int nFlag;

	work->camera = NewProgramCamera( name, 0, GM_CAMERA_PROG1, 128 );
   work->camera_zoom_state = kCamState_Normal;
   work->zoom_pressure = 0.0f;

#if defined(BP_VITA)
   GestureResetFrontTouchTappedStatus();
#endif

//#ifdef DEBUG_MODE
//	GM_SetCameraType( work->camera, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP |
//					  CAM_FLAG_PAD_ADJUST_NO_RESET );
//#else
//	GM_SetCameraType( work->camera, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX | 
//					  CAM_FLAG_PAD_ADJUST_NO_RESET );
//#endif

	/* カメラフラグ設定 */
	{
#ifdef DEBUG_MODE
		nFlag = CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP;
#else
		nFlag = CAM_FLAG_FIX;
#endif

		/* フラグの追加があるか？ */
		if(DM_CameraFlag & DEMO_CAMERA_NO_RESET){
			nFlag |= CAM_FLAG_PAD_ADJUST_NO_RESET;
		}
	}
	GM_SetCameraType( work->camera, GM_CAM_TYPE_CAMERA_AND_TARGET, nFlag);

	
	GM_SetCameraAngle( work->camera, 2.0F ) ;
	GM_SetCameraInterpMode( work->camera, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK, 0, 0);
#if 0
	/* 起動時には無効にしておく */
	work->camera->on = 0 ;
#else
	/* カメラを有効にする */
	work->camera->on = 1 ;
	GM_ChangeCamera( 0 );
#endif

#ifdef KP_WINDOWS
#ifdef INTERACTIVE_CAMERA
	work->interact_cam_ang_rate = 0.0f ;
#endif	// INTERACTIVE_CAMERA
#endif	// KP_WINDOWS

	return ( 0 );
}
/* ---------------------------------------------------------------- */
	/*
		シグナルレシーブ関数
	*/
static int ReceiveSignal( void *workp, int signal, int value )
{
	switch( signal ){

	  default:
		return ( GV_DefaultSignalFunc( workp, signal, value ) );
		break ;
	}
	return 0;
}
/* ---------------------------------------------------------------- */
	/*
		カメラ設定アクター起動
	*/
void *NewDemoCamera( int id, int name, int flag )
{
	Work		*work ;

	OPERATOR() ;
	/* アクター生成 */
#ifndef INTERACTIVE_CAMERA
#ifdef DEBUG_MODE
	work = (Work *)GV_NewActor( GV_ACTOR_CAMERA, sizeof( Work ) ) ;
#else
	work = (Work *)GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work ) ) ;
#endif
#else
	work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, sizeof( Work ), CAMERA_DAEMON_PRIO + 1 ) ;
#endif

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, id, name, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		/* シグナルレシーブ関数設定 */
		GV_SetActorSignalFunc( work, ReceiveSignal );
	}
	return ( work ) ;
}

