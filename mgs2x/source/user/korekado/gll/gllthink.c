/*
	gllthink.c
	ゴルルゴン思考処理

	2002/04/04 Y.Korekado
	$Id: gllthink.c,v 1.1.1.3 2002/11/19 11:44:13 Yoshizawa1 Exp $
*/

/*-----  --------------------------------------------------------*/
extern void ENE_SetNextnode( ROUTENAVI *rnavi ) ;
extern void GLL_DetectVibration( void ) ;
extern int	ENE_InRange( FVECTOR		*pos, FVECTOR		*trg, int range );
extern void *NewGllPikari(FMATRIX *world);

/*-----  --------------------------------------------------------*/
enum {	// work->think1
	TH1_NORMAL,TH1_DETECT, TH1_FIGHT_GME, TH1_FIGHT_GNO, TH1_FIGHT_GLL, TH1_DAMAGE
} ;
enum {	// work->think2
	TH2_STAND,TH2_ATTACK,TH2_HIT,TH2_HEAD, TH2_ACHOO,
} ;
enum {	// work->think3
	TH3_WAIT,TH3_GO_NEXT,TH3_POINT_ACTION, TH3_HOWL, TH3_MOVE_END,
	TH3_FIRE, TH3_TRACE, TH3_SEARCH, TH3_PUNCH, TH3_SONIC,
	TH3_COMBO,TH3_READY, TH3_BDMH, TH3_REGENE, TH3_FIND,
	TH3_SP_GOVER,TH3_SP_CLEAR, TH3_CHAFF, TH3_CHAFFBEEM, TH3_MOVE_END_WAIT,
	TH3_NO_SEARCH, TH3_NO_SEARCH_BEEM
} ;
/*-----  --------------------------------------------------------*/
enum {
	SP_NONE,
	SP_HOWL,	/* 雄叫び */
	SP_DETECT,	/* 発見 */
	SP_LOOK_L_12,	//覗き込み、左、高さ１２ｍ
	SP_LOOK_R_12,	//覗き込み、右、高さ１２ｍ
	SP_LOOK_L_7,	//覗き込み、左、高さ７ｍ
	SP_LOOK_R_7,	//覗き込み、右、高さ７ｍ
	SP_MOVE_END,	//移動終了
	SP_FIRE,	//ビーム攻撃
	SP_PUNCH_R,	//右パンチ攻撃
	SP_PUNCH_L,	//左パンチ攻撃
	SP_SONIC,	//超音波攻撃
	SP_REGENE,
	SP_BDMH,
	SP_BOOK,
	SP_SNEEZE,
	SP_LOOK_F_12,	//覗き込み、正面、高さ１２ｍ
	SP_LOOK_F_16,	//覗き込み、正面、高さ１６ｍ
	SP_LOOK_F_7,	//覗き込み、正面、高さ７ｍ
	SP_MYFRIEND,
	SP_CHAFFDAM,
	SP_CHAFFBEEM,
	SP_ACHOO,
	SP_FIRE_F,
} ;

static int	PointAction[]={
	SP_NONE,
	SP_HOWL,	/* 雄叫び */
	SP_LOOK_L_12,	//覗き込み、左、高さ１２ｍ
	SP_LOOK_R_12,	//覗き込み、右、高さ１２ｍ
	SP_LOOK_L_7,	//覗き込み、左、高さ７ｍ
	SP_LOOK_R_7,	//覗き込み、右、高さ７ｍ
	SP_LOOK_F_12,	//覗き込み、正面、高さ１２ｍ
	SP_LOOK_F_16,	//覗き込み、正面、高さ１６ｍ
	SP_LOOK_F_7,	//覗き込み、正面、高さ７ｍ
} ;

#include	"gllact.c"
#include	"gllfight.c"
#include	"gmefight.c"
#include	"gnofight.c"

/*-------------------------------------------------------------------*/
static int RouteCheck( Work *work )
{
	ROUTENAVI	*rnavi ;
	NPCWORK	*npc ;

	rnavi = work->npc.rnavi ;
	npc = &work->npc ;
	if ( rnavi->next_route == rnavi->c_route ) return 0 ;

	printf("GLL [%d] Change Root [%d]->[%d]\n",work->id, rnavi->c_route, rnavi->next_route ) ;
	NPC_InitRouteNavi ( npc, rnavi, rnavi->next_route ) ;

	rnavi->next_node = ( rnavi->chang_node < 0 ) ? 0 : rnavi->chang_node ;
	GM_SetNaviTargetFromPos( npc->nvtrg, &npc->rnavi->nodes[(int)npc->rnavi->next_node], npc->ctrl->hzx_id  ) ;

printf("trg[%f][%f][%f] \n",npc->nvtrg->pos.vx,npc->nvtrg->pos.vy,npc->nvtrg->pos.vz );

	work->think1 = TH1_NORMAL ;
	work->think2 = TH2_STAND ;
	work->think3 = TH3_GO_NEXT ; 
	work->count3 = 0 ;

	return  1 ;
}

