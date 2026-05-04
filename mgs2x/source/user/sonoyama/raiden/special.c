/*
   special.c 
   ライデン／特殊モード
   
   1999/07/28 M.Sonoyama
   $Id: special.c,v 1.1.1.3 2002/11/19 11:51:02 Yoshizawa1 Exp $
*/

/*----------------------------------------------------------------*/

/* グローバル */

void		PL_Blow( PlayerWork *work, int time )
{
	Blow( work, time ) ;
}


/*----------------------------------------------------------------*/


/* 押し押され */
static	void	PushOrPushed( work, time )
Work			*work ;
int				time ;
{
	int			act, flag, motion ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON |
	     FLAG_CANNOT_CHANGE_ITEM   |
	     FLAG_DONOT_CHECK_WATCH    |
	     FLAG_NO_HEADTOENEMY       ) ;
	SetStatus( PLAYER_MENU_DISABLE ) ;
	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	/*主観移動の時*/
	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;


    if ( time == 0 ) {
		flag = work->post ;
		if ( (flag & MAIL_PUSH_WIN) || PL_SubjectMove ) {
			motion = work->motion1 ;
			if ( motion == MS.change[ Mwalk ] ||
				 motion == MS.change[ Mrun ] ||
				 motion == MS.change[ Mdash ] ||
				 motion == MS.change[ Mwalk_on ] ) {
				SetAction( work, motion, 6 ) ;
			} else if ( motion == MS.change[ Mstair_walk ] ||
					    motion == MS.change[ Mstair_run ] ) {
				SetAction( work, motion, 6 ) ;
			} else {
				SetAction( work, MS.change[ Mwalk ], 6 ) ;
			}
			if ( work->motion2 == MS.shared[ Mpush_walk_l_l ] ) {
				act = Mpush_walk_l_r ;
			} else {
				act = Mpush_walk_l_l ;
			}
			SetAction2( work, MS.shared[ act ], 6, 0x01ffe ) ;
			work->data = 0 ;
#if 0		/* 反転は廃止 */
			/* 反転指定。SetAction2のあとに行うこと */
			if ( work->post & MAIL_PUSH_RIGHT ) {
				work->body.m_ctrl->flag |= MT_FLAG_REVERSAL1 ;
			}
#endif
		} else {
			if ( flag & MAIL_PUSH_FRONT ) {
				act = Mpushed_lf ;
			} else {
				act = Mpushed_lb ;
			}
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {	
				if ( work->post & MAIL_PUSH_RIGHT ) {
					if ( act == Mpushed_lf ) act = Mdamage_caution ;
					else 					 act = Mout_caution ;
				} 
				SetAction( work, MS.shared[ act ], 6 ) ;
			} else {
				SetAction( work, MS.shared[ act ], 6 ) ;
				if ( work->post & MAIL_PUSH_RIGHT ) {
					work->body.m_ctrl->flag |= ( MT_FLAG_REVERSAL1 | MT_FLAG_REVERSAL2 ) ;
				}
			}
			work->data = 1 ;
		}
		work->post &= ~MAIL_PUSH_ALL ;
    }

	if ( work->data == 0 ) {
		int		max ;
		float	t ;

		t = work->body.m_ctrl->mt3_ctrl[ 0 ].play_time 
			/ work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
		max = PadForce ;
		if ( max != 0 ) {
		    /* 主観移動は押されても向きを変えない。 */
		    if ( PL_SubjectMove == 0 ) {
			if ( max > PAD_WALK_TH ) {
				work->control.turn.vy = ReactWall( PadTo ) ;
			} else {
				work->control.turn.vy = PadTo ;
			}
		    }
			if ( Status( PLAYER_ON_CORPSE ) ) {
				SetAction4( work, MS.change[ Mwalk_on ], t, 6 ) ;
			} else if ( work->floor_atr & HZX_FLOOR_STEP ) {
				if ( max < PAD_WALK_TH ) {
					SetAction4( work, MS.change[ Mstair_walk ], t, 6 ) ;
				} else {
					SetAction4( work, MS.change[ Mstair_run ], t, 6 ) ;
				}
			} else {
				if ( max < PAD_WALK_TH ) {
					SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
				} else if ( max < PAD_RUN_TH ) {
					//SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
					SetAction4( work, MS.change[ Mwalk ], t, 6 ) ;
				} else {
					//SetAction4( work, MS.change[ Mdash ], t, 6 ) ;
					SetAction4( work, MS.change[ Mrun ], t, 6 ) ;
				}
			}
			SetStatus( PLAYER_MOVE ) ;
		} else {
			SetAction( work, MS.change[ Mstand ], 6 ) ;			
		}
	}

	if ( work->data == 1 ) SetFlag( FLAG_NO_TOUCH_DAMAGE ) ;

    if ( ( work->data == 1 && EndMotion( work ) ) ||
		 ( work->data == 0 && EndMotion2( work ) ) ) {
		if ( work->data == 1 ) work->touch_invincible_time = 32 ;
		SetAction2( work, -1, 6, 0x01ffe ) ;
		SetMode( work, StillMode[ work->stance ] ) ;
		return ;
    }
	if ( work->weapon == WP_None ) {
		/* 素手攻撃可 */
		CheckAttack( work ) ;
		if ( work->action != PushOrPushed ) {
			SetAction2( work, -1, 6, 0x01ffe ) ;
		}
	}
}

