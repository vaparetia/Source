/*
   force.c 
   ライデン／強制系
   
   1999/09/29 M.Sonoyama
   $Id: force.c,v 1.1.1.3 2002/11/19 11:50:55 Yoshizawa1 Exp $
   */

#include "BP_TrophySystem.h"

#ifdef KP_XBOX	// プレイヤーが出す音は遮蔽計算不必要 ADD M.Kobayashi 2002/08/01
#define GM_SeSetMode( a, b, c ) GM_SeSetModeAddr( a, b, c, GM_INVALID_ADDR )
#endif

/* 強制モーション */
static	void	ForceAct( work, time )
Work		*work ;
int		time ;
{
    FORCE		*f ;
    float		max, min, mode ;
    int			end_flag, interp ;

    f = &ForceWork ;
	end_flag = 0 ;

	mode = ( f->e_stance == FA_END_BEYOND ) ? FA_PROCMODE_ELUDE_IN : FA_PROCMODE_MOTION ;

    if ( time == 0 ) {

		//printf( "force act start!!!\n" ) ;

		PL_ModeName( work, "ForceAct" ) ;
		LeaveSubject( work ) ;
		/* Ｘ回転のリセット */
		work->control.turn.vx = work->control.rot.vx = 0 ;
		/* モーションファイルチェンジ */
		if ( !( f->flag & FA_USE_DEFAULT ) ) {
			PL_ChangeMotionArc( work, f->marfile ) ;
		} else if ( work->current_mar != work->org_motion ) {
			PL_ChangeMotionArc( work, work->org_motion ) ;
		}

		/* 装備外し */
		if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
		if ( ItemType( work ) & IT_TYPE_ABNORMAL ) {	
			if ( !( ItemType( work ) & IT_TYPE_CBBOX ) ||
				 !( f->flag & FA_BOX_EXIST ) ) {
				//printf( "force act noequip!!!\n" ) ;
				NoEquip( work ) ;
			}
		}

		/* 当たりチェック関連 */
		if ( f->flag & FA_NO_CHECK_SEG ) {
			work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
		}	
		if ( f->flag & FA_NO_CHECK_FLR ) {
			work->control.skip_flag |= CTRL_SKIP_FLR_CHECK ;
		}
		if ( f->flag & FA_NO_CHECK_TRP ) {
			work->control.skip_flag |= CTRL_SKIP_TRAP ;
		}
		if ( f->flag & FA_USE_NOW_HEIGHT ) {
			/* 無理やり今の高さからはじめる */
			SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y |
					 FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ;
		}
		interp = 6 ;
		if ( f->flag & FA_NO_INTERP ) {
			interp = 0 ;
		}
		/* 開始Ｙ座標指定があるときは以下 */
		if ( f->flag & FA_SET_START_Y ) {
			SetFlag( FLAG_NO_MOTION_STEP_Y | FLAG_NO_WAIST_INTERP ) ;
			if ( !( f->flag & FA_NO_CHECK_FLR ) ) {
				work->control.skip_flag |= CTRL_RESET_HZX_BASE ;
			}
		}
		f->flag &= ~FA_NO_INTERP ;
		if ( f->s_turn >= 0 ) {
			if ( GV_DiffDirAbs( work->control.rot.vy, f->s_turn ) > 1024 ) {
				interp = 0 ;
			} 
			SetAction( work, f->motion, interp ) ;
			work->control.rot.vy = work->control.turn.vy = f->s_turn ;
		} else {
			SetAction( work, f->motion, interp ) ;
		}
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		DG_PutObjs( work->body.objs ) ;
    }

	/* ダンボールフラグ */
	if ( ( ItemType( work ) & IT_TYPE_CBBOX ) && ( f->flag & FA_BOX_EXIST ) ) {
		SetStatus( PLAYER_CB_BOX ) ;
	}

    /* 強制モーション中は、重力計算無し */
    /* 独自の高さ更新が必要 */
	if ( !( f->flag & FA_GRAVITY_ON) ) {
	    work->control.step.vy = 0.0F ;
	}
	/* Ｙ座標指定がある場合、最初のフレームではやらない */
	if ( !( f->flag & FA_SET_START_Y ) || time > 0 ) {
		GetModelHeight( work, &max, &min ) ;    
		work->hzx_base_adjust = min - work->control.hzx_base ;
		work->hzx_height_adjust = work->control.mov.vy - min - work->body.height ;
	}

    /* 床チェックしてない場合でも影用のチェックが必要 */
    if ( f->flag & FA_NO_CHECK_FLR ) PL_LevelCheck( work ) ;

    /* 武器非表示 */
    if ( f->flag & FA_NO_WEAPON ) SetStatus( PLAYER_WEAPON_INVISIBLE ) ;

    /* フラグ関連 */
    SetFlag( FLAG_FORCE | FLAG_DONOT_CHECK_WATCH | FLAG_CANNOT_CHANGE_WEAPON | 
			FLAG_CANNOT_CHANGE_ITEM | FLAG_NO_IK | FLAG_NO_TOUCH_DAMAGE ) ;
    SetStatus( PLAYER_MENU_DISABLE ) ;

	if ( EndMotion( work ) ) f->loop -- ;
    if ( ( f->loop <= 0 ) || ( f->flag & FA_CANCEL ) ) {
		if ( f->flag & FA_STAGESTART ) {
			DG_COPY_VEC( &GM_PlayerFirstPosition, &work->control.mov ) ;
		}
		f->flag &= ~FA_CANCEL ;
		end_flag = 1 ;
		SetFlag( FLAG_FORCE_END ) ;
		//UnsetInvincible( work ) ;
		SetInvincible( work, 4 ) ;	/* 次も強制系だったときの為、４フレーム無敵 */
//		if ( f->e_turn >= 0 ) 
//			work->control.turn.vy = work->control.rot.vy = f->e_turn ;
		if ( f->e_stance == FA_END_BEYOND ) {
			/* ビヨンドモードへ */
			/* FORCE_ENDフラグは立てない */
			SetStatus( PLAYER_BEYOND ) ;
			/* 強制ステートは切る */
			UnsetStatus( PLAYER_FORCE ) ;
			UnsetFlag( FLAG_FORCE_END ) ;
			/* 再無敵 */
			SetInvincible( work, 0 ) ;
//			SetMode( work, BeyondMode ) ;
			SetMode( work, EludeStart ) ;
			GM_SetMenuStatus( MENU_MENU_OFF ) ; /* メニューが一瞬表示されるのを防止 */
		} else if ( f->e_stance == FA_END_DOWN_FACE ) {
			/* うつぶせダウン */
			work->down_dir = DOWN_FACE ;
			SetStatus( PLAYER_GROUND | PLAYER_DOWNED ) ;
			SetMode( work, Down ) ;
		} else if ( f->e_stance == FA_END_DOWN_BACK ) {
			/* あおむけダウン */
			work->down_dir = DOWN_BACK ;
			SetStatus( PLAYER_GROUND | PLAYER_DOWNED ) ;
			SetMode( work, Down ) ;
		} else if ( f->e_stance > FA_END_DOWN_BACK ) {	
			PL_PluginSet	*pls, *list ;
			int	( *init )( Work *, GV_MSG *, int ) ;

			/* プラグインリストから検索 */
			pls = NULL ;
			list = PL_PluginList.next ;
			while( list != NULL ) {
				if ( f->e_stance == list->plugin_no ) {
					pls = list ; break ;
				}
				list = list->next ;
			}
			ASSERT( pls != NULL ) ;
			init = pls->init ;
			( *init )( work, NULL, 0 ) ;
			if ( pls->action != NULL ) SetMode( work, pls->action ) ;
		} else if ( f->e_stance >= 0 ) {
			SetMode( work, StillMode[ f->e_stance ] ) ;
			SetFlag2( FLAG2_STAGESTART ) ;
			PL_ChangeStance( work, ( f->e_stance == INTRUDE ) ? GROUND : f->e_stance ) ;
			UnsetFlag2( FLAG2_STAGESTART ) ;
		} else {
			SetMode( work, StillMode[ work->stance ] ) ;
		}
		ResetAddress( work ) ;
    }
	PL_ExecForceActProc( work, mode, time, end_flag ) ;
}

/*-------------------------------------------------------------------------*/

/* ビヨンドモード */
#define	BY_PAD_ADJUST	(512)
#define	BY_BOUND_ADJ	(200.0F)

#define	BY_FALL_DAMAGE_HEIGHT	(3200)
#define	BY_FALL_DEAD_HEIGHT		(6400)

#if 0	/* 旧エルード用 */
#define	Root_height	work->fdata
#define	Fall_level	work->idata
#define	Chin_up		work->sv.vx
#define	StartPos	work->fv
#define	UnitVec		work->fv2

static	void	SetRootPosition( Work *work, FVECTOR *root, FVECTOR *unit )
{
	FVECTOR		diff, pos ;
	float		t ;

	_sceVu0SubVector( &diff, &work->control.mov, root ) ;
	t = _sceVu0InnerProduct( &diff, unit ) ;
	_sceVu0ScaleVector( &pos, unit, t ) ;
	_sceVu0AddVector( &pos, root, &pos ) ;
	GM_ResetControlPosition( &work->control, &pos ) ;
	SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
}
#endif

/*----------------------------------------------------------------*/

/* 新エルードモード */
enum {
	ELUDE_STILL = 		0x0001,
	ELUDE_MOVE_RIGHT = 	0x0002,
	ELUDE_MOVE_LEFT = 	0x0004,
	ELUDE_CHINUP = 		0x0008,
	ELUDE_RETURN = 		0x0010
} ;

/* エルード初期状態セット */
static	inline	void	EludeInitParam( Work *work )
{
	/* 開始時高さ保存 */
	work->fdata = work->control.mov.vy ;
	/* ルート位置保存 */
	DG_COPY_VEC( &work->fv, &work->control.mov ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot ) ;
	GV_SetVec3( &work->fv2, 1.0F, 0.0F, 0.0F ) ;
	DG_RotVector( &work->fv2, &work->fv2, 1 ) ;
	work->data3 = 0 ;	/* 端っこフラグ */
	work->sv.vx = 0 ;	/* 疲れフラグ */
	work->sv.vy = 0 ;	/* ダメージフラグ */
	/* 握力 */
	work->hand_power.value = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
	work->hand_power.dec = work->hand_power.value ;
	work->hand_power.prev = work->hand_power.value ;
	GM_VisibleGage( &work->hand_power ) ;
	/* メニューＯＦＦ */
	GM_SetMenuStatus( MENU_MENU_OFF ) ;
}

