/********************************************************************************/
/*	har_target.c								*/
/*	ハリアターゲット関連							*/
/*	2001/04/19 H.Satoyoshi							*/
/*	$Id: har_target.c,v 1.1.1.3 2002/11/19 11:48:25 Yoshizawa1 Exp $		*/
/********************************************************************************/
#define	HAR_DAMAGE	(175)

/********************************************************************************/
/*	define      								*/
/********************************************************************************/
#define	TARGET_MOVEROT( _target, _x, _y, _z, _rx, _ry, _rz )\
{\
    tmpvec.vx = (_x);						\
    tmpvec.vy = (_y);						\
    tmpvec.vz = (_z);						\
    tmprot.vx = (_rx);						\
    tmprot.vy = (_ry);						\
    tmprot.vz = (_rz);						\
    DG_SetPos( parentmat);					\
    DG_MovePos( &tmpvec );					\
    DG_RotatePos( &tmprot );					\
    DG_GetPos( &mat  );						\
    GM_MoveTarget2Map( (_target), &mat,				\
    work->control.map);						\
}

#define	TARGET_MOVE( _target, _x, _y, _z )\
{\
    tmpvec.vx = (_x);						\
    tmpvec.vy = (_y);						\
    tmpvec.vz = (_z);						\
    DG_SetPos( parentmat);					\
    DG_MovePos( &tmpvec );					\
    DG_GetPos( &mat  );						\
    GM_MoveTarget2Map( (_target), &mat,				\
    work->control.map);						\
}

#define	TARGET_SETUP( _target, _x, _y, _z )\
{\
    targ_size.vx = (_x);						\
    targ_size.vy = (_y);						\
    targ_size.vz = (_z);						\
    GM_SetTarget( (_target), flag,map, ENEMY_SIDE, 			\
		  &targ_size, &DG_ZeroVector );				\
    GM_SetTargetCallBack( (_target), targCallBack_body, work );	\
    GM_SetTargetWeaponType( (_target) ,0);			       	\
    GM_SetPowerTarget( (_target) , &work->har_pow_tgt,			\
		       POWER_DECREASE, 1000, 0, 0, &DG_ZeroVector );	\
    GM_PutTarget( (_target) );						\
}





