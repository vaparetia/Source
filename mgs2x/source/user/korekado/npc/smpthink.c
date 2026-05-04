/*
	smpthink.c
	NPCサンプル思考処理

	2001/02/09 Y.Korekado
	$Id: smpthink.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/
#define SMP_DEBUG_THINK (1)
//#define __FATMAN (1)
/*-----  --------------------------------------------------------*/
enum {	// work->think1
	TH1_NORMAL, TH1_FATMAN, TH1_DAMAGE
} ;
enum {	// work->think2
	TH2_PLAYER_TRACE, TH2_ENEMY_ATTACK, TH2_SAMPLE_DAMAGE, TH2_SYSTEM_DAMAGE, TH2_SET_BOMB,
} ;
enum {	// work->think3
	TH3_STAND, TH3_MOVE, TH3_SQUAT, TH3_SLIDING, TH3_SUMMER,
	TH3_WAIT, TH3_ATTACK_GUN, TH3_WAKEUP
} ;
/*----- Think3 --------------------------------------------------------*/
static void Think3_Stand( Work *work )
{
	if ( work->pl_dis > 3000 ) {
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}
#if 1
	if ( work->count3 > COUNT_VMODE( 120 ) ) {
		work->think3 = TH3_SQUAT ;
		work->count3 = 0 ;
		return ;
	}
#endif
	work->count3 ++ ;
}

