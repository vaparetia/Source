/*
   attack.c 
   ライデン／攻撃関連
   
   1999/07/23 M.Sonoyama
   $Id: attack.c,v 1.1.1.3 2002/11/19 11:50:53 Yoshizawa1 Exp $
   */

/*------------------------------------------------------------*/

#include "BP_Misc.h"

/* つかみターゲット設定 */
static	void	SetCaptureTarget( Work *work, int flag, 
								  float size_x, float size_y, float size_z,
								  float force_x, float force_y, float force_z,
								  int objNo, 
								  float shift_x, float shift_y, float shift_z,
								  long64 weapon_type, TARGET_CALLBACK callback )
{
	FVECTOR		mov, size, force, shift ;
	FMATRIX		mat ;
	TARGET		*t ;
	CAPTURE_TARGET	*c ;

	size.vx = size_x ;
	size.vy = size_y ;
	size.vz = size_z ;
	force.vx = force_x ;
	force.vy = force_y ;
	force.vz = force_z ;
	shift.vx = shift_x ;
	shift.vy = shift_y ;
	shift.vz = shift_z ;
	if ( objNo >= 0 ) DG_SetPos( &work->body.objs->objs[ objNo ].world ) ;
	else			  DG_SetPos( &work->body.objs->world ) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &mat ) ;
	GV_MatToVec( &mat, &mov ) ;
	t = &( work->offense ) ;
	c = &( work->capture ) ;
	GM_SetTarget( t, flag, GM_CurrentMap, ENEMY_SIDE,
				  &size, &force ) ;
	GM_SetTargetWeaponType( t, weapon_type ) ;
	GM_SetCaptureTarget( t, c, &( work->control ), &( work->body ) ) ;
	if ( callback != NULL ) GM_SetTargetCallBack( t, callback, work ) ;
	GM_MoveTarget( t, &mov ) ;
	GM_PutTarget( t ) ;
//	NewTargetView2( t, 32, 232, 186 ) ;
}
								 

/* ホーミング先との間に壁が無いかチェック */
static	HOMING_TRG	*CheckHazardBetweenItoHoming( work, homing )
Work		*work ;
HOMING_TRG	*homing ;
{
    FVECTOR		to ;
    
    if ( homing == NULL ) return NULL ;
    GV_MatToVec( homing->world, &to ) ;
    if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov,
							   &to, HZX_CHK_ALL, 
							   HZX_SEG_NO_PLAYER | HZX_SEG_NO_ENEMY,
							   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_NO_ENEMY ) ) {
			       
		return NULL ;
    }
    return homing ;
}

/* ターゲット先とのとの間に壁が無いかチェック */
static		int		CheckHazardBetweenItoTarget( work, targ )
Work		*work ;
TARGET		*targ ;
{
	if ( targ == NULL ) return 0 ;
    if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
							   &work->control.mov, &targ->center,
							   HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
							   HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
							   HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
		return 1 ;
	}
	return 0 ;
}

/*-------------------------------------------------------------------------*/

/* 死体つかみコールバック */
static	void	DeadCaptureCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;
    /* つかめない場合 */
    if ( !( def->class & TARGET_DEAD ) || /* 死んでない */
		( off->capture->capture != NULL ) || /* もう捕まえたやつがいる */
		( off->center.vy < def->center.vy ) || /* 死体の方が上にある */
		CheckBetweenHazard( work, &( off->capture->ctrl->mov ),	/* 壁がある */
						   &( def->capture->ctrl->mov ) ) ) {
		def->damaged &= ~TARGET_CAPTURE ;
		def->weapon_type &= ~( off->weapon_type ) ;
		if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
		return ;
    }
#if 0
    /* 向きがあわないとつかめない */
    {
		CONTROL		*my, *you ;

		my = off->capture->ctrl ;
		you = def->capture->ctrl ;
		if ( GV_DiffDirAbs( my->rot.vy, you->rot.vy ) > 512 &&
			GV_DiffDirAbs( my->rot.vy, you->rot.vy ) < 1536 ) {
			def->damaged &= ~TARGET_CAPTURE ;
			def->weapon_type &= ~( off->weapon_type ) ;
			if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
			return ;
		}
    }
#endif
    /* 頭ＯＲ足 */
    {
		OBJECT	*my, *you ;
		float	len, len2 ;
		FVECTOR	diff, foot ;

		my = off->capture->body ;
		you = def->capture->body ;
		diff.vx = my->objs->world.m[ 3 ][ 0 ] - you->objs->objs[ 12 ].world.m[ 3 ][ 0 ] ;
		diff.vy = 0.0F ;
		diff.vz = my->objs->world.m[ 3 ][ 2 ] - you->objs->objs[ 12 ].world.m[ 3 ][ 2 ] ;
		len = GV_VecLen3F( &diff ) ;
		foot.vx = ( you->objs->objs[ 16 ].world.m[ 3 ][ 0 ] + 
				   you->objs->objs[ 20 ].world.m[ 3 ][ 0 ] ) / 2.0F ;
		foot.vy = 0.0F ;
		foot.vz = ( you->objs->objs[ 16 ].world.m[ 3 ][ 2 ] + 
				   you->objs->objs[ 20 ].world.m[ 3 ][ 2 ] ) / 2.0F ;
		diff.vx = my->objs->world.m[ 3 ][ 0 ] - foot.vx ;
		diff.vy = 0.0F ;
		diff.vz = my->objs->world.m[ 3 ][ 2 ] - foot.vz ;
		len2 = GV_VecLen3F( &diff ) ;
		if ( len > 750.0F && len2 > 750.0F ) {
			def->damaged &= ~TARGET_CAPTURE ;
			def->weapon_type &= ~( off->weapon_type ) ;
			if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
			return ;
		}

#if 1
		diff.vx = you->objs->objs[ 0 ].world.m[ 3 ][ 0 ] - my->objs->objs[ 0 ].world.m[ 3 ][ 0 ] ;
		diff.vz = you->objs->objs[ 0 ].world.m[ 3 ][ 2 ] - my->objs->objs[ 0 ].world.m[ 3 ][ 2 ] ;
		work->data2 = GV_VecDir2( &diff ) ; /* 調節 */
#else
		diff.vx = you->objs->objs[ 2 ].world.m[ 3 ][ 0 ] - you->objs->world.m[ 3 ][ 0 ] ;
		diff.vz = you->objs->objs[ 2 ].world.m[ 3 ][ 2 ] - you->objs->world.m[ 3 ][ 2 ] ;
		work->data2 = GV_VecDir2( &diff ) - 140 ; /* 調節 */
#endif
		off->capture->flag |= def->capture->flag ;
		if ( len < len2 ) {		/* 頭 */
			off->capture->flag |= CAPTURE_HEAD ;
			def->capture->flag |= CAPTURE_HEAD ;
		} else {				/* 足 */
			off->capture->flag |= CAPTURE_FOOT ;
			def->capture->flag |= CAPTURE_FOOT ;
		}
    }
    GM_CaptureTarget( off, def ) ;
}

/* 首絞めつかみ＆投げ＆ホールドアップ用コールバック */
static	void	HangCaptureCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;

    /* つかめない場合 */
    if ( ( def->capture == NULL ) ||		 /* capture_targetがない */ 
		 ( def->capture->flag & ( CAPTURE_HANG | CAPTURE_THROW ) ) ||	/* 誰かがつかまえている */
		 ( def->capture->ctrl->attribute & CTRL_ATR_HANG_THROUGH ) ||	/* ハイテク兵 */
		( off->capture->capture != NULL ) || /* もう捕まえたやつがいる */
		( def->class & TARGET_DEAD ) || /* 死んでいる */
		( CheckBetweenHazard( work, &off->capture->ctrl->mov,	/* 壁がある */
							 ( FVECTOR * )&( def->capture->body->objs->world.m[ 3 ] ) ) ) ) {
		def->damaged &= ~TARGET_CAPTURE ;
		def->weapon_type &= ~( off->weapon_type ) ;
		if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
		if ( def->capture->ctrl->attribute & CTRL_ATR_HANG_THROUGH ) {
			printf( "raiden/attack.c : ハイテク兵つかみコール\n" ) ;
			work->post |= MAIL_HANG_THROUGH ;
		}
		return ;
    }
    GM_CaptureTarget( off, def ) ;
	//def->capture->flag |= CAPTURE_HANG ;
}

static	void	ThrowCaptureCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;

    /* つかめない場合 */
    if ( ( def->capture == NULL ) ||		 /* capture_targetがない */ 
		 ( def->capture->flag & ( CAPTURE_HANG | CAPTURE_THROW ) ) ||	/* 誰かがつかまえている */
		( off->capture->capture != NULL ) || /* もう捕まえたやつがいる */
		( def->class & TARGET_DEAD ) || /* 死んでいる */
		( CheckBetweenHazard( work, &off->capture->ctrl->mov,	/* 壁がある */
							 ( FVECTOR * )&( def->capture->body->objs->world.m[ 3 ] ) ) ) ) {
		def->damaged &= ~TARGET_CAPTURE ;
		def->weapon_type &= ~( off->weapon_type ) ;
		if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
		return ;
    }
    GM_CaptureTarget( off, def ) ;
	//def->capture->flag |= CAPTURE_THROW ;
}

/* Ｃ４つけコールバック */
static	void	C4CaptureCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;
    /* つかめない場合 */	
    if ( ( def->capture == NULL ) ||		  /* capturetargetがない */
		 ( off->capture->capture != NULL ) || /* もう捕まえたやつがいる */
		 ( def->capture->flag & CAPTURE_C4EXIST ) || /* もうＣ４ついている */
		( def->class & TARGET_DEAD ) || /* 死んでいる */
		CheckBetweenHazard( work, &( off->capture->ctrl->mov ),	/* 壁がある */
						   &( def->capture->ctrl->mov ) ) ||
		GV_DiffDirAbs( work->control.rot.vy, 		/* 向きが違う */
					   def->capture->ctrl->rot.vy ) > 512 ) {
		def->damaged &= ~TARGET_CAPTURE ;
		def->weapon_type &= ~( off->weapon_type ) ;
		if ( off->capture->capture == NULL ) off->damaged &= ~TARGET_CAPTURE ;
		return ;
    }
    GM_CaptureTarget( off, def ) ;

	/* 即座にクリア */
	def->damaged &= ~TARGET_CAPTURE ;
	def->weapon_type &= ~( off->weapon_type ) ;
	def->capture->capture = NULL ;

	work->stick_wp_work.target = def ;
	work->stick_wp_work.capture = def->capture ;
}

/* ひきずり、つかみステートのセット */
static	void	SetCaptureStatus( long64 state )
{
	Work		*work ;

	work = GM_PlayerWork ;
	if ( work->capture.capture != NULL &&
		 work->capture.capture->ctrl != NULL ) {
		if ( !( work->capture.capture->ctrl->attribute & CTRL_ATR_NPC ) ) {
			SetStatus( state ) ;
		}
		/* ＮＰＣつかみでもステルス濃い */
		if ( work->item == IT_Stealth ) GM_SetPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH ) ;
	}
}

/*------------------------------------------------------------*/

/* 素手 */

/* ひきずりアジャスト */
static	void	PullBodyAdjust( Work *work )
{
	FVECTOR		from, to, diff ;
	SVECTOR		rot ;
    CAPTURE_TARGET	*c ;

    c = &( work->capture ) ;
	if ( c->capture == NULL ) return ;

	from.vx = work->control.mov.vx ;
	from.vy = work->control.levels[ 0 ] ;
	from.vz = work->control.mov.vz ;
	to.vx = c->capture->body->objs->world.m[ 3 ][ 0 ] ;
	to.vy = c->capture->body->objs->world.m[ 3 ][ 1 ] ;
	to.vz = c->capture->body->objs->world.m[ 3 ][ 2 ] ;
	if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
							   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) & 1 ) {
		to.vy = HZX_GetFloorLevel() ;
	}
	_sceVu0SubVector( &diff, &to, &from ) ;
	GV_VecToRot( &diff, &rot ) ;
	if ( rot.vx > 2048 ) rot.vx -= 4096 ;
	else if ( rot.vx < -2048 ) rot.vx += 4096 ;
	if ( rot.vx < 0 ) {
		rot.vx = rot.vx * 2 / 3 ;
		rot.vy = 0 ; rot.vz = 0 ;
		SetRotAdjust( work, &rot, HUMAN21_ONAKA ) ;
	}
}

/* 死体ひきずり */
static	void	PullBody(Work *work, int time )
{
    TARGET		*t ;
    CAPTURE_TARGET	*c ;

    t = &( work->offense ) ;
    c = &( work->capture ) ;
    /* 主観には入れない */
	//    SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
    /* 武器変えられない */
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 320 ;

    if ( time == 0 ) {
		/* つかみ判定 */
		SetCaptureTarget( work, TARGET_OFFENSE,
						  300.0F, 950.0F, 300.0F,
						  0.0F, 0.0F, 0.0F,
						  HUMAN21_KOSHI, 0.0F, 0.0F, 0.0F, 
						  WP_HANG, DeadCaptureCallBack ) ;
		/* 主観から強制ぬけ */
		//	LeaveSubject( work ) ;
		work->act_name = PULL_BODY ;
		return ;
    }

    switch ( work->data ) {
    case 0 :					/* つかみ始め */
		if ( work->ftime <= 1 ) {
			/* つかめなかった */
			if ( !( t->damaged & TARGET_CAPTURE ) ||
				 ( t->capture->capture != NULL &&
				   ( HZX_OnlineHazardCheck( work->control.hzx_id, 
										    &work->control.mov, 
										    &work->capture.capture->ctrl->mov,
										    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ||
					 HZX_OnlineHazardCheck( work->control.hzx_id, 
										    &work->control.mov, 
										    &work->pre_mov,
										    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) ) ) {
				ClearCaptureTarget( work ) ;
				if ( work->stance == STAND ) {
					if ( PadTo < 0 ) SetMode( work, StandStill ) ;	
					else			 SetModeName( work, StandRun, "StandRun" ) ;
				} else {
					SetMode( work, SquatStill ) ;
				}
				//		SetAction( work, MS.shared[ Mcarry_miss ], 6 ) ;
				//		work->data = 6 ;
				return ;
			}
			ChangeStance( work, STAND ) ;
#if 1
			work->control.turn.vy = work->data2 ;
#else
			/* 回転と位置を合わせる */
			if ( c->capture->flag & CAPTURE_HEAD ) {
				//		work->control.turn.vy = work->control.rot.vy = work->data2 + 2048 ;
				work->control.turn.vy = work->data2 + 2048 ;
				//		work->control.turn.vy = 0 ; c->capture->ctrl->rot.vy = 2048 ;
				dir.vy = work->data2 ; dir.vx = dir.vz = 0 ;
				//		DG_SetPos2( &c->capture->ctrl->mov, &dir ) ;
				DG_SetPos2( &c->capture->ctrl->mov, &( work->control.turn ) ) ;
				if ( c->capture->flag & CAPTURE_FRONT ) {
					DG_PutVector( &Shift[ 1 ], &pos, 1 ) ;
				} else {
					DG_PutVector( &Shift[ 3 ], &pos, 1 ) ;
				}
			} else {
				work->control.turn.vy = work->control.rot.vy = work->data2 ;
				dir.vy = work->data2 ; dir.vx = dir.vz = 0 ;
				//		DG_SetPos2( &c->capture->ctrl->mov, &dir ) ;
				DG_SetPos2( &c->capture->ctrl->mov, &( work->control.turn ) ) ;
				if ( c->capture->flag & CAPTURE_FRONT ) {		
					DG_PutVector( &Shift[ 0 ], &pos, 1 ) ;
				} else {
					DG_PutVector( &Shift[ 2 ], &pos, 1 ) ;
				}
			}
#endif
		}
		/* 開始位置に移動 */
		//	work->control.mov.vx = GV_NearExp8( work->control.mov.vx, pos.vx ) ;
		//	work->control.mov.vz = GV_NearExp8( work->control.mov.vz, pos.vz ) ;
		//	if ( work->ftime == 8 ) work->data ++ ;
#if 0
		work->control.step.vx = pos.vx - work->control.mov.vx ;
		work->control.step.vz = pos.vz - work->control.mov.vz ;
#endif
		work->data ++ ;
		/* ステータスセット */
//		SetStatus( PLAYER_ENEMY_PULL ) ;
		/* つかみ中だけ主観不可 */
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		LeaveSubject( work ) ;
		break ;
    case 1 :					/* つかみモーション */
		/* このときだけ主観不可 */
		SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_NO_IK ) ;

		/* 気絶から覚めによる離し */
		if ( work->capture.flag & CAPTURE_FREE ) {
			work->data = 5 ; return ;
		}

		if ( c->flag & CAPTURE_HEAD ) {
			if ( c->capture->flag & CAPTURE_FRONT ) {
				SetAction( work, MS.shared[ Mcarry_b_start_f ], 6 ) ;
			} else {
				SetAction( work, MS.shared[ Mcarry_b_start_b ], 6 ) ;
			}
		} else {
			SetAction( work, MS.shared[ Mcarry_l_start ], 6 ) ;
		}
		/* 直方体チェック開始 */
		work->control.skip_flag |= CTRL_COLLIDE_CHECK ;

		/* アジャスト */
		PullBodyAdjust( work ) ;
#if 0
		{
			FVECTOR		from, to, diff ;
			SVECTOR		rot ;

			from.vx = work->control.mov.vx ;
			from.vy = work->control.levels[ 0 ] ;
			from.vz = work->control.mov.vz ;
			to.vx = c->capture->body->objs->world.m[ 3 ][ 0 ] ;
			to.vy = c->capture->body->objs->world.m[ 3 ][ 1 ] ;
			to.vz = c->capture->body->objs->world.m[ 3 ][ 2 ] ;
			if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &to, HZX_CHK_ALL, 
									   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) & 1 ) {
				to.vy = HZX_GetFloorLevel() ;
			}
			_sceVu0SubVector( &diff, &to, &from ) ;
			GV_VecToRot( &diff, &rot ) ;
			if ( rot.vx > 2048 ) rot.vx -= 4096 ;
			else if ( rot.vx < -2048 ) rot.vx += 4096 ;
			if ( rot.vx < 0 ) {
				rot.vy = 0 ; rot.vz = 0 ;
				SetRotAdjust( work, &rot, HUMAN21_ONAKA ) ;
			}
		}
#endif
		/* 間に壁があったら離し */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->capture.capture->ctrl->mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			work->data = 5 ; 
			return ;			
		}
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->pre_mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			work->data = 5 ; 
			return ;			
		}
		/* 高さが大きく違ったら離し */
		if ( DG_FABS( work->control.mov.vy 
					 - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
			work->data = 5 ; 
			return ; 
		}
		if ( EndMotion( work ) ) {
			PL_FootPrintForce( work->foot_work, 3 ) ;
			work->data ++ ;
		}
		/* ステータスセット */
//		SetStatus( PLAYER_ENEMY_PULL ) ;
		break ;
    case 2 :					/* つかみ静止 */
#if 0
		{
			FVECTOR		diff, rot ;
			FMATRIX		m0 ;

			diff.vx = c->capture->ctrl->mov.vx - work->control.mov.vx ;
			diff.vy = c->capture->ctrl->mov.vy - work->control.mov.vy ;
			diff.vz = c->capture->ctrl->mov.vz - work->control.mov.vz ;
			GV_ItoFVector( &rot, &( work->control.rot ) ) ;
			_sceVu0UnitMatrix( &m0 ) ;
			_sceVu0RotMatrix( &m0, &m0, &rot ) ;
			_sceVu0TransposeMatrix( &m0, &m0 ) ;
			_sceVu0ApplyMatrix( &diff, &m0, &diff ) ;
			DumpVec( &diff ) ;
		}
#endif
		if ( c->flag & CAPTURE_HEAD ) {
			SetAction( work, MS.shared[ Mcarry_b ], 6 ) ;
		} else {
			SetAction( work, MS.shared[ Mcarry_l ], 6 ) ;
		}

		if ( c->flag & CAPTURE_HEAD ) {
			SVECTOR		rot = { 0, work->control.rot.vy, 0 } ;
			/* 壁際でカメラを上げる */
			if ( Status( PLAYER_WATCH ) &&
				 CheckMovRotLenSegment( work->control.hzx_id, &work->camera, 
									    &DG_ZeroVector, &rot, 650.0F, HZX_CHK_ALL,
									    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
				work->camera.vy += 80.0F ;
			}
		}

		/* 気絶から覚めによる離し */
		if ( work->capture.flag & CAPTURE_FREE ) {
			work->data = 5 ; return ;
		}

		/* アジャスト */
		PullBodyAdjust( work ) ;

		/* 間に壁があったら離し */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->capture.capture->ctrl->mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			printf( "raiden/attack.c : kabe hanasi1\n" ) ;
			work->data = 5 ; 
			return ;			
		}
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->pre_mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			printf( "raiden/attack.c : kabe hanasi2\n" ) ;
			work->data = 5 ; 
			return ;			
		}
		/* 高さが大きく違ったら離し */
		if ( DG_FABS( work->control.mov.vy 
					 - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
			work->data = 5 ; 
			return ; 
		}
		if ( !( work->pad->status & PL_PAD_CAPUTRE ) ) {
			work->data = 4 ;	/* 離しへ */
			return ;
		}

		/*主観移動 回転 */
		if ( PL_SubjectMove )
		  SubjectMoveTurn( work ) ;

		if ( Status( PLAYER_WATCH ) ) {
			SubjectTurn( work ) ;
		} else {
			if ( work->pad->status & PAD_UDLR ) work->data = 3 ; /* 歩きへ */
		}
		/* ステータスセット */
		SetCaptureStatus( PLAYER_ENEMY_PULL ) ;
//		GM_ReverseSlideCameraDir( work->chanl ) ;
		break ;
    case 3 :					/* つかみ歩き */
		/* 気絶から覚めによる離し */
		if ( work->capture.flag & CAPTURE_FREE ) {
			work->data = 5 ; return ;
		}

		/* アジャスト */
		PullBodyAdjust( work ) ;

		/* 間に壁があったら離し */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->capture.capture->ctrl->mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			printf( "raiden/attack.c : kabe hanasi3\n" ) ;
			work->data = 5 ; 
			return ;			
		}
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
								    &work->control.mov, 
								    &work->pre_mov,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
			printf( "raiden/attack.c : kabe hanasi4\n" ) ;
			work->data = 5 ; 
			return ;			
		}
		/* 高さが大きく違ったら離し */
		if ( DG_FABS( work->control.mov.vy 
					 - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
			work->data = 5 ; 
			return ; 
		}
		
		if ( !( work->pad->status & PL_PAD_CAPUTRE ) ) work->data = 4 ; /* 離しへ */
		else if ( !( work->pad->status & PAD_UDLR ) ||
				 Status( PLAYER_WATCH ) ) work->data = 2 ; /* 静止へ */
		if ( work->data != 3 ) {
			c->capture->flag &= ~CAPTURE_MOVE ;
			break ;
		}
		c->capture->flag |= CAPTURE_MOVE ;
		if ( c->flag & CAPTURE_HEAD ) {
			SetAction( work, MS.shared[ Mcarry_b_walk ], 6 ) ;
			//	    work->control.turn.vy += 2048 ;
			PadTo = ( PadTo + 2048 ) & 4095 ;
		} else {
			SetAction( work, MS.shared[ Mcarry_l_walk ], 6 ) ;
			PadTo = ( PadTo + 2048 ) & 4095 ;
		}	
//		GM_VctrlSetLoop( &work->vctrl ) ;
		/* 階段中は回転不可 */
//		if ( !( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ) SeekTurn( work ) ; 
		/*主観移動 回転 */
		if ( PL_SubjectMove )
		  SubjectMoveTurn( work ) ;
		else
		  SeekTurn( work ) ; 

		SetStatus( PLAYER_MOVE ) ;
		/* ステータスセット */
		SetCaptureStatus( PLAYER_ENEMY_PULL ) ;
		GM_ReverseSlideCameraDir( work->chanl ) ;
		/* 足跡 */
		PL_FootPrintAct( work->foot_work, 0, 1, 34 ) ;
		break ;
    case 4 :					/* つかみおろし */	
		if ( c->capture != NULL ) {
			if ( c->flag & CAPTURE_HEAD ) {
				SetAction( work, MS.shared[ Mcarry_b_end ], 6 ) ;
			} else {
				SetAction( work, MS.shared[ Mcarry_l_end ], 6 ) ;
			}		
			ClearCaptureTarget( work ) ;
		}
		if ( EndMotion( work ) ) {
			SetMode( work, StandStill ) ;	
		}
//		GM_ReverseSlideCameraDir( work->chanl ) ;
		break ;
    case 5 :					/* つかみ終了処理 */
		ClearCaptureTarget( work ) ;
		SetMode( work, StandStill ) ;	
		break ;
    case 6 :
		/* つかめなかった */
		if ( EndMotion( work ) || work->pad->press ) {
			SetMode( work, SquatStill ) ;
		}
    }
}

