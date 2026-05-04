//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   cam_set.c
   カメラ設定関数
   
   1999/07/13 M.Sonoyama
   $Id: cam_set.c,v 1.1.1.3 2002/11/19 11:41:45 Yoshizawa1 Exp $
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
#include	"util.h"
#include	"camera.h"

#include "bp_matrix.h"

#define	MSG_ENTER	(6753643)
#define	MSG_LEAVE	(6411627)
#define	MSG_INSIDE	(5547371)

static	FVECTOR	Bound[] = {
    { -1000000.0F, -1000000.0F, -1000000.0F },
    { 1000000.0F, 1000000.0F, 1000000.0F }    
} ;

#define	Previous 	GM_PreviousCamera 

#if 0
static	int	DiffVec( FVECTOR *v1, FVECTOR *v2 )
{
	if ( DG_FABS( v1->vx - v2->vx ) > 200.0F ||
		 DG_FABS( v1->vy - v2->vy ) > 200.0F ||
		 DG_FABS( v1->vz - v2->vz ) > 200.0F ) return 1 ;
	return 0 ;
}
#endif

static	 void	SetCameraType( cam, cam_flag )
GM_CameraSet	*cam ;
int				cam_flag ;
{
    int			type, on, pre ;
    int			buf[ 4 ], same_out ;
    GM_CameraSet	*now = NULL ;
	FVECTOR		v1, v2 ;

   BP_Camera_SetIdFromScript( cam );   //BP_CAMERA - update camera id

    same_out = on = 0 ;
    if ( GCL_GetOption( 's' ) != NULL ) {
		pre = cam->on ;
		on = GCL_GetNextInt() ;
		if ( on == 1 || on == MSG_ENTER || on == MSG_INSIDE ) cam->on = 1 ;
		else				  cam->on = 0 ;
		//	cam->on = GCL_GetNextInt() ;
		if ( cam->on != 0 ) {
			if ( cam->off_time == GV_Time ) {
				/* 強制ＯＦＦと同フレームではＯＮできない */
				cam->on = 0 ;
				//printf( "force off frame off! %d\n", GV_Time ) ;
			} else {
				//printf( "%x on now\n", cam ) ;
				cam->flag |= CAM_FLAG_ON_NOW ;
			}
		} else {
			if ( cam->flag & CAM_FLAG_ON_NOW ) {
				/* ＯＮになった同じフレームでのＯＦＦは
				   認めないという謎な処理。（trap対策） */
				//printf( "same frame out : ignored\n" ) ;	    
				/* on == -1 ならば強制ＯＦＦ */
				if ( on != -1 ) {
					cam->on = 1 ;
					same_out = 1 ;
				} 
			}
			if ( on == -1 ) {
				/* 強制ＯＦＦされたフレームでは、
				   再ＯＮを不可にする。 */
				//printf( "camera force off time %d\n", GV_Time ) ;
				cam->off_time = GV_Time ;
			}
		}
		//printf( "[ %d ] on/off %x : %d : %x\n", GV_Time, cam, 
				 //	   cam->on, cam->flag ) ;
		//printf( "%d -> %d\n", pre, cam->on ) ;
		now = GM_GetCurrentCameraSet( cam->chanl ) ;
		/* on == MSG_INSIDEの時は、
		   ChangeCameraしない（off->onになったときは例外） */
		/* 現在有効でないカメラがＯＦＦになったときもしない */
		/* 現在有効なカメラが自分よりレベルが上のときもしない */
		if ( ( on != MSG_INSIDE || pre == 0 ) &&
			!( cam->on == 0 && now != cam ) &&
			( ( cam->level <= now->level ) ||	/* バグってるがこのままいく */
			/*( ( cam->level < now->level ) ||	   こっちが正解 */
			  ( cam->level == now->level && cam->priority <= now->priority ) ) ) {
			GM_ChangeCamera( cam->chanl ) ;
		} else if ( GM_Camera->chanl[ cam->chanl ].flag & GM_FLAG_CAMERA_FIRST ) {
			GM_ChangeCamera( cam->chanl ) ;
		}
		/* ディレイカメラ */
		if ( cam->on == 1 ) {
			if ( GCL_GetOption( 'o' ) != NULL ) {
				extern void *NewDelayCamera( GM_CameraSet *, int, int ) ;
				
				if ( ( on == MSG_ENTER || on == 1 ) &&
					( ( now->level > cam->level ) ||
					  ( now->level == cam->level && now->priority > cam->priority ) ) ) {
					NewDelayCamera( now, GCL_GetNextInt(), cam->priority ) ;
				}
			} else {
				if ( GM_CameraDelayTime[ cam->chanl ] != 0 &&	
					( ( now->level > cam->level ) ||
					  ( now->level == cam->level && now->priority > cam->priority ) ) ) {
//					printf( "delay reset %d\n", GV_Time ) ;
					GM_CameraDelayTime[ cam->chanl ] = 0 ;
				}
			}
		}
		/* ＯＦＦ命令が有効ならば何も設定しない */
		if ( cam->on == 0 ) return ; 
    }

    /* 同フレームアウトの時は、パラメータセットしない */
    if ( same_out ) return ;

    if ( GCL_GetOption( 'c' ) != NULL ) {
		type = GCL_GetNextInt() ;
		cam->flag &= CAM_FLAG_ON_NOW ;
    } else {
		type = -1 ;
    }
    switch ( type ) {
    case 0 :					/* 上俯瞰追従 */
		/* bound, limit, track & rotate */
		if ( GCL_GetOption( 'b' ) != NULL ) {
			DG_COPY_VEC( &v1, &cam->bound1 ) ;
			DG_COPY_VEC( &v2, &cam->bound2 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->bound1 ), 3 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->bound2 ), 3 ) ;
#if 0
			if ( DiffVec( &v1, &cam->bound1 ) || DiffVec( &v2, &cam->bound2 ) ) {
				if ( now == cam ) GM_SetCameraQuick( cam->chanl ) ;
			}
#endif
		} else if ( !( cam->flag & CAM_FLAG_SET ) ) {
			cam->bound1 = Bound[ 0 ] ;
			cam->bound2 = Bound[ 1 ] ;
		}
		if ( GCL_GetOption( 'l' ) != NULL ) {
			DG_COPY_VEC( &v1, &cam->limit1 ) ;
			DG_COPY_VEC( &v2, &cam->limit2 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->limit1 ), 3 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->limit2 ), 3 ) ;
