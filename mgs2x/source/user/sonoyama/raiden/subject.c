/*
   subject.c 
   ライデン／通常移動関連
   
   2002/03/24 T.Morita
   $Id: subject.c,v 1.1.1.3 2002/11/19 11:51:03 Yoshizawa1 Exp $			
   */


/*-----------------------------主観移動のモード別--------------------------------*/

/* このモードは、右スティックを倒した分だけ銃が動き、横に対して振り切
   ると横回転する。マウスっぽさを出そうとしたが、さらに狙いにくい結果
   になる。要は、狙いを定めておくのが困難になった。

   SUBMV_STICK_MOVE　と併用不可能*/
//#define SUBMV_STICK_AIM


/* このモードは移動すると、だんだん縦回転が戻っていく。（下や上の向き
   過ぎで移動しないことを前提。）ある程度になると戻らない。

   SUBMV_STICK_AIM　と併用不可能*/
//#define SUBMV_AUTO_HORIZON


/* このモードは移動すると、だんだん縦回転が戻っていく。（下や上の向き
   過ぎで移動しないことを前提。）ある程度になると戻らない。

   SUBMV_STICK_AIM　と併用不可能*/
//#define SUBMV_TURN_ACCEL


/* このモードは、左スティックを前後移動＆左右回転、右スティックが上下
   左右回転する。*/
#define SUBMV_STICK_MOVE






#define SUBJECT_MARGIN_F 48.0f


