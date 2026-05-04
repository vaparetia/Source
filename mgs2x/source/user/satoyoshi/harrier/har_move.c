/********************************************************************************/
/*	har_move.c								*/
/*	ハリア移動								*/
/*	2001/04/19 H.Satoyoshi							*/
/*	$Id: har_move.c,v 1.1.1.3 2002/11/19 11:48:22 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/

/********************************************************************************/
/*	define									*/
/********************************************************************************/
#define HROT_L		(work->rot_speed)
#define ROT_SPEEDH	(40)

/********************************************************************************/
/*	Program									*/
/********************************************************************************/


/*******************************<Global function>********************************/
/*	名前:	void Har_SetHokanMove						*/
/*	引数:	Work	*work   					       	*/
/*		FMATRIX *goal_mat 目標地点/向き				       	*/
/*		int	 step	  補間総ステップ数			       	*/
/*	説明:	補間移動設定				       			*/
/********************************************************************************/
void Har_SetHokanMove(Work *work, FMATRIX* goal_mat, int step)
{
    _sceVu0CopyMatrix(&work->from_mat, &work->body.objs->objs[0].world );
    //    work->from_mat.vw = 1.0f;
    work->end_frame = step; 

}

/*******************************<Global function>********************************/
/*	名前:	void Har_HokanMove						*/
/*	引数:	Work	*work   					       	*/
/*		FMATRIX *goal_mat 目標地点/向き				       	*/
/*		int	 step	  補間ステップ				       	*/
/*	説明:	クォータニオン補間移動			       			*/
/********************************************************************************/
void Har_HokanMove(FMATRIX* ans_mat, Work *work, FMATRIX* goal_mat, int step)
{
    FVECTOR	now_quat;
    FVECTOR	to_quat;

    // ***********一旦全てクォータニオンに変換
    MT_MatToQuat( &now_quat, &work->from_mat );
    MT_MatToQuat( &to_quat, goal_mat );

    // ***********マトリックスに戻す
    MT_QuatSlerp( &to_quat,
		  &now_quat, &to_quat,
		  (1.0f-sinf( (M_PI*step/work->end_frame)-M_PI/2.0f))/2.0f
	);			//補間
    MT_QuatNormalize( &to_quat, &to_quat);	//正規化
    MT_QuatToMat( ans_mat, &to_quat);	//回転部分について解を保存

#if 0
    // **********内挿ベクトルを計算してそのまま代入
    _sceVu0InterVector(
	(FVECTOR*)(ans_mat->m[3]),			//出力
	(FVECTOR*)(goal_mat->m[3]),			//先
	(FVECTOR*)(work->body.objs->objs[0].world.m[3]),//元
	1.0f/step );
#endif


    // **********内挿ベクトルを計算してそのまま代入
    _sceVu0InterVector(
	(FVECTOR*)(ans_mat->m[3]),			//出力
	(FVECTOR*)(goal_mat->m[3]),			//先
	(FVECTOR*)(work->from_mat.m[3]),		//元
	(1.0f-sinf( (M_PI*step/work->end_frame)-M_PI/2.0f))/2.0f
	);

    //    printf ("f           %f \n",(1.0f-sinf( (M_PI*step/work->end_frame)-M_PI/2.0f))/2.0f );

    _sceVu0CopyVector(&work->control.mov, (FVECTOR*)(ans_mat->m[3]));

}

#if 0
/*******************************<Global function>********************************/
/*	名前:	void Har_HokanMove						*/
/*	引数:	Work	*work   					       	*/
/*		FMATRIX *goal_mat 目標地点/向き				       	*/
/*		int	 step	  補間ステップ				       	*/
/*	説明:	クォータニオン補間移動			       			*/
/********************************************************************************/
void Har_BurnMove(FMATRIX* ans_mat, Work *work, FMATRIX* goal_mat, int step)
{
    FVECTOR	now_quat;
    FVECTOR	to_quat;

    // ***********一旦全てクォータニオンに変換
    MT_MatToQuat( &now_quat, &work->body.objs->objs[0].world );
    MT_MatToQuat( &to_quat, goal_mat );

    // ***********マトリックスに戻す
    MT_QuatSlerp( &to_quat,
		  &now_quat, &to_quat,
		  1.0f/step );	//補間

    MT_QuatNormalize( &to_quat, &to_quat);	//正規化
    MT_QuatToMat( ans_mat, &to_quat);	//回転部分について解を保存


#if 0
    // **********内挿ベクトルを計算してそのまま代入
    _sceVu0InterVector(
	(FVECTOR*)(ans_mat->m[3]),			//出力
	(FVECTOR*)(goal_mat->m[3]),			//先
	(FVECTOR*)(work->body.objs->objs[0].world.m[3]),//元
	1.0f/step );
#endif


    // **********内挿ベクトルを計算してそのまま代入
    _sceVu0InterVector(
	(FVECTOR*)(ans_mat->m[3]),			//出力
	(FVECTOR*)(goal_mat->m[3]),			//先
	(FVECTOR*)(work->body.objs->objs[0].world.m[3]),//元
	( 1.0f-cosf( M_PI*2.0f* step/(40*5) ) )/2.0f 
	);

    _sceVu0CopyVector(&work->control.mov, (FVECTOR*)(ans_mat->m[3]));

}
#endif


