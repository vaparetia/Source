/*
   routine.c 
   ライデン／各種関数
   
   1999/07/07 M.Sonoyama
   $Id: routine.c,v 1.1.1.3 2002/11/19 11:51:02 Yoshizawa1 Exp $			
*/

#if defined(BP_VITA)
#include <systemgesture.h>
#include "gesture_vta.h"
#endif

extern int gBP_UseVitaControlOverrides;

/*----------------------------------------------------------
  
  statics
  
----------------------------------------------------------*/

/*----------------------------------------------------------
  
  グローバル
  
----------------------------------------------------------*/

/* 主観モードに入る */
void		PL_IntoSubject( work )
PlayerWork	*work ;
{
   //BP_CAMERA - Inherit camera rotation?
   if( gBP_Camera_InheritRot )
   {
      // Copy 3rd person camera to 1st person player
      work->g_rot          = BP_NormalizeIntegerRot( GM_Camera->chanl[0].current.rotate.vx );//pitch
      work->control.rot.vy = BP_NormalizeIntegerRot( GM_Camera->chanl[0].current.rotate.vy );//yaw
   }
   //BP_CAMERA - Inherit camera rotation?

    SetStatus( PLAYER_WATCH ) ;

#if 0 //BP def PSX2
    /*HMD用のコード*/
    GM_SetCurrentCalcMat( 0, GM_SetHMDCamera ) ; /*HMD用のマトリックス計算*/
#endif

    //    _sceVu0CopyVector( &( work->peep_base ), &( work->control.mov ) ) ;
    _sceVu0CopyVector( &( work->peep_base ), &( work->camera ) ) ;

	GM_CameraDir.vx = work->camdir.vx = work->g_rot ;

    if ( Status( PLAYER_INTRUDE ) ) return ;
    if ( GM_CameraMode == 0 ) {
		SetVWait( work, - SUBJECT_WAIT_TIME_IN ) ;
    }
    GM_CameraMode = 1 ;
	if ( work->subject_camera->on == 0 ) {
		GM_ChangeCamera( work->subject_camera->chanl ) ;
		/* 入った瞬間だけ装備チェンジ禁止 */
		SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
		/* はだかのときだけ瞬時に消す */
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		}
	}
#ifndef NO_SUBJECT
	work->subject_camera->on = 1 ;
#endif
	PL_SubjectCameraOn() ;
    work->camdir.vy = work->camdir.pad = work->control.rot.vy ;
    GM_RadarSetFlag( &( work->radar ), RADAR_SIGHT ) ;
}

/* 主観モードから抜ける */
void		PL_LeaveSubject( work )
PlayerWork	*work ;
{
   //BP_CAMERA - Inherit camera rotation?
   if( gBP_Camera_InheritRot )
   {
      // Copy 1st person player to 3rd person camera
      gBP_3rdPersonCamera_Rot.vx = BP_NormalizeIntegerRot( work->camdir.vx ); // pitch
      gBP_3rdPersonCamera_Rot.vy = BP_NormalizeIntegerRot( work->camdir.vy ); // yaw
   }
   //BP_CAMERA - Inherit camera rotation?

    UnsetStatus( PLAYER_WATCH ) ;
    UnsetFlag( FLAG_PEEPING ) ;

    work->camdir.vx = work->g_rot ;	

#ifdef PSX2
    /*HMD用のコード*/
    if ( PL_SubjectMove == 0 ) {
	GM_SetCurrentCalcMat( 0, DG_SetCamera2 ) ; /*デフォルトのマトリックス計算*/
    }
#endif

    if ( Status( PLAYER_INTRUDE ) ) {
		return ;
	}
    if ( GM_CameraMode == 1 ) {
		SetVWait( work, SUBJECT_WAIT_TIME_OUT ) ;
    }
    GM_CameraMode = 0 ;
#if 0
	if ( work->subject_camera->on == 1 || 
		 work->weapon_camera->on == 1 ) {
		work->subject_camera->on = 0 ;
		work->weapon_camera->on = 0 ;
		GM_ChangeCamera( work->subject_camera->chanl ) ;
	}
#endif
	if ( Flag2( FLAG2_SUBJECT_START ) ) {
		GM_SetCameraInterpMode( work->subject_camera, GM_CAM_INTERP_INTO_SUBJECT,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;		  		
		GM_SetCameraInterpMode( work->weapon_camera, GM_CAM_INTERP_INTO_SUBJECT,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;		
		UnsetFlag2( FLAG2_SUBJECT_START ) ;
	}
	PL_SubjectCameraOff() ;
    GM_RadarResetFlag( &( work->radar ), RADAR_SIGHT ) ;
}

/* イントルード入る */
void			PL_IntoIntrude( work )
PlayerWork		*work ;
{
    if ( Status( PLAYER_INTRUDE ) ) return ;
    if ( GM_CameraMode == 0 ) {
		SetVWait( work, - SUBJECT_WAIT_TIME_IN ) ;
    }
    GM_CameraMode = 1 ;
    GM_ChangeCamera( work->subject_camera->chanl ) ;
	PL_SubjectCameraOn() ;
#ifndef NO_SUBJECT
    work->subject_camera->on = 1 ;
#endif
    UnsetStatus( PLAYER_WATCH ) ;
    SetStatus( PLAYER_INTRUDE ) ;
    SetFlag( FLAG_NO_STEP ) ;
    StopTurn( work ) ;
    GM_RadarSetFlag( &( work->radar ), RADAR_SIGHT ) ;
}

/* イントルード出る */
void		PL_LeaveIntrude( PlayerWork *work )
{
    if ( !Status( PLAYER_INTRUDE ) ) return ;
    if ( GM_CameraMode == 1 ) {
		SetVWait( work, SUBJECT_WAIT_TIME_OUT ) ;
    }
    GM_CameraMode = 0 ;
	PL_SubjectCameraOff() ;
#if 0
    GM_ChangeCamera( work->subject_camera->chanl ) ;
    work->subject_camera->on = 0 ;
    work->weapon_camera->on = 0 ;
#endif
    UnsetStatus( PLAYER_INTRUDE | PLAYER_WATCH ) ;
    UnsetFlag( FLAG_PEEPING ) ;
    SetFlag( FLAG_NO_STEP ) ;
    StopTurn( work ) ;
    GM_RadarResetFlag( &( work->radar ), RADAR_SIGHT ) ;
}

/* はりつき解除 */
void		PL_LeaveCaution( work )
PlayerWork	*work ;
{
    work->behind_camera_enable = 0 ;
    UnsetStatus( PLAYER_BEHIND_CAMERA_ENABLE ) ;

    UnsetStatus( PLAYER_CAUTION ) ;
    GM_PlayerCautionDir = -1 ;
    work->r_sphere = NORMAL_SPHERE ;
    GM_ConfigControlResetCollide( &( work->control ) ) ;

    if (work->inCautionMode)
    {
       work->inCautionMode = FALSE;
    }
}

/* 無敵状態になる */
void		PL_SetInvincible( work, time )
PlayerWork	*work ;
{
    work->invincible_time = time ;
    GM_ClearTargetDamage( &( work->def ) ) ;
    //GM_TargetSetSkip( &( work->def ) ) ;
	work->touch.damaged = 0 ;
	if ( Flag( FLAG_NOMUTEKI_ATTACKED ) ) return ;
    SetStatus( PLAYER_INVINCIBLE ) ;
}

/* 無敵解除 */
void		PL_UnsetInvincible( work )
PlayerWork	*work ;
{
	//if ( Flag( FLAG_NOMUTEKI_ATTACKED ) ) {
		//printf( "[%d]nomuteki kaijo\n", GV_Time ) ;
	//}
	UnsetFlag( FLAG_NOMUTEKI_ATTACKED ) ;
    UnsetStatus( PLAYER_INVINCIBLE ) ;
//    GM_TargetResetSkip( &( work->def ) ) ;
    work->invincible_time = 0 ;
}

/* メッセージ送信 */
void		PL_SendMessage( to, mesg, len )
int		to ;
int		*mesg ;
int		len ;
{
    GV_MSG	msg ;

    msg.address = to ;
    msg.message = mesg ;
    msg.message_len = len ;
    GV_SendMessage( &msg ) ;
}

/* 強制系プロック呼び出し */
void	PL_ExecForceActProc( PlayerWork *work, int mode, int time, int end_flag )
{
	FORCE		*f ;
	GCL_ARGS	args ;
	int			buf[ 8 ], proc, proc_flag ;
	int			exec, end_exec, evt ;

	f = PL_Force ;
	exec = end_exec = 0 ;
	if ( f->proc != 0 ) {
		if ( end_flag == 0 ) {
			if ( time == 0 || ( f->proc_flag & FA_PROC_FLAG_EVERY ) ) {
				exec = 1 ;
			}
		} else {
			if ( f->proc_flag & FA_PROC_FLAG_END ) {
				exec = 1 ;
				end_exec = 1 ;
			}
		}
	}
	if ( exec == 1 ) {
		if ( time == 0 ) evt = GM_STRCODE_ENTER ;
		else if ( end_flag ) evt = GM_STRCODE_LEAVE ;
		else					evt = GM_STRCODE_INSIDE ;
		args.argv = buf ;
		args.argc = 6 ;
		buf[ 0 ] = (int)work->control.mov.vx ;
		buf[ 1 ] = (int)work->control.mov.vy ;
		buf[ 2 ] = (int)work->control.mov.vz ;
		buf[ 3 ] = time ;
		buf[ 4 ] = mode ;
		buf[ 5 ] = evt ;

		/* 終了プロック内で、新しい強制プロックを設定できるように */
		proc = f->proc ;
		proc_flag = f->proc_flag ;
		if ( end_flag ) PL_ClearForceActProc() ;

		if ( proc_flag & FA_PROC_FLAG_BLOCK ) {
			GM_ExecBlock( ( char * )proc, &args ) ;
		} else {
			GM_ExecProc( proc, &args ) ;
		}
	}
	if ( end_exec == 0 &&
		( f->end_proc != 0 && end_flag ) &&
		( f->end_proc_motion < 0 || f->end_proc_motion == f->motion ) ) {
		/* 終了プロック内で、新しい強制プロックを設定できるように */
		proc = f->end_proc ;
		PL_ClearForceActProc() ;
		args.argv = buf ;
		args.argc = 5 ;
		buf[ 0 ] = mode ;
		buf[ 1 ] = f->proc_args[ 0 ] ;
		buf[ 2 ] = f->proc_args[ 1 ] ;
		buf[ 3 ] = f->proc_args[ 2 ] ;
		buf[ 4 ] = f->proc_args[ 3 ] ;
		GM_ExecProc( proc, &args ) ;
	}
}

void	PL_ClearForceActProc( void )
{
	PL_Force->proc = 0 ;
	PL_Force->proc_flag = 0 ;
	PL_Force->end_proc = 0 ;
	PL_Force->end_proc_motion = -1 ;
}

/* ローカル床チェック */
void	PL_LevelCheck( work )
PlayerWork	*work ;
{
    int		flag ;

    flag = HZX_LevelHazardCheck( work->control.hzx_id, 
								&( work->control.mov ), work->control.hzx_check_type, 
								work->control.flr_flag ) ;
    if ( flag != 0 ) {
		HZX_GetLevelHeight( work->control.levels ) ;
		HZX_GetLevelHazard( work->control.level[ 0 ], work->control.flr_atrs ) ;
    } 
    if ( !( flag & 1 ) ) work->control.levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
    if ( !( flag & 2 ) ) work->control.levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
    work->control.level_found = flag ;
}


/* 敵つかみ解除 */
void		PL_ClearCaptureTarget( work )
PlayerWork	*work ;
{
    CAPTURE_TARGET	*c ;

    c = &( work->capture ) ;
    if ( c->capture != NULL ) {
		c->capture->flag |= CAPTURE_FREE ;
		c->capture = NULL ;
		work->def.damaged &= ~TARGET_CAPTURE ;
		c->flag = 0 ;
		work->control.skip_flag &= ~CTRL_COLLIDE_CHECK ;
    }
}

/* 現在のモデルの前後長を求める */
void			PL_GetModelLength2( PlayerWork *work, float *front, float *back, int flag )
{
    int		i ;
    FMATRIX	inv ;
    FVECTOR	v, v2, mov ;
    DG_OBJ	*objs ;
	float	maxz, minz, len ;
	FVECTOR	base = { 0.0F, 0.0F, 1.0F } ;
	SVECTOR	rot ;
	
	rot.vx = work->g_rot ;
//	rot.vy = work->control.rot.vy ;
	rot.vy = rot.vz = 0 ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_RotVector( &base, &base, 1 ) ;

    objs = work->body.objs->objs ;
    _sceVu0TransposeMatrix( &inv, &work->body.objs->world ) ;
    GV_MatToVec( &work->body.objs->world, &mov ) ;
    inv.m[ 3 ][ 0 ] = 0.0F ;
    inv.m[ 3 ][ 1 ] = 0.0F ;
    inv.m[ 3 ][ 2 ] = 0.0F ;
    DG_SetPos( &inv ) ;
	if ( front != NULL ) *front = 0.0F ;
	if ( back != NULL ) *back = 0.0F ;
	maxz = -1000000.0F ;
	minz = 1000000.0F ;
    for ( i = 0; i < N_UNITS; i ++, objs ++ ) {
		if ( !( ( flag >> i ) & 1 ) ) continue ;
		GV_MatToVec( &objs->world, &v ) ;
		_sceVu0SubVector( &v, &v, &mov ) ;
		DG_PutVector( &v, &v, 1 ) ;
		_sceVu0ScaleVector( &v2, &base, _sceVu0InnerProduct( &v, &base ) ) ;
		len = GV_VecLen3F( &v2 ) ;
		if ( v.vz > 0.0F ) {
			if ( front != NULL && len > *front ) *front = len ;
		} else {
			if ( back != NULL && len > *back ) *back = len ;
		}
    }
}

void			PL_GetModelLength( PlayerWork *work, float *front, float *back )
{
	PL_GetModelLength2( work, front, back, 0x1fffff ) ;
}

/* 最も強く押されている十字キーの値 */
int		PL_StrongestUDLRValue( pad )
GV_PAD	*pad ;
{
	int		max, tmp ;
	u_char	*pr ;

	if ( pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		max = pad->left_dx - 128 ;
		if ( max < 0 ) max = -max ;
		tmp = pad->left_dy - 128 ;
		if ( tmp < 0 ) tmp = -tmp ;
		if ( max < tmp ) max = tmp ;

      //BP_INPUT - X360 controller is normalized unlike PS3 controller
      if( PlayerPad.enable )
      {
         // Use actual magnitude of stick direction rather than a single axis
         int dx = pad->left_dx - 128;
         int dy = pad->left_dy - 128;
         max = (int)(sqrtf((dx*dx) + (dy*dy)) + 0.5f);
         if( max > 128 ) 
         {
            max = 128;
         }
      }
      //BP_INPUT - X360 controller is normalized unlike PS3 controller

		if ( max >= ANALOG_MARGIN ) {
			max = ( int )( ( float )( max - ANALOG_MARGIN ) * 256.0F 
						  / ( float )( 128 - ANALOG_MARGIN ) ) ;
			return max ;
		}
		return 0 ;
	} else {
      if (!gBP_UseVitaControlOverrides || (GM_GameStatus & STATE_PAD_DEMO))
      {
		   pr = pad->pressure ;
		   max = pr[ PAD_PRESS_U ] ;

		   tmp = pr[ PAD_PRESS_D ] ;
         if ( max < tmp ) 
            max = tmp ;

		   tmp = pr[ PAD_PRESS_L ] ;
		   if ( max < tmp ) 
            max = tmp ;

		   tmp = pr[ PAD_PRESS_R ] ;
		   if ( max < tmp ) 
            max = tmp ;

		   return max ;
      }
      else
      {
         return 0;
      }
	}
}

/* 姿勢ステータス変更 */
static	inline	void	SetStanceState( state )
int		state ;
{
	long64 c ;

    c = 0 ;
    switch ( state ) {
    case STAND :
		c = Status( PLAYER_SQUAT ) ;
		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;
		break ;
    case SQUAT :
		c = !( Status( PLAYER_SQUAT | PLAYER_GROUND ) ) ;
		UnsetStatus( PLAYER_GROUND ) ;	
		SetStatus( PLAYER_SQUAT ) ;
		break ;
    case GROUND :
		UnsetStatus( PLAYER_SQUAT ) ;	
		SetStatus( PLAYER_GROUND ) ;
    }
    if ( c && !Flag2( FLAG2_STAGESTART ) ) {
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			//GM_SeSetMode( SD_P_STAND02N, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_P_STAND02N, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else {
			//GM_SeSetMode( SD_P_STAND02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_P_STAND02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		}
	}
}

/* 当たりチェック高さ変更 */
void		PL_ChangeStance( PlayerWork *work, int stance )
{
    if ( stance == work->stance ) return ;
    work->stance = stance ;
    work->hzx_height = HzxHeight[ stance ] ;
    SetStanceState( stance ) ;
    GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ stance ] ;
    if ( stance != GROUND ) {
		work->g_rot = 0 ;
		GM_ConfigControlResetCollide( &( work->control ) ) ;
    } else {
		GM_ConfigControlSetCollide( &( work->control ), 300.0F, 1000.0F ) ;
		SetArmAction( work, AMstand ) ;
    }
	//    work->control.step.vx = 0.0F ;
	//    work->control.step.vz = 0.0F ;
}

/* 武器毎のモーションセットを奪う */
void	PL_SetMotionChangeSets( int wp, short *set )
{
	ChangeSets[ wp ] = set ;
}

/* モーションを武器毎にセット */
void	PL_SetMotionSet( int wp )
{
    ASSERT( wp >= 0 && wp < MAX_WEAPONS ) ;
    MS.change = ChangeSets[ wp ] ;
    MS.attack = AttackSets[ wp ] ;
    MS.shared = SharedSet ;
	SetFlag( FLAG_ACTION_MUST_CHANGE ) ;
}

/* 移動速度レベルを返す */
/* PadForceが設定されていること */
int		PL_MoveLevel( PlayerWork *work )
{
	int			max ;

	max = PadForce ;
	if ( !( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) ) 
   {
      if (!gBP_UseVitaControlOverrides || (GM_GameStatus & STATE_PAD_DEMO))
		   max = ( int )( ( float )max * 2.40F ) ;
      else
         max = 0;
	}
	if ( max == 0 ) return 0 ;
	if ( max < PAD_WALK_TH ) return 1 ;
	return 2 ;
}

/* 指定位置、指定方向、指定距離に壁があるか調べる */
#ifdef DEBUG_MODE
static	int	MovRotLenDebug = 0 ;
#endif

static	FVECTOR		MovRotLenChkFrom ;
static	FVECTOR		MovRotLenChkTo ;

int		PL_CheckMovRotLenSegment( HZX_GROUP_ID hzx_id, 
								  FVECTOR *mov, FVECTOR *shift, SVECTOR *rot, 
								  float len, int chk, int flag, int fflag ) 
{
    FVECTOR	shf, from, to, vec ;
    int		c ;

	ResultHzxPtr = NULL ;
	GM_PlayerWork->work_l.result_len = 10000000.0F ;

    DG_SetPos2( mov, rot ) ;
    if ( shift->vx != 0.0F || shift->vy != 0.0F ||
		shift->vz != 0.0F ) {
		DG_RotVector( shift, &shf, 1 ) ;
		_sceVu0AddVector( &from, mov, &shf ) ;
		/* mov と from の間に壁があるときはＮＧ */
		c = HZX_OnlineHazardCheck( hzx_id, mov, &from, chk, flag, fflag ) ;
		if ( c ) {
			DG_COPY_VEC( &MovRotLenChkFrom, mov ) ;
			DG_COPY_VEC( &MovRotLenChkTo, &from ) ;
#ifdef DEBUG_MODE
			if ( MovRotLenDebug ) printf( "movrotlen NG\n" ) ;
#endif
			return c ;
		}
    } else {
		DG_COPY_VEC( &from, mov ) ;
    }
    vec.vx = vec.vy = 0.0F ; vec.vz = len ;
    DG_RotVector( &vec, &vec, 1 ) ;
    _sceVu0AddVector( &to, &from, &vec ) ;

#ifdef DEBUG_MODE
	if ( MovRotLenDebug ) {
		ViewFromTo( &from, &to, 32, 232, 32 ) ;
	}
#endif

	DG_COPY_VEC( &MovRotLenChkFrom, &from ) ;
	DG_COPY_VEC( &MovRotLenChkTo, &to ) ;

    c = HZX_OnlineHazardCheck( hzx_id, &from, &to, chk, flag, fflag ) ;

	if ( c ) {
		ResultHzxPtr = &ResultHzx ;
		HZX_GetOnlineHazard( ResultHzxPtr, &ResultAtr ) ;
#ifdef DEBUG_MODE
		if ( MovRotLenDebug ) {
			if ( ResultHzx.type == HZX_TYPE_SEGMENT ) HZX_ViewSegment( ResultHzxPtr ) ;
		}
#endif
		HZX_GetOnlinePoint( &ResultPoint ) ;
		{
			FVECTOR		v ;
			
			HZX_GetOnlineVector( &v ) ;
			GM_PlayerWork->work_l.result_len = GV_VecLen3F( &v ) ;
		}
	}
    return c ;
}

/* 最も近い敵兵までの距離 */
float				PL_NearestEnemyLen( void )
{
	HOMING_TRG		*hom ;
	float			len, minlen ;

	minlen = 1000000.0F ;
	hom = GM_GetHoming() ;
	while( hom != NULL ) {
		if ( ( hom->status & HOMING_ENEMY ) &&
			 !( hom->status & HOMING_UNREAL ) ) {
			len = GV_VecLen3F2( ( FVECTOR * )hom->world->m[ 3 ], &GM_PlayerPosition ) ;
			if ( len < minlen ) minlen = len ;
		}
		hom = hom->next ;
	}	
	return minlen ;
}

/* 次の攻撃がヒットしたときのＳＥを指定する */
void				PL_SetHitSE( int se )
{
	GM_PlayerWork->work_l.hit_se = se ;
}

#define	IntoSubject( _w )		PL_IntoSubject( _w )
#define	LeaveSubject( _w )		PL_LeaveSubject( _w )
#define	IntoIntrude( _w )		PL_IntoIntrude( _w )
#define	LeaveIntrude( _w )		PL_LeaveIntrude( _w )
#define	LeaveCaution( _w )		PL_LeaveCaution( _w )
#define	SetInvincible( _w, _t )		PL_SetInvincible( _w, _t )
#define	UnsetInvincible( _w )		PL_UnsetInvincible( _w )
#define	SendMessage( _t, _m, _l )	PL_SendMessage( _t, _m, _l )
#define	ClearCaptureTarget( _w )	PL_ClearCaptureTarget( _w ) 
#define	GetModelLength( _w, _f, _b )	PL_GetModelLength( _w, _f, _b )	
#define	StrongestUDLRValue( _p )	PL_StrongestUDLRValue( _p )	
#define	ChangeStance( _w, _s )		PL_ChangeStance( _w, _s )
#define	SetMotionSet( _w )			PL_SetMotionSet( _w )
#define	NearestEnemyLen()			PL_NearestEnemyLen()

static	int		CheckMovRotLenSegment( HZX_GROUP_ID hzx_id, FVECTOR *mov, FVECTOR *shift,
									   SVECTOR *rot, float len, int chk, int flag, int fflag ) {
	return PL_CheckMovRotLenSegment( hzx_id, mov, shift, rot, len, chk, flag, fflag ) ;
}

/*----------------------------------------------------------
  
  ローカル
  
  ----------------------------------------------------------*/

/* カメラセット */
static	void	SetCamera( work )
Work		*work ;
{
    FVECTOR	trg ;

    trg = work->control.mov ;
    if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) 
		_sceVu0AddVector( &trg, &trg, &work->root_diff ) ;
    if ( !Flag( FLAG_CAMERA_ON_WAIST ) ) {
		trg.vy = work->body.objs->objs[ HEAD_UNIT ].world.m[ 3 ][ 1 ] ;
    }
    work->camera = trg ;
}

