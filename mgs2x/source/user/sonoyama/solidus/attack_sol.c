/*
   attack_sol.c
   ソリダス／攻撃

   2001/03/27	M.Sonoyama
   $Id: attack_sol.c,v 1.1.1.3 2002/11/19 11:51:03 Yoshizawa1 Exp $
*/

/* 切り下ろし右 */
static	void	SlashDownR( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mslash_r, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMslash_r, 12 ) ;
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			MotionSpeed( work, 0.75F ) ;
		}
		DirPlayer( work ) ;
	}
	/* 左側が手薄 */
	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_ATTACK | SOL_FLAG_SLASH ) ;

	if ( CheckMotionTime( work, 16 ) ) {
		SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;		
	}

	if ( work->mtime1 < 32 ) {
		/* はじかれる隙 */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
		SetFlag( work, SOL_FLAG_FILLIP_NOW | SOL_FLAG_COMBO_IGNORE ) ;
	} else {
		if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
	}

	if ( work->mtime1 > 32 && work->mtime1 < 40 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	}

	work->kwt_trg = BLD_TRIG_SLASH ;
	if ( work->mtime1 >= 20 && work->mtime1 <= 26 ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY && 
			 GM_GameLevel == GM_LEVEL_EXTREME ) {
			SOL_SetBladeDamage( 10, 0.0F ) ;
		} else {
			SOL_SetBladeDamage( 5, 0.0F ) ;
		}
		work->kwt_trg |= BLD_TRIG_FIRE ;
	} 

	if ( work->data == 0 && MotionRate( work ) > 0.75 ) {
		work->data = 1 ;
		if ( GV_Time & 1 ) {
			SetMode( work, SlashDownL ) ;
			return ;
		}
	}

	if ( EndMotion( work ) ) {
		if ( GV_Time & 1 ) SetMode( work, StandStillK ) ;
		else			   SetMode( work, SlashDownL ) ;
	}
}

/* 斬り左 */
static	void	SlashDownL( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mslash_l, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMslash_l, 12 ) ;
		DirPlayer( work ) ;
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			MotionSpeed( work, 0.75F ) ;
		}
	}
	/* 左側が手薄 */
	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_ATTACK | SOL_FLAG_SLASH_L ) ;

	if ( CheckMotionTime( work, 10 ) ) {
		SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;		
	}

	if ( work->mtime1 < 24 ) {
		/* はじかれる隙 */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
		SetFlag( work, SOL_FLAG_FILLIP_NOW | SOL_FLAG_COMBO_IGNORE ) ;
	} else {
		if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
	}

	if ( work->mtime1 > 24 && work->mtime1 < 40 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	}

	work->mst_trg = BLD_TRIG_SLASH ;
	if ( work->mtime1 >= 16 && work->mtime1 <= 20 ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY && 
			 GM_GameLevel == GM_LEVEL_EXTREME ) {
			SOL_SetBladeDamage( 10, 0.0F ) ;
		} else {
			SOL_SetBladeDamage( 5, 0.0F ) ;
		}
		work->mst_trg |= BLD_TRIG_FIRE ;
	}
	if ( EndMotion( work ) ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			if ( GV_Time & 1 ) SetMode( work, StandStillK ) ;
			else			   SetMode( work, SlashThrust ) ;
		} else {
			SetMode( work, StandStillK ) ;
		}
	}
}

