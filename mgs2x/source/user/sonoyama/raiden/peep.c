/*
   peep.c 
   ライデン／覗き込み関連
   
   2000/03/16 M.Sonoyama
   $Id: peep.c,v 1.1.1.3 2002/11/19 11:50:57 Yoshizawa1 Exp $
*/

//#define	PEEP_DEBUG

#ifdef PEEP_DEBUG	
#define		DP( a )		a
#else
#define		DP( a )
#endif

#define	PeepDir			workL->idata 
#define	PeepStance		workL->idata2
#define	PeepStartPos	workL->fv

#define	PEEP_FAST_TH		(160)

int gArm_PeekLAnalogMinValue = 2048 - 1024;
int gArm_PeekLAnalogMaxValue = 2048 - 256;
int gArm_PeekRAnalogMinValue = 2048 + 256;
int gArm_PeekRAnalogMaxValue = 2048 + 1024;

enum {
	PMstill = 0,
	PMside,
	PMshift,
	PMback_start,
	PMback,
	PMback_end,
	PMjump_start,
	PMjump_still,
	PMjump_end,
} ;

/* 覗き込みモーションリスト */
static	int	PeepStandLMotions[] = { 
	Mcaution, Mbehind_r, Mcaution_r, Mbehind_rb_start, Mbehind_rb,
	Mbehind_rb_end, Mbehind_at_l_s, Mready, Mbehind_at_l_e 
} ;

static	int	PeepStandRMotions[] = {
	Mcaution, Mbehind_l, Mcaution_l, Mbehind_lb_start, Mbehind_lb,
	Mbehind_lb_end, Mbehind_at_r_s, Mready, Mbehind_at_r_e 
} ;

static	int	PeepSquatLMotions[] = {
	Mcaution_squat, Mbehind_r_sq, Mcaution_r_sq, Mbehind_rb_start_sq, Mbehind_rb_sq,
	Mbehind_rb_end_sq, Mbehind_at_r_s_sq, Mready, Mbehind_at_r_e_sq
} ;

static	int	PeepSquatRMotions[] = {
	Mcaution_squat, Mbehind_l_sq, Mcaution_l_sq, Mbehind_lb_start_sq, Mbehind_lb_sq,
	Mbehind_lb_end_sq, Mbehind_at_l_s_sq, Mready, Mbehind_at_l_e_sq
} ;

static	int	*PeepMotions[] = {
	PeepStandLMotions, PeepStandRMotions,
	PeepSquatLMotions, PeepSquatRMotions
} ;

/* 覗き込みモーションセット */
static	void	SetPeepActionEx( Work *work, int action, int interp, float t )
{
	int			motion ;

	motion = MS.change[ PeepMotions[ PeepStance * 2 + PeepDir ][ action ] ] ;
	PL_SetAction4( work, motion, t, interp ) ;
}

#define	SetPeepAction( _w, _a, _i )	SetPeepActionEx( _w, _a, _i, 0.0F )

/* 飛び出し撃ち可能？ */
static	inline	int	EnableJumpOut( Work *work )
{
#if 1
	if ( ( MS.change[ PeepMotions[ PeepStance * 2 + PeepDir ][ PMjump_start ] ] != NO_ACT ) &&
		 ( BehindPtr != NULL ) &&
		 !( BehindPtr->flag & ( HZX_BEHIND_NO_LEFT_OUT << PeepDir ) ) ) return 1 ;
#else
	if ( ( WeaponType( work ) & WP_TYPE_BH_ATTACK ) &&
		 ( BehindPtr != NULL ) && 
		 !( BehindPtr->flag & ( HZX_BEHIND_NO_LEFT_OUT << PeepDir ) ) ) return 1 ;
#endif
	return 0 ;
}

/* 覗きこみボタンチェック */
static	inline	int	CheckPeepPad( work, mode )
Work			*work ;
int				mode ;
{
	int			pad ;
   int         pad_dir;

	pad = ( PeepDir == BEHIND_LEFT ) ? PL_PAD_PEEP_L : PL_PAD_PEEP_R ;
   pad_dir = (work->camdir.vy - work->pad->dir);
   if (pad_dir < 0)
      pad_dir = -pad_dir;

   if (mode)
   {
      if (work->pad->press & pad)
      {
         return 1;
      }
      else 
      {
         if (pad == PL_PAD_PEEP_L)
            return (pad_dir > gArm_PeekLAnalogMinValue && pad_dir < gArm_PeekLAnalogMaxValue);
         else
            return (pad_dir > gArm_PeekRAnalogMinValue && pad_dir < gArm_PeekRAnalogMaxValue);
      }
   }
   else
   {
      if (work->pad->status & pad)
      {
         return 1;
      }
      else 
      {
         if (pad == PL_PAD_PEEP_L)
            return (pad_dir > gArm_PeekLAnalogMinValue && pad_dir < gArm_PeekLAnalogMaxValue);
         else
            return (pad_dir > gArm_PeekRAnalogMinValue && pad_dir < gArm_PeekRAnalogMaxValue);
      }
   }

   return 0;
}