static	void	SetRotAdjust( Work *, SVECTOR *, int ) ;
static 	void 	PL_AdjustXYZ( Work *work, FVECTOR *shift ) ;
static void CheckSubjectMoveCamera( Work *work ) ;

//BP_CAMERA - override 3rd person
int BP_3rdPersonCamera_GetAnalog( float d, float s )
{
   d -= 128.0f;
   if(d < -ANALOG_MARGIN_F)
   {
      return (int)((d + ANALOG_MARGIN_F) * s);
   }
   else if(d > ANALOG_MARGIN_F)
   {
      return (int)((d - ANALOG_MARGIN_F) * s);
   }
   else
   {
      return 0;
   }
}
//BP_CAMERA - override 3rd person

static	inline	void	PutCamera( work )
Work		*work ;
{
	//    GV_NearExp4VF( &GM_CameraTarget, &( work->camera ), 3 ) ;
    /* 主観腕モデルの位置設定 */
//    ArmBodyPosition = work->control.mov ;
    if ( Status( PLAYER_WATCH ) ) {
		if ( Flag( FLAG_PEEPING ) ) {
			FVECTOR		diff ;
			/* 主観腕の位置を計算 */
			_sceVu0SubVector( &diff, &work->peep, &work->camera ) ;
			//_sceVu0AddVector( &ArmBodyPosition, &ArmBodyPosition, &diff ) ;
						/* 本体位置移動 */
			if ( !Flag( FLAG_PEEPING_UP ) ) {
				float		levels[ 2 ] ;
				/* 覗きこみ位置の床高さが元位置の高さと大きく異なるときは
				   モデル移動しない */
				if ( HZX_LevelHazardCheck( work->control.hzx_id, &work->peep,
										   HZX_CHK_ALL, HZX_FLOOR_NO_PLAYER ) & 1 ) {
					HZX_GetLevelHeight( levels ) ;
				} else {
					levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
				}
				diff.vy = 0 ;
				if ( DG_FABS( work->control.levels[ 0 ] - levels[ 0 ] ) < 999.0F ) {
					DG_COPY_VEC( &work->peep_diff, &diff ) ;
				}
				PL_AdjustXYZ( work, &work->peep_diff ) ;
				DG_COPY_VEC( &PL_ObjShift, &diff ) ;
				PL_ObjShift.vw = 1.0F ;
				SetFlag2( FLAG2_OBJ_SHIFT ) ;
			}
			_sceVu0CopyVector( &work->camera, &work->peep ) ;
			/* ターゲットも移動 */
			work->def.center.vx = work->camera.vx ;
			work->def.center.vz = work->camera.vz ;
			if ( Flag( FLAG_PEEPING_UP ) ) work->def.center.vy = work->camera.vy ;
		} else {
			DG_COPY_VEC( &work->peep_diff, &DG_ZeroVector ) ;
		}
		/* プレイヤー発見位置移動 */
		if ( !Flag( FLAG_FINDPOS_IS_WAIST ) ) {
			//_sceVu0CopyVector( &GM_PlayerFindPos, &( work->camera ) ) ;
            GM_PlayerFindPos.vx = work->camera.vx ;
            GM_PlayerFindPos.vz = work->camera.vz ;
            if ( Flag( FLAG_PEEPING_UP ) || Status( PLAYER_HOLD ) ) {
				GM_PlayerFindPos.vy = work->camera.vy ;
			}
		} 
		/* 影用処理 */
		if ( CheckInvisible( work ) ) {
			SVECTOR			rot ;

			rot = work->camdir ;
			rot.vx -= work->g_rot ;
			rot.vy = 0 ;
			if ( Flag( FLAG_SUBJECT_ARM_ADJUST ) ) {
				SetRotAdjust( work, &rot, HUMAN21_ATAMA ) ;
				SetRotAdjust( work, &rot, HUMAN21_MIGI_UDE1 ) ;
				SetRotAdjust( work, &rot, HUMAN21_HIDARI_UDE1 ) ;
			} else {
				SetRotAdjust( work, &rot, HUMAN21_ATAMA ) ;
			}
		}
    } else {
		DG_COPY_VEC( &work->peep_diff, &DG_ZeroVector ) ;
	}
    _sceVu0CopyVector( &GM_CameraTarget, &( work->camera ) ) ;
#ifndef CAMERA_TURN
    GM_CameraDir = work->control.rot ;
#else
    GM_CameraDir.vx = GV_NearExp4P( GM_CameraDir.vx, work->camdir.vx ) ;
    if ( Flag( FLAG_SUBJECT_HORIZON_LIMIT ) ) {
		GM_CameraDir.vy = 
			GV_NearExp4P( GM_CameraDir.vy, work->camdir.vy ) ;
    } else {
		GM_CameraDir.vy = work->camdir.vy = work->control.rot.vy ;
    }
    GM_CameraDir.vz = 0 ;
#endif

	/* 死体上＆匍匐主観のとき */
	if ( Status( PLAYER_ON_CORPSE ) && 
		 Status( PLAYER_WATCH     ) && 
		!Status( PLAYER_INTRUDE   ) && 
		 work->stance == GROUND ) {
		GM_CameraTarget.vy += 250.0F ;
	}

	{
		FVECTOR		pos ;
		SVECTOR		rot ;

		DG_COPY_VEC( &pos, &GM_CameraTarget ) ;
		if ( Status( PLAYER_WATCH ) && 
			 Status( PLAYER_INTRUDE ) &&
			 work->action2 == NULL &&
			 Flag( FLAG_SUBJECTCAMERA_ADJUST ) ) {
			FVECTOR		aim, vec ;
			float		len ;

			rot = GM_CameraDir ;
			rot.vz = 0 ;
			DG_SetPos2( &pos, &rot ) ;
			DG_PutVector( &PL_SubjectCameraShift, &aim, 1 ) ;
			if ( HZX_OnlineHazardCheck( work->control.hzx_id, &pos, &aim, 
									    HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, 
									    HZX_FLOOR_RECOIL_TYPE ) ) {
				HZX_GetOnlineVector( &vec ) ;
				len = GV_VecLen3F( &vec ) ;
				if ( len > 100.0F ) {
					GV_LenVec3F( &vec, &vec, 0.0F, len - 100.0F ) ;
					_sceVu0AddVector( &pos, &pos, &vec ) ;
				} else {
					DG_COPY_VEC( &pos, &pos ) ;
				}
			} else {
				DG_COPY_VEC( &pos, &aim ) ;
			}
		}
		DG_COPY_VEC( &work->subject_camera->position, &pos ) ;
	}

	/*
	  主観移動のカメラ関係はここにまとめてありまする
	  */
	if ( PL_SubjectMove ) {
		CheckSubjectMoveCamera( work ) ;
	}


	/*
	  イントルードから立ち上がる時 主観の時に壁にメリコマナイ
	 */
	if ( (Status(PLAYER_WATCH) || PL_SubjectMove) &&
		 work->action==GroundToStand ) {
	    FVECTOR v ;

	    _sceVu0ScaleVector( &v, (FVECTOR*)DG_Chanl(0)->eye.m[2], 600.0f ) ;
	    _sceVu0SubVector( &work->subject_camera->position,
			      &work->subject_camera->position,
			      &v ) ;
	} 

   //BP_CAMERA - override 3rd person
   if(gBP_3rdPersonCamera_Override)
   {
      // Compute target
      BP_Vec3_Copy( &gBP_3rdPersonCamera_Target, &work->camera );

      // Compute eye position exactly like the function "GM_CameraInterpOutSubject" so in/out of 1st/3rd person works correctly
      GM_CameraMakeCamera( &gBP_3rdPersonCamera_Eye, &gBP_3rdPersonCamera_Target, &gBP_3rdPersonCamera_Rot, &gBP_3rdPersonCamera_Dist ) ;

      // Perform basic collision test - really need swept sphere or swept aabbox test
      if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &gBP_3rdPersonCamera_Target, &gBP_3rdPersonCamera_Eye,
         HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ) 
      {
         FVECTOR		v ;
         HZX_GetOnlineVector( &v ) ;
         BP_Vec3_AddVec( &gBP_3rdPersonCamera_Eye, &gBP_3rdPersonCamera_Target, &v );
      }

      // Update rotation
      if(work->pad->analog_input & GV_PAD_ANALOG_R_USE)
      {
         gBP_3rdPersonCamera_Rot.vx += BP_3rdPersonCamera_GetAnalog(work->pad->right_dy, gBP_3rdPersonCamera_VSpeed);   //pitch
         gBP_3rdPersonCamera_Rot.vy -= BP_3rdPersonCamera_GetAnalog(work->pad->right_dx, gBP_3rdPersonCamera_HSpeed);   //yaw
      }

      // Range check
      gBP_3rdPersonCamera_Rot.vx = BP_NormalizeIntegerRot( gBP_3rdPersonCamera_Rot.vx );  //pitch
      gBP_3rdPersonCamera_Rot.vy = BP_NormalizeIntegerRot( gBP_3rdPersonCamera_Rot.vy );  //yaw
      if( gBP_3rdPersonCamera_Rot.vx < -800 )
         gBP_3rdPersonCamera_Rot.vx = -800;
      else if( gBP_3rdPersonCamera_Rot.vx > 800 )
         gBP_3rdPersonCamera_Rot.vx = 800;
   }
   //BP_CAMERA - override 3rd person
}