/* のけぞりダメージ */
static	void	Shrink( work, time )
Work		*work ;
int		time ;
{
    int		act ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY ) ;
	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

    if ( time == 0 ) {
		work->motion1 = -1 ;	/* 連続攻撃も有るかも */
		if ( work->stance == GROUND ) {
			act = MS.shared[ Mdamage_crouch ] ;
		} else {
			if ( GV_Time & 1 ) act = MS.shared[ Mdamage_f ] ;
			else act = MS.shared[ Mdamage_b ] ; 
		}
		SetAction( work, act, 6 ) ;
		SetInvincible( work, 0 ) ;
    }

	if ( work->stance == GROUND ) {
		if ( PL_SubjectMove ) {
			GV_MatToVec( ObjWorld( work, HUMAN21_KUBI  ), &work->camera ) ;
		} else {
			GV_MatToVec( ObjWorld( work, HUMAN21_ATAMA ), &work->camera ) ;
		}
		GroundIK2( work, 500.0F ) ;
		SetFlag( FLAG_CAMERA_ON_WAIST ) ;
		work->camera.vy += 250.0F ;
	}

    if ( GM_CheckObject_IsEnd( &( work->body ), 0 ) ) {
		SetInvincible( work, INVINCIBLE_TIME ) ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		SetMode( work, StillMode[ work->stance ] ) ;
    }
}

/* 張り付きのけぞりダメージ */
static	void	CautionShrink( work, time )
Work			*work ;
int				time ;
{
    int		act ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY |
			FLAG_NO_MOTION_STEP_XZ ) ;
	SetStatus( PLAYER_NARROW | PLAYER_MENU_DISABLE ) ;
	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

    if ( time == 0 ) {
		/* CheckDamageで解除されるので再セット */
		SetStatus( PLAYER_CAUTION ) ;
		work->r_sphere = CAUTION_SPHERE ;
		if ( work->stance == SQUAT ) {
			act = MS.shared[ Mdamage_caution_sq ] ;
		} else {
			act = MS.shared[ Mdamage_caution ] ;			
		}
		SetAction( work, act, 6 ) ;
		SetInvincible( work, 0 ) ;
    }
    if ( EndMotion( work ) ) {
		SetInvincible( work, INVINCIBLE_TIME ) ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		if ( work->stance == SQUAT ) SetMode( work, SquatCautionStill ) ;
		else						 SetMode( work, StandCautionStill ) ;
    }
}

