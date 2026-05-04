//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   scope.c
   双眼鏡
   
   1999/12/13 M.Sonoyama
   $Id: scope.c,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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
#include	"camera.h"

/*------------------------------------------------------------------*/

#define	ANGLE_MAX	(24.00F)
#define	ANGLE_MIN	(2.00F)
#define	ANGLE_STEP	(0.15F)

extern int PL_PAD_ZOOMIN        ;
extern int PL_PAD_PRESS_ZOOMIN  ;
extern int PL_PAD_ZOOMOUT       ;
extern int PL_PAD_PRESS_ZOOMOUT ;

#define	PAD_ANGLE_INC	(PL_PAD_ZOOMOUT)
#define	PAD_ANGLE_DEC	(PL_PAD_ZOOMIN)

#define	FOCUS		(403770)	/* 双眼鏡フォーカス */

/*------------------------------------------------------------------*/

extern	void	PL_SendMessage( int, int *, int ) ;
extern  void	*NewSK_ScopeSight( float );
/* ズームカメラ (sonoyama/etc/zoomcam.c) */
extern	void		*NewCameraZoomControl( GM_CameraSet **res, GM_CameraSet *parent, GV_PAD *pad, 
										  float angle_min, float angle_max,
										  float angle_step, int mode, int name ) ;
extern  void	*NewFarFocusControl( GM_CameraSet *camera );

enum {
	SGT_FLAG_NONE = 		0x0000,
	SGT_FLAG_ZOOM_IN =		0x0001,
	SGT_FLAG_ZOOM_OUT =		0x0002,
	SGT_FLAG_ANGLE_SET =	0x0004,
	SGT_FLAG_HIDE =			0x1000,
} ;	/* t_sight.c */

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX	actor ;
    OBJECT		equip ;

    CONTROL		*ctrl ;
    OBJECT		**body ;
    int			*unit ;
    u_int		*trigger ;

    GM_CameraSet	*subject ;
    GM_CameraSet	*camera ;
    GV_PAD		*pad ;
    void		*focus ;
	void		*zctrl ;

	int			flag ;
	int			chanl ;
    int			delay ;
} Work ;

/*------------------------------------------------------------------*/

/* ズームカメラ起動 */
static	void	StartZoomCamera( Work *work )
{
	if ( work->zctrl == NULL ) {
		work->zctrl = NewCameraZoomControl( &work->camera, GM_PlayerSubjectCamera[ 0 ], GV_PadData,
										    ANGLE_MIN, ANGLE_MAX, ANGLE_STEP, 0,
										    PL_SCOPE_ZOOMCAMERA_NAME ) ;
		GV_SetActorChild( work, work->zctrl ) ;
	}
	if ( work->focus == NULL ) {
		work->focus = NewFarFocusControl( NULL ) ;
		GV_SetActorChild( work, work->focus );		
	}
}


