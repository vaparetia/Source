/*
   util_sol.c
   ソリダス／汎用・便利関数
   
   2001/03/22	M.Sonoyama
   $Id: util_sol.c,v 1.1.1.3 2002/11/19 11:51:07 Yoshizawa1 Exp $
*/

/* プロトタイプ宣言 */
static	int		ExprGroundRX( Work *work, int ry ) ;

/*----------------------------------------------------------------*/

/* 間合いカウントチェック */
static	inline	int	MaaiCheck( Work *work, int count )
{
	if ( work->maai_count < count ) return 0 ;
	return 1 ;
}

/* モーション停止／再開 */
static	inline	void	MotionSleep( Work *work, int layer )
{
	work->body.m_ctrl->mt3_ctrl[ layer ].flag |= MT3_SLEEP ;
}

static	inline	void	MotionActive( Work *work, int layer )
{
	work->body.m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_SLEEP ;	
}

/* モーション経過時間 */
static	inline	int	MotionTime( Work *work )
{
	return ( int )( work->body.m_ctrl->mt3_ctrl->play_time /
				    ( float )NTSC_TIME_BASE ) ;
}

/* モーション速度 */
static	inline	void	MotionSpeed( Work *work, float fast )
{
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE / fast ) ;
}

static	inline	float	MotionRate( Work *work )
{
	float		t ;

	t = ( float )work->body.m_ctrl->mt3_ctrl[ 0 ].play_time /
		( float )work->body.m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;
	return t ;
}

static	inline	int		CheckMotionTime( Work *work, int time )
{
	return MT_CHECK_MOTION_TIME( work->body.m_ctrl, 0, time * NTSC_TIME_BASE ) ;
}

/* モデルの現在のバウンディングを求める */
static	void	GetModelBounding( DG_OBJS *objs, FVECTOR *min, FVECTOR *max )
{
	int			i ;
	FMATRIX		inv ;
	FVECTOR		v, vmin, vmax ;
	DG_OBJ		*obj ;

	_sceVu0InversMatrix( &inv, &objs->world ) ;
	GV_SetVec3( max, -1000000.0F, -1000000.0F, -1000000.0F ) ;
	GV_SetVec3( min, 1000000.0F, 1000000.0F, 1000000.0F ) ;
	for ( i = 0; i < objs->def->n_models; i ++ ) {
		obj = &objs->objs[ i ] ;
		GV_MatToVec( &obj->world, &v ) ;
		v.vw = 1.0F ;
		_sceVu0ApplyMatrix( &v, &inv, &v ) ;
		_sceVu0AddVector( &vmax, &v, &obj->bound_max ) ;
		_sceVu0AddVector( &vmin, &v, &obj->bound_min ) ;
		max->vx = DG_MAX( vmax.vx, max->vx ) ;
		max->vy = DG_MAX( vmax.vy, max->vy ) ;
		max->vz = DG_MAX( vmax.vz, max->vz ) ;
		min->vx = DG_MIN( vmin.vx, min->vx ) ;
		min->vy = DG_MIN( vmin.vy, min->vy ) ;
		min->vz = DG_MIN( vmin.vz, min->vz ) ;
	}
}

/* ある関節にターゲットをつける */
static	inline	void	UpdateJointTarget( TARGET *t, int where, OBJECT *body )
{
	GM_MoveTarget2Map( t, &body->objs->objs[ t->name ].world, where ) ;	
//	NewTargetView2( t, 32, 232, 32 ) ;	
}

/* 子ターゲットの全更新 */
static	void	UpdateChildTarget( Work *work )
{
	int			i ;

	for ( i = 0; i < MAX_TARGET_CHILDREN; i ++ ) {
		UpdateJointTarget( &work->child[ i ], work->control.map, &work->body ) ;
	}
}