/* ダウンダメージ */
static	void	DownDamage( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			 FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY | FLAG_NO_IK ) ;
    if ( time == 0 ) {
		if ( work->down_dir == DOWN_FACE ) {
			SetAction( work, MS.shared[ Mdamage_down ], 6 ) ;
		} else {
			SetAction( work, MS.shared[ Mdamage_down_f ], 6 ) ;
		}
		SetInvincible( work, 0 ) ;
		/* ダウンのチェック位置は立ち */
		work->hzx_height = HzxHeight[ STAND ] ;
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
    }

	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

    GroundIK2( work, 500.0F ) ;

    if ( GM_CheckObject_IsEnd( &( work->body ), 0 ) ) {
		SetMode( work, Down ) ;
		SetInvincible( work, INVINCIBLE_TIME ) ;
		UnsetStatus( PLAYER_DAMAGED ) ;
    }    
}

/* ふっとび */
static	void	Blow( work, time )
Work		*work ;
int		time ;
{
    int		sink, dir ;
    SVECTOR	rot ;

    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			 FLAG_NO_TOUCH_DAMAGE |
			 FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY | FLAG_NO_IK ) ;

    if ( time == 0 ) {
		LeaveSubject( work ) ;
		SetInvincible( work, 0 ) ;
		UnsetStatus( PLAYER_SQUAT ) ; /* SE対策 */
		ChangeStance( work, STAND ) ;
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
		if ( work->data == 1 ) {	/* ソフト */
			SetAction( work, MS.shared[ Mblow_soft ], 6 ) ;
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
				/* はだかはうつぶせ */
				work->down_dir = DOWN_FACE ;	
			} else {
				work->down_dir = DOWN_BACK ;
			}
			/* スネークは反転 */
			//if ( Status( PLAYER_SNAKE ) ) work->control.turn.vy += 2048 ;
			work->data = 0 ;
		} else {
			if ( work->post & MAIL_SPS_DAMAGE_B ) {
				/* ショットガンうつ伏せ */
				SetAction( work, MS.shared[ Mdamage_b_sps ], 6 ) ;
				work->down_dir = DOWN_FACE ;
			} else if ( work->post & MAIL_SPS_DAMAGE_F ) {
				/* ショットガン仰向け */
				SetAction( work, MS.shared[ Mdamage_f_sps ], 6 ) ;
				work->down_dir = DOWN_BACK ;
			} else if ( work->post & MAIL_TUMBLE_F ) {
				/* 転びうつぶせ */
				SetAction( work, MS.shared[ Mdam_trip_f ], 6 ) ;
				work->down_dir = DOWN_FACE ;
			} else if ( work->post & MAIL_TUMBLE_B ) {
				/* 転び仰向け */
				SetAction( work, MS.shared[ Mdam_trip_b ], 6 ) ;
				work->down_dir = DOWN_BACK ;
			} else {
				SetAction( work, MS.shared[ Mdamage_near ], 6 ) ;
//				work->control.turn.vy += 2048 ;
				work->down_dir = DOWN_FACE ;
			}
			work->post &= ~( MAIL_SPS_DAMAGE_F | MAIL_SPS_DAMAGE_B |
							 MAIL_TUMBLE_F | MAIL_TUMBLE_B ) ;
		}
		/* 最初に方向を決めるため */
		if ( CheckGroundWidth( work, &work->control.rot ) & 7 ) {
			work->data2 = 17 ;
		}
		work->idata = 0 ;
    }

	/* 発見位置腰 */
	GM_PlayerFindPos.vy = work->control.mov.vy ;
	/* トラップチェック位置腰 */
	work->control.root_offset = NULL ;

	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

