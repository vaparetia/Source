/*
   event.c 
   ライデン／各種イベント対応
   
   1999/07/07 M.Sonoyama
   $Id: event.c,v 1.3 2002/11/23 12:42:26 Yoshizawa1 Exp $			
   */

/*---------------------------------------------------------------------------*/

/* 主観関連 */

/*---------------------------------------------------------------------------*/

extern float BP_AdjustTick3(float);

#include "BP_EndianSupport.h"
#include "BP_Misc.h"

/* 主観モードチェック */
extern int PL_PadType     ; /* パッドタイプ       */
extern int PL_PadTypePrev ; /* パッドタイプ       */

extern int gBP_UseVitaControlOverrides;
extern int BP_Pad_GetSlowMovementAssignment();

static inline int use_vita_tweaked_controls()
{
   int useVitaTweakedControls = 0;
#ifdef BP_VITA
   if (!(GM_GameStatus & STATE_PAD_DEMO))
   {
      useVitaTweakedControls = 1;
   }
#endif
   return useVitaTweakedControls;
}

static	void	CheckWatch( Work *work )
{
    if ( Flag( FLAG_DONOT_CHECK_WATCH ) ) return ;

   //BP_CAMERA - extra camera functionality
   if( (gBP_1stPersonCamera_Override) || (gBP_1stPersonCamera_Active) )
   {
      int state = 0;

      // Override?
      if(gBP_1stPersonCamera_Override)
      {
         // Set toggle mode
         PL_SubjectToggle = gBP_1stPersonCamera_Toggle;

         // Compute requested state
         if(PL_SubjectToggle)
         {
            // Toggle 1st/3rd person?
            state = ((work->pad->press & PL_PAD_SUBJECT) != 0) ^ gBP_1stPersonCamera_Active;
         }
         else
         {
            // 1st/3rd person comes directly from holding down button
            state = ((work->pad->status & PL_PAD_SUBJECT) != 0);
         }
      }

      // Switch between 1st/3rd person camera?
      if( state != gBP_1stPersonCamera_Active )
      {
         // Record new state
         gBP_1stPersonCamera_Active = state;

         // Enter 1st person?
         if( state )
         {
            // Switch camera to 1st person
            IntoSubject( work ) ;
            
            // Switch to subject move logic
            PL_SubjectMove = gBP_1stPersonCamera_Move;

            // Clear watch flag so move logic input is applied
            if( gBP_1stPersonCamera_Move )
            {
               PL_UnsetStatus( PLAYER_WATCH );
            }
         }
         else
         {
            // Switch back to regular movement
            PL_SubjectMove = 0;
            
            // Stop pad config from being reset
            PL_PadTypePrev = PL_PadType;

            // Switch camera to 3rd person
            LeaveSubject( work ) ;
#ifdef CAMERA_TURN
            work->camdir.vx = work->g_rot ;
#else
            work->control.turn.vx = work->g_rot ;
#endif
         }
      }
      return;
   }
   //BP_CAMERA - extra camera functionality


#ifndef KP_WINDOWS
	if ( PL_SubjectToggle ) {
		/* 主観の入りがトグルになっている */
		if ( work->pad->press & PL_PAD_SUBJECT ) {
			if ( Status( PLAYER_WATCH ) ) {
				LeaveSubject( work ) ;
#ifdef CAMERA_TURN
				work->camdir.vx = work->g_rot ;
#else
				work->control.turn.vx = work->g_rot ;
#endif
			} else {
				IntoSubject( work ) ;
			}
		}
	} else {
		/* 主観がボタンを押した状態 */
		if ( work->pad->status & PL_PAD_SUBJECT ) {
			if ( !Status( PLAYER_WATCH ) ) {
				IntoSubject( work ) ;
#ifdef CAMERA_TURN
				//GM_CameraDir.vx = work->camdir.vx = work->g_rot ; /* IntoSubject内で処理 */
#endif
			}
		} else {
			if ( Status( PLAYER_WATCH ) ) {
				LeaveSubject( work ) ;
#ifdef CAMERA_TURN
				//work->camdir.vx = work->g_rot ;	/* LeaveSubject内で処理 */
#else
				work->control.turn.vx = work->g_rot ;
#endif
			}
		}
	}
#else
	/* Windows版ではトグル版を同時に共存させる */
	if ( Status( PLAYER_WATCH ) ) {
		/* 主観中 */
		if ( PL_SubjectToggle ) {
			/* トグル中 */
			if ( work->pad->press & PL_PAD_SUBJECT ) {
				/* トグル状態解除 */
				PL_SubjectToggle = 0 ;
			}
			if ( work->pad->press & PL_PAD_SUBJECT_TOGGLE ) {
				/* 客観へ変更 */
				LeaveSubject( work ) ;
#ifdef CAMERA_TURN
				work->camdir.vx = work->g_rot ;
#else
				work->control.turn.vx = work->g_rot ;
#endif
				/* トグル状態解除 */
				PL_SubjectToggle = 0 ;
			}
		} else {
			/* ボタン押し主観中 */
			if ( !(work->pad->status & PL_PAD_SUBJECT) ) {
				/* 客観へ変更 */
				LeaveSubject( work ) ;
#ifdef CAMERA_TURN
				//work->camdir.vx = work->g_rot ;	/* LeaveSubject内で処理 */
#else
				work->control.turn.vx = work->g_rot ;
#endif
			}
		}
	} else {
		/* 客観中 */
		if ( work->pad->status & PL_PAD_SUBJECT ) {
			IntoSubject( work ) ;
#ifdef CAMERA_TURN
			//GM_CameraDir.vx = work->camdir.vx = work->g_rot ; /* IntoSubject内で処理 */
#endif
			PL_SubjectToggle = 0 ;
		} else if ( work->pad->press & PL_PAD_SUBJECT_TOGGLE ) {
			IntoSubject( work ) ;
			PL_SubjectToggle = 1 ;
		}
	}
#endif
}

/* 覗き込み先にキャラがいないかチェック */
static	int			CheckPeepPosChara( FVECTOR *pos )
{
	HOMING_TRG		*hom ;
	float			len ;

	hom = GM_GetHoming() ;
	while( hom != NULL ) {
	    if ( !(hom->status & (HOMING_SKIP|HOMING_UNREAL)) ||
		  (hom->status & HOMING_DEAD) ) {
		len = GV_VecLen3F2( ( FVECTOR * )hom->world->m[ 3 ], pos ) ;
		if ( len < 1000.0F ) return 1 ;
	    }
	    hom = hom->next ;
	}	
	return 0 ;
}



/* オンラインのあとに呼ぶべき関数 */
static void CalcWallSegVector( FVECTOR *wallto, int flag, FVECTOR *dir )
{
	HZX_HZD	resultHzx ;	/* オンライン検出ハザード */
	int	    resultAtr ;	/* オンライン検出ハザード属性 */
	float   scale ;

#define WALL_APART 80.0f /* 壁から遠ざける距離(mm) */
	if ( flag & 1 ) {
		HZX_GetOnlineHazard( &resultHzx, &resultAtr ) ;
		wallto->vx = resultHzx.p2.z - resultHzx.p1.z ;
		wallto->vz = resultHzx.p1.x - resultHzx.p2.x ;
		wallto->vy = 0.0f ;
		wallto->vw = 0.0f ;
		_sceVu0Normalize( wallto, wallto ) ;
		if ( _sceVu0InnerProduct( wallto, dir ) < 0.0f ) {
			scale =  WALL_APART ;
		} else {
			scale = -WALL_APART ;
		}
		_sceVu0ScaleVector( wallto, wallto, scale ) ;
	}
}


/* 覗き込み */
#define	ANALOG_PEEP
#define	ANALOG_PEEP_STEP	(4)
#define	ANALOG_PEEP_PADMAX	(160)

static	void	SubjectPeep( Work *work, int status )
{
    static FVECTOR	PeepLevel[] = {
		{ -PEEP_LEVEL, 0.0F, 0.0F },
		{  PEEP_LEVEL, 0.0F, 0.0F },
		{  0.0F, PEEP_LEVEL_Y_SNA, 0.0F }
    } ;
	int		c1, c2 ;
   FVECTOR	peep, vec, vec2, check = { 0 }, *mov, chk_mov ;
	FVECTOR	chk_mov2, check2, tvec ;
    float	len, len2, th ;
	int     flag ;
    u_char	*pressure ;
	int		chkflag, flrflag ;
#ifdef ANALOG_PEEP
	int		prslevel = 0, p ;
#endif
	FVECTOR wallto, wallto2 ;

	mov = &( work->camera ) ;
	DG_COPY_VEC( &chk_mov, &work->camera ) ;
	chkflag = HZX_CHK_ALL ;
	flrflag = HZX_FLOOR_NO_PLAYER ;

    if ( Flag( FLAG_SUBJECT_HORIZON_LIMIT ) ) {
		DG_SetPos2( &chk_mov, &( work->camdir ) ) ;
    } else {
		DG_SetPos2( &chk_mov, &( work->control.turn ) ) ;
    }
    if ( Flag( FLAG_CANNOT_PEEP_LR ) ) 
    {
		if ( ( status & ( PL_PAD_PEEP_R | PL_PAD_PEEP_L ) ) !=
			( PL_PAD_PEEP_R | PL_PAD_PEEP_L ) ) 
      {
#if defined(BP_VITA)
         GestureConfigureDeadZone(kGesture_DeadZone_Default);
#endif
         return ;
      }
    }

#if defined(BP_VITA)
    GestureConfigureDeadZone(kGesture_DeadZone_None);
#endif

    pressure = work->pad->pressure ;
    if( (status & ( PL_PAD_PEEP_R | PL_PAD_PEEP_L )) == PL_PAD_PEEP_R) {	/* 右 */
		DG_COPY_VEC( &vec, &PeepLevel[ 0 ] ) ;
#ifdef ANALOG_PEEP

      if (!use_vita_tweaked_controls())
      {
   		prslevel = pressure[ PL_PAD_PRESS_PEEP_R ] + 1 ;
      }
#ifdef BP_VITA
      else
      {
         prslevel = (int)( GestureGetRightDragAmount(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work), 0.0f, 1.5f) * ANALOG_PEEP_PADMAX);
      }
#endif
		if ( prslevel > ANALOG_PEEP_PADMAX ) prslevel = ANALOG_PEEP_PADMAX ;
		prslevel = prslevel / ANALOG_PEEP_STEP * ANALOG_PEEP_STEP ;	
		vec.vx *= ( float )prslevel / ( float )ANALOG_PEEP_PADMAX ;
#endif
		DG_PutVector( &vec, &peep, 1 ) ;	    
		vec.vx -= ( float )work->control.r_sphere ;
		DG_PutVector( &vec, &check, 1 ) ;	    	
	} else if ( (status & ( PL_PAD_PEEP_R | PL_PAD_PEEP_L )) ==  PL_PAD_PEEP_L ) {	/* 左 */
		DG_COPY_VEC( &vec, &PeepLevel[ 1 ] ) ;
#ifdef ANALOG_PEEP

      if (!use_vita_tweaked_controls())
      {
   		prslevel = pressure[ PL_PAD_PRESS_PEEP_L ] + 1 ;
      }
#ifdef BP_VITA
      else
      {
         prslevel = (int)( GestureGetLeftDragAmount(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work), 0.0f, 1.5f) * ANALOG_PEEP_PADMAX);
      }
#endif
		if ( prslevel > ANALOG_PEEP_PADMAX ) prslevel = ANALOG_PEEP_PADMAX ;
		prslevel = prslevel / ANALOG_PEEP_STEP * ANALOG_PEEP_STEP ;	
		vec.vx *= ( float )prslevel / ( float )ANALOG_PEEP_PADMAX ;
#endif
		DG_PutVector( &vec, &peep, 1 ) ;	    
		vec.vx += ( float )work->control.r_sphere ;
		DG_PutVector( &vec, &check, 1 ) ;	    	
	} else {		/* 上 */
		if ( Status( PLAYER_SNAKE ) ) PeepLevel[ 2 ].vy = PEEP_LEVEL_Y_SNA ;
		else						  PeepLevel[ 2 ].vy = PEEP_LEVEL_Y_RAI ;

		DG_COPY_VEC( &vec, &PeepLevel[ 2 ] ) ;
#ifdef ANALOG_PEEP
		p = pressure[ PL_PAD_PRESS_PEEP_R ] ;
		if ( p < pressure[ PL_PAD_PRESS_PEEP_L ] ) p = pressure[ PL_PAD_PRESS_PEEP_L ] ;

      if (!use_vita_tweaked_controls())
      {
         prslevel = p + 1 ;
      }
#ifdef BP_VITA
      else if (GestureGetTriggerButtonMask(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work)) == (PL_PAD_PEEP_R | PL_PAD_PEEP_L))
      {
         float avgPressure = (GestureGetLeftDragAmount(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work), 0.0f, 1.0f) + 
            GestureGetRightDragAmount(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work), 0.0f, 1.0f)) / 2.0f;
         prslevel = (int)(avgPressure * ANALOG_PEEP_PADMAX);
      }
#endif
		if ( prslevel > ANALOG_PEEP_PADMAX ) prslevel = ANALOG_PEEP_PADMAX ;
		prslevel = prslevel / ANALOG_PEEP_STEP * ANALOG_PEEP_STEP ;	
		vec.vy *= ( float )prslevel / ( float )ANALOG_PEEP_PADMAX ;