/* ダブル斬り */
static	void	SlashDouble( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mslash_w, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMslash_w, 6 ) ;
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			MotionSpeed( work, 0.60F ) ;
		} else {
			MotionSpeed( work, 0.90F ) ;
		}
	}

	/* パーティクルモード */
	if ( work->mtime1 < 70 ) {
	    work->energy_mode = 1;
	}

	SetFlag( work, SOL_FLAG_ATTACK | SOL_FLAG_SLASH ) ;
	SetFlag( work, SOL_FLAG_NO_IK ) ;

	work->kwt_trg = BLD_TRIG_SLASH ;
	work->mst_trg = BLD_TRIG_SLASH ;

	ChestSlopeControl( work ) ;
	if ( work->turn_adj[ HUMAN21_MUNE ].vx > 0 ) {
		work->turn_adj[ HUMAN21_MUNE ].vx = 0 ;
	} else {
		work->turn_adj[ HUMAN21_MUNE ].vx *= 3 ;
	}

	if ( work->mtime1 < 32 ) DirPlayer( work ) ;

	if ( work->mtime1 >= 56 && work->mtime1 <= 71 ) {
		SOL_SetBladeDamage( 10, 250.0F ) ;
		work->kwt_trg |= BLD_TRIG_FIRE ;
		work->mst_trg |= BLD_TRIG_FIRE ;
		/* 防御不能 */
		SOL_SetBladeWeaponType( WP_SHOTGUN_NEAR ) ;
	} 

	if ( CheckMotionTime( work, 56 ) ) {
		if ( GV_Time & 1 ) {
			SeSet( SD_V_SOLATK06, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else {
			SeSet( SD_V_SOLATK08, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}

	LeftSideNoGuard( work ) ;
	if ( work->mtime1 >= 56 && work->mtime1 < 128 ) {
		SetFlag( work, SOL_FLAG_COMBO_IGNORE ) ;
	} 
	if ( work->mtime1 < 128 ) {
		/* はじかれる隙 */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
		SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
	} else {
		//if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;
	}	
}

/* 突き */
static	void	SlashThrust( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;
		MotionSpeed( work, 0.50F ) ;
		DirPlayer( work ) ;
		SeSet( SD_V_SOLATK01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	}
	/* 左側が手薄 */
	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_ATTACK | SOL_FLAG_SLASH ) ;

	switch( work->data ) {
	case 0 :
		DirPlayer( work ) ;
		if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mthrust, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMthrust, 12 ) ;
			if ( GV_Time & 1 ) {
				SeSet( SD_V_SOLATK05, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			} else {
				SeSet( SD_V_SOLATK07, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			}
			work->data = 1 ;
		}
		break ;
	case 1 :
		work->kwt_trg = BLD_TRIG_THRUST ;
		if ( work->mtime1 >= 5 && work->mtime1 <= 20 ) {
			SOL_SetBladeDamage( 15, 0.0F ) ;
			work->kwt_trg |= BLD_TRIG_FIRE ;
		}
		if ( work->mtime1 < 52 ) {
			ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
			SetFlag( work, SOL_FLAG_COMBO_IGNORE ) ;
		} else {
			//if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
			/* 隙だらけタイム */
			ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
		}
		SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;

		if ( EndMotion( work ) ) {
			SetMode( work, StandStillK ) ;
		}
	} 
}

/* 斬りコンボ */
static	void	SlashCombo( Work *work, int time ) 
{
	if ( time == 0 ) {
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 12 ) ;
		MotionSpeed( work, 0.75F ) ;
		DirPlayer( work ) ;
		SeSet( SD_V_SOLATK01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	}

	/* パーティクルモード */
	if ( work->mtime1 < 50 ) {
	    work->energy_mode = 1;
	}

	LeftSideNoGuard( work ) ;
	
	if ( work->data == 0 ) {
		DirPlayer( work ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mslash_combo, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMslash_combo, 6 ) ;
			work->data = 1 ;
		} else {
			return ;
		}
	} 

	SetFlag( work, SOL_FLAG_ATTACK | SOL_FLAG_SLASH | SOL_FLAG_NO_IK ) ;

	if ( work->mtime1 < 16 ) {
		work->mst_trg = BLD_TRIG_SLASH ;
	} else if ( work->mtime1 >= 16 && work->mtime1 < 32 ) {
		work->kwt_trg = BLD_TRIG_SLASH ;
	} else if ( work->mtime1 >= 32 && work->mtime1 < 96 ) {
		work->mst_trg = BLD_TRIG_SLASH ;
		work->kwt_trg = BLD_TRIG_SLASH ;
	}

	if ( CheckMotionTime( work, 8 ) ) {
		work->mst_trg |= BLD_TRIG_FIRE ;
		SOL_SetBladeDamage( 1, 150.0F ) ;
		SOL_SetBladeWeaponType( WP_AKS | WP_BLADEGUARD | WP_NOMUTEKI ) ;
		//SOL_SetBladeForce( 32.0F ) ;
	} else if ( CheckMotionTime( work, 24 ) ) {
		work->kwt_trg |= BLD_TRIG_FIRE ;
		SOL_SetBladeDamage( 1, 150.0F ) ;
		SOL_SetBladeWeaponType( WP_AKS | WP_BLADEGUARD | WP_NOMUTEKI ) ;
		//SOL_SetBladeForce( 32.0F ) ;
	} else if ( CheckMotionTime( work, 50 ) ) {
		work->mst_trg |= BLD_TRIG_FIRE ;
		work->kwt_trg |= BLD_TRIG_FIRE ;
		SOL_SetBladeDamage( 5, 250.0F ) ;
	}

	if ( work->mtime1 >= 8 && work->data2 < 8 ) {
		SeSet( SD_V_SOLATK02, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	} else if ( work->mtime1 >= 23 && work->data2 < 23 ) {
		SeSet( SD_V_SOLATK01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	} else if ( work->mtime1 >= 50 && work->data2 < 50 ) {
		SeSet( SD_V_SOLATK06, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	}

	work->data2 = work->mtime1 ;

	if ( work->mtime1 < 8 ) DirPlayer( work ) ;

	if ( work->mtime1 < 70 ) {
		SetFlag( work, SOL_FLAG_COMBO_IGNORE ) ;
		ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
		SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
	} else if ( work->mtime1 < 120 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;
	}	
}


/* 打撃攻撃コールバック */
static	void	BodyHit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;

	work = ( Work * )ptr ;

	if ( GM_PlayerControl != NULL && 
		 def->name == GM_PlayerControl->name ) {
		/* ローリング中は当たらない */
		if ( GM_CheckPlayerStatus( PLAYER_ROLLING ) ) {
			if ( def->weapon_type == off->weapon_type ) {
				def->damaged &= ~TARGET_POWER ;
			}
			def->weapon_type &= ~off->weapon_type ;
		}
	}
}

/* エルボー */
static	void	Elbow( Work *work, int time )
{
	int		ptime, mtime ;

	if ( time == 0 ) {
		SetAction( work, Melbow, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMelbow, 12 ) ;
		//SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		SeSet( SD_V_SOLATK04, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		if ( work->data == 0 ) DirPlayer( work ) ;
		work->idata = 0 ;
	}

	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_NO_IK ) ;
	SetFlag( work, SOL_FLAG_ATTACK ) ;
	if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_IGNORE ) ;

	mtime = MotionTime( work ) ;
	ptime = work->idata ;
	work->idata = mtime ;

	if ( ptime < 7 && mtime >= 7 ) {
		TARGET			*t ;
		POWER_TARGET	*p ;
		FVECTOR			force, size ;
		int				damage = 5 ;

		t = &work->offense ;
		p = &work->pt_offense ;
		GV_SetVec3( &size, 750.0F, 350.0F, 750.0F ) ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_NAME_IS_SE, work->control.map, PLAYER_SIDE, 
					  &size, &DG_ZeroVector ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		GV_SetVec3( &force, 0.0F, 0.0F, 96.0F ) ;
		DG_RotVector( &force, &force, 1 ) ;
		GM_SetTargetWeaponType( t, WP_KICK | WP_BLADEGUARD | WP_NOMUTEKI ) ;
		t->name = SD_E_ATARU02 ;

		/* 難易度別 */
		switch( SOL_GameLevel ) {
		case GM_LEVEL_VERYEASY :
			damage -= 2 ;
			break ;
		case GM_LEVEL_EASY :
			damage -= 1 ;
			break ;
		case GM_LEVEL_HARD :
			damage += 5 ;
			break ;
		case GM_LEVEL_EXTREME :
			damage = damage * 2 ;
			break ;
		case GM_LEVEL_NORMAL :
		default :
		  ;
		}
		if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
		if ( damage <= 0 ) damage = 1 ;

		GM_SetPowerTarget( t, p, POWER_CONST, 255, 0, damage, &force ) ;
		GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_HIDARI_UDE2 ].world ) ;
		GM_SetTargetCallBack( t, BodyHit, work ) ;
		GM_PutTarget( t ) ;
	}
	
	if ( mtime > 16 && mtime < 90 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_IGNORE ) ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;		
	}
}