//    if ( work->body.height < 250.0F ) GroundIK2( work ) ;
	GroundIK2( work, 500.0F ) ;
	SetFlag2( FLAG2_ADJUST_SLOW ) ;

    switch( work->data ) {
    case 0 :
		/* めり込みチェック */
		rot.vx = 0 ;
		rot.vy = work->control.rot.vy ;
		rot.vz = 0 ;
		sink = AvoidSink( work, 200.0F, 0.0F ) ;
		if ( sink != 0 ) work->data2 ++ ;
		else if ( time != 0 ) work->data2 = 0 ;
		if ( work->data2 > 16 || ( sink & 4 ) || ( ( sink & 3 ) == 3 ) /*|| ( sink & 0xf0 )*/ ) {
			StopTurn( work ) ;
			work->data = 1 ;
			goto search_dir ;
//			if ( ( sink & 4 ) || ( ( sink & 3 ) == 3 ) ) work->data = 1 ;
//			else					 work->data = 3 ;
		}
		break ;
    case 1 :
search_dir :
	    /* 匍匐出来る向きが無ければここをループすることになる */
	    if ( work->idata == 0 || work->data2 <= 16 ) dir = CheckGroundEnableDir( work ) ;
		else dir = -1 ;
printf( "find dir %d : %d\n", dir, work->data2 ) ;
		if ( work->data2 > 16 ) work->idata = 1 ;
		work->data2 = 0 ;
		if ( dir != -1 ) {
			work->control.turn.vy = dir ;
			work->control.interp = 8 ;
			work->data = 2 ;
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		} else {
			SetFlag( FLAG_NO_STEP | FLAG_NO_MOTION_STEP ) ;
		}
		break ;
    case 2 :
		if ( work->control.interp == 0 ) work->data = 3 ;
		break ;
    case 3 :
		sink = AvoidSink( work, 200.0F, 0.0F ) ;
		if ( sink != 0 ) work->data2 ++ ;
		else			 work->data2 = 0 ;
		if ( work->data2 > 16 || ( sink & 4 ) || ( ( sink & 3 ) == 3 ) /*|| ( sink & 0xf0 )*/ ) {
			StopTurn( work ) ;
			printf( "search dir again %d\n", work->data2 ) ;
			goto search_dir ;
		} 
    }

	if ( !Flag( FLAG_NO_MOTION_STEP_XZ ) ) {
		work->control.step.vx += work->force.vx ;
		work->control.step.vz += work->force.vz ;
		GV_NearExp8VF( &( work->force ), &DG_ZeroVector, 3 ) ;
	}

	if ( EndMotion( work ) ) {
		SetStatus( PLAYER_DOWNED | PLAYER_GROUND ) ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		SetInvincible( work, INVINCIBLE_TIME ) ;
		if ( work->power.vital <= 0 ) SetMode( work, Dead ) ;
		else SetMode( work, Down ) ;
    }
}

/* ねてる */
static	void	Down( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY | FLAG_NO_IK ) ;
    if ( time == 0 ) {
		ChangeStance( work, GROUND ) ;
		SetStatus( PLAYER_DOWNED ) ;
		if ( work->down_dir == DOWN_FACE ) {
			SetAction( work, MS.shared[ Mdown ], 6 ) ;
		} else {
			SetAction( work, MS.shared[ Mdown_b ], 6 ) ;
		}
		/* ダウンのチェック位置は立ち */
		work->hzx_height = HzxHeight[ STAND ] ;
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
    }

	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	if ( work->ftime < 16 ) {
		GroundIK2( work, 500.0F ) ;
	} else {
		KeepRotAndHeightAdjust( work ) ;
	}
    /* めりこみチェック */
    if ( work->data == 0 ) {
		if ( AvoidSink( work, 200.0F, 0.0F ) == 0 ) work->data = 1 ;
    }
	work->data ++ ;
	if ( ( work->pad->status & PAD_UDLR ) ||
		 ( work->pad->press & PAD_ABXY ) ) work->data += 4 ;
    if ( work->data > 60 && work->pad->press ) {
		SetInvincible( work, 0 ) ;
		/* 念のため天井チェック */
		if ( !CheckCeil( work ) ) {
			SetMode( work, GroundStill ) ;
			SetInvincible( work, INVINCIBLE_TIME ) ;
		} else {
			SetMode( work, Rise ) ;
		}
    }
}