#endif
		_sceVu0AddVector( &peep, &chk_mov, &vec ) ;
		vec.vy += ( float )work->control.r_sphere ;
		_sceVu0AddVector( &check, &chk_mov, &vec ) ;
		work->control.step.vy = 0.0F ;
		SetFlag( FLAG_PEEPING_UP | FLAG_NO_IK ) ;
		chkflag |= HZX_CHK_RECOIL_TYPE ;
		flrflag = 0 ;
    }

	DG_COPY_VEC( &chk_mov2, &chk_mov ) ;
	DG_COPY_VEC( &check2, &check ) ;
	chk_mov2.vy += ( work->control.hzx_base + HzxHeight[ work->stance ] ) - chk_mov.vy ;
	check2.vy += ( work->control.hzx_base + HzxHeight[ work->stance ] ) - check.vy ;

	/* カメラ位置チェック */
	_sceVu0CopyVector( &wallto, &DG_ZeroVector ) ;
	flag = HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov, &check,
								  chkflag, HZX_SEG_NO_PLAYER,
								  flrflag ) ;
	if ( flag ) {
		HZX_GetOnlineVector( &vec ) ;		
		CalcWallSegVector( &wallto, flag, &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		c1 = 1 ;
	} else {
		/* ６ｃｍ前チェック */
		GV_SetVec3( &tvec, 0.0F, 0.0F, 60.0F ) ;
		DG_RotVector( &tvec, &tvec, 1 ) ;
		_sceVu0AddVector( &chk_mov, &tvec, &chk_mov ) ;
		_sceVu0AddVector( &check, &tvec, &check ) ;
		flag = HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov, &check,
									  chkflag, HZX_SEG_NO_PLAYER,
									  flrflag ) ;

		if ( flag ) {
			HZX_GetOnlineVector( &vec ) ;		
			CalcWallSegVector( &wallto, flag, &vec ) ;
			len = GV_VecLen3F( &vec ) ;
			c1 = 1 ;		
		} else {
			len = 100000.0F ;
			DG_COPY_VEC( &vec, &DG_ZeroVector ) ;
			c1 = 0 ;
		}
	}

	/* 当たりチェック高さでチェック */
	DG_COPY_VEC( &wallto2, &DG_ZeroVector ) ;
	flag = HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov2, &check2,
								  chkflag, HZX_SEG_NO_PLAYER,
								  flrflag ) ;
	if ( flag ) {
		HZX_GetOnlineVector( &vec2 ) ;		
		CalcWallSegVector( &wallto2, flag, &vec2 ) ;/* 壁から少し遠ざけるため */
		len2 = GV_VecLen3F( &vec2 ) ;
		c2 = 1 ;
	} else {
		/* ６ｃｍ前チェック */
		GV_SetVec3( &tvec, 0.0F, 0.0F, 60.0F ) ;
		DG_RotVector( &tvec, &tvec, 1 ) ;
		_sceVu0AddVector( &chk_mov2, &tvec, &chk_mov2 ) ;
		_sceVu0AddVector( &check2, &tvec, &check2 ) ;
		flag = HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov2, &check2,
									  chkflag, HZX_SEG_NO_PLAYER,
								      flrflag );
		if ( flag ) {
			HZX_GetOnlineVector( &vec2 ) ;
			CalcWallSegVector( &wallto2, flag, &vec2 ) ;
			len2 = GV_VecLen3F( &vec2 ) ;
			c2 = 1 ;
		} else {
			len2 = 100000.0F ;
			DG_COPY_VEC( &vec2, &DG_ZeroVector ) ;
			c2 = 0 ;
		}
	}

	if ( c1 == 0 && c2 == 0 ) {
		_sceVu0CopyVector( &( work->peep ), &peep ) ;
	} else if ( len < len2 ) {
		th = len - ( float )work->control.r_sphere ;
		if ( th > 0.0F ) {
			GV_LenVec3F( &vec, &vec, len, th ) ;
		} else {
			DG_COPY_VEC( &vec, &DG_ZeroVector ) ;
		}
		_sceVu0AddVector( &work->peep, &vec, &chk_mov ) ;
		_sceVu0AddVector( &work->peep, &work->peep, &wallto ) ;
	} else {
		th = len2 - ( float )work->control.r_sphere ;
		if ( th > 0.0F ) {
			GV_LenVec3F( &vec2, &vec2, len2, th ) ;
		} else {
			DG_COPY_VEC( &vec2, &DG_ZeroVector ) ;
		}
		_sceVu0AddVector( &work->peep, &vec2, &chk_mov2 ) ;		
		_sceVu0AddVector( &work->peep, &work->peep, &wallto2 ) ;
		work->peep.vy -= chk_mov2.vy - chk_mov.vy ;
	}
#if 0
    if ( HZX_OnlineHazardCheck( work->control.hzx_id, &chk_mov, &check,
							   HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
							   HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		th = len - ( float )work->control.r_sphere ;
		if ( th > 0.0F ) {
			GV_LenVec3F( &vec, &vec, len, th ) ;
		} else {
			DG_COPY_VEC( &vec, &DG_ZeroVector ) ;
		}
		_sceVu0AddVector( &( work->peep ), &vec, &chk_mov ) ;
    } else {
		_sceVu0CopyVector( &( work->peep ), &peep ) ;
    }
#endif
	/* キャラチェック */
	if ( CheckPeepPosChara( &work->peep ) ) {
		DG_COPY_VEC( &work->peep, &work->camera ) ;
	}
    SetFlag( FLAG_PEEPING ) ;	
}

#ifdef PATTERN_C
static	void	SubjectPeepC( work )
Work		*work ;
{
    FVECTOR	peep, vec, *mov ;
    float	peeplen, len, th, dx, dy ;

    mov = &( work->peep_base ) ;
    DG_SetPos2( mov, &( work->control.turn ) ) ;
    
    dx = ( float )work->pad->left_dx - 128.0F ;
    dy = ( float )( work->pad->left_dy - 128.0F ) ;

    if ( fabs( dx ) < ANALOG_MARGIN ) dx = 0.0F ;
    if ( fabs( dy ) < ANALOG_MARGIN ) dy = 0.0F ;

    peep.vx = PEEP_LEVEL * dx / 128.0F ;
    peep.vy = PEEP_LEVEL * dy / 128.0F ;
    peep.vz = 0.0F ;

    peeplen = GV_VecLen3( &peep ) ;
    DG_PutVector( &peep, &peep, 1 ) ;	    

    if ( HZX_OnlineHazardCheck( work->control.hzx_id, mov, &peep,
							   HZX_CHK_ALL, HZX_SET_NO_PLAYER,
							   HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVectorF( &vec ) ;
		len = GV_VecLen3F( &vec ) ;
		th = peeplen + ( float )work->control.r_sphere ;
		if ( len < th ) {
			th = len - ( float )work->control.r_sphere ;
			if ( th <= 0.0F ) {
				_sceVu0CopyVector( &vec, &DG_ZeroVector ) ;
			} else {
				GV_LenVec3F( &vec, &vec, len, th ) ;
			}
			_sceVu0AddVector( &( work->peep ), &vec, mov ) ;
		}
    } else {
		_sceVu0CopyVector( &( work->peep ), &peep ) ;
    }
    work->control.step.vy = 0.0F ;
    SetFlag( FLAG_PEEPING ) ;
}
#endif

/* 主観モード操作 */
#define	VALUE_ADJ	(127.0F - ANALOG_MARGIN_F)
static	void	SubjectTurn( Work *work )
{
    int		status, turn, g_rot ;
    float	vstep, hstep ;
	int		vmax[ 2 ], hmax[ 2 ] ;
    u_char	*pressure ;
    short	*ac ;
    float	dx, dy ;

    /*主観移動の時は,こっちは通らない*/
    if ( PL_SubjectMove )
	return ;

    ac = work->subject_turn_accel ;
    vstep = ( GM_SubjectVStepTmp >= 0 ) ? ( float )GM_SubjectVStepTmp : ( float )GM_SubjectVStep ;
    hstep = ( GM_SubjectHStepTmp >= 0 ) ? ( float )GM_SubjectHStepTmp : ( float )GM_SubjectHStep ;

	if ( GM_SubjectAngleTmp > 0.0F ) {
		GM_SetCameraAngle( work->subject_camera, GM_SubjectAngleTmp ) ;
		GM_SetCameraAngle( work->weapon_camera, GM_SubjectAngleTmp ) ;
		GM_SubjectAngleTmp = -1.0F ;
	} else {
		GM_SetCameraAngle( work->subject_camera, 2.00F ) ;
		GM_SetCameraAngle( work->weapon_camera, 2.00F ) ;
	}

    if ( Status( PLAYER_SIGHT_LOCKON ) ) {
		vstep /= 2.0F ; hstep /= 2.0F ;
    }

    status = work->pad->status ;
    pressure = work->pad->pressure ;

#ifdef BP_VITA
    if (use_vita_tweaked_controls())
    {
       status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work));
    }
#endif

    if ( !Flag( FLAG_CANNOT_PEEP ) && ( status & PL_PAD_PEEP ) ) SubjectPeep( work, status ) ;
    g_rot = work->g_rot ;
    dx = dy = 0.0F ;

    if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		dx = -( ( float )work->pad->left_dx - 128.0F ) ;
		dx += ( dx > 0.0F ) ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;
		dy = ( float )work->pad->left_dy - 128.0F ;
		dy += ( dy > 0.0F ) ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;
    } else {
       if (!gBP_UseVitaControlOverrides || (GM_GameStatus & STATE_PAD_DEMO))
       {
          if ( status & PAD_U ) 
			   dy = - ( ( float )pressure[ PAD_PRESS_U ] * ( VALUE_ADJ * 2.0F / 255.0F ) ) ;
		    else if ( status & PAD_D ) 
			   dy = ( float )pressure[ PAD_PRESS_D ] * ( VALUE_ADJ * 2.0F / 255.0F ) ;
		    
          if ( status & PAD_L ) 
			    dx = ( float )pressure[ PAD_PRESS_L ] * ( VALUE_ADJ * 2.0F / 255.0F ) ;
		    else if ( status & PAD_R ) 
			    dx = -( ( float )pressure[ PAD_PRESS_R ] * ( VALUE_ADJ * 2.0F / 255.0F ) ) ;
		    
          if ( dy < -VALUE_ADJ ) 
             dy = -VALUE_ADJ ;
		    else if ( dy > VALUE_ADJ ) 
             dy = VALUE_ADJ ;
		    if ( dx < -VALUE_ADJ ) 
             dx = -VALUE_ADJ ;
		    else if ( dx > VALUE_ADJ ) 
             dx = VALUE_ADJ ;
       }
    }

    // On Vita, scale analog values if we are pressing square to allow for slow aiming since we do not use the D-Pad
    // MGSTWO-3000 - Slow aiming and sword block share the same button, so don't slow aim with the sword
    if ((gBP_UseVitaControlOverrides && !(GM_GameStatus & STATE_PAD_DEMO))
       && (status & BP_Pad_GetSlowMovementAssignment())
       && GM_Weapon != WP_Blade)
    {
       dx *= 0.1f;
       dy *= 0.1f;
    }

    if (use_vita_tweaked_controls())
    {
       dx *= 0.9f;
       dy *= 0.9f;
    }

	/* 主観操作反転 */
	PL_ShukanReverse( &dy, NULL ) ;


#define  ACCEL(_a) ((int)BP_AdjustTick3(_a))

