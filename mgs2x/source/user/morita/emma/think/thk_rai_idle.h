/*
  thk_rai_idle.h

  ライデン手繋ぎ静止時 思考処理

  2001/02/14 T.Morita Revised
  $Id: thk_rai_idle.h,v 1.1.1.3 2002/11/19 11:46:05 Yoshizawa1 Exp $
*/

#define DIR_VEL_MAX  200 
#define DIR_VEL_RATE 8 

/* くるりんバグ防止 */
void EMA_ChangeDir( Work *work, int turn )
{

#if 1 /*8.3*/
    turn -= work->control.rot.vy ;
    turn  = EMA_RecalcDir( turn ) ;
    if ( !EMA_Flag( EMA_F_FIRST_LINK ) )
    {
	if ( work->diff_turn> 2048 && turn>0 )
	    work->frce_turn = -4096 ;
	if ( work->diff_turn<-2048 && turn<0 )
	    work->frce_turn =  4096 ;
	if ( (work->frce_turn < 0 && work->diff_turn < -(turn+work->frce_turn)/8)||
	     (work->frce_turn > 0 && work->diff_turn >  (turn+work->frce_turn)/8) )
	    work->frce_turn = 0 ;
	turn += work->frce_turn ;
    }
#else
    turn -= work->control.rot.vy ;
    turn  = EMA_RecalcDir( turn ) ;

    if ( work->diff_turn>0 && turn>0 )
	turn -= 4096 ;
    if ( work->diff_turn<0 && turn<0 )
	turn += 4096 ;
#endif

    turn = (turn> DIR_VEL_MAX*DIR_VEL_RATE ?  DIR_VEL_MAX :
	    turn<-DIR_VEL_MAX*DIR_VEL_RATE ? -DIR_VEL_MAX :
	                                      turn/DIR_VEL_RATE) ;

    work->control.turn.vy = work->control.rot.vy += turn ;
}


static void EMA_ThinkRaiIdle( Work *work )
{
    FVECTOR diff, aim, play ;
    FMATRIX world ;
    float   x, z, d ;
    //float   dist ;
    //float   rot  ;
    float speed ;

    /* エマの実際の速度を計る(壁を歩き続けるので..) */
    _sceVu0SubVector( &diff, &work->control.mov, &work->prev_pos ) ;
    speed = diff.vx*diff.vx + diff.vz*diff.vz ;


#if 0    /* 「ゾーンが違うから機能」はうまくいかなかった */
    if ( HZX_Zone1( GM_PlayerControl->addr ) !=
	 HZX_Zone1( work->control.addr     ) )
	if ( !GM_Navi( &work->navigate, &work->navitrg, 300 ) )
	{
	    work->npc.action.dir = work->navigate.next_dir ;
	    work->npc.action.pad = RAI_WALK ;
	    return ;
	}
#endif


    if ( GM_CheckPlayerStatus( PLAYER_WATCH ) )/*主観時は,移動しない*/
    {
	x = GM_PlayerPosition.vx - work->control.mov.vx ;
	z = GM_PlayerPosition.vz - work->control.mov.vz ;

#if 0
	d = _sceVu0InnerProduct( work->body.objs->world.m[Z], 
				 GM_PlayerBody->objs->world.m[Z] ) ;
				 /*DG_Chanls[0].eye.m[Z]) ;*/
#else
	d = (x * GM_PlayerBody->objs->world.m[Z][X] +
	     z * GM_PlayerBody->objs->world.m[Z][Z]) / work->pl_dis ;
#endif

	if ( d <= 0.2f )
	    PLY_EE_ResetGraspModeAndTrigger() ;

	work->npc.action.pad = RAI_IDLE ;
	return ;
    }
    work->control.interp = 20 ;

    EMA_ThinkDestinatePos( work, &diff, &aim, &world, 5 ) ;
    d = diff.vx*diff.vx + diff.vz*diff.vz ;
    if ( d < 800.0f*800.0f )
	EMA_SetFlag( EMA_F_ENB_ARM_IK ) ;
    else
	EMA_ResetFlag( EMA_F_ENB_ARM_IK ) ;

    if ( d >= 150.0f*150.0f )
    {
	_sceVu0SubVector( &play, &aim, &GM_PlayerControl->mov ) ;
	_sceVu0Normalize( &play, &play ) ;
	_sceVu0Normalize( &diff, &diff ) ;
	if ( *work->ext_trg != work->prv_trg )
	{
	    if ( diff.vx*play.vz < diff.vz*play.vx )
		EMA_SetFlag( EMA_F_IS_ON_LEFT ) ;
	    else
		EMA_ResetFlag( EMA_F_IS_ON_LEFT ) ;
	}

	/*目的地がライデンによって遮られている？*/
	if ( d >= 600.0f*600.0f )
	    if ( _sceVu0InnerProduct( &play, &diff ) > 0.90596017f )
	    {
		/*目的地が遮られているのでライデンを避けて回り込む*/
		_sceVu0ApplyMatrix( &aim,
				    &world,
				    &EMA_Shifts[ EMA_Flag(EMA_F_IS_ON_LEFT) ? 3 : 4 ] ) ;
		_sceVu0SubVector( &diff, &aim, &work->control.mov ) ;
	    }

	if ( work->time>1 && speed < 10.0f )
	    EMA_SetFlag( EMA_F_FIRST_WALL ) ;
	if ( EMA_UtilDestinateToWall( work, &diff ) ||
	     EMA_Flag( EMA_F_FIRST_WALL ) )
	{
	    work->control.step.vx = 0.0f ;
	    work->control.step.vz = 0.0f ;
	    work->npc.action.pad = RAI_IDLE ;
	}
	else
	{
#if 0
	    work->control.turn.vy = RAD2ANG( atan2f( diff.vx, diff.vz ) ) ;
	    work->control.turn.vy = EMA_RecalcDir( work->control.turn.vy ) ;
#else
	    EMA_ChangeDir( work, RAD2ANG( atan2f( diff.vx, diff.vz ) ) ) ;
#endif
	    work->npc.action.pad = RAI_WALK ;
	}
    }
    else
    {
	/* 位置を２０フレームで修正 */
	EMA_SetAdjustPosition( work, &aim, 10.0f ) ;
	/* 静止モーションに修正 */
	work->npc.action.pad = RAI_IDLE ;

	EMA_ChangeDir( work, GM_PlayerControl->turn.vy ) ;
    }
}
