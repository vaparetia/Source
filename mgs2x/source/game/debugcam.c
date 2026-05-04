//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   debugcam.c
   デバッグカメラ（シナリオでカメラを設定するときに使用）
   1999/07/16 M.Sonoyama
   $Id: debugcam.c,v 1.1.1.3 2002/11/19 11:41:47 Yoshizawa1 Exp $
   
   */

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"
#endif

#include	"gameheader.h"
#include	"camera.h"

#include	"game.h"

#include "BP_BuildDefines.h"

#if BP_ENABLE_DEBUG_CAMERA()

#define	OX	(-10)

enum {
    CAMERA_NORMAL,
    CAMERA_FIX,
    CAMERA_VIEW,
    CAMERA_MODE_MAX
};

#define	SPEED_UP	(10)

#define	ROT_SPEED	(16)
#define	TRACK_SPEED	(48)
#define	ANGLE_SPEED	(0.01F)
#define	SHIFT_SPEED	(30.0F)

#define	DISP_SHIFT	(40)

#define	TEST_CAMERA	GV_StrCode( "テストカメラ" ) 
static	int		ViewCameraMode = 0 ;
static	int		DispMode = 0 ;

static	int		BackupOK = 0 ;
static	int		BehindBuf ;
static	FVECTOR	BehindShift = { 0.0F, 0.0F, 0.0F } ;
static	FVECTOR	RstickShift = { 0.0F, 0.0F, 0.0F } ;

static	int		CameraChanl = 0 ;
static	int		CameraLevel = 0 ;

static	int		Which = 0 ;

static	int		DebugCameraOn[ 3 ] = { 0 } ;
static	int		CharaNo = 0 ;
static	CONTROL	*Chara ;

typedef	struct	{
	GV_ACT		actor ;

	FMATRIX		world ;
	FVECTOR		pos ;
	FVECTOR		uvs[ 2 ] ;

	GM_CameraSet	backup ;

	int		screen_sw ;
	int		print_sw ;
	int		mode ;
	GV_PAD		*pad ;

	GM_CameraSet	*camera ;
	//	DG_PRIM		*prim_sprt ;
	int		level ;
	int		reserved ;
} Work ;

