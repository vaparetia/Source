/*
   normal.c 
   ライデン／通常移動関連
   
   1999/07/07 M.Sonoyama
   $Id: normal.c,v 1.1.1.3 2002/11/19 11:50:56 Yoshizawa1 Exp $			
   */

/*-----------------------------------------------------------*/

#include "BP_TrophySystem.h"

//#define	STEP_NO_GROUND

/* プロトタイプ宣言 */
//static	void	SubjectHoming( Work * ) ;
static	void	Homing( Work *, u_char ) ;

/*----------------------------------------------------------------*/

/* グローバル */

void			PL_RunToSquat( PlayerWork *work, int time )
{
	RunToSquat( work, time ) ;
}

/*----------------------------------------------------------------*/


/*　長もの武器用の壁立ち時の銃下げ */
void		PL_AvoidWeaponIntoWall( PlayerWork *work )
{
	if ( work->weapon == WP_m4 || work->weapon == WP_Aks ) {
		SVECTOR rot = { 0,0,0,0 } ;

		if ( work->arm_count ) {
			if ( !EnableShootBullet3( work, 0, 0.0F, 1100.0F ) ) {
				work->arm_count-- ;
			} else {
				work->arm_count = 8 ;
			}
		} else if ( EnableShootBullet( work ) ) { 
			work->arm_count = 8 ;
		}
		if ( work->arm_count > 0 ) {
			rot.vx = 600 ;
			SetRotAdjust( work, &rot, HUMAN21_MIGI_UDE1 ) ;
			rot.vx = 640 ;
			SetRotAdjust( work, &rot, HUMAN21_HIDARI_UDE1 ) ;
		}
	} else {
		work->arm_count = 0 ;
	}
}

/* 立ち静止 */
static	void	StandStill( Work *work, int time )
{
    int		diff ;

	/* いまは、スネーク用に長もの武器のめり込みチェックをする */
	if ( (GM_GameStatus & STATE_VR_ANOTHER) && Status( PLAYER_SNAKE ) )
	  PL_AvoidWeaponIntoWall( work ) ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetStatus( PLAYER_EVENT_ENABLE ) ;
    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		work->act_name = STAND_STILL ;
		if ( work->data == 0 ) PL_FootPrintForce( work->foot_work, 3 ) ;
    }

	if ( CheckWeaponIntoWall( work ) ) {
		SetAction( work, MS.change[ Mstand_wall ], 6 ) ;
	} else if ( !UpsideStep( work ) ) {
		SetAction( work, MS.change[ Mstand ], 6 ) ;	
    } else if ( GM_AlertMode == ALERT_MODE_ALERT &&
			   work->weapon == WP_None ) {
		float	len ;

		len = NearestEnemyLen() ;
		if ( len < 4000.0F ) {
			SetAction( work, MS.change[ Mstand_alert ], 6 ) ;
		} else {
			if ( work->motion1 == MS.change[ Mstand_alert ] && len < 6000.0F ) {
				SetAction( work, MS.change[ Mstand_alert ], 6 ) ;
			} else {
				SetAction( work, MS.change[ Mstand ], 6 ) ;			
			}
		} 
    } else {
		SetAction( work, MS.change[ Mstand ], 6 ) ;
    }

    if ( Status( PLAYER_NEED_NEW_PRESS ) ) return ;



	/*Subjective need only move*/
	if ( PL_SubjectMove == 2 || PL_SubjectMove == 3 ) {
		if ( !Flag( FLAG_CANNOT_PEEP ) &&
		     (work->pad->status & PL_PAD_STOP_AIM) &&
		     (work->pad->status & PL_PAD_PEEP    ) ) {
			SubjectPeep( work, work->pad->status ) ;
			GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
		}
	}

    if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
		if ( work->pad->press & PL_PAD_SQUAT ) {
			SetMode( work, SquatStill ) ;
			return ;
		}
    } else {
	/*Progress at the time of the attack buttons*/
	if (
	    (PL_SubjectMove==2 || PL_SubjectMove == 3) ? /* Take in the future */

	    (work->pad->status & PAD_UDLR &&
	     !(work->pad->status & PL_PAD_STOP_AIM) )

	    : /*Take in the future*/

	    work->pad->status & PAD_UDLR

	    ) {

	    /*Subjective movement*/
	    if ( PL_SubjectMove/*T.Morita Added*/ ) {
		SetModeName( work, StandRun, "StandRun" ) ;
		return ;
	    }
			if ( !Status( PLAYER_ON_CORPSE ) &&
				 Liable == FRONT && 
				 GV_DiffDirAbs( WallTo, PadTo ) < 256 ) {
				int			c = 0 ;
				
				if ( GV_DiffDirAbs( WallTo, work->control.rot.vy + 2048 ) < 256 ) {
					c = 1 ;
				} else if ( ++ work->data2 >= 6 && PadForce > PAD_CAUTION_TH ) {
					c = 1 ;
				} 
				if ( c ) {
					/* 十字キーの状態を記憶（ 主観用 ） */
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ; 
					SetMode( work, StandCautionStill ) ;
					return ;
				}
			} else {
				work->data2 = 0 ;
			}
			if ( work->data != 0 ) {
				-- work->data ; 
				return ;
			}
			if ( work->action2 != NULL ) {
				if ( ( work->pad->status & PL_PAD_READY_RUN)  || 
#if 0
					( GM_WeaponTypes[ work->weapon ] & WP_TYPE_GRENADE ) ||
					( Flag( FLAG_RELOADING ) ) ) {
#else
					( GM_WeaponTypes[ work->weapon ] & WP_TYPE_GRENADE ) ) {
#endif
                    SetModeName( work, StandRun, "StandRun" ) ;
				} else {
					SeekTurn( work ) ;
				}
			} else {
				diff = GV_DiffDirAbs( work->control.turn.vy, PadTo ) ;
				if ( diff > TURN_MARGIN ) { /* 主観移動モード以外のときに方向を見る */
					work->data = diff / TURN_COUNT_VALUE + 1 ;
				} else {
					SetModeName( work, StandRun, "StandRun" ) ;
					return ;
				}
				if ( PadTo >= 0 )
				work->control.turn.vy = PadTo ;
			}
		} else {
			if ( work->pad->press & PL_PAD_SQUAT ) {
				SetMode( work, SquatStill ) ;	
				return ;
			}
    	}
    }

    CheckAttack( work ) ;
}

static	inline	void	EndStandRun( Work *work )
{
	UnsetStandWallAction2( work ) ;
}

/* 走り */
static	void	StandRun( Work *work, int time )
{
	int		enable, max, to ;
	float	t ;

	/* いまは、スネーク用に長もの武器のめり込みチェックをする */
	if ( (GM_GameStatus & STATE_VR_ANOTHER) && Status( PLAYER_SNAKE ) )
	  PL_AvoidWeaponIntoWall( work ) ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;

	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		PL_FootPrintAct( work->foot_work, 1, -1, -1 ) ;
		work->idata = -1 ;
    }

	enable = CheckWeaponIntoWall( work ) ;
	if ( enable ) {
		SetStandWallAction( work, 3 ) ;
	} else {
		UnsetStandWallAction2( work ) ;
	}

    if ( Status( PLAYER_WATCH ) ) {
	SetMode( work, StandStill ) ;
	return ;
    } else {
	if ( work->action2 != NULL &&
	     ( !( work->pad->status & PL_PAD_READY_RUN || PL_SubjectMove/*T.Morita Added*/ ) &&
	       !( GM_WeaponTypes[ work->weapon ] & WP_TYPE_GRENADE ) &&
	       !Flag( FLAG_RELOADING ) ) ) {
	    if ( work->homing != NULL ) {
		work->control.rot.vy += work->rot_adjusts[ HUMAN21_KOSHI ].vy ;
		work->control.turn.vy = work->control.rot.vy ;
	    }
	    EndStandRun( work ) ;
	    SetMode( work, StandStill ) ;
	    return ;
	} else if ( work->pad->press & PL_PAD_SQUAT ) {
	    if ( work->motion1 != MS.change[ Mwalk ] &&
		 work->motion1 != MS.change[ Mstair_walk ] &&
		 !( ( WeaponType( work ) & WP_TYPE_GRENADE ) && ( work->action2 != NULL ) ) &&
		 ( work->control.grounded & 1 ) &&
		 !CheckTrap( work, NO_CROUCH_TRAP ) &&	
		 ( ( CheckGroundWidth( work, &work->control.rot ) & 4 ) == 0 ) ) {
		StopTurn( work ) ;
		SetMode2( work, NULL ) ;
		SetMode( work, RunToSquat ) ;	
      if( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) )
      {
         //AndyO: Removed in MGS_2_and_3_Trophy_Info_v2_110527_rev.xls
         //BP_TrophySystem_UnlockTrophy( kTRP_NakedCartwheel );
      }

		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	    } else {
		SetMode( work, SquatStill ) ;
		work->data = 1 ;	/* 足跡つける */
	    }
	    EndStandRun( work ) ;
	    return ;
	} else if (
	    (PL_SubjectMove==2 || PL_SubjectMove == 3) ? /* Take in the future */

	    /*Progress at the time of the attack buttons*/
	    work->pad->status & PAD_UDLR &&
	    !(work->pad->status & PL_PAD_STOP_AIM)

	    : /*Take in the future*/

	    work->pad->status & PAD_UDLR

	    ) {

	    if ( !Status( PLAYER_ON_CORPSE ) &&
		 Liable == FRONT &&
		 !PL_SubjectMove && /* 主観移動モードではビハインドしない */
		 GV_DiffDirAbs( WallTo, PadTo ) < 256 ) {
                /* 足音をならさないようにする */
                work->seNoFlr = -1 ;
                if ( PadForce > PAD_CAUTION_TH ) work->data += 2 ;
                else work->data ++ ;
		if ( work->data >= 12 ) { /* これ以上張り付き可能なら張り付く */
		    /* 十字キーの状態を記憶（ 主観用 ） */
		    work->caution_dir = ( work->pad->status & PAD_UDLR ) ; 
		    SetMode( work, StandCautionStill ) ;
		    EndStandRun( work ) ;
		    return ;
		}
	    } else {
		work->data = 0 ;
		if ( PadForce > PAD_WALK_TH ) {
		    to = ReactWall( PadTo ) ;
		} else {
		    to = PadTo ;
		}
		if ( !PL_SubjectMove ) { /* 主観移動モード以外のときに方向を更新する */
		    /* チャタリング防止策 */
		    if ( work->idata >= 0 && GV_DiffDirAbs( work->idata, to ) < 256 ) {
			work->control.turn.vy = to ;
		    }
		} else {
		    /* 主観移動用のステップ計算 */
		    SubjectMoveStep( work ) ; /* subject.c */
		}


		/* 体の傾き */
		work->idata = to ;
		if ( work->motion1 != MS.change[ Mwalk ] &&
		     work->motion1 != MS.change[ Mwalk_on ] &&
		     work->motion1 != MS.change[ Mstair_walk ] && 
		     work->motion1 != MS.change[ Mstair_run ] ) Incline( work ) ;
		else work->control.turn.vz = 0 ;
	    }
	} else {
	    SetMode( work, StandStill ) ;
	    EndStandRun( work ) ;
	    return ;
	}
    }
		