/* カメラ操作 */
static	void	CameraControl( work )
Work		*work ;
{
#if 0
    GM_CameraSet	*cam ;
    GV_PAD		*pad ;
    int			status ;
    static int		buf[ 5 ] = { 0,0,0,100,1000 } ;
    float		angle, adj, step ;

    cam = work->camera ;
    pad = work->pad ;
    status = pad->status ;
    angle = cam->angle ;
    adj = ANGLE_MAX - angle ; 
    if ( adj == 0.0F ) adj = 1.0F ;
    adj = ANGLE_STEP * ( ANGLE_MAX - ANGLE_MIN ) / adj ;
    if ( status & PAD_ANGLE_INC ) {
		angle += adj ;
		if ( angle < ANGLE_MAX && ( GV_Time & 6 ) == 0 ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
    } else if ( status & PAD_ANGLE_DEC ) {
		angle -= adj ;
		if ( angle > ANGLE_MIN && ( GV_Time & 6 ) == 0 ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_ZOOM01 ) ;
    }
    if ( angle > ANGLE_MAX ) angle = ANGLE_MAX ;
    if ( angle < ANGLE_MIN ) angle = ANGLE_MIN ;

#if 0	/* サイト側で実行 */

#if 1
    if ( DG_FABS( cam->angle - angle ) < 0.01F ) {
		if ( work->delay > 0 ) {
			if ( -- work->delay == 0 ) {
				buf[ 0 ] = 0 ;
				PL_SendMessage( FOCUS, buf, 1 ) ;
			}
		}
    } else {
		if ( work->delay < 0 ) work->delay = 0 ;
		if ( work->delay < 16 ) ++ work->delay ;
    }
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
		PL_SendMessage( FOCUS, buf, 5 ) ;
    }
#else
    buf[ 0 ] = 2 ;
    buf[ 1 ] = 0 ;
    buf[ 2 ] = 0 ;
    if ( status & PAD_L1 ) {
		if ( status & PAD_X ) buf[ 4 ] += 1000 ;
		else if ( status & PAD_Y ) buf[ 4 ] -= 1000 ;
		if ( buf[ 4 ] < 1000 ) buf[ 4 ] = 1000 ;
    } else {
		if ( status & PAD_X ) buf[ 3 ] += 1000 ;
		else if ( status & PAD_Y ) buf[ 3 ] -= 1000 ;
		if ( buf[ 3 ] < 100 ) buf[ 3 ] = 100 ;
    }
	//    MENU_Locate( 260, 180, 0 ) ;
	//    MENU_Printf( "near - far %d %d\n", buf[ 3 ], buf[ 4 ] ) ;
    PL_SendMessage( FOCUS, buf, 5 ) ;
#endif

#endif
    cam->angle = angle ;
    _sceVu0CopyVector( &cam->position, &GM_CameraTarget ) ;

    cam->rotate.vx = GM_CameraDir.vx ;
//    cam->rotate.vy = GV_NearExp4P( cam->rotate.vy, work->ctrl->turn.vy ) ;
    cam->rotate.vy = GM_CameraDir.vy ;
    cam->rotate.vz = 0 ;
//    GM_SetCameraQuick( 0 ) ;    

	//    MENU_Locate( 360, 140, 0 ) ;
	//    MENU_Printf( "TURN %d %d\n", cam->rotate.vx, cam->rotate.vy ) ;
	//    MENU_Printf( "ANGLE %.2f\n", cam->angle ) ;

    step = 16.0F - 16.0F * ( ( angle - ANGLE_MIN ) / ( ANGLE_MAX - ANGLE_MIN ) ) ;
    if ( angle > 20.0F ) step /= 3.0F ;
    else if ( angle > 10.0F ) step /= 2.0F ;
    if ( status & PAD_L1 ) step /= 2.0F ;
    if ( step < 1.0F ) {
		if ( GV_Time & 1 ) GM_SubjectVStep = 1 ;
		else		   GM_SubjectVStep = 0 ;
    } else {
		GM_SubjectVStep = ( int )step ;
    }
    GM_SubjectHStep = GM_SubjectVStep ;

#if 0
    GM_SubjectVStep = 16 - ( int )( ( angle - ANGLE_MIN ) * 14.0F / ( ANGLE_MAX - ANGLE_MIN ) ) ;
    GM_SubjectHStep = 16 - ( int )( ( angle - ANGLE_MIN ) * 14.0F / ( ANGLE_MAX - ANGLE_MIN ) ) ;

    if ( pad->type & GV_PAD_ANALOG_L_USE ) {
		if ( status & PAD_L1 ) {
			GM_SubjectVStep /= 2 ;
			GM_SubjectHStep /= 2 ;
		}
    }
    if ( GM_SubjectVStep < 2 ) GM_SubjectVStep = 2 ;
    if ( GM_SubjectHStep < 2 ) GM_SubjectHStep = 2 ;
#endif
#endif
}

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
    u_int		trg ;
//	void		*child ;

	StartZoomCamera( work ) ;