/* スピンキック */
static	void	SpinKick( Work *work, int time )
{
	int		ptime, mtime ;

	if ( time == 0 ) {
		SetAction( work, Mspinkick, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMspinkick, 12 ) ;
		DirPlayer( work ) ;
		work->idata = 0 ;
		if ( GV_Time & 1 ) {
			SeSet( SD_V_SOLATK05, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else {
			SeSet( SD_V_SOLATK07, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}

	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_NO_IK ) ;
	SetFlag( work, SOL_FLAG_ATTACK ) ;
	if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_IGNORE ) ;

	mtime = MotionTime( work ) ;
	ptime = work->idata ;
	work->idata = mtime ;

	if ( ptime < 22 && mtime >= 22 ) {
		TARGET			*t ;
		POWER_TARGET	*p ;
		FVECTOR			force, size ;
		int				damage = 5 ;

		t = &work->offense ;
		p = &work->pt_offense ;
		GV_SetVec3( &size, 1000.0F, 1000.0F, 1000.0F ) ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_NAME_IS_SE, work->control.map, 
					  PLAYER_SIDE, &size, &DG_ZeroVector ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		GV_SetVec3( &force, 0.0F, 0.0F, 128.0F ) ;
		DG_RotVector( &force, &force, 1 ) ;
		GM_SetTargetWeaponType( t, WP_KICK | WP_BLADEGUARD | WP_NOMUTEKI ) ;
		t->name = SD_E_ATARU02 ;

		/* 難易度別 */
		switch( SOL_GameLevel ) {
		case GM_LEVEL_VERYEASY :
			damage -= 2 ;
			break ;
		case GM_LEVEL_EASY :
			damage -= 1 ;
			break ;
		case GM_LEVEL_HARD :
			damage += 1 ;
			break ;
		case GM_LEVEL_EXTREME :
			damage += 2 ;
			break ;
		case GM_LEVEL_NORMAL :
		default :
		  ;
		}
		if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
		if ( damage <= 0 ) damage = 1 ;

		GM_SetPowerTarget( t, p, POWER_CONST, 255, 0, damage, &force ) ;
		GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_MIGI_ASHI1 ].world ) ;
		GM_SetTargetCallBack( t, BodyHit, work ) ;
		GM_PutTarget( t ) ;
	}

	if ( mtime > 22 && mtime < 100 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_IGNORE ) ;
	}

	if ( EndMotion( work ) ) {
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			if ( GV_Time & 1 ) SetMode( work, StandStillK ) ;		
			else			   SetMode( work, Elbow ) ;
		} else {
			SetMode( work, StandStillK ) ;		
		}
	}
}

