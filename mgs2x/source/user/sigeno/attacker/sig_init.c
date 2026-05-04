/*---------------------------------------------------------------*/
static	void	DefTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work	*work ;
    work = ( Work * )ptr ;

//	printf("HiT !! [%d] \n",work->enethink.id ) ;
	ENE_DefTargCallBack( off, def, &work->enethink ) ;

}
static int SIG_ENE_GetResources( Work *work, int name, int type ,int *mt_array)
{

	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body, *weapon, *sub_weapon  ;
	int		root, node, bodyname, wpname ,sub_wpname ,life, faint, value,equip;
//	FVECTOR	d_pos;
	char	*d_pos_opt, *opt ;
	AT_THK *at_thk ;

	equip = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );


	entk = &work->enethink ;
	COM_SetIDNumber( entk ) ;

	/*entkの中身を可視*/

	(AT_THK *) entk->character = &work->at_thk ;
	at_thk = &work->at_thk ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;
	weapon = &work->weapon ;
	sub_weapon = &work->sub_weapon ;

	entk->w = act->w = ( void * )work ;	/* 親子関係用にワークのポインタをセット */
	if ( (entk->status = GCL_GetOptionValue( 's', -1 )) < 0  ) {
		entk->status = 0 ;
	}
	if ( (root = GCL_GetOptionValue( 'r', -1 )) < 0  ) {
		return -1 ;
	}
	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		root += GM_RouteOffset ; 
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

	/*視力設定*/
//	eye_s = GCL_GetOptionValue( 'i', DEF_AT_EYE_SIGHT );
	entk->sense.eye_s_s[ALERT_MODE_SNEAK] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_ALERT] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_AVOID] = DEF_AT_EYE_SIGHT ;
	entk->sense.eye_s_s[ALERT_MODE_SEARCH] = -1 ;
	if ( ( opt = GCL_GetOption( 'i' ) ) != NULL ){
		ENE_GclGetInt( opt, &entk->sense.eye_s_s[0] ) ;
	}
	if ( entk->sense.eye_s_s[ALERT_MODE_SEARCH] < 0 ) {
		entk->sense.eye_s_s[ALERT_MODE_SEARCH] = entk->sense.eye_s_s[ALERT_MODE_SNEAK] ;
	}

	act->bodyp.type = ENE_TYPE_ATTACKER;
	if(equip == AT_EQUIP_HITECH_1){
		type = ENE_TYPE_HITECH ;
printf("AT_EQUIP_HITECH_1\n");
//		act->bodyp.type |= ENE_TYPE_HITECH;
	}

//	bodyname = GV_StrCode( "tng_def" ) ;

	switch (type){
		case ENE_TYPE_ATTACKER :
			bodyname = ENE_MDL_NAME_GBA ;
			break ;
		case ENE_TYPE_EVENT_A :
			bodyname = ENE_MDL_NAME_GBS ;
			break ;
		case 	ENE_TYPE_TNG_A :
			bodyname = ENE_MDL_NAME_TNG ;
			break ;
		case	ENE_TYPE_HITECH :
			bodyname = ENE_MDL_NAME_HTC ;
printf("bodyname = ENE_MDL_NAME_HTC\n");
			break ;
		default :
			bodyname = ENE_MDL_NAME_GBA ;
			break ;
	}
	/*装備品*/
	/*こっちは兵士の種類を決定*/
//	at_thk->equip_req = GCL_GetOptionValue( 'e', AT_EQUIP_NORMAL );
	at_thk->equip_req = equip ;

	/*ショットガン装備兵*/
	if(at_thk->equip_req == AT_EQUIP_SHOTGUN){
		wpname = E_WP_SPS ;
		act->bodyp.type |= ENE_TYPE_SHOTGUN ;
	}else if(at_thk->equip_req == AT_EQUIP_ABAKAN){
		wpname = E_WP_ABAKAN ;
		act->bodyp.type |= ENE_TYPE_ABAKAN ;
	}else if(type & ENE_TYPE_TNG_A){
		wpname = E_WP_P90 ;
	}else{
printf("wpname = E_WP_AKS\n");
		wpname = E_WP_AKS ;
	}

	sub_wpname = E_WP_MKR ;

printf("ATTACKER COMSET Id NUMBER!!!!!!!!!!!!!!\n") ;

	ENE_SetSenseParam( &(entk->sense), 0, DEF_EYE_RANGE,
	entk->sense.eye_s_s[0], DEF_HEARING, DEF_SMELL ) ;