/* 銃構え時は加速付き */
    if ( work->action2 == ShootBullet ||
		( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) ||
		( PL_ItemType( work ) & IT_TYPE_SUBJECT ) ) {
//		work->item == IT_Scope ) { /* きめうち */
		if ( dx != 0.0F ) {
			if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
				switch( GM_PadDemoVersion ) {
				case 1 :
				case 2 :
            case 3 :
					ac[ 0 ] += 8 ; 
					break ;
				default :
					ASSERT( 0 ) ;
				}
			} else
         {
            if (use_vita_tweaked_controls())
            {
               //MGSTWO-3232 - Acceleration was too low on Vita.
               ac[0] += ACCEL(8);
            }
            else
            {
               ac[0] += ACCEL(8);
            }
			}
			if ( ac[ 0 ] > 256 ) ac[ 0 ] = 256 ;
		} else {
			ac[ 0 ] = 0 ;
		}
		if ( dy != 0.0F ) {
			if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
				switch( GM_PadDemoVersion ) {
				case 1 :
				case 2 :
            case 3 :
					ac[ 1 ] += 8 ; 
					break ;
				default :
					ASSERT( 0 ) ;
				}
			} else
         {
            if (use_vita_tweaked_controls())
            {
               //MGSTWO-3232 - Acceleration was too low on Vita.
               ac[1] += ACCEL(8);
            }
            else
            {
               ac[1] += ACCEL(8);
            }
			}
			if ( ac[ 1 ] > 256 ) ac[ 1 ] = 256 ;
		} else {
			ac[ 1 ] = 0 ;
		}
		dx = dx * ( float )ac[ 0 ] / 256.0F ;
		dy = dy * ( float )ac[ 1 ] / 256.0F ;
		if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
			switch( GM_PadDemoVersion ) {
			case 1 :
				vstep *= 1.5F ; hstep *= 1.5F ;
				break ;
			case 2 :	
         case 3 :	
				vstep *= 2.4F ; hstep *= 2.4F ;				
				break ;
			default :
				ASSERT( 0 ) ;
			}
		} else {
			vstep *= 2.4F ; hstep *= 2.4F ;
		}
    } else {
		vstep *= 2.4F ; hstep *= 2.4F ;
		ac[ 0 ] = ac[ 1 ] = 0 ;
    }

    if ( BP_IsPAL()==TRUE )//BP_FRAMERATE - adjust for runtime PAL
    {
	   vstep *= 1.20F ; hstep *= 1.20F ;
    }

   vmax[ 0 ] = ( GM_SubjectVMaxTmp[ 0 ] != 0 ) ? GM_SubjectVMaxTmp[ 0 ] : GM_SubjectVMax[ 0 ] ;
	vmax[ 1 ] = ( GM_SubjectVMaxTmp[ 1 ] != 0 ) ? GM_SubjectVMaxTmp[ 1 ] : GM_SubjectVMax[ 1 ] ;
	hmax[ 0 ] = ( GM_SubjectHMaxTmp[ 0 ] != 0 ) ? GM_SubjectHMaxTmp[ 0 ] : GM_SubjectHMax[ 0 ] ;
	hmax[ 1 ] = ( GM_SubjectHMaxTmp[ 1 ] != 0 ) ? GM_SubjectHMaxTmp[ 1 ] : GM_SubjectHMax[ 1 ] ;

    if ( ( status & ( PAD_U | PAD_D ) ) && !Flag( FLAG_CANNOT_SUBJECT_UD ) ) {
#ifndef CAMERA_TURN
		turn = ( int )( vstep * dy / VALUE_ADJ ) ;
#else
        turn = ( int )( vstep * dy / VALUE_ADJ ) ;
#endif
		if ( turn == 0 && vstep != 0.0F && dy != 0.0F ) {
			turn = ( dy > 0.0F ) ? 1 : -1 ;
		}
		turn += work->camdir.vx ;

		if ( GM_GameStatus & STATE_GNO ) {//add kore
			if( vmax[ 1 ] <= -800 ) vmax[1] = -650 ;
		}

		if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
		else if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
		if ( turn < -1023 ) turn = -1023 ;
		else if ( turn > 1023 ) turn = 1023 ;

//printf("turn [%d] vmax[%d] + g_rot[%d]\n",turn,vmax[ 1 ], g_rot ) ;

		if ( work->stance == GROUND &&
		     work->weapon != WP_Psg1 && work->weapon != WP_Psg1T ) {
			if ( g_rot != 0 ) {
				if ( turn > g_rot + 8 ) turn = g_rot + 8 ;
			} else {
				if ( turn > g_rot + 128 ) turn = g_rot + 128 ;
			}
		}
#ifndef CAMERA_TURN
		work->control.turn.vx = turn ;
#else
		work->camdir.vx = turn ;
#endif
    } else {
		//	work->control.turn.vx = g_rot ;	
		if ( !Flag( FLAG_HOMING ) ) {
			turn = GM_CameraDir.vx ;
			if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
			else if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
			work->camdir.vx = turn ;
		}
    }
    if ( ( status & ( PAD_L | PAD_R ) ) && !Flag( FLAG_CANNOT_SUBJECT_LR ) ) {
		if ( Flag( FLAG_SUBJECT_HORIZON_LIMIT ) ) {
			turn = ( int )( hstep * dx / VALUE_ADJ ) ;
			if ( turn == 0 && hstep != 0.0F && dx != 0.0F ) {
				turn = ( dx > 0.0F ) ? 1 : -1 ;
			}
			work->camdir.vy += turn ;
			if ( hmax[ 1 ] < 0 &&
				 GV_DiffDirS( work->camdir.pad, work->camdir.vy ) < hmax[ 1 ] ) {
				work->camdir.vy = work->camdir.pad + hmax[ 1 ] ;
			} else if ( hmax[ 0 ] > 0 &&
					    GV_DiffDirS( work->camdir.pad, work->camdir.vy ) > hmax[ 0 ] ) {
				work->camdir.vy = work->camdir.pad + hmax[ 0 ] ;
			}
		} else {
			turn = ( int )( hstep * dx / VALUE_ADJ ) ;
			if ( turn == 0 && hstep != 0.0F && dx != 0.0F ) {
				turn = ( dx > 0.0F ) ? 1 : -1 ;
			}	    
			work->control.turn.vy += turn ;
		}
#ifndef CAMERA_TURN
		turn = work->control.turn.vx ;
#else
        turn = work->camdir.vx ;
#endif
		if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
		if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
		if ( turn < -1023 ) turn = -1023 ;
		else if ( turn > 1023 ) turn = 1023 ;
		if ( work->stance == GROUND &&
		     work->weapon != WP_Psg1 && work->weapon != WP_Psg1T ) {
			if ( g_rot != 0 ) {
				if ( turn > g_rot + 8 ) turn = g_rot + 8 ;
			} else {
				if ( turn > g_rot + 128 ) turn = g_rot + 128 ;
			}
//			if ( turn > g_rot + 8 ) turn = g_rot + 8 ;
//			if ( turn > g_rot ) turn = g_rot ;
		}
#ifndef CAMERA_TURN
		work->control.turn.vx = turn ;		
#else
		work->camdir.vx = turn ;
#endif
    } else {
		if ( !Flag( FLAG_HOMING ) ) {
			StopTurn( work ) ;
		}
	}
#ifndef CAMERA_TURN
	//    if ( g_rot != work->control.turn.vx ) SetFlag( FLAG_NO_STEP ) ;
#else
	//    if ( g_rot != work->camdir.vx ) SetFlag( FLAG_NO_STEP ) ;
#endif
}

/* シーク回転 */
static	void	SeekTurn( work )
Work			*work ;
{
    int		turn_to, diff ;

    if ( PadTo < 0 ) return ;
    if ( PL_SubjectMove ) return ;/* 主観移動の場合はSubjectMoveTurnでやる */

    turn_to = work->control.turn.vy ;
    diff = GV_DiffDirS( turn_to, PadTo ) ;
    if ( diff != 0 ) {
		turn_to = GV_NearPhase( turn_to, PadTo ) ;
		work->control.turn.vy = GV_NearSpeed( turn_to, PadTo, 32 ) ;
    }
}

/*---------------------------------------------------------------------------*/

/* メッセージ関連 */

/*---------------------------------------------------------------------------*/

static void EludeUpdateChinUpData( Work *work ) ;

/* 強制状態に入れそうかチェック */
/* トラップチェックで強制状態に1フレームだけ入ってしまうのを
   なんとか阻止 */
static	int		CheckEnableForceAct( Work *work, int code, GV_MSG *msg )
{
	if ( code == PL_MSG_MOTION ) {
		int		flag, eludefall ;

		eludefall = 0 ;
		flag = ( msg->message[ 5 ] & ~FA_CANCEL ) ;
		/* エルード再つかまり特殊 */
		if ( flag & FA_ELUDE_FALL_SPECIAL ) {
			eludefall = 1 ;
		}
		if ( Flag( FLAG_SPECIAL_MODE ) ||
			Status( PLAYER_LOCKER | PLAYER_DEAD | PLAYER_LADDER | PLAYER_STOP ) ) {
			printf( "raiden/event.c : force act cancel < unable mode > %x %lx\n",
				    work->flag, GM_PlayerStatus ) ;
			return 0 ;
		}
		if ( Status( PLAYER_BEYOND ) && eludefall == 0 ) {
			printf( "raiden/event.c : force act cancel < is elude >\n" ) ;
			return 0 ;
		}
		
		/* ビヨンド行きの時は、
		   nobeyondトラップ中ならば無視 */
		/* エルード落下中再つかまりはＯＫ */
		if ( msg->message[ 4 ] == FA_END_BEYOND ) {
			if ( eludefall == 0 && CheckTrap( work, NOBEYOND_TRAP ) ) {
				printf( "raiden/event.c : elude cancel!! < nobeyond >\n" ) ;
				return 0 ;
			}
			/* Ｙ座標指定のないエルード行のときは、
			   groundedでないとダメ。
			   エルード落下中はＯＫ */
			if ( msg->message_len < 9 &&
				 eludefall == 0 &&
				!( work->control.grounded & 1 ) ) {
				printf( "raiden/event.c : elude cancel!! < no floor >\n" ) ;
				return 0 ;
			}
			/* はだかのときも無視 */
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
				printf( "raiden/event.c : elude cancel!! < nude >\n" ) ;
				return 0 ;
			}
#if 0
			/* 何かいるときも無視 */
			if ( PL_EludeDisturbFunc != NULL ) {
				FVECTOR		mov ;

				DG_COPY_VEC( &mov, &work->control.mov ) ;
				mov.vy = work->control.levels[ 0 ] ;
				if ( PL_EludeDisturbFunc( &mov, 750.0F ) ) {
					printf( "raiden/event.c : elude cancel!! < kamome >\n" ) ;
					return 0 ;			
				}
			}
#endif
		}
	}
	return 1 ;
}

/* 強制モーションセット */
static	int	SetForceAct( work, msg )
Work		*work ;
GV_MSG		*msg ;
{
	int	   	flag, eludefall ;

	/* もう一度入れるかチェック */
	if ( !CheckEnableForceAct( work, PL_MSG_MOTION, msg ) ) {
		if ( Flag( FLAG_SET_FORCE_NOW ) ) UnsetStatus( PLAYER_FORCE ) ;
		PL_MotionActive( work, 0 ) ;
		printf( "raiden/event.c : msg motion : throw away\n" ) ;		
		return 0 ;
	}
	eludefall = 0 ;
    flag = ( msg->message[ 5 ] & ~FA_CANCEL ) ;
	/* エルード再つかまり特殊 */
	if ( flag & FA_ELUDE_FALL_SPECIAL ) {
		UnsetStatus( PLAYER_BEYOND ) ;			
		UnsetFlag( FLAG_FORCE ) ;
		flag &= ~FA_ELUDE_FALL_SPECIAL ;
		eludefall = 1 ;
		{
			/* 懸垂回数＋１０ */
			if ( Status( PLAYER_SNAKE ) ) GM_SnakeChin_Up += 9 ;
			else 						  GM_RaidenChin_Up += 9 ;
			EludeUpdateChinUpData( work ) ;

			/* ＳＥ */
			GM_SeSetMode( SD_V_PELUDE01, &work->control.mov, GM_SEMODE_BOMB ) ;
			GM_SeSetMode( SD_E_S_DOWN01, &work->control.mov, GM_SEMODE_BOMB ) ;
			/* 振動 */
			NewPadVibration2( GV_StrCode( "rai_catch" ), 0 ) ;
		}
	}

	/* 以下の状態のときは、状態解除まで保留 */
    if ( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) return -1 ;

#if 0
	/* 以下の状態のときは捨てる */
	if ( Flag( FLAG_SPECIAL_MODE ) ||
		 Status( PLAYER_LOCKER | PLAYER_BEYOND | 
				 PLAYER_DEAD | PLAYER_LADDER | PLAYER_STOP ) ) {
		if ( Flag( FLAG_SET_FORCE_NOW ) ) UnsetStatus( PLAYER_FORCE ) ;
		PL_MotionActive( work, 0 ) ;
		printf( "raiden/event.c : msg motion : throw away\n" ) ;
		GV_ERROR( GV_ERROR_PL_MESG_FAIL ) ;
		return 0 ;
	}
#endif

    ForceWork.motion = msg->message[ 1 ] ;
    ForceWork.s_turn = msg->message[ 2 ] ;
    ForceWork.e_turn = msg->message[ 3 ] ;
    ForceWork.e_stance = msg->message[ 4 ] ;
    ForceWork.flag = flag ;
	if ( ForceWork.loop < 2 ) ForceWork.loop = 1 ;

#if 0
    /* ビヨンド行きの時は、
       nobeyondトラップ中ならば無視 */
	/* エルード落下中再つかまりはＯＫ */
    if ( ( ForceWork.e_stance == FA_END_BEYOND ) ) {
		if ( eludefall == 0 && CheckTrap( work, NOBEYOND_TRAP ) ) {
			printf( "elude cancel!! < nobeyond >\n" ) ;
			UnsetStatus( PLAYER_FORCE ) ;
			return 0 ;
		}
		/* はだかのときも無視 */
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			printf( "raiden/event.c : elude cancel!! < nude >\n" ) ;
			UnsetStatus( PLAYER_FORCE ) ;
			return 0 ;
		}
		/* 何かいるときも無視 */
		if ( PL_EludeDisturbFunc != NULL ) {
			FVECTOR		mov ;

			DG_COPY_VEC( &mov, &work->control.mov ) ;
			mov.vy = work->control.levels[ 0 ] ;
			if ( PL_EludeDisturbFunc( &mov, 750.0F ) ) {
				printf( "raiden/event.c : elude cancel!! < kamome >\n" ) ;
				UnsetStatus( PLAYER_FORCE ) ;
				return 0 ;			
			}
		}
		SetStatus( PLAYER_BEYOND ) ;
		ForceWork.flag |= FA_NO_WEAPON ;
    }
#else
    if ( ( ForceWork.e_stance == FA_END_BEYOND ) ) {
		SetStatus( PLAYER_BEYOND ) ;
		ForceWork.flag |= FA_NO_WEAPON ;
	}
#endif

	/* 引数が７つ以上の場合、７，８，９番目の引数が
	   開始時座標 */
	ForceWork.flag &= ~FA_SET_START_Y ;
	if ( msg->message_len >= 7 ) {
		FVECTOR		pos ;

		pos.vx = ( float )msg->message[ 6 ] ;
		pos.vz = ( float )msg->message[ 7 ] ;
		pos.vy = ( msg->message_len >= 9 ) 
			? ( float )msg->message[ 8 ] : work->control.mov.vy ;
		GM_ResetControlPositionAndGroup( &( work->control ), &pos, 0 ) ;
		if ( msg->message_len >= 9 ) {
			/* 開始時Ｙ座標指定あり */
			ForceWork.flag |= FA_SET_START_Y ;
		}
	}

    /* 無敵など */
	UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
    LeaveCaution( work ) ;
    SetInvincible( work, 0 ) ;
	ClearCaptureTarget( work ) ;
    UnsetFlag( FLAG_FORCE_END ) ;
    SetFlag( FLAG_FORCE ) ;
    SetStatus( PLAYER_FORCE ) ;
    SetMode( work, ForceAct ) ;
	SetMode2( work, NULL ) ;
    work->body.m_ctrl->mt3_ctrl->flag &= ~MT3_SLEEP ;

    return 1 ;
}

