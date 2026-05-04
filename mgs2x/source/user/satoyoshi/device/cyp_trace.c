/********************************************************************************/
/*	cypher.c								*/
/*	サイファ	*NewCypher						*/
/*	2000/01/23 H.Satoyoshi							*/
/*	$Id: cyp_trace.c,v 1.1.1.3 2002/11/19 11:48:15 Yoshizawa1 Exp $			*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	extern									*/
/********************************************************************************/

/********************************************************************************/
/*	define      								*/
/********************************************************************************/

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void set_alltrg_skip						*/
/*	引数:	Work *work   						       	*/
/*	説明:	全てのターゲットをスキップ状態に      				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void set_alltrg_skip(Work *work){
    int loop;
    work->h_trg.class |= TARGET_SKIP;
    work->r_trg.class |= TARGET_SKIP;
    for(loop=0; loop<6; loop++){
	work->b_trg[loop].class |= TARGET_SKIP;
    }
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void unset_alltrg_skip						*/
/*	引数:	Work *work   						       	*/
/*	説明:	全てのターゲットをnotスキップ状態に    				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void unset_alltrg_skip(Work *work){
    int loop;
    work->h_trg.class &= ~TARGET_SKIP;
    work->r_trg.class &= ~TARGET_SKIP;
    for(loop=0; loop<6; loop++){
	work->b_trg[loop].class &= ~TARGET_SKIP;
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void 								*/
/*	引数:	Work *work   						       	*/
/*	説明:					       				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void cet_tmp(Work *work){
    work->rnavi.p_action = work->rnavi.pa_action[(int)work->rnavi.next_node];
    work->rnavi.p_acttime = work->rnavi.pa_time[(int)work->rnavi.next_node] ;
    work->rnavi.p_dir = work->rnavi.pa_dir[(int)work->rnavi.next_node];
    work->rnavi.p_con = work->rnavi.pa_con[(int)work->rnavi.next_node];
    work->rnavi.p_actstatus = work->rnavi.pa_flag[(int)work->rnavi.next_node];
}





/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CYP_InitTracePoint						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ツイビ用ルートを設定する		       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_InitTracePoint(Work *work) {
    ROUTENAVI 	*rnavi;
    FVECTOR	*point;
    short	loop1, loop2;

    rnavi = &work->rnavi;
    point = work->trace_point;

#ifdef DEBUG_MODE
    if (rnavi->n_nodes > MAX_POINT_NUM){
	ASSERT(0);	//ここに来てはいけない
    }
#endif
    _sceVu0CopyVector(&point[0], &rnavi->nodes[0]);//コピー
    work->tp_num = 1;

    //全てのノードをツイビルートにコピー
    for (loop1=1; loop1<rnavi->n_nodes; loop1++){
	
	//全く同じ位置のツイビルートがないか検索
	//あれば上書き
	for(loop2=0; loop2<work->tp_num; loop2++){
	    if ( (point[loop2].vx == rnavi->nodes[loop1].vx) &&
		 (point[loop2].vy == rnavi->nodes[loop1].vy) &&
		 (point[loop2].vz == rnavi->nodes[loop1].vz) ){
#ifdef DEBUG_MODE
		printf("    ****NODE %d <> %d is SAME**** \n",loop1, loop2);
#endif
		work->loop_flag = 1;	//ループフラグをつけて下ルート禁止
		break;
	    }
	}
	_sceVu0CopyVector(&point[loop2], &rnavi->nodes[loop1]);//コピー
#ifdef DEBUG_MODE
		printf(" P:%d Add  \n",loop1);
#endif
	if (loop2+1 > work->tp_num){	//ツイビルートの数＋１
	    work->tp_num ++ ;
	}
    }

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void SetPointAction						*/
/*	引数:	Work *work   						       	*/
/*	説明:	ルートポイントでのアクション等の設定				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYPSetPointAction( Work *work ){
    ROUTENAVI *rnavi;
    rnavi = &work->rnavi;

    if(rnavi->p_acttime == P_TIME_WAIT){	//メッセージ待ちの待機
	work->status = CYP_WAIT;
	work->body.objs->flag |= DG_FLAG_INVISIBLE;
	work->hom.status |= HOMING_SKIP;
	set_alltrg_skip(work);
	work->r_trg.class &= ~TARGET_LOCKON;
	work->rctrl.flag &= ~RADAR_VISIBLE;
	_sceVu0CopyVector(&work->control.step, &DG_ZeroVector);



    }

    if (0){	
	printf("	    p_action %d\n",rnavi->p_action);
	printf("            p_acttime%d\n",rnavi->p_acttime); 
	printf("            p_dir    %d\n",rnavi->p_dir);
	printf("            p_con    %d\n",rnavi->p_con); 
	printf("            p_actstat%d\n",rnavi->p_actstatus);
	printf("            Change to Wait Mode\n");
    }

    if ((work->zidouset == ON)&&(work->noreturn == OFF)){
	if ((GM_AlertMode == ALERT_MODE_SNEAK)||(GM_AlertMode == ALERT_MODE_SEARCH)){
	    short loop;
	    for (loop=1; loop<3; loop++){
		if (work->rnavi.c_route == work->zido_root[loop]){
		    
		    work->route =  work->zido_root[loop-1];
		    if (loop-1 == 0){
			work->mokuhyo_flg = BK2;
printf ("							++BK2\n");
		    }
		    else {
			work->mokuhyo_flg = BK1;
printf ("							++BK1\n");
		    }

		    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
		    work->trg.pos = work->rnavi.nodes[0] ;
		    work->trg.map = work->rnavi.mapbit[0] ;
		    work->trg.addr = HZX_GetAddress( work->trg.map,
						     &work->trg.pos, -1 ) ;
		    CYP_InitTracePoint( work );
		    
		}
	    }
	}
    }



    // *********************自動移動モード
    if ( (work->zidouset == ON)&&(work->cyp_call_flag == ON)){
	short loop;
	for (loop=1; loop>=0; loop--){
	    if (work->rnavi.c_route == work->zido_root[loop]){

		work->route =  work->zido_root[loop+1];
		if (loop+1 == 1){
		    work->mokuhyo_flg = GO1;
printf ("							++GO1\n");
		}
		else {
		    work->mokuhyo_flg = GO2;
printf ("							++GO2\n");
		}
		    

		ENE_InitRouteNavi( &work->rnavi, work->route, 0);
		CYPSetPointAction( work );
		work->trg.pos = work->rnavi.nodes[0] ;
		work->trg.map = work->rnavi.mapbit[0] ;
		work->trg.addr = HZX_GetAddress( work->trg.map,
						 &work->trg.pos, -1 ) ;
		CYP_InitTracePoint( work );
	    }
	}
    }


    rnavi->p_action = rnavi->pa_action[(int)rnavi->next_node];
    rnavi->p_acttime = rnavi->pa_time[(int)rnavi->next_node] ;
    rnavi->p_dir = rnavi->pa_dir[(int)rnavi->next_node];
    rnavi->p_con = rnavi->pa_con[(int)rnavi->next_node];
    rnavi->p_actstatus = rnavi->pa_flag[(int)rnavi->next_node];


}



/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void CYP_SetNextnode						*/
/*	引数:	ROUTENAVI	*rnavi						*/
/*		Work		*work	アクターワーク				*/
/*		char		flag	到達プロックを呼ぶかどうかのフラグ	*/
/*	説明:	ルートの次のポイントを目標地に設定				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void CYP_SetNextnode(ROUTENAVI * rnavi, Work *work, char flag)
{
    TRGPOINT	*trgp = &work->trg;

    // **方向固定フラグ
    work->rot_flag = rnavi->p_action;	//アクション番号を設定する

    // **注視点の保存
    Dir_from_2Vec(&work->camera,&rnavi->aimnodes[(int)rnavi->next_node],&work->look_dir);


    if ((work->reach_proc_id != NULL) && (flag == 1)){	// 到達プロック実行許可
	if ((rnavi->c_route == work->reach_proc_route)&&(rnavi->next_node == work->reach_proc_node)){
	    GCL_ARGS	args;
	    args.argc = 1;
	    args.argv = &work->name;
	    GCL_ExecProc(work->reach_proc_id, &args);

#ifdef DEBUG_MODE
	    printf ("Call Proc in CYP_SetNextnode !!------------------ %d\n", work->reach_proc_route);
#endif
	    
	}
    }


    ENE_SetNextnode( rnavi );		//次のノードの設定

    trgp->pos = rnavi->nodes[ (short)rnavi->next_node ] ;
    trgp->map = rnavi->mapbit[ (short)rnavi->next_node ] ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void DecayStep							*/