#ifdef WALK_PATTERN_A
	if ( work->data2 == 0 ) {
		work->data2 = 1 ;
		t = 0.0F ; 
	} else {
		t = ( work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
			 + work->body.m_ctrl->mt3_ctrl[ 0 ].motion_time_base )
			/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
	}

	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 1.0F ) ;
	if ( Status( PLAYER_ON_CORPSE ) ) {
		//SetAction4( work, MS.change[ Mwalk_on ], t, 6 ) ;
		SetAction( work, MS.change[ Mwalk_on ], 6 ) ;
		if ( GM_AlertMode == ALERT_MODE_ALERT ) {
			/* 危険モード時２倍 */
			MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 2.00F ) ;
		}
        SetStatus( PLAYER_WALK ) ;
        SetFlag( FLAG_NO_IK ) ; /* 死体走りはＩＫなし */
		max = 1 ;
	} else {
		max = PL_MoveLevel( work ) ;
		if ( max != 0 ) {
			if ( !UpsideStep( work ) ) {
				if ( max == 1 ) {
					SetAction4( work, MS.change[ Mstair_walk ], t, 6 ) ;
				} else {
					SetAction4( work, MS.change[ Mstair_run ], t, 6 ) ;
				}
			} else {
				if ( max == 1 ) {				
					SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
				} else {
					SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
				}
				//SetFlag( FLAG_NO_IK ) ; /* 通常走りはＩＫあり */
			}
			if ( max == 1 || time < 12 * TIME_BASE ) SetStatus( PLAYER_WALK ) ;
			else 			SetStatus( PLAYER_DASH ) ;
		}
#if 0
		max = PadForce ;
		if ( !( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) ) {
			max = ( int )( ( float )max * 2.4F ) ;
		}
		if ( max != 0 ) {
			if ( !UpsideStep( work ) ) {
				if ( max < PAD_WALK_TH ) {
					SetAction4( work, MS.change[ Mstair_walk ], t, 6 ) ;
					SetStatus( PLAYER_WALK ) ;
				} else {
					SetAction4( work, MS.change[ Mstair_run ], t, 6 ) ;
                    SetStatus( PLAYER_DASH ) ;
//					SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
//                    MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 0.7F ) ;
//                    SetFlag( FLAG_NO_IK ) ;
				}
			} else {
				if ( max < PAD_WALK_TH ) {
					SetStatus( PLAYER_WALK ) ;
					SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
				} else if ( max < PAD_RUN_TH ) {
					SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
				} else {
//					SetAction4( work, MS.change[ Mdash ], t, 6 ) ;
					SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
                    SetStatus( PLAYER_DASH ) ;
				}
			}
		}
#endif
	}
#else
	if ( work->pad->status & PAD_AL ) {
		SetAction( work, MS.change[ Mwalk ], 6 ) ;	
	} else {
		SetAction( work, MS.change[ Mdash ], 6 ) ;
	}
#endif

	/* 足跡 */
	if ( !Status( PLAYER_ON_CORPSE ) ) {
		if ( max == 1 ) {
			if ( Status( PLAYER_SNAKE ) ) {
				PL_FootPrintAct( work->foot_work, 0, 13, 42 ) ;
			} else {
				PL_FootPrintAct( work->foot_work, 0, 16, 46 ) ;
			}
		} else if ( max == 2 ) {
			if ( Status( PLAYER_SNAKE ) ) {
				PL_FootPrintAct( work->foot_work, 0, 30, 10 ) ;
			} else {
				if ( work->weapon == WP_Blade ) {/* 刀装備のときはこっち */
					PL_FootPrintAct( work->foot_work, 0, 34, 15 ) ;
				} else {
					PL_FootPrintAct( work->foot_work, 0, 37, 17 ) ;
				}
			}
		}
	}

	CheckAttack( work ) ;

	/* めりこみ防止処理（ちょっと重い） */
	{
		float	front, back ;	
		FVECTOR	shift = { 0.0F, 0.0F, 0.0F } ;
		FVECTOR	mov ;

		PL_GetModelLength2( work, &front, &back, 0x1 << HUMAN21_ATAMA ) ; /* 頭だけ */
		front += 250.0F ;
		DG_COPY_VEC( &mov, &work->control.mov ) ;
		mov.vy = ObjHeight( work, HUMAN21_ATAMA ) ;
		if ( CheckMovRotLenSegment( work->control.hzx_id, &mov, &DG_ZeroVector,
								    &work->control.rot, front, 
								    HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0 ) ) {
			shift.vz = GV_VecLen3F2( &mov, &ResultPoint ) - front ;
			DG_SetPos2( &work->control.mov, &work->control.rot ) ;
			DG_RotVector( &shift, &shift, 1 ) ;
			AdjustXZ( work, &shift ) ;
		}
	}
}

static	void	RunToSquatCallback( TARGET *off, TARGET *def, void *ptr )
{
	if ( def->class & ( TARGET_DOWN | TARGET_DEAD ) ) {
		GM_TargetHitCancel( off, def ) ;
		off->damaged &= ~TARGET_POWER ;
	}
}

/* 転がりしゃがみ静止 */
/* だったけれども、立ち静止に変更になりました */
static	void	RunToSquat( Work *work, int time )
{
	static 	u_char 	TouchDownVibH[] = { 127, 2, 0, 0 } ;
	static 	u_char	TouchDownVibL[] = { 64, 6, 28, 20, 64, 6, 0, 0 } ;

	TARGET	*t ;
	FVECTOR	force ;
	int		side, res, dir ;

	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	if ( time == 0 ) {
		/* 主観から抜ける */
		LeaveSubject( work ) ;
		ChangeStance( work, STAND ) ;
		StopTurn( work ) ;
		SetAction( work, MS.shared[ Mrun_squat ], 6 ) ;
		work->idata = 0 ;
		work->fdata = work->control.levels[ 0 ] ;
		work->control.step.vy = 0.0F ;
		work->sv.vx = 0 ;
	}

	SetStatus( PLAYER_MOVE | PLAYER_ROLLING ) ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_IK | FLAG_NO_TOUCH_DAMAGE |
			 FLAG_DONOT_CHECK_WATCH ) ;
	/* トラップチェック位置 */
	work->control.root_offset = NULL ;

	/* 敵にぶつかったら吹っ飛び */
	side = ENEMY_SIDE ;
	if ( work->ftime > 0 && work->ftime <= 24 &&
		( work->offense.damaged & TARGET_POWER ) ) {
		//side = PLAYER_SIDE ;	/* 自分は吹っ飛ばないように改変 */
        //NewPadVibration( DamageVib2H, 1 | VAR_FLAG_FORCE ) ;
		//NewPadVibration( DamageVib2L, 2 | VAR_FLAG_FORCE ) ;
	}
	if ( work->ftime < 24 || side == PLAYER_SIDE ) {
		t = &work->offense ;
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_POWER | TARGET_ONLINE | TARGET_ONLINE_MIN,
					 GM_CurrentStageMap, side, &DG_ZeroVector, &DG_ZeroVector ) ;
		GM_MoveOnlineTarget( t, &work->pre_mov, &work->control.mov ) ;
		if ( side == ENEMY_SIDE ) GM_SetTargetWeaponType( t, WP_KICK ) ;
		else			  GM_SetTargetWeaponType( t, WP_PUNCH ) ;
		_sceVu0SubVector( &force, &work->control.mov, &work->pre_mov ) ;
		GM_SetPowerTarget( t, &work->attack, POWER_CONST, 255, 2, 0, &force ) ;
		GM_SetTargetCallBack( t, RunToSquatCallback, work ) ;
		GM_PutTarget( t ) ;
	}
	/* 匍匐ＩＫ */
#if 0
	{
		FVECTOR		max, min ;

		GetModelHeight2( work, &max, &min ) ;
		if ( max.vz - min.vz > 1200.0F ) {
			GroundIK2( work ) ;
		} else {
			ResetGroundAdjust( work ) ;
		}
	}
#endif
	/* めりこみ阻止 */