/* メッセージ受信 */
static	void	ReceiveMessage( work )
Work		*work ;
{
    GV_MSG	*msg, *msg2 ;
    int		n_msg, code, enable_mode_exist ;

	UnsetFlag( FLAG_SET_FORCE_NOW ) ;
    if ( Flag( FLAG_DONOT_RECEIVE_MESSAGE ) ) return ;
    n_msg = GV_ReceiveMessage( work->control.name, &msg ) ;
    msg = msg + n_msg - 1 ;
    msg2 = work->message + work->n_msg ;
	PL_PluginPreCheck = 1 ;
	enable_mode_exist = 0 ;
    while ( -- n_msg >= 0 ) {
		if ( work->n_msg == MAX_LOCAL_MESSAGES ) break ;
		/* 強制ステートは、トラップチェックの前に
		   セットが必要 */
		if ( enable_mode_exist == 0 && !Status( PLAYER_DEAD ) ) {
			code = msg->message[ 0 ] ;
			/* ダメージ中でも発動しなければならないもの */
			if ( ( code == PL_MSG_MOTION ||
				   code == PL_MSG_STANCE ||
				   code == PL_MSG_RUN ) && 
				 !Status( PLAYER_FORCE ) &&
				 !( Flag( FLAG_FORCE ) && !Flag( FLAG_FORCE_END ) ) ) {
				/* 強制状態に入れそうかチェック */
				if ( CheckEnableForceAct( work, code, msg ) ) {
					SetStatus( PLAYER_FORCE ) ;
					SetFlag( FLAG_SET_FORCE_NOW ) ;

					GM_ClearTargetDamage( &( work->def ) ) ;
					work->touch.damaged = 0 ;
					UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
					enable_mode_exist = 1 ;
				}
				if ( code == PL_MSG_MOTION ) {
					/* 強制発行の瞬間、移動を止める */
					/* もう一回チェックが入るので、
					   EnableCheckの結果に関わらず止める */
					SetFlag( FLAG_NO_STEP | FLAG_NO_MOTION_STEP ) ;
					/* モーションも止める */
					//work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ; /* 止めない */
				}
			} else if ( !Flag( FLAG_SPECIAL_MODE ) ) {
				/* プラグインの発動プレチェック */
				if ( code == PL_MSG_LOCKER ) {
					PL_PluginSet		*this, *list ;

					this = NULL ;
					list = PL_PluginList.next ;
					while( list != NULL ) {
						if ( code == list->plugin_no ) {
							this = list ;
							break ;
						}
						list = list->next ;
					}
					if ( this != NULL ) {
						if ( ( *this->init )( work, msg, msg->message_len ) > 0 ) {
							SetStatus( PLAYER_FORCE ) ;
							SetFlag( FLAG_SET_FORCE_NOW ) ;
							GM_ClearTargetDamage( &( work->def ) ) ;
							work->touch.damaged = 0 ;
							UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ;
							SetFlag( FLAG_NO_STEP | FLAG_NO_MOTION_STEP ) ;
							enable_mode_exist = 1 ;
						}
					}
				}
			}
		}
		*msg2 = *msg ;
		memmove( &work->msgbuf[ work->n_msg ][ 0 ], msg->message, 
				sizeof( int ) * msg->message_len ) ;
		msg2->message = &work->msgbuf[ work->n_msg ][ 0 ] ;
		work->n_msg ++ ;
		msg -- ; msg2 ++ ;
    }    
	PL_PluginPreCheck = 0 ;
}

/* メッセージ実行 */
static	void	ExecMessage( work )
Work		*work ;
{
    GV_MSG	*msg, *msg2 ;
    int		n_msg, n_msg2, code, len, c, cr, res ;
    int		*elm ;
    FVECTOR	vec ;

    /* メッセージ実行 */
    if ( Flag( FLAG_DONOT_EXEC_MESSAGE ) ) return ;
    c = 0 ;
    n_msg = work->n_msg ; n_msg2 = 0 ;
    msg = msg2 = work->message ;
    while ( -- n_msg >= 0 ) {
#if 0
		{
			int			a ;

			printf( "raiden/event.c : [%d]exec : ", GV_Time ) ;
			for ( a = 0; a < msg->message_len; a ++ ) {
				printf( "%d ", msg->message[ a ] ) ;
			}
			printf( "\n" ) ;
		}
#endif
		code = msg->message[ 0 ] ;
		switch ( code ) {
		case PL_MSG_MOTION :	/* 強制モーション */
			if ( c || ( cr = SetForceAct( work, msg ) ) < 0 ) {
				//printf( "raiden/event.c : msg motion : wait\n" ) ;
				break ;
			}
			msg->message_len = 0 ;
			if ( cr ) {
				c = 1 ;
				printf( "raiden/event.c : [%d] MSG_MOTION EXEC %d\n", GV_Time, msg->message[ 1 ] ) ;
			}
			break ;
		case PL_MSG_POSITION :	/* 強制位置セット */
			//if ( !Status( PLAYER_DAMAGED ) ) {
				vec.vx = ( float )msg->message[ 1 ] ;
				vec.vy = ( float )msg->message[ 2 ] ;
				vec.vz = ( float )msg->message[ 3 ] ;
				printf( "raiden/event.c : [%d] MSG_POSITION %f %f %f\n", GV_Time, vec.vx, vec.vy, vec.vz ) ;
				PL_AdjustXZFromTo( work, &work->control.mov, &vec ) ;
				GM_ResetControlPositionAndGroup( &( work->control ), &vec, 0 ) ;
			//}
			msg->message_len = 0 ;
			break ;
		case PL_MSG_POSITION2 : /* 強制位置セット２ */
			//if ( !Status( PLAYER_DAMAGED ) ) {
				vec = work->control.mov ;
				len = msg->message_len - 1 ;
				elm = &( msg->message[ 1 ] ) ;
				while( len >= 2 ) {
					switch ( elm[ 0 ] ) {
					case 0 :
						vec.vx = ( float )elm[ 1 ] ;
						break ;
					case 1 :
						vec.vy = ( float )elm[ 1 ] ;
						break ;
					case 2 :
						vec.vz = ( float )elm[ 1 ] ;
					}
					len -= 2 ; elm += 2 ;
				}
printf( "raiden/event.c : [%d] MSG_POSITION2 %f %f %f\n", GV_Time, vec.vx, vec.vy, vec.vz ) ;
				PL_AdjustXZFromTo( work, &work->control.mov, &vec ) ;
				GM_ResetControlPositionAndGroup( &( work->control ), &vec, 0 ) ;
			//}
			msg->message_len = 0 ;
			break ;
        case PL_MSG_MENU :
			if ( msg->message[ 1 ] == PL_MSGSW_ON ) {
				GM_ResetMenuStatusScn( MENU_MENU_OFF ) ;
			} else {
				GM_SetMenuStatusScn( MENU_MENU_OFF ) ;
			}
            break ;
		case PL_MSG_BEYOND_TRAP :
			Beyond.trap_id = msg->message[ 1 ] ;
			msg->message_len = 0 ;
			break ;
		case PL_MSG_SHADOW :
			if ( msg->message[ 1 ] == 0 ) {
				SetFlag( FLAG_MSG_SHADOW_OFF ) ;
			} else {
				UnsetFlag( FLAG_MSG_SHADOW_OFF ) ;
			}
			msg->message_len = 0 ;
			break ;	    
        case PL_MSG_ROTATE :
            work->control.turn.vy = msg->message[ 1 ] ;
			msg->message_len = 0 ;
            break ;
#ifdef DEBUG_MODE
		case PL_MSG_HEADMARK :
			{
				extern void AN_HeadMark( FMATRIX *, int ) ;
				AN_HeadMark( &( work->body.objs->objs[ HUMAN21_ATAMA ].world ), 0 ) ;
			}
			break ;
#endif	   
		default :
			/* プラグインリストから検索 */
            if ( Flag( FLAG_SPECIAL_MODE ) ) {
				/* 捨てる */
				if ( Flag( FLAG_SET_FORCE_NOW ) ) {
					printf( "raiden/event.c : ---------- a -------------\n" ) ;
					UnsetStatus( PLAYER_FORCE ) ;
				}
				msg->message_len = 0 ;				
				break ;
			}
			if ( c ) {
				/* 保留 */
				if ( code == PL_MSG_STANCE || 
					 code == PL_MSG_RUN ) {
					break ;
				}
				/* 上記以外は捨てる */
				if ( Flag( FLAG_SET_FORCE_NOW ) ) {
					printf( "raiden/event.c : ---------- b -------------\n" ) ;
					UnsetStatus( PLAYER_FORCE ) ;
				}
				printf( "raiden/event.c : [%d] PLUGIN MSG %d throw away\n", GV_Time, code ) ;
				msg->message_len = 0 ;				
				break ;				
			}
			len = msg->message_len ;
			{
				PL_PluginSet	*pls, *list ;
				int	( *init )( Work *, GV_MSG *, int ) ;

				pls = NULL ;
				list = PL_PluginList.next ;
				while( list != NULL ) {
					if ( code == list->plugin_no ) {
						pls = list ; break ;
					}
					list = list->next ;
				}
				if ( pls != NULL ) {
					init = pls->init ;
					res = ( *init )( work, msg, len ) ;
					if ( res != 0 ) {
						msg->message_len = 0 ;
						if ( res == 1 ) c = 1 ;
					}
					if ( res != 1 && Flag( FLAG_SET_FORCE_NOW ) ) {
						printf( "raiden/event.c : ---------- c -------------\n" ) ;
						UnsetStatus( PLAYER_FORCE ) ;
					}
				}
			}
			break ;
		}
		/* 未処理のメッセージを再登録 */
		if ( msg->message_len != 0 ) {
			*msg2 = *msg ;
			memmove( &work->msgbuf[ n_msg2 ][ 0 ], msg->message, 
					sizeof( int ) * msg->message_len ) ;
			msg2->message = &work->msgbuf[ n_msg2 ][ 0 ] ;
			n_msg2 ++ ;
			msg2 ++ ;
		}
		msg ++ ;
    }
    work->n_msg = n_msg2 ;
}


/*---------------------------------------------------------------------------*/

/* イントルード関連 */

/*---------------------------------------------------------------------------*/

/* イントルードターン */
static	void	IntrudeTurn( work ) 
Work		*work ;
{
	SetFlag( FLAG_CANNOT_SUBJECT_UD ) ;
	SubjectTurn( work ) ;
	work->camdir.vx = work->g_rot ;
#if 0
    int		status, turn, g_rot ;
	int		vmax[ 2 ] ;
    float	dx, dy ;

    status = work->pad->status ;
    g_rot = work->g_rot ;
    dx = dy = 0.0F ;

	vmax[ 0 ] = ( GM_SubjectVMaxTmp[ 0 ] != 0 ) ? GM_SubjectVMaxTmp[ 0 ] : GM_SubjectVMax[ 0 ] ;
	vmax[ 1 ] = ( GM_SubjectVMaxTmp[ 1 ] != 0 ) ? GM_SubjectVMaxTmp[ 1 ] : GM_SubjectVMax[ 1 ] ;

    if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		dx = -( ( float )work->pad->left_dx - 128.0F ) ;
		dy = ( float )work->pad->left_dy - 128.0F ;
    } else {
		if ( status & PAD_U ) dy = -128.0F ;
		else if ( status & PAD_D ) dy = 128.0F ;
		if ( status & PAD_L ) dx = 128.0F ;
		else if ( status & PAD_R ) dx = -128.0F ;
    }
    if ( status & ( PAD_L | PAD_R ) ) {
		work->control.turn.vy += ( int )( ( float )GM_SubjectHStep * dx / 127.0F ) ;
#ifndef CAMERA_TURN
		turn = work->control.turn.vx ;
#else
		turn = work->camdir.vx ;
#endif
		if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
		if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
		if ( turn < -1023 ) turn = -1023 ;
		else if ( turn > 1023 ) turn = 1023 ;
		if ( work->stance == GROUND ) {
			if ( turn > g_rot + 8 ) turn = g_rot + 8 ;
		}
#ifndef CAMERA_TURN
		work->control.turn.vx = turn ;
#else
		work->camdir.vx = turn ;
#endif
    }
#endif
}

/* イントルードトラップ進入判定 */
static	int	CheckIntrude( work )
PlayerWork	*work ;
{
    return CheckTrap( work, HIDDEN_TRAP ) ;
}

/*---------------------------------------------------------------------------*/

/* 攻撃関連 */

/*---------------------------------------------------------------------------*/