#if 0
static void InitPacket( DG_PRIM_PACKET *packet, DG_TEX *tex, int n, int which )
{
	DG_SPRT	*sprt ;
	int					data_size, i ;

	sprt = (DG_SPRT*)packet->prim_top ;
	/* プリミティブ描画設定初期化処理 */
	*(u_long64*)&packet->prim_init.tex0 = tex->tex0_base ;
	*(u_long64*)&packet->prim_init.clamp = tex->clamp ;
	*(u_long64*)&packet->prim_init.alpha = SCE_GS_SET_ALPHA( 2,0,0,1,0 ) ; /* 加算半透明 */
	/* 半透明属性付加 */
	packet->gif_tag.PRIM |= SCE_GS_PRIM_ABE ;
	/* プリミティブデータ部の初期化処理 */
	for ( i = n ; i > 0 ; i-- ){
		DG_SET_RGBA1( sprt, 128, 128, 128, 128 );
		DG_SET_STQ1( sprt, 0.0F * tex->u_scale + tex->u_offset, 0.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_STQ2( sprt, 1.0F * tex->u_scale + tex->u_offset, 1.0F * tex->v_scale + tex->v_offset, 1.0F );
		DG_SET_XYZF1( sprt, (2048-256)*16, (2048-224)*16, DRAW_Z_MAX+1, 255*16 );
		DG_SET_XYZF2( sprt, (2048-20)*16, (2048-30)*16, DRAW_Z_MAX+1, 255*16 );
		sprt++ ;
	}
}

void InitParam_all( Work *work )
{

	
}

void InitParam( Work *work )
{

}
#endif

static	CONTROL		*NextChara( void )
{
    CharaNo ++ ;
    if ( CharaNo >= GM_N_WhereList ) CharaNo = 0 ;
    return GM_WhereList[ CharaNo ] ;
}

/* アナログ右のカメラ移動（プレイヤーの関数と同じ） */
static	void	GetCameraShiftforAnalogR( FVECTOR *shift, FVECTOR *pos, FVECTOR *trg,
										  GV_PAD *pad ) 
{
    FVECTOR		vec, diff ;
	SVECTOR		rot ;
	float		len, len2 ;

	DG_COPY_VEC( shift, &DG_ZeroVector ) ;
	if ( !( pad->analog_input & GV_PAD_ANALOG_R_USE ) ) return ;
    vec.vx = -( float )( pad->right_dx - 128 ) ;
	vec.vy = -( float )( pad->right_dy - 128 ) ;
	
	PL_ShukanReverse( &vec.vy, NULL ) ;

	if ( DG_FABS( vec.vx ) < ANALOG_MARGIN_F ) vec.vx = ANALOG_MARGIN_F ;
	if ( DG_FABS( vec.vy ) < ANALOG_MARGIN_F ) vec.vy = ANALOG_MARGIN_F ;
	vec.vx = ( vec.vx > 0.0F ) ? vec.vx - ANALOG_MARGIN_F : vec.vx + ANALOG_MARGIN_F ;
	vec.vy = ( vec.vy > 0.0F ) ? vec.vy - ANALOG_MARGIN_F : vec.vy + ANALOG_MARGIN_F ;
	vec.vx *= 8.0F ;
	vec.vy *= 8.0F ;
    vec.vz = 0.0F ;
	vec.vw = 1.0F ;
	_sceVu0SubVector( &diff, trg, pos ) ;
	len = GV_VecLen3F( &diff ) ;
	vec.vx *= len / 2000.0F ;
	vec.vy *= len / 2000.0F ;
	GV_VecToRot( &diff, &rot ) ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_PutVector( &vec, shift, 1 ) ;

	diff.vy = 0.0F ;
	len = GV_VecLen3F( &diff ) ;
	_sceVu0Normalize( &diff, &diff ) ;
	len2 = _sceVu0InnerProduct( shift, &diff ) ;
	if ( -len2 < len ) return ;
	len *= 0.8F ;
	_sceVu0ScaleVector( shift, shift, DG_FABS( len / len2 ) ) ;
}

static	int	RotSpeed( pad, p1, p2 )
GV_PAD		*pad ;
int		p1, p2 ;
{
    float	v ;

    if ( pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		if ( p1 & ( PAD_L | PAD_R ) ) {
			v = DG_FABS( ( float )( pad->left_dx - 128 ) ) ;
		} else {
			v = DG_FABS( ( float )( pad->left_dy - 128 ) ) ;
		}
		v -= ANALOG_MARGIN_F ;
		v /= ( 127.0F - ANALOG_MARGIN_F ) ;
    } else {
		v = ( float )pad->pressure[ p2 ] / 256.0F ;
    }
    return ( int )( ( float )ROT_SPEED * v ) ;
}

static	int	TrackSpeed( pad, p1 )
GV_PAD		*pad ;
int		p1 ;
{
    float	v ;

    v = ( float )pad->pressure[ p1 ] / 256.0F ;
    return ( int )( ( float )TRACK_SPEED * v ) ;
}

static	int	ShiftSpeed( pad, p1, p2 )
GV_PAD		*pad ;
int		p1, p2 ;
{
    float	v ;

    if ( pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		if ( p1 & ( PAD_L | PAD_R ) ) {
			v = DG_FABS( ( float )( pad->left_dx - 128 ) ) ;
		} else {
			v = DG_FABS( ( float )( pad->left_dy - 128 ) ) ;
		}
		v -= ANALOG_MARGIN_F ;
		v /= ( 127.0F - ANALOG_MARGIN_F ) ;
    } else {
		v = ( float )pad->pressure[ p2 ] / 256.0F ;
    }
    return ( int )( ( float )SHIFT_SPEED * v ) ;
}

static void Act( Work *work )
{
    int		pad, status, press, mode ;	
    int		speed_up ;
    GM_CameraSet	*cam, *now, *back ;

    if ( GM_Debug2PMode != GM_DEBUG_MODE_CAMERA_SET &&
		GM_Debug2PMode != GM_DEBUG_MODE_CAMERA_VIEW ) {
		if ( work->camera->on != 0 ) {
			work->camera->on = 0 ;
			GM_ChangeCamera( work->camera->chanl ) ;
		}
		//	DG_InvisiblePrim( work->prim_sprt ) ;
		GM_ResetPlayerStatus( PLAYER_STOP ) ;	
		DebugCameraOn[ 0 ] = 0 ;
		DebugCameraOn[ 1 ] = 0 ;
		DebugCameraOn[ 2 ] = 0 ;
		Which = 0 ;
		return ;
    }
    cam = work->camera ;
    status = pad = work->pad->status ;
    press = work->pad->press ;

    if ( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_SET &&
		cam->chanl == 0 && work->level == 0 ) {
		if ( GV_PadDataDirect[ 1 ].press & PAD_AR ) {
			Which ++ ;
			BackupOK = 0 ;
			DebugCameraOn[ 0 ] = 0 ;
			DebugCameraOn[ 1 ] = 0 ;
			DebugCameraOn[ 2 ] = 0 ;
			if ( Which >= 3 ) Which = 0 ;
		} 
		switch( Which ) {
		case 0 :
			CameraChanl = 0 ;
			CameraLevel = 0 ;
			break ;
		case 1 :
			CameraChanl = 0 ;
			CameraLevel = 1 ;
			break ;	   
		case 2 :
		default :
			CameraChanl = 1 ;
			CameraLevel = 0 ;	    
		}
    } else if ( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_VIEW ) {
		Which = 0 ;
		CameraChanl = 0 ;
		CameraLevel = 0 ;	
    }

#if 0
    if ( CameraChanl != cam->chanl ) {
		status &= ~( PAD_UDLR | PAD_A | PAD_B | PAD_X | PAD_Y ) ;
		press &= ~( PAD_UDLR | PAD_A | PAD_B | PAD_X | PAD_Y ) ;
    }
#endif
	//    GM_ChangeCamera( 0 ) ;
	//    DG_VisiblePrim( work->prim_sprt ) ;
	//    DG_InvisiblePrim( work->prim_sprt ) ;
#if 0
    if ( ( work->pad->press & PAD_STA ) && DebugCameraOn[ Which ] == 1 ) {
		GM_ChangeCamera( cam->chanl ) ;
		now = GM_FindCameraSet( TEST_CAMERA, cam->chanl ) ;
		if ( now != NULL ) now->on = 0 ;
		//	printf( "debug camera mode : view\n" ) ;
    }
#endif
    if ( press & PAD_L1 ){
		work->print_sw= 1 - work->print_sw ;
    }
    if ( cam->chanl != CameraChanl ||
		work->level != CameraLevel ) {
		if ( cam->on != 0 ) {
			cam->on = 0 ;
			GM_ChangeCamera( cam->chanl ) ;
		}
		return ;
    }

    if ( DebugCameraOn[ Which ] == 0 ) {
		/* 初期設定 */
		now = GM_GetCurrentCameraSet( cam->chanl ) ;
		ASSERT( now != NULL ) ;
		if ( BackupOK == 0 ) {
			back = &( work->backup ) ;
			*back = *now ;
		}
		BackupOK = 1 ;
		cam->position = now->position ;
		cam->target = now->target ;
		cam->rotate = now->rotate ;
		cam->track = now->track ;
		cam->angle = now->angle ;
		cam->on = 1 ;
		//	work->backup = *cam ;
		DebugCameraOn[ Which ] = 1 ;
		BehindBuf = 0 ;
		mode = work->mode = CAMERA_NORMAL ;
		GM_ChangeCamera( cam->chanl ) ;
		//	printf( "debug camera mode : normal\n" ) ;
    } 

    mode = work->mode ;
    if ( GM_Debug2PMode == GM_DEBUG_MODE_CAMERA_SET ) {
		if ( press & PAD_R1 ) {
			if ( ++ mode >= CAMERA_VIEW ) {
				mode = CAMERA_NORMAL ;
				cam->target = GM_CameraTargetValue[ cam->chanl ] ;
				//		printf( "debug camera mode : normal\n" ) ;
			} else {
				//		printf( "debug camera mode : fix\n" ) ;
			}
			work->mode = mode ;
			GM_ChangeCamera( cam->chanl ) ;
			
			if ( mode == CAMERA_FIX ) {
				DG_COPY_VEC( &RstickShift, &DG_ZeroVector ) ;
			}
		}
		if ( press & PAD_SEL ) {
			now = GM_FindCameraSet( GV_StrCode( "ステージカメラ" ), cam->chanl ) ;
			if ( now != NULL ) {
				GM_CopyCamera( cam, now ) ;
				cam->flag = CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP ;
			}
		}
		speed_up = ( GV_PadDataDirect[ 1 ].status & PAD_L2 ) ? SPEED_UP : 1 ;
    } else {
		mode = work->mode = CAMERA_VIEW ;
        if ( work->camera->on != 0 ) {
			work->camera->on = 0 ;
			GM_ChangeCamera( work->camera->chanl ) ;
		}
		speed_up = 0 ;
    }

    switch( mode ) {
    case CAMERA_NORMAL :
        if ( press & PAD_AL ) {
			if ( GM_PlayerDebugMode != GM_PDM_NOBEHIND ) 
				GM_PlayerDebugMode = GM_PDM_NOBEHIND ;
			else 
				GM_PlayerDebugMode = 0 ;
		}
        if ( GV_PadDataDirect[ 1 ].press & PAD_R2 ) Chara = NextChara() ;
        if ( CharaNo == 0 ) cam->target = GM_CameraTarget ;
        else cam->target = Chara->mov ;
		if ( status & PAD_D ) cam->rotate.vx = ( cam->rotate.vx - RotSpeed( work->pad, PAD_D, PAD_PRESS_D ) ) & 4095 ;
		if ( status & PAD_U ) cam->rotate.vx = ( cam->rotate.vx + RotSpeed( work->pad, PAD_U, PAD_PRESS_U ) ) & 4095 ;
		if ( status & PAD_L ) cam->rotate.vy = ( cam->rotate.vy - RotSpeed( work->pad, PAD_L, PAD_PRESS_L ) ) & 4095 ;
		if ( status & PAD_R ) cam->rotate.vy = ( cam->rotate.vy + RotSpeed( work->pad, PAD_R, PAD_PRESS_R ) ) & 4095 ;
		if ( status & PAD_X ) cam->angle -= ANGLE_SPEED ;
		if ( status & PAD_Y ) cam->angle += ANGLE_SPEED ;
		if ( status & PAD_A ) cam->track -= TrackSpeed( work->pad, PAD_PRESS_A ) * speed_up ;
		if ( status & PAD_B ) cam->track += TrackSpeed( work->pad, PAD_PRESS_B ) * speed_up ;
        if ( cam->track < 100 ) cam->track = 100 ;
		GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;
		cam->type = GM_CAM_TYPE_TARGET_AND_ROTATE ;
		break ;
    case CAMERA_FIX :
        if ( press & PAD_AL ) {
			if ( GM_PlayerDebugMode != GM_PDM_NOBEHIND ) 
				GM_PlayerDebugMode = GM_PDM_NOBEHIND ;
			else 
				GM_PlayerDebugMode = 0 ;
		}
        cam->position.vx -= BehindShift.vx ;
        cam->position.vz -= BehindShift.vz ;
        cam->target.vx -= BehindShift.vx ;
        cam->target.vz -= BehindShift.vz ;
        BehindShift.vx = 0.0F ;
        BehindShift.vz = 0.0F ;

		/* ２コンのアナログＲでビハインドのようなカメラ */
		_sceVu0SubVector( &cam->target, &cam->target, &RstickShift ) ;

        if ( status & PAD_B ) {
			FVECTOR	shift ;

			shift = DG_ZeroVector ;
			if ( !( status & PAD_R2 ) ) {
				if ( status & PAD_U ) shift.vy = (float)( ShiftSpeed( work->pad, PAD_U, PAD_PRESS_U ) * speed_up ) ;
				if ( status & PAD_D ) shift.vy = (float)( - ShiftSpeed( work->pad, PAD_D, PAD_PRESS_D ) * speed_up );
				if ( status & PAD_R ) shift.vx = (float)( - ShiftSpeed( work->pad, PAD_R, PAD_PRESS_R ) * speed_up );
				if ( status & PAD_L ) shift.vx = (float)( ShiftSpeed( work->pad, PAD_L, PAD_PRESS_L ) * speed_up );
			}
			if ( status & PAD_A ) {
				shift.vz = shift.vy ;
				shift.vy = 0.0F ;
			} 
			DG_SetPos2( &DG_ZeroVector, &( cam->rotate ) ) ;
			DG_RotVector( &shift, &shift, 1 ) ;
			_sceVu0AddVector( &( cam->position ), &( cam->position ), &shift ) ;
			_sceVu0AddVector( &( cam->target ), &( cam->target ), &shift ) ;
			if ( status & PAD_X ) cam->angle -= ANGLE_SPEED ;
			if ( status & PAD_Y ) cam->angle += ANGLE_SPEED ;
			GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
								&( cam->rotate ), &( cam->track ) ) ;
			GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
								&( cam->rotate ), &( cam->track ) ) ;		
			cam->type = GM_CAM_TYPE_CAMERA_AND_TARGET ;	    
		} else {
			if ( !( status & PAD_R2 ) ) {
				if ( status & PAD_D ) 
					cam->rotate.vx = ( cam->rotate.vx - RotSpeed( work->pad, PAD_D, PAD_PRESS_D ) ) & 4095 ;
				if ( status & PAD_U ) 
					cam->rotate.vx = ( cam->rotate.vx + RotSpeed( work->pad, PAD_U, PAD_PRESS_U ) ) & 4095 ;
				if ( status & PAD_L ) 
					cam->rotate.vy = ( cam->rotate.vy - RotSpeed( work->pad, PAD_L, PAD_PRESS_L ) ) & 4095 ;
				if ( status & PAD_R ) 
					cam->rotate.vy = ( cam->rotate.vy + RotSpeed( work->pad, PAD_R, PAD_PRESS_R ) ) & 4095 ;
			}
			if ( status & PAD_A ) {
				if ( status & PAD_X ) cam->track -= TrackSpeed( work->pad, PAD_PRESS_X ) * speed_up ;
				if ( status & PAD_Y ) cam->track += TrackSpeed( work->pad, PAD_PRESS_Y ) * speed_up ;
				GM_CameraMakeTarget( &( cam->position ), &( cam->target ), 
									&( cam->rotate ), &( cam->track ) ) ;
			} else {
				if ( status & PAD_X ) cam->angle -= ANGLE_SPEED ;
				if ( status & PAD_Y ) cam->angle += ANGLE_SPEED ;
				GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
									&( cam->rotate ), &( cam->track ) ) ;
			}
			cam->type = GM_CAM_TYPE_TARGET_AND_ROTATE ;
		}
		/* ２コンのアナログＲでビハインドのようなカメラ */
		{
			GetCameraShiftforAnalogR( &RstickShift, &cam->position, &cam->target, GV_PadDataDirect + 1 ) ;
			_sceVu0AddVector( &cam->target, &cam->target, &RstickShift ) ;
		}
        /* Ｒ２で覗き込み Ｂを押してると駄目よ */
        if ( ( status & PAD_R2 ) && !( status & PAD_B ) ) {
//			static FVECTOR	pos ;
			FVECTOR	diff;
//			FVECTOR	vec ;
			SVECTOR	dir ;
//			float	th;
			float	len ; 

			if ( status & PAD_U ) BehindBuf ++ ;
			else if ( status & PAD_D ) BehindBuf -- ;
			if ( status & ( PAD_L | PAD_R ) ) {
				_sceVu0SubVector( &diff, &cam->position, &cam->target ) ;
				diff.vy = 0.0F ;
				len = GV_VecLen3F( &diff ) ;
				dir.vy = GV_VecDir2( &diff ) ;
				dir.vx = dir.vz = 0 ;
#if 0
				if ( GM_BehindMode == 0 ) {
					if ( status & PAD_L ) dir.vy -= BehindBuf ;
					else  		  dir.vy += BehindBuf ;
					dir.vy &= 4095 ;
					GV_DirVec3( &dir, ( int )len, &diff ) ;
					_sceVu0AddVector( &pos, &cam->target, &diff ) ;
					/* 当たり見る */
					if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id,
											   &cam->position, &pos, 
											   HZX_CHK_ALL, 0, 0 ) ) {
						HZX_GetOnlineVector( &vec ) ;
						len = GV_VecLen3F( &vec ) ;
						th = len - 250.0F ;
						if ( th <= 0.0F ) {
							_sceVu0CopyVector( &vec, &DG_ZeroVector ) ;
						} else {
							GV_LenVec3F( &vec, &vec, len, th ) ;
						}
						_sceVu0AddVector( &pos, &cam->position, &vec ) ;
					}
					cam->position.vx = pos.vx ;
					cam->position.vz = pos.vz ;
				} else {
					if ( work->pad->status & PAD_L ) {
						dir.vy -= 1024 ;
					} else {
						dir.vy += 1024 ;
					}
					dir.vy &= 4095 ;
					GV_DirVec3( &dir, BehindBuf, &diff ) ;
					_sceVu0AddVector( &pos, &cam->position, &diff ) ;
					/* 当たり見る */
					if ( HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id,
											   &cam->position, &pos, 
											   HZX_CHK_ALL, 0, 0 ) ) {
						HZX_GetOnlineVector( &vec ) ;
						len = GV_VecLen3F( &vec ) ;
						th = len - 250.0F ;
						if ( th <= 0.0F ) {
							_sceVu0CopyVector( &diff, &DG_ZeroVector ) ;
						} else {
							GV_LenVec3F( &vec, &diff, len, th ) ;
						}
					}		
					cam->position.vx += diff.vx ;
					cam->position.vz += diff.vz ;
					cam->target.vx += diff.vx ;
					cam->target.vz += diff.vz ;
					BehindShift.vx = diff.vx ;
					BehindShift.vz = diff.vz ;
				}