//	res = AvoidSink( work, 250.0F ) ;

	if ( Status( PLAYER_SNAKE ) ) {
		/* スネークの場合 */
		res = AvoidSink2( work, NEAR_HEIGHT_GROUND, 900.0F, 1090.0F, 1999.0F ) ;
	} else {
		/* ライデンの場合 */
		if ( ( work->control.grounded & 1 ) ||
			 ( work->control.hzx_base - work->control.levels[ 0 ] < 500.0F ) ) {
			if ( work->data == 0 || work->data == 2 ) {
				res = AvoidSink2( work, NEAR_HEIGHT_STAND, 950.0F, 1040.0F, 1999.0F ) ;
			} else {
				res = AvoidSink2( work, NEAR_HEIGHT_GROUND, 950.0F, 1040.0F, 1999.0F ) ;
			}
		} else {
			res = 0 ;
		}
	}

	switch( work->data ) {
	case 0 :		/* 跳び開始 */
		/* 落下速度をソフトに（ライデン） */
		if ( !Status( PLAYER_SNAKE ) ) {
			SetFlag( FLAG_NO_GRAVITY ) ;
			if ( !( work->control.grounded & 1 ) ) {
				SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
			} else if ( work->control.levels[ 0 ] - work->fdata > 250.0F ) {
				SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
			}
			work->control.step.vy = GV_NearExpNF( work->control.step.vy, -GRAVITY, 
												  64.0F ) ;
			/* 重力＆移動量調整 */
			if ( DG_FABS( PL_JumpGravity ) > 0.10F ) {
				FVECTOR		stepadj ;

				work->control.step.vy += PL_JumpGravity ;
				stepadj.vx = work->body.m_ctrl->step.vx ;
				stepadj.vz = work->body.m_ctrl->step.vz ;
				stepadj.vy = 0.0F ;
				GV_LenVec3F( &stepadj, &stepadj, 0.0F, PL_JumpGravity * 12.0F ) ;
				work->control.step.vx += stepadj.vx ;
				work->control.step.vz += stepadj.vz ;
				//DumpVec( &work->control.step ) ;
			}

			GM_ConfigControlNearCheck2( &work->control, 
									    work->fdata + NEAR_HEIGHT_STAND - work->control.hzx_base ) ;
		}

		GroundIK2( work, 800.0F ) ;
		/* めり込み回避 */
		if ( res & 0x01 ) work->idata |= 2 ;
		if ( work->idata & 2 ) SetFlag( FLAG_NO_MOTION_STEP ) ;
		if ( work->control.interp == 0 ) {
			if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) ) {
				work->idata |= 1 ;
#if 0
				dir = CheckGroundEnableDir( work ) ;
				if ( dir != 1 ) {
printf( "raiden/normal.c : roling search dir\n" ) ;
					work->control.turn.vy = dir ;
					work->control.interp = 8 ;
				} else {
printf( "raiden/normal.c : roling no dir\n" ) ;
				}
#endif
			}
		}
		if ( ( PL_CheckMotionTime( work, 17 ) ) &&
			 ( Status( PLAYER_SNAKE ) || 
			   work->fdata < work->control.levels[ 0 ] ) &&
			 ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) {
			ChangeStance( work, GROUND ) ;
//			SetAction3( work, MS.shared[ Mrun_squat_end_face ], 17, 6 ) ;
			SetAction( work, MS.shared[ Mrun_squat_end_face ], 6 ) ;
			SetFlag( FLAG_NO_STEP ) ;
			work->data = 4 ; work->data2 = 0 ;	/* うつ伏せダウンへ */
			break ;
		} else if ( PL_CheckMotionTime( work, 36 ) ) {
			if ( Status( PLAYER_SNAKE ) ) {
				if ( ( ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ||
					   ( work->fdata - work->control.levels[ 0 ] > 2000.0F && 
						 work->fdata - work->control.levels[ 0 ] < 4000.0F ) ) ) {
					work->down_dir = DOWN_BACK ;
					NewPadVibration( DamageVib1H, 1 ) ;
					NewPadVibration( DamageVib1L, 2 ) ;
					GM_SeSetMode( SD_E_DOWN03, &work->control.mov, GM_SEMODE_BOMB ) ;
					SetFlag( FLAG_NO_STEP ) ;
					SetMode( work, DownDamage ) ;		/* 仰向けダウンへ */
					break ;							
				}
			} else {
				if ( ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) &&
					 ( work->fdata < work->control.levels[ 0 ] ) ) {
					if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
						work->down_dir = DOWN_FACE ;	/* はだかのときはうつぶせ（要チェック） */
					} else {
						work->down_dir = DOWN_BACK ;
					} 
					NewPadVibration( DamageVib1H, 1 ) ;
					NewPadVibration( DamageVib1L, 2 ) ;
					GM_SeSetMode( SD_E_DOWN03, &work->control.mov, GM_SEMODE_BOMB ) ;
					SetFlag( FLAG_NO_STEP ) ;
					SetMode( work, DownDamage ) ;		/* 仰向けダウンへ */
					break ;							
				}
			}
		}
		if ( EndMotion( work ) ) {
#if 0
			if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
				SetAction( work, MS.shared[ Mrun_squat_end_do ], 6 ) ;
				work->data = 3 ; work->data2 = 0 ;
			} else if ( PadTo >= 0 && 
					   GV_DiffDirAbs( PadTo, work->control.rot.vy ) < 512 ) {
				SetAction( work, MS.shared[ Mrun_squat_end_cr ], 6 ) ;
				work->data = 1 ;
			} else {
				//		SetAction( work, MS.shared[ Mrun_squat_end_sq ], 6 ) ;
				SetAction( work, MS.shared[ Mrun_squat_end_st ], 6 ) ;
				work->data = 2 ;
			}
#else
			if ( ( ( work->idata & 1 ) == 0 ) && ( work->pad->status & PL_PAD_SQUAT ) && 
				( work->control.grounded & 1 ) &&
				!Status( PLAYER_ON_CORPSE ) ) {
				ChangeStance( work, GROUND ) ;
				SetAction( work, MS.shared[ Mrun_squat_end_cr ], 6 ) ;
				work->data = 1 ;			/* 匍匐終了へ */
				if ( !Status( PLAYER_SNAKE ) && ( work->control.grounded & 1 ) ) {
					NewPadVibration2( GV_StrCode( "rai_tumble_end" ), 0 ) ;
					work->sv.vx = 1 ;
				}
			} else {
				//SetAction( work, MS.shared[ Mrun_squat_end_sq ], 6 ) ;
				SetAction( work, MS.shared[ Mrun_squat_end_st ], 6 ) ;
				work->data = 2 ;			/* 立ち終了へ */
				if ( !Status( PLAYER_SNAKE ) && ( work->control.grounded & 1 ) ) {
					NewPadVibration2( GV_StrCode( "rai_tumble_end" ), 0 ) ;
					work->sv.vx = 1 ;
				}
			}
#endif
		}
		break ;
	case 1 :		/* 匍匐終了へ */
		/* 立ち高さでもチェック */
		GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_STAND ) ;
		GroundIK2( work, 800.0F ) ;
		/* めり込み回避 */
		if ( work->control.interp == 0 ) {
			if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) || 
				 ( ( work->control.grounded & 1 ) && ( res & 0xf0  ) ) ) {
				dir = CheckGroundEnableDir( work ) ;
				if ( dir != 1 ) {
printf( "raiden/normal.c : roling search dir\n" ) ;
					work->control.turn.vy = dir ;
					work->control.interp = 8 ;
				} else {
printf( "raiden/normal.c : roling no dir \n" ) ;
				}
			}
		}
		if ( !Status( PLAYER_SNAKE ) && work->sv.vx == 0 ) {
			if ( work->control.grounded & 1 ) {
				if ( work->fdata - work->control.levels[ 0 ] > 3000.0F ) {
					NewPadVibration( TouchDownVibH, 1 ) ;
					NewPadVibration( TouchDownVibL, 2 ) ;
				} else {
					NewPadVibration2( GV_StrCode( "rai_tumble_end" ), 0 ) ;
				}
				work->sv.vx = 1 ;
			}
		}
		if ( EndMotion( work ) ) {	
			SetMode( work, GroundStill ) ;
		}
		break ;
	case 2 :		/* 立ち終了へ */
		GroundIK2( work, 300.0F ) ;
		if ( res != 0 ) work->idata |= 2 ;
		if ( work->idata & 2 ) SetFlag( FLAG_NO_MOTION_STEP ) ;
		if ( !Status( PLAYER_SNAKE ) && work->sv.vx == 0 ) {
			if ( work->control.grounded & 1 ) {
				if ( work->fdata - work->control.levels[ 0 ] > 3000.0F ) {
					NewPadVibration( TouchDownVibH, 1 ) ;
					NewPadVibration( TouchDownVibL, 2 ) ;
				} else {
					NewPadVibration2( GV_StrCode( "rai_tumble_end" ), 0 ) ;
				}
				work->sv.vx = 1 ;
			}
		}
		if ( EndMotion( work ) ) {	
			//	    SetMode( work, SquatStill ) ;
			//	    SetStatus( PLAYER_SQUAT ) ; /* SE対策 */
			SetMode( work, StandStill ) ;
		}
		break ;
	case 3 :		/* 仰向けダウンへ（未使用） */
		GroundIK2( work, 300.0F ) ;
		/* めり込み回避 */
		if ( work->control.interp == 0 ) {
			if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) || ( res & 0xf0 ) ) {
				dir = CheckGroundEnableDir( work ) ;
				if ( dir != 1 ) {
					work->control.turn.vy = dir ;
					work->control.interp = 8 ;
				}
			}
		}
		work->down_dir = DOWN_BACK ;
		if ( PL_CheckMotionTime( work, 52 ) ) {
			NewPadVibration( DamageVib1H, 1 ) ;
			NewPadVibration( DamageVib1L, 2 ) ;
		}
		if ( EndMotion( work ) ) {
			SetMode( work, Down ) ;
		}
		break ;
	case 4 :		/* うつ伏せダウンへ */
		GroundIK2( work, 800.0F ) ;
		/* めり込み回避 */
		if ( work->control.interp == 0 ) {
			if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) || ( res & 0xf0 ) ) {
				dir = CheckGroundEnableDir( work ) ;
				if ( dir != 1 ) {
					work->control.turn.vy = dir ;
					work->control.interp = 8 ;
				}
			}
		}
		if ( Status( PLAYER_SNAKE ) ) work->down_dir = DOWN_FACE ;
		else						  work->down_dir = DOWN_BACK ; /* ライデンは仰向け */
		if ( PL_CheckMotionTime( work, 22 ) ) {
			NewPadVibration( DamageVib1H, 1 ) ;
			NewPadVibration( DamageVib1L, 2 ) ;
		}
		if ( EndMotion( work ) ) {
			SetMode( work, Down ) ;
		}
		break ;
	}
}

