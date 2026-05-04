/********************************************************************************/
/*	har_kastgt.c								*/
/*	カサッカのターゲット関連						*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: 		*/
/********************************************************************************/

#define	TARGET_MOVEROT( _target, _vx, _vy, _vz, _rx, _ry, _rz )\
{\
    tmpvec.vx = (_vx);						\
    tmpvec.vy = (_vy);						\
    tmpvec.vz = (_vz);						\
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

#define	TARGET_SETUP( _target, _x, _y, _z, _sx, _sy, _sz, _fnc )\
{								\
    b_size.vx = (_x);						\
    b_size.vy = (_y);						\
    b_size.vz = (_z);						\
    s_size.vx = (_sx);						\
    s_size.vy = (_sy);						\
    s_size.vz = (_sz);						\
    GM_SetTarget( (_target), flag,map, ENEMY_SIDE, 		\
		  &b_size, &s_size ) ;				\
    GM_SetTargetCallBack( (_target), callback_fnc, work );	\
    GM_SetTargetWeaponType((_target),0);			\
    GM_SetPowerTarget( (_target), (_fnc),			\
	       POWER_DECREASE, 1000, 0, 0, &DG_ZeroVector );	\
    GM_PutTarget( (_target) );					\
}


/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void move_Kas_target						*/
/*	引数:	Work	*work							*/
/*	説明:	カサッカ ターゲットの移動					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void move_Kas_target(Kas_Work *work){
    
    FMATRIX	*parentmat;
    FVECTOR	tmpvec;
    SVECTOR	tmprot;
    FMATRIX	mat;
    
    GM_MoveTarget2Map( &work->tgt_body[0], &work->snake.objs->objs[HUMAN21_ATAMA].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[1], &work->snake.objs->objs[HUMAN21_KOSHI].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[2], &work->snake.objs->objs[HUMAN21_MIGI_UDE1].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[3], &work->snake.objs->objs[HUMAN21_HIDARI_UDE1].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[4], &work->snake.objs->objs[HUMAN21_MIGI_ASHI1].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[5], &work->snake.objs->objs[HUMAN21_HIDARI_ASHI1].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[6], &work->snake.objs->objs[HUMAN21_MIGI_ASHI2].world, work->control.map);
    GM_MoveTarget2Map( &work->tgt_body[7], &work->snake.objs->objs[HUMAN21_HIDARI_ASHI2].world, work->control.map);
    
    parentmat = &work->body.objs->objs[0].world;
    
    GM_MoveTarget2Map( &work->kak_body[0], &work->body.objs->objs[0].world, work->control.map);
    
    TARGET_MOVEROT(&work->kak_body[1], 0.0f, -1420.0f,  2800.0f,  430, 0, 0);	//キャノピー
    TARGET_MOVEROT(&work->kak_body[2], 0.0f, 980.0f, -7200.0f, -225, 0, 0);	//しっぽとさか
    
    GM_MoveTarget2Map( &work->kak_body[3], &work->body.objs->objs[1].world, work->control.map);	
    GM_MoveTarget2Map( &work->kak_body[4], &work->body.objs->objs[0].world, work->control.map);
    GM_MoveTarget2Map( &work->kak_body[7], &work->body.objs->objs[0].world, work->control.map);
    GM_MoveTarget2Map( &work->kak_body[8], &work->body.objs->objs[0].world, work->control.map);
    
    TARGET_MOVEROT( &work->kak_body[5],  1510.0f, -1200.0f, -5200.0f, -160, 0, 0);	//垂直尾翼右
    TARGET_MOVEROT( &work->kak_body[6], -1510.0f, -1200.0f, -5200.0f, -160, 0, 0);	//垂直尾翼右
    
    TARGET_MOVEROT( &work->kak_body[9], 0.0f, -160.0f, -1500.0f, -180, 0, 0);	//斜め天井後ろ
    TARGET_MOVEROT(&work->kak_body[10], 0.0f, -270.0f,  1300.0f,  240, 0, 0);	//斜め天井前
    TARGET_MOVEROT(&work->kak_body[11], 0.0f, -820.0f, -7100.0f,  300, 0, 0);	//テールローター
    
    TARGET_MOVEROT(&work->kak_body[12], 650.0f, -1420.0f, 1500.0f, 0, -60, 0);		//左側面前
    TARGET_MOVEROT(&work->kak_body[13], 700.0f, -1120.0f, -1200.0f, 0, 60, 0);		//左側面後ろ
    
    TARGET_MOVEROT(&work->kak_body[14], -650.0f,-1420.0f, 1500.0f, 0, 60, 0);		//左側面前
    TARGET_MOVEROT(&work->kak_body[15], -700.0f,-1120.0f, -1000.0f, 0, -60, 0);		//左側面後ろ
    
    
    TARGET_MOVEROT(&work->kak_body[16], 450.0f,-1320.0f, -3250.0f, 0, 65, 0);		//しっぽつけね
    
    TARGET_MOVEROT(&work->kak_body[17], -450.0f,-1320.0f, -3250.0f, 0, -65, 0);		//しっぽつけね
    
    
    TARGET_MOVEROT(&work->kak_body[18], 0.0f, -1770.0f, -2150.0f, 130, 0, 0);	//しっぽつけね床
    
    TARGET_MOVEROT(&work->kak_body[19], 0.0f, -2020.0f, 3200.0f, -110, 0, 0);	//はな
}



/**************************<-------local function------>*************************/
/*	名前:	void *tgtCallBack_snk						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void	*ptr							*/
/*	説明:	スネークのコールバック						*/
/********************************************************************************/
static void tgtCallBack_snk(TARGET *off, TARGET *def, void *ptr )
{
    Kas_Work	*work ;
    work = ( Kas_Work * )ptr ;

    if (work->snk_mtk_time > 0){
	return;
    }
    
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {

	    // ****スネークは怒る
	    work->snake_ikari_tame++;	//怒りは蓄積する
	    if (work->snake_ikari_tame < 4){
		work->snake_ikari = work->snake_ikari_tame;
	    }
	    else {
		work->snake_ikari=(work->snake_ikari_tame-3)*4;
	    }
	    work->snake_sizume_time = 60 * 5;
	    work->snake_motion = SET_SNK_DAM;	//やられ

	    if (work->damage < off->power->damage){ // 高い方のダメージを適用

		work->damage = off->power->damage; 
		work->snk_mtk_time = 120 * 5;

		// ****スネークの発する声
		if (work->gageset.value > work->damage){
		    if(def->weapon_type & WP_BULLET){
			if (RAND(2)==0){
			    GM_SeSetFromVolCurve(SD_V_SNADMG11, &work->control.mov, &kac_se_curves);//うっ
			}
			else {
			    GM_SeSetFromVolCurve(SD_V_SNADMG12, &work->control.mov, &kac_se_curves);//ううっ
			}
		    }
		    else {
			if (RAND(2)==0){
			    GM_SeSetFromVolCurve(SD_V_SNADMG13, &work->control.mov, &kac_se_curves);//おうっ
			}
			else {
			    GM_SeSetFromVolCurve(SD_V_SNADMG14, &work->control.mov, &kac_se_curves);//うおっ！
			}
		    }
		}
	    }
	    
	    
	}
    }
}

