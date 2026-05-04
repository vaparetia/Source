/********************************************************************************/
/*	har_missile.c								*/
/*	ハリアのミサイル							*/
/*	2001/01/25 H.Satoyoshi							*/
/*	$Id: har_missile.c,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

#define AMRAM_MAX_SPEED (2200.0f)

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:	終了処理:ワーク解放    						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Mis_Die(Mis_Work *work)
{
    GM_FreeObject(&(work->body));
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void MissileBomb						*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル爆発！							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void MissileBomb(Mis_Work *work){
    work->mis_mode = HAR_SIGNAL_NONE;	
    GM_SeSetMode (SD_E_AM_EXP04, &work->position, GM_SEMODE_BOMB);
    work->am_def_tgt.class |= TARGET_SKIP;
    work->body.objs->objs[0].flag |= DG_FLAG_INVISIBLE;
    work->mis_flag = 3;	//ミサイル追従エフェクト消去 爆発
    call_pad_vibration(VIB_AM_EXP);	//パッド振動
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mis_act_equip						*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル発射							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mis_act_equip(Mis_Work *work){
    OBJECT	*tmp;
    tmp = ((Work *)(work->oya_work))->control.object;
    
    //マトリクスの計算 libDGを使用
    DG_SetPos(&tmp->objs->world);
    DG_MovePos( &work->shift_pos);
    DG_MovePos(&work->body.objs->objs[0].trans);
    DG_GetPos(&work->body.objs->world);
    DG_GetPos(&work->body.objs->objs[0].world);
    work->body.objs->objs[0].flag &= ~DG_FLAG_INVISIBLE;
    work->mis_flag = 0;	//ミサイル追従エフェクト許可
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_fire							*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル発射							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void act_fire(Mis_Work *work){
    OBJECT	*tmp;
    tmp = ((Work *)work->oya_work)->control.object;
    
    //マトリクスの計算 libDGを使用
    DG_SetPos(&tmp->objs->world);
    DG_MovePos( &work->shift_pos);
    DG_MovePos(&work->body.objs->objs[0].trans);
    DG_GetPos(&work->body.objs->world);
    DG_GetPos(&work->body.objs->objs[0].world);    

    {
	FMATRIX	tmpmat;
	FVECTOR	tmpshift = {0.0f, 0.0f, 1000.0f, 1.0f};
	
	//場所を渡す
	Fvec_from_Matrix(&work->position, &work->body.objs->objs[0].world);
	
	//方向ベクトルを計算
	_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[0].world);
	tmpmat.m[ 3 ][ 0 ] = 0.0f;
	tmpmat.m[ 3 ][ 1 ] = 0.0f;
	tmpmat.m[ 3 ][ 2 ] = 0.0f;
	DG_SetPos(&tmpmat);
	DG_MovePos(&tmpshift);
	DG_GetPos(&tmpmat);
	Fvec_from_Matrix(&work->vector, &tmpmat);
	_sceVu0Normalize(&work->vector, &work->vector);
	
    }
    
    if (work->timer == 0){	//発射エフェクト
	int rgba0, rgba1;

	SetRGBA_CharR(((Work *)GET_HAR_WORK)->missile_burn_col2, &rgba0);
	SetRGBA_CharR(&((Work *)GET_HAR_WORK)->missile_burn_col2[4], &rgba1);

	work->mis_flag = 0;	// 点火
	NewHarrierMissileFire2(&work->body.objs->objs[0].world, &work->mis_flag, rgba0, rgba1);
	GM_SeSetMode (SD_E_MISFIRL5, &work->position, GM_SEMODE_BOMB);//SE
    }
    
    work->timer++;
    
    if (work->timer == 35){	//エフェクト
	work->timer = 0;
	work->mis_mode = HAR_SIGNAL_FLY;	
	work->mis_flag = 1;	// 切り離し
	{
	    FVECTOR	to_pos = {0.0f, 0.0f, 10000.0f, 0.0f};
	    FVECTOR	from_pos;
	    FMATRIX	tmpmat;
	    
	    Fvec_from_Matrix(&from_pos, &work->body.objs->objs[0].world);
	    DG_SetPos(&work->body.objs->objs[0].world);
	    DG_MovePos(&to_pos);
	    DG_GetPos(&tmpmat);
	    Fvec_from_Matrix(&to_pos, &tmpmat);
	    Dir_from_2Vec(&from_pos, &to_pos, &work->rotation);


	if(work->shift_pos.vx > 0.0f){
	    work->rotation.vx += 20;	
	    work->rotation.vy += 17;	
	}
	else {
	    work->rotation.vx += 20;	
	    work->rotation.vy -= 17;	
	}

	    GM_SeSetMode (SD_E_CLMOPN03, &work->position, GM_SEMODE_BOMB);//SE爆弾投下開始	    
	    work->am_def_tgt.class &= ~TARGET_SKIP;
	}    
    }
}


#define CHECK_ROT(_rot)		\
{				\
    if ((_rot) >= 2048){	\
       (_rot) -= 4096;		\
    }				\
    if ((_rot) < -2048){	\
      (_rot) += 4096;		\
    }				\
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void act_fly							*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル飛行							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void act_fly(Mis_Work *work){
    
    FVECTOR	tmpvec = {0.0f, 0.0f, 600.0f, 0.0f};
    FVECTOR	from_pos, to_pos;
    SVECTOR	tmprot;
    int		dist;

    Fvec_from_Matrix(&work->position, &work->body.objs->objs[0].world);
    _sceVu0CopyVector(&from_pos, &work->position);
    {
	FVECTOR	tmpvec;
	_sceVu0CopyVector(&tmpvec, &GM_PlayerFindPos);
	if(work->shift_pos.vx > 0.0f){
	    tmpvec.vz += 200.0f;
	    tmpvec.vx += 200.0f;
	    if( GM_CheckGameStatus(STATE_CHAFF) ){
		tmpvec.vy += 2500.0f;
		tmpvec.vz += 2000.0f;
	    }
	}
	else {
	    tmpvec.vz -= 200.0f;
	    tmpvec.vx -= 200.0f;
	    tmpvec.vy += 200.0f;
	    if( GM_CheckGameStatus(STATE_CHAFF) ){
		tmpvec.vy += 2500.0f;
		tmpvec.vz -= 2000.0f;
	    }
	}
	Dir_from_2Vec(&from_pos, &tmpvec, &tmprot);
	_sceVu0SubVector(&tmpvec, &from_pos, &tmpvec);

	dist = (int)_FVecLen3(&tmpvec);
    }

    
    
    {	//  ****************ＳＥ再生
	static	int	amtime = 0;
	static	int	alerttime = 0;

	amtime += TIME_BASE;
	if (amtime >= 60){
	    amtime = 0;
	    GM_SeSetMode (SD_E_AM_FLY01, &work->position, GM_SEMODE_BOMB);//SEミサイル飛翔音
	}

	//  ****************ＳＥ再生
	if (work->am_timer == 0){
	    if (dist < 150*AMRAM_MAX_SPEED/TIME_BASE){
		GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_AM_SWIN1);
		work->am_timer = 1;
	    }
	}

	
	alerttime += TIME_BASE;
	if (alerttime > dist/1500){
	    GM_SeSet(GM_PAN_CENTER, 0x3f, SD_E_AMALERT1);
	    alerttime = 0;
	}
    }



    //    printf("D:%d\n", dist);


    CHECK_ROT(tmprot.vx);
    CHECK_ROT(tmprot.vy);

    tmprot.vx = tmprot.vx - work->rotation.vx;
    tmprot.vy = tmprot.vy - work->rotation.vy;

    CHECK_ROT(tmprot.vx);
    CHECK_ROT(tmprot.vy);

    {
	float tmp;
	tmp =  bp_sqrtf( (tmprot.vx * tmprot.vx * 1.0f) +  //BP_MATH - emulate PS2 sqrtf
		      (tmprot.vy * tmprot.vy * 1.0f) ); 
	if (tmp != 0){
	    tmprot.vx = (short)(16 * tmprot.vx / (short)tmp);
	    tmprot.vy = (short)(16 * tmprot.vy / (short)tmp);
	}
    }

    if (work->timer == 40*5){	//加速
	work->mis_flag = 2;	// 加速
	GM_SeSetMode (SD_E_MISFIRL1, &work->position, GM_SEMODE_BOMB);//SE
    }

    if (work->timer > 40*5){	//加速
	//	tmpvec.vz = 95/0.0f;
	//	tmpvec.vz = 500.0f;

	if (dist >= 100000){
	    tmpvec.vz = AMRAM_MAX_SPEED;
	}
	else {
	    switch( GM_GameLevel ){
	    case GM_LEVEL_VERYEASY:
		tmpvec.vz = 16.0f;
		break;
	    case ST_LEV_EASY:
		tmpvec.vz = 18.0f;
		break;
	    case ST_LEV_NORMAL:
		tmpvec.vz = 20.0f;
		break;

	    case ST_LEV_UPNORM:
		tmpvec.vz = 22.0f;
		break;

	    case GM_LEVEL_HARD:
		tmpvec.vz = 25.0f;
		break;
	    case GM_LEVEL_E_EXTREME:
	    case GM_LEVEL_EXTREME:
		tmpvec.vz = 40.0f;
		break;
	    default:
		tmpvec.vz = 20.0f;
		break;
	    }
	}


	// *****追尾
	if (((Work *)(work->oya_work))->har_flag & HAR_DOUSA_FLAG_FOLLOW){
	    work->rotation.vx += tmprot.vx;
	    work->rotation.vy += tmprot.vy;
	}

	CHECK_ROT(work->rotation.vx);
	CHECK_ROT(work->rotation.vy);
    }
    
    {	//    ********************ハリアの速度加算
	tmpvec.vz += (work->oya_speed*0.5f);
    }

    DG_SetPos2(&work->position, &work->rotation);
    DG_MovePos(&tmpvec);
    DG_GetPos(&work->body.objs->world);
    DG_GetPos(&work->body.objs->objs[0].world);



    Fvec_from_Matrix(&to_pos, &work->body.objs->objs[0].world);

    GM_MoveOnlineTarget (&work->vul_target, &from_pos, &to_pos);
    GM_PutTarget(&work->vul_target);


    {
	FVECTOR	check_pos;
	FMATRIX	tmpmat;
	FVECTOR	shift = {0.0f, 0.0f, 1000.0f, 1.0f};

	DG_SetPos(&work->body.objs->objs[0].world);
	DG_MovePos(&shift);
	DG_GetPos(&tmpmat);
	check_pos = *(FVECTOR*)tmpmat.m[3];
#if 0
	FVECTOR verts1[2];
	FVECTOR verts2[2];
	verts1[0] = check_pos;
	verts1[1] = GM_PlayerFindPos;
	NewLineView(verts1, 1, 255, 0, 0);
	verts2[0] = check_pos;
	verts2[1] = *(FVECTOR*)GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world.m[3];
	NewLineView(verts2, 1, 255, 0, 0);
#endif

    if ( HZX_OnlineHazardCheck(0, &from_pos, &check_pos, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,	//ハザードに接触
			       (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE), (HZX_SEG_NO_BULLET&HZX_SEG_NO_MISSILE)) ){
	FVECTOR	point, direction, normal;
	HZX_HZD 	seg;
	int		tmp;
	if ((HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &to_pos, &GM_PlayerFindPos,
					       HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE )) &&
		      (HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &to_pos,
					      (FVECTOR*)GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world.m[3],
					      HZX_CHK_ALL, HZX_SEG_RECOIL_TYPE, HZX_FLOOR_RECOIL_TYPE )) ){

	printf ("Behind wall !! \n");
	}
	else {
	    GM_MoveTarget (&work->am_off_tgt, &to_pos);
	    GM_PutTarget (&work->am_off_tgt );
//	    NewTargetView2(&work->am_off_tgt, 255, 50, 50);
	}
	HZX_GetOnlineHazard( &seg, &tmp );	//ハザードを取得
	HZX_GetOnlinePoint( &point );	//接触点を取得
	HZX_GetOnlineVector( &direction );	//弾の方向ベクトルを取得
	HZX_GetNormal (&normal, &seg); 	//ハザードの法線ベクトルを取得




	// *****ミサイル爆発処理
	MissileBomb(work);

	NewHarrierMisileExplosionE3(
	    &to_pos,
	    &normal,
	    &direction );
	
	//振動
	if (!GM_CheckGameStatus(STATE_PLAY_DEMO)){
	    NewShakeCamera2(0, 512, 10, &to_pos);
	}
    }
    }




    //    work->body.objs->world.m[3][1] -= 100.0f;
    //    work->body.objs->objs[0].world.m[3][1] -= 100.0f;

    {
	FMATRIX	tmpmat;
	FVECTOR	tmpshift = {0.0f, 0.0f, 1000.0f, 1.0f};
	
	//場所を渡す
	Fvec_from_Matrix(&work->position, &work->body.objs->objs[0].world);
	
	//方向ベクトルを計算
	_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[0].world);
	tmpmat.m[ 3 ][ 0 ] = 0.0f;
	tmpmat.m[ 3 ][ 1 ] = 0.0f;
	tmpmat.m[ 3 ][ 2 ] = 0.0f;
	DG_SetPos(&tmpmat);
	DG_MovePos(&tmpshift);
	DG_GetPos(&tmpmat);
	Fvec_from_Matrix(&work->vector, &tmpmat);
	_sceVu0Normalize(&work->vector, &work->vector);
	
    }
    
    if (work->timer > 3000*5){
	// *****ミサイル爆発処理
	MissileBomb(work);
    }
    work->timer += TIME_BASE;
}

/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mis_act_none							*/
/*	引数:	Mis_Work	*work						*/
/*	説明:	非装備								*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void mis_act_none(Mis_Work *work){
    
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void mis_act_none						*/
/*	引数:	Mis_Work	*work						*/
/*	説明:	プレイヤ着団爆発						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void isogi(Mis_Work *work){
    FVECTOR		normal={0.0f, 1.0f, 0.0f, 0.0f};
    FVECTOR		direction={0.0f, 0.0f, -1.0f, 0.0f};
    FVECTOR		point;


    _sceVu0CopyVector(&point, &GM_PlayerFindPos);

    GM_MoveTarget (&work->am_off_tgt, &point);
    GM_PutTarget (&work->am_off_tgt );
    
    // *****ミサイル爆発処理
    MissileBomb(work);

    NewHarrierMisileExplosionE3(
	&point,
	&normal,
	&direction );
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void do_Act							*/
/*	引数:	Work	*work							*/
/*	説明:	指定のオブジェに追従						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Mis_Act(Mis_Work *work)
{
    switch (work->mis_mode){
	
	//	*****ミサイル装備中 *****
    case HAR_SIGNAL_EQUIP:
	mis_act_equip(work);
	work->bk_flg = OFF;
	work->am_timer = 0;
	break;
	
	//	*****ミサイル発射 *****
    case HAR_SIGNAL_FIRE:
	mis_act_equip(work);
	work->bk_flg = OFF;
	work->am_timer += TIME_BASE;		
	if (work->am_timer > 40*5){
	    work->mis_mode = HAR_SIGNAL_FIRENOW;
	    work->am_timer = 0;
	}
	break;


    case HAR_SIGNAL_FIRENOW:
	act_fire(work);
	work->am_timer = 0;	
	break;
	
	//	*****ミサイル飛行 *****
    case HAR_SIGNAL_FLY:
	if (work->bk_flg){
	    isogi(work);
	}
	else {
	    act_fly(work);
	}
	break;
	
	//	*****非装備 *****
    case HAR_SIGNAL_NONE:
	mis_act_none(work);
	break;
    }

    GM_MoveTarget2( &work->am_def_tgt, &work->body.objs->objs[0].world);

    if (work->shift_pos.vx < 0.0f){
	((Work*)work->oya_work)->r_amr_pos = *((FVECTOR*)work->body.objs->world.m[3]);
    }
    else {
	((Work*)work->oya_work)->l_amr_pos = *((FVECTOR*)work->body.objs->world.m[3]);
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
static int Mis_ReceiveSignal(void *work, int signal, int value){
    
    if (signal == GV_SIGNAL_KILL){
	((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
	GV_DestroyActor( work);
	return 1;
    }
    else if ( (signal & HAR_SIGNAL_AMRAM_MASK) == HAR_SIGNAL_AMRAM ){
	((Mis_Work*)work)->mis_mode = ((signal) & (~HAR_SIGNAL_AMRAM));
	((Mis_Work*)work)->timer = 0;
	printf ("                     *****Receive Mess:MISSILE  !!!!!\n");
	((Mis_Work *)work)->oya_speed = (int)_FVecLen3( &(((Work *)((Mis_Work *)work)->oya_work)->control.step));

	return 1;
    }
    
    return 0;
}



/**************************<-------local function------>*************************/
/*	名前:	void *Targ_MPCallBack						*/
/*	引数:	TARGET	*off							*/
/*		TARGET	*off							*/
/*		void	*ptr							*/
/*	説明:						*/
/********************************************************************************/
void *stgfunc( TARGET *off, TARGET *def, void *ptr )
{
    Mis_Work	*work ;
    printf ("MIs hit\n");

    work = ( Mis_Work * )ptr ;
    
    work->bk_flg = ON;
    return 0;
}