/* 踏み付け */
static	void	Stamp( Work *work, int time )
{
	int		ptime, mtime ;

	if ( time == 0 ) {
		SetAction( work, Mstamp, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMstamp, 12 ) ;
		//SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		if ( work->data == 0 ) DirPlayer( work ) ;
		work->idata = 0 ;
	}

	LeftSideNoGuard( work ) ;
	SetFlag( work, SOL_FLAG_NO_IK ) ;
	SetFlag( work, SOL_FLAG_ATTACK ) ;
	if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;

	mtime = MotionTime( work ) ;
	ptime = work->idata ;
	work->idata = mtime ;

	if ( ptime < 40 && mtime >= 40 ) {
		TARGET			*t ;
		POWER_TARGET	*p ;
		FVECTOR			force, size ;
		int				damage = 5 ;
		t = &work->offense ;
		p = &work->pt_offense ;
		GV_SetVec3( &size, 500.0F, 350.0F, 500.0F ) ;
		GM_SetTarget( t, TARGET_OFFENSE, work->control.map, PLAYER_SIDE, &size, &DG_ZeroVector ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		GV_SetVec3( &force, 0.0F, 0.0F, 0.0F ) ;
		DG_RotVector( &force, &force, 1 ) ;
		GM_SetTargetWeaponType( t, WP_AKS | WP_BLADEGUARD | WP_NOMUTEKI ) ;

		/* 難易度別 */
		switch( SOL_GameLevel ) {
		case GM_LEVEL_VERYEASY :
			damage -= 2 ;
			break ;
		case GM_LEVEL_EASY :
			damage -= 1 ;
			break ;
		case GM_LEVEL_HARD :
			damage += 1 ;
			break ;
		case GM_LEVEL_EXTREME :
			damage += 2 ;
			break ;
		case GM_LEVEL_NORMAL :
		default :
		  ;
		}
		if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
		if ( damage <= 0 ) damage = 1 ;

		GM_SetPowerTarget( t, p, POWER_CONST, 255, 0, damage, &force ) ;
		GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_HIDARI_TSUMASAKI ].world ) ;
		GM_SetTargetCallBack( t, BodyHit, work ) ;
		GM_PutTarget( t ) ;
		SeSet( SD_V_SOLATK06, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
	}
	if ( mtime > 40 && mtime < 108 ) {
		/* 隙きだらけタイム */
		ResetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;		
	}
}