#if 0
			if ( DiffVec( &v1, &cam->limit1 ) || DiffVec( &v2, &cam->limit2 ) ) {
				if ( now == cam ) GM_SetCameraQuick( cam->chanl ) ;
			}
#endif
		} else if ( !( cam->flag & CAM_FLAG_SET ) ) {
			cam->limit1 = Bound[ 0 ] ;
			cam->limit2 = Bound[ 1 ] ;
		}
		if ( GCL_GetOption( 'r' ) != NULL ) {	
			GCL_GetNextSV( ( short * )&( cam->rotate ) ) ;
		}
		if ( GCL_GetOption( 'f' ) != NULL ) {
			cam->track = GCL_GetNextInt() ;
		} 
		/* 追従カメラのトラックは700以上 */
		if ( cam->track < 700 ) cam->track = 700 ;

		cam->type = GM_CAM_TYPE_TARGET_AND_ROTATE ;
		cam->flag |= CAM_FLAG_TRACE | CAM_FLAG_BOUND | CAM_FLAG_LIMIT ;

//		DG_COPY_VEC( &cam->target, &GM_CameraTargetValue[ cam->chanl ] ) ;
		cam->target.vx = GV_NearRangeF( cam->target.vx, GM_CameraTargetValue[ cam->chanl ].vx,
									    750.0F ) ;
		cam->target.vy = GV_NearRangeF( cam->target.vy, GM_CameraTargetValue[ cam->chanl ].vy,
									    300.0F ) ;
		cam->target.vz = GV_NearRangeF( cam->target.vz, GM_CameraTargetValue[ cam->chanl ].vz,
									    750.0F ) ;

		GM_CameraMakeCamera( &( cam->position ), &( cam->target ), 
							 &( cam->rotate ), &( cam->track ) ) ;	
		break ;
    case 1 :					/* 位置・角度固定 */
		/* camera & target */
		if ( GCL_GetOption( 'p' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->position ), 3 ) ;
		}
		if ( GCL_GetOption( 't' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->target ), 3 ) ;
		}
		cam->type = GM_CAM_TYPE_CAMERA_AND_TARGET ;
		cam->flag |= CAM_FLAG_FIX ;
		GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
							&( cam->rotate ), &( cam->track ) ) ;

		/* リミットありＦＩＸカメラ */
		if ( cam_flag & CAM_FLAG_LIMITBOUND_EXIST ) {
			if ( GCL_GetOption( 'b' ) != NULL ) {
				DG_COPY_VEC( &v1, &cam->bound1 ) ;
				DG_COPY_VEC( &v2, &cam->bound2 ) ;
				GCL_GetNextIV( buf ) ;
				GV_IVtoFV( buf, ( float * )&( cam->bound1 ), 3 ) ;
				GCL_GetNextIV( buf ) ;
				GV_IVtoFV( buf, ( float * )&( cam->bound2 ), 3 ) ;
				cam->flag |= CAM_FLAG_BOUND ;
			} else {
				cam->flag &= ~CAM_FLAG_BOUND ;
			}
			if ( GCL_GetOption( 'l' ) != NULL ) {
				DG_COPY_VEC( &v1, &cam->limit1 ) ;
				DG_COPY_VEC( &v2, &cam->limit2 ) ;
				GCL_GetNextIV( buf ) ;
				GV_IVtoFV( buf, ( float * )&( cam->limit1 ), 3 ) ;
				GCL_GetNextIV( buf ) ;
				GV_IVtoFV( buf, ( float * )&( cam->limit2 ), 3 ) ;
				cam->flag |= CAM_FLAG_LIMIT ;
			} else {
				cam->flag &= ~CAM_FLAG_LIMIT ;
			}			
		} else {
			cam->flag &= ~( CAM_FLAG_BOUND | CAM_FLAG_LIMIT ) ;
		}

		break ;
    case 2 :					/* 位置固定・追従 */
		cam->flag |= CAM_FLAG_FIX_TRACE ;
		cam->flag &= ~( CAM_FLAG_BOUND | CAM_FLAG_LIMIT ) ;
		goto set_flag_skip ;
    case 3 :					/* 位置固定・Ｘ追従 */
		/* camera & limit */
		cam->flag |= CAM_FLAG_FIX_X_TRACE ;
		cam->flag &= ~( CAM_FLAG_BOUND | CAM_FLAG_LIMIT ) ;
		goto set_flag_skip ;
	case 5 :
		cam->flag |= CAM_FLAG_YROT_FIX_TRACE ;
		cam->flag &= ~( CAM_FLAG_BOUND | CAM_FLAG_LIMIT ) ;
		if ( GCL_GetOption( 'r' ) != NULL ) {	
			GCL_GetNextSV( ( short * )&( cam->rotate ) ) ;
		}