/* エルード終了処理 */
static	void	EludeEnd( Work *work )
{
#if defined(BP_VITA)
   GestureConfigureDeadZone(kGesture_DeadZone_Default);
#endif

	SetFlag( FLAG_FORCE_END ) ;
	GM_ResetMenuStatus( MENU_MENU_OFF ) ;
	UnsetStatus( PLAYER_BEYOND | PLAYER_FORCE ) ;
	UnsetInvincible( work ) ;
	work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
								  CTRL_SKIP_TRAP | CTRL_SKIP_MESSAGE ) ;
	ForceWork.flag &= ~FA_USE_DEFAULT ;
	Beyond.trap_id = -1 ; /* トラップ名初期化 */
	ResetAddress( work ) ;
}

/* ルート位置強制セット */
static	void	EludeSetRootPosition( Work *work, int mode ) 
{
	FVECTOR		diff, pos ;
	float		t ;

	_sceVu0SubVector( &diff, &work->control.mov, &work->fv ) ;
	t = _sceVu0InnerProduct( &diff, &work->fv2 ) ;
	_sceVu0ScaleVector( &pos, &work->fv2, t ) ;
	_sceVu0AddVector( &pos, &work->fv, &pos ) ;
	if ( mode != 0 ) {
		pos.vy = work->control.mov.vy ;
	} else {
		SetFlag( FLAG_NO_WAIST_INTERP | FLAG_NO_MOTION_STEP_Y ) ;
		if ( !Status( PLAYER_SNAKE ) ) {
			/* ライデン用微調整 */
			if ( work->action == EludeMoveRight ||
				 work->action == EludeMoveLeft ) {
				pos.vy += 58.943F ;
			}
		}
	}
	GM_ResetControlPosition( &work->control, &pos ) ;
}

/* エルード共通処理 */
static	void	EludeAction( Work *work )
{
	BEYOND		*b ;

	b = &Beyond ;
	GM_SubjectVMaxTmp[ 0 ] = 960 ;
	GM_SubjectVMaxTmp[ 1 ] = -960 ;
    /* 足影ＯＦＦ */
    if ( b->flag & BY_NOFOOTSHADOW ) {
		work->shadow = 0 ;
    }
    /* 床チェックしてない場合でも影用のチェックが必要 */
    if ( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) PL_LevelCheck( work ) ;
    /* フラグセット */
    SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_NO_IK | FLAG_SUBJECT_HORIZON_LIMIT | FLAG_CANNOT_PEEP ) ;
	SetFlag2( FLAG2_SUBJECT_MOVE ) ;
    SetStatus( PLAYER_MENU_DISABLE | PLAYER_WEAPON_INVISIBLE ) ;
//	/* 発見位置あたま */
//	PL_ObjPos( work, HUMAN21_ATAMA, &GM_PlayerFindPos ) ;
	/* 発見位置腰 */
	PL_ObjPos( work, HUMAN21_KOSHI, &GM_PlayerFindPos ) ;
    /* 重力計算なし */
    work->control.step.vy = 0.0F ;

	AddFindObj( b->object ) ;
	AddFindObj( HUMAN21_ATAMA ) ;
	AddFindObj( HUMAN21_MIGI_KAKATO ) ;
}

/* エルード妨害物チェック */
static	int		EludeDisturbCheck( Work *work, int cur )
{
#if 0	/* 没です */
	FVECTOR		tmp1, tmp2 ;
	FVECTOR		chk_pos = { 0.0F, 0.0F, 250.0F } ;
	SVECTOR		rot ;

	if ( PL_EludeDisturbFunc == NULL ) return 0 ;
	rot = DG_ZeroSVector ; rot.vy = work->control.rot.vy ;
	if ( cur == ELUDE_MOVE_RIGHT ) rot.vy -= 1024 ;
	else if ( cur == ELUDE_MOVE_LEFT ) rot.vy += 1024 ;
	else chk_pos.vz = 0.0F ;
	PL_ObjPos( work, HUMAN21_MIGI_TE, &tmp1 ) ;
	PL_ObjPos( work, HUMAN21_HIDARI_TE, &tmp2 ) ;
	tmp1.vx += tmp2.vx ; tmp1.vx /= 2.0F ;
	tmp1.vz += tmp2.vz ; tmp1.vz /= 2.0F ;
	DG_SetPos2( &tmp1, &rot ) ;
	DG_PutVector( &chk_pos, &chk_pos, 1 ) ;
	return PL_EludeDisturbFunc( &chk_pos, 300 ) ;
#else	/* 没りました */
	return 0 ;
#endif
}

/* 移動判定処理 */
static	int		EludeMoveCheck( Work *work, int cur )
{
	int				move ;
	int				dir, right, left ;
	GM_CameraSet	*cam ;
	GV_PAD			*pad ;
	BEYOND			*b ;
   
	b = &Beyond ;
	move = 0 ;
	pad = work->pad ;
   
	if ( ( pad->status & PL_PAD_BEYOND ) && 
		 !( cur & ELUDE_RETURN ) ) {
		/* 戻り */
		if ( !CheckTrap( work, NOBEYOND_TRAP ) && 
			 !EludeDisturbCheck( work, ELUDE_STILL ) ) {
			SetInvincible( work, 0 ) ;
			SetStatus( PLAYER_FORCE ) ;
			SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
			SetMode( work, EludeReturn ) ;
			return ELUDE_RETURN ;
		}
	}


	/* 移動不可エルード */
	if ( b->flag & BY_NO_MOVE_LR ) return ELUDE_STILL ;

#if defined(BP_VITA)
   pad->status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work));
#endif

	if ( (( pad->status & PAD_L2 ) && ( pad->status & PAD_R2 ) ))
   {
		if ( b->up_start != 255 ) {
			/* 懸垂 */
			if ( !( cur & ELUDE_CHINUP ) ) {
				SetMode( work, EludeChinUp ) ;
				SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
			}
			return ELUDE_CHINUP ;
		}
	}
	if ( Status( PLAYER_WATCH ) ) {
		if ( (pad->status & PAD_R2) ) move = ELUDE_MOVE_RIGHT ;
		else if ( (pad->status & PAD_L2) ) move = ELUDE_MOVE_LEFT ;		
	} else {
		if ( (pad->status & PAD_R2) ) move = ELUDE_MOVE_RIGHT ;
		else if ( (pad->status & PAD_L2) ) move = ELUDE_MOVE_LEFT ;		
		if ( !move ) {
			cam = GM_GetCurrentCameraSet( 0 ) ;
			dir = ( work->control.rot.vy - cam->rotate.vy ) & 4095 ;
			right = ( dir + 1024 ) & 4095 ;
			left = ( dir - 1024 ) & 4095 ;
			dir = pad->dir ;
			if ( dir < 0 ) {
				if ( !( cur & ELUDE_STILL ) ) {
					SetMode( work, EludeStill ) ;
					SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
				}
				return ELUDE_STILL ;
			}
			dir -= GV_GetPadOrigin() ;
			if ( GV_DiffDirAbs( right, dir ) < BY_PAD_ADJUST ) move = ELUDE_MOVE_RIGHT ;
			else if ( GV_DiffDirAbs( left, dir ) < BY_PAD_ADJUST ) move = ELUDE_MOVE_LEFT ;
		}
	}

   // AS MCampbell - Need to flip the direction the player moves if the camera is facing in almost the opposite 
   // direction.
   cam = GM_GetCurrentCameraSet(0);
   dir = fabsf(work->control.rot.vy - cam->rotate.vy);
   if ( dir > (2048 - 512) && dir < (2048 + 512) )
   {
      if (work->pad->status & PAD_L2)
      {
         work->pad->status &= ~PAD_L2;
         work->pad->status |= PAD_R2;
         move = ELUDE_MOVE_RIGHT;
      }
      else if (work->pad->status & PAD_R2)
      {
         work->pad->status &= ~PAD_R2;
         work->pad->status |= PAD_L2;
         move = ELUDE_MOVE_LEFT;
      }
   }

	if ( !move ) {
		if ( !( cur & ELUDE_STILL ) ) {
			SetMode( work, EludeStill ) ;
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		}
		return ELUDE_STILL ;
	}

	if ( move == ELUDE_MOVE_RIGHT && ( work->data3 & 0x2 ) ) {
		if ( !( cur & ELUDE_STILL ) ) {
			SetMode( work, EludeStill ) ;
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		}
		return ELUDE_STILL ;
	}
	if ( move == ELUDE_MOVE_LEFT && ( work->data3 & 0x1 ) ) {
		if ( !( cur & ELUDE_STILL ) ) {
			SetMode( work, EludeStill ) ;
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		}
		return ELUDE_STILL ;
	}
	if ( move == ELUDE_MOVE_RIGHT ) {
		if ( !( cur & ELUDE_MOVE_RIGHT ) ) SetMode( work, EludeMoveRight ) ;
	} else if ( move == ELUDE_MOVE_LEFT ) {
		if ( !( cur & ELUDE_MOVE_LEFT ) ) SetMode( work, EludeMoveLeft ) ;
	}
	return move ;
}

/* 落下判定処理 */
static	int		EludeFallCheck( Work *work )
{
	int			grip, dec ;
	int			damage ;
	BEYOND		*b ;

	b = &Beyond ;
	grip = work->hand_power.value ;
	dec = 0 ;
	damage = work->sv.vy ;
#ifdef DEBUG_MODE
	if ( !GM_DebugModeEnable || !( work->pad->status & PAD_L1 ) ) {
#endif
		if ( grip < DIRECT_TICK( ELUDE_HAND_POWER_MAX ) / 3 && 
			( GV_Time % FRAME_PER_SEC ) == 0 ) {
			if ( PL_PadEnable() ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
			work->hand_power.flag |= GM_GAGE_WARNING ;
		} else {
			work->hand_power.flag &= ~GM_GAGE_WARNING ;
		}
		if ( PL_PadEnable() ) {
			/* ライフ値によって減り方が違う */
			if ( work->power.vital == GM_VitalityMax ) {
				dec = 1 ;
			} else if ( work->power.vital > GM_VitalityMax / 3 ) {
				dec = 2 ;
			} else {
				dec = 3 ;
			}
			/* 懸垂中はいっぱい減る */
			if ( !( b->flag & BY_NO_MOVE_LR ) ) {
				if ( ( work->pad->status & PAD_L2 ) &&
					( work->pad->status & PAD_R2 ) ) dec += 2 ;
			}
		}
#ifdef DEBUG_MODE
	}
#endif
	/* 危ないエルード */
	if ( b->flag & BY_DEC_GRIP_FAST ) dec *= 4 ;

	grip -= dec ;
	if ( grip < 0 ) grip = 0 ;

	/* 握力無限カツラ */
	if ( work->item == IT_WigB ) grip = work->hand_power.max ;

	work->hand_power.value = grip ;
	if ( ( grip <= 0 ) ||				/* 握力なし */
		 ( b->flag & BY_FALL_IMMEDIATELY ) ||	/* すぐおりエルード */
		 ( ( work->pad->status & PL_PAD_BEYOND_DOWN ) &&	/* ×ボタン */
		  !( b->flag & BY_CANNOT_FALL ) ) ||
		( Status( PLAYER_DEAD ) ) ) {		/* 失血死、燃え死など */
		GM_InvisibleGage( &work->hand_power ) ;
		/* ＸＺ方向移動禁止（床ずれ防止） */
		SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_MOTION_STEP_XZ ) ;
		SetInvincible( work, 0 ) ;
		/* ＳＥ */
		if ( !Status( PLAYER_DEAD ) && !damage ) {
			if ( ( b->flag & BY_FALL_DEAD ) ) {
				/* うわぁ */
				/* 落ちたら死ぬ場所のとき */
				GM_SeSetMode( SD_V_PFALL01, &work->control.mov, GM_SEMODE_BOMB ) ;
			} else {
				/* 気合 */
				/* 上以外のとき */
				/* グリップ０でないとき */
				if ( grip > 0 ) GM_SeSetMode( SD_V_PKIAI01, &work->control.mov, GM_SEMODE_BOMB ) ;
			}
		} else {
			if ( !Status( PLAYER_DEAD ) ) {
				/* 通常ダメージ（死んではいない） */
				if ( ( b->flag & BY_FALL_DEAD ) ) {
					/* うわぁ */
					/* 落ちたら死ぬ場所のとき */
					GM_SeSetMode( SD_V_PFALL01, &work->control.mov, GM_SEMODE_BOMB ) ;
				}
			}
		}
		/* ここでやらないと grounded を 0 にできない */
		work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ; 
		SetMode( work, EludeFall ) ;
		return 1 ;
	}
	return 0 ;
}

