/*
   normal_sol.c
   ソリダス／静止、移動
   
   2001/03/22	M.Sonoyama
   $Id: normal_sol.c,v 1.1.1.3 2002/11/19 11:51:05 Yoshizawa1 Exp $
*/

/* 刀立ち静止 */
static	void	StandStillK( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_idle, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMkwt_idle, 32 ) ;
	}

	/* 間合いフェーズ中 */
	if ( work->flag & SOL_FLAG_CAREFUL ) {
		SetMode( work, StandStillC ) ;
		return ;
	}

	switch( work->data ) {
	case 0 :
		if ( work->len_diff_player < MAAI_LEN ) {
			SetAction( work, Mkwt_fire_ready, 0, 12 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_ready, 32 ) ;
			work->data = 2 ;
		}
		break ;
	case 1 :
		if ( work->len_diff_player > MAAI_LEN + 700.0F ) {
			SetAction( work, Mkwt_fire_end, 0, 12 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_end, 32 ) ;
			work->data = 3 ;
		}
		break ;
	case 2 :	/* 立ち ～ 構え */
		if ( EndMotion( work ) ) {
			SetAction( work, Mkwt_fire_idle, 0, 12 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_idle, 32 ) ;			
			work->data = 1 ;
		}	
		break ;
	case 3 :	/* 構え ～ たち */
		if ( EndMotion( work ) ) {
			SetAction( work, Mkwt_idle, 0, 12 ) ;
			SOL_SetSnakeArmMotion( AMkwt_idle, 32 ) ;
			work->data = 0 ;
		}			
		break ;
	}

	if ( !HeadToPlayer( work ) ) DirPlayer( work ) ;
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;

	/* 間合いフェーズとの切り替え */
	if ( CheckChangePhaseCareful( work ) ) {
		return ;
	}
#if 0
	/* 起動直後は止まっていよう */
	if ( GV_Time - work->wakeup_time < ( 300 / TIME_BASE ) * 1 ) return ;
#endif
	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}

	if ( CheckTriangleJump( work ) ) {
		SetMode( work, TriangleJump ) ;
		return ;
	}

	if ( work->len_diff_player > MAAI_LEN + 1200.0F || !MaaiCheck( work, -60 ) ) {
		/* ダメージ中 */
		if ( GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ) {
			SetMode( work, StandWalkK ) ;
			return ;
		}
		if ( ( GV_Time - work->wakeup_time > ( 300 / TIME_BASE ) * 3 ) &&
			 work->len_diff_player > MAAI_LEN + 1200.0F ) {
			SetMode( work, StandRunK ) ;
		} else {
			SetMode( work, StandWalkK ) ;
		}
		return ;
	}
	if ( Flag( work, SOL_FLAG_FAST_AWAY ) ) {
		work->far_think.missile_count = 0 ;
		SetMode( work, FastAway ) ;
		return ;
	}
	if ( CheckAttack( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
		return ;
	}
	if ( Flag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, ShotArmMissile ) ;
		}
		return ;
	}
	if ( Flag( work, SOL_FLAG_LIFTUP_ENABLE ) ) {
		SelectArmAttack( work ) ;
		return ;
	}
	if ( Flag( work, SOL_FLAG_ELBOW_ENABLE ) ) {
		//SetMode( work, Elbow ) ;
        if ( GV_Time & 1 ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, SlashThrust ) ;
		}
		return ;
	}
	if ( work->len_diff_player <= MAAI_LEN && PL_SlashNow() ) {
		SetMode( work, WalkBackK ) ;
	}
}