/* 飛び出し撃ちへ */
static	void	SetJumpOut( work )
Work			*work ;
{
	static ACTION Actions[] = {
		BehindAttackL, BehindAttackR, SquatBehindAttackL, SquatBehindAttackR
	} ;
	SetMode( work, Actions[ PeepStance * 2 + PeepDir ] ) ;
}

/*----------------------------------------------------------------

  ビハインド覗き込み

----------------------------------------------------------------*/

/* 覗き込み共通ルーチン */
static	void	CautionPeep( Work *work, int time )
{
    int			shift_enable ;
    int			peep_enable ;
    int			stance ;
    float		len, height ;
    FVECTOR		diff ;
	
	stance = work->stance ;
	height = ( stance == STAND ) ? 1248.0F : 748.0F ;
    SetFlag( FLAG_BEHIND_PEEP | FLAG_DONOT_CHECK_WATCH ) ;

	if ( stance == SQUAT ) SetFlag( FLAG_RECOVER_ENABLE ) ;

    if ( time == 0 ) {
		/* 主観不可 */
		GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
		LeaveSubject( work ) ;
		if ( work->data > 0 ) {
			/* モード＆モーション設定済 */
			/* グレネード投げから戻った時用 */
			work->idata = work->data ;
			goto peep_init_skip ;
		}
		shift_enable = CheckBehindCameraShift( PeepDir ) ;
		InitBehindPeepCamera( work ) ;
		len = BehindEdgeLen( work, PeepDir, &diff ) ;
		SetBehindAttackPosition( work, &diff, len, PeepDir ) ;

		/* フラグがセットしてある場合 */
		if ( shift_enable && BehindPtr != NULL ) {
			int		peep_flag ;

			peep_flag = BehindPtr->flag ;
			if ( PeepDir == BEHIND_LEFT ) peep_flag >>= 2 ;
			peep_flag &= ( HZX_BEHIND_RIGHT_PEEP_SIDE | HZX_BEHIND_RIGHT_PEEP_BACK ) ;
			if ( peep_flag != 0 ) {
				if ( peep_flag & HZX_BEHIND_RIGHT_PEEP_SIDE ) {
					/* 横覗き */
					work->data = 3 ;
					SetPeepAction( work, PMside, 6 ) ;						
				} else {
					if ( len <= BEHIND_PEEP_START_LEN ) {
						/* 後ろ覗きスタート */
						work->data = 0 ;
						SetPeepAction( work, PMback_start, 6 ) ;
						DG_COPY_VEC( &PeepStartPos, &work->control.mov ) ;
					} else {
						/* 横移動 */
						work->data = 4 ;
						SetPeepAction( work, PMshift, 6 ) ;
					}
				}
				goto peep_init_skip ;
			}
		}

		work->data = 0 ;
		work->idata = 0 ;
		work->data2 = CheckBehindPeepSegment4( work, PeepDir, BEHIND_PEEP_START_LEN, height ) ;

		if ( work->data2 == 0 ) {
			/* 壁なし */
			if ( shift_enable == 0 ) {
				/* カメラ設定なし */
				if ( len <= BEHIND_PEEP_START_LEN ) {
					/* すでに端、なにもしない */
					work->data = 5 ;
					SetPeepAction( work, PMstill, 6 ) ;
					DP( printf( "none\n" ) ; ) 
				} else {
					/* 横移動 */
					work->data = 4 ;
					SetPeepAction( work, PMshift, 6 ) ;
					DP( printf( "shift\n" ) ; )
				}
			} else {
				/* カメラ設定あり */
				/* 端でないときは、飛び出し位置指定無視 */
				if ( len > BEHIND_PEEP_START_LEN + 12.0F ) {
					printf( "[%d] zerozero %f\n", GV_Time, len ) ;
					work->behind_atk_mov.vw = 0.0F ;
				}
				work->data = 0 ;
				/* 後ろ覗きスタート */
				SetPeepAction( work, PMback_start, 6 ) ;
				//MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
				DG_COPY_VEC( &PeepStartPos, &work->control.mov ) ;
				DP( printf( "back\n" ) ; )
			}
		} else {
			/* 壁あり */
			if ( len <= BEHIND_PEEP_START_LEN ) {
				/* 既に端 */
				if ( shift_enable == 0 ) {
					/* カメラ設定なし なにもしない */
					work->data = 5 ;
					SetPeepAction( work, PMstill, 6 ) ;
					DP( printf( "none2\n" ) ; )
				} else {
					/* カメラ設定あり 横覗き */
					work->data = 3 ;
					SetPeepAction( work, PMside, 6 ) ;
					DP( printf( "side2\n" ) ; )
				}		
			} else {
#if 1
				/* 端まで行けば果たして覗けるのか */
				peep_enable = CheckBehindPeepSegment4( work, PeepDir, len, height ) ;
				if ( peep_enable ) { /* 覗けない */
					if ( shift_enable == 0 ) {
						/* カメラ設定なし、端までは行く、横移動 */
						work->data = 4 ;
						SetPeepAction( work, PMshift, 6 ) ;
//						/* カメラ設定なし、何もしない */
//						work->data = 5 ;
//						SetPeepAction( work, PMstill, 6 ) ;
						DP( printf( "none3\n" ) ; )
					} else {
						/* 横覗き */
						work->data = 3 ;
						SetPeepAction( work, PMside, 6 ) ;
						DP( printf( "side3\n" ) ; )
					}
				} else {
					/* 覗けそう 横移動 */
					work->data = 4 ;
					SetPeepAction( work, PMshift, 6 ) ;
					DP( printf( "shift3\n" ) ; )
				}
#else
				/* 端まではいこう */
				work->data = 4 ;
				SetPeepAction( work, PMshift, 6 ) ;
#endif
			}
		}
    }
peep_init_skip :
    switch ( work->data ) {
    case 0 :
		/* 後ろ覗き開始 */
		if ( PL_CheckMotionRate( work ) < 0.80F ) {
			if ( PeepDir == BEHIND_LEFT ) {
				if ( work->pad->pressure[ PAD_PRESS_L2 ] > PEEP_FAST_TH ) {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
				} else {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.00F ) ;
				}
			} else {
				if ( work->pad->pressure[ PAD_PRESS_R2 ] > PEEP_FAST_TH ) {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
				} else {
					MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.00F ) ;
				}
			}
		}
		/* グレネード装備時特殊、左に持ちかえる */
		if ( PeepDir == BEHIND_LEFT && 
			 ( WeaponType( work ) & WP_TYPE_GRENADE ) ) {
			if ( PL_MotionTime( work ) > 20 ) {
				work->trigger |= ( TRIG_MAG_LEFT_HAND << 16 ) ;
			}
		}

		if ( EndMotion( work ) ) {
			SetPeepAction( work, PMback, 6 ) ;
			work->data ++ ;
			/* 覗き込みカウンタ */
			/* 一定値以上になったら発見部位に頭追加 */
			work->data2 = 0 ; 
		}
		if ( work->ftime > BEHIND_PEEP_IN_TIME ) {
			ShiftBehindCamera( work, PeepDir ) ;
		}
		/* 武器ボタンで飛び出し */
		if ( ( work->pad->press & PL_PAD_WEAPON ) &&
			EnableJumpOut( work ) &&
			!CheckBehindAttackEnable( work, PeepDir ) ) {
			SetJumpOut( work ) ;
			return ;
		}