#endif
			}
		}        
		break ;
    case CAMERA_VIEW :
		if ( GV_PadDataDirect[ 1 ].press & PAD_R1 ) {
			ViewCameraMode = 1 - ViewCameraMode ;
		}
		if ( GV_PadDataDirect[ 1 ].press & PAD_A ) {
			DispMode = 1 - DispMode ;
			if ( DispMode == 0 ) {
				DG_SetDrawEnv( DG_Chanls, 0, 0, DRAW_WIDTH, DRAW_HEIGHT ) ;
			} else {
				DG_SetDrawEnv( DG_Chanls, 0, 0, DRAW_WIDTH / 2, DRAW_HEIGHT / 2 ) ;
			}
		}
		if ( ViewCameraMode == 0 ) {
			now = GM_GetCurrentCameraSet( cam->chanl ) ;
		} else {
			now = GM_GetCurrentCamera( cam->chanl ) ;
#if 0
			now = GM_FindCameraSet( TEST_CAMERA, cam->chanl ) ;
			if ( now == NULL ) {
				//		printf( "test camera not found \n" ) ;
				now = GM_GetCurrentCameraSet( cam->chanl ) ;
			} else {
				now->on = 1 ;
			}
#endif
		}
		GM_ResetPlayerStatus( PLAYER_STOP ) ;
		//	now = GM_GetCurrentCameraSet( 0 ) ;
		ASSERT( now != NULL ) ;
		cam->position = now->position ;
		cam->target = now->target ;
		cam->rotate = now->rotate ;
		cam->track = now->track ;
		cam->angle = now->angle ;
        cam->bound1 = now->bound1 ;
        cam->bound2 = now->bound2 ;
        cam->limit1 = now->limit1 ;
        cam->limit2 = now->limit2 ;
		cam->type = GM_CAM_TYPE_CAMERA_AND_TARGET ;
        work->backup = *cam ;
    }
    if ( work->print_sw ) {
#if 1
//now = GM_GetCurrentCameraSet( 0 ) ;
//printf( "[ %d : %x ]\n", GV_Time, now ) ;
//GM_DumpCamera( now ) ;

		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12, 0 );
		DEBUG_Printf( "Chanl : %d : ", cam->chanl ) ;
		switch ( mode ) {
		case CAMERA_NORMAL :
			DEBUG_Printf( "MODE : NORMAL\n" ) ;
			break ;
		case CAMERA_FIX :
			DEBUG_Printf( "MODE : FIX\n" ) ;
			break ;	    
		case CAMERA_VIEW :
			if ( ViewCameraMode == 0 ) {
				DEBUG_Printf( "MODE : VIEW/NEXT\n" ) ;
			} else {
//				DEBUG_Printf( "MODE : TEST" ) ;
                DEBUG_Printf( "MODE : VIEW/CUR\n" ) ;
			}
			break ;
		}
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12, 0 );
		DEBUG_Printf( "POS : %.0f %.0f %.0f\n", cam->position.vx, cam->position.vy, cam->position.vz ) ;
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*2, 0 );
		DEBUG_Printf( "TRG : %.0f %.0f %.0f\n", cam->target.vx, cam->target.vy, cam->target.vz ) ;
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*3, 0 );
		DEBUG_Printf( "ROT : %d %d %d\n", cam->rotate.vx, cam->rotate.vy, cam->rotate.vz ) ;
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*4, 0 );
		DEBUG_Printf( "TRACK : %d\n", cam->track ) ;
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*5, 0 );
		DEBUG_Printf( "ANGLE : %.2f\n", cam->angle ) ;
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*6, 0 );
		//	DEBUG_Printf( "BEHIND : %d\n", BehindBuf ) ;	
		if ( work->level == 0 ) {
			DEBUG_Printf( "FORCE : WEAK\n" ) ;
		} else {
			DEBUG_Printf( "FORCE : STRONG\n" ) ;
		}
