//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   camerad.c
   新カメラシステムデーモン (new camera system daemon)
   
   1999/07/12 M.Sonoyama
   $Id: camerad.c,v 1.1.1.3 2002/11/19 11:41:46 Yoshizawa1 Exp $
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
#include "BP_Camera.h"//BP_CAMERA

#define	DEBUG_CAMERA	(12924084)	/* デバッグカメラ */

typedef	GM_CameraWork	Work ;

GM_CameraWork	*GM_Camera = NULL ;

/*---------------------------------------------------------------------------*/

/* グローバル */
float	GM_CameraAmplitudeValue[ 4 ] = { 0.0F } ;
float	GM_CameraCycleValue[ 4 ] = { 0.0F } ;
float	GM_CameraAmplitudeValueDef[ 4 ] = { 0.0F } ;
float	GM_CameraCycleValueDef[ 4 ] = { 0.0F } ;

GM_CameraSet		*GM_PreviousCamera = NULL ;

int	GM_CameraExclamationTime = 0 ;

FVECTOR	GM_CameraAdjust[ 4 ] ;
int	GM_CameraAdjustExist[ 4 ] ;

/*---------------------------------------------------------------------------*/

static	int		N_Cameras ;
static	GM_CameraSet	*CameraList[ MAX_CAMERAS ] ;

/* ステージカメラパッド補正値 */
static	int		StagePadAdjust[ MAX_CHANLS ] = { 0 } ;


/*---------------------------------------------------------------------------*/


/* カメラをリスト登録 */
void	GM_EntryCameraList( cam )
GM_CameraSet	*cam ;
{
    ASSERT( N_Cameras < MAX_CAMERAS ) ;
    CameraList[ N_Cameras ] = cam ;
    N_Cameras ++ ;
}

/* カメラをリストから削除 */
void	GM_DeleteCameraList( name )
u_int	name ;
{
    int			i, n ;
    GM_CameraSet	**list, *cur ;

    list = CameraList ;
    for ( i = 0; i < N_Cameras; i ++ ) {	
		cur = *list ;
		if ( cur->name == name ) {
			cur->flag |= CAM_FLAG_DELETED ;
			break ;
		}
		list ++ ;
    }

	if ( i == N_Cameras ) return ;

    for ( n = i; n < N_Cameras - 1; n ++ ) {
		*list = *( list + 1 ) ;
		list ++ ;
    }
    if ( N_Cameras > 0 ) N_Cameras -- ;
}

/* カメラを検索 */
GM_CameraSet	*GM_FindCameraSet( name, chanl )
u_int		name ;
int		chanl ;
{
    int			n ;
    GM_CameraSet	**list, *res ;

    n = N_Cameras ;
    list = CameraList ;
    res = NULL ;
    while( -- n >= 0 ) {
		res = *( list ++ ) ;
		if ( res->name == name && 
			res->chanl == chanl ) {
			return res ;
		}
    }
	//    printf( "camera %d not found\n", name ) ;
    return NULL ;
}

GM_CameraSet	*GM_FindCameraOld( name )
u_int		name ;
{
    int			n ;
    GM_CameraSet	**list, *res ;

    n = N_Cameras ;
    list = CameraList ;
    res = NULL ;
    while( -- n >= 0 ) {
		res = *( list ++ ) ;
		if ( res->name == name ) {
			return res ;
		}
    }
	//    printf( "camera %d not found\n", name ) ;
    return NULL ;
}

/*---------------------------------------------------------------------------*/

static	GM_CameraSet	*GM_FindNextCamera( GM_CameraChanl *chanl )
{
    GM_CameraSet	*next, *list ;
	int				i, find ;

	list = chanl->list ;
	find = 0 ;
	next = NULL ;
	for ( i = 0; i < GM_CAMERA_MAX; i ++ ) {
		next = list ;
		while ( 1 ) {
			next = next->next ;
			if ( next == NULL ) break ;
			if ( next->on == 0 ) continue ;
			find = 1 ;
			break ;
		}
		if ( find ) break ;
		list ++ ;
	}	
	return next ;
}