/* 蛇手ミサイル */
static	void	ShotArmMissile( Work *work, int time )
{
	if ( time == 0 ) {
		/* 0.90Fは、think_sol.cにもあるので注意 */
		if ( !CheckLife( work, 0.90F ) || work->len_diff_player < 6000.0F ) {
			SetAction( work, Mkwt_idle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_idle, 6 ) ;			
			work->far_think.missile_count = -DIRECT_TICK( 128 ) ;
			SetMode( work, StandStillK ) ;
			return ;
		}
		SetAction( work, Mmissle_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMmissle_ready, 12 ) ;
		SeSet( ReadySE[ irnd() % 5 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		work->idata = 0 ;
		work->idata2 = 0 ;
		work->far_think.last_missile_time = GV_Time ;
	}

	work->control.turn.vy = work->rot_diff_player.vy ;	

	switch( work->data ) {
	case 0 :			/* 立ち～構え */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( EndMotion( work ) ) {
			SetAction( work, Mmissle_fire_idle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMmissle_fire_idle, 12 ) ;
			work->data = 1 ;
			work->data2 = 0 ;
		}
		break ;
	case 1 :			/* 構え静止 */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( ++ work->data2 == DIRECT_TICK( 1 ) ) {
			SetAction( work, Mmissle_fire, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMmissle_fire, 12 ) ;
			work->data = 2 ;
		}
	case 2 :			/* 発射 */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( work->idata2 < 1 && work->mtime1 >= 1 ) {
			SOL_SnakeArmFireMissile( 0 ) ;
			//NewSnakeArmMissile( &work->body.objs->objs[ HUMAN21_ATAMA ].world, 
								 //work->control.map ) ;
		} else if ( work->idata2 < 30 && work->mtime1 >= 30 ) {
			SOL_SnakeArmFireMissile( 1 ) ;
			//NewSnakeArmMissile( &work->body.objs->objs[ HUMAN21_ATAMA ].world, 
								 //work->control.map ) ;
		}
		if ( EndMotion( work ) ) {
			++ work->idata ;

			if ( SOL_GameLevel == GM_LEVEL_VERYEASY ||
				 SOL_GameLevel == GM_LEVEL_EASY ) work->idata ++ ;

			if( work->len_diff_player < 3000.0F || work->idata > 1 ) {
				SetAction( work, Mmissle_end, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMmissle_end, 12 ) ;
				work->data = 3 ;
			} else {
				SetAction( work, Mmissle_fire_idle, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMmissle_fire_idle, 12 ) ;
				work->data = 1 ;
				work->data2 = 0 ;
			}
		}
		break ;
	case 3 :			/* 立ちに戻る */
		if ( EndMotion( work ) ) {
			work->far_think.missile_count = -DIRECT_TICK( 128 ) ;
			if ( work->mode == SOL_MODE_BLADE_ONLY &&
				!GM_CheckPlayerStatus( PLAYER_DAMAGED | PLAYER_DOWNED ) && 
				work->len_diff_player > 5000.0F ) SetMode( work, FastAttack ) ;
			else SetMode( work, StandStillK ) ;
		}
	}
	work->idata2 = work->mtime1 ;
}

/* 蛇手足払い */
static	void	Trip( Work *work, int time )
{
	if ( time == 0 ) {
		SetInvincible( work, 0 ) ;
		SetAction( work, Mtrip_ready, 0, 24 ) ;
		SOL_SetSnakeArmMotion( AMtrip_ready, 24 ) ;
		DirPlayer( work ) ;
		//GM_SeSetMode( SD_E_HEBREDY1, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	//SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	ChestSlopeControl( work ) ;

	switch ( work->data ) {
	case 0 :	/* ため */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		DirPlayer( work ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mtrip_r, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMtrip_r, 6 ) ;
			SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			//GM_SeSetMode( SD_E_HEBTKAM1, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->data = 1 ;
		}
		break ;
	case 1 :	/* 放ち右 */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( CheckSolStatus( SOL_CS_SNAKEARM_TRIP_SUCCESS ) ) {
			work->data2 = 1 ;
		}
		if ( work->data2 == 0 && work->mtime1 > 12 && work->mtime1 < 32 ) {
			SOL_SnakeArmAttackTrip( 0 ) ;
		}
		if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) ) {
			ResetInvincible( work ) ;
			SetMode( work, SlashFillipedR ) ;
			return ;
		}
		SOL_SnakeArmSetDefenseTarget() ;
		if ( EndMotion( work ) ) {
			ResetInvincible( work ) ;
			if ( work->data2 == 1 ) {
				work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
				SetMode( work, PursueWalk ) ;
			} else {
				SetAction( work, Mtrip_l, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMtrip_l, 6 ) ;
				//GM_SeSetMode( SD_E_HEBTKAM1, &work->control.mov, GM_SEMODE_BOMB ) ;	
				work->data = 2 ;
			}
		}	
		break ;
	case 2 :	/* 放ち左 */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( CheckSolStatus( SOL_CS_SNAKEARM_TRIP_SUCCESS ) ) {
			work->data2 = 1 ;
		}
		if ( work->data2 == 0 && work->mtime1 > 12 && work->mtime1 < 32 ) {
			SOL_SnakeArmAttackTrip( 1 ) ;
		}
		if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) ) {
			ResetInvincible( work ) ;
			SetMode( work, SlashFillipedL ) ;
			return ;
		}
		SOL_SnakeArmSetDefenseTarget() ;
		if ( EndMotion( work ) ) {
			ResetInvincible( work ) ;
			work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
			if ( work->data2 == 1 ) {
				SetMode( work, PursueWalk ) ;
			} else {
				SetMode( work, StandStillK ) ;					
			}
		}	
	}
}