/**************************<-------local function------>*************************/
/*	名前:	void *tgtCallBack_kak						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*def							*/
/*		void	*ptr							*/
/*	説明:	カサッカのコールバック						*/
/********************************************************************************/
static void tgtCallBack_kak(TARGET *off, TARGET *def, void *ptr )
{
    Kas_Work	*work ;
    work = ( Kas_Work * )ptr ;
    
    if ( def->damaged & TARGET_POWER ) {
	if ( off->class & TARGET_POWER ) {


	    // ****スネークは怒る
	    if (work->snake_ikari_tame < 4){
		work->snake_ikari = work->snake_ikari_tame;
	    }
	    else {
		work->snake_ikari=(work->snake_ikari_tame-3)*4;
	    }
	    work->snake_sizume_time = 60 * 5;
	    
	    HarCallSpark(&def->hit, &off->power->force);
	    
	    if (work->gageset.value > work->damage){	    
		if (work->ot_speek_time == 0){
		    switch(RAND(4)){
		    case 0:
			GM_SeSetFromVolCurve(SD_V_OTADMG01, &work->control.mov, &kac_se_curves);//うっ
			break;
		    case 1:
			GM_SeSetFromVolCurve(SD_V_OTADMG02, &work->control.mov, &kac_se_curves);//ううっ
			break;
		    case 2:
			GM_SeSetFromVolCurve(SD_V_OTADMG03, &work->control.mov, &kac_se_curves);//おうっ
			break;
		    case 3:
			GM_SeSetFromVolCurve(SD_V_OTADMG04, &work->control.mov, &kac_se_curves);//うおっ！
			break;
		    }
		    work->ot_speek_time = 90*5;	//オタコン喋りタイマー
		}
	    }
	    
	    if((def->weapon_type & WP_BULLET)&&(work->damage == 0)){
		work->damage = 1;
	    }
	    else if (work->damage < off->power->damage){ 
		work->damage = off->power->damage; 
	    }
	}
    }
}



/**************************<-------local function------>*************************/
/*	名前:	void *grn_callback_func						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:	グレネードのオンライン当たり					*/
/********************************************************************************/
void *grn_callback_func( TARGET *off, TARGET *def, void *ptr )
{
    Kas_Work	*work ;
    work = ( Kas_Work * )ptr ;
    
    work->grn_hit[off->name] = ON;
    return 0;
}