/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void * am_bomb_func						*/
/*	引数:	TARGET	*off							*/
/*	    	TARGET	*def							*/
/*	    	void*	ptr							*/
/*	説明:	防御ターゲット当たりのコールバック				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void * am_bomb_func( TARGET *off, TARGET *def, void *ptr ){
    Mis_Work	*work;
    work = ( Mis_Work * )ptr ;

    if ( off->class & TARGET_POWER ) {
	if( !(def->weapon_type & WP_COLDSPRAY)){
	    MissileBomb(work);
	    printf ("Hit\n");
	}
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
static int Mis_GetResources(Mis_Work *work, Work *oya_work, FVECTOR *shift_pos, int where)
{
    work->map = where;

    if (shift_pos->vx > 0.0f){
	GET_MISS_WORK_R = (void*)work;
    }
    else{
	GET_MISS_WORK_L = (void*)work;
    }

    work->oya_work = (void *)oya_work;			//親のワーク登録
    _sceVu0CopyVector(&work->shift_pos, shift_pos);	//ミサイル位置保存
    
    GM_InitObject(&work->body,
		  GV_StrCode("hri_amraam"  ),DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_FOGPARAM|DG_FLAG_IRREACTION);
    DG_SetFogParamObjs (work->body.objs, FOG_NEAR, FOG_FAR);    


    GV_SetActorSignalFunc( work, Mis_ReceiveSignal );	//シグナル受信関数設定


    // ****本体プレイヤーへのあたり
    GM_SetTarget( &work->vul_target,
		  TARGET_POWER|TARGET_OFFENSE|TARGET_ONLINE|TARGET_THROUGH,
		  where, PLAYER_SIDE,
		  &DG_ZeroVector, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->vul_target,
			  (void*)stgfunc,
			  work);

    
    // ********************************::ターゲット設定
    {
	int	flag;
	FVECTOR	targ_size = {80.0f, 80.0f, 100.0f, 0.0f};
	FVECTOR	offset = {0.0f, 0.0f, 1400.0f, 0.0f};
	//	FVECTOR	size = {2300.0f, 1000.0f, 2300.0f, 1.0f};
	FVECTOR	size = {1300.0f, 1300.0f, 1300.0f, 1.0f};
	FVECTOR	force = {250.0f, 0.0f, 0.0f, 0.0f};
	
	// ***防御ターゲット設定
	flag =(TARGET_DEFENSE|TARGET_SEEK
	       |TARGET_ROTATE
	       |TARGET_POWER);

	GM_SetTarget( &work->am_def_tgt, flag,where, ENEMY_SIDE,
		      &targ_size, &offset );
	GM_SetTargetCallBack( &work->am_def_tgt, (void *)am_bomb_func, work );
	GM_PutTarget( &work->am_def_tgt );

	//攻撃ターゲット設定
	GM_SetTarget( &work->am_off_tgt,
		      TARGET_OFFENSE,
		      where, PLAYER_SIDE,
		      &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( &work->am_off_tgt,
				WP_GRENADE);	// グレネード );
	{
	    int damage = 17;
    switch( GM_GameLevel ){
    case GM_LEVEL_VERYEASY:
	damage = 20;
	break;
    case ST_LEV_EASY:
	damage = 25;
	break;
    case ST_LEV_NORMAL:
	damage = 30;
	break;

    case ST_LEV_UPNORM:
	damage = 30;
	break;

    case GM_LEVEL_HARD:
	damage = 30;
	break;
    case GM_LEVEL_E_EXTREME:
    case GM_LEVEL_EXTREME:
	damage = 50;
	break;
    }
	GM_SetPowerTarget( &work->am_off_tgt,
			   &work->am_pow_tgt,
			   POWER_ONCE,
			   50,
			   17,
			   damage,
			   &force );
	}
	GM_SetTargetCallBack( &work->am_off_tgt,
			      (void *)dumfunc,
			      NULL);

	//ターゲット表示
	//	NewTargetView( &work->am_def_tgt, 200, 0, 0);	
    }
    return 1;
}

/*******************************<Global function>********************************/
/*	名前:	void *NewHarMissile						*/
/*	引数:	Work	*oya_work						*/
/*		FVECTOR	*shift_pos						*/
/*	説明:	ミサイルを表示							*/
/********************************************************************************/
void *NewHarMissile(Work *oya_work, FVECTOR *shift_pos, int where)
{
    Mis_Work *work ;
    work = (Mis_Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				       sizeof( Mis_Work ), 255 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),Mis_Act,Mis_Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!Mis_GetResources(work, oya_work, shift_pos, where)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}