/* しゃがみ静止 */
static	void	SquatStill( Work *work, int time )
{
	int		diff, dir, c ;

	/* いまは、スネーク用に長もの武器のめり込みチェックをする */
	if ( (GM_GameStatus & STATE_VR_ANOTHER) && Status( PLAYER_SNAKE ) )
	  PL_AvoidWeaponIntoWall( work ) ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetStatus( PLAYER_EVENT_ENABLE ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;
	if ( time == 0 ) {
		ChangeStance( work, SQUAT ) ;
		SetAction( work, MS.change[ Msquat ], 6 ) ;
		work->act_name = SQUAT_STILL ;
		/* 最初の数フレームだけ */
		/* バグりそう・・・ */
		work->hzx_height = 	
			work->control.hzx_height = HzxHeight[ STAND ] ;
		if ( work->data == 1 ) PL_FootPrintForce( work->foot_work, 3 ) ;
		work->data = 0 ;
	}
	if ( work->ftime == 1 ) work->hzx_height = HzxHeight[ SQUAT ] ;
	if ( work->control.hzx_height < 450.0F && Liable != NEUTRAL ) {
		UnsetStatus( PLAYER_NEED_NEW_PRESS ) ;
	}
	if ( Status( PLAYER_NEED_NEW_PRESS ) ) return ;
	if ( work->control.hzx_height >= 500.0F ) return ;

	switch( work->data2 ) {
	case 0 :
		if ( work->pad->press & PL_PAD_SQUAT ) {
			/* 立ち移行 */
			SetMode( work, StandStill ) ;
			work->data = 1 ;	/* 足跡ださない */
			return ;
		}

      //BP_INPUT - check for holding squat button to go to ground
      if( PL_IsPressSquat( work ) )
      {
         // Goto ground
         work->control.rot.vy = work->control.turn.vy ;
         SetFlag( FLAG_NO_STEP ) ;
         SetMode( work, SquatToGround ) ;
         SetMode2( work, NULL ) ;
         return;
      }
      //BP_INPUT - check for holding squat button to go to ground

		if ( Status( PLAYER_WATCH ) ) {
			SubjectTurn( work ) ;
		} else {
			if ( work->data != 0 ) {
				/* 回転中 */
				-- work->data ;
				return ;
			}
			if ( work->pad->status & PAD_UDLR ) {
				if ( !Status( PLAYER_ON_CORPSE ) &&
				     !PL_SubjectMove && /*主観移動モードでは張り付かない*/
				     Liable != NEUTRAL ) {
					/* はりつきへ */
					/* 十字キーの状態を記憶（ 主観用 ） */
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
					SetMode( work, SquatCautionStill ) ;
					return ;
				}
				if ( work->action2 != NULL ) {
					SeekTurn( work ) ;
				} else {
					diff = GV_DiffDirAbs( work->control.rot.vy, PadTo ) ;
					/* 主観移動以外のとき */
					if ( diff > TURN_MARGIN &&
					     !PL_SubjectMove ) {
						/* 向き替え */
						work->data = diff / TURN_COUNT_VALUE + 1 ;
						work->control.turn.vy = PadTo ;
					} else {
					    if ( !PL_SubjectMove ) {
						work->control.turn.vy = PadTo ;
					    }

						if ( !CheckTrap( work, NO_CROUCH_TRAP ) &&
							 !Status( PLAYER_ON_CORPSE ) ) {
							c = CheckGroundWidth( work, &work->control.turn ) ;
							if ( ( c & 0xf4 ) == 0 && ( c & 3 ) != 3 ) {
								/* その方向に匍匐出来る */
								work->control.interp = 4 ;
								work->data2 = 1 ;
							} else {
								/* 匍匐出来そうな方向にむいたのち、
								   匍匐行き */
								dir = CheckGroundEnableDir( work ) ;
								if ( dir >= 0 ) {
									work->control.turn.vy = dir ;
									work->control.interp = 16 ;
									work->data2 = 1 ;
								}
							}
						}
					}
				}
			}
		}
		break ;
	case 1 :
		if ( work->control.interp == 0 ) {
			work->control.rot.vy = work->control.turn.vy ;
			SetFlag( FLAG_NO_STEP ) ;
			SetMode( work, SquatToGround ) ;
			SetMode2( work, NULL ) ;
		}
		return ;
	}
	if ( work->data2 != 1 ) CheckAttack( work ) ;
}

/* しゃがみから匍匐 */
static	void	SquatToGround( work, time )
Work		*work ;
int		time ;
{
	int		act, flr_flag ;
	FVECTOR	head, mov ;
	float	levels[ 2 ] ;

	if ( time == 0 ) {
		/* 主観禁止 */
		LeaveSubject( work ) ;
		ChangeStance( work, GROUND ) ;
		StopTurn( work ) ;
		act = MS.change[ Msquat_crouch ] ;
		/* 匍匐した先の頭上に低い天井がある場合はモーションが違う */
		DG_COPY_VEC( &mov, &work->control.mov ) ;
		mov.vy = work->control.levels[ 0 ] + 150.0F ;
		GV_GetShiftPos( &head, &mov, &work->control.rot, 0.0F, 0.0F, 750.0F ) ;
		work->fdata = 0.0F ;
		if ( !HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &head, HZX_CHK_ALL,
									 HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
			flr_flag = HZX_LevelHazardCheck( work->control.hzx_id, &head, HZX_CHK_ALL, 
											 HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ;
			if ( flr_flag & 2 ) {
				HZX_GetLevelHeight( levels ) ;
				if ( levels[ 1 ] - head.vy < 1500.0F ) {
					act = MS.change[ Msquat_crouch_int ] ;
					if ( !Status( PLAYER_SNAKE ) ) {
						work->fdata = -64.0F ;
					}
				}
			}
		}
		SetAction( work, act, 6 ) ;
	}

	/* ライデン用位置調整 */
	if ( !Status( PLAYER_SNAKE ) && work->fdata < 0.0F ) {
		FVECTOR		adj = { 0.0F, 0.0F, work->fdata } ;

		work->fdata = GV_NearExp4F( work->fdata, 0.0F ) ;
		DG_SetPos( &work->body.objs->world ) ;
		DG_RotVector( &adj, &adj, 1 ) ;
		work->control.step.vx += adj.vx ;
		work->control.step.vz += adj.vz ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_IK | FLAG_DONOT_CHECK_WATCH ) ;
	if ( PL_SubjectMove ) {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &work->camera ) ;
	} else {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
	}

	/* 匍匐ＩＫ */
	GroundIK2( work, 750.0F ) ;
	AvoidSink( work, NEAR_HEIGHT_GROUND, 0.0F ) ;
	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			SetAction( work, MS.change[ Mcrouch ], 6 ) ;
			work->data ++ ;
		}
		break ;
	case 1 :
		SetFlag( FLAG_NO_STEP ) ;
		if ( CheckIntrude( work ) ) {
			IntoIntrude( work ) ;
			SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;
			SetMode( work, GroundFromToIntrude ) ;
		} else {
			SetMode( work, GroundStill ) ;
		}
	}
}

/* 匍匐静止 */
static	void	GroundStill( Work *work, int time )
{
	int		press, diff, n, turn, c ;
	SVECTOR	rot ;


	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	SetStatus( PLAYER_EVENT_ENABLE ) ;
	SetFlag( FLAG_RECOVER_ENABLE | FLAG_CAMERA_ON_WAIST ) ;	
	work->camera.vy += 250.0F ;

	press = work->pad->press ;
	if ( time == 0 ) {
		ChangeStance( work, GROUND ) ;
		SetAction( work, MS.change[ Mcrouch ], 6 ) ;
		work->act_name = GROUND_STILL ;
		work->idata = -1 ;
	}

	GroundIK2( work, 500.0F ) ;
#ifdef STEP_NO_GROUND
	if ( ( ( press & PL_PAD_SQUAT ) && CheckCeil( work ) ) ||
		( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) 
#else
	if ( ( press & PL_PAD_SQUAT ) && CheckCeil( work ) ) 
#endif
	{
		/* 立ち移行 */
		StopTurn( work ) ;
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundToStand ) ;
		SetArmAction( work, AMstand ) ;
		if ( work->action2 != ThrowGrenade ) {
		    SetMode2( work, NULL ) ;
		}
		return ;
	}
	if ( CheckIntrude( work ) ) {
		/* イントルード移行 */
		SetFlag( FLAG_NO_STEP ) ;
		IntoIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : [GroundStill] is intrude!! -> leave intrude\n" ) ;
		LeaveIntrude( work ) ;
	}
		
	turn = work->control.turn.vy ;

	/* 死体の上では匍匐できないモード */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ON_CORPSE_STAND ) &&
		 Status( PLAYER_ON_CORPSE ) ) {
		if ( CheckCeil( work ) ) {	
			StopTurn( work ) ;
			SetFlag( FLAG_NO_STEP ) ;
			SetMode( work, GroundToStand ) ;
			SetArmAction( work, AMstand ) ;
			SetMode2( work, NULL ) ;
			return ;
		} 
	}
		
	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	} else {
		if ( work->action2 != NULL ) {
			/* 構え中など */
			SeekTurn( work ) ;
		} else {
			/* 前進後退回転 */
			if ( work->pad->status & PAD_UDLR ) {
				diff = GV_DiffDirAbs( work->control.rot.vy, PadTo ) ;
				if ( diff <= 512 + 32 ) {
					SetMode( work, GroundMoveFront ) ;
					return ;
				} else if ( diff >= 1536 - 32 ) {
					SetMode( work, GroundMoveBack ) ;
					return ;
				} else if ( !PL_SubjectMove ){ /* 主観移動以外のとき */
					diff = GV_DiffDirS( work->control.turn.vy, PadTo ) ;
					work->control.turn.vy += ( diff > 0 ) ? 32 : -32 ;
				}
			}
		}
	}

	if ( PL_SubjectMove )
	{
	    CheckAttack( work ) ;
	    return ;
	}

	/* めりこみチェック */
	rot = work->control.turn ;
	if ( time == 0 ) {
		AvoidSink( work, NEAR_HEIGHT_GROUND, 0.0F ) ;
		c = n = 0 ;
	} else {
		n = CheckGroundWidth( work, &rot ) ; 
		c = 1 ;
		work->idata = work->control.rot.vy ;		
	} 
	if ( n ) {
		if ( work->idata != -1 ) {
			work->control.turn.vy = work->control.rot.vy = work->idata ;
		} else {
			work->control.turn.vy = work->control.rot.vy = work->pre_turn.vy ;
		}
		if ( ( ( n & 3 ) != 3 ) && !( n & 4 ) && !( n & 0xf0 ) ) {
		    /* 前か後ろがつかえている */
		    if ( !Status( PLAYER_WATCH ) && work->action2 == NULL ) {
			if ( n & 1 ) {
			    /* 前がつかえている */
			    /* しゃがみに移行 */
			    SetFlag( FLAG_NO_MOTION_STEP ) ;
			    _sceVu0AddVector( &work->control.step,
					      &work->control.step, 
					      &FrontAdj ) ;
			    if ( CheckCeil( work ) ) {
				SetFlag( FLAG_NO_STEP ) ;
				SetMode( work, GroundToSquatFront ) ;
			    }
			    return ;
			} else {
			    /* 後ろがつかえている */
			    /* 自動で前進 */
			    work->data = 16 ;
			    work->data2 = 0 ;
			    SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;
			}
		    }
		} else if ( !Status( PLAYER_WATCH ) && ( n & 0xf0 ) ) {
			//int		dir ;
			work->control.turn.vy = work->pre_turn.vy ;
#if 0
			/* 無理矢理向けそうな方を向いてしまう。 */
			dir = CheckGroundEnableDir( work ) ;
			if ( dir >= 0 && GV_DiffDirAbs( work->control.rot.vy, dir ) < 512 ) {
				//printf( "raiden/normal.c : gururunn still %d\n", dir ) ;
				SetFlag( FLAG_NO_STEP ) ;			
				work->control.turn.vy = dir ;
			} else {
				goto to_squat_front ;
			}
#endif
		} else {
			/* 前も後ろもつかえている */
			if ( !Status( PLAYER_WATCH ) &&
				work->action2 == NULL &&
				++ work->data2 >= 16 &&
				CheckCeil( work ) ) {
				/* しゃがみ移行 */
				StopTurn( work ) ;
				SetFlag( FLAG_NO_STEP ) ;
				if ( !( n & 8 ) ) {
					SetMode( work, GroundToSquatFront ) ;
					SetArmAction( work, AMstand ) ;
					SetMode2( work, NULL ) ;
					return ;
				}
			}
		}
	} else {
		work->data2 = 0 ;
	}
	if ( work->data > 0 && -- work->data == 0 ) {
		SetAction( work, MS.change[ Mcrouch ], 6 ) ;
	}
	/* グレネード系不可きめうち */
//	if ( !( PL_WeaponType( work ) & WP_TYPE_MODE1FUNC ) ) {
		CheckAttack( work ) ;
//	}
}