/*-------- 首絞め ----------*/

/* 首絞め開始 */
static	void	HangStart( work, time )
Work		*work ;
int		time ;
{
    TARGET		*t ;
    int			ftime ;

    ftime = work->ftime ;
    t = &( work->offense ) ;
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;

    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		work->capture_count = 0 ;
		work->post &= ~MAIL_HANG_THROUGH ;
		/* つかみ判定 */
		SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE,
						  300.0F, 450.0F, 300.0F,
						  0.0F, 0.0F, 0.0F,
						  HUMAN21_KOSHI, 0.0F, 0.0F, 300.0F, 
						  WP_HANG, HangCaptureCallBack ) ;
		return ;
    }
    if ( ftime == 1 ) {
		/* ハイテク兵だった */
		if ( work->post & MAIL_HANG_THROUGH ) {
			printf( "raiden/attack.c : ハイテク兵つかみだぜ\n" ) ;
			SetMode( work, HangRelease ) ;
			return ;
		} 
		/* つかめなかった */
		if ( !( t->damaged & TARGET_CAPTURE ) ) {
			ClearCaptureTarget( work ) ;
//			SetMode( work, StandStill ) ;	
#if 0
			SetAction( work, MS.shared[ Mhang_miss ], 6 ) ;
			work->data = 1 ;
#else
			/* 死体つかみに挑戦 */
			SetMode( work, PullBody ) ;
#endif
			return ;
		}
		LeaveSubject( work ) ;
		work->capture.capture->flag |= CAPTURE_HANG ;
		SetAction( work, MS.shared[ Mhang ], 6 ) ;
    }

	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;

	switch( work->data ) {
	case 0 :	/* つかめた */
		/* よけられた */
		if ( work->capture.flag & CAPTURE_ESCAPE ) {
			ClearCaptureTarget( work ) ;
			SetAction( work, MS.shared[ Mhang_miss ], 6 ) ;
			work->data = 1 ;	
			break ;
		}
		//SetCaptureStatus( PLAYER_ENEMY_HANG ) ;
		/* ここに吸い込み計算をいれる */
		if ( ftime == 8 ) {
			SetMode( work, HangStill ) ;
		}
		break ;
	case 1 :	/* つかめない */
		if ( EndMotion( work ) ) {
			SetMode( work, StandStill ) ;
		}
	}
}

/* つかみ静止 */
static	void	HangStill( work, time )
Work		*work ;
int		time ;
{
    int		press ;

    if ( time == 0 ) {
		SetAction( work, MS.shared[ Mhang ], 6 ) ;
		PL_FootPrintForce( work->foot_work, 3 ) ;
		work->act_name = ENEMY_HANG ;
    }	
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;


	/*主観移動 回転 */
	if ( PL_SubjectMove )
	    SubjectMoveTurn( work ) ;

	/* 壁際でカメラを下げる */
	{
		SVECTOR		rot = { 0, work->control.rot.vy, 0 } ;
		FVECTOR		shift = { 0.0F, 0.0F, 0.0F } ;
		if ( Status( PLAYER_WATCH ) ) {
			if ( CheckMovRotLenSegment( work->control.hzx_id, &work->camera, 
									    &DG_ZeroVector, &rot, 850.0F, HZX_CHK_ALL,
									    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
				shift.vz = -300.0F ;
			} else {
				shift.vz = -150.0F ;
			}
			DG_SetPos2( &work->camera, &rot ) ;
			DG_PutVector( &shift, &work->camera, 1 ) ;
		}
	}

    /* 敵の命令による離し */
    if ( work->capture.flag & CAPTURE_FREE ) {
		SetMode( work, HangRelease ) ;
		if ( !( work->capture.flag & CAPTURE_ESCAPE ) ) work->data = 1 ;
		ClearCaptureTarget( work ) ;
		return ;
    }
    press = work->pad->press ;

	/* 高さが大きく違ったら離し */
	if ( DG_FABS( work->control.mov.vy 
				  - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, HangRelease ) ;
		work->data = 1 ;
		return ; 
	}

    if ( !( work->pad->status & PL_PAD_HANG ) ) { /* 離し */
		if ( ++ work->data == 8 ) {
			ClearCaptureTarget( work ) ;
			SetMode( work, HangRelease ) ;
			work->data = 1 ;
			return ;
		}
    } else if ( press & PL_PAD_HANG ) {
		SetMode( work, HangTie ) ;
    } else if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
    } else if ( work->pad->status & PAD_UDLR ) {
		SetMode( work, HangRun ) ;
    } 

	SetCaptureStatus( PLAYER_ENEMY_HANG ) ;
	GM_ReverseSlideCameraDir( work->chanl ) ;
}

/* つかみ首絞め */
static	void	HangTie( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;

	/* 壁際でカメラを下げる */
	{
		SVECTOR		rot = { 0, work->control.rot.vy, 0 } ;
		FVECTOR		shift = { 0.0F, 0.0F, 0.0F } ;
		if ( Status( PLAYER_WATCH ) ) {
			if ( CheckMovRotLenSegment( work->control.hzx_id, &work->camera, 
									    &DG_ZeroVector, &rot, 850.0F, HZX_CHK_ALL,
									    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
				shift.vz = -300.0F ;
			} else {
				shift.vz = -150.0F ;
			}
			DG_SetPos2( &work->camera, &rot ) ;
			DG_PutVector( &shift, &work->camera, 1 ) ;
		}
	}

    if ( time == 0 ) {
		work->motion1 = -1 ;
		SetAction( work, MS.shared[ Mtie ], 6 ) ;
		work->capture.capture->flag |= CAPTURE_HANG ;
		work->act_name = ENEMY_HANG ;
		if ( ++ work->capture_count >= 10 ) {
			/* 首折りへ */
			work->capture.capture->flag |= CAPTURE_BREAK ;
			SetMode( work, HangBreak ) ;
			return ;
		}
    }	

	/* 高さが大きく違ったら離し */
	if ( DG_FABS( work->control.mov.vy 
				  - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, HangRelease ) ;
		work->data = 1 ;
		return ; 
	}

    /* 敵の命令による離し */
    if ( work->capture.flag & CAPTURE_FREE ) {
		SetMode( work, HangRelease ) ;
		if ( !( work->capture.flag & CAPTURE_ESCAPE ) ) work->data = 1 ;
		ClearCaptureTarget( work ) ;
		return ;
    }
	SetCaptureStatus( PLAYER_ENEMY_HANG ) ;
    if ( work->pad->press & PL_PAD_HANG ) {
		work->time = 0 ;
    }

    if ( EndMotion( work ) ) {
		SetMode( work, HangStill ) ;
    }
	GM_ReverseSlideCameraDir( work->chanl ) ;
}

/* つかみ首折り */
static	void	HangBreak( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		SetAction( work, MS.shared[ Mkill ], 6 ) ;
		work->act_name = ENEMY_HANG ;
    }

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;

	/* 壁際でカメラを下げる */
	{
		SVECTOR		rot = { 0, work->control.rot.vy, 0 } ;
		FVECTOR		shift = { 0.0F, 0.0F, 0.0F } ;
		if ( Status( PLAYER_WATCH ) ) {
			if ( CheckMovRotLenSegment( work->control.hzx_id, &work->camera, 
									    &DG_ZeroVector, &rot, 850.0F, HZX_CHK_ALL,
									    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
				shift.vz = -300.0F ;
			} else {
				shift.vz = -150.0F ;
			}
			DG_SetPos2( &work->camera, &rot ) ;
			DG_PutVector( &shift, &work->camera, 1 ) ;
		}
	}

	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
	//SetStatus( PLAYER_ENEMY_HANG ) ;
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY | FLAG_DONOT_CHECK_WATCH ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

    if ( EndMotion( work ) ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, StandStill ) ;
    }
	GM_ReverseSlideCameraDir( work->chanl ) ;
}

/* つかみ首絞め移動 */
static	void	HangRun( work, time )
Work		*work ;
int		time ;
{
    if ( time == 0 ) {
		work->capture.capture->flag |= CAPTURE_MOVE ;
		SetAction( work, MS.shared[ Mhang_walk ], 6 ) ;
		work->act_name = ENEMY_HANG ;
    }
	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
	SetStatus( PLAYER_MOVE ) ;
	SetCaptureStatus( PLAYER_ENEMY_HANG ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM |     
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;

	/*主観移動 回転 */
	if ( PL_SubjectMove )
	    SubjectMoveTurn( work ) ;

    /* 離し */
    if ( work->capture.flag & CAPTURE_FREE ) {
		SetMode( work, HangRelease ) ;
		if ( !( work->capture.flag & CAPTURE_ESCAPE ) ) work->data = 1 ;
		ClearCaptureTarget( work ) ;
		return ;
    }
	/* 高さが大きく違ったら離し */
	if ( DG_FABS( work->control.mov.vy 
				  - work->capture.capture->ctrl->mov.vy ) > 2000.0F ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, HangRelease ) ;
		work->data = 1 ;
		return ; 
	}
	/* 間に壁があったら離し */
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
							   &work->control.mov, 
							   &work->capture.capture->ctrl->mov,
							   HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, HangRelease ) ;
		work->data = 1 ;
		return ;		
	}
	if ( HZX_OnlineHazardCheck( work->control.hzx_id, 
							    &work->control.mov, 
							    &work->pre_mov,
							    HZX_CHK_SEGMENT, HZX_SEG_NO_ENEMY, 0 ) ) {
		ClearCaptureTarget( work ) ;
		SetMode( work, HangRelease ) ;
		work->data = 1 ;		
		return ;			
	}

    /* 静止へ */
    if ( Status( PLAYER_WATCH ) ||
		!( work->pad->status & PAD_UDLR ) ||
		!( work->pad->status & PL_PAD_HANG ) ) {	
		work->capture.capture->flag &= ~CAPTURE_MOVE ;
		SetMode( work, HangStill ) ;
		return ;
    }
    /* 首絞めへ */
    if ( work->pad->press & PL_PAD_HANG ) {
		work->capture.capture->flag &= ~CAPTURE_MOVE ;
		SetMode( work, HangTie ) ;
		return ;
    }
    PadTo = ( PadTo + 2048 ) & 4095 ;
    SeekTurn( work ) ; 
	GM_ReverseSlideCameraDir( work->chanl ) ;
	/* 足跡 */
	PL_FootPrintAct( work->foot_work, 0, 1, 34 ) ;
}

/* つかみ首絞め逃げられ */
/* ここにくる前に、work->dataが１にセットされているときは
   自分から離した場合 */
static	void	HangRelease( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;

	/* 主観のときあんまり下を向けなくする */
	GM_SubjectVMaxTmp[ 0 ] = 160 ;

	if ( time == 0 ) {
		work->act_name = 8741186 ;	/* HangRelease */
		if ( work->post & MAIL_HANG_THROUGH ) {
			SetAction( work, non_hang_through, 6 ) ;
		} else if ( work->data == 1 ) {
			SetAction( work, MS.shared[ Mhang_release ], 6 ) ;
		} else {
			SetAction( work, MS.shared[ Mhang_escape ], 6 ) ;
		}
		work->post &= ~MAIL_HANG_THROUGH ;
		work->act_name = ENEMY_HANG ;
	}

	/* 壁際でカメラを下げる */
	{
		SVECTOR		rot = { 0, work->control.rot.vy, 0 } ;
		FVECTOR		shift = { 0.0F, 0.0F, 0.0F } ;
		if ( Status( PLAYER_WATCH ) ) {
			if ( CheckMovRotLenSegment( work->control.hzx_id, &work->camera, 
									    &DG_ZeroVector, &rot, 850.0F, HZX_CHK_ALL,
									    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
				shift.vz = -300.0F ;
			} else {
				shift.vz = -150.0F ;
			}
			DG_SetPos2( &work->camera, &rot ) ;
			DG_PutVector( &shift, &work->camera, 1 ) ;
		}
	}

	if ( EndMotion( work ) ) {
		SetMode( work, StandStill ) ;
	}
	GM_ReverseSlideCameraDir( work->chanl ) ;
}

/* 投げ */
static	void	Throw( work, time )
Work		*work ;
int		time ;
{
    TARGET		*t ;
    int			ftime ;

    ftime = work->ftime ;
    t = &( work->offense ) ;
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;

    if ( time == 0 ) {
		ChangeStance( work, STAND ) ;
		/* つかみ判定 */
		SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE,
						  100.0F, 650.0F, 100.0F,
						  0.0F, 0.0F, 0.0F,
						  HUMAN21_MIGI_TE, 0.0F, 0.0F, 0.0F, 
						  WP_THROW, ThrowCaptureCallBack ) ;
		return ;
    }
    if ( ftime == 1 ) {
		/* つかめなかった */
		if ( !( t->damaged & TARGET_CAPTURE ) ) {
			/* 左で再挑戦 */
			SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE,
							 100.0F, 650.0F, 100.0F,
							 0.0F, 0.0F, 0.0F,
							 HUMAN21_HIDARI_TE, 0.0F, 0.0F, 0.0F, 
							 WP_THROW, ThrowCaptureCallBack ) ;
			return ;
		} else {
			/* つかめた */
			work->capture.capture->flag |= CAPTURE_THROW ;
			SetAction( work, MS.shared[ Mthrow_r ], 6 ) ;
			work->data = 1 ;
		}
    }
    if ( ftime == 2 ) {
		if ( work->data == 0 ) {
			/* やっぱりつかめなかった */
			if ( !( t->damaged & TARGET_CAPTURE ) ) {
				ClearCaptureTarget( work ) ;
//				SetModeName( work, StandRun, "StandRun" ) ;
				/* 死体つかみに挑戦 */
				SetMode( work, PullBody ) ;
				return ;
			} else {
				/* やっとつかめた */
				work->capture.capture->flag |= CAPTURE_THROW ;
				SetAction( work, MS.shared[ Mthrow_l ], 6 ) ;
				work->data = 1 ;
			}
		}
    }

    /* ここに吸い込み計算をいれる */

    if ( EndMotion( work ) ) {
		ClearCaptureTarget( work ) ;
//		SetMode( work, StandStill ) ;
//		SetMode( work, StandRun ) ;
		if ( work->pad->status & PAD_UDLR ) {
			SetAction( work, MS.change[ Mwalk ], 6 ) ;
			SetModeName( work, StandRun, "StandRun" ) ;
		} else {
			SetMode( work, StandStill ) ;
		}
    }    
}

/*-------------------------------------------------------------------------*/

/*-------- コムボ ----------*/

/* コムボ用ホーミング */
static	void	ComboHoming( work )
Work		*work ;
{
    HOMING_TRG	*homing ;
    CONTROL	*ctrl ;
    FVECTOR	pos, trg ;
    SVECTOR	adj ;

    ctrl = &( work->control ) ;
    homing = GM_GetHomingTrg( &( ctrl->mov ), ctrl->rot.vy, 
							 ( int )HOMING_LEN, 
							 HOMING_RANGE, ctrl->hzx_id ) ;    
    if ( homing == NULL ) return ;
    if ( CheckHazardBetweenItoHoming( work, homing ) == NULL ) return ;
	
    MatToVec( &( work->body.objs->objs[ 2 ].world ), &pos ) ;
	if ( homing->body != NULL ) {
		MatToVec( &( homing->body->objs->objs[ 2 ].world ), &trg ) ;
	} else {
		MatToVec( homing->world, &trg ) ;
	}
    GM_TrgToAdjRot( &pos, ctrl->rot.vy, &trg, &adj ) ;
    ctrl->turn.vy = ctrl->rot.vy + adj.vy ;    
}

/* コムボ用コールバック */
static	void	ComboCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;
    
    /* 間に壁がある時はヒットしない */
    if ( !( def->class & TARGET_HAZARD ) && 
		CheckHazardBetweenItoTarget( work, def ) ) {
		if ( def->weapon_type == off->weapon_type ) {
			def->damaged &= ~TARGET_POWER ;
		}
		def->weapon_type &= ~( off->weapon_type ) ;
		return ;
    }
	/* 当たったら振動 */
	GM_VctrlResetSkip( &work->vctrl ) ;
}

/* コムボ本体 */
static	void	Combo( Work *work, int time )
{
    static FVECTOR	Force = { 0.0F, 0.0F, -100.0F } ;
    static FVECTOR	Force2 = { -100.0F, 0.0F, -100.0F } ;
    int				ftime, faint, wt, mtime, ptime, mode ;
	int				arm_attack = 0, chktime, objNo1, objNo2 ;
	long64			weapon_type ;
    TARGET			*t ;
    POWER_TARGET	*p ;
    FVECTOR			v, force ;
	float			len ;

    ftime = work->ftime ;

    if ( BP_IsPAL()==TRUE )
	   ftime = ( int )( ( float )ftime * 60.0F / 50.0F ) ; /* モーションのフレームに変換 */

    /* フラグ設定 */
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON |
	     FLAG_CANNOT_CHANGE_ITEM   | 
	     FLAG_CANNOT_PEEP          |
	     FLAG_NO_HEADTOENEMY       |
	     FLAG_NO_TOUCH_DAMAGE ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_COMBO ) ;

	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ){
	    SetFlag( FLAG_NO_STEP ) ;
	}

    if ( time == 0 ) {
		work->act_name = COMBO ;
		ComboHoming( work ) ;
		work->motion1 = -1 ;
		if ( work->weapon == WP_m4 || work->weapon == WP_Aks ||
			 work->weapon == WP_Rgb6 ) {
			SetAction( work, m4a_combo, 6 ) ;
			SetArmAction( work, AMm4a_combo ) ;
		} else if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			SetAction( work, MS.shared[ Mcombo ], 6 ) ;
			SetArmAction( work, AMnkd_combo ) ;
		} else {
			SetAction( work, MS.shared[ Mcombo ], 6 ) ;
			SetArmAction( work, AMcombo ) ;
		}
		SetWeaponCamera( work ) ;
//		work->data2 = EnableShootBullet( work ) ;

		work->idata = CheckMovRotLenSegment( work->control.hzx_id, &work->control.mov,
											&DG_ZeroVector, &work->control.rot, 
											1200.0F, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
											HZX_FLOOR_NO_PLAYER ) ;
		if ( work->idata ) {
			work->fdata = GV_VecLen3F2( &work->control.mov, &ResultPoint ) ;
			/* 後ろにも壁があったらやめる */
			if ( CheckMovRotLenSegment( work->control.hzx_id, &work->control.mov,
									    &DG_ZeroVector, &work->control.rot, 
									    -650.0F, HZX_CHK_ALL, HZX_SEG_NO_PLAYER,
									    HZX_FLOOR_NO_PLAYER ) ) {
				work->idata = 0 ;
			}
		}
		GM_VctrlSetSkip( &work->vctrl ) ;
		work->sv.vx = 0 ;
    } 
	if ( work->idata ) {
		PL_GetModelLength( work, &len, NULL ) ;
		len += 50.0F ;
		if ( len > work->fdata ) {
			DG_SetPos2( &DG_ZeroVector, &work->control.rot ) ;
			GV_SetVec3( &v, 0.0F, 0.0F, work->fdata - len ) ;
			DG_RotVector( &v, &v, 1 ) ;
			AdjustXZ( work, &v ) ;
		}
	}
#if 0	
	/* ＳＥがなっちまうから切っておく */
    if ( ftime == 4 && work->data2 != 0 ) {
		SetMode( work, StandStill ) ;
		return ;	
    }
#endif	
	mtime = PL_MotionTime( work ) ;
	ptime = work->sv.vx ;

	/* はだか特殊 */