/*	引数:	CONTROL	*ctrl							*/
/*		float	rate							*/
/*	説明:	ベクトルの減衰/増加を行なう					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void DecayStep(CONTROL *ctrl ,float rate){
    FVECTOR *step;
    step = &ctrl->step ;

    step->vx *= rate ;
    step->vy *= rate ;
    step->vz *= rate ;
}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int DirectTrace3D						*/
/*	引数:	Work	*work	  					       	*/
/*		float	range	誤差						*/
/*		int	*brake							*/
/*	説明:	３Ｄ目標地点に移動？						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int DirectTrace3D(Work *work, float range,int *brake){

    FVECTOR		shift ,*pos;
    float		dis, accele;
    TRGPOINT *trgp ;
    CONTROL	*ctrl;


    trgp = &work->trg ;
    ctrl = &work->control ;

    pos = &ctrl->mov;
    SAT_Minus_FVECTOR(&shift, &trgp->pos, pos);

    //目標地点の方向と距離を算出
    trgp->dir = _FVecDir2( &shift ) ;
    
    accele = dis = GV_VecLen3F( &shift ) ;

    //加速度の制限
    if (accele > CYP_ACCELE){
    	GV_LenVec3F( &shift, &shift, 0.0F, CYP_ACCELE ) ;
    }

    if( dis > range ) {    //目標地点への距離がレンジ以上なら移動
	DecayStep(ctrl,DECAY_RATE) ;
	/*現在の速度	サイファの加速度が決まっている  */
	ctrl->step.vx += shift.vx;
	ctrl->step.vy += shift.vy;
	ctrl->step.vz += shift.vz;
    }
    else {    //目標地点への距離がレンジ以下なら
	DecayStep(ctrl,BRAKE_RATE2) ;





	return -1;
    }

    dis = GV_VecLen3F( &ctrl->step );
    if( dis > work->max_speed) {
	GV_LenVec3F( &ctrl->step, &ctrl->step, 0.0F, work->max_speed ) ;
    }

    return 0 ;
}




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void InLineCheck						*/
/*	引数:	FVECTOR	*answer		ある点。直接求めた座標を入れる		*/
/*		FVECTOR	*pos1	  	線分を成す２点			       	*/
/*		FVECTOR	*pos2	  		〃			       	*/
/*		short	mode		比較する値	0:	vx		*/
/*							1:	vy		*/
/*							2:	vz		*/
/*	説明:	ある点が線分の中に入るようにする				*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void InLineCheck(FVECTOR *answer, FVECTOR *pos1, FVECTOR *pos2, short mode){

    float	num1	= 0.0f;
    float	num2	= 0.0f;
    float	ans	= 0.0f;

    switch	(mode){
    case 0:	// vxの比較
	num1 = pos1->vx;
	num2 = pos2->vx;
	ans =  answer->vx;
	break;

    case 1:	// vyの比較
	num1 = pos1->vy;
	num2 = pos2->vy;
	ans =  answer->vy;
	break;

    case 2:	// vzの比較
	num1 = pos1->vz;
	num2 = pos2->vz;
	ans =  answer->vz;
	break;

#ifdef DEBUG_MODE	
    default:
	ASSERT(0);	//ここに来てはいけない
#endif
    }

    //		*****比較開始
    if (num1 < num2){
	if (ans < num1){	//pos1が最近接点
	    _sceVu0CopyVector(answer, pos1);
	}
	else if (num2 < ans){	//pos2が最近接点
	    _sceVu0CopyVector(answer, pos2);
	}
    }
    else if (num1 > num2){
	if (ans > num1){	//pos1が最近接点
	    _sceVu0CopyVector(answer, pos1);
	}
	else if (num2 > ans){	//pos2が最近接点
	    _sceVu0CopyVector(answer, pos2);
	}
    }
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void GetShortestP						*/
/*	引数:	FVECTOR	*answer		求めた座標を入れる			*/
/*		FVECTOR	*point	  					       	*/
/*		FVECTOR	*pos1	  					       	*/
/*		FVECTOR	*pos2	  					       	*/
/*										*/
/*	説明:	pos1,pos2の2点間を結ぶ直線にpointから下ろす垂線の足の座標	*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma optimize( "", off )
static void GetShortestP(FVECTOR *answer, FVECTOR *point, FVECTOR *pos1, FVECTOR *pos2){

    FVECTOR	vecV0_p1_pot;		//pos1 > point の方向ベクトル [V0]
    FVECTOR	vecV1_p1_p2;		//pos1 > pos2  の方向ベクトル [V1]
    FVECTOR	OutProOV0_V0V1;		//方向ベクトル V0 と V1 の外積 [OV0]
    FVECTOR	OutProOV1_OV0V0;	//外積 OV0 と 方向ベクトル V1 の外積 [OV1]
    float	block1, block2, block3;

    //		****直線がZ軸だった場合
    if ((pos1->vx == pos2->vx)&&(pos1->vy == pos2->vy)){
	answer->vx = pos1->vx;
	answer->vy = pos1->vy;
	answer->vz = point->vz;
	InLineCheck(answer, pos1, pos2, 2);	//vzの比較
    }
    //		****直線がY軸だった場合
    else if ((pos1->vx == pos2->vx)&&(pos1->vz == pos2->vz)){
	answer->vx = pos1->vx;
	answer->vz = pos1->vz;
	answer->vy = point->vy;
	InLineCheck(answer, pos1, pos2, 1);	//vyの比較
    }
    //		****直線がX軸だった場合
    else if ((pos1->vz == pos2->vz)&&(pos1->vy == pos2->vy)){
	answer->vz = pos1->vz;
	answer->vy = pos1->vy;
	answer->vx = point->vx;
	InLineCheck(answer, pos1, pos2, 0);	//vxの比較
    }
    //		****マトモな計算
    else {

//		***pos1 > point の方向ベクトル [V0]***
    SAT_Minus_FVECTOR(&vecV0_p1_pot, point, pos1);
//		***pos1 > pos2  の方向ベクトル [V1]***
    SAT_Minus_FVECTOR(&vecV1_p1_p2, pos2, pos1);
//		***方向ベクトル V0 と V1 の外積 [OV0]***
    _sceVu0OuterProduct(&OutProOV0_V0V1, &vecV0_p1_pot, &vecV1_p1_p2);
//		***外積 OV0 と 方向ベクトル V1 の外積 [OV1]***
    _sceVu0OuterProduct(&OutProOV1_OV0V0, &OutProOV0_V0V1, &vecV1_p1_p2);

#ifdef KP_XBOX
    if ( OutProOV1_OV0V0.vx == 0.0f ) OutProOV1_OV0V0.vx = 0.00001f ;
    if ( OutProOV1_OV0V0.vy == 0.0f ) OutProOV1_OV0V0.vy = 0.00001f ;
    if ( OutProOV1_OV0V0.vz == 0.0f ) OutProOV1_OV0V0.vz = 0.00001f ;
#endif

//	プレイヤのいる位置 点 point から pos1 を含み法線 OutProOv1_OV0V0 を持つ
//      平面に垂線を下ろした場合の垂線の足と平面の交点を求める
//      以下は Ｘ の値
//    (a^2 + b^2 + c^2) X = a (ap + bq + cr - bt - cu) + s (b^2 + c^2)
//     |----block3----|        |-------block2--------|     |-block2--| :STEP1
//     |----block3----|     |---------block2---------|   |---block2--| :STEP2
//     |----block3----|     |------------------block2----------------| :STEP3

//		****STEP1****
    block1 = (OutProOV1_OV0V0.vx * pos1->vx) +
	     (OutProOV1_OV0V0.vy * pos1->vy) +
	     (OutProOV1_OV0V0.vz * pos1->vz) -
	     (OutProOV1_OV0V0.vy * point->vy) -
	     (OutProOV1_OV0V0.vz * point->vz);

    block2 = (OutProOV1_OV0V0.vy * OutProOV1_OV0V0.vy) +
	     (OutProOV1_OV0V0.vz * OutProOV1_OV0V0.vz);
        
    block3 = (OutProOV1_OV0V0.vx * OutProOV1_OV0V0.vx) +
	     (OutProOV1_OV0V0.vy * OutProOV1_OV0V0.vy) +
	     (OutProOV1_OV0V0.vz * OutProOV1_OV0V0.vz);

//		****STEP2****
//	block1に a を掛ける
    block1 = block1 * OutProOV1_OV0V0.vx;
//	block2に s を掛ける
    block2 = block2 * point->vx;

//		****STEP3****
    block2 = block2 + block1;


//	Ｘの解
#ifdef KP_XBOX
    if ( block3 == 0.0f ) block3 = 0.00001f ;
#endif
    block3 = block2 / block3;

//	Ｙの解	Ｘを代入して求める(ひょっとしら誤差が大きくなる？)
//	Y = b (x - s) / a + t
    block2 = (OutProOV1_OV0V0.vy*(block3 - point->vx))/OutProOV1_OV0V0.vx + point->vy;

//	Zの解	Ｘを代入して求める(ひょっとしら誤差が大きくなる？)
//	Z = c (x - s) / a + u
    block1 = (OutProOV1_OV0V0.vz*(block3 - point->vx))/OutProOV1_OV0V0.vx + point->vz;

    answer->vx = block3;
    answer->vy = block2; 
    answer->vz = block1;
    }


    //		****線分の外ではないかのチェック
    InLineCheck(answer, pos1, pos2, 0);	//vxの比較


}
#pragma optimize( "", on )




/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int 	check_zidou_node					*/
/*	引数:	Work	*work	  					       	*/
/*	説明:	自動ノード移動							*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void check_zidou_node(Work *work){
    // *********************自動移動モード
    if (work->zidouset == ON){
	
	if ((GM_AlertMode == ALERT_MODE_ALERT)||(GM_AlertMode == ALERT_MODE_AVOID)||
	    (work->cyp_call_flag == ON)){
	    short loop;
	    for (loop=1; loop>=0; loop--){
		if (work->rnavi.c_route == work->zido_root[loop]){

		    work->route =  work->zido_root[loop+1];
		    if (loop+1 == 1){
			work->mokuhyo_flg = GO1;
printf ("							++GO1\n");
		    }
		    else {
			work->mokuhyo_flg = GO2;
printf ("							++GO2\n");
		    }
		    

		    ENE_InitRouteNavi( &work->rnavi, work->route, 0);
		    CYPSetPointAction( work );
		    work->trg.pos = work->rnavi.nodes[0] ;
		    work->trg.map = work->rnavi.mapbit[0] ;
		    work->trg.addr = HZX_GetAddress( work->trg.map,
						     &work->trg.pos, -1 ) ;
		    CYP_InitTracePoint( work );
		    
		}
	    }
	}
    }
}

#define	NONE_MOVE	0
#define	PLUS_MOVE	1
#define	MINUS_MOVE	2
#define CHANGE_NODE_DIST	1800
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int TracePlayer3D						*/
/*	引数:	Work	*work	  					       	*/
/*	説明:	ルート上でプレイヤを追跡移動					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int TracePlayer3D(Work *work, FVECTOR *temppos){
    
    FVECTOR	*target_pos, *cyp_pos;
    FVECTOR	tmp;
    FVECTOR	shift;
    short	loop;
    int		min_dist = 0;	//最近接ノードまでの距離距離
    char	min_num = 0;	//最短距離ノードナンバー
    char	now_num = 0;	//現在ノード間ナンバー
    short	node0 = 0;
    short	node1 = 0;    
    int		distance = 0;
    FVECTOR	answer_point = { 0 };
    
    tmp = *temppos;

    tmp.vy = tmp.vy += 500.0f;	//基本的にプレイヤより上に陣どる
    target_pos	= &tmp;
    // ****************サイファの位置******************
    cyp_pos	= &work->control.mov;
    
    
    /**************************サイファがどの点から一番近いか算出**************************/
    {	/***********線分上のどの点が近いか************/
	FVECTOR 	ans;
	FVECTOR		sht;
	short		nd0, nd1;    
	
	for (loop=0; loop<work->tp_num; loop++){
	    if (loop!=0){	//全てのノードからの距離を計算
		GetShortestP(&ans, cyp_pos,
			     &work->trace_point[loop],
			     &work->trace_point[loop-1]);
		nd0 = loop;
		nd1 = loop-1;
	    }
	    else {
		GetShortestP(&ans, cyp_pos,
			     &work->trace_point[0],
			     &work->trace_point[work->tp_num-1]);
		nd0 = 0;
		nd1 = work->tp_num-1;

		if (work->loop_flag == 1){	//ループしていたら０番無効
		    ans.vy -= 9000000.0f;
		}

	    }
	    SAT_Minus_FVECTOR(&sht, &ans, cyp_pos);	//距離を算出

#ifdef SATO_DEBUG
    MENU_Locate( 30, 50+loop*20, 0 ) ;
    MENU_SetColor( 150, 0, 0 ) ;
    MENU_Printf( "%d :: %d",
		 loop,
		 (int)_FVecLen3( &sht)
		 );
#endif

	    
	    //最小値の比較・保存
	    if ( (loop==0)||(min_dist > (int)_FVecLen3( &sht) )){
		now_num = loop;
		min_dist = (int)_FVecLen3( &sht);
		answer_point.vx = ans.vx;
		answer_point.vy = ans.vy;
		answer_point.vz = ans.vz;
		node0 = nd0;
		node1 = nd1;
	    }
	}
    }
    
    {	/***********結局どの点が近いか************/
	short dist1, dist2;
	SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);	//距離を算出
	dist1 = (int)_FVecLen3( &shift);
	
	SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出
	dist2 = (int)_FVecLen3( &shift);
	
	if (dist1 < dist2){
	    work->rnavi.next_node = node0 ;
	    
	    // **************ノードに到着
	    if (dist1 < CHANGE_NODE_DIST){
		if (work->reach_proc_id != NULL){		//到達プロック
		    if ((work->rnavi.c_route == work->reach_proc_route)&&
			(work->rnavi.next_node == work->reach_proc_node)){
			GCL_ARGS	args;
			args.argc = 1;
			args.argv = &work->name;
			GCL_ExecProc(work->reach_proc_id, &args);
#ifdef DEBUG_MODE
			printf ("Call Proc in dist1 !!\n");
#endif
		    }
		}
			// ********自動ノード移動
			check_zidou_node(work);
	    }
	}
	else {
	    work->rnavi.next_node = node1 ;
	    
	    // **************ノードに到着
	    if (dist2 < CHANGE_NODE_DIST){
		if (work->reach_proc_id != NULL){		//到達プロック
		    if ((work->rnavi.c_route == work->reach_proc_route)&&
			(work->rnavi.next_node == work->reach_proc_node)){
			GCL_ARGS	args;
			args.argc = 1;
			args.argv = &work->name;
			GCL_ExecProc(work->reach_proc_id, &args);
#ifdef DEBUG_MODE
			printf ("Call Proc in dist2 !!\n");
#endif
		    }
		}
		// ********自動ノード移動
		check_zidou_node(work);
	    }
	}
	work->rnavi.p_action = 0;	//アクション番号なし
    }
    
    //@@@@
    work->trg.pos = work->rnavi.nodes[ (short)work->rnavi.next_node ] ;
    
    
    //課題:サイファからの距離 で ノードが一個の場合
    
    
    /****プレイヤからどの点が一番近いか算出****/
    //   *****************************************ノードが一個だった場合
    if (work->tp_num == 1){
	FVECTOR		sht;
	SAT_Minus_FVECTOR(&sht, &work->trace_point[0], target_pos);	//距離を算出
	min_num = 0;
	min_dist = (int)_FVecLen3( &sht);
	answer_point.vx = work->trace_point[0].vx;
	answer_point.vy = work->trace_point[0].vy;
	answer_point.vz = work->trace_point[0].vz;
	/**一番近い点とサイファの距離**/
	SAT_Minus_FVECTOR(&shift, &answer_point, cyp_pos);	//距離を算出
	distance = (int)_FVecLen3( &shift);
	
    }
    else {	/****プレイヤからどの点が一番近いか算出****/
	FVECTOR 	ans;
	FVECTOR		sht;
	
	for (loop=0; loop<work->tp_num; loop++){
	    if (loop!=0){
		GetShortestP(&ans, target_pos,
			     &work->trace_point[loop],
			     &work->trace_point[loop-1]);
	    }
	    else {
		GetShortestP(&ans, target_pos,
			     &work->trace_point[0],
			     &work->trace_point[work->tp_num-1]);
		if (work->loop_flag == 1){	//ループしていたら０番無効
		    ans.vy = ans.vy - 9000000.0f;
		}
	    }
	    
	    if (ans.vy < target_pos->vy ){	//絶対下にはいかせない
		
	    }
	    
	    SAT_Minus_FVECTOR(&sht, &ans, target_pos);	//距離を算出
	    
	    if ( (loop==0)||(min_dist > (int)_FVecLen3( &sht) )){//最小値の比較・保存
		min_num = loop;
		min_dist = (int)_FVecLen3( &sht);
		answer_point.vx = ans.vx;
		answer_point.vy = ans.vy;
		answer_point.vz = ans.vz;
	    }
	}

	// ********特別高高度ルート
	answer_point.vy += 1.0f*work->high_root;


	/**一番近い点とサイファの距離**/
	SAT_Minus_FVECTOR(&shift, &answer_point, cyp_pos);	//距離を算出
	distance = (int)_FVecLen3( &shift);
    }
    

    {
	FVECTOR shift00;
	int	distance;

	//	sat_viewp(&answer_point, 255, 0, 0);
	SAT_Minus_FVECTOR(&shift00, &answer_point, &GM_PlayerFindPos);
	distance = (int)_FVecLen2( &shift00);
	//	printf("D:%d\n", distance);


	if (distance <= 2000){
	    int dis00, dis01;

	    _sceVu0SubVector (&shift00, &work->trace_point[node0], &answer_point);
	    dis00 = (int)_FVecLen2( &shift00);

	    _sceVu0SubVector (&shift00, &work->trace_point[node1], &answer_point);
	    dis01 = (int)_FVecLen2( &shift00);

	    if (dis00 > dis01){
		_sceVu0SubVector (&shift00, &work->trace_point[node0], &answer_point);
		_sceVu0Normalize(&shift00, &shift00);
		_sceVu0ScaleVector(&shift00, &shift00, 2000.0f);
		_sceVu0AddVector(&answer_point, &answer_point, &shift00);
		SAT_Minus_FVECTOR(&shift, &answer_point, cyp_pos);	//距離を算出
		//		sat_viewp(&answer_point, 255, 255, 0);
		distance = (int)_FVecLen3( &shift);
	    }
	    else {
		_sceVu0SubVector (&shift00, &work->trace_point[node1], &answer_point);
		_sceVu0Normalize(&shift00, &shift00);
		_sceVu0ScaleVector(&shift00, &shift00, 2000.0f);
		_sceVu0AddVector(&answer_point, &answer_point, &shift00);
		SAT_Minus_FVECTOR(&shift, &answer_point, cyp_pos);	//距離を算出
		//		sat_viewp(&answer_point, 255, 255, 0);
		distance = (int)_FVecLen3( &shift);
	    }
	}
    }
    




    // ********ルートを真面目に巡ってツイビ
    if ((min_num != now_num)&&(work->skip_flag == 0)&&(work->tp_num != 1)){
	if ( ((min_num > now_num)&&((min_num-now_num)*2<work->tp_num)) ||
	     ((min_num < now_num)&&((now_num-min_num)*2>work->tp_num))) 
	    
	{	//正順廻り
	    SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);


	    _sceVu0CopyVector(&answer_point, &work->trace_point[node0]);


	    distance = (int)_FVecLen3( &shift);

#ifdef SATO_DEBUG
	MENU_Locate( 50, 150, 0 ) ;
	MENU_SetColor( 150, 0, 0 ) ;
	MENU_Printf( "M:%d  N:%d",
		     min_num,
		     now_num);
#endif
	    
	    if (distance < CHANGE_NODE_DIST){//ノードに到着
		node0 = node0+1;
		if (node0 >= work->tp_num){
		    node0 = 0;
		}
		SAT_Minus_FVECTOR(&shift, &work->trace_point[node0], cyp_pos);	//距離を算出

	    _sceVu0CopyVector(&answer_point, &work->trace_point[node0]);


		distance = (int)_FVecLen3( &shift);
	    }
	    
	}
	else {
	    SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出

	    _sceVu0CopyVector(&answer_point, &work->trace_point[node1]);

	    distance = (int)_FVecLen3( &shift);
	    
	    if (distance < CHANGE_NODE_DIST){//ノードに到着
		node1 = node1-1;
		if (node1 < 0){
		    node1 = work->tp_num-1;
		}
		SAT_Minus_FVECTOR(&shift, &work->trace_point[node1], cyp_pos);	//距離を算出

		_sceVu0CopyVector(&answer_point, &work->trace_point[node1]);

		distance = (int)_FVecLen3( &shift);
	    }
	    
	}
	
    }
    //		***************近接追尾***************
    else {
    }
    
    //		***************実際の移動***************
    if (distance > CYP_ACCELE){    //加速度の制限
	GV_LenVec3F( &shift, &shift, 0.0F, CYP_ACCELE ) ;
    }
    


    //  **加速つき過ぎてルート外れそし
    {
	FVECTOR	tmppos, tmpsht;
	
	_sceVu0CopyVector(&tmppos,&work->control.mov);
	tmppos.vx += work->control.step.vx;
	tmppos.vy += work->control.step.vy;
	tmppos.vz += work->control.step.vz;
	SAT_Minus_FVECTOR(&tmpsht, &answer_point, &tmppos);
	
	if (((int)_FVecLen3( &tmpsht)>distance)||
	    (HZX_NearHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ),
				  &work->control.mov, 2000.0f, HZX_CHK_ALL|HZX_CHK_RECOIL_TYPE,
				  HZX_SEG_ALL, 0.0f))){ 
	    DecayStep(&work->control,0.8f);
	}
    }

    DecayStep(&work->control,DECAY_RATE) ;

    //  **加速つき過ぎてルート外れそし
    if (distance > (int)_FVecLen3( &work->control.step)*15+600){
	work->control.step.vx += shift.vx;
	work->control.step.vz += shift.vz;
	work->control.step.vy += shift.vy;
    }
    else {
	DecayStep(&work->control,BRAKE_RATE2) ;
    }

    if ((int)_FVecLen3( &work->control.step) > work->max_speed){	//速度制限
	GV_LenVec3F( &work->control.step, &work->control.step, 0.0F, work->max_speed) ;
    }
    
    return 1;
}

