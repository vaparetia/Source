/*
  thk_afraid.h
  危険モード思考処理

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: thk_afraid.h,v 1.1.1.3 2002/11/19 11:46:04 Yoshizawa1 Exp $
*/


/*

  セーフゾーンの方向に向かった場合,その方向に敵がいるかどうかの判定。
  near_flagが立っていると3m以上はなれた敵を考慮に入れない。

*/
static int IsValidZone( HZX_ZONE_ADD zoneaddr, HZX_ZONE_ADD safeaddr )
{
    /* ゾーンが途切れているので無効 */
    if ( GM_GetRIntrptZ2Z( zoneaddr, safeaddr ) )
	return 0 ;
    if ( GM_GetRIntrptCheckStatus( zoneaddr, safeaddr, ROOT_INTRPT_NONPC ) )
	return 0 ;
    return 1 ;
}

static int IsNoEnemy( HZX_ZONE_ADD zoneaddr, HZX_ZONE_ADD safeaddr,
		      CONTROL *ctrl, float *min_dir, int near_flag )
{
    HOMING_TRG *trgt ;
    HZX_ZON    *safe = HZX_GetZoneFromAdd( safeaddr ) ;
    float       max_dir = 0.0f ;
    float       avr_dir = 0.0f ;

    /* セーフゾーンが侵入禁止である */
    if ( safe->flag & HZX_ZONE_ZINTRPT )
	return 0 ;

    /* 全部の敵を検索して,ゾーンの方向に無いことを確かめる */
    for ( trgt=GM_GetHoming() ; trgt ; trgt=trgt->next )
	if ( !(trgt->status & HOMING_SKIP) )
	    if ( trgt->status & HOMING_ENEMY )
	    {
		float sx,sz, vx,vz ;
		float d ;

		sx = safe->x - ctrl->mov.vx ;
		sz = safe->z - ctrl->mov.vz ;
		vx = trgt->ctrl->mov.vx - ctrl->mov.vx ;
		vz = trgt->ctrl->mov.vz - ctrl->mov.vz ;

		d = vx*vx + vz*vz ;
		if ( near_flag )
		    if ( d > 3000.0f*3000.0f )
			continue ;
		d  = sceVu0Sqrt( d ) ;
		d *= sceVu0Sqrt( sx*sx + sz*sz ) ;
		d  = (sx*vx + sz*vz)/d ;

		avr_dir += d ;

		/* セーフゾーンの方に敵がいるの？ */
		if ( max_dir < d )
		    max_dir = d ;
	    }

    /* 前の方に敵がいない */
    if ( max_dir > 0.77f )
	return 0 ;
    /* 方向的に最も敵が少ない */
    if ( *min_dir <= avr_dir )
	return 0 ;
    *min_dir = avr_dir ;

    return 1 ;
}









/*
  最も近い安全地帯を探索 

  危険地帯のゾーンアドレス(zoneaddr) と 自分のコントロール(ctrl) を渡すと
  安全なゾーンアドレスを返す。
 */
int EMA_GetAvoidZone( HZX_ZONE_ADD zoneaddr, CONTROL *ctrl )
{
    int      addr, safeaddr ;
    int      i ;
    HZX_ZON *zon = HZX_GetZoneFromAdd( zoneaddr ) ;
    float    min_dir ;

    ASSERT( zon ) ;

    min_dir = 1.0f ;
    addr = zoneaddr | 0xffff ;
    for( i=6 ; --i>=0 ; )
	if ( zon->nears[i] != 255 )
	{
	    safeaddr  = zoneaddr & 0xffff0000 ;
	    safeaddr |= (int)zon->nears[i] << 8 ;
	    safeaddr |= (int)zon->nears[i] << 0 ;

	    /* ゾーン先に行けるのか？ */
	    if ( IsValidZone( zoneaddr, safeaddr ) )
		/* セーフゾーンの先に敵はいないか */
		if ( IsNoEnemy( zoneaddr, safeaddr, ctrl, &min_dir, 1 ) )
		    addr = safeaddr ;
	}

    /* 同じゾーン番号なので移動の必要が無い→ */
    if ( HZX_Zone1( zoneaddr) == HZX_Zone1( addr ) )
	return zoneaddr | 0xffff ;
    return addr ;
}

/*
  最も近い安全地帯を探索 

  危険地帯のゾーンアドレス(zoneaddr) と 自分のコントロール(ctrl) を渡すと
  安全なゾーンアドレスを返す。
 */
int EMA_GetNearestSafeZone( HZX_ZONE_ADD zoneaddr, CONTROL *ctrl )
{
    int      addr, safeaddr ;
    int      i ;
    HZX_ZON *enemy = HZX_GetZoneFromAdd( zoneaddr ) ;
    float    min_dir ;

    ASSERT( enemy ) ;

    min_dir = 1.0f ;
    addr = zoneaddr | 0xffff ;
    for( i=HZX_MAX_SAFEZONE_NUM ; --i>=0 ; )
	if ( enemy->safes[i] != 255 )
	{
	    safeaddr  = zoneaddr & 0xffff0000 ;
	    safeaddr |= (int)enemy->safes[i] << 8 ;
	    safeaddr |= (int)enemy->safes[i] << 0 ;

	    /* ゾーン先に行けるのか？ */
	    if ( IsValidZone( zoneaddr, safeaddr ) )
		/* セーフゾーンの先に敵はいないか */
		if ( IsNoEnemy( zoneaddr, safeaddr, ctrl, &min_dir, 0 ) )
		    addr = safeaddr ;
	}

    /* 同じゾーン番号なので移動の必要が無い→ */
    if ( HZX_Zone1( zoneaddr) == HZX_Zone1( addr ) )
	return zoneaddr | 0xffff ;

    return addr ;
}