/* ダメージ判定処理 */
static	int		EludeDamageCheck( Work *work )
{
	int			damage ;

	damage = 0 ;
	if ( Status( PLAYER_DAMAGED ) ) {
		damage = 1 ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		if ( work->post & MAIL_ELUDE_STAMP ) damage = 2 ;
		work->post &= ~MAIL_ELUDE_STAMP ;
	}	
	work->sv.vy = damage ;
	if ( damage ) {
		if ( work->hand_power.value <= 0 || Status( PLAYER_DEAD ) ) {
			return 0 ; /* 落ちる */
		}
		SetInvincible( work, 0 ) ;
		SetMode( work, EludeDamage ) ;
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		work->data = damage ;
	}
	return damage ;
}

/* エルード領域端までの距離 */
static	float	GetEludeEdgeLen( Work *work, int cur )
{
	BEYOND		*b ;
	FVECTOR		chk_pos, tmp_pos ;
	FVECTOR		bound1, bound2, diff ;
	//FVECTOR		adj ;
	HZX_BLOCK	*blk ;
	HZX_TRP		*traps, *trp ;
	int			i, dir ;
	float		len ;

	/* トラップ検索 */
	b = &Beyond ;
	PL_ObjPos( work, HUMAN21_MIGI_TE, &chk_pos ) ;
	PL_ObjPos( work, HUMAN21_HIDARI_TE, &tmp_pos ) ;
	chk_pos.vx += tmp_pos.vx ;	chk_pos.vx /= 2.0F ;
	chk_pos.vz += tmp_pos.vz ;  chk_pos.vz /= 2.0F ;
	
	/* 少し下げる */
	//GV_SetVec3( &adj, 0.0F, 0.0F, -5.0F ) ;
	//DG_SetPos2( &chk_pos, &work->control.rot ) ;
	//DG_PutVector( &adj, &chk_pos, 1 ) ;

	blk = HZX_GetInsideBlock( work->control.hzx_id, &chk_pos ) ;
	ASSERT( blk != NULL ) ;
	trp = NULL ;
	traps = blk->traps ;
	for ( i = 0; i < blk->n_traps; i ++, traps ++ ) {
		if ( traps->name_id == b->trap_id ) {
			trp = traps ;
			break ;
		}
	}
	if ( trp != NULL ) {
		bound1.vx = trp->b1.vx + blk->tx ;
		bound1.vy = trp->b1.vy + blk->ty ;
		bound1.vz = trp->b1.vz + blk->tz ;
		bound2.vx = trp->b2.vx + blk->tx ;
		bound2.vy = trp->b2.vy + blk->ty ;
		bound2.vz = trp->b2.vz + blk->tz ;
		if ( b->flag & BY_X_RANGE_EXIST ) {
			bound1.vx = ( bound1.vx > b->x_min ) ? bound1.vx : b->x_min ;
			bound2.vx = ( bound2.vx < b->x_max ) ? bound2.vx : b->x_max ;
		}
		if ( b->flag & BY_Z_RANGE_EXIST ) {
			bound1.vz = ( bound1.vz > b->z_min ) ? bound1.vz : b->z_min ;
			bound2.vz = ( bound2.vz < b->z_max ) ? bound2.vz : b->z_max ;
		}
#ifdef DEBUG_MODE
		if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) {
			NewBoundingBoxView( &bound1, &bound2, 32, 232, 32 ) ;
		}
#endif
		if ( cur == ELUDE_MOVE_RIGHT ) dir = work->control.rot.vy - 1024 ;
		else 						   dir = work->control.rot.vy + 1024 ;
		len = BoundingEdgeLen( &chk_pos, dir, &bound1, &bound2, &diff ) ;	
		return len ;
	} else {
#ifdef DEBUG_MODE
		printf( "trap not found %d\n", b->trap_id ) ;
		DumpVec( &chk_pos ) ;
#endif
	}
	return 0.0F ;	/* バウンディング求められない。*/
}

/* 移動領域外での反対方向移動量をキャンセルする */
static	void	EludeBackMoveCancel( Work *work, int cur )
{
	float		len, ip ;
	FVECTOR		v = { 0.0F, 0.0F, 1.0F }, diff ;
	SVECTOR		rot ;

	if ( cur == ELUDE_MOVE_RIGHT ) len = GetEludeEdgeLen( work, ELUDE_MOVE_LEFT ) ;
	else 						   len = GetEludeEdgeLen( work, ELUDE_MOVE_RIGHT ) ;
	if ( len >= BY_BOUND_ADJ ) return ;

	rot.vx = rot.vz = 0 ;
	if ( cur == ELUDE_MOVE_RIGHT ) rot.vy = work->control.rot.vy - 1024 ;
	else						   rot.vy = work->control.rot.vy + 1024 ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_RotVector( &v, &v, 1 ) ;
	
	_sceVu0SubVector( &diff, &work->control.mov, &work->pre_mov ) ;	
	diff.vy = 0.0F ;

	ip = _sceVu0InnerProduct( &diff, &v ) ;
	if ( ip < 0.0F ) {
		printf( "back move cancel!\n" ) ;
		//_sceVu0ScaleVector( &diff, &diff, -1.0F ) ;
		_sceVu0ScaleVector( &diff, &v, ip ) ;
		work->control.mov.vx -= diff.vx ;
		work->control.mov.vz -= diff.vz ;
		PL_AdjustXZ( work, &diff ) ;
		//SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	}
}

/* 移動領域境界判定 */
static	int		EludeLimitCheck( Work *work, int cur )
{
	float		len1 ;

	len1 = GetEludeEdgeLen( work, cur ) ;
	if ( len1 < 0.0F ) printf( "warning : outside of elude area\n" ) ;
	if ( len1 < BY_BOUND_ADJ ) {
		/* 端っこフラグセット */
		if ( cur == ELUDE_MOVE_RIGHT ) work->data3 |= 0x2 ;
		else  						   work->data3 |= 0x1 ;
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		SetMode( work, EludeStill ) ;
		return 1 ;
	}
	/* 妨害物チェック */
	if ( EludeDisturbCheck( work, cur ) ) {
		/* 端っこフラグセット */
		if ( cur == ELUDE_MOVE_RIGHT ) work->data3 |= 0x2 ;
		else 						   work->data3 |= 0x1 ;
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		SetMode( work, EludeStill ) ;
		return 1 ;		
	}
	return 0 ;
}

/* 懸垂回数カウント */
static	void	EludeUpdateChinUpData( Work *work ) 
{
	int			grip ;

	grip = work->hand_power.value ;
	if ( grip < 0 ) grip = 0 ;

   //AndyO: Removed in MGS_2_and_3_Trophy_Info_v2_110527_rev.xls
   //BP_TrophySystem_UnlockTrophy( kTRP_DoPullUp );

	if ( Status( PLAYER_SNAKE ) ) {
		if ( GM_SnakeChin_Up > 30000 ) {
			GM_SnakeChin_Up = 30000 ;
			return ;
		}
		GM_SnakeChin_Up ++ ;
		if ( GM_SnakeChin_Up >= ELUDE_HAND_POWER_LEVELUP2 &&
			 ELUDE_HAND_POWER_MAX < GM_SnakeGripMax[ 2 ] ) {
			ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 2 ] ;
			grip = grip * GM_SnakeGripMax[ 2 ] / GM_SnakeGripMax[ 1 ] ;
			work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
			strcpy( work->hand_power.name, "GRIP Lv3" ) ;
			work->hand_power.r[ 1 ] = 90 ;
			work->hand_power.g[ 1 ] = 90 ;
			work->hand_power.b[ 1 ] = 92 ;
			work->hand_power.r[ 2 ] = 150 ;
			work->hand_power.g[ 2 ] = 150 ;
			work->hand_power.b[ 2 ] = 152 ;
         BP_TrophySystem_UnlockTrophy( kTRP_GripLevel3 );
		} else if ( GM_SnakeChin_Up >= ELUDE_HAND_POWER_LEVELUP1 &&
				    ELUDE_HAND_POWER_MAX < GM_SnakeGripMax[ 1 ] ) {
			ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 1 ] ;
			grip = grip * GM_SnakeGripMax[ 1 ] / GM_SnakeGripMax[ 0 ] ;
			work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
			strcpy( work->hand_power.name, "GRIP Lv2" ) ;
			work->hand_power.r[ 1 ] = 191 ;
			work->hand_power.g[ 1 ] = 102 ;
			work->hand_power.b[ 1 ] = 0 ;
			work->hand_power.r[ 2 ] = 220 ;
			work->hand_power.g[ 2 ] = 227 ;
			work->hand_power.b[ 2 ] = 100 ;
		}
	} else {
		if ( GM_RaidenChin_Up > 30000 ) {
			GM_RaidenChin_Up = 30000 ;
			return ;
		}
		GM_RaidenChin_Up ++ ;
		if ( GM_RaidenChin_Up >= ELUDE_HAND_POWER_LEVELUP2 &&
			 ELUDE_HAND_POWER_MAX < GM_RaidenGripMax[ 2 ] ) {
			ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 2 ] ;
			grip = grip * GM_RaidenGripMax[ 2 ] / GM_RaidenGripMax[ 1 ] ;
			work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
			strcpy( work->hand_power.name, "GRIP Lv3" ) ;
			work->hand_power.r[ 1 ] = 90 ;
			work->hand_power.g[ 1 ] = 90 ;
			work->hand_power.b[ 1 ] = 92 ;
			work->hand_power.r[ 2 ] = 150 ;
			work->hand_power.g[ 2 ] = 150 ;
			work->hand_power.b[ 2 ] = 152 ;
         BP_TrophySystem_UnlockTrophy( kTRP_GripLevel3 );
		} else if ( GM_RaidenChin_Up >= ELUDE_HAND_POWER_LEVELUP1 &&
				    ELUDE_HAND_POWER_MAX < GM_RaidenGripMax[ 1 ] ) {
			ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 1 ] ;
			grip = grip * GM_RaidenGripMax[ 1 ] / GM_RaidenGripMax[ 0 ] ;
			work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
			strcpy( work->hand_power.name, "GRIP Lv2" ) ;
			work->hand_power.r[ 1 ] = 191 ;
			work->hand_power.g[ 1 ] = 102 ;
			work->hand_power.b[ 1 ] = 0 ;
			work->hand_power.r[ 2 ] = 220 ;
			work->hand_power.g[ 2 ] = 227 ;
			work->hand_power.b[ 2 ] = 100 ;
		}
	}
	work->hand_power.value = grip ;
}