//		DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*8, 0 );
		DEBUG_Printf( "CHARA : %d\n", CharaNo ) ;	
		if ( GM_PlayerDebugMode == GM_PDM_NOBEHIND ) {
//			DEBUG_Locate( DISP_SHIFT + 20 + 2 + OX , 12+12*10, 0 );
			DEBUG_Printf( "BEHIND CAMERA DISABLE\n" ) ;
		}
        if ( mode == CAMERA_VIEW ) {
			DEBUG_Printf( "FLAG : %x \n", GM_GetNextCamera( cam->chanl )->flag ) ;
			DEBUG_Printf( "NAME : %x \n", GM_GetNextCamera( cam->chanl )->name ) ;
			if ( GM_Camera->chanl[ cam->chanl ].flag & GM_FLAG_CAMERA_INTERP ) {
				DEBUG_Printf( "INTERP : %d\n", GM_Camera->chanl[ cam->chanl ].time ) ;
			}
		}
        GM_SetCurrentMap( GM_CurrentStageMap ) ;
        //if ( GV_PauseLevel == 0 ) NewPointView( &cam->target, 32, 32, 232 ) ;
#if 0
		DEBUG_Printf( "\n" ) ;
		DEBUG_Printf( "BOUND : %.0f %.0f %.0f - ", cam->bound1.vx,
					cam->bound1.vy, cam->bound1.vz ) ;
		DEBUG_Printf( "%.0f %.0f %.0f\n", cam->bound2.vx,
					cam->bound2.vy, cam->bound2.vz ) ;
		DEBUG_Printf( "LIMIT : %.0f %.0f %.0f - ", cam->limit1.vx,
					cam->limit1.vy, cam->limit1.vz ) ;
		DEBUG_Printf( "%.0f %.0f %.0f\n", cam->limit2.vx,
					cam->limit2.vy, cam->limit2.vz ) ;
