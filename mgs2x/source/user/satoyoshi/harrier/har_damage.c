/********************************************************************************/
/*	Har_main.c								*/
/*	ハリアメイン *NewHarrier						*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_damage.c,v 1.1.1.3 2002/11/19 11:48:20 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

#define SET_DAMAGE_MODE(_mod)	\
    work->dmg_mode = (_mod);	\
    work->dmg_mtk_count = 0;	\
    work->damage_downy = 0;	\
    work->damage_step = 0

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void harDamageaction						*/
/*	引数:	Work *work   						       	*/
/*	説明:	再生時に狙いをのっとる			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void harDamageaction(Work *work){
 


    if (work->dmg_mode == HAR_DAMAGE_MOD_NEAR){
	SVECTOR tmp ={0, 0, 0, 0};
	switch (work->damage_step){
	case 0:
	    if (RAND(2)){
		work->damage_rotvz = -350;
	    }
	    else {
		work->damage_rotvz = 350;
	    }
	    work->damage_rotvx = RAND(101)-50;
	    work->damage_rotvy = RAND(101)-50;

	    if (HAR_MODE_L1 == H_MOD_L1_BURNING){	// *******排気炎焼き
		work->damage_downy = 30;		//チョイUP
	    }
	    if (HAR_MODE_L1 == H_MOD_L1_MISSILEPOD){	// *******ミサイルポッド
		work->damage_downy = 40;
	    }
	    else {
		work->damage_downy = 200;
	    }
	    work->damage_step++;
	    break;
	case 1:
	    tmp.vx = work->damage_rotvx;
	    tmp.vy = work->damage_rotvy;
	    tmp.vz = work->damage_rotvz;
	    work->damage_downy = work->damage_downy * 96/100;	//ダメージ挙動の時の上昇/下降
	    if (work->damage_rotvz==0){
		work->damage_step++;
	    }
	    break;
	case 2:
	    work->damage_rotvx = 0;
	    work->damage_rotvy = 0;
	    work->damage_rotvz = 0;
	    work->damage_downy = 0;
	    work->dmg_mtk_count = 0;
	    work->damage_step = 0;
	    work->dmg_mode = HAR_NO_DAMAGE_MOD;
	    break;
	}
	work->control.mov.vy += (RAND(151)*1.0f - 70.0f)+work->damage_downy*1.0f;	// ****上下の激しい振動
	work->damage_rotvx = work->damage_rotvx * 37/40;
	work->damage_rotvy = work->damage_rotvy * 37/40;
	work->damage_rotvz = work->damage_rotvz * 37/40;
	DG_RotatePos(&tmp);
    }


    // ***************ダメージ挙動
    if ((work->dmg_mode == HAR_DAMAGE_MOD_MPODR)||(work->dmg_mode == HAR_DAMAGE_MOD_MPODL)){
	SVECTOR tmp ={0, 0, 0, 0};
	switch (work->damage_step){
	case 0:
	    if (work->dmg_mode == HAR_DAMAGE_MOD_MPODR){
		work->damage_rotvz = -500;
	    }
	    else if (work->dmg_mode == HAR_DAMAGE_MOD_MPODL){
		work->damage_rotvz = 500;
	    }
	    work->damage_downy = -300;
	    work->damage_step++;
	    break;
	case 1:
	    tmp.vz = work->damage_rotvz;
	    work->damage_downy = work->damage_downy * 96/100;
	    if (work->damage_rotvz==0){
		work->damage_step++;
	    }
	    break;
	case 2:
	    work->damage_downy = 0;
	    work->dmg_mtk_count = 0;
	    work->damage_step = 0;
	    work->dmg_mode = HAR_NO_DAMAGE_MOD;
	    break;
	}
	work->control.mov.vy += (RAND(200)*1.0f - 100.0f)+work->damage_downy*1.0f;	// ****上下の激しい振動
	work->damage_rotvz = work->damage_rotvz * 18/20;
	DG_RotatePos(&tmp);
    }


    // ***************ダメージ挙動
    if (work->dmg_mode == HAR_DAMAGE_MOD_FAR){
	SVECTOR tmp ={0, 0, 0, 0};
	
	// ****上下の激しい振動
	work->control.mov.vy += (RAND(400)*1.0f - 200.0f);
	
	switch (work->damage_step){
	case 0:
	{
	    short	tmp;
	    work->damage_rotvz = -512;
	    work->damage_rotvz=0;
	    work->control.mov.vy += (RAND(500)*1.0f + 500.0f);
	    work->control.mov.vx += (RAND(1400)*1.0f - 700.0f);
	    
	    tmp = ( (RAND(120)+80) * (1 -(2*RAND(2))) );
	    work->control.rot.vy += tmp;
	    
	    tmp = (RAND(60) + 60);
	    work->control.rot.vx -= tmp;
	    
	    work->damage_step++;
	}
	break;
	
	case 1:
	    tmp.vz = work->damage_rotvz + 512;
	    if (work->damage_rotvz<10){
		work->damage_rotvz = 768;
		work->damage_step++;
	    }
	    break;
	    
	case 2:
	    tmp.vz = work->damage_rotvz -256;
	    if (work->damage_rotvz<10){
		work->damage_rotvz = -256;
		work->damage_step++;
	    }
	    break;

	case 3:
	    tmp.vz = work->damage_rotvz;
	    if (work->damage_rotvz==0){
		work->damage_step++;
	    }
	    break;

	case 4:
	    work->dmg_mtk_count = 0;
	    work->damage_step = 0;
	    work->dmg_mode = HAR_NO_DAMAGE_MOD;
	    break;
	}
	
	work->damage_rotvz = work->damage_rotvz * 17 / 20;
	DG_RotatePos(&tmp);
	
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Har_damage_tex						*/
/*	引数:	Work	*work							*/
/*	説明:							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Har_damage_tex(Work *work, int num)
{
    int	tex_id[KOWARE_POINT_NUM] = {
	GV_StrCode("hri2_twing_s1"),
	GV_StrCode("hri2_twing_t1"),
	GV_StrCode("hri2_mwing_t1_r"),
	GV_StrCode("hri2_piron1_s_r"),
	GV_StrCode("hri2_mwing_t1"),
	GV_StrCode("hri2_piron1_s"),
	GV_StrCode("hri2_body_s1_r"),
	GV_StrCode("hri2_body_s1"),
	GV_StrCode("hri2_body_duct_s1_r"),
	GV_StrCode("hri2_body_duct_s1"),
	GV_StrCode("hri2_fan"),
	GV_StrCode("hri2_face_s1"),
    };

    int	brk_tex_id[KOWARE_POINT_NUM] = {
	GV_StrCode("hri2_twing_s1_brk"),
	GV_StrCode("hri2_twing_t1_brk"),
	GV_StrCode("hri2_mwing_t1_r_brk"),
	GV_StrCode("hri2_piron1_s_r_brk"),
	GV_StrCode("hri2_mwing_t1_brk"),
	GV_StrCode("hri2_piron1_s_brk"),
	GV_StrCode("hri2_body_s1_r_brk"),
	GV_StrCode("hri2_body_s1_brk"),
	GV_StrCode("hri2_body_duct_s1_r_brk"),
	GV_StrCode("hri2_body_duct_s1_brk"),
	GV_StrCode("hri2_fan_brk"),
	GV_StrCode("hri2_face_s1_brk"),
    };

    printf ("Request break is %d\n", num);

    if (work->koware_p[num] != NULL){	//既に壊れ済み
	int	loop = 0;

	while (work->koware_p[loop] != NULL){
	    loop++;
	    if (loop == KOWARE_POINT_NUM){	//壊れてないところ無いや
		printf ("But I don't have nonbreak point\n");


		{
		int loop2;
		for (loop2=0; loop2<KOWARE_POINT_NUM; loop2++){
		    if (work->koware_p[loop2] != NULL){
			DG_ResetMoveReplaceTexture( work->koware_p[loop2] );
			DG_FreeMoveReplacePacket( work->koware_p[loop2] );
			work->koware_p[loop2] = NULL;
		    }
		}
		}




		return;
	    }
	}
	num = loop;
	printf ("But I'll break %d\n", num);
    }


    work->koware_p[num] =
	DG_MakeMoveReplacePacket(GV_StrCode("har01"),
				 tex_id[num],
				 brk_tex_id[num]);
    DG_SetMoveReplaceTexture(work->koware_p[num]);

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Clean_damage_tex_set					*/
/*	引数:	Work *work   						       	*/
/*	説明:	ダメージの壊れテクスチャの後始末	       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Clean_damage_tex_set(Work *work){
    int	loop = 0;

    for (loop=0; loop<KOWARE_POINT_NUM; loop++){
	if (work->koware_p[loop] != NULL){
	    DG_ResetMoveReplaceTexture( work->koware_p[loop] );
	    DG_FreeMoveReplacePacket( work->koware_p[loop] );
	    work->koware_p[loop] = NULL;
	}
    }
}

#define	IS_TARGHIT(_num) (work->damaged_point & ((0x01)<<(_num)))

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void choose_damage_tex						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ダメージテクスチャの選出		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void choose_damage_tex(Work *work){

    if ( HAR_Canp_Break == OFF)
    if IS_TARGHIT(19){
	HAR_Canp_Break = ON;
	printf ("キャノピーひび\n");
    }

    //		鼻ヒット
    if (work->koware_p[11] == NULL)
    if IS_TARGHIT(20){
	Har_damage_tex(work, 11);
	printf ("鼻壊れ\n");
	return;
    }

    //		垂直尾翼
    if (work->koware_p[0] == NULL)
    if IS_TARGHIT(16){
	Har_damage_tex(work, 0);
	printf ("垂直尾翼壊れ\n");
	return;
    }

    //		水平尾翼
    if (work->koware_p[1] == NULL)
    if (IS_TARGHIT(14)||IS_TARGHIT(15)){
	Har_damage_tex(work, 1);
	printf ("水平尾翼壊れ\n");
	return;
    }

    //		右翼
    if (work->koware_p[2] == NULL)
    if (IS_TARGHIT(22)&&IS_TARGHIT(23)&&((!IS_TARGHIT(10))||(!IS_TARGHIT(6)))){
	Har_damage_tex(work, 2);
	Har_damage_tex(work, 3);
	printf ("右翼壊れ\n");
	return;
    }

    //		左翼
    if (work->koware_p[4] == NULL)
    if (IS_TARGHIT(24)&&IS_TARGHIT(25)&&((!IS_TARGHIT(9))||(!IS_TARGHIT(6)))){
	Har_damage_tex(work, 4);
	Har_damage_tex(work, 5);
	printf ("左翼壊れ\n");
	return;
    }

    //		右胴体
    if (work->koware_p[6] == NULL)
    if (IS_TARGHIT(22)&&(!IS_TARGHIT(23))&&(!IS_TARGHIT(5))){
	Har_damage_tex(work, 6);
	printf ("右胴体壊れ\n");
	return;
    }

    //		左胴体
    if (work->koware_p[7] == NULL)
    if (IS_TARGHIT(24)&&(!IS_TARGHIT(25))&&(!IS_TARGHIT(5))){
	Har_damage_tex(work, 7);
	printf ("左胴体壊れ\n");
	return;
    }



    //		エアインテーク左側
    if ((work->koware_p[9] == NULL)&&
	IS_TARGHIT(7)&&(IS_TARGHIT(8)||IS_TARGHIT(9))){
	Har_damage_tex(work, 9);
	printf ("左エアインテーク壊れ\n");
    }
    //		エアインテーク右側
    else if ((work->koware_p[9] == NULL)&&
	     IS_TARGHIT(7)&&(IS_TARGHIT(8)||IS_TARGHIT(10))){
	Har_damage_tex(work, 8);
	printf ("右エアインテーク壊れ\n");
    }

    //		エアインテーク両側壊れで ファンも壊れる
    if (work->koware_p[10] == NULL)
    if ( (work->koware_p[8] != NULL)&&(work->koware_p[9] != NULL) ){
	Har_damage_tex(work, 10);
	printf ("エアインテークファン壊れ\n");
    }
}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void check_damages						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ダメージ関連の処理/チェック		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void check_damages(Work *work){


    //  ********************ミサイルポッドダメージ表現
    if (work->mispod_dmg_flg & POD_RIGHT_DAMAGE){
	work->mispod_dmg_flg &= ~POD_RIGHT_DAMAGE;
	work->rpod_bomb_timer = 120*5;	
	printf ("Set rpod timer\n");
    }

    if (work->mispod_dmg_flg & POD_LEFT_DAMAGE){
	work->mispod_dmg_flg &= ~POD_LEFT_DAMAGE;
	work->lpod_bomb_timer = 120*5;
	printf ("Set lpod timer\n");
    }

    //  ********************ミサイルポッド小爆発
    if (work->rpod_bomb_timer>0){
	work->rpod_bomb_timer -= TIME_BASE;
	if (work->rpod_bomb_timer <= 0){
	    Mpod_Work* mpwork = (Mpod_Work*)GET_MPOD_WORK_R;
	    NewMissileExplosion((FVECTOR*)mpwork->mpod_body.objs->world.m[3], 500.0f);
	    NewCrashDebris((FVECTOR*)mpwork->mpod_body.objs->world.m[3]);

	    work->rpod_bomb_timer = 0;
	    SET_DAMAGE_MODE(HAR_DAMAGE_MOD_MPODR);

	    if (work->dam_mpsmk_flg[0] == ON){	// 煙発生
		work->dam_mpsmk_flg[0] = OFF;
		NewRisingSmoke2 ((FVECTOR*)mpwork->mpod_body.objs->world.m[3], 400.0f, &work->dam_mpsmk_flg[0]);
		((Mpod_Work*)GET_MPOD_WORK_R)->break_flag = ON;
	    }
	}
    }
    if (work->lpod_bomb_timer>0){
	work->lpod_bomb_timer -= TIME_BASE;
	if (work->lpod_bomb_timer <= 0){
	    Mpod_Work* mpwork = (Mpod_Work*)GET_MPOD_WORK_L;
	    NewMissileExplosion((FVECTOR*)mpwork->mpod_body.objs->world.m[3], 500.0f);
	    NewCrashDebris((FVECTOR*)mpwork->mpod_body.objs->world.m[3]);

	    work->lpod_bomb_timer = 0;
	    SET_DAMAGE_MODE(HAR_DAMAGE_MOD_MPODL);

	    if (work->dam_mpsmk_flg[1] == ON){	// 煙発生
		work->dam_mpsmk_flg[1] = OFF;
		NewRisingSmoke2 ((FVECTOR*)mpwork->mpod_body.objs->world.m[3], 400.0f, &work->dam_mpsmk_flg[1]);
		((Mpod_Work*)GET_MPOD_WORK_L)->break_flag = ON;
	    }
	}
    }

    
#ifdef DEBUG_MODE
    if (HAR_TGT_VIEW == ON){
	static int num = 0;
	if (GV_PadDataDirect[0].press & PAD_A){
	    Har_damage_tex(work, num);
	    if (num < KOWARE_POINT_NUM-1){
		num ++;
	    }
	    else {
		num = 0;
	    }
	}
    }
#endif
    
    if (work->har_damage > 0){

	GM_SeSetMode (SD_E_H_DMGB03, &work->control.mov, GM_SEMODE_BOMB);

	choose_damage_tex(work);		// ****ダメージテクスチャを選んで貼る

	work->dmg_mtk_count = MUTEKI_TIME;	//無敵時間

	if ( (HAR_MODE_L1==H_MOD_L1_AMRAM) ||
	     (HAR_MODE_L1==H_MOD_L1_PROGRAM_MOVE) ||
	     (HAR_MODE_L1==H_MOD_L1_ACROBAT_MOVE) ||
	     (HAR_MODE_L1==H_MOD_L1_CLASTER) ||
	     (HAR_MODE_L1==H_MOD_L1_AMRAM) ){
	    SET_DAMAGE_MODE(HAR_DAMAGE_MOD_FAR);	//ダメージの動きの種類
	}
	else {
	    SET_DAMAGE_MODE(HAR_DAMAGE_MOD_NEAR);	//ダメージの動きの種類
	}

#ifdef DEBUG_MODE
	if (HAR_Muteki == ON){
	    work->damage_num = 0;	//ダメージ回数をリセット
	}
	else
#endif
	{
	    printf ("Harrier Damage %d - %d\n",work->gageset.value, work->har_damage);

	    // ===============クラスター前に死亡 に 蓋
	    if ( (work->har_damage>=work->gageset.value)&&(work->claster_num == 0) ){
		work->har_damage = work->gageset.value/2;
		if (work->gageset.value == 1){
		    work->har_damage = 0;
		}
	    }

	    work->gageset.value = work->gageset.value - work->har_damage;
	    work->damage_num += 1;	//ダメージ回数を保存
	}
	work->har_damage = 0;
	
	
	{	// ************ダメージ煙発生
	    short loop;
	    for (loop=0; loop<5; loop++){
		
		if( (work->gageset.value < work->gageset.max/(loop+2))&&
		    (work->dam_smk_flg[loop] == ON) )
		{	
		    work->dam_smk_flg[loop] = OFF;
		    NewRisingSmoke2 (&work->dam_smk_pos[loop], 1000.0f, &work->dam_smk_flg[loop]);
		}
	    }
	}
    }

    //  **無敵のカウントダウン
    if (work->dmg_mtk_count > 0){
	work->dmg_mtk_count--;
    }
    if (work->gageset.value <= 0){	//ゲームオーバー
    //    if (1){	//ゲームオーバー

	work->gageset.value = 0;

	if (work->GAME_OVER_FLAG == OFF){
	    GM_SeSet( GM_PAN_CENTER, 0x3f, SD_V_SOLOUT01);
	    printf ("Harrier out\n");
	    if (har_gameover_check_ok()){
		if ( work->die_proc_id != 0 ){
		    GCL_ARGS	args;
		    args.argc = 1;
		    args.argv = &work->name;
		    GCL_ExecProc(work->die_proc_id, &args);

		    GM_MecaKillCount ++;
		    if ( GM_MecaKillCount > GM_MAX_RESULT_COUNT ) GM_MecaKillCount = GM_MAX_RESULT_COUNT ;

		    work->GAME_OVER_FLAG = ON;
		    printf ("					Die proc Call. ID = %d\n", work->name);
		    return;
		}
	    }
	}
	return;
    }
}



#if 0
void asobi_clear(Work *work, short *ov_count, short *hok_count){


    MENU_Locate( 220, 220, 0 ) ;
    MENU_SetColor( 200, 0, 0 ) ;
    MENU_Printf( "CLEAR");
    work->gageset.value = 0;
    
    if (*hok_count==180){
	Har_SetHokanMove(work, &work->body.objs->world, 180);
    }
    
    if (*hok_count>0){
	FMATRIX tmpmat;
	FVECTOR tmppos = {0.0f, 30000.0f, -W25A_STAGE_Z_SHIFT, 1.0f};
	SVECTOR tmprot = {500, 0, 0, 0};
	DG_SetPos2 (&tmppos, &tmprot);
	DG_GetPos (&tmpmat);
	Har_HokanMove( &work->body.objs->world, work, &tmpmat, *hok_count);
	*hok_count=*hok_count-1;
    }
    else {
	if (*ov_count==0){
	    work->control.mov.vx = 0.0f;
	    work->control.mov.vy = 30000.0f;
	    work->control.mov.vz = - W25A_STAGE_Z_SHIFT;
	}
	
	if (work->control.mov.vy > 1500.0f){
	    work->control.mov.vy -= 600.0f;
	    
	}
	
	if (*ov_count < 60){
	    work->control.mov.vy += RAND(301)*1.0f -150.0f;
	}
	
	if (*ov_count < 120){
	    work->control.turn.vz += (120-*ov_count)*3;
	    work->control.rot.vz = work->control.turn.vz;
	}
	if (*ov_count < 240){
	    if (*ov_count % 30==0){
		FVECTOR		normal={0.0f, 1.0f, 0.0f, 0.0f};
		FVECTOR		direction={0.0f, 0.0f, -1.0f, 0.0f};
		GM_SeSetMode (SD_W_EXPLOS02, &work->control.mov, GM_SEMODE_BOMB);
		NewHarrierMisileExplosionE3(
		    &work->control.mov,
		    &normal,
		    &direction );
	    }
	    *ov_count=*ov_count+1;
	}
	else {

	    if ( work->die_proc_id != 0 ){
		GCL_ARGS	args;
		args.argc = 1;
		args.argv = &work->name;
		GCL_ExecProc(work->die_proc_id, &args);
		printf ("					Die proc Call. ID = %d\n", work->name);
	    }

	}

	work->control.turn.vx = 500;
    }
    har_set_move_object(work); //ダメージ挙動
    directAiming(work);
    Har_MoveParts(work);
    Har_SeEffect(work);
    Har_MoveTarget(work);
    SetObjs(work);
    SET_GUN_MODE(GUN_FIRE_FLG_NOFIRE, GUN_FIRE_FLG_NOHIT);
    harclearpaddata(&work->act_work); //パッドワークくりあ
    MACRO_HARRIER_STOP;
    
    
    set_targnzl_lock(work, ON);	//ノズルロック不可能
    set_hartarg_skip(work, OFF);
    set_targbodyall_skip(work, ON);
}
#endif