set_flag_skip :
		if ( GCL_GetOption( 'p' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->position ), 3 ) ;
		}
		if ( GCL_GetOption( 'b' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->bound1 ), 3 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->bound2 ), 3 ) ;
			cam->flag |= CAM_FLAG_BOUND ;
		} else if ( !( cam->flag & CAM_FLAG_SET ) ) {
			cam->bound1 = Bound[ 0 ] ;
			cam->bound2 = Bound[ 1 ] ;
		}
		if ( GCL_GetOption( 'l' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->limit1 ), 3 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, ( float * )&( cam->limit2 ), 3 ) ;
			cam->flag |= CAM_FLAG_LIMIT ;
		} else if ( !( cam->flag & CAM_FLAG_SET ) ) {
			cam->limit1 = Bound[ 0 ] ;
			cam->limit2 = Bound[ 1 ] ;
		}
		cam->type = GM_CAM_TYPE_CAMERA_AND_TARGET ;
		cam->flag |= CAM_FLAG_TRACE | CAM_FLAG_LIMIT ;
		if ( type != 5 ) {
			GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
								 &( cam->rotate ), &( cam->track ) ) ;
		} else {
			int		ry ;

			ry = cam->rotate.vy ;
			DG_COPY_VEC( &cam->target, &GM_CameraTargetValue[ cam->chanl ] ) ;
			GM_CameraMakeRotate( &( cam->position ), &( cam->target ), 
								 &( cam->rotate ), &( cam->track ) ) ;
			cam->rotate.vy = ry ;
		}
		break ;
    case 4 :					/* ターゲット固定・追従 */
		if ( GCL_GetOption( 't' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, &( cam->target ), 3 ) ;
		}	
		if ( GCL_GetOption( 'b' ) != NULL ) {
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, &( cam->bound1 ), 3 ) ;
			GCL_GetNextIV( buf ) ;
			GV_IVtoFV( buf, &( cam->bound2 ), 3 ) ;
		} else if ( !( cam->flag & CAM_FLAG_SET ) ) {
			cam->bound1 = Bound[ 0 ] ;
			cam->bound2 = Bound[ 1 ] ;
		}
		if ( GCL_GetOption( 'f' ) != NULL ) {
			cam->track = GCL_GetNextInt() ;
		} 	
		{
			FVECTOR	diff ;

			_sceVu0SubVector( &diff, &( cam->target ), &GM_CameraTarget ) ;
			GV_LenVec3F( &diff, &diff, GV_VecLen3F( &diff ), ( float )cam->track ) ;
			_sceVu0SubVector( &( cam->position ), &( cam->target ), &diff ) ;
		}
		cam->type = GM_CAM_TYPE_CAMERA_AND_TARGET ;
		cam->flag |= CAM_FLAG_TRACE | CAM_FLAG_BOUND | CAM_FLAG_FIX_TARGET_TRACE ;
		GM_CameraMakeRotate2( &( cam->position ), &( cam->target ), 
							 &( cam->rotate ), &( cam->track ) ) ;
		break ;
    default :
		;
    } 
    if ( GCL_GetOption( 'a' ) != NULL ) {
		buf[ 0 ] = GCL_GetNextInt() ;
		cam->angle = ( float )buf[ 0 ] / 100.0F ;
    }
    if ( GCL_GetOption( 'i' ) != NULL ) {
		int mode1 = GCL_GetNextInt() ;
		int mode2 = GCL_GetNextInt() ;
		int time1 = GCL_GetNextInt() ;
		int time2 = GCL_GetNextInt() ;
		GM_SetCameraInterpMode( cam, mode1, mode2, time1, time2 ) ;

		now = GM_GetCurrentCameraSet( cam->chanl ) ;
		if ( now == cam ) {
			if ( !( GM_Camera->chanl[ cam->chanl ].flag 
				   & ( GM_FLAG_CAMERA_CHANGED | GM_FLAG_CAMERA_INTERP ) ) ) {
				now = GM_GetCurrentCamera( cam->chanl ) ;
                if ( now->interp_func1 != cam->interp_func1 ||
					 now->interp_func2 != cam->interp_func2 ) {
//					printf( "current camera interp func is changed.\n" ) ;				
#if 0
					now->interp_func1 = cam->interp_func1 ;
					now->interp_time1 = cam->interp_time1 ;
					now->interp_func2 = cam->interp_func2 ;
					now->interp_time2 = cam->interp_time2 ;
#else
                    GM_ChangeCurrentCameraInterpFunc1( cam->chanl, 
													   cam->interp_func1, cam->interp_time1 ) ;
                    GM_ChangeCurrentCameraInterpFunc2( cam->chanl, 
													   cam->interp_func2, cam->interp_time2 ) ;
#endif
				}
			}
		}
    } else if ( !( cam->flag & CAM_FLAG_SET ) ) {
		GM_SetCameraInterpMode( cam, 0, -1, 0, 0 ) ;
    }

    if ( GCL_GetOption( 'k' ) != NULL ) {
		if ( GCL_GetNextInt() ) cam->flag |= CAM_FLAG_KILL ;
    }
    if ( GCL_GetOption( 'd' ) != NULL ) {
		if ( GCL_GetNextInt() ) cam->flag |= CAM_FLAG_PAD_ADJUST ;
		else			cam->flag &= ~CAM_FLAG_PAD_ADJUST ;
    }
    /* フラグを一度リセット */
    cam->flag &= ~( CAM_FLAG_NO_CUSHION | CAM_FLAG_BOUNDCHANGE_QUICK |
				    CAM_FLAG_INTERP_CAMERA_SET ) ;
