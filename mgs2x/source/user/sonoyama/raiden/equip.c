/*
   equip.c
   プレイヤー／装備品特殊
   
   1999/12/10 M.Sonoyama
   $Id: equip.c,v 1.1.1.3 2002/11/19 11:50:55 Yoshizawa1 Exp $
*/

/* ダンボール主観 */
void			PL_SubjectTurnCB( PlayerWork *work )
{
	GV_PAD		*pad ;
	FVECTOR		right = { 96.0F, 0.0F, 80.0F } ;
	FVECTOR		left = { -96.0F, 0.0F, 80.0F } ;
	FVECTOR		up = { 0.0F, 64.0F, 80.0F } ;
	FVECTOR		aim, diff ;
	short		turn_diff ;
	float		interp ;
	int			p, status, c, ai ;

	work->camdir.vx = 0 ;
	pad = work->pad ;
	DG_SetPos2( &work->camera, &work->control.rot ) ;
	status = pad->status & ( PAD_R2 | PAD_L2 ) ;
	if ( status != 0 ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		if ( status == PAD_R2 ) {
			DG_PutVector( &right, &aim, 1 ) ;
			interp = ( float )pad->pressure[ PAD_PRESS_R2 ] / 255.0F ;	
			turn_diff = -400 ;
		} else if ( status == PAD_L2 ) {
			DG_PutVector( &left, &aim, 1 ) ;
			interp = ( float )pad->pressure[ PAD_PRESS_L2 ] / 255.0F ;
			turn_diff = 400 ;
		} else {
			DG_PutVector( &up, &aim, 1 ) ;
			p = pad->pressure[ PAD_PRESS_R2 ] ;
			if ( p < pad->pressure[ PAD_PRESS_L2 ] ) p = pad->pressure[ PAD_PRESS_L2 ] ;
			interp = ( float )p / 255.0F ;
			turn_diff = 0 ;
			work->camdir.vx = ( short )( 240.0F * interp ) ;
		}
		_sceVu0SubVector( &diff, &aim, &work->camera ) ;
		_sceVu0ScaleVector( &diff, &diff, interp ) ;
		_sceVu0AddVector( &work->camera, &work->camera, &diff ) ;
		work->camdir.vy = work->control.rot.vy + ( short )( ( float )turn_diff * interp ) ;
	} else {
		status = pad->status & PAD_UDLR ;
		DG_COPY_VEC( &diff, &DG_ZeroVector ) ;
		c = 0 ;
		if ( status != 0 ) {
			SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
			diff.vz = 80.0F ;
			c = 1 ;
		}
		ai = pad->analog_input & GV_PAD_ANALOG_L_USE ;
		interp = 0.0F ;
		turn_diff = 0 ;
		if ( status & PAD_U ) {
			if ( ai ) interp = DG_FABS( ( float )( pad->left_dy - 128 ) / 128.0F ) ;
			else interp = ( float )pad->pressure[ PAD_PRESS_U ] / 255.0F ;
			work->camdir.vx = -( short )( 240.0F * interp ) ;
			diff.vy = -96.0F * interp ;
		} else if ( status & PAD_D ) {
			if ( ai ) interp = DG_FABS( ( float )( pad->left_dy - 128 ) / 128.0F ) ;
			else 	  interp = ( float )pad->pressure[ PAD_PRESS_D ] / 255.0F ;
			work->camdir.vx = ( short )( 240.0F * interp ) ;
			diff.vy = 96.0F * interp ;
		}
		/* 主観操作反転 */
		PL_ShukanReverse( &diff.vy, &work->camdir.vx ) ;

		if ( status & PAD_L ) {
			if ( ai ) interp = DG_FABS( ( float )( pad->left_dx - 128 ) / 128.0F ) ;
			else	  interp = ( float )pad->pressure[ PAD_PRESS_L ] / 255.0F ;
			diff.vx = -96.0F * interp ;
			turn_diff = 400 ;
		} else if ( status & PAD_R ) {
			if ( ai ) interp = DG_FABS( ( float )( pad->left_dx - 128 ) / 128.0F ) ;
			else      interp = ( float )pad->pressure[ PAD_PRESS_R ] / 255.0F ;
			diff.vx = 96.0F * interp ;
			turn_diff = -400 ;
		}
		if ( c ) {
			DG_RotVector( &diff, &diff, 1 ) ;
			_sceVu0AddVector( &work->camera, &work->camera, &diff ) ;
			work->camdir.vy = work->control.rot.vy + ( short )( ( float )turn_diff * interp ) ;		
		}
	}
}

