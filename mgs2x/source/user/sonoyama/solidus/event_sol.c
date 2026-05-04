/*
   event_sol.c
   ソリダス イベント処理
   
   2001/03/21	M.Sonoyama
   $Id: event_sol.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
*/

/* 刀防御火花 */
/* offenseがOnlineのとき専用 */
static	void	BladeGuardSpark( TARGET *off, TARGET *def ) 
{
	FMATRIX		mat ;
	SVECTOR		rot ;
				
	if ( off->power != NULL ) {
		GV_VecToRot( &off->power->force, &rot ) ;
	} else {
		rot = DG_ZeroSVector ;
	}
	DG_SetPos2( &def->hit, &rot ) ;
	DG_GetPos( &mat ) ;
	NewBladeSparkEffect( &mat, 2 ) ;
}

/* コールバック */
static	void	DefenseCallback( TARGET *off, TARGET *def, void *ptr )
{
	long64		wt ;
	Work		*work ;
	int			dam ;

	work = ( Work * )ptr ;
	if ( Status( work, SOL_STATE_INVINCIBLE ) ) {
		GM_ClearTargetDamage( def ) ;
		return ;
	}
	wt = off->weapon_type ;
	work->weapon_type = wt ;
	if ( wt & ( WP_BLADE | WP_BLADEFAINT | WP_BLADESTAB ) ) {
		if ( GV_Time == work->last_damage_time ) {
			/* 同フレームに他の子ターゲットに
			   当たっている */
			GM_ClearTargetDamage( def ) ;
			return ;
		}
		/* ブレードは剣振り中毎フレーム攻撃がくるので以下の処理 */
		if ( GV_Time - work->last_damage_time < 8 ) {
			work->last_damage_time = GV_Time ;
			GM_ClearTargetDamage( def ) ;
			return ;
		}

		if ( off->class & TARGET_ONLINE ) { 
			DG_COPY_VEC( &work->hit, &def->hit ) ;
		} else {
			DG_COPY_VEC( &work->hit, &off->center ) ;			
		}

		SetStatus( work, SOL_STATE_DAMAGED ) ;

		NewPadVibration( BladeDamVibH, 1 | VAR_FLAG_FORCE ) ;
		NewPadVibration( BladeDamVibL, 2 | VAR_FLAG_FORCE ) ;

		work->last_damage_time = GV_Time ;

#if 0
		if ( Flag( work, SOL_FLAG_CAREFUL ) ) {
			work->guard_hit = 1 ;
			if ( work->hit.vy > work->control.mov.vy ) {
				work->guard_mode = GUARD_SWAY_U ;
			} else {
				work->guard_mode = GUARD_SWAY_D ;
			}
			GM_SeSetMode( SD_V_SOLATK02, &work->control.mov, GM_SEMODE_BOMB ) ;
		} else if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) {
#else
		if ( Flag( work, SOL_FLAG_BLADE_GUARD ) ) {
#endif
			//printf( "game level %d\n", GM_GameLevel ) ;
			if ( SOL_GameLevel == GM_LEVEL_VERYEASY ) {
				SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
				ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
				goto guard_cancel ;
			} else if ( SOL_GameLevel == GM_LEVEL_EASY ) {
				if ( GV_Time % 8 > 2 ) {
					SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
					ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
					goto guard_cancel ;
				}
			}
			work->guard_hit = 1 ;
			if ( wt & WP_BLADESTAB ) {
				work->guard_mode = GUARD_DOUBLE ;
				PL_FillipSlash() ;
			}
			else if ( Flag( work, SOL_FLAG_SLASH ) ) work->guard_mode = GUARD_LEFT ;
			else if ( Flag( work, SOL_FLAG_SLASH_L ) ) work->guard_mode = GUARD_RIGHT ;
			else work->guard_mode = GUARD_DOUBLE ;
			GM_SeSetMode( SD_E_GURDKEN1, &work->control.mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
			BladeGuardSpark( off, def ) ;
		} else {
guard_cancel :
			work->guard_hit = 0 ;
			if ( !Flag( work, SOL_FLAG_FILLIP_NOW ) ) {
				if ( wt & WP_BLADEFAINT ) {
					GM_SeSetMode( SD_A_SWORDHIT, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_katana_02" ), 0 ) ;
				} else if ( wt & WP_BLADESTAB ) {
					GM_SeSetMode( SD_A_SWORDBIT, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_katana_03" ), 0 ) ;
				} else {
					GM_SeSetMode( SD_A_SWORDCUT, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_katana_01" ), 0 ) ;
				}
				dam = SOL_BLADE_DAMAGE ;
				if ( SOL_GameLevel < GM_LEVEL_NORMAL ) dam += work->cont_hit_count + 1 ;
				if ( SOL_GameLevel == GM_LEVEL_HARD ) dam = dam * 2 / 3 ;
				else if ( SOL_GameLevel == GM_LEVEL_EXTREME ) dam = dam * 1 / 3 ;
				if ( work->cont_hit_count == 2 ) dam ++ ;
				if ( dam <= 0 ) dam = 1 ;
				if ( wt & WP_BLADEFAINT ) {
					work->faint_damage += dam ;
				} else {
					work->damage += dam ;
				}
			} else {
				GM_SeSetMode( SD_A_SWORDHAD, &work->control.mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
				BladeGuardSpark( off, def ) ;
			}
		}
		GM_ClearTargetDamage( def ) ;
	} else if ( wt & ( WP_PUNCHR | WP_PUNCHL | WP_KICK ) ) {
		if ( GV_Time == work->last_damage_time ) {
			/* 同フレームに他の子ターゲットに
			   当たっている */
			GM_ClearTargetDamage( def ) ;
			return ;
		}
		/* 無視状態 */
		if ( Flag( work, SOL_FLAG_COMBO_IGNORE ) ) {
			GM_ClearTargetDamage( def ) ;
			return ;
		}

		if ( off->class & TARGET_ONLINE ) { 
			DG_COPY_VEC( &work->hit, &def->hit ) ;
		} else {
			DG_COPY_VEC( &work->hit, &off->center ) ;
		}

		SetStatus( work, SOL_STATE_DAMAGED ) ;
		work->last_damage_time = GV_Time ;

		if ( Flag( work, SOL_FLAG_COMBO_GUARD ) ) {
			if ( SOL_GameLevel == GM_LEVEL_VERYEASY ) {
				SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
				ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
				goto combo_guard_cancel ;
			} else if ( SOL_GameLevel == GM_LEVEL_EASY ) {
				if ( GV_Time % 8 > 2 ) {
					SetFlag( work, SOL_FLAG_FILLIP_NOW ) ;
					ResetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
					goto combo_guard_cancel ;
				}
			}
			work->guard_hit = 1 ;
			GM_SeSetMode( SD_A_SWORDHAD, &work->control.mov, GM_SEMODE_BOMB ) ;
			NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
		} else {
combo_guard_cancel :
			work->guard_hit = 0 ;
			if ( !Flag( work, SOL_FLAG_FILLIP_NOW ) ) {
				if ( wt & WP_KICK ) {
					dam = SOL_KICK_DAMAGE ;
					if ( SOL_GameLevel == GM_LEVEL_HARD ) dam = dam * 2 / 3 ;
					else if ( SOL_GameLevel == GM_LEVEL_EXTREME ) dam = dam * 1 / 3 ;
					if ( dam <= 0 ) dam = 1 ;
					work->faint_damage += dam ;
					work->cont_hit_count = 2 ;
				} else {
					dam = SOL_PUNCH_DAMAGE ;
					if ( SOL_GameLevel == GM_LEVEL_HARD ) dam = dam * 2 / 3 ;
					else if ( SOL_GameLevel == GM_LEVEL_EXTREME ) dam = dam * 1 / 3 ;
					if ( dam <= 0 ) dam = 1 ;
					work->faint_damage += dam ;
					//work->cont_hit_count = 0 ;/* 連続パンチ可能 */
				}
				if ( wt & WP_PUNCHR ) {
					GM_SeSetMode( SD_P_PUNCH02, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ;
				} else if ( wt & WP_PUNCHL ) {
					GM_SeSetMode( SD_P_PUNCH02, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_punch_02" ), 0 ) ;
				} else {
					GM_SeSetMode( SD_P_KICK02, &work->control.mov, GM_SEMODE_BOMB ) ;
					NewPadVibration2( GV_StrCode( "rai_kick_02" ), 0 ) ;
				}
			} else {
				GM_SeSetMode( SD_A_SWORDHAD, &work->control.mov, GM_SEMODE_BOMB ) ;
				NewPadVibration2( GV_StrCode( "rai_katana_04" ), 0 ) ;
			}
		}
		GM_ClearTargetDamage( def ) ;
	}
}

/* ダメージチェック */
static	void	CheckDamage( Work *work )
{
	CheckInvincible( work ) ;
	if ( Status( work, SOL_STATE_DAMAGED ) ) {
		if ( work->action == TriangleJump ) {
			work->control.skip_flag &= ~( CTRL_SKIP_FLR_CHECK | CTRL_SKIP_SEG_CHECK ) ;
			work->control.skip_flag |= CTRL_RESET_HZX_BASE ;			
			SetFlag( work, SOL_FLAG_NO_WAIST_INTERP ) ;
		}
		ResetStatus( work, SOL_STATE_DAMAGED ) ;
		DashFireStop( work ) ;
		if ( GM_IsGameOver() ) {
			/* プレイヤーが死んでいる */
			if ( work->life_gage.value <= 0 ) work->life_gage.value = 1 ;
			if ( work->life_gage.m9_value <= 0 ) work->life_gage.m9_value = 1 ;
			SetInvincible( work, 0 ) ;
		}
		if ( work->life_gage.value <= 0 ||
			work->life_gage.m9_value <= 0 ) {
			GV_SetActorChild( work, NewSolidusDamageSpark( &work->body ) ) ;
			if ( work->life_gage.value <= 0 ) {
 				if ( GM_KillCount < 30000 ) GM_KillCount ++ ;
			}
			SetMode( work, Dead ) ;
			PL_PlayerSetInvincible() ;
			GV_PadReleaseOn( 0 ) ;
			GM_SetMenuStatus( MENU_GAGE_OFF ) ;
		} else if ( work->guard_hit ) {
			work->cont_hit_count = 0 ;
#if 0
			if ( work->guard_mode == GUARD_SWAY_U ) {
				SetMode( work, SwayUpper ) ;
			} else if ( work->guard_mode == GUARD_SWAY_D ) {
				SetMode( work, SwayLower ) ;				
			} else if ( work->action != BladeGuard ) {
				SetMode( work, BladeGuard ) ;
			}
#endif
			if ( work->action != BladeGuard ) {
				SetMode( work, BladeGuard ) ;
			}
		} else {
			if ( Flag( work, SOL_FLAG_FILLIP_NOW ) ) {
				if ( Flag( work, SOL_FLAG_SLASH_L ) ) {
					SetMode( work, SlashFillipedL ) ;
				} else {
					SetMode( work, SlashFillipedR ) ;
				}
			} else {
				GV_SetActorChild( work, NewSolidusDamageSpark( &work->body ) ) ;
				work->cont_hit_count ++ ;
				if ( work->cont_hit_count > 3 ||
					( work->cont_hit_count == 3 &&
					 !( work->weapon_type & ( WP_PUNCHR | WP_PUNCHL ) ) ) ) {
					SetMode( work, Blow ) ;
					work->cont_hit_count = 0 ;
					work->control.turn.vy = work->rot_diff_player.vy ;
				} else {
					SetMode( work, BladeDamage ) ;
				}
			}
		}
	}
}

/* 攻撃チェック */
static	int		CheckAttack( Work *work, int flag )
{
//	if ( flag & SOL_FLAG_BLADE_ATTACK_ENABLE && 
//		 Flag( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
	if ( Flag( work, SOL_FLAG_BLADE_ATTACK_ENABLE ) ) {
		if ( work->len_diff_player < 1200.0F ) {
//			SetMode( work, Combo ) ;
			SetMode( work, SpinKick ) ;
		} else {
			if ( GM_CheckPlayerStatus( PLAYER_GROUND | PLAYER_DOWNED ) &&
				 GV_Time - work->last_slash_time < 300 / TIME_BASE * 5 ) {
				SetMode( work, PursueWalk ) ;
			} else if ( Flag( work, SOL_FLAG_BLADE_ATTACK_W ) ) {
				if ( ( irnd() % 16 ) < 5 ) SetMode( work, SlashCombo ) ;
				else					   SetMode( work, SlashDouble ) ;
			} else {
				if ( work->mode == SOL_MODE_BLADE_ONLY ) {
					if ( ( irnd() % 16 ) < 12 ) SetMode( work, SlashCombo ) ;
					else					    SetMode( work, SlashDownR ) ;
				} else {
					if ( ( irnd() % 16 ) < 4 ) SetMode( work, SlashCombo ) ;
					else					   SetMode( work, SlashDownR ) ;
				}
				//SetMode( work, SlashDown ) ;
			}
		}
		work->last_slash_time = GV_Time ;
		return 1 ;
	}
	return 0 ;
}

/* 三角飛びチェック */
static	int		CheckTriangleJump( Work *work )
{
	if ( GV_Time - work->far_think.triangle_count < 10 * ( 300 / TIME_BASE ) ) return 0 ;
	if ( work->mode == SOL_MODE_BLADE_ONLY ) return 0 ;	
	if ( work->len_diff_player < 2000.0F ) return 0 ;
	if ( work->control.mov.vz < 4000.0F ||
		 work->control.mov.vz > 18000.0F ) return 0 ;	/* 決めうちバウンディング */
	if ( DG_FABS( work->control.mov.vx - ( WALL_X - TRIANGLE_SHIFT ) ) < 500.0F ) return 1 ;
	return 0 ;
}

/* エルード攻撃チェック */
static	int		CheckEludeAttack( Work *work )
{
	if ( GM_CheckPlayerStatus( PLAYER_BEYOND ) &&
		 !GM_CheckPlayerStatus( PLAYER_FORCE ) ) {
		return 1 ;
	}
	return 0 ;
}	

/* 蛇手攻撃選択 */
static	void	SelectArmAttack( Work *work )
{
	int			c, next ;

	do {
		c = irnd() % 6 ;
		switch( c ) {
		case 0 :
		case 1 :
		case 2 :
			next = 0 ;
			break ;
		case 3 :
			next = 1 ;
			break ;
		case 4 :
		case 5 :
		default :
			next = 2 ;
		
		}
	} while ( work->last_arm_attack == next ) ;

	work->last_arm_attack = next ;

	switch( next ) {
	case 0 :
		SetMode( work, LiftUpStart ) ;
		break ;
	case 1 :
		SetMode( work, Trip ) ;
		break ;
	case 2 :
		SetMode( work, Claw ) ;
		break ;
	}
}
