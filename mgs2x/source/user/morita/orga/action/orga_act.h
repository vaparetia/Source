/*
   orga_act.h
   オルガ 行動処理関数群

   2000/01/05 T.Morita
   $Id: orga_act.h,v 1.1.1.3 2002/11/19 11:46:21 Yoshizawa1 Exp $
*/

/***

  Subroutines for Acting as for <work->act>

  ***/
static float FaceAtoB( Work *work, FVECTOR *a, FVECTOR *b )
{
    FVECTOR d ;

    _sceVu0SubVector( &d, a, b ) ;
    work->control.turn.vy = (short)(2048.0f / M_PI * atan2f( d.vx, d.vz )) ;

    return d.vx*d.vx+d.vz*d.vz ;
}

static inline float CheckDestanceLineAndPoint( FVECTOR *from, FVECTOR *to, FVECTOR *p )
{
    FVECTOR t1, t2, a ;

    _sceVu0SubVector( &t1, to, from ) ;
    _sceVu0SubVector( &t2,  p, from ) ;
    _sceVu0OuterProduct( &a, &t2, &t1 ) ;
    _sceVu0OuterProduct( &a, &a , &t1 ) ;
    _sceVu0Normalize( &a, &a ) ;

    return -_sceVu0InnerProduct( &a, &t2 ) ;
}

static inline void FaceEyeAt( Work *work, FVECTOR *a )
{
    work->trgt_eye = a ;
}

/*
*/
static void FaceAimAt( Work *work )
{
    work->trgt_aim = ORG_RecogPlayerPos( HUMAN21_KUBI/*ORG_AimJoint[i]*/ ) ;
    FaceEyeAt( work, work->trgt_aim ) ;

    if ( !(work->flag & ORGA_F_NOMISS_SHOT) && GM_GameLevel<GM_LEVEL_EXTREME )
    {
	int i ;
	float d, min = 1000.0f ;
	FVECTOR *hand = BODYPOS( &work->body, HUMAN21_MIGI_TE ) ;

	if (/* !(work->ply_hide & ORGA_F_PLYR_HIDING) &&*/
	     (work->ply_hide & ORGA_F_PLYR_HIDTIM) > 2 )
	    for ( i=ORG_N_DUMMYPOS ; --i>=0 ; )
		if ( (d = CheckDestanceLineAndPoint( hand, work->trgt_aim, &ORG_DummyPos[i] )) < min )
		    work->trgt_aim = &ORG_DummyPos[i], min = d ;
    }
}

static inline void ChangeFlag( Work *work, u_int set, u_int reset )
{
    work->flag &= ~reset ;
    work->flag |=  set   ;
}

static inline void CalcCurrentPosAdjToTrgt( Work *work, FVECTOR *offset, float frames )
{
    int r = work->control.turn.vy & 0x0fff ;
    FMATRIX mtx ;
    FVECTOR *trgt = work->trgt_nxt ? work->trgt_nxt : work->trgt_pos ;

    if ( trgt )
    {
	_sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
	_sceVu0CopyVector( (FVECTOR*)&mtx.m[W], BODYPOS( &work->body, HUMAN21_KOSHI ) ) ;
	_sceVu0ApplyMatrix( &work->pos_adj, &mtx, offset ) ;
	_sceVu0SubVector( &work->pos_adj, trgt, &work->pos_adj ) ;
	_sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/frames ) ;

	work->pos_adj.vw = frames ;
    }
}

static inline void CalcCurrentPosAdj( Work *work, FVECTOR *offset, float frames )
{
    int r = work->control.turn.vy & 0x0fff ;
    FMATRIX mtx ;

    _sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, (r-4096*(r>>11))*(float)M_PI/2048.0F ) ;
    ApplyMatrixXYZ( &work->pos_adj, &mtx, offset ) ;
    _sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/frames ) ;

    work->pos_adj.vw = frames ;
}