//	if ( GM_GetNextCamera( work->camera->chanl ) != work->camera ) {
//		GM_ChangeCamera( work->camera->chanl ) ;
//	}
	if ( work->flag == 0 ) {
		if ( ( *work->body )->objs->flag & ( DG_FLAG_INVISIBLE0 << work->chanl ) ) {
//			extern void	*NewScopeSight( float ) ;
//			child = NewScopeSight( 1.0F ) ;
//			if ( child != NULL ) {
//				GV_SetActorChild( &work->actor, child ) ;
			GM_ResetSightStatus( SGT_Scope );
			work->flag = 1 ;
//			}
		}
	}
    trg = *( work->trigger ) ;
    CameraControl( work ) ;

	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		static FVECTOR offset = { 20.0f, 0.0f, 0.0f, 1.0f } ;
		DG_OBJS *scope = work->equip.objs ;
		DG_OBJ  *hand = &GM_PlayerBody->objs->objs[HUMAN21_MIGI_TE] ;

		_sceVu0CopyMatrix( &scope->world, &hand->world ) ;
		_sceVu0ApplyMatrix( &scope->world.m[3], &hand->world, &offset ) ;
	}

}

static	void	Die( work )
Work		*work ;
{
//    GM_ChangeCamera( work->camera->chanl ) ;
//    GM_DeleteCamera( work->camera ) ;
//    GM_SubjectVStep = 16 ;
//    GM_SubjectHStep = 16 ;
#if 0
    if ( work->focus != NULL ) {
		GV_DestroyActor( work->focus ) ;
    }
#endif
    GM_FreeObject( &work->equip ) ;
}

/*------------------------------------------------------------------*/

/* カメラ起動 */
static	void	InitCamera( work, ctrl )
Work		*work ;
CONTROL		*ctrl ;
{
	StartZoomCamera( work ) ;
	GM_SetSightStatus( SGT_Scope );
	GV_SetActorChild( work , NewSK_ScopeSight( 1.0f ) ) ;
#if 0
    GM_CameraSet	*cam ;
    int			name ;

    name = GV_StrCode( "双眼鏡カメラ" ) ;
    work->camera = cam = NewProgramCamera( name, GM_CurrentCameraChanl, 
										  GM_CAMERA_SUBJECT, SCOPE_CAMERA_PRIO ) ; 
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE,
					 CAM_FLAG_PAD_ADJUST ) ;
    GM_SetCameraRotate( cam, &GM_CameraDir ) ;
    GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
    cam->position = GM_CameraTarget ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT,
						   GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
    cam->on = work->parent->on ;
    if ( cam->on == 1 ) GM_ChangeCamera( work->camera->chanl ) ;
#endif
}

static	int	GetResources( work, ctrl, body, unit )
Work		*work ;
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
{
    /* カメラ起動 */
    InitCamera( work, ctrl ) ;
    work->pad = GV_PadData + GM_CurrentCameraChanl ;
    work->subject = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
#if 0	/* サイト側で起動 */
    /* ぼかし起動 */
    work->focus = NewFarFocusEffect( FOCUS, 8, 1000, 1000 ) ;
    if ( work->focus == NULL ) {
		printf( "warning : focus effect could not start\n" ) ;
    }
    /* 最初はＯＦＦ */
    buf[ 0 ] = 0 ;
    PL_SendMessage( FOCUS, buf, 1 ) ;
#endif
	/* 子画面、鏡用モデル */
	GM_InitObject( &work->equip, MDL_SCOPE, DG_FLAG_ONEPIECE|DG_FLAG_SHADE ) ;
	if ( work->equip.objs == NULL ) return -1 ;

	if ( !GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		GM_ConfigObjectRoot( &work->equip, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
	}

	DG_InvisibleObjs( work->equip.objs ) ;
	DG_VisibleObjsChanl( work->equip.objs, 1 ) ;

    return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewScope( ctrl, body, unit, trigger )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_ITEM_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, ctrl, body, unit ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->chanl = GM_CurrentCameraChanl ;
    }
    return work ;
}