#if 0
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) && EndMotion( work ) ) {
		SetMode( work, StandStill ) ;
		GM_VctrlStopVibration( &work->vctrl ) ;
		return ;		
	}
#endif

    /* 連続判定 */
#if 0
	chktime = GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ? 24 : 16 ;
#else
	chktime = 16 ;
#endif
    if ( ptime < chktime && mtime >= chktime ) {
		if ( !( work->data & PL_PAD_PUNCH ) ) {
#if 0
			//SetArmAction( work, AMstand ) ;
			//UnsetWeaponCamera( work ) ;
            if ( !GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
				SetMode( work, StandStill ) ;
				GM_VctrlStopVibration( &work->vctrl ) ;
				return ;
			}
		} else if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
			SetMode( work, Combo ) ;
			return ;
#else
			SetMode( work, StandStill ) ;
			GM_VctrlStopVibration( &work->vctrl ) ;
			return ;
#endif
		}
		work->data &= ~PL_PAD_PUNCH ;
    }
    if ( ptime < 29 && mtime >= 29 ) {
		/* 2500以上の高さがないとキックはでない */
		if ( !( work->data & PL_PAD_PUNCH ) || !CheckCeil2( work, 2500.0F ) ||
			 GM_CheckPlayerStatusEX( I64(0), PLAYER2_NO_KICK ) ) {
//			SetArmAction( work, AMstand ) ;
//			UnsetWeaponCamera( work ) ;
			SetMode( work, StandStill ) ;
            GM_VctrlStopVibration( &work->vctrl ) ;
			return ;
		}
		work->data &= ~PL_PAD_PUNCH ;
    }
	
    /* ダメージ */

	mode = -1 ;
	if ( ptime < 9 && mtime >= 9 ) mode = 0 ;
	else if ( ptime < 23 && mtime >= 23 ) mode = 1 ;
	else if ( ptime < 46 && mtime >= 46 ) mode = 2 ;
	else if ( !Status( PLAYER_SNAKE ) && ptime < 59 && mtime >= 59 ) mode = 3 ;
	else if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) && ptime < 19 && mtime >= 19 ) mode = 4 ;

	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
		if ( mode == 1 ) mode = -1 ;
		else if ( mode == 4 ) mode = 1 ;
	}
	
	if ( mode >= 0 ) {
		t = &( work->offense ) ;
		p = &( work->attack ) ;
		v.vx = v.vy = v.vz = 350.0F ;
		GM_SetTarget( t, TARGET_OFFENSE, 0, 
					 ENEMY_SIDE, &v, &DG_ZeroVector ) ;
		DG_SetPos( &work->body.objs->world ) ;
		switch ( mode ) {
		case 0 :				/* 左手（ライデンは右） */
			objNo1 = Status( PLAYER_SNAKE ) ? HUMAN21_HIDARI_TE : HUMAN21_MIGI_TE ;
			objNo2 = Status( PLAYER_SNAKE ) ? HUMAN21_HIDARI_KATA : HUMAN21_MIGI_KATA ;
			weapon_type = Status( PLAYER_SNAKE ) ? 
				( WP_PUNCHL | WP_PLAYER ) : ( WP_PUNCHL | WP_PLAYER ) ;
			faint = 1 ;

			if ( !Status( PLAYER_SNAKE ) ) {
				wt = PL_WeaponType( work ) ;
				if ( wt & WP_TYPE_PUNCH_S ) faint = 2 ;
				else if ( wt & WP_TYPE_PUNCH_M ) faint = 3 ;
				else if ( wt & WP_TYPE_PUNCH_L ) faint = 7 ;
			}

			DG_RotVector( &Force, &force, 1 ) ;
			if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
				PL_SetArmAttack( WP_PUNCHL | WP_PLAYER, &force, 
								objNo2, objNo1,
								faint, 0, 950.0F ) ;
				arm_attack = 1 ;
			} else {
				GM_SetTargetWeaponType( t, weapon_type ) ;
				GM_MoveTarget3( t, &( work->body.objs->objs[ objNo1 ].world ) ) ;
			}
			break ;
		case 1 :				/* 右手（ライデンは左手） */
			/* はだかは右手 */
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
				objNo1 = HUMAN21_MIGI_TE ;
				objNo2 = HUMAN21_MIGI_KATA ;
				weapon_type = ( WP_PUNCHR | WP_PLAYER ) ;
			} else {
				objNo1 = Status( PLAYER_SNAKE ) ? HUMAN21_MIGI_TE : HUMAN21_HIDARI_TE ;
				objNo2 = Status( PLAYER_SNAKE ) ? HUMAN21_MIGI_KATA : HUMAN21_HIDARI_KATA ;
				weapon_type = Status( PLAYER_SNAKE ) ? 
					( WP_PUNCHR | WP_PLAYER ) : ( WP_PUNCHL | WP_PLAYER ) ;
			}
			faint = 1 ;

			if ( Status( PLAYER_SNAKE ) || 
				 work->weapon == WP_Rgb6 || work->weapon == WP_m4 || work->weapon == WP_Aks ) {
				wt = PL_WeaponType( work ) ;
				if ( wt & WP_TYPE_PUNCH_S ) faint = 2 ;
				else if ( wt & WP_TYPE_PUNCH_M ) faint = 3 ;
				else if ( wt & WP_TYPE_PUNCH_L ) faint = 7 ;
			}

			DG_RotVector( &Force, &force, 1 ) ;
			if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
				PL_SetArmAttack( WP_PUNCHR | WP_PLAYER, &force, 
								objNo2, objNo1,
								faint, 0, 950.0F ) ;
				arm_attack = 1 ;
			} else {
				GM_SetTargetWeaponType( t, WP_PUNCHR | WP_PLAYER ) ;
				GM_MoveTarget3( t, &( work->body.objs->objs[ objNo1 ].world ) ) ;
			}
			break ;
		case 2 :				/* 左膝 */
			if ( Status( PLAYER_SNAKE ) ) {
				faint = 2 ;
				weapon_type = WP_KICK | WP_PLAYER ;
			} else {
				faint = 1 ;
				weapon_type = WP_KICK | WP_PLAYER | WP_KICK1 ;
			}
			GM_SetTargetWeaponType( t, weapon_type ) ;
			v.vx = v.vz = 750.0F ; v.vy = 750.0F ;
			GM_SetTargetSize( t, &v ) ;
			GM_MoveTarget3( t, &( work->body.objs->objs[ HUMAN21_HIDARI_ASHI2 ].world ) ) ;
			DG_RotVector( &Force2, &force, 1 ) ;
			break ;
		case 3 :				/* 右足（ライデンのみ） */
		default :
			faint = 2 ;
			GM_SetTargetWeaponType( t, WP_KICK | WP_PLAYER ) ;
			v.vx = v.vz = 750.0F ; v.vy = 750.0F ;
			GM_SetTargetSize( t, &v ) ;
			GM_MoveTarget3( t, &( work->body.objs->objs[ HUMAN21_MIGI_ASHI2 ].world ) ) ;
			DG_RotVector( &Force2, &force, 1 ) ;
			break ;			
		}
		if ( arm_attack == 0 ) {
			GM_SetPowerTarget( t, p, POWER_CONST, 255, faint, 0, &v ) ;
			GM_SetTargetCallBack( t, ComboCallBack, ( void * )work ) ;
			GM_PutTarget( t ) ;
#ifdef DEBUG_MODE			
			if ( PlayerDebugMenuStatus & PDMS_TARGETVIEW ) {
				NewTargetView2( t, 32, 232, 32 ) ;
			}
#endif
		}
		GM_VctrlSetSkip( &work->vctrl ) ;
	}

    work->data |= work->pad->press ;

	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		FVECTOR		to = { 0.0F, 0.0F, 10.0F } ;
		FVECTOR		from ;

		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		work->arm_trigger |= ARM_IS_COMBO ;
		PL_ObjPos( work, HUMAN21_ATAMA, &from ) ;
		DG_SetPos( PL_ObjWorld( work, HUMAN21_ATAMA ) ) ;
		DG_PutVector( &to, &to, 1 ) ;
		work->camdir.vy = GV_VecDir2FromTo( &from, &to ) ;
	}

	if ( EndMotion( work ) ) {
//		SetArmAction( work, AMstand ) ;
//		UnsetWeaponCamera( work ) ;
		SetMode( work, StandStill ) ;
        return ;
	}

	if ( Status( PLAYER_SNAKE ) ) {
		//if ( mtime > 44 && mtime < 53 ) work->vanime_flag = 1 ;
		if ( ( mtime > 5 && mtime < 10 ) ||	 
			 ( mtime > 15 && mtime < 22 ) ||
			 ( mtime > 41 && mtime < 59 ) ) {
			work->actblur_flag = 1 ;
		}
	} else {
		if ( ( mtime > 5 && mtime < 10 ) ||	 
			 ( mtime > 15 && mtime < 20 ) ||
			 ( mtime > 41 && mtime < 48 ) ||
			 ( mtime > 55 && mtime < 62 ) ) {
			work->actblur_flag = 1 ;
		}
		if ( mtime > 40 ) {
			GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;
			UnsetWeaponCamera( work ) ;
		}
	}
	work->sv.vx = mtime ;
}

/* ニキータパンチ */
static	void	NikitaStrike( Work *work, int time )
{
    static FVECTOR	Force = { -100.0F, 0.0F, -100.0F } ;
    int				mtime ;
    TARGET			*t ;
    POWER_TARGET	*p ;
    FVECTOR			v, force ;

    /* フラグ設定 */
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON |
	     FLAG_CANNOT_CHANGE_ITEM   | 
	     FLAG_CANNOT_PEEP          |
	     FLAG_NO_HEADTOENEMY       |
	     FLAG_NO_TOUCH_DAMAGE ) ;
	GM_SetPlayerStatusEX( I64(0), PLAYER2_COMBO ) ;

	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ){
	    SetFlag( FLAG_NO_STEP ) ;
	}

    if ( time == 0 ) {
		work->act_name = COMBO ;
		ComboHoming( work ) ;
		work->motion1 = -1 ;
		SetAction( work, nkt_fire_strike, 6 ) ;
		SetArmAction( work, AMnkt_strike ) ;
		//SetWeaponCamera( work ) ;
		GM_VctrlSetSkip( &work->vctrl ) ;
		work->camdir.vx = 0 ;
    } 

	mtime = PL_MotionTime( work ) ;

	if ( mtime >= 40 && mtime < 50 ) {
		t = &( work->offense ) ;
		p = &( work->attack ) ;
		v.vx = v.vz = 750.0F ; v.vy = 250.0F ;
		GM_SetTarget( t, TARGET_OFFENSE, 0, 
					  ENEMY_SIDE, &v, &DG_ZeroVector ) ;
		DG_SetPos( &work->body.objs->world ) ;
		GM_SetTargetWeaponType( t, WP_KICK | WP_PLAYER ) ;
		GM_MoveTarget3( t, &( work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ) ;
		DG_RotVector( &Force, &force, 1 ) ;
		GM_SetPowerTarget( t, p, POWER_CONST, 255, 10, 0, &v ) ;
		GM_SetTargetCallBack( t, ComboCallBack, ( void * )work ) ;
		GM_PutTarget( t ) ;
#ifdef DEBUG_MODE			
		if ( PlayerDebugMenuStatus & PDMS_TARGETVIEW ) {
			NewTargetView2( t, 32, 232, 32 ) ;
		}
#endif
	}

	if ( mtime > 88 ) UnsetWeaponCamera( work ) ;
	else if ( mtime >= 44 ) SetWeaponCamera( work ) ;

	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		FVECTOR		to = { 0.0F, 0.0F, 10.0F } ;
		FVECTOR		from ;

		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
		//work->arm_trigger |= ARM_IS_COMBO ;
		PL_ObjPos( work, HUMAN21_ATAMA, &from ) ;
		if ( mtime < 44 ) {
			DG_SetPos( PL_ObjWorld( work, HUMAN21_ATAMA ) ) ;
			DG_PutVector( &to, &to, 1 ) ;
			//PL_ObjPos( work, HUMAN21_MIGI_UDE1, &to ) ;
		} else {
			PL_ObjPos( work, HUMAN21_MIGI_TE, &to ) ;
		}
		work->camdir.vy = GV_VecDir2FromTo( &from, &to ) ;
		if ( mtime > 52 ) work->camdir.vy = work->control.rot.vy ;
		else if ( mtime > 46 ) work->camdir.vy += 256 ;
		//else work->camdir.vy -= 128 ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, StandStill ) ;
	}
}

/*------------------------------------------------------------*/

/* 武器 */

/* ホーミング */
static	void	Homing( Work *work, u_char first ) 
{
	CONTROL	*ctrl ;
	OBJECT	*arm ;
	FVECTOR	trg, pos ;
//	FVECTOR	dif ;
	SVECTOR	tmp, tmp2, limit, rot ;
	int		gr, gr2 ;
	HOMING_TRG	*homing ;

	/* 押している間だけ */
	if ( !( work->pad->status & PL_PAD_LOCKON ) && first == 0 ) {
        if (/* 主観移動のときは入力のみが解除の条件 */
			( PL_SubjectMove && ( work->pad->press & PAD_UDLR || work->pad->analog_input & (GV_PAD_ANALOG_L_USE|GV_PAD_ANALOG_R_USE))) ||
			/* 主観移動以外のとき */
			( PadTo >= 0 && 
			 ( !Flag( FLAG_BEHIND_ATTACK ) || 
			  ( Status( PLAYER_WATCH ) && work->pad->press & PAD_UDLR )) ) ) {
			work->homing = NULL ;
		}        
        if ( work->homing == NULL ) {
			UnsetFlag( FLAG_HOMING ) ;
			return ;
		}
	} else if ( !Flag( FLAG_BEHIND_ATTACK ) && first != 0 && PadTo >= 0 ) {
		work->homing = NULL ;
		UnsetFlag( FLAG_HOMING ) ;
		return ;
	} 

	/* 覗き込み時、禁止 */
	if ( Flag( FLAG_PEEPING ) ) {
		work->homing = NULL ;
		UnsetFlag( FLAG_HOMING ) ;
		return ;		
	}

	ctrl = &( work->control ) ;
	gr = GetGRot( work, 500.0F ) ;
	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		GV_SetVec3( &rot, work->camdir.vx, work->camdir.vy, 0 ) ;
	} else {
		GV_SetVec3( &rot, gr, ctrl->rot.vy, 0 ) ;
	}
	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		limit.vx = ( first ) ? 512 : 768 ; 
		limit.vy = ( first ) ? 512 : 1024 ;
	} else {
		limit.vx = 768 ;
		limit.vy = ( work->action == StandRun ) ? 768 : -1 ;
		if ( limit.vy == -1 ) {
			limit.vy = ( Flag( FLAG_BEHIND_ATTACK ) ) ? 768 : -1 ;
		}
	}

	homing = work->homing ;
	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
//		PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
		arm = GM_PlayerArmBody ;
        GV_MatToVec( &arm->objs->objs[ HUMAN21_MIGI_TE ].world, &pos ) ;
	} else {
		DG_COPY_VEC( &pos, &work->control.mov ) ;
		pos.vy = PL_ObjHeight( work, HUMAN21_MIGI_TE ) ;
	}
	
	if ( homing == NULL || first || work->pad->press & PL_PAD_LOCKON ) {
		homing = GM_GetHomingTrgInSight( &pos, &rot, &limit, homing, 
										 work->homing_near, ctrl->hzx_id ) ;
		if ( work->homing != NULL &&
			( homing == NULL || work->homing == homing ) ) {
			homing = work->homing ;
			work->homing_near = 1 - work->homing_near ;
			homing = GM_GetHomingTrgInSight( &pos, &rot, &limit, homing, 
											 work->homing_near, ctrl->hzx_id ) ;
		}
	} else if ( homing != NULL ) {
		if ( !GM_CheckHomingEnable( homing, &pos, &rot, &limit, 
								    ctrl->hzx_id ) ) {
//			homing = NULL ;
            homing = GM_GetHomingTrgInSight( &pos, &rot, &limit, NULL, 
											 work->homing_near, ctrl->hzx_id ) ;
            if ( homing == NULL ) {
				work->homing_near = 1 - work->homing_near ;
				homing = GM_GetHomingTrgInSight( &pos, &rot, &limit, NULL, 
												 work->homing_near, ctrl->hzx_id ) ;
			}
        }
	}
	if ( homing == NULL ) {
		work->homing = NULL ;
		return ;
	}
	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		arm = GM_PlayerArmBody ;
		MatToVec( &( arm->objs->objs[ HUMAN21_MIGI_TE ].world ), &pos ) ;
		MatToVec( homing->world, &trg ) ;
		GM_TrgToAdjRot( &pos, rot.vy, &trg, &tmp ) ;
		if ( Status( PLAYER_SNAKE ) && !(( (WeaponSet *)work->wp_set )->type & WP_TYPE_CONSECUTIVE) )
		    tmp.vx -= 32 ;
		else
		    tmp.vx += 6 ;
		tmp.vy -= 0 ;
//		work->camdir.vx = GV_NearExp4P( work->camdir.vx, tmp.vx ) ;
		work->camdir.vx = tmp.vx ;
		gr2 = ( work->g_rot == 0 ) ? gr : work->g_rot ;
		if ( work->camdir.vx < GM_SubjectVMax[ 1 ] + gr2 ) 
			work->camdir.vx = GM_SubjectVMax[ 1 ] + gr2 ;
		else if ( work->camdir.vx > GM_SubjectVMax[ 0 ] + gr2 ) 
			work->camdir.vx = GM_SubjectVMax[ 0 ] + gr2 ;

		GM_CameraDir.vx = work->camdir.vx ;
		if ( Flag( FLAG_SUBJECT_HORIZON_LIMIT ) ) {
			GM_CameraDir.vy = work->camdir.vy = rot.vy + tmp.vy ;
		} else {
			ctrl->turn.vy = rot.vy + tmp.vy ;
			ctrl->rot.vy = ctrl->turn.vy ;
		}

	} else {
		MatToVec( homing->world, &trg ) ;
		GM_TrgToAdjRot( &pos, rot.vy, &trg, &tmp ) ;
		/* 決めうち処理 */
		if ( work->action == StandRun ) {
			if ( Status( PLAYER_SNAKE ) && !(( (WeaponSet *)work->wp_set )->type & WP_TYPE_CONSECUTIVE) ) tmp.vx -= 48 ;
			else						  tmp.vx -= 0 ;
			tmp.vy -= 0 ;
			tmp2 = tmp ;
			tmp2.vy = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ATAMA ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_MIGI_UDE1 ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_HIDARI_UDE1 ) ;
			tmp2 = tmp ;
			tmp2.vx = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ONAKA ) ;
		} else {
			/* 微調整 */
			if ( Status( PLAYER_SNAKE ) && !(( (WeaponSet *)work->wp_set )->type & WP_TYPE_CONSECUTIVE) ) {
				tmp.vx -= 48 ;
			} else {
				if ( WeaponType( work ) & WP_TYPE_PRESSURE ) {
					tmp.vx -= 0 ;
				} else {
					tmp.vx += 12 ;
				}
			}
			tmp.vy -= 0 ;
			tmp2 = tmp ;
			tmp2.vy = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ATAMA ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_MIGI_UDE1 ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_HIDARI_UDE1 ) ;
			ctrl->turn.vy = rot.vy + tmp.vy ;
			ctrl->rot.vy = GV_NearExp8P( ctrl->rot.vy, ctrl->turn.vy ) ;
		}
	}
	SetFlag( FLAG_HOMING ) ;
	work->homing = homing ;
	if ( homing != NULL ) homing->status |= HOMING_YOU ;
}