#if 0
		/* グレネード系投擲 */
		if ( ( work->pad->press & PL_PAD_WEAPON ) && 
			 ( WeaponType( work ) & WP_TYPE_GRENADE ) ) {
			/* 今は右だけ */
			if ( PeepDir == BEHIND_RIGHT ) {
				FVECTOR		mov, shift ;

				DG_COPY_VEC( &mov, &work->control.mov ) ;
				mov.vy = work->control.levels[ 0 ] + 250.0F ;
				GV_SetVec3( &shift, 500.0F, 0.0F, 0.0F ) ;
				if ( !CheckMovRotLenSegment( work->control.hzx_id, &mov, &shift, 
											 &work->control.rot, -500.0F, 
											 HZX_CHK_ALL, HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
											 HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) { 
					SetMode( work, ThrowGrenadeBehindR ) ;
					return ;
				}
			}
		}
#endif
		break ;
	case 1 :
		/* 後ろ覗き静止 */
		if ( work->ftime > BEHIND_PEEP_IN_TIME || work->idata != 0 ) {
			ShiftBehindCamera( work, PeepDir ) ;
		}
		if ( PeepDir == BEHIND_RIGHT ) SetFlag( FLAG_BEHIND_PEEP_R_MAX ) ;
		else						   SetFlag( FLAG_BEHIND_PEEP_L_MAX ) ;
		/* グレネード装備時特殊、左に持ちかえる */
		if ( PeepDir == BEHIND_LEFT && 
			 ( WeaponType( work ) & WP_TYPE_GRENADE ) ) {
			work->trigger |= ( TRIG_MAG_LEFT_HAND << 16 ) ;
		}
		if ( ++ work->data2 > 16 ) AddFindObj( HUMAN21_ATAMA ) ;
		/* 武器ボタンで飛び出し */
		if ( work->pad->press & PL_PAD_WEAPON &&
			 EnableJumpOut( work ) && 
			!CheckBehindAttackEnable( work, PeepDir ) ) {
			SetJumpOut( work ) ;
			return ;
		}
		/* グレネード系投擲 */
		if ( ( WeaponType( work ) & WP_TYPE_GRENADE ) &&
			 ( work->pad->press & PL_PAD_WEAPON ) && 
			 GM_WeaponNum( work->weapon ) > 0 ) {
			if ( PeepDir == BEHIND_RIGHT ) {
				SetMode( work, ThrowGrenadeBehindR ) ;
				return ;
			} else {
				SetMode( work, ThrowGrenadeBehindL ) ;
				return ;
			}
		}
		break ;
	case 2 :
		/* 後ろ覗き終わり */
		if ( work->ftime > BEHIND_PEEP_IN_TIME || work->idata != 0 ) {
			ShiftBehindCameraReturn( work ) ;
		}
		/* グレネード装備時特殊、左に持ちかえる */
		if ( PeepDir == BEHIND_LEFT && 
			 ( WeaponType( work ) & WP_TYPE_GRENADE ) ) {
			if ( PL_MotionTime( work ) < 14 ) {
				work->trigger |= ( TRIG_MAG_LEFT_HAND << 16 ) ;
			}
		}
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		{
			FVECTOR		v ;

			DG_COPY_VEC( &v, &work->control.mov ) ;
			GV_NearExp8VF( &v, &PeepStartPos, 3 ) ;
			_sceVu0SubVector( &v, &v, &work->control.mov ) ;
			work->control.step.vx = v.vx ;
			work->control.step.vz = v.vz ;
		}
		if ( EndMotion( work ) || 
			( !CheckPeepPad( work, 0 ) && !( work->pad->status & PAD_UDLR ) ) ) {
			UnsetFlag( FLAG_BEHIND_PEEP ) ;
			SetFlag( FLAG_NEED_NEWPRESS_PEEP ) ;
			if ( PeepStance == STAND ) SetMode( work, StandCautionStill ) ;
			else					   SetMode( work, SquatCautionStill ) ;
			GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;

#if 0 //  段差に対してもアタリを見るように T.Morita 2002.05.31
			{
				FVECTOR		v ;

				DG_COPY_VEC( &v, &PeepStartPos ) ;
				v.vy = work->control.mov.vy ;
				GM_ResetControlPosition( &work->control, &v ) ;
			}
//			GM_ResetControlPosition( &work->control, &PeepStartPos ) ;
#else
			work->control.step.vx = PeepStartPos.vx - work->control.mov.vx ;
			work->control.step.vz = PeepStartPos.vz - work->control.mov.vz ;
#endif
		}
		break ;
	case 3 :
		/* 横覗き */
		if ( work->ftime > BEHIND_PEEP_IN_TIME || work->idata != 0 ) {
			ShiftBehindCamera( work, PeepDir ) ;
		}
		break ;
	case 4 :
		/* 覗きポイントまで移動 */
		work->time = 0 ;
		work->data = 4 ;
#if 1
		/* めりこみチェック */
		{
			int	res ;

			res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 750.0F ) ;
			if ( res & 1 ) {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				GV_NearExp4VF( &work->control.step, &LeftAdj, 3 ) ;
			} else if ( res & 2 ) {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				GV_NearExp4VF( &work->control.step, &RightAdj, 3 ) ;
			}
		}