static inline void CalcTargetPosAdj( Work *work, FVECTOR *trgt_pos, float frames )
{
    if ( trgt_pos )
    {
	_sceVu0SubVector( &work->pos_adj, trgt_pos, &work->control.mov ) ;
	_sceVu0ScaleVector( &work->pos_adj, &work->pos_adj, 1.0f/frames ) ;
	work->pos_adj.vw = frames ;
    }
}


static void ResetAll( Work *work, int set, int reset )
{
    /* 反転ビットを元に戻す */
    work->body.m_ctrl->flag &= ~(MT_FLAG_REVERSAL1| MT_FLAG_REVERSAL2) ;

    /* オーバーレイを切る */
    if ( work->flag & ORGA_F_OVERLAYED )
	GM_ConfigObjectAction( &work->body, 1, -1, 0, ORGA_BODY_ALL,
			       30*work->act_speed/ORGA_NORMAL_SPEED ) ;
    /* 再生速度をデフォルトに */
    MT_SetMotionSpeed( work->body.m_ctrl, -1 ) ;

    /* 目と射撃のデフォルトはプレイヤーに */
    //work->trgt_aim = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
    //FaceEyeAt( work, work->trgt_aim ) ;

    work->weap_usp.vw = -4.0f ; /* USPを右手に持つ */
    work->weap_amo    = -2    ; /* hide cartridge  */
    work->weap_sgr.vw = -2.0f ; /* SGRを隠す（持っている物だけ） */
    work->voice_tim   =  0    ;
    work->speed       =  1.0f ;
    ChangeFlag( work, set, reset| (ORGA_F_OVERLAYED| ORGA_F_NVR_RESET) ) ;
}

static inline void FullResetAll( Work *work, int set, int reset )
{
    ResetAll( work, set, reset ) ;

    /* 突発性なためターゲット位置をクリアする */
    work->n_trgt_his = 0 ;
    work->trgt_pos = NULL ;

    /* 攻撃用のパラメータをクリア */
    work->avoid_flg = 0 ;
    work->aim_flg   = 0 ;
    work->exposed   = 0 ;

    /* 行動時間をリセットする */
    work->act_time = 0 ;
    work->act_tic  = 0 ;
}




/***

  Act Routines <work->act>

  ***/
static inline void ORG_ActMotionOnceLayer0( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
	work->act++ ;
}
static void ORG_ActAvoidMotionOnceLayer0( Work *work )
{
    if ( work->flag & (ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) )
	if ( work->avoid_flg )
	    work->act = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID ) ;
    ORG_ActMotionOnceLayer0( work ) ;
}
static inline void ORG_ActMotionOnceLayer1( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 1 ) )
	work->act++ ;
}

#if 0 
/* 今は使われていないので コメントアウト*/
static void ORG_ActAvoidMotionOnceLayer1( Work *work )
{
    if ( work->flag & (ORGA_F_UNRECOG_AIM| ORGA_F_UNRECOG_POS) )
    if ( work->avoid_flg )
	work->act = ORG_ActionAbort, ChangeFlag( work, ORGA_F_NONE, ORGA_F_ENB_AVOID ) ;
    ORG_ActMotionOnceLayer1( work ) ;
}

static void ORG_ActMotionOnceLayer0ReversalEnd( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 0 ) )
    {
	work->act++ ;
	MT_SetMotionInterp( work->body.m_ctrl, 50, ORGA_BODY_ALL );
	work->body.m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2) ;
    }
}

static void ORG_ActMotionOnceLayer1ReversalEnd( Work *work )
{
    if ( GM_CheckObject_IsEnd( &work->body, 1 ) )
    {
	work->act++ ;
	MT_SetMotionInterp( work->body.m_ctrl, 50, ORGA_BODY_ALL );
	work->body.m_ctrl->flag &= ~(MT_FLAG_REVERSAL1|MT_FLAG_REVERSAL2) ;
    }
}
#endif