/* ダンボール静止 */
static	void	CB_BoxStill( work, time )
Work		*work ;
int		time ;
{
	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_WEAPON_DISABLE ) ;
	SetFlag( FLAG_FINDPOS_IS_WAIST | FLAG_CANNOT_CHANGE_WEAPON | FLAG_RECOVER_ENABLE ) ;
	GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ SQUAT ] ;
    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		if ( work->floor_atr & HZX_FLOOR_STEP ) {
			work->data = 1 ;
//			SetAction( work, none_stand, 6 ) ;
		} else if ( Status( PLAYER_ON_CORPSE ) ) {
			work->data = 1 ;
		} else {
//			SetAction( work, MS.shared[ Mbox_idle ], 6 ) ;
		}
		PL_FootPrintForce( work->foot_work, 3 ) ;
    }

	/* 主観移動 */
	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	if ( work->data == 0 ) {
		if ( CB_BoxIncline( work ) == 2 ) { /* 階段 */
			work->data = 1 ;
			goto cb_boxstill_stand ;
		}
		SetFlag( FLAG_NO_IK ) ;
		SetAction( work, MS.shared[ Mbox_idle ], 6 ) ;
		work->it_trg = work->g_rot ;
	} else {
cb_boxstill_stand :
		SetAction( work, none_stand, 6 ) ;
		SetStatus( PLAYER_CB_BOX_STAND ) ;
		work->it_trg = 0 ;
        if ( !( work->floor_atr & HZX_FLOOR_STEP ) &&
			 !Status( PLAYER_ON_CORPSE ) ) {
			work->data = 0 ;
		}
	}

    SetStatus( PLAYER_CB_BOX ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

    if ( Status( PLAYER_WATCH ) ) {
		if ( work->data == 0 ) {
			work->camera.vy = work->control.mov.vy + 360.0F ;
		} else {
			work->camera.vy = work->control.mov.vy + 500.0F ;
		}
		PL_SubjectTurnCB( work ) ;
    } else {
		if ( PadTo >= 0 ) {
			SetMode( work, CB_BoxMove ) ;
		}
    }
	
	/* 取られた */
	if ( Status( PLAYER_CB_BOX_CANCELED ) ) {
		SetStatus( PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
		SetModeName( work, CB_BoxCanceled, "CB_BoxCanceled" ) ;
	}
}

/* ダンボール移動 */
static	void	CB_BoxMove( Work *work, int time )
{
	int			interp ;

	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_WEAPON_DISABLE ) ;
	SetFlag( FLAG_FINDPOS_IS_WAIST | FLAG_CANNOT_CHANGE_WEAPON ) ;
	GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ SQUAT ] ;
    if ( time == 0 ) {
		//	SetAction( work, MS.shared[ Mstair_run ], 6 ) ;
    }
	interp = ( time == 0 ) ? 4 : 6 ;

    SetStatus( PLAYER_CB_BOX | PLAYER_MOVE ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	if ( work->floor_atr & HZX_FLOOR_STEP ) {
		work->data = 1 ;
	} else if ( Status( PLAYER_ON_CORPSE ) ) {
		work->data = 1 ;
	} else {
		work->data = 0 ;
	}


	if ( work->data == 0 ) {
		SetFlag( FLAG_NO_IK ) ;
		work->it_trg = work->g_rot ;
		if ( CB_BoxIncline( work ) == 2 ) {	/* 階段付近 */
			work->data = 1 ;
			goto cb_boxmove_stand ;
		}
	} else {
cb_boxmove_stand :		
	    if ( !Status( PLAYER_ON_CORPSE ) ) SetStatus( PLAYER_CB_BOX_STAND ) ;
		work->it_trg = 0 ;
	}

    if ( PadTo < 0 || Status( PLAYER_WATCH ) /*||
		 ( CheckMovRotLenSegment( work->control.hzx_id, &work->control.mov,
								   &DG_ZeroVector, &work->control.rot, 550.0F,
								   HZX_CHK_ALL, HZX_SEG_NO_PLAYER, 
								   HZX_FLOOR_NO_PLAYER ) &&
		   CheckMovRotLenSegment( work->control.hzx_id, &work->control.mov,
								   &DG_ZeroVector, &work->control.rot, -550.0F,
								   HZX_CHK_ALL, HZX_SEG_NO_PLAYER, 
								   HZX_FLOOR_NO_PLAYER ) )*/ ) {
//		SetMode( work, CB_BoxStill ) ;
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		if ( work->data == 0 ) {
			StopTurn( work ) ;
			SetMode( work, CB_BoxStop ) ;
		} else {
			SetMode( work, CB_BoxStill ) ;
		}
		return ;
	} else {
	    /* 主観移動 */
	    if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;
	    else
		work->control.turn.vy = PadTo ;
	}

    if ( PL_SubjectMove ) { /* 主観移動モード以外のときに方向を更新する */
	/* 主観移動用のステップ計算 */
	SubjectMoveStep( work ) ; /* subject.c */
    }

    if ( work->pad->analog_input & GV_PAD_ANALOG_L_USE ) {
		int	max, tmp ;
	
		max = work->pad->left_dx - 128 ;
		if ( max < 0 ) max = -max ;
		tmp = work->pad->left_dy - 128 ;
		if ( tmp < 0 ) tmp = -tmp ;
		if ( max < tmp ) max = tmp ;	
		if ( max >= ANALOG_MARGIN ) {
			if ( work->data == 1 ) {
				if ( Status( PLAYER_ON_CORPSE ) ) {
					SetAction( work, MS.shared[ Mbox_walk_on ], interp ) ;
					SetStatus( PLAYER_WALK ) ;
				} else if ( max < 80 ) {
					SetAction( work, none_stair_walk, interp ) ;
					SetStatus( PLAYER_WALK ) ;
				} else {
					SetAction( work, none_stair_run, interp ) ;
				}
			} else {
				if ( Status( PLAYER_ON_CORPSE ) ) {
					SetAction( work, MS.shared[ Mbox_walk_on ], interp ) ;
					SetStatus( PLAYER_WALK ) ;
				} else if ( max < 80 ) {
					SetAction( work, MS.shared[ Mbox_walk ], interp ) ;
					SetStatus( PLAYER_WALK ) ;
				} else if ( max < 124 ) {
					SetAction( work, MS.shared[ Mbox_run ], interp ) ;
				} else {
					SetAction( work, MS.shared[ Mbox_run ], interp ) ;
					SetStatus( PLAYER_DASH ) ;
				}
			}
		}
    } else {
		int	max, tmp ;
			
		max = work->pad->pressure[ PAD_PRESS_U ] ;
		tmp = work->pad->pressure[ PAD_PRESS_D ] ;
		if ( max < tmp ) max = tmp ;
		tmp = work->pad->pressure[ PAD_PRESS_L ] ;
		if ( max < tmp ) max = tmp ;
		tmp = work->pad->pressure[ PAD_PRESS_R ] ;
		if ( max < tmp ) max = tmp ;
		max /= 2 ;
		if ( work->data == 1 ) {
			if ( Status( PLAYER_ON_CORPSE ) ) {
				SetAction( work, MS.shared[ Mbox_walk_on ], interp ) ;
				SetStatus( PLAYER_WALK ) ;
			} else if ( max < 64 ) {
				SetAction( work, none_stair_walk, interp ) ;
				SetStatus( PLAYER_WALK ) ;
			} else {
				SetAction( work, none_stair_run, interp ) ;
			}
		} else {
			if ( Status( PLAYER_ON_CORPSE ) ) {
				SetAction( work, MS.shared[ Mbox_walk_on ], interp ) ;
				SetStatus( PLAYER_WALK ) ;
			} else if ( max < 64 ) {
				SetAction( work, MS.shared[ Mbox_walk ], interp ) ;
				SetStatus( PLAYER_WALK ) ;
			} else if ( max < 124 ) {
				SetAction( work, MS.shared[ Mbox_run ], interp ) ;
			} else {
//				SetAction( work, MS.shared[ Mbox_dash ], interp ) ;
				SetAction( work, MS.shared[ Mbox_run ], interp ) ;
                SetStatus( PLAYER_DASH ) ;
			}
		}
    }    

	/* 足跡 */
	if ( work->motion1 == MS.shared[ Mbox_walk ] ) {
		if ( Status( PLAYER_SNAKE ) ) {
			PL_FootPrintAct( work->foot_work, 0, 1, 36 ) ;
		} else {
			PL_FootPrintAct( work->foot_work, 0, 37, 7 ) ;
		}
	} else if ( work->motion1 == MS.shared[ Mbox_run ] ) {
		if ( Status( PLAYER_SNAKE ) ) {
			PL_FootPrintAct( work->foot_work, 0, 25, 4 ) ;
		} else {
			PL_FootPrintAct( work->foot_work, 0, 22, 2 ) ;
		}
	}

	/* 取られた */
	if ( Status( PLAYER_CB_BOX_CANCELED ) ) {
		SetStatus( PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
		SetModeName( work, CB_BoxCanceled, "CB_BoxCanceled" ) ;
	}

	if ( work->ftime > 32 ) SetStatus( PLAYER_CBBOX_RUN ) ;
}

/* ダンボール停止 */
static	void	CB_BoxStop( work, time )
Work		*work ;
int		time ;
{
	SetStatus( PLAYER_EVENT_ENABLE | PLAYER_WEAPON_DISABLE ) ;
	SetFlag( FLAG_NO_IK | FLAG_NO_MOTION_STEP_XZ | FLAG_FINDPOS_IS_WAIST |
			 FLAG_CANNOT_CHANGE_WEAPON ) ;
	GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ SQUAT ] ;

	/* 主観移動 */
	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		if ( work->floor_atr & HZX_FLOOR_STEP ) {
			SetAction( work, none_stand, 6 ) ;
			SetStatus( PLAYER_CB_BOX | PLAYER_CB_BOX_STAND ) ;
			SetMode( work, CB_BoxStill ) ;
			return ;
		}
		SetAction( work, MS.shared[ Mbox_stop ], 6 ) ;
    }
	if ( CB_BoxIncline( work ) == 2 ) {
		SetAction( work, none_stand, 6 ) ;
		SetStatus( PLAYER_CB_BOX | PLAYER_CB_BOX_STAND ) ;
		SetMode( work, CB_BoxStill ) ;
		return ;
	}
