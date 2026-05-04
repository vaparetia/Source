/********************************************************************************/
/*	har_claster.c								*/
/*	ハリアのクラスター爆弾							*/
/*	2001/01/23 H.Satoyoshi							*/
/*	$Id: har_claster.c,v 1.1.1.3 2002/11/19 11:48:20 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:	終了処理:ワーク解放    						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Cl_Die(Cls_Work *work)
{
    GM_FreeObject(&(work->body));
    if (work->clst_warhead != NULL){
	DG_DequeueComdlObjs( work->clst_warhead );
	DG_FreeComdl( work->clst_warhead );
	work->clst_warhead = NULL;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Get_clst_pos						*/
/*	引数:	FVECTOR	*shift	位置ベクトル    				*/
/*		short	number	位置番号	    				*/
/*	説明:	クラスター爆弾の位置を取得					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Get_clst_pos(FVECTOR *shift, short number)
{
    switch(number){
    case 0:
	shift->vx = 0.0f;
	shift->vy = 90.0f;
	shift->vz = 0.0f;
	break;
    case 1:
	shift->vx = 43.0f;
	shift->vy = 80.0f;
	shift->vz = 0.0f;
	break;
    case 2:
	shift->vx = 80.0f;
	shift->vy = 43.0f;
	shift->vz = 0.0f;
	break;
    case 3:
	shift->vx = 90.0f;
	shift->vy = 0.0f;
	shift->vz = 0.0f;
	break;
    case 4:
	shift->vx = 80.0f;
	shift->vy = -43.0f;
	shift->vz = 0.0f;
	break;
    case 5:
	shift->vx = 43.0f;
	shift->vy = -80.0f;
	shift->vz = 0.0f;
	break;
    case 6:
	shift->vx = 0.0f;
	shift->vy = -90.0f;
	shift->vz = 0.0f;
	break;
    case 7:
	shift->vx = -43.0f;
	shift->vy = -80.0f;
	shift->vz = 0.0f;
	break;
    case 8:
	shift->vx = -80.0f;
	shift->vy = -43.0f;
	shift->vz = 0.0f;
	break;
    case 9:
	shift->vx = -90.0f;
	shift->vy = 0.0f;
	shift->vz = 0.0f;
	break;
    case 10:
	shift->vx = -80.0f;
	shift->vy = 43.0f;
	shift->vz = 0.0f;
	break;
    case 11:
	shift->vx = -43.0f;
	shift->vy = 80.0f;
	shift->vz = 0.0f;
	break;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void bunretu_claster						*/