static void* dumfunc(){
    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void HarCallSpark						*/
/*	引数:	FVECTOR *pos							*/
/*	引数:	FVECTOR *force							*/
/*	説明:	火花を呼ぶ	       						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void HarCallSpark(FVECTOR *pos ,FVECTOR *force) {
    SVECTOR	rot ;
    FMATRIX	w ;
    
    _FVecToRotXY( force, &rot ) ;
    DG_SetPos2( pos, &rot ) ;
    DG_GetPos( &w ) ;
    NewSpark( &w ) ;
}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_targnzl_lock						*/
/*	引数:	Work *work							*/
/*	説明:	ターゲットロックオン設定       					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_targnzl_lock(Work *work, int flg)
{
    if (flg){
	work->target_b_nzl[0].class |= TARGET_LOCKON;
	work->target_b_nzl[1].class |= TARGET_LOCKON;
    }
    else {
	work->target_b_nzl[0].class &= ~TARGET_LOCKON;
	work->target_b_nzl[1].class &= ~TARGET_LOCKON;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_targbodyall_big						*/
/*	引数:	Work *work							*/
/*	説明:	ターゲット巨大化	       					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_targbodyall_big(Work *work)
{
    FVECTOR targ_size = {3500.0f, 4000.0f, 14000.0f, 0.0f};
    GM_SetTargetSize( &work->target_body_all, &targ_size );
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_targbodyall_big						*/
/*	引数:	Work *work							*/
/*	説明:	ターゲット巨大化	       					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_targbodyall_fat(Work *work)
{
    FVECTOR targ_size = {3500.0f, 7000.0f, 9000.0f, 0.0f};
    GM_SetTargetSize( &work->target_body_all, &targ_size );
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_targbodyall_small						*/
/*	引数:	Work *work							*/
/*	説明:	ターゲット小さく	       					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_targbodyall_small(Work *work)
{
    FVECTOR targ_size = {3500.0f, 1900.0f, 4200.0f, 0.0f};
    GM_SetTargetSize( &work->target_body_all, &targ_size );
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_targbodyall_skip						*/
/*	引数:	Work *work							*/
/*		int  flag	ON:スキップ 	OFF:解除			*/
/*	説明:	bodyallのスキップを設定解除する	 				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_targbodyall_skip(Work *work, int flag)
{
    if (flag){
	work->target_body_all.class |= TARGET_SKIP;
    }
    else {
	work->target_body_all.class &= ~TARGET_SKIP;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	set_hartarg_skip						*/
/*	引数:	Work *work							*/
/*		int  flag	ON:スキップ 	OFF:解除			*/
/*	説明:	全体ターゲットのスキップを設定解除する				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_hartarg_skip(Work *work, int flag)
{
    if (flag){
	work->target_b_nzl[0].class |= TARGET_SKIP;
	work->target_b_nzl[1].class |= TARGET_SKIP;
	work->target_body_center.class |= TARGET_SKIP;
	work->target_body_side[0].class |= TARGET_SKIP;
	work->target_body_side[1].class |= TARGET_SKIP;
	work->target_body_side[2].class |= TARGET_SKIP;
	work->target_body_side[3].class |= TARGET_SKIP;
	work->target_body_tale.class |= TARGET_SKIP;
	work->target_body_pod[0].class |= TARGET_SKIP;
	work->target_body_pod[1].class |= TARGET_SKIP;
	work->target_body_pod[2].class |= TARGET_SKIP;
	work->target_wepon_mpod[0].class |= TARGET_SKIP;
	work->target_wepon_mpod[1].class |= TARGET_SKIP;
	work->target_rwing[0].class |= TARGET_SKIP;
	work->target_rwing[1].class |= TARGET_SKIP;
	work->target_lwing[0].class |= TARGET_SKIP;
	work->target_lwing[1].class |= TARGET_SKIP;
	work->target_tale_wing[2].class |= TARGET_SKIP;
	work->target_tale_wing[0].class |= TARGET_SKIP;
	work->target_tale_wing[1].class |= TARGET_SKIP;
	work->target_body_canopy.class |= TARGET_SKIP;
	work->target_body_nose.class |= TARGET_SKIP;
	work->target_body_centertop.class |= TARGET_SKIP;
	work->target_wheel_bar[0].class |= TARGET_SKIP;
	work->target_wheel_bar[1].class |= TARGET_SKIP;
    }
    else {
	work->target_b_nzl[0].class &= ~TARGET_SKIP;
	work->target_b_nzl[1].class &= ~TARGET_SKIP;
	work->target_body_center.class &= ~TARGET_SKIP;
	work->target_body_side[0].class &= ~TARGET_SKIP;
	work->target_body_side[1].class &= ~TARGET_SKIP;
	work->target_body_side[2].class &= ~TARGET_SKIP;
	work->target_body_side[3].class &= ~TARGET_SKIP;
	work->target_body_tale.class &= ~TARGET_SKIP;
	work->target_body_pod[0].class &= ~TARGET_SKIP;
	work->target_body_pod[1].class &= ~TARGET_SKIP;
	work->target_body_pod[2].class &= ~TARGET_SKIP;
	work->target_wepon_mpod[0].class &= ~TARGET_SKIP;
	work->target_wepon_mpod[1].class &= ~TARGET_SKIP;
	work->target_rwing[0].class &= ~TARGET_SKIP;
	work->target_rwing[1].class &= ~TARGET_SKIP;
	work->target_lwing[0].class &= ~TARGET_SKIP;
	work->target_lwing[1].class &= ~TARGET_SKIP;
	work->target_tale_wing[2].class &= ~TARGET_SKIP;
	work->target_tale_wing[0].class &= ~TARGET_SKIP;
	work->target_tale_wing[1].class &= ~TARGET_SKIP;
	work->target_body_canopy.class &= ~TARGET_SKIP;
	work->target_body_nose.class &= ~TARGET_SKIP;
	work->target_body_centertop.class &= ~TARGET_SKIP;
	work->target_wheel_bar[0].class &= ~TARGET_SKIP;
	work->target_wheel_bar[1].class &= ~TARGET_SKIP;
    }
}


void har_call_se(Work* work){

    if (work->gageset.value <= work->har_damage){
	printf ("Har End Damage\n");
	return;
    }

    if (work->gageset.value > work->gageset.max/2){
	while (1){
	    switch (RAND(5)){
	    case 0:
		if (work->str_num != SD_V_SOLDM101){	//ぬぅ
		    SetSolVoice(work, SD_V_SOLDM101, 15*5);
		    return;
		}
		break;
	    case 1:
		if (work->str_num != SD_V_SOLDM103){	//ちっ
		    SetSolVoice(work, SD_V_SOLDM103, 20*5);
		    return;
		}
		break;
	    case 2:
		if (work->str_num != 8){	//やるなぁ
		    SetSolVoice(work, 8, 13*5);
		    return;
		}
		break;
	    case 3:
		if (work->str_num != SD_V_SOLDM301){	//うおっ
		    SetSolVoice(work, SD_V_SOLDM301, 10*5);
		    return;
		}
		break;
	    case 4:
		if (work->str_num != SD_V_SOLDM302){	//うう
		    SetSolVoice(work, SD_V_SOLDM302, 10*5);
		    return;
		}
		break;
	    }
	}
    }
    else {
	while (1){
	    switch (RAND(4)){
	    case 0:
		if (work->str_num != SD_V_SOLDM102){	//糞
		    SetSolVoice(work, SD_V_SOLDM102, 30*5);
		    return;
		}
		break;
	    case 1:
		if (work->str_num != SD_V_SOLDM401){	//なにぃ
		    SetSolVoice(work, SD_V_SOLDM401, 30*5);
		    return;
		}
		break;
	    case 2:
		if (work->str_num != SD_V_SOLDM403){	//ぐあぁぁ
		    SetSolVoice(work, SD_V_SOLDM403, 10*5);
		    return;
		}
		break;
	    case 3:
		if (work->str_num != SD_V_SOLDM402){	//馬鹿なっ
		    SetSolVoice(work, SD_V_SOLDM402, 30*5);
		    return;
		}
		break;
	    }
	}
    }
}

    
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void targCallBack_nozl						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	ノズル当たりのコールバック					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void targCallBack_nozl( TARGET	*off, TARGET *def, void	*ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;
    
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {
	    if( def->weapon_type & WP_BLAST){


		    {
			if (work->damaged_point & DMGD_POINT_NEW_FLAG){
			    work->damaged_point = 0;
			}
			work->damaged_point |= ((0x01)<<def->name);
		    }


		if(((work->dmg_mtk_count == 0)||(work->dmg_mtk_count == MUTEKI_TIME))&&
		   (work->har_damage < HAR_DAMAGE)){
		    if( def->weapon_type & WP_NOPLAYER){	//素根ー九の攻撃
			printf ("Snake's atack %d\n",work->gageset.value);
			    if (work->gageset.value <= 175){
				work->har_damage = work->gageset.value-1;
			    }
			    else {
				work->har_damage = HAR_DAMAGE;
			    }
		    }
		    else {
			work->har_damage = HAR_DAMAGE;
		    }
		    har_call_se(work);
		    work->dmg_mtk_count = MUTEKI_TIME;
		}
	    }
	}
    }
    def->weapon_type = 0;
}




/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void *targCallBack_body						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	胴体当たりのコールバック					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void targCallBack_body( TARGET	*off, TARGET *def, void	*ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;
    
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {
	    
	    // *************銃で撃たれた
	    if( def->weapon_type & (WP_M92|WP_PSG1T) ){	//跳弾のみ
		HarCallSpark(&def->hit, &off->power->force);
	    }
	    if( def->weapon_type & WP_BULLET){
		HarCallSpark(&def->hit, &off->power->force);
		
		printf ("GUN_HIT\n");
		work->gun_hit_count++;
		work->gageset.value = work->gageset.value - off->power->damage; 
		
		//		printf("GUN SHOT %d\n",off->power->damage); 
	    }
	    // ************爆発物
	    else if( def->weapon_type & WP_BLAST){
		
		
		{
		    if (work->damaged_point & DMGD_POINT_NEW_FLAG){
			work->damaged_point = 0;
		    }
		    work->damaged_point |= ((0x01)<<def->name);
		}
		
		
		
		
		if ((work->dmg_mtk_count == 0)||(work->dmg_mtk_count == MUTEKI_TIME)){
		    if (PL_LockonTarget==def){
			if (def->name == MIGI_MISPOD){	//ミサイル右
			    work->mispod_dmg_flg |= POD_RIGHT_DAMAGE;
			    printf ("右 ミサイルポッドに ダメージ !!!!!\n");
			}
			if (def->name == HIDA_MISPOD){	//ミサイル左
			    work->mispod_dmg_flg |= POD_LEFT_DAMAGE;
			    printf ("左 ミサイルポッドに ダメージ !!!!!\n");
			}
		    }
		}
		
		if(((work->dmg_mtk_count == 0)||(work->dmg_mtk_count == MUTEKI_TIME))&&
		   (work->har_damage < HAR_DAMAGE)){
		    if( def->weapon_type & WP_NOPLAYER){	//素根ー九の攻撃
			printf ("Snake's atack %d\n",work->gageset.value);
			if (work->gageset.value <= 175){
			    work->har_damage = work->gageset.value-1;
			}
			else {
			    work->har_damage = HAR_DAMAGE;
			}
		    }
		    else {
			work->har_damage = HAR_DAMAGE;
		    }
		    har_call_se(work);
		    work->dmg_mtk_count = MUTEKI_TIME;
		}
		
		//		printf("BOMB DAMAG %d\n",off->power->damage); 
	    }
	}
    }
    def->weapon_type = 0;
}





/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void *targCallBack_body						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	全体当たりのコールバック					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void targCallBack_body_all( TARGET	*off, TARGET *def, void	*ptr )
{
    Work	*work ;
    work = ( Work * )ptr ;
    
    if( def->weapon_type & WP_WEAPONCORE){
	if ((off->name == WP_Stinger)||(off->name == WP_Rgb6)){
	    
	    if(((work->dmg_mtk_count == 0)||(work->dmg_mtk_count == MUTEKI_TIME))&&
	       (work->har_damage < HAR_DAMAGE)){
		if( def->weapon_type & WP_NOPLAYER){	//素根ー九の攻撃
		    printf ("Snake's atack %d\n",work->gageset.value);
		    if (work->gageset.value <= 175){
			work->har_damage = work->gageset.value-1;
		    }
		    else {
			work->har_damage = HAR_DAMAGE;
		    }
		}
		else {
		    work->har_damage = HAR_DAMAGE;
		}
		har_call_se(work);
		work->dmg_mtk_count = MUTEKI_TIME;
	    }

	}
    }
    def->weapon_type = 0;
}


/*******************************<Global function>********************************/
/*	名前:	void Har_SetGageTarget						*/
/*	引数:	Work	*work							*/
/*	説明:	ターゲット＆ゲージの設定 GetResource内で呼びだし		*/
/********************************************************************************/
void Har_SetGageTarget(Work *work)
{
    /* ゲージの初期化 */
    //    FVECTOR	b_size = { 1100.0f,400.0f,450.0f,0.0f } ;
    FVECTOR	b_size = { 700.0f,700.0f,1500.0f,0.0f } ;
    FVECTOR	targ_size;
    GM_GageSet	*gs ;
    int		max, cur ;
    
    if (GCL_GetOption( 'l' ) != NULL){
	cur = GCL_GetNextInt() ;
	max = GCL_GetNextInt() ;
	gs = &work->gageset ;
	
	switch( GM_GameLevel ){
	case GM_LEVEL_VERYEASY:
	    cur = cur * 90;
	    max = max * 90;
	    break;
	case ST_LEV_EASY:
	    cur = cur * 100;
	    max = max * 100;
	    break;
	case ST_LEV_NORMAL:
	    cur = cur * 130;
	    max = max * 130;
	    break;

	case ST_LEV_UPNORM:
	    cur = cur * 140;
	    max = max * 140;
	    break;

	case GM_LEVEL_HARD:
	    cur = cur * 160;
	    max = max * 160;
	    break;
	case GM_LEVEL_E_EXTREME:
	case GM_LEVEL_EXTREME:
	    cur = cur * 190;
	    max = max * 190;
	    break;
	default:
	    cur = cur * 100;
	    max = max * 100;
	    break;
	}
	cur = cur / 40;
	max = max / 40;

	GM_InitGageSet( gs, "HARRIER", 16, 270, GM_DEFAULT_GAGE_HEIGHT*2+10,
			cur, max, 0, 30, GM_GAGE_LEVEL_ENEMY) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_ENEMY_LIFE) ;
	GM_AppendGageSet( gs ) ;
	GM_VisibleGage( gs ) ;
    }
    
    {
	int		flag, map;
	
	// *****ターゲット設定 ノズルはロックオンされる
	//	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_LOCKON|TARGET_ROTATE|TARGET_POWER);
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_LOCKON|TARGET_ROTATE|TARGET_POWER);
	map = work->control.map;
	
	//後ろノズル
	GM_SetTarget( &work->target_b_nzl[0], flag,map, ENEMY_SIDE, 
		      &b_size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->target_b_nzl[0], targCallBack_nozl, work ) ;
	GM_SetTargetWeaponType(&work->target_b_nzl[0],0);
	GM_SetPowerTarget( &work->target_b_nzl[0], &work->har_pow_tgt,
			   POWER_DECREASE, 1000, 0, 0, &DG_ZeroVector );
	GM_PutTarget( &work->target_b_nzl[0] );
	
	
	//前ノズル
	GM_SetTarget( &work->target_b_nzl[1], flag,map, ENEMY_SIDE, 
		      &b_size, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->target_b_nzl[1], targCallBack_nozl, work ) ;
	GM_SetTargetWeaponType(&work->target_b_nzl[1],0);
	GM_SetPowerTarget( &work->target_b_nzl[1], &work->har_pow_tgt,
			   POWER_DECREASE, 1000, 0, 0, &DG_ZeroVector );
	GM_PutTarget( &work->target_b_nzl[1] );


	//胴体全体 スティンガー特別ヒットCallBack あり
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER|TARGET_CALL_CALLBACK_THROUGH_HIT);

	targ_size.vx = 3500.0f;						
	targ_size.vy = 1900.0f;					
	targ_size.vz = 4200.0f;						
	GM_SetTarget( &work->target_body_all, flag,map, ENEMY_SIDE, 			
		      &targ_size, &DG_ZeroVector );				
	GM_SetTargetCallBack( &work->target_body_all, targCallBack_body_all, work );	
	GM_SetTargetWeaponType( &work->target_body_all ,0);			       	
	GM_SetPowerTarget( &work->target_body_all , &work->har_pow_tgt,			
			   POWER_DECREASE, 1000, 0, 0, &DG_ZeroVector );	
	GM_PutTarget( &work->target_body_all );