#ifdef SUBMV_STICK_AIM
static int now_turn = 0 ;
#endif
/* 主観方向回転 */
static	void	SubjectMoveTurn( Work *work )
{
    int		status, turn, g_rot ;
    float	vstep, hstep ;
    int		vmax[ 2 ], hmax[ 2 ] ;
    u_char	*pressure ;
    short	*ac ;
    float	dx, dy ;
    float ax, ay ;

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
    g_rot = work->g_rot ;

    /* 主観移動モードのカメラ操作は右スティックで */
    if ( work->pad->analog_input & (GV_PAD_ANALOG_L_USE|GV_PAD_ANALOG_R_USE) ||
	 (work->pad->status & PAD_UDLR) ) {

	/* 主観系アイテムのときは,方向たるものすべてが反応する */
	if ( (WeaponType( work ) & WP_TYPE_SUBJECT) ||
	     (ItemType( work ) & IT_TYPE_SUBJECT) ) {
		float tmp ;

		tmp = ( float )work->pad->right_dx - 128.0F ;
		ax = tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;
		tmp = ( float )work->pad->right_dy - 128.0F ;
		ay = tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;

      //BP_INPUT - disable right stick pitch if using spray item since it's used for spray strength
      if( PlayerPad.enable )
      {
         // On Vita, we use square for spray and spray at full strength.
#if !defined(BP_VITA)
         if( work->weapon == WP_ColdSpray )
         {
            ay = 0;
         }
#endif
      }
      //BP_INPUT - disable right stick pitch is using spray item since it's used for spray strength

		tmp = ( float )work->pad->left_dy - 128.0F ;
		ay += tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;
		tmp = ( float )work->pad->left_dx - 128.0F ;
		ax += tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;

		if ( work->pad->left_dx>=128 - ANALOG_MARGIN_F &&
		     work->pad->left_dx<=128 + ANALOG_MARGIN_F &&
		     work->pad->left_dy>=128 - ANALOG_MARGIN_F &&
		     work->pad->left_dy<=128 + ANALOG_MARGIN_F ) {
		    if ( work->pad->status & PAD_R )
			ax =  110.0f ;
		    else if ( work->pad->status & PAD_L )
			ax = -110.0f ;
		    if ( work->pad->status & PAD_D )
			ay =  110.0f ;
		    else if ( work->pad->status & PAD_U )
			ay = -110.0f ;
		}

	} else if ( PL_SubjectMove==2 || PL_SubjectMove==3 ) { /* 将来的にとる */
		float tmp ;

		tmp = ( float )work->pad->right_dx - 128.0F ;
		ax = tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;
		tmp = ( float )work->pad->right_dy - 128.0F ;
		ay = tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;

		if ( status & PL_PAD_STOP_AIM ) {
		    if ( work->pad->left_dx>=128 - ANALOG_MARGIN_F &&
			 work->pad->left_dx<=128 + ANALOG_MARGIN_F &&
			 work->pad->left_dy>=128 - ANALOG_MARGIN_F &&
			 work->pad->left_dy<=128 + ANALOG_MARGIN_F ) 
      {

         // AS MCampbell - Disable dpad movement on Vita unless a controller tutorial is running.
         // Bug: MGSTWO-3269
         if ( GM_CheckGameStatus( STATE_PAD_DEMO ) )
         {
			   if ( work->pad->status & PAD_R )
			       ax =  110.0f ;
			   else if ( work->pad->status & PAD_L )
			       ax = -110.0f ;
			   if ( work->pad->status & PAD_D )
			       ay =  110.0f ;
			   else if ( work->pad->status & PAD_U )
			       ay = -110.0f ;
         }
		 }

		    tmp = ( float )work->pad->left_dy - 128.0F ;
		    ay += tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;
		}
		tmp = ( float )work->pad->left_dx - 128.0F ;
		ax += tmp<ANALOG_MARGIN_F && tmp>ANALOG_MARGIN_F ? 0.0f : tmp ;

	} else { /* 将来的にとる */
	
		ax = ay = 0.0f ;		
		if ( work->pad->right_dx<128 - ANALOG_MARGIN_F ||
		     work->pad->right_dx>128 + ANALOG_MARGIN_F ||
		     work->pad->right_dy<128 - ANALOG_MARGIN_F ||
		     work->pad->right_dy>128 + ANALOG_MARGIN_F ) 
      {
		    ax = ( float )work->pad->right_dx - 128.0F ;
		    ay = ( float )work->pad->right_dy - 128.0F ;
		} 
      else if ( work->pad->left_dx>=128 - ANALOG_MARGIN_F && work->pad->left_dx<=128 + ANALOG_MARGIN_F ) 
      {
         // AS MCampbell - Disable dpad movement on Vita unless a controller tutorial is running.
         // Bug: MGSTWO-3269
         if ( GM_CheckGameStatus( STATE_PAD_DEMO ) )
         {
		       if ( work->pad->status & PAD_R )
			   ax = 110.0f ;
		       else if ( work->pad->status & PAD_L )
			   ax =-110.0f ;
         }
		}

	} /* 将来的にとる */
    } else {
	ax = 0.0f ;
	ay = 0.0f ;
    }

    dx = -ax ;
    dx += dx>0.0F ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;
    dy =  ay ;
    dy += dy>0.0F ? -ANALOG_MARGIN_F : ANALOG_MARGIN_F ;

    /* 主観操作反転 */
    PL_ShukanReverse( &dy, NULL ) ;
	
#ifdef SUBMV_TURN_ACCEL //加速はとった
    /* 銃構え時は加速付き */

#if 1
    if ( work->action2 != ShootBullet &&
	 !( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) &&
	 !( PL_ItemType( work ) & IT_TYPE_SUBJECT ) ) {
#endif

	if ( dx != 0.0F ) {
	    if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
		switch( GM_PadDemoVersion ) {
		case 1 :
		case 2 :
      case 3 :
		    ac[ 0 ] += 4 ; /*8*/
		    break ;
		default :
		    ASSERT( 0 ) ;
		}
	    } else {
		ac[ 0 ] += 4 ; /* 8*/
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
		    ac[ 1 ] += 4 ; /*8*/
		    break ;
		default :
		    ASSERT( 0 ) ;
		}
	    } else {
		ac[ 1 ] += 4 ; /*8*/ 
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
    }

#endif /*SUBMV_TURN_ACCEL*/

    if ( BP_IsPAL()==TRUE )//BP_FRAMERATE - adjust for runtime PAL
    {
	   vstep *= 1.20F ; hstep *= 1.20F ;
    }

   vmax[ 0 ] = ( GM_SubjectVMaxTmp[ 0 ] != 0 ) ? GM_SubjectVMaxTmp[ 0 ] : GM_SubjectVMax[ 0 ] ;
	vmax[ 1 ] = ( GM_SubjectVMaxTmp[ 1 ] != 0 ) ? GM_SubjectVMaxTmp[ 1 ] : GM_SubjectVMax[ 1 ] ;
	hmax[ 0 ] = ( GM_SubjectHMaxTmp[ 0 ] != 0 ) ? GM_SubjectHMaxTmp[ 0 ] : GM_SubjectHMax[ 0 ] ;
	hmax[ 1 ] = ( GM_SubjectHMaxTmp[ 1 ] != 0 ) ? GM_SubjectHMaxTmp[ 1 ] : GM_SubjectHMax[ 1 ] ;


    if ( (ay >= SUBJECT_MARGIN_F || ay <= -SUBJECT_MARGIN_F) && !Flag( FLAG_CANNOT_SUBJECT_UD ) ) {

#ifdef SUBMV_STICK_AIM
		work->camdir.vx =
		  (ay>0.0f ? ay-SUBJECT_MARGIN_F : ay+SUBJECT_MARGIN_F)*6 ;
#else
		turn = ( int )( vstep * dy / VALUE_ADJ ) ;
		if ( turn == 0 && vstep != 0.0F && dy != 0.0F ) {
			turn = ( dy > 0.0F ) ? 1 : -1 ;
		}
		turn += work->camdir.vx ;
		if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
		else if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
		if ( turn < -1023 ) turn = -1023 ;
		else if ( turn > 1023 ) turn = 1023 ;
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

#endif
    } else {
		//	work->control.turn.vx = g_rot ;	
		if ( !Flag( FLAG_HOMING ) ) {
			turn = GM_CameraDir.vx ;
			if ( turn < vmax[ 1 ] + g_rot ) turn = vmax[ 1 ] + g_rot ;
			else if ( turn > vmax[ 0 ] + g_rot ) turn = vmax[ 0 ] + g_rot ;
#ifdef SUBMV_STICK_AIM
			work->camdir.vx = 0 ;
#else

#ifdef  SUBMV_AUTO_HORIZON
			if ( PL_SubjectMove==2 || PL_SubjectMove==3 ) { /* 将来的にとる */

			/*移動すると元に戻そうとする*/
			if ( work->pad->status & PAD_UDLR && !(work->pad->status & PL_PAD_WEAPON) ) {
				if ( turn > 80 ) turn -= 80 ;
				if ( turn <-80 ) turn += 80 ;
			}

			} /* 将来的にとる */
#endif
			work->camdir.vx = turn ;
#endif
		}
    }



    if ( (ax >= SUBJECT_MARGIN_F || ax <= -SUBJECT_MARGIN_F) && !Flag( FLAG_CANNOT_SUBJECT_LR ) ) {
#ifdef SUBMV_STICK_AIM
		if ( ax > 120 || ax < -120 ){
			now_turn -= ax/4 ;
			ax = ax>120 ? 120 : -120 ;
		}
		work->control.turn.vy = now_turn - 
		  (ax>0.0f ? ax-SUBJECT_MARGIN_F : ax+SUBJECT_MARGIN_F)*6 ;
#else
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
			work->control.turn.vy += turn * 2 ;
		}
#endif

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
		}