/*
   主観移動のカメラ調整
   */
static void CheckSubjectMoveCamera( Work *work )
{
	FVECTOR v ;
	FMATRIX *eye ;
	FMATRIX m ;
	int     rot ;

	/* 体の向きマトリックス */
	rot = work->control.rot.vy & 4095 ;
	rot -= rot>2048 ? 4096 : 0 ;
	_sceVu0RotMatrixY( &m,
			   &DG_UnitMatrix,
			   rot*M_PI/2048.0f ) ;

	/*画角調整*/
	if ( !(WeaponType( work ) & WP_TYPE_SUBJECT) &&
		 !(ItemType( work )   & IT_TYPE_SUBJECT) ) {
		GM_CameraSet *cam;
		cam = GM_GetCurrentCameraSet( 0 ); /* カメラの取得*/
		cam->angle = PL_SubjectAngle ;
	} 

	/*匍匐は、ちょっとカメラをZ方向に下げる*/
	eye = &DG_Chanl(0)->eye ;
	if ( Status(PLAYER_ENEMY_HANG) ) {
		/* 首締めのカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 100.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 185.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
	} else if ( (work->action==IntrudeMoveFront) ) {
		/* イントルードのカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 200.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;

	} else if ( (work->action==IntrudeMoveBack ) ) {
		/* イントルードのカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], -100.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;

	} else if ( (work->action==GroundStill    ) ) {

		/* 匍匐のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR *)m.m[2], 200.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
	} else if ( (work->action==NikitaStrike) ||
			    (work->action==Combo       ) ) {

		/* パンチのカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR *)m.m[2], 300.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;

	} else if ( work->action==Shrink ) {
		/* 匍匐のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR *)m.m[2], 400.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;

	} else if ( (work->action==GroundMoveFront) ||
			    (work->action==DownDamage     ) ) {
		/* 匍匐のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR *)m.m[2], 400.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;

	} else if ( work->action==StandStill ||	work->action==StandRun ) {
		/*立ちモーション時のカメラ調整 */
		GM_CameraTarget.vy = work->control.hzx_base + 1500.0f ;
		
	} else if ( work->action==SquatStill ) {
		/*立ちモーション時のカメラ調整 */
		GM_CameraTarget.vy = work->control.hzx_base + 1000.0f ;
		
	} else if ( work->action==GroundToStand ) {
		
		_sceVu0ScaleVector( &v, (FVECTOR*)DG_Chanl(0)->eye.m[2], 400.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		
	} else if ( work->action==NikitaStrike ) {
		/* ニキータストライクのカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 600.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 400.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[0], 400.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
	} else if ( work->action==CB_BoxStop ) {
		/* ダンボール関係のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 20.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 200.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
	} else if ( work->action==CB_BoxMove ) {
		/* ダンボール関係のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 25.0f ) ;
		_sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		if ( work->pad->status & PAD_U ) {
			_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 200.0f ) ;
			_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		}
	} else if ( work->action==CB_BoxStill ) {
		/* ダンボール関係のカメラ調整 */
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 50.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		_sceVu0ScaleVector( &v, (FVECTOR*)eye->m[2], 100.0f ) ;
		_sceVu0AddVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
	}
	
	/*階段の時 Yを上げる*/
	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
		if ( (work->action==DownDamage) || (work->action==Rise) ) {
		    _sceVu0ScaleVector( &v, (FVECTOR*)eye->m[1], 500.0f ) ;
		    _sceVu0SubVector( &GM_CameraTarget, &GM_CameraTarget, &v ) ;
		} else if ( Status(PLAYER_GROUND|PLAYER_DOWNED) ||
				   (work->action==Down       ) ||
				   (work->action==GroundStill) ||
				   (work->action==RunToSquat ) ) {
		    GM_CameraTarget.vy += 400.0f ;
		}
	}

	/* カメラ位置でＨＺＸ当たりを見る */
	if ( !(GM_VRStatus & GM_VR_IDLE) )
	{
		FVECTOR		v = { 0.0F, 0.0F, 0.0F } ;
		FVECTOR		vecs[ 2 ] ;
		HZX_HZD		segs[ 2 ] ;
		signed char	is_edge[ 2 ] ;
		int			flag, atrs[ 2 ] ;
		float       levels[2] ;
		float       height ;

		/* まずNEARチェック */
		flag = HZX_NearHazardCheck( work->control.hzx_id,
								   &GM_CameraTarget, 800.0f, 
								   HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
								   500.0f ) ;
		if ( flag > 0 ) {
			HZX_GetNearVector( vecs ) ;
			HZX_GetIsEdge( is_edge ) ;
			HZX_GetNearHazard( segs, atrs ) ;
			flag = GM_HzxCheckConflict( flag, &GM_CameraTarget, segs, vecs, is_edge ) ;
			GM_GetHazardReaction( &v, flag, vecs, 500 ) ;
		}

		/* 次にFloorチェックの底上げをどれにするか */
		height = (Status(PLAYER_ROLLING) ? 500.0f :
				  Status(PLAYER_DOWNED)  ? 600.0f :
				  Status(PLAYER_GROUND)  ? 75.0f  :
				  Status(PLAYER_INTRUDE) ? 100.0f : 400.0f) ;

		v.vx += GM_CameraTarget.vx ;
		v.vy  = GM_CameraTarget.vy + height ;
		v.vz += GM_CameraTarget.vz ;

		/* 次にFloorチェック */
		flag = HZX_LevelHazardCheck( work->control.hzx_id,
									&v,
								    HZX_CHK_ALL,
									HZX_FLOOR_NO_PLAYER|HZX_FLOOR_IK ) ;
		if ( flag ) {
			HZX_GetLevelHeight( levels ) ; 
			levels[0] += height ;
			levels[1] -= height ;
			if ( (flag & 1) && GM_CameraTarget.vy < levels[0] ){
				GM_CameraTarget.vy = levels[0] ;
			}
			if ( (flag & 2) && GM_CameraTarget.vy > levels[1] ){
				GM_CameraTarget.vy = levels[1] ;
			}
		}
		GM_CameraTarget.vx = v.vx ;
		GM_CameraTarget.vz = v.vz ;
	}
}


/* アナログ右パッドでカメラ注視点を移動 */
/* 移動ベクトルを第1引数で返す */
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

/* ビハインドカメラの注視点をアナログ右で動かす */
static	void	GetBehindCameraShiftforAnalogR( FVECTOR *shift, float *v, GV_PAD *pad )
{
	FVECTOR	  	pos, trg ;

#if 0
	if ( GM_PlayerWork->behind_camera != GM_GetNextCamera( 0 ) ||
		GM_Camera->chanl[ 0 ].time > 4 ) {
		DG_COPY_VEC( shift, &DG_ZeroVector ) ;
		return ;
	}
#else	
	if ( GM_PlayerWork->behind_time < BEHIND_IN_DELAY + DIRECT_TICK( 36 ) ) {
		DG_COPY_VEC( shift, &DG_ZeroVector ) ;
		return ;
	}
#endif
	trg.vx = v[ 0 ] ;
	trg.vy = v[ 1 ] ;
	trg.vz = v[ 2 ] ;
	pos.vx = v[ 3 ] ;
	pos.vy = v[ 4 ] ;
	pos.vz = v[ 5 ] ;
	GetCameraShiftforAnalogR( shift, &pos, &trg, pad ) ;
}

/* ビハインドカメラチェック */
static	void	CheckBehindCamera( work )
Work		*work ;
{
    HZX_BEHIND		*bhs[ 2 ], *bh ;
    int				n_bhs ;
    GM_CameraSet	*cam ;
	FVECTOR			sft ;
	FVECTOR			root ;

    if ( Flag( FLAG_BEHIND_PEEP ) ) {
		bh = BehindPtr ;
		if ( bh != NULL ) {
			work->behind_time ++ ;
			GetBehindCameraShiftforAnalogR( &sft, &bh->v[ 0 ], work->pad ) ;
			cam = work->behind_camera ;	
#if 0
			cam->target.vx = bh->v[ 0 ] + sft.vx ;
			cam->target.vy = bh->v[ 1 ] + sft.vy ;
			cam->target.vz = bh->v[ 2 ] + sft.vz ;
#else
			cam->target.vx = bh->v[ 0 ] ;
			cam->target.vy = bh->v[ 1 ] ;
			cam->target.vz = bh->v[ 2 ] ;
#endif
			cam->position.vx = bh->v[ 3 ] ;
			cam->position.vy = bh->v[ 4 ] ;
			cam->position.vz = bh->v[ 5 ] ;
		}
		return ;
    }
    BehindPtr = NULL ;
    if ( Status( PLAYER_CAUTION ) ) {
		//	 GM_AlertMode != ALERT_MODE_ALERT ) { /* 危険モード */
		if ( ++ work->behind_camera_enable == BEHIND_IN_DELAY ) {
			SetStatus( PLAYER_BEHIND_CAMERA_ENABLE ) ;
		}
		/* トラップチェック位置と同じ場所でチェック */
		if ( !Flag2( FLAG2_TRPCHECK_NOROOT ) ) {
			_sceVu0AddVector( &root, &work->control.mov, &work->root_diff ) ;
			n_bhs = HZX_CheckBehind( work->control.hzx_id,
									 bhs, &root,	&( work->control.rot ) ) ;
		} else {
			n_bhs = HZX_CheckBehind( work->control.hzx_id,
									 bhs, &work->control.mov, &( work->control.rot ) ) ;
		}
		if ( n_bhs > 0 ) {
			if ( n_bhs == 1 ) {
				bh = bhs[ 0 ] ;
				BehindPtr = ( HZX_BEHIND * )bh ;
			} else {
				FVECTOR		c, d ;
				int		dir1, dir2 ;
				c.vx = ( bhs[ 0 ]->b1.vx + bhs[ 0 ]->b2.vx ) / 2.0F ;
				c.vy = 0.0F ;
				c.vz = ( bhs[ 0 ]->b1.vz + bhs[ 0 ]->b2.vz ) / 2.0F ;
				_sceVu0SubVector( &d, &c, &( work->control.mov ) ) ;
				dir1 = GV_VecDir2( &d ) ;
				c.vx = ( bhs[ 1 ]->b1.vx + bhs[ 1 ]->b2.vx ) / 2.0F ;
				c.vy = 0.0F ;
				c.vz = ( bhs[ 1 ]->b1.vz + bhs[ 1 ]->b2.vz ) / 2.0F ;
				_sceVu0SubVector( &d, &c, &( work->control.mov ) ) ;
				dir2 = GV_VecDir2( &d ) ;
				dir1 = GV_DiffDirAbs( dir1, work->control.rot.vy ) ;
				dir2 = GV_DiffDirAbs( dir2, work->control.rot.vy ) ;
				bh = ( dir1 < dir2 ) ? bhs[ 1 ] : bhs[ 0 ] ;
				BehindPtr = bh ;
			}
			cam = work->behind_camera ;
			if ( ++ work->behind_time >= BEHIND_IN_DELAY ||
				 Status( PLAYER_BEHIND_CAMERA_ENABLE ) ) {
				if ( cam->on == 0 ) {
					SetStatus( PLAYER_BEHIND ) ;
#ifdef DEBUG_MODE
					if ( GM_PlayerDebugMode == GM_PDM_NOBEHIND ) return ;
#endif	

#ifdef DEBUG_MODE
					switch( PlayerDebugBehindType ) {
					case 0 :
					default :
						if ( bh->flag & HZX_BEHIND_CAMERA_TYPE2 ) {
							GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
												    GM_CAM_INTERP_INTO_BEHIND2, 0, 0 ) ;
						} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE3 ) {
							GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK,
												    0, 0 ) ;
						} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE4 ) {
							GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_QUICK,
												    0, 0 ) ;						
						} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE5 ) {
							GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
												    GM_CAM_INTERP_QUICK, 0, 0 ) ;						
						} else {
							GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_BEHIND,
												    0, 0 ) ;
						}
						break ;
					case 1 :
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
											    GM_CAM_INTERP_INTO_BEHIND2, 0, 0 ) ;
						break ;
					case 2 :
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK,
											    0, 0 ) ;						
						break ;
					case 3 :
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_QUICK,
											    0, 0 ) ;						
						break ;
					case 4 :
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
											    GM_CAM_INTERP_QUICK, 0, 0 ) ;								  
						break ;
					}