/* 蛇手爪攻撃 */
static	void	Claw( Work *work, int time )
{
	if ( time == 0 ) {
		SetInvincible( work, 0 ) ;
		SetAction( work, Mclaw_ready, 0, 24 ) ;
		SOL_SetSnakeArmMotion( AMclaw_ready, 24 ) ;
		DirPlayer( work ) ;
		//GM_SeSetMode( SD_E_HEBREDY1, &work->control.mov, GM_SEMODE_BOMB ) ;
	}
	//SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	ChestSlopeControl( work ) ;
	switch ( work->data ) {
	case 0 :	/* ため */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		DirPlayer( work ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mclaw_fire, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMclaw_fire, 6 ) ;
			SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			//GM_SeSetMode( SD_E_HEBTKAM1, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->data = 1 ;
		}
		break ;
	case 1 :	/* はなち */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( CheckSolStatus( SOL_CS_SNAKEARM_CLAW_SUCCESS ) ) {
			work->data2 = 1 ;
		}
		if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) ) {
			ResetInvincible( work ) ;
			work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
			SetMode( work, SlashFillipedL ) ;
			return ;
		}
		if ( work->mtime1 < 24 ) {
			if ( work->data2 == 0 ) SOL_SnakeArmAttackClaw() ;
		} else {
			ResetInvincible( work ) ;
		}
		SOL_SnakeArmSetDefenseTarget() ;
		if ( EndMotion( work ) ) {
			ResetInvincible( work ) ;
			work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
			SetMode( work, SnakeArmAttackEnd ) ;
		}	
		break ;
	}
}

/* 蛇手、ライデンつかみにいく */
static	void	LiftUpStart( Work *work, int time ) 
{
	if ( time == 0 ) {
		SetInvincible( work, 0 ) ;
		SetAction( work, Mclaw_ready, 0, 24 ) ;
		SOL_SetSnakeArmMotion( AMclaw_ready, 24 ) ;
		DirPlayer( work ) ;
		//GM_SeSetMode( SD_E_HEBREDY1, &work->control.mov, GM_SEMODE_BOMB ) ;
	}
	//SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	ChestSlopeControl( work ) ;
	switch ( work->data ) {
	case 0 :	/* ため */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		//LeftSideNoGuard( work ) ;
		//if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) SetFlag( work, SOL_FLAG_COMBO_GUARD ) ;
		DirPlayer( work ) ;
		if ( EndMotion( work ) ) {
			SetAction( work, Mseize_start, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMseize_start, 6 ) ;
			SeSet( SlashSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			//GM_SeSetMode( SD_E_HEBTKAM1, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->data = 1 ;
		}
		break ;
	case 1 :	/* はなち */
		SOL_SnakeArmSetFlowFlag( 1 ) ;
		work->energy_mode = 2;
		if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_DAMAGED ) &&
			 !CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ) {
			ResetInvincible( work ) ;
			SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_END ) ;
			work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
			SetMode( work, SlashFillipedL ) ;
			return ;
		}
		if ( work->mtime1 < 24 ) SOL_SnakeArmAttackOnline( 14, 30 ) ;
		else {
			//ResetInvincible( work ) ;
			SetInvincible( work, 4 ) ;
		}
		SOL_SnakeArmSetDefenseTarget() ;
		if ( EndMotion( work ) ) {
			if ( !PL_LiftupNow() || !CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ) {
				SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_END ) ;
				ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_SUCCESS ) ;
				SetAction( work, Mseize_miss, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMseize_miss, 6 ) ;
				work->data = 2 ;
			} else {
				SetMode( work, LiftUpUp ) ;
			}
		}	
		break ;
	case 2 :	/* つかみミス */
		if ( EndMotion( work ) ) {
			work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
			SetMode( work, StandStillK ) ;		
		}
	}
}

/* 蛇手、ライデンつかんで持ち上げる */
static	void	LiftUpUp( Work *work, int time ) 
{
	if ( time == 0 ) {
		SetAction( work, Mseize_liftup, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMseize_liftup, 6 ) ;
		/* 床無し部分に落とさないように、
		   マップ中心を向かせる */
		work->control.turn.vy = GV_VecDir2FromTo( &work->control.mov, &work->map_center ) ;
		work->control.interp = DIRECT_TICK( 15 ) ;
	}
	SOL_SnakeArmSetFlowFlag( 1 ) ;
	work->energy_mode = 2;
	SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	ChestSlopeControl( work ) ;
	SOL_SnakeArmAttackOnline( 14, 30 ) ;
	SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, LiftUpIdle ) ;
	}
}

/* 蛇手、ライデンつかんで持ち上げて静止 */
static	void	LiftUpIdle( Work *work, int time ) 
{
	if ( time == 0 ) {
		SetAction( work, Mseize_liftidle, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMseize_liftidle, 6 ) ;
		if ( GM_Vitality < GM_VitalityMax / 3 ) {
			SeSet( ChouhatsuSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		} else {
			SeSet( ChouhatsuSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}

	if ( ( GV_Time % DIRECT_TICK( 48 ) ) == 0 ) {
		//GM_SeSetMode( SD_E_HEBSIME1, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
	}
	SOL_SnakeArmSetFlowFlag( 1 ) ;
	work->energy_mode = 2;
	ChestSlopeControl( work ) ;
	SOL_SnakeArmAttackOnline( 14, 30 ) ;
	SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING ) ;
	if ( CheckSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL ) || 
		 GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
		ResetInvincible( work ) ; 
		ResetSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL ) ;
		SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_END ) ;
		work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
		if ( !GM_CheckPlayerStatus( PLAYER_DEAD ) ) {
			SeSet( KawashiSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
		SetMode( work, LiftUpEscaped ) ;
		return ;
	}
	if ( work->ftime > DIRECT_TICK( 120 ) ) {
		SetMode( work, LiftUpFling ) ;
	} else {
		SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_CANCEL_ENABLE ) ;
	}
}