#endif
		break ;
	case 5 :
		/* シフトカメラ無し何もしない */
	  ;
	}
		
	if ( !( CheckPeepPad( work, 0 ) ) || !Status( PLAYER_BEHIND ) ) {
#if 0
		if ( work->data == 0 || work->data >= 3 ) {
			UnsetFlag( FLAG_BEHIND_PEEP ) ;
			SetFlag( FLAG_NEED_NEWPRESS_PEEP ) ;
			if ( PeepStance == STAND ) SetMode( work, StandCautionStill ) ;
			else					   SetMode( work, SquatCautionStill ) ;
			GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;
		} else if ( work->data == 1 ) {
			SetPeepAction( work, PMback_end, 6 ) ;
			work->data ++ ;
		}
#endif
		if ( work->data >= 3 ) {
			UnsetFlag( FLAG_BEHIND_PEEP ) ;
			SetFlag( FLAG_NEED_NEWPRESS_PEEP ) ;
			if ( PeepStance == STAND ) SetMode( work, StandCautionStill ) ;
			else					   SetMode( work, SquatCautionStill ) ;
			GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;
		} else if ( work->data == 0 ) {
			float			t ;

			t = ( work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
				 + work->body.m_ctrl->mt3_ctrl[ 0 ].motion_time_base )
				/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;			
			SetPeepActionEx( work, PMback_end, 6, 1.0F - t ) ;
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
			work->data = 2 ;
		} else if ( work->data == 1 ) {
			SetPeepAction( work, PMback_end, 6 ) ;
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
			work->data ++ ;
		}
	}	
	/* 変な事をしてる */
	if ( work->data == 4 ) work->data = -1 ;
}

static	void	StandCautionPeepL( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_LEFT ;
	PeepStance = STAND ;
	CautionPeep( work, time ) ;
}

static	void	StandCautionPeepR( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_RIGHT ;
	PeepStance = STAND ;
	CautionPeep( work, time ) ;
}

static	void	SquatCautionPeepL( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_LEFT ;
	PeepStance = SQUAT ;
	CautionPeep( work, time ) ;
}

static	void	SquatCautionPeepR( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_RIGHT ;
	PeepStance = SQUAT ;
	CautionPeep( work, time ) ;
}

/*----------------------------------------------------------------

  飛び出し撃ち

----------------------------------------------------------------*/