/* 攻撃チェック（グローバル） */
void PL_CheckAttack( Work *work )
{
    int		press, status ;
    ACTION	func ;
    PL_WeaponSet	*wpset ;

    /* 武器独自の攻撃チェックがある */
    if ( PL_CheckAttackFunc != NULL ) {
	void	( *chkat )( PlayerWork *work ) ;

	/* 武器変更されてたらだめ */
	if ( GM_WeaponChanged || GM_ItemChanged ) {
	    printf( "raiden/event.c : attack cancel < equip changed >\n" ) ;
	    return ;
	}

	chkat = PL_CheckAttackFunc ;
	( *chkat )( work ) ;
	return ;
    }
    if ( work->action2 != NULL ) return ;
    press = work->pad->press ;
    status = work->pad->status ;
    wpset = ( PL_WeaponSet * )work->wp_set ;
    if ( wpset->type & WP_TYPE_PRESS ) status = press ;
    if ( status & PL_PAD_WEAPON ) {

		/* 武器変更されてたらだめ */
		if ( GM_WeaponChanged || GM_ItemChanged ) {
			printf( "raiden/event.c : attack cancel < equip changed >\n" ) ;
			return ;
		}

		/* 押し判定アナログ有り */
		if ( wpset->type & WP_TYPE_PRESSURE &&
			work->pad->pressure[ PL_PAD_PRESS_WEAPON ] < PL_PAD_WEAPON_TH ) {
			return ;
		}

		/* ＭＡＸチェック */
		if ( PL_WeaponType( work ) & WP_TYPE_MAXCHECK ) {
			switch( work->weapon ) {
			case WP_Claymore :
				if ( GM_N_Claymores >= GM_CLAYMORE_MAX ) {
					GM_Buzzer() ;
					return ;
				}
				break ;
			case WP_C4Bomb :
				if ( GM_N_C4Bombs >= GM_C4_MAX ) {
					GM_Buzzer() ;
					return ;
				}
				break ;
			case WP_Book :
				if ( GM_N_Books >= GM_BOOK_MAX ) {
					GM_Buzzer() ;
					return ;
				}
				break ;
			}
		}

		func = ( ( PL_WeaponSet * )( work->wp_set ) )->shoot ;
		if ( func != NULL ) {
			if ( wpset->type & WP_TYPE_MODE1FUNC ) {
				/* ニキータ特殊 */
				if ( work->weapon == WP_Nikita && 
					( ( Status( PLAYER_CAUTION ) && !Flag( FLAG_BEHIND_ATTACK ) ) ||
					 GM_NikitaAlive[ 0 ] != NKT_NONE ||
					 GM_CheckGameStatus( STATE_CHAFF | STATE_RADAR_JAMMING ) ) ) {
					if ( press & PL_PAD_WEAPON ) GM_Buzzer() ;
					return ;
				}
				SetMode( work, func ) ;
			} else {
				SetMode2( work, func ) ;
			}
		} else if ( work->weapon == WP_None ) {				/* 素手 */
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE | PLAYER2_KAGESHIBARI_NOW ) ) {
				/* はだか か 影縛り */
				return ;
			}

			switch( work->stance ) {
			case STAND :
				if ( PadForce > 0 ) {
					/* なげ */
					SetMode( work, Throw ) ;
				} else {
					/* 首絞め */
					SetMode( work, HangStart ) ;
				}
				SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
				SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
				break ;
			case SQUAT :
				/* 死体つかみ */
				SetMode( work, PullBody ) ;
				SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
				SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
				break ;
			}
		}
    } else if ( press & PL_PAD_PUNCH ) {

		/* 武器変更されてたらだめ */
		if ( GM_WeaponChanged || GM_ItemChanged ) {
			printf( "raiden/event.c : attack cancel < equip changed >\n" ) ;
			return ;
		}

		/* コムボ */
		if ( work->stance == STAND ) {
			if ( work->weapon == WP_Nikita ) {
				SetMode( work, NikitaStrike ) ;
			} else {
				SetMode( work, Combo ) ;
			}
			SetMode2( work, NULL ) ;
			SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
			SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
		}
    }
}

#if 0
/* Ｃ４壁付けチェック */
static	void	CheckC4BombWall( work )
Work			*work ;
{
	SVECTOR		rot ;
	FVECTOR		mov ;

	GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &mov ) ;
	rot = work->control.rot ; rot.vy += 2048 ;
	if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector, &rot,
							    450.0F, HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT |
							    HZX_CHK_RECOIL_TYPE_ONLY,
							    HZX_SEG_NO_C4, 0 ) ) {
		SetMode( work, SetC4BombWall ) ;
		work->data = ( int )ResultHzxPtr ;
	}
}
#endif

/*---------------------------------------------------------------------------*/

/* 壁関係 */

/*---------------------------------------------------------------------------*/

/* 壁こんこんチェック */
/* 立ちは、右移動時以外は、左コンコン */
/* しゃがみは状態によっていろいろ */
static	void	CheckKnockWall( work ) 
Work		*work ;
{
    if ( work->pad->press & PL_PAD_KNOCK || work->knockHappened) {
		if ( work->stance == STAND ) {	/* 立ち */
			if ( work->action == StandCautionRight &&
				 !CheckCautionWallHeightIsLowerThan1M( work ) ) {
				if ( work->action2 == KnockWallRight ||
					 work->action2 == KnockWallRightAgain ) {
					switch ( work->data3 ) {
					case 1 :
						SetMode2( work, KnockWallRightAgain ) ;
						break ;
					case 2 :
						SetMode2( work, KnockWallRight ) ;
						break ;
					}
				} else {
					SetMode2( work, KnockWallRight ) ;
				}
			} else {
				if ( work->action2 == KnockWallLeft ||
					 work->action2 == KnockWallLeftAgain ) {
					switch ( work->data3 ) {
					case 1 :
						SetMode2( work, KnockWallLeftAgain ) ;
						break ;
					case 2 :
						SetMode2( work, KnockWallLeft ) ;
						break ;
					}
				} else {
					SetMode2( work, KnockWallLeft ) ;
				}
			}
		} else {						/* しゃがみ */
			/* KnockWallLeft内で、しゃがみ専用モーションを出す */
			if ( work->action2 == KnockWallLeft ||
				 work->action2 == KnockWallLeftAgain ) {
				switch ( work->data3 ) {
				case 1 :
					SetMode2( work, KnockWallLeftAgain ) ;
					break ;
				case 2 :
					SetMode2( work, KnockWallLeft ) ;
					break ;
				}
			} else {
				SetMode2( work, KnockWallLeft ) ;
			}
		}
	}
}

/* 音コード取り出し */
#if 0	/* libmt.hで定義 */
#ifndef PAL    // BP JG - unused
#define TIME_BASE	(5) ;
#else
#define TIME_BASE	(6) ;
#endif
#endif
static	int		ExistSeCodeNext( Work *work, int layer )
{
	SAR_INST		*inst ;
	SEV_LOCAL_HEADER	*head ;
	SEV_ELEMENT		*elem ;
	SEV_PARAM		*param ;
	int				id, secode, time ;

	if ( work->body.m_ctrl->sar_ctrl == NULL ) return -1 ;
	inst = work->body.m_ctrl->sar_ctrl->instance + layer ;
	if ( !( inst->play_flag & SAR_FLAG_PLAY ) ) return -1 ;
	if ( inst->play_flag & ( SAR_FLAG_PAUSE | SAR_FLAG_MASK ) ) return -1 ;

	time = inst->time + inst->base_tick ;
	elem = inst->data_top ;
	if ( ( time < BP_LE_SwapUShort(elem->time_le) ) || ( time == 0xffff ) ) return -1 ;
	head = inst->local_header ;
	id = elem->id ;
	param = &head->params[ id ] ;
	secode = BP_LE_SwapUShort(param->se_code_le);
//	printf( "raiden/event.c : [%d] %d : code %x\n", GV_Time, MotionTime2( work ), secode ) ;
	return param->joint ;
}

/* 壁たたき位置で壁チェック */
static	int		CheckKnockSegment( work )
Work			*work ;
{
	FVECTOR		pos ;
//	FVECTOR		shift = { 0.0F, -160.0F, 0.0F } ;
	int			joint, again ;
	float		chklen, len ;

	if ( ( joint = ExistSeCodeNext( work, 1 ) ) < 0 ) return 0 ;
//	DG_SetPos( ObjWorld( work, joint ) ) ;
//	DG_PutVector( &shift, &pos, 1 ) ;
	PL_ObjPos( work, joint, &pos ) ;
printf( "raiden/event.c : se joint %d\n", joint ) ;
//printf( "knock " ) ;
//DumpVec( &pos ) ;

	{
		FVECTOR		adj = { 0.0F, 0.0F, 100.0F } ;
		/* すこし前方から判定 */
		DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
		DG_RotVector( &adj, &adj, 1 ) ;
		_sceVu0AddVector( &pos, &pos, &adj ) ;
	}

	again = 0 ;
checkknockwall_again :
	/* たたき位置と腰の間に壁がある */
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &pos,
							    HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
		HZX_GetOnlineVector( &pos ) ;
		len = GV_VecLen3F( &pos ) ;	
		if ( len < 5.0F ) {
			DG_COPY_VEC( &pos, &DG_ZeroVector ) ;
		} else {
			GV_LenVec3F( &pos, &pos, 0.0F, len - 4.0F ) ;
		}
		_sceVu0AddVector( &pos, &work->control.mov, &pos ) ;
	}

	chklen = ( Status( PLAYER_SNAKE ) ) ? -520.0F : -550.0F ;
	chklen -= 100.0F ;	/* 少し前方から判定するようにしたので */

	/* 武器を持っていて、左手で右側をたたくときはもう少し伸ばす */
	if ( GM_Weapon != WP_None ) {
		FVECTOR chk = { -100.0F, 0.0F, 0.0F }, diff, mov ;
		float	ip ;

		DG_COPY_VEC( &mov, &work->control.mov ) ;
		mov.vy = pos.vy ;
		_sceVu0SubVector( &diff, &pos, &mov ) ;
		DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
		DG_RotVector( &chk, &chk, 1 ) ;
		ip = _sceVu0InnerProduct( &diff, &chk ) ;
		if ( ip > 0.0F ) {
			chklen -= 200.0F ;
		}
	}
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) MovRotLenDebug = 1 ;
#endif
	if ( CheckMovRotLenSegment( work->control.hzx_id, &pos, &DG_ZeroVector, 
							    &work->control.rot, chklen, 
							    HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
							    0, 0 ) ) {
		//HZX_ViewHazard( ResultHzxPtr ) ;
        PL_KnockSegment.ptr = NULL ;
		PL_KnockSegment = *ResultHzxPtr ;
        PL_KnockJoint = joint ;
		if ( PL_KnockSegment.type == HZX_TYPE_FLOOR ||
			 !( ResultAtr & HZX_SEG_NO_KNOCK_SE ) ) {
			work->seNoSeg = HZX_GetSeCode( ResultAtr ) ;
printf( "raiden/event.c : knock seg %d\n", work->seNoSeg ) ;
		} else {
printf( "raiden/event.c : no knock se\n" ) ;			
		}
		SetFlag( FLAG_KNOCK_WALL ) ;
	} else {
		printf( "raiden/event.c : no knock wall\n" ) ;
		if ( again == 0 ) {
			printf( "raiden/event.c : try again\n" ) ;
			again = 1 ;
			pos.vy -= 60.0F ;
			goto checkknockwall_again ;
		}
	}
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) MovRotLenDebug = 0 ;
#endif
	return 1 ;
}

/*---------------------------------------------------------------------------*/

/* ダメージ関連 */

/*----------------------------------------------------------------*/

/* 交差コールバック */
static	void	TouchCallback( off, def, ptr )
TARGET			*off, *def ;
void			*ptr ;
{
	Work		*work ;
	FVECTOR		diff ;
	int			c, dir ;
    void		( *callback )( Work *, TARGET *, TARGET * ) ;

	work = ( Work * )ptr ;
	c = 0 ;
	if ( Status( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_DEAD |
			     PLAYER_BEYOND | PLAYER_FORCE | 
			     PLAYER_LOCKER | PLAYER_LADDER | PLAYER_INVINCIBLE |
				 PLAYER_INVINCIBLE_OUT | PLAYER_DEAD ) ) c = 1 ;
	else if ( work->current_mar != work->org_motion ) c = 1 ;
	else if ( Flag( FLAG_NO_TOUCH_DAMAGE ) ) c = 1 ;
	else if ( work->capture.capture != NULL &&
			  def->capture != NULL &&
			  work->capture.capture == def->capture ) c = 1 ;
	if ( c ) {
		off->damaged = 0 ;
		def->damaged &= ~TARGET_TOUCH ;
		return ;
	}
//	def->damaged &= ~TARGET_TOUCH ;	
	_sceVu0AddVector( &diff, &def->center, &def->offset ) ;
	_sceVu0SubVector( &diff, &diff, &work->control.mov ) ;
	dir = GV_VecDir2( &diff ) ;
	/* 勝ち負けチェック */
	if ( Status( PLAYER_MOVE ) && !Status( PLAYER_ENEMY_HANG ) ) {
		/* 動いてれば勝ち */
		work->post |= MAIL_PUSH_WIN ;
		/* 左右チェック */
		if ( GV_DiffDirS( work->control.rot.vy, dir ) >= 0 ) {
			/* 左からぶつかった */
printf( "raiden/event.c : touch left\n" ) ;
			work->post |= MAIL_PUSH_LEFT ;
		} else {
			/* 右からぶつかった */
printf( "raiden/event.c : touch right\n" ) ;
			work->post |= MAIL_PUSH_RIGHT ;
		}
		NewPadVibration2( GV_StrCode( "push" ), 0 ) ;
	} else {
		/* とまっていれば負け */
		work->post |= MAIL_PUSH_LOSE ;
		/* 前後チェック */
		if ( GV_DiffDirAbs( dir, work->control.rot.vy ) < 1024 ) {
			/* 前からぶつかられた */
			work->post |= MAIL_PUSH_FRONT ;
		} else {
			/* 後ろからぶつかられた */
			work->post |= MAIL_PUSH_BACK ;
		}
		/* 左右チェック */
		if ( GV_DiffDirS( work->control.rot.vy, dir ) >= 0 ) {
			/* 左からぶつかられた */
			work->post |= MAIL_PUSH_LEFT ;
		} else {
			/* 右からぶつかられた */
			work->post |= MAIL_PUSH_RIGHT ;
		}
	}
	/* ダメージコールバック */
	if ( work->dmg_callback != NULL ) {
		callback = work->dmg_callback ;
		( *callback )( work, off, def ) ;
	}
	
}