#endif
#endif
    }
#if 0
    if ( press & PAD_L2 ){
		work->screen_sw= 1 - work->screen_sw ;
    }
    packet = (DG_PRIM_PACKET*)( (int)work->prim_sprt->packs[DG_Clock] );
    sprt = (DG_SPRT*)packet->prim_top ;
    if(work->screen_sw ){
		DG_SET_RGBA1( sprt, 0, 0, 0, 0 );
    }else{
		DG_SET_RGBA1( sprt, 64, 64, 64, 64 );
    }
#endif
    if ( press & PAD_L2 ) {
		printf( "-p %.0f,%.0f,%.0f ", cam->position.vx, cam->position.vy, cam->position.vz ) ;
		printf( "-t %.0f,%.0f,%.0f \\\n", cam->target.vx, cam->target.vy, cam->target.vz ) ;
		printf( "-r %d,%d,%d ", cam->rotate.vx, cam->rotate.vy, cam->rotate.vz ) ;
		printf( "-f %d \\\n", cam->track ) ;
		printf( "-a %.0f \\\n\n", cam->angle * 100.0F ) ;
		printf( "%.0f,%.0f,%.0f ", cam->target.vx, cam->target.vy, cam->target.vz ) ;
		printf( "%.0f,%.0f,%.0f\n", cam->position.vx, cam->position.vy, cam->position.vz ) ;
		printf( "--------------------------------\n" ) ;
    }
}