/* 使用するカメラを検索 */
static	GM_CameraSet	*FindUseCamera( chanl, time )
GM_CameraChanl		*chanl ;
int					*time ;
{
    GM_CameraSet	*next, *cur, *prev ;

    if ( *time > 0 ) {
		*time = *time - 1 ;
    }
    if ( GM_DelayCameraWork != NULL && *time == 0 ) {
//		printf( "delay end[%d]\n", GV_Time ) ;
		GV_DestroyOtherActorQuick( GM_DelayCameraWork ) ;
		GM_DelayCameraWork = NULL ;
		chanl->flag |= GM_FLAG_CAMERA_CHANGED ;
    }
    if ( chanl->flag & GM_FLAG_CAMERA_CHANGED ) {
		/* カメラが切り替わった */
		prev = chanl->next ;
		next = GM_FindNextCamera( chanl ) ;
		if ( next != NULL ) {
			chanl->flag &= ~GM_FLAG_CAMERA_CHANGED ;
			if ( next == chanl->next ) {
				/* 主観から主観への移行になってしまった時は
				   何もしない */
				if ( next->level == GM_CAMERA_SUBJECT ) return next ;
			}
			chanl->flag |= GM_FLAG_CAMERA_INTERP ;
			/* そのチャンネルのカメラがＯＮになった直後は、
			   補完なしでカメラをセットする */
			if ( chanl->flag & GM_FLAG_CAMERA_FIRST ) {
				chanl->current = *next ;
				chanl->flag &= ~GM_FLAG_CAMERA_FIRST ;
				chanl->interp_func = GM_CameraInterpQuick ;
				return next ;
			}
			/* 補完関数を選択 */
			cur = &( chanl->current ) ;
			/* 主観に移行する場合 */
			if ( next->level == GM_CAMERA_SUBJECT ) {
				if ( prev->flag & CAM_FLAG_BOUND ) {
					DG_COPY_VEC( &cur->bound1, &prev->bound1 ) ;
					DG_COPY_VEC( &cur->bound2, &prev->bound2 ) ;
					cur->flag |= CAM_FLAG_BOUND ;
				} else {
					cur->flag &= ~CAM_FLAG_BOUND ;
				}
				if ( prev->flag & CAM_FLAG_LIMIT ) {
					DG_COPY_VEC( &cur->limit1, &prev->limit1 ) ;
					DG_COPY_VEC( &cur->limit2, &prev->limit2 ) ;
					cur->flag |= CAM_FLAG_LIMIT ;
				} else {
					cur->flag &= ~CAM_FLAG_LIMIT ;
				}
			}
			/* 優先度の高い方の補完関数を優先する */
			if ( cur->level == next->level ) {
				if ( cur->priority > next->priority ) {
					/* nextのほうが優先高のときはnext_In */
					chanl->interp_func = next->interp_func1 ;
					chanl->time = next->interp_time1 ;
				} else {
					if ( cur->interp_func2 != NULL ) {
						/* out用補完関数が設定されている場合は */
						/* それを使う */
						chanl->interp_func = cur->interp_func2 ;
						chanl->time = cur->interp_time2 ;
					} else {
						chanl->interp_func = next->interp_func1 ;
						chanl->time = next->interp_time1 ;
					}
				}
			} else {
				if ( cur->level > next->level ) {
					/* next_IN */
					chanl->interp_func = next->interp_func1 ;
					chanl->time = next->interp_time1 ;
				} else {
					if ( cur->interp_func2 != NULL ) {		    
						/* cur_OUT */
						chanl->interp_func = cur->interp_func2 ;
						chanl->time = cur->interp_time2 ;			
					} else {
						/* next_IN */
						chanl->interp_func = next->interp_func1 ;
						chanl->time = next->interp_time1 ;
					}
				}
			}
//			printf( "[%d] %x %x %x %d\n", GV_Time, prev->name, next->name, 
//				     chanl->interp_func, chanl->time ) ;
			/* 更新処理 */
			cur->level = next->level ;
			cur->priority = next->priority ;
			cur->interp_func1 = next->interp_func1 ;
			cur->interp_time1 = next->interp_time1 ;
			cur->interp_func2 = next->interp_func2 ;
			cur->interp_time2 = next->interp_time2 ;
		}
    } else {
		next = chanl->next ;
    }
	if ( next == NULL ) printf( "[%d] warning : not found next camera!!\n", GV_Time ) ;
    return next ;
}