#else
					if ( bh->flag & HZX_BEHIND_CAMERA_TYPE2 ) {
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
											    GM_CAM_INTERP_INTO_BEHIND2, 0, 0 ) ;
					} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE3 ) {
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK, GM_CAM_INTERP_QUICK,
											   0, 0 ) ;
					} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE4 ) {
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_QUICK,
											   0, 0 ) ;						
					} else if ( bh->flag & HZX_BEHIND_CAMERA_TYPE5 ) {
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_BEHIND2, 
											   GM_CAM_INTERP_QUICK, 0, 0 ) ;						
					} else {
						GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_BEHIND,
											   0, 0 ) ;
					}
#endif
					GM_ChangeCamera( work->behind_camera->chanl ) ;
					cam->on = 1 ;
				}
			}
#ifdef DEBUG_MODE
			if ( GM_PlayerDebugMode == GM_PDM_NOBEHIND ) return ;
#endif
			GV_NearExp16VF( &work->behindcam_add, &DG_ZeroVector, 3 ) ;
			GV_NearExp16VF( &work->behindtrg_add, &DG_ZeroVector, 3 ) ;

			GetBehindCameraShiftforAnalogR( &sft, &bh->v[ 0 ], work->pad ) ;

			cam->target.vx = bh->v[ 0 ] + work->behindtrg_add.vx + sft.vx ;
			cam->target.vy = bh->v[ 1 ] + work->behindtrg_add.vy + sft.vy ;
			cam->target.vz = bh->v[ 2 ] + work->behindtrg_add.vz + sft.vz ;
			cam->position.vx = bh->v[ 3 ] + work->behindcam_add.vx ;
			cam->position.vy = bh->v[ 4 ] + work->behindcam_add.vy ;
			cam->position.vz = bh->v[ 5 ] + work->behindcam_add.vz ;
			return ;
		}
    } 
    work->behind_time = 0 ;
    if ( Status( PLAYER_BEHIND ) ) {
		UnsetStatus( PLAYER_BEHIND ) ;
		work->behind_camera->on = 0 ;
		GM_ChangeCamera( work->behind_camera->chanl ) ;
		work->behindtrg_add = DG_ZeroVector ;
		work->behindcam_add = DG_ZeroVector ;
    }
}

/* ビハインドカメラシフトチェック */
/* ちゃんと設定されているか */
static	int	CheckBehindCameraShift( shift )
int	shift ;
{
    HZX_BEHIND	*bh ;

    bh = BehindPtr ;
    if ( BehindPtr != NULL ) {
		if ( shift == BEHIND_LEFT ) {
			if ( bh->left[ 0 ] == 0.0F &&	
				bh->left[ 1 ] == 0.0F &&
				bh->left[ 2 ] == 0.0F ) return 0 ;	    
		} else {
			if ( bh->right[ 0 ] == 0.0F &&	
				bh->right[ 1 ] == 0.0F &&
				bh->right[ 2 ] == 0.0F ) return 0 ;	    
		}
		return 1 ;
    }
    return 0 ;
}

/* ビハインドカメラシフト */
static	void	ShiftBehindCamera( work, shift )
Work		*work ;
int		shift ;
{
    GM_CameraSet	*cam ;
    FVECTOR		diff, diff2, sft ;
    SVECTOR		dir ;
    float		len ;
#ifdef DEBUG_MODE
    if ( GM_PlayerDebugMode == GM_PDM_NOBEHIND ) return ;
#endif
    if ( !CheckBehindCameraShift( shift ) ) return ;

	/* ビハインドカメラに補完しきっていない */
#if 0
	if ( GM_PlayerWork->behind_camera != GM_GetNextCamera( 0 ) ||
		GM_Camera->chanl[ 0 ].time > 0 ) return ;
#else
	if ( work->behind_time < BEHIND_IN_DELAY + DIRECT_TICK( 36 ) ) {
		return ;
	}
#endif

    cam = work->behind_camera ;    
    _sceVu0SubVector( &diff, &cam->position, &cam->target ) ;
    diff.vy = 0.0F ;
    len = GV_VecLen3F( &diff ) ;
    dir.vy = GV_VecDir2( &diff ) ;
    dir.vx = dir.vz = 0 ;

    if ( BehindPtr != NULL ) {
		HZX_BEHIND	*bh ;
		float		v[ 6 ] ;

		bh = BehindPtr ;
		if ( shift == BEHIND_LEFT ) {    
#if 0
			GetBehindCameraShiftforAnalogR( &sft, &bh->left[ 0 ], work->pad ) ;
			diff.vx = bh->left[ 0 ] - cam->target.vx + sft.vx ;
			diff.vy = bh->left[ 1 ] - cam->target.vy + sft.vy ;
			diff.vz = bh->left[ 2 ] - cam->target.vz + sft.vz ;
#else
			diff.vx = bh->left[ 0 ] - cam->target.vx ;
			diff.vy = bh->left[ 1 ] - cam->target.vy ;
			diff.vz = bh->left[ 2 ] - cam->target.vz ;
#endif
			diff2.vx = bh->left[ 3 ] - cam->position.vx ;
			diff2.vy = bh->left[ 4 ] - cam->position.vy ;
			diff2.vz = bh->left[ 5 ] - cam->position.vz ;
		} else if ( shift == BEHIND_RIGHT ) {
#if 0
			GetBehindCameraShiftforAnalogR( &sft, &bh->right[ 0 ], work->pad ) ;
			diff.vx = bh->right[ 0 ] - cam->target.vx + sft.vx ;
			diff.vy = bh->right[ 1 ] - cam->target.vy + sft.vy ;
			diff.vz = bh->right[ 2 ] - cam->target.vz + sft.vz ;
#else
			diff.vx = bh->right[ 0 ] - cam->target.vx ;
			diff.vy = bh->right[ 1 ] - cam->target.vy ;
			diff.vz = bh->right[ 2 ] - cam->target.vz ;
#endif
			diff2.vx = bh->right[ 3 ] - cam->position.vx ;
			diff2.vy = bh->right[ 4 ] - cam->position.vy ;
			diff2.vz = bh->right[ 5 ] - cam->position.vz ;
		}
		GV_NearExp16VF( &work->behindtrg_add, &diff, 3 ) ;
		cam->target.vx += work->behindtrg_add.vx ;
		cam->target.vy += work->behindtrg_add.vy ;
		cam->target.vz += work->behindtrg_add.vz ;

		GV_NearExp16VF( &work->behindcam_add, &diff2, 3 ) ;
		cam->position.vx += work->behindcam_add.vx ;
		cam->position.vy += work->behindcam_add.vy ;
		cam->position.vz += work->behindcam_add.vz ;

		v[ 0 ] = cam->target.vx ;
		v[ 1 ] = cam->target.vy ;
		v[ 2 ] = cam->target.vz ;
		v[ 3 ] = cam->position.vx ;
		v[ 4 ] = cam->position.vy ;
		v[ 5 ] = cam->position.vz ;
		GetBehindCameraShiftforAnalogR( &sft, v, work->pad ) ;	   
		_sceVu0AddVector( &cam->target, &cam->target, &sft ) ;
    }
}

static	void	ShiftBehindCameraReturn( work )
Work		*work ;
{
    GM_CameraSet	*cam ;
#ifdef DEBUG_MODE
    if ( GM_PlayerDebugMode == GM_PDM_NOBEHIND ) return ;
#endif
    cam = work->behind_camera ;    
    GV_NearExp16VF( &work->behindtrg_add, &DG_ZeroVector, 3 ) ;
    cam->target.vx += work->behindtrg_add.vx ;
    cam->target.vy += work->behindtrg_add.vy ;
    cam->target.vz += work->behindtrg_add.vz ;
    GV_NearExp16VF( &work->behindcam_add, &DG_ZeroVector, 3 ) ;
    cam->position.vx += work->behindcam_add.vx ;
    cam->position.vy += work->behindcam_add.vy ;
    cam->position.vz += work->behindcam_add.vz ;
}

/*---------------------------------------------------------------------*/

/* フラグによってはステップ値を変更 */
static	inline	void	CheckStep( work )
Work		*work ;
{
    if ( Flag( FLAG_NO_STEP ) ) {
		work->control.step.vx = 0.0F ;
		work->control.step.vz = 0.0F ;
	} else if ( Status( PLAYER_WATCH ) && !Flag2( FLAG2_SUBJECT_MOVE ) ) {
		if ( !Status( PLAYER_BEYOND ) ) {	/* エルード中はＯＫ */
			work->control.step.vx = 0.0F ;
			work->control.step.vz = 0.0F ;
		}
    } else if ( Flag( FLAG_NO_MOTION_STEP_XZ ) ) {
		work->control.step.vx -= work->body.m_ctrl->step.vx ;
		work->control.step.vz -= work->body.m_ctrl->step.vz ;
    }
}

/*---------------------------------------------------------------------*/

/* 覗き込みボタンＯＫ */
static	int		CheckPeepButtonEnable( Work *work, int dir )
{
   int pad_dir = (work->camdir.vy - work->pad->dir);
      
   if (pad_dir < 0)
      pad_dir = -pad_dir;

   if (!(GM_PlayerStatus & PLAYER_MOVE))
   {
      if (pad_dir > 2048)
      {
         if ((pad_dir - 2048) < 128)
            work->allow_analog_peep = 1;
      }
      else
      {
         if ((2048 - pad_dir) < 128)
            work->allow_analog_peep = 1;
      }
   }
   else
   {
      work->allow_analog_peep = 0;
   }

#if defined(BP_VITA)
   work->pad->press |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Front, PL_PadGetGestureRelease(work));
   work->pad->status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Front, PL_PadGetGestureRelease(work));
#endif

   if (pad_dir > gArm_PeekLAnalogMinValue && pad_dir < gArm_PeekLAnalogMaxValue && work->allow_analog_peep)
   {
      work->enable_analog_peep_l = 1;
   }
   else
   {
      work->enable_analog_peep_l = 0;
   }

   if (pad_dir > gArm_PeekRAnalogMinValue && pad_dir < gArm_PeekRAnalogMaxValue && work->allow_analog_peep)
   {
      work->enable_analog_peep_r = 1;
   }
   else
   {
      work->enable_analog_peep_r = 0;
   }

	if ( Flag( FLAG_NEED_NEWPRESS_PEEP ) )
   {
      if ( work->pad->press & ( PL_PAD_PEEP_R | PL_PAD_PEEP_L ))
      {
			UnsetFlag( FLAG_NEED_NEWPRESS_PEEP ) ;
		}
      else 
      {
         if (work->enable_analog_peep_l || work->enable_analog_peep_r)
         {
            UnsetFlag( FLAG_NEED_NEWPRESS_PEEP );
         }
         else
         {
            return 0;
         }
      }
	}
   
	if ( dir == BEHIND_LEFT )
   {
      if ( (( work->pad->status & PL_PAD_PEEP_L) 
         && ( GV_Time - work->behind_peep_lastpressL >= GM_MENU_QUICK_TIME )) || work->enable_analog_peep_l)
      {
			return 1 ;
		}
	}
   else
   {
      if ( (( work->pad->status & PL_PAD_PEEP_R )
         && ( GV_Time - work->behind_peep_lastpressR >= GM_MENU_QUICK_TIME )) || work->enable_analog_peep_r)
      {
			return 1 ;
		}
	}
	return 0 ;
}

/* 壁方向、はりつき条件をチェック */
static	void	CheckDirection( work )
Work		*work ;
{	
    int		n_touches, diff, diff2, to ;
	int		edge ;

	if ( work->pad->press & PL_PAD_PEEP_R ) work->behind_peep_lastpressR = GV_Time ;
	if ( work->pad->press & PL_PAD_PEEP_L ) work->behind_peep_lastpressL = GV_Time ;

    Wall = NULL ;
    WallTo = -1 ;
    Liable = NEUTRAL ;

    // On Vita we do not use the directional pad for movement so only set the PadTo variable if using the analog stick.
    if (!gBP_UseVitaControlOverrides || (GM_GameStatus & STATE_PAD_DEMO))
    {
      PadTo = work->pad->dir ;
    }
    else
    {
       if (!work->pad->analog_input)
       {
          if (!work->pad->pressure[PAD_PRESS_U] && !work->pad->pressure[PAD_PRESS_D] && 
             !work->pad->pressure[PAD_PRESS_L] && !work->pad->pressure[PAD_PRESS_R])
          {
            PadTo = -1;
          }
       }
       else
       {
          PadTo = work->pad->dir;
       }
    }
    
	PadForce = PL_StrongestUDLRValue( work->pad ) ;
	//printf( "padto %d\n", PadTo ) ;
    n_touches = work->control.n_touches ;
    if ( n_touches == 0 || PadTo == -1 ) return ;
    /* しゃがみ特殊 */
    if ( work->stance == SQUAT && 
		work->control.is_edge[ 0 ] == 1 ) return ;
    Wall = work->control.segs[ 0 ] ;
    WallTo = GV_VecDir2( work->control.vecs ) ;
    WallVec = work->control.vecs ;
    WallAtr = work->control.atrs[ 0 ] ;
	edge = work->control.is_edge[ 0 ] ;
    if ( n_touches == 2 && work->control.is_edge[ 1 ] == 0 &&
		 !( work->control.atrs[ 1 ] & HZX_SEG_NO_HARITSUKI ) ) {
		to = GV_VecDir2( work->control.vecs + 1 ) ;
		diff = GV_DiffDirAbs( PadTo, WallTo ) ;
		diff2 = GV_DiffDirAbs( PadTo, to ) ;
		if ( abs( diff - diff2 ) < 4 ) {
			/* 今の自分の向きに近い方を選択 */
			diff = GV_DiffDirAbs( work->control.rot.vy + 2048, WallTo ) ;
			diff2 = GV_DiffDirAbs( work->control.rot.vy + 2048, to ) ;
		} 
		if ( diff2 < diff ) { 
			WallTo = to ;
			Wall = work->control.segs[ 1 ] ;
			WallVec = work->control.vecs + 1 ;
			WallAtr = work->control.atrs[ 1 ] ;
			edge = work->control.is_edge[ 1 ] ;
		}
    }
	if ( edge ) return ;
    if ( ( WallAtr & HZX_SEG_NO_HARITSUKI ) ||	
		Flag( FLAG_NO_CAUTION ) ) return ;
    diff = GV_DiffDirS( PadTo, WallTo ) ;
    if ( diff > 1024 - LIE_ANGLE_MARGIN ||
		diff < -1024 + LIE_ANGLE_MARGIN ) return ;
    Liable = FRONT ;
    if ( diff >= 0 ) {
		if ( diff > 384 ) Liable = LEFT ;
    } else {
		if ( diff < -384 ) Liable = RIGHT ;
    }	
	if ( Liable == FRONT ) 
   {
		if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) Liable = RIGHT ;
		else if  ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) Liable = LEFT ;
	}
}

