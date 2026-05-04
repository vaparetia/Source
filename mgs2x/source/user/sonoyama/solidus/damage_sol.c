/*
   damage_sol.c
   ソリダス／ダメージ処理 

   2001/03/22	M.Sonoyama
   $Id: damage_sol.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
*/

/* スウェー上 */
static	void	SwayUpper( Work *work, int time )
{
	if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, Mguard_u, 0, 12 ) ;
        SOL_SetSnakeArmMotion( AMguard_u, 16 ) ;
		MotionSpeed( work, 1.50F ) ;
		SetInvincible( work, 0 ) ;
		DirPlayer( work ) ;
		work->fdata = -64.0F ;
		work->careful_time -= DIRECT_TICK( 60 ) ;
	}

	{
		FVECTOR			step ;
		int				diff ;

		GV_SetVec3( &step, 0.0F, 0.0F, work->fdata ) ;
		if ( work->control.n_touches ) {
			diff = GV_DiffDirAbs( work->control.turn.vy + 2048, 
								  GV_VecDir2( &work->control.vecs[ 0 ] ) ) ;
			if ( diff < 128 ) {
				work->control.turn.vy -= 512 ;
				step.vz *= 1.50F ;
			}
		} 
		DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
		DG_RotVector( &step, &step, 1 ) ; step.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	}
	work->fdata = GV_NearExp8F( work->fdata, 0.0F ) ;

	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;

	if ( work->mtime1 > 30 ) {
		ResetInvincible( work ) ;
		if ( CheckSway( work ) ) return ;
	}

	if ( EndMotion( work ) ) {
		ResetInvincible( work ) ;
		SetMode( work, StandStillC ) ;
	}
}

/* スウェー下 */
static	void	SwayLower( Work *work, int time )
{
	if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, Mguard_d, 0, 12 ) ;
        SOL_SetSnakeArmMotion( AMguard_d, 16 ) ;
		MotionSpeed( work, 1.50F ) ;
		SetInvincible( work, 0 ) ;
		DirPlayer( work ) ;
		work->fdata = -64.0F ;
		work->careful_time -= DIRECT_TICK( 60 ) ;
	}

	{
		FVECTOR			step ;
		int				diff ;

		GV_SetVec3( &step, 0.0F, 0.0F, work->fdata ) ;
		if ( work->control.n_touches ) {
			diff = GV_DiffDirAbs( work->control.turn.vy + 2048, 
								  GV_VecDir2( &work->control.vecs[ 0 ] ) ) ;
			if ( diff < 128 ) {
				work->control.turn.vy -= 512 ;
				step.vz *= 1.50F ;
			}
		} 
		DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
		DG_RotVector( &step, &step, 1 ) ; step.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	}
	work->fdata = GV_NearExp8F( work->fdata, 0.0F ) ;

	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;

	if ( work->mtime1 > 30 ) {
		ResetInvincible( work ) ;
		if ( CheckSway( work ) ) return ;
	}

	if ( EndMotion( work ) ) {
		ResetInvincible( work ) ;
		SetMode( work, StandStillC ) ;
	}
}