/* ビハインド攻撃最初のホーミング */
static	void	BehindHoming( work )
Work			*work ;
{
	int			ry, ty ;
	SVECTOR		adj ;

	if ( PL_MotionTime( work ) > 10 ) {
		ry = work->control.rot.vy ;
		ty = work->control.turn.vy ;
		work->control.rot.vy += 2048 ;
		Homing( work, ( int )work->sv.vy ) ;
		adj.vx = adj.vz = 0 ;
		adj.vy = GV_DiffDirS( work->control.rot.vy, work->control.turn.vy ) ;
		work->control.rot.vy = ry ;
		work->control.turn.vy = ty ;
		if ( work->homing != NULL ) {
			SetRotAdjust( work, &adj, HUMAN21_KOSHI ) ;
			if ( !EndMotion( work ) ) {
				work->turn_adjusts[ HUMAN21_ATAMA ].vx *= -1 ;
				work->turn_adjusts[ HUMAN21_MIGI_UDE1 ].vx *= -1 ;
				work->turn_adjusts[ HUMAN21_HIDARI_UDE1 ].vx *= -1 ;
			} else {
				work->rot_adjusts[ HUMAN21_ATAMA ].vx *= -1 ;
				work->rot_adjusts[ HUMAN21_MIGI_UDE1 ].vx *= -1 ;
				work->rot_adjusts[ HUMAN21_HIDARI_UDE1 ].vx *= -1 ;
			}
			work->sv.vy = 0 ;
		} else {
			/* 飛び出し位置指定があるときのみ */
			/* 斜め飛び出し撃ちチェック */
			if ( DG_FABS( work->behind_atk_mov.vw ) > 0.10F ) {			
				SVECTOR		rot ;
				FVECTOR		from, normal, diff ;
				FVECTOR		to = { 0.0F, 0.0F, 2000.0F } ;
				HZX_HZD		hzd ;
				float		ip ;

				DG_COPY_VEC( &from, &work->behind_atk_mov ) ;
				from.vy = work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] ;
				GV_SetVec3( &rot, 0, work->control.rot.vy + 2048, 0 ) ;
				DG_SetPos2( &from, &rot ) ;
				DG_PutVector( &to, &to, 1 ) ;
				if ( HZX_OnlineHazardCheck( work->control.hzx_id, &from, &to,
										    HZX_CHK_SEGMENT, 
										    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
										    0 ) ) {
					HZX_GetOnlineHazard( &hzd, NULL ) ;
					HZX_GetOnlineVector( &diff ) ;
					HZX_GetNormal( &normal, &hzd ) ;
					_sceVu0Normalize( &normal, &normal ) ;
					ip = _sceVu0InnerProduct( &diff, &normal ) ;
					_sceVu0ScaleVector( &normal, &normal, ip ) ;
					_sceVu0SubVector( &diff, &diff, &normal ) ;
					_sceVu0AddVector( &to, &from, &diff ) ;
					//NewPointView( &to, 32, 232, 32 ) ;
					adj.vx = adj.vz = 0 ;
					adj.vy = GV_DiffDirS( rot.vy, GV_VecDir2( &diff ) ) ;
					/* 大きすぎるときはやらない */
					if ( adj.vy > -768 && adj.vy < 768 ) {
						SetRotAdjust( work, &adj, HUMAN21_KOSHI ) ;
					}
				}
			}
		}
	}
}

/* ビハインド攻撃最初の一撃 */
static	void	BehindAttack1st( work )
Work			*work ;
{
	WeaponSet	*wpset ;
	ACTION		func ;

//	if ( work->data2 ) {
		wpset = ( WeaponSet * )work->wp_set ;
		ASSERT( wpset != NULL ) ;
		func = wpset->shoot ;		
		ASSERT( func != NULL ) ;		
		if ( !( wpset->type & WP_TYPE_MODE1FUNC ) ) SetMode2( work, func ) ;
//	}
	work->behind_atk_mov.vw = 0.0F ;
	if ( work->stance == STAND ) SetAction( work, MS.attack[ Mready ], 6 ) ;
	else {
		SetAction( work, MS.change[ Msquat ], 6 ) ;
		SetAction2( work, MS.attack[ Mready ], 6, 0x1ffe ) ;
	}
	/* 向きを保存 */
	work->data2 
		= work->control.turn.vy - work->rot_adjusts[ HUMAN21_KOSHI ].vy ;
	work->turn_adjusts[ HUMAN21_KOSHI ] = DG_ZeroSVector ;
	work->rot_adjusts[ HUMAN21_KOSHI ] = DG_ZeroSVector ;	
}

/* ビハインド戻り角度アジャストセット */
static	void	BehindReturnSetAdjust( work )
Work			*work ;
{
	SVECTOR		adj ;

	adj.vx = adj.vz = 0 ;
	adj.vy = GV_DiffDirS( work->data2, work->control.rot.vy ) ;
	work->turn_adjusts[ HUMAN21_KOSHI ] = adj ;
	work->rot_adjusts[ HUMAN21_KOSHI ] = adj ;
	work->body.m_ctrl->adjust_flag |= 1 << HUMAN21_KOSHI ;
	work->control.turn.vy = work->data2 ;
	work->control.rot.vy = work->data2 ;

	/* old_abs_rotsをアジャスト*/
	{
		extern void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 ) ;
		FVECTOR		quat, *old ;
		int			i ;

		GM_RotToQuat( &adj, &quat ) ;
		old = work->body.m_ctrl->old_abs_rots ;
		for ( i = work->body.m_ctrl->n_joints; i > 0; i -- ) {
			MT_QuatMul( old, &quat, old ) ;
			old ++ ;
		}
	}
	work->body.m_ctrl->merge_flag = 0x0000 ;
	work->body.m_ctrl->mt3_ctrl[ 0 ].merge_flag = 0x0000 ;
	work->body.m_ctrl->mt3_ctrl[ 1 ].merge_flag = 0x0000 ;
}