static int TimeManage( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( npc->rnavi->p_acttime >= 0 ) {
		if ( work->count3 >= npc->rnavi->p_acttime ) return 1 ;
	} else if ( npc->rnavi->p_acttime == PTIME_MOTIONEND ) {
		if ( npc->action.act_end == 1 ) return 1 ;
	} else if ( npc->rnavi->p_acttime == PTIME_FLAGWAIT ) {
		if ( npc->rnavi->next_route != npc->rnavi->c_route ) return 1 ;
	}

	return 0 ;
}

/*----- Think3 --------------------------------------------------------*/
static void Think3_StandWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 > 0 ) {
		if ( npc->rnavi->n_nodes > 1 ) {
//			ENE_SetNextnode( npc->rnavi ) ;
			npc->rnavi->next_node = 0 ;
			GM_SetNaviTargetFromPos( npc->nvtrg, &npc->rnavi->nodes[(int)npc->rnavi->next_node], npc->ctrl->hzx_id  ) ;
			work->think3 = TH3_GO_NEXT ; 
		} else {
			npc->action.dir =  npc->rnavi->pa_dir[(int)npc->rnavi->next_node] ;
			npc->rnavi->p_acttime = npc->rnavi->pa_time[(int)npc->rnavi->next_node] ;
			if ( npc->rnavi->p_acttime > 0 ) {
				npc->rnavi->p_acttime = COUNT_VMODE(npc->rnavi->p_acttime) ;
			}
			work->think3 = TH3_POINT_ACTION ; 
		}
		work->count3 = 0 ;
		return ;
	}
	work->count3 ++ ;
}

static void Think3_GoNext( Work *work )
{
	ROUTENAVI	*rnavi ;
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( GM_InZoneNavi( npc->navi, npc->nvtrg, 4000 ) ) {/*高さは無視*/

		if ( RouteCheck( work ) ) return ;

		rnavi = npc->rnavi ;
		rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node] ;
		rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
		if ( npc->rnavi->p_acttime > 0 ) {
			rnavi->p_acttime = COUNT_VMODE(rnavi->p_acttime) ;
		}
		rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node] ;
		/* 移動方向指定 */
		npc->action.dir = npc->navi->next_dir ;

		if ( rnavi->p_acttime == 0 ) {
			ENE_SetNextnode( rnavi ) ;
			GM_SetNaviTargetFromPos( npc->nvtrg, &rnavi->nodes[(int)rnavi->next_node], npc->ctrl->hzx_id  ) ;
		} else {
//rnavi->p_acttime = -1 ;
//rnavi->p_action = SP_LOOK_L_7 ;
//rnavi->p_action = SP_HOWL ;
			work->think3 = TH3_MOVE_END ;
			work->count3 = 0 ;
		}
		return ;
	}

 	/* 移動方向指定 */
	npc->action.dir = npc->navi->next_dir ;

	work->count3 ++ ;
}

static void Think3_MoveEnd( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( npc->action.act_end == 1 ) {
		work->think3 = TH3_MOVE_END_WAIT ;
		work->count3 = 0 ;
		return ;
	}

	npc->action.pad = SP_MOVE_END ;
	npc->action.dir = npc->rnavi->p_dir ;

	work->count3 ++ ;
}

static void Think3_MoveEndWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 > COUNT_VMODE(60) ) {
		work->think3 = TH3_POINT_ACTION ;
		work->count3 = 0 ;
		return ;
	}

	work->count3 ++ ;
}

