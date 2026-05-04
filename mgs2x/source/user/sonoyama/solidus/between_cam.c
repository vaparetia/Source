//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   between_cam.c
   ２体カメラ

   2001/03/22	M.Sonoyama
   $Id: between_cam.c,v 1.1.1.3 2002/11/19 11:51:03 Yoshizawa1 Exp $
*/

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
#include	"camera.h"

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT				actor ;
	GM_CameraSet		*camera ;
	OBJECT				*body1 ;
	OBJECT				*body2 ;
	int					ry_next ;	
	int					min_track ;
	int					def_rotx ;
	FVECTOR				cam_targ ;
	int					again ;
} Work ;

typedef	struct	{
	FVECTOR				pos1 ;
	FVECTOR				pos2 ;
	FVECTOR				diff ;
	float				len ;
} ScrPad ;

#define	SCRPAD			((ScrPad *)SCRPAD_ADDR)
#define	POS1			(&(SCRPAD->pos1))
#define	POS2			(&(SCRPAD->pos2))
#define	DIFF			(&(SCRPAD->diff))
#define	LEN				(SCRPAD->len)

/*----------------------------------------------------------------*/

/* ターゲット位置を更新 */
static	inline	void	SetCameraTarget( Work *work )
{
	FVECTOR			targ ;
	static FVECTOR	Cushion = { 250.0F, 300.0F, 250.0F } ;

	_sceVu0AddVector( &targ, POS1, POS2 ) ;
	_sceVu0ScaleVector( &targ, &targ, 0.50F ) ;
	GV_NearExp2VF( &work->cam_targ, &targ, 3 ) ;
	GM_CameraCushionTrace( &work->camera->target, &work->cam_targ, &Cushion ) ;
}

/* カメラ角度計算 */
static	inline	void	SetCameraRotate( Work *work )
{
	SVECTOR		rot ;
	int			ry_diff, ry_diff2, rynext1, rynext2 ;

	GV_VecToRot( DIFF, &rot ) ;
	ry_diff = GV_DiffDirAbs( work->camera->rotate.vy, rot.vy ) ;
	if ( ry_diff < 512 || ry_diff > 2048 - 512 ) {
		rynext1 = rot.vy + 1024 ;
		rynext2 = rynext1 + 2048 ;
		ry_diff = GV_DiffDirAbs( work->camera->rotate.vy, rynext1 ) ;
		ry_diff2 = GV_DiffDirAbs( work->camera->rotate.vy, rynext2 ) ;
		if ( ry_diff2 < ry_diff ) {
			rynext1 = rynext2 ;
		}
		work->ry_next = rynext1 ;
	}
}

/* カメラ角度更新 */
static	inline	void	UpdateCameraRotate( Work *work )
{
	if ( GV_PadData->status & PAD_UDLR ) {
		work->camera->rotate.vy = GV_NearExpNP( work->camera->rotate.vy, work->ry_next, 24 ) ;
	} else {
		work->camera->rotate.vy = GV_NearExpNP( work->camera->rotate.vy, work->ry_next, 12 ) ;
	}
}