/**************************<-------local function------>*************************/
/*	名前:	void kak_settarget						*/
/*	引数:	Kas_Work	*work						*/
/*	説明:						*/
/********************************************************************************/
static void kak_settarget(Kas_Work *work){
    
    int	flag, map;
    FVECTOR	b_size;
    FVECTOR	s_size;
    TARGET_CALLBACK	callback_fnc;
    
    // *****ターゲットの設定 ノズルはロックオンされる
    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_LOCKON|TARGET_POWER);
    map = work->control.map;
    callback_fnc = tgtCallBack_snk;
    
    TARGET_SETUP( &work->tgt_body[1], 180.0f, 370.0f, 140.0f, 0.0f,  120.0f, 0.0f,
		  &work->har_snk_ptgt);						//体	
    TARGET_SETUP( &work->kak_body[11], 300.0f,  800.0f,  920.0f,		//テールローター
		  0.0f,    0.0f,    0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[3], 700.0f,   50.0f, 700.0f,			//メインプロペラ
		  0.0f,  200.0f,    0.0f, &work->har_snk_ptgt);

    
    flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_ROTATE|TARGET_POWER);
    TARGET_SETUP( &work->tgt_body[0],  80.0f, 110.0f, 100.0f, 0.0f,   40.0f, 0.0f,
		  &work->har_snk_ptgt);						//どたま
    TARGET_SETUP( &work->tgt_body[2],  60.0f, 200.0f,  60.0f, 0.0f, -125.0f, 0.0f,
		  &work->har_snk_ptgt);						//みぎて
    TARGET_SETUP( &work->tgt_body[3],  60.0f, 200.0f,  60.0f, 0.0f, -150.0f, 0.0f,
		  &work->har_snk_ptgt);						//ひだりて
    TARGET_SETUP( &work->tgt_body[4], 100.0f, 250.0f, 100.0f, 0.0f, -150.0f, 0.0f,
		  &work->har_snk_ptgt);						//みぎもも
    TARGET_SETUP( &work->tgt_body[5], 100.0f, 250.0f, 100.0f, 0.0f, -150.0f, 0.0f,
		  &work->har_snk_ptgt);						//ひだりもも
    TARGET_SETUP( &work->tgt_body[6],  80.0f, 350.0f,  60.0f, 0.0f, -200.0f, 0.0f,
		  &work->har_snk_ptgt);						//みぎあし
    TARGET_SETUP( &work->tgt_body[7],  80.0f, 350.0f,  60.0f, 0.0f, -200.0f, 0.0f,
		  &work->har_snk_ptgt);						//ひだりあし
    
    
    callback_fnc = tgtCallBack_kak;
    
    
    TARGET_SETUP( &work->kak_body[0],  600.0f,   80.0f, 1700.0f,			//中心床
		  0.0f,-2270.0f,  800.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[1], 600.0f,  300.0f,  750.0f,			//キャノピー
		  0.0f,    0.0f,    0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[2], 150.0f, 1000.0f,  250.0f,			//しっぽとさか
		  0.0f,    0.0f,    0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[4], 1450.0f,   50.0f,  230.0f,			//水平尾翼
		  0.0f, -1250.0f,-5140.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[5],   50.0f,  700.0f,  300.0f,			//水直尾翼左
		  0.0f, 0.0f, 0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[6],   50.0f,  700.0f,  300.0f,			//水直尾翼右
		  0.0f, 0.0f,0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[7], 1200.0f,  50.0f,   250.0f,			//水平尾翼上
		  0.0f,  1910.0f,-7800.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[8],  300.0f,  600.0f, 1600.0f,			//しっぽ
		  0.0f,-1170.0f,-4800.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[9],  650.0f,  300.0f, 1300.0f,			//斜め天井後ろ
		  0.0f,    0.0f,    0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[10], 600.0f,   80.0f, 1300.0f,			//斜め天井前
		  0.0f,    0.0f,    0.0f, &work->har_snk_ptgt);

    TARGET_SETUP( &work->kak_body[12],   80.0f,  900.0f,  600.0f,			//左側面前
		  0.0f, 0.0f, 0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[13],   80.0f,  900.0f, 850.0f,			//左側面後
		  0.0f, 0.0f,0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[14],   80.0f,  900.0f,  600.0f,			//左側面前
		  0.0f, 0.0f, 0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[15],   80.0f,  900.0f, 1000.0f,			//左側面後
		  0.0f, 0.0f,0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[16],   80.0f,  500.0f, 1100.0f,			//しっぽつけね
		  0.0f, 0.0f, 0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[17],   80.0f,  500.0f, 1100.0f,			//しっぽつけね
		  0.0f, 0.0f, 0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[18],  500.0f, 300.0f, 1300.0f,			//しっぽつけね床
		  0.0f,  0.0f,  0.0f, &work->har_snk_ptgt);
    TARGET_SETUP( &work->kak_body[19], 450.0f,  200.0f,  800.0f,			//はな
		  0.0f,   0.0f,    0.0f, &work->har_snk_ptgt);


    {	// *********************攻撃ターゲット
	short	loop;

	for (loop=0; loop<SNAKE_GRND_NUM; loop++){
	    GM_SetTarget( &work->grn_tgt[loop], TARGET_POWER|TARGET_OFFENSE|TARGET_ONLINE|TARGET_THROUGH,
			  GM_PlayerMap, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector);
	    GM_SetTargetCallBack( &work->grn_tgt[loop], (void*)grn_callback_func, 
				  work ) ;
	    work->grn_tgt[loop].name = loop;
	}


    }
}	

