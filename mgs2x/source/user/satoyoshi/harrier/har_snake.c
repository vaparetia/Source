/********************************************************************************/
/*	har_kasacka.c								*/
/*	カサッカ								*/
/*	2001/05/03 H.Satoyoshi							*/
/*	$Id: */
/********************************************************************************/

/********************************************************************************/
/*	Program									*/
/********************************************************************************/

/* cos 45度/2 */
//#define COS_ADJUST_LR_LIMIT			0.923879533f
#define COS_ADJUST_LR_LIMIT			(cosf(M_PI*7.0f/24.0f))
/* sin 45度/2 */
//#define SIN_ADJUST_LR_LIMIT			0.382683432f
#define SIN_ADJUST_LR_LIMIT			(sinf(M_PI*7.0f/24.0f))

#define ADJUST_PARAM_1		0.25f
#define ADJUST_PARAM_0		0.1f





/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	CalcAdjustUD							*/
/*	引数:	Kas_Work	*work						*/
/*		int		flag	フラグ					*/
/*	説明:	上下の狙いを定める						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void CalcAdjustUD(Kas_Work *work,int flag)
{
	DG_OBJS *objs=work->snake.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	FMATRIX m;
	int i;
	int parent;
	float param;

	if(flag){
		vu0_Ldv0(work->homing_p);
		vu0_Ldv1((FVECTOR *)&(objs->objs[HUMAN21_KUBI].world.m[3][0]));
		vu0_Subv2v0v1();
		vu0_Stv0(&vec1);
		vu0_Stv1(&vec0);
		vu0_Stv2(&vec2);
		vec2.vy=0.0f;
		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);
		param=ADJUST_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;
		param=ADJUST_PARAM_0;
	}
	MT_QuatSlerp(&q,&(work->pre_adjust0),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust0),&q);
	MT_QuatToMat(&m,&q);
	vu0_Ldm0(&m);
	for(i=HUMAN21_MUNE;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs->objs[i].world));
		vu0_Mulm2m0m1();
		vu0_Stm2(&(objs->objs[i].world));
	}
	i=HUMAN21_MUNE;
	parent=objs->objs[i].parent;
	vu0_Ldv0(&(objs->objs[i].trans));
	vu0_Ldm0(&(objs->objs[parent].world));
	vu0_Setv0w1();
	i++;
	for( ;i<=HUMAN21_ATAMA;i++){
		vu0_Mulv1m0v0();
		parent=objs->objs[i].parent;
		vu0_Ldv0(&(objs->objs[i].trans));
		vu0_Stv1((FVECTOR *)&(objs->objs[i-1].world.m[3][0]));
		vu0_Ldm0(&(objs->objs[parent].world));
		vu0_Setv0w1();
	}
	vu0_Mulv1m0v0();
	vu0_Stv1((FVECTOR *)&(objs->objs[i-1].world.m[3][0]));
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	CalcAdjustLR							*/
/*	引数:	Kas_Work	*work						*/
/*		int		flag	フラグ					*/
/*	説明:	左右の狙いを定める						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static inline void CalcAdjustLR(Kas_Work *work,int flag)
{
	DG_OBJS *objs=work->snake.objs;
	FVECTOR vec0,vec1,vec2,q,qq;
	FMATRIX m;
	int i;
	int parent;
	float param;
	if(flag){
		fpu_CopyVector(&vec1,work->homing_p);
		fpu_CopyVector(&vec0,(FVECTOR *)&(objs->world.m[3][0]));
		fpu_CopyVector(&vec2,(FVECTOR *)&(objs->objs[HUMAN21_ATAMA].world.m[2][0]));
		vec0.vy=0.0f;
		vec1.vy=0.0f;
		vec2.vy=0.0f;
		UTL_MakeQuat(&qq,&vec0,&vec1,&vec2);
		/* 曲がり過ぎに注意 */
		if(qq.vw<COS_ADJUST_LR_LIMIT && flag==1){
			float s=fpu_Rsqrt(1.0f-qq.vw*qq.vw,SIN_ADJUST_LR_LIMIT);
			qq.vx*=s;
			qq.vy*=s;
			qq.vz*=s;
			qq.vw=COS_ADJUST_LR_LIMIT;
		}
		param=ADJUST_PARAM_1;
	}
	else{
		fpu_ClearVector(&qq);
		qq.vw=1.0f;
		param=ADJUST_PARAM_0;
	}
	MT_QuatSlerp(&q,&(work->pre_adjust1),&qq,param);
	MT_QuatNormalize(&q,&q);
	fpu_CopyVector(&(work->pre_adjust1),&q);
	MT_QuatToMat(&m,&q);
	vu0_Ldm0(&m);
	for(i=HUMAN21_ONAKA;i<=HUMAN21_ATAMA;i++){
		vu0_Ldm1(&(objs->objs[i].world));
		vu0_Mulm2m0m1();
		vu0_Stm2(&(objs->objs[i].world));
	}
	i=HUMAN21_ONAKA;
	parent=objs->objs[i].parent;
	vu0_Ldv0(&(objs->objs[i].trans));
	vu0_Ldm0(&(objs->objs[parent].world));
	vu0_Setv0w1();
	i++;
	for( ;i<=HUMAN21_ATAMA;i++){
		vu0_Mulv1m0v0();
		parent=objs->objs[i].parent;
		vu0_Ldv0(&(objs->objs[i].trans));
		vu0_Stv1((FVECTOR *)&(objs->objs[i-1].world.m[3][0]));
		vu0_Ldm0(&(objs->objs[parent].world));
		vu0_Setv0w1();
	}
	vu0_Mulv1m0v0();
	vu0_Stv1((FVECTOR *)&(objs->objs[i-1].world.m[3][0]));
}