/* 起き上がり */
static	void	Rise( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			FLAG_DONOT_CHECK_WATCH | FLAG_NO_HEADTOENEMY | FLAG_NO_IK ) ;
    if ( time == 0 ) {
		UnsetStatus( PLAYER_SQUAT ) ; /* SE対策 */
		ChangeStance( work, STAND ) ;
		SetFlag( FLAG_HZX_HEIGHT_QUICK ) ;
		if ( work->down_dir == DOWN_FACE ) {
			if ( work->weapon == WP_Nikita || 
				 work->weapon == WP_Aks || 
				 work->weapon == WP_Rgb6 || 
				 work->weapon == WP_m4 ) {
				/* ニキータらだけ別 */
				SetAction( work, nkt_rise, 6 ) ;				
			} else {
				SetAction( work, MS.shared[ Mrise ], 6 ) ;
			}
		} else {
			if ( work->weapon == WP_Nikita ||
				 work->weapon == WP_Aks || 
				 work->weapon == WP_Rgb6 || 
				 work->weapon == WP_m4 ) {
				/* ニキータらだけ別 */
				SetAction( work, nkt_rise_f, 6 ) ;				
			} else {
				SetAction( work, MS.shared[ Mrise_b ], 6 ) ;
			}
		}
    }

	/* 立ち高さと匍匐高さでニアチェックする */
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

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
    if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_DOWNED ) ;
		SetMode( work, StandStill ) ;
		SetInvincible( work, INVINCIBLE_TIME ) ;
		/* 一応デフォルトのうつぶせに戻す */
		work->down_dir = DOWN_FACE ;
    }
}