/*	引数:	Cls_Work	*work						*/
/*	説明:	子爆弾飛び散り計算						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void bunretu_claster(Cls_Work *work, DG_COMDL_POS *pos_s, short loop)
{
    FVECTOR	shift;
    OBJECT		*tmp;
    tmp = &work->body;

		pos_s->color.vw = 128;
		Get_clst_pos(&shift, loop%12);
		shift.vz += (loop/12)*140 -140.0f*CLST_BOMB_NUM/24;

		work->c_bomb_speed[loop].vx = 0.0f;
		work->c_bomb_speed[loop].vy = 200.0f;
		work->c_bomb_speed[loop].vz = 0.0f;

		//何処を中心にして小爆弾が散らばるか 将来的には定数
		work->c_bomb_speed[loop].vz += (loop/12)*130 -130.0f*CLST_BOMB_NUM/24;

		SAT_Minus_FVECTOR(&work->c_bomb_speed[loop], &shift, &work->c_bomb_speed[loop]); 
		work->c_bomb_speed[loop].vy = work->c_bomb_speed[loop].vy*0.4f; 

		//前後に引き延ばし
		//work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*1.2f;	2001/7/7


		GV_LenVec3F( &work->c_bomb_speed[loop], &work->c_bomb_speed[loop], 0.0F, 30.0f);

		//work->c_bomb_speed[loop].vx = work->c_bomb_speed[loop].vx*2.8f; 	2001/7/7
		work->c_bomb_speed[loop].vx = work->c_bomb_speed[loop].vx*1.1f;


		//work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*6.0f;	2001/7/7
		work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*5.0f;

		//		work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*4.0f; 

		//内側によせる
		if (work->shift_pos.vx > 0.0f){
		    //work->c_bomb_speed[loop].vx -= 8.0f;
		    work->c_bomb_speed[loop].vx -= 9.0f;
		}
		else {
		    //work->c_bomb_speed[loop].vx += 8.0f;
		    work->c_bomb_speed[loop].vx += 9.0f;
		}


		//乱数
		{
		    //		    work->c_bomb_speed[loop].vx += RAND(61)*0.1f-3.0f;
		    //		    work->c_bomb_speed[loop].vy += RAND(601)*0.1f-45.0f;
		    //		    work->c_bomb_speed[loop].vz += RAND(61)*0.1f-3.0f;

		    work->c_bomb_speed[loop].vx += RAND(201)*0.1f-10.0f;
		    work->c_bomb_speed[loop].vy += RAND(601)*0.1f-45.0f;
		    work->c_bomb_speed[loop].vz += RAND(141)*0.1f-7.0f;
		}
		{
		    FMATRIX	tmpmat;
		    _sceVu0CopyMatrix(&tmpmat, &tmp->objs->world);

		    tmpmat.m[ 3 ][ 0 ] = 0.0f;
		    tmpmat.m[ 3 ][ 1 ] = 0.0f;
		    tmpmat.m[ 3 ][ 2 ] = 0.0f;

		    DG_SetPos(&tmpmat);
		    work->c_bomb_speed[loop].vw = 1.0f;
		    DG_MovePos(&work->c_bomb_speed[loop]);
		    DG_GetPos(&tmpmat);
		    Fvec_from_Matrix( &work->c_bomb_speed[loop], &tmpmat );
		}

      if ( BP_IsPAL()==TRUE )
      {
		   work->c_bomb_speed[loop].vx = work->c_bomb_speed[loop].vx * 1.2f;
		   work->c_bomb_speed[loop].vy = work->c_bomb_speed[loop].vy * 1.2f;
		   work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz * 1.2f;
      }

}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void cl_act_equip							*/
/*	引数:	Work	*work							*/
/*	説明:	装備状態							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cl_act_equip(Cls_Work *work)
{
    short	loop;
    OBJECT	*tmp;

    tmp = ((Work*)(work->oya_work))->control.object;

    DG_SetPos(&tmp->objs->world);
    DG_MovePos( &work->shift_pos);
    DG_GetPos(&work->body.objs->world);

    for (loop=0; loop<10; loop++){
	DG_SetPos(&work->body.objs->world);
	DG_MovePos(&work->body.objs->objs[loop].trans);
	DG_GetPos(&work->body.objs->objs[loop].world);
    }



    _sceVu0SubVector(&work->shell_speed,
		     &((Work*)(work->oya_work))->control.mov, &work->oldpos);	//速度の保存

    work->clst_warhead->flag |= DG_COMDL_INVISIBLE;

    work->oldpos = ((Work*)(work->oya_work))->control.mov;
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mini_bomb_in_air						*/
/*	引数:	Work	*work							*/
/*	説明:	空中小爆発							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mini_bomb_in_air(Cls_Work *work)
{
    FVECTOR	tmpvec;
    Fvec_from_Matrix( &tmpvec, &work->body.objs->world);

    work->shell_speed.vy -= 100.0f;
    GM_SeSetMode (SD_E_CLMPAN04, &tmpvec, GM_SEMODE_BOMB);	//SE分裂
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_droped							*/
/*	引数:	Work	*work							*/
/*	説明:	発射！！！！！							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void act_droped(Cls_Work *work)
{
    short	loop;
    //    SVECTOR	tmprot = {5, 0, 100, 0};
    SVECTOR	tmprot = {4, 0, 0, 0};

    DG_SetPos(&work->body.objs->world);
    DG_RotatePos(&tmprot);
    DG_GetPos(&work->body.objs->world);

    work->body.objs->world.m[ 3 ][ 0 ]+=work->shell_speed.vx;
    work->body.objs->world.m[ 3 ][ 1 ]+=work->shell_speed.vy-100.0f;
    work->body.objs->world.m[ 3 ][ 2 ]+=work->shell_speed.vz;

    work->shell_speed.vx = work->shell_speed.vx * 0.98f;
    work->shell_speed.vz = work->shell_speed.vz * 0.98f;

    for (loop=0; loop<10; loop++){
	DG_SetPos(&work->body.objs->world);
	DG_MovePos(&work->body.objs->objs[loop].trans);
	DG_GetPos(&work->body.objs->objs[loop].world);
    }

    if (work->timer_parent == 0){
	FVECTOR	tmpvec;
	Fvec_from_Matrix( &tmpvec, &work->body.objs->world);//終了点を保存 
	GM_SeSetMode (SD_E_CLMOPN03, &tmpvec, GM_SEMODE_BOMB);//SE爆弾投下開始	    
	GM_SeSetMode (SD_E_CLMFAL04, &tmpvec, GM_SEMODE_BOMB);//SE爆弾投下	    
    }
    work->timer_parent += TIME_BASE;

    if (work->timer_parent > CLST_BUNRETU_TIME*5){
	work->timer_parent = 0;
	work->mis_mode = CLASTER_FIRED;

	mini_bomb_in_air(work);			//空中小爆発エフェクト
	work->clst_warhead->flag &= ~DG_COMDL_INVISIBLE;
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_fired							*/
/*	引数:	Work	*work							*/
/*	説明:	ケース粉々							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void act_fired(Cls_Work *work)
{
    short	loop;
    work->drp_sp.vy -= CLASTER_GRAVITY;

    work->body.objs->world.m[ 3 ][ 0 ]+=work->drp_sp.vx+work->shell_speed.vx;
    work->body.objs->world.m[ 3 ][ 1 ]+=work->drp_sp.vy;
    work->body.objs->world.m[ 3 ][ 2 ]+=work->drp_sp.vz+work->shell_speed.vz;

    for (loop=0; loop<10; loop++){
	if (work->timer_child == 0){
	    //仮に原点として使用
	    work->c_shell_speed[loop].vx = 0.0f;
	    work->c_shell_speed[loop].vy = 0.0f;
	    work->c_shell_speed[loop].vz = 0.0f;
	    
	    //放射状に飛び散る
	    SAT_Minus_FVECTOR(&work->c_shell_speed[loop], &work->body.objs->objs[loop].trans, &work->c_shell_speed[loop]);
	    work->c_shell_speed[loop].vz = work->c_shell_speed[loop].vz*0.3f; 
	    //速度制限
	    GV_LenVec3F( &work->c_shell_speed[loop], &work->c_shell_speed[loop], 0.0F, RAND(1500)*0.1f+70.0f);

	    //ランダムに回転
	    work->c_shell_rot[loop].vx = RAND(5)+5;
	    work->c_shell_rot[loop].vy = RAND(25)+25;
	    work->c_shell_rot[loop].vz = RAND(50)+50;
	    if (RAND(2)){
		work->c_shell_rot[loop].vx *= -1;
	    }
	    if (RAND(2)){
		work->c_shell_rot[loop].vy *= -1;
	    }
	    if (RAND(2)){
		work->c_shell_rot[loop].vz *= -1;
	    }
	    //乱数
	    {
		work->c_shell_speed[loop].vx += RAND(601)*0.1f-30.0f;
		work->c_shell_speed[loop].vy += RAND(601)*0.1f-30.0f;
		work->c_shell_speed[loop].vz += RAND(601)*0.1f-30.0f;
	    }
	}

	//減速＆落下
	work->c_shell_speed[loop].vx = work->c_shell_speed[loop].vx * 0.9999f;
	work->c_shell_speed[loop].vy -= CLASTER_GRAVITY;
	work->c_shell_speed[loop].vz = work->c_shell_speed[loop].vz * 0.9999f;

	DG_SetPos(&work->body.objs->objs[loop].world);
	DG_RotatePos(&work->c_shell_rot[loop]);	//ケース弾頭回転
	DG_GetPos(&work->body.objs->objs[loop].world);


	work->body.objs->objs[loop].world.m[3][0] += work->c_shell_speed[loop].vx+work->shell_speed.vx;
	work->body.objs->objs[loop].world.m[3][1] += work->c_shell_speed[loop].vy;
	work->body.objs->objs[loop].world.m[3][2] += work->c_shell_speed[loop].vz+work->shell_speed.vz;

    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void cl_act_none						*/