/* ターゲット更新 */
static	void	UpdateTarget( Work *work )
{
	FVECTOR		min, max, size ;
	TARGET		*t ;

	t = &work->defense ;
	GetModelBounding( work->body.objs, &min, &max ) ;
	max.vy += 200.0F ; /* 頭の分 */
	t->offset.vx = ( max.vx + min.vx ) / 2.0F ;
	t->offset.vy = ( max.vy + min.vy ) / 2.0F ;
	t->offset.vz = ( max.vz + min.vz ) / 2.0F ;
	t->offset.vw = 1.0F ;
	size.vx = DG_FABS( max.vx - min.vx ) / 2.0F ;
	size.vy = DG_FABS( max.vy - min.vy ) / 2.0F ;
	size.vz = DG_FABS( max.vz - min.vz ) / 2.0F ;

	t->class |= TARGET_CHILD_ALWAYS ;
	if ( !GM_CheckPlayerStatus( PLAYER_WATCH ) &&
		 size.vy < 850.0F ) {
		t->offset.vy += 350.0F ;
		t->size.vy += 350.0F ;
		t->class &= ~TARGET_CHILD_ALWAYS ;
	} 
	GM_SetTargetSize( t, &size ) ;
//NewTargetView2( t, 32, 232, 32 ) ;
	GM_MoveTarget2( t, &work->body.objs->world ) ;
	UpdateChildTarget( work ) ;
}

/* プレイヤーとの位置関係更新 */
static	void	UpdateDiffPlayer( Work *work )
{
	_sceVu0SubVector( &work->pos_diff_player, &GM_PlayerControl->mov, &work->control.mov ) ;
	GV_VecToRot( &work->pos_diff_player, &work->rot_diff_player ) ;
	work->len_diff_player = GV_VecLen3F( &work->pos_diff_player ) ;
}

/* モーションセット */
static	void	SetAction( Work *work, int action, int time, int interp )
{
	int			rc ;

    if ( work->motion1 == action ) return ;
	MotionActive( work, 0 ) ;
	MotionSpeed( work, 1.0F ) ;
    GM_ConfigObjectAction( &( work->body ), 0, action,
						  time * NTSC_TIME_BASE, 0xfffff, interp * NTSC_TIME_BASE ) ;
	if ( interp == 0 ) work->body.m_ctrl->interp_flag = 0 ;
    work->motion1 = action ;
	work->mtime1 = time ;
	work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL2 ;
	if ( work->motion2 == -1 ) {
		work->body.m_ctrl->flag &= ~MT_FLAG_REVERSAL1 ;
	}
    if ( ( rc = work->body.m_ctrl->rot_correct ) != 0 ) {
		work->control.turn.vy += rc ;
		work->control.rot.vy = work->control.turn.vy ;
    }
	MT_SetMotionSpeed( work->body.m_ctrl, ( float )TIME_BASE ) ;
}

/* モードセット */
static	void	SetMode( Work *work, ACTION action )
{
	work->time = work->ftime_count = 0 ;
	work->action = action ;
	work->control.turn.vz = 0 ;
	work->data = work->data2 = 0 ;
}

/* ステータスセット */
static	inline	void	SetStatus( Work *work, int state )
{
	work->status |= state ;
}

/* ステータスリセット */
static	inline	void	ResetStatus( Work *work, int state )
{
	work->status &= ~state ;
}

/* ステータスチェック */
static	inline	int		Status( Work *work, int state )
{
	return ( work->status & state ) ;
}	

/* フラグセット */
static	inline	void	SetFlag( Work *work, int state )
{
	work->flag |= state ;
}

/* ステータスリセット */
static	inline	void	ResetFlag( Work *work, int state )
{
	work->flag &= ~state ;
}

/* ステータスチェック */
static	inline	int		Flag( Work *work, int state )
{
	return ( work->flag & state ) ;
}	

/* 無敵にする */
static	void	SetInvincible( Work *work, int time )
{
	work->invincible_time = DIRECT_TICK( time ) ;
	SetStatus( work, SOL_STATE_INVINCIBLE ) ;
}

/* 無敵解除 */
static	void	ResetInvincible( Work *work )
{
	work->invincible_time = 0 ;
	ResetStatus( work, SOL_STATE_INVINCIBLE ) ;
}

/* 無敵処理 */
static	void	CheckInvincible( Work *work )
{
	if ( work->invincible_time > 0 ) {
		-- work->invincible_time ;
		if ( work->invincible_time <= 0 ) {
			ResetInvincible( work ) ;
		}
	}
}

/* ＳＶＥＣＴＯＲが０ベクトルかどうかチェック */
static	inline	int		CheckZeroSVector( v )
SVECTOR			*v ;
{
#if 1//BP_MATH - replaced with version that works for both little and big endian
   return( (v->vx == 0) && (v->vy == 0) && (v->vz == 0) );
#else
	return ( ( ( *( long64 * )v & I64(0xffffffffffff) ) ) ? 0 : 1 ) ;
#endif
}