/* 補完処理 */
static	void	InterpCamera( chanl )
GM_CameraChanl	*chanl ;
{
    GM_CameraSet	*current, *next ;
    int			rotz ;
    void		( *func )( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;

    current = &( chanl->current ) ;
    next = chanl->next ;
    if ( chanl->flag & GM_FLAG_CAMERA_INTERP ) {
		func = chanl->interp_func ;
		( *func )( chanl, current, next ) ;
#if 0
		if ( chanl->flag & GM_FLAG_CAMERA_QUICK ) {
			_sceVu0CopyVector( &current->position, &next->position ) ;
			_sceVu0CopyVector( &current->target, &next->target ) ;
			current->angle = next->angle ;
			chanl->flag &= ~GM_FLAG_CAMERA_QUICK ;
		}
#endif
    } else if ( next != NULL ) {
		if ( chanl->flag & GM_FLAG_CAMERA_QUICK ) {
         current->bp_settings = next->bp_settings;   //BP_CAMERA - interpolate extra BP tweakables for HD
			_sceVu0CopyVector( &current->position, &next->position ) ;
			_sceVu0CopyVector( &current->target, &next->target ) ;
			current->angle = next->angle ;
			chanl->flag &= ~GM_FLAG_CAMERA_QUICK ;
		} else {
         GV_NearExp4BP( &current->bp_settings, &next->bp_settings ) ;  //BP_CAMERA - interpolate extra BP tweakables for HD
			GV_NearExp4VF( &current->position, &next->position, 3 ) ;
			GV_NearExp4VF( &current->target, &next->target, 3 ) ;
			current->angle = GV_NearExp4F( current->angle, next->angle ) ;
		}
		current->rotate = next->rotate ;
		current->track = next->track ;
		current->level = next->level ;
    }
#if 1
	if ( next->flag & CAM_FLAG_BOUNDCHANGE_QUICK ) {
		/* リミット、バウンドを再計算 */
		if ( next->flag & CAM_FLAG_BOUND ) {
			GM_CameraBoundTrace( &current->position, &next->bound1, &next->bound2 ) ;
		}
		if ( next->flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &current->target, &next->limit1, &next->limit2 ) ;
		}
	}
#endif
    /* Ｚ回転を代入 */
    rotz = current->rotate.vz & 4095 ;
    if ( rotz > 2048 ) rotz -= 4096 ;
    current->position.vw = ( float )rotz * TPI / 4096.0F ;
#if 1
	/* パラメータをセット */
	if ( !( current->flag & CAM_FLAG_FINISHCALC ) ) {
		GM_CameraMakeRotate( &current->position, &current->target,
							 &current->rotate, &current->track ) ;
	}
	current->flag &= ~CAM_FLAG_FINISHCALC ;
#endif
}