static void Think3_PointAction( Work *work )
{
	ROUTENAVI	*rnavi ;
	NPCWORK	*npc ;

	npc = &work->npc ;
	rnavi = npc->rnavi ;

	if ( TimeManage( work ) ) {
		FVECTOR	bef_pos ;
		
		bef_pos = rnavi->nodes[(int)rnavi->next_node] ;

		if ( RouteCheck( work ) ) {
			if ( !(ENE_InRange( &bef_pos, &rnavi->nodes[(int)rnavi->next_node], 450 )) ||
				 ( npc->ctrl->turn.vy != rnavi->pa_dir[(int)rnavi->next_node] )  ) {
			} else {
				rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node] ;
				rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
				if ( npc->rnavi->p_acttime > 0 ) {
					rnavi->p_acttime = COUNT_VMODE(rnavi->p_acttime) ;
				}
				rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node] ;
				work->think3 = TH3_POINT_ACTION ;
			}
			work->count3 = 0 ;
			return ;
		}

		if ( rnavi->n_nodes > 1 ) {
			ENE_SetNextnode( rnavi ) ;
#if 0
{
	FVECTOR *p1, *p2 ;
	
	p1 = &bef_pos ;
	p2 = &rnavi->nodes[(int)rnavi->next_node] ;
printf("mov[%f][%f][%f] nodes[%f][%f][%f]\n",p1->vx,p1->vy,p1->vz,p2->vx,p2->vy,p2->vz );
printf("turn.vy [%d] pa_dir[%d]\n",npc->ctrl->turn.vy,rnavi->pa_dir[(int)rnavi->next_node] );
}
#endif
			if ( !(ENE_InRange( &bef_pos, &rnavi->nodes[(int)rnavi->next_node], 450 )) ||
				 ( npc->ctrl->turn.vy != rnavi->pa_dir[(int)rnavi->next_node] )  ) {
				GM_SetNaviTargetFromPos( npc->nvtrg, &rnavi->nodes[(int)rnavi->next_node], npc->ctrl->hzx_id  ) ;
				work->think3 = TH3_GO_NEXT ; 
			} else {
				rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node] ;
				rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
				if ( npc->rnavi->p_acttime > 0 ) {
					rnavi->p_acttime = COUNT_VMODE(rnavi->p_acttime) ;
				}
				rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node] ;
			}
		}
		work->count3 = 0 ;
		return ;
	}

	if ( rnavi->p_action ) {
		npc->action.pad = PointAction[rnavi->p_action] ;
	}

	work->count3 ++ ;
}

static void Think3_Detect( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( work->count3 == 0 ) {
		npc->action.pad = SP_DETECT ;
		AN_HeadMark( &BODYWORLD( npc->body, HUMAN21_ATAMA ), RED_AT_GENORA );
		NewGllPikari( &BODYWORLD( npc->body, HUMAN21_ATAMA ) ) ;
		GM_SeSetMode( SD_E_BIKKRI01 , &npc->ctrl->mov, GM_SEMODE_BOMB ) ;
		GLLCOM_Detect( npc->ctrl ) ;
		GLL_DetectVibration( ) ;
	}

	if ( npc->action.act_end == 1 ) {
		work->think3 = TH3_HOWL ; 
		work->count3 = 0 ;
		return ;
	}

	npc->action.dir = work->pl_dir ;

	work->count3 ++ ;
}

static void Think3_Howl( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( npc->action.act_end == 1 ) {
		ENE_SetNextnode( npc->rnavi ) ;
		GM_SetNaviTargetFromPos( npc->nvtrg, &npc->rnavi->nodes[(int)npc->rnavi->next_node], npc->ctrl->hzx_id  ) ;
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_GO_NEXT ; 
		return ;
	}
	npc->action.pad = SP_HOWL ;

	work->count3 ++ ;
}

static void Think3_DamageStandWait( Work *work )
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	if ( StatusFightGME ( work->status ) ) {
		SET_FLAG ( work->status2, GLL_STATUS2_NO_HEAD ) ;
		SET_FLAG ( work->status2, GLL_STATUS2_INVALID_CHAFF ) ;
	}

	if ( npc->action.act_end == 1 ) {
		if ( StatusFightGME ( work->status ) ) {
			work->think1 = TH1_FIGHT_GME ;
			work->think2 = TH2_HEAD ;
			work->think3 = TH3_WAIT ;
		} else if ( StatusFightGLL ( work->status ) ) {
			work->think1 = TH1_FIGHT_GLL ;
			work->think2 = TH2_ATTACK ;
			work->think3 = TH3_TRACE ;
		} else {
			work->think1 = TH1_NORMAL ;
			work->think2 = TH2_STAND ;
			work->think3 = TH3_WAIT ;
		}
		work->count3 = 0 ;
		return ;
	}


	work->count3 ++ ;
}

/*----- Think2 --------------------------------------------------------*/
static void Think2_Stnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_StandWait( work ) ;
		break ;
		case  TH3_GO_NEXT :
			Think3_GoNext( work ) ;
		break ;
		case  TH3_POINT_ACTION :
			Think3_PointAction( work ) ;
		break ;
		case  TH3_MOVE_END :
			Think3_MoveEnd( work ) ;
		break ;
		case  TH3_MOVE_END_WAIT :
			Think3_MoveEndWait( work ) ;
		break ;
	}
}

static void Think2_DetectStnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_POINT_ACTION :
			Think3_Detect( work ) ;
		break ;
		case  TH3_HOWL :
			Think3_Howl( work ) ;
		break ;
	}
}