/* 飛び出し撃ち共通ルーチン */
static	void	BehindAttack( work, time )
Work			*work ;
int				time ;
{
	FVECTOR	mov ;
					
	SetFlag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK | FLAG_CANNOT_CHANGE ) ;
	SetFlag2( FLAG2_NO_TIMERESET_WHEN_CHANGE ) ;
	SetStatus( PLAYER_BEHIND_ATTACK ) ;
	if ( time == 0 ) {
		PL_LeaveSubject( work ) ;
		GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
		SetPeepAction( work, PMjump_start, 6 ) ;
		SetInvincible( work, 0 ) ;
		work->sv.vx = work->weapon ;
		work->sv.vy = 1 ;	/* ホーミング用 */
	}

	/* 武器が変更されてたら */
	if ( work->weapon != work->sv.vx ) {
		UnsetInvincible( work ) ;
		LeaveCaution( work ) ;
		if ( PeepStance == STAND ) SetMode( work, StandStill ) ;
		else					   SetMode( work, SquatStill ) ;
		SetMode2( work, NULL ) ;
		return ;
	}

	switch( work->data ) {
	case 0 :
		SetStatus( PLAYER_MOVE ) ;
		ShiftBehindCamera( work, PeepDir ) ;
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		if ( DG_FABS( work->behind_atk_mov.vw ) > 0.10F ) {
			/* behind_atk_mov.vw != 0.0Fのときはセットされた位置へ行く */
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			mov.vx = GV_NearExp16F( work->control.mov.vx, work->behind_atk_mov.vx ) ;
			mov.vz = GV_NearExp16F( work->control.mov.vz, work->behind_atk_mov.vz ) ;
			work->control.step.vx = mov.vx - work->control.mov.vx ;
			work->control.step.vz = mov.vz - work->control.mov.vz ;
		}
		if ( work->pad->press & PL_PAD_WEAPON ) work->data2 = 1 ;
		BehindHoming( work ) ;
		if ( EndMotion( work ) ) {
			SetInvincible( work, 32 ) ;
			BehindAttack1st( work ) ;
			work->data ++ ;
		}
		break ;
	case 1 :
		ShiftBehindCamera( work, PeepDir ) ;
		if ( work->counter == 0 && !Status( PLAYER_DAMAGED ) ) CheckAttack( work ) ;
		/* 攻撃終了で戻る */
		if ( work->action2 == NULL ) {
			if ( Status( PLAYER_DAMAGED ) ) {
				/* ダメージ */
				SetInvincible( work, 0 ) ;
				SetFlag( FLAG_NO_STEP ) ;
				if ( GV_Time & 1 ) SetAction( work, MS.shared[ Mdamage_f ], 6 ) ;
				else 			   SetAction( work, MS.shared[ Mdamage_b ], 6 ) ; 
				UnsetStatus( PLAYER_DAMAGED ) ;
				work->counter = 1 ;
				work->data = 3 ;
			} else {
				/* 主観中なら直接立ち移行 */
				/* ゆっくり離しのときも */
				if ( Status( PLAYER_WATCH ) ||
					work->post & MAIL_ATTACK_CANCEL ) {
					LeaveCaution( work ) ;
					if ( PeepStance == STAND ) SetMode( work, StandStill ) ;
					else					   SetMode( work, SquatStill ) ;
					if ( work->post & MAIL_ATTACK_CANCEL ) {
						SetStatus( PLAYER_NEED_NEW_PRESS ) ;
						work->post &= ~MAIL_ATTACK_CANCEL ;		    
					}
					work->control.turn.vy = work->control.rot.vy 
						= GM_CameraDir.vy ;
				} else {
					work->data ++ ;
					SetMode2( work, NULL ) ;
					SetPeepAction( work, PMjump_end, 6 ) ;
					SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
					/* 主観中の回転をリセット */
					BehindReturnSetAdjust( work ) ;
					SetInvincible( work, 48 ) ;
				}
				return ;
			}
		} else if ( Status( PLAYER_WATCH ) ) {
			/* 覗きボタンはもう押されているので
			   覗き込みは無し */
			SetFlag( FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT ) ;
			SubjectTurn( work ) ;
		} 
		/* 構えモード８フレーム経過までホーミングはここでやる */
		if ( work->ftime2 < 8 ) Homing( work, work->sv.vy ) ;
		/* 覗き再押しで立ち移行 */
#if 0
		if ( CheckPeepPad( work, 1 ) ) {
			LeaveCaution( work ) ;
			SetMode2( work, NULL ) ;
			if ( PeepStance == STAND ) SetMode( work, StandStill ) ;
			else					   SetMode( work, SquatStill ) ;
			GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
		}
#endif
		/* このときだけ武器チェンジ（Ｑｕｉｃｋのみ）可能 */
		UnsetFlag( FLAG_CANNOT_CHANGE_WEAPON ) ;
		break ;
	case 2 :
		ShiftBehindCameraReturn( work ) ;
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		/* Ｍ９特殊 */
		if ( work->weapon == WP_m92 && work->motion2 != -1 ) {
			int		now, total ;

			now = MotionTime( work ) ;
			//total = ( int )( work->body.m_ctrl->mt3_ctrl->motion_total_time 
			//				 / work->body.m_ctrl->mt3_ctrl->motion_time_base ) ;
			total = ( int )( work->body.m_ctrl->mt3_ctrl->motion_total_time 
							/ ( float )NTSC_TIME_BASE ) ;
			if ( now >= total - 7 ) SetAction2( work, -1, 6, 0x01ffe ) ;
		}
		if ( EndMotion( work ) ) {
			UnsetFlag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK ) ;
			if ( work->pad->status & PAD_UDLR ) {
				SetPeepAction( work, PMstill, 0 ) ;
				work->control.turn.vy = WallTo + 2048 ;
				work->control.rot.vy = work->control.turn.vy ;
				SetFlag( FLAG_NEED_NEWPRESS_PEEP ) ;
				if ( PeepStance == STAND ) SetMode( work, StandCautionStill ) ;
				else					   SetMode( work, SquatCautionStill ) ;
			} else {
				LeaveCaution( work ) ;
				if ( PeepStance == STAND ) SetMode( work, StandStill ) ;
				else					   SetMode( work, SquatStill ) ;
			}
			GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;
		}
		break ;
	case 3 :	/* ダメージ */
		/* ダメージチェックで張り付きを解除されているので復活させる */
		SetStatus( PLAYER_CAUTION ) ;
		work->r_sphere = CAUTION_SPHERE ;

		ShiftBehindCamera( work, PeepDir ) ;
		SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_NO_STEP ) ;
		if ( EndMotion( work ) ) {
			if ( work->stance == STAND ) SetAction( work, MS.attack[ Mready ], 6 ) ;
			else {
				SetAction( work, MS.change[ Msquat ], 6 ) ;
				SetAction2( work, MS.attack[ Mready ], 6, 0x1ffe ) ;
			}
			SetInvincible( work, INVINCIBLE_TIME ) ;
			work->data = 1 ;
		}
	}
}