/* お亡くなり */
static	void	Dead( work, time )
Work		*work ;
int		time ;
{
	int			to, av, c ;

	GM_SetMenuStatus( MENU_WEAPON_OFF | MENU_ITEM_OFF ) ;
	PL_SetDeadFlag( work ) ;
    if ( time == 0 ) {
		//LeaveSubject( work ) ;
        work->idata = 0 ;
        if ( !Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			/* 主観でなかったら最初にはずす */
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
		}
		if ( Status( PLAYER_INTRUDE ) ) {
			/* イントルード */
			SetAction( work, MS.shared[ Mout_intrude ], 6 ) ;
		} else if ( Status( PLAYER_GROUND | PLAYER_DOWNED ) ) {
			/* 匍匐／ダウン */
			if ( work->down_dir == DOWN_FACE ) {
				float	levels[ 2 ] ;
				FVECTOR	pos ;

				levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
				PL_ObjPos( work, HUMAN21_ATAMA, &pos ) ;
				if ( HZX_LevelHazardCheck( work->control.hzx_id, &pos, HZX_CHK_ALL,
										   HZX_FLOOR_NO_PLAYER ) & 2 ) {
					HZX_GetLevelHeight( levels ) ;
				}
				if ( levels[ 1 ]
					- work->body.objs->objs[ HUMAN21_ATAMA ].world.m[ 3 ][ 1 ] < 1500.0F ) {
					SetAction( work, MS.shared[ Mout_intrude ], 6 ) ;
				} else if ( work->control.levels[ 1 ] 
						    - work->body.objs->objs[ HUMAN21_KOSHI ].world.m[ 3 ][ 1 ] < 1500.0F ) {
					SetAction( work, MS.shared[ Mout_intrude ], 6 ) ;
				} else {
					SetAction( work, MS.shared[ Mdamage_down_out ], 6 ) ;
				}
			} else {
				SetAction( work, MS.shared[ Mdamage_down_out_f ], 6 ) ;
			}
		} else if ( CheckTrap( work, WALLTO_TRAP ) ) {
			/* 張り付きエリア */
			SetStatus( PLAYER_CAUTION ) ;
			work->r_sphere = CAUTION_SPHERE ;
			work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
			if ( work->stance == SQUAT ) {
				SetAction( work, MS.shared[ Mout_caution_sq ], 6 ) ;
			} else {
				SetAction( work, MS.shared[ Mout_caution ], 6 ) ;
			}
		} else {
			SetAction( work, MS.shared[ Mdamage_out ], 6 ) ;
		}
		SetInvincible( work, 0 ) ;
        if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			GV_SetActorChild( &work->actor, ( void * )NewScrCrack( 0, 0 ) ) ;
		}
    }

	if ( !Status( PLAYER_CAUTION ) ) {
		SetFlag2( FLAG2_ADJUST_SLOW ) ;
#if 0
		if ( PL_CheckMotionRate( work ) < 0.80F ) GroundIK2( work, 300.0F ) ;
		else									  KeepRotAndHeightAdjust( work ) ;
#else
		if ( !Status( PLAYER_GROUND | PLAYER_DOWNED ) ) GroundIK2( work, 400.0F ) ;
		else											GroundIK2( work, 600.0F ) ;
#endif
		if ( work->idata == 0 && work->control.interp == 0 ) {
			av = AvoidSink2( work, NEAR_HEIGHT_GROUND, 750.0F, 1100.0F, 1900.0F ) ;
			if ( ( av & 4 ) || ( ( av & 3 ) /*== 3*/ ) /*|| ( av & 0xf0 )*/ ) {
				if ( !Status( PLAYER_GROUND | PLAYER_DOWNED ) ||
					( ( av & 4 ) || ( ( av & 3 ) == 3 ) ) ) {
					to = CheckGroundEnableDir( work ) ;
					if ( to >= 0 ) {	
						if ( GV_DiffDirAbs( work->control.rot.vy, to ) > 1200 ) {
							to += 2048 ;
						}
						work->control.turn.vy = to ;
						work->control.interp = 8 ;
					} 
					work->idata = 1 ;
				} else if ( GetGRot( work, 500.0F ) != 0 ) {
					/* 斜めのところでは移動量なし */
					SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
				}
			} else if ( av == 0 && GetGRot( work, 500.0F ) != 0 ) {
				/* 斜めのところでは移動量なし */
				SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
			}
		} else if ( work->idata == 1 || GetGRot( work, 500.0F ) != 0 ) {
			/* 斜めのところでは移動量なし */
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		}
	}
#if 1
	if ( work->stance == STAND ) {
		GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	}
#endif
	/* 主観カメラ位置 */
	if ( work->body.height < 500.0F ) {
		work->camdir.vx = work->g_rot ;
		SetFlag( FLAG_CAMERA_ON_WAIST ) ;
		work->camera.vy += 250.0F ;
	} 
	/* 敵がカクカクするので、
	   FindPos腰 */
	GM_PlayerFindPos.vy = work->control.mov.vy ;

	c = 0 ;
	if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		/* 腰とカメラの間に壁があったら主観解除 */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &work->camera,
								    HZX_CHK_ALL, 0, 0 ) ) {
			c = 1 ;
		}
	} 
	if ( work->data2 == 0 && ( PL_CheckMotionRate( work ) > 0.80F || c == 1 ) ) {
		/* 主観だったらここではずす */
		if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
			SetFlag2( FLAG2_NO_TIMERESET_WHEN_CHANGE ) ;
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
			UnsetFlag2( FLAG2_NO_TIMERESET_WHEN_CHANGE ) ;
		}
		LeaveSubject( work ) ;
		//LeaveIntrude( work ) ;
		if ( !Status( PLAYER_INTRUDE ) ) NewScrCrack( -1, 0 ) ;
		work->data2 = 1 ;
	}

    if ( !Status( PLAYER_GROUND | PLAYER_DOWNED | PLAYER_CAUTION ) ) {
//		work->control.step.vx += work->force.vx / 4.0F ;
//		work->control.step.vz += work->force.vz / 4.0F ;
		GV_NearExp8VF( &( work->force ), &DG_ZeroVector, 3 ) ;
    }
    if ( work->data == 0 && EndMotion( work ) ) {
		work->body.m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
		work->body.m_ctrl->mt3_ctrl[ 1 ].flag |= MT3_SLEEP ;
		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
		GM_GameOverProcEnd( &work->actor ) ;
		work->data = 1 ;
    }