/* アジャスト値リセット */
static	void	ResetRotAdjust( work )
Work			*work ;
{
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	int				i, flag, flag2 ;

	adj = work->rot_adj ;
	tadj = work->turn_adj ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;
	flag2 = 0 ;
	GV_ZeroMemory( tadj, sizeof( SVECTOR ) * 24 ) ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		if ( ( flag & 1 ) && !CheckZeroSVector( &adj[ i ] ) ) {
			flag2 |= ( 1 << i ) ;
		}
		flag >>= 1 ;
	}
	m_ctrl->adjust_flag = flag2 ;
}

/* アジャスト値計算 */
static	void	RotAdjust( work )
Work			*work ;
{
	FVECTOR			quat ;
	MOTION_CONTROL	*m_ctrl ;
	SVECTOR			*adj, *tadj ;
	int				i, flag ;

	adj = work->rot_adj ;
	tadj = work->turn_adj ;
	/* 角度 */
	m_ctrl = work->body.m_ctrl ;
	flag = m_ctrl->adjust_flag ;
	for ( i = 0; i < N_UNITS; i ++ ) {
		if ( flag & 1 ) {
			GV_NearExp4PV( &adj[ i ], &tadj[ i ], 3 ) ;
			if ( adj[ i ].vy != 0 ) {
				GM_RotToQuat( &adj[ i ], &quat ) ;
			} else {
				GM_RotToQuatXAfterY( &adj[ i ], &quat ) ;
			}
			DG_COPY_VEC( &m_ctrl->adjust[ i ], &quat ) ;
		}
		flag >>= 1 ;
	}
}

/* 角度アジャスト値セット */
static	void	SetRotAdjust( Work *work, SVECTOR *rot, int n )
{
	work->turn_adj[ n ] = *rot ;
	work->body.m_ctrl->adjust_flag |= 1LL /* BP - Added LL because adjust_flag is 64bit */ << n ;
}

/* プレイヤーのほうを向く */
static	int		HeadToPlayer( Work *work )
{
	SVECTOR		rot, rot2 ;

	rot = work->rot_diff_player ;
	GV_SetVec3( &rot2, 0, 0, 0 ) ;

	rot.vy = GV_DiffDirS( work->control.rot.vy, rot.vy ) ;
	if ( rot.vy < -512 || rot.vy > 512 ) rot.vy = 0 ;
	rot.vx &= 4095 ;
	if ( rot.vx > 2048 ) rot.vx -= 4096 ;
	if ( rot.vx < -512 || rot.vx > 512 ) rot.vx = 0 ;

	if ( rot.vy < -256 ) {
		rot2.vy = rot.vy + 256 ;
		rot.vy = -256 ;
	} else if ( rot.vy > 256 ) {
		rot2.vy = rot.vy - 256 ;
		rot.vy = 256 ;
	}
	if ( rot2.vy != 0 ) {
		SetRotAdjust( work, &rot2, HUMAN21_MUNE ) ;
	}
	if ( rot.vx != 0 || rot.vy != 0 ) {
		SetRotAdjust( work, &rot, HUMAN21_ATAMA ) ;
	}
	return ( rot.vy != 0 ) ;
}

/* 頂点アニメ更新 */
static	void	UpdateVAnime( Work *work, float t )
{
	int					i ;
	VERTEX_ANIME_WORK	*va_work ;

	if ( DG_FABS( work->va_t - t ) > 0.0010F ) {
		work->va_t = t ;
		for ( i = 0; i < N_UNITS; i ++ ) {
			va_work = work->va_work[ i ] ;
			va_work->p[ 0 ] = t ;
			va_work->p[ 1 ] = 1.0F - t ;
			va_work->count = 8 ;
		}
	}
	for ( i = 0; i < N_UNITS; i ++ ) {
		va_work = work->va_work[ i ] ;
		if ( va_work->count != 0 ) SimpleVertexAnimation( va_work ) ;
	}
}

/* 左側が手薄 */
static	void	LeftSideNoGuard( Work *work )
{
	int			diff, diffabs ;

	diff = GV_DiffDirS( work->control.rot.vy, work->rot_diff_player.vy ) ;
	diffabs = GV_DiffDirAbs( work->control.rot.vy, work->rot_diff_player.vy ) ;
	if ( diff < 64 && diffabs < 1200 ) SetFlag( work, SOL_FLAG_BLADE_GUARD ) ;
}