static	void	BehindAttackL( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_LEFT ;
	PeepStance = STAND ;
	BehindAttack( work ,time ) ;
}

static	void	BehindAttackR( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_RIGHT ;
	PeepStance = STAND ;
	BehindAttack( work ,time ) ;
}

static	void	SquatBehindAttackL( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_LEFT ;
	PeepStance = SQUAT ;
	BehindAttack( work ,time ) ;
}

static	void	SquatBehindAttackR( work, time )
Work			*work ;
int				time ;
{
	PeepDir = BEHIND_RIGHT ;
	PeepStance = SQUAT ;
	BehindAttack( work ,time ) ;
}

/* グレネードビハインド投擲 */
static	void	ThrowGrenadeBehindR( Work *work, int time )
{
	int			force ;
	int			mask ;

	SetFlag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK | FLAG_CANNOT_CHANGE ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_MENU_DISABLE | PLAYER_HOLD ) ;
	/* ダメージ処理の関係で、BEHIND_ATTACK ステータスにはしない */
	ShiftBehindCamera( work, BEHIND_RIGHT ) ;

	mask = ( work->stance == STAND ) ? 0xffffff : 0x01ffe ;

	if ( time == 0 ) {
		GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
		LeaveSubject( work ) ;
		if ( work->weapon != WP_Magazine ) {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start_bhl ], 0, 0.0F, 6, mask ) ;
		} else {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start_bhl ], -1, 0.80F, 6, mask ) ;
			work->trigger = TRIG_SET ;
		}
		if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
		work->data3 = work->data4 = 0 ;
		work->counter = 0 ;
	}	

	/* パッドリリースされたら、なかったことにする */
	if ( !PL_PadEnable() || GM_CheckPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ) {
		if ( work->stance == STAND ) SetMode( work, StandCautionPeepR ) ;
		else						 SetMode( work, SquatCautionPeepR ) ;
		work->data = 1 ; /* 後ろ覗き静止へ */
		if ( GM_WeaponNum( work->weapon ) <= 0 ) {
			NoWeapon( work ) ;
		}
		work->motion1 = -1 ;
		SetAction2( work, -1, 6, mask ) ;
		SetPeepAction( work, PMback, 6 ) ;
		return ;
	}

	if ( mask & 1 ) SetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;

	switch( work->data3 ) {
	case 0 :
		if ( work->weapon != WP_Magazine && 
			 PL_CheckMotionTime2( work, 9 ) ) work->trigger = TRIG_SET ;
		if ( EndMotion2( work ) ) {
			PL_SetAction2( work, MS.attack[ Mgm_5pose_bhl ], 6, mask ) ;
			if ( Status( PLAYER_SQUAT ) ) {
				work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			}
			work->data3 = 1 ;
		}
		break ;
	case 1 :
		force = ( work->pad->pressure[ PL_PAD_PRESS_WEAPON ] - 140 ) ;
		force /= ( 114 / 4 ) ;
		if ( force < 0 ) force = 0 ;
		else if ( force > 4 ) force = 4 ;
		if ( force > work->data4 ) {
			work->data4 = force ;
            work->counter = 6 ;
		}
		PL_SetAction2Ex( work, MS.attack[ Mgm_5pose_bhl ], work->data4, 0.0F, work->counter, mask ) ;
		work->body.m_ctrl->interp_flag = 0xffffff ;
		if ( work->counter > 0 ) work->counter = 0 ; 

		if ( !( work->pad->status & PL_PAD_WEAPON ) ) {
			if ( work->data4 < 2 ) {
				SetAction2( work, MS.attack[ Mgm_min_bhl ], 6, mask ) ;
			} else {
				SetAction2( work, MS.attack[ Mgm_max_bhl ], 6, mask ) ;
			}
			if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->data3 = 2 ;
		}
		break ;
	case 2 :
		if ( PL_CheckMotionTime2( work, 26 ) ) {
			work->trigger = TRIG_FIRE_LEVEL0 + work->data4 ;
		}
		if ( EndMotion2( work ) ) {
			if ( work->stance == STAND ) SetMode( work, StandCautionPeepR ) ;
			else						 SetMode( work, SquatCautionPeepR ) ;
			work->data = 1 ; /* 後ろ覗き静止へ */
			if ( GM_WeaponNum( work->weapon ) <= 0 ) {
				NoWeapon( work ) ;
			}
			work->motion1 = -1 ;
			SetAction2( work, -1, 6, mask ) ;
			SetPeepAction( work, PMback, 6 ) ;
		}
	}
}