/*******************************<Global function>********************************/
/*	名前:	void Har_Move2Goal						*/
/*	引数:	Work *work   						       	*/
/*	説明:	補間移動				       			*/
/********************************************************************************/
void Har_Move2Goal(Work *work)
{
    SVECTOR	goal_dir;
    FVECTOR	speed = {0.0f, 0.0f, work->max_speed, 1.0f};
    short	z_rot;
    FVECTOR	verts0[2];	
    FVECTOR	verts1[2];
    
    // *******加速減速
    if (work->accel_speed != 0){
	work->max_speed += (work->target_speed - work->max_speed)/(work->accel_speed*1.0f);
	work->accel_speed--;
    }

    {// ***********************Ｚ軸回転
	float	dir ;
	float	kyori;
	FVECTOR	distance;
	
	SAT_Minus_FVECTOR(&distance, &work->goal_pos, &work->control.mov);
	kyori = _FVecLen2( &distance );
	dir = atan2f( 
	    kyori,
	    distance.vy);
	z_rot = 4095 & ( short )( ( dir * 2048.0F / PI ) + 0.5F ) ; //* 四捨五入 */
    }
    
    
    {// *********************移動方向 ＸＹ方向
	short	bunbo;
	short	maxx, maxy;
	
	//方向を計算して求める
	Dir_from_2Vec(&work->control.mov, &work->goal_pos, &goal_dir);
	//現在方向との差をとる
	goal_dir.vx = goal_dir.vx - work->control.rot.vx;
	goal_dir.vy = goal_dir.vy - work->control.rot.vy;
	
	if (goal_dir.vx > 2048){	//Ｘ方向
	    goal_dir.vx -= 4096;
	}
	else if (goal_dir.vx < -2048){
	    goal_dir.vx += 4096;
	}
	if (goal_dir.vy > 2048){	//Ｙ方向
	    goal_dir.vy -= 4096;
	}
	else if (goal_dir.vy < -2048){
	    goal_dir.vy += 4096;
	}
	
	maxx = goal_dir.vx;	//最大値を保存
	maxy = goal_dir.vy;
	
	//回転の比率
	bunbo = abs(goal_dir.vx) + abs(goal_dir.vy);
	if (bunbo != 0){
	    goal_dir.vx = HROT_L * goal_dir.vx / bunbo;
	    goal_dir.vy = HROT_L * goal_dir.vy / bunbo;
	}
	//MAX回転でリミットをかける
	if ( ((maxx > 0)&&(goal_dir.vx > maxx)) ||
	     ((maxx < 0)&&(goal_dir.vx < maxx)) ){
	    goal_dir.vx = maxx;
	}
	if ( ((maxy > 0)&&(goal_dir.vy > maxy)) ||
	     ((maxy < 0)&&(goal_dir.vy < maxy)) ){
	    goal_dir.vy = maxy;
	}
    }
    
    //********尾翼を動かす
    if (goal_dir.vy){
	if (work->talewing_rot < 0.05f){
	    work->talewing_rot = work->talewing_rot + 0.010f;
	}
    }
    else {
	work->talewing_rot = work->talewing_rot * 0.96f;
    }
    
    work->control.turn.vx = work->control.rot.vx + goal_dir.vx;
    work->control.turn.vy = work->control.rot.vy + goal_dir.vy;
    
    if (work->control.turn.vx < -2048){
	work->control.turn.vx += 4096;
    }
    if (work->control.turn.vy < -2048){
	work->control.turn.vy += 4096;
    }
    if (work->control.turn.vx > 2048){
	work->control.turn.vx -= 4096;
    }
    if (work->control.turn.vy > 2048){
	work->control.turn.vy -= 4096;
    }
    
    
    //*************回転方向指定が入っている
    if (work->z_turn_speed){
	work->control.turn.vz += work->z_turn_speed;
	work->control.interp = 2;
    }
    //*************旋回方向に対応した傾き
    else if ((abs(goal_dir.vx)+abs(goal_dir.vy)) > 60){
	short	rotation;
	
	//逆廻りの時は逆向き
	if (goal_dir.vy > 0){
	    z_rot = -z_rot;
	}
	
	rotation = z_rot - work->control.turn.vz;
	//回転速度設定
	rotation %= 4096;
	if (rotation > 2048){
	    rotation = -ROT_SPEEDH;
	}
	if (rotation < -2048){
	    rotation = ROT_SPEEDH;
	}
	if (rotation > ROT_SPEEDH){
	    rotation = ROT_SPEEDH;
	}
	if (rotation < -ROT_SPEEDH){
	    rotation = -ROT_SPEEDH;
	}
	
	//********エルロンを動かす
	if (rotation > 0){
	    if (work->l_elr_rot > -0.2f){
		work->l_elr_rot = work->l_elr_rot - 0.11f;
		work->r_elr_rot = work->r_elr_rot - 0.11f;
	    }
	}
	else if (rotation < 0){
	    if (work->l_elr_rot < 0.2f){
		work->l_elr_rot = work->l_elr_rot + 0.11f;
		work->r_elr_rot = work->r_elr_rot + 0.11f;
	    }
	}
	else {
	    work->l_elr_rot = work->l_elr_rot * 0.96f;
	    work->r_elr_rot = work->r_elr_rot * 0.96f;
	}
	work->control.turn.vz += rotation;
    }
    //*************元の傾きに戻る
    else {
	if (work->control.turn.vz > 2048){
	    work->control.turn.vz = work->control.turn.vz-4096;
	}

	if (work->control.turn.vz < -2048){
	    work->control.turn.vz = work->control.turn.vz+4096;
	}

	work->control.turn.vz = work->control.turn.vz * 0.9;

	work->l_elr_rot = work->l_elr_rot * 0.96f;
	work->r_elr_rot = work->r_elr_rot * 0.96f;
	work->lader_rot = work->lader_rot * 0.98f; 
	//パタパタ
	if (work->l_elr_rot > 0.0f){
	    work->l_elr_rot = work->l_elr_rot - 0.1f;
	}
	else if (work->l_elr_rot < 0.0f){
	    work->l_elr_rot = work->l_elr_rot + 0.1f;
	}
	if (work->r_elr_rot > 0.0f){
	    work->r_elr_rot = work->r_elr_rot - 0.1f;
	}
	else if (work->r_elr_rot < 0.0f){
	    work->r_elr_rot = work->r_elr_rot + 0.1f;
	}
    }
    
    if (work->control.turn.vz > 10){
	//垂直尾翼
	if (work->lader_rot < 0.15f){
	    work->lader_rot = work->lader_rot + 0.08f; 
	}
    }
    else if (work->control.turn.vz < -10){
	//垂直尾翼
	if (work->lader_rot > -0.15f){
	    work->lader_rot = work->lader_rot - 0.08f; 
	}
    }
    else {
	work->lader_rot = work->lader_rot * 0.98f; 
    }
    
    
    if (0){ // work->har_flag & HAR_DOUSA_FLAG_DEBUG){
	MENU_Locate( 10, 127, 0 );	//実移動量
	MENU_Printf( "ShifDir:  %5d, %5d, %5d ",
		     goal_dir.vx,
		     goal_dir.vy,
		     goal_dir.vz );
	MENU_Locate( 10, 144, 0 );	//目標角度
	MENU_Printf( "To--Dir:  %5d, %5d, %5d ",
		     work->control.turn.vx,
		     work->control.turn.vy,
		     work->control.turn.vz );
	MENU_Locate( 10, 161, 0 );	//現在角度
	MENU_Printf( "Rot-Dir:  %5d, %5d, %5d ",
		     work->control.rot.vx,
		     work->control.rot.vy,
		     work->control.rot.vz );
    }
    
    
    //**************スピードの計算
    {
	FMATRIX	tmpmat;
	_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[0].world);
	
	tmpmat.m[ 3 ][ 0 ] = 0.0f;
	tmpmat.m[ 3 ][ 1 ] = 0.0f;
	tmpmat.m[ 3 ][ 2 ] = 0.0f;
	
	DG_SetPos(&tmpmat);
	DG_MovePos(&speed);
	DG_GetPos(&tmpmat);
	Fvec_from_Matrix( &speed, &tmpmat );
    }
    _sceVu0CopyVector(&work->control.step, &speed);

    //    if (work->har_flag & HAR_DOUSA_FLAG_DEBUG){    //ライン引き
    {
	FMATRIX	tmpmat;
	FVECTOR	line = {0.0f, 0.0f, 30000.0f, 1.0f};	
	
	_sceVu0CopyVector(&verts0[0], &work->control.mov);
	_sceVu0CopyVector(&verts0[1], &work->goal_pos);
	//		NewLineView(verts0, 1, 200,200,200);
	
	_sceVu0CopyMatrix(&tmpmat, &work->body.objs->objs[0].world);
	DG_SetPos(&tmpmat);
	DG_MovePos(&line);
	DG_GetPos(&tmpmat);
	Fvec_from_Matrix( &line, &tmpmat );
	_sceVu0CopyVector(&verts1[0], &line);
	_sceVu0CopyVector(&verts1[1], &work->control.mov);
	//		NewLineView(verts1, 1, 255,50,50);
    }
		//    }

}