/* 刀立ち歩き */
static	void	StandWalkK( Work *work, int time )
{
	if ( time == 0 ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			SetAction( work, Mkwt_run, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_run, 32 ) ;
		} else {
			SetAction( work, Mkwt_walk, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_walk, 32 ) ;
		}
	}

	SetFlag( work, SOL_FLAG_NO_IK ) ;
	DirPlayer( work ) ;

	if ( ( ( GV_Time - work->wakeup_time > ( 300 / TIME_BASE ) * 3 ) &&
		  !GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) && 
		  !MaaiCheck( work, -60 ) ) ) {
		SetMode( work, StandRunK ) ;
		return ;
	}

	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}
	if ( CheckTriangleJump( work ) ) {
		SetMode( work, TriangleJump ) ;
		return ;
	}

	if ( work->mode != SOL_MODE_BLADE_ONLY &&
		 GV_Time - work->far_think.triangle_count > 10 * ( 300 / TIME_BASE ) &&
		 GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DOWNED ) &&
		 GM_PlayerPosition.vx > WALL_X - TRIANGLE_SHIFT - 4000.0F &&
		 work->control.mov.vz > 4000.0F && 
		 work->control.mov.vz < 18000.0F && 
		 DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) > 1250.0F && 
		 DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) < 2000.0F ) {
		SetMode( work, TriangleReady ) ;
		return ;
	}

	if ( CheckAttack( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
		return ;
	}
	if ( Flag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, ShotArmMissile ) ;
		}
		return ;
	}
	if ( Flag( work, SOL_FLAG_LIFTUP_ENABLE ) ) {
		SelectArmAttack( work ) ;
		return ;
	}
	if ( Flag( work, SOL_FLAG_ELBOW_ENABLE ) ) {
//		SetMode( work, Elbow ) ;
        if ( GV_Time & 1 ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, SlashThrust ) ;
		}
		return ;
	}
	if ( work->len_diff_player <= MAAI_LEN ) {
		SetMode( work, StandStillK ) ;
	}	
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
}

/* 刀立ち走り */
static	void	StandRunK( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_run, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMkwt_run, 32 ) ;
	}

	SetFlag( work, SOL_FLAG_NO_IK ) ;
	DirPlayer( work ) ;

	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}
	if ( CheckTriangleJump( work ) ) {
		SetMode( work, TriangleJump ) ;
		return ;
	}

	/* ダメージ中 */
	if ( GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) ) {
		SetMode( work, StandWalkK ) ;
		return ;
	}

	if ( work->len_diff_player <= MAAI_LEN ) {
		SetMode( work, StandStillK ) ;
		return ;
	}  
	if ( Flag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, ShotArmMissile ) ;
		}
		return ;
	}
	if ( CheckAttack( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
		return ;
	}
	if ( Flag( work, SOL_FLAG_LIFTUP_ENABLE ) ) {
		SelectArmAttack( work ) ;
		return ;
	}
	if ( Flag( work, SOL_FLAG_ELBOW_ENABLE ) ) {
//		SetMode( work, Elbow ) ;
        if ( GV_Time & 1 ) {
			SetMode( work, FastAway ) ;
		} else {
			SetMode( work, SlashThrust ) ;
		}
		return ;
	}
	/* 追いつけない */
	if ( work->ftime > DIRECT_TICK( 180 ) ) {
		work->far_think.missile_count = 0 ;
		SetMode( work, FastAway ) ;
	}
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
}

/* 後ずさり */
static	void	WalkBackK( Work *work, int time )
{
	FVECTOR		shift = { 0.0F, 0.0F, -750.0F } ;

	if ( time == 0 ) {
		SetAction( work, Mwalk_b, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMwalk_b, 32 ) ;
		MotionSpeed( work, 0.25F ) ;
	}	

	DirPlayer( work ) ;

	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD | SOL_FLAG_NO_IK ) ;	

	if ( CheckTriangleJump( work ) ) {
		SetMode( work, TriangleJump ) ;
		return ;
	}

	DG_SetPos( &work->body.objs->world ) ;
	DG_PutVector( &shift, &shift, 1 ) ;
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &shift,
							    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
		/* 背後に壁 */
		SetMode( work, FastAway ) ;
		return ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;
	}
}

