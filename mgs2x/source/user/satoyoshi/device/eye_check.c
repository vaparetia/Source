/********************************************************************************/
/*	Sv_camera.c								*/
/*	監視カメラ  *NewSvCamera						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: eye_check.c,v 1.1.1.3 2002/11/19 11:48:16 Yoshizawa1 Exp $		*/
/********************************************************************************/
/********************************************************************************/
/*	include files								*/
/********************************************************************************/
/********************************************************************************/
/*	extern									*/
/********************************************************************************/
extern int CP_GetCorpNum( void ) ;
extern NEWCORP *CP_GetCorpFromEye( EYEPARAM * ,FVECTOR * ) ;
extern NEWCORP *CP_GetCorpN_start( void );
extern FVECTOR *EMA_CommandGetPosition();
/********************************************************************************/
/*	define      								*/
/********************************************************************************/

static float CheckDistanceLineAndPoint( FVECTOR *lp, FVECTOR *ld, FVECTOR *p )
{
    FVECTOR v, k, l ;

    _sceVu0SubVector( &v, p, lp ) ;
    _sceVu0OuterProduct( &k, &v, ld ) ;
    _sceVu0OuterProduct( &l, &k, ld ) ;
    _sceVu0Normalize( &l, &l ) ;

    return -_sceVu0InnerProduct( &l, &v ) ;
}



/**************************<-------local function------>*************************/
/*	名前:	int Eye_Sight_Check						*/
/*	返値:	int								*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	四角錘内 座標チェック						*/
/********************************************************************************/
static int Eye_Sight_Check( EYEPARAM *eye , FVECTOR* trgpos)
{
	FVECTOR vec ;
	float dis ;
	int dir ,diff;

	_sceVu0SubVector( &vec, trgpos , eye->eyepos ) ;
	dis = GV_VecLen3F( &vec ) ;

	if ( dis > eye->length ){
	    //	    printf ("Out range\n");
	    return 0 ;
	}
	dir = GV_VecDir2( &vec ) ;
	if ( GV_DiffDirAbs( eye->rot.vy, dir ) > eye->range.vy ){
	    //	    printf ("Out of dir y %d %d %d\n",eye->rot.vy, dir,eye->range.vy);
	    return 0 ;
	}
	dir = GV_VecDir2X( &vec )  ;
	diff = GV_DiffDirS( eye->rot.vx, dir ) ;
	if ( diff > eye->range.vx || diff < - eye->range.vx ){
	    //	    printf ("Out of dir x %d %d %d\n",eye->rot.vx, dir,eye->range.vx);
	    return 0 ;
	}
	return 1 ;
}


static int EyeOnlineCheck( hzx_id, from, to )
int hzx_id ;
FVECTOR *from ;
FVECTOR *to ;
{
	return HZX_OnlineHazardCheck( hzx_id, from, to,
				HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, HZX_FLOOR_NO_ENEMY_EYES ) ;
}