/* 匍匐前進 */
static	void	GroundMoveFront( Work *work, int time )
{
	int		status, diff, n, max ;


	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;
	
	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;
		work->act_name = GROUND_MOVE_FRONT ;
	}

	/* 早匍匐 */
	max = PadForce ;
	if ( max > 0 ) {
		if ( max < PAD_RUN_TH ) {
			SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;		
		} else {
			SetAction( work, MS.change[ Mcrouch_f_fast ], 6 ) ;
		}
	}

	GroundIK2( work, 500.0F ) ;
	
	if ( CheckIntrude( work ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		IntoIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : ground front : is intrude!! -> leave intrude\n" ) ;
		LeaveIntrude( work ) ;
	}

#ifdef STEP_NO_GROUND
	if ( ( ( work->pad->press & PL_PAD_SQUAT ) && CheckCeil( work ) ) ||
		( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) 
#else
	if ( ( work->pad->press & PL_PAD_SQUAT ) && CheckCeil( work ) ) 
#endif
	{
		StopTurn( work ) ;
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundToStand ) ;
		return ;
	}
		
	//    SetFlag( FLAG_CAMERA_ON_WAIST ) ;
	if ( PL_SubjectMove ) {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &work->camera ) ;
	} else {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
	}
		
	status = work->pad->status ;
#if 0
	if ( !( status & PAD_UDLR ) || Status( PLAYER_WATCH ) ||
		Status( PLAYER_ON_CORPSE ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
		return ;
	}
#else
	if ( !( status & PAD_UDLR ) || Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
		return ;
	}
	/* 死体の上では匍匐できないモード */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ON_CORPSE_STAND ) && Status( PLAYER_ON_CORPSE ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
		return ;		
	}

#endif		
	diff = GV_DiffDirAbs( work->control.rot.vy, PadTo ) ;
	if ( diff <= 768 + 32 ) {
	    if ( !PL_SubjectMove ){ /* 主観移動以外のとき */
		if ( work->control.interp == 0 ) {
			diff = GV_DiffDirS( work->control.turn.vy, PadTo ) ;
			work->control.turn.vy += ( diff > 0 ) ? 32 : -32 ;
		}
	    }
	} else if ( diff >= 2046 - 768 - 32 ) {
		SetMode( work, GroundMoveBack ) ;
	} else {
		SetMode( work, GroundStill ) ;
	}    

	/* 主観移動のとき体のメリコミチェックはなし */
	if ( PL_SubjectMove )
	{
	    CheckAttack( work ) ;
	    if ( work->action2 != NULL ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
	    }
	    return ;
	}

	n = CheckGroundWidth( work, &work->control.turn ) ;
	if ( n & 3 ) {
		work->control.turn.vy = work->pre_turn.vy ;
		n = CheckGroundWidth( work, &work->control.turn ) ;
		if ( n & 4 ) {
			StopTurn( work ) ;
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			_sceVu0SubVector( &work->control.step, &work->pre_mov, &work->control.mov ) ;
		} else if ( n & 3 ) {
			StopTurn( work ) ;
			if ( ( n & 3 ) == 3 ) printf( "raiden/normal.c : ouch\n" ) ;
			if ( n & 1 ) {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				_sceVu0AddVector( &work->control.step, &work->control.step, 
								 &FrontAdj ) ;
			} else {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				_sceVu0AddVector( &work->control.step, &work->control.step, 
								 &BackAdj ) ;
			}
		} 
	} else if ( work->control.interp == 0 && ( n & 0xf0 ) ) {
		/* 無理矢理いけそうな方に向いてしまう */
		work->control.turn.vy = work->pre_turn.vy ;
		if ( n & 0x10 ) SetFlag( FLAG_NO_STEP ) ;
		StopTurn( work ) ;
	} 
	if ( ( n & 1 ) && PL_SubjectMove==0 ) {
		work->data2 = 4 ;
		if ( ++ work->data >= 4 && CheckCeil( work ) ) {
			SetFlag( FLAG_NO_STEP ) ;			
			SetMode( work, GroundToSquatFront ) ;
		}
	} else if ( work->data2 > 0 ) {
		if ( -- work->data2 == 0 ) work->data = 0 ;
	}
}

/* 匍匐後退 */
static	void	GroundMoveBack( work, time )
Work		*work ;
int		time ;
{
	int		status, diff, n ;


	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcrouch_b ], 6 ) ;
		work->act_name = GROUND_MOVE_BACK ;
	}

	GroundIK2( work, 500.0F ) ;

	if ( CheckIntrude( work ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		IntoIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : ground back : is intrude!! -> leave intrude\n" ) ;
		LeaveIntrude( work ) ;
	}

#ifdef STEP_NO_GROUND
	if ( ( ( work->pad->press & PL_PAD_SQUAT ) && CheckCeil( work ) ) ||
		( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) 
#else
	if ( ( work->pad->press & PL_PAD_SQUAT ) && CheckCeil( work ) ) 
#endif
	{
		StopTurn( work ) ;
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundToStand ) ;
		return ;
	}

	if ( PL_SubjectMove ) {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &work->camera ) ;
	} else {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
	}

	status = work->pad->status ;
	if ( !( status & PAD_UDLR ) || Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
		return ;
	}
	/* 死体の上では匍匐できないモード */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_ON_CORPSE_STAND ) && Status( PLAYER_ON_CORPSE ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
		return ;		
	}


	diff = GV_DiffDirAbs( work->control.rot.vy, PadTo ) ;
	if ( diff <= 768 + 32 ) {
		SetMode( work, GroundMoveFront ) ;
	} else if ( diff >= 2048 - 768 - 32 ) {
	    if ( !PL_SubjectMove ){ /* 主観移動以外のとき */
		if ( work->control.interp == 0 ) {
			diff = GV_DiffDirS( work->control.turn.vy, PadTo ) ;
			work->control.turn.vy += ( diff > 0 ) ? -32 : 32 ;
		}
	    }
	} else {
		SetMode( work, GroundStill ) ;
	}    

	/* 主観移動のとき体のメリコミチェックはなし */
	if ( PL_SubjectMove )
	{
	    if ( work->action2 != NULL ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, GroundStill ) ;
	    }
	    CheckAttack( work ) ;
	    return ;
	}

	n = CheckGroundWidth( work, &work->control.turn ) ;
	if ( n & 3 ) {
		work->control.turn.vy = work->pre_turn.vy ;
		n = CheckGroundWidth( work, &work->control.turn ) ;
		if ( n & 4 ) {
			SetFlag( FLAG_NO_STEP ) ;
			GM_ResetControlPosition( &work->control, &work->pre_mov ) ;
		} if ( n & 3 ) {
			StopTurn( work ) ;
			if ( ( n & 3 ) == 3 ) printf( "raiden/normal.c : ouch\n" ) ;
			if ( n & 1 ) {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				_sceVu0AddVector( &work->control.step, &work->control.step, 
								 &FrontAdj ) ;
			} else {
				SetFlag( FLAG_NO_MOTION_STEP ) ;
				_sceVu0AddVector( &work->control.step, &work->control.step, 
								 &BackAdj ) ;
			}
		}

	} else if ( work->control.interp == 0 && ( n & 0xf0 ) ) {
		/* 無理矢理いけそうな方に向いてしまう */
		work->control.turn.vy = work->pre_turn.vy ;
		if ( n & 0x20 ) SetFlag( FLAG_NO_STEP ) ;
		StopTurn( work ) ;
	} 

	if ( PL_SubjectMove==0 && (n & 2) ) {	
		work->data2 = 4 ;
		if ( ++ work->data >= 4 && CheckCeil( work ) ) {
			SetFlag( FLAG_NO_STEP ) ;			
			SetMode( work, GroundToSquatBack ) ;
		}
	} else if ( work->data2 > 0 ) {
		if ( -- work->data2 == 0 ) work->data = 0 ;
	}

	GM_ReverseSlideCameraDir( work->chanl ) ;
}

/* 匍匐から立ち */
static	void	GroundToStand( Work *work, int time )
{
	int		flr_flag, act ;
	FVECTOR	head ;
	float	levels[ 2 ] ;

	if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
		act = MS.change[ Mcrouch_stand ] ;
		/* 頭上に低い天井がある場合はモーションが違う */
		PL_ObjPos( work, HUMAN21_ATAMA, &head ) ;
		head.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
		flr_flag = HZX_LevelHazardCheck( work->control.hzx_id, &head, HZX_CHK_ALL, 
										 HZX_FLOOR_NO_PLAYER ) ;
		if ( flr_flag & 2 ) {
			HZX_GetLevelHeight( levels ) ;
			if ( levels[ 1 ] - head.vy < 500.0F ) {
				/* ちょっと変なことをしている */
				work->control.r_sphere = 8.0F ;
				act = MS.change[ Mcrouch_stand_int ] ;
			}
		}
		SetAction( work, act, 6 ) ;
	}
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK ) ;
	/* 主観変更禁止 */
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;

	/* 匍匐ＩＫ */
#if 0
	{
		FVECTOR	max, min ;

		GetModelHeight2( work, &max, &min ) ;
		if ( max.vz - min.vz > 1200.0F ) {
			GroundIK2( work ) ;
		} else {
			ResetGroundAdjust( work ) ;
		}
	}
#endif
	GroundIK2( work, 300.0F ) ;

	if ( PL_SubjectMove ) {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_KUBI ].world, &work->camera ) ;
	} else {
		GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;
	}

	if ( EndMotion( work ) ) {
		SetMode( work, StandStill ) ;
	}
}

/* 匍匐からしゃがみ（前） */
static	void	GroundToSquatFront( work, time )
Work			*work ;
int				time ;
{
	if ( time == 0 ) {
		ChangeStance( work, SQUAT ) ;
		work->hzx_height = HzxHeight[ STAND ] ; /* 当たりチェック立ち扱い */
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
		SetAction( work, MS.change[ Mcr2sq_f ], 6 ) ;
		if ( !Status( PLAYER_SNAKE ) ) work->fdata = 120.0F ;
		else						   work->fdata = 0.0F ;
	}
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	/* しゃがみ移行後すぐ張りつけるように調整 */
	if ( MotionTime( work ) > 36 && work->fdata > 0.0F ) {
		FVECTOR		adj ;
		float		f ;
		
		f = GV_NearExp4F( 0.0F, work->fdata ) ;
		work->fdata -= f ;
		GV_SetVec3( &adj, 0.0F, 0.0F, f ) ;
		DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
		DG_RotVector( &adj, &adj, 1 ) ;
		work->control.step.vx += adj.vx ;
		work->control.step.vz += adj.vz ;
	}

	/* 匍匐ＩＫ */
	GroundIK2( work, 300.0F ) ;
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, SquatStill ) ;
		SetStatus( PLAYER_NEED_NEW_PRESS ) ;
	}	
}

/* 匍匐からしゃがみ（後ろ） */
static	void	GroundToSquatBack( Work *work, int time )
{
	if ( time == 0 ) {
		ChangeStance( work, SQUAT ) ;
		work->hzx_height = HzxHeight[ STAND ] ; /* 当たりチェック立ち扱い */
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
		work->control.hzx_height = work->hzx_height ;
		SetAction( work, MS.change[ Mcr2sq_b ], 6 ) ;
	}
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	/* 匍匐ＩＫ */
	GroundIK2( work, 300.0F ) ;
	GV_MatToVec( &work->body.objs->objs[ HUMAN21_ATAMA ].world, &work->camera ) ;

	if ( EndMotion( work ) ) {
		SetMode( work, SquatStill ) ;
		SetStatus( PLAYER_NEED_NEW_PRESS ) ;
	}		
}