static void Think2_DamageStnad( Work *work )
{
	switch( work->think3 ) {
		case  TH3_WAIT :
			Think3_DamageStandWait( work ) ;
		break ;
	}
}
/*----- Think1 --------------------------------------------------------*/
static int DamageCheck( Work *work )
{
	NPCWORK	*npc ;
	
	npc = &work->npc ;
	if ( npc->action.status & NPC_ACT_STATUS_DAMAGE ) {
	}
	return 0 ;
}

static void NormalNoticeCheck( Work *work )
{
	if ( work->sight == EYE_INFO_SIGHT_IN ) {
		work->think1 = TH1_DETECT ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_POINT_ACTION ;
		work->count3 = 0 ;
	}
}

static void Think1_Normal( Work *work )
{
	if ( DamageCheck( work ) ) return ;

	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_Stnad( work ) ;
		break ;
	}

	NormalNoticeCheck( work ) ;
#if 0
{
	CONTROL *ctrl ;
	
	ctrl = &work->control ;
printf("turn.vy[%d] rot.vy[%d] mov[%f][%f][%f]\n",ctrl->turn.vy,ctrl->rot.vy, 
ctrl->mov.vx,ctrl->mov.vy,ctrl->mov.vz ) ;
}
#endif
}

static void Think1_Detect( Work *work )
{
	if ( DamageCheck( work ) ) return ;

	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_DetectStnad( work ) ;
		break ;
	}
}

static void Think1_Damage( Work *work )
{
	switch( work->think2 ) {
		case  TH2_STAND :
			Think2_DamageStnad( work ) ;
		break ;
	}
}

/*----- 思考処理メイン --------------------------------------------------------*/
static void Think( Work *work )
{
	work->status2 = 0 ;
	
	switch( work->think1 ) {
		case  TH1_NORMAL :
			Think1_Normal( work ) ;
		break ;
		case  TH1_DETECT :
			Think1_Detect( work ) ;
		break ;
		case  TH1_DAMAGE :
			Think1_Damage( work ) ;
		break ;

		case  TH1_FIGHT_GME :		// gmefight.c
			Think1_Fight_GME( work ) ;
		break ;
		case  TH1_FIGHT_GNO :	// gnofight.c
			Think1_Fight_GNO( work ) ;
		break ;
		case  TH1_FIGHT_GLL :	// gllfight.c
			Think1_Fight_GLL( work ) ;
		break ;
	}

#ifdef DEBUG_MODE
if ( debug_flag&GLL_DEBUG_THINK ) {
//	if ( work->status & GLL_STATUS_GLL ) {
		DEBUG_Locate( 40+(70*work->id), 130, 0 );
		DEBUG_Printf( "TH1=%2d\n", work->think1 );
		DEBUG_Printf( "TH2=%2d\n", work->think2 );
		DEBUG_Printf( "TH3=%2d\n", work->think3 );
		DEBUG_Printf( "CT3=%2d\n", work->count3 );
		if ( !(work->status & GLL_STATUS_FIGHT) ) {
			DEBUG_Printf( "ROT%2d\n", work->npc.rnavi->c_route );
			DEBUG_Printf( "NOD%2d\n", work->npc.rnavi->next_node );
			DEBUG_Printf( "ACT%2d\n", work->npc.rnavi->p_action );
			DEBUG_Printf( "TIM%2d\n", work->npc.rnavi->p_acttime );
		}
		DEBUG_Printf( "DIR%2d\n", work->npc.ctrl->rot.vy );
		if ( StatusFightGNO( work->status ) ) {
			DEBUG_Printf( "ACHOOO %2d\n", GLL_ACHOO_TIME );
		}
//	}
}
#endif
}
/*----- --------------------------------------------------------*/
static void StartThink( Work *work )
{
	NPC_SetCheckPad( &work->npc, GllCheckPad ) ;
	NPC_SetCheckDamage( &work->npc, GllCheckDamage ) ;
	NPC_SetActMode( &work->npc, ActStandStill ) ;

	work->tmp1 = 0 ;
	work->tmp2 = 0 ;
	if ( StatusFightGME( work->status ) ) {
		work->think1 = TH1_FIGHT_GME ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
	} else if ( StatusFightGNO( work->status ) ) {
		work->think1 = TH1_FIGHT_GNO ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
	} else if ( StatusFightGLL( work->status ) ) {
		work->think1 = TH1_FIGHT_GLL ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
	} else {
		work->think1 = TH1_NORMAL ;
		work->think2 = TH2_STAND ;
		work->think3 = TH3_WAIT ;
	}
	work->count3 = 0 ;
}