#if 0
    if ( work->pad->press & PAD_STA ) {
		work->power.vital = 30 ;
		SetMode( work, StandStill ) ;
		SetInvincible( work, 60 ) ;
		UnsetStatus( PLAYER_DAMAGED | PLAYER_DOWNED | PLAYER_DEAD |
					PLAYER_MENU_DISABLE ) ;
    }
#endif
}

/* 壁たたき左 */
static	void	KnockWallLeft( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		if ( work->stance == SQUAT ) {
			SetAction2( work, MS.change[ Mknock_sq1 ], 4, 0x1ffe ) ;
		} else {
			SetAction2( work, MS.change[ Mknock_l1 ], 4, 0x1ffe ) ;
		}
    }

	GM_SetPlayerStatusEX( I64(0), PLAYER2_KNOCK_WALL ) ;
	if ( CheckKnockSegment( work ) ) work->data3 = 1 ;
    if ( PL_CheckMotionTime2( work, 35 ) ) work->data3 = 2 ;

    if ( EndMotion2( work ) || 
		!Status( PLAYER_CAUTION ) ||
		Status( PLAYER_WATCH ) ) {
		SetMode2( work, NULL ) ;
    }
}

/* 壁たたき右 */
static	void	KnockWallRight( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		SetAction2( work, MS.change[ Mknock_r1 ], 4, 0x1ffe ) ;
    }

	GM_SetPlayerStatusEX( I64(0), PLAYER2_KNOCK_WALL ) ;
	if ( CheckKnockSegment( work ) ) work->data3 = 1 ;
    if ( PL_CheckMotionTime2( work, 35 ) ) work->data3 = 2 ;

    if ( EndMotion2( work ) || 
		!Status( PLAYER_CAUTION ) ||
		Status( PLAYER_WATCH ) ) {
		SetMode2( work, NULL ) ;
    }
}

/* 壁たたき右再び */
static	void	KnockWallRightAgain( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		SetAction2( work, MS.change[ Mknock_r2 ], 4, 0x1ffe ) ;
    }

	GM_SetPlayerStatusEX( I64(0), PLAYER2_KNOCK_WALL ) ;
	if ( CheckKnockSegment( work ) ) work->data3 = 1 ;

    if ( EndMotion2( work ) || 
		!Status( PLAYER_CAUTION ) ||
		Status( PLAYER_WATCH ) ) {
		SetMode2( work, NULL ) ;
    }
}

/* 壁たたき左再び */
static	void	KnockWallLeftAgain( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		if ( work->stance == SQUAT ) {
			SetAction2( work, MS.change[ Mknock_sq2 ], 4, 0x1ffe ) ;
		} else {
			SetAction2( work, MS.change[ Mknock_l2 ], 4, 0x1ffe ) ;
		}
    }

	GM_SetPlayerStatusEX( I64(0), PLAYER2_KNOCK_WALL ) ;
	if ( CheckKnockSegment( work ) ) work->data3 = 1 ;

    if ( EndMotion2( work ) || 
		!Status( PLAYER_CAUTION ) ||
		Status( PLAYER_WATCH ) ) {
		SetMode2( work, NULL ) ;
    }
}