/* 蛇手、ライデンに逃げられる */
static	void	LiftUpEscaped( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mseize_undo, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMseize_undo, 6 ) ;
	}
	ChestSlopeControl( work ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, SnakeArmAttackEnd ) ;
	}	
}

/* 蛇手、ライデン叩き付ける */
static	void	LiftUpFling( Work *work, int time ) 
{
	if ( time == 0 ) {
		SetAction( work, Mseize_fling, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMseize_fling, 6 ) ;
	}

	SOL_SnakeArmSetFlowFlag( 1 ) ;
	work->energy_mode = 2;
	SetFlag( work, SOL_FLAG_NO_PUSH ) ;
	ChestSlopeControl( work ) ;
	SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING ) ;
	
	switch( work->data2 ) {
	case 0 :
		SOL_SnakeArmAttackOnline( 14, 30 ) ;
		if ( work->data < 126 && work->mtime1 >= 126 ) {
			SeSet( KiaiSE[ irnd() % 4 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_FLING_GROUND ) ;
			work->data2 = 1 ;
		}
		break ;
	case 1 :
	  ;
	}

	work->data = work->mtime1 ;

	if ( EndMotion( work ) ) {
		ResetInvincible( work ) ; 
		SetSolStatus( SOL_CS_SNAKEARM_LIFTUP_END ) ;
		work->far_think.liftup_count = -DIRECT_TICK( 80 ) ;
		SetMode( work, SnakeArmAttackEnd ) ;
	}
}

/* 蛇手攻撃終了 */
static	void	SnakeArmAttackEnd( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mclaw_end, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMclaw_end, 6 ) ;
	}
	ChestSlopeControl( work ) ;
	if ( EndMotion( work ) ) {
		SetMode( work, StandStillK ) ;
	}	
}

/* 歩き追撃 */
static	void	PursueWalk( Work *work, int time )
{
	if ( time == 0 ) {
		if ( work->len_diff_player <= 1600.0F ) {
			SetAction( work, Mkwt_fire_idle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_idle, 32 ) ;		
		} else if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			SetAction( work, Mkwt_run, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_run, 32 ) ;
		} else {
			SetAction( work, Mkwt_walk, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_walk, 32 ) ;
		}
	}
	
	switch ( work->data ) {
	case 0 :
		SetFlag( work, SOL_FLAG_NO_IK ) ;
		DirPlayer( work ) ;
		if ( work->len_diff_player <= 1600.0F ) {
			SetAction( work, Mkwt_fire_idle, 0, 6 ) ;
			SOL_SetSnakeArmMotion( AMkwt_fire_idle, 32 ) ;		
			work->data ++ ;
			work->data2 = 0 ;
		}
		break ;
	case 1 :
		/* ちょっと待つ */
		if ( time > 300 * 2 ) {
			//SetMode( work, SlashDouble ) ;
			SetMode( work, Stamp ) ;
			return ;
		}
	}
	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
	} else if ( GM_PlayerBody->height > 750.0F ) {
		work->last_slash_time = 0 ;
		SetMode( work, StandStillK ) ;
	} 
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
}