/* パッド調整 */
static	inline	void	PadAdjust( cam )
GM_CameraSet		*cam ;
{	
	int			chanl ;

	chanl = cam->chanl ;
	/* マルチチャンネル対応はまだ */
	/* ０チャンネルのみ */
	if ( chanl != 0 ) return ;

   //BP_CAMERA - use BP 3rd person camera pad adjust?
   if( cam->flag & CAM_FLAG_PAD_ADJUST_BP_3RD_PERSON )
   {
      GV_OriginPadSystem( cam->rotate.vy + 2048 ) ;
   }
   //BP_CAMERA - use BP 3rd person camera pad adjust?
    else if ( cam->flag & CAM_FLAG_PAD_ADJUST ) {
		GV_OriginPadSystem( cam->rotate.vy + 2048 ) ;
		if ( cam->level > GM_CAMERA_BEHIND ) {
			StagePadAdjust[ chanl ] = cam->rotate.vy + 2048 ;
		}
    } else if ( cam->flag & CAM_FLAG_PAD_ADJUST_NO_RESET ) {
		GV_OriginPadSystem( StagePadAdjust[ chanl ] ) ;
    } else {
		GV_OriginPadSystem( 0 ) ;
		StagePadAdjust[ chanl ] = 0 ;
    }
}

static float Theta[ MAX_CHANLS ] = { 0.0F }, Theta2[ MAX_CHANLS ] = { 0.0F } ;
static float AmplitudeDef = 64.0F, CycleDef = 2.0F * PI / 179.0F ;


/* 揺らしパラメータをセットしてカメラセット */
static	inline	void	SwingCamera( int i,
				     GM_CameraSet       *cam,
				     GM_CameraSet       *next,
				     GM_CameraSet       *master,
				     GM_CameraMatrixFunc set_mat )
{
    float	 t, amp, damp ;
    FVECTOR	pos, trg ;
    int		chanl ;

    chanl = cam->chanl ;
    _sceVu0CopyVector( &pos, &cam->position ) ;
    _sceVu0CopyVector( &trg, &cam->target ) ;

    if ( cam->level > GM_CAMERA_SUBJECT && 
		DG_FABS( GM_CameraAmplitudeValue[ chanl ] ) > 0.10F &&
		!( GM_Camera->chanl[ chanl ].flag & GM_FLAG_CAMERA_NO_SWING ) ) {
		if ( DG_FABS( GM_CameraCycleValue[ chanl ] ) > 0.001F ) {
			if ( !( GV_PauseLevel & ( GV_PAUSE_PAUSE | GV_PAUSE_MENU ) ) ) {
				Theta[ chanl ] += GM_CameraCycleValue[ chanl ] ; 
				Theta2[ chanl ] += CycleDef ;
			}
			amp = GM_CameraAmplitudeValue[ chanl ] ;
			damp = ( amp > AmplitudeDef ) ?	AmplitudeDef : amp ;
			t = amp * sinf( Theta[ chanl ] ) ;
			t += damp * sinf( Theta2[ chanl ] ) ;	
//			if ( t > amp ) t = amp ;
		} else {
			t = GM_CameraAmplitudeValue[ chanl ] ;
		}
		pos.vy += t ; trg.vy += t ;
		if ( Theta[ chanl ] > PI * 1000000.0F ) Theta[ chanl ] = 0.0F ;
		if ( Theta2[ chanl ] > PI * 1000000.0F ) Theta2[ chanl ] = 0.0F ;
		//	GM_CameraAmplitude = 0.0F ;
#if 0
		if ( next->flag & CAM_FLAG_BOUND ) {
			GM_CameraBoundTrace( &pos, &( next->bound1 ), &( next->bound2 ) ) ;
		}		
		if ( next->flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &trg, &( next->limit1 ), &( next->limit2 ) ) ;
		}		
#endif
    }
#if 0
    /* びっくりカメラ実験 */
    if ( GM_CameraExclamationTime > 0 ) {
		if ( GM_CameraExclamationTime & 1 ) {
			pos.vw += ( float )( BP_PS2_rand() % 8 ) / 100.0F ;
		} else {
			pos.vw -= ( float )( BP_PS2_rand() % 8 ) / 100.0F ;
		}
		GM_CameraExclamationTime -- ;
    }