//    SetStatus( PLAYER_CB_BOX | PLAYER_MOVE ) ;
    SetStatus( PLAYER_CB_BOX ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

    if ( EndMotion( work ) || Status( PLAYER_WATCH ) || PadTo >= 0 ) {
		SetMode( work, CB_BoxStill ) ;
    } 
	/* 取られた */
	if ( Status( PLAYER_CB_BOX_CANCELED ) ) {
		SetStatus( PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
		SetModeName( work, CB_BoxCanceled, "CB_BoxCanceled" ) ;
	}
}

/* ダンボール取られた */
static	void	CB_BoxCanceled( Work *work, int time )
{
	SetFlag( FLAG_NO_IK | FLAG_DONOT_CHECK_WATCH | FLAG_FINDPOS_IS_WAIST |
			 FLAG_CANNOT_CHANGE_WEAPON ) ;
//    SetStatus( PLAYER_CB_BOX | PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
    SetStatus( PLAYER_CB_BOX ) ;
	GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ SQUAT ] ;
    if ( time == 0 ) {
		LeaveSubject( work ) ;	/* 主観抜け */
		ChangeStance( work, STAND ) ;
		SetAction( work, MS.shared[ Mbox_idle ], 6 ) ;
    }
	CB_BoxIncline( work ) ;
	GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
    work->it_trg = work->g_rot ;
	if ( work->pad->status & PAD_UDLR ) {
		NoEquip( work ) ;	/* 素手にする */
		SetModeName( work, StandStill, "StandStill" ) ;
	}
}