/*ここを変えれば目標変更可能*/
	ENE_SetEyeInfo( &(entk->pl_eyei), &GM_PlayerFindPos, 
	&GM_PlayerAddress, NULL, &GM_PlayerMap ) ;
	ENE_InitControl( entk,ctrl, name ) ;

	/* モーション名セット */
	ENE_SetMotionName( entk, ENE_MOT_BASE, ENE_MOT_STAGE ) ;

	switch (type){
		case ENE_TYPE_ATTACKER :
			ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
			break ;
		case ENE_TYPE_HITECH :
			ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
			break ;
		case ENE_TYPE_EVENT_A :
			ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
			break ;
		case ENE_TYPE_TNG_A :
//			SET_FLAG(entk->status, ENE_STATUS_NO_MOUTH ) ;
			SET_FLAG(entk->status, ENE_STATUS_NO_FINGER) ;
			SET_FLAG(entk->status, ENE_STATUS_NO_EYEANIM);
			SET_FLAG(entk->status, ENE_STATUS_NO_VANIM)  ;
			ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
//			ENE_InitObject( entk, body, bodyname, ctrl, work->lights ) ;
//			ENE_InitObject_Tng( entk, body, bodyname, ctrl, work->lights,GV_StrCode("teng_a") ) ;
		break ;
		default :
			bodyname = ENE_MDL_NAME_GBS ;
			break ;
	}



	/* 防御ターゲットの初期化(装備より先にする)*/
	ENE_InitTarget( entk, ctrl, body, act ) ;
    GM_SetTargetCallBack( &act->bodyp.deftrg, DefTargCallBack, work ) ;
	
	ENE_InitWeapon( entk, body, weapon, wpname, NULL ) ;
	ENE_InitSubWeapon( entk, body, sub_weapon, sub_wpname ) ;

//	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,mt_array) ;
	ENE_InitAction( entk, act, ctrl, body, life, faint, DEF_BLOOD ,NULL) ;

	/* 耐久力セット */
	AT_SetDurable( &act->bodyp, 0, 5, 3, 0 ) ;

//	at_thk->life_buf = entk->act->bodyp.life ;
	at_thk->life_buf = life ;

	if ( (value = GCL_GetOptionValue( 'v', -1 )) < 0  ) {
		entk->voice_chara = entk->id%4 ;
	} else {
		entk->voice_chara = value ;
	}

	if ( ENE_InitThink( entk, ctrl, &work->routenavi, 
		&work->zonenavi, act, root, node ) < 0 ) {
		return -1 ;
	}


//	ENE_InitRoute( &work->cl_route ) ;
	entk->rnavi2 = &work->cl_route ;


	if( GM_AlertMode == ALERT_MODE_SEARCH ) {
		ENE_AttackerStartModeSearchWarp( &work->enethink ) ;
	} else {
		ENE_AttackerStartModeSneak( &work->enethink ) ;
	}

	work->action.CheckDamage = ENE_EnemyDamagePad ;

	at_thk->at_status = 0 ;
	/* 守備位置セット */
	d_pos_opt = GCL_GetOption( 'd' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &entk->def_pos ) ;
		entk->def_mapbit = 
			GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt( ) ) );
		at_thk->def_addr = 
//		HZX_GetAddress( GV_GetBit( int no ), &entk->def_pos, -1 );
	HZX_GetAddress( entk->def_mapbit, &entk->def_pos, -1 );

		at_thk->at_status |= AT_ST_DEFENSE ;

printf("Scen Set def_map = %x \n",entk->def_mapbit );
	} else {
		entk->def_pos = ctrl->mov ;
		entk->def_mapbit = 0 ;
	}
	/*守備位置からの待避位置*/
	d_pos_opt = GCL_GetOption( 'o' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &at_thk->def_pos2 ) ;
		at_thk->at_status |= AT_ST_DODGE ;
	}
	entk->search_route = GCL_GetOptionValue( 'c', root ) ;


	/*盾壊れた時連絡*/
	/*装備系 設定初期化*/
	entk->sw.shield = 0;
	act->sw->sub_weapon = 0;

	if((at_thk->equip_req == AT_EQUIP_SHIELD)
	||(at_thk->equip_req == AT_EQUIP_LIGHT_SHIELD)
	){
#if 1
	GV_SetActorChild( entk->w, NewAT_Shield( &work->body ,
	entk ,&entk->act->sub_obj) );
#else
	NewAT_Shield( &work->body ,entk ,&entk->act->sub_obj);
#endif
		if(entk->sw.shield==1) {
			AT_SetType( entk->act, ENE_TYPE_SHIELD );
			entk->act->sw->sub_weapon = 1;
			/*盾を握る*/
			entk->sw.finger = SW_FLAG_SWITCH5 ;
		}
	}
	/*装備系設定後 装弾数セット*/
	if(entk->act->sw->sub_weapon == 1){
		/*さぶを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.sub_weapon) ;
	}else {
		/*めいんを使用*/
		entk->max_bullet = ENE_GetAmmoMax(entk->name_id.weapon) ;
	}


	at_thk->found_addr = HZX_NO_ZONE ;


#ifdef DEBUG_PRIM
	InitPrim( work ,0,0) ;
#endif
	return (0);
}