#endif

    /* 最終調整 */
    if ( GM_CameraAdjustExist[ i ] && !( cam->flag & CAM_FLAG_NO_BLASTCAMERA ) ) {
		FVECTOR		adj ;

		GM_CameraAdjustExist[ i ] = 0 ;
		DG_SetPos2( &DG_ZeroVector, &cam->rotate ) ;
		DG_RotVector( &GM_CameraAdjust[ i ], &adj, 1 ) ;
		_sceVu0AddVector( &pos, &pos, &adj ) ;
		_sceVu0AddVector( &trg, &trg, &adj ) ;
		if ( next->flag & CAM_FLAG_BOUND ) {
			GM_CameraBoundTrace( &pos, &( next->bound1 ), &( next->bound2 ) ) ;
		}		
		if ( next->flag & CAM_FLAG_LIMIT ) {
			GM_CameraBoundTrace( &trg, &( next->limit1 ), &( next->limit2 ) ) ;
		}
    }

	/* DGシステムのフレームにマトリックスをセットする */
	/* DG_SetCamera2を拡張した(マトリックスを掛けてくれる) T.Morita */
    BP_Camera_SetActiveCamera( next );  //BP_CAMERA - set active camera for wide screen tweak system
	(*set_mat)( DG_Chanls + i, &pos, &trg, cam->angle ) ;

#if 1
	/* マスターをセット */
	*master = *cam ;
	DG_COPY_VEC( &master->position, &pos ) ;
	DG_COPY_VEC( &master->target, &trg ) ;
	GM_CameraMakeRotate( &master->position, &master->target,
						&master->rotate, &master->track ) ;
#endif
	/* リンク変数の更新 */
	if ( i == 0 ) {
		GM_CameraX = ( int )pos.vx ;
		GM_CameraY = ( int )pos.vy ;
		GM_CameraZ = ( int )pos.vz ;
		GM_CamTargX = ( int )trg.vx ;
		GM_CamTargY = ( int )trg.vy ;
		GM_CamTargZ = ( int )trg.vz ;
		GM_CamRotX = master->rotate.vx ;
		GM_CamRotY = master->rotate.vy ;
	}
}


/*---------------------------------------------------------------------------*/

static	void	Act( Work *work ) 
{
    GM_CameraSet	*current, *next ;
    GM_CameraChanl	*chanl ;
    int			i, *time ;
    int pauseLevelNoDebug = ( GV_PauseLevel & ~( GV_PAUSE_DEBUG ) );

    chanl = work->chanl ;
    time = GM_CameraDelayTime ;
    for ( i = 0; i < MAX_CHANLS; i ++, chanl ++, time ++ ) {
		if ( chanl->flag & GM_FLAG_CAMERA_NO_USE ) continue ;
		if ( DG_Chanl( i )->flag == 0 ) continue ;	/* 非表示中 */
		current = &( chanl->current ) ;
		/* 使用するカメラをチェック */
		next = FindUseCamera( chanl, time ) ;

		if ( next == NULL ){
			FVECTOR		from = {0,0,0,0};
			FVECTOR		to = {0,0,1,0};
         BP_Camera_SetActiveCamera( NULL );  //BP_CAMERA - set active camera for wide screen tweak system
			DG_SetCamera2( DG_Chanl(i), &from, &to, 0.0f );
		}
		if ( next == NULL ) continue ;
		if ( i == 0 ) {
			/* カットインステータス（チャンネル０のみ） */
			if ( next->level == GM_CAMERA_CUT_IN ) {
				GM_SetGameStatus( STATE_CUT_IN ) ;
			} else {
				GM_ResetGameStatus( STATE_CUT_IN ) ;
			}
		}
		chanl->next = next ;
#ifdef DEBUG_MODE
		if ( pauseLevelNoDebug && !( next->flag & CAM_FLAG_PAUSE_NO_STOP ) ) continue ;
#else
		if ( pauseLevelNoDebug ) continue ;
#endif
		/* 補完処理 */
		InterpCamera( chanl ) ;
		/* パッド調整 */
		PadAdjust( chanl->next ) ;
		/* カメラをセット */
		SwingCamera( i, current, chanl->next, &chanl->master, chanl->set_mat ) ;
		/* ある種のフラグはリセット */
		chanl->flag &= ~GM_FLAG_CAMERA_SLIDECAMDIR_REVERSE ;	
    }
}