static	void	ThrowGrenadeBehindL( Work *work, int time )
{
	int			force ;
	int			mask ;

	SetFlag( FLAG_BEHIND_PEEP | FLAG_BEHIND_ATTACK | FLAG_CANNOT_CHANGE ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_MENU_DISABLE | PLAYER_HOLD ) ;
	/* ダメージ処理の関係で、BEHIND_ATTACK ステータスにはしない */
	ShiftBehindCamera( work, BEHIND_LEFT ) ;

	mask = ( work->stance == STAND ) ? 0xffffff : 0x01ffe ;

	if ( time == 0 ) {
		GM_SetMenuStatus( MENU_MENU_NEWPRESS ) ;
		LeaveSubject( work ) ;	
		if ( work->weapon != WP_Magazine ) {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start_bhr ], 0, 0.0F, 6, mask ) ;
		} else {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start_bhr ], -1, 0.80F, 6, mask ) ;
			work->trigger = TRIG_SET ;
		}
		if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
		work->data3 = work->data4 = 0 ;
		work->counter = 0 ;
	}	

	/* パッドリリースされたら、なかったことにする */
	if ( !PL_PadEnable() || GM_CheckPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ) {
		if ( work->stance == STAND ) SetMode( work, StandCautionPeepL ) ;
		else						 SetMode( work, SquatCautionPeepL ) ;
		work->data = 1 ; /* 後ろ覗き静止へ */
		if ( GM_WeaponNum( work->weapon ) <= 0 ) {
			NoWeapon( work ) ;
		}
		work->motion1 = -1 ;
		SetAction2( work, -1, 6, mask ) ;
		SetPeepAction( work, PMback, 6 ) ;
		return ;
	}

	switch( work->data3 ) {
	case 0 :
		if ( work->weapon != WP_Magazine &&
			 PL_CheckMotionTime2( work, 9 ) ) work->trigger = TRIG_SET ;
		if ( EndMotion2( work ) ) {
			PL_SetAction2( work, MS.attack[ Mgm_5pose_bhr ], 6, mask ) ;
			if ( Status( PLAYER_SQUAT ) ) {
				work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			}
			work->data3 = 1 ;
		}
		break ;
	case 1 :
		force = ( work->pad->pressure[ PL_PAD_PRESS_WEAPON ] - 140 ) ;
		force /= ( 114 / 4 ) ;
		if ( force < 0 ) force = 0 ;
		else if ( force > 4 ) force = 4 ;
		if ( force > work->data4 ) {
			work->data4 = force ;
            work->counter = 6 ;
		}
		PL_SetAction2Ex( work, MS.attack[ Mgm_5pose_bhr ], work->data4, 0.0F, work->counter, mask ) ;
		work->body.m_ctrl->interp_flag = 0xffffff ;
		if ( work->counter > 0 ) work->counter = 0 ; 

		if ( !( work->pad->status & PL_PAD_WEAPON ) ) {
			if ( work->data4 < 2 ) {
				SetAction2( work, MS.attack[ Mgm_min_bhr ], 6, mask ) ;
			} else {
				SetAction2( work, MS.attack[ Mgm_max_bhr ], 6, mask ) ;
			}
			if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->data3 = 2 ;
		}
		break ;
	case 2 :
		if ( PL_CheckMotionTime2( work, 26 ) ) {
			work->trigger = TRIG_FIRE_LEVEL0 + work->data4 ;
		}
		if ( EndMotion2( work ) ) {
			if ( work->stance == STAND ) SetMode( work, StandCautionPeepL ) ;
			else						 SetMode( work, SquatCautionPeepL ) ;
			work->data = 1 ; /* 後ろ覗き静止へ */
			if ( GM_WeaponNum( work->weapon ) <= 0 ) {
				NoWeapon( work ) ;
			}
			work->motion1 = -1 ;
			SetAction2( work, -1, 6, mask ) ;
			SetPeepAction( work, PMback, 6 ) ;
			return ;
		}
	}

	/* 左手に持ってる */
	work->trigger |= ( TRIG_MAG_LEFT_HAND << 16 ) ;
}