#ifdef DEBUG_MODE
extern void *NewLineView(FVECTOR * ,int,u_char,u_char,u_char) ;
#endif
/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	int MoveRoute							*/
/*	引数:	Work *work   						       	*/
/*	説明:	ルートを廻る				       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int MoveRoute(Work *work){
    ROUTENAVI *rnavi ;
    FVECTOR	aim;
    int brake = 0;
    rnavi = &work->rnavi ;


    /*移動制御と到達チェック*/
    if(DirectTrace3D(work,1000.0F,&brake) < 0 ){
	/*ポイント到達*/
	if(rnavi->p_acttime == 0 ){

	    /*待機時間無しなのですぐに次ポイントへ*/
	    work->nowpos = work->trg.pos ;
	    CYP_SetNextnode( rnavi, work, 1 ) ;
	    //	printf("d\n");
	    CYPSetPointAction( work );
	}else {
	    /*待機モードへ*/
	    work->mode = ACTION_MODE ;
	    
	    // printf("TM %d\n", rnavi->p_acttime);

	}
    }else {
	/*移動中の顔方向チェック*/
	/*発見していなければ*/
	/*注視点見ながら移動*/
	if(rnavi->p_actstatus & PA_CON_AIM_FACE_MOVE){
	    aim = rnavi->aimnodes[(int)rnavi->next_node];
	    Dir_from_2Vec(&work->camera,&aim,&work->control.turn);
	}
	else if(work->rot_flag == CYP_FLAG_FIXROT){ 

	    work->control.turn.vx = work->look_dir.vx;
	    work->control.turn.vy = work->look_dir.vy;
	    work->control.turn.vz = work->look_dir.vz;
	}
	else {
	    /*進行方向*/
	    work->control.turn.vy = work->trg.dir ;
	    work->control.turn.vx = 0 ;
	}
    }
    return	brake ;
}