#ifndef CAMERA_TURN
		work->control.turn.vx = turn ;		
#else
		work->camdir.vx = turn ;
#endif
    } else {
		if ( !Flag( FLAG_HOMING ) ) {
#ifdef SUBMV_STICK_AIM
			work->control.turn.vy = now_turn ;
#else
			StopTurn( work ) ;
#endif
		}
    }
}

extern float BP_AdjustTick3(float);
#define STRAFE_MOVE (BP_AdjustTick3(60.0f))

/* 平行移動の処理関数 */
void SubjectMoveStep( Work *work )
{
    FVECTOR v ;

    v.vx = v.vz = 0.0f ;
    /* 主観移動モードのときに
       パッドのスティックの方向に移動させる */
    if ( PL_SubjectMove==3 || PL_SubjectMove==2 ) { /* 将来的にとる */

	    /*攻撃ボタンの時は進まない*/
	    if ( !(work->pad->status & PL_PAD_STOP_AIM) ) {
		
		if ( work->pad->left_dx<128 - ANALOG_MARGIN_F ||
		     work->pad->left_dx>128 + ANALOG_MARGIN_F ){
		    v.vz = -((float)work->pad->left_dy - 128.0f)*0.5f ;
		} else {

#if !defined(BP_VITA)
		    if ( work->pad->status & PAD_R )
			v.vx = STRAFE_MOVE ;
		    else if ( work->pad->status & PAD_L )
			v.vx =-STRAFE_MOVE ;
		    if ( work->pad->status & PAD_U )
			v.vz = STRAFE_MOVE ;
		    else if ( work->pad->status & PAD_D )
			v.vz =-STRAFE_MOVE ;
#else
         if ( work->pad->status & PAD_R && !work->pad->pressure[PAD_PRESS_R])
            v.vx = STRAFE_MOVE ;
         else if ( work->pad->status & PAD_L && !work->pad->pressure[PAD_PRESS_L])
            v.vx =-STRAFE_MOVE ;
         if ( work->pad->status & PAD_U && !work->pad->pressure[PAD_PRESS_U])
            v.vz = STRAFE_MOVE ;
         else if ( work->pad->status & PAD_D && !work->pad->pressure[PAD_PRESS_D])
            v.vz =-STRAFE_MOVE ;
#endif
		}
	    }

    }else { /* 将来的にとる */

	if ( work->pad->left_dx<128 - ANALOG_MARGIN_F ||
	     work->pad->left_dx>128 + ANALOG_MARGIN_F ){
		v.vx =  ((float)work->pad->left_dx - 128.0f)*0.5f ;
		v.vz = -((float)work->pad->left_dy - 128.0f)*0.5f ;
	} else {
#if !defined(BP_VITA)
	    if ( work->pad->status & PAD_U )
		v.vz = STRAFE_MOVE ;
	    else if ( work->pad->status & PAD_D )
		v.vz =-STRAFE_MOVE ;
#else
      if ( work->pad->status & PAD_U && !work->pad->pressure[PAD_PRESS_U])
         v.vz = STRAFE_MOVE ;
      else if ( work->pad->status & PAD_D && !work->pad->pressure[PAD_PRESS_D])
         v.vz =-STRAFE_MOVE ;
#endif
	}	

    } /* 将来的にとる */


	/* 最終的な移動幅を計算する */
	{
		float l ;

#define  ANGtoRAD(_a) ((_a)*M_PI/4096.0f)
		v.vw = v.vy = 0.0f ;
		_sceVu0ApplyMatrix( &v, &DG_Chanls[0].eye, &v ) ;
		l = bp_sqrtf( v.vx*v.vx + v.vz*v.vz ) ;   //BP_MATH - emulate PS2 sqrtf
		l = l>0.0f ? STRAFE_MOVE/l : 0.0f ;
		work->control.step.vy -= GRAVITY/2 ;
		work->control.step.vx = v.vx * l ;
		work->control.step.vz = v.vz * l ;
		SetFlag( FLAG_NO_MOTION_STEP ) ;
	}
}