/* プレイヤーのほうにからだを向ける */
/* ローリングしていると向くのが遅くなる */
static	void	DirPlayer( Work *work )
{
	if ( GM_CheckPlayerStatus( PLAYER_ROLLING ) ) {
//		work->control.turn.vy = GV_NearExp16P( work->control.turn.vy,
//											  work->rot_diff_player.vy ) ;
		work->control.turn.vy = work->rot_diff_player.vy ;
	} else {
		work->control.turn.vy = work->rot_diff_player.vy ;
	}
}

/* 逃げる位置を決める */
static	void	SetMovePosition( Work *work )
{
	static int 	Way[] = { 0, 0, 2, 2, 2, 2, 1, 1, 1, 1, 3, 3, 3, 3, 0 } ;
	MoveThink	*mt ;
	FVECTOR		aim, mov, maxPos ;
	float		len, max ;
	SVECTOR		rot ;
	int			i, maxNo ;
	
	mt = &work->move_think ;
	GV_SetVec3( &rot, 0, work->rot_diff_player.vy, 0 ) ;
	DG_COPY_VEC( &mov, &work->control.mov ) ;
	mov.vy = work->control.levels[ 0 ] + 250.0F ;
	max = -1000000.0F ;
	maxNo = 0 ;

	rot.vy += 256 ;
	for ( i = 0; i <= ( 4096 - 512 ) / 256; i ++ ) {
		rot.vx = ExprGroundRX( work, rot.vy ) ;
		DG_SetPos2( &mov, &rot ) ;
		GV_SetVec3( &aim, 0.0F, 0.0F, 16000.0F ) ;
		DG_PutVector( &aim, &aim, 1 ) ;
		rot.vy += 256 ;

 		len = 12000.0F ;
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &mov, &aim,
								    HZX_CHK_ALL, HZX_SEG_NO_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			FVECTOR			hit ;

			HZX_GetOnlinePoint( &hit ) ;
			len = GV_VecLen3F2( &mov, &hit ) ;
		} 
		if ( len > max ) {
			max = len ;
			maxNo = i ;
			DG_COPY_VEC( &maxPos, &aim ) ;
		}
		if ( work->mode != SOL_MODE_BLADE_ONLY ) {
			if ( max > 16000.0F ) break ;
		}
	}
	DG_COPY_VEC( &mt->aim_position, &maxPos ) ;
	mt->aim_len = max ;
	mt->aim_dir = GV_VecDir2FromTo( &work->control.mov, &mt->aim_position ) ;
	mt->aim_way = Way[ maxNo ] ;
	//printf( "%d %d %d\n", maxNo, GV_DiffDirS( work->rot_diff_player.vy, rot.vy - 256 ), Way[ maxNo ] ) ;
}

/* 逃げる位置に向かう */
static	int	MovePosition( Work *work, int ry, float speed )
{
	MoveThink	*mt ;
	FVECTOR		step ;
	SVECTOR		rot ;
	
	mt = &work->move_think ;

	if ( mt->aim_len <= 0 ) return 1 ;

	GV_SetVec3( &step, 0.0F, 0.0F, speed ) ;
	GV_SetVec3( &rot, 0, ry, 0 ) ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_RotVector( &step, &step, 1 ) ;

	_sceVu0AddVector( &work->control.step, &work->control.step, &step ) ;
	SetFlag( work, SOL_FLAG_NO_MOTION_STEP_XZ ) ;
	
	mt->aim_len -= speed ;
	if ( mt->aim_len <= 0 ) return 1 ;
	return 0 ;
}

/* 傾き計算 */
static	int		ExprGroundRX( Work *work, int ry )
{
	FVECTOR		head, vec ;
	SVECTOR		rot ;
	float		hh, diff ;
	int			grot ;
	
	if ( ( work->control.level[ 0 ] == NULL ) ||
		 !( work->control.grounded & 1 ) ||
		( work->control.level[ 0 ]->attribute & HZX_FLOOR_FLAT ) ) return 0 ; 
	if ( ry < 0 ) {
		GV_SetVec3( &rot, 0, work->control.rot.vy, 0 ) ;
	} else {	
		GV_SetVec3( &rot, 0, ry, 0 ) ;
	}
	GV_SetVec3( &head, 0.0F, 0.0F, 500.0F ) ;
	DG_SetPos2( &work->control.mov, &rot ) ;
	DG_PutVector( &head, &head, 1 ) ;
	HZX_SlopeFloorLevel( &hh, &head, work->control.level[ 0 ] ) ;
	diff = hh - work->control.levels[ 0 ] ;
	vec.vx = diff ; vec.vz = 500.0F ;
	grot = - GV_VecDir2( &vec ) ;
	if ( grot < -2048 ) grot += 4096 ;	
	return grot ;
}