/*----------------------------------------------------------------*/

/* エルード開始 */
static	void	EludeStart( Work *work, int time )
{
	BEYOND		*b ;

#if defined(BP_VITA)
   GestureConfigureDeadZone(kGesture_DeadZone_None);
#endif

	b = &Beyond ;
	UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;
	LeaveSubject( work ) ;
	/* 必ずモーションファイルチェンジ */
	PL_ChangeMotionArc( work, ForceWork.marfile ) ;
	/* 当たりチェック関連 */
	work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
		CTRL_SKIP_MESSAGE ;
	if ( b->s_turn >= 0 ) {
		if ( GV_DiffDirAbs( b->s_turn, work->control.rot.vy ) > 1024 ) {
			SetAction( work, b->still, 0 ) ;
		} else {
			SetAction( work, b->still, 6 ) ;
		}
		work->control.rot.vy = work->control.turn.vy = b->s_turn ;
	} else {
		SetAction( work, b->still, 6 ) ;
	}
	/* トラップ検索 */
	if ( b->trap_id < 0 ) {
		int	n_inside ;

		n_inside = work->control.evt.n_inside ;
		ASSERT( n_inside > 0 ) ;
		b->trap_id = *( work->control.evt.inside ) ;
	}
	printf( "beyond trap_id %d\n", b->trap_id ) ;
	/* 初期状態セット */
	EludeInitParam( work ) ;
	/* 無敵解除 */
	UnsetInvincible( work ) ;
	/* エルード共通処理 */
	EludeAction( work ) ;
	SetMode( work, EludeStill ) ;
}

/* エルード戻り */
static	void	EludeReturn( Work *work, int time )
{
	BEYOND		*b ;
	FVECTOR		wall_obj ;
	int			end_flag = 0 ;
	int			stamp_s, stamp_e, mtime ;
	float		left_len, len ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		GM_InvisibleGage( &work->hand_power ) ;
		LeaveSubject( work ) ;
		/* 壁チェック */
		GV_MatToVec( &( work->body.objs->objs[ b->wall_obj ].world ),
					 &wall_obj ) ;
		/* ちぇっく位置が壁の中 */
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &wall_obj,
								    HZX_CHK_ALL, HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
			HZX_GetOnlineVector( &wall_obj ) ;
			len = GV_VecLen3F( &wall_obj ) ;
			if ( len < 5.0F ) {
				DG_COPY_VEC( &wall_obj, &DG_ZeroVector ) ;
			} else {
				GV_LenVec3F( &wall_obj, &wall_obj, 0.0F, len - 4.0F ) ;
			}
			_sceVu0AddVector( &wall_obj, &work->control.mov, &wall_obj ) ;
		}
		if ( CheckMovRotLenSegment( work->control.hzx_id,
								    &wall_obj, &DG_ZeroVector, 	
								    &work->control.rot, 750.0,
								    HZX_CHK_SEGMENT,
								    HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
			SetAction( work, b->back2, 6 ) ;
		} else {
			SetAction( work, b->back, 6 ) ;
		}
		/* 左端チェック */
		left_len = GetEludeEdgeLen( work, ELUDE_MOVE_LEFT ) ;
		if ( left_len < 1000.0F ) {
			work->body.m_ctrl->flag |= ( MT_FLAG_REVERSAL1 | MT_FLAG_REVERSAL2 ) ;
		}
		/* もうやってあるが念のため */
		SetInvincible( work, 0 ) ;
		SetStatus( PLAYER_FORCE ) ;
		work->act_name = ELUDE_RETURN_MODE ;
		work->sv.vz = 0 ;
		work->idata = 0 ;
	} 

	mtime = PL_MotionTime( work ) ;

	/* ふみつけ */
	if ( Status( PLAYER_SNAKE ) ) {
		stamp_s = 90 ;
		stamp_e = 100 ;
	} else {
		stamp_s = 70 ;
		stamp_e = 75 ;
	}
	if ( !( b->flag & BY_NO_RETURNSTAMP ) &&
		 mtime > stamp_s && mtime < stamp_e ) {
		FVECTOR			size ;

		size.vx = size.vy = size.vz = 240.0F ;
		GM_SetTarget( &work->offense, TARGET_OFFENSE | TARGET_POWER, 0,
					  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->offense, WP_KICK ) ;
		GM_MoveTarget3( &work->offense, ObjWorld( work, HUMAN21_MIGI_KAKATO ) ) ;
		GM_PutTarget( &work->offense ) ;
//		NewTargetView2( &work->offense, 32, 232, 32 ) ;
	}

	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	EludeAction( work ) ;

	/* 足場が無い（ライデン） */
	if ( !Status( PLAYER_SNAKE ) ) {
		if ( work->sv.vz == 0 && mtime > 64 && 
			 ExistSeCodeNext( work, 0 ) >= 0 ) {
			FVECTOR		front = { 0.0F, 0.0F, 240.0F } ;

			printf( "yuka check %d\n", mtime ) ;
			work->sv.vz = -1 ;
			DG_SetPos2( &work->control.mov, &work->control.rot ) ;
			DG_PutVector( &front, &front, 1 ) ;
			//NewPointView( &front, 32, 32, 232 ) ;
			if ( HZX_LevelHazardCheck( work->control.hzx_id, &front, HZX_CHK_ALL,
									   HZX_FLOOR_NO_PLAYER | HZX_FLOOR_PITFALL ) & 1 ) {
				float		levels[ 2 ] ;

				HZX_GetLevelHeight( levels ) ;
				//printf( "yuka ari %f %f\n", front.vy, levels[ 0 ] ) ;
				if ( front.vy - levels[ 0 ] > 4000.0F ) {
					work->sv.vz = work->idata = mtime ;
					work->control.grounded = 0 ;
					PL_MotionSleep( work, 0 ) ;					
					/* トラップチェック位置腰 */
					work->control.root_offset = NULL ;
				}
			} else {
				work->sv.vz = work->idata = mtime ;
				work->control.grounded = 0 ;
				PL_MotionSleep( work, 0 ) ;
				/* トラップチェック位置腰 */
				work->control.root_offset = NULL ;
			}
		}
		if ( work->sv.vz > 0 ) {
			work->sv.vz ++ ;
			printf( "yuka nashi elude return %x\n", work->control.grounded ) ;
			work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
			work->control.step.vy = - ( ( float )( work->sv.vz - work->idata ) * GRAVITY ) ;
			/* トラップチェック位置腰 */
			work->control.root_offset = NULL ;
#if 1
			/* 床なしのはずだが、床がある */
			if ( work->control.grounded & 1 ) {
				if ( !( work->control.flr_atrs[ 0 ] & HZX_FLOOR_PITFALL ) ) {
					printf( "yuka arujann!!\n" ) ;
					PL_MotionActive( work, 0 ) ;
					work->sv.vz = -1 ;
				}
			}
#endif
		}
	}

	if ( EndMotion( work ) ) {
		EludeEnd( work ) ;
		end_flag = 1 ;
		SetMode( work, StandStill ) ;
		if ( b->e_turn >= 0 ) {
			ForceWork.e_turn = b->e_turn ;
		} else {
			ForceWork.e_turn = -1 ;
		}
		if ( b->flag & BY_RESET_HEIGHT ) {
			GM_ControlResetHeightMotion( &work->control ) ;
			PL_ChangeMotionArc( work, work->org_motion ) ;
			SetAction( work, MS.change[ Mstand ], 2 ) ;
			b->flag &= ~BY_RESET_HEIGHT ;
		}
	}
	PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_OUT, time, end_flag ) ;
}

/* エルード静止 */
static	void	EludeStill( Work *work, int time )
{
	BEYOND		*b ;
	int			still ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		work->act_name = ELUDE_STILL_MODE ;
	}
	/* 静止モーションはどっち？ */
	if ( work->hand_power.value <= ELUDE_HAND_POWER_MAX / 3 ) {
		still = b->tired ;
	} else {
		still = b->still ;
	}
	/* 疲れたとき */
	if ( still == b->tired &&
		 b->still != b->tired &&
		 work->sv.vx == 0 ) {	
		work->sv.vx = 1 ;
		GM_SeSetMode( SD_V_PELUDE01, &work->control.mov, GM_SEMODE_BOMB ) ;
	}
	SetAction( work, still, 6 ) ;
	EludeAction( work ) ;
	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	} 
	if ( !EludeDamageCheck( work ) && !EludeFallCheck( work ) ) {
		EludeMoveCheck( work, ELUDE_STILL ) ;
	}
}

/* エルードダメージ */
static	void	EludeDamage( Work *work, int time )
{
	BEYOND		*b ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		SetAction( work, b->damage, 6 ) ;
		work->act_name = ELUDE_STILL_MODE ;
	}
	EludeAction( work ) ;

	EludeBackMoveCancel( work, ELUDE_MOVE_RIGHT ) ;
	EludeBackMoveCancel( work, ELUDE_MOVE_LEFT ) ;

	if ( EndMotion( work ) ) {
		UnsetInvincible( work ) ;
		if ( work->data == 1 ) {
			/* 無敵セット（ふまれのときは無敵時間なし） */
			SetInvincible( work, INVINCIBLE_TIME ) ;
		}
		SetMode( work, EludeStill ) ;
	}
}

