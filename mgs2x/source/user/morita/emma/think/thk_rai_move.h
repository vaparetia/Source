/*
  thk_rai_move.h
  ライデン手繋ぎ移動時 思考処理

  2001/02/14 T.Morita Revised
  $Id: thk_rai_move.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/


static void EMA_ThinkRaiMove( Work *work )
{
    FVECTOR diff ;
    FVECTOR stay ;
    float speed ;
    float t, d, dist ;
    short dir ;

    /* 変に離れているので手を切る */
    if ( work->pl_dis > 1500.0f )
    {
	PLY_EE_ResetGraspModeAndTrigger() ;
	return ;
    }

    /* IKおよび顔追尾をＯＮにする */
    EMA_ResetSetFlag( EMA_F_FIRST_WALL|EMA_F_NON_FACING, EMA_F_ENB_ARM_IK ) ;

    /* 初めて手を繋いできたらturn_diffをリセットする */
    if ( EMA_Flag( EMA_F_FIRST_LINK ) )
    {
	EMA_ResetFlag( EMA_F_FIRST_LINK ) ;
	EMA_ThinkResetDiff( work, 1 ) ;
    }

    /* プレーヤの実際の速度を計る(壁を歩き続けるので..) */
    _sceVu0SubVector( &diff, &GM_PlayerControl->mov, &work->ply_prv_pos ) ;
    _sceVu0CopyVector( &work->ply_prv_pos, &GM_PlayerControl->mov ) ;
    speed = diff.vx*diff.vx + diff.vz*diff.vz ;

#if 0
    /* 手の距離で止まるかどうかを決める */
    _sceVu0SubVector( &diff, 
		      BODYPOS(&work->body, HUMAN21_HIDARI_TE ),
		      BODYPOS(GM_PlayerBody, HUMAN21_MIGI_TE ) ) ;
#endif

    /* プレーヤーどっち周り？ */
    dir = EMA_DirectionDiff( GM_PlayerControl->turn.vy -
			     GM_PlayerControl->rot.vy    ) ;
    if ( dir >= 0 )
	/* 右周りなのでの右手を目指す         */
	_sceVu0CopyVector( &diff, BODYPOS(GM_PlayerBody, HUMAN21_MIGI_TE ) ) ;
    else
	/* 左周りなのでの後方の中心部を目指す */
	_sceVu0ApplyMatrix( &diff, &GM_PlayerBody->objs->world,
			    &EMA_Shifts[1] ) ;

    /* 静止位置への補間 スピード下がれば静止位置を目標にし出す */
    /* 目標補正係数  ０に近いほど目標位置を静止位置に近付ける */
    t = speed / 620.0f ;
    if ( t < 1.0f )
    {
	_sceVu0ApplyMatrix( &stay,
			    &GM_PlayerBody->objs->world,
			    &EMA_Shifts[0] ) ;
	_sceVu0ScaleVector( &stay, &stay, 1.0f-t ) ;
	_sceVu0ScaleVector( &diff, &diff, t      ) ;
	_sceVu0AddVector( &diff, &diff, &stay ) ;    
	if ( t < 0.2f )
	    work->npc.action.pad = RAI_WALK ;
    }
    _sceVu0SubVector( &diff, &diff, &work->control.mov ) ;
    dist = diff.vx*diff.vx + diff.vz*diff.vz ;

    /* 静止半径をエマの歩行(状態)スピードに合わせる  */
#define RAI_WALK_DIST 100.0f*100.0f
#define RAI_RUN_DIST  300.0f*300.0f
	if ( speed == 0.0f ) speed = 0.00001f ;
    d = 22500.0f*700.0f/speed ;
    if ( work->npc.action.pad == RAI_WALK && d > RAI_WALK_DIST )
	d = RAI_WALK_DIST ;
    if ( work->npc.action.pad == RAI_RUN  && d > RAI_RUN_DIST  )
	d = RAI_RUN_DIST ;
    if ( dist >= d )
    {
	/* 手が追いついていないのでモーションの再生を早める */
#if 1
	d = sceVu0Sqrt( dist ) -/*160.0f*/ 200.0f ;
	if ( work->npc.action.pad == RAI_WALK )
	    d /= EMA_Flag( EMA_F_IS_ON_STEP ) ? 260.0f : 100.0f ;
	else
	    d /= EMA_Flag( EMA_F_IS_ON_STEP ) ? 260.0f : 300.0f ;
	d = d<0.3f ? 0.3f : d ;
	//d = d<0.1f ? 0.1f : d ;
#else
	d = dist -100.0f*100.0f ;
	d /= EMA_Flag( EMA_F_IS_ON_STEP ) ? 260.0f*260.0f : 460.0f*460.0f ;
	d = d<0.5f ? 0.5f : d ;
#endif
	MT_SetMotionSpeed( work->body.m_ctrl,  (float)TIME_BASE*d ) ;

	/* ライデンの手の方向を計算する */
	dir = RAD2ANG( atan2f( diff.vx, diff.vz ) ) ;

	/* ライデンの手を追いかける */
	EMA_ChangeDir( work, dir ) ;

	//work->walk_cnt = t < 0.3f ? 2 : 11 ;
	work->walk_cnt = 11 ;
    }

    /* 近過ぎるので止まる */
    else if ( work->walk_cnt <= 0 )
    {
	EMA_ChangeDir( work, work->pl_dir ) ;
	work->npc.action.pad = RAI_IDLE ;
    }

    /* 惰性で進む */
    else
    {
	/* 再生＆補間スピードを元に戻す */
	MT_SetMotionSpeed( work->body.m_ctrl, -1.0f ) ;
	//work->control.interp = 0 ;

	work->walk_cnt-- ;
    }
}