//    if ( GCL_GetOption( 'z' ) != NULL ) {
//		cam->flag |= GCL_GetNextInt() ;
//    }
	cam->flag |= cam_flag ;

    /* スライドカメラ設定 */
    if ( GCL_GetOption( 'E' ) != NULL ) {
		short	dir ;	
		int	slide_type, slide_wait_count ;
		float	slide1, slide2 ;
		int	slide_step_int, flag ;

		slide_type = GCL_GetNextInt() + 1 ;
		slide_wait_count = GCL_GetNextInt() ;
		slide1 = ( float )GCL_GetNextInt() ;
		slide2 = ( float )GCL_GetNextInt() ;
#ifdef KP_XBOX
		// 下の計算でNanになったときの挙動がPS2と異なるので、細工。
		if( slide1 == 0.0F && slide2 == 0.0F ){
			// Errorになる。
			printf( "setcamera:slide Error 0\n" );
			slide1 = 0.000001F;
			slide2 = 0.0000001F;
		}
		if( slide1 == slide2 ){
			slide1 += 0.1F;
		}
#endif
		slide_step_int = GCL_GetNextInt() ;

		if ( GCL_NextStr() != NULL ) flag = GCL_GetNextInt() ;
		else			     flag = 0 ;

		if ( slide_type == 1 ) slide1 = -slide1 ;
		if ( on != MSG_INSIDE ||
			slide_type != cam->slide_type ||
			slide_wait_count != cam->slide_wait_count ||
			slide1 != cam->slide1 ||
			slide2 != cam->slide2 ||
			slide_step_int != cam->slide_step_int ) {
			/* パラメータに何らかの変化があったら再設定 */
			cam->slide_type = slide_type ;
			cam->slide_wait_count = slide_wait_count ;
			cam->slide1 = slide1 ;
			cam->slide2 = slide2 ;
			cam->slide_step_int = slide_step_int ;

			cam->slide_wait1 = cam->slide_wait2 = 0 ;

			if ( flag == 1 ) {
				if ( cam->slide_type == 1 ) {
					cam->slide_interp 
						= DG_FABS( slide1 ) / ( DG_FABS( slide1 ) + DG_FABS( slide2 ) ) ; 
				} else {
					/* 前後カメラのときはまだ考えてない */
					cam->slide_interp = 0.0F ;
				}
			} else {
				dir = GM_CheckPlayerStatus( PLAYER_CAUTION ) ? GM_PlayerCautionDir : GM_PlayerDir ;
				dir &= 4095 ;	    
				if ( cam->slide_type == 1 ) {
					/* 左右カメラ */
					if ( GV_DiffDirAbs( dir, 3072 ) < 512 ) {
						/* 左向き */
						cam->slide_interp = 0.0F ;
					} else if ( GV_DiffDirAbs( dir, 1024 ) < 512 ) {
						/* 右向き */
						cam->slide_interp = 1.0F ;
					} else {
						cam->slide_interp = 0.5F ;
					}		
				} else {
					/* 前後カメラ */
					if ( GV_DiffDirAbs( dir, 0 ) < 512 ) {
						/* 前向き */
						cam->slide_interp = 0.0F ;
					} else if ( GV_DiffDirAbs( dir, 2048 ) < 512 ) {
						/* 後ろ向き */
						cam->slide_interp = 1.0F ;
					} else {
						cam->slide_interp = 0.0F ;
					}
				}
			}
			cam->slide_step = BP_SafeDivideFEx( ( float )slide_step_int, DG_FABS( slide2 - slide1 ), 0.0f );
		}
		cam->flag &= ~( CAM_FLAG_NO_CUSHION_X | CAM_FLAG_NO_CUSHION_Z ) ;
		if ( cam->slide_type == 1 ) cam->flag |= CAM_FLAG_NO_CUSHION_X ;
		else cam->flag |= CAM_FLAG_NO_CUSHION_Z ;
		cam->slide_rot = cam->rotate ;
		cam->slide_track = cam->track ;
    } else {
		cam->slide_type = 0 ;
    }
    cam->flag |= CAM_FLAG_SET ;