static	int	ReactVector( to, dir )
int		to, dir ;
{
    int		diff ;

    if ( dir >= 0 ) {
		diff = 4095 & ( to - dir ) ;
		if ( diff < 1024 ) to = dir + 1024 ;
		if ( diff > 3072 ) to = dir - 1024 ;	
    }
    return to ;
}

/* かべにそった方向を返す */
static	inline	int	ReactWall( to )
int		to ;
{
	if ( WallAtr & HZX_SEG_NO_HARITSUKI ) return to ;
    to = ReactVector( to, WallTo ) ;
    return to ;
}

/* かべにそった方向を返す（グローバル） */
int		PL_ReactWall( int to )
{
	return ReactWall( to ) ;
}

/* 傾く */
static	inline	void	Incline( Work *work )
{
    int		incline ;

    if ( PadTo < 0 ) {
		work->control.turn.vz = 0 ;
		return ;
    }
    incline = GV_DiffDirS( work->control.turn.vy, work->control.rot.vy ) ;
    if ( incline > 128 ) incline = 128 ;
    else if ( incline < -128 ) incline = -128 ;
    work->control.turn.vz = incline ;    
}

static	void	GroundIK2( Work *work, float height ) ;
static	int		CheckGroundWidth( Work *, SVECTOR * ) ;
static	int		GetGRotEx( Work *work, float z, int ry ) ;

/* 匍匐時、回転限界制御、カメラ高さ制御 */
/* work->idata使用 */
static	void	GroundRotateLimitControl( Work *work )
{
	int			n ;
	SVECTOR		rot ;

	if ( !Status( PLAYER_GROUND ) ) return ;
	if ( !Status( PLAYER_INTRUDE ) ) {
		work->camera.vy += 250.0F ;		
		GroundIK2( work, 500.0F ) ;
		rot = work->control.turn ;
		n = CheckGroundWidth( work, &rot ) ; 
		work->idata = work->control.rot.vy ;
		if ( n ) {
			work->control.turn.vy = work->control.rot.vy = work->idata ;
		} 
	} else {
		work->g_rot = GetGRotEx( work, 750.0F, work->control.rot.vy ) ;
		work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_STILL ;	
	}
}

/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

/* 武器カメラをセット */
static	void	SetWeaponCamera( Work *work )
{
#if 0
    GM_CameraSet	*cam ;

    cam = work->weapon_camera ;
    if ( cam->on == 0 && Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
#ifndef NO_SUBJECT
		GM_ChangeCamera( work->weapon_camera->chanl ) ;
		cam->on = 1 ;
#endif
    }
#endif
	SetFlag( FLAG_WEAPON_CAMERA_ON ) ;
	work->vwait2 = 0 ;
}

/* 武器カメラＯＦＦ */
static	inline	void	UnsetWeaponCamera( Work *work )
{
    if ( work->vwait2 <= 0 ) work->vwait2 = 4 ;
}

static	inline	void	UnsetWeaponCameraQuick( Work *work )
{
    work->vwait2 = 1 ;
}

static	inline	void	CheckWeaponCamera( Work *work )
{
    if ( work->vwait2 != 0 ) {
		if ( -- work->vwait2 == 0 ) {
			UnsetFlag( FLAG_WEAPON_CAMERA_ON ) ;
		}
    }
	if ( Flag( FLAG_WEAPON_CAMERA_ON ) ) {
		if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			if ( work->weapon_camera->on == 0 ) {
				GM_ChangeCamera( work->weapon_camera->chanl ) ;
			}			
			work->weapon_camera->on = 1 ;
		} else {
			if ( work->weapon_camera->on ) {
				GM_ChangeCamera( work->weapon_camera->chanl ) ;
			}
			work->weapon_camera->on = 0 ;
		}
	} else {
		if ( work->weapon_camera->on ) {
			GM_ChangeCamera( work->weapon_camera->chanl ) ;
		}
		work->weapon_camera->on = 0 ;
	}
}

/*---------------------------------------------------------------------*/

/* 主観可視制御 */
static	void	CheckVWait2( Work *work )
{
    float		len, chklen ; 
    GM_CameraSet	*cam1, *cam2 ;
	int				c ;

    /* 主観移動の腕の可視不可視 */
    if ( PL_SubjectMove ) { 
	    work->weapon_camera->on = 1 ; /* 主観移動では武器カメラだけでいく*/
	if ( GM_VRStatus & GM_VR_CLEAR ||
		 GM_VRStatus & GM_VR_IDLE  ||
	     (work->action2==NULL  &&
		  work->action !=Combo &&
	      ((WeaponType( work ) & WP_TYPE_GRENADE) ||/* 苦肉の策 */
	       (PL_CurrentItem()==IT_Scope          ) ||
	       (PL_CurrentItem()==IT_Camera         ) ||
	       (PL_CurrentItem()==IT_NightVision    ) ||
	       (PL_CurrentItem()==IT_Thermal        ) ||
	       (PL_CurrentItem()==IT_TnkCamera      ) ||
	       (PL_CurrentWeapon()==WP_Claymore     ) ||
		   (PL_CurrentWeapon()==WP_None         ) ||
	       (PL_CurrentWeapon()==WP_C4Bomb       ) )) ) {
	    work->arm_trigger |= ARM_INVISIBLE ;
	} else {
	    work->arm_trigger &= ~ARM_INVISIBLE ;
	}
    } else {
		CheckWeaponCamera( work ) ;
	}


#ifdef NO_SUBJECT
    if ( work->vwait == 0 ) return ;
    if ( work->vwait > 0 ) {
		work->vwait = 0 ;
		DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
		work->arm_trigger |= ARM_INVISIBLE ;
		work->weapon_body = &( work->body ) ;
		work->weapon_unit = RIGHT_HAND_UNIT ;
    } else {
		work->vwait = 0 ;
		DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
		//	if ( !Status( PLAYER_INTRUDE ) ) work->arm_trigger &= ~ARM_INVISIBLE ;
		work->arm_trigger &= ~ARM_INVISIBLE ;
		work->weapon_body = GM_PlayerArmBody ;
		work->weapon_unit = ARM_RIGHT_HAND_UNIT ;
    } 
#else
    if ( work->vwait == 0 ) return ;
    if ( work->vwait > 0 ) {
		if ( GM_CameraMode == 1 || work->subject_camera->on ||
			 work->weapon_camera->on ) return ;
		cam1 = work->subject_camera ;
		cam2 = GM_GetCurrentCamera( work->subject_camera->chanl ) ;
		len = GV_VecLen3F2( &cam1->position, &cam2->position ) ;

		if ( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ) ++ work->vwait ;
		{
			work->vwait = 0 ;
			DG_VisibleObjsChanl( work->body.objs, work->chanl ) ;
			work->arm_trigger |= ARM_INVISIBLE ;
			work->weapon_body = &( work->body ) ;
			work->weapon_unit = RIGHT_HAND_UNIT ;
			if ( Flag( FLAG_JOINT_INVISIBLE ) ) {
				PL_VisibleObjsAll( work->body.objs, work->chanl ) ;
				/* ゴルキャップ中は頭首関節非表示 */	
				if ( work->item == IT_Uniform &&
					( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
					PL_VisibleHead( 0, work->body.objs, -1 ) ;
#if 0
					work->body.objs->objs[ HUMAN21_KUBI ].flag |= DG_FLAG_INVISIBLE ;
					work->body.objs->objs[ HUMAN21_ATAMA ].flag |= DG_FLAG_INVISIBLE ;
#endif
				}
			}
		}
    } else {
		if ( GM_CameraMode == 0 && work->subject_camera->on == 0 &&
			 work->weapon_camera->on == 0 ) return ;
		cam1 = GM_GetCurrentCamera( work->subject_camera->chanl ) ;
		if ( work->weapon_camera->on ) cam2 = work->weapon_camera ;
		else						   cam2 = work->subject_camera ;
		//cam2 = work->subject_camera ;

		len = GV_VecLen3F2( &cam1->position, &cam2->position ) ;	
		if ( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ) -- work->vwait ;
		chklen = 650.0F ;
		if ( work->stance == STAND && 	
			 work->action2 == ShootBullet ) {
			/* 各武器構え時の調整 */
			if ( work->weapon == WP_m4 || work->weapon == WP_Aks ) chklen = 200.0F ;
			else if ( work->weapon == WP_Rgb6 ) chklen = 100.0F ;
		}
		if ( work->action2 == ThrowGrenade ) {
			if ( work->stance != GROUND ) chklen = 200.0F ;
			else						  chklen = 70.0F ;
		}
		c = 0 ;
		if ( len < chklen || work->vwait < DIRECT_TICK( -48 ) ) c = 1 ;
		else if ( Status( PLAYER_CAUTION ) && !Flag( FLAG_BEHIND_ATTACK ) ) {
			if ( GV_DiffDirAbs( work->control.rot.vy, work->subject_camera->rotate.vy ) < 128 ) {
				if ( CheckMovRotLenSegment( work->control.hzx_id, 
										   ( FVECTOR * )work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ],
										   &DG_ZeroVector, &work->control.rot, -350,
										   HZX_CHK_ALL,
										   HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE ) ) {
					c = 1 ;
				}
			}
		}
		if ( c ) {
			work->vwait = 0 ;
			DG_InvisibleObjsChanl( work->body.objs, work->chanl ) ;
			//	    if ( !Status( PLAYER_INTRUDE ) ) work->arm_trigger &= ~ARM_INVISIBLE ;
			work->arm_trigger &= ~ARM_INVISIBLE ;
			work->weapon_body = GM_PlayerArmBody ;
			work->weapon_unit = HUMAN21_MIGI_TE ;
			/* 写り込みの非表示 */
			/* いつも消してみる */
			//if ( work->stance == GROUND ) {
				PL_InvisibleObjsAll( work->body.objs, work->chanl ) ;
				SetFlag( FLAG_JOINT_INVISIBLE ) ;
			//}
			//if ( Status( PLAYER_INTRUDE ) )	NewIntrudeInDuct2D_2( 0, 0 ) ;
		}	
    }
#endif
}

/*---------------------------------------------------------------------*/

/* 無敵チェック */
static	inline	void	CheckInvincible( Work *work )
{
    int		time ;

    time = work->invincible_time ;
    if ( time > 0 ) {
		if ( -- time == 0 ) UnsetInvincible( work ) ;
		work->invincible_time = time ;
    } else if ( Status( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) ) {
		/* 絶対無敵中、交差ダメージなし */
		work->touch.damaged = 0 ;
	}
}

/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/