/*	引数:	Work	*work							*/
/*	説明:	待機								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cl_act_none(Cls_Work *work)
{
    work->clst_warhead->flag |= DG_COMDL_INVISIBLE;

#ifdef DEBUG_MODE
    if (HAR_DbgMode != 1)
#endif
    {
	GM_FreeObject(&(work->body));
	if (work->clst_warhead != NULL){
	    DG_DequeueComdlObjs( work->clst_warhead );
	    DG_FreeComdl( work->clst_warhead );
	    work->clst_warhead = NULL;
	    printf ("クラスター爆弾のメモリ解放！！\n");
	}
    }
#ifdef DEBUG_MODE
    else {
    }
#endif

}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_none							*/
/*	引数:	Work	*work							*/
/*	説明:	待機								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cls_fired(Cls_Work *work)
{
    DG_COMDL_POS	*pos_s;
    short		loop;
    FVECTOR		from_vec, to_vec, shift;

    //	**********  一個一個のクラスター爆弾の処理  **********
    for (pos_s=work->clst_warhead->pos,loop=0; loop < CLST_BOMB_NUM; loop++, pos_s++){

	if (work->timer_child == 0){
	    if (((Work *)(work->oya_work))->har_flag & HAR_DOUSA_FLAG_BOSSMD){
		bunretu_claster(work, pos_s, loop);	//クラスター爆弾飛散
	    }
	    else {
		bunretu_claster(work, pos_s, loop);	//クラスター爆弾飛散E3
	    }
	    pos_s->color.vw = 128;
	}
	    
	work->c_bomb_speed[loop].vy -= CLASTER_GRAVITY;
	work->c_bomb_speed[loop].vx = work->c_bomb_speed[loop].vx*0.999f; 
	work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*0.999f; 

	_sceVu0CopyVector(&shift, &work->c_bomb_speed[loop]);

	DG_SetPos( &pos_s->world );
	Fvec_from_Matrix( &from_vec, &pos_s->world );	//開始点を保存 

	DG_GetPos( &pos_s->world );

	pos_s->world.m[ 3 ][ 0 ]+=shift.vx;
	pos_s->world.m[ 3 ][ 1 ]+=shift.vy;
	pos_s->world.m[ 3 ][ 2 ]+=shift.vz;


	pos_s->world.m[ 3 ][ 0 ] += work->shell_speed.vx;
	pos_s->world.m[ 3 ][ 2 ] += work->shell_speed.vz;

	Fvec_from_Matrix( &to_vec, &pos_s->world );	//終了点を保存 

	    
	if (work->timer_child > 10){	//既に発射済みで

	    if ( (to_vec.vy < 0.0f)&&
		 (-3000.0f < to_vec.vy)&&(to_vec.vy < 3000.0f)&&
		 ( -158000.0f < to_vec.vz)&&(to_vec.vz < -144000.0f) ) {

		if (work->hit_flag[loop] == CLASTER_NOT_YET ){	//爆発フラグが立っていなければ
		    work->hit_flag[loop] = CLASTER_BOMED;	//爆発フラグを立てる
		    pos_s->color.vw = 0;
		}
	    }


	    if (to_vec.vy < 5300.0f){
		if (work->hit_flag[loop] == CLASTER_NOT_YET ){	//爆発フラグが立っていなければ
		    if (to_vec.vy < 400.0f){
			if (RAND(15)==0){
			    work->hit_flag[loop] = CLASTER_BOMED;	//爆発フラグを立てる
			    pos_s->color.vw = 0;
			}
		    }
		    else if (to_vec.vy < 2000.0f){
			if (RAND( ((short)(to_vec.vy)/100-5) + 15 )==0){
			    work->hit_flag[loop] = CLASTER_BOMED;	//爆発フラグを立てる
			    pos_s->color.vw = 0;
			}
		    }
		    else if (to_vec.vy < 5300.0f){
			if (RAND(30)==0){
			    work->hit_flag[loop] = CLASTER_BOMED;	//爆発フラグを立てる
			    pos_s->color.vw = 0;
			}
		    }
		}
	    }
	}
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Move_claster						*/
/*	引数:	Cls_Work	*work						*/
/*	説明:	子爆弾の動き							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Move_claster(Cls_Work *work)
{
    OBJECT		*tmp;
    DG_COMDL_POS	*pos_s;
    short		loop;
    FVECTOR		from_vec, to_vec, shift;

    tmp = &work->body;
    shift.vw = 0.0f;

    // *****発射時の制御
    if (work->mis_mode == CLASTER_FIRED){
    }
    // *****装備時の制御
    else {
	//	**********  一個一個のクラスター爆弾の処理  **********
	for (pos_s=work->clst_warhead->pos,loop=0; loop < CLST_BOMB_NUM; loop++, pos_s++){
	    DG_SetPos( &tmp->objs->world);
	    Fvec_from_Matrix( &from_vec, &pos_s->world );	//開始点を保存 
	    Get_clst_pos(&shift, loop%12);
	    shift.vz += (loop/12)*140 -140.0f*CLST_BOMB_NUM/24;

	    // *****移動セット	    
	    DG_MovePos( &shift );
	    DG_GetPos( &pos_s->world );
	    Fvec_from_Matrix( &to_vec, &pos_s->world );	//終了点を保存 
	    pos_s->color.vw = 0;
	}
    }
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void do_Act							*/
/*	引数:	Cls_Work	*work						*/
/*	説明:	実行関数							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Cl_Act(Cls_Work *work)
{

    static int sd_timer = 0;

    if (work->clst_warhead == NULL){
	return;
    }

    switch (work->mis_mode){
    case CLASTER_EQUIP:	// *****まだ装備状態
	cl_act_equip(work);
	sd_timer = 0;
	break;
    case CLASTER_DROPED:// ******発射！！！！
	act_droped(work);
	work->drp_sp.vx = work->drp_sp.vz = 0.0f;
	work->drp_sp.vy = -100.0f;
	break;
    case CLASTER_FIRED:
	sd_timer += TIME_BASE;
	work->shell_speed.vx = work->shell_speed.vx * 0.98f;
	work->shell_speed.vz = work->shell_speed.vz * 0.98f;
	act_fired(work);	// *****ケース粉々
	cls_fired(work);	// *****クラスター発射

	if ((10000>sd_timer)&&(sd_timer > 40*5)){
	    sd_timer = 10000;
	    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_CLMFLY04);
    	}

	work->timer_child ++;
	if (work->timer_child > 300){
	    DG_COMDL_POS	*pos_s;
	    short		loop;
	    work->timer_child = 0;
	    work->mis_mode = CLASTER_NONE;
	    for (pos_s=work->clst_warhead->pos,loop=0; loop < CLST_BOMB_NUM; loop++, pos_s++){
		work->hit_flag[loop] = CLASTER_NOT_YET;		//爆発フラグを解除
	    }
	}
	break;
    case CLASTER_NONE:	// *****待機
	cl_act_none(work);
	break;
    }

    if (work->clst_warhead == NULL){
	return;
    }

    //クラスター爆弾を動かす
    Move_claster(work);


    {
	Work	*oya_work;
	oya_work = (Work*)work->oya_work;

	if (work->shift_pos.vx > 0.0f){
	    oya_work->r_cls_pos = *(FVECTOR*)work->body.objs->world.m[3];
	}
	else {
	    oya_work->l_cls_pos = *(FVECTOR*)work->body.objs->world.m[3];
	}
    }

    {	//  *****非表示親にひきずられます
	Work *pwork;
	pwork  = (Work*)GET_HAR_WORK;
	work->body.objs->flag |= pwork->body.objs->flag & DG_FLAG_INVISIBLE;
    }

}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int ReceiveSignal						*/
/*	引数:	Work	*work							*/
/*	    :	int	signal							*/
/*	    :	int	value							*/
/*	説明:	シグナルを受信する						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Cl_ReceiveSignal(void *work, int signal, int value){

    if (signal == GV_SIGNAL_KILL){
	((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
	GV_DestroyActor( work);
	return 1;
    }
    else if ( (signal & HAR_SIGNAL_CLASTER_MASK) == HAR_SIGNAL_CLASTER ){
	((Cls_Work*)work)->mis_mode = ((signal) & (~HAR_SIGNAL_CLASTER));
	((Cls_Work*)work)->timer_child = 0;
	return 1;
    }

    return 0;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int Cl_GetResources(Cls_Work *work, Work *oya_work, FVECTOR *shift_pos, int where)
{
    
    work->oya_work = oya_work;		//被装備オブジェクト登録
    _sceVu0CopyVector(&work->shift_pos, shift_pos);	//ミサイル位置保存
    
    work->map = where;
    
    // *****************オブジェクトの初期化
    GM_InitObject(&work->body,
		  GV_StrCode("hri_clst_math"  ),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION|DG_FLAG_FOGPARAM);
    DG_SetFogParamObjs (work->body.objs, FOG_NEAR, FOG_FAR);
    
    //バウンディングを書き換え
    work->body.objs->bound_min.vx = -9990000.0f;
    work->body.objs->bound_min.vy = -9990000.0f;
    work->body.objs->bound_min.vz = -9990000.0f;
    work->body.objs->bound_max.vx =  9990000.0f;
    work->body.objs->bound_max.vy =  9990000.0f;
    work->body.objs->bound_max.vz =  9990000.0f;
    
    {
	DG_DEF		*def;
	DG_COMDL_POS	*pos_s;
	short		loop;
	
	def = (DG_DEF*)GV_GetCache( GV_CacheID( GV_StrCode("hri_clst_warhead_cm"), 'k' ) );
	
#ifdef DEBUG_MODE
	if ( def == NULL ) {
	    printf( "moel not found in har_claster in har_claster\n" ) ;
	    ASSERT( 0 ) ;	    
	}
#endif
	
	work->clst_warhead = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS|DG_COMDL_NOFOG, CLST_BOMB_NUM, 0 );
	
#ifdef DEBUG_MODE
	if ( work->clst_warhead == NULL ) {
	    printf( "warning : model make objs failed\n") ;
	}
#endif
	DG_QueueComdlObjs( work->clst_warhead );
	
	GM_GroupObject(work->clst_warhead, where);
	
	pos_s = work->clst_warhead->pos;
	for (loop=0; loop < CLST_BOMB_NUM; loop++, pos_s++){
	    pos_s->world = oya_work->body.objs->world;
	    pos_s->color.vx = pos_s->color.vy = pos_s->color.vz = 128;
	    pos_s->color.vw = 128;
	}
    }
    
	
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	CLASTER_GRAVITY = 2.0f;
	break;
    case ST_LEV_EASY:
	CLASTER_GRAVITY = 3.0f;
	break;
    case ST_LEV_NORMAL:
	CLASTER_GRAVITY = 4.2f;
	break;

    case ST_LEV_UPNORM:
	CLASTER_GRAVITY = 5.7f;
	break;


    case GM_LEVEL_HARD:
	CLASTER_GRAVITY = 7.2f;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	CLASTER_GRAVITY = 36.0f;
	break;
    }

    if ( BP_IsPAL()==TRUE )
    {
      CLASTER_GRAVITY = CLASTER_GRAVITY * 1.2f;
    }
    
    GV_SetActorSignalFunc( work, Cl_ReceiveSignal );	//シグナル受信関数設定
    
    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarClaster						*/
