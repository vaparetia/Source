/*
   careful_sol.c
   ソリダス／間合いを取ったり、躱したり

   2001/06/10	M.Sonoyama
   $Id: careful_sol.c,v 1.1.1.3 2002/11/19 11:51:04 Yoshizawa1 Exp $
*/

/* 
   間合いフェーズ

   ４Ｍ前後をキープしようとする
*/

#define	KEEP_LENGTH		(2800.0F)
#define	FAR_LENGTH		(3600.0F)
#define	LENGTH_CUSHION	(500.0F)

/* 間合いフェーズ静止 */
static	void	StandStillC( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_fire_idle, 0, 12 ) ;
		SOL_SetSnakeArmMotion( AMkwt_fire_idle, 16 ) ;
	}

	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD ) ;
	if ( !HeadToPlayer( work ) ) DirPlayer( work ) ;

	if ( time < 150 ) return ;

	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}

	/* 攻撃フェーズとの切り替え */
	if ( CheckChangePhaseAttack( work ) ) {
		return ;
	}
	if ( CheckSway( work ) ) {
		return ;
	}

	if ( work->len_diff_player < KEEP_LENGTH ) {
		SetMode( work, WalkBackC ) ;
	} else if ( work->len_diff_player > FAR_LENGTH ) {
		SetMode( work, StandWalkC ) ;
	}

	/* 静止が続くようならちょっかい */
	if ( time > ( int )( 300.0F * 1.50F ) ) {
		SelectArmAttack( work ) ;
	}
}

/* 間合いつめ */
static	void	StandWalkC( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mkwt_run, 0, 12 ) ;
		SOL_SetSnakeArmMotion( AMkwt_run, 16 ) ;
		MotionSpeed( work, 1.50F ) ;
	}
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD | SOL_FLAG_NO_IK ) ;
	DirPlayer( work ) ;

	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}

	/* 攻撃フェーズとの切り替え */
	if ( CheckChangePhaseAttack( work ) ) {
		return ;
	}
	if ( CheckSway( work ) ) {
		return ;
	}

	if ( work->len_diff_player < FAR_LENGTH - LENGTH_CUSHION ) {
		SetMode( work, StandStillC ) ;
	}
}

/* 間合いとり */
static	void	WalkBackC( Work *work, int time )
{
	if ( time == 0 ) {
		SetAction( work, Mwalk_b, 0, 12 ) ;
		SOL_SetSnakeArmMotion( AMwalk_b, 16 ) ;
		MotionSpeed( work, 0.30F ) ;
	}
	SetFlag( work, SOL_FLAG_BLADE_GUARD | SOL_FLAG_COMBO_GUARD | SOL_FLAG_NO_IK ) ;
	DirPlayer( work ) ;

	if ( CheckEludeAttack( work ) ) {
		SetMode( work, EludeAttackReady ) ;
		return ;
	}

	/* 後に壁がある */
	{
		FVECTOR	chk_back = { 0.0F, 0.0F, -650.0F } ;

		DG_SetPos2( &work->control.mov, &work->control.rot ) ;
		DG_PutVector( &chk_back, &chk_back, 1 ) ;
		if ( HZX_OnlineHazardCheck( work->control.hzx_id,
								    &work->control.mov, &chk_back,
								    HZX_CHK_ALL, 0, 0 ) ) {
			work->careful_time = 0 ;
		}
	}

	/* 攻撃フェーズとの切り替え */
	if ( CheckChangePhaseAttack( work ) ) {
		return ;
	}
	if ( CheckSway( work ) ) {
		return ;
	}

	//if ( work->len_diff_player > KEEP_LENGTH + LENGTH_CUSHION ) {
	if ( EndMotion( work ) ) {
		SetMode( work, StandStillC ) ;
	}
}