/* 双眼鏡 */
static	void	SetScope( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		IntoSubject( work ) ;
		GM_CameraDir.vx = work->camdir.vx = work->g_rot ;
		SetAction( work, StanceAct( work->stance ), 6 ) ;
		work->idata = -1 ;
    }

    SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP ) ;
	SetStatus( PLAYER_WEAPON_INVISIBLE ) ;

	/* 主観移動 */
	if ( PL_SubjectMove )
	    SubjectMoveTurn( work ) ;
	else
	    SubjectTurn( work ) ;

	if ( work->item == IT_Scope ) {
		SetSubjectMotionMaster( work, scp_fire_p, -1, scp_fire_p_cr ) ;
	} else {
		SetSubjectMotionMaster( work, cam_fire_p, -1, cam_fire_p_cr ) ;
	}

	/* 匍匐のとき */
	if ( work->stance == GROUND ) {
#if 0
		int			n ;
		SVECTOR		rot ;
#endif
		SetFlag( FLAG_RECOVER_ENABLE | FLAG_CAMERA_ON_WAIST ) ;	
#if 0
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
			work->g_rot = GetGRot( work, 750.0F ) ;
			work->camera.vy = work->control.levels[ 0 ] + INTRUDE_CAMERA_HEIGHT_STILL ;	
		}
#endif
		GroundRotateLimitControl( work ) ;
	}
}