/* 加速装置で離れる */
static	void	FastAway( Work *work, int time )
{
	int			arrived, fire_time ;

	if ( time == 0 ) {
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;
		MotionSpeed( work, 0.25F ) ;
		SetInvincible( work, 0 ) ;
		/* どこへ行くか */
		SetMovePosition( work ) ;
		work->fdata = 0.0F ;
printf( "away count %d\n", work->away_count ) ;
		if ( work->move_think.aim_len < 5000.0F ) {
			printf( "no way!!!\n" ) ;
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				work->away_count = 0 ;
				SetMode( work, FastAttack ) ;
				return ;
			}
//			SetMode( work, StandStillK ) ;
//			return ;
		}
		SeSet( KasokuSE[ irnd() % 2 + 1 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	}

	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			static int WayDir[] = { 0, 2048, -1024, 1024 } ;
			SetAction( work, Mdash_f + work->move_think.aim_way, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdash_f + work->move_think.aim_way, 32 ) ;
			work->control.turn.vy = work->move_think.aim_dir ;
			work->control.turn.vy += WayDir[ work->move_think.aim_way ] ;
			work->data = 1 ;
			work->idata = 0 ;
			
			if ( work->mode == SOL_MODE_BLADE_AND_SNAKEARM ) {
				fire_time = DIRECT_TICK( 240 ) ;
			} else {
				fire_time = DIRECT_TICK( 48 ) ;
			}
			if ( work->move_think.aim_way == 0 ||
				work->move_think.aim_way == 3 ) {
				DashFire( work, HUMAN21_MIGI_TSUMASAKI, fire_time ) ;
			} else {
				DashFire( work, HUMAN21_HIDARI_TSUMASAKI, fire_time ) ;
			}
		}
		break ;
	case 1 :
#if 0
		if ( ++ work->idata > 8 && ( work->ftime % 2 ) == 0 ) {
			extern void	*NewSolTraceFlame( FVECTOR *pos, int life ) ;
			FVECTOR		pos ;

			DG_COPY_VEC( &pos, &work->control.mov ) ;
			pos.vy = work->control.levels[ 0 ] ;

			GV_SetActorChild( work, NewSolTraceFlame( &pos, 120 ) ) ;
		}
#endif
		SetFlag( work, SOL_FLAG_NO_IK ) ;
	
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			work->fdata = GV_NearExp8F( work->fdata, 640.0F ) ;
		} else {
			work->fdata = GV_NearExp8F( work->fdata, 320.0F ) ;
		}

		if ( EndMotion( work ) && 
			 !( work->body.m_ctrl->mt3_ctrl[ 0 ].flag & MT3_SLEEP ) ) {
			printf( "1 loop end!!\n" ) ;
			MotionSleep( work, 0 ) ;
		}

		arrived = MovePosition( work, work->move_think.aim_dir, work->fdata ) ;
		if ( work->away_count == 0 && Flag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ) {
			DashFireStop( work ) ;
			MotionActive( work, 0 ) ;
			SetAction( work, Mdash_end_f + work->move_think.aim_way, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdash_end_f + work->move_think.aim_way, 32 ) ;
			MotionSpeed( work, 0.75F ) ;
			work->data2 = 1 ;
			work->data = 2 ;

			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				int			max_away ;

				max_away = ( CheckLife( work, 0.20F ) ) ? 3 : 2 ;
				if ( work->away_count >= max_away ) {
					work->away_count = 0 ;
					SetMode( work, FastAttack ) ;
				} else {
					if ( GV_Time & 5 ) {
						work->away_count ++ ;
						SetMode( work, FastAway ) ;
					} else { 
						SetMode( work, FastAttack ) ;
						work->away_count = 0 ;
					}
				}
				return ;
			}

			break ;
		} 
		if ( arrived ) {
			DashFireStop( work ) ;
			MotionActive( work, 0 ) ;
			SetAction( work, Mdash_end_f + work->move_think.aim_way, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdash_end_f + work->move_think.aim_way, 32 ) ;
			MotionSpeed( work, 0.75F ) ;
			work->data = 2 ;
			work->data2 = 0 ;

			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				int			max_away ;

				max_away = ( CheckLife( work, 0.20F ) ) ? 3 : 2 ;
				if ( work->away_count >= max_away ) {
					work->away_count = 0 ;
					SetMode( work, FastAttack ) ;
				} else {
					if ( GV_Time & 5 ) {
						work->away_count ++ ;
						SetMode( work, FastAway ) ;
					} else { 
						SetMode( work, FastAttack ) ;
						work->away_count = 0 ;
					}
				}
				return ;
			}
		}
		break ;
	case 2 :
//		if ( MotionRate( work ) > 0.85F ) DirPlayer( work ) ;
		if ( MotionRate( work ) < 0.35F ) {
#if 0
			if ( ( work->ftime % 4 ) == 0 ) {
				extern void	*NewSolTraceFlame( FVECTOR *pos, int life ) ;
				FVECTOR		pos ;

				DG_COPY_VEC( &pos, &work->control.mov ) ;
				pos.vy = work->control.levels[ 0 ] ;

				GV_SetActorChild( work, NewSolTraceFlame( &pos, 120 ) ) ;
			}
#endif
		}
		if ( EndMotion( work ) ) {
			ResetInvincible( work ) ;
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				int			max_away ;

				max_away = ( CheckLife( work, 0.20F ) ) ? 3 : 2 ;
				if ( work->away_count >= max_away ) {
					work->away_count = 0 ;
					SetMode( work, FastAttack ) ;
				} else {
					if ( GV_Time & 4 ) {
						work->away_count ++ ;
						SetMode( work, FastAway ) ;
					} else { 
						SetMode( work, FastAttack ) ;
						work->away_count = 0 ;
					}
				}
			} else {
				work->away_count = 0 ;
				if ( work->data2 == 0 ) {
					SetMode( work, StandStillK ) ;
				} else {
					SetMode( work, ShotArmMissile ) ;
				}
			}
		}
	} 
}