/*	引数:	Work	*oya_work	親のワークへのポインタ			*/
/*		FVECTOR	*shift_pos	シフト量				*/
/*	説明:	ミサイルを表示							*/
/********************************************************************************/
void *NewHarClaster(Work *oya_work, FVECTOR *shift_pos, int where)
{
    Cls_Work *work ;
    work = (Cls_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Cls_Work ), 255 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Cl_Act,Cl_Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!Cl_GetResources(work, oya_work, shift_pos, where)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}


#if 0
/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void bunretu_claster						*/
/*	引数:	Cls_Work	*work						*/
/*	説明:	子爆弾飛び散り計算						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void bunretu_clasterE3(Cls_Work *work, DG_COMDL_POS *pos_s, short loop)
{
    FVECTOR	shift;
    OBJECT		*tmp;
    tmp = &work->body;

		pos_s->color.vw = 128;
		Get_clst_pos(&shift, loop%12);
		shift.vz += (loop/12)*140 -140.0f*CLST_BOMB_NUM/24;


		//E３分散
		shift.vx = shift.vx * 4.0f;
		shift.vz = shift.vz * 4.0f;
		shift.vy = shift.vy * 4.0f;
 


		work->c_bomb_speed[loop].vx = 0.0f;
		work->c_bomb_speed[loop].vy = 200.0f;
		work->c_bomb_speed[loop].vz = 0.0f;

		//何処を中心にして小爆弾が散らばるか 将来的には定数
		work->c_bomb_speed[loop].vz += (loop/12)*130 -130.0f*CLST_BOMB_NUM/24;



		//E３分散
		work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz * 4.0f;



		SAT_Minus_FVECTOR(&work->c_bomb_speed[loop], &shift, &work->c_bomb_speed[loop]); 
		work->c_bomb_speed[loop].vy = work->c_bomb_speed[loop].vy*0.9f; 

		//前後に引き延ばし
		work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*1.2f;
		GV_LenVec3F( &work->c_bomb_speed[loop], &work->c_bomb_speed[loop], 0.0F, 250.0f);


		work->c_bomb_speed[loop].vz = work->c_bomb_speed[loop].vz*2.0f; 

		//内側によせる
		if (work->shift_pos.vx > 0.0f){
		    work->c_bomb_speed[loop].vx -= 8.0f;
		}
		else {
		    work->c_bomb_speed[loop].vx += 8.0f;
		}


		//乱数
		{
		    work->c_bomb_speed[loop].vx += RAND(241)*0.1f-12.0f;
		    work->c_bomb_speed[loop].vy += RAND(2401)*0.1f-180.0f;
		    work->c_bomb_speed[loop].vz += RAND(241)*0.1f-12.0f;
		}
		{
		    FMATRIX	tmpmat;
		    _sceVu0CopyMatrix(&tmpmat, &tmp->objs->world);

		    tmpmat.m[ 3 ][ 0 ] = 0.0f;
		    tmpmat.m[ 3 ][ 1 ] = 0.0f;
		    tmpmat.m[ 3 ][ 2 ] = 0.0f;

		    DG_SetPos(&tmpmat);
		    work->c_bomb_speed[loop].vw = 1.0f;
		    DG_MovePos(&work->c_bomb_speed[loop]);
		    DG_GetPos(&tmpmat);
		    Fvec_from_Matrix( &work->c_bomb_speed[loop], &tmpmat );

		    work->c_bomb_speed[loop].vx += work->shell_speed.vx*0.8f;	//初速度を加算
		    work->c_bomb_speed[loop].vz += work->shell_speed.vz*0.8f;
		}
}
#endif