/* 立ちはりつき静止（グローバル） */
void		PL_StandCautionStill( Work *work, int time )
{
	int		wallto ;

	SetStatus( PLAYER_EVENT_ENABLE ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		IntoCaution( work ) ;
		SetAction( work, MS.change[ Mcaution ], 6 ) ;
		GM_PlayerCautionDir = work->control.turn.vy ;
		work->act_name = STAND_CAUTION_STILL ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		if ( wallto ) {
			CautionStep( work ) ;
			SetFlag( FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT ) ;
			SubjectTurn( work ) ;
		} else if ( work->caution_dir != ( work->pad->status & PAD_UDLR ) ) {
			/* 十字キー入力が変化したら張り付き解除 */
			LeaveCaution( work ) ;
			SetMode( work, StandStill ) ;
			return ;
		}
		if ( !wallto && 
			( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
			SetMode( work, SquatCautionStill ) ;
		}
	} else {
		/* 張り付き壁変更 */
		ChangeCautionWall( work ) ;
		/* 張り付きステップ計算 */
		CautionStep( work ) ;
		/* 覗き込み */
		/* はりつき壁が１ｍ以下だったら飛び出し撃ちは可能 */
		if ( !wallto && 
			( Status( PLAYER_BEHIND ) || CheckCautionWallHeightIsLowerThan1M( work ) ) ) {
			if ( Status( PLAYER_BEHIND ) && CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, StandCautionPeepL ) ;
				return ;
			} else if ( Status( PLAYER_BEHIND ) && CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, StandCautionPeepR ) ;
				return ;
			} else if ( ( GV_DiffDirAbs( work->control.rot.vy, WallTo + 2048 ) < 64 ) &&
					    work->pad->press & PL_PAD_WEAPON &&
					    work->action2 == NULL &&
//					   MS.change[ Mbehind_at_l_s ] != NO_ACT ) {
				       ( WeaponType( work ) & WP_TYPE_BH_JUMPOUT ) ) {
				float	rlen, llen ;
				int	rseg, lseg ;
				FVECTOR	rdiff, ldiff ;
				/* 攻撃 */
				/* まずビハインド端までの距離を求める */
				if ( Status( PLAYER_BEHIND ) ) {
					rlen = BehindEdgeLen( work, BEHIND_RIGHT, &rdiff ) ;
					llen = BehindEdgeLen( work, BEHIND_LEFT, &ldiff ) ;
				} else {
					rlen = llen = BEHIND_ATTACK_ENABLE_LEN - 1.0F ;
					DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
					rdiff.vx = 1.0F ; rdiff.vy = rdiff.vz = 0.0F ;
					ldiff.vx = -1.0F ; ldiff.vy = ldiff.vz = 0.0F ;
					DG_RotVector( &rdiff, &rdiff, 1 ) ;		    
					DG_RotVector( &ldiff, &ldiff, 1 ) ;		    
				}
				if ( rlen < BEHIND_ATTACK_ENABLE_LEN ||
					llen < BEHIND_ATTACK_ENABLE_LEN ) {
					/* ビハインド端にいる */
					rseg = CheckBehindAttackEnable( work, BEHIND_RIGHT ) ;
					lseg = CheckBehindAttackEnable( work, BEHIND_LEFT ) ;
					rseg |= CheckBehindPeepSegment3( work, BEHIND_RIGHT, rlen ) ;
					lseg |= CheckBehindPeepSegment3( work, BEHIND_LEFT, llen ) ;
					if ( BehindPtr != NULL ) {
						rseg |= BehindPtr->flag & HZX_BEHIND_NO_RIGHT_OUT ;
						lseg |= BehindPtr->flag & HZX_BEHIND_NO_LEFT_OUT ;
					}
					if ( rseg == 0 && lseg == 0 ) {
						/* 両方飛び出せる時は近い方 */
						if ( rlen < llen ) {
							SetBehindAttackPosition( work, &rdiff, rlen, BEHIND_RIGHT ) ;
							InitBehindPeepCamera( work ) ;
							SetMode( work, BehindAttackR ) ;
						} else {
							SetBehindAttackPosition( work, &ldiff, llen, BEHIND_LEFT ) ;
							InitBehindPeepCamera( work ) ;
							SetMode( work, BehindAttackL ) ;
						}
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					} else if ( rseg == 0 && rlen < BEHIND_ATTACK_ENABLE_LEN ) {
						/* 右飛び出し */
						SetBehindAttackPosition( work, &rdiff, rlen, BEHIND_RIGHT ) ;
						InitBehindPeepCamera( work ) ;
						SetMode( work, BehindAttackR ) ;			
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					} else if ( lseg == 0 && llen < BEHIND_ATTACK_ENABLE_LEN ) {
						/* 左飛び出し */
						SetBehindAttackPosition( work, &ldiff, llen, BEHIND_LEFT ) ;
						InitBehindPeepCamera( work ) ;	
						SetMode( work, BehindAttackL ) ;
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					}		    
					/* どちらにも飛び出せない */
					goto behind_attack_check_again_stand ;
				} else {
					behind_attack_check_again_stand :

					if ( BehindPtr != NULL ) {
						rseg |= BehindPtr->flag & HZX_BEHIND_NO_RIGHT_OUT ;
						lseg |= BehindPtr->flag & HZX_BEHIND_NO_LEFT_OUT ;
					}
					/* その場で飛び出し先に壁があるかチェック */
					if ( !CheckBehindPeepSegment( work, BEHIND_RIGHT ) &&
						!CheckBehindAttackEnable( work, BEHIND_RIGHT ) ) {
						/* その場で右飛び出し可 */
						if ( BehindPtr == NULL ||
							 ( BehindPtr != NULL && 
							   !( BehindPtr->flag & HZX_BEHIND_NO_RIGHT_OUT ) ) ) {
							SetBehindAttackPosition( work, &rdiff, 0.0F, BEHIND_RIGHT ) ;
							InitBehindPeepCamera( work ) ;
							SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
							SetMode( work, BehindAttackR ) ;		    
							return ;
						}
					} else if ( !CheckBehindPeepSegment( work, BEHIND_LEFT ) &&
							   !CheckBehindAttackEnable( work, BEHIND_LEFT ) ) {
						/* その場で左飛び出し可 */
						if ( BehindPtr == NULL ||
							( BehindPtr != NULL && 
							 !( BehindPtr->flag & HZX_BEHIND_NO_LEFT_OUT ) ) ) {
							SetBehindAttackPosition( work, &ldiff, 0.0F, BEHIND_LEFT ) ;
							InitBehindPeepCamera( work ) ;
							SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
							SetMode( work, BehindAttackL ) ;
							return ;
						}
					}		    
				}
			}
		}

		switch ( Liable ) {
		case NEUTRAL :
			/* はりつき終わり */
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;
					SetMode( work, StandStill ) ;
				}
			} 
			break ;
		case FRONT :
			if ( !wallto && 
				( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
				SetMode2( work, NULL ) ;
				SetMode( work, SquatCautionStill ) ;
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
			break ;
		case LEFT :
			if ( !wallto && 
				( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
				SetMode( work, SquatCautionStill ) ;
			} else if ( CheckBehindPeepSegment4( work, BEHIND_RIGHT, 
												 BEHIND_PEEP_START_LEN, 650.0F ) ) { 
				/* はりつき左移動 */
				SetMode( work, StandCautionLeft ) ;
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
			break ;
		case RIGHT :
			if ( !wallto && 
				( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
				SetMode( work, SquatCautionStill ) ;
			} else if ( CheckBehindPeepSegment4( work, BEHIND_LEFT, 
												 BEHIND_PEEP_START_LEN, 650.0F ) ) { 
				/* はりつき右移動 */
				SetMode( work, StandCautionRight ) ;
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
		}
	}

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
#if 0
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		CheckC4BombWall( work ) ;
	}
#endif
	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* しゃがみ張り付き静止 */
static	void	SquatCautionStill( work, time )
Work		*work ;
int		time ;
{
	int		wallto ;

	SetStatus( PLAYER_EVENT_ENABLE ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;
	SetFlag( FLAG_NO_IK ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		ChangeStance( work, SQUAT ) ;
		IntoCaution( work ) ;
		SetAction( work, MS.change[ Mcaution_squat ], 6 ) ;
		GM_PlayerCautionDir = work->control.turn.vy ;
		/* 通常しゃがみから来たとき用 */
		work->hzx_height = HzxHeight[ SQUAT ] ;
		work->act_name = SQUAT_CAUTION_STILL ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		if ( wallto ) {
			CautionStep( work ) ;
			SetFlag( FLAG_CANNOT_PEEP | FLAG_SUBJECT_HORIZON_LIMIT ) ;
			SubjectTurn( work ) ;
		} else if ( work->caution_dir != ( work->pad->status & PAD_UDLR ) ) {
			/* 十字キー入力が変化したら張り付き解除 */
			LeaveCaution( work ) ;
			SetMode( work, SquatStill ) ;
			return ;
		} 
		if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
			if ( !SquatOnly( work ) ) SetMode( work, StandCautionStill ) ;
		}
	} else {
		/* 張り付き壁変更 */
		ChangeCautionWall( work ) ;
		/* 張り付きステップ計算 */
		CautionStep( work ) ;
		/* 覗き込み */
		if ( !wallto && Status( PLAYER_BEHIND ) ) {
			if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, SquatCautionPeepL ) ;
				return ;
			} else if ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, SquatCautionPeepR ) ;
				return ;
			} else if ( ( GV_DiffDirAbs( work->control.rot.vy, WallTo + 2048 ) < 64 ) &&
					    work->pad->press & PL_PAD_WEAPON &&
					    work->action2 == NULL &&
//					   MS.change[ Mbehind_at_l_s ] != NO_ACT ) {
				       ( WeaponType( work ) & WP_TYPE_BH_JUMPOUT ) ) {
				float	rlen, llen ;
				int	rseg, lseg ;
				FVECTOR	rdiff, ldiff ;
				/* 攻撃 */
				/* まずビハインド端までの距離を求める */
				rlen = BehindEdgeLen( work, BEHIND_RIGHT, &rdiff ) ;
				llen = BehindEdgeLen( work, BEHIND_LEFT, &ldiff ) ;
				if ( rlen < BEHIND_ATTACK_ENABLE_LEN ||
					llen < BEHIND_ATTACK_ENABLE_LEN ) {
					/* ビハインド端にいる */
					rseg = CheckBehindAttackEnable( work, BEHIND_RIGHT ) ;
					lseg = CheckBehindAttackEnable( work, BEHIND_LEFT ) ;
					rseg |= CheckBehindPeepSegment3( work, BEHIND_RIGHT, rlen ) ;
					lseg |= CheckBehindPeepSegment3( work, BEHIND_LEFT, llen ) ;
					if ( BehindPtr != NULL ) {
						rseg |= BehindPtr->flag & HZX_BEHIND_NO_RIGHT_OUT ;
						lseg |= BehindPtr->flag & HZX_BEHIND_NO_LEFT_OUT ;
					}
					if ( rseg == 0 && lseg == 0 ) {
						/* 両方飛び出せる時は近い方 */
						if ( rlen < llen ) {
							SetBehindAttackPosition( work, &rdiff, rlen, BEHIND_RIGHT ) ;
							InitBehindPeepCamera( work ) ;
							SetMode( work, SquatBehindAttackR ) ;
						} else {
							SetBehindAttackPosition( work, &ldiff, llen, BEHIND_LEFT ) ;
							InitBehindPeepCamera( work ) ;
							SetMode( work, SquatBehindAttackL ) ;
						}
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					} else if ( rseg == 0 && rlen < BEHIND_ATTACK_ENABLE_LEN ) {
						/* 右飛び出し */
						SetBehindAttackPosition( work, &rdiff, rlen, BEHIND_RIGHT ) ;
						InitBehindPeepCamera( work ) ;
						SetMode( work, SquatBehindAttackR ) ;			
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					} else if ( lseg == 0 && llen < BEHIND_ATTACK_ENABLE_LEN ) {
						/* 左飛び出し */
						SetBehindAttackPosition( work, &ldiff, llen, BEHIND_LEFT ) ;
						InitBehindPeepCamera( work ) ;
						SetMode( work, SquatBehindAttackL ) ;
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					}		    
					/* どちらにも飛び出せない */
					goto behind_attack_check_again_squat ;
				} else {
					behind_attack_check_again_squat :
					/* ビハインド端でない */
					/* その場で飛び出し先に壁があるかチェック */
					if ( BehindPtr != NULL ) {
						rseg = BehindPtr->flag & HZX_BEHIND_NO_RIGHT_OUT ;
						lseg = BehindPtr->flag & HZX_BEHIND_NO_LEFT_OUT ;
					} else {
						rseg = lseg = 0 ;
					}
					if ( rseg == 0 && 
						!CheckBehindPeepSegment( work, BEHIND_RIGHT ) &&
						!CheckBehindAttackEnable( work, BEHIND_RIGHT ) ) {
						/* その場で右飛び出し可 */
						SetBehindAttackPosition( work, &rdiff, 0.0F, BEHIND_RIGHT ) ;
						InitBehindPeepCamera( work ) ;
						SetMode( work, SquatBehindAttackR ) ;		    
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					} else if ( lseg == 0 &&
							    !CheckBehindPeepSegment( work, BEHIND_LEFT ) &&
							   !CheckBehindAttackEnable( work, BEHIND_LEFT ) ) {
						/* その場で左飛び出し可 */
						SetBehindAttackPosition( work, &ldiff, 0.0F, BEHIND_LEFT ) ;
						InitBehindPeepCamera( work ) ;
						SetMode( work, SquatBehindAttackL ) ;		    
						SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE ) ;
						return ;
					}		    
				}
			}
		}	

		switch ( Liable ) {
		case NEUTRAL :
			/* はりつき終わり */
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;
					SetMode( work, SquatStill ) ;
				}
			} 
			break ;
		case FRONT :
			if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
				if ( !SquatOnly( work ) ) {
					SetMode2( work, NULL ) ;
					SetMode( work, StandCautionStill ) ;
				}
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
			break ;
		case LEFT :
			if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
				if ( !SquatOnly( work ) ) {
					SetMode( work, StandCautionStill ) ;
				}
			} else if ( CheckBehindPeepSegment4( work, BEHIND_RIGHT, 
												 BEHIND_PEEP_START_LEN, 200.0F ) ) { 
				/* はりつき左移動 */
				SetMode( work, SquatCautionLeft ) ;
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
			break ;
		case RIGHT :
			if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
				if ( !SquatOnly( work ) ) {
					SetMode( work, StandCautionStill ) ;
				}
			} else if ( CheckBehindPeepSegment4( work, BEHIND_LEFT, 
												 BEHIND_PEEP_START_LEN, 200.0F ) ) { 
				/* はりつき右移動 */
				SetMode( work, SquatCautionRight ) ;
			} else {
				/* 壁こんこん */
				if ( !wallto ) CheckKnockWall( work ) ;
			}
		}
	}

	{
		int	res ;
			
		res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 500.0F ) ;
		if ( res & 1 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			GV_NearExp4VF( &work->control.step, &LeftAdj, 3 ) ;
		} else if ( res & 2 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			GV_NearExp4VF( &work->control.step, &RightAdj, 3 ) ;
		}
	}

	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* はりつき左移動 */