#ifdef DEBUG_MODE
    if ( cam->name == GV_StrCode( "テストカメラ" ) ) {
		/* 設定はするけどＯＮにはしない */
		cam->on = 0 ;
    }
#endif
//	printf( "camset %x [%d]\n", cam->name, GV_Time ) ;
}

/* カメラパラメータセット */
void	*NewSetCamera( name, map )
int	name, map ;
{
    GM_CameraSet	*cam ;
	int				flag, time = 0 ;

	if ( GM_Camera == NULL ) return NULL ;
#if 0	/* この方法やめる */
	/* 前回の名前と比較 */
    if ( Previous == NULL ) {
		cam = GM_FindCameraOld( name ) ;
	} else {
		if ( Previous->name == name ) cam = Previous ;
		else {
			cam = GM_FindCameraOld( name ) ;
		}
	}
    if ( cam == NULL ) {
		printf( "camera %d not found\n", name ) ;
		return NULL ;
    }
	Previous = cam ;
#else
	cam = GM_FindCameraOld( name ) ;
    if ( cam == NULL ) {
		printf( "camera %d not found\n", name ) ;
		return NULL ;
    }
#endif

	flag = GCL_GetOptionValue( 'z', 0 ) ;
	
	/* 補完カメラ準備 */
	if ( flag & CAM_FLAG_INTERP_CAMERA_SET ) {
		time = GCL_GetOptionValue( 'N', 0 ) ;
		if ( time > 0 ) {
			if ( GM_InitInterpCameraWork( cam, time ) < 0 ) {
				flag &= ~CAM_FLAG_INTERP_CAMERA_SET ;
			} else {
				/* 現在の状態を保存 */
				*cam->interp_camera_aim = *cam ;
			}
		} else {
			flag &= ~CAM_FLAG_INTERP_CAMERA_SET ;
		}
	}

    SetCameraType( cam, flag ) ;

	if ( !( flag & CAM_FLAG_INTERP_CAMERA_SET ) ) {
		/* 補完カメラ後始末 */
		GM_FreeInterpCameraWork( cam ) ;
	} else {
		/* 補完カメラセット */
		GM_CameraSet	tmp ;

		tmp = *cam->interp_camera_aim ;
		*cam->interp_camera_aim = *cam ;
		*cam = tmp ;
		cam->flag |= CAM_FLAG_INTERP_CAMERA_SET ;
		cam->interp_camera_time = DIRECT_TICK( time ) ;
	}
    return cam ;
}