static	void	Die( GM_CameraWork *work )
{
	GM_Camera = NULL ;
}

/*---------------------------------------------------------------------------*/

/* カメラシステム初期化 */
void	GM_InitCamera( void )
{
    int			i, n, *time ;
    GM_CameraSet	*cam ;
    GM_CameraChanl	*chanl ;

    GM_DelayCameraWork = NULL ;
    n = MAX_CHANLS ;
    chanl = GM_Camera->chanl ;
    time = GM_CameraDelayTime ;
    while( -- n >= 0 ) {
		cam = chanl->list ;
		for ( i = 0; i < GM_CAMERA_MAX; i ++ ) {
			cam->next = NULL ;
			cam->priority = GM_CAMERA_PRIO_MAX ;
			cam ++ ;
		}
		chanl->set_mat = DG_SetCamera2 ;/*カメラマトリックを作る関数をセット（これがデフォルト）*/
		chanl->next = &( chanl->current ) ;
		chanl->time = 0 ;
		chanl->interp_func = NULL ;
		chanl->flag = GM_FLAG_CAMERA_NO_USE ;
		chanl ++ ;
		*time = 0 ;
		time ++ ;
		GM_CameraModeValue[ n ] = 0 ;
		GM_CameraAdjustExist[ n ] = 0 ;
		GM_CameraAmplitudeValue[ n ] = GM_CameraAmplitudeValueDef[ n ] = 0.0F ;
		GM_CameraCycleValue[ n ] = GM_CameraCycleValueDef[ n ] = 0.0F ;
		Theta[ n ] = 0.0F ;
		Theta2[ n ] = 0.0F ;
    }
    N_Cameras = 0 ;
	memset( StagePadAdjust, 0, sizeof( int ) * MAX_CHANLS ) ; 
}

/*---------------------------------------------------------------------------*/

/* カメラデーモン起動 */
void	*GM_StartCamera( void )
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_CAMERA, GV_CLASS_OBJECT, 
									sizeof( Work ), CAMERA_DAEMON_PRIO ) ;
    GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL ) ;
    if ( work != NULL ) {
		GV_SetActor( work, Act, Die ) ;
		GM_Camera = work ;
		GM_InitCamera() ;
		/* デフォルトはチャンネル０のカメラを使用 */
		GM_UseCamera( 0 ) ;
    }

    GM_CameraTargetPtr[ 0 ] = NULL ;
    GM_CameraTargetPtr[ 1 ] = NULL ;
	GM_PreviousCamera = NULL ;

    return work ;
}

/*--------------------------------------------------------------*/

/* カメラ状態をチェックする。
   ＯＮになっていれば１を返す。*/
int	NewCameraSwitchCheck( void )
{
    int			name ;
    GM_CameraSet	*cam ;

    if ( GCL_GetOption( 's' ) != NULL ) {
		name = GCL_GetNextInt() ;
		cam = GM_FindCameraSet( name, 0 ) ;
		if ( cam == NULL ) return 0 ;
		return ( cam->on ) ;
    }    
    return 0 ;
}

/* 上下振動のＯＮ／ＯＦＦ */
int		NewCameraSwingMode( void )
{
	int		which, mode ;

	which = GCL_GetOptionValue( 'w', 0 ) ;
	mode = GCL_GetOptionValue( 'm', 0 ) ;
	if ( mode == GV_StrCode( "on" ) ) {
		GM_Camera->chanl[ which ].flag &= ~GM_FLAG_CAMERA_NO_SWING ;
	} else {
		GM_Camera->chanl[ which ].flag |= GM_FLAG_CAMERA_NO_SWING ;
	}
	return 0 ;
}