static	void	StandCautionLeft( work, time ) 
Work		*work ;
int		time ;
{
	int		wallto ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcaution_l ], 6 ) ;
		work->act_name = STAND_CAUTION_LEFT ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	if ( !wallto && 
		( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
		SetMode2( work, NULL ) ;
		SetMode( work, SquatCautionStill ) ;
		return ;
	}

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	/* 張り付き壁変更 */
	ChangeCautionWall( work ) ;
	/* 張り付きステップ計算 */
	CautionStep( work ) ;
	GM_PlayerCautionDir = ( WallTo - 1024 ) & 4095 ;

	if ( Status( PLAYER_WATCH ) ) {
		work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
		SetMode( work, StandCautionStill ) ;
	} else {
		/* 覗き込み */
		if ( !wallto && Status( PLAYER_BEHIND ) ) {
			if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, StandCautionPeepL ) ;
				return ;
			} else if ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, StandCautionPeepR ) ;
				return ;
			}
		}
		/* ビハインド端で止まる */
		if ( !CheckBehindPeepSegment4( work, BEHIND_RIGHT, BEHIND_PEEP_START_LEN, 650.0F ) ) {
			SetMode( work, StandCautionStill ) ;
			return ;
		}
		switch ( Liable ) {
		case NEUTRAL :
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;	
					SetMode( work, StandStill ) ;
				} else {
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
					SetMode( work, StandCautionStill ) ;
				}
			}
			break ;
		case FRONT :
			work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
			SetMode( work, StandCautionStill ) ;
			break ;
		case RIGHT :
			SetMode( work, StandCautionRight ) ;
			break ;
		default :
			/* 壁こんこん */
			if ( !wallto ) CheckKnockWall( work ) ;
		}
	}

	{
		int	res ;

		res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 750.0F ) ;
		if ( res & 1 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			_sceVu0AddVector( &work->control.step, &work->control.step, &LeftAdj ) ;
			AdjustXZ( work, &LeftAdj ) ;
		}
	}

	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* はりつき右移動 */
static	void	StandCautionRight( work, time ) 
Work		*work ;
int		time ;
{
	int			wallto ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcaution_r ], 6 ) ;
		work->act_name = STAND_CAUTION_RIGHT ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	if ( !wallto && 
		( ( work->pad->press & PL_PAD_SQUAT ) || SquatOnly( work ) ) ) {
		SetMode2( work, NULL ) ;
		SetMode( work, SquatCautionStill ) ;
		return ;
	}

	/* 張り付き壁変更 */
	ChangeCautionWall( work ) ;
	/* 張り付きステップ計算 */
	CautionStep( work ) ;
	GM_PlayerCautionDir = ( WallTo + 1024 ) & 4095 ;

	if ( Status( PLAYER_WATCH ) ) {
		work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
		SetMode( work, StandCautionStill ) ;
	} else {
		/* 覗き込み */
		if ( !wallto && Status( PLAYER_BEHIND ) ) {
			if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, StandCautionPeepL ) ;
				return ;
			} else if ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, StandCautionPeepR ) ;
				return ;
			}
		}
		/* ビハインド端で止まる */
		if ( !CheckBehindPeepSegment4( work, BEHIND_LEFT, BEHIND_PEEP_START_LEN, 650.0F ) ) {
			SetMode( work, StandCautionStill ) ;
			return ;
		}
		switch ( Liable ) {
		case NEUTRAL :
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;
					SetMode( work, StandStill ) ;
				} else {
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
					SetMode( work, StandCautionStill ) ;
				}
			}
			break ;
		case FRONT :
			work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
			SetMode( work, StandCautionStill ) ;
			break ;
		case LEFT :
			SetMode( work, StandCautionLeft ) ;
			break ;
		default :
			/* 壁こんこん */
			if ( !wallto ) CheckKnockWall( work ) ;
		}
	}

	{
		int	res ;

		res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 750.0F ) ;
		if ( res & 2 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			_sceVu0AddVector( &work->control.step, &work->control.step, &RightAdj ) ;
			AdjustXZ( work, &RightAdj ) ;
		}
	}

	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* しゃがみ張り付き左移動 */
static	void	SquatCautionLeft( Work *work, int time )
{
	int		wallto ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcaution_l_sq ], 6 ) ;
		work->act_name = SQUAT_CAUTION_LEFT ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
		if ( !SquatOnly( work ) ) {
			SetMode2( work, NULL ) ;
			SetMode( work, StandCautionStill ) ;
			return ;
		}
	}

	/* 張り付き壁変更 */
	ChangeCautionWall( work ) ;
	/* 張り付きステップ計算 */
	CautionStep( work ) ;

	GM_PlayerCautionDir = ( WallTo - 1024 ) & 4095 ;

	if ( Status( PLAYER_WATCH ) ) {
		work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
		SetMode( work, SquatCautionStill ) ;
	} else {
		/* 覗き込み */
		if ( !wallto && Status( PLAYER_BEHIND ) ) {
			if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, SquatCautionPeepL ) ;
				return ;
			} else if ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, SquatCautionPeepR ) ;
				return ;
			}
		}
		/* ビハインド端で止まる */
		if ( !CheckBehindPeepSegment4( work, BEHIND_RIGHT, BEHIND_PEEP_START_LEN, 200.0F ) ) {
			SetMode( work, SquatCautionStill ) ;
			return ;
		}
		switch ( Liable ) {
		case NEUTRAL :
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;	
					SetMode( work, SquatStill ) ;
				} else {
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
					SetMode( work, SquatCautionStill ) ;
				}
			}
			break ;
		case FRONT :
			work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
			SetMode( work, SquatCautionStill ) ;
			break ;
		case RIGHT :
			SetMode( work, SquatCautionRight ) ;
			break ;
		default :
			/* 壁こんこん */
			if ( !wallto ) CheckKnockWall( work ) ;
		}
	}

	{
		int	res ;

		res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 750.0F ) ;
		if ( res & 1 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			_sceVu0AddVector( &work->control.step, &work->control.step, &LeftAdj ) ;
			AdjustXZ( work, &LeftAdj ) ;
		}
	}

	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* しゃがみ張り付き右移動 */