/* 加速突進 */
static	void	FastAttack( Work *work, int time )
{
	FVECTOR		plpos ;

	if ( time == 0 ) {
		SetAction( work, Mdash_ready, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMdash_ready, 32 ) ;
		if ( GM_GameLevel == GM_LEVEL_EXTREME ) MotionSpeed( work, 0.50F ) ;
		SetInvincible( work, 0 ) ;
		if ( work->len_diff_player >= 3000.0F ) {
			SeSet( ReadySE[ irnd() % 5 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
		}
	}
	switch( work->data ) {
	case 0 :
		DirPlayer( work ) ;
		if ( EndMotion( work ) ) {
			if ( work->len_diff_player < 3000.0F ) {
				ResetInvincible( work ) ;
				SetMode( work, Elbow ) ;
				work->data = 1 ;
			} else {
				DG_COPY_VEC( &work->fv1, &GM_PlayerPosition ) ;
				SeSet( KasokuSE[ irnd() % 3 ], &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
				SetAction( work, Mdash_f, 0, 6 ) ;
				SOL_SetSnakeArmMotion( AMdash_f, 32 ) ;
				work->fdata = 0.0F ;
				work->idata = 0 ;
				work->move_think.aim_len = 1000000.0F ;
				work->data = 1 ;
				DashFire( work, HUMAN21_MIGI_TSUMASAKI, DIRECT_TICK( 30 ) ) ;
			}
		}
		break ;
	case 1 :
#if 0
		if ( ( work->ftime & 2 ) == 0 ) {
			extern void	*NewSolTraceFlame( FVECTOR *pos, int life ) ;
			FVECTOR		pos ;

			DG_COPY_VEC( &pos, &work->control.mov ) ;
			pos.vy = work->control.levels[ 0 ] ;

			GV_SetActorChild( work, NewSolTraceFlame( &pos, 120 ) ) ;

		}
#endif
		if ( work->mode == SOL_MODE_BLADE_ONLY ) {
			work->fdata = GV_NearExp8F( work->fdata, 640.0F ) ;
		} else {
			work->fdata = GV_NearExp8F( work->fdata, 480.0F ) ;
		}
		DirPlayer( work ) ;
		MovePosition( work, work->control.rot.vy, work->fdata ) ;
		work->fv1.vy = work->control.mov.vy ;

		DG_COPY_VEC( &plpos, &GM_PlayerPosition ) ;
		plpos.vy = work->control.mov.vy ;
		/* プレイヤーが近すぎるときは攻撃判定 */
		if ( GV_VecLen3F2( &work->control.mov, &plpos ) < 750.0F ) {
			TARGET			*t ;
			POWER_TARGET	*p ;
			FVECTOR			force, size ;
			int				damage = 5 ;

			t = &work->offense ;
			p = &work->pt_offense ;
			GV_SetVec3( &size, 500.0F, 350.0F, 500.0F ) ;
			GM_SetTarget( t, TARGET_OFFENSE, work->control.map, PLAYER_SIDE, &size, &DG_ZeroVector ) ;
			DG_SetPos2( &work->control.mov, &work->control.rot ) ;
			GV_SetVec3( &force, 0.0F, 0.0F, 0.0F ) ;
			DG_RotVector( &force, &force, 1 ) ;
			GM_SetTargetWeaponType( t, WP_KICK ) ;
			
			/* 難易度別 */
			switch( SOL_GameLevel ) {
			case GM_LEVEL_VERYEASY :
				damage -= 2 ;
				break ;
			case GM_LEVEL_EASY :
				damage -= 1 ;
				break ;
			case GM_LEVEL_HARD :
				damage += 1 ;
				break ;
			case GM_LEVEL_EXTREME :
				damage += 2 ;
				break ;
			case GM_LEVEL_NORMAL :
			default :
			  ;
			}
			if ( SOL_GameLevel < GM_LEVEL_EXTREME ) damage += 2 ;
			if ( damage <= 0 ) damage = 1 ;
			
			GM_SetPowerTarget( t, p, POWER_CONST, 255, 0, damage, &force ) ;
			GM_MoveTarget3( t, &work->body.objs->objs[ HUMAN21_HIDARI_TSUMASAKI ].world ) ;
			GM_SetTargetCallBack( t, BodyHit, work ) ;
			GM_PutTarget( t ) ;
		}

		/* 2000を変えないように！！ */
		if ( GV_VecLen3F2( &work->control.mov, &work->fv1 ) < 2000.0F ||	
			 ++ work->idata > DIRECT_TICK( 90 ) ) {
			DashFireStop( work ) ;
			ResetInvincible( work ) ;
			if ( GV_Time & 4 ) {
				SetMode( work, Elbow ) ;
				work->data = 1 ;
			} else {
				SetMode( work, SlashDownR ) ;
			}
		}
	}
}

/* エルードしているところまで走る */
static	void	EludeAttackReady( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_run, 0, 6 ) ;
		SOL_SetSnakeArmMotion( AMkwt_run, 16 ) ;
	}
	SetFlag( work, SOL_FLAG_NO_IK ) ;
	DirPlayer( work ) ;
	if ( work->len_diff_player <= 2500.0F ) {
		if ( GV_Time - work->last_elude_attack_time > 10 * ( 300 / TIME_BASE ) ) {
			if ( GV_Time & 1 ) {
				SeSet( SD_V_SOLELU01, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			} else {
				SeSet( SD_V_SOLELU02, &work->control.mov, GM_SEMODE_BOMB, 1 ) ;
			}
		}
		work->last_elude_attack_time = GV_Time ;
		SetMode( work, SlashDouble ) ;
		return ;
	}
	if ( !GM_CheckPlayerStatus( PLAYER_BEYOND ) ||
		 GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) ) {
		SetMode( work, StandStillK ) ;
	}	
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
}