/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void ActionSeq							*/
/*	引数:	Work *work   						       	*/
/*	説明:	時間が来るまで待機						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void ActionSeq(Work *work)
{
    if(work->rnavi.p_dir >=0 ) {
	if(work->rot_flag != CYP_FLAG_FIXROT){ 
	    work->control.turn.vy = work->rnavi.p_dir ;
	    work->control.turn.vx = 0 ;
	}
    }
    if((work->rnavi.p_acttime>0)
       &&(work->rnavi.p_acttime != P_TIME_WAIT)){

	/*待機時間中*/
	work->rnavi.p_acttime--;
    }
    else if((work->rnavi.p_acttime <= 0)
	     &&(work->rnavi.p_acttime != P_TIME_WAIT)){

	/*待機終了 次のポイントセット*/
	work->nowpos = work->trg.pos ;
	CYP_SetNextnode( &work->rnavi, work,1 ) ;
	//	printf("e\n");	
	CYPSetPointAction( work );
	work->mode = MOVE_MODE ;
    }
    /*メッセージ受信するまで待機*/	
    else if (work->rnavi.p_acttime == P_TIME_WAIT){

	if(work->status == CYP_ACTIVE ){
	    /*待機終了 次のポイントセット*/
	    work->nowpos = work->trg.pos ;
	    CYP_SetNextnode( &work->rnavi, work, 1 ) ;
	    //	printf("f\n");
	    CYPSetPointAction( work );
	    work->mode = MOVE_MODE ;
	}
    }
}