static	void	SquatCautionRight( Work *work, int time )
{
	int		wallto ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;
	wallto = CheckTrap( work, WALLTO_TRAP ) ;
	/* "wallto"内処理 */
	if ( wallto ) InsideWallToTrap( work ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcaution_r_sq ], 6 ) ;
		work->act_name = SQUAT_CAUTION_RIGHT ;
	}

	if ( !wallto ) CautionBackWallAdjust( work ) ;

	/* 死体がいたらはりつけない */
	if ( !wallto && Status( PLAYER_ON_CORPSE ) ) {
		LeaveCaution( work ) ;
		SetMode( work, StandStill ) ;
		return ;
	}

	if ( wallto || ( work->pad->press & PL_PAD_SQUAT ) ) {
		if ( !SquatOnly( work ) ) {
			SetMode2( work, NULL ) ;
			SetMode( work, StandCautionStill ) ;
			return ;
		}
	}

	/* 張り付き壁変更 */
	ChangeCautionWall( work ) ;
	/* 張り付きステップ計算 */
	CautionStep( work ) ;

	GM_PlayerCautionDir = ( WallTo + 1024 ) & 4095 ;

	if ( Status( PLAYER_WATCH ) ) {
		work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
		SetMode( work, SquatCautionStill ) ;
	} else {
		/* 覗き込み */
		if ( !wallto && Status( PLAYER_BEHIND ) ) {
			if ( CheckPeepButtonEnable( work, BEHIND_LEFT ) ) {
				SetMode( work, SquatCautionPeepL ) ;
				return ;
			} else if ( CheckPeepButtonEnable( work, BEHIND_RIGHT ) ) {
				SetMode( work, SquatCautionPeepR ) ;
				return ;
			}
		}
		/* ビハインド端で止まる */
		if ( !CheckBehindPeepSegment4( work, BEHIND_LEFT, BEHIND_PEEP_START_LEN, 200.0F ) ) {
			SetMode( work, SquatCautionStill ) ;
			return ;
		}
		switch ( Liable ) {
		case NEUTRAL :
			if ( !wallto && ++ work->data >= END_CAUTION_WAIT ) {
				if ( PL_PadEnable() ) {
					LeaveCaution( work ) ;	
					SetMode( work, SquatStill ) ;
				} else {
					work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
					SetMode( work, SquatCautionStill ) ;
				}
			}
			break ;
		case FRONT :
			work->caution_dir = ( work->pad->status & PAD_UDLR ) ;
			SetMode( work, SquatCautionStill ) ;
			break ;
		case LEFT :
			SetMode( work, SquatCautionLeft ) ;
			break ;
		default :
			/* 壁こんこん */
			if ( !wallto ) CheckKnockWall( work ) ;
		}
	}

	{
		int	res ;

		res = CheckLRWidth( work, &work->control.rot, CAUTION_SINK, CAUTION_SINK, 750.0F ) ;
		if ( res & 2 ) {
			SetFlag( FLAG_NO_MOTION_STEP ) ;
			_sceVu0AddVector( &work->control.step, &work->control.step, &RightAdj ) ;
			AdjustXZ( work, &RightAdj ) ;
		}
	}

	/* Ｃ４セット */
	if ( !wallto && ( work->weapon == WP_C4Bomb ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) {
		if ( GM_N_C4Bombs >= GM_C4_MAX ) GM_Buzzer() ;		
		else SetMode( work, SetC4BombWall ) ;
	}
}

/* イントルード出入り */
static	void	GroundFromToIntrude( work, time )
Work		*work ;
int		time ;
{
	int		status, res ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;

	GroundIK2( work, 500.0F ) ;
	res = CheckGroundWidth( work, &work->control.rot ) ;
	if ( ( res & 4 ) || ( ( res & 3 ) == 3 ) ) {
		printf( "raiden/normal.c : ouch\n" ) ;
	} else if ( res & 1 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &FrontAdj ) ;	
	} else if ( res & 2 ) {
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		_sceVu0AddVector( &work->control.step, &work->control.step, 
						 &BackAdj ) ;	
	}

	if ( work->ftime == ( 300 / TIME_BASE ) || work->motion1 == MS.change[ Mcrouch ] ) {
		UnsetInvincible( work ) ;
		SetFlag( FLAG_NO_STEP ) ;
		if ( !CheckIntrude( work ) ) {
			LeaveIntrude( work ) ;
			SetMode( work, GroundStill ) ;
		} else {
			IntoIntrude( work ) ;
			status = work->pad->status ;
			if ( status & PAD_U ) {
				SetMode( work, IntrudeMoveFront ) ;
			} else if ( status & PAD_D ) {
				SetMode( work, IntrudeMoveBack ) ;
			} else {
				SetMode( work, IntrudeStill ) ;
			}
		}
	}
	work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_MOVE ;
}

/* イントルード静止 */
static	void	IntrudeStill( Work *work, int time )
{
	int		status ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetStatus( PLAYER_EVENT_ENABLE ) ;
	SetFlag( FLAG_RECOVER_ENABLE ) ;
	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcrouch ], 6 ) ;
		work->act_name = INTRUDE_STILL ;
	}

	SetFlag( FLAG_CAMERA_ON_WAIST ) ;
	work->g_rot = GetGRot( work, 750.0F ) ;
	if ( work->action2 == NULL ) {
		work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_STILL ;
	} else {
		work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_MOVE ;
	}

	if ( !CheckIntrude( work ) ) {
		SetFlag( FLAG_NO_STEP | FLAG_CANNOT_CHANGE ) ;
		LeaveIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( !Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : (intrude still) not intrude!! -> set intrude\n" ) ;
		IntoIntrude( work ) ;
	}

	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	} else {
		status = work->pad->status ;
		if ( work->action2 == NULL ) {
			if ( status & PAD_U ) {
				SetMode( work, IntrudeMoveFront ) ;
			} else if ( status & PAD_D ) {
				SetMode( work, IntrudeMoveBack ) ;	    
			} else if ( !PL_SubjectMove ) {
				IntrudeTurn( work ) ;
			}
		} else if ( !PL_SubjectMove ) {
			IntrudeTurn( work ) ;
		}
		if ( !PL_SubjectMove ) {
		    work->camdir.vx = work->g_rot ;
		}
	}
	/* 攻撃 */
	if ( ( ( WeaponSet * )work->wp_set )->type & WP_TYPE_INTRUDE_OK ) {
		CheckAttack( work ) ;
	} else {
		if ( ( work->pad->press & PL_PAD_WEAPON ) &&
			 ( work->weapon == WP_C4Bomb ||
			   work->weapon == WP_Claymore ||
			   work->weapon == WP_Book ) ) {
			GM_Buzzer() ;
		}
	}
}

/* イントルード前進 */
static	void	IntrudeMoveFront( Work *work, int time )
{
	int		status, pre_grot ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;
	}

	pre_grot = work->g_rot ;
	work->g_rot = GetGRot( work, 750.0F ) ;

	if ( GV_DiffDirAbs( pre_grot, work->g_rot ) > 64 ) {
		GM_SeSetMode( SD_P_DCTDANSA, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	work->camdir.vx = work->g_rot ;
	if ( PadForce > 0 ) {
		if ( PadForce < PAD_RUN_TH ) {
			SetAction( work, MS.change[ Mcrouch_f ], 6 ) ;		
		} else {
			SetAction( work, MS.change[ Mcrouch_f_fast ], 6 ) ;
		}
	}

	if ( !CheckIntrude( work ) ) {
		SetFlag( FLAG_NO_STEP | FLAG_CANNOT_CHANGE ) ;
		LeaveIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( !Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : (intrude front) not intrude!! -> set intrude\n" ) ;
		IntoIntrude( work ) ;
	}

	status = work->pad->status ;
	if ( !( status & PAD_UDLR ) || Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, IntrudeStill ) ;
		return ;
	}

	if ( status & PAD_D ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, IntrudeMoveBack ) ;	    
	} else if ( !PL_SubjectMove ) { /* 主観移動以外のとき回転 */
		IntrudeTurn( work ) ;
	}
	work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_MOVE ;    
	/* 攻撃 */
	if ( ( ( WeaponSet * )work->wp_set )->type & WP_TYPE_INTRUDE_OK ) {
		CheckAttack( work ) ;
		if ( work->action2 != NULL ) SetMode( work, IntrudeStill ) ;
	} else {
		if ( ( work->pad->press & PL_PAD_WEAPON ) &&
			 ( work->weapon == WP_C4Bomb ||
			   work->weapon == WP_Claymore ||
			   work->weapon == WP_Book ) ) {
			GM_Buzzer() ;
		}
	} 
}	

/* イントルード後退 */
static	void	IntrudeMoveBack( Work *work, int time  )
{
	int		status, pre_grot ;

	if ( PL_SubjectMove ) { /*主観移動用のアクトに変える*/
		SubjectMoveTurn( work ) ;
	}

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_MOVE ) ;

	if ( time == 0 ) {
		SetAction( work, MS.change[ Mcrouch_b ], 6 ) ;
	}

	pre_grot = work->g_rot ;
	work->g_rot = GetGRot( work, 750.0F ) ;

	if ( GV_DiffDirAbs( pre_grot, work->g_rot ) > 64 ) {
		GM_SeSetMode( SD_P_DCTDANSA, &work->control.mov, GM_SEMODE_BOMB ) ;
	}

	work->camdir.vx = work->g_rot ;
	if ( !CheckIntrude( work ) ) {
		SetFlag( FLAG_NO_STEP | FLAG_CANNOT_CHANGE ) ;
		LeaveIntrude( work ) ;
		SetMode( work, GroundFromToIntrude ) ;
		SetInvincible( work, 0 ) ;
		return ;
	}

	if ( !Status( PLAYER_INTRUDE ) ) {
		printf( "raiden/normal.c : (intrude back) not intrude!! -> set intrude\n" ) ;
		IntoIntrude( work ) ;
	}

	status = work->pad->status ;
	if ( !( status & PAD_UDLR ) || Status( PLAYER_WATCH ) ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, IntrudeStill ) ;
		return ;
	}

	if ( status & PAD_U ) {
		SetFlag( FLAG_NO_STEP ) ;
		SetMode( work, IntrudeMoveFront ) ;	    
	} else if ( !PL_SubjectMove ) { /* 主観移動以外のとき回転 */
		IntrudeTurn( work ) ;
	}
	work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_MOVE ;    
	/* 攻撃 */
	if ( ( ( WeaponSet * )work->wp_set )->type & WP_TYPE_INTRUDE_OK ) {
		CheckAttack( work ) ;
		if ( work->action2 != NULL ) SetMode( work, IntrudeStill ) ;
	} else {
		if ( ( work->pad->press & PL_PAD_WEAPON ) &&
			 ( work->weapon == WP_C4Bomb ||
			   work->weapon == WP_Claymore ||
			   work->weapon == WP_Book ) ) {
			GM_Buzzer() ;
		}
	}    
}	