#define CB_BOX_FAIL	(PLAYER_MOVE|PLAYER_CB_BOX_STAND)
/**************************<-------local function------>*************************/
/*	名前:	void Eye_Check_PC						*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	視界チェック プレイヤー関連					*/
/********************************************************************************/
static void Eye_Check_PC(EYEPARAM *eye){
    COMMANDER	*commander;
    HZX_GROUP_ID hzx_id = eye->check_hzx_id;

    commander = COM_GetCommander();
    eye->PC_CK_FLG = EYECK_PC_NOT_IN_SIGHT;

    if(commander->status & CMST_ENEMY_SIGHT_OFF ){
	/*視界off*/
	return;
    }

    if (EMA_CommandGetPosition() != NULL){	//エマチェック
	if(Eye_Sight_Check( eye , EMA_CommandGetPosition())){
	    if(!EyeOnlineCheck( hzx_id, eye->eyepos,  EMA_CommandGetPosition()) ){
		eye->PC_CK_FLG |= EYECK_PC_EMMA_IN_SIGHT;
		printf("Find Emma!!\n");
		if (!(ENE_GameStatus & ENE_GMSTATUS_NPC_EMMA)){
		    printf("Satoyoshi: I found Emma but There is no Emma Flag Setting in scenario\n");
		    ASSERT(0);
		}
	    }
	}
    }

    if (PL_GetPlayerItem() == IT_Stealth){		//ステルス
	eye->PC_CK_FLG |= EYECK_PC_NOT_IN_SIGHT;
	return;
    }

    if(commander->status & CMST_ENEMY_SIGHT_OFF ){	//視界off
	eye->PC_CK_FLG |= EYECK_PC_NOT_IN_SIGHT;
	return;
    }

    if (GM_PlayerStatus & PLAYER_INTRUDE){		//イントルード
	eye->PC_CK_FLG |= EYECK_PC_NOT_IN_SIGHT;
	return;
    }

    // *****ダンボールは見えない*****
    if ( (GM_PlayerStatus & PLAYER_CB_BOX) &&
	 (!(GM_PlayerStatus & CB_BOX_FAIL)) &&
	 (!(COM_AlertStatus()&COM_ALERT_PLAYER_DETECT)) &&
	 ( GM_AlertMode != ALERT_MODE_ALERT )){
	eye->PC_CK_FLG |= EYECK_PC_NOT_IN_SIGHT;
	return;
    }
    

    // *****通常チェック*****
    if(Eye_Sight_Check( eye , &GM_PlayerFindPos)){
	if(EyeOnlineCheck( hzx_id, eye->eyepos, &GM_PlayerFindPos) ){
	    if (HZX_GetZone( GM_PlayerControl->hzx_id, HZX_Zone1(GM_PlayerAddress) )->flag & HZX_ZONE_NO_AVOID){
		//		printf ("No Avoid\n");
		eye->PC_CK_FLG |= EYECK_PC_NO_AVOID;
	    }
	}
	else {
	    eye->PC_CK_FLG |= EYECK_PC_IN_SIGHT;
	    //	printf ("Find PC\n");
	    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~変装している	
	    //	    if ( (PL_GetPlayerItem() == IT_Uniform)	&&
	    //		 (PL_GetPlayerWeapon() == WP_Aks)	&&	/* AKを装備 */
	    //		 (PL_GetPlayerWeapon() == WP_m4)  )		/* M4を装備 */
	    
	    if ( (PL_GetPlayerItem() == IT_Uniform)
		 && (GM_PlayerStateFlag & PL_GBSCAP_EXIST)
		 && !(COM_StageKind()&ENE_STAGE_GPS) )
	    {
		
		eye->PC_CK_FLG |= EYECK_PC_IN_UNIFORM;
		
		// ===================近くを狙っている
		if (GM_CheckPlayerStatus( PLAYER_WATCH ) ){
		    if ( !(PL_GetPlayerWeapon() == WP_None)){
			if (CheckDistanceLineAndPoint( (FVECTOR*)DG_Chanl(0)->eye.m[3],
						       (FVECTOR*)DG_Chanl(0)->eye.m[2],
						       eye->eyepos) < 350.0f){
			    eye->PC_CK_FLG |= EYECK_PC_AIMING_NEAR;
			    //			    printf ("Aim Near\n");
			}
		    }
		}
		
		if (GM_CheckPlayerStatus( PLAYER_ROLLING ) ){
		    eye->PC_CK_FLG |= EYECK_PC_ROLLING;
		    //		    printf ("Rolling\n");
		}
		
		if (GM_WeaponFire != -1){
		    eye->PC_CK_FLG |= EYECK_PC_FIRE_ARMS;
		    //		    printf ("Fire  %d\n",GM_WeaponFire);
		}
		
		
		if  ( (GM_PlayerStatus & PLAYER_GROUND) ||
		      (GM_PlayerStatus & PLAYER_DOWNED) ){
		    eye->PC_CK_FLG |= EYECK_PC_PLONE;
		    //		    printf ("PC Plone \n");
		}
	    }
	}
    }


}






//#define CAM_ENE_PRINT