#if 0
static	void	HomingOld( work, first )
Work		*work ;
u_char		first ;
{
	static int	HomingDir[] = { RIGHT_HOMING, LEFT_HOMING } ;
	CONTROL	*ctrl ;
	FVECTOR	trg, pos, diff ;
	SVECTOR	tmp, tmp2 ;
	HOMING_TRG	*homing, *homing2 ;
	float	len ; 
	int		dir, ry ;

	ctrl = &( work->control ) ;
	ry = ctrl->rot.vy ;
	/* 押している間だけ */
	if ( !( work->pad->status & PL_PAD_LOCKON ) && first == 0 ) {
		return ;
	}
	homing = work->homing ;
	MatToVec( &( work->body.objs->objs[ HUMAN21_MIGI_TE ].world ), &pos ) ;
	if ( homing == NULL ) {
		homing = GM_GetHomingTrg( &pos, ry, 
								 ( int )HOMING_LEN, 
								 HOMING_RANGE, ctrl->hzx_id ) ;
	} else {
		/* 現在のホーミング先が、まだホーミング範囲にいるかチェック */
		MatToVec( homing->world, &trg ) ;
		_sceVu0SubVector( &diff, &trg, &pos ) ;
		len = GV_VecLen3F( &diff ) ;
		dir = GV_VecDir2( &diff ) ;
		/* すでに範囲外のときは、ホーミングしなおし */
		if ( len > HOMING_LEN || ( homing->status & HOMING_SKIP ) ||
			( GV_DiffDirAbs( dir, ry ) > HOMING_RANGE ) ) {
			homing = GM_GetHomingTrg( &pos, ry, 
									 ( int )HOMING_LEN, 
									 HOMING_RANGE, ctrl->hzx_id ) ;	    
		} else if ( work->pad->press & PL_PAD_LOCKON ) {
			/* ロックオンボタンが押されたらホーミング先変更 */
			dir = HomingDir[ work->homing_near ] ;
			homing2 = GM_GetHomingNearTrg( &pos, ry, 
										  ( int )HOMING_LEN, 
										  HOMING_RANGE, homing,
										  dir, ctrl->hzx_id ) ;
			if ( homing2 == NULL || homing2 == homing ) {
				dir = HomingDir[ 1 - work->homing_near ] ;
				work->homing_near = 1 - work->homing_near ;
				homing2 = GM_GetHomingNearTrg( &pos, ry, 
											  ( int )HOMING_LEN, 
											  HOMING_RANGE, homing,
											  dir, ctrl->hzx_id ) ;
			}
			if ( homing2 != NULL ) {
				homing = homing2 ;
			}
		}
	}

	if ( homing == NULL ) {
		work->homing = NULL ;
		return ;
	}
	if ( Status( PLAYER_WATCH ) || PL_SubjectMove ) {
		MatToVec( &( work->arm->objs->objs[ HUMAN21_MIGI_TE ].world ), &pos ) ;
		MatToVec( homing->world, &trg ) ;
		GM_TrgToAdjRot( &pos, ry, &trg, &tmp ) ;
		tmp.vx -= 16 ;
		tmp.vy -= 0 ;
		ctrl->turn.vy = ry + tmp.vy ;
#ifdef CAMERA_TURN
		work->camdir.vx = GV_NearExp4P( work->camdir.vx, tmp.vx ) ;
		if ( work->camdir.vx < GM_SubjectVMax[ 1 ] + work->g_rot ) 
			work->camdir.vx = GM_SubjectVMax[ 1 ] + work->g_rot ;
		else if ( work->camdir.vx > GM_SubjectVMax[ 0 ] ) 
			work->camdir.vx = GM_SubjectVMax[ 0 ] + work->g_rot ;
#else
		ctrl->turn.vx = GV_NearExp4P( work->turn.vx, tmp.vx ) ;
		if ( ctrl->turn.vx < GM_SubjectVMax[ 1 ] + work->g_rot ) 
			ctrl->turn.vx = GM_SubjectVMax[ 1 ] + work->g_rot ;
		else if ( ctrl->turn.vx > GM_SubjectVMax[ 0 ] + work->g_rot ) 
			ctrl->turn.vx = GM_SubjectVMax[ 0 ] + work->g_rot ;
#endif
	} else {
		MatToVec( homing->world, &trg ) ;
		GM_TrgToAdjRot( &pos, ry, &trg, &tmp ) ;
		/* 決めうち処理 */
		if ( work->action == StandRun ) {
			tmp.vx -= 0 ;
			tmp.vy -= 0 ;
			tmp2 = tmp ;
			tmp2.vy = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ATAMA ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_MIGI_UDE1 ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_HIDARI_UDE1 ) ;
			tmp2 = tmp ;
			tmp2.vx = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ONAKA ) ;
		} else {
			/* 微調整 */
			if ( Status( PLAYER_SNAKE ) ) tmp.vx -= 0 ;
			else tmp.vx -= 256 ;
			tmp.vy -= 0 ;
			tmp2 = tmp ;
			tmp2.vy = 0 ;
			SetRotAdjust( work, &tmp2, HUMAN21_ATAMA ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_MIGI_UDE1 ) ;
			SetRotAdjust( work, &tmp2, HUMAN21_HIDARI_UDE1 ) ;
			ctrl->turn.vy = ry + tmp.vy ;
		}
		SetFlag( FLAG_HOMING ) ;
	}
	work->homing = homing ;
}
#endif



/* 弾撃ち */
static	void	ShootBullet( Work *work, int time )
{
	int		trg, ftime, adj, armadj ;
	int		c, wtype, press, mtime, chktime ;
	int		pmask, mask ;
//	static 	u_char	PrevPress, First ;
//	static	int	   	enable_count ;
#define	PrevPress		work->sv2.vx 
#define	First			work->sv2.vy
#define	enable_count	work->sv2.vz 

	ftime = work->ftime2 ;
	SetStatus( PLAYER_HOLD ) ;
	SetFlag( FLAG_NO_CAUTION | FLAG_SUBJECT_ARM_ADJUST ) ;
	adj = ( work->stance == GROUND ) ? Mready_crouch : 0 ;
	armadj = ( work->stance == GROUND ) ? AM_CROUCH_SHIFT : 0 ;

	wtype = ( ( WeaponSet * )work->wp_set )->type ;
	press = work->pad->pressure[ PL_PAD_PRESS_WEAPON ] ;

	pmask = (int) work->body.m_ctrl->mt3_ctrl[ 1 ].mask ;
	mask = ( Status( PLAYER_MOVE | PLAYER_SQUAT ) || 
			 Flag( FLAG_BEHIND_ATTACK ) ) ? 0x01ffe : 0xfffff ;

	if ( time == 0 ) {
		//GM_SeSetMode( SD_W_EQUIP02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		PL_SeSetSubject( SD_W_EQUIP02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		SetAction2( work, MS.attack[ Mready +adj ], 6, mask ) ;
		SetArmAction( work, AMready ) ;
		work->homing = NULL ;
		work->adj_rot = DG_ZeroSVector ;
		work->homing_near = RIGHT_HOMING ;
		PrevPress = press ;
		First = ( Flag( FLAG_SUBJECT_NEWPRESS ) || !Status( PLAYER_WATCH ) ) ? 1 : 0 ;
		UnsetFlag( FLAG_SUBJECT_NEWPRESS ) ;
		enable_count = 1 ;
		work->idata2 = 0 ;
		work->counter2 = GV_Time ;	/* ハンドガン用、パッドが押された時間を記憶 */
		work->sv2.pad = 1 ;			/* ハンドガン用、連射チェック */
		work->post &= ~MAIL_EMPTY_SHOT ;
		//PL_ObjPos( work, HUMAN21_HIDARI_TSUMASAKI, &work->fv2 ) ;
		work->fv2.vw = 0.0F ;
	}

	if ( work->motion2 >= 0 && pmask != mask ) {
		work->body.m_ctrl->mt3_ctrl[ 1 ].mask = mask ;
		MT_SetMotionInterp( work->body.m_ctrl, 6 * NTSC_TIME_BASE, mask ) ;
	}

	if ( mask & 1 ) SetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;
	//if ( mask & 1 ) SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
	
	if ( Status( PLAYER_CAUTION ) && !Flag( FLAG_BEHIND_ATTACK ) ) {
		SetArmAction( work, AMstand ) ;
		SetMode2( work, NULL ) ;
		work->homing = NULL ;
		return ;
	}

	SetWeaponCamera( work ) ;

	/* 初期壁チェック */
	if ( ftime == 2 && EnableShootBullet( work ) ) {
		work->data3 = 4 ; work->data4 = 0 ;
		SetArmAction( work, AMwall ) ;
		SetAction2( work, -1, 6, mask ) ;
		SetStandWallAction( work, 2 ) ;
	}

	if ( ftime < 8 ) return ;
	if ( ftime == 8 && work->data3 != 4 ) {
		/* ホールドアップノイズ */
		GM_SetNoise( NOISE_HOLD, &( work->control.mov ), work->control.map ) ;
	}

//	First = 0 ;	/* 初期ホーミングなし */

	trg = TRIG_YET ;
	switch( work->data3 ) {
	case 0 :					/* 構え */
		if ( !Status( PLAYER_GROUND ) && 
			( wtype & WP_TYPE_HOMING ) ) Homing( work, (PL_SubjectMove ? 0 : First) ) ;
		First = 0 ;
		/* リロードチェック */
		if ( ( wtype & WP_TYPE_MAGAZINE ) && GM_Magazine == 0 &&
			GM_WeaponNum( work->weapon ) > 0 ) {
			if ( work->stance == GROUND ) {
				SetAction( work, MS.attack[ Mreload + adj ], 6 ) ;
				SetAction2( work, -1, 6, mask ) ;
			} else {
				SetAction2( work, MS.attack[ Mreload + adj ], 6, mask ) ;
			}
			SetArmAction( work, AMreload ) ;
//			PL_SetMagazine( work->weapon, 0 ) ;
			work->data3 = 3 ; 
			work->data4 = 0 ;
			if ( work->weapon == WP_Rgb6 ) trg = TRIG_RELOAD_START ;
			break ;
		}
#if 1
		/* 壁チェック */
		if ( EnableShootBullet( work ) ) {
			work->data3 = 4 ; work->data4 = 0 ;
			SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, mask ) ;
			SetStandWallAction( work, 2 ) ;
			break ;
		}
#endif
		trg = TRIG_SET ;
		work->post &= ~MAIL_ATTACK_CANCEL ;
		/* ゆっくり離す */
#ifndef NEW_HG

      //BP_INPUT - soft release fire (PL_PAD_WEAPON_TH==24) to cancel firing socom pistol?
		if ( ( wtype & WP_TYPE_PRESSURE ) &&                  
			 (    ( (PlayerPad.weaponState == WS_Holster) && (PlayerPad.buttonState == BS_SoftRelease) )    //BP_INPUT - added BP player pad logic for release
            || ( PlayerPad.weaponState == WS_HolsterQuick )
            || ( (PrevPress > 0) && (PrevPress < PL_PAD_WEAPON_TH) && (press > 0) && (press < PL_PAD_WEAPON_TH) ) ) )
      {
			work->data3 += 2 ;
			work->post |= MAIL_ATTACK_CANCEL ;
			//GM_SeSetMode( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
			PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;

         //BP_INPUT - exit to holster quick?
         if(PlayerPad.weaponState == WS_HolsterQuick)
         {
            // Skip to exit quickly
            work->data3 = 5;
         }
         //BP_INPUT - exit to holster quick?
		}
#else
		/* 連射武器系にあわせる */
		if ( ( wtype & WP_TYPE_PRESSURE ) && 
			GV_Time - work->counter2 > 16 && 	/* 押してから１６フレーム以上経っている */
			press <= 0 ) {						/* パッドはなされた */
			work->data3 += 2 ;
			work->post |= MAIL_ATTACK_CANCEL ;
			//GM_SeSetMode( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
			PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
		}		
#endif

		/* 連射武器弱押しから離す */
		if ( ( wtype & WP_TYPE_CONSECUTIVE ) &&
			 ( ( press <= 0 && work->idata2 == 0 ) || 
			   ( press < PL_PAD_WEAPON_TH2 && work->idata2 != 0 ) ) ) {
			if ( work->idata2 == 0 ) {
				work->data3 = 6 ;
				//GM_SeSetMode( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
				PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
				work->post |= MAIL_ATTACK_CANCEL ;
			} else {
				work->data3 = 2 ;
			}
			//work->post |= MAIL_ATTACK_CANCEL ;
			break ;
		}
		/* マシンガン時、から撃ちフラグの制御 */
		if ( ( wtype & WP_TYPE_CONSECUTIVE ) && press < PL_PAD_WEAPON_TH2 ) {
			work->post &= ~MAIL_EMPTY_SHOT ;
			/* 構えモーションに戻す */
			SetAction2Check( work, MS.attack[ Mready + adj ], 0, mask ) ;
			PL_SetArmAction3( work, AMready + armadj, 0, 0 ) ;
#if 0
			if ( work->weapon == WP_Rgb6 ) {
				work->idata2 = 0 ;
			}
#endif
		}
#if 0
		if ( work->weapon == WP_Rgb6 ) {		
			if ( work->idata2 != 0 ) press = PL_PAD_WEAPON_TH2 - 1 ;
		}
#endif

#ifndef NEW_HG
      //BP_INPUT - check for firing: pistol(pressure)=released fire button hard, auto(consecutive)=holding fire button hard
		if ( ( ( wtype & WP_TYPE_PRESSURE ) && !( work->pad->status & PL_PAD_WEAPON ) ) || 
			 ( ( wtype & WP_TYPE_CONSECUTIVE ) && press >= PL_PAD_WEAPON_TH2 ) ) {
#else
		if ( work->sv2.pad == 0 && press < PL_PAD_WEAPON_TH3 ) work->sv2.pad = 1 ;
		if ( ( ( wtype & WP_TYPE_PRESSURE ) && work->sv2.pad == 1 &&
			  ( ( press > PL_PAD_WEAPON_TH3 ) || 
			   ( press <= 0 && ( GV_Time - work->counter2 <= 16 ) ) ) ) ||
			 ( ( wtype & WP_TYPE_CONSECUTIVE ) && press >= PL_PAD_WEAPON_TH2 ) ) {
#endif

#if 0
			/* ニキータ飛行中チェック */
			if ( ( wtype & WP_TYPE_NIKITA ) && GM_NikitaAlive[ work->chanl ] ) {
				work->data3 = 2 ;
				work->post |= MAIL_ATTACK_CANCEL ;
				break ;
			}
#endif
			/* ＲＧＢ－６飛行中チェック */
			if ( work->weapon == WP_Rgb6 && 
				 ( GM_WeaponAlive & WP_CANNOT_FIRE_RGB6 ) ) {
				/* 構え状態を維持 */
				//work->data3 = 2 ;
				//work->post |= MAIL_ATTACK_CANCEL ;
				break ;				
			}
			/* パッド無効中 */
			if ( !PL_PadEnable() ) {
				work->data3 = 2 ;
				work->post |= MAIL_ATTACK_CANCEL ;
				break ;
			}

			/* 壁ありから戻った時用 */
			if ( enable_count > 0 ) {
				if ( enable_count > 8 ) enable_count = 8 ;
				if ( -- enable_count > 0 ) break ;
			}
			if ( ftime > 8 ) {
				work->sv2.pad = 0 ;		/* 連射フラグ */
				if ( GM_WeaponNum( work->weapon ) > 0 ) {
					trg = TRIG_FIRE ;
					SetAction2( work, MS.attack[ Mfire + adj ], 0, mask ) ;
					PL_SetArmAction( work, AMfire ) ;
					work->arm_interp = 0 ;
					work->data3 = 1 ;
					work->data4 = 0 ;
					work->idata2 = GV_Time ;
#if 0
					/* ニキータ特殊 */
					if ( wtype & WP_TYPE_NIKITA ) {
						SetMode( work, ShootNikita ) ;
						SetFlag( FLAG_NO_STEP | FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH ) ;
						LeaveSubject( work ) ;
					}
#endif
					/* 特殊。ＵＳＰ、ＳＯＣＯＭ撃ち時は移動させない */
					if ( ( work->weapon == WP_Usp || work->weapon == WP_Socom ) &&
						Flag( FLAG_MOTION_STEP_OVERRIDE ) ) {
						//SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
					}
					work->post &= ~MAIL_EMPTY_SHOT ;
				} else {
					/* から撃ち */
					if ( !( wtype & WP_TYPE_CONSECUTIVE ) || 
						 ( ( wtype & WP_TYPE_CONSECUTIVE ) && 
						   !( work->post & MAIL_EMPTY_SHOT ) ) ) {
						/* ハンドガンは毎回、マシンガンは撃ち直し時だけ、から撃ちさせる */
						printf( "raiden/attack.c : karauchi!!\n" ) ;
						trg = TRIG_FIRE ;
					} 
					work->post |= MAIL_EMPTY_SHOT ;
					if ( wtype & WP_TYPE_CONSECUTIVE ) {
						/* マシンガンは構えモーションのまま */
						if ( work->motion2 != MS.attack[ Mready + adj ] ) {
							SetAction2( work, MS.attack[ Mready + adj ], 0, mask ) ;
							PL_SetArmAction( work, AMready ) ;
						}
					}
					work->data3 = 2 ; work->data4 = 0 ;
				}
			}
		} 
		break ;
	case 1 :					/* 撃ち */
#if 1
		/* 壁チェック */
		if ( EnableShootBullet( work ) ) {
			work->data3 = 4 ; work->data4 = 0 ;
			SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, mask ) ;
			SetStandWallAction( work, 2 ) ;
			break ;
		}
#endif
		if ( !Status( PLAYER_GROUND ) && 
			( wtype & WP_TYPE_HOMING ) ) Homing( work, 0 ) ;

		/* Ｍ９２のモーションを使いまわしている関係で */
		switch( work->weapon ) {
		case WP_m92 :
			if ( Status( PLAYER_BEHIND_ATTACK ) && 
				 !Status( PLAYER_WATCH ) &&
				 !( work->pad->status & PL_PAD_WEAPON ) &&
				 MotionTime2( work ) >= 8 ) {
				/* 飛び出し撃ち中の廃莢キャンセル */
				if ( ++ work->data4 >= 8 ) {
					work->data3 = 5 ;
					SetFlag( FLAG_RESET_MOTION2_AT_END ) ;
					if ( Status( PLAYER_SNAKE ) ) {
						work->work_l.motion2cutframe = 0 ;
					} else {
						work->work_l.motion2cutframe = 25 ;
					}
					c = 0 ;
					break ;
				}
			} else work->data4 = 0 ;
			/* 廃莢中はリロード扱いにしてみる */
			if ( MotionTime2( work ) >= 8 ) SetFlag( FLAG_RELOADING ) ;
			c = EndMotion2( work ) ;
			break ;
		case WP_Aks :
		case WP_m4 :
		case WP_Rgb6 :
			trg = TRIG_SET ;	/* レーザーサイト対策 */
			c = EndMotion2( work ) ;
			break ;
		case WP_Usp :
		case WP_Socom :
		case WP_Nikita :
#if 0
			/* 特殊。ＵＳＰ、ＳＯＣＯＭ撃ち時は移動させない */
			if ( Flag( FLAG_MOTION_STEP_OVERRIDE ) ) {
				if ( work->fv2.vw > 0.0F ) {
					FVECTOR		diff ;

					PL_ObjPos( work, HUMAN21_HIDARI_TSUMASAKI, &diff ) ;
					_sceVu0SubVector( &diff, &work->fv2, &diff ) ;
					PL_AdjustXZ( work, &diff ) ;
				} 
			} 
#endif
			c = 0 ;
			if ( MotionTime2( work ) >= 4 ) c = 1 ;
			break ;
		default :
			c = 0 ;
		}

		if ( c ) {
			if ( !( wtype & WP_TYPE_CONSECUTIVE ) ) {
				/* 連射武器でなければ、一発ごとに構えモーションに戻る */
				SetAction2( work, MS.attack[ Mready + adj ], 6, mask ) ;
				SetArmAction( work, AMready ) ;
#if 0
			} else if ( work->weapon == WP_Rgb6 ) {
				/* ＲＧＢ */
				SetAction2( work, MS.attack[ Mready + adj ], 6, mask ) ;
				SetArmAction( work, AMready ) ;
#endif
			} else {
				//work->idata2 = 0 ;
			}
			work->data3 ++ ; work->data4 = 0 ;
		}
		break ;
	case 2 :					/* 撃ち終わり */
#if 1
		/* 壁チェック */
		if ( EnableShootBullet( work ) ) {
			work->data3 = 4 ; work->data4 = 0 ;
			SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, mask ) ;
			SetStandWallAction( work, 2 ) ;
			break ;
		}