/* ターゲットコールバック */
static	void	TargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    FVECTOR	pos, force ;
	int		dmg = 0 ;
	long64	wt ;
    void	( *callback )( Work *, TARGET *, TARGET * ) ;
    int		( *chk_callback )( Work *, TARGET *, TARGET * ) ;
	static 	TARGET	offbuf ;
    int		p_vital ;

    work = ( Work * )ptr ;

	/* 箱蹴られ */
	if ( ( def->weapon_type & WP_BOXKICK ) && Status( PLAYER_CB_BOX ) &&
		 ( GM_ItemTypes[ work->item ] & IT_TYPE_CBBOX ) ) {
		work->control.turn.vy = GV_VecDir2FromTo( &def->center, &off->center ) ;
		NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
		SetMode( work, CB_BoxStop ) ;
	}
	if ( Status( PLAYER_DEAD ) ||
		( Status( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) &&
		 !( def->weapon_type & WP_NOGUARD ) ) ) {
#ifdef DEBUG_MODE
		if ( Status( PLAYER_INVINCIBLE_OUT ) ) {
			printf( "raiden/event.c : outsideprg or scenario invincible\n" ) ;
		}
#endif
		GM_ClearTargetDamage( def ) ;
		return ;
	}
    
   if ( PlayerDebugMenuStatus & PDMS_MUTEKIMODE1 )
   {
      if( !( def->weapon_type & WP_NOGUARD) )
      {
   		GM_ClearTargetDamage( def ) ;
	   	return ;
      } 
   }

   p_vital = def->power->vital ;

	if ( work->last_damagecallback_time != GV_Time ) {
		GM_PlayerDamagedWeaponType = I64(0) ;
		work->last_damagecallback_time = GV_Time ;
	}
	GM_PlayerDamagedWeaponType |= off->weapon_type ;

	wt = off->weapon_type & ~( WP_THROWG | WP_BOXKICK | WP_NOPLAYER ) ;
	/* スタンでもダメージを受けない */
	wt = wt & ~( WP_STUNGRENADE | WP_STUNFAR ) ;

	if ( off->power == NULL || wt == 0 ) {
		/* 攻撃方法によってはダメージ計算しない */
		if ( def->weapon_type == off->weapon_type ) {
			GM_ClearTargetDamage( def ) ;
		}
		def->weapon_type &= ~( off->weapon_type ) ;
		return ;
	}

	/* 無敵時間なし攻撃 */
	if ( wt & WP_NOMUTEKI ) {
		//printf( "raiden/event.c : [%d]nomuteki attack!\n", GV_Time ) ;
		SetFlag( FLAG_NOMUTEKI_ATTACKED ) ;
	} else {
		//printf( "raiden/event.c : [%d]not nomuteki attack!\n", GV_Time ) ;
		UnsetFlag( FLAG_NOMUTEKI_ATTACKED ) ;
	}

	/* 特殊コールバックが設定してある場合 */
	if ( PL_TargetCallbackFunc != NULL ) {
		chk_callback = PL_TargetCallbackFunc ;
		if ( ( *chk_callback )( work, off, def ) != 0 ) return ;
	}

    if ( ( def->damaged & TARGET_POWER ) && ( def->weapon_type != 0 ) ) {
		dmg = off->power->damage ;
#if 1
		if ( ( wt & WP_BULLET ) && ( work->item == IT_Jacket ) ) {
			/* 弾系は防弾チョッキで１／２ */
			off->power->damage /= 2 ;
		}
#else
		/* 日本版から。全部１／２ */
		if ( work->item == IT_Jacket ) {
			off->power->damage /= 2 ;
		}
#endif
		if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			/* はだかのときは１．５倍 */
			off->power->damage *= 3 ;
			off->power->damage /= 2 ;
		}

		/* force をクリア */
		DG_COPY_VEC( &def->power->force, &DG_ZeroVector ) ;

		GM_DamageTarget( off, def ) ;
		GM_DamageCount += off->power->damage ;
		if ( GM_DamageCount > 30000 ) GM_DamageCount = 30000 ;

		off->power->damage = dmg ;
		SetStatus( PLAYER_DAMAGED ) ;
		/* とりあえず、首ら辺から出血 */
		pos.vx = work->body.objs->objs[ 11 ].world.m[ 3 ][ 0 ] ;
		pos.vy = work->body.objs->objs[ 11 ].world.m[ 3 ][ 1 ] ;
		pos.vz = work->body.objs->objs[ 11 ].world.m[ 3 ][ 2 ] ;
		force.vx = pos.vx - off->center.vx ;
		force.vy = pos.vy - off->center.vy ;
		force.vz = pos.vz - off->center.vz ;
		
      if ( PlayerDebugMenuStatus & PDMS_MUTEKIMODE2 ) 
         def->power->vital = p_vital ;

		if ( off->weapon_type & WP_NORECOVER ) SetStatus( PLAYER_NORECOVER ) ;
		if ( !( off->weapon_type & WP_NO_BLOOD ) && off->power->damage > 0 ) {
#if 0
			if ( def->power->vital + GM_VitalityAdjust + GM_VitalityAdjustNoDead > 0 ) {
				NewBlood( &( work->body.objs->objs[ 11 ].world ), &pos, &force, 0, 0 ) ;
			} else {
				NewBlood( &( work->body.objs->objs[ 11 ].world ), &pos, &force, 1, 0 ) ;
			}
#endif
			SetFlag( FLAG_BLOOD_SPLASH ) ;
			DG_COPY_VEC( &workL->blood_force, &force ) ;
		}
		/* 体力１／３以下でぽたぽた血 */
#ifdef BLOOD_EVERY
		work->blood_drop_count = BLOOD_DROP_TIME ;
		SetStatus( PLAYER_BLOOD_DROP ) ;
		work->blood_drop_time = 0 ;
#else
		if ( dmg > 0 && !Status( PLAYER_BLOOD_DROP ) &&
			( def->power->vital < GM_VitalityMax / 3 ) ) {
			work->blood_drop_count = BLOOD_DROP_TIME ;
			SetStatus( PLAYER_BLOOD_DROP ) ;
		}
#endif
		if ( def->weapon_type & ( WP_BLOW | WP_SOFTBLOW | WP_SHOTGUN_NEAR ) ) {
			DG_COPY_VEC( &work->force, &def->power->force ) ;
		} else {
			FVECTOR		defp, offp ;

			GM_TargetGetCenter( &defp, def ) ;
			GM_TargetGetCenter( &offp, off ) ;
			_sceVu0SubVector( &work->force, &offp, &defp ) ;
		}
		if ( def->weapon_type & ( WP_BLOW | WP_SOFTBLOW | WP_SHOTGUN_NEAR ) ) {
			work->post &= ~( MAIL_SPS_DAMAGE_F | MAIL_SPS_DAMAGE_B |
							 MAIL_TUMBLE_F | MAIL_TUMBLE_B ) ;
			if ( !Status( PLAYER_NARROW | PLAYER_BEYOND | PLAYER_GROUND | PLAYER_DOWNED |
						  PLAYER_IN_THE_WATER | PLAYER_LADDER ) ) {
				work->control.turn.vy = GV_VecDir2( &( def->power->force ) ) ;
printf( "raiden/event.c : blow dir %d\n", work->control.turn.vy ) ;
				if ( wt & WP_SHOTGUN_NEAR ) {	/* ショットガン特殊 */
					/* force 決め決め */
					GV_LenVec3F( &work->force, &work->force, 0.0F, 128.0F ) ;
					if ( GV_DiffDirAbs( work->control.rot.vy, work->control.turn.vy ) < 1024 ) {
						work->post |= MAIL_SPS_DAMAGE_B ;
						work->control.rot.vy = work->control.turn.vy ;
					} else {
						work->control.turn.vy += 2048 ;
						work->post |= MAIL_SPS_DAMAGE_F ;
						work->control.rot.vy = work->control.turn.vy ;
					}	
printf( "raiden/event.c : shotgun dir %d\n", work->control.turn.vy ) ;
				} else if ( wt & WP_TUMBLE ) {	/* 転び */
					/* force 決め決め */
					GV_LenVec3F( &work->force, &work->force, 0.0F, 128.0F ) ;
					if ( GV_DiffDirAbs( work->control.rot.vy, work->control.turn.vy ) < 1024 ) {
						work->post |= MAIL_TUMBLE_B ;
						work->control.rot.vy = work->control.turn.vy ;
					} else {
						work->control.turn.vy += 2048 ;
						work->post |= MAIL_TUMBLE_F ;
						work->control.rot.vy = work->control.turn.vy ;
printf( "raiden/event.c : tumble dir %d\n", work->control.turn.vy ) ;
					}	
				} else {						/* 通常吹っ飛び */
					work->control.turn.vy += 2048 ;
					work->control.rot.vy = work->control.turn.vy ;
				}
			} else {
				if ( wt & ( WP_SHOTGUN_NEAR | WP_TUMBLE ) ) {
					/* force 決め決め */
					GV_LenVec3F( &work->force, &work->force, 0.0F, 128.0F * 3.0F ) ;
				}
			}
		}

//		def->power->reserved = off ;
		offbuf = *off ;
		def->power->reserved = &offbuf ;

		/* ダメージコールバック */
		if ( work->dmg_callback != NULL ) {
			callback = work->dmg_callback ;
printf( "raiden/event.c : call dmgcallback %x\n", callback ) ;
			( *callback )( work, off, def ) ;
		}
		/* ダメージを受けた瞬間は移動量なしにする */
		SetFlag( FLAG_NO_STEP ) ;
		StopTurn( work ) ;

		/* ＳＥ鳴らす */
		if ( off->class & TARGET_NAME_IS_SE ) {
			work->work_l.hit_se = off->name ;
		}
	} else {
		GM_ClearTargetDamage( def ) ;
	}
}

static void	NoWeapon( Work * ) ;
static void	NoEquip( Work * ) ;