/* プレイヤーの刀防御 */
static	void	BladeGuard( Work *work, int time )
{
	static int Motions[] = { Mguard_l, Mguard_r, Mguard_w } ;
	static int ArmMotions[] = { AMguard_l, AMguard_r, AMguard_w } ;
	if ( time == 0 ) {
        work->motion1 = -1 ;
		SetAction( work, Motions[ work->guard_mode ], 16, 6 ) ;
        SOL_SetSnakeArmMotionEX( ArmMotions[ work->guard_mode ], 32, 1 ) ;
		SeSet( SD_V_SOLATK03, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		work->guard_hit = 0 ;
		work->fdata = -64.0F ;
	}

	switch( work->data ) {
	case 0 :
		if ( work->guard_hit ) {
			work->fdata = -64.0F ;
			work->guard_hit = 0 ;
			work->motion1 = -1 ;
			SetAction( work, Motions[ work->guard_mode ], 16, 6 ) ;
			SOL_SetSnakeArmMotionEX( ArmMotions[ work->guard_mode ], 32, 1 ) ;
			if ( ++ work->data2 > 2 ) work->data2 = 0 ;
			work->data = 1 ;
			PL_FillipSlash() ;
		}
		break ;
	case 1 :
		if ( work->guard_hit ) {
			work->fdata = -64.0F ;
			work->guard_hit = 0 ;
			if ( GV_Time - work->guard_last_time > 8 ) {
				work->guard_last_time = GV_Time ;
				work->motion1 = -1 ;
				SetAction( work, Motions[ work->guard_mode ], 16, 6 ) ;
				SOL_SetSnakeArmMotionEX( ArmMotions[ work->guard_mode ], 32, 1 ) ;
				if ( work->data2 == 2 ) {
					SeSet( ChouhatsuSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
				}

				if ( ++ work->data2 > 2 ) work->data2 = 0 ;
				PL_FillipSlash() ;
				/* はじきが成功したら少し無敵にしてみる */
				SetInvincible( work, 32 ) ;
			}
		}		
		break ;
	}

	work->control.turn.vy = work->rot_diff_player.vy ;
	{
		FVECTOR			step ;
		int				diff ;

		GV_SetVec3( &step, 0.0F, 0.0F, work->fdata ) ;
		if ( work->control.n_touches ) {
			diff = GV_DiffDirAbs( work->control.turn.vy + 2048, 
								  GV_VecDir2( &work->control.vecs[ 0 ] ) ) ;
			if ( diff < 128 ) {
				work->control.turn.vy -= 512 ;
				step.vz *= 1.50F ;
			}
		} 
		DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
		DG_RotVector( &step, &step, 1 ) ; step.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	}
	work->fdata = GV_NearExp8F( work->fdata, 0.0F ) ;

	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	work->control.turn.vy = work->rot_diff_player.vy ;

	if ( work->data != 0 && MotionRate( work ) > 0.75F ) {
		if ( CheckAttack( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
			return ;
		}
	}
	if ( EndMotion( work ) ) {
		if ( !CheckAttack( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
			if ( work->len_diff_player > 2000.0F ) {
				SetMode( work, StandStillK ) ;
			} else {
				SetMode( work, WalkBackK ) ;
			}
		}
	}
}

/* 刀攻撃はじかれ右 */
static	void	SlashFillipedR( Work *work, int time )
{
	if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, Mdam_filliped_r, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdam_filliped_r, 32 ) ;
		if ( GV_Time & 1 ) {
			SeSet( KawashiSE[ irnd() & 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else if ( GV_Time & 2 ) {
			SeSet( SD_V_SOLFRA01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else {
			SeSet( SD_V_SOLFRA02, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}
	if ( EndMotion( work ) ) {
		if ( work->len_diff_player > 2000.0F ) {
			SetMode( work, StandStillK ) ;
		} else {
			SetMode( work, WalkBackK ) ;
		}
	}	
}

/* 刀攻撃はじかれ左 */
static	void	SlashFillipedL( Work *work, int time )
{
	if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, Mdam_filliped_l, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdam_filliped_l, 32 ) ;
		if ( GV_Time & 1 ) {
			SeSet( KawashiSE[ irnd() & 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else if ( GV_Time & 2 ) {
			SeSet( SD_V_SOLFRA01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else {
			SeSet( SD_V_SOLFRA02, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}
	if ( EndMotion( work ) ) {
		if ( work->len_diff_player > 2000.0F ) {
			SetMode( work, StandStillK ) ;
		} else {
			SetMode( work, WalkBackK ) ;
		}
	}	
}

/* 刀ダメージ */
static	void	BladeDamage( Work *work, int time )
{
	if ( time == 0 ) {
		int		dir ;

		work->motion1 = -1 ;
		
		dir = GV_DiffDirS( work->control.rot.vy, work->rot_diff_player.vy ) ;
		if ( work->cont_hit_count < 2 ||
			 ( work->cont_hit_count == 2 && 
			   ( work->weapon_type & ( WP_PUNCHR | WP_PUNCHL ) ) ) ) {
			/* 一回目ＯＲパンチ */
			SetAction( work, Mkwt_dam_f, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_dam_f, 32 ) ;			
			work->data = 1 ;
		} else if ( -2048 + 341 > dir ) {
			/* 後方中央 */
			SetAction( work, Mdam_b_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_b_m, 32 ) ;
		} else if ( -1024 > dir ) {
			/* 後方右 */
			SetAction( work, Mdam_b_r, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_b_r, 32 ) ;
		} else if ( -1024 + 682 > dir ) {
			/* 前方右 */
			SetAction( work, Mdam_f_r, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_f_r, 32 ) ;
		} else if ( 0 + 341 > dir ) {
			/* 前方中央 */
			SetAction( work, Mdam_f_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_f_m, 32 ) ;
		} else if ( 1024 > dir ) {
			/* 前方左 */
			SetAction( work, Mdam_f_l, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_f_l, 32 ) ;
		} else if ( 1024 + 682 > dir ) {
			/* 後方左 */
			SetAction( work, Mdam_b_l, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_b_l, 32 ) ;
		} else {
			/* 後方中央 */
			SetAction( work, Mdam_b_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdam_b_m, 32 ) ;
		}
		SeSet( DamageSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 2 ) ;
		if ( work->mode == SOL_MODE_BLADE_AND_SNAKEARM &&
			 CheckArmUndoGo( work ) ) {
			/* 外しデモ前は無敵にする */
			SetInvincible( work, 0 ) ;
		}
	}
	if ( ( work->data == 1 && MotionRate( work ) > 0.75F ) ||
		 ( work->data == 0 && MotionRate( work ) > 0.50F ) ) {
		LeftSideNoGuard( work ) ;
	}
	if ( EndMotion( work ) ) {
		if ( work->mode == SOL_MODE_BLADE_AND_SNAKEARM &&
			 CheckArmUndoGo( work ) ) {	/* 2個所あります */
			ChangeMode( work, SOL_MODE_BLADE_ONLY ) ;
			SetMode( work, ArmUndoReady ) ;
			return ;
		}
		if ( work->len_diff_player > 2000.0F ) {
			SetMode( work, StandStillK ) ;
		} else {
			if ( ( irnd() % 16 ) < 4 ) SetMode( work, FastAway ) ;
			else					   SetMode( work, WalkBackK ) ;
		}
//		SetInvincible( work, 60 ) ;
		work->cont_hit_count = 0 ;
		work->far_think.missile_count = 0 ;
		work->control.turn.vy = work->rot_diff_player.vy ;
	}
}

/* 吹っ飛び */
static	void	Blow( Work *work, int time )
{
	if ( time == 0 ) {
		int		dir ;
		
		dir = GV_DiffDirS( work->control.rot.vy, work->rot_diff_player.vy ) ;
		if ( -2048 + 341 > dir ) {
			/* 後方中央 */
			SetAction( work, Mblow_b_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_b_m, 16 ) ;
			work->down_dir = 1 ;
		} else if ( -1024 > dir ) {
			/* 後方右 */
			SetAction( work, Mblow_b_r, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_b_r, 16 ) ;
			work->down_dir = 0 ;
		} else if ( -1024 + 682 > dir ) {
			/* 前方右 */
			SetAction( work, Mblow_f_r, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_f_r, 16 ) ;
			work->down_dir = 1 ;
		} else if ( 0 + 341 > dir ) {
			/* 前方中央 */
			SetAction( work, Mblow_f_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_f_m, 16 ) ;
			work->down_dir = 0 ;
		} else if ( 1024 > dir ) {
			/* 前方左 */
			SetAction( work, Mblow_f_l, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_f_l, 16 ) ;
			work->down_dir = 1 ;
		} else if ( 1024 + 682 > dir ) {
			/* 後方左 */
			SetAction( work, Mblow_b_l, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_b_l, 16 ) ;
			work->down_dir = 0 ;
		} else {
			/* 後方中央 */
			SetAction( work, Mblow_b_m, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMblow_b_m, 16 ) ;
			work->down_dir = 1 ;
		}
		SetInvincible( work, 0 ) ;
		work->fdata = -24.0F ;
		SeSet( DamageSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 2 ) ;
	}

	SetFlag( work, SOL_FLAG_NO_IK ) ;
	GroundSlopeControl( work ) ;

	{
		FVECTOR			step ;
		int				diff ;

		GV_SetVec3( &step, 0.0F, 0.0F, work->fdata ) ;
		if ( work->control.n_touches ) {
			diff = GV_DiffDirAbs( work->control.turn.vy + 2048, 
								  GV_VecDir2( &work->control.vecs[ 0 ] ) ) ;
			if ( diff < 128 ) {
				work->control.turn.vy -= 512 ;
				step.vz *= 1.50F ;
			}
		} 
		DG_SetPos2( &DG_ZeroVector, &work->control.turn ) ;
		DG_RotVector( &step, &step, 1 ) ; step.vy = 0.0F ;
		_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	}
	work->fdata = GV_NearExp8F( work->fdata, 0.0F ) ;

	if ( EndMotion( work ) ) {
//		SetMode( work, Down ) ;
        SetMode( work, Rise ) ;
	}
}

#if 0
/* ダウン */
static	void	Down( Work *work, int time )
{

}
#endif

/* 起き上がり */
static	void	Rise( Work *work, int time )
{
	if ( time == 0 ) {
		if ( work->down_dir == 1 ) {
			SetAction( work, Mrise_b, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMrise_b, 6 ) ;
		} else {
			SetAction( work, Mrise_f, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMrise_f, 6 ) ;
		}
	}

	SetFlag( work, SOL_FLAG_NO_IK ) ;
	GroundSlopeControl( work ) ;

	if ( EndMotion( work ) ) {
		if ( work->mode == SOL_MODE_BLADE_AND_SNAKEARM &&
			 CheckArmUndoGo( work ) ) {	/* 2個所あります */
			ChangeMode( work, SOL_MODE_BLADE_ONLY ) ;
			SetMode( work, ArmUndoReady ) ;
			return ;
		}
		SetInvincible( work, 30 ) ;
//		SetMode( work, StandStillK ) ;
		work->far_think.missile_count = 0 ;
		if ( work->len_diff_player < 2000.0F ) {
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				if ( GV_Time & 1 ) SetMode( work, SpinKick ) ;
				else			   SetMode( work, FastAway ) ;			
			} else {	
				if ( ( GV_Time & 1 ) && 
					work->control.mov.vz > 4000.0F &&
					work->control.mov.vz < 18000.0F &&
					DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) < 500.0F ) {
					SetMode( work, TriangleJump ) ;
				} else if ( !( GV_Time & 1 ) && 
						   work->control.mov.vz > 4000.0F &&
						   work->control.mov.vz < 18000.0F &&
						   DG_FABS( work->control.mov.vx 
								   - ( WALL_X - TRIANGLE_SHIFT ) ) < 1500.0F ) {
					SetMode( work, TriangleReady ) ;					
				} else {
					SetMode( work, FastAway ) ;
				}
			}
		} else if ( work->len_diff_player < 3500.0F ) {
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				if ( GV_Time & 1 ) SetMode( work, WalkBackK ) ;
				else			   SetMode( work, FastAway ) ;
			} else {
				if ( ( GV_Time & 1 ) && 
					work->control.mov.vz > 4000.0F &&
					work->control.mov.vz < 18000.0F &&
					DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) < 500.0F ) {
					SetMode( work, TriangleJump ) ;
				} else if ( !( GV_Time & 1 ) && 
						   work->control.mov.vz > 4000.0F &&
						   work->control.mov.vz < 18000.0F &&
						   DG_FABS( work->control.mov.vx 
								   - ( WALL_X - TRIANGLE_SHIFT ) ) < 1500.0F ) {
					SetMode( work, TriangleReady ) ;					
				} else {
					if ( GV_Time & 1 ) SetMode( work, WalkBackK ) ;
					else			   SetMode( work, FastAway ) ;
				}
			}
		} else if ( work->len_diff_player < 6000.0F ) {
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				if ( GV_Time & 1 ) SetMode( work, FastAttack ) ;
				else 			   SetMode( work, FastAway ) ;
			} else {
				SetMode( work, FastAway ) ;
			}
		} else {
			if ( work->mode == SOL_MODE_BLADE_ONLY ) {
				SetMode( work, FastAttack ) ;
			} else {
				SetMode( work, ShotArmMissile ) ;
			}
		}
	}
}

/* 死に */
static	void	Dead( Work *work, int time )
{
	extern void *NewCrossFadeEffectCustom( int time, int capture_interval, 
										   int bright_time, int alpha_time, int flag ) ;
#if 1
	if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, Mkwt_dam_f, 0, 0 ) ;
		SOL_SetSnakeArmMotion( AMkwt_dam_f, 32 ) ;			
		//SetAction( work, Mdam_out, 0, 6 ) ;
		SetInvincible( work, 0 ) ;
	}
	SetFlag( work, SOL_FLAG_NO_IK ) ;
	//GroundSlopeControl( work ) ;
	if ( time > TIME_BASE * DIRECT_TICK( 10 ) && work->data == 0 ) {
		NewCrossFadeEffectCustom( 1200, 1200, 0, 0, 1 ) ;
		MotionSleep( work, 0 ) ;
		work->data = 1 ;
	}
	if ( time > TIME_BASE * DIRECT_TICK( 40 ) && work->data == 1 ) {
		if ( work->procs[ SOL_PROC_DEAD ] > 0 ) {
			GM_ExecProc( work->procs[ SOL_PROC_DEAD ], NULL ) ;
		}
		work->data = 2 ;
	}

#endif
}

/* 蛇手外しポジションに移動 */
static	void	ArmUndoReady( Work *work, int time )
{	
	static	FVECTOR		Shift[] = {
		{ 6000.0F, 0.0F, 4000.0F },
		{ 6000.0F, 0.0F, 20000.0F },
	} ;
	FVECTOR	plpos, pos ;
	float	len1, len2 ;

	if ( time == 0 ) {
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;
		SetInvincible( work, 0 ) ;
		SeSet( SD_V_SOLSPE02, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		/* ライデンから遠い方へ */
		DG_COPY_VEC( &plpos, &GM_PlayerPosition ) ; 
		plpos.vy = 0.0F ;
		len1 = GV_VecLen3F2( &plpos, &Shift[ 0 ] ) ;
		len2 = GV_VecLen3F2( &plpos, &Shift[ 1 ] ) ;
		if ( len1 > len2 ) work->idata = 0 ;
		else			   work->idata = 1 ;
		work->idata2 = 0 ;
		ExecProc( work, SOL_PROC_ARMUNDO_MOVE_START, 0, 0 ) ;
	}
	switch( work->data ) {
	case 0 :
		work->control.turn.vy = GV_VecDir2FromTo( &work->control.mov, &Shift[ work->idata ] ) ;
		if ( EndMotion( work ) ) {
			//GM_SeSetMode( KasokuSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB ) ;
			SetAction( work, Mdash_f, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMdash_f, 6 ) ;
			work->fdata = 0.0F ;
			work->move_think.aim_len = 1000000.0F ;
			work->data = 1 ;
			DashFire( work, HUMAN21_MIGI_TSUMASAKI, 0 ) ;
		}
		break ;
	case 1 :
		work->control.turn.vy = GV_VecDir2FromTo( &work->control.mov, &Shift[ work->idata ] ) ;
		work->fdata = GV_NearExp8F( work->fdata, 480.0F ) ;
		MovePosition( work, work->control.rot.vy, work->fdata ) ;
		DG_COPY_VEC( &pos, &work->control.mov ) ;
		pos.vy = 0.0F ;
		if ( GV_VecLen3F2( &pos, &Shift[ work->idata ] ) < 500.0F ||
			 ++ work->idata2 > 8 * ( 300 / TIME_BASE ) ) {	/* 保険8秒 */
			//ResetInvincible( work ) ;
			DashFireStop( work ) ;
			SetAction( work, Mdash_end_f, 0, 6 ) ;
			SetFlag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ;
			SOL_SetSnakeArmMotion( AMdash_end_f, 6 ) ;
			work->data = 2 ;
		}
		break ;
	case 2 :
		SetFlag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ;
		if ( EndMotion( work ) ) {
			if ( work->idata == 0 ) work->control.turn.vy = 0 ;
			else work->control.turn.vy = 2048 ;	
			SetMode( work, ArmUndo ) ;
		}
	}
}

/* 蛇手外し */
static	void	ArmUndo( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_idle, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMkwt_idle, 6 ) ;		
		SetInvincible( work, 0 ) ;
		GV_PadReleaseOn( 0 ) ;
		GM_SetGameStatus( STATE_PAUSE_DISABLE | STATE_PRG_DEMO ) ;
		GM_SetMenuStatus( MENU_MENU_OFF | MENU_RADIO_DISABLE | MENU_GAGE_OFF ) ;
		//SeSet( SD_V_SOLSTE01, &work->control.mov, GM_SEMODE_BOMB, 2 ) ;
		if ( !GM_IsGameOver() ) {
			/* ボスサバイバル対応。プレイヤーがスネークのときは
			   台詞なし */
			if ( !GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_V_SOLSTE01 ) ;
			}
		}
		//DirPlayer( work ) ;
		ExecProc( work, SOL_PROC_ARMUNDO_START, work->idata, 0 ) ;
		/* ExecProcしてからidataリセット */
		work->idata = 0 ;
		/* プレイヤーを不可視 */
		PL_InvisiblePlayer() ;

		DG_InvisibleObjs( work->faceguard ) ;
	}

	SetFlag( work, SOL_FLAG_NO_IK ) ;

	if ( time == work->caption_time1 * TIME_BASE ) JimakuShow( work->caption1 ) ;
	if ( time == work->caption_time2 * TIME_BASE ) {
		JimakuHide() ;
		JimakuShow( work->caption2 ) ;
	}
	if ( time == work->caption_end * TIME_BASE ) {	
		JimakuHide() ;
	}

	/* 途中でゲームオーバーになったときの対処 */
	if ( GM_IsGameOver() ) {
		ExecProc( work, SOL_PROC_ARMUNDO_END, 0, 0 ) ;
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_V_SOLMUTE1 ) ;
		JimakuHide() ;
	}

	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			SetAction( work, Marm_undo_start, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMarm_undo_start, 6 ) ;
			work->data ++ ;
		}	
		break ;
	case 1 :
		//SOL_SnakeArmSetFlowFlag( 1 ) ;
		if ( work->idata < 147 && work->mtime1 >= 147 ) {
			if ( !GM_IsGameOver() ) SeSet( SD_V_SOLATK02, &work->control.mov, GM_SEMODE_BOMB, 2 ) ;
		}
		if ( work->idata < 210 && work->mtime1 >= 210 ) {
			if ( !GM_IsGameOver() ) {
				SeSet( SD_V_SOLMUS02, &work->control.mov, GM_SEMODE_BOMB, 2 ) ;
				GM_SeSetMode( SD_E_TMISSTA1, &work->control.mov, GM_SEMODE_BOMB ) ;
			}
		}
		if ( EndMotion( work ) ) {
			SOL_SnakeArmUndo() ;
			SetAction( work, Marm_undo_end, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMarm_undo_end, 6 ) ;
			work->data ++ ;
		}	
		break ;
	case 2 :
		//if ( work->mtime1 < 120 ) SOL_SnakeArmSetFlowFlag( 1 ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mkwt_fire_ready, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_ready, 6 ) ;					
			work->data ++ ;
		}			
		break ;
	case 3 :
		if ( CheckMotionTime( work, 58 ) ) {
			GM_SeSetMode( SD_A_SWORDSET, &work->control.mov, GM_SEMODE_BOMB ) ;
		}
		if ( EndMotion( work ) ) {
			SetAction( work, Mkwt_fire_idle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_idle, 6 ) ;					
			work->data ++ ;
		}			
		break ;
	case 4 :
		if ( EndMotion( work ) ) {
			ExecProc( work, SOL_PROC_ARMUNDO_END, 0, 0 ) ;
			JimakuHide() ;
			GV_PadReleaseOff( 0 ) ;
			GM_ResetGameStatus( STATE_PAUSE_DISABLE | STATE_PRG_DEMO ) ;
			GM_ResetMenuStatus( MENU_MENU_OFF | MENU_RADIO_DISABLE | MENU_GAGE_OFF ) ;
			ResetInvincible( work ) ;
			work->cont_hit_count = 0 ;
			SetMode( work, FastAway ) ;
			/* プレイヤーを可視に戻す */
			PL_VisiblePlayer() ;
			ResetFlag( work, SOL_FLAG_MISSILE_ATTACK_ENABLE ) ;
			work->far_think.last_missile_time = GV_Time ;
		}			
	}
    work->idata = work->mtime1 ;
}