#endif
		if ( !Status( PLAYER_GROUND ) && 
			( wtype & WP_TYPE_HOMING ) ) Homing( work, 0 ) ;
		trg = TRIG_YET ;
		if ( wtype & WP_TYPE_PRESSURE ) {			/* ハンドガン */
#ifndef NEW_HG
			if ( press < PL_PAD_WEAPON_TH ) {
#else
			if ( press <= 0 ) {
#endif
				if ( ++ work->data4 >= 8 ) {
#if 0				/* 今のところない */			
					if ( wtype & WP_TYPE_RELEASE_MOTION ) {
						SetAction2( work, MS.attack[ Mfire_end + adj ], 6, mask ) ;
						SetArmAction( work, AMfire_end ) ;
					}
#endif
					work->data3 = 5 ;
				}
			} else {
				work->counter2 = GV_Time ;
				if ( press > PL_PAD_WEAPON_TH3 ) {
					work->sv2.pad = 0 ;
				} else {
					work->sv2.pad = 1 ;
				}
				enable_count = 4 ;
				work->data3 = 0 ; work->data4 = 0 ;
			}
		} else if ( wtype & WP_TYPE_CONSECUTIVE ) {	/* マシンガン */
			if ( press < PL_PAD_WEAPON_TH2 || GM_WeaponNum( work->weapon ) <= 0 ) {
				if ( ( wtype & WP_TYPE_RELEASE_MOTION ) && 
					 !( work->post & MAIL_EMPTY_SHOT ) ) {
					SetAction2( work, MS.attack[ Mfire_end + adj ], 6, mask ) ;
					SetArmAction( work, AMfire_end ) ;
				} else {
					trg = TRIG_SET ;	/* レーザーサイト対策 */
				}
				work->data3 = 5 ;
			} else {
				trg = TRIG_SET ;	/* レーザーサイト対策 */
				if ( GM_WeaponNum( work->weapon ) <= 0 /* || work->weapon == WP_Rgb6 */ ) {
					if ( work->motion2 != MS.attack[ Mready + adj ] ) {
						SetAction2( work, MS.attack[ Mready + adj ], 0, mask ) ;
						PL_SetArmAction3( work, AMready + armadj, 0, 0 ) ;
					}
				} else {
					SetAction2( work, MS.attack[ Mfire + adj ], 0, mask ) ;
					work->arm_trigger |= ARM_MOTION_SET_OVER ;
					PL_SetArmAction3( work, AMfire + armadj, 0, 0 ) ;		
				}
				enable_count = 1 ;
				work->data3 = 0 ; work->data4 = 0 ;
			}			
		} else {									/* その他（ないかも） */
			if ( press <= 0 && ++ work->data4 >= 8 ) {
#if 0			/* 今のところない */
				if ( wtype & WP_TYPE_RELEASE_MOTION ) {
					SetAction2( work, MS.attack[ Mfire_end + adj ], 6, mask ) ;
					SetArmAction( work, AMfire_end ) ;
				}
#endif
				work->data3 = 5 ;
			} else {
				enable_count = 1 ;
				work->data3 = 0 ; work->data4 = 0 ;
			}
		}
#if 0
		if ( !( work->pad->status & PL_PAD_WEAPON ) ||
			( wtype & ( WP_TYPE_PRESSURE | WP_TYPE_CONSECUTIVE ) && press < PL_PAD_WEAPON_TH ) ) {
			if ( ++ work->data4 == 8 ) {
				if ( wtype & WP_TYPE_RELEASE_MOTION ) {
					SetAction2( work, MS.attack[ Mfire_end + adj ], 6, mask ) ;
					SetArmAction( work, AMfire_end ) ;
				}
				work->data3 = 5 ;
			}
		} else {
			if ( wtype & WP_TYPE_CONSECUTIVE ) {
				SetAction2( work, MS.attack[ Mready + adj ], 0, mask ) ;
				work->arm_trigger |= ARM_MOTION_SET_OVER ;
				PL_SetArmAction3( work, AMready + armadj, 0, 0 ) ;				
			}
			enable_count = 1 ;
			work->data3 = 0 ; work->data4 = 0 ;
		}
#endif
		break ;
	case 3 :					/* リロード */
		trg = TRIG_YET ;
		if ( work->weapon == WP_Rgb6 ) trg = TRIG_RELOADING ;
		work->arm_trigger |= ARM_IS_RELOAD ;
		UnsetStatus( PLAYER_HOLD ) ;
		SetFlag( FLAG_RELOADING ) ;
		if ( ( work->stance != GROUND && PL_CheckMotionTime2( work, 47 ) ) ||
			 ( work->stance == GROUND && PL_CheckMotionTime( work, 47 ) ) ) {
			PL_SetMagazine( work->weapon, 0 ) ;
		}
		if ( ( work->stance != GROUND && EndMotion2( work ) ) ||
			( work->stance == GROUND && EndMotion( work ) ) ) {
//			PL_SetMagazine( work->weapon, 0 ) ;
			if ( work->stance == GROUND ) SetAction( work, MS.change[ Mcrouch ], 6 ) ;
			SetAction2( work, MS.attack[ Mready + adj ], 6, mask ) ;
			if ( !( work->pad->status & PL_PAD_WEAPON ) ) {
				SetMode2( work, NULL ) ;
				work->homing = NULL ;
				SetArmAction( work, AMstand ) ;
			} else {
				work->counter2 = GV_Time ;
				if ( press > PL_PAD_WEAPON_TH3 ) {
					work->sv2.pad = 0 ;
				} else {
					work->sv2.pad = 1 ;
				}
				work->post &= ~MAIL_EMPTY_SHOT ;
				enable_count = 1 ;
				SetArmAction( work, AMready ) ;
				work->data3 = 0 ;
				work->idata2 = 0 ;
			}
		}
		/* マガジン制御 */
		mtime = ( work->stance != GROUND ) ? MotionTime2( work ) : MotionTime( work ) ;
		if ( work->weapon == WP_Usp ) {
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 48 ) : PL_CheckMotionTime( work, 48 ) ;
			if ( chktime ) trg = TRIG_MAG_LEFT_HAND_AND_FALL ;
			else if ( ( 28 < mtime && mtime < 48 ) || 
					 ( 48 < mtime && mtime < 60 ) ) trg = TRIG_MAG_LEFT_HAND ;
			else if ( 60 <= mtime && mtime < 67 ) trg = TRIG_MAG_LEFT_HAND2 ;
			else if ( mtime >= 67 ) trg = TRIG_MAG_INVISIBLE ;
		} else if ( work->weapon == WP_m92 ) {
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 48 ) : PL_CheckMotionTime( work, 48 ) ;
			if ( chktime ) trg = TRIG_MAG_LEFT_HAND_AND_FALL ;
			else if ( ( 28 < mtime && mtime < 48 ) || 
					 ( 48 < mtime && mtime < 60 ) ) trg = TRIG_MAG_LEFT_HAND ;
			else if ( 60 <= mtime && mtime < 67 ) trg = TRIG_MAG_LEFT_HAND2 ;
			else if ( mtime >= 67 ) trg = TRIG_MAG_INVISIBLE ;
		} else if ( work->weapon == WP_Socom ) {
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 48 ) : PL_CheckMotionTime( work, 48 ) ;
			if ( chktime ) trg = TRIG_MAG_LEFT_HAND_AND_FALL ;
			else if ( ( 28 < mtime && mtime < 48 ) || 
					 ( 48 < mtime && mtime < 60 ) ) trg = TRIG_MAG_LEFT_HAND ;
			else if ( 60 <= mtime && mtime < 67 ) trg = TRIG_MAG_LEFT_HAND2 ;
			else if ( mtime >= 67 ) trg = TRIG_MAG_INVISIBLE ;
		} else if ( work->weapon == WP_Aks ) {
		    if ( Status( PLAYER_SNAKE ) ) {
			if ( work->stance == GROUND ) {
				if ( PL_CheckMotionTime( work, 7 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 7 && mtime < 32 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 63-7 ) trg = TRIG_MAG_LEFT_HAND ;
			} else {
				if ( PL_CheckMotionTime2( work, 17 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 17 && mtime < 34 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 67 ) trg = TRIG_MAG_LEFT_HAND ;
			}
		    } else {
			if ( work->stance == GROUND ) {
				if ( PL_CheckMotionTime( work, 5 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 5 && mtime < 32 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 54 ) trg = TRIG_MAG_LEFT_HAND ;
			} else {
				if ( PL_CheckMotionTime2( work, 13 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 13 && mtime < 34 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 54 ) trg = TRIG_MAG_LEFT_HAND ;
			}
		    }
		} else if ( work->weapon == WP_m4 ) {
		    if ( Status( PLAYER_SNAKE ) ) {
			if ( work->stance == GROUND ) {
				if ( PL_CheckMotionTime( work, 7 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 7 && mtime < 32 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 63-7 ) trg = TRIG_MAG_LEFT_HAND ;
			} else {
				if ( PL_CheckMotionTime2( work, 17 ) ) trg = TRIG_MAG_FALL ;
				else if ( mtime >= 17 && mtime < 34 ) trg = TRIG_MAG_INVISIBLE ;
				else if ( mtime >= 32 && mtime < 67 ) trg = TRIG_MAG_LEFT_HAND ;
			}
		    } else {
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 13 ) : PL_CheckMotionTime( work, 13 ) ;
			if ( chktime ) trg = TRIG_MAG_FALL ;
			else if ( 13 <= mtime && 35 > mtime ) trg = TRIG_MAG_INVISIBLE ;
			else if ( 35 <= mtime && 63 > mtime ) trg = TRIG_MAG_LEFT_HAND ;
		    }
		} else if ( work->weapon == WP_Rgb6 ) {
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 26 ) : PL_CheckMotionTime( work, 26 ) ;
			if ( chktime ) trg = TRIG_MAG_LEFT_HAND ;
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 50 ) : PL_CheckMotionTime( work, 52 ) ;
			if ( chktime ) trg = TRIG_MAG_INVISIBLE ;
			chktime = ( work->stance != GROUND ) ? 
				PL_CheckMotionTime2( work, 69 ) : PL_CheckMotionTime( work, 71 ) ;
			if ( chktime ) trg = TRIG_RGB_AMOGRIP_INVISIBLE ;
		}
		break ;

	case 4 :					/* 壁があるだす */
		//UnsetStatus( PLAYER_HOLD ) ;
		//UnsetWeaponCamera( work ) ;
		UnsetFlag( FLAG_SUBJECT_ARM_ADJUST ) ;
		SetStandWallAction( work, 2 ) ;
		work->arm_trigger |= ARM_IS_WALL ;
		/* 一応ホーミング */
		if ( !Status( PLAYER_GROUND ) && 
			( wtype & WP_TYPE_HOMING ) ) Homing( work, (PL_SubjectMove ? 0 : First) ) ;
		First = 0 ;
		if ( !( work->pad->status & PL_PAD_WEAPON ) ||
#ifndef NEW_HG
			( wtype & WP_TYPE_PRESSURE && press < PL_PAD_WEAPON_TH ) ) {
#else
			( wtype & WP_TYPE_PRESSURE && press <= 0 ) ) {
#endif
			if ( ++ work->data4 >= 8 ) {
				/* 攻撃おわり */
				work->data3 = 5 ;
				break ;
			} 
		} else {
			work->data4 = 0 ;
		}
		if ( !EnableShootBullet3( work, 0, 0.0F, 1100.0F ) ) {
			/* 壁がなくなった */
			/* チェック距離を少し長めにして振動するのを防ぐ */
			if ( ++ enable_count >= 8 ) {
				work->counter2 = GV_Time ;
				if ( press > PL_PAD_WEAPON_TH3 ) {
					work->sv2.pad = 0 ;
				} else {
					work->sv2.pad = 1 ;
				}
				work->post &= ~MAIL_EMPTY_SHOT ;
				SetArmAction( work, AMready ) ;	    
				SetAction2( work, MS.attack[ Mready + adj ], 6, mask ) ;
				//enable_count = 1 ;
				enable_count = 8 ;
				work->data3 = 0 ;	    
			}
		} else {
			enable_count = 0 ;
		}
		if ( work->work_l.result_len < 300.0F ) {
			work->arm_trigger |= ARM_NEAR_WALL2 ;
		} else if ( work->work_l.result_len < 450.0F ) {
			work->arm_trigger |= ARM_NEAR_WALL ;
		}
		break ;
	case 5 :					/* 構え終了 */	
		c = 0 ;
		UnsetStatus( PLAYER_HOLD ) ;
#ifndef NEW_HG
		if ( ( wtype & WP_TYPE_PRESSURE ) && press >= PL_PAD_WEAPON_TH ) {		
#else
		if ( ( wtype & WP_TYPE_PRESSURE ) && press > 0 ) {
#endif
			/* ハンドガン撃ち直し */
			work->counter2 = GV_Time ;
			if ( press > PL_PAD_WEAPON_TH3 ) {
				work->sv2.pad = 0 ;
			} else {
				work->sv2.pad = 1 ;
			}
			enable_count = 4 ;
			work->data3 = 0 ; work->data4 = 0 ;			
			break ;
		} else if ( ( wtype & WP_TYPE_CONSECUTIVE ) && press >= PL_PAD_WEAPON_TH2 ) {
			/* マシンガン撃ち直し */
			if ( ( work->post & MAIL_EMPTY_SHOT ) 
				 /* || work->weapon == WP_Rgb6 */ ) {
				if ( work->motion2 != MS.attack[ Mready + adj ] ) {
					SetAction2( work, MS.attack[ Mready + adj ], 0, mask ) ;
					PL_SetArmAction3( work, AMready + armadj, 0, 0 ) ;				
				}
			} else if ( GM_WeaponNum( work->weapon ) > 0 ) {
				SetAction2( work, MS.attack[ Mfire + adj ], 0, mask ) ;
				work->arm_trigger |= ARM_MOTION_SET_OVER ;
				PL_SetArmAction3( work, AMfire + armadj, 0, 0 ) ;	
			}
			trg = TRIG_SET ;	/* レーザーサイト対策 */
			enable_count = 1 ;
			work->data3 = 0 ; work->data4 = 0 ;			
			break ;			
		}

		if ( ( wtype & WP_TYPE_RELEASE_MOTION ) &&
			( work->motion2 == MS.attack[ Mfire_end + adj ] ) ) {
			if ( EndMotion2( work ) || 
				( work->pad->status & PAD_UDLR ) ) c = 1 ;
		} else c = 1 ;
		if ( c ) {
			SetArmAction( work, AMstand ) ;
			SetMode2( work, NULL ) ;
			work->homing = NULL ;	
		}
		break ;
	case 6 :					/* 連射武器を撃たずに構え終了 */
		SetArmAction( work, AMstand ) ;
		SetMode2( work, NULL ) ;
		work->homing = NULL ;			
		break ;
	}
	work->trigger = trg ;
	PrevPress = press ;


	/*主観移動の場合はＺＸの移動を止める*/
	if ( PL_SubjectMove != 0 ) {
		if ( work->stance == STAND ) SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	} else if ( work->stance == STAND &&
		 ( work->weapon == WP_Socom || work->weapon == WP_Usp ) &&
		 ( work->motion2 == MS.attack[ Mready + adj ] ||
		   work->motion2 == MS.attack[ Mreload + adj ] ||
		   work->motion2 == MS.attack[ Mfire + adj ] ) ) {
		/* 特殊。ＵＳＰ、ＳＯＣＯＭ撃ち時は移動させない */
		if ( Flag( FLAG_MOTION_STEP_OVERRIDE ) &&
			 work->pre_turn.vy == work->control.rot.vy ) {
			float		vw ;

			work->fv2.vw += 1.0F ;
			if ( work->fv2.vw >= 5.0F ) {
				FVECTOR		diff ;
				
				vw = work->fv2.vw ;
				PL_ObjPos( work, HUMAN21_HIDARI_TSUMASAKI, &diff ) ;
				_sceVu0SubVector( &diff, &work->fv2, &diff ) ;
				PL_AdjustXZ( work, &diff ) ;
				work->control.step.vx += diff.vx ;
				work->control.step.vz += diff.vz ;
				work->fv2.vw = vw ;
			} 
			if ( work->fv2.vw >= 4.0F ) {
				vw = work->fv2.vw ;
				PL_ObjPos( work, HUMAN21_HIDARI_TSUMASAKI, &work->fv2 ) ;
				work->fv2.vw = vw ;
			} else {
				SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
			}
		} else {
			work->fv2.vw = 0.0F ;
		}
	} else {
		if ( work->stance != STAND ) work->fv2.vw = -32.0F ; 
		else						 work->fv2.vw = 0.0F ;
	}
}

/* ニキータ発射＆飛行中 */
static	void	ShootNikita( Work *work, int time )
{
	int		trg, ftime, act, act2 ;
	int		c, wtype, press ;
	float	height ;

	ftime = work->ftime ;
	SetStatus( PLAYER_HOLD ) ;
	SetFlag( FLAG_NO_CAUTION | FLAG_SUBJECT_ARM_ADJUST | FLAG_NO_STEP |
			 FLAG_CANNOT_CHANGE_ITEM | FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP |
			 FLAG_CANNOT_SUBJECT_U | FLAG_CANNOT_SUBJECT_D ) ;
	wtype = ( ( WeaponSet * )work->wp_set )->type ;
	press = work->pad->pressure[ PL_PAD_PRESS_WEAPON ] ;

	if ( time == 0 ) {
		/* 撃てない状態 */
#if 0
		if ( GM_CheckGameStatus( STATE_CHAFF | STATE_RADAR_JAMMING ) || 
			( Status( PLAYER_CAUTION ) && !Flag( FLAG_BEHIND_ATTACK ) ) ) {
			SetArmAction( work, AMstand ) ;
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetMode( work, PL_StillMode[ work->stance ] ) ;
			work->homing = NULL ;
			return ;
		}
#endif
		//PL_IntoSubject( work ) ;
		//GM_SeSetMode( SD_W_EQUIP02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		PL_SeSetSubject( SD_W_EQUIP02, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		switch( work->stance ) {
		case STAND :
			act = MS.change[ Mstand ] ;
			act2 = nkt_fire_p ;
			break ;
		case SQUAT :
			act = MS.change[ Msquat ] ;
			act2 = nkt_fire_p_sq ;
			break ;
		case GROUND :
		default :
			act = MS.change[ Mcrouch ] ;
			act2 = nkt_fire_p_cr ;
		}
		SetAction( work, act, 6 ) ;
		SetAction2( work, act2, 6, 0x01ffe ) ;
		work->idata2 = act2 ;
		//SetArmAction( work, AMready ) ;
		work->homing = NULL ;
		work->adj_rot = DG_ZeroSVector ;
		work->homing_near = RIGHT_HOMING ;
		work->sv.vx = press ;
		work->sv.vy = ( Flag( FLAG_SUBJECT_NEWPRESS ) || !Status( PLAYER_WATCH ) ) ? 1 : 0 ;
		UnsetFlag( FLAG_SUBJECT_NEWPRESS ) ;
		work->sv.vz = 1 ;
		work->counter2 = GV_Time ;
		work->sv.pad = 1 ;
	}

//	SetWeaponCamera( work ) ;
	/* 主観カメラをいじる */
	{
		if ( work->stance == STAND ) height = 1250.0F ;
		else if ( work->stance == SQUAT ) height = 750.0F ;
		else {
			/* LimitControlで250上げられる */
			if ( ftime >= 8 && ( work->data == 0 || work->data == 4 ) ) {
				if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
					height = StepHeightAdjust( work->control.level[ 0 ], 
											   work->control.levels[ 0 ] ) ;
				} else {
					height = 0.0F ;
				}
			} else {
				GroundIK2( work, 500.0F ) ;
				if ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) {
					height = StepHeightAdjust( work->control.level[ 0 ], 
											   work->control.levels[ 0 ] ) + 250.0F ;
				} else {
					height = 250.0F ;				
				}
			}
		}
		work->camera.vx = work->control.mov.vx ;
		work->camera.vz = work->control.mov.vz ;
		work->camera.vy = work->control.levels[ 0 ] + height ;
	}

	/* 初期壁チェック */
	if ( ftime == 2 ) {
		if ( EnableShootBullet2( work, 1, height ) ) {
			work->data = 4 ; work->data2 = 0 ;
			//SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, 0x01ffe ) ;	
			SetStandWallAction( work, 2 ) ;
			PL_LeaveSubject( work ) ;
		} else {
			PL_IntoSubject( work ) ;
		}
	}

	if ( ftime < 8 ) return ;
	if ( ftime == 8 && work->data != 4 ) {
		GM_SetNoise( NOISE_HOLD, &( work->control.mov ), work->control.map ) ;
	}

	trg = TRIG_YET ;
	switch( work->data ) {
	case 0 :					/* 構え */
	    if ( PL_SubjectMove ) {
		SubjectMoveTurn( work ) ;
	    } else {
		PL_SubjectTurn( work ) ;
	    }
		GroundRotateLimitControl( work ) ;
		work->sv.vy = 0 ;
		/* 壁チェック */
		if ( EnableShootBullet2( work, 1, height ) ) {
			work->data = 4 ; work->data2 = 0 ;
			//SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetStandWallAction( work, 2 ) ;
			PL_LeaveSubject( work ) ;
			break ;
		}
		trg = TRIG_SET ;
		work->post &= ~MAIL_ATTACK_CANCEL ;

      // AS MCampbell - Use similar logic to the pistols when the holster button is pressed to cancel the firing state.
      // This simulates the user slow releasing the shoot button on PS3.
      // Bug: MGSTWO-2939
      if ( ( wtype & WP_TYPE_PRESSURE ) &&                  
         (    ( (PlayerPad.weaponState == WS_Holster) && (PlayerPad.buttonState == BS_SoftRelease) )  
         || ( PlayerPad.weaponState == WS_HolsterQuick )
         || ( (PrevPress > 0) && (PrevPress < PL_PAD_WEAPON_TH) && (press > 0) && (press < PL_PAD_WEAPON_TH) ) ) )
      {
         work->data = 2 ;
         work->post |= MAIL_ATTACK_CANCEL ;
         PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
      }

		/* ゆっくり離す */
#ifndef NEW_HG
		if ( wtype & WP_TYPE_PRESSURE &&
			work->sv.vx > 0 && work->sv.vx < PL_PAD_WEAPON_TH &&
			press > 0 && press < PL_PAD_WEAPON_TH ) {
			work->data = 2 ;
			work->post |= MAIL_ATTACK_CANCEL ;
			//GM_SeSetMode( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
			PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
		}
#else
		if ( ( wtype & WP_TYPE_PRESSURE ) && 
			GV_Time - work->counter2 > 16 && 	/* 押してから１６フレーム以上経っている */
			press <= 0 ) {						/* パッドはなされた */
			work->data += 2 ;
			work->post |= MAIL_ATTACK_CANCEL ;
			//GM_SeSetMode( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
			PL_SeSetSubject( SD_W_EQUDEC01, &work->control.mov, GM_SEMODE_NORMAL ) ;
		}		
#endif
#ifndef NEW_HG
		if ( !( work->pad->status & PL_PAD_WEAPON ) ) {
#else
		if ( work->sv.pad == 0 && press < PL_PAD_WEAPON_TH3 ) work->sv.pad = 1 ;
		if ( ( ( wtype & WP_TYPE_PRESSURE ) && work->sv.pad == 1 &&
			  ( ( press > PL_PAD_WEAPON_TH3 ) || 
			   ( press <= 0 && ( GV_Time - work->counter2 <= 16 ) ) ) ) ) {
#endif
			/* ニキータ飛行中チェック */
			if ( GM_NikitaAlive[ work->chanl ] ||
				 GM_CheckGameStatus( STATE_CHAFF | STATE_RADAR_JAMMING ) ) {
				work->data = 2 ;
				work->post |= MAIL_ATTACK_CANCEL ;
				break ;
			}
			/* パッド無効中 */
			if ( !PL_PadEnable() ) {
				work->data = 2 ;
				work->post |= MAIL_ATTACK_CANCEL ;
				break ;
			}
			/* 壁ありから戻った時用 */
			if ( work->sv.vz > 0 ) {
				if ( work->sv.vz > 8 ) work->sv.vz = 8 ;
				if ( -- work->sv.vz > 0 ) break ;
			}
			if ( ftime > 8 ) {
				work->sv.pad = 0 ;
				trg = TRIG_FIRE ;
				if ( GM_WeaponNum( WP_Nikita ) > 0 ) {
					SetAction2( work, work->idata2, 0, 0x01ffe ) ;
					//SetArmAction( work, AMfire ) ;
					work->data = 1 ;
					work->data2 = 0 ;
				} else {
					/* から撃ち */
					work->data = 2 ; work->data2 = 0 ;
				}
			}
		} 
		break ;
	case 1 :					/* 撃ち */
		/* 壁チェック */
		if ( EnableShootBullet2( work, 1, height ) ) {
			work->data = 4 ; work->data2 = 0 ;
			//SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetStandWallAction( work, 2 ) ;
			PL_LeaveSubject( work ) ;
			break ;
		}
		/* Ｍ９２のモーションを使いまわしている関係で */
		c = 0 ;
		if ( MotionTime2( work ) >= 4 ) c = 1 ;

		if ( c ) {
			/* 構えに戻る */
			SetAction2( work, work->idata2, 6, 0x01ffe ) ;
			//SetArmAction( work, AMready ) ;
			work->data = 2 ; work->data2 = 0 ;
		} 
		break ;
	case 2 :					/* 撃ち終わり */
		/* 壁チェック */
		if ( EnableShootBullet2( work, 1, height ) ) {
			work->data = 4 ; work->data2 = 0 ;
			//SetArmAction( work, AMwall ) ;	    
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetStandWallAction( work, 2 ) ;
			PL_LeaveSubject( work ) ;
			break ;
		}
		trg = TRIG_YET ;
		
		/* かつて,ここにあったコードはハンドガン用だったので取った(ソノヤマさんに確認済み) T.Morita */
		if ( ++ work->data2 == 8 ) {
		    work->data = 5 ;
		}


		break ;
	case 4 :					/* 壁があるだす */
		/* 主観の回転方法 */
		SetFlag( FLAG_CANNOT_SUBJECT_UD | FLAG_CANNOT_PEEP ) ;
		if ( PL_SubjectMove ) {
		    SubjectMoveTurn( work ) ;
		} else {
		    PL_SubjectTurn( work ) ;
		}

		GroundRotateLimitControl( work ) ;
		UnsetFlag( FLAG_SUBJECT_ARM_ADJUST ) ;
		work->arm_trigger |= ARM_IS_WALL ;
		First = 0 ;
#ifndef NEW_HG
		if ( !( work->pad->status & PL_PAD_WEAPON ) ||
			( wtype & WP_TYPE_PRESSURE && press < PL_PAD_WEAPON_TH ) ) {
#else
		if ( press <= 0 ) {
#endif
			if ( ++ work->data2 >= 8 ) {
				/* 攻撃おわり */
				work->data = 5 ;
				break ;
			} 
		} else {
			work->data2 = 0 ;
		}
		if ( !EnableShootBullet2( work, 1, height ) ) {
			/* 壁がなくなった */
			if ( ++ work->sv.vz >= 8 ) {
				work->counter2 = GV_Time ;
				if ( press > PL_PAD_WEAPON_TH3 ) {
					work->sv.pad = 0 ;
				} else {
					work->sv.pad = 1 ;
				}
				//SetArmAction( work, AMready ) ;	    
				SetAction2( work, work->idata2, 6, 0x01ffe ) ;
				work->sv.vz = 8 ;
				work->data = 0 ;	    
				PL_IntoSubject( work ) ;
			}
		} else {
			work->sv.vz = 0 ;
		}
		break ;
	case 5 :					/* 構え終了 */	
		c = 0 ;
		UnsetStatus( PLAYER_HOLD ) ;
#if 0
		if ( ( work->pad->press & PL_PAD_WEAPON ) ||
			 ( wtype & WP_TYPE_PRESSURE && press >= PL_PAD_WEAPON_TH ) ) {
			/* 撃ち直し */
			PL_IntoSubject( work ) ;
			work->sv.vz = 1 ;
			work->data = 0 ; work->data2 = 0 ;			
			break ;
		}
#endif
		if ( ( wtype & WP_TYPE_RELEASE_MOTION ) &&
			( work->motion2 == MS.attack[ Mfire_end ] ) ) {
			if ( EndMotion2( work ) ) {
				//SetArmAction( work, AMstand ) ;
				SetAction2( work, -1, 6, 0x01ffe ) ;
				c = 1 ;
			}
		} else c = 1 ;
		if ( c ) {
			//SetArmAction( work, AMstand ) ;
			SetAction2( work, work->idata2, 6, 0x01ffe ) ;
			//SetAction2( work, -1, 6, 0x01ffe ) ;
			work->homing = NULL ;	
			work->data = 6 ;
			work->data2 = 0 ;
		}
		break ;
	case 6 :					/* ミサイル制御不能まで待ち */
		if ( ++ work->data2 == DIRECT_TICK( 60 ) ) PL_LeaveSubject( work ) ;

		if ( GM_NikitaAlive[ work->chanl ] == NKT_NONE || 
			 GM_NikitaAlive[ work->chanl ] == NKT_JAMMING ) {
			if ( Status(PLAYER_WATCH) ) {
				PL_LeaveSubject( work ) ;
			}
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetMode( work, StillMode[ work->stance ] ) ;
		} else {
			if ( work->data2 > DIRECT_TICK( 64 ) ) SetFlag2( FLAG2_FORCE_VISIBLE ) ;
			if ( HZX_CheckInsideTrap( HZX_CurrentGroupID, &GM_NikitaPosition[ 0 ],
									  HIDDEN_TRAP ) ) {
				SetFlag2( FLAG2_INTRUDE_EFFECT_ON ) ;
			} else {
				SetFlag2( FLAG2_INTRUDE_EFFECT_OFF ) ;
			}
		}
	}
	work->trigger = trg ;
	work->sv.vx = press ;
}

#if 0
static	void	ShootNikita( work, time )
Work		*work ;
int		time ;
{
	int		act ;

	if ( time == 0 ) {
		switch( work->stance ) {
		case STAND :
			act = MS.change[ Mstand ] ;
			break ;
		case SQUAT :
			act = MS.change[ Msquat ] ;
			break ;
		case GROUND :
		default :
			act = MS.change[ Mcrouch ] ;
		}
		SetAction( work, act, 6 ) ;
	}
	SetFlag( FLAG_NO_STEP | FLAG_CANNOT_CHANGE_ITEM | FLAG_DONOT_CHECK_WATCH ) ;
	if ( GM_NikitaAlive[ work->chanl ] == 0 ) {
		SetMode( work, StillMode[ work->stance ] ) ;
	}
}
#endif

/* Ｃ４セット */
static	void	SetC4BombCheck( Work *work, int time )
{
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_NO_TOUCH_DAMAGE | FLAG_CANNOT_PEEP ) ;
	SetStatus( PLAYER_MENU_DISABLE ) ;
	if ( time == 0 ) {
		if ( work->stance == STAND ) {
			/* 敵付けチェック */
			SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE | TARGET_THROUGH,
							 300.0F, 650.0F, 300.0F,
							 0.0F, 0.0F, 0.0F,
							 -1, 0.0F, 0.0F, 650.0F, 
							 WP_PUTC4, C4CaptureCallBack ) ;
		} else {
			/* 床つけ */
			SetMode( work, SetC4BombFloor ) ;
		}
		return ;
	}   
	if ( work->offense.damaged & TARGET_CAPTURE ) {
		/* 敵付け */
		SetMode( work, SetC4BombEnemy ) ;
	} else {
		/* 床つけ */
		SetMode( work, SetC4BombFloor ) ;
	}
}