/* ダウン傾き */
static	void	GroundSlopeControl( Work *work )
{
	SVECTOR		rot ;
	int			grot ;

	if ( work->control.height > 500.0F ) return ;
	grot = ExprGroundRX( work, -1 ) ;
	rot.vx = grot ;
	rot.vy = rot.vz = 0 ;
	SetRotAdjust( work, &rot, HUMAN21_KOSHI ) ;
}

/* 蛇手用胸調整 */
static	void	ChestSlopeControl( Work *work )
{
	SVECTOR		rot ;
	int			grot ;

	grot = ExprGroundRX( work, -1 ) ;
	rot.vx = grot ;
	rot.vy = rot.vz = 0 ;
	SetRotAdjust( work, &rot, HUMAN21_MUNE ) ;
}

/* ライデンを押してみる */
static	void	PushRaiden( Work *work )
{
	FVECTOR		diff ;
	float		len ;
	
	if ( Flag( work, SOL_FLAG_NO_PUSH ) ) return ;
	DG_COPY_VEC( &diff, &work->pos_diff_player ) ;
	diff.vy = 0.0F ;
	len = GV_VecLen3F( &diff ) ;
	if ( len < 750.0F ) {
		GV_LenVec3F( &diff, &diff, 0.0F, 750.0F - len ) ;
		if ( GM_CheckPlayerStatus( PLAYER_FORCE | PLAYER_BEYOND ) ) {
			/* ソリダスが動く */
			work->control.step.vx -= diff.vx ;
			work->control.step.vz -= diff.vz ;
		} else {
			/* ライデンが動く */
			if ( GM_PlayerControl != NULL ) {
				GM_PlayerControl->step.vx += diff.vx ;
				GM_PlayerControl->step.vz += diff.vz ;
			}
		}
	}
}

/* フェイズチェンジ */
static	void	ChangeMode( Work *work, int mode )
{
	switch( mode ) {
	case SOL_MODE_BLADE_ONLY :
		if ( work->mode != mode ) {
			work->mode = mode ;
			//GM_SeSetMode( SD_V_SOLSTE01, &work->control.mov, GM_SEMODE_BOMB ) ;
			//SOL_SnakeArmDestroy() ;
		}
		break ;
	default :
	  ;
	}
}

/* ライフチェック */
static	int		CheckLife( Work *work, float rate )
{
	float		max ;

	max = ( float )work->life_gage.max ;
	if ( work->life_gage.value < ( int )( max * rate ) ) return 1 ;
	max = ( float )work->life_gage.m9_max ;
	if ( work->life_gage.m9_value < ( int )( max * rate ) ) return 1 ;
	return 0 ;
}

/* プロック実行（引数２つ） */
static	void	ExecProc( Work *work, int proc_no, int v1, int v2 )
{
	GCL_ARGS	args ;
	int			buf[ 2 ], proc ;

	proc = work->procs[ proc_no ] ;
	if ( proc <= 0 ) return ;

	args.argc = 2 ;
	args.argv = buf ;
	buf[ 0 ] = v1 ;
	buf[ 1 ] = v2 ;
	GM_ExecProc( proc, &args ) ;
}

/* フェイズチェンジチェック */
static	int		CheckChangePhaseAttack( Work *work )
{
	if ( work->mode == SOL_MODE_BLADE_ONLY ||
		 GV_Time - work->careful_time > 6 * ( 300 / TIME_BASE ) ) {
		printf( "sol change phase -> attack\n" ) ;
		work->careful_time = GV_Time ;
		work->flag &= ~SOL_FLAG_CAREFUL ;
		SetMode( work, StandStillK ) ;
		return 1 ;
	}
	return 0 ;
}

static	int		CheckChangePhaseCareful( Work *work )
{
	if ( work->mode == SOL_MODE_BLADE_ONLY ) return 0 ;
	if ( GV_Time - work->careful_time > 12 * ( 300 / TIME_BASE ) &&
		 work->len_diff_player > 3000.0F ) {
		printf( "sol change phase -> careful\n" ) ;
		work->careful_time = GV_Time ;
		work->flag |= SOL_FLAG_CAREFUL ;
		SetMode( work, StandStillC ) ;
		return 1 ;
	}
	return 0 ;
}