/*--------------------------------------------------------------*/

/* カメラ振動セット */
void	*NewCameraOscillation( name, map )
int	name, map ;
{
	if ( GM_Camera == NULL ) return NULL ;

    GCL_GetOption( 'a' ) ;
    GM_CameraAmplitude = ( float )GCL_GetNextInt() ;
    GCL_GetOption( 'c' ) ;
    GM_CameraCycle = 2.0F * PI / ( float )GCL_GetNextInt() ;

	GM_PushCameraOscillationValue( 0 ) ;

	return GM_Camera ;
}

/* カメラ振動セット（プログラム） */
void	GM_SetCameraOscillation( int chanl, float amplitude, float cycle )
{
	if ( amplitude >= 0.0F ) GM_CameraAmplitudeValue[ chanl ] = amplitude ;
	if ( cycle >= 0.0F ) GM_CameraCycleValue[ chanl ] = cycle ;
}

void	GM_PushCameraOscillationValue( int chanl )
{
	GM_CameraAmplitudeValueDef[ chanl ] = GM_CameraAmplitudeValue[ chanl ] ;
	GM_CameraCycleValueDef[ chanl ] = GM_CameraCycleValue[ chanl ] ;
}

void	GM_PopCameraOscillationValue( int chanl )
{
	GM_CameraAmplitudeValue[ chanl ] = GM_CameraAmplitudeValueDef[ chanl ] ;
	GM_CameraCycleValue[ chanl ] = GM_CameraCycleValueDef[ chanl ] ;
}

/* カメラ調整セット */
void	GM_SetCameraAdjust( chanl, adj )
int	chanl ;
FVECTOR	*adj ;
{
    GM_CameraAdjustExist[ chanl ] = 1 ;
    GM_CameraAdjust[ chanl ].vx = -adj->vx ;
    GM_CameraAdjust[ chanl ].vy = -adj->vy ;
}

/* 各チャンネルの表示マップを設定 */
int		NewSetChanlTargetMap( void )
{
	int		chanl, map ;
	int		id ;

	chanl = GCL_GetOptionValue( 'c', 0 ) ;
	map = GCL_GetOptionValue( 'm', 0 ) ;
	id = GM_GetMapID( map ) ;
	GM_SetChanlTargetMap( chanl, id ) ;
	GM_CalcChanlMap() ;
	return 0 ;
}