static	void	SetC4BombEnemy( Work *work, int time )
{
	PL_StickWPWork	*stwp ;

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_CANNOT_PEEP ) ;
	SetStatus( PLAYER_MENU_DISABLE ) ;

	if ( time == 0 ) {
		SetAction( work, c4_set_enemy, 6 ) ;
	}
	if ( PL_CheckMotionTime( work, 25 - 1 ) ) {
		/* もう一度敵つけチェック */
		work->data2 = ( int )work->capture.capture ;
		work->capture.capture = NULL ;
		work->offense.damaged &= ~TARGET_CAPTURE ;
		SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE | TARGET_THROUGH,
						 300.0F, 650.0F, 300.0F,
						 0.0F, 0.0F, 0.0F,
						 -1, 0.0F, 0.0F, 650.0F, 
						 WP_PUTC4, C4CaptureCallBack ) ;
		work->data = 1 ;
		return ;
	}

	stwp = &work->stick_wp_work ;

	if ( work->data == 1 ) {
		if ( ( work->offense.damaged & TARGET_CAPTURE ) &&
			 ( work->capture.capture == ( CAPTURE_TARGET * )work->data2 ) ) {
			/* つける */
			work->capture.capture->flag |= CAPTURE_C4EXIST ;
			work->capture.capture = NULL ;		
			stwp->trigger = TRIG_FIRE_C4ENEMY ;
		} else {
			/* 逃げられたか */
		}
	}
	if ( EndMotion( work ) ) {
		UnsetFlag( FLAG_CANNOT_CHANGE ) ;
		UnsetStatus( PLAYER_MENU_DISABLE ) ;
		SetMode( work, StillMode[ work->stance ] ) ;
	}
}

static	int		SetClaymoreArmIK( Work *work, float height ) ; /* 兼用 */

static	void	SetC4BombFloor( Work *work, int time )
{
	PL_StickWPWork	*stwp ;
	int				stance ;
	
	stance = work->stance ;
	if ( time == 0 ) {
		/* 死体の上はダメ */
		if ( Status( PLAYER_ON_CORPSE ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ stance ] ) ;
			return ;
		}
		/* 前方壁 */
		if ( stance == GROUND && 
			 EnableShootBulletEX3( work, 1, 748.0F, 950.0F, 
								   HZX_SEG_NO_PLAYER,
								   0, NULL, HZX_CHK_SEGMENT ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ stance ] ) ;
			return ;
		}
		/* 天井 */
		if ( stance == GROUND && !CheckCeil2( work, 1000.0F ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ stance ] ) ;
			return ;
		}
		if ( stance != GROUND ) {
			SetAction( work, c4_set_floor, 6 ) ;
			work->data2 = 28 ;
		} else {
			SetAction( work, c4_set_floor_cr, 6 ) ;
			work->data2 = 32 ;
		}
		work->idata = -1 ;
		work->sv.vx = 0 ;	/* クレイモアＩＫ使用の為 */
		work->sv.vy = 64 * 3 ;
		work->sv.vz = DIRECT_TICK( 32 ) ;
		work->sv.pad = 1 ;
	}

	if ( work->stance != GROUND ) GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;

	SetStatus( PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE | FLAG_CANNOT_PEEP | FLAG_NO_IK ) ;

	if ( stance == GROUND && Status( PLAYER_WATCH ) ) {
		PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
		work->camera.vy += 250.0F ;
	}

	stwp = &work->stick_wp_work ;

	if ( stance == GROUND ) {
		GroundRotateLimitControl( work ) ;
	} else {
		int		tmp ;

		tmp = work->control.turn.vy ;
		AvoidSink( work, 500.0F, 950.0F ) ;
		work->control.turn.vy = tmp ;	
	}	

	work->idata2 = SetClaymoreArmIK( work, 18.20F ) ;

	if ( work->data == 1 || PL_CheckMotionTime( work, work->data2 - 1 ) ) {
		FVECTOR			shift = { 47.5F, -131.5F, 7.5F } ;
		FVECTOR			pos ;
		float			levels[ 2 ] ;
		HZX_HZD			flr[ 2 ] ;
		int				c1 ;

		if ( work->idata2 == 0 ) {
			work->data = 1 ;
			PL_MotionSleep( work, 0 ) ;
			goto set_c4_floor_set_skip ;
		} else {
			PL_MotionActive( work, 0 ) ;
			work->data = 0 ;
		}

		DG_SetPos( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ;
		DG_PutVector( &shift, &pos, 1 ) ;
		pos.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
		/* 床チェック */
		levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
		c1 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &pos, 
								   HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 	 
								   0 ) ;
		if ( c1 & 1 ) {
			HZX_GetLevelHeight( levels ) ;
			HZX_GetLevelHazard( flr, NULL ) ;
			printf( "raiden/attack.c : flr %x %x\n", flr->ptr, flr->attribute ) ;
			if ( !( flr[ 0 ].attribute & HZX_FLOOR_NO_C4 ) &&
				 !CheckNearEnemy( &pos, work->control.rot.vy, 768, 1000.0F, HOMING_DEAD ) &&
				 pos.vy - levels[ 0 ] < 125.0F ) {
				stwp->trigger = TRIG_FIRE ;
				stwp->hzd = flr[ 0 ] ;
				stwp->target = NULL ;
				stwp->capture = NULL ;
			} else {
				printf( "raiden/attack.c : c4 yuka tooi %f\n", pos.vy - levels[ 0 ] ) ;
				GM_Buzzer() ;
			}
		} else {
			printf( "raiden/attack.c : c4 yuka nashi\n" ) ;
			GM_Buzzer() ;	
		}
		printf( "raiden/attack.c : set %f %f\n", levels[ 0 ], pos.vy ) ;
	}

set_c4_floor_set_skip :

	work->sv.vy -= 2 ;
	if ( work->sv.vy < 16 ) work->sv.vy = 16 ;

	if ( Status( PLAYER_WATCH ) ) {
		FVECTOR		diff, adj, pos ;
		SVECTOR		rot ;
		float		len ;
		int			adjr ;

		PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
		//work->camdir.vx = 380 + ( float )work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.45F ;
		if ( work->camera.vy < pos.vy + 500.0F ) {
			work->camera.vy = pos.vy + 500.0F ;
		}
		_sceVu0SubVector( &diff, &pos, &work->camera ) ;
		diff.vy = 0.0F ;
		if ( ( len = GV_VecLen3F( &diff ) ) > 320.0F ) {
			//printf( "raiden/attack.c : herohero %f\n", len ) ;
			GV_LenVec3F( &diff, &adj, 0.0F, len - 160.0F ) ;
			work->camera.vx += adj.vx ;
			work->camera.vz += adj.vz ;
		}
		_sceVu0SubVector( &diff, &pos, &work->camera ) ;
		GV_VecToRot( &diff, &rot ) ;
		adjr = ( stance == GROUND ) ? 160 : 320 ;
		work->camdir.vx = rot.vx - adjr ;
	}	

	if ( EndMotion( work ) ) {
		if ( stance == GROUND ) work->camdir.vx = GM_CameraDir.vx = work->g_rot ;
		else					work->camdir.vx = GM_CameraDir.vx = 0 ;
		UnsetStatus( PLAYER_MENU_DISABLE ) ;
		UnsetFlag( FLAG_CANNOT_CHANGE ) ;
		SetMode( work, StillMode[ stance ] ) ;
	}
}

#if 0
static	void	SetC4Bomb( work, time )
Work		*work ;
int		time ;
{
	CAPTURE_TARGET	*enemy ;
	PL_StickWPWork	*stwp ;
	int				ftime, mtime ;

	SetFlag( FLAG_CANNOT_CHANGE | FLAG_DONOT_CHECK_WATCH | 
			 FLAG_NO_TOUCH_DAMAGE ) ;

	ftime = work->ftime ;
	stwp = &work->stick_wp_work ;
	if ( time == 0 ) {
		if ( GM_N_C4Bombs == GM_C4_MAX ||
			 GM_WeaponNum( WP_C4Bomb ) == 0 ) {
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		if ( work->stance == STAND ) {
			/* 敵付けチェック */
			SetCaptureTarget( work, TARGET_OFFENSE | TARGET_CHECK_ONE,
							 300.0F, 650.0F, 300.0F,
							 0.0F, 0.0F, 0.0F,
							 -1, 0.0F, 0.0F, 650.0F, 
							 WP_PUTC4, C4CaptureCallBack ) ;		
			return ;
		}
	}   
	if ( ftime == 1 ) {
		if ( work->offense.damaged & TARGET_CAPTURE ) {
			/* 敵付け */
			SetAction( work, c4_set_enemy, 6 ) ;
			work->capture.capture->flag |= CAPTURE_C4EXIST ;
			work->capture.capture = NULL ;
			work->data = 0 ;
		} else {
			/* 敵にはつけられない。地面設置 */
			SetAction( work, c4_set_floor, 6 ) ;

			stwp->trigger = TRIG_FIRE ;
			stwp->hzd = *work->control.level[ 0 ] ;
			stwp->target = NULL ;
			stwp->capture = NULL ;
			SetMode( work, StillMode[ work->stance ] ) ;
			work->data = 1 ;
			return ;
		}
	}
	mtime = MotionTime( work ) ;
	enemy = stwp->capture ;

	/* すいこみ */
	if ( mtime < 42 ) {
		FVECTOR	Shift = { 0.0F, 0.0F, -450.0F } ;
		FVECTOR	aim, mov ;

		DG_SetPos2( &enemy->ctrl->mov, &enemy->ctrl->rot ) ;
		DG_PutVector( &Shift, &aim, 1 ) ;
		DG_COPY_VEC( &mov, &work->control.mov ) ;
		GV_NearExp4VF( &mov, &aim, 3 ) ;
		_sceVu0SubVector( &work->control.step, &mov, &work->control.mov ) ;
		SetFlag( FLAG_NO_MOTION_STEP ) ;
		work->control.turn.vy = enemy->ctrl->rot.vy ;
	}

	if ( PL_CheckMotionTime( work, 42 ) && 
		 !( stwp->target->class & TARGET_DEAD ) ) {	/* 死んでない */
		stwp->trigger = TRIG_FIRE_C4ENEMY ;
	}
	if ( EndMotion( work ) ) {
		SetMode( work, StillMode[ work->stance ] ) ;
	}
}
#endif

/* Ｃ４壁付け */
static	void	SetC4BombWall( work, time )
Work			*work ;
int				time ;
{	
	FVECTOR		pos ;
	int			squat ;

	SetFlag( FLAG_BEHIND_PEEP ) ; /* ビハインドカメラから出ないように */
	SetStatus( PLAYER_MENU_DISABLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE ) ;

	squat = ( work->stance == SQUAT ) ? 1 : 0 ;

	if ( time == 0 ) {
		IntoCaution( work ) ;
		if ( squat ) {
			SetAction( work, c4_set_behind_sq, 6 ) ;
			work->data2 = 23 ;	/* セットするフレーム */
		} else {
			SetAction( work, c4_set_behind, 6 ) ;
			work->data2 = 24 ;	/* セットするフレーム */
		}
	}
	
	/* ２フレーム前にチェック（ＳＥ対策） */
	if ( PL_CheckMotionTime( work, work->data2 - 2 ) ) {
		SVECTOR			rot ;
		FVECTOR			shift = { 47.5F, -131.5F, 7.5F } ;

		DG_SetPos( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ;
		DG_PutVector( &shift, &pos, 1 ) ;

		/* 腰との間に壁があるとNG */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &pos,
								    HZX_CHK_SEGMENT | HZX_CHK_RECOIL_TYPE_ONLY,
								    0, 0 ) ) {
			GM_Buzzer() ;			
		} else {
			GV_SetVec3( &rot, 0, work->control.turn.vy + 2048, 0 ) ;
			if ( CheckMovRotLenSegment( work->control.hzx_id, &pos, &DG_ZeroVector, 
									   &rot, 250.0F, 
									   HZX_CHK_SEGMENT | HZX_CHK_RECOIL_TYPE_ONLY, 
									   HZX_SEG_NO_C4, 0 ) ) {	
				work->data = 1 ;
				work->stick_wp_work.hzd = ResultHzx ;
			} else {
				GM_Buzzer() ;
				//if ( squat ) SetMode( work, SquatCautionStill ) ;
				//else		 SetMode( work, StandCautionStill ) ;
				//return ;
			}
		}
	} 

	/* セット */
	if ( PL_CheckMotionTime( work, work->data2 ) && work->data == 1 ) {
		work->stick_wp_work.trigger = TRIG_FIRE_C4WALL ;
		work->stick_wp_work.target = NULL ;
		work->stick_wp_work.capture = NULL ;
	}

	if ( EndMotion( work ) ) {
		UnsetStatus( PLAYER_MENU_DISABLE ) ;
		UnsetFlag( FLAG_CANNOT_CHANGE ) ;
		if ( squat ) SetMode( work, SquatCautionStill ) ;
		else		 SetMode( work, StandCautionStill ) ;
	}
}

/* 雑誌手先ＩＫ */
static	void	SetBookHandIK( Work *work )
{
	//SVECTOR	HandAdj = { 768, 320, 380 } ;
	SVECTOR	HandAdj = { 700, 400, 400 } ;
	SVECTOR	RightArm2Adj = { 0, -480, 0 } ;
	SVECTOR	LeftArm2Adj = { 0, 80, 0 } ;

	HandAdj.vx += work->turn_adjusts[ HUMAN21_MIGI_TE ].vx ;
	SetRotAdjust( work, &HandAdj, HUMAN21_MIGI_TE ) ;
	SetRotAdjust( work, &HandAdj, HUMAN21_HIDARI_TE ) ;
	//printf( "raiden/attack.c : handadj %d\n", HandAdj.vx ) ;
	SetRotAdjust( work, &RightArm2Adj, HUMAN21_MIGI_UDE2 ) ;
	SetRotAdjust( work, &LeftArm2Adj, HUMAN21_HIDARI_UDE2 ) ;
}

/* クレイモアセット用腕ＩＫ */
static	int		SetClaymoreArmIK( Work *work, float height )
{
	FVECTOR		p1, p2, chkp2 ;
	FVECTOR		p3, chkp3 ;
	SVECTOR		rot ;
	float		levels[ 2 ], levels2[ 2 ] ;
	int			c = 1 ;
	int			flr_flag ;

	if ( work->weapon == WP_Book ) {
		flr_flag = HZX_FLOOR_IK ;
	} else {
		flr_flag = HZX_FLOOR_NO_PLAYER ;
	}

	PL_ObjPos( work, HUMAN21_KOSHI, &p1 ) ;
	PL_ObjPos( work, HUMAN21_MIGI_TE, &p2 ) ;
	PL_ObjPos( work, HUMAN21_HIDARI_TE, &p3 ) ;

	p1.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
	p2.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
	p3.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;

	levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
	levels2[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;

	//_sceVu0AddVector( &p2, &p2, &p3 ) ;
	//_sceVu0ScaleVector( &p2, &p2, 0.50F ) ;

	DG_COPY_VEC( &chkp2, &p2 ) ;
	DG_COPY_VEC( &chkp3, &p3 ) ;
	//chkp2.vy += 300.0F ;
	//chkp3.vy += 300.0F ;
	if ( work->stance != GROUND ) {
		chkp2.vy += 500.0F - height ;
		chkp3.vy += 500.0F - height ;
	} else {
		chkp2.vy += 350.0F - height ;
		chkp3.vy += 350.0F - height ;
	}
	if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &chkp2, 
							   HZX_CHK_ALL, flr_flag ) ) {
#if 0
		{	
			HZX_HZD		flr[ 2 ] ;

			HZX_GetLevelHazard( flr, NULL ) ;
			HZX_ViewFloor( &flr[ 0 ], flr[ 0 ].p4.h ) ;
		}
#endif
		HZX_GetLevelHeight( levels ) ;
	}

	if ( HZX_LevelHazardCheck( HZX_CurrentGroupID, &chkp3, 
							   HZX_CHK_ALL, flr_flag ) ) {
#if 0 
		{	
			HZX_HZD		flr[ 2 ] ;

			HZX_GetLevelHazard( flr, NULL ) ;
			HZX_ViewFloor( &flr[ 0 ], flr[ 0 ].p4.h ) ;
		}
#endif
		HZX_GetLevelHeight( levels2 ) ;
	}
	if ( work->sv.pad == 0 && levels2[ 0 ] > levels[ 0 ] ) {
		levels[ 0 ] = levels2[ 0 ] ;
		DG_COPY_VEC( &chkp2, &chkp3 ) ;
		DG_COPY_VEC( &p2, &p3 ) ;
	}

	if ( DG_FABS( work->control.levels[ 0 ] - levels[ 0 ] ) < 1.0F ) {
		if ( p2.vy > levels[ 0 ] ) {
			if ( work->weapon == WP_Book ) SetBookHandIK( work ) ;
			return 1 ;
		}
	} else if ( DG_FABS( work->control.levels[ 0 ] - levels[ 0 ] ) > 1000.0F ) {
		if ( work->weapon == WP_Book ) SetBookHandIK( work ) ;
		return 1 ;
	}

	rot = work->rot_adjusts[ HUMAN21_MUNE ] ;

	chkp2.vy = levels[ 0 ] + height ;

	if ( p2.vy < chkp2.vy ) {	
		if ( ++ work->sv.vx > DIRECT_TICK( work->sv.vz ) ) {
			c = 1 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 32.0F ) {
			rot.vx -= work->sv.vy ; c = 0 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 8.0F ) {
			rot.vx -= 16 ; c = 0 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 4.0F ) {
			rot.vx -= 4 ;  
			if ( p2.vy > levels[ 0 ] ) {
				c = 1 ;
			}
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 2.0F ) {
			rot.vx -= 2 ;  
			if ( p2.vy > levels[ 0 ] ) {	
				c = 1 ;
			}
		}
		if ( rot.vx < -768 ) {
			rot.vx = -768 ; c = 1 ;
		}
	} else {
		if ( ++ work->sv.vx > DIRECT_TICK( work->sv.vz ) ) {
			c = 1 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 32.0F ) {
			rot.vx += work->sv.vy ; c = 0 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 8.0F ) {
			rot.vx += 16 ; c = 0 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 4.0F ) {
			rot.vx += 4 ;  c = 1 ;
		} else if ( DG_FABS( p2.vy - chkp2.vy ) > 2.0F ) {
			rot.vx += 2 ;  c = 1 ;
		}
		if ( rot.vx > 64 ) {
			rot.vx = 64 ; c = 1 ;
		}
	}

	rot.vy = 0 ; 
	rot.vz = 0 ;

	SetRotAdjust( work, &rot, HUMAN21_MUNE ) ;
	SetRotAdjust( work, &rot, HUMAN21_MIGI_KATA ) ;
	SetRotAdjust( work, &rot, HUMAN21_HIDARI_KATA ) ;

	rot.vx *= -2 ; 
	rot.vx -= work->rot_adjusts[ HUMAN21_KOSHI ].vx ;
	SetRotAdjust( work, &rot, HUMAN21_MIGI_TE ) ;
	SetRotAdjust( work, &rot, HUMAN21_HIDARI_TE ) ;	

	if ( work->weapon == WP_Book ) SetBookHandIK( work ) ;

	return c ;
}