/* 三角飛び位置へ */
static	void	TriangleReady( Work *work, int time )
{
	FVECTOR		aim_pos ;

	if ( time == 0 ) {
		SetInvincible( work, 0 ) ;
		SetAction( work, Mkwt_run, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMkwt_run, 32 ) ;		
	}

	aim_pos.vx = WALL_X - TRIANGLE_SHIFT ;
	aim_pos.vz = work->control.mov.vz ;
	work->control.turn.vy = GV_VecDir2FromTo( &work->control.mov, &aim_pos ) ;

	if ( DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) < 500.0F ) {
		SetMode( work, TriangleJump ) ;
	}
}

/* 三角飛び */
static	void	TriangleJump( Work *work, int time )
{
	FVECTOR		aim_pos ;
	
	if ( time == 0 ) {
		SetInvincible( work, 0 ) ;
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;		
		work->far_think.triangle_count = GV_Time ;
	}

	SOL_SnakeArmSetFlowFlag( 1 ) ;
	work->energy_mode = 2;

	switch( work->data ) {
	case 0 :
		aim_pos.vx = WALL_X - TRIANGLE_SHIFT ;
		aim_pos.vz = work->control.mov.vz ;
		work->control.turn.vy = TRIANGLE_TURN ;
		SetFlag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ;
		work->control.step.vx = 0.0F ;
		work->control.step.vz = 0.0F ;
		work->control.mov.vx = GV_NearExp8F( work->control.mov.vx, aim_pos.vx ) ;
		work->control.mov.vz = GV_NearExp8F( work->control.mov.vz, aim_pos.vz ) ;
		if ( DG_FABS( work->control.mov.vx - aim_pos.vx ) < 32.0F ||
			 EndMotion( work ) ) {
			SetAction( work, Mtriangle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMtriangle, 32 ) ;		
			SetFlag( work, SOL_FLAG_NO_GRAVITY | SOL_FLAG_NO_IK ) ;
			work->control.skip_flag |= CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ;
			work->data = 1 ;
		}
		break ;
	case 1 :
		if ( CheckMotionTime( work, 32 ) ) {
			SeSet( SD_V_SOLKIA15, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
		if ( CheckMotionTime( work, 84 ) ) {
			SeSet( SD_V_SOLMUS01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}

		/* プレイヤー位置に */
		if ( CheckMotionTime( work, 88 ) ) {
			FVECTOR		fv1 ;

			DG_COPY_VEC( &fv1, &GM_PlayerPosition ) ;
			if ( fv1.vx < work->jump_bound1.vx ) {
				fv1.vx = work->jump_bound1.vx ;
			} else if ( fv1.vx > work->jump_bound2.vx ) {
				fv1.vx = work->jump_bound2.vx ;
			}
			if ( fv1.vz < work->jump_bound1.vz ) {
				fv1.vz = work->jump_bound1.vz ;
			} else if ( fv1.vz > work->jump_bound2.vz ) {
				fv1.vz = work->jump_bound2.vz ;
			}
			work->fv1.vx = ( fv1.vx - work->control.mov.vx ) / ( float )( 139 - 88 ) * 1.0F ;
			work->fv1.vz = ( fv1.vz - work->control.mov.vz ) / ( float )( 139 - 88 ) * 1.0F ;
			work->fv2.vx = fv1.vx ;
			work->fv2.vz = fv1.vz ;
		}
		if ( work->mtime1 >= 88 && work->mtime1 < 139 ) {
			int			dir ;

			SetFlag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ;
			work->control.step.vx = work->fv1.vx ;
			work->control.step.vz = work->fv1.vz ;
			dir = GV_VecDir2( &work->fv1 ) ;
			work->control.turn.vy = GV_NearExp16P( work->control.turn.vy, dir + 2048 ) ;
		}
		/* 床高さ調整 */
		if ( work->mtime1 >= 139 && work->mtime1 < 154 ) {
			float		diff, hadj ;

			diff = work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ;
			if ( diff > 0.0F ) diff = 0.0F ;
			hadj = TRIANGLE_DIFFH * ( diff / TRIANGLE_DIFFV ) - TRIANGLE_DIFFH ;
			work->control.mov.vy += hadj / ( float )( 154 - 139 ) ;
		}
		/* 範囲からでないように */
		if ( work->mtime1 > 120 ) {
			if ( work->control.mov.vx < work->jump_bound1.vx ) {
				work->control.mov.vx = work->jump_bound1.vx ;
			} else if ( work->control.mov.vx > work->jump_bound2.vx ) {
				work->control.mov.vx = work->jump_bound2.vx ;
			}
			if ( work->control.mov.vz < work->jump_bound1.vz ) {
				work->control.mov.vz = work->jump_bound1.vz ;
			} else if ( work->control.mov.vz > work->jump_bound2.vz ) {
				work->control.mov.vz = work->jump_bound2.vz ;
			}
		}

		if ( CheckMotionTime( work, 154 ) ) {
			/* ふみつけ */
			TARGET			*t, *t2 ;
			POWER_TARGET	*p, *p2 ;
			FVECTOR			force, size, size2 ;
			FVECTOR			shift = { 0.0F, 0.0F, 500.0F } ;
			int				damage = GM_VitalityMax / 4 ;

			t = &work->offense ;
			p = &work->pt_offense ;
			t2 = &work->offense2 ;
			p2 = &work->pt_offense2 ;
			GV_SetVec3( &size, 750.0F, 750.0F, 750.0F ) ;
			GV_SetVec3( &size2, 1500.0F, 1500.0F, 850.0F ) ;
			GM_SetTarget( t, TARGET_OFFENSE | TARGET_ROTATE, work->control.map, PLAYER_SIDE, &size, &shift ) ;
			GM_SetTarget( t2, TARGET_OFFENSE | TARGET_ROTATE, work->control.map, PLAYER_SIDE, &size2, &shift ) ;
			DG_SetPos2( &work->control.mov, &work->control.rot ) ;
			GV_SetVec3( &force, 0.0F, 0.0F, 32.0F ) ;
			DG_RotVector( &force, &force, 1 ) ;
			GM_SetTargetWeaponType( t, WP_GRENADE | WP_NOMUTEKI ) ;
			GM_SetTargetWeaponType( t2, WP_GRENADE | WP_NOMUTEKI ) ;

			/* 難易度別 */
			switch( SOL_GameLevel ) {
			case GM_LEVEL_VERYEASY :
				damage -= 10 ;
				break ;
			case GM_LEVEL_EASY :
				damage -= 5 ;
				break ;
			case GM_LEVEL_HARD :
				damage += 8 ;
				break ;
			case GM_LEVEL_EXTREME :
				damage += 12 ;
				break ;
			case GM_LEVEL_NORMAL :
			default :
			  ;
			}
			if ( damage <= 0 ) damage = 1 ;
			GM_SetPowerTarget( t, p, POWER_CONST, 255, 0, damage, &force ) ;
			GM_SetPowerTarget( t2, p2, POWER_CONST, 255, 0, damage, &force ) ;
			GM_MoveTarget2( t, &work->body.objs->objs[ HUMAN21_MIGI_TSUMASAKI ].world ) ;
			GM_MoveTarget2( t2, &work->body.objs->objs[ HUMAN21_MIGI_TSUMASAKI ].world ) ;
			GM_PutTarget( t ) ;
			GM_PutTarget( t2 ) ;

			{	
				FVECTOR		mov ;
				GV_MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TSUMASAKI ].world, &mov ) ;
				NewSolidusDunkSmoke( &mov, 1200.0F ) ;
			}
		}

		SetFlag( work, SOL_FLAG_NO_GRAVITY | SOL_FLAG_NO_IK | SOL_FLAG_NO_PUSH ) ;
		if ( work->mtime1 >= 156 ) {
			if ( !GM_CheckPlayerStatus( PLAYER_DOWNED | PLAYER_DAMAGED ) ) {
				ResetFlag( work, SOL_FLAG_NO_PUSH ) ;
			} 
			ResetInvincible( work ) ;
		}
		if ( EndMotion( work ) ) {
			work->control.skip_flag &= ~( CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ) ;
			work->control.skip_flag |= CTRL_RESET_HZX_BASE ;
			ResetInvincible( work ) ;

			if ( work->len_diff_player > 4000.0F ) {
				SetAction( work, Mmissle_ready, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMmissle_ready, 12 ) ;
				SetMode( work, ShotArmMissile ) ;
			} else {
				SetAction( work, Mdash_ready, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;				
				SetMode( work, FastAway ) ;
			}
			SetFlag( work, SOL_FLAG_NO_WAIST_INTERP ) ;
		}
	}
}