#ifdef DEBUG_MODE
	if (work->har_flag&HAR_DOUSA_FLAG_DEBUG){				
	    NewTargetView ( &work->target_body_all, 250, 250, 250);			
	}
#endif
	
	// *****ターゲット設定 ロックオンされない
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);

	TARGET_SETUP ( &work->target_body_center,		//胴体の芯
		       450.0f, 600.0f, 3900.0f);
	TARGET_SETUP ( &work->target_body_side[0],		//胴体下
		       900.0f, 120.0f, 1000.0f);
	TARGET_SETUP ( &work->target_body_side[1],		//胴体上
		       900.0f, 120.0f, 1000.0f);
	TARGET_SETUP ( &work->target_body_side[2],		//胴体左
		       120.0f, 400.0f, 1000.0f);
	TARGET_SETUP ( &work->target_body_side[3],		//胴体右
		       120.0f, 400.0f, 1000.0f);
	TARGET_SETUP ( &work->target_body_tale,			//胴体しっぽ
		       400.0f, 400.0f, 1800.0f);
	TARGET_SETUP ( &work->target_body_pod[0],		//胴体下面ポッド右
		       170.0f, 170.0f, 1450.0f);
	TARGET_SETUP ( &work->target_body_pod[1],		//胴体下面ポッド右
		       170.0f, 170.0f, 1450.0f);
	TARGET_SETUP ( &work->target_body_pod[2],		//胴体下面ポッド
		       250.0f, 150.0f, 600.0f);
	TARGET_SETUP ( &work->target_wepon_mpod[0],		//ミサイルポッド右
		       180.0f, 180.0f, 780.0f);
	TARGET_SETUP ( &work->target_wepon_mpod[1],		//ミサイルポッド左
		       180.0f, 180.0f, 780.0f);
	
	TARGET_SETUP ( &work->target_rwing[0],		//右翼0
		       1000.0f, 120.0f, 1000.0f);
	TARGET_SETUP ( &work->target_rwing[1],		//右翼1
		       1100.0f, 120.0f, 600.0f);
	TARGET_SETUP ( &work->target_lwing[0],		//左翼0
		       1000.0f, 120.0f, 1000.0f);
	TARGET_SETUP ( &work->target_lwing[1],		//左翼1
		       1100.0f, 120.0f, 600.0f);
	TARGET_SETUP ( &work->target_tale_wing[2],		//垂直尾翼
		       80.0f, 1100.0f, 600.0f);
	TARGET_SETUP ( &work->target_tale_wing[0],		//垂平尾翼0
		       900.0f, 70.0f, 500.0f);
	TARGET_SETUP ( &work->target_tale_wing[1],		//垂平尾翼1
		       900.0f, 70.0f, 500.0f);
	TARGET_SETUP ( &work->target_body_canopy,		//キャノピー
		       280.0f, 300.0f, 1500.0f);
	TARGET_SETUP ( &work->target_body_nose,		//ノーズ
		       300.0f, 280.0f, 700.0f);
	TARGET_SETUP ( &work->target_body_centertop,	//胴体の上
		       150.0f, 150.0f, 3600.0f);
	TARGET_SETUP ( &work->target_wheel_bar[0],		//車輪右
		       80.0f, 130.0f, 1200.0f);
	TARGET_SETUP ( &work->target_wheel_bar[1],		//車輪左
		       80.0f, 130.0f, 1200.0f);



	{
	    int loop;
	    TARGET* tgp = work->target_wheel_bar;
	    for (loop=0; loop<26; loop++){
		tgp->name = loop;
		tgp++;
	    }
	}



	work->target_wepon_mpod[0].name = MIGI_MISPOD;//ミサイル右
	work->target_wepon_mpod[1].name = HIDA_MISPOD;//ミサイル左
	
    }