/* エルード右移動 */
static	void	EludeMoveRight( Work *work, int time )
{
	BEYOND		*b ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		work->data3 &= ~0x1 ; /* 左端っこフラグ解除 */
		SetAction( work, b->move1, 6 ) ;
		work->act_name = ELUDE_STILL_MODE ;
	} 
	EludeAction( work ) ;
	EludeBackMoveCancel( work, ELUDE_MOVE_RIGHT ) ;

	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	} 
	if ( !EludeDamageCheck( work ) &&
		 !EludeFallCheck( work ) &&
		 !EludeLimitCheck( work, ELUDE_MOVE_RIGHT ) ) { /* 移動領域チェック */
		EludeMoveCheck( work, ELUDE_MOVE_RIGHT ) ;
	} else {
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	}
}

/* エルード左移動 */
static	void	EludeMoveLeft( Work *work, int time )
{
	BEYOND		*b ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		work->data3 &= ~0x2 ; /* 右端っこフラグ解除 */
		SetAction( work, b->move2, 6 ) ;
		work->act_name = ELUDE_STILL_MODE ;
	} 
	EludeAction( work ) ;
	EludeBackMoveCancel( work, ELUDE_MOVE_LEFT ) ;

	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	} 
	if ( !EludeDamageCheck( work ) &&
		 !EludeFallCheck( work ) &&
		 !EludeLimitCheck( work, ELUDE_MOVE_LEFT ) ) { /* 移動領域チェック */
		EludeMoveCheck( work, ELUDE_MOVE_LEFT ) ;
	} else {
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
	}
}

/* エルード落下 */
static	void	EludeFall( Work *work, int time )
{
	BEYOND		*b ;
	FVECTOR		size ;
	FVECTOR		mov ;
	FVECTOR		vecs[ 2 ] ;
	int			n_touches, flag ;
	float	    pre_floor, fall_height ;

	b = &Beyond ;
	if ( time == 0 ) {
		work->act_name = ELUDE_FALL ;
		SetAction( work, b->fall_start, 6 ) ;
		work->data2 = -ELUDE_FALL_SPEED_FIRST ;
	}

	if ( GM_IsGameOver() || ( b->flag & BY_FALL_DEAD ) ) {
		PL_LeaveSubject( work ) ;
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	}

	GM_SetPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ;
	PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_FALL, time, 0 ) ;
	/* トラップチェック位置腰 */
	work->control.root_offset = NULL ;

	EludeAction( work ) ;

	/* 死んで、落ちすぎのときはゲームオーバーエンド */
	if ( Status( PLAYER_DEAD ) &&
		 work->fdata - work->control.mov.vy > 80000.0F ) {	/* 80m */
		GM_GameOverProcEnd( &work->actor ) ;
		GM_ResetPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ;
		return ;
	}

	/* ふみつけ */
#if 0
	if ( !( b->flag & BY_NO_STAMP ) && ( work->control.grounded & 1 ) ) {
#else
	if ( !( b->flag & BY_NO_STAMP ) && ( work->control.mov.vy - work->control.levels[ 0 ] < 3500.0F ) ) {
#endif
		if ( Status( PLAYER_SNAKE ) ) {
			size.vx = size.vz = 240.0F ;
			size.vy = 750.0F ;
		} else {
			GV_SetVec3( &size, 320.0F, 750.0F, 320.0F ) ;
		}
		GM_SetTarget( &work->offense, TARGET_OFFENSE | TARGET_POWER, 0,
					  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
		GM_SetTargetWeaponType( &work->offense, WP_STAMP ) ;
		GM_MoveTarget3( &work->offense, ObjWorld( work, HUMAN21_MIGI_KAKATO ) ) ;
		GM_PutTarget( &work->offense ) ;
//		NewTargetView2( &work->offense, 32, 232, 32 ) ;
	}

	if ( work->motion1 == b->fall_start && EndMotion( work ) ) {
		SetAction( work, b->fall_loop, 6 ) ;
	}
#if 0
	SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
			 FLAG_NO_MOTION_STEP_XZ ) ;
#else
	SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_MOTION_STEP_XZ ) ;
	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	}
#endif
	if ( work->control.mov.vy > GM_WaterLevel - 1000.0F ) work->data2 -= ELUDE_FALL_SPEED ;
	else												  work->data2 = -ELUDE_FALL_SPEED_FIRST ;

	work->control.step.vy = ( float )work->data2 ;

	if ( ( work->control.grounded & 1 ) || work->control.mov.vy <= GM_WORLD_LIMIT_BOTTOM ) {
		/* 壁の真上に落ちないようにする */
		if ( work->control.grounded & 1 ) {
			DG_COPY_VEC( &mov, &work->control.mov ) ;
			mov.vy = work->control.levels[ 0 ] + 750.0F ;
			n_touches = HZX_NearHazardCheck( work->control.hzx_id, &mov, 500.0F,
											 HZX_CHK_ALL, HZX_SEG_NO_PLAYER, 450.0F ) ;
			if ( n_touches > 0 ) {
				HZX_GetNearVector( vecs ) ;
				//DumpVec( &vecs[ 0 ] ) ;
				if ( ( DG_FABS( vecs[ 0 ].vx ) < 1.000F ) &&
					 ( DG_FABS( vecs[ 0 ].vz ) < 1.000F ) ) {
					/* 壁上に落ちてしまった */	
					/* ちょっと位置をずらす */
					printf( "warning : wall wall\n" ) ;
					DumpVec( &vecs[ 0 ] ) ;
					DumpVec( &mov ) ;
					GV_GetShiftPos( &mov, &work->control.mov, &work->control.rot,
								    4.0F, 0.0F, 0.0F ) ;
					pre_floor = work->control.levels[ 0 ] ;
					GM_ResetControlPosition( &work->control, &mov ) ;
					/* 床チェックしなおし */
					flag = HZX_LevelHazardCheck( work->control.hzx_id, 
												 &mov, work->control.hzx_check_type, 
												 work->control.flr_flag ) ;
					if ( flag != 0 ) {
						HZX_GetLevelHeight( work->control.levels ) ;
						HZX_GetLevelHazard( work->control.level[ 0 ], work->control.flr_atrs ) ;
					} 
					if ( !( flag & 1 ) ) work->control.levels[ 0 ] = GM_WORLD_LIMIT_BOTTOM ;
					if ( !( flag & 2 ) ) work->control.levels[ 1 ] = GM_WORLD_LIMIT_UPPER ;
					/* 再チェックの結果、床の高さが変わってしまった */
					if ( pre_floor > work->control.levels[ 0 ] ) {
						return ;
					}
				}
			}
		}
		fall_height = work->fdata - work->control.mov.vy ;
		if ( Status( PLAYER_DEAD ) || ( b->flag & BY_FALL_DEAD ) ) {
			fall_height = BY_FALL_DEAD_HEIGHT + 1 ; /* 無理矢理殺す */
		}			
		if ( fall_height < BY_FALL_DAMAGE_HEIGHT ) {
			SetMode( work, EludeTouchDownSafe ) ;
			GM_ResetPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ;
		} else if ( fall_height < BY_FALL_DEAD_HEIGHT ) {
			/* ダメージ */
			work->power.vital -= ( ELUDE_FALL_DAMAGE * ( ( int )fall_height - 2000 ) / 1000 ) ;
			/* 死なないように */
			if ( work->power.vital <= 0 ) work->power.vital = 1 ;
			/* ゲージを出す。（後でちゃんと直そう） */
			work->gagetime = 0 ;
			if ( !GM_CheckMenuStatus( MENU_MENU_OPEN ) ) {
				GM_GageStatus = GM_GAGE_STATE_APPEAR ;
			}
			SetMode( work, EludeTouchDownDamage ) ;
			GM_ResetPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ;
		} else {
			SetMode( work, EludeTouchDownDead ) ;
			GM_ResetPlayerStatusEX( I64(0), PLAYER2_ELUDE_FALL ) ;
		}
		ResetAddress( work ) ;
	}
}