/* ダメージチェック */
static	void	CheckDamage( work )
Work		*work ;
{
	int		seNo, dead, abnormal ;
    long64	weapon_type ;

    /* 無敵時処理 */
	if ( !Status( PLAYER_DEAD ) ) CheckInvincible( work ) ;

    if ( work->def.damaged & TARGET_POWER ) {
		dead = ( work->power.vital <= 0 ) ? 1 : 0 ;
		abnormal = ( ( WeaponType( work ) & WP_TYPE_SUBJECT ) ||
					 ( ItemType( work ) & IT_TYPE_ABNORMAL ) ) ? 1 : 0 ;
		if ( work->item == IT_Uniform && 
			 ( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
			/* 変装とける */
			/* 変装イベント後は解けない */
			NoEquip( work ) ;
		} else if ( work->item == IT_Stealth ) {
			/* ステルスはずれる */
			NoEquip( work ) ;
		}
		weapon_type = work->def.weapon_type ;
		SetStatus( PLAYER_DAMAGED ) ;
		/* 打撃音 */
		if ( work->work_l.hit_se != 0 ) {
			printf( "raiden/event.c : hit se %x\n", work->work_l.hit_se ) ;
			PL_SeSetSubject( work->work_l.hit_se, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			work->work_l.hit_se = 0 ;
		} else if ( weapon_type & WP_PUNCH ) {
			PL_SeSetSubject( SD_E_ATARU02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( weapon_type & WP_BLADESTAB ) {
			PL_SeSetSubject( SD_A_SWORDBIT, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( weapon_type & WP_BLADE ) {
			PL_SeSetSubject( SD_A_SWORDCUT, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( weapon_type & WP_KICK ) {
			PL_SeSetSubject( SD_P_KICK02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		}
		/* 攻撃タイプによってダメージモード振り分け */
		if ( PL_DamageFunc != NULL ) {
			printf( "raiden/event.c : plugin damage func!\n" ) ;
			( *PL_DamageFunc )( work, weapon_type, dead ) ;
			return ;
		} else if ( Status( PLAYER_BEYOND ) ) { 	/* エルード特殊 */
			if ( weapon_type & WP_STAMP ) { /* 踏まれ */
				work->hand_power.value -= ELUDE_HAND_POWER_MAX / 5.0F ; /* ハンドパワー減らし（適当） */
				PL_SeSetSubject( SD_V_PDMG02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
				work->post |= MAIL_ELUDE_STAMP ;
			} else {						/* 撃たれ */
				if ( dead ) {
					SetStatus( PLAYER_DEAD ) ;
					PL_SeSetSubject( SD_V_POUT0001, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
				} else {
					/* ハンドパワー減らし（適当） */
					work->hand_power.value -= ELUDE_HAND_POWER_MAX / 5.0F ; 
					PL_SeSetSubject( SD_V_PDMG02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
				}
			}
			if ( work->item == IT_Jacket ) {
				NewPadVibration( DamageVib1HHlf, 1 | VAR_FLAG_FORCE ) ;
				NewPadVibration( DamageVib1LHlf, 2 | VAR_FLAG_FORCE ) ;	    
			} else {
				NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
				NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
			}
			GM_VctrlStopVibration( &work->vctrl ) ;
			GM_ClearTargetDamage( &( work->def ) ) ;
			return ;
		} else if ( weapon_type & ( WP_BLOW | WP_SOFTBLOW | WP_SHOTGUN_NEAR ) ) { /* 吹っ飛び */
			if ( Status( PLAYER_CB_BOX ) ) {
				extern void *PutCBoxBreakBody( FMATRIX *world, int map ) ;
				GM_DecrementItem( work->item, 5 ) ;
				if ( GM_ItemNum( work->item ) <= 0 ) {
					/* 壊れたダンボール */
					PutCBoxBreakBody( &work->body.objs->world, work->control.map ) ;
				}
			}
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
			NewPadVibration( DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
			NewPadVibration( DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
			seNo = SD_V_PDMG01 ;
			if ( Status( PLAYER_INTRUDE ) ) {
				/* イントルード中 */
				if ( dead ) {
					SetMode( work, Dead ) ;
					seNo = SD_V_POUT0001 ;
				} else {
					/* ニキータはこれが要るみたいですね。近くで爆発 */
					if ( work->weapon==WP_Nikita && Status(PLAYER_WATCH) ) {
						LeaveSubject( work ) ;
					}
					SetMode( work, IntrudeStill ) ;
					SetInvincible( work, INVINCIBLE_TIME * 2 ) ;
					UnsetStatus( PLAYER_DAMAGED ) ;				
				}
			} else if ( CheckTrap( work, WALLTO_TRAP ) && 
					    Status( PLAYER_CAUTION ) ) {
				/* 張り付きＯＮＬＹエリア */
				if ( dead ) {
					SetMode( work, Dead ) ;
					seNo = SD_V_POUT0001 ;
				} else {
					SetMode( work, CautionShrink ) ;	
				}
			} else if ( Status( PLAYER_DOWNED ) ) {
				/* ダウン */
				if ( dead ) {
					SetMode( work, Dead ) ;
					seNo = SD_V_POUT0001 ;					
				} else {
					SetMode( work, DownDamage ) ;
				}
			} else if ( Status( PLAYER_GROUND ) ) {				
				/* 匍匐中 */
				if ( dead ) {
					if ( !CheckCeil( work ) ) SetMode( work, Dead ) ;
					else					 SetMode( work, Blow ) ;
					seNo = SD_V_POUT0001 ;					
				} else {
					if ( !CheckCeil( work ) ) SetMode( work, Shrink ) ;
					else					 SetMode( work, Blow ) ;
				}				
			} else {
				if ( dead ) seNo = SD_V_POUT0001 ;
				if ( !CheckTrap( work, NO_CROUCH_TRAP ) ) {
					SetMode( work, Blow ) ;
					if ( !( weapon_type & ( WP_BLOW | WP_SHOTGUN_NEAR ) ) ) {
						work->data = 1 ; /* ソフト吹っ飛び */
					}
				} else {
					/* 匍匐禁止エリア */
					if ( dead ) SetMode( work, Dead ) ;
					else		SetMode( work, Shrink ) ;
				}
			}
			//GM_SeSetMode( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( Status( PLAYER_DOWNED ) ) {		/* ダウンダメージ */
			/* イントルード中はこないはず！ */
			if ( dead ) {
				/* 死に */
				NewPadVibration( DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
				NewPadVibration( DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
				SetMode( work, Dead ) ;
				seNo = SD_V_POUT0001 ;
			} else if ( abnormal || Status( PLAYER_INTRUDE ) ) {
				/* 主観武器・アイテム中 */
				if ( work->item == IT_Jacket ) {
					NewPadVibration( DamageVib1HHlf, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1LHlf, 2 | VAR_FLAG_FORCE ) ;	    
				} else {
					NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
				}
				SetInvincible( work, INVINCIBLE_TIME * 2 ) ;
				UnsetStatus( PLAYER_DAMAGED ) ;				
				if ( Status( PLAYER_WATCH ) ) {
					work->control.turn.vy += ( ( GV_Time % 16 - 7 ) * 8 ) ;
				}
				seNo = SD_V_PDMG02 ;
				PL_DamageCamera( DIRECT_TICK( 48 ), PLAYER_WATCH | PLAYER_INTRUDE ) ;
			} else {
				if ( work->item == IT_Jacket ) {
					NewPadVibration( DamageVib1HHlf, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1LHlf, 2 | VAR_FLAG_FORCE ) ;	    
				} else {
					NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
				}
				SetMode( work, DownDamage ) ;
				seNo = SD_V_PDMG02 ;
			}
			//GM_SeSetMode( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else {									/* そのた */
			if ( ( weapon_type & WP_BOXREMOVE ) &&
				 ( ItemType( work ) & IT_TYPE_CBBOX ) ) {
				NoEquip( work ) ;	
				abnormal = 0 ;
			}
			if ( dead ) {
				/* 死に */
				NewPadVibration( DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
				NewPadVibration( DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
				SetMode( work, Dead ) ;
				seNo = SD_V_POUT0001 ;
			} else if ( abnormal | Status( PLAYER_INTRUDE ) ) {
				/* 主観武器・アイテム中・イントルード中 */
				if ( work->item == IT_Jacket ) {
					NewPadVibration( DamageVib1HHlf, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1LHlf, 2 | VAR_FLAG_FORCE ) ;	    
				} else {
					NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
				}
				SetInvincible( work, INVINCIBLE_TIME * 2 ) ;
				UnsetStatus( PLAYER_DAMAGED ) ;
				if ( Status( PLAYER_CB_BOX ) ) GM_DecrementItem( work->item, 1 ) ;
				if ( Status( PLAYER_WATCH ) ) {
					work->control.turn.vy += ( ( GV_Time % 16 - 7 ) * 8 ) ;
				}
				seNo = SD_V_PDMG02 ;
				PL_DamageCamera( DIRECT_TICK( 48 ), PLAYER_WATCH | PLAYER_INTRUDE ) ;
				if ( !abnormal && Status( PLAYER_INTRUDE ) ) SetMode( work, IntrudeStill ) ;
			} else {
				if ( work->item == IT_Jacket ) {
					NewPadVibration( DamageVib1HHlf, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1LHlf, 2 | VAR_FLAG_FORCE ) ;	    
				} else {
					NewPadVibration( DamageVib1H, 1 | VAR_FLAG_FORCE ) ;
					NewPadVibration( DamageVib1L, 2 | VAR_FLAG_FORCE ) ;	    
				}
				if ( Status( PLAYER_BEHIND_ATTACK ) ) {
					/* 飛び出し撃ち特殊 */
				} else if ( CheckTrap( work, WALLTO_TRAP ) && 
						    Status( PLAYER_CAUTION ) ) {
					/* 張り付きＯＮＬＹエリア */
					SetMode( work, CautionShrink ) ;	
				} else {
					SetMode( work, Shrink ) ;
				}
				seNo = SD_V_PDMG02 ;
			}
			//GM_SeSetMode( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( seNo, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		}
#if 0
		} else if ( work->power.vital <= 0 ) {	/* 死に */
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
			NewPadVibration( DamageVib2H, 1 ) ;
			NewPadVibration( DamageVib2L, 2 ) ;
			SetMode( work, Dead ) ;
			PL_SeSetSubject( SD_V_POUT0001, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( weapon_type & ( WP_BLOW | WP_SOFTBLOW ) ) { /* 吹っ飛び */
			if ( Status( PLAYER_CB_BOX ) ) {
				GM_DecrementItem( work->item, 5 ) ;
				if ( work->item == IT_CBBoxWet ) GM_DecrementItem( work->item, 5 ) ;
			}
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
			NewPadVibration( DamageVib2H, 1 ) ;
			NewPadVibration( DamageVib2L, 2 ) ;
			if ( CheckTrap( work, WALLTO_TRAP ) && 
				 Status( PLAYER_CAUTION ) ) {
				/* 張り付きＯＮＬＹエリア */
				SetMode( work, CautionShrink ) ;	
			} else {
				SetMode( work, Blow ) ;
				if ( !( weapon_type & WP_BLOW ) ) work->data = 1 ; /* ソフト吹っ飛び */
			}
			PL_SeSetSubject( SD_V_PDMG01, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else if ( Status( PLAYER_DOWNED ) ) {	/* ダウンダメージ */
			NewPadVibration( DamageVib1H, 1 ) ;
			NewPadVibration( DamageVib1L, 2 ) ;
			SetMode( work, DownDamage ) ;
			PL_SeSetSubject( SD_V_PDMG02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		} else {				/* その他 */
			if ( ( WeaponType( work ) & WP_TYPE_SUBJECT ) ||
				( ItemType( work ) & IT_TYPE_ABNORMAL ) ) {
				/* 主観武器・アイテム中 */
				SetInvincible( work, INVINCIBLE_TIME * 2 ) ;
				UnsetStatus( PLAYER_DAMAGED ) ;
				if ( Status( PLAYER_CB_BOX ) ) {
					GM_DecrementItem( work->item, 1 ) ;
					if ( work->item == IT_CBBoxWet ) GM_DecrementItem( work->item, 1 ) ;
				}
			} else {
				if ( Status( PLAYER_BEHIND_ATTACK ) ) {
					/* 飛び出し撃ち特殊 */
				} else if ( CheckTrap( work, WALLTO_TRAP ) && 
						    Status( PLAYER_CAUTION ) ) {
					/* 張り付きＯＮＬＹエリア */
					SetMode( work, CautionShrink ) ;	
				} else {
					SetMode( work, Shrink ) ;
				}
			}
			NewPadVibration( DamageVib1H, 1 ) ;
			NewPadVibration( DamageVib1L, 2 ) ;
			PL_SeSetSubject( SD_V_PDMG02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		}
#endif
		work->last_damaged_time = GV_Time ;
		work->control.skip_flag &= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_TRAP ) ;
		LeaveCaution( work ) ;
		UnsetWeaponCamera( work ) ;
		ClearCaptureTarget( work ) ;
		SetMode2( work, NULL ) ;
		SetArmAction( work, AMstand ) ;
	    GM_VctrlStopVibration( &work->vctrl ) ;
		GM_ClearTargetDamage( &( work->def ) ) ;
		work->trigger = TRIG_FALL ;	/* グレネード落とし */
		if ( GM_NikitaAlive[ work->chanl ] == NKT_NORMAL ) {
			GM_NikitaAlive[ work->chanl ] = NKT_CANCEL ;
		}
	    GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
	    /* リロード中で装填前なら装填する */
	    if ( Flag( FLAG_RELOADING ) && 
			( WeaponType( work ) & WP_TYPE_MAGAZINE ) &&
			GM_Magazine == 0 ) {
			PL_SetMagazine( work->weapon, 0 ) ;
		}
    } else if ( work->touch.damaged & TARGET_TOUCH ) {

		/* 交差ダメージ */
		work->touch.damaged = 0 ;
		if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
		if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
		//if ( ItemType( work ) & IT_TYPE_SUBJECT ) NoEquip( work ) ;

//		SetStatus( PLAYER_DAMAGED ) ;
		LeaveCaution( work ) ;
		LeaveSubject( work ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_TRAP ) ;
		ClearCaptureTarget( work ) ;
		SetMode2( work, NULL ) ;
		SetArmAction( work, AMstand ) ;
	    GM_VctrlStopVibration( &work->vctrl ) ;
		GM_ClearTargetDamage( &( work->def ) ) ;
		work->trigger = TRIG_FALL ;	/* グレネード落とし */
		if ( GM_NikitaAlive[ work->chanl ] == NKT_NORMAL ) {
			GM_NikitaAlive[ work->chanl ] = NKT_CANCEL ;
		}
		PL_SeSetSubject( SD_V_PDMG02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;

		if ( work->item == IT_Uniform &&
			( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
			/* 変装とける */
			NoEquip( work ) ;
		} else if ( work->item == IT_Stealth ) {
			/* ステルスはずれる */
			NoEquip( work ) ;
		}
		SetMode( work, PushOrPushed ) ;
	    GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
	    /* リロード中で装填前なら装填する */
	    if ( Flag( FLAG_RELOADING ) && 
			( WeaponType( work ) & WP_TYPE_MAGAZINE ) &&
			GM_Magazine == 0 ) {
			PL_SetMagazine( work->weapon, 0 ) ;
		}
	}
    work->work_l.hit_se = 0 ;
}

/*---------------------------------------------------------------------------*/

/* 武器チェンジ関連 */

/*---------------------------------------------------------------------------*/

/* 武器変更本体 */
static	void	SetWeapon( work, no_change )
Work		*work ;
int		no_change ;
{
    void	*act ;
    PL_WeaponSet	*wp ;
    WEAPON	*func ;
    ACTION	callback ;
    int		gm_weapon ;

    gm_weapon = GM_Weapon ;
    wp = &( PL_WeaponSets[ gm_weapon ] ) ;
    /* 新しい武器を起動 */
	//if ( GM_Weapon != work->weapon ) GM_Magazine = 0 ; /* タクティカルリロード対策 */
	if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
		if ( GM_Weapon != WP_None && 
			 GM_WeaponPrev != GM_Weapon ) GM_Magazine = 0 ;
	} else {
		if ( GM_Weapon != WP_None && 
			 GM_WeaponPrev != WP_None &&
			 GM_WeaponPrev != GM_Weapon ) GM_Magazine = 0 ;
	}
    if ( ( func = wp->func ) != NULL ) {
		/* 設置タイプはトリガーが別 */
		if ( wp->type & WP_TYPE_STICK ) {
			act = ( *func )( &( work->control ), &( work->weapon_body ), 
							 &( work->weapon_unit ), ( int * )&work->stick_wp_work, ENEMY_SIDE ) ; 
		} else {
			act = ( *func )( &( work->control ), &( work->weapon_body ), 
							 &( work->weapon_unit ), &( work->trigger ), ENEMY_SIDE ) ; 
		}
		work->trigger = TRIG_YET ;	/* トリガーリセット */
    } else {
		act = NULL ;
    }
    if ( act != NULL || func == NULL ) {
		if ( wp->action != NULL ) {
			LeaveCaution( work ) ;
			SetMode( work, wp->action ) ;
		} else if ( work->wp_set != NULL && 
				   ( ( ( PL_WeaponSet * )work->wp_set )->type & 
					( WP_TYPE_CALLFUNC | WP_TYPE_MODE1FUNC ) ) ) {
			/* 前装備がＣＡＬＬＦＵＮＣタイプの時 */
			if ( !no_change ) {
				int	StillAct[] = { STAND_STILL, SQUAT_STILL, GROUND_STILL } ;
				LeaveCaution( work ) ;
				SetMode( work, StillMode[ work->stance ] ) ;
				work->act_name = StillAct[ work->stance ] ;
			} else {
				if ( !PL_Flag2( FLAG2_NO_TIMERESET_WHEN_CHANGE ) ) {
					work->time = 0 ;
					work->ftime_count = 0 ;
				}
			}
		} else {
			if ( !PL_Flag2( FLAG2_NO_TIMERESET_WHEN_CHANGE ) ) {
				work->time = 0 ;
				work->ftime_count = 0 ;
			}
		}
		/* 主観装備だったら抜ける */
		if ( ( WeaponType( work ) & WP_TYPE_SUBJECT ) &&
			!( wp->type & WP_TYPE_SUBJECT ) ) {
			LeaveSubject( work ) ;
		}
		/* 前装備解除コールバック */
		if ( work->wp_set != NULL && 
			( callback = ( ( PL_WeaponSet * )work->wp_set )->callback ) != NULL ) {
			( *callback )( work, 0 ) ;
		}
		work->weapon = GM_Weapon ;
		work->wp_act = act ;
		work->wp_set = wp ;
		GM_WeaponChanged = 0 ;
		ClearCaptureTarget( work ) ;
		SetMode2( work, NULL ) ;
		SetArmAction( work, AMstand ) ;
		work->arm_interp = 0 ;
		UnsetWeaponCameraQuick( work ) ;
		GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;
		GM_CallCallbackProc( GM_CALLBACK_WEAPON ) ;
    }
}

/* 武器変更チェック */
static	void	CheckChangeWeapon( work ) 
Work		*work ;
{
    int		no_change ;

    /* 主観時武器替え不可 */
    if ( Status( PLAYER_WATCH ) &&
		!( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) &&
		!( PL_ItemType( work ) & IT_TYPE_SUBJECT ) ) {
		/* 弾なし時は強制素手チェンジするように */
		if ( GM_WeaponNum( work->weapon ) <= 0 &&
			( WeaponType( work ) & WP_TYPE_ZERO_TO_NONE ) ) {
			/* 主観でも変える */
		} else {
			SetStatus( PLAYER_WEAPON_DISABLE ) ;
		}
    }
	/* 特殊アイテム時武器替え不可 */
	if ( PL_ItemType( work ) & ( IT_TYPE_SUBJECT | IT_TYPE_ABNORMAL ) ) {
		SetStatus( PLAYER_WEAPON_DISABLE ) ;
	}

	/* 絶対無敵時変更不可 */
	if ( Status( PLAYER_INVINCIBLE ) && 
		 work->invincible_time == 0 ) {
		SetFlag( FLAG_CANNOT_CHANGE_WEAPON ) ;
	}

    no_change = 0 ;

    /* ビハインド時クイックのみ可 */
    if ( Status( PLAYER_BEHIND | PLAYER_CAUTION ) ) {
		if ( !Flag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK ) ) {
			SetStatus( PLAYER_WEAPON_QUICK_ONLY ) ;
			no_change = 1 ;
		} else {
			if ( Flag( FLAG_BEHIND_ATTACK ) ) {
				/* 飛び出し撃ち特殊 */
				SetStatus( PLAYER_WEAPON_QUICK_ONLY ) ;
				no_change = 0 ;
			} else {
				SetStatus( PLAYER_WEAPON_DISABLE ) ;
			}
		}
    }

	/* シナリオで変えられた */
	if ( GM_WeaponChanged == 0 &&
		 GM_Weapon != work->weapon ) {
		GM_WeaponPrev = work->weapon ;
		GM_WeaponChanged = 1 ;
	}

	if ( GM_WeaponPrevChangedScn >= 0 ) {
		GM_WeaponPrev = GM_WeaponPrevChangedScn ;
		GM_WeaponPrevChangedScn = -1 ;
	}

    if ( GM_WeaponChanged == 0 ||
		Status( PLAYER_WEAPON_DISABLE ) ||
		Flag2( FLAG2_CANNOT_CHANGE_WEAPON2 ) ||
		Flag( FLAG_CANNOT_CHANGE_WEAPON ) ) {
		/* 自分が死んでいないＧａｍｅＯｖｅｒ時は、
		   素手になら変更できる */
		if ( GM_IsGameOver() &&
			 GM_WeaponChanged == 1 && 
			 !Status( PLAYER_DEAD ) && 	
			 GM_Weapon == WP_None ) {
			printf( "raiden/event.c : gameover sude change OK!\n" ) ;
		} else {
			return ;
		}
	}

	/* マガジンもの以外は同じならreturn */
	if ( work->weapon == GM_Weapon &&
		 !( GM_WeaponTypes[ GM_Weapon ] & WP_TYPE_MAGAZINE ) ) {
		GM_WeaponChanged = 0 ;
		return ;
	}

    /* 以前の武器を破棄 */
    if ( work->wp_act != NULL ) {
		GV_DestroyOtherActorQuick( work->wp_act ) ;
		work->wp_act = NULL ;
		GM_ResetWeaponFire() ;	/* 発射フラグリセット */
    } 
    SetWeapon( work, no_change ) ;
    SetMotionSet( work->weapon ) ;

	/* 飛び出し撃ち特殊 */
	if ( Flag( FLAG_BEHIND_ATTACK ) ) {
		SetStatus( PLAYER_NEED_NEW_PRESS ) ;
	}
}

/*---------------------------------------------------------------------------*/

/* アイテムチェンジ関連 */

/*---------------------------------------------------------------------------*/

static	void	SetItem( work, it, no_change )
Work		*work ;
PL_ItemSet		*it ;
int		no_change ;
{
    void	*act ;
    ITEM	*func ;
    ACTION	callback ;
    int		prev, gm_item, callfunc = 0 ;

	prev = work->item ;
    gm_item = GM_Item ;
	//    it = &( PL_ItemSets[ gm_item ] ) ;
    /* 新しいアイテムを起動 */
    if ( ( func = it->func ) != NULL ) {
		work->item_unit = it->unit ;
		act = ( *func )( &( work->control ), &( work->item_body ), 
						&( work->item_unit ), &( work->it_trg ) ) ;
		if ( act == NULL ) {
			printf( "raiden/event.c : [%d] new item %d start failed!!\n", GV_Time, GM_Item ) ;
		}
    } else {
		act = NULL ;
    }
    if ( act != NULL || func == NULL ) {
		if ( it->action != NULL ) {
			LeaveCaution( work ) ;
			SetMode( work, it->action ) ;
			if ( it->action == CB_BoxStop ) SetStatus( PLAYER_MOVE ) ;	
			callfunc = 1 ;
		} else if ( work->it_set != NULL && 
				   ( ( PL_ItemSet * )work->it_set )->type & IT_TYPE_CALLFUNC ) {
			/* 前装備がＣＡＬＬＦＵＮＣタイプの時 */
			if ( !no_change ) {
				LeaveCaution( work ) ;
				SetMode( work, StillMode[ work->stance ] ) ;
				callfunc = 1 ;
			}
		} else {
//			work->time = 0 ;
//			work->ftime_count = 0 ;
		}
		/* 主観装備だったら抜ける */
		if ( ( ItemType( work ) & IT_TYPE_SUBJECT ) &&
			!( it->type & IT_TYPE_SUBJECT ) ) {
			LeaveSubject( work ) ;
		}
		/* 前装備解除コールバック */
		if ( work->it_set != NULL && 
			( callback = ( ( PL_ItemSet * )work->it_set )->callback ) != NULL ) {
			( *callback )( work, 0 ) ;
		}
		work->item = GM_Item ;
		work->it_act = act ;
		work->it_set = it ;
		if ( callfunc ) {
			ClearCaptureTarget( work ) ;
			SetMode2( work, NULL ) ;
			SetArmAction( work, AMstand ) ;
		}

		/* ものによっては炎を消す処理 */
		if ( no_change == 0 && 
			( ( GM_ItemTypes[ prev ] | GM_ItemTypes[ GM_Item ] ) & IT_TYPE_FLAMEVANISH ) ) {
			int			mesg = 0 ;
			PL_SendMessage( GM_PLAYER_CHAR_BODY_FLAME, &mesg, 1 ) ;
		}
		/* 足跡消す処理 */
		if ( prev == IT_Uniform || GM_Item == IT_Uniform ) {
			PL_FootPrintWakeUp( work->foot_work ) ;
		}

		GM_CallCallbackProc( GM_CALLBACK_ITEM ) ;
    }
}

static	void	CheckChangeItem( work ) 
Work		*work ;
{
    PL_ItemSet	*it ;
    int		no_change ;

    /* 主観時アイテム替え不可 */
    if ( Status( PLAYER_WATCH ) && 
		!( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) &&
		!( PL_ItemType( work ) & IT_TYPE_SUBJECT ) ) {
        SetStatus( PLAYER_ITEM_DISABLE ) ;
    }

	/* 絶対無敵時変更不可 */
	if ( Status( PLAYER_INVINCIBLE ) && 
		 work->invincible_time == 0 ) {
		SetFlag( FLAG_CANNOT_CHANGE_ITEM ) ;
	}

    no_change = 0 ;
    /* ビハインド時クイックのみ可 */
    if ( Status( PLAYER_BEHIND | PLAYER_CAUTION ) ) {
		if ( !Flag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK ) ) {
			SetStatus( PLAYER_ITEM_QUICK_ONLY ) ;
			no_change = 1 ;
		} else {
			if ( Flag( FLAG_BEHIND_ATTACK ) ) {
				/* 飛び出し撃ち特殊 */
				if ( !( GM_ItemTypes[ GM_ItemPrev ] & IT_TYPE_CALLFUNC ) ) {
					SetStatus( PLAYER_ITEM_QUICK_ONLY ) ;
					no_change = 1 ;
				} else {
					SetStatus( PLAYER_ITEM_DISABLE ) ;
				}
			} else {	
				SetStatus( PLAYER_ITEM_DISABLE ) ;
			}
//			SetStatus( PLAYER_ITEM_DISABLE ) ;
		}
    }

	/* シナリオで変えられた */
	if ( GM_ItemChanged == 0 &&
		 GM_Item != work->item ) {
		GM_ItemPrev = work->item ;			
		GM_ItemChanged = 1 ;
	}

	if ( GM_ItemPrevChangedScn >= 0 ) {
		GM_ItemPrev = GM_ItemPrevChangedScn ;
		GM_ItemPrevChangedScn = -1 ;
	}

    if ( GM_Item == work->item ) return ;

    /* フラグが立っていてもCALLFUNCタイプでなければチェンジ出来る */
	/* 自分が死んだゲームオーバー中はダメ */
    it = &( PL_ItemSets[ GM_Item ] ) ;
    if ( ( Status( PLAYER_ITEM_DISABLE ) || 
		  Flag( FLAG_CANNOT_CHANGE_ITEM ) || 
		  Flag2( FLAG2_CANNOT_CHANGE_ITEM2 ) ) && 
		( ( GM_IsGameOver() && Status( PLAYER_DEAD ) ) || 
		  ( it->type & IT_TYPE_CALLFUNC ) ) ) return ;

    /* 以前のアイテムを破棄 */
    if ( work->it_act != NULL ) {
		GV_DestroyOtherActorQuick( work->it_act ) ;
		work->it_act = NULL ;
    }
    SetItem( work, it, no_change ) ;
}

/* 武器解除 */
static	void	NoWeapon( work )
Work			*work ;
{
    GM_WeaponChanged = 1 ;
    GM_Weapon = WP_None ;
	if ( work->weapon != WP_None ) GM_WeaponPrev = work->weapon ;
    if ( work->wp_act != NULL ) {
		GV_DestroyOtherActorQuick( work->wp_act ) ;
		work->wp_act = NULL ;
    } 
    SetWeapon( work, 1 ) ;
    SetMotionSet( work->weapon ) ;    
}

/* アイテム解除 */
static	void	NoEquip( Work *work )
{
    PL_ItemSet	*it ;

    GM_Item = IT_None ;
	if ( work->item != WP_None ) GM_ItemPrev = work->item ;
    it = &( PL_ItemSets[ GM_Item ] ) ;
    if ( work->it_act != NULL ) {
		GV_DestroyOtherActorQuick( work->it_act ) ;
		work->it_act = NULL ;
    }
    SetItem( work, it, 1 ) ;
    printf( "raiden/event.c : no equip!!\n" ) ;
}

/*---------------------------------------------------------------------*/

/* グローバル */
void	PL_SubjectTurn( PlayerWork *work )
{
	SubjectTurn( work ) ;
}

void	PL_SubjectPeep( PlayerWork *work )
{
	SubjectPeep( work, work->pad->status ) ;
}

void	PL_SeekTurn( PlayerWork *work )
{
	PadTo = work->pad->dir ;
	SeekTurn( work ) ;
}

void	PL_NoWeapon( PlayerWork *work )
{
	NoWeapon( work ) ;
}

void	PL_NoEquip( PlayerWork *work )
{
	NoEquip( work ) ;
}

int		PL_CheckIntrude( PlayerWork *work )
{
	return CheckIntrude( work ) ;
}

/* 特殊武器、アイテムだったら外す */
int		PL_UnequipSpecials( void )
{
	Work	*work = GM_PlayerWork ;
	ASSERT( work != NULL ) ;
	if ( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
	if ( PL_ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;	
	//if ( GM_WeaponTypes[ GM_Weapon ] & WP_TYPE_SUBJECT ) GM_Weapon = WP_None ;
	//if ( GM_ItemTypes[ GM_Item ] & IP_TYPE_SUBJECT ) GM_Item = IT_None ;
	return 1 ;
}