#ifdef DEBUG_MODE
    if (work->har_flag&HAR_DOUSA_FLAG_DEBUG){
		NewTargetView (&work->target_body_all, 255, 0, 0);
		NewTargetView (&work->target_b_nzl[0], 255, 0, 0);
		NewTargetView (&work->target_b_nzl[1], 255, 0, 0);
   }
#endif
}


/*******************************<Global function>********************************/
/*	名前:	void Har_Movetarget						*/
/*	引数:	Work	*work							*/
/*	説明:	ターゲットの移動						*/
/********************************************************************************/
void Har_MoveTarget(Work *work)
{
    FMATRIX	mat;
    FVECTOR	tmpvec = { 0.0f, 0.0f, 0.0f, 0.0f } ;
    SVECTOR	tmprot = { 0, 0, 0, 0 } ;
    FMATRIX	*parentmat;
    parentmat = &work->body.objs->objs[0].world;
    
    TARGET_MOVE (&work->target_wheel_bar[0], -2525.0f, 200.0f, -1290.0f);//車輪右
    TARGET_MOVE (&work->target_wheel_bar[1], 2525.0f, 200.0f, -1290.0f);//車輪左
    TARGET_MOVEROT (&work->target_body_centertop, 0.0f, 950.0f, -400.0f, -40, 0, 0);	//胴体の芯上
    TARGET_MOVEROT (&work->target_body_canopy, 0.0f, 1100.0f, 4600.0f, 200, 0, 0);	//キャノピー
    TARGET_MOVE (&work->target_body_nose, 0.0f, 200.0f, 6600.0f);		//ノーズ
    TARGET_MOVEROT (&work->target_tale_wing[2], 0.0f, 1500.0f, -5100.0f, -650, 0, 0);	//垂直尾翼
    TARGET_MOVE (&work->target_wepon_mpod[0], -1950.0f, -220.0f, 680.0f);//ミサイルポッド右
    TARGET_MOVE (&work->target_wepon_mpod[1], 1950.0f, -220.0f, 680.0f);//ミサイルポッド左
    TARGET_MOVEROT (&work->target_rwing[0], -1650.0f, 550.0f, 0.0f, 0, -220, 160);//右翼0
    TARGET_MOVEROT (&work->target_rwing[1], -3400.0f, 150.0f, -700.0f, 0, -240, 160);//右翼1
    TARGET_MOVEROT (&work->target_lwing[0], 1650.0f, 550.0f, 0.0f, 0, 220, -160);//左翼0
    TARGET_MOVEROT (&work->target_lwing[1], 3400.0f, 150.0f, -700.0f, 0, 240, -160);//左翼1
    TARGET_MOVE (&work->target_body_pod[2], 0.0f, -670.0f, 470.0f);	//胴体下面ポッド
    TARGET_MOVE (&work->target_body_pod[0], -450.0f, -680.0f, 870.0f);	//胴体下面ポッド右
    TARGET_MOVE (&work->target_body_pod[1], 450.0f, -680.0f, 870.0f);	//胴体下面ポッド左
    TARGET_MOVE (&work->target_body_center, 0.0f, 200.0f, 1700.0f);	//胴体の芯
    TARGET_MOVE (&work->target_body_side[0], 0.0f, -300.0f,  2900.0f);	//胴体下
    TARGET_MOVE (&work->target_body_side[1], 0.0f,  700.0f,  2900.0f);	//胴体上
    TARGET_MOVE (&work->target_body_side[2], 930.0f, 200.0f, 2900.0f);	//胴体左
    TARGET_MOVE (&work->target_body_side[3], -930.0f, 200.0f, 2900.0f);	//胴体左
    TARGET_MOVE (&work->target_body_tale, 0.0f, 200.0f, -3900.0f);	//胴体尻尾
    
    TARGET_MOVE (&work->target_body_all, 0.0f, 0.0f, 0.0f);		//胴体の全体

    //    parentmat = &work->body.objs->objs[1].world;
    parentmat = &work->nozel[2];
    TARGET_MOVE (&work->target_b_nzl[0], 0.0f, 0.0f, 1000.0f);

    
    //    parentmat = &work->body.objs->objs[10].world;
    parentmat = &work->nozel[3];
    TARGET_MOVE (&work->target_b_nzl[1], 0.0f, 0.0f, 1000.0f);
    
    parentmat = &work->body.objs->objs[9].world;
    TARGET_MOVEROT (&work->target_tale_wing[0], -900.0f, -400.0f, -500.0f, 0, -300, 180);//垂平尾翼0
    TARGET_MOVEROT (&work->target_tale_wing[1], 900.0f, -400.0f, -500.0f, 0, 300, -180);//垂平尾翼1

    {	
	if (work->dist_new < 50000.0f){
	    work->target_wepon_mpod[0].class |= TARGET_LOCKON;
	    work->target_wepon_mpod[1].class |= TARGET_LOCKON;
	    work->target_wepon_mpod[0].class &= ~TARGET_SKIP;
	    work->target_wepon_mpod[1].class &= ~TARGET_SKIP;
	}
	else {
	    work->target_wepon_mpod[0].class &= ~TARGET_LOCKON;
	    work->target_wepon_mpod[1].class &= ~TARGET_LOCKON;
	    work->target_wepon_mpod[0].class |= TARGET_SKIP;
	    work->target_wepon_mpod[1].class |= TARGET_SKIP;
	}
    }

    work->damaged_point |= DMGD_POINT_NEW_FLAG;


#ifdef DEBUG_MODE
    if (HAR_TGT_VIEW == ON){
	static int num = 0; 
	int loop;
	TARGET* tgp = work->target_wheel_bar;

	if (GV_PadDataDirect[1].press & PAD_L2){
	    num++;
	    if (num>=26){
		num = 0;
	    }
	    printf ("TARGET NUM = %d\n", num);
	}

	for (loop=0; loop<26; loop++){
	    if (num == loop){
		NewTargetView2( tgp, 250, 250, 0 );
	    }
	    else if (work->damaged_point & ((0x01)<<tgp->name)){
		NewTargetView2( tgp, 200, 0, 0 );
	    }
	    else {
		NewTargetView2( tgp, 0, 250, 0 );
	    }
	    tgp++;
	}
    }
#endif
}