/* カメラトラック計算 */
static	void	SetCameraTrack( Work *work )
{
	GM_CameraSet		*cam ;
	FMATRIX				world, inv ;
	FVECTOR				cpos1, cpos2 ;
	float				t1, t2 ;
	int					pre_rotx, again_flag ;
	int					pre_track ;

	again_flag = 0 ;
	cam = work->camera ;
	pre_rotx = cam->rotate.vx ;
	pre_track = cam->track ;
	cam->rotate.vx = work->def_rotx ;
try_again :
	GV_MatToVec( &work->body1->objs->world, POS1 ) ;
	GV_MatToVec( &work->body2->objs->world, POS2 ) ;
	POS1->vw = POS2->vw = 1.0F ;
	cam->track = pre_track ;
	GM_CameraMakeCamera( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
	DG_SetPos2( &cam->position, &cam->rotate ) ;
	DG_GetPos( &world ) ;
	_sceVu0InversMatrix( &inv, &world ) ;
	_sceVu0ApplyMatrix( &cpos1, &inv, POS1 ) ;
	_sceVu0ApplyMatrix( &cpos2, &inv, POS2 ) ;
	
	t1 = DG_FABS( cpos2.vx - cpos1.vx ) ;
	t2 = DG_FABS( cpos2.vy - cpos1.vy ) ;
	t1 = t1 / 2.0F * cam->angle ;	
	t2 = t2 / 2.0F * cam->angle * ( float )DRAW_WIDTH / ( float )DRAW_HEIGHT * 1.40F ;
	if ( t2 > t1 ) t1 = t2 ;
	if ( t1 > 8000.0F ) t1 *= 1.80F ;
	else if ( t1 > 7000.0F ) t1 *= 1.80 + ( 8000.0F - t1 ) * 0.0002F ;
	else 				t1 *= 2.00F ;

	if ( t1 < work->min_track ) t1 = work->min_track ;
	cam->track = ( int )t1 ;

	GM_CameraMakeCamera( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;

//ViewFromTo( &cam->target, &cam->position, 32, 232, 32 ) ;
	if ( HZX_OnlineHazardCheck( HZX_AllMapID, 
							    &cam->target, &cam->position,
							    HZX_CHK_ALL, 
							    0, 0 ) ) {
		if ( again_flag < 0 ) {
			work->camera->rotate.vx = GV_NearExp16P( pre_rotx, work->camera->rotate.vx ) ;
			return ;
		}
		work->camera->rotate.vx += 32 ;
		again_flag = 1 ;
		if ( work->camera->rotate.vx > 992 ) {
			work->camera->rotate.vx = 992 ;
			again_flag = -1 ;
		} 
		goto try_again ;
	} else {
		if ( GV_DiffDirAbs( pre_rotx, work->camera->rotate.vx ) > 16 ) {
			work->camera->rotate.vx = GV_NearExp16P( pre_rotx, work->camera->rotate.vx ) ;
		} 
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GV_MatToVec( &work->body1->objs->world, POS1 ) ;
	GV_MatToVec( &work->body2->objs->world, POS2 ) ;
	POS1->vw = POS2->vw = 1.0F ;
	_sceVu0SubVector( DIFF, POS2, POS1 ) ;
	LEN = GV_VecLen3F( DIFF ) ;

	SetCameraTarget( work ) ;
	SetCameraRotate( work ) ;
	UpdateCameraRotate( work ) ;
	SetCameraTrack( work ) ;
}

static	void	Die( Work *work )
{
	GM_DeleteCamera( work->camera ) ;
}

/*----------------------------------------------------------------*/

/* カメラ生成 */
static	void	InitCamera( Work *work, int name )
{
	GM_CameraSet	*cam ;
	
	work->camera = cam = NewProgramCamera( name, 0, GM_CAMERA_PROG3, 160 ) ;
	GM_SetCameraType( cam, GM_CAM_TYPE_TARGET_AND_ROTATE, CAM_FLAG_PAD_ADJUST ) ;
	PL_GetOptionFV( 'p', &cam->position ) ;
	PL_GetOptionSV( 'r', &cam->rotate ) ;
	cam->angle = ( ( float )GCL_GetOptionValue( 'a', 2 ) ) / 100.0F ;
	cam->track = GCL_GetOptionValue( 't', 4000 ) ;
	GM_CameraMakeTarget( &cam->position, &cam->target, &cam->rotate, &cam->track ) ;
	GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4, GM_CAM_INTERP_NO_SET, 0, 0 ) ;
	cam->on = 1 ;
	GM_ChangeCamera( 0 ) ;
//	cam->on = 0 ;

	work->min_track = GCL_GetOptionValue( 'm', 4000 ) ;
	work->ry_next = cam->rotate.vy ;
	work->def_rotx = cam->rotate.vx ;

	DG_COPY_VEC( &work->cam_targ, &cam->target ) ;

	GCL_GetOption( 'b' ) ;
	if ( GCL_NextStr() != NULL ) {
		PL_GetNextFV( &cam->bound1 ) ;
		PL_GetNextFV( &cam->bound2 ) ;
		cam->flag |= CAM_FLAG_BOUND | CAM_FLAG_BOUNDCHANGE_QUICK ;
	}
	GCL_GetOption( 'l' ) ;
	if ( GCL_NextStr() != NULL ) {
		PL_GetNextFV( &cam->limit1 ) ;
		PL_GetNextFV( &cam->limit2 ) ;
		cam->flag |= CAM_FLAG_LIMIT | CAM_FLAG_BOUNDCHANGE_QUICK ;
	}
}

/* オブジェクトの検索 */
static	void	SearchObject( Work *work )
{
	CONTROL		*ctrl ;

	GCL_GetOption( 'n' ) ;
	ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	ASSERT( ctrl != NULL && ctrl->object != NULL ) ;
	work->body1 = ctrl->object ;
	ctrl = GM_SearchWhere( GCL_GetNextInt() ) ;
	ASSERT( ctrl != NULL && ctrl->object != NULL ) ;
	work->body2 = ctrl->object ;
}

/*----------------------------------------------------------------*/

/* 初期化メイン */
static	int		GetResources( Work *work, int name )
{

	InitCamera( work, name ) ;
	SearchObject( work ) ;


	return 0 ;
}

/* 起動 */
void	*NewBetweenCamera( int name, int where )
{
	Work			*work ;

	/* AFTER LEVEL （１フレーム後れは気にしない） */
	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_OBJECT,
									 sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