extern void MazzleM4A1_Snk( FMATRIX *, int, int );
extern void CartridgeM4A1_SNK( FMATRIX *, OBJECT *, CONTROL * );
extern void MazzleM4demo_Snk( FMATRIX *, int, int );

#define	EFFECT_FLAGS_ON_TMP	(0x08)
#define MT3_PLAYEND		(0x00000001)		/* 最終フレームセット完了フラグ */

extern void *NewBullet( FMATRIX*, u_int, u_int, u_int, u_int, u_int, u_int, int);
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void snake_act							*/
/*	引数:	Work	*work							*/
/*	説明:	スネークのアクション						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void snake_act(Kas_Work *work){

    switch(work->snake_motion){

    case SET_SNK_M4FIRE:	//Ｍ４射撃中
    {
	work->m4gun_timer += TIME_BASE;
	if (work->m4gun_timer>25){
	    work->m4gun_timer -= 25;
	    MazzleM4A1_Snk( &work->m4gl.objs->world, 0, EFFECT_FLAGS_ON_TMP);	//マズルフラッシュ
	    CartridgeM4A1_SNK( &work->m4gl.objs->world , &work->m4gl, NULL );

	    GM_SeSetMode (SD_E_KA_GUN01, (FVECTOR*)work->m4gl.objs->world.m[3], GM_SEMODE_BOMB);	//スネーク機銃音

	    if (RAND(2)==0){		//	弾と軌跡発生
		SVECTOR rot;
		FMATRIX tmpmat;
		FVECTOR shift = {0.0f, -500.0f, 0.0f, 1.0f};
		int type = (BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_NO_PLAYER);

		Dir_from_2Vec( (FVECTOR*)work->m4gl.objs->world.m[3], &work->aimtarget, &rot);
		rot.vx -= 1024;
		DG_SetPos2((FVECTOR*)work->m4gl.objs->world.m[3], &rot);
		DG_MovePos(&shift);

		rot.vx = RAND(2);
		rot.vz = RAND(2);
		rot.vy = RAND(4096);

		DG_RotatePos (&rot);
		DG_GetPos(&tmpmat);
		NewBullet( &tmpmat, type, BOTH_SIDE, 25, 5, 100000, 2000, WP_m4 );
	    }

	}
	if (work->set_snake != SET_SNK_ENABLE){		//変更指示
        if(GM_CheckObject_IsEnd(&work->snake, 0)){	//モーション変更なら
		GM_ConfigObjectAction(&(work->snake), 0, MODE_SNK_M4END, 0, 0xfffff, 0);
		work->snake_motion = SET_SNK_M4END;	//Ｍ４射撃終了
	    }
	}
	// ** 怒りのＭ４射撃
	if (work->m4time>0){
	    work->m4time -= TIME_BASE;
	    if (work->m4time<=0){
		work->m4time=0;
		GM_ConfigObjectAction(&(work->snake), 0, MODE_SNK_M4END, 0, 0xfffff, 0);
		work->snake_motion = SET_SNK_M4END;	//Ｍ４射撃終了
		work->snake_ikari=0;
	    }
	}
    }
    break;

    case SET_SNK_GLFIRE:
	// *********グレネード発射
	if (work->m4gun_timer == 0){
	    MazzleM4demo_Snk( &work->m4gl.objs->world, 0, EFFECT_FLAGS_ON_TMP);
	    work->m4gun_timer += TIME_BASE;
	    shoot_grn(work);

	    // **怒りを鎮める
	    if (work->snake_ikari>0){
		work->snake_ikari -= 4;
		if (work->snake_ikari<=0){
		    work->snake_ikari=0;
		}
	    }
	}
	if(GM_CheckObject_IsEnd(&work->snake, 0)){	//モーション変更なら
	    GM_ConfigObjectAction(&(work->snake), 0, SET_SNK_IDEL, 0, 0xfffff, 40);
	    work->snake_motion = SET_SNK_IDEL;	//待機状態
	}
	break;

    case SET_SNK_M4END:
	if(GM_CheckObject_IsEnd(&work->snake, 0)){	//モーション変更なら
	    GM_ConfigObjectAction(&(work->snake), 0, SET_SNK_IDEL, 0, 0xfffff, 0);
	    work->snake_motion = SET_SNK_IDEL;	//待機状態
	}
	break;

    case SET_SNK_IDEL:
	if (work->snk_mtk_time == 0){
	if (work->set_snake != SET_SNK_ENABLE){		//変更指示
	    if (work->set_snake == SET_SNK_ITEM){
		GM_ConfigObjectAction(&(work->snake), 0, work->set_snake, 0, 0xfffff, 40);
	    }
	    else {
		GM_ConfigObjectAction(&(work->snake), 0, work->set_snake, 0, 0xfffff, 0);
	    }

	    // ****アイテムなげなら一緒に箱も発動
	    if (work->set_snake == SET_SNK_ITEM){
		GM_ConfigObjectAction(&work->hako, 0, 1, 		  0, 0xfffff, 0);
		
#if 0
vc104518	//弾だ,受けとれ			05
vc104519	//ライデン,受けとれ		06
vc104520	//こいつを使え			07
#endif
 //  ***** ついでに喋っておこうか知らん
		if (work->str_hdl_snk == 0){
		    if (KAS_ITEM_NUM==0){
//			work->str_hdl_snk = GM_VoxStream(work->str_id[6+RAND(2)], 0);
			work->str_hdl_snk = GM_VoxStream(work->str_id[6+RAND(2)], GM_STREAM_FLAG_3D);
		    }
		    else {
			if (RAND(2)){
//			    work->str_hdl_snk = GM_VoxStream(work->str_id[5], 0);
			    work->str_hdl_snk = GM_VoxStream(work->str_id[5], GM_STREAM_FLAG_3D);
			}
			else {
//			    work->str_hdl_snk = GM_VoxStream(work->str_id[5+RAND(3)], 0);
			    work->str_hdl_snk = GM_VoxStream(work->str_id[5+RAND(3)], GM_STREAM_FLAG_3D);
			}
		    }
		}
                kas_call_event_proc(work, 10);
		
	    }

	    work->m4gun_timer = 0;
	    work->snake_motion = work->set_snake;
	    work->set_snake = SET_SNK_ENABLE;
	}
	}
	break;

    case SET_SNK_ITEM:
	if(GM_CheckObject_IsEnd(&work->snake, 0)){	//モーション変更なら
	    GM_ConfigObjectAction(&(work->snake), 0, SET_SNK_IDEL, 0, 0xfffff, 0);
	    work->snake_motion = SET_SNK_IDEL;	//待機状態
	}
	break;

    case SET_SNK_DAM:
	// *********やられ
	GM_ConfigObjectAction(&(work->snake), 0, SET_SNK_DAM, 0, 0xfffff, 0);
 	work->snake_motion = SET_SNK_DAMING;	//待機状態
	break;

    case SET_SNK_DAMING:	// ダメージ中
	if(GM_CheckObject_IsEnd(&work->snake, 0)){	//モーション変更なら
	    GM_ConfigObjectAction(&(work->snake), 0, SET_SNK_IDEL, 0, 0xfffff, 40);
	    work->snake_motion = SET_SNK_IDEL;	//待機状態
	}
	break;

    }

}






