/*
	wcgetres.c
	見張り兵初期化

	2001/06/21 Y.Korekado
	$Id: wcgetres.c,v 1.1.1.3 2002/11/19 11:44:31 Yoshizawa1 Exp $
	
*/

/* ターゲットコールバック関数 */
static	void	DefTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;

	ENE_DefTargCallBack( off, def, &work->enethink ) ;
}

static int WatcherGetResources( Work *work, int name )
{
	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body, *weapon, *sub_weapon ;
	int		root, node, bodyname, wpname, sub_wpname, faint, life, hearing, value ;
	char	*opt ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
	sub_weapon = &work->sub_weapon ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */

/* シナリオリード */
	if ( (entk->status = GCL_GetOptionValue( 's', -1 )) < 0  ) {
		entk->status = 0 ;
	}
	if ( (entk->status_status = GCL_GetOptionValue( 't', -1 )) < 0  ) {
		entk->status_status = 0 ;
	}
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		entk->death_proc.argc = ENE_GclGetProc( &(entk->death_proc.proc), &entk->death_proc.argv[0] ) ;
		if ( entk->death_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		entk->death_proc.proc = 0 ;
	}

	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = DEF_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = -1 ;
	if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){
		ENE_GclGetInt( opt, &entk->sense.eye_s_s[0] ) ;
	}
	if ( entk->sense.eye_s_s[ALERT_MODE_SEARCH] < 0 ) {
		entk->sense.eye_s_s[ALERT_MODE_SEARCH] = entk->sense.eye_s_s[ALERT_MODE_SNEAK] ;
	}

	if ( (root = GCL_GetOptionValue( 'r', -1 )) < 0  ) {
		return -1 ;
	}
	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		root += COM_GetRootOfset( ) ;
	}

	if ( (node = GCL_GetOptionValue( 'n', -1 )) < 0  ) {
		node = 0 ;
	}
	if ( (life = GCL_GetOptionValue( 'l', -1 )) < 0  ) {
		life = DEF_LIFE ;
	}
	if ( (faint = GCL_GetOptionValue( 'f', -1 )) < 0  ) {
		faint = DEF_FAINT ;
	}
	if ( (hearing  = GCL_GetOptionValue( 'h', -1 )) < 0  ) {
		hearing = DEF_HEARING ;
	}

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		entk->waiting_pos_num = GCL_GetOptionValue( 'm', -1 ) ;
	} else {
		entk->waiting_pos_num = -1 ;
	}

	if ( COM_StageKind() & ENE_STAGE_GPS ) {
		SET_FLAG( entk->status, ENE_STATUS_URBAN ) ;
	}


	if ( entk->status_status & ENE_STST_VRBODY ) {
		bodyname = ENE_MDL_NAME_VR ;
		AT_SetType( act, ENE_TYPE_VR ) ;
	} else if ( entk->status & ENE_STATUS_URBAN ) {
		bodyname = ENE_MDL_NAME_GPS ;
	} else {
		bodyname = ENE_MDL_NAME_GBS ;
	}
//bodyname = GV_StrCode( "gll_def" ) ;

	if ( entk->status & ENE_STATUS_URBAN ) {
		wpname = E_WP_ABAKAN ;
	} else {
		wpname = E_WP_AKS_SP ;
	}
//wpname = GV_StrCode( "gps_def_mt" ) ;

	sub_wpname = E_WP_MKR ;
/* 敵兵標準初期化 */

	/***コマンダへの登録****/
	COM_SetIDNumber( entk ) ;

	/* パラメータセット */
	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE, entk->sense.eye_s_s[0], hearing, DEF_SMELL ) ;
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, &GM_PlayerAddress, NULL, &GM_PlayerHzxID ) ;

	/* NPCセット 
		FVECTOR *EMA_CommandGetPosition() ;エマの現在の位置を得る
		OBJECT *EMA_CommandGetBody() ;OBJECTを得る
		CONTROL *EMA_CommandGetControl() ;コントロールを得る
	*/
	if ( ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA ) {
		extern CONTROL *EMA_CommandGetControl() ;
		CONTROL	*ema_ctrl ;
		
		ema_ctrl = EMA_CommandGetControl() ;
		ENE_SetEyeInfo( &(entk->npc_eyei), &ema_ctrl->mov, 
				&ema_ctrl->addr, NULL, &ema_ctrl->hzx_id ) ;
	}

	/* コントロールの初期化 */
	ENE_InitControl( entk, ctrl, name ) ;

	/* モーション名セット */
	ENE_SetMotionName( entk, ENE_MOT_BASE, ENE_MOT_STAGE ) ;

	/* オブジェクトの初期化 */
	ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;

	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;

	/* 装備関係初期化 */
	ENE_InitWeapon( entk, body, weapon, wpname, &act->bodyp.deftrg ) ;
//	ENE_InitSubWeapon( entk, body, sub_weapon, sub_wpname ) ;
	ENE_InitSubWeapon( entk, body, NULL, 0 ) ;

	/* アクション初期化 */
	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;

	/* 耐久力セット */
//	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;
	AT_SetDurable( &act->bodyp, 0, 0, 0, 0 ) ;

	if ( (value = GCL_GetOptionValue( 'v', -1 )) < 0  ) {
		entk->voice_chara = entk->id%4 ;
	} else {
		entk->voice_chara = value ;
	}

	if ( entk->status & ENE_STATUS_URBAN ) {
		AT_SetType( act, (ENE_TYPE_URBAN|ENE_TYPE_ABAKAN) ) ;
	}

	/* 思考パラメータ初期化 */
	if ( ENE_InitThink( entk, ctrl, &work->routenavi, &work->zonenavi, act, root, node ) < 0 ) {
		return -1 ;
	}

	/* 双眼鏡装着時の視力セット */
	if ( (value = GCL_GetOptionValue( 'z', -1 )) >= 0  ) {
		entk->sense.glasses_dis = value ;
	}

	return 0 ;
}