CONTROL *EMA_GetNearestEnemy( FVECTOR *from, float *min, int *dir )
{
    HOMING_TRG *trgt = GM_GetHoming() ;
    HOMING_TRG *near = NULL ;
    FVECTOR     diff ;
    float       d ;

    *min = 8000.0f*8000.0f ; /* 索敵範囲 最大距離 */
    while( trgt )
    {
	if ( !(trgt->status & HOMING_SKIP) )
	    if ( trgt->status & HOMING_ENEMY )
	    {
		_sceVu0SubVector( &diff, from, &trgt->ctrl->mov ) ;
		d = diff.vx*diff.vx+diff.vz*diff.vz ;
		if ( d < *min )
		{
		    *min = d, near = trgt ;
		    if ( dir )
			*dir = atan2( diff.vx, diff.vz ) * 2048.0f / M_PI ;
		}
	    }
	trgt = trgt->next ;
    }

    if ( near )
	return near->ctrl ;
    else
	return NULL ;
}


static int EMA_ThinkIsMovable( Work *work )
{
    /* 徘徊禁止なので止める */
    if ( !EMA_Flag( EMA_F_ENB_WANDER ) )
	return 0 ;

    /* w31d個別対応 ガンカメ部屋でガンカメが壊れているので止める */
    if ( work->stage == EMA_STG_W31D )
	if ( EMA_GetGunCameraBrokenNum() == 2 )
	    if ( work->control.mov.vx >=   -8600.0f &&
		 work->control.mov.vx <=   -5000.0f &&
		 work->control.mov.vz >= -234000.0f &&
		 work->control.mov.vz <= -231000.0f )
		return 0 ;

    return 1 ;
}

static void EMA_ThinkAfraidIdle( Work *work )
{
    int      addr ;
    CONTROL *enemy ;
    float    radius ;

    /* 再生＆補間スピードを元に戻す */
    MT_SetMotionSpeed( work->body.m_ctrl, -1.0f ) ;
    work->npc.action.pad = AFRAID_IDLE ;

    /* 徘徊禁止なので止める */
    if ( !EMA_ThinkIsMovable( work ) )
	return ;

    if ( EMA_ThinkCheckOutSideEmma( work ) )
    {
	if ( EMA_SetNaviTarget( work, GM_PlayerControl->addr, NULL ) )
	    work->npc.action.pad = AFRAID_WALK ;
    }

    else if ( (enemy = EMA_GetNearestEnemy( &work->control.mov,
					    &radius, NULL )) )
    {
	if ( radius < 2000.0f*2000.0f )
	{
	    addr = EMA_GetAvoidZone( work->control.addr, &work->control ) ;

	    if ( EMA_SetNaviTarget( work, addr, NULL ) )
		if ( !EMA_ThinkCheckOutSide( work, &work->navitrg.pos ) )
		    work->npc.action.pad = AFRAID_WALK ;
	}

	else if ( work->time > 50 || radius < 4000.0f*4000.0f )
	{
	    /* 安全な場所を捜す */
	    addr = EMA_GetNearestSafeZone( enemy->addr, &work->control ) ;

	    if ( EMA_SetNaviTarget( work, addr, NULL ) )
		if ( !EMA_ThinkCheckOutSide( work, &work->navitrg.pos ) )
		    work->npc.action.pad = AFRAID_WALK ;
	}
    }
}


static void EMA_ThinkAfraidMove( Work *work )
{
#if 0
    HZX_ViewZone( work->control.hzx_id, work->navitrg.addr ) ;
#endif

    if ( !EMA_ThinkIsMovable( work ) )
    {
	work->npc.action.pad = AFRAID_IDLE ;
	return ;
    }

    /* nvtrgに向ってゾーン移動 */
    if ( GM_Navi( &work->navigate, &work->navitrg, 500 ) )
    {
	EMA_ResetFlag( EMA_F_IS_COMMING ) ;
	work->npc.action.pad = AFRAID_IDLE ;
	EMA_SetAdjustPosition( work, &work->navitrg.pos, 20.0f ) ;
    }
    else
    {

	//printf ( "Emma : NAVI Interrpt %x\n", GM_GetRIntrpt( work->control.addr, work->navigate.next_addr ) ) ;

	if ( !EMA_Flag( EMA_F_IS_COMMING ) )
	{
	    R_INTRPT   *r ;

	    /* ドアがいきなり閉まった場合に備えて */
	    r = GM_GetRIntrpt( work->control.addr, work->navigate.next_addr ) ;
	    if ( r )
	    {
		//printf( "Emma : GM_GetRIntrpt stat %x\n", r->status & (ROOT_INTRPT_NONPC|ROOT_INTRPT_CLOSE) ) ;
	
		if ( r->status & (ROOT_INTRPT_NONPC|ROOT_INTRPT_CLOSE) )
//		    if ( work->navigate.next_addr != work->navitrg.addr )
		    {
			work->npc.action.pad = AFRAID_IDLE ;
			return ;
		    }
	    }
	}

	/* 移動方向指定 */
	work->npc.action.dir = work->navigate.next_dir ;
	work->npc.action.pad = AFRAID_WALK ;
#if 0
	/* 向きが変わっていない可能性があるので少し経ってから */
	if ( work->time > 20 )
	{
	    CONTROL *enemy  ;
	    float    radius ;
	    int      dir    ;
	    enemy = EMA_GetNearestEnemy( &work->control.mov,
					 &radius, &dir ) ;
	    dir = EMA_RecalcDir( dir - work->control.rot.vy ) ;
	    if ( radius < 2000.0f*2000.0f && !(dir/512) )
		work->npc.action.pad = AFRAID_IDLE ;
	}
#endif
    }
}