static void Think3_Move( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromCtrl( npc->nvtrg, GM_PlayerControl ) ;

	/* nvtrgに向ってゾーン移動 */
	if ( GM_Navi( npc->navi, npc->nvtrg, 1000 ) ) {
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	work->count3 ++ ;
}

static void Think3_Squat( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;
	npc->action.pad = PAD_SQUAT ;

	if ( work->count3 == 0 ) {
		NPC_CallHeadMark( npc, HMK2_TYPE_WHT_AT  ) ;
	}

	if ( work->pl_dis > 3000 ) {
#if 0
		work->think2 = TH2_ENEMY_ATTACK ;
#endif
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}


static void Think3_EnemyMove( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	/* nvtrg に 目標のposとzoneaddress をセット */
	GM_SetNaviTargetFromCtrl( npc->nvtrg, GM_PlayerControl ) ;

	/* nvtrgに向ってゾーン移動 */
	if ( GM_NaviNear( npc->navi, npc->nvtrg, 2000 ) ) {
		npc->action.dir = work->pl_dir ;
		work->think3 = TH3_SLIDING ;
		work->count3 = 0 ;
		return ;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	work->count3 ++ ;
}

static void Think3_Sliding( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	npc->action.pad = PAD_SLIDING ;
	
	if ( npc->action.act_end ) {
		work->think3 = TH3_SUMMER ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_Summer( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	if ( work->count3 < COUNT_VMODE( 8 ) ) {
		npc->action.dir = work->pl_dir ;
	} else {
		npc->action.pad = PAD_SUMMER ;
	}

	if ( npc->action.act_end ) {
		work->think2 = TH2_PLAYER_TRACE ;
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_SmpDamageWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;
	if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) {
		work->speed += 0.5 ;
#ifdef __FATMAN
		work->think1 = TH1_FATMAN ;
#else
		work->think1 = TH1_NORMAL ;
#endif
		work->think2 = TH2_PLAYER_TRACE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
	}

	/* 途中でシステムダメージ処理に変化 */
	if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
		work->think2 = TH2_SYSTEM_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
	}
}

static void Think3_SysDamageWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;
	if ( npc->action.status & NPC_ACT_STATUS_FAINT_END ) {
		work->think3 = TH3_WAKEUP ;
		work->count3 = 0 ;
		return ;
	}

	if ( !(npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM) ) {
#ifdef __FATMAN
		work->speed += 0.5 ;
		work->think1 = TH1_FATMAN ;
#else
		work->think1 = TH1_NORMAL ;
#endif
		work->think2 = TH2_PLAYER_TRACE ;
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}
}

static void Think3_WakeUp( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = PAD_WAKEUP ;
	}

	if ( npc->action.act_end ) {
#ifdef __FATMAN
		work->speed += 0.5 ;
		work->think1 = TH1_FATMAN ;
#else
		work->think1 = TH1_NORMAL ;
#endif
		work->think2 = TH2_PLAYER_TRACE ;
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_FatmanStand( Work *work )
{
	if ( work->pl_dis > 4000 ||  work->count3 > COUNT_VMODE( 300 ) ) {
		work->think3 = TH3_ATTACK_GUN ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think3_FatmanMoveRun( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;

	
	if ( work->count3 == 0 ) {
		int	n ;
		HZX_ZONE_ADD trgzone ;
		HZX_GRP *group;

		group = HZX_GetGroupFromNo( HZX_ZoneMapNo( npc->ctrl->addr ) ) ;
		n = group->n_zones ;
		n = KR_RandU( n ) ;
		trgzone = HZX_Address( npc->ctrl->hzx_id, n, n ) ;
printf(" FATMAN GO Zone Addr [%x]\n",trgzone ) ;
		if ( GM_GetRIntrptZ2Z( npc->ctrl->addr, trgzone ) != NULL ) {
			work->think3 = TH3_STAND ;
			work->count3 = 0 ;
			return ;
		}
		GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone ) ;
	}

	npc->action.pad = PAD_FAT_RUN ;

	/* nvtrgに向ってゾーン移動 */
	if ( GM_Navi( npc->navi, npc->nvtrg, 1500 ) ) {
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	work->count3 ++ ;
}

static void Think3_FatmanWait( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	npc->action.pad = PAD_SQUAT ;

	if ( work->count3 > COUNT_VMODE( 300 ) ) {
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_FatmanAttackGun( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	npc->action.pad = PAD_FAT_READY ;

	if ( !(work->count3 % COUNT_VMODE( 30 )) ) {
		npc->action.pad = PAD_FAT_SHOT ;
	}

	if ( work->count3 > COUNT_VMODE( 300 ) ) {
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}

	npc->action.dir = work->pl_dir ;
	work->count3 ++ ;
}

static void Think3_FatmanSetBomb( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( work->bomb_zone != HZX_Zone1(npc->ctrl->addr) ) {
		printf("set bomb go zone[%d]\n",work->bomb_zone);
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think3_FatmanBombMove( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( work->count3 == 0 ) {
		HZX_ZONE_ADD trgzone ;

		trgzone = HZX_Address( npc->ctrl->hzx_id, work->bomb_zone,  work->bomb_zone ) ;
		GM_SetNaviTargetFromZoneAddr( npc->nvtrg, trgzone ) ;
	}

	npc->action.pad = PAD_FAT_RUN ;

	/* nvtrgに向ってゾーン移動 */
	if ( GM_Navi( npc->navi, npc->nvtrg, 1500 ) ) {
		work->think3 = TH3_STAND ;
		work->count3 = 0 ;
		return ;
	}

	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	work->count3 ++ ;
}

/*----- Think2 --------------------------------------------------------*/
static void Think2_PlayerTrace( Work *work )
{
	switch( work->think3 ) {
		case  TH3_STAND :
			Think3_Stand( work ) ;
		break ;
		case  TH3_MOVE :
			Think3_Move( work ) ;
		break ;
		case  TH3_SQUAT :
			Think3_Squat( work ) ;
		break ;
	}
}
static void Think2_EnemyAttack( Work *work )
{
	switch( work->think3 ) {
		case  TH3_MOVE :
			Think3_EnemyMove( work ) ;
		break ;
		case  TH3_SLIDING :
			Think3_Sliding( work ) ;
		break ;
		case  TH3_SUMMER :
			Think3_Summer( work ) ;
		break ;
	}
}

static void Think2_FatmanRun( Work *work )
{
	switch( work->think3 ) {
		case  TH3_STAND :
			Think3_FatmanStand( work ) ;
		break ;
		case  TH3_MOVE :
			Think3_FatmanMoveRun( work ) ;
		break ;
		case  TH3_WAIT :
			Think3_FatmanWait( work ) ;
		break ;
		case  TH3_ATTACK_GUN :
			Think3_FatmanAttackGun( work ) ;
		break ;
	}
	
	if ( work->mode == SMP_MODE_BOMB ) {
		work->think2 = TH2_SET_BOMB ;
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}
}

static void Think2_SetBomb( Work *work )
{
	switch( work->think3 ) {
		case  TH3_STAND :
			Think3_FatmanSetBomb( work ) ;
		break ;
		case  TH3_MOVE :
			Think3_FatmanBombMove( work ) ;
		break ;
	}

	if ( work->mode == SMP_MODE_NORMAL ) {
		work->think2 = TH2_PLAYER_TRACE ;
		work->think3 = TH3_MOVE ;
		work->count3 = 0 ;
		return ;
	}
}

static void Think2_SampleDamage( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_SmpDamageWait( work ) ;
		break ;
		case  TH3_WAKEUP :
			Think3_WakeUp( work ) ;
		break ;
	}
}

static void Think2_SystemDamage( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_SysDamageWait( work ) ;
		break ;
		case  TH3_WAKEUP :
			Think3_WakeUp( work ) ;
		break ;
	}
}

/*----- Think1 --------------------------------------------------------*/
static int DamageCheck( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_SAMPLE_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}
	if ( npc->action.status & NPC_ACT_STATUS_NPCSYS_DAM ) {
		work->think1 = TH1_DAMAGE ;
		work->think2 = TH2_SYSTEM_DAMAGE ;
		work->think3 = TH3_WAIT ;
		work->count3 = 0 ;
		return 1 ;
	}
	return 0 ;
}

static void Think1_Normal( Work *work )
{
	if ( DamageCheck( work ) ) return ;

	switch( work->think2 ) {
		case  TH2_PLAYER_TRACE :
			Think2_PlayerTrace( work ) ;
		break ;
		case  TH2_ENEMY_ATTACK :
			Think2_EnemyAttack( work ) ;
		break ;
	}

}

static void Think1_Fatman( Work *work )
{
	if ( DamageCheck( work ) ) return ;

	switch( work->think2 ) {
		case  TH2_PLAYER_TRACE :
			Think2_FatmanRun( work ) ;
		break ;
		case  TH2_SET_BOMB :
			Think2_SetBomb( work ) ;
		break ;
	}

}

static void Think1_Damage( Work *work )
{
	switch( work->think2 ) {
		case  TH2_SAMPLE_DAMAGE :
			Think2_SampleDamage( work ) ;
		break ;
		case  TH2_SYSTEM_DAMAGE :
			Think2_SystemDamage( work ) ;
		break ;
	}

}

/*----- 思考処理メイン --------------------------------------------------------*/
static void Think( Work *work )
{
	switch( work->think1 ) {
		case  TH1_NORMAL :
			Think1_Normal( work ) ;
		break ;

		case  TH1_FATMAN :
			Think1_Fatman( work ) ;
		break ;

		case  TH1_DAMAGE :
			Think1_Damage( work ) ;
		break ;
	}
#ifdef SMP_DEBUG_THINK
DEBUG_Locate( 40, 30, 0 );
DEBUG_Printf( "TH1=%2d\n", work->think1 );
DEBUG_Printf( "TH2=%2d\n", work->think2 );
DEBUG_Printf( "TH3=%2d\n", work->think3 );
DEBUG_Printf( "CT3=%2d\n", work->count3 );
DEBUG_Printf( "FAINT%2d\n", work->npc.action.faint );
DEBUG_Printf( "LIFE%2d\n", work->npc.action.life );
#endif
}
/*----- --------------------------------------------------------*/
static void StartThink( Work *work )
{
	NPC_SetCheckPad( &work->npc, SampleCheckPad ) ;
	NPC_SetCheckDamage( &work->npc, SmpCheckDamage ) ;

#ifdef __FATMAN
	work->think1 = TH1_FATMAN ;
#else
	work->think1 = TH1_NORMAL ;
#endif

	work->think2 = TH2_PLAYER_TRACE ;
	work->think3 = TH3_STAND ;
	work->count3 = 0 ;
}