static void Die( Work *work )
{
    //GM_DeleteCamera( work->camera ) ;
    GM_ResetPlayerStatus( PLAYER_STOP ) ;
	Chara = NULL ;
	CharaNo = 0 ;
}


static int GetResources( Work *work, int chanl, int level )
{
    {
		GM_CameraSet	*cam ;
		int		name ;

		name = GV_StrCode( "デバッグカメラ" ) | ( ( chanl << 4 | level << 0 ) << 24 ) ;

		work->level = level ;
		if ( level == 0 ) {
			/* ビハインドより弱い */
			work->camera = cam = NewProgramCamera( name, chanl, GM_CAMERA_PROG3, 128 ) ;
		} else {
			work->camera = cam = NewProgramCamera( name, chanl, GM_CAMERA_PROG1, 128 ) ;
		}
		GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET, CAM_FLAG_FIX | CAM_FLAG_PAUSE_NO_STOP ) ;
		GM_SetCameraAngle( cam, 2.0F ) ;
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP4, GM_CAM_INTERP_NO_SET, 0, 0 ) ;
		cam->on = 0 ;
    }
    work->pad = GV_PadDataDirect + 1 ; /* 2P */
    work->mode = CAMERA_NORMAL ;

    work->screen_sw = 1 ;
    work->print_sw = 1 ;

	if ( chanl == 0 && level == 0 ) {
		ViewCameraMode = 0 ;
		DispMode = 0 ;
		BackupOK = 0 ;
		BehindBuf = 0 ;
		BehindShift = DG_ZeroVector ;
		RstickShift = DG_ZeroVector ;
		CameraChanl = 0 ;
		CameraLevel = 0 ;
		Which = 0 ;
		DebugCameraOn[ 0 ] = 0 ;
		DebugCameraOn[ 1 ] = 0 ;
		DebugCameraOn[ 2 ] = 0 ;
		CharaNo = 0 ;
		Chara = NULL ;
	}

    return 0;
}

/* デバッグカメラ */
void *NewDebugCamera( chanl )
int		chanl ;
{
    Work	*work ;

    OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
								  sizeof( Work ), CAMERA_DEBUG_PRIO ) ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, chanl, 0 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

void *NewDebugCameraForce( chanl )
int		chanl ;
{
    Work	*work ;

    OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
								  sizeof( Work ), CAMERA_DEBUG_PRIO ) ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, chanl, 1 ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return (void *)work ;
}

#endif // defined BP_ENABLE_DEBUG_CAMERA()