/* クレイモアセット */
static	void	SetClaymore( Work *work, int time )
{
	int		ground ;
	
	if ( PL_SubjectMove )
		SubjectMoveTurn( work ) ;

	ground = ( work->stance == GROUND ) ? 1 : 0 ;
	if ( time == 0 ) {
		/* 死体の上はダメ */
		if ( Status( PLAYER_ON_CORPSE ) ) {
			GM_Buzzer() ;			
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		/* 前方壁チェック */
		if ( EnableShootBulletEX3( work, 1, 248.0F, 950.0F, 
								  HZX_SEG_NO_PLAYER,
								  0, NULL, HZX_CHK_SEGMENT ) ) {
			GM_Buzzer() ;			
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		if ( EnableShootBulletEX3( work, 1, 748.0F, 950.0F, 
								  HZX_SEG_NO_PLAYER,
								  0, NULL, HZX_CHK_SEGMENT ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		/* 天井 */
		if ( ground && !CheckCeil2( work, 1000.0F ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		if ( ground ) {
			SetAction( work, clay_set_floor_cr_start, 6 ) ;
			work->idata = -1 ;
		} else {
			SetAction( work, clay_set_floor_start, 6 ) ;
		}
		work->idata2 = work->control.rot.vy ;
		work->sv.vx = 0 ;
		work->sv.vy = 64 ;
		work->sv.vz = 32 ;
		work->sv.pad = 0 ;
	}   

	if ( work->stance != GROUND ) GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	SetStatus( PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK | FLAG_CANNOT_PEEP ) ;

	if ( ground && Status( PLAYER_WATCH ) ) {
		PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
		work->camera.vy += 250.0F ;
	}

	if ( GV_DiffDirAbs( work->idata2, work->control.rot.vy ) > 8 ) {
		work->sv.vx = 0 ;
	}
	work->idata2 = work->control.rot.vy ;

	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			if ( ground ) {
				SetAction( work, clay_set_floor_cr_idle, 6 ) ;
			} else {
				SetAction( work, clay_set_floor_idle, 6 ) ;
			}
			work->data ++ ;
		}

		if ( ground ) {
			GroundRotateLimitControl( work ) ;
		} else {
			int		tmp ;

			tmp = work->control.turn.vy ;
			AvoidSink( work, 500.0F, 950.0F ) ;
			work->control.turn.vy = tmp ;				
		}

		if ( ground ) {
			work->camdir.vx = work->g_rot - 32 ;
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
		} else {
			if ( PL_CheckMotionRate( work ) > 0.20F ) work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			work->camdir.vx = (short) (256 + work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F);
		}
		break ;
	case 1 :
		work->stick_wp_work.trigger = TRIG_SET ;
		if ( !( work->pad->status & PL_PAD_WEAPON ) && work->data2 == 1 ) {
			FVECTOR			pos, pos2, pos3 ;	
			HZX_HZD			flr[ 2 ] ;
			float			t ;
			float			levels[ 2 ], levels2[ 2 ] ;
			int				c1, c2 ;

			PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &pos2 ) ;
			PL_ObjPos( work, HUMAN21_KUBI, &pos3 ) ;

			pos.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
			pos2.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
			pos3.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;

			levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
			levels2[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
			c1 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &pos, 
									  HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									  0 ) ;
			if ( c1 & 1 ) {	
				HZX_GetLevelHazard( flr, NULL ) ;
				if ( !( flr[ 0 ].attribute & HZX_FLOOR_NO_CLAYMORE ) ) {
					HZX_GetLevelHeight( levels ) ;				
					//HZX_ViewFloor( &flr[ 0 ], flr[ 0 ].p4.h ) ;
				}
			}
			c2 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &pos2, 
									  HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									  0 ) ;
			if ( c2 & 1 ) {
				HZX_GetLevelHazard( flr, NULL ) ;
				if ( !( flr[ 0 ].attribute & HZX_FLOOR_NO_CLAYMORE ) ) {
					HZX_GetLevelHeight( levels2 ) ;				
					//HZX_ViewFloor( &flr[ 0 ], flr[ 0 ].p4.h ) ;
				}
			}

			if ( levels2[ 0 ] > levels[ 0 ] ) levels[ 0 ] = levels2[ 0 ] ;

			/* イントルードチェック */			
			/*if ( HZX_CheckInsideTrap( work->control.hzx_id, &pos, HIDDEN_TRAP ) ||
				 HZX_CheckInsideTrap( work->control.hzx_id, &pos2, HIDDEN_TRAP ) ) {

				t = 0.60F ;
				GM_Buzzer() ;
				printf( "raiden/attack.c : intrude area!!\n" ) ;
			} else*/ if ( !ground && HZX_OnlineHazardCheck( work->control.hzx_id, &pos3, &pos,	
														 HZX_CHK_ALL,
														 HZX_SEG_NO_PLAYER, 
														 HZX_FLOOR_NO_PLAYER ) ) {
				/* 壁チェック */
				/* 右手だけでＯＫとしよう */
				t = 0.60F ;
				GM_Buzzer() ;
				printf( "raiden/attack.c : kabe ari\n" ) ;
			} else if ( ( c1 & 1 ) || ( c2 & 1 ) ) {
				/* 床チェック */
				if ( !EnableShootBulletEX3( work, 1, 248.0F, 950.0F, 
										   HZX_SEG_NO_PLAYER,
										   0, NULL, HZX_CHK_SEGMENT ) &&
					 !CheckNearEnemy( &pos, work->control.rot.vy, 768, 1000.0F, HOMING_DEAD ) &&
					 ( pos.vy - levels[ 0 ] < 200.0F ) ) {
					work->stick_wp_work.trigger = TRIG_FIRE ;
					work->stick_wp_work.hzd = flr[ 0 ] ;
					t = 0.0F ;
				} else {
					GM_Buzzer() ;
					printf( "raiden/attack.c : yuka tooi %f\n", pos.vy - levels[ 0 ] ) ;
					t = 0.60F ;
				}
			} else {
				GM_Buzzer() ;
				printf( "raiden/attack.c : yuka nashi\n" ) ;
				t = 0.60F ;
			}
			if ( ground ) {
				PL_SetAction4( work, clay_set_floor_cr_end, t, 6 ) ;
			} else {
				PL_SetAction4( work, clay_set_floor_end, t, 6 ) ;
			}
			SetFlag( FLAG_CANNOT_CHANGE ) ;
			SetStatus( PLAYER_WEAPON_DISABLE ) ;
			work->data ++ ;
		} else {
			if ( Status( PLAYER_WATCH ) ) {
				//PL_SubjectTurn( work ) ;
				PL_SeekTurn( work ) ;
			} else {
				PL_SeekTurn( work ) ;
			}
		}

		if ( ground ) {
			GroundRotateLimitControl( work ) ;
		} else {
			int		tmp ;

			if ( EnableShootBulletEX3( work, 1, 248.0F, 950.0F, 
									   HZX_SEG_NO_PLAYER,
									   0, NULL, HZX_CHK_SEGMENT ) ) {
				work->control.rot.vy = work->control.turn.vy = work->pre_turn.vy ;
			} else {
				tmp = work->control.turn.vy ;
				AvoidSink( work, 500.0F, 950.0F ) ;
				work->control.turn.vy = tmp ;	
			}
		}

		if ( ground ) {
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			work->camdir.vx = work->g_rot - 32 ;
		} else {
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			work->camdir.vx = (short) (256 + work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F) ;
		}
		break ;
	case 2 :
		SetFlag( FLAG_CANNOT_CHANGE ) ;
		SetStatus( PLAYER_WEAPON_DISABLE ) ;
		if ( ground ) {
			GroundRotateLimitControl( work ) ;
		} else {
			int		tmp ;

			tmp = work->control.turn.vy ;
			AvoidSink( work, 500.0F, 950.0F ) ;
			work->control.turn.vy = tmp ;
		}

		if ( ground ) {
			if ( PL_CheckMotionRate( work ) < 0.90F ) SetClaymoreArmIK( work, 270.0F ) ;
			work->camdir.vx = work->g_rot ;
		} else {
			if ( PL_CheckMotionRate( work ) < 0.90F ) SetClaymoreArmIK( work, 270.0F ) ;
			work->camdir.vx = (short)(256 + work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F) ;
		}
		if ( EndMotion( work ) ) {
			UnsetStatus( PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
			UnsetFlag( FLAG_CANNOT_CHANGE_ITEM | FLAG_CANNOT_CHANGE_WEAPON ) ;
			SetMode( work, StillMode[ work->stance ] ) ;
			printf( "raiden/attack.c : clay set end %d\n", work->control.rot.vy ) ;
		}
	}
}


/* 雑誌セット */
static	void	SetBook( work, time )
Work		*work ;
int			time ;
{
	int		ground ;

	ground = ( work->stance == GROUND ) ? 1 : 0 ;
	if ( time == 0 ) {
		/* 死体の上はダメ */
		if ( Status( PLAYER_ON_CORPSE ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		/* 前方壁チェック */
		if ( EnableShootBulletEX3( work, 1, 248.0F, 950.0F, 
								  HZX_SEG_NO_PLAYER,
								  0, NULL, HZX_CHK_SEGMENT ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		if ( EnableShootBulletEX3( work, 1, 748.0F, 950.0F, 
								  HZX_SEG_NO_PLAYER,
								  0, NULL, HZX_CHK_SEGMENT ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		/* 天井 */
		if ( ground && !CheckCeil2( work, 1000.0F ) ) {
			GM_Buzzer() ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
		if ( ground ) {
			SetAction( work, clay_set_floor_cr_start, 6 ) ;
			work->idata = -1 ;
		} else {
			SetAction( work, clay_set_floor_start, 6 ) ;
		}
		work->idata2 = work->control.rot.vy ;
		work->sv.vx = 0 ;
		work->sv.vy = 64 ;
		work->sv.vz = 32 ;
		work->sv.pad = 0 ;
	}   

	if ( work->stance != GROUND ) GM_ConfigControlNearCheck2( &work->control, NEAR_HEIGHT_GROUND ) ;
	SetStatus( PLAYER_ITEM_DISABLE ) ;
	SetFlag( FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK | FLAG_CANNOT_PEEP ) ;

	if ( ground && Status( PLAYER_WATCH ) ) {
		PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
		work->camera.vy += 250.0F ;
	}

	switch( work->data ) {
	case 0 :
		if ( EndMotion( work ) ) {
			if ( ground ) {
				SetAction( work, clay_set_floor_cr_idle, 6 ) ;
			} else {
				SetAction( work, clay_set_floor_idle, 6 ) ;
			}
			work->data ++ ;
		}

		if ( ground ) {
			GroundRotateLimitControl( work ) ;
			AvoidSink2( work, 500.0F, 1200.0F, 100.0F, 1300.0F ) ;
		} else {
			int		tmp ;

			tmp = work->control.turn.vy ;
			AvoidSink2( work, 500.0F, 850.0F, 100.0F, 950.0F ) ;
			work->control.turn.vy = tmp ;	
		}

		if ( ground ) {
			//work->camdir.vx = work->g_rot - 32 ;
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
		} else {
			if ( PL_CheckMotionRate( work ) > 0.20F ) work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			//work->camdir.vx = 256 + ( float )work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F;
		}
		work->stick_wp_work.trigger = TRIG_OPEN_START ;
		break ;
	case 1 :
		if ( work->data2 == 1 ) {
			FVECTOR			pos, pos2, pos3 ;	
			HZX_HZD			flr[ 2 ] ;
			float			t ;
			float			levels[ 2 ] ;
			int				c1 ;

			PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &pos2 ) ;
			PL_ObjPos( work, HUMAN21_KUBI, &pos3 ) ;

			pos.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
			pos2.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;
			pos3.vy += work->rot_adjusts[ 22 ].vy + work->rot_adjusts[ 23 ].vy ;

			levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;

			_sceVu0AddVector( &pos, &pos, &pos2 ) ;
			_sceVu0ScaleVector( &pos, &pos, 0.50F ) ;

			c1 = HZX_LevelHazardCheck( HZX_CurrentGroupID, &pos, 
									  HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY,
									  0 ) ;
			if ( c1 & 1 ) {	
				HZX_GetLevelHazard( flr, NULL ) ;
				if ( !( flr[ 0 ].attribute & HZX_FLOOR_NO_CLAYMORE ) ) {
					HZX_GetLevelHeight( levels ) ;				
				}
			}

			if ( !ground && HZX_OnlineHazardCheck( work->control.hzx_id, &pos3, &pos,	
												  HZX_CHK_ALL,
												  HZX_SEG_NO_PLAYER, 
												  HZX_FLOOR_NO_PLAYER ) ) {
				/* 壁チェック */
				/* 右手だけでＯＫとしよう */
				t = 0.60F ;
				GM_Buzzer() ;
				printf( "raiden/attack.c : kabe ari\n" ) ;
			} else if ( c1 & 1 ) {
				/* 床チェック */
				if ( !EnableShootBulletEX3( work, 1, 248.0F, 950.0F, 
										   HZX_SEG_NO_PLAYER,
										   0, NULL, HZX_CHK_SEGMENT ) &&
					 !CheckNearEnemy( &pos, work->control.rot.vy, 768, 1000.0F, HOMING_DEAD ) &&
					 pos.vy - levels[ 0 ] < 600.0F ) {
					work->stick_wp_work.trigger = TRIG_FIRE ;
					work->stick_wp_work.hzd = flr[ 0 ] ;
					t = 0.60F ;
				} else {
					GM_Buzzer() ;
					printf( "raiden/attack.c : yuka tooi %f\n", pos.vy - levels[ 0 ] ) ;
					t = 0.60F ;
				}
			} else {
				GM_Buzzer() ;
				printf( "raiden/attack.c : yuka nashi\n" ) ;
				t = 0.60F ;
			}
			if ( ground ) {
				PL_SetAction4( work, clay_set_floor_cr_end, t, 6 ) ;
			} else {
				PL_SetAction4( work, clay_set_floor_end, t, 6 ) ;
			}
			SetFlag( FLAG_CANNOT_CHANGE ) ;
			SetStatus( PLAYER_WEAPON_DISABLE ) ;
			work->data ++ ;
		} else {
			work->stick_wp_work.trigger = TRIG_OPEN_START ;
		}

		if ( ground ) {
			GroundRotateLimitControl( work ) ;
		} else {
			int		tmp ;

			tmp = work->control.turn.vy ;
			AvoidSink( work, 500.0F, 950.0F ) ;
			work->control.turn.vy = tmp ;	
		}

		if ( ground ) {
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			//work->camdir.vx = work->g_rot - 32 ;
		} else {
			work->data2 = SetClaymoreArmIK( work, 135.0F ) ;
			//work->camdir.vx = 256 + ( float )work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F ;
		}
		break ;
	case 2 :
		SetFlag( FLAG_CANNOT_CHANGE ) ;
		SetStatus( PLAYER_WEAPON_DISABLE ) ;
		if ( ground ) {
			GroundRotateLimitControl( work ) ;
		} else {
			int		tmp ;

			tmp = work->control.turn.vy ;
			AvoidSink( work, 500.0F, 950.0F ) ;
			work->control.turn.vy = tmp ;
		}

		if ( ground ) {
			if ( PL_CheckMotionRate( work ) < 0.90F ) SetClaymoreArmIK( work, 135.0F ) ;
			//work->camdir.vx = work->g_rot ;
		} else {
			if ( PL_CheckMotionRate( work ) < 0.90F ) SetClaymoreArmIK( work, 135.0F ) ;
			//work->camdir.vx = 256 + ( float )work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.00F ;
		}
		if ( EndMotion( work ) ) {
			work->camdir.vx = GM_CameraDir.vx = work->g_rot ;
			UnsetStatus( PLAYER_ITEM_DISABLE | PLAYER_WEAPON_DISABLE ) ;
			UnsetFlag( FLAG_CANNOT_CHANGE_ITEM | FLAG_CANNOT_CHANGE_WEAPON ) ;
			SetMode( work, StillMode[ work->stance ] ) ;
			return ;
		}
	}

	if ( Status( PLAYER_WATCH ) ) {
		FVECTOR		diff, adj, pos ;
		SVECTOR		rot ;
		float		len ;
		int			adjr ;

		PL_ObjPos( work, HUMAN21_MIGI_TE, &pos ) ;
		//work->camdir.vx = 380 + ( float )work->turn_adjusts[ HUMAN21_MUNE ].vx * 1.45F ;
		if ( work->camera.vy < pos.vy + 500.0F ) {
			work->camera.vy = pos.vy + 500.0F ;
		}
		_sceVu0SubVector( &diff, &pos, &work->camera ) ;
		diff.vy = 0.0F ;
		if ( ( len = GV_VecLen3F( &diff ) ) > 320.0F ) {
			GV_LenVec3F( &diff, &adj, 0.0F, len - 160.0F ) ;
			work->camera.vx += adj.vx ;
			work->camera.vz += adj.vz ;
		}
		_sceVu0SubVector( &diff, &pos, &work->camera ) ;
		GV_VecToRot( &diff, &rot ) ;
		adjr = ( ground ) ? 160 : 80 ;
		work->camdir.vx = rot.vx - adjr ;
	}	
}

/* ＰＳＧ－１ */
static	void	ShootPsg1( Work *work, int time )
{
	int		trg ;
	int		pre_udlr ;

	/* 立ちでもしゃがみでも撃てる */
	if ( time == 0 ) {
		switch( work->stance ) {
		case STAND :
			SetAction4( work, psg_fire_stand, rnd(), 0 ) ;
			work->data = DIRECT_TICK( 96 ) ;
			break ;
		case SQUAT :
			SetAction4( work, psg_fire_squat, rnd(), 0 ) ;
			work->data = DIRECT_TICK( 128 ) ;
			break ;
		case GROUND :
		case INTRUDE :
			SetAction4( work, psg_fire_crouch, rnd(), 0 ) ;
			work->data = DIRECT_TICK( 160 );
		}
		MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE * 0.125F ) ;
		PL_MotionSleep( work, 0 ) ;
		work->sv.vx = work->sv.vy = 0 ;
		GM_CameraDir.vx = work->camdir.vx = work->g_rot ;
		IntoSubject( work ) ;
		work->idata = 0 ;
		GM_SetNoise( NOISE_HOLD, &( work->control.mov ), work->control.map ) ;		
	}
	SetStatus( PLAYER_HOLD ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP | FLAG_CANNOT_CHANGE_ITEM ) ;
	if ( Status( PLAYER_GROUND ) && !Status( PLAYER_INTRUDE ) ) {
		SetFlag( FLAG_SUBJECT_HORIZON_LIMIT ) ;
	}

	/* HORIZON_LIMITだが、回転はいくらでもＯＫ */
	GM_SubjectHMaxTmp[ 0 ] = -1 ;
	GM_SubjectHMaxTmp[ 1 ] = 1 ;

	if ( PL_SubjectMove )
	    SubjectMoveTurn( work ) ;
	else
	    SubjectTurn( work ) ;

	if ( Status( PLAYER_GROUND ) && !Status( PLAYER_INTRUDE ) ) {
		SVECTOR		rot ;

		rot.vx = rot.vz = 0 ;
		rot.vy = GV_DiffDirS( work->control.rot.vy, work->camdir.vy ) ;
		SetRotAdjust( work, &rot, HUMAN21_KOSHI ) ;
	}

	   pre_udlr = work->idata ;
      // Armature fix:
      // Don't put the sniper rifle sway to sleep when pressing a DPAD direction
      // unless in a pad demo :)
      // fixes MGSTWO-2922
      if ((GM_GameStatus & STATE_PAD_DEMO))
      {
   	   work->idata = work->pad->status & PAD_UDLR ;
      }
      else
      {
         work->idata = 0;
      }
	   if ( work->idata != 0 && pre_udlr == work->idata ) {
		   /* 一定時間のＵＤＬＲ入力があった */
		   if ( ++ work->data2 > DIRECT_TICK( 12 ) ) {
			   PL_MotionSleep( work, 0 ) ;
			   if ( work->data < DIRECT_TICK( 96 ) ) work->data = DIRECT_TICK( 96 ) ;
		   }
	   } else {
		   work->data2 = 0 ;
	   }

	if ( work->data > 0 ) {
		if ( -- work->data <= 0 ) {
			if ( GM_GameLevel > GM_LEVEL_VERYEASY ) {
				PL_MotionActive( work, 0 ) ;
			}
		}
	} else {
		if ( EndMotion( work ) ) {
			work->data = DIRECT_TICK( 96 + 32 * ( work->stance + 1 ) ) ;
			PL_MotionSleep( work, 0 ) ;
		}
	}

	if ( GM_DiazepamCount <= 0 && 
		 GM_GameLevel > GM_LEVEL_VERYEASY &&
		 work->ftime > 32 ) {	/* 手ぶれ */
		FVECTOR		v = { 0.0F, -10.0F, 0.0F, 1.0F } ;
		SVECTOR		r, adj ;
		float		angle ;
		float		scale ;

		DG_SetPos( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world ) ;
		DG_RotVector( &v, &v, 1 ) ;
		GV_VecToRot( &v, &r ) ; 

		angle = GM_GetCurrentCamera( 0 )->angle ;
		scale = 0.50F ;
		if ( work->stance == SQUAT ) scale = 1.25F;
		else if ( work->stance == STAND ) scale = 1.75F ;
		if ( angle < 16.0F ) scale *= 1.5F ;	/* angle -> 8.0F ～ 64.0F */
		adj.vx = ( int )( ( float )GV_DiffDirS( work->g_rot, r.vx ) * scale * 2.5F ) ;
		adj.vy = ( int )( ( float )GV_DiffDirS( work->control.rot.vy, r.vy ) * scale ) ;
		
		adj.vx = GV_NearExp4P( work->sv.vx, adj.vx  ) ;
		adj.vy = GV_NearExp4P( work->sv.vy, adj.vy ) ;

//printf( "raiden/attack.c : %d %d - %d %d\n", r.vx, r.vy, adj.vx, adj.vy ) ;
		work->camdir.vx = work->camdir.vx - work->sv.vx + adj.vx ;
		if ( Status( PLAYER_GROUND ) && !Status( PLAYER_INTRUDE ) ) {
			work->camdir.vy = work->camdir.vy - work->sv.vy + adj.vy ;
		} else {
			work->control.turn.vy = work->control.turn.vy - work->sv.vy + adj.vy ;
		}
		work->sv.vx = adj.vx ;
		work->sv.vy = adj.vy ;
	}

	trg = TRIG_SET ;
	if ( work->pad->press & PL_PAD_WEAPON ) trg = TRIG_FIRE ;
	work->trigger = trg ;
	
	if ( work->stance == GROUND ) {
		SetFlag( FLAG_CAMERA_ON_WAIST ) ;
		GroundRotateLimitControl( work ) ;
	}

#if 0
	int		trg ;

	if ( time == 0 ) {
		StopTurn( work ) ;
		if ( ( CheckGroundWidth( work, &work->control.turn ) & 4 ) != 0 ) {
			/* 今の向きには匍匐出来ない */
			GM_Weapon = WP_None ;
			GM_WeaponChanged = 1 ;
			return ;
		}
		switch( work->stance ) {
		case STAND :
		case SQUAT :
			LeaveSubject( work ) ;
			SetAction( work, MS.shared[ Msquat_crouch ], 6 ) ;
			/* 狙撃姿勢になるまで無敵 */
			SetInvincible( work, 0 ) ;
			break ;
		case GROUND :
			IntoSubject( work ) ;
			SetAction( work, MS.change[ Mcrouch ], 6 ) ;
			work->data = 1 ;
		}
	}
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP | FLAG_NO_IK ) ;
	trg = TRIG_SET ;
	switch( work->data ) {
	case 0 :
		SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
		/* 匍匐ＩＫ */
		GroundIK2( work, 500.0F ) ;

		/* めり込み防止 */
		AvoidSink( work, NEAR_HEIGHT_GROUND, 0.0F ) ;

		if ( EndMotion( work ) ) {
			ChangeStance( work, GROUND ) ;
			IntoSubject( work ) ;
			SetAction( work, MS.change[ Mcrouch ], 6 ) ;
			UnsetInvincible( work ) ;
			work->data = 1 ;
		}
		break ;
	case 1 :
#if 0
		MatToVec( &work->body.objs->objs[ HUMAN21_MIGI_TE ].world,
				 &work->camera ) ;
#else
		/* 今は仮で腰の位置 */
		_sceVu0CopyVector( &work->camera, &work->control.mov ) ;
#endif
		GroundIK2( work, 500.0F ) ;

		if ( work->pad->press & PL_PAD_WEAPON ) trg = TRIG_FIRE ;
		SubjectTurn( work ) ;
		
		{
			int		n ;

			n = 0 ;
			if ( work->pad->status & PAD_UDLR ) {
				n = CheckGroundWidth( work, &work->control.turn ) ;
			}
			if ( n ) work->control.turn.vy = work->pre_turn.vy ;
		}
	}
	work->trigger = trg ;
#endif
}

/* ＰＳＧ１リリース関数 */
static	void	ReleasePsg1( Work *work, int time )
{
	int			res ;
	SVECTOR		rot ;

	if ( Status( PLAYER_GROUND ) && !Status( PLAYER_INTRUDE ) &&
		 work->action == GroundStill ) {
		SetAction( work, none_crouch, 0 ) ;
		work->rot_adjusts[ HUMAN21_KOSHI ].vy = 0 ;
		if ( CheckCeil( work ) ) {
			work->control.turn.vy = work->control.rot.vy = work->camdir.vy ;
			res = CheckGroundWidth3( work, &work->control.rot, 2400.0F ) ;
			if ( res != 0 ) {
				SetMode( work, SquatStill ) ;
				work->act_name = SQUAT_STILL ;
			}
		} else {
			GV_SetVec3( &rot, 0, work->camdir.vy, 0 ) ;
			res = CheckGroundWidth3( work, &rot, 2400.0F ) ;
			if ( res != 0 ) {
				/* 仕方ないので主観になったときの角度に戻す */
				/* 何もしなくていいはず */
			} else {
				work->control.turn.vy = work->control.rot.vy = work->camdir.vy ;
			}
		}
	}
	if ( PL_SubjectMove ) {
		GM_CameraDir.vx = work->camdir.vx = 0 ;
	}
}

/* スティンガー */
static	void	ShootStinger( work, time )
Work		*work ;
int		time ;
{
	int		trg ;
	float	len ;

	/* 覗き込みは出来ない（武器チェンジと被る） */
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP | FLAG_CANNOT_CHANGE_ITEM ) ;
	SetStatus( PLAYER_HOLD ) ;

	if ( time == 0 ) {
		//SetAction( work, MS.attack[ Mready ], 6 ) ;
		SetAction( work, stg_fire_p, 6 ) ;
		SetArmAction( work, AMready ) ;
		ChangeStance( work, STAND ) ;
		IntoSubject( work ) ;
		GM_SetNoise( NOISE_HOLD, &( work->control.mov ), work->control.map ) ;		
	}

	/* 壁チェック */
	EnableShootBulletEX( work, 0, 1248.0F, 950.0F, 
						 HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ;
	len = work->work_l.result_len ;
	if ( len < 800.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL2 ;
	} else if ( len < 900.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL ;
	}

	/* ダメージ対策 */
	SetAction( work, stg_fire_p, 6 ) ;
	SetArmAction( work, AMready ) ;
	SetWeaponCamera( work ) ;

	trg = TRIG_SET ;
	if ( ( work->body.objs->flag & DG_FLAG_INVISIBLE0 ) &&
		 ( work->pad->press & PL_PAD_WEAPON ) ) trg = TRIG_FIRE ;
	SubjectTurn( work ) ;
	work->trigger = trg ;

	/* 腕非表示 */
	GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;
}

/* グレネード系投げ */
static	void	ThrowGrenade( Work *work, int time )
{
	int		force ;
	int		adjg, throw_t ;
	int		pmask, mask ;
	float	len ;

	SetStatus( PLAYER_MENU_DISABLE | PLAYER_HOLD ) ;
	SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_CAUTION ) ;
	SetFlag2( FLAG2_CANNOT_CHANGE_WEAPON2 ) ;
	adjg = ( Status( PLAYER_GROUND ) ) ? 4 : 0 ;
	pmask = (int) work->body.m_ctrl->mt3_ctrl[ 1 ].mask ;
	mask = ( Status( PLAYER_MOVE | PLAYER_SQUAT ) || work->action==GroundToStand ) ? 0x01ffe : 0xffffff ;

	if ( time == 0 ) {
		if ( work->weapon != WP_Magazine ) {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start + adjg ],  0,  0.0F, 6, mask ) ;
			SetArmAction3( work, AMgm_start + adjg, 0, 6 ) ;
		} else {
			PL_SetAction2Ex( work, MS.attack[ Mgm_start + adjg ], -1, 0.80F, 6, mask ) ;
			SetArmAction4( work, AMgm_start + adjg, 0.80F, 6 ) ;
			work->trigger = TRIG_SET ;
		}
		if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
		work->data3 = work->data4 = 0 ;
		work->counter = 0 ;
		work->idata2 = 0 ;
	}

	if ( work->idata2 == 0 ) SetWeaponCamera( work ) ;

	if ( pmask != mask ) {
		work->body.m_ctrl->mt3_ctrl[ 1 ].mask = mask ;
		MT_SetMotionInterp( work->body.m_ctrl, 6 * NTSC_TIME_BASE, mask ) ;
	}

	/* そのうち実装される */
	if ( mask & 1 ) SetFlag( FLAG_MOTION_STEP_OVERRIDE ) ;

	/* パッドリリースされたら、なかったことにする */
	if ( !PL_PadEnable() || GM_CheckPlayerStatusEX( I64(0), PLAYER2_BLAST_IN_HAND ) ) {
		UnsetStatus( PLAYER_MENU_DISABLE | PLAYER_HOLD ) ;
		UnsetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_CAUTION ) ;
		UnsetFlag2( FLAG2_CANNOT_CHANGE_WEAPON2 ) ;
		UnsetWeaponCamera( work ) ;
		SetArmAction( work, AMgm_stand ) ;
		//work->motion1 = -1 ;
		SetAction( work, StanceAct( work->stance ), 6 ) ;
		SetMode2( work, NULL ) ;
		return ;
	}

	/* 壁チェック */
	EnableShootBulletEX( work, 0, 0.0F, 600.0F, 
						 HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ;
	len = work->work_l.result_len ;
	if ( len < 400.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL2 ;
	} else if ( len < 500.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL ;
	}

	switch( work->data3 ) {
	case 0 :
		if ( work->weapon != WP_Magazine &&
			 PL_CheckMotionTime2( work, 9 ) ) work->trigger = TRIG_SET ;
		if ( EndMotion2( work ) ) {
			PL_SetAction2Ex( work, MS.attack[ Mgm_5pose + adjg ], 0, 0.0F, 6, mask ) ;
			SetArmAction3( work, AMgm_5pose + adjg, 0, 6 ) ;
			if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->data3 = 1 ;
		}
		break ;
	case 3:/* 投げ始め GroundToStand */
	case 1 :
		force = ( work->pad->pressure[ PL_PAD_PRESS_WEAPON ] - 140 ) ;
		force /= ( 114 / 4 ) ;
		if ( force < 0 ) force = 0 ;
		else if ( force > 4 ) force = 4 ;
		if ( force > work->data4 ) {
			work->data4 = force ;
            work->counter = 6 ;
		}
		work->motion2 = -1 ;
		work->arm_trigger |= ARM_MOTION_SET_OVER ;
		PL_SetAction2Ex( work, MS.attack[ Mgm_5pose + adjg ], work->data4, 0.0F, work->counter, mask ) ;
		SetArmAction3( work, AMgm_5pose + adjg, work->data4, work->counter ) ;
		if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
		work->body.m_ctrl->interp_flag = 0xfffff ;
		GM_PlayerArmBody->m_ctrl->interp_flag = 0xffffff ;
		if ( work->counter > 0 ) work->counter = 0 ; 

		if ( !( work->pad->status & PL_PAD_WEAPON ) || work->data3 == 3 ) {
			work->data3 = 3 ;
			if ( work->action==GroundToStand ) break ;
			if ( work->data4 < 2 ) {
				PL_SetAction2( work, MS.attack[ Mgm_throw_min + adjg ], 6, mask ) ;
				SetArmAction3( work, AMgm_min + adjg, 0, 6 ) ;
			} else {
				PL_SetAction2( work, MS.attack[ Mgm_throw_max + adjg ], 6, mask ) ;
				SetArmAction3( work, AMgm_max + adjg, 0, 6 ) ;
			}
			if ( Status( PLAYER_SQUAT ) ) work->body.m_ctrl->old_height = work->body.m_ctrl->height ;
			work->data3 = 2 ;
		}
		break ;

	case 2 :
		work->arm_trigger |= ARM_IS_RELOAD ; /* 主観カメラ位置の関係 */
		throw_t = ( Status( PLAYER_GROUND ) ) ? 20 : 38 ;
		if ( PL_CheckMotionTime2( work, throw_t ) ) {
			work->trigger = TRIG_FIRE_LEVEL0 + work->data4 ;
		}

		throw_t = ( Status( PLAYER_GROUND ) ) ? 40 : 60 ;
		if ( PL_MotionTime2( work ) > throw_t ) {
			/* 主観腕消す */
			UnsetWeaponCameraQuick( work ) ;
			work->idata2 = 1 ;
			GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;
		}

		if ( EndMotion2( work ) ) {
			UnsetStatus( PLAYER_MENU_DISABLE | PLAYER_HOLD ) ;
			UnsetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_NO_CAUTION ) ;
			UnsetFlag2( FLAG2_CANNOT_CHANGE_WEAPON2 ) ;
			UnsetWeaponCamera( work ) ;
			SetArmAction( work, AMgm_stand ) ;
			//work->motion1 = -1 ;
			SetAction( work, StanceAct( work->stance ), 6 ) ;
			SetMode2( work, NULL ) ;
			return ;
		}
	}
}

/* スプレー系 */
static	void	JetSpray( Work *work, int time )
{
	int			trg, armact, adj ;
	float		len, len2 ;
	FVECTOR		shift = { -100.0F, -60.0F, 0.0F } ;

	trg = TRIG_SET ;
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP ) ;
	if ( time == 0 ) {
		LeaveCaution( work ) ;
		IntoSubject( work ) ;
		SetAction( work, StanceAct( work->stance ), 6 ) ;
		//SetArmAction( work, AMstand ) ;
		work->sv.vx = 0 ;
		InitSubjectStanceControl( work ) ;
		work->sv.vy = DIRECT_TICK( 40 ) ;
		work->sv.vz = 0 ;
	}

	armact = AMready ;
	adj = ( work->stance == GROUND ) ? AM_CROUCH_SHIFT : 0 ;

	if ( work->sv.vy > 0 ) -- work->sv.vy ;

	if ( work->sv.vy == 0 &&
		 work->arm_motion != AMreload + adj &&
		( work->pad->status & PL_PAD_WEAPON ) ) {
		armact = AMfire ;
		trg = TRIG_FIRE ;
		if ( work->sv.vx < DIRECT_TICK( 600 ) ) ++ work->sv.vx ;
	} else {
		if ( work->sv.vx >= DIRECT_TICK( 600 ) ) {
			armact = AMreload ;
		}		
		work->sv.vx = 0 ;
	}

	/* 壁チェック */
#if 0
	EnableShootBulletEX( work, 0, 0.0F, 750.0F, 
						 HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ;
#else
	EnableShootBulletEX2( work, 0, 0.0F, 950.0F, 
							  HZX_SEG_NO_PLAYER,
							  HZX_FLOOR_NO_PLAYER, &shift ) ;
	len = work->work_l.result_len ;
	EnableShootBulletEX2( work, 0, 0.0F, 950.0F, 
							  HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
							  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE, &shift ) ;
	len2 = work->work_l.result_len ;
	if ( len2 < len ) len = len2 ;
#endif

	if ( len < 700.0F ) {
		//armact = AMwall ;
		work->arm_trigger |= ARM_NEAR_WALL2 | ARM_IS_WALL ;
#if 0	/* 壁まえ構えは廃し */
		if ( trg == TRIG_FIRE ) {
			trg = TRIG_SET ;
			if ( work->sv.vx <= DIRECT_TICK( 600 ) ) -- work->sv.vx ;
		}
#endif
	} else if ( len < 800.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL2 ;
	} else if ( len < 900.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL ;
	}

	if ( InterpCameraLen( work->subject_camera ) < 240.0F ) work->data = 1 ;
	if ( work->data == 1 && ( ++ work->data2 > DIRECT_TICK( 0 ) ) ) {
		/* ダメージ時対策で毎フレーム実行 */
		SetWeaponCamera( work ) ;
		if ( work->arm_motion != AMreload + adj ||
			( work->arm_motion == AMreload + adj &&
			 ( work->arm_trigger & ARM_MOTION_IS_END ) ) ) {
			SetArmAction( work, armact ) ;
			if ( work->ftime < 4 ) {
				work->arm_interp = 2 ;
				work->sv.vy = 4 ;
			} else {
				work->arm_interp = 16 ;
			}
		} 
	}
#if 0
	if ( work->sv.vz == 0 && work->data == 1 && trg == TRIG_FIRE ) {
		extern	void	*NewScrDrop( int life ) ;
		int		dir ;

		dir = work->camdir.vx & 4095 ;
		if ( dir > 2048 ) dir -= 4096 ;
		if ( dir < -640 && work->pad->pressure[ PL_PAD_PRESS_WEAPON ] > 128 ) {
			work->scrdrop = NewScrDrop( 0 ) ;
			work->sv.vz = 1 ;
		}
	}
#endif
	if ( IsSubjectStanceControlCrouchToStand( work ) ) trg = TRIG_SET ;
	work->trigger = trg ;

    if ( ActSubjectStanceControl( work, mic_fire_p, -1, mic_fire_p_cr ) ) {
		work->sv.vy = 16 ;
		return ;
	}

	if ( PL_SubjectMove ) /*主観移動の場合は移動用の回転*/
	    SubjectMoveTurn( work ) ;
	else
	    SubjectTurn( work ) ;

	if ( work->stance == GROUND ) {
		SetFlag( FLAG_CAMERA_ON_WAIST ) ;
		GroundRotateLimitControl( work ) ;
		if ( Status( PLAYER_INTRUDE ) ) work->camera.vy += 100.0F ;
	}

	/* 子画面用に腕をアジャスト */
	if ( work->motion1 == mic_fire_p ) {
		SVECTOR		rot = { 0, 0, 0 } ;

		rot.vx = -496 + GM_CameraDir.vx ; rot.vy = 196 ;
		SetRotAdjust( work, &rot, HUMAN21_MIGI_UDE1 ) ;
		rot.vx = -496 ; rot.vy = 0 ;
		SetRotAdjust( work, &rot, HUMAN21_MIGI_UDE2 ) ;
		rot.vx = 892 ; rot.vy = -196 ; 
		SetRotAdjust( work, &rot, HUMAN21_MIGI_TE ) ;
	}
}

/* マイク装備 */
static	void	SetMic( work, time )
Work			*work ;
int				time ;
{
	int			armact ;
	float		len, len2 ;
	FVECTOR		shift = { -120.0F, -60.0F, 0.0F } ;

	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_PEEP ) ;
	if ( time == 0 ) {
		LeaveCaution( work ) ;
		IntoSubject( work ) ;
		SetAction( work, StanceAct( work->stance ), 6 ) ;
		//SetArmAction( work, AMready ) ;	
		//work->arm_interp = 0 ;
		InitSubjectStanceControl( work ) ;
	}
	
	/* デモマイク */
	if ( work->weapon == WP_DemoMic ) {
		GM_SetPlayerStatusEX( I64(0), PLAYER2_ARM_INVISIBLE ) ;
		GM_SubjectAngleTmp = 9.00F ;
		work->data = 1 ;
	}


	armact = AMready ;

	/* 壁チェック */
#if 0
	EnableShootBulletEX( work, 0, 0.0F, 950.0F, 
						 HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE, 
						 HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ;
#else
	EnableShootBulletEX2( work, 0, 0.0F, 1200.0F, 
						 HZX_SEG_NO_PLAYER,
						 HZX_FLOOR_NO_PLAYER, &shift ) ;
	len = work->work_l.result_len ;
	EnableShootBulletEX2( work, 0, 0.0F, 1200.0F, 
						  HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
						  HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE, &shift ) ;
	len2 = work->work_l.result_len ;
	if ( len2 < len ) len = len2 ;
#endif
	len = work->work_l.result_len ;
	if ( len < 800.0F ) {
		//armact = AMwall ;
		work->arm_trigger |= ARM_NEAR_WALL2 | ARM_IS_WALL ;
	} else if ( len < 900.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL2 ;
	} else if ( len < 1000.0F ) {
		work->arm_trigger |= ARM_NEAR_WALL ;
	}

	if ( InterpCameraLen( work->subject_camera ) < 120.0F ) work->data = 1 ;
	if ( work->data > 0 ) {
		/* ダメージ時対策 */
		SetWeaponCamera( work ) ;
		SetArmAction( work, armact ) ;
		if ( work->ftime < 4 ) work->arm_interp = 2 ;	
		else				   work->arm_interp = 16 ;
	}

    if ( ActSubjectStanceControl( work, mic_fire_p, -1, mic_fire_p_cr ) ) {
		return ;
	}

	SubjectTurn( work ) ;

	if ( work->stance == GROUND ) {
		SetFlag( FLAG_CAMERA_ON_WAIST ) ;
		GM_SubjectVMaxTmp[ 0 ] = 1 ;
		GroundRotateLimitControl( work ) ;
		if ( Status( PLAYER_INTRUDE ) ) work->camera.vy += 100.0F ;
	}
}