/* エルード着地セーフ */
static	void	EludeTouchDownSafe( Work *work, int time )
{
	BEYOND		*b ;
	int			end_flag = 0;

	b = &Beyond ;
	if ( time == 0 ) {
		SetAction( work, b->fall_safe, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		//NewPadVibration( DamageVib1H, 1 ) ;
		//NewPadVibration( DamageVib1L, 2 ) ;
		work->body.m_ctrl->old_height = work->control.height ;
		work->body.m_ctrl->root_old_height = work->control.height ;
		work->act_name = ELUDE_TOUCHDOWN ;
	}

	EludeAction( work ) ;
	UnsetFlag( FLAG_NO_IK ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;

	work->control.step.vy = -GRAVITY ;

	if ( EndMotion( work ) ) {
		end_flag = 1 ;
		EludeEnd( work ) ;
		SetMode( work, StandStill ) ;
	}
	PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_FALL, 1, end_flag ) ;
}

/* エルード着地ダメージ */
static	void	EludeTouchDownDamage( Work *work, int time )
{
	BEYOND		*b ;
	int			end_flag = 0 ;

	b = &Beyond ;
	if ( time == 0 ) {
		SetAction( work, b->fall_damage, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		//NewPadVibration( DamageVib2H, 1 ) ;
		//NewPadVibration( DamageVib2L, 2 ) ;
		work->body.m_ctrl->old_height = work->control.height ;
		work->body.m_ctrl->root_old_height = work->control.height ;
		work->act_name = ELUDE_TOUCHDOWN ;
	}
	if ( PL_CheckMotionTime( work, 10 ) ) {
		GM_SeSetMode( SD_V_PDMG03, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;
	}
	EludeAction( work ) ;
	SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
	UnsetFlag( FLAG_NO_IK ) ;

	work->control.step.vy = -GRAVITY ;

	if ( EndMotion( work ) ) {
		end_flag = 1 ;
		EludeEnd( work ) ;
		SetMode( work, StandStill ) ;
	}
	PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_FALL, 1, end_flag ) ;
}

/* エルード着地死亡 */
static	void	EludeTouchDownDead( Work *work, int time )
{
	BEYOND		*b ;
	FVECTOR		pos, force ;

	b = &Beyond ;
	if ( time == 0 ) {
		SetAction( work, b->fall_out, 6 ) ;
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;

		NewPadVibration( DamageVib2H, 1 ) ;
		NewPadVibration( DamageVib2L, 2 ) ;
		
		/* 敵がこっちを向く */
		GM_SetNoise( NOISE_M, &work->control.mov, work->control.map ) ;

		GM_GameOverProcStart( &work->actor ) ;
		LeaveSubject( work ) ;
		if ( !Status( PLAYER_DEAD ) ) {
			/* 撃たれて死んだときはここでＳＥを呼ばない */
			GM_SeSetMode( SD_V_POUT0001, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
		}
		SetStatus( PLAYER_DEAD ) ;
		PL_SetDeadFlag( work ) ;
		work->power.vital = 0 ;

		/* 血 */
		GV_MatToVec( &work->body.objs->objs[ 11 ].world, &pos ) ;
		force.vx = force.vz = 0.0F ;
		force.vy = -320.0F ;
		NewBlood( &( work->body.objs->objs[ 11 ].world ), 
				  &pos, &force, 1, 0 ) ;
		work->body.m_ctrl->old_height = work->control.height ;
		work->body.m_ctrl->root_old_height = work->control.height ;
		work->act_name = ELUDE_TOUCHDOWN ;

	}
	EludeAction( work ) ;	
	/* 念のため */
	SetStatus( PLAYER_DEAD ) ;
	PL_SetDeadFlag( work ) ;

	work->control.step.vy = -GRAVITY ;

	GroundIK2( work, 500.0F ) ;	/* ダウン時傾き */
	AvoidSink( work, 200.0F, 0.0F ) ;

	if ( work->data == 0 && EndMotion( work ) ) {
		work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
		GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
		GM_GameOverProcEnd( &work->actor ) ;
		work->data = 1 ;
	}
}

/* エルード懸垂 */
static	void	EludeChinUp( Work *work, int time )
{
	BEYOND		*b ;
	float		t ;
	int			chinup ;

	b = &Beyond ;
	if ( time == 0 ) {
		EludeSetRootPosition( work, 0 ) ;
		SetAction( work, b->up_start, 6 ) ;
		work->act_name = ELUDE_STILL_MODE ;
	}
	EludeAction( work ) ;
	if ( Status( PLAYER_WATCH ) ) {
		SubjectTurn( work ) ;
	}

	EludeBackMoveCancel( work, ELUDE_MOVE_RIGHT ) ;
	EludeBackMoveCancel( work, ELUDE_MOVE_LEFT ) ;

#if defined(BP_VITA)
   work->pad->status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Back, PL_PadGetGestureRelease(work));
#endif

	chinup = ( ( work->pad->status & PAD_L2 ) && ( work->pad->status & PAD_R2 ) ) ;	

	switch( work->data ) {
	case 0 :
		if ( !chinup ) {
			t = PL_CheckMotionRate( work ) ;
			if ( t < 0.20F ) break ;	
			GM_VctrlStopVibration( &work->vctrl ) ;
			PL_SetAction4( work, b->up_end, 1.0F - t, 6 ) ;
			EludeSetRootPosition( work, 1 ) ;	
			SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
			work->data = 3 ;
			break ;
		}
		if ( EndMotion( work ) ) {
			SetAction( work, b->up_idle, 6 ) ;
			EludeUpdateChinUpData( work ) ;
			work->data = 1 ;
		}
		break ;
	case 1 :
		if ( !chinup ) {
			SetAction( work, b->up_end, 6 ) ;
			work->data = 2 ;
		}
		break ;
	case 2 :
		if ( EndMotion( work ) ) {
			if ( chinup ) {
				EludeSetRootPosition( work, 0 ) ;
				SetAction( work, b->up_start, 6 ) ;
				SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
				work->data = 0 ;
			} else {
				SetMode( work, EludeStill ) ;
				SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
			}
		}
		break ;
	case 3 :
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		if ( EndMotion( work ) ) {
			if ( chinup ) {
				EludeSetRootPosition( work, 0 ) ;
				SetAction( work, b->up_start, 6 ) ;
				work->data = 0 ;
			} else {
				SetMode( work, EludeStill ) ;
			}
		}
		break ;

	} 

	if ( !EludeDamageCheck( work ) &&
		 !EludeFallCheck( work ) ) {
		if ( work->data == 2 ||
			 work->data == 3 ) {
			EludeMoveCheck( work, ELUDE_CHINUP | ELUDE_STILL | 
						    ELUDE_MOVE_RIGHT | ELUDE_MOVE_LEFT ) ;
		}
	}
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#if 0 /* 旧エルードモード */
static	void	BeyondMode( work, time )
Work		*work ;
int		time ;
{
    BEYOND			*b ;
    FVECTOR			wall_obj, *mov, diff ;
	FVECTOR			chk_pos, tmp_pos ;
    FVECTOR			bound1, bound2, root ;
    HZX_BLOCK		*blk ;
    HZX_TRP			*trp, *traps ;
    GM_CameraSet	*cam ;
    int				dir = 0, right, left, i, motion, still ;
    int				r_ok, l_ok, damage, end_flag ;
    float			len1, len2 ;

    /* 規則に反して、data3、data4 fv2を使いまする。 */
    b = &Beyond ;
	damage = 0 ;
	end_flag = 0 ;
	if ( Status( PLAYER_DAMAGED ) ) {
		damage = 1 ;
		UnsetStatus( PLAYER_DAMAGED ) ;
		if ( work->post & MAIL_ELUDE_STAMP ) damage = 2 ;
		work->post &= ~MAIL_ELUDE_STAMP ;
	}
    if ( time == 0 ) {
		UnsetStatus( PLAYER_SQUAT | PLAYER_GROUND ) ;
		LeaveSubject( work ) ;
		/* 必ずモーションファイルチェンジ */
		PL_ChangeMotionArc( work, ForceWork.marfile ) ;
		/* 当たりチェック関連 */
        work->control.skip_flag |= CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
			CTRL_SKIP_MESSAGE ;
		if ( b->s_turn >= 0 ) {
			if ( GV_DiffDirAbs( b->s_turn, work->control.rot.vy ) > 1024 ) {
				SetAction( work, b->still, 0 ) ;
			} else {
				SetAction( work, b->still, 6 ) ;
			}
			work->control.rot.vy = work->control.turn.vy = b->s_turn ;
		} else {
			SetAction( work, b->still, 6 ) ;
		}
		/* トラップ検索 */
		if ( Beyond.trap_id < 0 ) {
			int	n_inside ;

			n_inside = work->control.evt.n_inside ;
			ASSERT( n_inside > 0 ) ;
			Beyond.trap_id = *( work->control.evt.inside ) ;
		}
		printf( "beyond trap_id %d\n", Beyond.trap_id ) ;
		/* 握力 */
		work->data2 = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
		work->hand_power.value = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
		GM_VisibleGage( &work->hand_power ) ;

		Root_height = work->control.mov.vy ;

		DG_COPY_VEC( &StartPos, &work->control.mov ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		GV_SetVec3( &UnitVec, 1.0F, 0.0F, 0.0F ) ;
		DG_RotVector( &UnitVec, &UnitVec, 1 ) ;

		/* 無敵解除 */
		UnsetInvincible( work ) ;

		/* ふみつけターゲット設定 */
		{
			FVECTOR		size ;

			size.vx = size.vy = size.vz = 240.0F ;
			GM_SetTarget( &work->offense, TARGET_OFFENSE | TARGET_POWER, 0,
						  ENEMY_SIDE, &size, &DG_ZeroVector ) ;
			GM_SetTargetWeaponType( &work->offense, WP_STAMP ) ;
		}

		/* 懸垂フラグ */
		Chin_up = 0 ;
		work->sv.vy = 0 ;
		/* 端っこフラグ */
		work->data3 = 0 ;
    } 

	GM_SubjectVMaxTmp[ 0 ] = 960 ;
	GM_SubjectVMaxTmp[ 1 ] = -960 ;

    /* 足影ＯＦＦ */
    if ( b->flag & BY_NOFOOTSHADOW ) {
		work->shadow = 0 ;
    }

    /* 静止モーションはどっち？ */
    if ( work->hand_power.value <= ELUDE_HAND_POWER_MAX / 3 ) {
		still = b->tired ;
	} else {
		still = b->still ;
	}

    /* 床チェックしてない場合でも影用のチェックが必要 */
    if ( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) PL_LevelCheck( work ) ;

    /* フラグセット */
    SetFlag( FLAG_FORCE | FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_NO_IK | FLAG_SUBJECT_HORIZON_LIMIT | FLAG_CANNOT_PEEP_LR ) ;
    SetStatus( PLAYER_MENU_DISABLE | PLAYER_WEAPON_INVISIBLE ) ;

	/* 発見位置あたま */
	PL_ObjPos( work, HUMAN21_ATAMA, &GM_PlayerFindPos ) ;

    /* 重力計算なし */
    work->control.step.vy = 0.0F ;
    switch( work->data ) {
    case 0 :
		AddFindObj( b->object ) ;
		AddFindObj( HUMAN21_ATAMA ) ;
		AddFindObj( HUMAN21_MIGI_KAKATO ) ;
		r_ok = l_ok = 0 ;
		motion = -1 ;
		if ( Status( PLAYER_WATCH ) ) {
			/* 主観中 */
			SubjectTurn( work ) ;
			if ( !Flag( FLAG_PEEPING ) ) {
				/* 覗き込んでなければＬＲで移動 */
				if ( work->pad->status & PAD_R2 ) r_ok = 1 ;
				else if ( work->pad->status & PAD_L2 ) l_ok = 1 ;
				Chin_up = 0 ;
			} else {
				/* 懸垂回数 */
				if ( Chin_up < 16 &&
					 ( work->pad->status & PAD_L2 ) &&
					 ( work->pad->status & PAD_R2 ) ) {
					if ( ++ Chin_up >= 16 ) {
						if ( Status( PLAYER_SNAKE ) ) {
							GM_SnakeChin_Up ++ ;
							if ( GM_SnakeChin_Up == ELUDE_HAND_POWER_LEVELUP2 ) {
                                ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 2 ] ;
                                work->data2 = work->data2 * GM_SnakeGripMax[ 2 ] / GM_SnakeGripMax[ 1 ] ;
								work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
							} else if ( GM_SnakeChin_Up == ELUDE_HAND_POWER_LEVELUP1 ) {
								ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 1 ] ;
								work->data2 = work->data2 * GM_SnakeGripMax[ 1 ] / GM_SnakeGripMax[ 0 ] ;
								work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
							}
						} else {
							GM_RaidenChin_Up ++ ;
							if ( GM_RaidenChin_Up == ELUDE_HAND_POWER_LEVELUP2 ) {
								ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 2 ] ;
								work->data2 = work->data2 * GM_RaidenGripMax[ 2 ] / GM_RaidenGripMax[ 1 ] ;
								work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
							} else if ( GM_RaidenChin_Up == ELUDE_HAND_POWER_LEVELUP1 ) {
								ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 1 ] ;
								work->data2 = work->data2 * GM_RaidenGripMax[ 1 ] / GM_RaidenGripMax[ 0 ] ;
								work->hand_power.max = DIRECT_TICK( ELUDE_HAND_POWER_MAX ) ;
							}
						}
					}
				} else if ( Chin_up != 0 &&
						    ( !( work->pad->status & PAD_L2 ) ||
							  !( work->pad->status & PAD_R2 ) ) ) {
					Chin_up = 0 ;
				}
			} 
			goto lr_move_check ;
		} else {
			/* ＬＲでも移動できる */
			if ( work->pad->status & PAD_R2 ) r_ok = 1 ;
			else if ( work->pad->status & PAD_L2 ) l_ok = 1 ;
			if ( r_ok || l_ok ) goto lr_move_check ;
		}
		cam = GM_GetCurrentCameraSet( 0 ) ;
		dir = ( work->control.rot.vy - cam->rotate.vy ) & 4095 ;
		right = ( dir + 1024 ) & 4095 ;
		left = ( dir - 1024 ) & 4095 ;
		dir = work->pad->dir ;
		if ( dir < 0 ) goto beyond_still ;
		dir -= GV_GetPadOrigin() ;
		if ( GV_DiffDirAbs( right, dir ) < BY_PAD_ADJUST ) r_ok = 1 ;
		else if ( GV_DiffDirAbs( left, dir ) < BY_PAD_ADJUST ) l_ok = 1 ;
lr_move_check :
	    /* すでに端っこにいたら動けない */
	    if ( r_ok && ( work->data3 & 0x2 ) ) r_ok = 0 ;
		else if ( l_ok && ( work->data3 & 0x1 ) ) l_ok = 0 ;
	    if ( r_ok || l_ok ) SetStatus( PLAYER_MOVE ) ;
		if ( r_ok ) {
			/* 右 */
			motion = b->move1 ; 
			dir = work->control.rot.vy - 1024 ;
			work->data3 &= ~0x1 ; /* 左端っこフラグ解除 */
		} else if ( l_ok ) {
			/* 左 */
			motion = b->move2 ;
			dir = work->control.rot.vy + 1024 ;
			work->data3 &= ~0x2 ; /* 右端っこフラグ解除 */
		} else {
beyond_still :
	        motion = -1 ;
            if ( still != work->motion1 ) {
				SetAction( work, still, 6 ) ;
                SetRootPosition( work, &StartPos, &UnitVec ) ;
			}
		}
#if 0
		/* トラップ検索 */
		mov = &( work->control.mov ) ;
		PL_ObjPos( work, HUMAN21_MIGI_TE, &chk_pos ) ;
		PL_ObjPos( work, HUMAN21_HIDARI_TE, &tmp_pos ) ;
		chk_pos.vx += tmp_pos.vx ;	chk_pos.vx /= 2.0F ;
		chk_pos.vz += tmp_pos.vz ;  chk_pos.vz /= 2.0F ;
		mov = &chk_pos ;

		blk = HZX_GetInsideBlock( work->control.hzx_id, mov ) ;
		ASSERT( blk != NULL ) ;
		trp = NULL ;
		traps = blk->traps ;
		for ( i = 0; i < blk->n_traps; i ++, traps ++ ) {
			if ( traps->name_id == Beyond.trap_id ) {
				trp = traps ;
				break ;
			}
		}
		if ( trp != NULL ) {
			bound1.vx = trp->b1.vx + blk->tx ;
			bound1.vy = trp->b1.vy + blk->ty ;
			bound1.vz = trp->b1.vz + blk->tz ;
			bound2.vx = trp->b2.vx + blk->tx ;
			bound2.vy = trp->b2.vy + blk->ty ;
			bound2.vz = trp->b2.vz + blk->tz ;
			if ( b->flag & BY_X_RANGE_EXIST ) {
				bound1.vx = ( bound1.vx > b->x_min ) ? bound1.vx : b->x_min ;
				bound2.vx = ( bound2.vx < b->x_max ) ? bound2.vx : b->x_max ;
			}
			if ( b->flag & BY_Z_RANGE_EXIST ) {
				bound1.vz = ( bound1.vz > b->z_min ) ? bound1.vz : b->z_min ;
				bound2.vz = ( bound2.vz < b->z_max ) ? bound2.vz : b->z_max ;
			}
#ifdef DEBUG_MODE
			if ( GM_PlayerDebugMode == GM_PDM_SEGMENTVIEW ) {
				NewBoundingBoxView(&bound1,&bound2,32,232,32) ;
			}
#endif
			if ( bound1.vx >= bound2.vx ||
				 bound1.vy >= bound2.vy ||
				 bound1.vz >= bound2.vz ) {
				printf( "warning : bound setting is wrong\n" ) ;
				len1 = -1.0F ;
			} else {
				len1 = BoundingEdgeLen( mov, dir, &bound1, &bound2, &diff ) ;
			}
			if ( len1 < 0.0F ) printf( "warning : outside of elude area\n" ) ;
		} else {
			len1 = -1.0F ;
			printf( "outside trap!\n" ) ;
		}
#endif
		if ( motion != -1 ) {
#if 1
			mov = &( work->control.mov ) ;
			PL_ObjPos( work, HUMAN21_MIGI_TE, &chk_pos ) ;
			PL_ObjPos( work, HUMAN21_HIDARI_TE, &tmp_pos ) ;
			chk_pos.vx += tmp_pos.vx ;	chk_pos.vx /= 2.0F ;
			chk_pos.vz += tmp_pos.vz ;  chk_pos.vz /= 2.0F ;
			mov = &chk_pos ;
			/* トラップ検索 */
			blk = HZX_GetInsideBlock( work->control.hzx_id, mov ) ;
			ASSERT( blk != NULL ) ;
			trp = NULL ;
			traps = blk->traps ;
			for ( i = 0; i < blk->n_traps; i ++, traps ++ ) {
				if ( traps->name_id == Beyond.trap_id ) {
					trp = traps ;
					break ;
				}
			}
			if ( trp != NULL ) {
				bound1.vx = trp->b1.vx + blk->tx ;
				bound1.vy = trp->b1.vy + blk->ty ;
				bound1.vz = trp->b1.vz + blk->tz ;
				bound2.vx = trp->b2.vx + blk->tx ;
				bound2.vy = trp->b2.vy + blk->ty ;
				bound2.vz = trp->b2.vz + blk->tz ;
				if ( b->flag & BY_X_RANGE_EXIST ) {
					bound1.vx = ( bound1.vx > b->x_min ) ? bound1.vx : b->x_min ;
					bound2.vx = ( bound2.vx < b->x_max ) ? bound2.vx : b->x_max ;
				}
				if ( b->flag & BY_Z_RANGE_EXIST ) {
					bound1.vz = ( bound1.vz > b->z_min ) ? bound1.vz : b->z_min ;
					bound2.vz = ( bound2.vz < b->z_max ) ? bound2.vz : b->z_max ;
				}
#ifdef DEBUG_MODE
				if ( GM_PlayerDebugMode == GM_PDM_SEGMENTVIEW ) {
					NewBoundingBoxView(&bound1,&bound2,32,232,32) ;
				}
#endif
				len1 = BoundingEdgeLen( mov, dir, &bound1, &bound2, &diff ) ;
				if ( len1 < 0.0F ) printf( "warning : outside of elude area\n" ) ;
#else
			if ( trp != NULL ) {
#endif
				if ( len1 < BY_BOUND_ADJ ) {
					/* 端っこフラグセット */
					if ( l_ok ) work->data3 |= 0x1 ;
					else if ( r_ok ) work->data3 |= 0x2 ;
					len2 = BoundingEdgeLen( mov, dir + 2048, 
										    &bound1, &bound2, NULL ) ;
					if ( len2 < BY_BOUND_ADJ ) {
						motion = still ;
					} else {
#if 0
						_sceVu0AddVector( &root, &root, &diff ) ;
						GV_LenVec3F( &diff, &diff, len1, BY_BOUND_ADJ ) ;
						_sceVu0SubVector( &root, &root, &diff ) ;
						_sceVu0SubVector( &root, &root, &work->root_diff ) ;
						root.vy = work->control.mov.vy ;
						GM_ResetControlPosition( &work->control, &root ) ;
#endif
						motion = still ;
					}
				}
			} 
#if 0
			if ( trp == NULL ) {
				printf( "outside trap!\n" ) ;
			}
#endif
			if ( motion != work->motion1 ) {
				SetAction( work, motion, 6 ) ;
//				if ( motion != still ) GM_VctrlSetLoop( &work->vctrl ) ;
                SetRootPosition( work, &StartPos, &UnitVec ) ;
			}
		}

		/* 戻り */
		if ( damage == 0 && ( work->pad->press & PL_PAD_BEYOND ) ) {
			if ( !CheckTrap( work, NOBEYOND_TRAP ) ) {
				/* 壁チェック */
				MatToVec( &( work->body.objs->objs[ Beyond.wall_obj ].world ),
						 &wall_obj ) ;
				if ( CheckMovRotLenSegment( work->control.hzx_id,
										   &wall_obj, &DG_ZeroVector, 	
										   &work->control.rot, 750.0,
										   HZX_CHK_SEGMENT,
										   HZX_SEG_NO_PLAYER, HZX_FLOOR_NO_PLAYER ) ) {
					SetAction( work, b->back2, 6 ) ;
				} else {
					SetAction( work, b->back, 6 ) ;
				}

				/* 戻り開始プロック呼び */
				PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_OUT, 0, 0 ) ;

				GM_InvisibleGage( &work->hand_power ) ;
				SetInvincible( work, 0 ) ;
				LeaveSubject( work ) ;
				SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
				SetStatus( PLAYER_FORCE ) ;
				work->data = 1 ;
				return ;
			}
		}

		/* 握力低下 */
#ifdef DEBUG_MODE
	    if ( !GM_DebugModeEnable || !( work->pad->status & PAD_L1 ) ) {
#endif
			if ( work->data2 < DIRECT_TICK( ELUDE_HAND_POWER_MAX ) / 3 && 
				( GV_Time % FRAME_PER_SEC ) == 0 ) {
				if ( PL_PadEnable() ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_O2DAMAGE ) ;
				work->hand_power.flag |= GM_GAGE_WARNING ;
			} else {
				work->hand_power.flag &= ~GM_GAGE_WARNING ;
			}

			if ( PL_PadEnable() ) {
				/* ライフ値によって減り方が違う */
				if ( work->power.vital == GM_VitalityMax ) {
					work->data2 -= 1 ;
				} else if ( work->power.vital > GM_VitalityMax / 3 ) {
					work->data2 -= 2 ;
				} else {
					work->data2 -= 3 ;
				}
				/* 懸垂中はいっぱい減る */
				if ( Status( PLAYER_WATCH ) &&
					( work->pad->status & PAD_L2 ) &&
					( work->pad->status & PAD_R2 ) ) work->data2 -= 2 ;
			}
#ifdef DEBUG_MODE
		}
#endif
		if ( ( work->data2 <= 0 ) ||				/* 握力なし */
			 ( b->flag & BY_FALL_IMMEDIATELY ) ||	/* すぐおりエルード */
			( ( work->pad->status & PL_PAD_BEYOND_DOWN ) &&	/* ×ボタン */
			 !( b->flag & BY_CANNOT_FALL ) ) ||
			( Status( PLAYER_DEAD ) ) ) {		/* 失血死など */
			SetAction( work, b->fall_start, 0 ) ;
			work->data = 2 ; 
			GM_InvisibleGage( &work->hand_power ) ;
			/* ＸＺ方向移動禁止（床ずれ防止） */
			SetFlag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT | FLAG_NO_MOTION_STEP_XZ ) ;
			work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;
			work->data2 = -64 ;
			SetInvincible( work, 0 ) ;
			/* ＳＥ */
			if ( !Status( PLAYER_DEAD ) ) {
				if ( damage || ( b->flag & BY_FALL_DEAD ) ) {
					/* うわぁ */
					/* 撃たれてパワー切れか、落ちたら死ぬ場所のとき */
					GM_SeSetMode( SD_V_PFALL01, &work->control.mov, GM_SEMODE_BOMB ) ;
				} else {
					/* 気合 */
					/* 上以外のとき */
					GM_SeSetMode( SD_V_PKIAI01, &work->control.mov, GM_SEMODE_BOMB ) ;
				}
			}
		} else if ( damage ) {
			if ( damage == 1 ) {	/* ダメージ */
				work->data = 5 ; 
				SetInvincible( work, 0 ) ;
			} else work->data = 6 ; /* ふまれ */
			_sceVu0AddVector( &root, &work->control.mov, &work->root_diff ) ;
			root.vy = Root_height ;
			GM_ResetControlPosition( &work->control, &root ) ;
			SetAction( work, b->damage, 6 ) ;
		}
		work->hand_power.value = work->data2 ;
		break ;
    case 1 :
		SetFlag( FLAG_DONOT_CHECK_WATCH ) ;
		if ( EndMotion( work ) ) {
			end_flag = 1 ;
			SetFlag( FLAG_FORCE_END ) ;
			UnsetStatus( PLAYER_BEYOND | PLAYER_FORCE ) ;
			UnsetInvincible( work ) ;
			work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
										 CTRL_SKIP_TRAP | CTRL_SKIP_MESSAGE ) ;
			ForceWork.flag &= ~FA_USE_DEFAULT ;
			Beyond.trap_id = -1 ; /* トラップ名初期化 */
//			SetMode( work, StillMode[ work->stance ] ) ;
			SetMode( work, StandStill ) ;
			ResetAddress( work ) ;
			if ( b->e_turn >= 0 ) {
				ForceWork.e_turn = b->e_turn ;
			} else {
				ForceWork.e_turn = -1 ;
			}
		}
	    PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_OUT, 1, end_flag ) ;
		break ;
    case 2 :	
		/* 落ちる */
		if ( work->data2 == -64 ) {
			Fall_level = work->control.mov.vy ;
			PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_FALL, 0, 0 ) ;
		}
		/* 死んで、落ちすぎのときはゲームオーバーエンド */
		if ( Status( PLAYER_DEAD ) &&
			 Fall_level - work->control.mov.vy > 80000.0F ) {	/* 80m */
			GM_GameOverProcEnd( &work->actor ) ;
			return ;
		}

		/* ふみつけ */
		if ( !( b->flag & BY_NO_STAMP ) ) {
			GM_MoveTarget3( &work->offense, ObjWorld( work, HUMAN21_MIGI_KAKATO ) ) ;
			GM_PutTarget( &work->offense ) ;
//			NewTargetView2( &work->offense, 32, 232, 32 ) ;
		}
		if ( work->motion1 == b->fall_start && EndMotion( work ) ) {
			SetAction( work, b->fall_loop, 6 ) ;
		}
		SetFlag( FLAG_DONOT_CHECK_WATCH | FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT |
				 FLAG_NO_MOTION_STEP_XZ ) ;
		work->data2 -= 8 ;
		work->control.step.vy = ( float )work->data2 ;
	    if ( ( work->control.grounded & 1 ) || work->control.mov.vy <= GM_WORLD_LIMIT_BOTTOM ) {
			/* 壁の真上に落ちないようにする */
			if ( work->control.grounded & 1 ) {
				FVECTOR		mov ;
				int			n_touches, flag ;
				FVECTOR		vecs[ 2 ] ;
				float	    pre_floor ;

				DG_COPY_VEC( &mov, &work->control.mov ) ;
				mov.vy = work->control.levels[ 0 ] + 750.0F ;
				n_touches = HZX_NearHazardCheck( work->control.hzx_id, &mov, 500.0F,
												 HZX_CHK_ALL, HZX_SEG_NO_PLAYER, 450.0F ) ;
				if ( n_touches > 0 ) {
					HZX_GetNearVector( vecs ) ;
					if ( ( DG_FABS( vecs[ 0 ].vx ) < 0.010F ) &&
						 ( DG_FABS( vecs[ 0 ].vz ) < 0.010F ) ) {
						/* 壁上に落ちてしまった */	
						/* ちょっと位置をずらす */
						printf( "warning : wall wall\n" ) ;
						GV_GetShiftPos( &mov, &work->control.mov, &work->control.rot,
									    4.0F, 0.0F, 0.0F ) ;
						GM_ResetControlPosition( &work->control, &mov ) ;
						/* 床チェックしなおし */
						pre_floor = work->control.levels[ 0 ] ;
						flag = HZX_LevelHazardCheck( work->control.hzx_id, 
													&mov, work->control.hzx_check_type, 
													work->control.flr_flag ) ;
						if ( flag != 0 ) {
							HZX_GetLevelHeight( work->control.levels ) ;
							HZX_GetLevelHazard( work->control.level[ 0 ], work->control.flr_atrs ) ;
						} 
						if ( !( flag & 1 ) ) work->control.levels[ 0 ] = -1000000.0F ;
						if ( !( flag & 2 ) ) work->control.levels[ 1 ] = 1000000.0F ;    

						/* 再チェックの結果、床の高さが変わってしまった */
						if ( pre_floor > work->control.levels[ 0 ] ) break ;
					}
				}
			}
#if 1
			Fall_level = Fall_level - work->control.mov.vy ;
			if ( Status( PLAYER_DEAD ) || ( b->flag & BY_FALL_DEAD ) ) {
				Fall_level = BY_FALL_DEAD_HEIGHT + 1 ; /* 特殊 */
			}			
#endif
			if ( Fall_level < BY_FALL_DAMAGE_HEIGHT ) {
				SetAction( work, b->fall_safe, 6 ) ;
				NewPadVibration( DamageVib1H, 1 ) ;
				NewPadVibration( DamageVib1L, 2 ) ;
				work->data = 3 ;
			} else if ( Fall_level < BY_FALL_DEAD_HEIGHT ) {
				SetAction( work, b->fall_damage, 6 ) ;
				NewPadVibration( DamageVib2H, 1 ) ;
				NewPadVibration( DamageVib2L, 2 ) ;
				work->power.vital -= ( ELUDE_FALL_DAMAGE * ( Fall_level - 2000 ) / 1000 ) ;
				/* 死なないように */
				if ( work->power.vital <= 0 ) work->power.vital = 1 ;
				/* ゲージを出す。（後でちゃんと直そう） */
				work->gagetime = 0 ;
				if ( !GM_CheckMenuStatus( MENU_MENU_OPEN ) ) {
					GM_GageStatus = GM_GAGE_STATE_APPEAR ;
				}
				work->data = 3 ; 
			} else {
#if 1
				GM_GameOverProcStart( &work->actor ) ;
				LeaveSubject( work ) ;
#endif
				if ( !Status( PLAYER_DEAD ) ) {
					/* 撃たれて死んだときはここでＳＥを呼ばない */
					GM_SeSetMode( SD_V_POUT0001, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
				}
				SetAction( work, b->fall_out, 6 ) ;
				SetStatus( PLAYER_DEAD ) ;
				PL_SetDeadFlag( work ) ;
				work->power.vital = 0 ;
				NewPadVibration( DamageVib2H, 1 ) ;
				NewPadVibration( DamageVib2L, 2 ) ;
				/* 血 */
				{
					FVECTOR	pos, force ;

					GV_MatToVec( &work->body.objs->objs[ 11 ].world, &pos ) ;
					force.vx = force.vz = 0.0F ;
					force.vy = -320.0F ;
					NewBlood( &( work->body.objs->objs[ 11 ].world ), 
							 &pos, &force, 1, 0 ) ;
				}
				work->data = 4 ;
			}
			ResetAddress( work ) ;
			work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
		}
		break ;
    case 3 :					/* 着地 */
		if ( ( work->motion1 == b->fall_damage ) &&
			( work->body.m_ctrl->mt3_ctrl->time == 10 ) ) {
			GM_SeSetMode( SD_V_PDMG03, &GM_PlayerPosition, GM_SEMODE_BOMB ) ;
			GM_SetNoise( NOISE_S, &work->control.mov, work->control.map ) ;
		}
		if ( EndMotion( work ) ) {
			end_flag = 1 ;
			SetFlag( FLAG_FORCE_END ) ;
			UnsetStatus( PLAYER_BEYOND ) ;
			UnsetInvincible( work ) ;
			work->control.skip_flag &= ~( CTRL_SKIP_TRAP | CTRL_SKIP_MESSAGE ) ;
			ForceWork.flag &= ~FA_USE_DEFAULT ;
			Beyond.trap_id = -1 ; /* トラップ名初期化 */
			//	    SetMode( work, StillMode[ work->stance ] ) ;
			SetMode( work, StandStill ) ;
		}
	    PL_ExecForceActProc( work, FA_PROCMODE_ELUDE_FALL, 1, end_flag ) ;
		break ;
    case 4 :					/* 死に */
		SetStatus( PLAYER_DEAD ) ;
   	    PL_SetDeadFlag( work ) ;
		GroundIK2( work, 500.0F ) ;	/* ダウン時傾き */
	    if ( EndMotion( work ) ) {
			work->body.m_ctrl->mt3_ctrl->flag |= MT3_SLEEP ;
			GM_VctrlSetVibration( &work->vctrl, -1, 0 ) ;
			GM_GameOverProcEnd( &work->actor ) ;
		}
		break ;
	case 5 :				/* ダメージ */
		if ( EndMotion( work ) ) {
			SetInvincible( work, INVINCIBLE_TIME ) ;
			work->data = 0 ;
		}
		break ;
	case 6 :				/* ふまれ */
		if ( EndMotion( work ) ) {
			work->data = 0 ;
		}
    }
	/* 疲れたとき */
	if ( b->still != b->tired ) {
		if ( work->sv.vy == 0 && work->motion1 == b->tired ) {
			GM_SeSetMode( SD_V_PELUDE01, &work->control.mov, GM_SEMODE_BOMB ) ;
			work->sv.vy = 1 ;
		}
	}
}
#endif  /* 旧エルードモード */

#ifdef KP_XBOX	// 一応復帰させておく
#undef GM_SeSetMode( a, b, c ) 
#endif