/**************************<-------local function------>*************************/
/*	名前:	int DEV_EnemyCheck						*/
/*	返値:	int	いない: -1   発見: uniq_id				*/
/*	引数:	EYEPARAM	*eye						*/
/*		HZX_GROUP_ID	hzx_id						*/
/*	説明:	ダメージ兵と敵死兵を探す					*/
/********************************************************************************/
static int Eye_EnemyCheck(EYEPARAM *eye , HZX_GROUP_ID hzx_id ){
    
    COMMANDER	*com ;
    int i,corp_num;
    short	tmp_uniq_id;
    ENETHINK *dam_entk ;
    FVECTOR		*trg ,res;
    SVECTOR		rgb;
    NEWCORP	*corp ;
    rgb.vx = 255; rgb.vy = 0; rgb.vz = 0;
	
    com = COM_GetCommander() ;

    eye->ENE_CK_FLG = 0;


    //	printf ("###				DENUM: %d\n", com->dam_en_num);

    for ( i=0; i<com->dam_en_num; i++ ) {

	dam_entk = com->dam_entk[i] ;
	trg = &dam_entk->ctrl->mov ;
	if(Eye_Sight_Check( eye , trg)){
	    if(! EyeOnlineCheck( hzx_id, eye->eyepos, trg ) ){
		int	flag = 0;

		if (dam_entk->act->status & ACT_STATUS_HOLD_UP){
			if ( !(dam_entk->act->bodyp.type & ENE_TYPE_HOLD_EXIT) ) {
#ifdef CAM_ENE_PRINT
		    printf("ホールドアップ！\n");
#endif
			    eye->ENE_CK_FLG |= EYECK_ENE_HOLDUP;
			    flag = 1;
			}
		}
		
		if (dam_entk->act->status & ACT_STATUS_MASUI_SASARU){
#ifdef CAM_ENE_PRINT
		    printf("麻酔がささる！\n");
#endif
		    eye->ENE_CK_FLG |= EYECK_ENE_MASUI;
		    flag = 1;
		}
		
		if (dam_entk->act->status & ACT_STATUS_DAMAGE_NOW){
#ifdef CAM_ENE_PRINT
		printf("ダメージ今うける！\n");
#endif
		    eye->ENE_CK_FLG |= EYECK_ENE_OUCH;
		    flag = 1;
		}
		
		if (dam_entk->act->status & ACT_STATUS_FAINT){
			if ( !ENE_FaintExit( dam_entk->act ) ) {
#ifdef CAM_ENE_PRINT
		printf("気絶中！\n");
#endif
			    eye->ENE_CK_FLG |= EYECK_ENE_FAINT;
		    	flag = 1;
		    }
		}
		
		if (dam_entk->act->status & ACT_STATUS_DAMAGE){
#ifdef CAM_ENE_PRINT
		printf("ダメージ中！\n");
#endif
		    eye->ENE_CK_FLG |= EYECK_ENE_DAMAGED;
		    flag = 1;
		}
		
		if (dam_entk->act->status & ACT_STATUS_CAPTURE){
#ifdef CAM_ENE_PRINT
		printf("ツカマッチョ！\n");
#endif
		    eye->ENE_CK_FLG |= EYECK_ENE_CAPTURED;
		    flag = 1;
		}
		
		if (dam_entk->act->status & ACT_STATUS_SLEEP){
			if ( !(dam_entk->act->bodyp.type & ENE_TYPE_FAINT_EXIT) ) {
#ifdef CAM_ENE_PRINT
		printf("眠っている！\n");
#endif
			    eye->ENE_CK_FLG |= EYECK_ENE_SLEEPING;
			    flag = 1;
			}
		}

		if (0){		// ********************::落下兵
#ifdef CAM_ENE_PRINT
		printf("落下中\n");
#endif
		    eye->ENE_CK_FLG |= EYECK_ENE_FALLING;
		    flag = 1;
		}

		tmp_uniq_id = dam_entk->uniq_id;

		if (flag == 0){
		    tmp_uniq_id = 0;
		}
		if (!COM_AccidentToID(tmp_uniq_id)){
		if(tmp_uniq_id){
		    eye->accident_hzx_id = dam_entk->ctrl->hzx_id ;
		    eye->accident_pos = dam_entk->ctrl->mov;
		    return dam_entk->uniq_id;
		}
		}
	    }
	    else {
#ifdef CAM_ENE_PRINT
		printf("オンライン弾き\n");
#endif
	    }
	}
    }

    // =====================死体チェック
    corp_num = CP_GetCorpNum();
    if(corp_num != 0){
	corp = CP_GetCorpFromEye( eye,&res) ;
	if(corp != NULL){
	    tmp_uniq_id = corp->uniq_id;
#ifdef CAM_ENE_PRINT
	    printf("死体４\n");
#endif
	    eye->ENE_CK_FLG |= EYECK_ENE_DEAD;


	    if (0){		// ********************::落下兵
#ifdef CAM_ENE_PRINT
		printf("落下中\n");
#endif
		eye->ENE_CK_FLG |= EYECK_ENE_FALLING;
	    }
	    if (!COM_AccidentToID(tmp_uniq_id)){
	    if(tmp_uniq_id){
		eye->accident_hzx_id = corp->ctrl->hzx_id ;
		eye->accident_pos = corp->ctrl->mov;
		return corp->uniq_id;
	    }
	    }
	}
    }
    return 0 ;
}



/**************************<-------local function------>*************************/
/*	名前:	void Eye_Check_ENEMYn						*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	視界チェック 敵兵関連						*/
/********************************************************************************/
static int Eye_Check_ENEMY(EYEPARAM *eye){
    int tmp_uniq_id;
    
    tmp_uniq_id = Eye_EnemyCheck( eye ,eye->check_hzx_id );   
    
    if(tmp_uniq_id){
	//	printf ("ダメージ＆死体兵隊通報 %d\n", tmp_uniq_id);
	return tmp_uniq_id;
    }
    return 0;
}


    
/**************************<-------local function------>*************************/
/*	名前:	void Meca_Eye_Check						*/
/*	引数:	EYEPARAM	*eye						*/
/*	説明:	視界チェック全般を行なう					*/
/********************************************************************************/
static void Meca_Eye_Check(EYEPARAM *eye){

    // ================プレイヤチェック
    
    Eye_Check_PC(eye);
    eye->accident_uniq_id = Eye_Check_ENEMY(eye);
    
    if(GM_NikitaAlive[0] == NKT_NORMAL){			// ニキータをチェック
	if (Eye_Sight_Check( eye, &GM_NikitaPosition[0])==1){
		printf ("Nikita check\n");
	    if(! EyeOnlineCheck( GM_PlayerControl->hzx_id, eye->eyepos, &GM_NikitaPosition[0]) ){	    
		eye->PC_CK_FLG |= EYECK_NIKITA_IN_SIGHT;
		printf ("Nikita in\n");
	    }
	}
    }
}
    
    