/* スウェーチェック */
static	int		CheckSway( Work *work )
{
	int			atk ;

	if ( !PL_SlashNow() ) return 0 ;
	if ( work->len_diff_player > 3500.0F ) return 0 ;
	if ( CheckChangePhaseAttack( work ) ) {
		return 1 ;
	}

	atk = PL_SlashMode() ;
	switch( atk ) {
	case ATK_DOWN_L :
	case ATK_DOWN_R :
	case ATK_VERTICAL :
	case ATK_RIGHT :
	case ATK_LEFT :
	case ATK_THRUST :
		SetMode( work, SwayUpper ) ;
		break ;
	case ATK_UP_L :
	case ATK_UP_R :
		SetMode( work, SwayLower ) ;
	}
	GM_SeSetMode( SD_V_SOLATK02, &work->control.mov, GM_SEMODE_BOMB ) ;
	return 1 ;
}

/* ＳＥセット */
static	void	SeSet( int seNo, FVECTOR *mov, int seMode, int mode )
{
	/* mode == 0 : 通常 mode == 1 : 強制 */
	if ( mode == 0 && 
		 GV_Time - SeTime < 2 * ( 300 / TIME_BASE ) ) {
		printf( "mode 0 se cancel!!\n" ) ;
		return ;
	}
	if ( mode == 1 &&
		 GV_Time - LastPlayerDamageSeTime < 3 * ( 300 / TIME_BASE ) ) {
		printf( "mode 1 se cancel!!\n" ) ;
		return ;
	}
	if ( seNo == SD_V_SOLCHO04 && GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		seNo = SD_V_SOLCHO02 ;
	}
	GM_JimakuSeSetMode( seNo, mov, seMode ) ;
}

/* ダッシュファイヤー */
static	int		DashFire( Work *work, int objNo, int time ) 
{
	int			no ;

	no = work->dash_fire_no ;
	if ( ++ work->dash_fire_no == 8 ) work->dash_fire_no = 0 ;
	work->dash_fire_flag[ no ] = 0 ;
	NewSolidusDashFire( ( FVECTOR * )work->body.objs->objs[ objNo ].world.m[ 3 ],
					    &work->dash_fire_flag[ no ], time ) ;
	//printf( "dash fire start %d\n", no ) ;
	work->dash_fire_current = no ;
	return no ;
}

static	void	DashFireStop( Work *work )
{
	if ( work->dash_fire_current < 0 ) return ;
	work->dash_fire_flag[ work->dash_fire_current ] = 1 ;
	//printf( "dash fire stop %d\n", work->dash_fire_current ) ;
	work->dash_fire_current = -1 ;
}

static	int		CheckArmUndoGo( Work *work )
{
	if ( SOL_GameLevel >= GM_LEVEL_HARD ) return CheckLife( work, 0.50F ) ;
	return CheckLife( work, 0.40F ) ;
}

static	void	JimakuShow( char *string )
{
	extern void	GM_JimakuShow( int, char * ) ;

	if ( GM_Configuration & GM_CONFIG_CAPTION_OFF ) return ;
	if ( string == NULL ) return ;
	/* ボスラッシュ対処。スネークのときはなにもしない */
	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) return ;
	GM_JimakuShow( 0, string ) ;
}

static	void	JimakuHide( void )
{
	extern void	GM_JimakuHide( void ) ;
	if ( GM_Configuration & GM_CONFIG_CAPTION_OFF ) return ;
	/* ボスラッシュ対処。スネークのときはなにもしない */
	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) return ;
	GM_JimakuHide() ;
}

/*----------------------------------------------------------------*/

/* 各国語ゲームレベル対応吸収 */
static	void	SetSolGameLevel( void )
{
	SOL_GameLevel = GM_GameLevel ;
	if ( GM_GameLevel == GM_LEVEL_VERYEASY ) {
		SOL_GameLevel = GM_LEVEL_VERYEASY ;
	} else if ( GM_GameLevel == GM_LEVEL_EASY ) {
		SOL_GameLevel = GM_LEVEL_NORMAL ;
	} else if ( GM_GameLevel == GM_LEVEL_NORMAL ) {
		SOL_GameLevel = GM_LEVEL_NORMAL ;
	} else if ( GM_GameLevel == GM_LEVEL_HARD ) {
		SOL_GameLevel = GM_LEVEL_HARD ;
	} else if ( GM_GameLevel >= GM_LEVEL_EXTREME ) {
		SOL_GameLevel = GM_LEVEL_EXTREME ;
	}
}