/* はりつき入り */
static	void	IntoCaution( Work *work )
{
    if ( Liable == NEUTRAL ) return ;
    if ( !Status( PLAYER_CAUTION ) ) {
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			GM_SeSetMode( SD_P_SENAKA2N, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else {
			GM_SeSetMode( SD_P_SENAKA02, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
    }
    SetStatus( PLAYER_CAUTION ) ;
    work->control.turn.vy = WallTo + 2048 ;
    work->r_sphere = CAUTION_SPHERE ;
    work->caution_time = 0 ;
    GM_ConfigControlSetCollide( &( work->control ), 450.0F, 300.0F ) ;

    if (!work->inCautionMode)
    {
       work->inCautionMode = TRUE;
    }
}

/* 張り付き壁変更 */
static	inline	void	ChangeCautionWall( Work *work )
{	
	FVECTOR		wallvec ;
	
	DG_COPY_VEC( &wallvec, WallVec ) ;
	wallvec.vy = 0.0F ;
    if ( Wall != NULL &&
		( ( ( work->control.turn.vy & 4095 ) != ( ( WallTo + 2048 ) & 4095 ) ) ||
		  ( work->caution_time > 16 && ( GV_VecLen3F( &wallvec ) > CAUTION_SPHERE ) ) ) ) {
		work->control.turn.vy = WallTo + 2048 ;
		GM_PlayerCautionDir = work->control.turn.vy & 4095 ;
		work->caution_time = 0 ;
    }
}

/* 張り付きステップ */
static	inline	void	CautionStep( Work *work )
{
    if ( ++ work->caution_time < 64 && Wall != NULL ) {
		work->control.step.vx += WallVec->vx / 8.0F ;
		work->control.step.vz += WallVec->vz / 8.0F ;
    }
}

/* "wallto" 内処理 */
static	void	InsideWallToTrap( Work *work )
{
	int			wallto, rot ;
	int			diff, diff2 ;

	SetFlag( FLAG_CANNOT_CHANGE ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;	/* 回復可能 */
	SetStatus( PLAYER_MENU_DISABLE | PLAYER_NARROW ) ;
	work->control.r_sphere = CAUTION_SPHERE ;

	if ( Liable == NEUTRAL ) {
		Liable = FRONT ;
	}

	rot = work->control.rot.vy + 2048 ;
	
    if ( work->control.n_touches > 0 ) {
		Wall = work->control.segs[ 0 ] ;
		WallTo = GV_VecDir2( work->control.vecs ) ;
		WallVec = work->control.vecs ;
		WallAtr = work->control.atrs[ 0 ] ;	
	} else {
		printf( "warning : no wall wallto\n" ) ;
		WallTo = rot ;	
	}
	if ( work->control.n_touches == 2 ) {
		Liable = FRONT ;
#if 0
		Wall = work->control.segs[ 0 ] ;
		WallTo = GV_VecDir2( work->control.vecs ) ;
		WallVec = work->control.vecs ;
		WallAtr = work->control.atrs[ 0 ] ;	
#endif		
		diff = GV_DiffDirAbs( rot, WallTo ) ;
		wallto = GV_VecDir2( work->control.vecs + 1 ) ;
		diff2 = GV_DiffDirAbs( rot, wallto ) ;
		if ( diff > diff2 ) {
			Wall = work->control.segs[ 1 ] ;
			WallTo = wallto ;
			WallVec = work->control.vecs + 1 ;
			WallAtr = work->control.atrs[ 1 ] ;
		}
	}

	if ( Status( PLAYER_WATCH ) ) return ;

	if ( PadTo >= 0 ) {
		diff = GV_DiffDirS( PadTo, rot ) ;
		if ( diff > 192 ) {
			Liable = LEFT ;
		} else if ( diff < -192 ) {
			Liable = RIGHT ;
		}
	} else {
		if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) Liable = RIGHT ;
		else if  ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) Liable = LEFT ;
	}
}

/*---------------------------------------------------------------------*/

/* 敵を押す */
static	inline	void	PushEnemy( Work *work )
{
#if 0
    TARGET	*t ;

    t = &( work->def ) ;
    GM_MoveTarget( t, &( work->control.mov ) ) ;
    GM_ClearTargetPush( t ) ;
    GM_PutTarget( t ) ;
#endif
}

/*---------------------------------------------------------------------*/

/* ２点間ハザードチェック */
static	int		CheckBetweenHazard( Work *work,
					    FVECTOR *from, FVECTOR *to )
{
    int		touch ;

	touch = HZX_OnlineHazardCheck( work->control.hzx_id, from, to, HZX_CHK_ALL,
								   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
	touch |= HZX_OnlineHazardCheck( work->control.hzx_id, from, to, HZX_CHK_ALL,
								    HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY | HZX_FLOOR_IK ) ;

    return touch ;
}

/* 現在のモデルの座標Ｙの最大値と最小値を求める */
static	void	GetModelHeight( Work *work, float *max, float *min )
{
    int		i ;
    float	now ;
    DG_OBJ	*objs ;

    objs = work->body.objs->objs ;
	*max = GM_WORLD_LIMIT_BOTTOM ;
	*min = GM_WORLD_LIMIT_UPPER ;
    for ( i = 0; i < N_UNITS; i ++, objs ++ ) {
		now = objs->world.m[ 3 ][ 1 ] ;
		if ( now > *max ) {
			*max = now ;
		} else if ( now < *min ) {
			*min = now ;
		}
#if 0
		now = objs->world.m[ 3 ][ 0 ] ;
		if ( now > maxx ) maxx = now ;
		else if ( now < minx ) minx = now ;
		now = objs->world.m[ 3 ][ 2 ] ;
		if ( now > maxz ) maxz = now ;
		else if ( now < minz ) minz = now ;
#endif
    }    
	//printf( "%f %f %f\n", maxx - minx, *max - *min, maxz - minz ) ;
}

static	void	GetModelHeight2( work, max, min )
Work		*work ;
FVECTOR		*max, *min ;
{
    int		i ;
    FMATRIX	inv, mat ;
	SVECTOR	rot ;
    FVECTOR	v, mov ;
    DG_OBJ	*objs ;

    objs = work->body.objs->objs ;
    GV_MatToVec( &objs[ HUMAN21_KOSHI ].world, &mov ) ;
	rot = work->control.rot ;
	rot.vx = work->g_rot ;
	DG_SetPos2( &mov, &rot ) ;
	DG_GetPos( &mat ) ;
    _sceVu0TransposeMatrix( &inv, &mat ) ;
    inv.m[ 3 ][ 0 ] = 0.0F ;
    inv.m[ 3 ][ 1 ] = 0.0F ;
    inv.m[ 3 ][ 2 ] = 0.0F ;
    DG_SetPos( &inv ) ;
    max->vx = max->vy = max->vz = - 1000000.0F ; 
    min->vx = min->vy = min->vz = 1000000.0F ;
    for ( i = 0; i < N_UNITS; i ++, objs ++ ) {
		GV_MatToVec( &objs->world, &v ) ;
		_sceVu0SubVector( &v, &v, &mov ) ;
		DG_PutVector( &v, &v, 1 ) ;
		if ( v.vx < min->vx ) min->vx = v.vx ;
		else if ( v.vx > max->vx ) max->vx = v.vx ;
		if ( v.vy < min->vy ) min->vy = v.vy ;
		else if ( v.vy > max->vy ) max->vy = v.vy ;
		if ( v.vz < min->vz ) min->vz = v.vz ;
		else if ( v.vz > max->vz ) max->vz = v.vz ;
    }
}

/* ターゲットサイズ更新 */
static	void	SetTargetSize( Work *work )
{
    FVECTOR	max, min, size ;
	FVECTOR	findpos ;
    TARGET	*t ;

    t = &( work->def ) ;
    GetModelHeight2( work, &max, &min ) ;

	/* はりつき時特別 */
	if ( Status( PLAYER_CAUTION ) ) {
		/* のぞき込み以外のときは、横幅１／２ */
		if ( !Flag( FLAG_BEHIND_PEEP ) ) {
			max.vx /= 2.0F ; min.vx /= 2.0F ; 
		}
		if ( Status( PLAYER_SQUAT ) ) max.vz /= 4.0F ;
		else max.vz /= 2.0F ;
	} else if ( !Status( PLAYER_SNAKE ) && Status( PLAYER_ROLLING ) ) {
		/* ローリング特別。全部２／３ */
		_sceVu0ScaleVector( &max, &max, 2.0F / 3.0F ) ;
		_sceVu0ScaleVector( &min, &min, 2.0F / 3.0F ) ;
	}

	if ( !Status( PLAYER_IN_THE_WATER ) &&
		 Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		_sceVu0SubVector( &findpos, &GM_PlayerFindPos, 
						  ( FVECTOR * )work->body.objs->world.m[ 3 ] ) ;
		if ( findpos.vx <= min.vx ) min.vx = findpos.vx - 50.0F ;
		else if ( findpos.vx >= max.vx ) max.vx = findpos.vx + 50.0F ;
		if ( findpos.vy <= min.vy ) min.vy = findpos.vy - 50.0F ;
		else if ( findpos.vy >= max.vy ) max.vy = findpos.vy + 50.0F ;
		if ( findpos.vz <= min.vz ) min.vz = findpos.vz - 50.0F ;
		else if ( findpos.vz >= max.vz ) max.vz = findpos.vz + 50.0F ;
	}

    t->offset.vx = ( max.vx + min.vx ) / 2.0F ;
    size.vx = DG_FABS( ( max.vx - min.vx ) / 2.0F ) ;
    t->offset.vy = ( max.vy + min.vy ) / 2.0F ;
    size.vy = DG_FABS( ( max.vy - min.vy ) / 2.0F ) + 100.0F ;
    t->offset.vz = ( max.vz + min.vz ) / 2.0F ;
    size.vz = DG_FABS( ( max.vz - min.vz ) / 2.0F ) ;

    GM_SetTargetSize( t, &size ) ;

	/* 交差用 */
	if ( Status( PLAYER_CB_BOX ) ) {
		GV_SetVec3( &size, 250.0F, 250.0F, 250.0F ) ;//korekado
	} else {
		GV_SetVec3( &size, 160.0F, 400.0F, 160.0F ) ;
	}
	GM_SetTargetSize( &work->touch, &size ) ;
}

/* ターゲット移動 */
static	void	MoveTarget( work )
Work			*work ;
{
	SVECTOR		rot ;
	FVECTOR		mov ;
	FMATRIX		mat ;

	GV_MatToVec( &work->body.objs->world, &mov ) ;
	rot = work->control.rot ;
	rot.vx = work->g_rot ;
	DG_SetPos2( &mov, &rot ) ;
	DG_GetPos( &mat ) ;
    GM_MoveTarget2( &( work->def ), &mat ) ;
    SetTargetSize( work ) ;
}

/* CONTROL->heightをオブジェクトから設定 */
static	inline	float	ObjectHeight( work )
Work		*work ;
{
    float	max, min ;
    
    GetModelHeight( work, &max, &min ) ;
//printf( "%f : %f %f / %f : %f : %f\n", work->control.mov.vy - min, max, min, work->control.mov.vy,
//	     work->body.objs->objs[ 0 ].world.m[ 3 ][ 1 ],
//	     work->body.objs->world.m[ 3 ][ 1 ] ) ;
//    return ( work->control.mov.vy - min ) ;
	return ( work->body.objs->world.m[ 3 ][ 1 ] - min ) ;
}

/*---------------------------------------------------------------------*/

/* 姿勢別静止モーション */
static	int	StanceAct( stance )
int		stance ;
{
    switch( stance ) {
    case STAND :
		return MS.change[ Mstand ] ;
    case SQUAT :
		return MS.change[ Msquat ] ;
    case GROUND :
		return MS.change[ Mcrouch ] ;
    }
    return -1 ;
}

/*---------------------------------------------------------------------*/

/* 顔が敵の方を向く */
static	void	HeadToEnemy( work )
Work		*work ;
{
    HOMING_TRG	*hom = NULL ;
    FVECTOR	pos, trg, quat ;
    CONTROL	*ctrl ;
    static SVECTOR adj = { 0, 0, 0 } ;

    /* まだ実装してない */
    return ;

    if ( Status( PLAYER_CAUTION | PLAYER_INTRUDE | PLAYER_WATCH |
				PLAYER_HOLD | PLAYER_DAMAGED | PLAYER_DOWNED |
				PLAYER_GROUND ) ) {
		SetFlag( FLAG_NO_HEADTOENEMY ) ;
    }

    ctrl = &( work->control ) ;
    if ( work->action2 != NULL && 
		work->homing != NULL ) hom = work->homing ;
    else if ( !Flag( FLAG_NO_HEADTOENEMY ) ) {
		hom = GM_GetHomingTrg( &( ctrl->mov ), ctrl->rot.vy, 
							  ( int )LOOK_LEN, 
							  LOOK_RANGE, ctrl->hzx_id ) ;
    }
    if ( hom == NULL || Flag( FLAG_NO_HEADTOENEMY ) ) {
		GV_NearExp4PV( &adj, &DG_ZeroSVector, 3 ) ;
		if ( adj.vx != 0 || adj.vy != 0 ) {
			GM_RotToQuat( &adj, &quat ) ;
			work->body.m_ctrl->adjust[ HUMAN21_ATAMA ] = quat ;
			work->body.m_ctrl->adjust_flag |= 1 << HUMAN21_ATAMA ;
		}
    } else {
		MatToVec( &( work->body.objs->objs[ 11 ].world ), &pos ) ;	
		MatToVec( &( hom->body->objs->objs[ 11 ].world ), &trg ) ;
		GM_TrgToNearRotLimit( &pos, ctrl->rot.vy, &trg, &adj ) ;
		GM_RotToQuat( &adj, &quat ) ;
		work->body.m_ctrl->adjust[ HUMAN21_ATAMA ] = quat ;
		work->body.m_ctrl->adjust_flag |= 1 << HUMAN21_ATAMA ;
    }
}

/*---------------------------------------------------------------------*/

/* 静止していれば体力回復など */
static	void	RecoverLife( work )
Work		*work ;
{
    if ( work->power.vital <= 0 ) return ;
	/* たばこダメージ */
	/* たばこダメージは１／３まで */
	if ( work->item == IT_Tabacco && 
		 GM_Vitality > GM_VitalityMax / 3 ) {
		if ( ( GV_Time % FRAME_PER_SEC ) == 0 ) {
			if ( !Status( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) ) {
				GM_VitalityAdjustNoDead -- ;		
				if ( PL_PadEnable() ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				work->life.flag |= GM_GAGE_WARNING ;
			}
		}
	}
	if ( !Flag( FLAG_RECOVER_ENABLE ) || Status( PLAYER_HOLD ) ) {
		/* 失血ダメージ */
		if ( Status( PLAYER_BLOOD_DROP ) &&
//			work->item != IT_Styptic && 
			!Status( PLAYER_INVINCIBLE ) &&
			++ work->decrease_count >= BLOOD_DROP_DECREASE_TIME ) {
			if ( !Status( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) ) {
				work->decrease_count = 0 ;
				if ( PL_PadEnable() ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				GM_VitalityAdjustNoDead -- ;
			}
		}
		return ;
	}
#if 0
	if ( Status( PLAYER_BLOOD_DROP ) && 
		GV_Time - work->last_damaged_time > 180 &&	
#else
	if ( GM_Vitality < GM_VitalityMax / 3 && 
		GV_Time - work->last_damaged_time > DIRECT_TICK( 180 ) &&	
#endif
		++ work->recover_count == BLOOD_DROP_RECOVER_TIME ) {
		work->recover_count = 0 ;
		GM_VitalityAdjust += 2 ;
    }
}

/* ジアゼパム処理 */
static	void	CheckDiazepam( Work *work )
{
	float		amp ;

	if ( GM_DiazepamCount > 0 ) -- GM_DiazepamCount ;
	if ( GM_CameraAmplitudeValueDef[ work->chanl ] == 0.0F ) return ;
	if ( GM_DiazepamCount > 0 ) {
		amp = GM_CameraAmplitudeValue[ work->chanl ] ;
		amp = GV_NearSpeedF( amp, 0.0F, 0.50F ) ;
		GM_SetCameraOscillation( work->chanl, amp, -1.0F ) ;
	} else {
		amp = GM_CameraAmplitudeValue[ work->chanl ] ;
		amp = GV_NearSpeedF( amp, GM_CameraAmplitudeValueDef[ work->chanl ], 0.50F ) ;
		GM_SetCameraOscillation( work->chanl, amp, -1.0F ) ;		
	}
}

/*---------------------------------------------------------------------*/

/* 攻撃不可状態チェック */
#define	EnableShootBullet( _w )				EnableShootBullet2( _w, 0, 0.0F )
#define	EnableShootBullet2( _w, _m, _h )	EnableShootBullet3( _w, _m, _h, 1000.0F )
#define	EnableShootBulletEX( _w,_m,_h,_l,_s,_f ) EnableShootBulletEX2( _w,_m,_h,_l,_s,_f,NULL )
#define	EnableShootBulletEX2(_w,_m,_h,_l,_s,_f,NULL) EnableShootBulletEX3(_w,_m,_h,_l,_s,_f,NULL,HZX_CHK_ALL)

static	int	EnableShootBulletEX3( Work *work, int mode, float height, float chk_len,
								  int seg_flag, int flr_flag, FVECTOR *shift, int chk_flag )
{	
#ifdef NEAR_SHOOT_OK	
	return 0 ;
#else
    FVECTOR	mov ;
    SVECTOR	rot ;
    int		res ;
	//float	ChkHeight[] = { 1500.0F, 1100.0F, 420.0F, 150.0F } ;
	float	ChkHeight1S[] = { 1500.0F, 1100.0F, 380.0F, 150.0F + 100.0F } ;
	float	ChkHeight1R[] = { 1500.0F, 950.0F, 380.0F, 150.0F + 100.0F } ;
	float	ChkHeight2[] = { 1100.0F, 750.0F, 380.0F, 150.0F + 100.0F } ;
	float	*ChkHeight1 ;
	int		stance ;

	if ( Flag( FLAG_CANNOT_ATTACK ) ) return 1 ;

	ChkHeight1 = ( Status( PLAYER_SNAKE ) ) ? ChkHeight1S : ChkHeight1R ;
	stance = Status( PLAYER_INTRUDE ) ? 3 : work->stance ;

	if ( mode == 0 ) {
		height = ( WeaponType( work ) & WP_TYPE_SHOOTHEIGHT2 ) ? 
			ChkHeight2[ stance ] : ChkHeight1[ stance ] ;
	}
    rot = work->control.rot ;
    if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		if ( Flag( FLAG_PEEPING ) ) {
			DG_COPY_VEC( &mov, &work->peep ) ;
			//mov.vy -= 100.0F ;	/* 微調整（頭→首） */
		} else {
			DG_COPY_VEC( &mov, &work->camera ) ;
		}
		mov.vy = work->control.hzx_base + height ;
		if ( Flag( FLAG_PEEPING ) ) mov.vy += work->peep.vy - work->camera.vy ;
		mov.vy += PL_ObjHeight( work, HUMAN21_KOSHI ) - work->control.mov.vy ;
		rot.vx = work->camdir.vx ;
		

		if ( GM_GameStatus & STATE_GNO ) {//add kore
			if ( (work->weapon == WP_m92) ||
				 (work->weapon == WP_Usp) ||
				 (work->weapon == WP_Psg1) ||
				 (work->weapon == WP_Rgb6) ||
				 (work->weapon == WP_m4) ||
				 (work->weapon == WP_Psg1T)
				  ) {
				if ( rot.vx < -400 ) rot.vx = -400 ;
			}
		}
    } else {
		DG_COPY_VEC( &mov, &work->control.mov ) ;
		mov.vy = work->control.hzx_base + height ;
		mov.vy += PL_ObjHeight( work, HUMAN21_KOSHI ) - work->control.mov.vy ;
		//mov.vy = ObjHeight( work, HUMAN21_KUBI ) ;
		rot.vx = work->g_rot ;
    }
	
	if ( work->stance == GROUND ) {
		int		flag, atr[ 2 ] ;
		HZX_HZD	hzd[ 2 ] ;

		flag = HZX_LevelHazardCheck( work->control.hzx_id, &mov, HZX_CHK_ALL,
									 work->control.flr_flag ) ;
		if ( flag & 1 ) {
			HZX_GetLevelHazard( hzd, atr ) ;
			if ( atr[ 0 ] & HZX_FLOOR_STEP ) {
				mov.vy += 420.0F - 150.0F ;
			}
		}
	}
	if ( shift == NULL ) shift = &DG_ZeroVector ;
    res = CheckMovRotLenSegment( work->control.hzx_id, &mov, shift,
								&rot, chk_len, chk_flag,
								seg_flag, flr_flag ) ;
    return res ;
#endif
}

static	int	EnableShootBullet3( Work *work, int mode, float height, float chk_len )
{
	int		c ;
	c = EnableShootBulletEX3( work, mode, height, chk_len,
							 0, 0, NULL, HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY ) ;
	if ( c ) {
		/* SEG_DYNAMIC, SEG_NO_BULLET は FLOORも同じ */
		if ( ( ResultAtr & HZX_SEG_DYNAMIC ) &&
			 ( ResultAtr & HZX_SEG_NO_BULLET ) ) {	
			/* 動的壁でNO_BULLETだったら見ない */
			c = 0 ;
		}
	}
	return c ;
}

#if 0
/* 指定位置から指定ベクトルシフトした位置の床高さを調べる */
static	int	CheckMovRotSftLevel( float *levels, HZX_GROUP_ID hzx_id, 
								 FVECTOR *mov, SVECTOR *rot, FVECTOR *shift, 
								 int chk, int fflag ) 
{
    FVECTOR	shf, from ;
    int		c ;

    DG_SetPos2( mov, rot ) ;
    if ( shift->vx != 0.0F || shift->vy != 0.0F ||
		shift->vz != 0.0F ) {
		DG_RotVector( shift, &shf, 1 ) ;
		_sceVu0AddVector( &from, mov, &shf ) ;
    } else {
		_sceVu0CopyVector( &from, mov ) ;
    }
    c = HZX_LevelHazardCheck( hzx_id, &from, chk, fflag ) ;
    if ( c ) HZX_GetLevelHeight( levels ) ;
    return c ;
}
#endif

/* 匍匐から立ち上がれるかをチェック */
/* 立ち上がれない時０を返す */
static	inline	int	CheckCeil2( Work *work, float height )
{
    if ( ( work->control.level_found & 2 ) &&
		( work->control.levels[ 1 ] - work->control.levels[ 0 ] < height ) ) return 0 ;
    return 1 ;
}

#define	CheckCeil( _w )	CheckCeil2( _w, 2000.0F ) 

/*---------------------------------------------------------------------*/

/* ある領域のはじっことある座標との距離、方向指定つき */
static	float	BoundingEdgeLen( FVECTOR *mov, int dir,
				 FVECTOR *b1, FVECTOR *b2,FVECTOR *diff )
{
	FVECTOR		df = { 0.0F, 0.0F, 1.0F } ;
	SVECTOR		rot ;
	float		len, x, z, x1, x2, z1, z2 ;

	rot.vx = rot.vz = 0 ; 
	rot.vy = dir ;
	DG_SetPos2( mov, &rot ) ;
	DG_RotVector( &df, &df, 1 ) ;	

    x = z = -1.0F ;
    if ( df.vx != 0.0F ) {
		x1 = ( b1->vx - mov->vx ) / df.vx ;
		x2 = ( b2->vx - mov->vx ) / df.vx ;
		/* 同じ符合だったら */
		if ( x1 * x2 >= 0.0F ) {
		    x = fabs(x1)>fabs(x2) ? fabs(x1) : fabs(x2) ;
		} else {
		    x = x1<0.0f ? x2 : x1 ;
		}
    }
    if ( df.vz != 0.0F ) {
		z1 = ( b1->vz - mov->vz ) / df.vz ;
		z2 = ( b2->vz - mov->vz ) / df.vz ;
		/* 同じ符合だったら */
		if ( z1 * z2 >= 0.0F ) {
		    z = fabs(z1)>fabs(z2) ? fabs(z1) : fabs(z2) ;
		} else {
		    z = z1<0.0f ? z2 : z1 ;
		}
    }
    if ( x < 0.0F ) {
		len = df.vz = z * df.vz ;
		if ( len < 0.0F ) len = -len ;
    } else if ( z < 0.0F ) {
		len = df.vx = x * df.vx ;
		if ( len < 0.0F ) len = -len ;
    } else if ( x < z ) {
		df.vx = x * df.vx ;
		df.vz = x * df.vz ;
		len = GV_VecLen3F( &df ) ;
    } else {
		df.vx = z * df.vx ;
		df.vz = z * df.vz ;
		len = GV_VecLen3F( &df ) ;
    }
    if ( diff != NULL ) {
		diff->vx = df.vx ;
		diff->vz = df.vz ;
		diff->vy = 0.0F ;
    }
    return len ;	
}

/* 現在のビハインド地域のはじっこと今の自分の座標との距離 */
static	float	BehindEdgeLen( Work *work,int dir, FVECTOR *df )
{
    HZX_BEHIND	*bh  ;
    FVECTOR	 mov ;
    float	 len ;
    int		 rot ;

    bh = BehindPtr ;
    if ( bh == NULL ) return 1.0F ;
    mov.vx = work->control.mov.vx ;
    mov.vz = work->control.mov.vz ;
    mov.vy = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ;
    rot = ( dir == BEHIND_LEFT ) ? work->control.rot.vy - 1024 : work->control.rot.vy + 1024 ;
    len = BoundingEdgeLen( &mov, rot, &bh->b1, &bh->b2, df ) ;
    return len ;
}

/* ビハインド覗き先に壁があるかどうか */
#if 0
static	int	CheckBehindPeepSegment( work, dir )
Work		*work ;
int		dir ;
{
    FVECTOR	mov, diff ;

    diff.vy = diff.vz = 0.0F ;
    if ( dir == BEHIND_LEFT ) diff.vx = -( BEHIND_PEEP_START_LEN + 10.0F ) ;
    else		      diff.vx = BEHIND_PEEP_START_LEN + 10.0F ;
	//    mov.vx = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 0 ] ;
    mov.vy = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ;
	//    mov.vz = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 2 ] ;
    mov.vx = work->control.mov.vx ;
    mov.vz = work->control.mov.vz ;
    return CheckMovRotLenSegment( work->control.hzx_id, &mov, &diff,
								 &( work->control.rot ), -500.0F, 
								 HZX_CHK_SEGMENT,
								 HZX_SEG_NO_PLAYER,
								 HZX_FLOOR_NO_PLAYER ) ;
}
#else
#define	CheckBehindPeepSegment( _w, _d ) CheckBehindPeepSegment3( _w, _d, BEHIND_PEEP_START_LEN )
#endif

#if 0
static	int	CheckBehindPeepSegment2( work, dir, shf )
Work		*work ;
int		dir ;
FVECTOR		*shf ;
{
    FVECTOR	mov, diff ;

    diff.vy = diff.vz = 0.0F ;
    if ( dir == BEHIND_LEFT ) diff.vx = -( BEHIND_PEEP_START_LEN + 10.0F ) ;
    else		      diff.vx = BEHIND_PEEP_START_LEN + 10.0F ;
	//    mov.vx = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 0 ] ;
    mov.vy = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ;
	//    mov.vz = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 2 ] ;
    mov.vx = work->control.mov.vx ;
    mov.vz = work->control.mov.vz ;
    mov.vx += shf->vx ;
    mov.vz += shf->vz ;
    return CheckMovRotLenSegment( work->control.hzx_id, &mov, &diff,
								 &( work->control.rot ), -500.0F, HZX_CHK_SEGMENT,
								 HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
}
#endif

static	int		CheckBehindPeepSegment4( Work *work, int dir, float len, float height ) ;

static	int	CheckBehindPeepSegment3( Work *work, int dir, float len )
{
	float			height ;

	height = ( work->stance == STAND ) ? 1248.0F : 748.0F ;
	return CheckBehindPeepSegment4( work, dir, len, height ) ;
}
									 
static	int	CheckBehindPeepSegment4( Work *work, int dir, float len, float height )
{
    FVECTOR	mov, diff ;
	SVECTOR	rot ;
	int		chk_flag, seg_flag, c ;

    DG_COPY_VEC( &mov, &work->control.mov ) ;
	//printf( "%f %f\n", height, len ) ;
	//DumpVec( &work->control.mov ) ;
	//DumpSVec( &work->control.rot ) ;

	rot = work->control.rot ;
	if ( Wall != NULL ) rot.vy = WallTo + 2048 ;

    mov.vy = work->control.levels[ 0 ] + height ;
    diff.vy = diff.vz = 0.0F ;
    if ( dir == BEHIND_LEFT ) diff.vx = -( len + 10.0F ) ;
    else		      diff.vx = len + 10.0F ;    

	if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_FLAT ) {
		chk_flag = HZX_CHK_SEGMENT ;
		seg_flag = HZX_SEG_NO_PLAYER | HZX_SEG_NO_PEEPINTRPT | HZX_SEG_NO_HARITSUKI ;
	} else {
		/* 斜め床中は床もみる */
		chk_flag = HZX_CHK_ALL ;
		seg_flag = HZX_SEG_NO_PLAYER | HZX_SEG_NO_PEEPINTRPT | HZX_SEG_NO_HARITSUKI ;
		/* 階段中はNO_HARITSUKIも見る */
		if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
			seg_flag &= ~HZX_SEG_NO_HARITSUKI ;
		}
	}

    c =  CheckMovRotLenSegment( work->control.hzx_id, &mov, &diff,
							    &rot, -500.0F, chk_flag,
							    seg_flag,
							    HZX_FLOOR_NO_PLAYER ) ;    
	return c ;
}

/* ビハインド飛び出し先との間に壁がないかチェック */
static	int	CheckBehindAttackEnable( work, dir )
Work		*work ;
int		dir ;
{
    FVECTOR	mov, diff, to ;

    diff.vy = diff.vz = 0.0F ;
    if ( dir == BEHIND_LEFT ) diff.vx = -750.0F ;
    else		      diff.vx = 750.0F ;

    DG_COPY_VEC( &mov, &work->control.mov ) ;
    mov.vy = work->control.levels[ 0 ] + HzxHeight[ STAND ] ;
    DG_SetPos2( &mov, &work->control.rot ) ;
    DG_PutVector( &diff, &to, 1 ) ;
    return HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &to,
								 HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;    
}

/* ビハインド飛び出し攻撃位置をセット */
static	void	SetBehindAttackPosition( Work *work, FVECTOR *diff, 
										 float len, int dir )
{
    FVECTOR	v ;

	if ( !Status( PLAYER_BEHIND ) ) {
		work->behind_atk_mov.vw = 0.0F ;
		return ;
	}

    if ( Status( PLAYER_SQUAT ) ) {
		GV_LenVec3F( diff, &( work->behind_atk_mov ), len, len + BEHIND_ATTACK_LEN_SQ ) ;
		/* 少し下げる */
		/* 戻りモーションのＺ移動値にする */
		v.vx = v.vy = 0.0F ;
		if ( dir == BEHIND_LEFT ) {
			if ( Status( PLAYER_SNAKE ) ) v.vz = -230.0F ;
			else 						  v.vz = -330.0F ;
		} else {
			v.vz = 450.0F ;
		}
    } else {
		GV_LenVec3F( diff, &( work->behind_atk_mov ), len, len + BEHIND_ATTACK_LEN ) ;
		/* 少し下げる */
		/* 戻りモーションのＺ移動値にする */
		v.vx = v.vy = 0.0F ;
		if ( dir == BEHIND_LEFT ) v.vz = 350.0F ;
		else		      		  v.vz = 630.0F ;
    }
    _sceVu0AddVector( &( work->behind_atk_mov ), &( work->behind_atk_mov ), 
					 &( work->control.mov ) ) ;
    DG_SetPos2( &DG_ZeroVector, &( work->control.rot ) ) ;
    DG_RotVector( &v, &v, 1 ) ;
    _sceVu0AddVector( &( work->behind_atk_mov ), &( work->behind_atk_mov ), &v ) ;
    work->behind_atk_mov.vw = 1.0F ;
}

/* 今張り付いている壁が１ｍ以下かどうかチェック */
/* １ｍ以下なら１を返す */
/* １ｍ２５ｃｍ以下に変更 */
static	int	CheckCautionWallHeightIsLowerThan1M( work )
Work		*work ;
{
    float	chk_height, height, base ;
    float	interp ;

    if ( Wall == NULL ) return 1 ;
    chk_height = work->control.levels[ 0 ] + 1248.0F ;
    if ( DG_FABS( Wall->p1.y - Wall->p2.y ) < 0.10F &&
		DG_FABS( Wall->p1.h - Wall->p2.h ) < 0.10F ) {
		/* 長方形壁 */
		height = Wall->p1.y + Wall->p1.h ;
		return ( height < chk_height ) ? 1 : 0 ;
    }
    if ( DG_FABS( Wall->p2.x - Wall->p1.x ) < 0.10F ) {
		interp = ( work->control.mov.vz - Wall->p1.z ) / ( Wall->p2.z - Wall->p1.z ) ;
    } else {
		interp = ( work->control.mov.vx - Wall->p1.x ) / ( Wall->p2.x - Wall->p1.x ) ;
    }
    base = Wall->p1.y + ( Wall->p2.y - Wall->p1.y ) * interp ;
    height = Wall->p1.h + ( Wall->p2.h - Wall->p1.h ) * interp ;
    height = base + height ;
    return ( height < chk_height ) ? 1 : 0 ;    
}

/*---------------------------------------------------------------------*/

/* トラップ進入チェック */
static	int	CheckTrap( work, trap_id )
Work		*work ;
int		trap_id ;
{
    u_int	*inside ;
    int		n_inside ;

    n_inside = work->control.evt.n_inside ;
    inside = work->control.evt.inside ;
    while ( -- n_inside >= 0 ) {
		if ( ( *inside & 0x00ffffff ) == 
			( trap_id & 0x00ffffff ) ) return ( *inside ) ;
		inside ++ ;
    }
    return 0 ;
}

static	void	CheckTrapEvery( work ) 
Work		*work ;
{
    static u_int     	Traps[] = { DARK_TRAP, ENHIDDEN_TRAP, WALLTO_HIDDEN_TRAP } ;
    int			res ;

	GM_PlayerDarkLevel = 0 ;
    res = GM_CheckInsideTrap( &work->control, Traps, 3, 0 ) ;
    if ( res & 1 ) GM_PlayerDarkLevel ++ ;
	if ( res & 2 ) SetStatus( PLAYER_ENEMY_HIDDEN ) ;
	if ( res & 4 ) GM_SetPlayerStatusEX( I64(0), PLAYER2_NARROW_HIDDEN ) ;

	/* ダークトラップフラグの計算（重いがどうか？） */
	{
		u_int			*inside ;
		int				n_inside ;
		HZX_GROUP_ID	hzx_id ;
		HZX_BLOCK		*blk ;
		HZX_TRP			*trp ;

		inside = work->control.evt.inside ;
		n_inside = work->control.evt.n_inside ;
		hzx_id = work->control.hzx_id ;
		while( -- n_inside >= 0 ) {
			if ( HZX_FindTrap( hzx_id, *inside, &blk, &trp ) ) {
				GM_PlayerDarkLevel += ( ( trp->b1.pad & 0x00f0 ) >> 4 ) ;
			}
			inside ++ ;
		}
	}
	if ( GM_PlayerDarkLevel > 0 ) SetStatus( PLAYER_DARK_AREA ) ;
}

/*---------------------------------------------------------------------*/

/* 武器壁めり込みチェック */
static	int			CheckWeaponIntoWall( Work *work )
{
	int		c ;
	float	height, chk_len ;

	if ( MS.change[ Mstand_wall ] == NO_ACT ) return 0 ;
	if ( WeaponType( work ) & WP_TYPE_SHOOTHEIGHT2 ) {
		height = 750.0F ;
		chk_len = 750.0F ;
	} else {
		height = 950.0F ;
		chk_len = 750.0F ;
	}
	//c = EnableShootBullet3( work, 1, height, chk_len ) ;
	c = EnableShootBulletEX( work, 1, height, chk_len,
							 HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ;
	if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		if ( EnableShootBullet2( work, 0, 0.0F ) ) SetFlag( FLAG_CANNOT_ATTACK ) ;
	}
	return c ;
}

/* 武器めりこみ回避モーション */
static	void			SetStandWallAction( work, mode )
Work					*work ;
{
	if ( work->stance != STAND || 
		 MS.change[ Mstand_wall ] == NO_ACT ) {
		SetAction2( work, -1, 6, 0x01ffe ) ;
		return ;
	}
	if ( mode == 1 ) {
		SetAction( work, MS.change[ Mstand_wall ], 6 ) ;
	} else {
		if ( mode == 3 && work->action2 != NULL ) return ;
		if ( work->motion2 != MS.change[ Mrun_wall ] &&
			 work->motion2 != MS.attack[ Mreload ] ) {
			if ( work->action == StandStill ) {
				SetAction2Check( work, MS.change[ Mstand_wall ], 6, 0x01ffe ) ;
			} else {
				SetAction2Check( work, MS.change[ Mrun_wall ], 6, 0x01ffe ) ;
			}
		}
	}
}

/* 武器めりこみ回避モーション終わり */
static	inline	void	UnsetStandWallAction2( work )
Work					*work ;
{
	if ( work->motion2 == MS.change[ Mstand_wall ] ||
		 work->motion2 == MS.change[ Mrun_wall ] ) {
		SetAction2( work, -1, 6, 0x01ffe ) ;
	}
}

/* ポーリング関数の実行 */
static	void	ExecPollingFunc( work )
Work			*work ;
{
	PL_PollingSet	*list ;
	int				( *func )( Work * ) ;

//	if ( Status( PLAYER_DAMAGED | PLAYER_DOWNED |
//				 PLAYER_DEAD | PLAYER_FORCE | PLAYER_BEYOND ) ) return ;
	list = &PL_PollingList ;
	while( ( list = list->next ) != NULL ) {
		func = list->func ;
		if ( ( *func )( work ) != 0 ) break ;
	}
}

/* 変更不可武器・アイテムのセット */
static	void	SetNoUse( Work *work )
{
	/* 張り付き時 */
	if ( Status( PLAYER_CAUTION ) ) {
		PL_SetNoUseWeaponType( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ;
		PL_SetNoUseItemType( IT_TYPE_CALLFUNC | IT_TYPE_ABNORMAL ) ;
		if ( work->weapon != WP_Blade ) PL_SetNoUseWeapon( WP_Blade ) ;
	}
	/* 匍匐時 */
	if ( Status( PLAYER_GROUND | PLAYER_INTRUDE ) ) {
		if ( !Status( PLAYER_DOWNED ) ) {
			PL_SetNoUseWeapon( WP_Stinger ) ;
			PL_SetNoUseItemType( IT_TYPE_CBBOX ) ;
		}
		if ( !CheckCeil( work ) ) {
			if ( work->weapon == WP_Blade ) {
				SetFlag( FLAG_CANNOT_CHANGE_WEAPON ) ;
				SetStatus( PLAYER_WEAPON_DISABLE ) ;
			} else {
				PL_SetNoUseWeapon( WP_Blade ) ;
			}
		}
	}
	if ( Status( PLAYER_INTRUDE ) ) {
		if ( work->weapon != WP_Blade ) PL_SetNoUseWeapon( WP_Blade ) ;
	}	
	/* ダンボール時 */
	if ( Status( PLAYER_CB_BOX ) ) {
		PL_SetNoUseWeaponType( WP_TYPE_CALLFUNC | WP_TYPE_SUBJECT ) ;
	}
	/* ダンボール取られ時*/
	if ( Status( PLAYER_CB_BOX_CANCELED ) ) {
		//PL_SetNoUseItemType( IT_TYPE_CBBOX ) ;
		PL_SetNoUseItem( IT_CBBox ) ;
		PL_SetNoUseItem( IT_CBBoxB ) ;
		PL_SetNoUseItem( IT_CBBoxC ) ;
		PL_SetNoUseItem( IT_CBBoxD ) ;
		PL_SetNoUseItem( IT_CBBoxE ) ;
		PL_SetNoUseItem( IT_CBBoxWet ) ;
		/* 今装備しているのはＯＫ */
		PL_ResetNoUseItem( work->item ) ;
	}
}

/* 主観タクティカルリロード */
static	void	SubjectTacticalReload( Work *work )
{
	if ( !Status( PLAYER_WATCH ) || Flag( FLAG_RELOADING ) || 
		 work->action2 != NULL ) {
		work->weapon_quick = 0 ;
		return ;
	}
	if ( !( GM_WeaponTypes[ work->weapon ] & WP_TYPE_TACTICAL ) ) {
		work->weapon_quick = 0 ;
		return ;
	}
	if ( GM_Magazine >= GM_MagazineMax ) return ;

	if ( work->pad->press & PAD_R2 ) {
		int			premag ;
		if ( GV_Time - work->weapon_quick < 12 ) {
			premag = GM_Magazine ;
			PL_SetMagazine( work->weapon, 0 ) ;
			if ( GM_Magazine > premag ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01 ) ;
		}
		work->weapon_quick = GV_Time ;
	}
}

/* 現在のカメラと目標との距離 */
static	float	InterpCameraLen( GM_CameraSet *next )
{
	GM_CameraSet	*cur ;

	cur = GM_GetCurrentCamera( 0 ) ;
	return GV_VecLen3F2( &cur->position, &next->position ) ;
}

/* 張付き時の後壁距離補正 */
static	void	CautionBackWallAdjust( Work *work )
{
	FVECTOR		from ;
	FVECTOR		to = { 0.0F, 0.0F, -300.0F } ;
	SVECTOR		rot = { 0, WallTo + 2048, 0 } ;
	float		len ;
	int			ilen ;
	HZX_HZD		hzd ;

	len = 10000000.0F ;
	GV_MatToVec( &work->body.objs->world, &from ) ;
	DG_SetPos2( &from, &rot ) ;
	DG_PutVector( &to, &to, 1 ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &from, &to,
							    HZX_CHK_SEGMENT | HZX_CHK_RECOIL_TYPE_ONLY, 
							    HZX_SEG_RECOIL_TYPE, 0 ) ) {
		HZX_GetOnlineHazard( &hzd, NULL ) ;
		if ( !( hzd.attribute & HZX_SEG_DYNAMIC ) ||
			 !( hzd.attribute & HZX_SEG_NO_PLAYER ) ) {
			HZX_GetOnlineVector( &to ) ;
			len = GV_VecLen3F( &to ) ;
			//HZX_ViewHazard( &hzd ) ;
			//DumpSegment( &hzd ) ;
		}
	}
	//printf( "[%d]len %f\n", GV_Time, len ) ;	
	/* 1cm単位 */
	ilen = ( int )len ;
	ilen = ( ilen + 5 ) / 10 * 10 ;
	if ( len < CAUTION_SPHERE ) {
		work->r_sphere = CAUTION_SPHERE + ( CAUTION_SPHERE - ilen ) ;
	} else {
		work->r_sphere = CAUTION_SPHERE ;
	}
}

/* 主観系装備時の本体モーション設定 */
static	void	InitSubjectStanceControl( Work *work )
{
	work->work_l.subjectStanceCtrl = 0 ;
}

static	void	SetSubjectMotionMaster( Work *work, int stand, int squat, int ground )
{
	switch( work->stance ) {
	case STAND :
		SetAction( work, stand, 6 ) ;
		if ( work->motion2 >= 0 ) SetAction2( work, -1, 6, 0x01ffe ) ;
		return ;
	case GROUND :
	case INTRUDE :
		SetAction( work, ground, 6 ) ;
		if ( work->motion2 >= 0 ) SetAction2( work, -1, 6, 0x01ffe ) ;
		return ;		
	case SQUAT :
		if ( squat >= 0 ) {
			SetAction( work, squat, 6 ) ;
			if ( work->motion2 >= 0 ) SetAction2( work, -1, 6, 0x01ffe ) ;
			return ;		
		}
		SetAction( work, StanceAct( SQUAT ), 6 ) ;
		if ( work->motion2 != stand ) SetAction2( work, stand, 6, 0x01ffe ) ;
	}
}

static	int		ActSubjectStanceControl( Work *work, int stand, int squat, int ground ) 
{
	if ( work->work_l.subjectStanceCtrl == 1 ) {
		if ( EndMotion( work ) ) {
			work->work_l.subjectStanceCtrl = 0 ;
			work->arm_trigger &= ~ARM_INVISIBLE ;
		} else {
			work->arm_trigger |= ARM_INVISIBLE ;
			UnsetWeaponCamera( work ) ;
			SetArmAction( work, AMstand ) ;
			GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
			return 1 ;
		}
	}

	if ( work->pad->press & PL_PAD_SQUAT ) {
		switch( work->stance ) {
		case STAND :
			PL_ChangeStance( work, SQUAT ) ;
			break ;
		case SQUAT :
			PL_ChangeStance( work, STAND ) ;
			break ;
		case GROUND :
			if ( !CheckCeil( work ) ) break ;
			PL_ChangeStance( work, STAND ) ;
			work->work_l.subjectStanceCtrl = 1 ;
			SetAction( work, none_crouch_stand, 6 ) ;
			work->arm_trigger |= ARM_INVISIBLE ;
			UnsetWeaponCamera( work ) ;
			SetArmAction( work, AMstand ) ;
			GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
			return 1 ;
		}
	}
	SetSubjectMotionMaster( work, stand, squat, ground ) ;
	return 0 ;
}

static	inline	int	IsSubjectStanceControlCrouchToStand( Work *work )
{
	return work->work_l.subjectStanceCtrl ;
}

/* 指定位置、範囲内に指定敵がいるか調べる */
static	int			CheckNearEnemy( FVECTOR *pos, int dir, int dir_range, float range, int ene_flag )
{
	HOMING_TRG		*hom ;
	float			len ;
	FVECTOR			mov ;
	int				dir2 ;

	if ( ene_flag == 0 ) ene_flag = HOMING_ENEMY ;
	hom = GM_GetHoming() ;
	while( hom != NULL ) {
		if ( ( hom->status & ene_flag ) &&
			 !( hom->status & HOMING_UNREAL ) ) {
			GV_MatToVec( hom->world, &mov ) ;
			dir2 = GV_VecDir2FromTo( pos, &mov ) ;
			if ( GV_DiffDirAbs( dir, dir2 ) < dir_range ) {
				len = GV_VecLen3F2( &mov, pos ) ;
				if ( len < range ) return 1 ;
			}
		}
		hom = hom->next ;
	}	
	return 0 ;
}

/* しゃがみ張付きオンリーチェック */
static	inline	int	SquatOnly( Work *work )
{
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_SQUAT_ONLY ) ) return 1 ;
	return 0 ;
}